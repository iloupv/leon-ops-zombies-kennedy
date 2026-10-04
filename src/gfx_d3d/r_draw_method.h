#pragma once
#include "r_rendercmds.h"

enum GfxFadeType : __int32
{                                       // XREF: ?R_GetTechType@@YAEUGfxCmdBufContext@@PBUGfxDrawSurfListInfo@@ETGfxDrawSurf@@@Z/r
                                        // ?R_UpdateDrawMethod@@YAXPAUGfxBackEndData@@PBUGfxViewInfo@@@Z/r
    GFX_TECHNIQUE_NO_FADE    = 0x0,
    GFX_TECHNIQUE_FADE       = 0x1,
    GFX_TECHNIQUE_FADE_TYPES = 0x2,
};

enum GfxTechCharredType : __int32
{                                       // XREF: ?R_GetTechType@@YAEUGfxCmdBufContext@@PBUGfxDrawSurfListInfo@@ETGfxDrawSurf@@@Z/r
                                        // ?R_UpdateDrawMethod@@YAXPAUGfxBackEndData@@PBUGfxViewInfo@@@Z/r
    GFX_TECHNIQUE_NOT_CHARRED   = 0x0,
    GFX_TECHNIQUE_CHARRED       = 0x1,
    GFX_TECHNIQUE_CHARRED_TYPES = 0x2,
};

void __cdecl R_InitDrawMethod();
void R_SetDefaultLitTechTypes();
void __cdecl R_ForceLitTechType(unsigned __int8 litTechType);
void __cdecl R_UpdateDrawMethod(GfxBackEndData *data);

extern GfxDrawMethod gfxDrawMethod;