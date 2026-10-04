#pragma once
#include "rb_state.h"
#include "r_rendercmds.h"

enum LightHasShadowMap : __int32
{                                       // XREF: ?R_SetLightProperties@@YAXPAUGfxCmdBufSourceState@@PBUGfxLight@@PBUGfxLightDef@@W4LightHasShadowMap@@M@Z/r
                                        // ?R_SetShadowableLight@@YAXPAUGfxCmdBufSourceState@@IPBUGfxViewInfo@@@Z/r
    LIGHT_HAS_SHADOWMAP    = 0x0,
    LIGHT_HAS_NO_SHADOWMAP = 0x1,
};

enum GfxLightType : __int32
{
    GFX_LIGHT_TYPE_NONE           = 0x0,
    GFX_LIGHT_TYPE_DIR            = 0x1,
    GFX_LIGHT_TYPE_SPOT           = 0x2,
    GFX_LIGHT_TYPE_OMNI           = 0x3,
    GFX_LIGHT_TYPE_COUNT          = 0x4,
    GFX_LIGHT_TYPE_DIR_SHADOWMAP  = 0x4,
    GFX_LIGHT_TYPE_SPOT_SHADOWMAP = 0x5,
    GFX_LIGHT_TYPE_OMNI_SHADOWMAP = 0x6,
    GFX_LIGHT_TYPE_COUNT_WITH_SHADOWMAP_VERSIONS = 0x7,
};

// local variable allocation has failed, the output may be wrong!
void    R_SetLightProperties(
                GfxCmdBufSourceState *source,
                GfxLight *light,
                const GfxLightDef *def,
                LightHasShadowMap hasShadowMap,
                float spotShadowFade);
void __cdecl R_SetShadowableLight(GfxCmdBufSourceState *source, unsigned int shadowableLightIndex);
void __cdecl R_SetDrawSurfsShadowableLight(GfxCmdBufSourceState *source, const GfxDrawSurfListInfo *info);
unsigned int __cdecl R_GetShadowableLightIndex(
                const GfxBackEndData *data,
                const GfxViewInfo *viewInfo,
                const GfxLight *light);
