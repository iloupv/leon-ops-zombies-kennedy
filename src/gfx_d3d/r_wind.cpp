#include "r_wind.h"
#include <universal/assertive.h>
#include <cstring>
#include "r_foliage.h"

WindState frontendWind;
WindState backendWind;

void __cdecl R_InitWind()
{
    float *v0; // ecx
    unsigned int n; // [esp+Ch] [ebp-4h]

    frontendWind.windDirection[0] = 1.0f;
    frontendWind.windDirection[1] = 0.0f;
    frontendWind.windDirection[2] = 0.0f;
    for ( n = 0; n < 0x10; ++n )
    {
        frontendWind.windSpringAngle[n] = 0.0f;
        frontendWind.windRustleAngle[n][0] = 0.0f;
        v0 = frontendWind.windRustleAngle[n];
        v0[1] = 0.0f;
        v0[2] = 0.0f;
    }
    frontendWind.customWind.location[0] = 0.0f;
    frontendWind.customWind.location[1] = 0.0f;
    frontendWind.customWind.location[2] = 0.0f;
    frontendWind.customWind.rustleAngle[0] = 0.0f;
    frontendWind.customWind.rustleAngle[1] = 0.0f;
    frontendWind.customWind.rustleAngle[2] = 0.0f;
    frontendWind.customWind.radius = 0.0f;
    memcpy(&backendWind, &frontendWind, sizeof(backendWind));
}

void __cdecl R_SetWindDirection(float *dir, float speed)
{
    frontendWind.speed = speed;
    frontendWind.windDirection[0] = *dir;
    frontendWind.windDirection[1] = dir[1];
    frontendWind.windDirection[2] = 0.0f;
    Vec3NormalizeFast(frontendWind.windDirection);
}

void __cdecl R_SetVariantWindSpringAngle(unsigned int index, float angle)
{
    if ( index >= 0x10
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_wind.cpp",
                    78,
                    0,
                    "index doesn't index ARRAY_COUNT(frontendWind.windSpringAngle)\n\t%i not in [0, %i)",
                    index,
                    16) )
    {
        __debugbreak();
    }
    frontendWind.windSpringAngle[index] = angle;
}

void __cdecl R_SetVariantWindRustleAngle(unsigned int windIndex, float *angles)
{
    float *v2; // [esp+0h] [ebp-4h]

    if ( windIndex >= 0x10
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_wind.cpp",
                    85,
                    0,
                    "windIndex doesn't index ARRAY_COUNT(frontendWind.windRustleAngle)\n\t%i not in [0, %i)",
                    windIndex,
                    16) )
    {
        __debugbreak();
    }
    v2 = frontendWind.windRustleAngle[windIndex];
    *v2 = *angles;
    v2[1] = angles[1];
    v2[2] = angles[2];
}

void __cdecl R_WindStartBackend()
{
    memcpy(&backendWind, &frontendWind, sizeof(backendWind));
}

void __cdecl R_SetWindShaderConstants(GfxCmdBufSourceState *source)
{
    float springConstant[4]; // [esp+10h] [ebp-24h] BYREF
    unsigned int idx; // [esp+20h] [ebp-14h]
    float wind4[4]; // [esp+24h] [ebp-10h] BYREF

    wind4[0] = backendWind.windDirection[0];
    wind4[1] = backendWind.windDirection[1];
    wind4[2] = backendWind.windDirection[2];
    wind4[3] = backendWind.speed;
    R_UpdateCodeConstantFromVec4(source, CONST_SRC_CODE_WIND_DIRECTION, wind4);
    for ( idx = 0; idx < 0x10; ++idx )
    {
        springConstant[0] = backendWind.windSpringAngle[idx];
        springConstant[1] = backendWind.windRustleAngle[idx][0];
        springConstant[2] = backendWind.windRustleAngle[idx][1];
        springConstant[3] = backendWind.windRustleAngle[idx][2];
        R_UpdateCodeConstantFromVec4(source, (CodeConstant)(idx + CONST_SRC_CODE_VARIANT_WIND_SPRING_0), springConstant);
    }
}

void __cdecl RB_SetCustomWindConstants(GfxCmdBufSourceState *source, const float *drawSurfListViewOrigin)
{
    float eyeOffset; // [esp+2Ch] [ebp-28h]
    float eyeOffset_4; // [esp+30h] [ebp-24h]
    float eyeOffset_8; // [esp+34h] [ebp-20h]
    float wind4[4]; // [esp+44h] [ebp-10h] BYREF

    if ( drawSurfListViewOrigin[3] == 0.0 )
    {
        eyeOffset = 0.0f;
        eyeOffset_4 = 0.0f;
        eyeOffset_8 = 0.0f;
    }
    else
    {
        eyeOffset = *drawSurfListViewOrigin;
        eyeOffset_4 = drawSurfListViewOrigin[1];
        eyeOffset_8 = drawSurfListViewOrigin[2];
    }
    wind4[0] = backendWind.customWind.location[0] - eyeOffset;
    wind4[1] = backendWind.customWind.location[1] - eyeOffset_4;
    wind4[2] = backendWind.customWind.location[2] - eyeOffset_8;
    wind4[3] = backendWind.customWind.radius;
    R_UpdateCodeConstantFromVec4(source, CONST_SRC_CODE_CUSTOMWIND_CENTER, wind4);
    wind4[0] = backendWind.customWind.rustleAngle[0];
    wind4[1] = backendWind.customWind.rustleAngle[1];
    wind4[2] = backendWind.customWind.rustleAngle[2];
    wind4[3] = 0.0f;
    R_UpdateCodeConstantFromVec4(source, CONST_SRC_CODE_CUSTOMWIND_SPRING, wind4);
}

void __cdecl R_SetLocalWind(const float *position, float radius, const float *rustleAngle)
{
    frontendWind.customWind.location[0] = *position;
    frontendWind.customWind.location[1] = position[1];
    frontendWind.customWind.location[2] = position[2];
    frontendWind.customWind.rustleAngle[0] = *rustleAngle;
    frontendWind.customWind.rustleAngle[1] = rustleAngle[1];
    frontendWind.customWind.rustleAngle[2] = rustleAngle[2];
    frontendWind.customWind.radius = radius;
}

