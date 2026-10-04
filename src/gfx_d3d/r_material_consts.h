#pragma once
#include "r_material.h"

struct GfxShaderConstantBlock // sizeof=0xC4
{                                       // XREF: R_ComparePixelConsts(Material const * * const,MaterialPass const * * const)+1EC/r
    unsigned int count;                 // XREF: R_ComparePixelConsts(Material const * * const,MaterialPass const * * const)+44/w
    unsigned __int16 dest[32];          // XREF: R_ComparePixelConsts(Material const * * const,MaterialPass const * * const)+223/r
    const float *value[32];             // XREF: R_ComparePixelConsts(Material const * * const,MaterialPass const * * const)+26C/r
};

void __cdecl R_GetPixelLiteralConsts(
                const Material *mtl,
                const MaterialPass *pass,
                GfxShaderConstantBlock *pixelLiteralConsts);
void __cdecl R_RegisterShaderConst(unsigned int dest, const float *value, GfxShaderConstantBlock *consts);
int __cdecl R_ComparePixelConsts(const Material **material, const MaterialPass **pass);
