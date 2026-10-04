#include "r_staticmodelcache.h"
#include <xanim/xmodel_utils.h>
#include "r_model_lighting.h"
#include "r_dobj_skin.h"
#include "r_dvars.h"
#include <qcommon/threads.h>
#include <universal/com_workercmds.h>
#include "r_workercmds.h"
#include "rb_logfile.h"
#include "r_xsurface.h"

static_model_cache_t s_cache;

void __cdecl R_SkinCachedStaticModelCmd(SkinCachedStaticModelCmd *skinCmd)
{
    float v1; // [esp+94h] [ebp-FCh]
    float v2; // [esp+A0h] [ebp-F0h]
    float scale; // [esp+A8h] [ebp-E8h]
    float4 useAxis[4]; // [esp+C0h] [ebp-D0h] BYREF
    GfxStaticModelDrawInst *smodelDrawInst; // [esp+104h] [ebp-8Ch]
    unsigned int surfIndex; // [esp+108h] [ebp-88h]
    int baseVertIndex; // [esp+10Ch] [ebp-84h]
    float4 normAxis[4]; // [esp+110h] [ebp-80h] BYREF
    const GfxCachedSModelSurf *cachedSurf; // [esp+154h] [ebp-3Ch]
    unsigned int surfCount; // [esp+158h] [ebp-38h]
    const static_model_leaf_t *leaf; // [esp+15Ch] [ebp-34h]
    GfxSModelCachedVertex *verts; // [esp+160h] [ebp-30h]
    const XSurface *xsurf; // [esp+164h] [ebp-2Ch]
    int fixedNormAxis[3][3]; // [esp+168h] [ebp-28h] BYREF
    XSurface *surfs; // [esp+18Ch] [ebp-4h] BYREF

    if ( !skinCmd
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_staticmodelcache.cpp", 868, 0, "%s", "skinCmd") )
    {
        __debugbreak();
    }
    if ( !rgp.world
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_staticmodelcache.cpp",
                    869,
                    0,
                    "%s",
                    "rgp.world") )
    {
        __debugbreak();
    }
    verts = &frontEndDataOut->smcPatchVerts[skinCmd->firstPatchVert];
    leaf = SMC_GetLeaf(skinCmd->cacheIndex);
    cachedSurf = &leaf->cachedSurf;
    if ( leaf->cachedSurf.smodelIndex >= rgp.world->dpvs.smodelCount
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_staticmodelcache.cpp",
                    877,
                    0,
                    "cachedSurf->smodelIndex doesn't index rgp.world->dpvs.smodelCount\n\t%i not in [0, %i)",
                    cachedSurf->smodelIndex,
                    rgp.world->dpvs.smodelCount) )
    {
        __debugbreak();
    }
    smodelDrawInst = &rgp.world->dpvs.smodelDrawInsts[cachedSurf->smodelIndex];
    *(_QWORD *)normAxis[0].v = *(_QWORD *)&smodelDrawInst->placement.axis[0][0];
    normAxis[0].u[2] = LODWORD(smodelDrawInst->placement.axis[0][2]);
    normAxis[0].u[3] = 0;
    normAxis[1].v[0] = smodelDrawInst->placement.axis[1][0];
    normAxis[1].v[1] = smodelDrawInst->placement.axis[1][1];
    normAxis[1].v[2] = smodelDrawInst->placement.axis[1][2];
    normAxis[1].u[3] = 0;
    normAxis[2].v[0] = smodelDrawInst->placement.axis[2][0];
    normAxis[2].v[1] = smodelDrawInst->placement.axis[2][1];
    normAxis[2].v[2] = smodelDrawInst->placement.axis[2][2];
    normAxis[2].u[3] = 0;
    scale = smodelDrawInst->placement.scale;
    useAxis[0].v[0] = scale * normAxis[0].v[0];
    useAxis[0].v[1] = scale * normAxis[0].v[1];
    useAxis[0].v[2] = scale * normAxis[0].v[2];
    useAxis[0].v[3] = scale * 0.0;
    v2 = smodelDrawInst->placement.scale;
    useAxis[1].v[0] = v2 * normAxis[1].v[0];
    useAxis[1].v[1] = v2 * normAxis[1].v[1];
    useAxis[1].v[2] = v2 * normAxis[1].v[2];
    useAxis[1].v[3] = v2 * 0.0;
    v1 = smodelDrawInst->placement.scale;
    useAxis[2].v[0] = v1 * normAxis[2].v[0];
    useAxis[2].v[1] = v1 * normAxis[2].v[1];
    useAxis[2].v[2] = v1 * normAxis[2].v[2];
    useAxis[2].v[3] = v1 * 0.0;
    useAxis[3].v[0] = smodelDrawInst->placement.origin[0];
    useAxis[3].v[1] = smodelDrawInst->placement.origin[1];
    useAxis[3].v[2] = smodelDrawInst->placement.origin[2];
    useAxis[3].u[3] = 0;
    SetupTransformUnitVec(normAxis, fixedNormAxis);
    surfCount = XModelGetSurfCount(smodelDrawInst->model, cachedSurf->lodIndex);
    XModelGetSurfaces(smodelDrawInst->model, &surfs, cachedSurf->lodIndex);
    for ( surfIndex = 0; surfIndex < surfCount; ++surfIndex )
    {
        xsurf = &surfs[surfIndex];
        baseVertIndex = cachedSurf->baseVertIndex + xsurf->baseVertIndex;
        R_SkinXSurfaceStaticVerts(
            useAxis,
            fixedNormAxis,
            baseVertIndex,
            xsurf->vertCount,
            xsurf->verts0,
            cachedSurf->smodelIndex,
            &verts[xsurf->baseVertIndex]);
    }
}

static_model_leaf_t *__cdecl SMC_GetLeaf(unsigned int cacheIndex)
{
    if ( !cacheIndex
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_staticmodelcache.cpp",
                    545,
                    0,
                    "%s",
                    "cacheIndex") )
    {
        __debugbreak();
    }
    // cacheIndex is 1-based; map flat into the 2D leafs array.
    return &s_cache.leafs[(cacheIndex - 1) >> 5][(cacheIndex - 1) & 0x1F];
}

void __cdecl SetupTransformUnitVec(const float4 *mtx, int (*fixedMtx)[3])
{
    // KB decompilation fix (MP and zombies): fld x; fmul 32768; fistp - round to nearest (no bias here), not truncate
    // (SP 0x0070ba30, 9 fistp).
    (*fixedMtx)[0] = (int)lrint((double)mtx->v[0] * 32768.0);
    (*fixedMtx)[1] = (int)lrint((double)mtx->v[1] * 32768.0);
    (*fixedMtx)[2] = (int)lrint((double)mtx->v[2] * 32768.0);
    (*fixedMtx)[3] = (int)lrint((double)mtx[1].v[0] * 32768.0);
    (*fixedMtx)[4] = (int)lrint((double)mtx[1].v[1] * 32768.0);
    (*fixedMtx)[5] = (int)lrint((double)mtx[1].v[2] * 32768.0);
    (*fixedMtx)[6] = (int)lrint((double)mtx[2].v[0] * 32768.0);
    (*fixedMtx)[7] = (int)lrint((double)mtx[2].v[1] * 32768.0);
    (*fixedMtx)[8] = (int)lrint((double)mtx[2].v[2] * 32768.0);
}

void __cdecl R_SkinXSurfaceStaticVerts(
                const float4 *useAxis,
                const int (*normAxis)[3],
                unsigned int baseVertIndex,
                unsigned int vertCount,
                const GfxPackedVertex *srcVertArray,
                unsigned int smodelIndex,
                GfxSModelCachedVertex *verts)
{
    PackedUnitVec v7; // [esp+24h] [ebp-14h]
    PackedUnitVec v8; // [esp+28h] [ebp-10h]
    const GfxPackedVertex *srcVert; // [esp+2Ch] [ebp-Ch]
    PackedLightingCoords packedBaseLighting; // [esp+30h] [ebp-8h] BYREF
    unsigned int vertIndex; // [esp+34h] [ebp-4h]

    R_GetPackedStaticModelLightingCoords(smodelIndex, &packedBaseLighting);
    for ( vertIndex = 0; vertIndex < vertCount; ++vertIndex )
    {
        srcVert = &srcVertArray[vertIndex];
        R_TransformSkelMat(srcVert->xyz, useAxis, verts[vertIndex].xyz);
        v8.packed = LocalTransformUnitVec(srcVert->normal, normAxis).packed;
        verts[vertIndex].normal = v8;
        verts[vertIndex].color.packed = srcVert->color.packed;
        verts[vertIndex].texCoord.packed = srcVert->texCoord.packed;
        v7.packed = LocalTransformUnitVec(srcVert->tangent, normAxis).packed;
        verts[vertIndex].tangent = v7;
        packedBaseLighting.array[3] = ((SLODWORD(srcVert->binormalSign) >> 30) & 0xFE) + 2;
        verts[vertIndex].baseLighting = packedBaseLighting;
    }
}

PackedUnitVec __cdecl LocalTransformUnitVec(PackedUnitVec in, const int (*fixedMtx)[3])
{
    PackedUnitVec out; // [esp+0h] [ebp-10h]

    out.array[0] = ((*fixedMtx)[3] * (in.array[3] + 192) * (in.array[1] - 127)
                                + (*fixedMtx)[0] * (in.array[3] + 192) * (in.array[0] - 127)
                                + (*fixedMtx)[6] * (in.array[3] + 192) * (in.array[2] - 127)
                                + 1069547520) >> 23;
    out.array[1] = ((*fixedMtx)[4] * (in.array[3] + 192) * (in.array[1] - 127)
                                + (*fixedMtx)[1] * (in.array[3] + 192) * (in.array[0] - 127)
                                + (*fixedMtx)[7] * (in.array[3] + 192) * (in.array[2] - 127)
                                + 1069547520) >> 23;
    out.array[2] = ((*fixedMtx)[5] * (in.array[3] + 192) * (in.array[1] - 127)
                                + (*fixedMtx)[2] * (in.array[3] + 192) * (in.array[0] - 127)
                                + (*fixedMtx)[8] * (in.array[3] + 192) * (in.array[2] - 127)
                                + 1069547520) >> 23;
    out.array[3] = 64;
    return out;
}

static_model_leaf_t *__cdecl R_GetCachedSModelSurf(unsigned int cacheIndex)
{
    static_model_leaf_t *leaf; // [esp+0h] [ebp-4h]

    if ( !cacheIndex
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_staticmodelcache.cpp",
                    919,
                    0,
                    "%s",
                    "cacheIndex") )
    {
        __debugbreak();
    }
    leaf = SMC_GetLeaf(cacheIndex);
    if ( leaf->cachedSurf.smodelIndex >= rgp.world->dpvs.smodelCount
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_staticmodelcache.cpp",
                    922,
                    0,
                    "leaf->cachedSurf.smodelIndex doesn't index rgp.world->dpvs.smodelCount\n\t%i not in [0, %i)",
                    leaf->cachedSurf.smodelIndex,
                    rgp.world->dpvs.smodelCount) )
    {
        __debugbreak();
    }
    return leaf;
}

unsigned __int16 __cdecl R_CacheStaticModelSurface(
                unsigned int smcIndex,
                unsigned int smodelIndex,
                const XModelLodInfo *lodInfo)
{
    static_model_tree_t *tree; // [esp+8h] [ebp-18h]
    static_model_tree_t *treea; // [esp+8h] [ebp-18h]
    static_model_leaf_t *cachedSurf; // [esp+Ch] [ebp-14h]
    static_model_leaf_t *cachedSurfa; // [esp+Ch] [ebp-14h]
    unsigned __int16 cacheIndex; // [esp+14h] [ebp-Ch]
    unsigned __int16 cacheIndexa; // [esp+14h] [ebp-Ch]
    SkinCachedStaticModelCmd skinSmodelCmd; // [esp+18h] [ebp-8h] BYREF
    unsigned int cachedVertsNeeded; // [esp+1Ch] [ebp-4h]

    if ( !lodInfo
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_staticmodelcache.cpp", 943, 0, "%s", "lodInfo") )
    {
        __debugbreak();
    }
    if ( (lodInfo->smcAllocBits < 4u || lodInfo->smcAllocBits > 9u)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_staticmodelcache.cpp",
                    944,
                    0,
                    "lodInfo->smcAllocBits not in [SMC_MIN_ALLOC_BITS, SMC_MAX_ALLOC_BITS]\n\t%i not in [%i, %i]",
                    lodInfo->smcAllocBits,
                    4,
                    9) )
    {
        __debugbreak();
    }
    if ( !r_smc_enable->current.enabled
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_staticmodelcache.cpp",
                    947,
                    0,
                    "%s",
                    "r_smc_enable->current.enabled") )
    {
        __debugbreak();
    }
    if ( !Sys_IsMainThread()
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_staticmodelcache.cpp",
                    949,
                    0,
                    "%s",
                    "Sys_IsMainThread()") )
    {
        __debugbreak();
    }
    if ( !Sys_QueryD3DDeviceOKEvent() )
        return 0;
    cacheIndex = rgp.world->dpvs.smodelDrawInsts[smodelIndex].smodelCacheIndex[lodInfo->lod];
    if ( cacheIndex )
    {
        cachedSurf = SMC_GetLeaf(cacheIndex);
        tree = &s_cache.trees[(cachedSurf - &s_cache.leafs[0][0]) / 32];
        if ( tree->frameCount != rg.frontEndFrameCount )
        {
            tree->frameCount = rg.frontEndFrameCount;
            tree->usedlist.next->prev = tree->usedlist.prev;
            tree->usedlist.prev->next = tree->usedlist.next;
            tree->usedlist.prev = &s_cache.usedlist[smcIndex];
            tree->usedlist.next = s_cache.usedlist[smcIndex].next;
            tree->usedlist.prev->next = &tree->usedlist;
            tree->usedlist.next->prev = &tree->usedlist;
        }
        if ( cachedSurf->cachedSurf.smodelIndex == 0xFFFF
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_staticmodelcache.cpp",
                        984,
                        0,
                        "%s",
                        "cachedSurf->smodelIndex != SMODEL_INDEX_NONE") )
        {
            __debugbreak();
        }
        return cacheIndex;
    }
    else
    {
        cachedVertsNeeded = 1 << lodInfo->smcAllocBits;
        if ( frontEndDataOut->smcPatchCount == 256 || cachedVertsNeeded + frontEndDataOut->smcPatchVertsUsed > 0x2000 )
        {
            return 0;
        }
        else
        {
            cacheIndexa = SMC_Allocate(smcIndex, lodInfo->smcAllocBits);
            if ( cacheIndexa )
            {
                frontEndDataOut->smcPatchList[frontEndDataOut->smcPatchCount++] = cacheIndexa;
                cachedSurfa = SMC_GetLeaf(cacheIndexa);
                rgp.world->dpvs.smodelDrawInsts[smodelIndex].smodelCacheIndex[lodInfo->lod] = truncate_cast<unsigned short>(cacheIndexa);
                cachedSurfa->cachedSurf.smodelIndex = truncate_cast<unsigned short>(smodelIndex);
                cachedSurfa->cachedSurf.lodIndex = lodInfo->lod;
                skinSmodelCmd.cacheIndex = cacheIndexa;
                skinSmodelCmd.firstPatchVert = truncate_cast<unsigned short>(frontEndDataOut->smcPatchVertsUsed);
                frontEndDataOut->smcPatchVertsUsed += cachedVertsNeeded;
                Sys_AddWorkerCmdInternal(&r_skin_cached_staticmodelWorkerCmd, (unsigned __int8 *)&skinSmodelCmd, 0);
                R_CacheStaticModelIndices(
                    cachedSurfa->cachedSurf.smodelIndex,
                    cachedSurfa->cachedSurf.lodIndex,
                    cachedSurfa->cachedSurf.baseVertIndex);
                treea = &s_cache.trees[(cachedSurfa - &s_cache.leafs[0][0]) / 32];
                treea->frameCount = rg.frontEndFrameCount;
                treea->usedlist.next->prev = treea->usedlist.prev;
                treea->usedlist.prev->next = treea->usedlist.next;
                treea->usedlist.prev = &s_cache.usedlist[smcIndex];
                treea->usedlist.next = s_cache.usedlist[smcIndex].next;
                treea->usedlist.prev->next = &treea->usedlist;
                treea->usedlist.next->prev = &treea->usedlist;
                return cacheIndexa;
            }
            else
            {
                return 0;
            }
        }
    }
}

unsigned __int16 __cdecl SMC_Allocate(unsigned int smcIndex, unsigned int bitCount)
{
    static_model_node_list_t *block; // [esp+8h] [ebp-2Ch]
    unsigned int listIndex; // [esp+Ch] [ebp-28h]
    static_model_tree_t *tree; // [esp+10h] [ebp-24h]
    unsigned int nodeIndex; // [esp+14h] [ebp-20h]
    static_model_leaf_t *leafs; // [esp+1Ch] [ebp-18h]
    static_model_node_list_t *freelist; // [esp+20h] [ebp-14h]
    unsigned int index; // [esp+24h] [ebp-10h]
    unsigned int treeIndex; // [esp+30h] [ebp-4h]

    if ( (bitCount < 4 || bitCount > 9)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_staticmodelcache.cpp",
                    571,
                    0,
                    "bitCount not in [SMC_MIN_ALLOC_BITS, SMC_MAX_ALLOC_BITS]\n\t%i not in [%i, %i]",
                    bitCount,
                    4,
                    9) )
    {
        __debugbreak();
    }
    listIndex = 9 - bitCount;
    freelist = &s_cache.freelist[smcIndex][9 - bitCount];
    if ( freelist->next == freelist && !SMC_GetFreeBlockOfSize(smcIndex, listIndex) )
        return 0;
    block = freelist->next;
    block->next->prev = block->prev;
    block->prev->next = block->next;
    treeIndex = ((static_model_leaf_t *)block - &s_cache.leafs[0][0]) / 32;
    if ( treeIndex >= 0x200
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_staticmodelcache.cpp",
                    596,
                    0,
                    "treeIndex doesn't index ARRAY_COUNT( s_cache.trees )\n\t%i not in [0, %i)",
                    treeIndex,
                    512) )
    {
        __debugbreak();
    }
    tree = &s_cache.trees[treeIndex];
    if ( !listIndex )
    {
        tree->usedlist.prev = &s_cache.usedlist[smcIndex];
        tree->usedlist.next = s_cache.usedlist[smcIndex].next;
        tree->usedlist.prev->next = &tree->usedlist;
        tree->usedlist.next->prev = &tree->usedlist;
    }
    leafs = s_cache.leafs[treeIndex];
    index = ((char *)block - (char *)leafs) / 8;
    if ( index >= 0x20
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_staticmodelcache.cpp",
                    613,
                    0,
                    "index doesn't index ARRAY_COUNT( s_cache.leafs[treeIndex] )\n\t%i not in [0, %i)",
                    index,
                    32) )
    {
        __debugbreak();
    }
    if ( block != (static_model_node_list_t *)&leafs[index]
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_staticmodelcache.cpp",
                    614,
                    0,
                    "%s\n\t(index) = %i",
                    "(block == &leafs[index].freenode)",
                    index) )
    {
        __debugbreak();
    }
    nodeIndex = ((index + 32) >> (5 - listIndex)) - 1;
    if ( nodeIndex >= 0x3F
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_staticmodelcache.cpp",
                    622,
                    0,
                    "nodeIndex doesn't index ARRAY_COUNT( tree->nodes )\n\t%i not in [0, %i)",
                    nodeIndex,
                    63) )
    {
        __debugbreak();
    }
    tree->nodes[nodeIndex].inuse = 1;
    while ( 1 )
    {
        tree->nodes[nodeIndex].usedVerts += 1 << bitCount;
        if ( !nodeIndex )
            break;
        nodeIndex = (nodeIndex - 1) >> 1;
    }
    if ( &leafs[index] != SMC_GetLeaf((unsigned __int16)(32 * treeIndex + index + 1))
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_staticmodelcache.cpp",
                    640,
                    0,
                    "%s",
                    "&leafs[index] == SMC_GetLeaf( cacheIndex )") )
    {
        __debugbreak();
    }
    leafs[index].cachedSurf.baseVertIndex = 16 * index + (treeIndex << 9);
    return 32 * treeIndex + index + 1;
}

char __cdecl SMC_GetFreeBlockOfSize(unsigned int smcIndex, unsigned int listIndex)
{
    static_model_node_list_t *block; // [esp+Ch] [ebp-1Ch]
    static_model_node_list_t *blocka; // [esp+Ch] [ebp-1Ch]
    static_model_tree_t *tree; // [esp+10h] [ebp-18h]
    static_model_leaf_t *leafs; // [esp+14h] [ebp-14h]
    static_model_node_list_t *freelist; // [esp+18h] [ebp-10h]
    unsigned int index; // [esp+1Ch] [ebp-Ch]
    unsigned int treeIndex; // [esp+24h] [ebp-4h]

    if ( listIndex >= 6
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_staticmodelcache.cpp",
                    472,
                    0,
                    "listIndex doesn't index ARRAY_COUNT( s_cache.freelist[smcIndex] )\n\t%i not in [0, %i)",
                    listIndex,
                    6) )
    {
        __debugbreak();
    }
    if ( s_cache.freelist[smcIndex][listIndex].next != &s_cache.freelist[smcIndex][listIndex]
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_staticmodelcache.cpp",
                    473,
                    0,
                    "%s",
                    "s_cache.freelist[smcIndex][listIndex].next == &s_cache.freelist[smcIndex][listIndex]") )
    {
        __debugbreak();
    }
    if ( s_cache.freelist[smcIndex][listIndex].prev != &s_cache.freelist[smcIndex][listIndex]
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_staticmodelcache.cpp",
                    474,
                    0,
                    "%s",
                    "s_cache.freelist[smcIndex][listIndex].prev == &s_cache.freelist[smcIndex][listIndex]") )
    {
        __debugbreak();
    }
    if ( !listIndex )
        return SMC_ForceFreeBlock(smcIndex);
    // Hex-Rays addressed s_cache.freelist[smcIndex][listIndex - 1] as
    // s_cache.leafs[511][31 + 6*smcIndex + listIndex] (one slot past leafs);
    // simplified to the freelist sentinel head for size-class (listIndex - 1).
    freelist = &s_cache.freelist[smcIndex][listIndex - 1];
    if ( freelist->next == freelist
        && !SMC_GetFreeBlockOfSize(smcIndex, listIndex - 1) )
    {
        return 0;
    }
    block = freelist->next;
    if ( block == freelist
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_staticmodelcache.cpp",
                    495,
                    0,
                    "%s",
                    "block != freelist") )
    {
        __debugbreak();
    }
    block->next->prev = block->prev;
    block->prev->next = block->next;
    treeIndex = ((static_model_leaf_t *)block - &s_cache.leafs[0][0]) / 32;
    if ( treeIndex >= 0x200
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_staticmodelcache.cpp",
                    506,
                    0,
                    "treeIndex doesn't index ARRAY_COUNT( s_cache.trees )\n\t%i not in [0, %i)",
                    treeIndex,
                    512) )
    {
        __debugbreak();
    }
    tree = &s_cache.trees[treeIndex];
    if ( listIndex == 1 )
    {
        tree->usedlist.prev = &s_cache.usedlist[smcIndex];
        tree->usedlist.next = s_cache.usedlist[smcIndex].next;
        tree->usedlist.prev->next = &tree->usedlist;
        tree->usedlist.next->prev = &tree->usedlist;
    }
    leafs = s_cache.leafs[treeIndex];
    index = ((char *)block - (char *)leafs) / 8;
    if ( index >= 0x20
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_staticmodelcache.cpp",
                    521,
                    0,
                    "index doesn't index ARRAY_COUNT( s_cache.leafs[treeIndex] )\n\t%i not in [0, %i)",
                    index,
                    32) )
    {
        __debugbreak();
    }
    if ( block != (static_model_node_list_t *)&leafs[index]
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_staticmodelcache.cpp",
                    522,
                    0,
                    "%s\n\t(index) = %i",
                    "(block == &leafs[index].freenode)",
                    index) )
    {
        __debugbreak();
    }
    block->prev = &s_cache.freelist[smcIndex][listIndex];
    block->next = s_cache.freelist[smcIndex][listIndex].next;
    block->prev->next = block;
    block->next->prev = block;
    blocka = (static_model_node_list_t *)&leafs[index + (1 << (5 - listIndex))];
    blocka->prev = &s_cache.freelist[smcIndex][listIndex];
    blocka->next = s_cache.freelist[smcIndex][listIndex].next;
    blocka->prev->next = blocka;
    blocka->next->prev = blocka;
    return 1;
}

char __cdecl SMC_ForceFreeBlock(unsigned int smcIndex)
{
    static_model_leaf_t *leafs; // [esp+8h] [ebp-8h]
    static_model_tree_t *treenode; // [esp+Ch] [ebp-4h]

    // s_cache.usedlist[smcIndex].prev is a tree_list_t* that lives at offset 0
    // of a static_model_tree_t (or is the sentinel itself); upcast to tree_t*.
    treenode = (static_model_tree_t *)s_cache.usedlist[smcIndex].prev;
    if ( treenode == (static_model_tree_t *)&s_cache.usedlist[smcIndex]
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_staticmodelcache.cpp",
                    421,
                    0,
                    "%s",
                    "treenode != &s_cache.usedlist[smcIndex]") )
    {
        __debugbreak();
    }
    if ( rg.frontEndFrameCount - treenode->frameCount < 4 )
        return 0;
    if ( treenode->nodes[0].usedVerts <= 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_staticmodelcache.cpp",
                    432,
                    0,
                    "%s\n\t(tree->nodes[0].usedVerts) = %i",
                    "(tree->nodes[0].usedVerts > 0)",
                    treenode->nodes[0].usedVerts) )
    {
        __debugbreak();
    }
    leafs = s_cache.leafs[treenode - s_cache.trees];
    SMC_FreeCachedSurface_r(treenode, leafs, 0, 5);
    treenode->usedlist.next->prev = treenode->usedlist.prev;
    treenode->usedlist.prev->next = treenode->usedlist.next;
    // Insert leafs at the head of freelist[smcIndex][0] (the largest size class).
    leafs->freenode.prev = &s_cache.freelist[smcIndex][0];
    leafs->freenode.next = s_cache.freelist[smcIndex][0].next;
    s_cache.freelist[smcIndex][0].next = &leafs->freenode;
    leafs->freenode.next->prev = &leafs->freenode;
    return 1;
}

void __cdecl SMC_FreeCachedSurface_r(
                static_model_tree_t *tree,
                static_model_leaf_t *leafs,
                int nodeIndex,
                int levelsToLeaf)
{
    static_model_leaf_t *cachedSurf; // [esp+0h] [ebp-Ch]
    static_model_leaf_t *freenode; // [esp+8h] [ebp-4h]

    if ( tree->nodes[nodeIndex].usedVerts )
    {
        tree->nodes[nodeIndex].usedVerts = 0;
        if ( tree->nodes[nodeIndex].inuse )
        {
            cachedSurf = &leafs[((nodeIndex + 1) << levelsToLeaf) - 32];
            if ( cachedSurf->cachedSurf.smodelIndex != 0xFFFF )
            {
                rgp.world->dpvs.smodelDrawInsts[cachedSurf->cachedSurf.smodelIndex].smodelCacheIndex[cachedSurf->cachedSurf.lodIndex] = 0;
                cachedSurf->cachedSurf.smodelIndex = -1;
            }
            tree->nodes[nodeIndex].inuse = 0;
        }
        else
        {
            SMC_FreeCachedSurface_r(tree, leafs, 2 * nodeIndex + 1, levelsToLeaf - 1);
            SMC_FreeCachedSurface_r(tree, leafs, 2 * nodeIndex + 2, levelsToLeaf - 1);
        }
    }
    else
    {
        // Unlink freenode from its freelist (the leaf's union doubles as a list node).
        freenode = &leafs[((nodeIndex + 1) << levelsToLeaf) - 32];
        freenode->freenode.next->prev = freenode->freenode.prev;
        freenode->freenode.prev->next = freenode->freenode.next;
    }
}

void __cdecl R_CacheStaticModelIndices(unsigned int smodelIndex, unsigned int lod, unsigned int cacheBaseVertIndex)
{
    unsigned int surfIndex; // [esp+0h] [ebp-2Ch]
    unsigned int baseIndex; // [esp+8h] [ebp-24h]
    XModel *model; // [esp+Ch] [ebp-20h]
    unsigned int surfCount; // [esp+10h] [ebp-1Ch]
    unsigned int *twoSrcIndices; // [esp+14h] [ebp-18h]
    unsigned int *twoSrcIndicesa; // [esp+14h] [ebp-18h]
    int iterationCount; // [esp+18h] [ebp-14h]
    const XSurface *xsurf; // [esp+1Ch] [ebp-10h]
    XSurface *surfs; // [esp+20h] [ebp-Ch] BYREF
    unsigned int *twoDstIndices; // [esp+24h] [ebp-8h]
    unsigned int twoBaseOffsets; // [esp+28h] [ebp-4h]

    model = rgp.world->dpvs.smodelDrawInsts[smodelIndex].model;
    XModelGetSurfaces(model, &surfs, lod);
    surfCount = XModelGetSurfCount(model, lod);
    for ( surfIndex = 0; surfIndex < surfCount; ++surfIndex )
    {
        xsurf = &surfs[surfIndex];
        twoBaseOffsets = (unsigned __int16)(cacheBaseVertIndex + xsurf->baseVertIndex)
                                     | ((unsigned __int16)(cacheBaseVertIndex + xsurf->baseVertIndex) << 16);
        twoSrcIndices = (unsigned int *)xsurf->triIndices;
        baseIndex = 3 * xsurf->baseTriIndex + 4 * cacheBaseVertIndex;
        if ( baseIndex >= 0x100000
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_staticmodelcache.cpp",
                        827,
                        0,
                        "%s",
                        "baseIndex < SMC_MAX_INDEX_IN_CACHE") )
        {
            __debugbreak();
        }
        if ( baseIndex + 3 * xsurf->triCount > 0x100000
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_staticmodelcache.cpp",
                        828,
                        0,
                        "%s",
                        "baseIndex + xsurf->triCount * 3 <= SMC_MAX_INDEX_IN_CACHE") )
        {
            __debugbreak();
        }
        twoDstIndices = (unsigned int *)&gfxBuf.smodelCache.indices[baseIndex];
        iterationCount = xsurf->triCount / 2;
        if ( 2 * iterationCount != xsurf->triCount
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_staticmodelcache.cpp",
                        835,
                        0,
                        "%s",
                        "iterationCount * 2 == xsurf->triCount") )
        {
            __debugbreak();
        }
        if ( !iterationCount
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_staticmodelcache.cpp",
                        836,
                        0,
                        "%s",
                        "iterationCount") )
        {
            __debugbreak();
        }
        do
        {
            *twoDstIndices++ = *twoSrcIndices + twoBaseOffsets;
            twoSrcIndicesa = twoSrcIndices + 1;
            *twoDstIndices++ = *twoSrcIndicesa + twoBaseOffsets;
            *twoDstIndices++ = *++twoSrcIndicesa + twoBaseOffsets;
            twoSrcIndices = twoSrcIndicesa + 1;
            --iterationCount;
        }
        while ( iterationCount );
    }
}

void __cdecl R_InitStaticModelCache()
{
    R_AllocStaticModelCache();
    SMC_ClearCache();
}

void *R_AllocStaticModelCache()
{
    if ( gfxBuf.smodelCacheVb
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_staticmodelcache.cpp",
                    1066,
                    0,
                    "%s",
                    "!gfxBuf.smodelCacheVb") )
    {
        __debugbreak();
    }
    return R_AllocDynamicVertexBuffer(&gfxBuf.smodelCacheVb, 0x800000);
}

unsigned int SMC_ClearCache()
{
    unsigned int result; // eax
    static_model_leaf_t *v1; // [esp+4h] [ebp-14h]
    unsigned int treeIter; // [esp+8h] [ebp-10h]
    unsigned int treeItera; // [esp+8h] [ebp-10h]
    unsigned int leafIter; // [esp+Ch] [ebp-Ch]
    unsigned int listIter; // [esp+10h] [ebp-8h]
    unsigned int smcIter; // [esp+14h] [ebp-4h]

    for ( treeIter = 0; treeIter < 0x200; ++treeIter )
    {
        for ( leafIter = 0; leafIter < 0x20; ++leafIter )
            s_cache.leafs[treeIter][leafIter].cachedSurf.smodelIndex = -1;
        result = treeIter + 1;
    }
    for ( smcIter = 0; smcIter < 4; ++smcIter )
    {
        // Empty usedlist: sentinel head points to itself.
        s_cache.usedlist[smcIter].prev = &s_cache.usedlist[smcIter];
        result = smcIter;
        s_cache.usedlist[smcIter].next = &s_cache.usedlist[smcIter];
        // Empty per-size-class freelists: each sentinel points to itself.
        for ( listIter = 0; listIter < 6; ++listIter )
        {
            s_cache.freelist[smcIter][listIter].prev = &s_cache.freelist[smcIter][listIter];
            s_cache.freelist[smcIter][listIter].next = &s_cache.freelist[smcIter][listIter];
            result = listIter + 1;
        }
        // Insert each owned tree's first leaf into freelist[smcIter][0].
        for ( treeItera = 0; treeItera < 0x80; ++treeItera )
        {
            v1 = s_cache.leafs[treeItera + (smcIter << 7)];
            v1->freenode.prev = &s_cache.freelist[smcIter][0];
            v1->freenode.next = s_cache.freelist[smcIter][0].next;
            s_cache.freelist[smcIter][0].next = &v1->freenode;
            result = (unsigned int)v1;
            v1->freenode.next->prev = &v1->freenode;
        }
    }
    return result;
}

void __cdecl R_FlushStaticModelCache()
{
    static_model_tree_list_t *next; // [esp+4h] [ebp-10h]
    unsigned int smcIter; // [esp+Ch] [ebp-8h]
    static_model_leaf_t *leafs; // [esp+10h] [ebp-4h]

    if ( s_cache.usedlist[0].next )
    {
        for ( smcIter = 0; smcIter < 4; ++smcIter )
        {
            while ( s_cache.usedlist[smcIter].next != &s_cache.usedlist[smcIter] )
            {
                if ( !s_cache.usedlist[smcIter].next
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_staticmodelcache.cpp",
                                1145,
                                0,
                                "%s",
                                "s_cache.usedlist[smcIter].next") )
                {
                    __debugbreak();
                }
                // usedlist link lives at offset 0 of static_model_tree_t — upcast.
                static_model_tree_t *tree = (static_model_tree_t *)s_cache.usedlist[smcIter].next;
                leafs = s_cache.leafs[tree - s_cache.trees];
                SMC_FreeCachedSurface_r(tree, leafs, 0, 5);
                next = s_cache.usedlist[smcIter].next;
                next->next->prev = next->prev;
                next->prev->next = next->next;
                // Insert leafs at head of freelist[smcIter][0].
                leafs->freenode.prev = &s_cache.freelist[smcIter][0];
                leafs->freenode.next = s_cache.freelist[smcIter][0].next;
                s_cache.freelist[smcIter][0].next = &leafs->freenode;
                leafs->freenode.next->prev = &leafs->freenode;
            }
        }
        SMC_ClearCache();
    }
}

void __cdecl R_ClearAllStaticModelCacheRefs()
{
    unsigned int smodelCount; // [esp+0h] [ebp-8h]
    unsigned int smodelIndex; // [esp+4h] [ebp-4h]

    if ( rgp.world )
    {
        smodelCount = rgp.world->dpvs.smodelCount;
        for ( smodelIndex = 0; smodelIndex < smodelCount; ++smodelIndex )
            R_UncacheStaticModel(smodelIndex);
    }
}

void __cdecl R_UncacheStaticModel(unsigned int smodelIndex)
{
    GfxStaticModelDrawInst *smodelDrawInst; // [esp+0h] [ebp-14h]
    static_model_leaf_t *cachedSurf; // [esp+4h] [ebp-10h]
    unsigned int lod; // [esp+Ch] [ebp-8h]

    smodelDrawInst = &rgp.world->dpvs.smodelDrawInsts[smodelIndex];
    for ( lod = 0; lod < 4; ++lod )
    {
        if ( smodelDrawInst->smodelCacheIndex[lod] )
        {
            cachedSurf = SMC_GetLeaf(smodelDrawInst->smodelCacheIndex[lod]);
            if ( cachedSurf->cachedSurf.smodelIndex != smodelIndex
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_staticmodelcache.cpp",
                            1225,
                            0,
                            "cachedSurf->smodelIndex == smodelIndex\n\t%i, %i",
                            cachedSurf->cachedSurf.smodelIndex,
                            smodelIndex) )
            {
                __debugbreak();
            }
            cachedSurf->cachedSurf.smodelIndex = -1;
            smodelDrawInst->smodelCacheIndex[lod] = 0;
        }
    }
}

void __cdecl R_ShutdownStaticModelCache()
{
    IDirect3DVertexBuffer9 *varCopy; // [esp+0h] [ebp-4h]

    R_FlushStaticModelCache();
    if ( gfxBuf.smodelCacheVb )
    {
        do
        {
            if ( r_logFile )
            {
                if ( r_logFile->current.integer )
                    RB_LogPrint("gfxBuf.smodelCacheVb->Release()\n");
            }
            varCopy = gfxBuf.smodelCacheVb;
            gfxBuf.smodelCacheVb = 0;
            R_ReleaseAndSetNULL<IDirect3DIndexBuffer9>(
                (IDirect3DSurface9 *)varCopy,
                "gfxBuf.smodelCacheVb",
                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_staticmodelcache.cpp",
                1310);
        }
        while ( alwaysfails );
    }
}

void __cdecl R_StaticModelCacheStats_f()
{
    unsigned int usedCount; // [esp+24h] [ebp-24h]
    unsigned int lodIter; // [esp+28h] [ebp-20h]
    unsigned int allocCount; // [esp+2Ch] [ebp-1Ch]
    unsigned int surfCount; // [esp+30h] [ebp-18h]
    unsigned int smodelIter; // [esp+34h] [ebp-14h]
    unsigned int lodCount; // [esp+38h] [ebp-10h]
    const GfxStaticModelDrawInst *drawInst; // [esp+3Ch] [ebp-Ch]
    unsigned int surfIter; // [esp+40h] [ebp-8h]
    XSurface *surfs; // [esp+44h] [ebp-4h] BYREF

    if ( rgp.world )
    {
        allocCount = 0;
        usedCount = 0;
        for ( smodelIter = 0; smodelIter < rgp.world->dpvs.smodelCount; ++smodelIter )
        {
            drawInst = &rgp.world->dpvs.smodelDrawInsts[smodelIter];
            lodCount = XModelGetNumLods(drawInst->model);
            for ( lodIter = 0; lodIter < lodCount; ++lodIter )
            {
                if ( drawInst->smodelCacheIndex[lodIter] )
                {
                    allocCount += 1 << XModelGetLodInfo(drawInst->model, lodIter)->smcAllocBits;
                    surfCount = XModelGetSurfCount(drawInst->model, lodIter);
                    XModelGetSurfaces(drawInst->model, &surfs, lodIter);
                    for ( surfIter = 0; surfIter < surfCount; ++surfIter )
                        usedCount += XSurfaceGetNumVerts(&surfs[surfIter]);
                }
            }
        }
        Com_Printf(8, "%.2f%% of cache is currently allocated.\n", (double)allocCount * 100.0 / 262144.0);
        if ( allocCount )
            Com_Printf(8, "%.2f%% allocated cache vertices are used.\n", (double)usedCount * 100.0 / (double)allocCount);
    }
}

void __cdecl R_StaticModelCacheFlush_f()
{
    R_SyncRenderThread();
    R_ClearAllStaticModelCacheRefs();
}

