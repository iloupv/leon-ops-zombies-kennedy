#pragma once
#include "r_utils.h"
#include "r_pointlights.h"

enum GfxSortedHistoryAdd : __int32
{                                       // XREF: R_SortedHistoryEntry/r
    SH_ADD_NEVER  = 0x0,
    SH_ADD_IF_NEW = 0x1,
};

enum GfxModelLightExtrapolation : __int32
{                                       // XREF: CalcLightingCmd/r
                                        // R_CalcModelLighting/r ...
    GFX_MODELLIGHT_EXTRAPOLATE  = 0x0,
    GFX_MODELLIGHT_SHOW_MISSING = 0x1,
};

struct GfxCompressedLightGridColors // sizeof=0xA8
{                                       // XREF: R_LoadLightGridPoints_Version15/r
    unsigned __int8 rgb[56][3];
};

struct GfxLightGridEntry // sizeof=0x4
{                                       // XREF: AnnotatedLightGridPoint/r
    unsigned __int16 colorsIndex;
    unsigned __int8 primaryLightIndex;  // XREF: std::_Insertion_sort1<AnnotatedLightGridPoint *,bool (*)(AnnotatedLightGridPoint const &,AnnotatedLightGridPoint const &),AnnotatedLightGridPoint>(AnnotatedLightGridPoint *,AnnotatedLightGridPoint *,bool (*)(AnnotatedLightGridPoint const &,AnnotatedLightGridPoint const &),AnnotatedLightGridPoint *)+4F/w
                                        // std::_Insertion_sort1<AnnotatedLightGridPoint *,bool (*)(AnnotatedLightGridPoint const &,AnnotatedLightGridPoint const &),AnnotatedLightGridPoint>(AnnotatedLightGridPoint *,AnnotatedLightGridPoint *,bool (*)(AnnotatedLightGridPoint const &,AnnotatedLightGridPoint const &),AnnotatedLightGridPoint *)+93/r ...
    unsigned __int8 needsTrace;
};

struct GfxLightGridRow // sizeof=0xC
{                                       // XREF: R_CompressLightGridRow_Version15/r
    unsigned __int16 colStart;          // XREF: R_CompressLightGridRow_Version15+21/w
                                        // R_CompressLightGridRow_Version15+3B/r ...
    unsigned __int16 colCount;          // XREF: R_CompressLightGridRow_Version15+44/w
    unsigned __int16 zStart;            // XREF: R_CompressLightGridRow_Version15+4E/w
                                        // R_CompressLightGridRow_Version15+59/r ...
    unsigned __int16 zCount;            // XREF: R_CompressLightGridRow_Version15+62/w
    unsigned int firstEntry;            // XREF: R_CompressLightGridRow_Version15+B/w
                                        // R_CompressLightGridRow_Version15+7D/r
};

struct GfxLightGrid // sizeof=0x38
{                                       // XREF: GfxWorld/r
    bool hasLightRegions;               // XREF: R_LoadLightRegions+41/w
                                        // R_LoadLightRegions+47/r
    // padding byte
    // padding byte
    // padding byte
    unsigned int sunPrimaryLightIndex;  // XREF: R_LoadLightGridPoints_Version15+1A8/w
                                        // R_LoadLightGridPoints_Version15+2C2/r ...
    unsigned __int16 mins[3];           // XREF: R_LoadLightGridPoints_Version15+1B3/w
                                        // R_LoadLightGridPoints_Version15+1BE/w ...
    unsigned __int16 maxs[3];           // XREF: R_LoadLightGridPoints_Version15+1D3/w
                                        // R_LoadLightGridPoints_Version15+1DB/w ...
    unsigned int rowAxis;               // XREF: R_LoadLightGridPoints_Version15+72E/w
                                        // R_LoadLightGridPoints_Version15:loc_A93BA4/w ...
    unsigned int colAxis;               // XREF: R_LoadLightGridPoints_Version15+738/w
                                        // R_LoadLightGridPoints_Version15+74E/w ...
    unsigned __int16 *rowDataStart;     // XREF: R_LoadLightGridPoints_Version15+886/w
                                        // R_LoadLightGridPoints_Version15+896/r ...
    unsigned int rawRowDataSize;        // XREF: R_LoadLightGridRowData+4/o
                                        // R_LoadLightGridRowData+18/r ...
    unsigned __int8 *rawRowData;        // XREF: R_LoadLightGridRowData+21/w
                                        // R_LoadLightGridRowData+42/w ...
    unsigned int entryCount;            // XREF: R_LoadLightGridPoints_Version15+8E5/w
                                        // R_LoadLightGridPoints_Version15+906/w ...
    GfxLightGridEntry *entries;         // XREF: R_LoadLightGridPoints_Version15+8E0/w
                                        // R_EmitDefaultLightGridEntry_Version15+B/r ...
    unsigned int colorCount;            // XREF: R_LoadLightGridPoints_Version15+207/r
                                        // R_LoadLightGridPoints_Version15+21C/r ...
    GfxCompressedLightGridColors *colors;
                                        // XREF: R_LoadLightGridPoints_Version15+650/r
                                        // R_LoadLightGridPoints_Version15+663/r ...
};

struct __declspec(align(8)) htab // sizeof=0x10
{
    unsigned __int64 value;
    htab *next;
    // padding byte
    // padding byte
    // padding byte
    // padding byte
};

struct GfxModelLightingPatch;
struct GfxBackEndData;
struct ComPrimaryLight;

void __cdecl R_ShowLightVisCachePoints(const float *viewOrigin, const DpvsPlane *clipPlanes, int clipPlaneCount);
bool __cdecl R_SortedHistoryEntry(
                unsigned __int16 x,
                unsigned __int16 y,
                unsigned __int16 z,
                GfxSortedHistoryAdd addMode);
htab *__cdecl addHash(const unsigned __int16 (*data)[3]);
htab *__cdecl htabAlloc();
unsigned __int64 __cdecl HashXyz(const unsigned __int16 (*p)[3]);
int __cdecl HashPackPoint(const unsigned __int16 (*p)[3]);
htab *__cdecl findhash(const unsigned __int16 (*data)[3]);
void __cdecl R_ApplyLightGridColorsPatch(const GfxModelLightingPatch *patch, unsigned __int8 *pixels);
void __cdecl R_SetLightGridColorsVec4(
                const GfxDecodedLightGridColors *colors,
                float primaryWeight,
                unsigned __int8 *pixels);
void __cdecl SetPixelColorLightGridVec4_PC(unsigned __int8 *v, const float *color, float primaryWeight);
void __cdecl R_DecodeLightGridColors(
                const GfxCompressedLightGridColors *colors,
                GfxDecodedLightGridColors *decodedColors);
void __cdecl R_DecodeLightGridColorsWeighted(
                const GfxCompressedLightGridColors *colors,
                GfxDecodedLightGridColors *decodedColors,
                float weight);
unsigned __int8 __cdecl R_GetPrimaryLightFromGrid(
                const GfxLightGrid *lightGrid,
                const float *samplePos,
                unsigned __int8 sunPrimaryLightIndex);
unsigned __int8 __cdecl R_LightGridLookup(
                const GfxLightGrid *lightGrid,
                const float *samplePos,
                float *cornerWeight,
                const GfxLightGridEntry **cornerEntry,
                unsigned int *defaultGridEntry);
void __cdecl R_ShowLightGrid(
                const GfxLightGrid *lightGrid,
                const unsigned int *pos,
                const float *samplePos,
                const GfxLightGridEntry **cornerEntry,
                bool *suppressEntry,
                bool honorSuppression);
void __cdecl R_ShowGridOrigin(const float *origin);
void __cdecl R_ShowGridBox(const unsigned int *pos);
void __cdecl R_ShowGridCorner(unsigned int x, unsigned int y, unsigned int z, float halfSize, const float *color);
void __cdecl R_UpdateVisHistory(const GfxLightGrid *lightGrid, const unsigned int *pos);
void AllocAllMemoryNeeded();
void __cdecl R_GetLightGridSampleEntryQuad(
                const GfxLightGrid *lightGrid,
                const unsigned int *pos,
                const GfxLightGridEntry **entries,
                unsigned int *defaultGridEntry);
bool __cdecl R_IsValidLightGridSample(
                const GfxLightGrid *lightGrid,
                const GfxLightGridEntry *entry,
                char cornerIndex,
                const unsigned int *pos,
                const float *samplePos);
unsigned int __cdecl R_GetLightingAtPoint(
                const GfxLightGrid *remoteLightGrid,
                const float *samplePos,
                unsigned int nonSunPrimaryLightIndex,
                unsigned __int16 dest,
                GfxModelLightExtrapolation extrapolateBehavior,
                bool useHeroLighting);
void __cdecl R_SetLightGridColorsFromIndex(
                const GfxLightGrid *lightGrid,
                unsigned int colorsIndex,
                const float *heroPos,
                float primaryLightWeight,
                unsigned __int16 dest);
GfxModelLightingPatch *__cdecl R_BackEndDataAllocAndClearModelLightingPatch(GfxBackEndData *frontEndDataOut);
void __cdecl R_BlendAndSetLightGridColors(
                const GfxLightGrid *lightGrid,
                unsigned __int8 *colorsIndex,
                const float *colorsWeight,
                unsigned int colorsCount,
                const float *heroPos,
                float primaryLightWeight,
                float weightNormalizeScale,
                unsigned __int16 dest);
unsigned __int8 __cdecl R_ExtrapolateLightingAtPoint(
                const GfxLightGrid *lightGrid,
                const float *heroPos,
                unsigned __int16 dest,
                GfxModelLightExtrapolation extrapolateBehavior,
                unsigned int defaultGridEntry);
char __cdecl R_CanLightInfluenceLightGridCorner(
                const GfxLightGrid *lightGrid,
                const ComPrimaryLight *light,
                const float *samplePos,
                char cornerIndex);
void __cdecl R_GetAverageLightingAtPoint(const float *samplePos, float *outColor);
void __cdecl R_GetAverageSecondaryLightingAtPoint(const float *samplePos, float *outColor);
void __cdecl R_SetLightGridSampleDeltas(int rowStride, int sliceStride);
void __cdecl R_InitLightVisHistory(const char *bspName);
void __cdecl R_LightVisHistoryFilename(char *bspName, char *filename);
void __cdecl R_SaveLightVisHistory();
void __cdecl HashUnpackPoint(unsigned __int64 value, unsigned __int16 (*p)[3]);
//int __cdecl find_hash(const char *a1); // in libtomcrypt
