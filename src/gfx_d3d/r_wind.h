#pragma once
#include "rb_state.h"

struct GfxCustomWind // sizeof=0x1C
{                                       // XREF: WindState/r
    float rustleAngle[3];               // XREF: R_InitWind(void)+DC/w
                                        // R_InitWind(void)+EC/w ...
    float location[3];                  // XREF: R_InitWind(void)+AC/w
                                        // R_InitWind(void)+BC/w ...
    float radius;                       // XREF: R_InitWind(void)+10C/w
                                        // RB_SetCustomWindConstants(GfxCmdBufSourceState *,float const * const)+BE/r ...
};

struct __declspec(align(8)) WindState // sizeof=0x130
{                                       // XREF: .data:frontendWind/r
                                        // .data:backendWind/r
    float windDirection[3];             // XREF: R_InitWind(void)+10/w
                                        // R_InitWind(void)+20/w ...
    float windSpringAngle[16];          // XREF: R_InitWind(void)+5B/w
                                        // R_SetVariantWindSpringAngle(uint,float)+35/w ...
    float windRustleAngle[16][3];       // XREF: R_InitWind(void)+6A/o
                                        // R_SetVariantWindRustleAngle(uint,float const * const)+34/o ...
    float speed;                        // XREF: R_SetWindDirection(float * const,float)+B/w
                                        // R_SetWindShaderConstants(GfxCmdBufSourceState *)+2D/r
    GfxCustomWind customWind;           // XREF: R_InitWind(void)+AC/w
                                        // R_InitWind(void)+BC/w ...
    // padding byte
    // padding byte
    // padding byte
    // padding byte
};

void __cdecl R_InitWind();
void __cdecl R_SetWindDirection(float *dir, float speed);
void __cdecl R_SetVariantWindSpringAngle(unsigned int index, float angle);
void __cdecl R_SetVariantWindRustleAngle(unsigned int windIndex, float *angles);
void __cdecl R_WindStartBackend();
void __cdecl R_SetWindShaderConstants(GfxCmdBufSourceState *source);
void __cdecl RB_SetCustomWindConstants(GfxCmdBufSourceState *source, const float *drawSurfListViewOrigin);
void __cdecl R_SetLocalWind(const float *position, float radius, const float *rustleAngle);
