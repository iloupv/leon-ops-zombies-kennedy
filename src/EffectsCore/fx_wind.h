#pragma once

struct FX_WindInfo;

void __cdecl FX_SetGlobalWind(const FX_WindInfo *windInput);
const FX_WindInfo *__cdecl FX_GetGlobalWind();
