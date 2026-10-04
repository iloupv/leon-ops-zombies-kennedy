#pragma once
#include "r_gfx.h"

GfxLightDef *__cdecl R_LoadLightDef(const char *name);
unsigned __int8 *__cdecl R_LoadLightImage(unsigned __int8 *readPos, GfxLightImage *lightImage);
