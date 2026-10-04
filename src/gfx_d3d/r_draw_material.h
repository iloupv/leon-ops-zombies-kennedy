#pragma once
#include "r_gfx.h"
#include "r_material.h"
#include "rb_state.h"
#include "r_rendercmds.h"

unsigned __int8 __cdecl RemoveSunShadowTech(unsigned __int8 srcTech);
unsigned int __cdecl R_GfxDrawSurf_GetFade(const GfxDrawSurf *drawSurf);
unsigned __int8 __cdecl R_Get_NvFloatZLitTech(const Material *material, unsigned __int8 originalTechType);
unsigned __int8 __cdecl R_GetTechType(
                GfxCmdBufContext context,
                const GfxDrawSurfListInfo *info,
                unsigned __int8 baseTechType,
                GfxDrawSurf drawSurf);
int __cdecl R_GetPossibleTechniqueTypes(unsigned __int8 techType, GfxDrawSurf drawSurf, unsigned __int8 *techTypes);
int __cdecl R_SetTechnique(
                GfxCmdBufContext context,
                GfxCmdBufContext *prepassContext,
                const GfxDrawSurfListInfo *info,
                GfxDrawSurf drawSurf);
int __cdecl R_SetPrepassMaterial(GfxCmdBufContext context, GfxDrawSurf drawSurf, unsigned __int8 techType);
int __cdecl R_SetMaterial(GfxCmdBufContext context, GfxDrawSurf drawSurf, unsigned __int8 techType);
int __cdecl R_TrySetMaterialWithFunc(
                int (__cdecl *func)(GfxCmdBufContext, GfxDrawSurf, const unsigned __int8),
                GfxCmdBufContext context,
                GfxDrawSurf drawSurf,
                unsigned __int8 *techType);
void __cdecl R_SetGameTime(GfxCmdBufSourceState *source, float gameTime);
int __cdecl R_UpdateMaterialTime(
                GfxCmdBufSourceState *source,
                float materialTime,
                float burn,
                float fade,
                float wetness);
