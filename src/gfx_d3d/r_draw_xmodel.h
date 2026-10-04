#pragma once
#include "r_rendercmds.h"
#include "r_dobj_skin.h"
#include "rb_state.h"

char __cdecl R_DrawXModelSurfCheckBreak(
                unsigned int gfxEntIndex,
                const GfxBackEndData *data,
                unsigned int depthHackFlags,
                float materialTime,
                float curDestructibleBurnAmount,
                float curDestructibleFadeAmount,
                float curWetness,
                bool TEXTURE_OVERRIDE,
                int currTextureOverride,
                unsigned int drawOutlineFlags);
unsigned int __cdecl R_DrawXModelSurfLit(
                const GfxDrawSurf *drawSurfList,
                unsigned int drawSurfCount,
                GfxCmdBufContext context);
unsigned int __cdecl R_DrawXModelSurfLitInternal(
                const GfxDrawSurf *drawSurfList,
                unsigned int drawSurfCount,
                GfxCmdBufContext context);
void __cdecl R_GetWorldMatrixForModelSurf(const GfxModelRigidSurface *modelSurf, float4 eyeOffset, vector4 *worldMat);
void __cdecl R_DrawXModelRigidModelSurf(GfxCmdBufContext context, XSurface *xsurf);
unsigned int __cdecl R_DrawXModelSurfCamera(
                const GfxDrawSurf *drawSurfList,
                unsigned int drawSurfCount,
                GfxCmdBufContext context);
unsigned int __cdecl R_DrawXModelSurfCameraInternal(
                const GfxDrawSurf *drawSurfList,
                unsigned int drawSurfCount,
                GfxCmdBufContext context);
unsigned int __cdecl R_DrawXModelSurf(
                const GfxDrawSurf *drawSurfList,
                unsigned int drawSurfCount,
                GfxCmdBufContext context);
unsigned int __cdecl R_DrawXModelSurfInternal(
                const GfxDrawSurf *drawSurfList,
                unsigned int drawSurfCount,
                GfxCmdBufContext context);
