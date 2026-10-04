#pragma once
#include "rb_state.h"
#include "r_rendercmds.h"

struct GfxLightingInfo // sizeof=0x8
{                                       // XREF: .data:lightingInfo/r
    unsigned __int8 primaryLightIndex;
    unsigned __int8 reflectionProbeIndex;
    // padding byte
    // padding byte
    unsigned int lightingHandle;
};

struct GfxFindLightForSphere // sizeof=0x14
{                                       // XREF: ?R_AllocModelLighting_Sphere@@YAIPBUGfxViewInfo@@QBMM1M_NPAGPAUGfxLightingInfo@@@Z/r
    const GfxViewInfo *viewInfo;        // XREF: R_AllocModelLighting_Sphere(GfxViewInfo const *,float const * const,float,float const * const,float,bool,ushort *,GfxLightingInfo *)+9/w
    float origin[3];                    // XREF: R_AllocModelLighting_Sphere(GfxViewInfo const *,float const * const,float,float const * const,float,bool,ushort *,GfxLightingInfo *)+13/w
                                        // R_AllocModelLighting_Sphere(GfxViewInfo const *,float const * const,float,float const * const,float,bool,ushort *,GfxLightingInfo *)+20/w ...
    float radius;                       // XREF: R_AllocModelLighting_Sphere(GfxViewInfo const *,float const * const,float,float const * const,float,bool,ushort *,GfxLightingInfo *)+37/w
};

struct GfxFindLightForBox // sizeof=0x1C
{                                       // XREF: ?R_AllocModelLighting_Box@@YAIPBUGfxViewInfo@@QBMM11_NPAGPAUGfxLightingInfo@@@Z/r
    const GfxViewInfo *viewInfo;        // XREF: R_AllocModelLighting_Box(GfxViewInfo const *,float const * const,float,float const * const,float const * const,bool,ushort *,GfxLightingInfo *)+9/w
    float midPoint[3];                  // XREF: R_AllocModelLighting_Box(GfxViewInfo const *,float const * const,float,float const * const,float const * const,bool,ushort *,GfxLightingInfo *)+22/w
                                        // R_AllocModelLighting_Box(GfxViewInfo const *,float const * const,float,float const * const,float const * const,bool,ushort *,GfxLightingInfo *)+3F/w ...
    float halfSize[3];                  // XREF: R_AllocModelLighting_Box(GfxViewInfo const *,float const * const,float,float const * const,float const * const,bool,ushort *,GfxLightingInfo *)+6D/w
};

enum GfxModelLightExtrapolation : __int32;

struct CalcLightingCmd // sizeof=0x2C
{                                       // XREF: R_AllocModelLighting/r
    unsigned int entryIndex;            // XREF: R_AllocModelLighting+44F/w
    float lightingOrigin[3];            // XREF: R_AllocModelLighting+459/w
                                        // R_AllocModelLighting+466/w ...
    unsigned int nonSunPrimaryLightIndex;
                                        // XREF: R_AllocModelLighting+47B/w
    GfxModelLightExtrapolation extrapolateBehavior;
                                        // XREF: R_AllocModelLighting+47E/w
    bool useHeroLighting;               // XREF: R_AllocModelLighting+488/w
    // padding byte
    // padding byte
    // padding byte
    unsigned __int8 *primaryLightIndex1;
                                        // XREF: R_AllocModelLighting+48E/w
    unsigned __int8 *primaryLightIndex2;
                                        // XREF: R_AllocModelLighting+49D/w
    unsigned __int8 *pixels;            // XREF: R_AllocModelLighting+4A8/w
    unsigned __int8 *modelLightGlobPrimaryLightIndex;
};

void __cdecl R_SetModelLightingCoords(unsigned __int16 handle, float *out);
unsigned int __cdecl R_ModelLightingIndexFromHandle(unsigned __int16 handle);
char __cdecl R_AllocStaticModelLighting(GfxStaticModelDrawInst *smodelDrawInst, unsigned int smodelIndex);
unsigned int __cdecl R_AllocModelLighting_DynEnts(
                float *lightingOrigin,
                unsigned int dynEntId,
                bool useHeroLighting,
                unsigned __int16 *cachedLightingHandle,
                GfxLightingInfo *lightingInfoOut);
unsigned int __cdecl R_AllocModelLighting(
                float *lightingOrigin,
                float lightingOriginToleranceSq,
                bool useHeroLighting,
                unsigned __int16 *cachedLightingHandle,
                unsigned int (__cdecl *GetPrimaryLightCallback)(const void *),
                const void *userData,
                GfxLightingInfo *lightingInfoOut);
unsigned int __cdecl R_DynEntPrimaryLightCallback(unsigned int *userData);
unsigned int __cdecl R_AllocModelLighting_PrimaryLight(
                float *lightingOrigin,
                unsigned int primaryLightIndex,
                bool useHeroLighting,
                unsigned __int16 *cachedLightingHandle,
                GfxLightingInfo *lightingInfoOut);
unsigned int __cdecl R_PassbackPrimaryLightCallback(const void *userData);
unsigned int __cdecl R_AllocModelLighting_Box(
                const GfxViewInfo *viewInfo,
                float *lightingOrigin,
                float originToleranceSq,
                const float *boxMins,
                const float *boxMaxs,
                bool useHeroLighting,
                unsigned __int16 *cachedLightingHandle,
                GfxLightingInfo *lightingInfoOut);
unsigned int __cdecl R_GetPrimaryLightForBoxCallback(const void *userData);
unsigned int __cdecl R_AllocModelLighting_Sphere(
                const GfxViewInfo *viewInfo,
                float *lightingOrigin,
                float originToleranceSq,
                const float *origin,
                float radius,
                bool useHeroLighting,
                unsigned __int16 *cachedLightingHandle,
                GfxLightingInfo *lightingInfoOut);
unsigned int __cdecl R_GetPrimaryLightForSphereCallback(const void *userData);
void __cdecl R_ToggleModelLightingFrame();
void __cdecl R_CalcModelLightingCmd(CalcLightingCmd *data);
void __cdecl R_CalcModelLighting(
                unsigned int entryIndex,
                const float *lightingOrigin,
                unsigned int nonSunPrimaryLightIndex,
                GfxModelLightExtrapolation extrapolateBehavior,
                bool useHeroLighting,
                unsigned __int8 *primaryLightIndex1,
                unsigned __int8 *primaryLightIndex2);
void __cdecl R_BeginAllStaticModelLighting();
void __cdecl R_SetAllStaticModelLighting();
void __cdecl R_SetStaticModelLighting(unsigned int smodelIndex);
void __cdecl R_SetModelGroundLighting(unsigned int entryIndex, const unsigned __int8 *groundLighting);
void __cdecl R_SetStaticModelLightingCoordsForSource(unsigned int smodelIndex, GfxCmdBufSourceState *source);
void __cdecl R_SetStaticModelLightingCoords(unsigned int smodelIndex, float *out);
void __cdecl R_GetPackedStaticModelLightingCoords(unsigned int smodelIndex, PackedLightingCoords *packedCoords);
bool __cdecl R_StaticModelHasLighting(unsigned int smodelIndex);
void __cdecl RB_PatchModelLighting(const GfxModelLightingPatch *patchList, unsigned int patchCount);
unsigned int R_SetModelLightingSampleDeltas();
void __cdecl R_SetModelLightingLookupScale(GfxCmdBufInput *input);
void __cdecl R_SetupDynamicModelLighting(GfxCmdBufInput *input);
void __cdecl R_SetupCachedStaticModelLighting(GfxCmdBufSourceState *source);
void __cdecl R_InitModelLightingGlobals();
char *__cdecl R_AllocModelLightingGlobal(unsigned int bytes);
void __cdecl R_ShutdownModelLightingGlobals();
void __cdecl R_ResetModelLighting();
void __cdecl R_InitModelLightingImage();
void __cdecl R_ShutdownModelLightingImage();
void __cdecl R_InitStaticModelLighting();
void __cdecl R_SetModelLightingCoordsForSource(unsigned __int16 handle, GfxCmdBufSourceState *source);

extern GfxLightingInfo lightingInfo[1024];
