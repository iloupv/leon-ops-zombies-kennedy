#pragma once

#include <universal/assertive.h>
#include <d3d9.h>

struct DpvsPlane;

unsigned int __cdecl R_HashAssetName(const char *name);
unsigned int __cdecl R_HashString(const char *string);
char *__cdecl R_AllocGlobalVariable(unsigned int bytes, const char *name);
void __cdecl R_FreeGlobalVariable(void *var);
char __cdecl R_CullPointAndRadius(const float *pt, float radius, const DpvsPlane *clipPlanes, int clipPlaneCount);
void __cdecl R_ConvertColorToBytes(const float *colorFloat, unsigned __int8 *colorBytes);
int __cdecl R_PickEntityBone(int traceMask, const float *org, const float *dir, int boneIndex, char **boneName);
double __cdecl FresnelTerm(float n0, float n1, float cosIncidentAngle);
char __cdecl R_GetClearColor(float *unpackedRgba);


template <typename T>
inline void __cdecl R_ReleaseAndSetNULL(
    IDirect3DSurface9 *var,
    const char *fn,
    const char *filename,
    int line)
{
    unsigned int useCount; // [esp+0h] [ebp-4h]

    iassert(var);
    useCount = var->Release();
    iassert(!useCount);
}