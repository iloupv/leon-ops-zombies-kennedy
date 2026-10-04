#pragma once
#include "r_rendercmds.h"

struct GfxBspPreTessDrawSurf // sizeof=0x4
{                                       // XREF: R_PreTessBspDrawSurfs/r
    unsigned __int16 baseSurfIndex;     // XREF: R_PreTessBspDrawSurfs+194/w
    unsigned __int16 totalTriCount;     // XREF: R_PreTessBspDrawSurfs+1A4/w
};

void __cdecl R_InitDrawSurfListInfo(GfxDrawSurfListInfo *info);
void __cdecl R_EmitDrawSurfList(const GfxDrawSurf *drawSurfs, unsigned int drawSurfCount, GfxBackEndData *data);
void __cdecl R_MergeAndEmitDrawSurfLists(unsigned int firstStage, unsigned int stageCount, GfxBackEndData *data);
unsigned int __cdecl R_GetKeyCount(
                const GfxDrawSurf *drawSurfs,
                unsigned int drawSurfCount,
                unsigned int primarySortKey);
void __cdecl R_BeginPreTess(bool lockBuffer);
unsigned __int16 *__cdecl R_AllocPreTessIndices(int count);
