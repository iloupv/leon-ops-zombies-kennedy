#pragma once

#include <universal/dvar.h>

struct Material;

struct sunflare_t // sizeof=0x60
{                                       // XREF: GfxWorld/r
    bool hasValidData;
    // padding byte
    // padding byte
    // padding byte
    Material *spriteMaterial;
    Material *flareMaterial;
    float spriteSize;
    float flareMinSize;
    float flareMinDot;
    float flareMaxSize;
    float flareMaxDot;
    float flareMaxAlpha;
    int flareFadeInTime;
    int flareFadeOutTime;
    float blindMinDot;
    float blindMaxDot;
    float blindMaxDarken;
    int blindFadeInTime;
    int blindFadeOutTime;
    float glareMinDot;
    float glareMaxDot;
    float glareMaxLighten;
    int glareFadeInTime;
    int glareFadeOutTime;
    float sunFxPosition[3];
};

void __cdecl R_RegisterSunDvars();
void __cdecl R_SetSunFromDvars(sunflare_t *sun);
void __cdecl R_LoadSunThroughDvars(const char *sunName, sunflare_t *sun);
unsigned int __cdecl R_GetSunDvarCount();
void __cdecl R_Cmd_LoadSun();
void __cdecl R_Cmd_SaveSun();
void __cdecl R_SaveSunFromDvars(const char *sunName);
void __cdecl R_FlushSun();

extern const dvar_t *r_sunsprite_shader;
extern const dvar_t *r_sunsprite_size;
extern const dvar_t *r_sunflare_shader;
extern const dvar_t *r_sunflare_min_size;
extern const dvar_t *r_sunflare_min_angle;
extern const dvar_t *r_sunflare_max_size;
extern const dvar_t *r_sunflare_max_angle;
extern const dvar_t *r_sunflare_max_alpha;
extern const dvar_t *r_sunflare_fadein;
extern const dvar_t *r_sunflare_fadeout;
extern const dvar_t *r_sunblind_min_angle;
extern const dvar_t *r_sunblind_max_angle;
extern const dvar_t *r_sunblind_max_darken;
extern const dvar_t *r_sunblind_fadein;
extern const dvar_t *r_sunblind_fadeout;
extern const dvar_t *r_sunglare_min_angle;
extern const dvar_t *r_sunglare_max_angle;
extern const dvar_t *r_sunglare_max_lighten;
extern const dvar_t *r_sunglare_fadein;
extern const dvar_t *r_sunglare_fadeout;
extern const dvar_t *r_sun_fx_position;