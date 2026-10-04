#pragma once
#include "r_gfx.h"

struct GfxViewInfo;
struct DObj;
struct GfxLight;
struct GfxDrawSurfListInfo;

struct BspSpotLightCallback // sizeof=0x4
{                                       // XREF: R_GetBspSpotLightSurfs/r
    const unsigned __int8 *surfaceVisData;
};

struct LightGlobals // sizeof=0x104
{                                       // XREF: .data:lightGlob/r
    int defCount;                       // XREF: R_RegisterLightDef_LoadObj+44/r
    GfxLightDef *defs[64];              // XREF: R_RegisterLightDef_LoadObj+53/r
};

struct StaticModelLightCallback // sizeof=0x14
{                                       // XREF: .data:g_staticModelLightCallback/r
    unsigned __int8 *smodelVisData;     // XREF: R_GetStaticModelLightSurfs(GfxLight const *,int)+5A/w
    float position[3];                  // XREF: R_GetStaticModelLightSurfs(GfxLight const *,int)+1E8/w
    float radiusSq;                     // XREF: R_GetStaticModelLightSurfs(GfxLight const *,int)+22C/w
};

GfxLightDef *__cdecl R_RegisterLightDef(const char *name);
GfxLightDef *__cdecl R_RegisterLightDef_LoadObj(const char *name);
GfxLightDef *__cdecl R_RegisterLightDef_FastFile(const char *name);
void __cdecl R_InitLightDefs();
void __cdecl R_ShutdownLightDefs();
int __cdecl R_GetPointLightPartitions(GfxLight *visibleLights);
void __cdecl R_MostImportantLights(const GfxLight **lights, int lightCount, int keepCount);
bool __cdecl R_LightImportanceGreaterEqual(const GfxLight *light0, const GfxLight *light1);
void __cdecl R_CalcSpotLightPlanes(const GfxLight *light, float dynamicSpotLightNearPlaneOffset, float (*planes)[4]);
void __cdecl R_CalcPlaneFromPointDir(float *plane, const float *origin, const float *dir);
void __cdecl R_ComputeSpotLightCrossDirs(const GfxLight *light, float (*crossDirs)[3]);
void __cdecl R_CalcPlaneFromCosSinPointDirs(
                float *plane,
                float fCos,
                float fSin,
                const float *origin,
                const float *forward,
                const float *lateral);
void __cdecl R_GetBspLightSurfs(const GfxLight *visibleLights, int visibleCount);
void __cdecl R_GetBspSpotLightSurfs(const GfxLight *light, int lightIndex, GfxBspDrawSurfData *surfData);
bool __cdecl R_SortBspLightSurfaces(GfxSurface *surface0, GfxSurface *surface1);
int __cdecl R_AllowBspSpotLightShadows(int surfIndex, void *__formal);
int __cdecl R_AllowBspSpotLight(int surfIndex, void *bspLightCallbackAsVoid);
void __cdecl R_GetStaticModelLightSurfs(const GfxLight *visibleLights, int visibleCount);
int __cdecl R_AllocDrawSurf(
                GfxDelayedCmdBuf *delayedCmdBuf,
                GfxDrawSurf drawSurf,
                GfxDrawSurfList *drawSurfList,
                unsigned int size);
int __cdecl R_AllowStaticModelOmniLight(int smodelIndex);
int __cdecl R_AllowStaticModelSpotLight(int smodelIndex);
void __cdecl R_GetSceneEntLightSurfs(const GfxLight *visibleLights, int visibleCount);
bool __cdecl R_SpotLightIsAttachedToDobj(const DObj *obj);
int __cdecl R_EmitPointLightPartitionSurfs(
                GfxViewInfo *viewInfo,
                const GfxLight *visibleLights,
                int visibleCount,
                const float *viewOrigin);
unsigned __int8 __cdecl R_GetTechniqueForLightType(const GfxLight *light, const GfxViewInfo *viewInfo);
void __cdecl R_EmitShadowedLightPartitionSurfs(
                GfxViewInfo *viewInfo,
                unsigned int lightDrawSurfCount,
                const GfxDrawSurf *lightDrawSurfs,
                GfxDrawSurfListInfo *info);
