#pragma once
#include "r_rendercmds.h"
#include "rb_backend.h"

void __cdecl RB_AllocSuperFlareQueries();
void __cdecl RB_FreeSuperFlareQueries();
void __cdecl RB_DrawSuperFlareOccluders(const GfxViewInfo *viewInfo);
void __cdecl RB_DrawSuperFlareOccluder(GfxGenericFilter *FilterInfo, int iFilter, int iPass);
GfxVertex *__cdecl RB_SetSuperFlareQuads();
