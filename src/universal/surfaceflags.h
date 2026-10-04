#pragma once

struct infoParm_t // sizeof=0x14
{
    const char *name;
    int clearSolid;
    int surfaceFlags;
    int contents;
    int toolFlags;
};

int __cdecl Com_SurfaceTypeFromName(const char *name);
const char *__cdecl Com_SurfaceTypeToName(int iTypeIndex);
bool __cdecl Com_SurfaceBurns(int iTypeIndex);
bool __cdecl Com_SurfaceFloats(int iTypeIndex);
double __cdecl Com_SurfaceDensity(int iTypeIndex);
double __cdecl Com_SurfaceFrictionScale(int iTypeIndex);
double __cdecl Com_SurfaceBounceScale(int iTypeIndex);

extern const char *Dtp_SurfaceTypeNames[9];