#pragma once
#include <xanim/xmodel.h>
#include "rb_backend.h"

struct SkinCachedStaticModelCmd // sizeof=0x4
{                                       // XREF: ?R_CacheStaticModelSurface@@YAGIIPBUXModelLodInfo@@@Z/r
    unsigned __int16 cacheIndex;        // XREF: R_CacheStaticModelSurface(uint,uint,XModelLodInfo const *)+302/w
    unsigned __int16 firstPatchVert;    // XREF: R_CacheStaticModelSurface(uint,uint,XModelLodInfo const *)+31A/w
};

struct static_model_cache_t // sizeof=0x410E0
{                                       // XREF: .data:s_cache/r
    static_model_tree_t trees[512];
    static_model_leaf_t leafs[512][32]; // XREF: R_CacheStaticModelSurface(uint,uint,XModelLodInfo const *)+133/o
                                        // R_CacheStaticModelSurface(uint,uint,XModelLodInfo const *)+36D/o ...
    static_model_node_list_t freelist[4][6]; // XREF: SMC_Allocate+50/o
                                        // SMC_GetFreeBlockOfSize+42/o ...
    static_model_tree_list_t usedlist[4]; // XREF: SMC_ForceFreeBlock+9/r
                                        // SMC_ClearCache+7C/w ...
};

void __cdecl R_SkinCachedStaticModelCmd(SkinCachedStaticModelCmd *skinCmd);
static_model_leaf_t *__cdecl SMC_GetLeaf(unsigned int cacheIndex);
void __cdecl SetupTransformUnitVec(const float4 *mtx, int (*fixedMtx)[3]);
void __cdecl R_SkinXSurfaceStaticVerts(
                const float4 *useAxis,
                const int (*normAxis)[3],
                unsigned int baseVertIndex,
                unsigned int vertCount,
                const GfxPackedVertex *srcVertArray,
                unsigned int smodelIndex,
                GfxSModelCachedVertex *verts);
PackedUnitVec __cdecl LocalTransformUnitVec(PackedUnitVec in, const int (*fixedMtx)[3]);
static_model_leaf_t *__cdecl R_GetCachedSModelSurf(unsigned int cacheIndex);
unsigned __int16 __cdecl R_CacheStaticModelSurface(
                unsigned int smcIndex,
                unsigned int smodelIndex,
                const XModelLodInfo *lodInfo);
unsigned __int16 __cdecl SMC_Allocate(unsigned int smcIndex, unsigned int bitCount);
char __cdecl SMC_GetFreeBlockOfSize(unsigned int smcIndex, unsigned int listIndex);
char __cdecl SMC_ForceFreeBlock(unsigned int smcIndex);
void __cdecl SMC_FreeCachedSurface_r(
                static_model_tree_t *tree,
                static_model_leaf_t *leafs,
                int nodeIndex,
                int levelsToLeaf);
void __cdecl R_CacheStaticModelIndices(unsigned int smodelIndex, unsigned int lod, unsigned int cacheBaseVertIndex);
void __cdecl R_InitStaticModelCache();
void *R_AllocStaticModelCache();
unsigned int SMC_ClearCache();
void __cdecl R_FlushStaticModelCache();
void __cdecl R_ClearAllStaticModelCacheRefs();
void __cdecl R_UncacheStaticModel(unsigned int smodelIndex);
void __cdecl R_ShutdownStaticModelCache();
void __cdecl R_StaticModelCacheStats_f();
void __cdecl R_StaticModelCacheFlush_f();
