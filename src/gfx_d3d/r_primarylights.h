#pragma once
#include "r_rendercmds.h"
#include <DynEntity/DynEntity_gamestate.h>
#include "r_rendercmds.h"
#include "r_light.h"

struct ComPrimaryLight // sizeof=0xDC
{
    unsigned __int8 type;
    unsigned __int8 canUseShadowMap;
    unsigned __int8 exponent;
    char priority;
    __int16 cullDist;
    unsigned __int8 _pad[2];
    float color[3];
    float dir[3];
    float origin[3];
    float radius;
    float cosHalfFovOuter;
    float cosHalfFovInner;
    float cosHalfFovExpanded;
    float rotationLimit;
    float translationLimit;
    float mipDistance;
    float diffuseColor[4];
    float specularColor[4];
    float attenuation[4];
    float falloff[4];
    float angle[4];
    float aAbB[4];
    float cookieControl0[4];
    float cookieControl1[4];
    float cookieControl2[4];
    const char *defName;
};

struct GfxShadowedLightEntry // sizeof=0x8
{                                       // XREF: GfxShadowedLightHistory/r
    unsigned __int8 shadowableLightIndex;
    bool isFadingOut;
    // padding byte
    // padding byte
    float fade;
};

struct GfxShadowedLightHistory // sizeof=0x28
{                                       // XREF: .data:s_shadowHistory/r
    GfxShadowedLightEntry entries[4];
    unsigned int entryCount;
    unsigned int lastUpdateTime;
};

struct GfxCandidateShadowedLight // sizeof=0x8
{                                       // XREF: ?R_ChooseShadowedLights@@YAXPAUGfxViewInfo@@@Z/r
    unsigned int shadowableLightIndex;  // XREF: R_ChooseShadowedLights(GfxViewInfo *)+339/r
    float score;
};

struct GfxLightRegionAxis // sizeof=0x14
{
    float dir[3];
    float midPoint;
    float halfSize;
};

struct GfxLightRegionHull // sizeof=0x50
{
    float kdopMidPoint[9];
    float kdopHalfSize[9];
    unsigned int axisCount;
    GfxLightRegionAxis *axis;
};

struct GfxLightRegion // sizeof=0x8
{
    unsigned int hullCount;
    GfxLightRegionHull *hulls;
};

void __cdecl R_ClearShadowedPrimaryLightHistory(int localClientNum);
void __cdecl R_AddDynamicShadowableLight(GfxViewInfo *viewInfo, const GfxLight *visibleLight);
bool __cdecl R_IsDynamicShadowedLight(unsigned int shadowableLightIndex);
bool __cdecl R_IsPrimaryLight(unsigned int shadowableLightIndex);
void __cdecl R_ChooseShadowedLights(GfxViewInfo *viewInfo);
unsigned int __cdecl R_AddPotentiallyShadowedLight(
                const GfxViewInfo *viewInfo,
                unsigned int shadowableLightIndex,
                GfxCandidateShadowedLight *candidateLights,
                unsigned int candidateLightCount);
double __cdecl R_ShadowedSpotLightScore(const GfxViewParms *viewParms, const GfxLight *light);
void __cdecl R_AddShadowsForLight(
                GfxViewInfo *viewInfo,
                unsigned int shadowableLightIndex,
                unsigned int totalSpotLightCount,
                float spotShadowFade);
void __cdecl R_AddShadowedLightToShadowHistory(
                GfxShadowedLightHistory *shadowHistory,
                unsigned int shadowableLightIndex,
                float fadeDelta);
void __cdecl R_FadeOutShadowHistoryEntries(GfxShadowedLightHistory *shadowHistory, float fadeDelta);
void __cdecl R_LinkSphereEntityToPrimaryLights(
                int localClientNum,
                unsigned int entityNum,
                const float *origin,
                float radius);
unsigned int __cdecl R_GetPrimaryLightEntityShadowBit(
                int localClientNum,
                unsigned int entnum,
                unsigned int primaryLightIndex);
void __cdecl R_LinkBoxEntityToPrimaryLights(
                int localClientNum,
                unsigned int entityNum,
                const float *mins,
                const float *maxs);
char __cdecl R_CullBoxFromLightRegion(
                const GfxLightRegion *region,
                const float *lightOrigin,
                const float *boxMidPoint,
                const float *boxHalfSize);
char __cdecl R_CullBoxFromLightRegionHull(
                const GfxLightRegionHull *hull,
                const float *boxMidPoint,
                const float *boxHalfSize);
void __cdecl R_LinkDynEntToPrimaryLights(
                unsigned int dynEntId,
                DynEntityDrawType drawType,
                const float *mins,
                const float *maxs);
bool __cdecl Com_CullBoxFromPrimaryLight(
                const ComPrimaryLight *light,
                const float *boxMidPoint,
                const float *boxHalfSize);
unsigned int __cdecl R_GetPrimaryLightDynEntShadowBit(unsigned int entnum, unsigned int primaryLightIndex);
void __cdecl R_UnlinkEntityFromPrimaryLights(int localClientNum, unsigned int entityNum);
void __cdecl R_UnlinkDynEntFromPrimaryLights(unsigned int dynEntId, DynEntityDrawType drawType);
bool __cdecl R_IsEntityVisibleToPrimaryLight(
                int localClientNum,
                unsigned int entityNum,
                unsigned int primaryLightIndex);
bool __cdecl R_IsDynEntVisibleToPrimaryLight(
                unsigned int dynEntId,
                DynEntityDrawType drawType,
                unsigned int primaryLightIndex);
int __cdecl R_IsEntityVisibleToAnyShadowedPrimaryLight(const GfxViewInfo *viewInfo, unsigned int entityNum);
bool __cdecl R_IsEntityVisibleToShadowedPrimaryLight(unsigned int baseBitIndex, unsigned int shadowableLightIndex);
int __cdecl R_IsDynEntVisibleToAnyShadowedPrimaryLight(
                const GfxViewInfo *viewInfo,
                unsigned int dynEntId,
                DynEntityDrawType drawType);
bool __cdecl R_IsDynEntVisibleToShadowedPrimaryLight(
                unsigned int baseBitIndex,
                DynEntityDrawType drawType,
                unsigned int shadowableLightIndex);
unsigned int __cdecl R_GetNonSunPrimaryLightForBox(
                const GfxViewInfo *viewInfo,
                const float *boxMidPoint,
                const float *boxHalfSize);
unsigned int __cdecl R_GetNonSunPrimaryLightForSphere(const GfxViewInfo *viewInfo, const float *origin, float radius);
char __cdecl R_CullSphereFromLightRegion(
                const GfxLightRegion *region,
                const float *lightOrigin,
                const float *origin,
                float radius);
char __cdecl R_CullSphereFromLightRegionHull(const GfxLightRegionHull *hull, const float *origin, float radius);
bool __cdecl Com_CullSphereFromPrimaryLight(const ComPrimaryLight *light, const float *origin, float radius);
void __cdecl R_ShowPrimaryLightDebugLine(const float *lineStartPosition, unsigned int primaryLightIndex, int context);
