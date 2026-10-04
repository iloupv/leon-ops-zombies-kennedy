#pragma once

#include <universal/dvar.h>

struct VariantWind // sizeof=0x38
{                                       // XREF: .data:VariantWind * g_VariantWind/r
                                        // LOCALIZED_WIND/r
    float wind_vector[3];
    float rustle_angular_frequency[3];  // XREF: CG_UpdateWind(int)+210/r
    float rustle_amplitude[3];          // XREF: CG_UpdateWind(int)+251/r
    float blade_acceleration;
    float blade_velocity;
    int reset_timer;
    int gust_start;
    int gust_dur;
};

struct GrassWind // sizeof=0x1C
{                                       // XREF: .data:GrassWind (* g_GrassWind)[16]/r
    float pos[2];                       // XREF: DisplayWindDebug(int)+329/r
                                        // DisplayWindDebug(int)+33C/r ...
    float dir[2];                       // XREF: CG_UpdateWind(int)+4D3/w
                                        // CG_UpdateWind(int)+517/w ...
    float strength_skew;                // XREF: CG_UpdateWind(int)+A19/r
    int life_span;                      // XREF: BG_SeedVarintWinds(int,int)+126/w
                                        // CG_UpdateWind(int)+48B/r ...
    int start_time;                     // XREF: BG_SeedVarintWinds(int,int)+10C/w
                                        // CG_UpdateWind(int)+484/r ...
};

struct __declspec(align(4)) LOCALIZED_WIND // sizeof=0x44
{                                       // XREF: .data:LOCALIZED_WIND g_localWind/r
    VariantWind wind;
    int entnum;                         // XREF: CG_UpdateWind(int)+31C/r
                                        // CG_UpdateWind(int):loc_4690EB/r ...
    float radius;
    bool active;
    // padding byte
    // padding byte
    // padding byte
};

struct FX_WindInfo // sizeof=0x1C
{                                       // XREF: .data:FX_WindInfo `anonymous namespace'::g_fxGlobalWind/r
                                        // ?CG_UpdateWind@@YAXH@Z/r
    float windVector[3];
    float lowWindAltitude;
    float hiWindAltitude;
    float lowWindStrengthPercent;
    float windVectorMagnitude;          // XREF: FX_SetGlobalWind(FX_WindInfo const &)+22/w
};

void __cdecl BG_Wind_RegisterDvars();
void __cdecl DisplayWindDebug();
void __cdecl SeedVariantWind(
                VariantWind *wind,
                int i,
                int start_time,
                const float *seedWindVector,
                float windLeafScale);
void __cdecl BG_SeedVarintWinds(unsigned int variant_seed, int start_time);
void    BG_UpdateVariantWind(
                VariantWind *wind,
                int variantIndex,
                const float *seedWindVector,
                int cur_time);
void    CG_UpdateWind(int cur_time);
void __cdecl BG_GetVariantWindVector(int which, float *out);
void __cdecl BG_GetGlobalWind(float *out);
void __cdecl CG_EnableLocalWindSource(int entnum);
double __cdecl CG_GetLocalWindSourceScale(int localClientNum, float *pos);
