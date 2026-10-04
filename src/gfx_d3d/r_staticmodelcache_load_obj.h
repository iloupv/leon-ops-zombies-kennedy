#pragma once
#include "r_bsp.h"

struct GfxSModelSurfStats // sizeof=0x10
{                                       // XREF: ??$_Insertion_sort1@PAUGfxSModelSurfStats@@P6A_NAAU1@0@ZU1@@std@@YAXPAUGfxSModelSurfStats@@0P6A_NAAU1@1@Z0@Z/r
                                        // ??$_Adjust_heap@PAUGfxSModelSurfStats@@HU1@P6A_NAAU1@0@Z@std@@YAXPAUGfxSModelSurfStats@@HHU1@P6A_NAAU1@2@Z@Z/r ...
    XModel *model;                      // XREF: std::_Adjust_heap<GfxSModelSurfStats *,int,GfxSModelSurfStats,bool (*)(GfxSModelSurfStats &,GfxSModelSurfStats &)>(GfxSModelSurfStats *,int,int,GfxSModelSurfStats,bool (*)(GfxSModelSurfStats &,GfxSModelSurfStats &)):loc_595D13/r
                                        // std::_Adjust_heap<GfxSModelSurfStats *,int,GfxSModelSurfStats,bool (*)(GfxSModelSurfStats &,GfxSModelSurfStats &)>(GfxSModelSurfStats *,int,int,GfxSModelSurfStats,bool (*)(GfxSModelSurfStats &,GfxSModelSurfStats &))+C6/w ...
    unsigned int lod;                   // XREF: std::_Adjust_heap<GfxSModelSurfStats *,int,GfxSModelSurfStats,bool (*)(GfxSModelSurfStats &,GfxSModelSurfStats &)>(GfxSModelSurfStats *,int,int,GfxSModelSurfStats,bool (*)(GfxSModelSurfStats &,GfxSModelSurfStats &))+C9/r
                                        // std::_Adjust_heap<GfxSModelSurfStats *,int,GfxSModelSurfStats,bool (*)(GfxSModelSurfStats &,GfxSModelSurfStats &)>(GfxSModelSurfStats *,int,int,GfxSModelSurfStats,bool (*)(GfxSModelSurfStats &,GfxSModelSurfStats &))+CC/w ...
    unsigned int smcAllocBits;          // XREF: std::_Adjust_heap<GfxSModelSurfStats *,int,GfxSModelSurfStats,bool (*)(GfxSModelSurfStats &,GfxSModelSurfStats &)>(GfxSModelSurfStats *,int,int,GfxSModelSurfStats,bool (*)(GfxSModelSurfStats &,GfxSModelSurfStats &))+CF/r
                                        // std::_Adjust_heap<GfxSModelSurfStats *,int,GfxSModelSurfStats,bool (*)(GfxSModelSurfStats &,GfxSModelSurfStats &)>(GfxSModelSurfStats *,int,int,GfxSModelSurfStats,bool (*)(GfxSModelSurfStats &,GfxSModelSurfStats &))+D2/w ...
    unsigned int useCount;              // XREF: std::_Adjust_heap<GfxSModelSurfStats *,int,GfxSModelSurfStats,bool (*)(GfxSModelSurfStats &,GfxSModelSurfStats &)>(GfxSModelSurfStats *,int,int,GfxSModelSurfStats,bool (*)(GfxSModelSurfStats &,GfxSModelSurfStats &))+D5/r
                                        // std::_Adjust_heap<GfxSModelSurfStats *,int,GfxSModelSurfStats,bool (*)(GfxSModelSurfStats &,GfxSModelSurfStats &)>(GfxSModelSurfStats *,int,int,GfxSModelSurfStats,bool (*)(GfxSModelSurfStats &,GfxSModelSurfStats &))+D8/w ...
};

void __cdecl R_AssignSModelCacheResources(GfxWorld *world);
unsigned __int8 __cdecl R_AssignSModelCacheIndex(
                char smcAllocBits,
                unsigned int maxEntryCount,
                unsigned int *smcUseCount);
bool __cdecl R_CompareSModelStats_Score(GfxSModelSurfStats &s0, GfxSModelSurfStats &s1);
unsigned int __cdecl R_GetEntryCount(GfxSModelSurfStats *stats);
unsigned int __cdecl R_OptimalSModelResourceStats(GfxWorld *world, GfxSModelSurfStats *stats, unsigned int statLimit);
unsigned int __cdecl R_AddSModelListStats(
                const GfxStaticModelDrawInst **drawInstArray,
                unsigned int drawInstCount,
                GfxSModelSurfStats *stats,
                unsigned int statsCount,
                unsigned int statsLimit);
int __cdecl R_GetSModelCacheAllocBits(const XModel *model, unsigned int lod);
unsigned int __cdecl R_MaxModelsInDistRange(
                const GfxStaticModelDrawInst **drawInstArray,
                unsigned int drawInstCount,
                const float *mins,
                const float *maxs,
                float distMin,
                float distMax);
bool R_CompareSModels_Model(const GfxStaticModelDrawInst *s0, const GfxStaticModelDrawInst *s1);
