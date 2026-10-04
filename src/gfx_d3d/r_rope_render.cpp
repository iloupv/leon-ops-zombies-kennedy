#include "r_rope_render.h"
#include "fxprimitives.h"
#include <xanim/xanim.h>
#include "r_drawsurf.h"
#include "r_model_lighting.h"
#include <universal/com_workercmds.h>
#include "r_primarylights.h"

const unsigned __int16 templateIndices_high[24] =
{
  0u,
  1u,
  5u,
  1u,
  6u,
  5u,
  1u,
  2u,
  6u,
  2u,
  7u,
  6u,
  2u,
  3u,
  7u,
  3u,
  8u,
  7u,
  3u,
  4u,
  8u,
  4u,
  9u,
  8u
};

const unsigned __int16 templateIndices_low[6] =
{ 0u, 1u, 2u, 1u, 3u, 2u };


float gRopeLOD = 150.0f;
GfxLightingInfo lightingInfo_0;
RopeRenderInfo g_ropeInfo;

void __cdecl R_Rope_GenerateVerts_Camera(RopeGenerateVertsCmd *cmd)
{
    R_Rope_GenerateVerts_Internal(cmd, 0, 9u, 0);
}

// Pack a unit vector into a PackedUnitVec using the fast inline approximation.
// This uses a fixed w=63 (scale factor) which is adequate for rope normals/tangents.
static PackedUnitVec Rope_PackUnitVec(const float *dir)
{
    PackedUnitVec result;
    result.array[0] = (int)(float)(dir[0] * 127.0f + 127.5f);
    result.array[1] = (int)(float)(dir[1] * 127.0f + 127.5f);
    result.array[2] = (int)(float)(dir[2] * 127.0f + 127.5f);
    result.array[3] = 63;
    return result;
}

// Pack a (u, v) texcoord pair into a PackedTexCoords using the engine's standard format.
// u_bits and v_bits are the float-as-int bit patterns (via COERCE_INT) of the texcoord values.
static PackedTexCoords Rope_PackTexCoord(int u_bits, int v_bits)
{
    float texcoord[2];
    *(int *)&texcoord[0] = v_bits;
    *(int *)&texcoord[1] = u_bits;
    return Vec2PackTexCoords(texcoord);
}

// Fill a GfxPackedVertex for a rope cross-section point.
static void Rope_EmitVert(
    GfxPackedVertex *vert,
    const float *pos,
    const float *normal,
    const float *tangent,
    GfxColor color,
    int ropeTextureUDist,
    int vTexcoordBits)
{
    vert->xyz[0] = pos[0];
    vert->xyz[1] = pos[1];
    vert->xyz[2] = pos[2];
    vert->binormalSign = 1.0f;
    vert->color = color;
    vert->normal = Rope_PackUnitVec(normal);
    vert->tangent = Rope_PackUnitVec(tangent);
    vert->texCoord = Rope_PackTexCoord(ropeTextureUDist, vTexcoordBits);
}

// aislop cleanup
void __cdecl R_Rope_GenerateVerts_Internal(
                const RopeGenerateVertsCmd *cmd,
                char sceneIndex,
                unsigned int drawsurfRegion,
                bool forceLowResLOD)
{
    R_BeginCodeMeshVerts();

    float (*segVerts)[3] = 0;

    if ( !cmd && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_rope_render.cpp", 88, 0, "%s", "cmd") )
        __debugbreak();
    if ( !cmd->ropeInfo
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_rope_render.cpp", 89, 0, "%s", "cmd->ropeInfo") )
    {
        __debugbreak();
    }

    RopeRenderInfo *ropeInfo = cmd->ropeInfo;

    float viewOrg[3];
    viewOrg[0] = cmd->vieworg[0];
    viewOrg[1] = cmd->vieworg[1];
    viewOrg[2] = cmd->vieworg[2];

    for ( int vropeIter = 0; vropeIter != ropeInfo->ropeCount; ++vropeIter )
    {
        const VisualRope *vrope = &ropeInfo->ropes[vropeIter];
        if ( ((1 << sceneIndex) & vrope->visibilityFlags) == 0 )
            continue;

        float beginRadius = vrope->beginRadius;
        float endRadius = vrope->endRadius;
        int segCount = vrope->segmentCount;
        segVerts = (float (*)[3])vrope->segmentVerts;

        // First and last segment positions
        float beamWorldBegin[3];
        beamWorldBegin[0] = segVerts[0][0];
        beamWorldBegin[1] = segVerts[0][1];
        beamWorldBegin[2] = segVerts[0][2];

        float beamWorldEnd[3];
        beamWorldEnd[0] = segVerts[segCount][0];
        beamWorldEnd[1] = segVerts[segCount][1];
        beamWorldEnd[2] = segVerts[segCount][2];

        // Compute bounding sphere center and half-extent for lighting/LOD
        float ropeCenter[3];
        ropeCenter[0] = (beamWorldBegin[0] + beamWorldEnd[0]) * 0.5f;
        ropeCenter[1] = (beamWorldBegin[1] + beamWorldEnd[1]) * 0.5f;
        ropeCenter[2] = (beamWorldBegin[2] + beamWorldEnd[2]) * 0.5f;

        float halfExtent[3];
        halfExtent[0] = (beamWorldBegin[0] - beamWorldEnd[0]) * 0.5f;
        halfExtent[1] = (beamWorldBegin[1] - beamWorldEnd[1]) * 0.5f;
        halfExtent[2] = (beamWorldBegin[2] - beamWorldEnd[2]) * 0.5f;

        float boundingRadius = sqrtf(
            halfExtent[0] * halfExtent[0]
            + halfExtent[1] * halfExtent[1]
            + halfExtent[2] * halfExtent[2]);

        float cameraDist = sqrtf(
            (viewOrg[0] - ropeCenter[0]) * (viewOrg[0] - ropeCenter[0])
            + (viewOrg[1] - ropeCenter[1]) * (viewOrg[1] - ropeCenter[1])
            + (viewOrg[2] - ropeCenter[2]) * (viewOrg[2] - ropeCenter[2]));

        unsigned int primaryLightIndex = R_GetNonSunPrimaryLightForBox(0, ropeCenter, halfExtent);
        if ( !primaryLightIndex )
            primaryLightIndex = rgp.world->sunPrimaryLightIndex;

        unsigned int lightHandle = R_AllocModelLighting_PrimaryLight(
                                        ropeCenter,
                                        primaryLightIndex,
                                        0,
                                        vrope->lightingHandle,
                                        &lightingInfo_0);
        if ( !lightHandle )
            continue;

        // LOD decision: use low-res (2 verts per ring) or high-res (5 verts per ring)
        bool useLowResLOD = forceLowResLOD || (cameraDist - boundingRadius) > gRopeLOD;
        int SLICE_COUNT = useLowResLOD ? 1 : 4;
        int TRI_COUNT = 2 * SLICE_COUNT;
        int INDEX_COUNT = 6 * SLICE_COUNT;
        const unsigned __int16 *templateIndices = useLowResLOD ? templateIndices_low : templateIndices_high;

        int indexCount = INDEX_COUNT * segCount;
        int vertexCount = (SLICE_COUNT + 1) * (segCount + 1);

        r_double_index_t *baseIndices;
        unsigned __int16 baseVertex;
        unsigned int argOffset;

        if ( !R_ReserveCodeMeshIndices(indexCount, &baseIndices)
            || !R_ReserveCodeMeshVerts(vertexCount, &baseVertex)
            || !R_ReserveCodeMeshArgs(1, &argOffset) )
        {
            break;
        }

        // Fill index buffer
        r_double_index_t *indices = baseIndices;
        int indexPairIter = 0;
        for ( int segIter = 0; segIter < segCount; ++segIter )
        {
            unsigned __int16 segBaseOffset = segIter * (SLICE_COUNT + 1);
            for ( int i = 0; i < INDEX_COUNT; i += 2 )
            {
                r_double_index_t workingIndex;
                workingIndex.value[0] = segBaseOffset + templateIndices[i] + baseVertex;
                workingIndex.value[1] = segBaseOffset + templateIndices[i + 1] + baseVertex;
                indices[indexPairIter++] = workingIndex;
            }
        }
        if ( indexPairIter != indexCount / 2
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_rope_render.cpp",
                        225,
                        0,
                        "%d,%d",
                        indexPairIter,
                        indexCount / 2) )
        {
            __debugbreak();
        }

        // Fill vertex buffer
        GfxPackedVertex *verts = R_GetCodeMeshVerts(baseVertex);
        float prevPos[3];
        prevPos[0] = beamWorldBegin[0];
        prevPos[1] = beamWorldBegin[1];
        prevPos[2] = beamWorldBegin[2];

        GfxColor lerpedColor;
        lerpedColor.packed = -1;
        float ropeTexU = 0.0f;

        for ( int segIter = 0; segIter <= segCount; ++segIter )
        {
            float alpha = (float)segIter / (float)segCount;
            float curRadius = (endRadius - beginRadius) * alpha + beginRadius;
            float invHalfCircumTimes4 = 0.25f / (curRadius * 3.1415927f);

            // Current segment position
            float basePos[3];
            basePos[0] = segVerts[segIter][0];
            basePos[1] = segVerts[segIter][1];
            basePos[2] = segVerts[segIter][2];

            // Compute segment direction (tangent along rope)
            float segDir[3];
            if ( segIter == 0 )
            {
                // First segment: forward difference to next segment
                segDir[0] = segVerts[1][0] - basePos[0];
                segDir[1] = segVerts[1][1] - basePos[1];
                segDir[2] = segVerts[1][2] - basePos[2];
            }
            else if ( segIter == segCount )
            {
                // Last segment: backward difference from previous
                segDir[0] = basePos[0] - segVerts[segIter - 1][0];
                segDir[1] = basePos[1] - segVerts[segIter - 1][1];
                segDir[2] = basePos[2] - segVerts[segIter - 1][2];
            }
            else
            {
                // Interior segment: central difference
                segDir[0] = segVerts[segIter + 1][0] - segVerts[segIter - 1][0];
                segDir[1] = segVerts[segIter + 1][1] - segVerts[segIter - 1][1];
                segDir[2] = segVerts[segIter + 1][2] - segVerts[segIter - 1][2];
            }
            Vec3NormalizeTo(segDir, segDir);

            // Camera-to-vertex vector
            float cameraVec[3];
            cameraVec[0] = basePos[0] - viewOrg[0];
            cameraVec[1] = basePos[1] - viewOrg[1];
            cameraVec[2] = basePos[2] - viewOrg[2];

            // Perpendicular direction: cross(segDir, cameraVec), gives the "horizontal" of the cross-section
            float perpDir[3];
            Vec3Cross(segDir, cameraVec, perpDir);
            Vec3NormalizeTo(perpDir, perpDir);

            // Perpendicular up direction: cross(segDir, perpDir), gives the "vertical" of the cross-section
            float perpUpDir[3];
            Vec3Cross(segDir, perpDir, perpUpDir);
            Vec3NormalizeTo(perpUpDir, perpUpDir);

            // Accumulate U texture distance based on segment length
            float stepVec[3];
            stepVec[0] = basePos[0] - prevPos[0];
            stepVec[1] = basePos[1] - prevPos[1];
            stepVec[2] = basePos[2] - prevPos[2];
            float stepLen = Vec3Length(stepVec);
            ropeTexU += stepLen * invHalfCircumTimes4;

            prevPos[0] = basePos[0];
            prevPos[1] = basePos[1];
            prevPos[2] = basePos[2];

            // Cross-section vertex normals at each position around the ring:
            //   slice 0: -perpDir                               (V=0.0)
            //   slice 1: -perpDir*0.707 + perpUpDir*0.707       (V=0.2)  [high LOD only]
            //   slice 2:  perpUpDir                              (V=0.4)  [high LOD only]
            //   slice 3:  perpDir*0.707 + perpUpDir*0.707        (V=0.8)  [high LOD only]
            //   slice 4:  perpDir                                (V=1.0)

            // Normals for each cross-section position
            float nrm0[3], nrm1[3], nrm2[3], nrm3[3], nrm4[3];
            nrm0[0] = -perpDir[0];
            nrm0[1] = -perpDir[1];
            nrm0[2] = -perpDir[2];

            nrm1[0] = perpDir[0];
            nrm1[1] = perpDir[1];
            nrm1[2] = perpDir[2];

            nrm2[0] = perpUpDir[0];
            nrm2[1] = perpUpDir[1];
            nrm2[2] = perpUpDir[2];

            nrm3[0] = (-perpDir[0]) * 0.70710677f + perpUpDir[0] * 0.70710677f;
            nrm3[1] = (-perpDir[1]) * 0.70710677f + perpUpDir[1] * 0.70710677f;
            nrm3[2] = (-perpDir[2]) * 0.70710677f + perpUpDir[2] * 0.70710677f;

            nrm4[0] = perpDir[0] * 0.70710677f + perpUpDir[0] * 0.70710677f;
            nrm4[1] = perpDir[1] * 0.70710677f + perpUpDir[1] * 0.70710677f;
            nrm4[2] = perpDir[2] * 0.70710677f + perpUpDir[2] * 0.70710677f;

            // Vertex positions: base + normal * radius
            float tpos0[3], tpos1[3], tpos2[3], tpos3[3], tpos4[3];
            for ( int c = 0; c < 3; ++c )
            {
                tpos0[c] = basePos[c] + nrm0[c] * curRadius;
                tpos1[c] = basePos[c] + nrm1[c] * curRadius;
                tpos2[c] = basePos[c] + nrm2[c] * curRadius;
                tpos3[c] = basePos[c] + nrm3[c] * curRadius;
                tpos4[c] = basePos[c] + nrm4[c] * curRadius;
            }

            int texUBits = COERCE_INT(ropeTexU);

            // Slice 0: -perpDir side (V = 0.0)
            Rope_EmitVert(verts, tpos0, nrm0, segDir, lerpedColor, texUBits, 0);
            ++verts;

            if ( !useLowResLOD )
            {
                // Slice 1: diagonal -perpDir+perpUpDir (V = 0.2)
                Rope_EmitVert(verts, tpos3, nrm3, segDir, lerpedColor, texUBits, COERCE_INT(0.2f));
                ++verts;

                // Slice 2: perpUpDir (V = 0.4)
                Rope_EmitVert(verts, tpos2, nrm2, segDir, lerpedColor, texUBits, COERCE_INT(0.4f));
                ++verts;

                // Slice 3: diagonal perpDir+perpUpDir (V = 0.8)
                Rope_EmitVert(verts, tpos4, nrm4, segDir, lerpedColor, texUBits, COERCE_INT(0.8f));
                ++verts;
            }

            // Last slice: +perpDir side (V = 1.0)
            Rope_EmitVert(verts, tpos1, nrm1, segDir, lerpedColor, texUBits, COERCE_INT(1.0f));
            ++verts;
        }

        // Set up lighting args
        float4 *meshArgs = (float4 *)R_GetCodeMeshArgs(argOffset);
        meshArgs->u[0] = lightHandle;
        meshArgs->u[1] = lightHandle;
        meshArgs->u[2] = lightHandle;
        meshArgs->u[3] = lightHandle;

        R_AddRopeCodeMeshDrawSurf(
            vrope->material,
            baseIndices,
            indexCount,
            argOffset,
            1u,
            primaryLightIndex,
            drawsurfRegion);
    }
    R_EndCodeMeshVerts();
    Sys_AssistAndWaitWorkerCmdInternal(&r_model_lightingWorkerCmd);
}

void __cdecl R_Rope_ClearAll()
{
    g_ropeInfo.ropeCount = 0;
}

void __cdecl R_Rope_Add(const VisualRope *vropeIn)
{
    VisualRope *vr; // [esp+8h] [ebp-4h]

    if ( g_ropeInfo.ropeCount != 96 )
    {
        vr = &g_ropeInfo.ropes[g_ropeInfo.ropeCount];
        memcpy(vr, vropeIn, sizeof(VisualRope));
        vr->drawSurf = 0;
        if ( !vr->lightingHandle
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_rope_render.cpp",
                        424,
                        0,
                        "%s",
                        "vr->lightingHandle") )
        {
            __debugbreak();
        }
        ++g_ropeInfo.ropeCount;
    }
}

RopeRenderInfo *__cdecl R_Rope_GetInfo()
{
    return &g_ropeInfo;
}

unsigned __int8 __cdecl R_Rope_CalcVisibility(const float *minmax, unsigned int localClientNum)
{
    signed int sceneIdx; // [esp+0h] [ebp-8h]
    unsigned __int8 visFlag; // [esp+7h] [ebp-1h]

    visFlag = 0;
    for ( sceneIdx = 0; sceneIdx < 3; ++sceneIdx )
    {
        if ( !R_CullBoxCurDpvs_SceneSelect(minmax, localClientNum, sceneIdx) )
            visFlag |= 1 << sceneIdx;
    }
    return visFlag;
}

const Material *__cdecl R_Rope_GetMaterialFromGfxWorld()
{
    return rgp.world->ropeMaterial;
}

