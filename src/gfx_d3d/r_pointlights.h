#pragma once

struct GfxDecodedLightGridColors // sizeof=0x380
{                                       // XREF: ?R_ApplyLightGridColorsPatch@@YAXPBUGfxModelLightingPatch@@PAE@Z/r
    float rgb[56][4];                   // XREF: R_ApplyLightGridColorsPatch(GfxModelLightingPatch const *,uchar *)+180/r
};

struct GfxHeroLight // sizeof=0x38
{
    unsigned __int8 type;
    unsigned __int8 unused[3];
    float color[3];
    float dir[3];
    float origin[3];
    float radius;
    float cosHalfFovOuter;
    float cosHalfFovInner;
    int exponent;
};

struct GfxHeroLightTree // sizeof=0x18
{
    float mins[3];
    float maxs[3];
};

void __cdecl GenerateLightGridBasisDirs();
void __cdecl GatherIncidentEnergyInSpaceForLightFromDir(
                const float *energy,
                const float *lightFromDir,
                float (*incidentEnergy)[3]);
void __cdecl AddLightGridLightingForDir(float (*lightingForDir)[3], GfxDecodedLightGridColors *colors);
char __cdecl EvaluateHeroLightForGrid(
                const GfxHeroLight *light,
                const float *xyz,
                float *dirToLight,
                float *attenuatedColor);
void __cdecl R_AdjustLightColorSamples(GfxDecodedLightGridColors *colors);
void __cdecl R_AddHeroOnlyLightsToGridColors(GfxDecodedLightGridColors *packed, const float *heroPos);
void R_InitHeroLights();
