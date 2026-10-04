#include "r_model_pose.h"
#include <xanim/xmodel.h>
#include <xanim/dobj_utils.h>
#include "r_dvars.h"
#include <cgame_mp/cg_pose_mp.h>
#include <EffectsCore/fx_marks.h>

float4 g_worldMins = { { -131072.0, -131072.0, -131072.0, 0.0 } }; // BO1TODO: better spot
float4 g_worldMaxs = { { 131072.0, 131072.0, 131072.0, 0.0 } }; // BO1TODO: better spot


DObjAnimMat * R_UpdateSceneEntBounds(
                GfxSceneEntity *sceneEnt,
                GfxSceneEntity **pLocalSceneEnt,
                const DObj **pObj,
                int waitForCullState)
{
    // aislop!
    float radiusSqMaxs;
    float radiusSqMins;
    float radiusHi;
    float radiusLo;
    float *maxs;
    float *mins;
    int v_neg[12]; // sign mask temporaries
    float v42, v43, v44, v45, v46, v47; // bone bounds
    XBoneInfo *boneInfo;
    DObjSkelMat boneAxis;
    DObjAnimMat *bone;
    int boneIndex;
    unsigned int animPartBit;
    int boneCount;
    XBoneInfo *boneInfoArray[160];
    float4 minWorld;
    float4 maxWorld;
    DObjAnimMat *boneMatrix;
    int surfaceCount;
    int partBits[5];
    int offscreenCull;
    const DpvsPlane *frustumPlanes;
    int frustumPlaneIdx;
    int frustumPlaneCount;
    DpvsView *dpvsView;
    bool offscreen;
    DObj *obj;
    GfxSceneEntity *localSceneEnt;
    unsigned int state;

    // Bitmask min/max helpers
    // selects a or b per-component based on sign mask
    // min: pick whichever is smaller
    // max: pick whichever is larger
    // implemented via integer bitmask trick: (a & mask) | (b & ~mask)
    // where mask = -1 if condition true, 0 otherwise

    if (InterlockedCompareExchange((volatile LONG *)&sceneEnt->cull, 1, 0))
    {
        *pLocalSceneEnt = 0;
        if (waitForCullState)
        {
            do
            {
                state = sceneEnt->cull.state;
                iassert(state >= CULL_STATE_BOUNDED_PENDING);
            } while (state == CULL_STATE_BOUNDED_PENDING);

            if (state == CULL_STATE_DONE)
            {
                return 0;
            }
            else
            {
                localSceneEnt = sceneEnt;
                *pLocalSceneEnt = sceneEnt;
                obj = localSceneEnt->obj;
                *pObj = obj;
                iassert(obj);
                return I_dmaGetDObjSkel(obj);
            }
        }
        else
        {
            return 0;
        }
    }
    else
    {
        localSceneEnt = sceneEnt;
        *pLocalSceneEnt = sceneEnt;
        iassert(localSceneEnt->obj);
        obj = localSceneEnt->obj;
        *pObj = obj;
        iassert(obj);

        // New in this version: offscreen LOD scaling
        offscreen = false;
        if (r_offscreenCasterLodScale->current.value > 1.0f)
        {
            dpvsView = dpvsGlob.views[scene.dpvs.localClientNum];
            if (localSceneEnt->cull.state >= CULL_STATE_BOUNDED)
            {
                frustumPlaneCount = dpvsView->frustumPlaneCount;
                frustumPlaneIdx = 0;
                frustumPlanes = dpvsView->frustumPlanes;
                offscreenCull = 0;
                while (frustumPlaneIdx < frustumPlaneCount)
                {
                    if (R_DpvsPlaneMaxSignedDistToBox(frustumPlanes, localSceneEnt->cull.mins) <= 0.0f)
                    {
                        offscreenCull = 1;
                        break;
                    }
                    ++frustumPlaneIdx;
                    ++frustumPlanes;
                }
                if (offscreenCull)
                    offscreen = true;
            }
        }

        DObjGetSurfaceData(
            obj,
            localSceneEnt->placement.base.origin,
            localSceneEnt->placement.scale,
            offscreen,
            localSceneEnt->cull.lods);

        if (useFastFile->current.enabled || !DObjBad(obj))
        {
            if (obj->localModels)
                surfaceCount = DObjGetSurfaces(obj, partBits, localSceneEnt->cull.lods);
            else
                surfaceCount = 0;

            if (!surfaceCount)
                goto LABEL_NODRAW;

            // New in this version: recorded DObj path
            if (DObjIsRecorded(obj) && DObjGetRotTransArray(obj))
            {
                boneMatrix = I_dmaGetDObjSkel(obj);
                DObjGetSetBones(obj, partBits);
                DObjSkelSetSkel(obj, partBits);
            }
            else
            {
                boneMatrix = R_DObjCalcPose(localSceneEnt, obj, partBits);
            }

            if (boneMatrix)
            {
                iassert(DObjSkelAreBonesUpToDate(obj, partBits));

                // New in this version: use g_worldMaxs/g_worldMins instead of hardcoded 131072
                minWorld = g_worldMaxs;
                maxWorld = g_worldMins;

                DObjGetBoneInfo(obj, boneInfoArray);
                boneCount = DObjNumBones(obj);
                animPartBit = 0x80000000;
                boneIndex = 0;

                while (boneIndex < boneCount)
                {
                    if ((animPartBit & partBits[boneIndex >> 5]) != 0)
                    {
                        bone = &boneMatrix[boneIndex];

                        // New in this version: ConvertQuatToSkelMat instead of manual quat math
                        ConvertQuatToSkelMat(bone, &boneAxis);

                        boneInfo = boneInfoArray[boneIndex];
                        v42 = boneInfo->bounds[0][0];
                        v43 = boneInfo->bounds[0][1];
                        v44 = boneInfo->bounds[0][2];
                        v45 = boneInfo->bounds[1][0];
                        v46 = boneInfo->bounds[1][1];
                        v47 = boneInfo->bounds[1][2];

                        // New in this version: SIMD-style bitmask min/max
                        // Computes transformed AABB corners using sign-bit selection
                        // for each axis component, select bounds[0] or bounds[1]
                        // based on sign of the axis component, then accumulate

                        // maxWorld corner (pick larger bound per axis sign)
                        float maxX =
                            (boneAxis.axis[2][0] >= 0.0f ? v47 : v44) * boneAxis.axis[2][0] +
                            (boneAxis.axis[1][0] >= 0.0f ? v46 : v43) * boneAxis.axis[1][0] +
                            (boneAxis.axis[0][0] >= 0.0f ? v45 : v42) * boneAxis.axis[0][0] +
                            boneAxis.origin[0];
                        float maxY =
                            (boneAxis.axis[2][1] >= 0.0f ? v47 : v44) * boneAxis.axis[2][1] +
                            (boneAxis.axis[1][1] >= 0.0f ? v46 : v43) * boneAxis.axis[1][1] +
                            (boneAxis.axis[0][1] >= 0.0f ? v45 : v42) * boneAxis.axis[0][1] +
                            boneAxis.origin[1];
                        float maxZ =
                            (boneAxis.axis[2][2] >= 0.0f ? v47 : v44) * boneAxis.axis[2][2] +
                            (boneAxis.axis[1][2] >= 0.0f ? v46 : v43) * boneAxis.axis[1][2] +
                            (boneAxis.axis[0][2] >= 0.0f ? v45 : v42) * boneAxis.axis[0][2] +
                            boneAxis.origin[2];
                        float maxW =
                            (boneAxis.axis[2][3] >= 0.0f ? v47 : v44) * boneAxis.axis[2][3] +
                            (boneAxis.axis[1][3] >= 0.0f ? v46 : v43) * boneAxis.axis[1][3] +
                            (boneAxis.axis[0][3] >= 0.0f ? v45 : v42) * boneAxis.axis[0][3] +
                            boneAxis.origin[3];

                        // minWorld corner (pick smaller bound per axis sign — swap bounds)
                        float minX =
                            (boneAxis.axis[2][0] >= 0.0f ? v44 : v47) * boneAxis.axis[2][0] +
                            (boneAxis.axis[1][0] >= 0.0f ? v43 : v46) * boneAxis.axis[1][0] +
                            (boneAxis.axis[0][0] >= 0.0f ? v42 : v45) * boneAxis.axis[0][0] +
                            boneAxis.origin[0];
                        float minY =
                            (boneAxis.axis[2][1] >= 0.0f ? v44 : v47) * boneAxis.axis[2][1] +
                            (boneAxis.axis[1][1] >= 0.0f ? v43 : v46) * boneAxis.axis[1][1] +
                            (boneAxis.axis[0][1] >= 0.0f ? v42 : v45) * boneAxis.axis[0][1] +
                            boneAxis.origin[1];
                        float minZ =
                            (boneAxis.axis[2][2] >= 0.0f ? v44 : v47) * boneAxis.axis[2][2] +
                            (boneAxis.axis[1][2] >= 0.0f ? v43 : v46) * boneAxis.axis[1][2] +
                            (boneAxis.axis[0][2] >= 0.0f ? v42 : v45) * boneAxis.axis[0][2] +
                            boneAxis.origin[2];
                        float minW =
                            (boneAxis.axis[2][3] >= 0.0f ? v44 : v47) * boneAxis.axis[2][3] +
                            (boneAxis.axis[1][3] >= 0.0f ? v43 : v46) * boneAxis.axis[1][3] +
                            (boneAxis.axis[0][3] >= 0.0f ? v42 : v45) * boneAxis.axis[0][3] +
                            boneAxis.origin[3];

                        if (minX < minWorld.v[0]) minWorld.v[0] = minX;
                        if (minY < minWorld.v[1]) minWorld.v[1] = minY;
                        if (minZ < minWorld.v[2]) minWorld.v[2] = minZ;
                        if (minW < minWorld.v[3]) minWorld.v[3] = minW;

                        if (maxX > maxWorld.v[0]) maxWorld.v[0] = maxX;
                        if (maxY > maxWorld.v[1]) maxWorld.v[1] = maxY;
                        if (maxZ > maxWorld.v[2]) maxWorld.v[2] = maxZ;
                        if (maxW > maxWorld.v[3]) maxWorld.v[3] = maxW;
                    }
                    ++boneIndex;
                    animPartBit = (animPartBit << 31) | (animPartBit >> 1);
                }

                mins = localSceneEnt->cull.mins;
                mins[0] = minWorld.v[0];
                mins[1] = minWorld.v[1];
                mins[2] = minWorld.v[2];

                maxs = localSceneEnt->cull.maxs;
                maxs[0] = maxWorld.v[0];
                maxs[1] = maxWorld.v[1];
                maxs[2] = maxWorld.v[2];

                iassert(localSceneEnt->cull.state == CULL_STATE_BOUNDED_PENDING);

                // New in this version: radius validation
                radiusLo = obj->radius - 16.0f;
                if (radiusLo < 0.0f) radiusLo = 0.0f;
                radiusHi = obj->radius + 16.0f;
                radiusSqMins = Vec3DistanceSq(localSceneEnt->placement.base.origin, localSceneEnt->cull.mins);
                radiusSqMaxs = Vec3DistanceSq(localSceneEnt->placement.base.origin, localSceneEnt->cull.maxs);
                float radiusSqActual = radiusSqMins > radiusSqMaxs ? radiusSqMins : radiusSqMaxs;
                if ((radiusLo * radiusLo) > radiusSqActual || radiusSqActual > (radiusHi * radiusHi))
                    localSceneEnt->obj->radius = sqrtf(radiusSqActual);

                localSceneEnt->cull.state = CULL_STATE_BOUNDED;
                return boneMatrix;
            }
            else
            {
            LABEL_NODRAW:
                R_SetNoDraw(sceneEnt);
                return 0;
            }
        }
        else
        {
            R_SetNoDraw(sceneEnt);
            return 0;
        }
    }
}

DObjAnimMat *__cdecl R_DObjCalcPose(const GfxSceneEntity *sceneEnt, DObj *obj, int *partBits)
{
    int i; // [esp+0h] [ebp-1Ch]
    int completePartBits[5]; // [esp+8h] [ebp-14h] BYREF

    if ( !sceneEnt
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_model_pose.cpp", 49, 0, "%s", "sceneEnt") )
    {
        __debugbreak();
    }
    if ( !obj && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_model_pose.cpp", 50, 0, "%s", "obj") )
        __debugbreak();
    for ( i = 0; i < 5; ++i )
        completePartBits[i] = partBits[i];
    return CG_DObjCalcPose(sceneEnt->info.pose, obj, completePartBits);
}

void __cdecl R_SetNoDraw(GfxSceneEntity *sceneEnt)
{
    if ( sceneEnt->cull.state != 1
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_model_pose.cpp",
                    71,
                    0,
                    "%s\n\t(sceneEnt->cull.state) = %i",
                    "(sceneEnt->cull.state == CULL_STATE_BOUNDED_PENDING)",
                    sceneEnt->cull.state) )
    {
        __debugbreak();
    }
    sceneEnt->cull.state = 4;
}

void __cdecl R_UpdateGfxEntityBoundsCmd(GfxSceneEntity **data)
{
    const DObj *obj; // [esp+0h] [ebp-10h] BYREF
    GfxSceneEntity *localSceneEnt; // [esp+4h] [ebp-Ch] BYREF
    GfxSceneEntity *sceneEnt; // [esp+8h] [ebp-8h]
    GfxSceneEntity **pSceneEnt; // [esp+Ch] [ebp-4h]
    int savedregs; // [esp+10h] [ebp+0h] BYREF

    localSceneEnt = 0;
    obj = 0;
    if ( !data && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_model_pose.cpp", 413, 0, "%s", "data") )
        __debugbreak();
    pSceneEnt = data;
    sceneEnt = *data;
    if ( R_UpdateSceneEntBounds(sceneEnt, &localSceneEnt, &obj, 0)
        && !localSceneEnt
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_model_pose.cpp", 423, 0, "%s", "localSceneEnt") )
    {
        __debugbreak();
    }
}

