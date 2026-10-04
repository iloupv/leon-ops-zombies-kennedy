#pragma once
#include <universal/fft.h>

#include <d3d9.h>

struct water_t;
union GfxColor;

struct WaterGlob // sizeof=0x9000
{                                       // XREF: .data:waterGlob/r
    complex_s H[4096];
    unsigned __int8 pixels[4096];       // XREF: R_UploadWaterTextureInternal(void *)+48/o
                                        // R_UploadWaterTextureInternal(void *)+59/o ...
};

struct WaterGlobStatic // sizeof=0x1C00
{                                       // XREF: .data:waterGlobStatic/r
    float sinTable[1024];               // XREF: WaterFrequenciesAtTime+187/r
                                        // WaterFrequenciesAtTime+198/r ...
    complex_s fftTrigTable[256];        // XREF: WaterAmplitudesFromFrequencies:loc_AC4971/o
                                        // WaterAmplitudesFromFrequencies:loc_AC4A32/o ...
    int fftBitswap[256];                // XREF: WaterAmplitudesFromFrequencies+146/o
                                        // WaterAmplitudesFromFrequencies+207/o ...
};

void __cdecl R_UploadWaterTextureInternal(water_t **data);
void __cdecl WaterFrequenciesAtTime(complex_s *H, const water_t *water, float t);
void __cdecl WaterAmplitudesFromFrequencies(complex_s *H, const water_t *water);
void __cdecl TransposeArray(complex_s *H, unsigned int M);
void __cdecl WaterPixelsFromAmplitudes(GfxColor *pixels, complex_s *H, const water_t *water);
void __cdecl GenerateMipMaps(_D3DFORMAT format, unsigned __int8 *pixels, water_t *water);
void __cdecl R_UploadWaterTexture(water_t *water, float floatTime);
void __cdecl R_InitWater();
void __cdecl Load_PicmipWater(water_t **waterRef);


extern volatile unsigned int g_waterLock;
