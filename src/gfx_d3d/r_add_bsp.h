#pragma once
#include "r_material.h"
#include "r_gfx.h"

void __cdecl R_InitBspDrawSurf(GfxSModelDrawSurfLightingData *surfData);
void __cdecl R_AddBspDrawSurfs(
                GfxDrawSurf drawSurf,
                unsigned __int8 *list,
                unsigned int count,
                GfxBspDrawSurfData *surfData);
char __cdecl R_PreTessBspDrawSurfs(
                GfxDrawSurf drawSurf,
                const unsigned __int16 *list,
                unsigned int count,
                GfxBspDrawSurfData *surfData);
void __cdecl R_AddAllBspDrawSurfacesCamera(int localClientNum, const GfxLight *visibleLights, int visibleLightCount);
void __cdecl R_AddAllBspDrawSurfacesRangeCamera(
                unsigned int beginSurface,
                unsigned int endSurface,
                unsigned int stage,
                unsigned int maxDrawSurfCount,
                int localClientNum,
                const GfxLight *visibleLights,
                int visibleLightCount);
unsigned __int8 __cdecl GetSurfaceFlags(unsigned int surfIndex);
char __cdecl R_WorldLod_GetFadeForDrawSurf(int surface, int *fade, int localClientNum);
int __cdecl R_WorldLod_GetLodVal(int surfIndex, int localClientNum);
void __cdecl R_AddAllBspDrawSurfacesCameraNonlit(
                unsigned int beginSurface,
                unsigned int endSurface,
                unsigned int stage);
void __cdecl R_AddAllBspDrawSurfacesSunShadow(int localClientNum);
void __cdecl R_AddAllBspDrawSurfacesRangeSunShadow(
                unsigned int partitionIndex,
                unsigned int beginSurface,
                unsigned int endSurface,
                unsigned int maxDrawSurfCount,
                int localClientNum);
void __cdecl R_AddAllBspDrawSurfacesSpotShadow(unsigned int spotShadowIndex, unsigned int primaryLightIndex);
