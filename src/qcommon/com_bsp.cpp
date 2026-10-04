#include "com_bsp.h"
#include <win32/win_main.h>

ComWorld comWorld;

char __cdecl Com_CanPrimaryLightAffectPoint(const ComPrimaryLight *light, const float *point)
{
    float lightRadius; // [esp+1Ch] [ebp-1Ch]
    float deltaToLight; // [esp+20h] [ebp-18h]
    float deltaToLight_4; // [esp+24h] [ebp-14h]
    float deltaToLight_8; // [esp+28h] [ebp-10h]
    float cosHalfFov; // [esp+2Ch] [ebp-Ch]
    float spotDotTimesDist; // [esp+30h] [ebp-8h]
    float distSq; // [esp+34h] [ebp-4h]

    if ( !light && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\com_bsp.cpp", 24, 0, "%s", "light") )
        __debugbreak();
    if ( light->type != 2
        && light->type != 3
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\com_bsp.cpp",
                    25,
                    0,
                    "%s\n\t(light->type) = %i",
                    "(light->type == GFX_LIGHT_TYPE_SPOT || light->type == GFX_LIGHT_TYPE_OMNI)",
                    light->type) )
    {
        __debugbreak();
    }
    if ( !point && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\com_bsp.cpp", 26, 0, "%s", "point") )
        __debugbreak();
    deltaToLight = light->origin[0] - *point;
    deltaToLight_4 = light->origin[1] - point[1];
    deltaToLight_8 = light->origin[2] - point[2];
    distSq = (float)((float)(deltaToLight * deltaToLight) + (float)(deltaToLight_4 * deltaToLight_4))
                 + (float)(deltaToLight_8 * deltaToLight_8);
    lightRadius = light->radius;
    if ( light->type == 2 )
        lightRadius = lightRadius / light->cosHalfFovOuter;
    if ( distSq >= (float)(lightRadius * lightRadius) )
        return 0;
    //if ( light->type == 3 || COERCE_FLOAT(LODWORD(light->cosHalfFovOuter) ^ _mask__NegFloat_) >= light->rotationLimit )
    if ( light->type == 3 || -(light->cosHalfFovOuter) >= light->rotationLimit )
        return 1;
    spotDotTimesDist = (float)((float)(deltaToLight * light->dir[0]) + (float)(deltaToLight_4 * light->dir[1]))
                                     + (float)(deltaToLight_8 * light->dir[2]);
    if ( light->rotationLimit == 1.0 )
    {
        cosHalfFov = light->cosHalfFovOuter;
    }
    else
    {
        cosHalfFov = CosOfSumOfArcCos(light->cosHalfFovOuter, light->rotationLimit);
        if ( cosHalfFov <= 0.0 )
            return (float)(cosHalfFov * light->radius) >= spotDotTimesDist;
    }
    return spotDotTimesDist > 0.0
            && (float)((float)(cosHalfFov * cosHalfFov) * distSq) <= (float)(spotDotTimesDist * spotDotTimesDist);
}

double __cdecl CosOfSumOfArcCos(float cos0, float cos1)
{
    float v3; // [esp+0h] [ebp-10h]
    float v4; // [esp+4h] [ebp-Ch]

    v4 = cos0 * cos1;
    v3 = sqrt((float)(1.0 - (float)(cos0 * cos0)) * (float)(1.0 - (float)(cos1 * cos1)));
    return v4 - v3;
}

void __cdecl Com_UnloadWorld()
{
    if ( !useFastFile->current.enabled
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\com_bsp.cpp", 91, 0, "%s", "IsFastFileLoad()") )
    {
        __debugbreak();
    }
    if ( comWorld.isInUse )
        Sys_Error((char*)"Cannot unload world while it is in use");
}

