#pragma once
#include <gfx_d3d/fxprimitives.h>

struct FxPostLight // sizeof=0x24
{                                       // XREF: FxPostLightInfo/r
                                        // CG_Laser_Add_Core/r
    float begin[3];                     // XREF: CG_Laser_Add_Core+636/w
    // CG_Laser_Add_Core+640/w ...
    float end[3];                       // XREF: CG_Laser_Add_Core+654/w
    // CG_Laser_Add_Core+65E/w ...
    float radius;                       // XREF: CG_Laser_Add_Core+67D/w
    GfxColor color;                     // XREF: CG_Laser_Add_Core+6B1/w
    Material *material;                 // XREF: CG_Laser_Add_Core+6BA/w
};

struct FxPostLightInfo // sizeof=0xD84
{                                       // XREF: .data:g_postLightInfo/r
    FxPostLight postLights[96];
    int postLightCount;                 // XREF: FX_PostLight_Begin(void)+3/w
    // FX_PostLight_Add(FxPostLight *)+5/r ...
};

void __cdecl FX_PostLight_GenerateVerts(FxPostLightInfo *postLightInfoAddr, FxSystem *system);
void __cdecl FX_PostLight_Begin();
void __cdecl FX_PostLight_Add(FxPostLight *postLight);
FxPostLightInfo *__cdecl FX_PostLight_GetInfo();
