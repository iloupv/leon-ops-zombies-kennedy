#include <universal/q_shared.h>

#include "fx_wind.h"
#include <bgame/bg_wind.h>

#include <cstring>


FX_WindInfo g_fxGlobalWind = { { 0.0, 0.0, 0.0 }, 0.0, 1.0, 0.0, 0.0 };

void __cdecl FX_SetGlobalWind(const FX_WindInfo *windInput)
{
    memcpy(&g_fxGlobalWind, windInput, sizeof(g_fxGlobalWind));
    g_fxGlobalWind.windVectorMagnitude = Vec3Length(g_fxGlobalWind.windVector);
}

const FX_WindInfo *__cdecl FX_GetGlobalWind()
{
    //return &`anonymous namespace'::g_fxGlobalWind;
    return &g_fxGlobalWind;
}

