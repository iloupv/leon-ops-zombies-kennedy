#pragma once
#include "r_screenshot.h"
#include <cgame_mp/cg_local_mp.h>

struct GfxReflectionProbeVolumeData // sizeof=0x60
{
    float volumePlanes[6][4];
};

struct GfxReflectionProbe // sizeof=0x18
{
    float origin[3];
    GfxImage *reflectionImage;
    GfxReflectionProbeVolumeData *probeVolumes;
    unsigned int probeVolumeCount;
};

struct DiskGfxDummyReflectionProbe // sizeof=0xDC
{                                       // XREF: R_CreateDummyProbe/r
    float origin[3];                    // XREF: R_CreateDummyProbe+1B/w
    char colorCorrectionFilename[64];   // XREF: R_CreateDummyProbe+47/o
    unsigned __int8 pixels[144];        // XREF: R_CreateDummyProbe+56/o
};

struct ColorCorrectionData // sizeof=0x54
{                                       // XREF: .data:s_colorCorrectionDataEntries/r
    char name[64];
    float black_level;
    float white_level;
    float gamma;
    float saturation;
    float range;
};

void __cdecl R_GetReflectionProbePosition(unsigned int index, float *pos);
void __cdecl R_ReflectionProbeRegisterDvars();
void __cdecl R_CalcCubeMapViewValues(refdef_s *refdef, CubemapShot cubemapShot, int cubemapSize);
void __cdecl R_GenerateReflections(char *mapname, GfxReflectionProbe *probes, unsigned int probeCount);
void __cdecl R_GenerateReflectionRawDataAll(DiskGfxReflectionProbe *probeRawData, int probeCount, bool *generateProbe);
void    R_GenerateReflectionRawData(DiskGfxReflectionProbe *probeRawData);
char __cdecl R_CopyReflectionsFromLumpData(
                DiskGfxReflectionProbe *probeRawData,
                const DiskGfxReflectionProbe *probeRawLumpData,
                int lumpProbeCount);
bool __cdecl R_ReflectionProbeGenerateExitWhenDone();
void __cdecl R_SetBlackProbe(unsigned __int8 *target, unsigned int bytestozero);
void __cdecl R_GenerateDummyReflectionImages(
                GfxReflectionProbe *probes,
                const DiskGfxDummyReflectionProbe *probeRawData,
                int probeCount,
                int probeBaseIndex,
                int probeHighMipSize);
GfxImage *__cdecl R_GenerateReflectionImageFromRawData(
                const unsigned __int8 *rawPixels,
                int probeIndex,
                int probeHighMipSize);
void R_LoadColorCorrectionData();
void __cdecl R_ParseColorCorrectionData(const char *buf, const char *filename);
char __cdecl R_VerifyFieldNames(const char **buf, const char *filename);
ColorCorrectionData *R_CreateDefaultColorCorrectionEntry();
void __cdecl R_GenerateReflectionImages(
                GfxReflectionProbe *probes,
                const DiskGfxReflectionProbe *probeRawData,
                int probeCount,
                int probeBaseIndex,
                int probeHighMipSize);

extern const dvar_t *r_reflectionProbeRegenerateAll;
extern const dvar_t *r_reflectionProbeGenerateExit;