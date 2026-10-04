#pragma once
#include "r_rendercmds.h"
#include "rb_state.h"

struct SpotShadowMapCallbackUserData
{
    const GfxSpotShadow *shadow;
    const GfxDrawSurfListInfo *drawList;
};

void __cdecl R_DrawSpotShadowMapCallback(const void *userData, GfxCmdBufContext context, GfxCmdBufContext prepassContext);
void __cdecl RB_SpotShadowMaps(const GfxBackEndData *data, const GfxViewInfo *viewInfo);
void    R_DrawSpotShadowMapArray(const GfxViewInfo *viewInfo, GfxCmdBuf *cmdBuf);
void __cdecl RB_DrawSpotShadowOverlay();
void __cdecl RB_SetSpotShadowOverlayScaleAndBias(const GfxSpotShadow *spotShadow);

static const float shadowmapClearColor[4] = { 1.0, 1.0, 1.0, 1.0 };