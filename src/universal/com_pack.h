#pragma once

union PackedTexCoords // sizeof=0x4
{                                                                             // XREF: Vec2PackTexCoords(float const * const)+120/w
    unsigned int packed;

    PackedTexCoords()
    {
        packed = 0;
    }
    PackedTexCoords(unsigned int i)
    {
        packed = i;
    }
};

union PackedUnitVec // sizeof=0x4
{                                                                             // XREF: FX_GenSpriteVerts+8BE/w
                                                                                // FX_GenSpriteVerts+986/w ...
    unsigned int packed;
    unsigned __int8 array[4];
};

union PackedLightingCoords // sizeof=0x4
{                                       // XREF: R_SkinXSurfaceStaticVerts+E1/w
                                        // R_SkinXSurfaceStaticVerts+ED/r ...
    unsigned int packed;
    unsigned __int8 array[4];
};

PackedUnitVec __cdecl Vec3PackUnitVec(const float *unitVec);
void __cdecl Vec3UnpackUnitVec(PackedUnitVec in, float *out);
int __cdecl Vec4PackQuat(const float *in);
int __cdecl FloatPack9S(float unitFloat);
int __cdecl FloatPack10S(float unitFloat);
PackedTexCoords __cdecl Vec2PackTexCoords(const float *in);
void __cdecl Vec2UnpackTexCoords(PackedTexCoords in, float *out);
void __cdecl Byte4UnpackRgba(const unsigned __int8 *from, float *to);
