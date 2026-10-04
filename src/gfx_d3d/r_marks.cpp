#include "r_marks.h"
#include "r_dpvs.h"
#include "r_init.h"
#include <xanim/dobj_utils.h>
#include <cgame_mp/cg_pose_mp.h>
#include <client/splitscreen.h>
#include "r_material_load_obj.h"
#include <EffectsCore/fx_dvars.h>
#include "r_warn.h"
#include <cgame_mp/cg_ents_mp.h>
#include <glass/glass_client.h>
#include <universal/com_math_anglevectors.h>
#include <xanim/xmodel_utils.h>
#include <EffectsCore/fx_marks.h>
#include <xanim/xanim.h>
#include "r_scene.h"

void    R_BoxSurfaces(
                const float *mins,
                const float *maxs,
                int (__cdecl **allowSurf)(int, void *),
                void *callbackContext,
                GfxSurface ***surfLists,
                unsigned int surfListSize,
                unsigned int *surfCounts,
                unsigned int listCount)
{
    // kcod4
    unsigned __int8 cellBits[128]; // [esp-4h] [ebp-9Ch] BYREF

    iassert(rgp.world);
    iassert(rgp.world->dpvsPlanes.cellCount <= (1024));
    iassert(rgp.world->cellBitsCount <= ((1024) >> 3));

    Com_Memset(cellBits, 0, rgp.world->cellBitsCount);

    for (unsigned int i = 0; i < listCount; ++i)
        surfCounts[i] = 0;

    R_BoxSurfaces_r(
        (mnode_t *)rgp.world->dpvsPlanes.nodes,
        mins,
        maxs,
        allowSurf,
        callbackContext,
        surfLists,
        surfListSize,
        surfCounts,
        listCount,
        (unsigned __int8 *)cellBits);

    for (unsigned int i = 0; i < listCount; ++i)
    {
        iassert(surfCounts[i] <= surfListSize);
    }
}

void __cdecl R_BoxSurfaces_r(
                mnode_t *node,
                const float *mins,
                const float *maxs,
                int (__cdecl **allowSurf)(int, void *),
                void *callbackContext,
                GfxSurface ***surfLists,
                unsigned int surfListSize,
                unsigned int *surfCounts,
                unsigned int listCount,
                unsigned __int8 *cellBits)
{
    const cplane_s *side; // [esp+0h] [ebp-10h]
    int cellIndex; // [esp+4h] [ebp-Ch]
    int cellCount; // [esp+8h] [ebp-8h]

    cellCount = rgp.world->dpvsPlanes.cellCount + 1;
    while ( 1 )
    {
        cellIndex = node->cellIndex;
        if ( cellIndex - cellCount < 0 )
            break;
        side = (const cplane_s *)BoxOnPlaneSide(
                                                             mins,
                                                             maxs,
                                                             &rgp.world->dpvsPlanes.planes[cellIndex - cellCount]);
        if ( side == (const cplane_s *)1 )
        {
            ++node;
        }
        else
        {
            if ( side != (const cplane_s *)2 )
                R_BoxSurfaces_r(
                    node + 1,
                    mins,
                    maxs,
                    allowSurf,
                    callbackContext,
                    surfLists,
                    surfListSize,
                    surfCounts,
                    listCount,
                    cellBits);
            node = (mnode_t *)((char *)node + 2 * node->rightChildOffset);
        }
    }
    if ( node->cellIndex )
    {
        if ( listCount == 1 )
        {
            R_CellSurfaces(
                cellIndex - 1,
                mins,
                maxs,
                *allowSurf,
                callbackContext,
                *surfLists,
                surfListSize,
                surfCounts,
                cellBits);
        }
        else
        {
            if ( listCount != 2
                && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_marks.cpp", 802, 0, "%s", "listCount == 2") )
            {
                __debugbreak();
            }
            R_CellSurfacesTwoLists(
                cellIndex - 1,
                mins,
                maxs,
                allowSurf,
                callbackContext,
                surfLists,
                surfListSize,
                surfCounts,
                cellBits);
        }
    }
}

void __cdecl R_CellSurfaces(
                int cellIndex,
                const float *mins,
                const float *maxs,
                int (__cdecl *allowSurf)(int, void *),
                void *callbackContext,
                GfxSurface **surfList,
                unsigned int surfListSize,
                unsigned int *surfCount,
                unsigned __int8 *cellBits)
{
    GfxCell *cell; // [esp+0h] [ebp-24h]
    int surfIndex; // [esp+4h] [ebp-20h]
    int remappedSurfIndex; // [esp+Ch] [ebp-18h]
    int cullGroupIndex; // [esp+10h] [ebp-14h]
    GfxCullGroup *group; // [esp+14h] [ebp-10h]
    GfxSurface *surf; // [esp+18h] [ebp-Ch]
    int cellIndexBit; // [esp+1Ch] [ebp-8h]
    int triSurfIndex; // [esp+20h] [ebp-4h]

    cellIndexBit = 1 << (cellIndex & 7);
    if ( ((unsigned __int8)cellIndexBit & cellBits[cellIndex >> 3]) == 0 )
    {
        cellBits[cellIndex >> 3] |= cellIndexBit;
        cell = &rgp.world->cells[cellIndex];
        for ( cullGroupIndex = 0; cullGroupIndex < cell->cullGroupCount; ++cullGroupIndex )
        {
            group = &rgp.world->dpvs.cullGroups[cell->cullGroups[cullGroupIndex]];
            if ( *mins <= group->maxs[0]
                && group->mins[0] <= *maxs
                && mins[1] <= group->maxs[1]
                && group->mins[1] <= maxs[1]
                && mins[2] <= group->maxs[2]
                && group->mins[2] <= maxs[2] )
            {
                triSurfIndex = 0;
                surfIndex = group->startSurfIndex;
                while ( triSurfIndex < group->surfaceCount && *surfCount < surfListSize )
                {
                    remappedSurfIndex = rgp.world->dpvs.sortedSurfIndex[surfIndex];
                    if ( allowSurf(remappedSurfIndex, callbackContext) )
                    {
                        surf = &rgp.world->dpvs.surfaces[remappedSurfIndex];
                        if ( *mins <= surf->bounds[1][0]
                            && surf->bounds[0][0] <= *maxs
                            && mins[1] <= surf->bounds[1][1]
                            && surf->bounds[0][1] <= maxs[1]
                            && mins[2] <= surf->bounds[1][2]
                            && surf->bounds[0][2] <= maxs[2] )
                        {
                            R_AddSurfaceToList(surf, surfList, surfCount);
                        }
                    }
                    ++triSurfIndex;
                    ++surfIndex;
                }
            }
        }
        R_AABBTreeSurfaces_r(cell->aabbTree, mins, maxs, allowSurf, callbackContext, surfList, surfListSize, surfCount);
    }
}

void __cdecl R_AABBTreeSurfaces_r(
                GfxAabbTree *tree,
                const float *mins,
                const float *maxs,
                int (__cdecl *allowSurf)(int, void *),
                void *callbackContext,
                GfxSurface **surfList,
                unsigned int surfListSize,
                unsigned int *surfCount)
{
    unsigned int listIndex; // [esp+0h] [ebp-1Ch]
    GfxAabbTree *children; // [esp+4h] [ebp-18h]
    unsigned int surfIndex; // [esp+8h] [ebp-14h]
    unsigned int childIndex; // [esp+Ch] [ebp-10h]
    unsigned int remappedSurfIndex; // [esp+10h] [ebp-Ch]
    GfxSurface *surf; // [esp+14h] [ebp-8h]
    unsigned int surfNodeIndex; // [esp+18h] [ebp-4h]

    if ( *mins <= tree->maxs[0]
        && tree->mins[0] <= *maxs
        && mins[1] <= tree->maxs[1]
        && tree->mins[1] <= maxs[1]
        && mins[2] <= tree->maxs[2]
        && tree->mins[2] <= maxs[2] )
    {
        if ( tree->childCount )
        {
            children = (GfxAabbTree *)((char *)tree + tree->childrenOffset);
            for ( childIndex = 0; childIndex < tree->childCount; ++childIndex )
                R_AABBTreeSurfaces_r(
                    &children[childIndex],
                    mins,
                    maxs,
                    allowSurf,
                    callbackContext,
                    surfList,
                    surfListSize,
                    surfCount);
        }
        else
        {
            surfNodeIndex = 0;
            surfIndex = tree->startSurfIndex;
            while ( surfNodeIndex < tree->surfaceCount && *surfCount < surfListSize )
            {
                remappedSurfIndex = rgp.world->dpvs.sortedSurfIndex[surfIndex];
                if ( allowSurf(remappedSurfIndex, callbackContext) )
                {
                    surf = &rgp.world->dpvs.surfaces[remappedSurfIndex];
                    if ( *mins <= surf->bounds[1][0]
                        && surf->bounds[0][0] <= *maxs
                        && mins[1] <= surf->bounds[1][1]
                        && surf->bounds[0][1] <= maxs[1]
                        && mins[2] <= surf->bounds[1][2]
                        && surf->bounds[0][2] <= maxs[2] )
                    {
                        for ( listIndex = 0; listIndex < *surfCount; ++listIndex )
                        {
                            if ( surfList[listIndex] == surf
                                && !Assert_MyHandler(
                                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_marks.cpp",
                                            408,
                                            0,
                                            "%s",
                                            "surfList[listIndex] != surf") )
                            {
                                __debugbreak();
                            }
                        }
                        surfList[(*surfCount)++] = surf;
                    }
                }
                ++surfNodeIndex;
                ++surfIndex;
            }
        }
    }
}

void __cdecl R_AddSurfaceToList(GfxSurface *surf, GfxSurface **surfList, unsigned int *surfCount)
{
    unsigned int listIndex; // [esp+0h] [ebp-4h]

    for ( listIndex = 0; listIndex < *surfCount; ++listIndex )
    {
        if ( surfList[listIndex] == surf )
            return;
    }
    surfList[(*surfCount)++] = surf;
}

void __cdecl R_CellSurfacesTwoLists(
                int cellIndex,
                const float *mins,
                const float *maxs,
                int (__cdecl **allowSurf)(int, void *),
                void *callbackContext,
                GfxSurface ***surfLists,
                unsigned int surfListSize,
                unsigned int *surfCounts,
                unsigned __int8 *cellBits)
{
    GfxCell *cell; // [esp+0h] [ebp-24h]
    int surfIndex; // [esp+4h] [ebp-20h]
    int remappedSurfIndex; // [esp+Ch] [ebp-18h]
    int cullGroupIndex; // [esp+10h] [ebp-14h]
    GfxCullGroup *group; // [esp+14h] [ebp-10h]
    GfxSurface *surf; // [esp+18h] [ebp-Ch]
    int cellIndexBit; // [esp+1Ch] [ebp-8h]
    int triSurfIndex; // [esp+20h] [ebp-4h]

    cellIndexBit = 1 << (cellIndex & 7);
    if ( ((unsigned __int8)cellIndexBit & cellBits[cellIndex >> 3]) == 0 )
    {
        cellBits[cellIndex >> 3] |= cellIndexBit;
        cell = &rgp.world->cells[cellIndex];
        for ( cullGroupIndex = 0; cullGroupIndex < cell->cullGroupCount; ++cullGroupIndex )
        {
            group = &rgp.world->dpvs.cullGroups[cell->cullGroups[cullGroupIndex]];
            if ( *mins <= group->maxs[0]
                && group->mins[0] <= *maxs
                && mins[1] <= group->maxs[1]
                && group->mins[1] <= maxs[1]
                && mins[2] <= group->maxs[2]
                && group->mins[2] <= maxs[2] )
            {
                triSurfIndex = 0;
                surfIndex = group->startSurfIndex;
                while ( triSurfIndex < group->surfaceCount && *surfCounts < surfListSize && surfCounts[1] < surfListSize )
                {
                    remappedSurfIndex = rgp.world->dpvs.sortedSurfIndex[surfIndex];
                    surf = &rgp.world->dpvs.surfaces[remappedSurfIndex];
                    if ( *mins <= surf->bounds[1][0]
                        && surf->bounds[0][0] <= *maxs
                        && mins[1] <= surf->bounds[1][1]
                        && surf->bounds[0][1] <= maxs[1]
                        && mins[2] <= surf->bounds[1][2]
                        && surf->bounds[0][2] <= maxs[2] )
                    {
                        if ( (*allowSurf)(remappedSurfIndex, callbackContext) )
                            R_AddSurfaceToList(surf, *surfLists, surfCounts);
                        if ( allowSurf[1](remappedSurfIndex, callbackContext) )
                            R_AddSurfaceToList(surf, surfLists[1], surfCounts + 1);
                    }
                    ++triSurfIndex;
                    ++surfIndex;
                }
            }
        }
        R_AABBTreeSurfacesTwoLists_r(
            cell->aabbTree,
            mins,
            maxs,
            allowSurf,
            callbackContext,
            surfLists,
            surfListSize,
            surfCounts);
    }
}

void __cdecl R_AABBTreeSurfacesTwoLists_r(
                GfxAabbTree *tree,
                const float *mins,
                const float *maxs,
                int (__cdecl **allowSurf)(int, void *),
                void *callbackContext,
                GfxSurface ***surfLists,
                unsigned int surfListSize,
                unsigned int *surfCounts)
{
    unsigned int i; // [esp+0h] [ebp-20h]
    unsigned int listIndex; // [esp+4h] [ebp-1Ch]
    GfxAabbTree *children; // [esp+8h] [ebp-18h]
    unsigned int surfIndex; // [esp+Ch] [ebp-14h]
    unsigned int childIndex; // [esp+10h] [ebp-10h]
    unsigned int remappedSurfIndex; // [esp+14h] [ebp-Ch]
    GfxSurface *surf; // [esp+18h] [ebp-8h]
    unsigned int surfNodeIndex; // [esp+1Ch] [ebp-4h]

    if ( *mins <= tree->maxs[0]
        && tree->mins[0] <= *maxs
        && mins[1] <= tree->maxs[1]
        && tree->mins[1] <= maxs[1]
        && mins[2] <= tree->maxs[2]
        && tree->mins[2] <= maxs[2] )
    {
        if ( tree->childCount )
        {
            children = (GfxAabbTree *)((char *)tree + tree->childrenOffset);
            for ( childIndex = 0; childIndex < tree->childCount; ++childIndex )
                R_AABBTreeSurfacesTwoLists_r(
                    &children[childIndex],
                    mins,
                    maxs,
                    allowSurf,
                    callbackContext,
                    surfLists,
                    surfListSize,
                    surfCounts);
        }
        else
        {
            surfNodeIndex = 0;
            surfIndex = tree->startSurfIndex;
            while ( surfNodeIndex < tree->surfaceCount && *surfCounts < surfListSize && surfCounts[1] < surfListSize )
            {
                remappedSurfIndex = rgp.world->dpvs.sortedSurfIndex[surfIndex];
                surf = &rgp.world->dpvs.surfaces[remappedSurfIndex];
                if ( *mins <= surf->bounds[1][0]
                    && surf->bounds[0][0] <= *maxs
                    && mins[1] <= surf->bounds[1][1]
                    && surf->bounds[0][1] <= maxs[1]
                    && mins[2] <= surf->bounds[1][2]
                    && surf->bounds[0][2] <= maxs[2] )
                {
                    for ( listIndex = 0; listIndex < *surfCounts; ++listIndex )
                    {
                        if ( (*surfLists)[listIndex] == surf
                            && !Assert_MyHandler(
                                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_marks.cpp",
                                        453,
                                        0,
                                        "%s",
                                        "surfLists[0][listIndex] != surf") )
                        {
                            __debugbreak();
                        }
                    }
                    for ( i = 0; i < surfCounts[1]; ++i )
                    {
                        if ( surfLists[1][i] == surf
                            && !Assert_MyHandler(
                                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_marks.cpp",
                                        455,
                                        0,
                                        "%s",
                                        "surfLists[1][listIndex] != surf") )
                        {
                            __debugbreak();
                        }
                    }
                    if ( (*allowSurf)(remappedSurfIndex, callbackContext) )
                        (*surfLists)[(*surfCounts)++] = surf;
                    if ( allowSurf[1](remappedSurfIndex, callbackContext) )
                        surfLists[1][surfCounts[1]++] = surf;
                }
                ++surfNodeIndex;
                ++surfIndex;
            }
        }
    }
}

int    R_BoxStaticModels(
                const float *mins,
                const float *maxs,
                int (__cdecl *allowSModel)(int),
                unsigned __int16 *smodelList,
                int smodelListSize)
{
    int smodelCount; // [esp-Ch] [ebp-A0h] BYREF
    unsigned __int8 cellBits[128]; // [esp-8h] [ebp-9Ch] BYREF

    iassert( rgp.world );
    iassert(rgp.world->dpvsPlanes.cellCount <= (1024));
    iassert(rgp.world->cellBitsCount <= ((1024) >> 3));

    //PROF_SCOPED("R_BoxStaticModels");

    Com_Memset(cellBits, 0, rgp.world->cellBitsCount);
    smodelCount = 0;
    R_BoxStaticModels_r(
        (mnode_t *)rgp.world->dpvsPlanes.nodes,
        mins,
        maxs,
        allowSModel,
        smodelList,
        smodelListSize,
        &smodelCount,
        cellBits);
    iassert( smodelCount <= smodelListSize );
    return smodelCount;
}

void __cdecl R_BoxStaticModels_r(
                mnode_t *node,
                const float *mins,
                const float *maxs,
                int (__cdecl *allowSModel)(int),
                unsigned __int16 *smodelList,
                int smodelListSize,
                int *smodelCount,
                unsigned __int8 *cellBits)
{
    const cplane_s *side; // [esp+0h] [ebp-10h]
    int cellIndex; // [esp+4h] [ebp-Ch]
    int cellCount; // [esp+8h] [ebp-8h]

    cellCount = rgp.world->dpvsPlanes.cellCount + 1;
    while ( 1 )
    {
        cellIndex = node->cellIndex;
        if ( cellIndex - cellCount < 0 )
            break;
        side = (const cplane_s *)BoxOnPlaneSide(
                                                             mins,
                                                             maxs,
                                                             &rgp.world->dpvsPlanes.planes[cellIndex - cellCount]);
        if ( side == (const cplane_s *)1 )
        {
            ++node;
        }
        else
        {
            if ( side != (const cplane_s *)2 )
                R_BoxStaticModels_r(node + 1, mins, maxs, allowSModel, smodelList, smodelListSize, smodelCount, cellBits);
            node = (mnode_t *)((char *)node + 2 * node->rightChildOffset);
        }
    }
    if ( node->cellIndex )
        R_CellStaticModels(cellIndex - 1, mins, maxs, allowSModel, smodelList, smodelListSize, smodelCount, cellBits);
}

void __cdecl R_CellStaticModels(
                int cellIndex,
                const float *mins,
                const float *maxs,
                int (__cdecl *allowSModel)(int),
                unsigned __int16 *smodelList,
                int smodelListSize,
                int *smodelCount,
                unsigned __int8 *cellBits)
{
    int cellIndexBit; // [esp+8h] [ebp-4h]

    cellIndexBit = 1 << (cellIndex & 7);
    if ( ((unsigned __int8)cellIndexBit & cellBits[cellIndex >> 3]) == 0 )
    {
        cellBits[cellIndex >> 3] |= cellIndexBit;
        R_AABBTreeStaticModels_r(
            rgp.world->cells[cellIndex].aabbTree,
            mins,
            maxs,
            allowSModel,
            smodelList,
            smodelListSize,
            smodelCount);
    }
}

void __cdecl R_AABBTreeStaticModels_r(
                GfxAabbTree *tree,
                const float *mins,
                const float *maxs,
                int (__cdecl *allowSModel)(int),
                unsigned __int16 *smodelList,
                int smodelListSize,
                int *smodelCount)
{
    GfxAabbTree *children; // [esp+0h] [ebp-14h]
    int smodelIndexIter; // [esp+4h] [ebp-10h]
    int childIndex; // [esp+8h] [ebp-Ch]
    const GfxStaticModelInst *smodelInst; // [esp+Ch] [ebp-8h]
    int smodelIndex; // [esp+10h] [ebp-4h]

    if ( *mins <= tree->maxs[0]
        && tree->mins[0] <= *maxs
        && mins[1] <= tree->maxs[1]
        && tree->mins[1] <= maxs[1]
        && mins[2] <= tree->maxs[2]
        && tree->mins[2] <= maxs[2] )
    {
        if ( tree->childCount )
        {
            children = (GfxAabbTree *)((char *)tree + tree->childrenOffset);
            for ( childIndex = 0; childIndex < tree->childCount; ++childIndex )
                R_AABBTreeStaticModels_r(
                    &children[childIndex],
                    mins,
                    maxs,
                    allowSModel,
                    smodelList,
                    smodelListSize,
                    smodelCount);
        }
        else
        {
            for ( smodelIndexIter = 0;
                        smodelIndexIter < tree->smodelIndexCount && *smodelCount < smodelListSize;
                        ++smodelIndexIter )
            {
                smodelIndex = tree->smodelIndexes[smodelIndexIter];
                if ( allowSModel(smodelIndex) )
                {
                    smodelInst = &rgp.world->dpvs.smodelInsts[smodelIndex];
                    if ( *mins <= smodelInst->maxs[0]
                        && smodelInst->mins[0] <= *maxs
                        && mins[1] <= smodelInst->maxs[1]
                        && smodelInst->mins[1] <= maxs[1]
                        && mins[2] <= smodelInst->maxs[2]
                        && smodelInst->mins[2] <= maxs[2] )
                    {
                        R_AddStaticModelToList(smodelIndex, smodelList, smodelCount);
                    }
                }
            }
        }
    }
}

void __cdecl R_AddStaticModelToList(int smodelIndex, unsigned __int16 *smodelList, int *smodelCount)
{
    int listIndex; // [esp+0h] [ebp-4h]

    for ( listIndex = 0; listIndex < *smodelCount; ++listIndex )
    {
        if ( smodelList[listIndex] == smodelIndex )
            return;
    }
    smodelList[(*smodelCount)++] = smodelIndex;
}

void __cdecl R_MarkUtil_GetDObjAnimMatAndHideParts(
                DObj *dobj,
                const cpose_t *pose,
                const DObjAnimMat **outBoneMtxList,
                unsigned int *outHidePartBits)
{
    char zeroLods[32]; // [esp+0h] [ebp-3Ch] BYREF
    int surfaceCount; // [esp+24h] [ebp-18h]
    int partBits[5]; // [esp+28h] [ebp-14h] BYREF

    memset(zeroLods, 0, sizeof(zeroLods));
    surfaceCount = DObjGetSurfaces(dobj, partBits, zeroLods);
    if ( !surfaceCount
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_marks.cpp", 2242, 0, "%s", "surfaceCount") )
    {
        __debugbreak();
    }
    *outBoneMtxList = CG_DObjCalcPose(pose, dobj, partBits);
    if ( !*outBoneMtxList
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_marks.cpp", 2246, 0, "%s", "*outBoneMtxList") )
    {
        __debugbreak();
    }
    DObjGetHidePartBits(dobj, outHidePartBits);
}

void __cdecl R_MarkFragments_Begin(
                MarkInfo *markInfo,
                MarkFragmentsAgainstEnum markAgainst,
                const float *origin,
                const float (*axis)[3],
                float radius,
                const float *viewOffset,
                Material *material)
{
    int savedregs; // [esp+10h] [ebp+0h] BYREF

    markInfo->origin[0] = *origin;
    markInfo->origin[1] = origin[1];
    markInfo->origin[2] = origin[2];
    AxisCopy(axis, markInfo->axis);
    markInfo->viewOffset[0] = *viewOffset;
    markInfo->viewOffset[1] = viewOffset[1];
    markInfo->viewOffset[2] = viewOffset[2];
    markInfo->radius = radius;
    markInfo->material = material;
    markInfo->markHasLightmap = (material->info.gameFlags & 2) != 0;
    markInfo->markHasReflection = (material->info.gameFlags & 0x10) != 0;
    markInfo->markAgainst = markAgainst;
    R_GetMarkFragmentBounds(markInfo->origin, markInfo->axis, markInfo->radius, markInfo->mins, markInfo->maxs);
    R_GetMarkFragmentClipPlanes(markInfo->origin, markInfo->axis, markInfo->radius, &markInfo->clipPlanes);
    if ( markAgainst )
    {
        if ( markAgainst != MARK_FRAGMENTS_AGAINST_MODELS
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_marks.cpp",
                        2391,
                        0,
                        "%s",
                        "markAgainst == MARK_FRAGMENTS_AGAINST_MODELS") )
        {
            __debugbreak();
        }
        markInfo->smodelCollidedCount = R_BoxStaticModels(
                                                                            markInfo->mins,
                                                                            markInfo->maxs,
                                                                            (int (__cdecl *)(int))CL_LocalClient_GetActiveCount,
                                                                            markInfo->smodelsCollided,
                                                                            32);
        markInfo->sceneDObjCollidedCount = 0;
    }
    else
    {
        markInfo->sceneBModelCollidedCount = 0;
    }
}

void __cdecl R_GetMarkFragmentBounds(
                const float *origin,
                const float (*axis)[3],
                float radius,
                float *mins,
                float *maxs)
{
    int coord; // [esp+14h] [ebp-8h]
    float offset; // [esp+18h] [ebp-4h]

    for ( coord = 0; coord < 3; ++coord )
    {
        offset = (float)((float)(fabs((*axis)[coord])
                                                     + fabs((*axis)[coord + 3]))
                                     + fabs((*axis)[coord + 6]))
                     * radius;
        mins[coord] = origin[coord] - offset;
        maxs[coord] = origin[coord] + offset;
    }
}

#if 0
void __cdecl R_GetMarkFragmentClipPlanes(
                const float *origin,
                const float (*axis)[3],
                float radius,
                MarkClipPlaneSet *clipPlanes)
{
    char *v4; // [esp+4h] [ebp-1Ch]
    float *v5; // [esp+10h] [ebp-10h]
    float *v6; // [esp+14h] [ebp-Ch]
    int axisIndex; // [esp+18h] [ebp-8h]
    int planeIndex; // [esp+1Ch] [ebp-4h]
    int planeIndexa; // [esp+1Ch] [ebp-4h]

    planeIndex = 0;
    for ( axisIndex = 0; axisIndex < 3; ++axisIndex )
    {
        v5 = clipPlanes->planes[planeIndex];
        v6 = (float *)&(*axis)[3 * axisIndex];
        *v5 = *v6;
        v5[1] = v6[1];
        v5[2] = v6[2];
        v5[3] = (float)((float)((float)(*v5 * *origin) + (float)(v5[1] * origin[1])) + (float)(v5[2] * origin[2])) - radius;
        planeIndexa = planeIndex + 1;
        v4 = (char *)clipPlanes->planes[planeIndexa];
        *(unsigned int *)v4 = *(unsigned int *)v6 ^ _mask__NegFloat_;
        *((unsigned int *)v4 + 1) = *((unsigned int *)v6 + 1) ^ _mask__NegFloat_;
        *((unsigned int *)v4 + 2) = *((unsigned int *)v6 + 2) ^ _mask__NegFloat_;
        *((float *)v4 + 3) = (float)((float)((float)(*(float *)v4 * *origin) + (float)(*((float *)v4 + 1) * origin[1]))
                                                             + (float)(*((float *)v4 + 2) * origin[2]))
                                             - radius;
        planeIndex = planeIndexa + 1;
    }
    clipPlanes->planeCount = planeIndex;
    if ( clipPlanes->planeCount != 6
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_marks.cpp",
                    1122,
                    0,
                    "%s",
                    "6 == clipPlanes->planeCount") )
    {
        __debugbreak();
    }
}
#endif

// aislopped
void __cdecl R_GetMarkFragmentClipPlanes(
    const float *origin,
    const float (*axis)[3],
    float radius,
    MarkClipPlaneSet *clipPlanes)
{
    int planeIndex = 0;

    for (int axisIndex = 0; axisIndex < 3; ++axisIndex)
    {
        const float *n = axis[axisIndex];

        float *p = clipPlanes->planes[planeIndex++];
        p[0] = n[0];
        p[1] = n[1];
        p[2] = n[2];
        p[3] = n[0] * origin[0] +
            n[1] * origin[1] +
            n[2] * origin[2] - radius;

        p = clipPlanes->planes[planeIndex++];
        p[0] = -n[0];
        p[1] = -n[1];
        p[2] = -n[2];
        p[3] = -(n[0] * origin[0] +
            n[1] * origin[1] +
            n[2] * origin[2]) - radius;
    }

    clipPlanes->planeCount = planeIndex;

    iassert(6 == clipPlanes->planeCount);
}


char __cdecl R_MarkFragments_AddDObj(MarkInfo *markInfo, DObj *dObj, cpose_t *pose, unsigned __int16 entityIndex)
{
    MarkInfoCollidedDObj *collidedDObj; // [esp+0h] [ebp-4h]

    if ( markInfo->sceneDObjCollidedCount == 32 )
        return 0;
    collidedDObj = &markInfo->sceneDObjsCollided[markInfo->sceneDObjCollidedCount];
    collidedDObj->dObj = dObj;
    collidedDObj->pose = pose;
    collidedDObj->entnum = entityIndex;
    ++markInfo->sceneDObjCollidedCount;
    return 1;
}

char __cdecl R_MarkFragments_AddBModel(
                MarkInfo *markInfo,
                GfxBrushModel *brushModel,
                cpose_t *pose,
                unsigned __int16 entityIndex)
{
    MarkInfoCollidedBModel *collidedBModel; // [esp+0h] [ebp-4h]

    if ( markInfo->sceneBModelCollidedCount == 32 )
        return 0;
    collidedBModel = &markInfo->sceneBModelsCollided[markInfo->sceneBModelCollidedCount];
    collidedBModel->brushModel = brushModel;
    collidedBModel->pose = pose;
    collidedBModel->entnum = entityIndex;
    ++markInfo->sceneBModelCollidedCount;
    return 1;
}

void __cdecl R_MarkFragments_Go(
                MarkInfo *markInfo,
                void (__cdecl *callback)(void *, int, FxMarkTri *, int, FxMarkPoint *, const float *, const float *, const float *, const bool),
                void *callbackContext,
                int maxTris,
                FxMarkTri *tris,
                int maxPoints,
                FxMarkPoint *points)
{
    bool error; // [esp+33h] [ebp-1h]

    PROF_SCOPED("R_MarkFragments_Go");

    if ( !markInfo->material
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_marks.cpp", 2435, 0, "%s", "markInfo->material") )
    {
        __debugbreak();
    }
    markInfo->maxTris = maxTris;
    markInfo->tris = tris;
    markInfo->maxPoints = maxPoints;
    markInfo->points = points;
    markInfo->usedTriCount = 0;
    markInfo->usedPointCount = 0;
    markInfo->callback = callback;
    markInfo->callbackContext = callbackContext;
    error = 0;
    if ( markInfo->markAgainst )
    {
        if ( markInfo->markAgainst != MARK_FRAGMENTS_AGAINST_MODELS
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_marks.cpp",
                        2455,
                        0,
                        "%s",
                        "markInfo->markAgainst == MARK_FRAGMENTS_AGAINST_MODELS") )
        {
            __debugbreak();
        }
        if ( markInfo->isSeeThruDecal
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_marks.cpp",
                        2456,
                        0,
                        "%s",
                        "!markInfo->isSeeThruDecal") )
        {
            __debugbreak();
        }
        if ( !R_MarkFragments_Models(markInfo) )
            error = 1;
    }
    else
    {
        error = !R_MarkFragments_Brushes(markInfo);
    }
    if ( error )
    {
        Com_Printf(
            14,
            "R_MarkFragments: Too many triangles to mark, max %d radius %.2f at %.2f %.2f %.2f\n",
            markInfo->maxTris,
            markInfo->radius,
            markInfo->origin[0],
            markInfo->origin[1],
            markInfo->origin[2]);
    }
}

bool __cdecl R_MarkFragments_Brushes(MarkInfo *markInfo)
{
    if ( !R_MarkFragments_WorldBrushes(markInfo) )
        return 0;
    if ( R_MarkFragments_EntBrushes(markInfo) )
        return R_MarkFragments_Glass(markInfo) != 0;
    return 0;
}

int(__cdecl *allowSurf[1])(int, void *) = { (int(*)(int, void*))R_AllowMarks };

char __cdecl R_MarkFragments_WorldBrushes(MarkInfo *markInfo)
{
    GfxSurface **surfacesArray[1]; // [esp+14h] [ebp-41Ch] BYREF
    unsigned int surfIndex; // [esp+18h] [ebp-418h]
    unsigned int surfCount; // [esp+1Ch] [ebp-414h] BYREF
    GfxSurface *surfaces[256]; // [esp+20h] [ebp-410h] BYREF
    GfxMarkContext markContext; // [esp+424h] [ebp-Ch] BYREF
    bool anyMarks; // [esp+42Fh] [ebp-1h] BYREF
    int savedregs; // [esp+430h] [ebp+0h] BYREF

    PROF_SCOPED("markfragments worldbrushes");

    if ( (markInfo->usedTriCount || markInfo->usedPointCount)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_marks.cpp",
                    1649,
                    0,
                    "%s",
                    "!markInfo->usedTriCount && !markInfo->usedPointCount") )
    {
        __debugbreak();
    }
    markContext.modelTypeAndSurf = 0;
    markContext.modelIndex = 0;
    surfacesArray[0] = surfaces;
    R_BoxSurfaces(
        markInfo->mins,
        markInfo->maxs,
        allowSurf,
        markInfo->material,
        surfacesArray,
        0x100u,
        &surfCount,
        1u);
    anyMarks = 0;
    for ( surfIndex = 0; surfIndex < surfCount; ++surfIndex )
    {
        if ( !R_MarkFragments_BrushSurface(
                        markInfo,
                        &markContext,
                        &markInfo->clipPlanes,
                        markInfo->axis[0],
                        surfaces[surfIndex],
                        &anyMarks) )
        {
            return 0;
        }
    }
    if ( anyMarks )
    {
        markInfo->callback(
            markInfo->callbackContext,
            markInfo->usedTriCount,
            markInfo->tris,
            markInfo->usedPointCount,
            markInfo->points,
            (const float *)markInfo,
            markInfo->axis[1],
            (const float *)markInfo->axis,
            markInfo->isSeeThruDecal);
        markInfo->usedTriCount = 0;
        markInfo->usedPointCount = 0;
    }
    if ( (markInfo->usedTriCount || markInfo->usedPointCount)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_marks.cpp",
                    1679,
                    0,
                    "%s",
                    "!markInfo->usedTriCount && !markInfo->usedPointCount") )
    {
        __debugbreak();
    }
    return 1;
}

bool __cdecl R_AllowMarks(int surfIndex, const Material *markMaterialAsVoid)
{
    return R_Mark_MaterialAllowsMarks(rgp.world->dpvs.surfaces[surfIndex].material, markMaterialAsVoid);
}

bool __cdecl R_Mark_MaterialAllowsMarks(const Material *markReceiverMaterialHandle, const Material *markMaterialHandle)
{
    bool v2; // esi

    v2 = Material_IsForSeeThruBulletHoleDecal(markMaterialHandle);
    if ( v2 != Material_IsSeeThruParallaxDecal(markReceiverMaterialHandle) )
        return 0;
    if ( (markReceiverMaterialHandle->stateFlags & 4) != 0 )
        return 0;
    if ( (markReceiverMaterialHandle->info.gameFlags & 4) != 0 )
        return 0;
    return (markReceiverMaterialHandle->info.surfaceTypeBits & markMaterialHandle->info.surfaceTypeBits) == markMaterialHandle->info.surfaceTypeBits;
}

const float SEETHRU_DECAL_EPSISON = 0.001f; // "EPSISON". 0 Google results. You are doomed to live in a world more complex than you can possibly ever comprehend.

#if 0
char __cdecl R_MarkFragments_BrushSurface(
                MarkInfo *markInfo,
                GfxMarkContext *markContext,
                const MarkClipPlaneSet *clipPlanes,
                const float *markDir,
                const GfxSurface *surface,
                bool *anyMarks)
{
    float v7; // [esp+4h] [ebp-3DCh]
    __int64 v8; // [esp+10h] [ebp-3D0h] BYREF
    float v9; // [esp+18h] [ebp-3C8h]
    PackedUnitVec v10; // [esp+1Ch] [ebp-3C4h]
    float *v11; // [esp+24h] [ebp-3BCh]
    PackedUnitVec v12; // [esp+28h] [ebp-3B8h]
    PackedUnitVec v13; // [esp+2Ch] [ebp-3B4h] BYREF
    PackedUnitVec v14; // [esp+30h] [ebp-3B0h]
    PackedUnitVec v15; // [esp+34h] [ebp-3ACh]
    PackedUnitVec v16; // [esp+38h] [ebp-3A8h]
    PackedUnitVec v17; // [esp+40h] [ebp-3A0h]
    __int64 out; // [esp+44h] [ebp-39Ch] BYREF
    PackedUnitVec v19; // [esp+4Ch] [ebp-394h]
    PackedUnitVec in; // [esp+50h] [ebp-390h]
    const float *v21; // [esp+58h] [ebp-388h]
    const float *v22; // [esp+5Ch] [ebp-384h]
    const float *v23; // [esp+60h] [ebp-380h]
    const float *v24; // [esp+64h] [ebp-37Ch]
    const float *v25; // [esp+68h] [ebp-378h]
    const unsigned __int8 *triVerts1; // [esp+B4h] [ebp-32Ch]
    FxMarkPoint *points; // [esp+B8h] [ebp-328h]
    float planeTestEpsilon; // [esp+BCh] [ebp-324h]
    FxWorldMarkPoint clipPoints[2][14]; // [esp+C0h] [ebp-320h] BYREF
    int baseIndex; // [esp+364h] [ebp-7Ch]
    FxMarkTri *tris; // [esp+368h] [ebp-78h]
    float normal[3][3]; // [esp+36Ch] [ebp-74h] BYREF
    FxWorldMarkPoint *clipPoint; // [esp+390h] [ebp-50h]
    int pointIndex; // [esp+394h] [ebp-4Ch]
    const unsigned __int16 *indices; // [esp+398h] [ebp-48h]
    unsigned int triVerts1Stride; // [esp+39Ch] [ebp-44h]
    int fragmentPointCount; // [esp+3A0h] [ebp-40h]
    const GfxWorldVertex *triVert1[3]; // [esp+3A4h] [ebp-3Ch]
    const GfxWorldVertex *triVerts0; // [esp+3B0h] [ebp-30h]
    int triIndex; // [esp+3B4h] [ebp-2Ch]
    int pingPong; // [esp+3B8h] [ebp-28h]
    const srfTriangles_t *triSurf; // [esp+3BCh] [ebp-24h]
    float lmapCoord[3][2]; // [esp+3C0h] [ebp-20h] BYREF
    int planeIndex; // [esp+3D8h] [ebp-8h]
    FxMarkPoint *point; // [esp+3DCh] [ebp-4h]

    markContext->lmapIndex = surface->lightmapIndex;
    if ( markInfo->isSeeThruDecal || markInfo->markHasLightmap == (markContext->lmapIndex != 31) )
    {
        markContext->primaryLightIndex = surface->primaryLightIndex;
        markContext->reflectionProbeIndex = surface->reflectionProbeIndex;
        if ( markInfo->markHasReflection == (markContext->reflectionProbeIndex != 0) )
        {
            markContext->primaryLightIndex = surface->primaryLightIndex;
            triSurf = &surface->tris;
            triVerts0 = &g_worldDraw->vd.vertices[surface->tris.firstVertex];
            indices = &g_worldDraw->indices[surface->tris.baseIndex];
            triVerts1 = (const unsigned __int8 *)triVerts0;
            triVerts1Stride = 44;
            if ( markInfo->isSeeThruDecal )
                v7 = SEETHRU_DECAL_EPSISON;
            else
                v7 = 0.5f;
            planeTestEpsilon = v7;
            triIndex = 0;
            while ( triIndex < triSurf->triCount )
            {
                if ( !R_MarkFragment_IsTriangleRejected(
                                markDir,
                                triVerts0[*indices].xyz,
                                triVerts0[indices[1]].xyz,
                                triVerts0[indices[2]].xyz) )
                {
                    R_MarkFragment_SetupWorldClipPoints(triVerts0, indices, clipPoints);
                    pingPong = 0;
                    fragmentPointCount = 3;
                    for ( planeIndex = 0; planeIndex < clipPlanes->planeCount; ++planeIndex )
                    {
                        fragmentPointCount = R_ChopWorldPolyBehindPlane(
                                                                     fragmentPointCount,
                                                                     clipPoints[pingPong],
                                                                     clipPoints[pingPong == 0],
                                                                     clipPlanes->planes[planeIndex],
                                                                     planeTestEpsilon);
                        if ( !fragmentPointCount )
                            goto LABEL_14;
                        if ( fragmentPointCount > 14
                            && !Assert_MyHandler(
                                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_marks.cpp",
                                        1445,
                                        0,
                                        "%s\n\t(fragmentPointCount) = %i",
                                        "(fragmentPointCount <= 3 + 11)",
                                        fragmentPointCount) )
                        {
                            __debugbreak();
                        }
                        pingPong ^= 1u;
                    }
                    if ( fragmentPointCount > markInfo->maxPoints - markInfo->usedPointCount
                        || 3 * (fragmentPointCount - 2) > markInfo->maxTris - markInfo->usedTriCount )
                    {
                        return 0;
                    }
                    tris = &markInfo->tris[markInfo->usedTriCount];
                    baseIndex = markInfo->usedPointCount;
                    for ( pointIndex = 2; pointIndex < fragmentPointCount; ++pointIndex )
                    {
                        tris->indices[0] = baseIndex + pointIndex - 1;
                        tris->indices[1] = pointIndex + baseIndex;
                        tris->indices[2] = baseIndex;
                        tris->context = *markContext;
                        ++tris;
                    }
                    triVert1[0] = (const GfxWorldVertex *)&triVerts1[triVerts1Stride * *indices];
                    triVert1[1] = (const GfxWorldVertex *)&triVerts1[triVerts1Stride * indices[1]];
                    triVert1[2] = (const GfxWorldVertex *)&triVerts1[triVerts1Stride * indices[2]];
                    v25 = triVert1[0]->lmapCoord;
                    *(_QWORD *)&lmapCoord[0][0] = *(_QWORD *)triVert1[0]->lmapCoord;
                    v23 = lmapCoord[1];
                    v24 = triVert1[1]->lmapCoord;
                    *(_QWORD *)&lmapCoord[1][0] = *(_QWORD *)triVert1[1]->lmapCoord;
                    v21 = lmapCoord[2];
                    v22 = triVert1[2]->lmapCoord;
                    *(_QWORD *)&lmapCoord[2][0] = *(_QWORD *)triVert1[2]->lmapCoord;
                    v17.packed = triVert1[0]->normal.packed;
                    //LODWORD(out) = v17;
                    //HIDWORD(out) = v17;
                    out = v17.packed;
                    v19.packed = v17.packed;
                    in.packed = v17.packed;
                    Vec3UnpackUnitVec(v17, (float *)&out);
                    in.packed = 0;
                    *(_QWORD *)&normal[0][0] = out;
                    LODWORD(normal[0][2]) = v19.packed;
                    v11 = normal[1];
                    v12.packed = (unsigned int)triVert1[1]->normal.packed;
                    v13.packed = v12.packed;
                    v14.packed = v12.packed;
                    v15.packed = v12.packed;
                    v16.packed = v12.packed;
                    Vec3UnpackUnitVec(v12, (float *)&v13.packed);
                    v16.packed = 0;
                    *v11 = *(float *)&v13.packed;
                    v11[1] = *(float *)&v14.packed;
                    v11[2] = *(float *)&v15.packed;
                    LODWORD(v8) = triVert1[2]->normal.packed;
                    HIDWORD(v8) = v8;
                    v9 = *(float *)&v8;
                    v10.packed = v8;
                    Vec3UnpackUnitVec((const PackedUnitVec)v8, (float *)&v8);
                    v10.packed = 0;
                    *(_QWORD *)&normal[2][0] = v8;
                    normal[2][2] = v9;
                    points = &markInfo->points[markInfo->usedPointCount];
                    for ( pointIndex = 0; pointIndex < fragmentPointCount; ++pointIndex )
                    {
                        point = &points[pointIndex];
                        clipPoint = &clipPoints[pingPong][pointIndex];
                        point->xyz[0] = clipPoint->xyz[0];
                        point->xyz[1] = clipPoint->xyz[1];
                        point->xyz[2] = clipPoint->xyz[2];
                        point->lmapCoord[0] = (float)((float)(clipPoint->vertWeights[0] * lmapCoord[0][0])
                                                                                + (float)(clipPoint->vertWeights[1] * lmapCoord[1][0]))
                                                                + (float)(clipPoint->vertWeights[2] * lmapCoord[2][0]);
                        point->lmapCoord[1] = (float)((float)(clipPoint->vertWeights[0] * lmapCoord[0][1])
                                                                                + (float)(clipPoint->vertWeights[1] * lmapCoord[1][1]))
                                                                + (float)(clipPoint->vertWeights[2] * lmapCoord[2][1]);
                        point->normal[0] = (float)((float)(clipPoint->vertWeights[0] * normal[0][0])
                                                                         + (float)(clipPoint->vertWeights[1] * normal[1][0]))
                                                         + (float)(clipPoint->vertWeights[2] * normal[2][0]);
                        point->normal[1] = (float)((float)(clipPoint->vertWeights[0] * normal[0][1])
                                                                         + (float)(clipPoint->vertWeights[1] * normal[1][1]))
                                                         + (float)(clipPoint->vertWeights[2] * normal[2][1]);
                        point->normal[2] = (float)((float)(clipPoint->vertWeights[0] * normal[0][2])
                                                                         + (float)(clipPoint->vertWeights[1] * normal[1][2]))
                                                         + (float)(clipPoint->vertWeights[2] * normal[2][2]);
                    }
                    if ( fragmentPointCount < 3
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_marks.cpp",
                                    1500,
                                    0,
                                    "%s\n\t(fragmentPointCount) = %i",
                                    "(fragmentPointCount >= 3)",
                                    fragmentPointCount) )
                    {
                        __debugbreak();
                    }
                    markInfo->usedPointCount += fragmentPointCount;
                    markInfo->usedTriCount = fragmentPointCount + markInfo->usedTriCount - 2;
                    *anyMarks = 1;
                }
LABEL_14:
                ++triIndex;
                indices += 3;
            }
            return 1;
        }
        else
        {
            if ( fx_marks_debug_text->current.enabled )
                R_WarnOncePerFrame(
                    R_WARN_MARKS_ERR_PROBE_MISMATCH,
                    markInfo->markHasReflection,
                    surface->reflectionProbeIndex,
                    markInfo->material->info.name,
                    surface->material->info.name);
            return 1;
        }
    }
    else
    {
        if ( fx_marks_debug_text->current.enabled )
            R_WarnOncePerFrame(
                R_WARN_MARKS_ERR_LMAP_MISMATCH,
                markInfo->markHasLightmap,
                surface->lightmapIndex,
                markInfo->material->info.name,
                surface->material->info.name);
        return 1;
    }
}
#endif

bool __cdecl R_MarkFragments_BrushSurface(
    MarkInfo *markInfo,
    GfxMarkContext *markContext,
    const MarkClipPlaneSet *clipPlanes,
    const float *markDir,
    const GfxSurface *surface,
    bool *anyMarks)
{
    const srfTriangles_t *triSurf;
    const GfxWorldVertex *verts;
    const unsigned __int16 *indices;

    FxWorldMarkPoint clipPoints[2][14];

    float normal[3][3];
    float lmapCoord[3][2];

    int triIndex;
    int planeIndex;
    int pointIndex;
    int pingPong;
    int fragmentPointCount;

    float planeTestEpsilon;

    markContext->lmapIndex = surface->lightmapIndex;

    /* Lightmap compatibility check */
    if (!markInfo->isSeeThruDecal &&
        markInfo->markHasLightmap != (markContext->lmapIndex != 31))
    {
        if (fx_marks_debug_text->current.enabled)
        {
            R_WarnOncePerFrame(
                R_WARN_MARKS_ERR_LMAP_MISMATCH,
                markInfo->markHasLightmap,
                surface->lightmapIndex,
                markInfo->material->info.name,
                surface->material->info.name);
        }
        return true;
    }

    markContext->primaryLightIndex = surface->primaryLightIndex;
    markContext->reflectionProbeIndex = surface->reflectionProbeIndex;

    /* Reflection compatibility check */
    if (markInfo->markHasReflection != (markContext->reflectionProbeIndex != 0))
    {
        if (fx_marks_debug_text->current.enabled)
        {
            R_WarnOncePerFrame(
                R_WARN_MARKS_ERR_PROBE_MISMATCH,
                markInfo->markHasReflection,
                surface->reflectionProbeIndex,
                markInfo->material->info.name,
                surface->material->info.name);
        }
        return true;
    }

    triSurf = &surface->tris;
    verts = &g_worldDraw->vd.vertices[triSurf->firstVertex];
    indices = &g_worldDraw->indices[triSurf->baseIndex];

    planeTestEpsilon = markInfo->isSeeThruDecal
        ? SEETHRU_DECAL_EPSISON
        : 0.5f;

    for (triIndex = 0; triIndex < triSurf->triCount; ++triIndex, indices += 3)
    {
        /* Facing test */
        if (R_MarkFragment_IsTriangleRejected(
            markDir,
            verts[indices[0]].xyz,
            verts[indices[1]].xyz,
            verts[indices[2]].xyz))
        {
            continue;
        }

        /* Initial clip triangle */
        R_MarkFragment_SetupWorldClipPoints(verts, indices, clipPoints);

        pingPong = 0;
        fragmentPointCount = 3;

        /* Clip against all planes */
        for (planeIndex = 0; planeIndex < clipPlanes->planeCount; ++planeIndex)
        {
            fragmentPointCount = R_ChopWorldPolyBehindPlane(
                fragmentPointCount,
                clipPoints[pingPong],
                clipPoints[pingPong ^ 1],
                clipPlanes->planes[planeIndex],
                planeTestEpsilon);

            if (!fragmentPointCount)
                break;

            iassert(fragmentPointCount <= 14);
            pingPong ^= 1;
        }

        if (!fragmentPointCount)
            continue;

        /* Capacity check */
        if (fragmentPointCount > markInfo->maxPoints - markInfo->usedPointCount ||
            3 * (fragmentPointCount - 2) > markInfo->maxTris - markInfo->usedTriCount)
        {
            return false;
        }

        /* Emit triangles */
        {
            FxMarkTri *tris = &markInfo->tris[markInfo->usedTriCount];
            int baseIndex = markInfo->usedPointCount;

            for (pointIndex = 2; pointIndex < fragmentPointCount; ++pointIndex)
            {
                tris->indices[0] = baseIndex;
                tris->indices[1] = baseIndex + pointIndex - 1;
                tris->indices[2] = baseIndex + pointIndex;
                tris->context = *markContext;
                ++tris;
            }
        }

        /* Triangle data */
        const GfxWorldVertex *v0 = &verts[indices[0]];
        const GfxWorldVertex *v1 = &verts[indices[1]];
        const GfxWorldVertex *v2 = &verts[indices[2]];

        Vec2Copy(v0->lmapCoord, lmapCoord[0]);
        Vec2Copy(v1->lmapCoord, lmapCoord[1]);
        Vec2Copy(v2->lmapCoord, lmapCoord[2]);

        Vec3UnpackUnitVec(v0->normal, normal[0]);
        Vec3UnpackUnitVec(v1->normal, normal[1]);
        Vec3UnpackUnitVec(v2->normal, normal[2]);

        /* Emit points */
        {
            FxMarkPoint *points = &markInfo->points[markInfo->usedPointCount];

            for (pointIndex = 0; pointIndex < fragmentPointCount; ++pointIndex)
            {
                const FxWorldMarkPoint *cp = &clipPoints[pingPong][pointIndex];
                FxMarkPoint *p = &points[pointIndex];

                Vec3Copy(cp->xyz, p->xyz);

                p->lmapCoord[0] =
                    cp->vertWeights[0] * lmapCoord[0][0] +
                    cp->vertWeights[1] * lmapCoord[1][0] +
                    cp->vertWeights[2] * lmapCoord[2][0];

                p->lmapCoord[1] =
                    cp->vertWeights[0] * lmapCoord[0][1] +
                    cp->vertWeights[1] * lmapCoord[1][1] +
                    cp->vertWeights[2] * lmapCoord[2][1];

                p->normal[0] =
                    cp->vertWeights[0] * normal[0][0] +
                    cp->vertWeights[1] * normal[1][0] +
                    cp->vertWeights[2] * normal[2][0];

                p->normal[1] =
                    cp->vertWeights[0] * normal[0][1] +
                    cp->vertWeights[1] * normal[1][1] +
                    cp->vertWeights[2] * normal[2][1];

                p->normal[2] =
                    cp->vertWeights[0] * normal[0][2] +
                    cp->vertWeights[1] * normal[1][2] +
                    cp->vertWeights[2] * normal[2][2];
            }
        }

        iassert(fragmentPointCount >= 3);

        markInfo->usedPointCount += fragmentPointCount;
        markInfo->usedTriCount += fragmentPointCount - 2;
        *anyMarks = true;
    }

    return true;
}
int __cdecl R_ChopWorldPolyBehindPlane(
                int inPointCount,
                FxWorldMarkPoint *inPoints,
                FxWorldMarkPoint *outPoints,
                const float *plane,
                float epsilon)
{
    const FxWorldMarkPoint *v6; // eax
    FxWorldMarkPoint *v7; // ecx
    const FxWorldMarkPoint *v8; // eax
    FxWorldMarkPoint *v9; // ecx
    int sideCount[3]; // [esp+Ch] [ebp-94h] BYREF
    float dists[15]; // [esp+18h] [ebp-88h]
    int nextIndex; // [esp+54h] [ebp-4Ch]
    float lerp; // [esp+58h] [ebp-48h]
    int pointIndex; // [esp+5Ch] [ebp-44h]
    int sides[15]; // [esp+60h] [ebp-40h]
    int outPointCount; // [esp+9Ch] [ebp-4h]

    if ( inPointCount > 14
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_marks.cpp",
                    262,
                    0,
                    "%s\n\t(inPointCount) = %i",
                    "(inPointCount <= 3 + 11)",
                    inPointCount) )
    {
        __debugbreak();
    }
    memset(sideCount, 0, sizeof(sideCount));
    for ( pointIndex = 0; pointIndex < inPointCount; ++pointIndex )
    {
        dists[pointIndex] = (float)((float)((float)(inPoints[pointIndex].xyz[0] * *plane)
                                                                            + (float)(inPoints[pointIndex].xyz[1] * plane[1]))
                                                            + (float)(inPoints[pointIndex].xyz[2] * plane[2]))
                                            - plane[3];
        if ( dists[pointIndex] <= epsilon )
        {
            if ( (-(epsilon)) <= dists[pointIndex] )
                sides[pointIndex] = 2;
            else
                sides[pointIndex] = 1;
        }
        else
        {
            sides[pointIndex] = 0;
        }
        ++sideCount[sides[pointIndex]];
    }
    sides[pointIndex] = sides[0];
    dists[pointIndex] = dists[0];
    if ( !sideCount[0] )
        return 0;
    if ( sideCount[1] )
    {
        outPointCount = 0;
        for ( pointIndex = 0; pointIndex < inPointCount; ++pointIndex )
        {
            if ( sides[pointIndex] == 2 )
            {
                if ( outPointCount >= 14
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_marks.cpp",
                                303,
                                0,
                                "%s\n\t(outPointCount) = %i",
                                "(outPointCount < 3 + 11)",
                                outPointCount) )
                {
                    __debugbreak();
                }
                v6 = &inPoints[pointIndex];
                v7 = &outPoints[outPointCount];
                v7->xyz[0] = v6->xyz[0];
                v7->xyz[1] = v6->xyz[1];
                v7->xyz[2] = v6->xyz[2];
                v7->vertWeights[0] = v6->vertWeights[0];
                v7->vertWeights[1] = v6->vertWeights[1];
                v7->vertWeights[2] = v6->vertWeights[2];
                ++outPointCount;
            }
            else
            {
                if ( !sides[pointIndex] )
                {
                    if ( outPointCount >= 14
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_marks.cpp",
                                    311,
                                    0,
                                    "%s\n\t(outPointCount) = %i",
                                    "(outPointCount < 3 + 11)",
                                    outPointCount) )
                    {
                        __debugbreak();
                    }
                    v8 = &inPoints[pointIndex];
                    v9 = &outPoints[outPointCount];
                    v9->xyz[0] = v8->xyz[0];
                    v9->xyz[1] = v8->xyz[1];
                    v9->xyz[2] = v8->xyz[2];
                    v9->vertWeights[0] = v8->vertWeights[0];
                    v9->vertWeights[1] = v8->vertWeights[1];
                    v9->vertWeights[2] = v8->vertWeights[2];
                    ++outPointCount;
                }
                if ( sides[pointIndex + 1] != 2 && sides[pointIndex + 1] != sides[pointIndex] )
                {
                    if ( outPointCount >= 14
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_marks.cpp",
                                    320,
                                    0,
                                    "%s\n\t(outPointCount) = %i",
                                    "(outPointCount < 3 + 11)",
                                    outPointCount) )
                    {
                        __debugbreak();
                    }
                    if ( dists[pointIndex] == dists[pointIndex + 1]
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_marks.cpp",
                                    321,
                                    0,
                                    "%s\n\t(dists[pointIndex]) = %g",
                                    "(dists[pointIndex] != dists[pointIndex + 1])",
                                    dists[pointIndex]) )
                    {
                        __debugbreak();
                    }
                    lerp = dists[pointIndex] / (float)(dists[pointIndex] - dists[pointIndex + 1]);
                    nextIndex = (pointIndex + 1) % inPointCount;
                    R_LerpModelMarkPoints(
                        (const FxModelMarkPoint *)&inPoints[pointIndex],
                        (const FxModelMarkPoint *)&inPoints[nextIndex],
                        lerp,
                        (FxModelMarkPoint *)&outPoints[outPointCount++]);
                }
            }
        }
        return outPointCount;
    }
    else
    {
        memcpy((unsigned __int8 *)outPoints, (unsigned __int8 *)inPoints, 24 * inPointCount);
        return inPointCount;
    }
}

void __cdecl R_LerpModelMarkPoints(
                const FxModelMarkPoint *from,
                const FxModelMarkPoint *to,
                float lerp,
                FxModelMarkPoint *out)
{
    Vec3Lerp(from->xyz, to->xyz, lerp, out->xyz);
    Vec3Lerp(from->vertWeights, to->vertWeights, lerp, out->vertWeights);
}

bool __cdecl R_MarkFragment_IsTriangleRejected(
                const float *markNormal,
                const float *xyz0,
                const float *xyz1,
                const float *xyz2)
{
    float edge01[3]; // [esp+0h] [ebp-28h] BYREF
    float scaledDot; // [esp+Ch] [ebp-1Ch]
    float scaledNormal[3]; // [esp+10h] [ebp-18h] BYREF
    float edge21[3]; // [esp+1Ch] [ebp-Ch] BYREF

    edge01[0] = *xyz0 - *xyz1;
    edge01[1] = xyz0[1] - xyz1[1];
    edge01[2] = xyz0[2] - xyz1[2];
    edge21[0] = *xyz2 - *xyz1;
    edge21[1] = xyz2[1] - xyz1[1];
    edge21[2] = xyz2[2] - xyz1[2];
    Vec3Cross(edge01, edge21, scaledNormal);
    scaledDot = (float)((float)(scaledNormal[0] * *markNormal) + (float)(scaledNormal[1] * markNormal[1]))
                        + (float)(scaledNormal[2] * markNormal[2]);
    return scaledDot < 0.0
            || (float)((float)((float)((float)(scaledNormal[0] * scaledNormal[0]) + (float)(scaledNormal[1] * scaledNormal[1]))
                                             + (float)(scaledNormal[2] * scaledNormal[2]))
                             * 0.25) > (float)(scaledDot * scaledDot);
}

void __cdecl R_MarkFragment_SetupWorldClipPoints(
                const GfxWorldVertex *triVerts0,
                const unsigned __int16 *indices,
                FxWorldMarkPoint (*clipPoints)[14])
{
    FxWorldMarkPoint *v3; // [esp+4h] [ebp-10h]
    const GfxWorldVertex *v4; // [esp+8h] [ebp-Ch]
    unsigned int pointIndex; // [esp+Ch] [ebp-8h]

    for ( pointIndex = 0; pointIndex < 3; ++pointIndex )
    {
        v3 = &(*clipPoints)[pointIndex];
        v4 = &triVerts0[indices[pointIndex]];
        v3->xyz[0] = v4->xyz[0];
        v3->xyz[1] = v4->xyz[1];
        v3->xyz[2] = v4->xyz[2];
        v3->vertWeights[0] = 0.0f;
        v3->vertWeights[1] = 0.0f;
        v3->vertWeights[2] = 0.0f;
        v3->vertWeights[pointIndex] = 1.0f;
    }
}

char __cdecl R_MarkFragments_EntBrushes(MarkInfo *markInfo)
{
    float invPoseMatrix[4][3]; // [esp+30h] [ebp-184h] BYREF
    float negatedPoseOrigin[3]; // [esp+60h] [ebp-154h] BYREF
    float transformedHitNormal[3]; // [esp+6Ch] [ebp-148h] BYREF
    float transformedTexCoordAxis[3]; // [esp+78h] [ebp-13Ch] BYREF
    int surfaceBegin; // [esp+84h] [ebp-130h]
    int surfaceIndex; // [esp+88h] [ebp-12Ch]
    int surfaceEnd; // [esp+8Ch] [ebp-128h]
    int brushModelCollidedIndex; // [esp+90h] [ebp-124h]
    MarkClipPlaneSet clipPlanes; // [esp+94h] [ebp-120h] BYREF
    int brushModelCollidedCount; // [esp+150h] [ebp-64h]
    MarkInfoCollidedBModel *brushModelCollided; // [esp+154h] [ebp-60h]
    float poseMatrix[4][3]; // [esp+158h] [ebp-5Ch] BYREF
    GfxBrushModel *brushModel; // [esp+188h] [ebp-2Ch]
    GfxMarkContext markContext; // [esp+18Ch] [ebp-28h] BYREF
    const GfxSurface *surface; // [esp+194h] [ebp-20h]
    float transformedOrigin[3]; // [esp+198h] [ebp-1Ch] BYREF
    bool anyMarks; // [esp+1A7h] [ebp-Dh] BYREF
    float markDir[3]; // [esp+1A8h] [ebp-Ch] BYREF

    PROF_SCOPED("markfragments entbrushes");

    brushModelCollidedCount = markInfo->sceneBModelCollidedCount;
    for ( brushModelCollidedIndex = 0; brushModelCollidedIndex != brushModelCollidedCount; ++brushModelCollidedIndex )
    {
        brushModelCollided = &markInfo->sceneBModelsCollided[brushModelCollidedIndex];
        markContext.modelTypeAndSurf = 0x80;
        markContext.modelIndex = markInfo->sceneBModelsCollided[brushModelCollidedIndex].entnum;
        CG_GetPoseAxis(markInfo->sceneBModelsCollided[brushModelCollidedIndex].pose, poseMatrix);
        CG_GetPoseOrigin(brushModelCollided->pose, poseMatrix[3]);
        R_Mark_TransformClipPlanes(&markInfo->clipPlanes, poseMatrix, &clipPlanes);
        MatrixTransposeTransformVector(markInfo->axis[0], poseMatrix, markDir);
        anyMarks = 0;
        brushModel = brushModelCollided->brushModel;
        surfaceBegin = brushModel->startSurfIndex;
        surfaceEnd = brushModel->surfaceCount + brushModel->startSurfIndex;
        for ( surfaceIndex = surfaceBegin; surfaceIndex != surfaceEnd; ++surfaceIndex )
        {
            surface = &rgp.world->dpvs.surfaces[surfaceIndex];
            if ( R_Mark_MaterialAllowsMarks(surface->material, markInfo->material)
                && !R_MarkFragments_BrushSurface(markInfo, &markContext, &clipPlanes, markDir, surface, &anyMarks) )
            {
                return 0;
            }
        }
        if ( anyMarks )
        {
            MatrixTranspose(poseMatrix, invPoseMatrix);
            negatedPoseOrigin[0] = -poseMatrix[3][0];
            negatedPoseOrigin[1] = -poseMatrix[3][1];
            negatedPoseOrigin[2] = -poseMatrix[3][2];
            MatrixTransformVector(negatedPoseOrigin, invPoseMatrix, invPoseMatrix[3]);
            MatrixTransformVector43(markInfo->origin, invPoseMatrix, transformedOrigin);
            MatrixTransposeTransformVector(markInfo->axis[1], poseMatrix, transformedTexCoordAxis);
            MatrixTransposeTransformVector(markInfo->axis[0], poseMatrix, transformedHitNormal);
            markInfo->callback(
                markInfo->callbackContext,
                markInfo->usedTriCount,
                markInfo->tris,
                markInfo->usedPointCount,
                markInfo->points,
                transformedOrigin,
                transformedTexCoordAxis,
                transformedHitNormal,
                markInfo->isSeeThruDecal);
            markInfo->usedTriCount = 0;
            markInfo->usedPointCount = 0;
        }
    }
    return 1;
}

void __cdecl R_Mark_TransformClipPlanes(
                const MarkClipPlaneSet *inClipPlanes,
                float (*matrix)[3],
                MarkClipPlaneSet *outClipPlanes)
{
    int planeIndex; // [esp+1Ch] [ebp-4h]

    outClipPlanes->planeCount = inClipPlanes->planeCount;
    for ( planeIndex = 0; planeIndex != inClipPlanes->planeCount; ++planeIndex )
    {
        outClipPlanes->planes[planeIndex][0] = (float)((float)(inClipPlanes->planes[planeIndex][0] * (*matrix)[0])
                                                                                                 + (float)(inClipPlanes->planes[planeIndex][1] * (*matrix)[1]))
                                                                                 + (float)(inClipPlanes->planes[planeIndex][2] * (*matrix)[2]);
        outClipPlanes->planes[planeIndex][1] = (float)((float)(inClipPlanes->planes[planeIndex][0] * (*matrix)[3])
                                                                                                 + (float)(inClipPlanes->planes[planeIndex][1] * (*matrix)[4]))
                                                                                 + (float)(inClipPlanes->planes[planeIndex][2] * (*matrix)[5]);
        outClipPlanes->planes[planeIndex][2] = (float)((float)(inClipPlanes->planes[planeIndex][0] * (*matrix)[6])
                                                                                                 + (float)(inClipPlanes->planes[planeIndex][1] * (*matrix)[7]))
                                                                                 + (float)(inClipPlanes->planes[planeIndex][2] * (*matrix)[8]);
        outClipPlanes->planes[planeIndex][3] = inClipPlanes->planes[planeIndex][3]
                                                                                 - (float)((float)((float)(inClipPlanes->planes[planeIndex][0] * (*matrix)[9])
                                                                                                                 + (float)(inClipPlanes->planes[planeIndex][1] * (*matrix)[10]))
                                                                                                 + (float)(inClipPlanes->planes[planeIndex][2] * (*matrix)[11]));
    }
}

char __cdecl R_MarkFragments_Glass(MarkInfo *markInfo)
{
    FxMarkPoint *v2; // [esp+8h] [ebp-190h]
    float v3; // [esp+10h] [ebp-188h]
    float v4; // [esp+18h] [ebp-180h]
    float v5; // [esp+1Ch] [ebp-17Ch]
    const float *origin; // [esp+48h] [ebp-150h]
    float pos_4; // [esp+54h] [ebp-144h]
    float pos_8; // [esp+58h] [ebp-140h]
    int p; // [esp+5Ch] [ebp-13Ch]
    GfxSurface *surface; // [esp+60h] [ebp-138h]
    unsigned int s; // [esp+64h] [ebp-134h]
    const GfxBrushModel *br; // [esp+68h] [ebp-130h]
    unsigned int i; // [esp+6Ch] [ebp-12Ch]
    MarkClipPlaneSet clipPlanes; // [esp+70h] [ebp-128h] BYREF
    float glassMatrix[4][3]; // [esp+12Ch] [ebp-6Ch] BYREF
    unsigned int num; // [esp+15Ch] [ebp-3Ch]
    GfxMarkContext markContext; // [esp+160h] [ebp-38h] BYREF
    const Glass *glasses[8]; // [esp+168h] [ebp-30h] BYREF
    bool anyMarks; // [esp+18Bh] [ebp-Dh] BYREF
    float markDir[3]; // [esp+18Ch] [ebp-Ch] BYREF

    PROF_SCOPED("markfragments glass");

    num = GlassCl_AreaGlasses(markInfo->mins, markInfo->maxs, glasses, 8u);
    for ( i = 0; i < num; ++i )
    {
        markContext.modelTypeAndSurf = 0;
        markContext.modelIndex = glasses[i]->index;
        AnglesToAxis(glasses[i]->angles, glassMatrix);
        origin = glasses[i]->origin;
        glassMatrix[3][0] = *origin;
        glassMatrix[3][1] = origin[1];
        glassMatrix[3][2] = origin[2];
        R_Mark_TransformClipPlanes(&markInfo->clipPlanes, glassMatrix, &clipPlanes);
        MatrixTransposeTransformVector(markInfo->axis[0], glassMatrix, markDir);
        anyMarks = 0;
        br = R_GetBrushModel(glasses[i]->brushModel);
        for ( s = 0; s < br->surfaceCount; ++s )
        {
            surface = &rgp.world->dpvs.surfaces[s + br->startSurfIndex];
            if ( R_Mark_MaterialAllowsMarks(surface->material, markInfo->material)
                && !R_MarkFragments_BrushSurface(markInfo, &markContext, &clipPlanes, markDir, surface, &anyMarks) )
            {
                return 0;
            }
        }
        if ( anyMarks )
        {
            for ( p = 0; p < markInfo->usedPointCount; ++p )
            {
                v5 = markInfo->points[p].xyz[0];
                v4 = markInfo->points[p].xyz[1];
                pos_4 = (float)(v4 * glassMatrix[1][1]) + (float)((float)(v5 * glassMatrix[0][1]) + glasses[i]->origin[1]);
                pos_8 = (float)(v4 * glassMatrix[1][2]) + (float)((float)(v5 * glassMatrix[0][2]) + glasses[i]->origin[2]);
                v2 = &markInfo->points[p];
                v3 = v2->xyz[2];
                v2->xyz[0] = (float)(v3 * glassMatrix[2][0])
                                     + (float)((float)(v4 * glassMatrix[1][0])
                                                     + (float)((float)(v5 * glassMatrix[0][0]) + glasses[i]->origin[0]));
                v2->xyz[1] = (float)(v3 * glassMatrix[2][1]) + pos_4;
                v2->xyz[2] = (float)(v3 * glassMatrix[2][2]) + pos_8;
            }
            markInfo->callback(
                markInfo->callbackContext,
                markInfo->usedTriCount,
                markInfo->tris,
                markInfo->usedPointCount,
                markInfo->points,
                (const float *)markInfo,
                markInfo->axis[1],
                (const float *)markInfo->axis,
                markInfo->isSeeThruDecal);
            markInfo->usedTriCount = 0;
            markInfo->usedPointCount = 0;
        }
    }
    return 1;
}

char __cdecl R_MarkFragments_Models(MarkInfo *markInfo)
{
    if ( markInfo->markHasLightmap
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_marks.cpp",
                    2353,
                    0,
                    "%s",
                    "!markInfo->markHasLightmap") )
    {
        __debugbreak();
    }
    if ( (markInfo->usedTriCount || markInfo->usedPointCount)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_marks.cpp",
                    2354,
                    0,
                    "%s",
                    "!( markInfo->usedTriCount || markInfo->usedPointCount )") )
    {
        __debugbreak();
    }
    if ( !R_MarkFragments_StaticModels(markInfo) )
        return 0;
    if ( (markInfo->usedTriCount || markInfo->usedPointCount)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_marks.cpp",
                    2357,
                    0,
                    "%s",
                    "!( markInfo->usedTriCount || markInfo->usedPointCount )") )
    {
        __debugbreak();
    }
    if ( !R_MarkFragments_SceneDObjs(markInfo) )
        return 0;
    if ( (markInfo->usedTriCount || markInfo->usedPointCount)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_marks.cpp",
                    2360,
                    0,
                    "%s",
                    "!( markInfo->usedTriCount || markInfo->usedPointCount )") )
    {
        __debugbreak();
    }
    return 1;
}

char __cdecl R_MarkFragments_SceneDObjs(MarkInfo *markInfo)
{
    int v1; // eax
    int submodelIndex; // [esp+50h] [ebp-44h]
    int boneIndex; // [esp+54h] [ebp-40h]
    DObj *dobj; // [esp+5Ch] [ebp-38h]
    const XModel *model; // [esp+64h] [ebp-30h]
    int submodelCount; // [esp+68h] [ebp-2Ch]
    unsigned int hidePartBits[5]; // [esp+6Ch] [ebp-28h] BYREF
    const DObjAnimMat *boneMtxList; // [esp+80h] [ebp-14h] BYREF
    GfxMarkContext markContext; // [esp+84h] [ebp-10h] BYREF
    unsigned __int16 entnum; // [esp+8Ch] [ebp-8h]
    int sceneDObjCollidedIndex; // [esp+90h] [ebp-4h]

    PROF_SCOPED("markfragments scenedobj");
    sceneDObjCollidedIndex = 0;
LABEL_2:
    if ( sceneDObjCollidedIndex == markInfo->sceneDObjCollidedCount )
    {
        return 1;
    }
    else
    {
        dobj = markInfo->sceneDObjsCollided[sceneDObjCollidedIndex].dObj;
        entnum = markInfo->sceneDObjsCollided[sceneDObjCollidedIndex].entnum;
        R_MarkUtil_GetDObjAnimMatAndHideParts(
            dobj,
            markInfo->sceneDObjsCollided[sceneDObjCollidedIndex].pose,
            &boneMtxList,
            hidePartBits);
        submodelCount = DObjGetNumModels(dobj);
        boneIndex = 0;
        for ( submodelIndex = 0; ; ++submodelIndex )
        {
            if ( submodelIndex == submodelCount )
            {
                ++sceneDObjCollidedIndex;
                goto LABEL_2;
            }
            markContext.reflectionProbeIndex = 0;
            markContext.primaryLightIndex = 0;
            markContext.modelIndex = entnum;
            markContext.modelTypeAndSurf = submodelIndex | 0xC0;
            if ( (submodelIndex & 0x3F) != submodelIndex
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_marks.cpp",
                            2286,
                            0,
                            "%s\n\t(submodelIndex) = %i",
                            "((markContext.modelTypeAndSurf & MARK_MODEL_SURF_MASK ) == submodelIndex)",
                            submodelIndex) )
            {
                __debugbreak();
            }
            model = DObjGetModel(dobj, submodelIndex);
            v1 = DObjNumBones(dobj);
            if ( !R_MarkFragments_AnimatedXModel(markInfo, model, hidePartBits, boneIndex, boneMtxList, v1, &markContext) )
                break;
            boneIndex += model->numBones;
            boneMtxList += model->numBones;
        }
        return 0;
    }
}

char __cdecl R_MarkFragments_AnimatedXModel(
                MarkInfo *markInfo,
                const XModel *model,
                const unsigned int *hidePartBits,
                int boneIndex,
                const DObjAnimMat *boneMtxList,
                int boneCount,
                GfxMarkContext *markContext)
{
    int boneOffset; // [esp+3B8h] [ebp-30h]
    const DObjAnimMat *baseMatList; // [esp+3CCh] [ebp-1Ch]
    int surfIndex; // [esp+3D0h] [ebp-18h]
    unsigned int vertListIndex; // [esp+3D4h] [ebp-14h]
    int surfCount; // [esp+3D8h] [ebp-10h]
    Material **materials; // [esp+3DCh] [ebp-Ch]
    XSurface *surfaces; // [esp+3E0h] [ebp-8h] BYREF
    XSurface *surface; // [esp+3E4h] [ebp-4h]
    int savedregs; // [esp+3E8h] [ebp+0h] BYREF

    surfCount = XModelGetSurfaces(model, &surfaces, 0);
    baseMatList = XModelGetBasePose(model);
    materials = XModelGetSkins(model, 0);
    if ( (markInfo->usedTriCount || markInfo->usedPointCount)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_marks.cpp",
                    2168,
                    0,
                    "%s",
                    "!markInfo->usedTriCount && !markInfo->usedPointCount") )
    {
        __debugbreak();
    }
    for ( surfIndex = 0; surfIndex != surfCount; ++surfIndex )
    {
        if ( R_Mark_MaterialAllowsMarks(materials[surfIndex], markInfo->material) )
        {
            surface = &surfaces[surfIndex];
            if ( (surface->flags & 0x80) == 0 )
            {
                if ( !surface->vertListCount
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_marks.cpp",
                                2182,
                                0,
                                "%s",
                                "surface->vertListCount > 0") )
                {
                    __debugbreak();
                }
                for ( vertListIndex = 0; vertListIndex != surface->vertListCount; ++vertListIndex )
                {
                    boneOffset = surface->vertList[vertListIndex].boneOffset >> 6;
                    if ( boneOffset + boneIndex >= boneCount
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_marks.cpp",
                                    2197,
                                    0,
                                    "%s",
                                    "(boneIndex + boneOffset) < boneCount") )
                    {
                        __debugbreak();
                    }
                    if ( (hidePartBits[(boneOffset + boneIndex) >> 5] & (0x80000000 >> ((boneOffset + boneIndex) & 0x1F))) == 0 )
                    {
                        markContext->lmapIndex = boneOffset;
                        if ( markContext->lmapIndex != boneOffset
                            && !Assert_MyHandler(
                                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_marks.cpp",
                                        2208,
                                        0,
                                        "%s\n\t(boneOffset) = %i",
                                        "(markContext->lmapIndex == boneOffset)",
                                        boneOffset) )
                        {
                            __debugbreak();
                        }
                        if ( !R_MarkFragments_AnimatedXModel_VertList(
                                        markInfo,
                                        vertListIndex,
                                        &boneMtxList[boneOffset],
                                        &baseMatList[boneOffset],
                                        markContext,
                                        surface) )
                            return 0;
                        if ( markInfo->usedTriCount || markInfo->usedPointCount )
                        {
                            if ( markInfo->usedTriCount )
                            {
                                if ( markInfo->usedPointCount )
                                    markInfo->callback(
                                        markInfo->callbackContext,
                                        markInfo->usedTriCount,
                                        markInfo->tris,
                                        markInfo->usedPointCount,
                                        markInfo->points,
                                        markInfo->localOrigin,
                                        markInfo->localTexCoordAxis,
                                        markInfo->localHitNormal,
                                        markInfo->isSeeThruDecal);
                            }
                            markInfo->usedTriCount = 0;
                            markInfo->usedPointCount = 0;
                        }
                    }
                }
            }
        }
    }
    if ( (markInfo->usedTriCount || markInfo->usedPointCount)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_marks.cpp",
                    2228,
                    0,
                    "%s",
                    "!markInfo->usedTriCount && !markInfo->usedPointCount") )
    {
        __debugbreak();
    }
    return 1;
}

// aislop
int __cdecl R_AddMarkFragment_0_(
    FxModelMarkPoint(*clipPoints)[14],
    const MarkClipPlaneSet *planes,
    const GfxMarkContext *markContext,
    unsigned __int16 baseIndex,
    int maxTris,
    FxMarkTri *tris,
    int maxPoints,
    PackedUnitVec **triNormals,
    const float (*transformNormalMatrix)[3], // intentionally unused
    FxMarkPoint *points)
{
    int pingPong = 0;
    int clipPointCount = 3;


    int planeIndex;
    int pointIndex;


    float normal[3][3];
    FxModelMarkPoint *clipPoint;
    FxMarkPoint *point;


    /* Clip triangle against all planes */
    for (planeIndex = 0; planeIndex < planes->planeCount; ++planeIndex)
    {
        clipPointCount = R_ChopPolyBehindPlane(
            clipPointCount,
            clipPoints[pingPong],
            clipPoints[pingPong ^ 1],
            planes->planes[planeIndex]);


        iassert(clipPointCount <= 14);


        pingPong ^= 1;


        if (!clipPointCount)
            return 0;
    }


    /* Capacity checks */
    if (clipPointCount > maxPoints || 3 * (clipPointCount - 2) > maxTris)
        return -1;


    /* Unpack triangle normals */
    Vec3UnpackUnitVec(*triNormals[0], normal[0]);
    Vec3UnpackUnitVec(*triNormals[1], normal[1]);
    Vec3UnpackUnitVec(*triNormals[2], normal[2]);


    /* Build mark points */
    for (pointIndex = 0; pointIndex < clipPointCount; ++pointIndex)
    {
        clipPoint = &clipPoints[pingPong][pointIndex];
        point = &points[pointIndex];


        Vec3Copy(clipPoint->xyz, point->xyz);


        point->lmapCoord[0] = 0.0f;
        point->lmapCoord[1] = 0.0f;


        point->normal[0] =
            clipPoint->vertWeights[0] * normal[0][0] +
            clipPoint->vertWeights[1] * normal[1][0] +
            clipPoint->vertWeights[2] * normal[2][0];


        point->normal[1] =
            clipPoint->vertWeights[0] * normal[0][1] +
            clipPoint->vertWeights[1] * normal[1][1] +
            clipPoint->vertWeights[2] * normal[2][1];


        point->normal[2] =
            clipPoint->vertWeights[0] * normal[0][2] +
            clipPoint->vertWeights[1] * normal[1][2] +
            clipPoint->vertWeights[2] * normal[2][2];
    }


    /* Emit triangles (fan) */
    for (pointIndex = 2; pointIndex < clipPointCount; ++pointIndex)
    {
        tris->indices[0] = baseIndex;
        tris->indices[1] = baseIndex + pointIndex - 1;
        tris->indices[2] = baseIndex + pointIndex;
        tris->context = *markContext;
        ++tris;
    }


    return clipPointCount;
}

char __cdecl R_MarkFragment_DoTriangle_0_(
    MarkInfo *markInfo,
    const MarkClipPlaneSet *clipPlanes,
    const GfxMarkContext *markContext,
    PackedUnitVec **triNormals,
    const float (*transformNormalMatrix)[3],
    FxModelMarkPoint(*clipPoints)[14])
{
    int fragmentPointCount; // [esp+0h] [ebp-4h]

    fragmentPointCount = R_AddMarkFragment_0_(
        clipPoints,
        clipPlanes,
        markContext,
        markInfo->usedPointCount,
        markInfo->maxTris - markInfo->usedTriCount,
        &markInfo->tris[markInfo->usedTriCount],
        markInfo->maxPoints - markInfo->usedPointCount,
        triNormals,
        transformNormalMatrix,
        &markInfo->points[markInfo->usedPointCount]);
    if (fragmentPointCount == -1)
        return 0;
    if (fragmentPointCount)
    {
        if (fragmentPointCount < 3
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_marks.cpp",
                1197,
                0,
                "%s\n\t(fragmentPointCount) = %i",
                "(fragmentPointCount >= 3)",
                fragmentPointCount))
        {
            __debugbreak();
        }
        markInfo->usedPointCount += fragmentPointCount;
        markInfo->usedTriCount = fragmentPointCount + markInfo->usedTriCount - 2;
    }
    return 1;
}

bool __cdecl R_MarkModelCoreCallback_0_(
    void *contextAsVoid,
    const unsigned __int8 **triVerts0,
    const unsigned __int8 **triVerts1)
{
    MarkModelCoreContext *context = (MarkModelCoreContext *)contextAsVoid;

    float *vertWeights; // [esp+2Ch] [ebp-2B8h]
    float *xyz; // [esp+30h] [ebp-2B4h]
    const float *pos; // [esp+34h] [ebp-2B0h]
    FxModelMarkPoint clipPoints[2][14]; // [esp+3Ch] [ebp-2A8h] BYREF
    int vertIndex; // [esp+2E0h] [ebp-4h]

    for (vertIndex = 0; vertIndex != 3; ++vertIndex)
    {
        pos = (const float*)triVerts0[vertIndex];
        xyz = clipPoints[0][vertIndex].xyz;
        *xyz = *pos;
        xyz[1] = pos[1];
        xyz[2] = pos[2];
        vertWeights = clipPoints[0][vertIndex].vertWeights;
        *vertWeights = 0.0f;
        vertWeights[1] = 0.0f;
        vertWeights[2] = 0.0f;
        clipPoints[0][vertIndex].vertWeights[vertIndex] = 1.0f;
    }
    return R_MarkFragment_IsTriangleRejected(
        context->markDir,
        clipPoints[0][0].xyz,
        clipPoints[0][1].xyz,
        clipPoints[0][2].xyz)
        || R_MarkFragment_DoTriangle_0_(
            context->markInfo,
            context->clipPlanes,
            context->markContext,
            (PackedUnitVec**)triVerts1,
            context->transformNormalMatrix,
            clipPoints) != 0;
}

#if 0
// local variable allocation has failed, the output may be wrong!
char    R_MarkFragments_AnimatedXModel_VertList(
                MarkInfo *markInfo,
                unsigned int vertListIndex,
                const DObjAnimMat *poseBone,
                const DObjAnimMat *baseBone,
                GfxMarkContext *markContext,
                XSurface *surface)
{
    unsigned int v8[2]; // [esp+18h] [ebp-35Ch] BYREF
    bool (__cdecl *visitorFunc)(void *, const unsigned __int8 **, const unsigned __int8 **); // [esp+20h] [ebp-354h]
    MarkModelCoreContext markModelCoreContext; // [esp+24h] [ebp-350h] BYREF
    float localMaxs[3]; // [esp+40h] [ebp-334h] BYREF
    float localMins[3]; // [esp+4Ch] [ebp-328h] BYREF
    float originalOrigin[3]; // [esp+58h] [ebp-31Ch] BYREF
    float invSurfaceMatrix[4][3]; // [esp+64h] [ebp-310h] BYREF
    float baseBoneMatrix[4][3]; // [esp+94h] [ebp-2E0h] BYREF
    float invPoseBoneMatrix[4][3]; // [esp+C4h] [ebp-2B0h] BYREF
    DObjSkelMat invPoseBoneSkelMat; // [esp+F4h] [ebp-280h] BYREF
    float v18[3]; // [esp+174h] [ebp-200h] BYREF
    float markDir[3]; // [esp+180h] [ebp-1F4h] BYREF
    _BYTE clipPlanes_172[12]; // [esp+238h] [ebp-13Ch] OVERLAPPED BYREF
    float surfaceMatrix[4][3]; // [esp+244h] [ebp-130h] BYREF
    float invBaseBoneMatrix[4][3]; // [esp+274h] [ebp-100h] BYREF
    float poseBoneMatrix[4][3]; // [esp+2A4h] [ebp-D0h] BYREF
    DObjSkelMat invBaseBoneSkelMat; // [esp+2D4h] [ebp-A0h] BYREF
    _BYTE poseBoneSkelMat_52[40]; // [esp+348h] [ebp-2Ch] OVERLAPPED BYREF
    _UNKNOWN *retaddr; // [esp+374h] [ebp+0h]

    *(unsigned int *)&poseBoneSkelMat_52[32] = a1;
    *(unsigned int *)&poseBoneSkelMat_52[36] = retaddr;
    memcpy(poseBoneSkelMat_52, poseBone, 0x20u);
    ConvertQuatToSkelMat((const DObjAnimMat *)poseBoneSkelMat_52, (DObjSkelMat *)&invBaseBoneSkelMat.origin[1]);
    ConvertQuatToInverseSkelMat(baseBone, (DObjSkelMat *)poseBoneMatrix[3]);
    DObjSkelMatToMatrix43((DObjSkelMat *)&invBaseBoneSkelMat.origin[1], (float (*)[3])invBaseBoneMatrix[3]);
    DObjSkelMatToMatrix43((const DObjSkelMat *)poseBoneMatrix[3], (float (*)[3])surfaceMatrix[3]);
    MatrixMultiply43(
        (const float (*)[3])surfaceMatrix[3],
        (const float (*)[3])invBaseBoneMatrix[3],
        (float (*)[3])clipPlanes_172);
    if ( markInfo->usedPointCount
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_marks.cpp",
                    2099,
                    0,
                    "%s",
                    "markInfo->usedPointCount == 0") )
    {
        __debugbreak();
    }
    R_Mark_TransformClipPlanes(&markInfo->clipPlanes, (float (*)[3])clipPlanes_172, (MarkClipPlaneSet *)markDir);
    MatrixTransposeTransformVector(markInfo->axis[0], (const float (*)[3])clipPlanes_172, v18);
    ConvertQuatToSkelMat(baseBone, (DObjSkelMat *)&invPoseBoneSkelMat.origin[1]);
    ConvertQuatToInverseSkelMat((const DObjAnimMat *)poseBoneSkelMat_52, (DObjSkelMat *)invPoseBoneMatrix[3]);
    DObjSkelMatToMatrix43((const DObjSkelMat *)invPoseBoneMatrix[3], (float (*)[3])baseBoneMatrix[3]);
    DObjSkelMatToMatrix43((DObjSkelMat *)&invPoseBoneSkelMat.origin[1], (float (*)[3])invSurfaceMatrix[3]);
    MatrixMultiply43(
        (const float (*)[3])baseBoneMatrix[3],
        (const float (*)[3])invSurfaceMatrix[3],
        (float (*)[3])originalOrigin);
    Vec3Copy(markInfo->origin, localMins);
    MatrixTransformVector43(localMins, (const float (*)[3])originalOrigin, markInfo->localOrigin);
    Vec3AddScalar(markInfo->localOrigin, -(markInfo->radius), localMaxs);
    Vec3AddScalar(markInfo->localOrigin, markInfo->radius, (float *)&markModelCoreContext.clipPlanes);
    v8[0] = markInfo;
    v8[1] = markContext;
    visitorFunc = (bool (__cdecl *)(void *, const unsigned __int8 **, const unsigned __int8 **))markInfo->localOrigin;
    markModelCoreContext.markInfo = (MarkInfo *)v18;
    markModelCoreContext.markContext = (GfxMarkContext *)markDir;

    if ( !XSurfaceVisitTrianglesInAabb(
                    surface,
                    vertListIndex,
                    localMaxs,
                    (const float *)&markModelCoreContext.clipPlanes,
                    (bool (__cdecl *)(void *, const unsigned __int8 **, const unsigned __int8 **))R_MarkModelCoreCallback_0_,
                    v8) )
        return 0;

    if ( markInfo->usedPointCount )
    {
        MatrixTransposeTransformVector(markInfo->axis[1], (const float (*)[3])clipPlanes_172, markInfo->localTexCoordAxis);
        MatrixTransposeTransformVector(markInfo->axis[0], (const float (*)[3])clipPlanes_172, markInfo->localHitNormal);
    }

    return 1;
}
#endif

bool R_MarkFragments_AnimatedXModel_VertList(
    MarkInfo *markInfo,
    unsigned int vertListIndex,
    const DObjAnimMat *poseBone,
    const DObjAnimMat *baseBone,
    GfxMarkContext *markContext,
    XSurface *surface)
{
    DObjAnimMat poseBoneWithViewOffset;
    DObjSkelMat poseBoneSkelMat;
    DObjSkelMat invBaseBoneSkelMat;
    DObjSkelMat baseBoneSkelMat;
    DObjSkelMat invPoseBoneSkelMat;

    float poseBoneMatrix[4][3];
    float invBaseBoneMatrix[4][3];
    float baseBoneMatrix[4][3];
    float invPoseBoneMatrix[4][3];
    float surfaceMatrix[4][3];
    float invSurfaceMatrix[4][3];

    MarkClipPlaneSet clipPlanes;
    float markDir[3];
    float originalOrigin[3];
    float localMins[3];
    float localMaxs[3];

    MarkModelCoreContext markModelCoreContext;

    memcpy(&poseBoneWithViewOffset, poseBone, sizeof(poseBoneWithViewOffset));

    ConvertQuatToSkelMat(&poseBoneWithViewOffset, &poseBoneSkelMat);
    ConvertQuatToInverseSkelMat(baseBone, &invBaseBoneSkelMat);
    DObjSkelMatToMatrix43(&poseBoneSkelMat, poseBoneMatrix);
    DObjSkelMatToMatrix43(&invBaseBoneSkelMat, invBaseBoneMatrix);
    MatrixMultiply43(invBaseBoneMatrix, poseBoneMatrix, surfaceMatrix);

    iassert(markInfo->usedPointCount == 0);

    R_Mark_TransformClipPlanes(&markInfo->clipPlanes, surfaceMatrix, &clipPlanes);
    MatrixTransposeTransformVector(markInfo->axis[0], surfaceMatrix, markDir);

    ConvertQuatToSkelMat(baseBone, &baseBoneSkelMat);
    ConvertQuatToInverseSkelMat(&poseBoneWithViewOffset, &invPoseBoneSkelMat);
    DObjSkelMatToMatrix43(&invPoseBoneSkelMat, invPoseBoneMatrix);
    DObjSkelMatToMatrix43(&baseBoneSkelMat, baseBoneMatrix);
    MatrixMultiply43(invPoseBoneMatrix, baseBoneMatrix, invSurfaceMatrix);

    Vec3Copy(markInfo->origin, originalOrigin);
    MatrixTransformVector43(originalOrigin, invSurfaceMatrix, markInfo->localOrigin);

    Vec3AddScalar(markInfo->localOrigin, -markInfo->radius, localMins);
    Vec3AddScalar(markInfo->localOrigin, markInfo->radius, localMaxs);

    markModelCoreContext.markInfo = markInfo;
    markModelCoreContext.markContext = markContext;
    markModelCoreContext.markOrigin = markInfo->localOrigin;
    markModelCoreContext.markDir = markDir;
    markModelCoreContext.clipPlanes = &clipPlanes;

    if (!XSurfaceVisitTrianglesInAabb(
            surface,
            vertListIndex,
            localMins,
            localMaxs,
            R_MarkModelCoreCallback_0_,
            &markModelCoreContext))
    {
        return false;
    }

    if (markInfo->usedPointCount)
    {
        MatrixTransposeTransformVector(markInfo->axis[1], surfaceMatrix, markInfo->localTexCoordAxis);
        MatrixTransposeTransformVector(markInfo->axis[0], surfaceMatrix, markInfo->localHitNormal);
    }

    return true;
}

char __cdecl R_MarkFragments_StaticModels(MarkInfo *markInfo)
{
    float v2; // [esp+60h] [ebp-6Ch]
    float v3; // [esp+68h] [ebp-64h]
    float v4; // [esp+6Ch] [ebp-60h]
    float v5; // [esp+70h] [ebp-5Ch]
    float v6; // [esp+78h] [ebp-54h]
    float v7; // [esp+7Ch] [ebp-50h]
    float v8; // [esp+80h] [ebp-4Ch]
    int smodelCollidedIndex; // [esp+90h] [ebp-3Ch]
    const GfxStaticModelDrawInst *smodelDraw; // [esp+94h] [ebp-38h]
    GfxMarkContext markContext; // [esp+98h] [ebp-34h] BYREF
    const XModel *xmodel; // [esp+A0h] [ebp-2Ch]
    float modelAxis[3][3]; // [esp+A4h] [ebp-28h] BYREF
    int smodelIndex; // [esp+C8h] [ebp-4h]

    PROF_SCOPED("markfragments smodel");

    for ( smodelCollidedIndex = 0; smodelCollidedIndex != markInfo->smodelCollidedCount; ++smodelCollidedIndex )
    {
        if ( (markInfo->usedTriCount || markInfo->usedPointCount)
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_marks.cpp",
                        2317,
                        0,
                        "%s",
                        "!markInfo->usedTriCount && !markInfo->usedPointCount") )
        {
            __debugbreak();
        }
        smodelIndex = markInfo->smodelsCollided[smodelCollidedIndex];
        smodelDraw = &rgp.world->dpvs.smodelDrawInsts[smodelIndex];
        xmodel = smodelDraw->model;
        markContext.lmapIndex = 31;
        markContext.reflectionProbeIndex = smodelDraw->reflectionProbeIndex;
        if ( markContext.reflectionProbeIndex != smodelDraw->reflectionProbeIndex
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_marks.cpp",
                        2327,
                        0,
                        "%s\n\t(smodelDraw->reflectionProbeIndex) = %i",
                        "(markContext.reflectionProbeIndex == smodelDraw->reflectionProbeIndex)",
                        smodelDraw->reflectionProbeIndex) )
        {
            __debugbreak();
        }
        markContext.primaryLightIndex = smodelDraw->primaryLightIndex;
        markContext.modelIndex = smodelIndex;
        if ( (unsigned __int16)smodelIndex != smodelIndex
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_marks.cpp",
                        2330,
                        0,
                        "%s\n\t(smodelIndex) = %i",
                        "(markContext.modelIndex == smodelIndex)",
                        smodelIndex) )
        {
            __debugbreak();
        }
        v2 = smodelDraw->placement.axis[0][2];
        v3 = smodelDraw->placement.axis[1][0];
        v4 = smodelDraw->placement.axis[1][1];
        v5 = smodelDraw->placement.axis[1][2];
        v6 = smodelDraw->placement.axis[2][0];
        v7 = smodelDraw->placement.axis[2][1];
        v8 = smodelDraw->placement.axis[2][2];
        *(_QWORD *)&modelAxis[0][0] = *(_QWORD *)&smodelDraw->placement.axis[0][0];
        modelAxis[0][2] = v2;
        modelAxis[1][0] = v3;
        modelAxis[1][1] = v4;
        modelAxis[1][2] = v5;
        modelAxis[2][0] = v6;
        modelAxis[2][1] = v7;
        modelAxis[2][2] = v8;
        if ( !R_MarkFragments_EntirelyRigidXModel(
                        markInfo,
                        xmodel,
                        modelAxis,
                        smodelDraw->placement.origin,
                        smodelDraw->placement.scale,
                        &markContext) )
        {
            return 0;
        }
        if ( markInfo->usedTriCount || markInfo->usedPointCount )
        {
            if ( markInfo->usedTriCount && markInfo->usedPointCount )
                markInfo->callback(
                    markInfo->callbackContext,
                    markInfo->usedTriCount,
                    markInfo->tris,
                    markInfo->usedPointCount,
                    markInfo->points,
                    (const float *)markInfo,
                    markInfo->axis[1],
                    (const float *)markInfo->axis,
                    markInfo->isSeeThruDecal);
            markInfo->usedTriCount = 0;
            markInfo->usedPointCount = 0;
        }
    }
    if ( (markInfo->usedTriCount || markInfo->usedPointCount)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_marks.cpp",
                    2346,
                    0,
                    "%s",
                    "!markInfo->usedTriCount && !markInfo->usedPointCount") )
    {
        __debugbreak();
    }
    return 1;
}

char __cdecl R_MarkFragments_EntirelyRigidXModel(
                MarkInfo *markInfo,
                const XModel *xmodel,
                const float (*modelAxis)[3],
                const float *modelOrigin,
                float modelScale,
                GfxMarkContext *markContext)
{
    int surfIndex; // [esp+C0h] [ebp-10h]
    int surfCount; // [esp+C4h] [ebp-Ch]
    Material **materials; // [esp+C8h] [ebp-8h]
    XSurface *surfaces; // [esp+CCh] [ebp-4h] BYREF

    surfCount = XModelGetSurfaces(xmodel, &surfaces, 0);
    materials = XModelGetSkins(xmodel, 0);
    for ( surfIndex = 0; surfIndex != surfCount && surfIndex <= 63; ++surfIndex )
    {
        if ( R_Mark_MaterialAllowsMarks(materials[surfIndex], markInfo->material) )
        {
            if ( (surfIndex & 0x3F) != surfIndex
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_marks.cpp",
                            2056,
                            0,
                            "%s\n\t(surfIndex) = %i",
                            "((surfIndex & MARK_MODEL_SURF_MASK) == surfIndex)",
                            surfIndex) )
            {
                __debugbreak();
            }
            markContext->modelTypeAndSurf = surfIndex | 0x40;
            if ( markContext->modelTypeAndSurf != (surfIndex | 0x40)
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_marks.cpp",
                            2058,
                            0,
                            "%s\n\t(surfIndex | MARK_MODEL_TYPE_WORLD_MODEL) = %i",
                            "(markContext->modelTypeAndSurf == (surfIndex | MARK_MODEL_TYPE_WORLD_MODEL))",
                            surfIndex | 0x40) )
            {
                __debugbreak();
            }
            if ( !R_MarkFragments_XModelSurface_Basic(
                            markInfo,
                            &surfaces[surfIndex],
                            modelAxis,
                            modelOrigin,
                            modelScale,
                            markContext) )
                return 0;
        }
    }
    return 1;
}

int __cdecl R_AddMarkFragment_1_(
    FxModelMarkPoint(*clipPoints)[14],
    const MarkClipPlaneSet *planes,
    const GfxMarkContext *markContext,
    unsigned __int16 baseIndex,
    int maxTris,
    FxMarkTri *tris,
    int maxPoints,
    PackedUnitVec **triNormals,
    const float (*transformNormalMatrix)[3],
    FxMarkPoint *points)
{
    PackedUnitVec v11; // [esp+54h] [ebp-8Ch] BYREF
    PackedUnitVec v12; // [esp+58h] [ebp-88h]
    PackedUnitVec v13; // [esp+5Ch] [ebp-84h]
    PackedUnitVec v14; // [esp+60h] [ebp-80h]
    PackedUnitVec v15; // [esp+68h] [ebp-78h]
    PackedUnitVec v16; // [esp+6Ch] [ebp-74h] BYREF
    PackedUnitVec v17; // [esp+70h] [ebp-70h]
    PackedUnitVec v18; // [esp+74h] [ebp-6Ch]
    PackedUnitVec v19; // [esp+78h] [ebp-68h]
    PackedUnitVec v20; // [esp+80h] [ebp-60h]
    float out[3]; // [esp+84h] [ebp-5Ch] BYREF
    PackedUnitVec v22; // [esp+88h] [ebp-58h]
    PackedUnitVec v23; // [esp+8Ch] [ebp-54h]
    PackedUnitVec in; // [esp+90h] [ebp-50h]
    float tempNormal[3]; // [esp+98h] [ebp-48h] BYREF
    float normal[3][3]; // [esp+A4h] [ebp-3Ch] BYREF
    FxModelMarkPoint *clipPoint; // [esp+C8h] [ebp-18h]
    int pointIndex; // [esp+CCh] [ebp-14h]
    int pingPong; // [esp+D0h] [ebp-10h]
    int clipPointCount; // [esp+D4h] [ebp-Ch]
    int planeIndex; // [esp+D8h] [ebp-8h]
    FxMarkPoint *point; // [esp+DCh] [ebp-4h]

    pingPong = 0;
    clipPointCount = 3;
    for (planeIndex = 0; planeIndex < planes->planeCount; ++planeIndex)
    {
        clipPointCount = R_ChopPolyBehindPlane(
            clipPointCount,
            &(*clipPoints)[14 * pingPong],
            &(*clipPoints)[14 * (pingPong == 0)],
            planes->planes[planeIndex]);
        if (clipPointCount > 14
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_marks.cpp",
                1031,
                0,
                "%s\n\t(clipPointCount) = %i",
                "(clipPointCount <= 3 + 11)",
                clipPointCount))
        {
            __debugbreak();
        }
        pingPong ^= 1u;
        if (!clipPointCount)
            return 0;
    }
    if (clipPointCount > maxPoints || 3 * (clipPointCount - 2) > maxTris)
        return -1;

    //v20.packed = (*triNormals)->packed;
    //out.packed = v20.packed;
    //v22.packed = v20.packed;
    //v23.packed = v20.packed;
    //in.packed = v20.packed;
    Vec3UnpackUnitVec(*triNormals[0], out);
    //in.packed = 0.0f;
    tempNormal[0] = out[0];
    tempNormal[1] = out[1];
    tempNormal[2] = out[2];
    MatrixTransformVector(tempNormal, transformNormalMatrix, normal[0]);

    //v15.packed = triNormals[1]->packed;
    //v16.packed = v15.packed;
    //v17.packed = v15.packed;
    //v18.packed = v15.packed;
    //v19.packed = v15.packed;
    Vec3UnpackUnitVec(*triNormals[1], out);
    //v19.packed = 0.0f;
    tempNormal[0] = out[0];
    tempNormal[1] = out[1];
    tempNormal[2] = out[2];
    MatrixTransformVector(tempNormal, transformNormalMatrix, normal[1]);

    //v11.packed = triNormals[2]->packed;
    //v12.packed = v11.packed;
    //v13.packed = v11.packed;
    //v14.packed = v11.packed;
    Vec3UnpackUnitVec(*triNormals[2], out);
    //v14.packed = 0.0f;
    tempNormal[0] = out[0];
    tempNormal[1] = out[1];
    tempNormal[2] = out[2];
    MatrixTransformVector(tempNormal, transformNormalMatrix, normal[2]);

    for (pointIndex = 0; pointIndex < clipPointCount; ++pointIndex)
    {
        point = &points[pointIndex];
        clipPoint = &(*clipPoints)[14 * pingPong + pointIndex];
        point->xyz[0] = clipPoint->xyz[0];
        point->xyz[1] = clipPoint->xyz[1];
        point->xyz[2] = clipPoint->xyz[2];
        point->lmapCoord[0] = 0.0f;
        point->lmapCoord[1] = 0.0f;
        point->normal[0] = ((clipPoint->vertWeights[0] * normal[0][0]) + (clipPoint->vertWeights[1] * normal[1][0]))
            + (clipPoint->vertWeights[2] * normal[2][0]);
        point->normal[1] = ((clipPoint->vertWeights[0] * normal[0][1]) + (clipPoint->vertWeights[1] * normal[1][1]))
            + (clipPoint->vertWeights[2] * normal[2][1]);
        point->normal[2] = ((clipPoint->vertWeights[0] * normal[0][2]) + (clipPoint->vertWeights[1] * normal[1][2]))
            + (clipPoint->vertWeights[2] * normal[2][2]);
    }
    for (pointIndex = 2; pointIndex < clipPointCount; ++pointIndex)
    {
        tris->indices[0] = baseIndex + pointIndex - 1;
        tris->indices[1] = pointIndex + baseIndex;
        tris->indices[2] = baseIndex;
        tris->context = *markContext;
        ++tris;
    }
    return clipPointCount;
}

char __cdecl R_MarkFragment_DoTriangle_1_(
    MarkInfo *markInfo,
    const MarkClipPlaneSet *clipPlanes,
    const GfxMarkContext *markContext,
    PackedUnitVec **triNormals,
    const float (*transformNormalMatrix)[3],
    FxModelMarkPoint(*clipPoints)[14])
{
    int fragmentPointCount; // [esp+0h] [ebp-4h]

    fragmentPointCount = R_AddMarkFragment_1_(
        clipPoints,
        clipPlanes,
        markContext,
        markInfo->usedPointCount,
        markInfo->maxTris - markInfo->usedTriCount,
        &markInfo->tris[markInfo->usedTriCount],
        markInfo->maxPoints - markInfo->usedPointCount,
        triNormals,
        transformNormalMatrix,
        &markInfo->points[markInfo->usedPointCount]);
    if (fragmentPointCount == -1)
        return 0;
    if (fragmentPointCount)
    {
        if (fragmentPointCount < 3
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_marks.cpp",
                1197,
                0,
                "%s\n\t(fragmentPointCount) = %i",
                "(fragmentPointCount >= 3)",
                fragmentPointCount))
        {
            __debugbreak();
        }
        markInfo->usedPointCount += fragmentPointCount;
        markInfo->usedTriCount = fragmentPointCount + markInfo->usedTriCount - 2;
    }
    return 1;
}

bool __cdecl R_MarkModelCoreCallback_1_(
    void *contextAsVoid,
    const unsigned __int8 **triVerts0,
    PackedUnitVec **triVerts1)
{
    MarkModelCoreContext *context = (MarkModelCoreContext *)contextAsVoid;

    float *vertWeights; // [esp+2Ch] [ebp-2B8h]
    FxModelMarkPoint clipPoints[2][14]; // [esp+3Ch] [ebp-2A8h] BYREF
    int vertIndex; // [esp+2E0h] [ebp-4h]

    for (vertIndex = 0; vertIndex != 3; ++vertIndex)
    {
        MatrixTransformVector43((const float*)triVerts0[vertIndex], context->transformMatrix, clipPoints[0][vertIndex].xyz);
        vertWeights = clipPoints[0][vertIndex].vertWeights;
        *vertWeights = 0.0f;
        vertWeights[1] = 0.0f;
        vertWeights[2] = 0.0f;
        clipPoints[0][vertIndex].vertWeights[vertIndex] = 1.0f;
    }
    return R_MarkFragment_IsTriangleRejected(
        context->markDir,
        clipPoints[0][0].xyz,
        clipPoints[0][1].xyz,
        clipPoints[0][2].xyz)
        || R_MarkFragment_DoTriangle_1_(
            context->markInfo,
            context->clipPlanes,
            context->markContext,
            triVerts1,
            context->transformNormalMatrix,
            clipPoints) != 0;
}

char __cdecl R_MarkFragments_XModelSurface_Basic(
                MarkInfo *markInfo,
                const XSurface *surface,
                const float (*modelAxis)[3],
                const float *modelOrigin,
                float modelScale,
                GfxMarkContext *markContext)
{
    float *v7; // [esp+8h] [ebp-A4h]
    float *v8; // [esp+Ch] [ebp-A0h]
    float localRadius; // [esp+10h] [ebp-9Ch]
    MarkModelCoreContext markModelCoreContext; // [esp+14h] [ebp-98h] BYREF
    float invModelScale; // [esp+30h] [ebp-7Ch]
    float localOrigin[3]; // [esp+34h] [ebp-78h]
    unsigned int vertListIndex; // [esp+40h] [ebp-6Ch]
    float localMaxs[3]; // [esp+44h] [ebp-68h] BYREF
    float localOriginRotated[3]; // [esp+50h] [ebp-5Ch] BYREF
    int dim; // [esp+5Ch] [ebp-50h]
    float localMins[3]; // [esp+60h] [ebp-4Ch] BYREF
    bool (__cdecl *visitorFunc)(void *, const unsigned __int8 **, const unsigned __int8 **); // [esp+6Ch] [ebp-40h]
    float surfTransform[4][3]; // [esp+70h] [ebp-3Ch] BYREF
    float localOriginTranslated[3]; // [esp+A0h] [ebp-Ch] BYREF

    for ( dim = 0; dim != 3; ++dim )
    {
        v7 = surfTransform[dim];
        v8 = (float *)&(*modelAxis)[3 * dim];
        *v7 = modelScale * *v8;
        v7[1] = modelScale * v8[1];
        v7[2] = modelScale * v8[2];
    }
    surfTransform[3][0] = *modelOrigin;
    surfTransform[3][1] = modelOrigin[1];
    surfTransform[3][2] = modelOrigin[2];
    invModelScale = 1.0 / modelScale;
    localOriginTranslated[0] = markInfo->origin[0] - *modelOrigin;
    localOriginTranslated[1] = markInfo->origin[1] - modelOrigin[1];
    localOriginTranslated[2] = markInfo->origin[2] - modelOrigin[2];
    MatrixTransposeTransformVector(localOriginTranslated, modelAxis, localOriginRotated);
    localOrigin[0] = (float)(1.0 / modelScale) * localOriginRotated[0];
    localOrigin[1] = (float)(1.0 / modelScale) * localOriginRotated[1];
    localOrigin[2] = (float)(1.0 / modelScale) * localOriginRotated[2];
    localRadius = markInfo->radius * (float)(1.0 / modelScale);
    localMins[0] = localOrigin[0] + -(localRadius);
    localMins[1] = localOrigin[1] + -(localRadius);
    localMins[2] = localOrigin[2] + -(localRadius);
    localMaxs[0] = localOrigin[0] + localRadius;
    localMaxs[1] = localOrigin[1] + localRadius;
    localMaxs[2] = localOrigin[2] + localRadius;
    markModelCoreContext.markInfo = markInfo;
    markModelCoreContext.markContext = markContext;
    markModelCoreContext.markOrigin = (const float *)markInfo;
    markModelCoreContext.markDir = markInfo->axis[0];
    markModelCoreContext.clipPlanes = &markInfo->clipPlanes;
    markModelCoreContext.transformMatrix = surfTransform;
    markModelCoreContext.transformNormalMatrix = modelAxis;
    visitorFunc = (bool (__cdecl *)(void *, const unsigned __int8 **, const unsigned __int8 **))R_MarkModelCoreCallback_1_;
    for ( vertListIndex = 0; vertListIndex != surface->vertListCount; ++vertListIndex )
    {
        if ( !XSurfaceVisitTrianglesInAabb(surface, vertListIndex, localMins, localMaxs, visitorFunc, &markModelCoreContext) )
            return 0;
    }
    return 1;
}

int __cdecl R_ChopPolyBehindPlane(
                int inPointCount,
                FxModelMarkPoint *inPoints,
                FxModelMarkPoint *outPoints,
                const float *plane)
{
    const FxModelMarkPoint *v5; // eax
    FxModelMarkPoint *v6; // ecx
    const FxModelMarkPoint *v7; // eax
    FxModelMarkPoint *v8; // ecx
    int sideCount[3]; // [esp+Ch] [ebp-94h] BYREF
    float dists[15]; // [esp+18h] [ebp-88h]
    int nextIndex; // [esp+54h] [ebp-4Ch]
    float lerp; // [esp+58h] [ebp-48h]
    int pointIndex; // [esp+5Ch] [ebp-44h]
    int sides[15]; // [esp+60h] [ebp-40h]
    int outPointCount; // [esp+9Ch] [ebp-4h]

    if ( inPointCount > 14
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_marks.cpp",
                    173,
                    0,
                    "%s\n\t(inPointCount) = %i",
                    "(inPointCount <= 3 + 11)",
                    inPointCount) )
    {
        __debugbreak();
    }
    memset(sideCount, 0, sizeof(sideCount));
    for ( pointIndex = 0; pointIndex < inPointCount; ++pointIndex )
    {
        dists[pointIndex] = (float)((float)((float)(inPoints[pointIndex].xyz[0] * *plane)
                                                                            + (float)(inPoints[pointIndex].xyz[1] * plane[1]))
                                                            + (float)(inPoints[pointIndex].xyz[2] * plane[2]))
                                            - plane[3];
        if ( dists[pointIndex] <= 0.5 )
        {
            if ( dists[pointIndex] >= -0.5 )
                sides[pointIndex] = 2;
            else
                sides[pointIndex] = 1;
        }
        else
        {
            sides[pointIndex] = 0;
        }
        ++sideCount[sides[pointIndex]];
    }
    sides[pointIndex] = sides[0];
    dists[pointIndex] = dists[0];
    if ( !sideCount[0] )
        return 0;
    if ( sideCount[1] )
    {
        outPointCount = 0;
        for ( pointIndex = 0; pointIndex < inPointCount; ++pointIndex )
        {
            if ( sides[pointIndex] == 2 )
            {
                if ( outPointCount >= 14
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_marks.cpp",
                                214,
                                0,
                                "%s\n\t(outPointCount) = %i",
                                "(outPointCount < 3 + 11)",
                                outPointCount) )
                {
                    __debugbreak();
                }
                v5 = &inPoints[pointIndex];
                v6 = &outPoints[outPointCount];
                v6->xyz[0] = v5->xyz[0];
                v6->xyz[1] = v5->xyz[1];
                v6->xyz[2] = v5->xyz[2];
                v6->vertWeights[0] = v5->vertWeights[0];
                v6->vertWeights[1] = v5->vertWeights[1];
                v6->vertWeights[2] = v5->vertWeights[2];
                ++outPointCount;
            }
            else
            {
                if ( !sides[pointIndex] )
                {
                    if ( outPointCount >= 14
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_marks.cpp",
                                    222,
                                    0,
                                    "%s\n\t(outPointCount) = %i",
                                    "(outPointCount < 3 + 11)",
                                    outPointCount) )
                    {
                        __debugbreak();
                    }
                    v7 = &inPoints[pointIndex];
                    v8 = &outPoints[outPointCount];
                    v8->xyz[0] = v7->xyz[0];
                    v8->xyz[1] = v7->xyz[1];
                    v8->xyz[2] = v7->xyz[2];
                    v8->vertWeights[0] = v7->vertWeights[0];
                    v8->vertWeights[1] = v7->vertWeights[1];
                    v8->vertWeights[2] = v7->vertWeights[2];
                    ++outPointCount;
                }
                if ( sides[pointIndex + 1] != 2 && sides[pointIndex + 1] != sides[pointIndex] )
                {
                    if ( outPointCount >= 14
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_marks.cpp",
                                    231,
                                    0,
                                    "%s\n\t(outPointCount) = %i",
                                    "(outPointCount < 3 + 11)",
                                    outPointCount) )
                    {
                        __debugbreak();
                    }
                    if ( dists[pointIndex] == dists[pointIndex + 1]
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_marks.cpp",
                                    232,
                                    0,
                                    "%s\n\t(dists[pointIndex]) = %g",
                                    "(dists[pointIndex] != dists[pointIndex + 1])",
                                    dists[pointIndex]) )
                    {
                        __debugbreak();
                    }
                    lerp = dists[pointIndex] / (float)(dists[pointIndex] - dists[pointIndex + 1]);
                    nextIndex = (pointIndex + 1) % inPointCount;
                    R_LerpModelMarkPoints(&inPoints[pointIndex], &inPoints[nextIndex], lerp, &outPoints[outPointCount++]);
                }
            }
        }
        return outPointCount;
    }
    else
    {
        memcpy((unsigned __int8 *)outPoints, (unsigned __int8 *)inPoints, 24 * inPointCount);
        return inPointCount;
    }
}

