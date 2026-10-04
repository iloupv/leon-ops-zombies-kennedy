#pragma once

#include "r_scene.h"
#include "r_gfx.h"

struct GfxSpotShadowEntCmd // sizeof=0x8
{                                       // XREF: R_AddSpotShadowModelEntities/r
    GfxSceneEntity *sceneEnt;           // XREF: R_AddSpotShadowModelEntities+42/w
                                        // R_AddSpotShadowModelEntities+45/r ...
    const GfxLight *light;              // XREF: R_AddSpotShadowModelEntities+10/w
};

void __cdecl R_AddSpotShadowEntCmd(const GfxSpotShadowEntCmd *data);
char __cdecl R_AddSpotShadowsForLight(
                GfxViewInfo *viewInfo,
                GfxLight *light,
                unsigned int shadowableLightIndex,
                unsigned int totalSpotLightCount,
                float spotShadowFade);
// local variable allocation has failed, the output may be wrong!
void    R_SetViewParmsForLight(
                const GfxLight *light,
                GfxViewParms *viewParms,
                float nearPlaneBias);
void __cdecl R_GetSpotShadowLookupMatrix(
                const GfxViewParms *shadowViewParms,
                int usingGridTiling,
                int usingFullShadowBuffer,
                unsigned int spotShadowIndex,
                unsigned int tileCount,
                GfxMatrix *lookupMatrix);
void __cdecl R_AddSpotShadowModelEntities(int localClientNum, unsigned int primaryLightIndex, const GfxLight *light);
void __cdecl R_GenerateAllSortedSpotShadowDrawSurfs(GfxViewInfo *viewInfo, int viewIndex);
void __cdecl R_GenerateSortedPrimarySpotShadowDrawSurfs(
                const GfxViewInfo *viewInfo,
                int viewIndex,
                unsigned int spotShadowIndex,
                unsigned int shadowableLightIndex);
void __cdecl R_EmitSpotShadowMapSurfs(GfxViewInfo *viewInfo);
