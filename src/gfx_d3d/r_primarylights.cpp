#include "r_primarylights.h"
#include <cgame_mp/cg_ents_mp.h>
#include <client/con_channels.h>
#include "r_dvars.h"
#include "r_spotshadow.h"
#include <physics/rope.h>
#include "r_debug.h"

GfxShadowedLightHistory s_shadowHistory[4];

void __cdecl R_ClearShadowedPrimaryLightHistory(int localClientNum)
{
    s_shadowHistory[localClientNum].entryCount = 0;
    s_shadowHistory[localClientNum].lastUpdateTime = 0;
}

void __cdecl R_AddDynamicShadowableLight(GfxViewInfo *viewInfo, const GfxLight *visibleLight)
{
    if ( frontEndDataOut->shadowableLightCount == 255 )
    {
        Com_PrintError(1, "Too many total shadowable lights (%d)\n", frontEndDataOut->shadowableLightCount);
    }
    else
    {
        Com_BitSetAssert(scene.shadowableLightIsUsed, frontEndDataOut->shadowableLightCount, 128);
        memcpy(
            &frontEndDataOut->shadowableLights[frontEndDataOut->shadowableLightCount++],
            visibleLight,
            sizeof(frontEndDataOut->shadowableLights[frontEndDataOut->shadowableLightCount++]));
    }
}

bool __cdecl R_IsDynamicShadowedLight(unsigned int shadowableLightIndex)
{
    return shadowableLightIndex >= Com_GetPrimaryLightCount();
}

bool __cdecl R_IsPrimaryLight(unsigned int shadowableLightIndex)
{
    return shadowableLightIndex < Com_GetPrimaryLightCount();
}

void __cdecl R_ChooseShadowedLights(GfxViewInfo *viewInfo)
{
    bool v1; // zf
    DWORD v2; // eax
    DWORD v3; // eax
    float v4; // [esp+Ch] [ebp-A0h]
    float v5; // [esp+18h] [ebp-94h]
    float cullDist; // [esp+28h] [ebp-84h]
    float distSq; // [esp+2Ch] [ebp-80h]
    GfxCandidateShadowedLight candidateLights[5]; // [esp+38h] [ebp-74h] BYREF
    unsigned int timeDelta; // [esp+60h] [ebp-4Ch]
    unsigned int entryIndex; // [esp+64h] [ebp-48h]
    unsigned int scanIndex; // [esp+68h] [ebp-44h]
    GfxShadowedLightHistory *shadowHistory; // [esp+6Ch] [ebp-40h]
    unsigned int leadingZeros; // [esp+70h] [ebp-3Ch]
    unsigned int bitIndex; // [esp+74h] [ebp-38h]
    unsigned int usedBits; // [esp+78h] [ebp-34h]
    float fadeDelta; // [esp+7Ch] [ebp-30h]
    unsigned int candidateLightIndex; // [esp+80h] [ebp-2Ch]
    unsigned int candidateLightCount; // [esp+84h] [ebp-28h]
    unsigned int scanLimit; // [esp+88h] [ebp-24h]
    unsigned int shadowableLightIsUsed[8]; // [esp+8Ch] [ebp-20h] BYREF

    shadowHistory = &s_shadowHistory[viewInfo->localClientNum];
    timeDelta = viewInfo->sceneDef.time - shadowHistory->lastUpdateTime;
    if ( timeDelta )
    {
        if ( frontEndDataOut->shadowableLightCount < Com_GetPrimaryLightCount()
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_primarylights.cpp",
                        210,
                        0,
                        "%s",
                        "frontEndDataOut->shadowableLightCount >= Com_GetPrimaryLightCount()") )
        {
            __debugbreak();
        }
        scanLimit = (frontEndDataOut->shadowableLightCount + 31) >> 5;
        for ( scanIndex = 0; scanIndex != scanLimit; ++scanIndex )
        {
            usedBits = scene.shadowableLightIsUsed[scanIndex];
            while ( 1 )
            {
                v1 = !_BitScanReverse(&v2, usedBits);
                if (v1)
                    v2 = 63;// `CountLeadingZeros'::`2': : notFound;
                leadingZeros = v2 ^ 0x1F;
                if ( (v2 ^ 0x1F) == 0x20 )
                    break;
                bitIndex = 31 - leadingZeros;
                usedBits &= ~(1 << (31 - leadingZeros));
                if ( frontEndDataOut->shadowableLights[32 * scanIndex + 31 - leadingZeros].type != 1
                    && frontEndDataOut->shadowableLights[32 * scanIndex + 31 - leadingZeros].type != 4 )
                {
                    cullDist = (float)frontEndDataOut->shadowableLights[32 * scanIndex + 31 - leadingZeros].cullDist;
                    distSq = Vec3DistanceSq(
                                         frontEndDataOut->shadowableLights[32 * scanIndex + 31 - leadingZeros].origin,
                                         viewInfo->cullViewInfo.viewParms.origin);
                    if ( distSq > (float)(cullDist * cullDist) )
                        Com_BitClearAssert(scene.shadowableLightIsUsed, 31 - leadingZeros + 32 * scanIndex, 128);
                }
            }
        }
        shadowHistory->lastUpdateTime = viewInfo->sceneDef.time;
        v5 = 0.001 / sm_spotShadowFadeTime->current.value * (double)timeDelta;
        if ( (float)(v5 - 1.0) < 0.0 )
            v4 = 0.001 / sm_spotShadowFadeTime->current.value * (double)timeDelta;
        else
            v4 = 1.0f;
        fadeDelta = v4;
        R_FadeOutShadowHistoryEntries(shadowHistory, v4);
        memcpy(shadowableLightIsUsed, scene.shadowableLightIsUsed, sizeof(shadowableLightIsUsed));
        candidateLightCount = 0;
        for ( scanIndex = 0; scanIndex != scanLimit; ++scanIndex )
        {
            usedBits = scene.shadowableLightIsUsed[scanIndex];
            while ( 1 )
            {
                v1 = !_BitScanReverse(&v3, usedBits);
                if (v1)
                    v3 = 63;// `CountLeadingZeros'::`2': : notFound;
                leadingZeros = v3 ^ 0x1F;
                if ( (v3 ^ 0x1F) == 0x20 )
                    break;
                bitIndex = 31 - leadingZeros;
                usedBits &= ~(1 << (31 - leadingZeros));
                candidateLightCount = R_AddPotentiallyShadowedLight(
                                                                viewInfo,
                                                                31 - leadingZeros + 32 * scanIndex,
                                                                candidateLights,
                                                                candidateLightCount);
            }
        }
        for ( candidateLightIndex = 0; candidateLightIndex < candidateLightCount; ++candidateLightIndex )
            R_AddShadowedLightToShadowHistory(
                shadowHistory,
                candidateLights[candidateLightIndex].shadowableLightIndex,
                fadeDelta);
    }
    else if ( rgp.world->sunPrimaryLightIndex
                 && Com_BitCheckAssert(scene.shadowableLightIsUsed, rgp.world->sunPrimaryLightIndex, 128) )
    {
        Com_BitSetAssert(frontEndDataOut->shadowableLightHasShadowMap, rgp.world->sunPrimaryLightIndex, 32);
    }
    if ( frontEndDataOut->spotShadowCount
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_primarylights.cpp",
                    266,
                    1,
                    "frontEndDataOut->spotShadowCount == 0\n\t%i, %i",
                    frontEndDataOut->spotShadowCount,
                    0) )
    {
        __debugbreak();
    }
    for ( entryIndex = 0; entryIndex < shadowHistory->entryCount; ++entryIndex )
        R_AddShadowsForLight(
            viewInfo,
            shadowHistory->entries[entryIndex].shadowableLightIndex,
            shadowHistory->entryCount,
            shadowHistory->entries[entryIndex].fade);
}

unsigned int __cdecl R_AddPotentiallyShadowedLight(
                const GfxViewInfo *viewInfo,
                unsigned int shadowableLightIndex,
                GfxCandidateShadowedLight *candidateLights,
                unsigned int candidateLightCount)
{
    float v5; // ecx
    unsigned int insertIndex; // [esp+14h] [ebp-8h]
    float score; // [esp+18h] [ebp-4h]

    if ( !shadowableLightIndex )
        return candidateLightCount;
    if ( shadowableLightIndex == rgp.world->sunPrimaryLightIndex )
    {
        Com_BitSetAssert(frontEndDataOut->shadowableLightHasShadowMap, shadowableLightIndex, 32);
        return candidateLightCount;
    }
    else if ( frontEndDataOut->shadowableLights[shadowableLightIndex].canUseShadowMap && sm_spotEnable->current.enabled )
    {
        score = R_ShadowedSpotLightScore(
                            &viewInfo->cullViewInfo.viewParms,
                            &frontEndDataOut->shadowableLights[shadowableLightIndex]);
        for ( insertIndex = candidateLightCount; insertIndex && score > candidateLights[insertIndex - 1].score; --insertIndex )
        {
            v5 = candidateLights[insertIndex - 1].score;
            candidateLights[insertIndex].shadowableLightIndex = candidateLights[insertIndex - 1].shadowableLightIndex;
            candidateLights[insertIndex].score = v5;
        }
        candidateLights[insertIndex].shadowableLightIndex = shadowableLightIndex;
        candidateLights[insertIndex].score = score;
        if ( (signed int)(candidateLightCount + 1) < sm_maxLights->current.integer )
            return candidateLightCount + 1;
        else
            return sm_maxLights->current.unsignedInt;
    }
    else
    {
        return candidateLightCount;
    }
}

double __cdecl R_ShadowedSpotLightScore(const GfxViewParms *viewParms, const GfxLight *light)
{
    float v3; // [esp+Ch] [ebp-40h]
    float value; // [esp+18h] [ebp-34h]
    float deltaToLight_4; // [esp+34h] [ebp-18h]
    float deltaToLight_8; // [esp+38h] [ebp-14h]
    float distToLightFocus; // [esp+3Ch] [ebp-10h]
    float deltaToFocus[3]; // [esp+40h] [ebp-Ch] BYREF

    if ( light->def == rgp.flashLightDef )
        return 100000000.0;
    value = sm_lightScore_eyeProjectDist->current.value;
    deltaToLight_4 = light->origin[1] - (float)((float)(value * viewParms->axis[0][1]) + viewParms->origin[1]);
    deltaToLight_8 = light->origin[2] - (float)((float)(value * viewParms->axis[0][2]) + viewParms->origin[2]);
    v3 = (-(light->radius)) * sm_lightScore_spotProjectFrac->current.value;
    deltaToFocus[0] = (float)(v3 * light->dir[0])
                                    + (float)(light->origin[0] - (float)((float)(value * viewParms->axis[0][0]) + viewParms->origin[0]));
    deltaToFocus[1] = (float)(v3 * light->dir[1]) + deltaToLight_4;
    deltaToFocus[2] = (float)(v3 * light->dir[2]) + deltaToLight_8;
    distToLightFocus = Vec3Length(deltaToFocus);
    return light->radius
             * (float)((float)((float)(light->color[0] * 0.29899999) + (float)(light->color[1] * 0.58700001))
                             + (float)(light->color[2] * 0.114))
             / (distToLightFocus + 1.0);
}

void __cdecl R_AddShadowsForLight(
                GfxViewInfo *viewInfo,
                unsigned int shadowableLightIndex,
                unsigned int totalSpotLightCount,
                float spotShadowFade)
{
    if ( R_AddSpotShadowsForLight(
                 viewInfo,
                 &frontEndDataOut->shadowableLights[shadowableLightIndex],
                 shadowableLightIndex,
                 totalSpotLightCount,
                 spotShadowFade) )
    {
        Com_BitSetAssert(frontEndDataOut->shadowableLightHasShadowMap, shadowableLightIndex, 32);
    }
}

void __cdecl R_AddShadowedLightToShadowHistory(
                GfxShadowedLightHistory *shadowHistory,
                unsigned int shadowableLightIndex,
                float fadeDelta)
{
    float v3; // [esp+0h] [ebp-Ch]
    unsigned int historyIndex; // [esp+8h] [ebp-4h]

    for ( historyIndex = 0; historyIndex != shadowHistory->entryCount; ++historyIndex )
    {
        if ( shadowHistory->entries[historyIndex].shadowableLightIndex == shadowableLightIndex )
        {
            shadowHistory->entries[historyIndex].isFadingOut = 0;
            if ( (float)((float)(shadowHistory->entries[historyIndex].fade + fadeDelta) - 1.0) < 0.0 )
                v3 = shadowHistory->entries[historyIndex].fade + fadeDelta;
            else
                v3 = 1.0f;
            shadowHistory->entries[historyIndex].fade = v3;
            return;
        }
    }
    if ( shadowHistory->entryCount < sm_maxLights->current.integer )
    {
        shadowHistory->entries[shadowHistory->entryCount].shadowableLightIndex = shadowableLightIndex;
        if ( shadowHistory->entries[shadowHistory->entryCount].shadowableLightIndex != shadowableLightIndex
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_primarylights.cpp",
                        119,
                        0,
                        "%s",
                        "shadowHistory->entries[shadowHistory->entryCount].shadowableLightIndex == shadowableLightIndex") )
        {
            __debugbreak();
        }
        shadowHistory->entries[shadowHistory->entryCount].isFadingOut = 0;
        shadowHistory->entries[shadowHistory->entryCount++].fade = fadeDelta;
    }
}

void __cdecl R_FadeOutShadowHistoryEntries(GfxShadowedLightHistory *shadowHistory, float fadeDelta)
{
    unsigned int v2; // edx
    int v3; // ecx
    float fade; // edx
    unsigned int entryIndex; // [esp+4h] [ebp-4h]

    entryIndex = 0;
    while ( entryIndex != shadowHistory->entryCount )
    {
        if ( shadowHistory->entries[entryIndex].fade <= 0.0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_primarylights.cpp",
                        132,
                        0,
                        "%s",
                        "shadowHistory->entries[entryIndex].fade > 0.0f") )
        {
            __debugbreak();
        }
        if ( shadowHistory->entries[entryIndex].isFadingOut )
        {
            shadowHistory->entries[entryIndex].fade = shadowHistory->entries[entryIndex].fade - fadeDelta;
            if ( shadowHistory->entries[entryIndex].fade < 0.0099999998 )
            {
                v2 = --shadowHistory->entryCount;
                v3 = *(unsigned int *)&shadowHistory->entries[v2].shadowableLightIndex;
                fade = shadowHistory->entries[v2].fade;
                *(unsigned int *)&shadowHistory->entries[entryIndex].shadowableLightIndex = v3;
                shadowHistory->entries[entryIndex].fade = fade;
            }
            else
            {
                ++entryIndex;
            }
        }
        else
        {
            shadowHistory->entries[entryIndex++].isFadingOut = 1;
        }
    }
}

void __cdecl R_LinkSphereEntityToPrimaryLights(
                int localClientNum,
                unsigned int entityNum,
                const float *origin,
                float radius)
{
    unsigned int primaryLightIndex; // [esp+24h] [ebp-Ch]
    const ComPrimaryLight *light; // [esp+28h] [ebp-8h]
    unsigned int bitIndex; // [esp+2Ch] [ebp-4h]

    for ( primaryLightIndex = rgp.world->sunPrimaryLightIndex + 1;
                primaryLightIndex < rgp.world->primaryLightCount;
                ++primaryLightIndex )
    {
        light = Com_GetPrimaryLight(primaryLightIndex);
        if ( light->type != 2
            && light->type != 3
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_primarylights.cpp",
                        314,
                        0,
                        "%s\n\t(light->type) = %i",
                        "(light->type == GFX_LIGHT_TYPE_SPOT || light->type == GFX_LIGHT_TYPE_OMNI)",
                        light->type) )
        {
            __debugbreak();
        }
        if ( Vec3DistanceSq(light->origin, origin) < (light->radius + radius) * (light->radius + radius)
            && (light->type != 2
             || light->cosHalfFovExpanded < 0.0
             || !CullSphereFromCone(light->origin, light->dir, light->cosHalfFovExpanded, origin, radius)) )
        {
            bitIndex = R_GetPrimaryLightEntityShadowBit(localClientNum, entityNum, primaryLightIndex);
            Com_BitSetAssert(rgp.world->primaryLightEntityShadowVis, bitIndex, 0xFFFFFFF);
        }
    }
}

unsigned int __cdecl R_GetPrimaryLightEntityShadowBit(
                int localClientNum,
                unsigned int entnum,
                unsigned int primaryLightIndex)
{
    if ( rgp.world->sunPrimaryLightIndex >= 2
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_primarylights.cpp",
                    279,
                    0,
                    "%s",
                    "rgp.world->sunPrimaryLightIndex == PRIMARY_LIGHT_SUN || rgp.world->sunPrimaryLightIndex == PRIMARY_LIGHT_NONE") )
    {
        __debugbreak();
    }
    if ( (rgp.world->sunPrimaryLightIndex + 1 > primaryLightIndex || primaryLightIndex > rgp.world->primaryLightCount - 1)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_primarylights.cpp",
                    280,
                    0,
                    "primaryLightIndex not in [rgp.world->sunPrimaryLightIndex + 1, rgp.world->primaryLightCount - 1]\n"
                    "\t%i not in [%i, %i]",
                    primaryLightIndex,
                    rgp.world->sunPrimaryLightIndex + 1,
                    rgp.world->primaryLightCount - 1) )
    {
        __debugbreak();
    }
    return primaryLightIndex
             - (rgp.world->sunPrimaryLightIndex
                + 1)
             + (rgp.world->primaryLightCount - (rgp.world->sunPrimaryLightIndex + 1))
             * (entnum + gfxCfg.entCount * localClientNum);
}

void __cdecl R_LinkBoxEntityToPrimaryLights(
                int localClientNum,
                unsigned int entityNum,
                const float *mins,
                const float *maxs)
{
    unsigned int primaryLightIndex; // [esp+40h] [ebp-24h]
    const ComPrimaryLight *light; // [esp+44h] [ebp-20h]
    unsigned int bitIndex; // [esp+48h] [ebp-1Ch]
    float boxHalfSize[3]; // [esp+4Ch] [ebp-18h] BYREF
    float boxMidPoint[3]; // [esp+58h] [ebp-Ch] BYREF

    boxMidPoint[0] = (float)(*mins + *maxs) * 0.5;
    boxMidPoint[1] = (float)(mins[1] + maxs[1]) * 0.5;
    boxMidPoint[2] = (float)(mins[2] + maxs[2]) * 0.5;
    boxHalfSize[0] = boxMidPoint[0] - *mins;
    boxHalfSize[1] = boxMidPoint[1] - mins[1];
    boxHalfSize[2] = boxMidPoint[2] - mins[2];
    for ( primaryLightIndex = rgp.world->sunPrimaryLightIndex + 1;
                primaryLightIndex < rgp.world->primaryLightCount;
                ++primaryLightIndex )
    {
        light = Com_GetPrimaryLight(primaryLightIndex);
        if ( light->type != 2
            && light->type != 3
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_primarylights.cpp",
                        342,
                        0,
                        "%s\n\t(light->type) = %i",
                        "(light->type == GFX_LIGHT_TYPE_SPOT || light->type == GFX_LIGHT_TYPE_OMNI)",
                        light->type) )
        {
            __debugbreak();
        }
        if ( PointToBoxDistSq(light->origin, mins, maxs) < light->radius * light->radius
            && (light->type != 2
             || light->cosHalfFovExpanded < 0.0
             || !CullBoxFromCone(light->origin, light->dir, light->cosHalfFovExpanded, boxMidPoint, boxHalfSize))
            && !R_CullBoxFromLightRegion(&rgp.world->lightRegion[primaryLightIndex], light->origin, boxMidPoint, boxHalfSize) )
        {
            bitIndex = R_GetPrimaryLightEntityShadowBit(localClientNum, entityNum, primaryLightIndex);
            Com_BitSetAssert(rgp.world->primaryLightEntityShadowVis, bitIndex, 0xFFFFFFF);
        }
    }
}

char __cdecl R_CullBoxFromLightRegion(
                const GfxLightRegion *region,
                const float *lightOrigin,
                const float *boxMidPoint,
                const float *boxHalfSize)
{
    unsigned int hullIter; // [esp+78h] [ebp-10h]
    float localBoxMidPoint[3]; // [esp+7Ch] [ebp-Ch] BYREF

    if ( !region->hullCount )
        return 0;
    localBoxMidPoint[0] = *boxMidPoint - *lightOrigin;
    localBoxMidPoint[1] = boxMidPoint[1] - lightOrigin[1];
    localBoxMidPoint[2] = boxMidPoint[2] - lightOrigin[2];
    for ( hullIter = 0; hullIter < region->hullCount; ++hullIter )
    {
        if ( !R_CullBoxFromLightRegionHull(&region->hulls[hullIter], localBoxMidPoint, boxHalfSize) )
            return 0;
    }
    return 1;
}

char __cdecl R_CullBoxFromLightRegionHull(
                const GfxLightRegionHull *hull,
                const float *boxMidPoint,
                const float *boxHalfSize)
{
    float halfSizeOnAxis; // [esp+68h] [ebp-10h]
    float halfSizeOnAxisa; // [esp+68h] [ebp-10h]
    float halfSizeOnAxisb; // [esp+68h] [ebp-10h]
    GfxLightRegionAxis *dir; // [esp+6Ch] [ebp-Ch]
    unsigned int axisIter; // [esp+74h] [ebp-4h]

    if ( fabs(*boxMidPoint - hull->kdopMidPoint[0]) >= (float)(*boxHalfSize + hull->kdopHalfSize[0]) )
        return 1;
    if ( fabs(boxMidPoint[1] - hull->kdopMidPoint[1]) >= (float)(boxHalfSize[1] + hull->kdopHalfSize[1]) )
        return 1;
    if ( fabs(boxMidPoint[2] - hull->kdopMidPoint[2]) >= (float)(boxHalfSize[2] + hull->kdopHalfSize[2]) )
        return 1;
    halfSizeOnAxis = *boxHalfSize + boxHalfSize[1];
    if ( fabs((float)(*boxMidPoint + boxMidPoint[1]) - hull->kdopMidPoint[3]) >= (float)(halfSizeOnAxis + hull->kdopHalfSize[3]) )
        return 1;
    if ( fabs((float)(*boxMidPoint - boxMidPoint[1]) - hull->kdopMidPoint[4]) >= (float)(halfSizeOnAxis + hull->kdopHalfSize[4]) )
        return 1;
    halfSizeOnAxisa = *boxHalfSize + boxHalfSize[2];
    if ( fabs((float)(*boxMidPoint + boxMidPoint[2]) - hull->kdopMidPoint[5]) >= (float)(halfSizeOnAxisa + hull->kdopHalfSize[5]) )
        return 1;
    if ( fabs((float)(*boxMidPoint - boxMidPoint[2]) - hull->kdopMidPoint[6]) >= (float)(halfSizeOnAxisa + hull->kdopHalfSize[6]) )
        return 1;
    halfSizeOnAxisb = boxHalfSize[1] + boxHalfSize[2];
    if ( fabs((float)(boxMidPoint[1] + boxMidPoint[2]) - hull->kdopMidPoint[7]) >= (float)(halfSizeOnAxisb + hull->kdopHalfSize[7]) )
        return 1;
    if ( fabs((float)(boxMidPoint[1] - boxMidPoint[2]) - hull->kdopMidPoint[8]) >= (float)(halfSizeOnAxisb + hull->kdopHalfSize[8]) )
        return 1;
    for ( axisIter = 0; axisIter < hull->axisCount; ++axisIter )
    {
        dir = &hull->axis[axisIter];
        if ( fabs(((((*boxMidPoint * dir->dir[0]) + (boxMidPoint[1] * dir->dir[1]))+ (boxMidPoint[2] * dir->dir[2])) - dir->midPoint)) 
            >= (float)((float)((float)((float)(*boxHalfSize
                                                                                                                            * fabs(dir->dir[0]))
                                                                                                            + (float)(boxHalfSize[1]
                                                                                                                            * fabs(dir->dir[1])))
                                                                                            + (float)(boxHalfSize[2]
                                                                                                            * fabs(dir->dir[2])))
                                                                            + dir->halfSize) )
            return 1;
    }
    return 0;
}

void __cdecl R_LinkDynEntToPrimaryLights(
                unsigned int dynEntId,
                DynEntityDrawType drawType,
                const float *mins,
                const float *maxs)
{
    unsigned int primaryLightIndex; // [esp+2Ch] [ebp-30h]
    const ComPrimaryLight *light; // [esp+30h] [ebp-2Ch]
    unsigned int bitIndex; // [esp+34h] [ebp-28h]
    float boxHalfSize[3]; // [esp+38h] [ebp-24h] BYREF
    unsigned int bestPrimaryLightIndex; // [esp+44h] [ebp-18h]
    float boxMidPoint[3]; // [esp+48h] [ebp-14h] BYREF
    float distSq; // [esp+54h] [ebp-8h]
    float minDistSq; // [esp+58h] [ebp-4h]

    boxMidPoint[0] = (float)(*mins + *maxs) * 0.5;
    boxMidPoint[1] = (float)(mins[1] + maxs[1]) * 0.5;
    boxMidPoint[2] = (float)(mins[2] + maxs[2]) * 0.5;
    boxHalfSize[0] = boxMidPoint[0] - *mins;
    boxHalfSize[1] = boxMidPoint[1] - mins[1];
    boxHalfSize[2] = boxMidPoint[2] - mins[2];
    bestPrimaryLightIndex = 0;
    minDistSq = FLT_MAX;
    for ( primaryLightIndex = rgp.world->sunPrimaryLightIndex + 1;
                primaryLightIndex < rgp.world->primaryLightCount;
                ++primaryLightIndex )
    {
        light = Com_GetPrimaryLight(primaryLightIndex);
        if ( light->type != 2
            && light->type != 3
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_primarylights.cpp",
                        380,
                        0,
                        "%s\n\t(light->type) = %i",
                        "(light->type == GFX_LIGHT_TYPE_SPOT || light->type == GFX_LIGHT_TYPE_OMNI)",
                        light->type) )
        {
            __debugbreak();
        }
        if ( !Com_CullBoxFromPrimaryLight(light, boxMidPoint, boxHalfSize)
            && !R_CullBoxFromLightRegion(&rgp.world->lightRegion[primaryLightIndex], light->origin, boxMidPoint, boxHalfSize) )
        {
            bitIndex = R_GetPrimaryLightDynEntShadowBit(dynEntId, primaryLightIndex);
            Com_BitSetAssert(rgp.world->primaryLightDynEntShadowVis[drawType], bitIndex, 0xFFFFFFF);
            distSq = Vec3DistanceSq(light->origin, boxMidPoint);
            if ( minDistSq > distSq )
            {
                bestPrimaryLightIndex = primaryLightIndex;
                minDistSq = distSq;
            }
        }
    }
    if ( drawType == DYNENT_DRAW_MODEL )
        rgp.world->nonSunPrimaryLightForModelDynEnt[dynEntId] = truncate_cast<unsigned char>(bestPrimaryLightIndex);
}

bool __cdecl Com_CullBoxFromPrimaryLight(
                const ComPrimaryLight *light,
                const float *boxMidPoint,
                const float *boxHalfSize)
{
    float lightRadius; // [esp+10h] [ebp-4h]

    lightRadius = light->radius;
    if ( light->type == 2 && (lightRadius = lightRadius / light->cosHalfFovOuter, light->cosHalfFovExpanded >= 0.0) )
        return CullBoxFromConicSectionOfSphere(
                         light->origin,
                         light->dir,
                         light->cosHalfFovExpanded,
                         lightRadius,
                         boxMidPoint,
                         boxHalfSize);
    else
        return CullBoxFromSphere(light->origin, lightRadius, boxMidPoint, boxHalfSize);
}

unsigned int __cdecl R_GetPrimaryLightDynEntShadowBit(unsigned int entnum, unsigned int primaryLightIndex)
{
    if ( rgp.world->sunPrimaryLightIndex >= 2
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_primarylights.cpp",
                    296,
                    0,
                    "%s",
                    "rgp.world->sunPrimaryLightIndex == PRIMARY_LIGHT_SUN || rgp.world->sunPrimaryLightIndex == PRIMARY_LIGHT_NONE") )
    {
        __debugbreak();
    }
    if ( primaryLightIndex <= rgp.world->sunPrimaryLightIndex
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_primarylights.cpp",
                    297,
                    0,
                    "primaryLightIndex > rgp.world->sunPrimaryLightIndex\n\t%i, %i",
                    primaryLightIndex,
                    rgp.world->sunPrimaryLightIndex) )
    {
        __debugbreak();
    }
    return primaryLightIndex
             - (rgp.world->sunPrimaryLightIndex
                + 1)
             + (rgp.world->primaryLightCount - (rgp.world->sunPrimaryLightIndex + 1)) * entnum;
}

void __cdecl R_UnlinkEntityFromPrimaryLights(int localClientNum, unsigned int entityNum)
{
    unsigned int primaryLightIndex; // [esp+Ch] [ebp-8h]
    unsigned int bitIndex; // [esp+10h] [ebp-4h]

    for ( primaryLightIndex = rgp.world->sunPrimaryLightIndex + 1;
                primaryLightIndex < rgp.world->primaryLightCount;
                ++primaryLightIndex )
    {
        bitIndex = R_GetPrimaryLightEntityShadowBit(localClientNum, entityNum, primaryLightIndex);
        Com_BitClearAssert(rgp.world->primaryLightEntityShadowVis, bitIndex, 0xFFFFFFF);
    }
}

void __cdecl R_UnlinkDynEntFromPrimaryLights(unsigned int dynEntId, DynEntityDrawType drawType)
{
    unsigned int primaryLightIndex; // [esp+Ch] [ebp-8h]
    unsigned int bitIndex; // [esp+10h] [ebp-4h]

    for ( primaryLightIndex = rgp.world->sunPrimaryLightIndex + 1;
                primaryLightIndex < rgp.world->primaryLightCount;
                ++primaryLightIndex )
    {
        bitIndex = R_GetPrimaryLightDynEntShadowBit(dynEntId, primaryLightIndex);
        Com_BitClearAssert(rgp.world->primaryLightDynEntShadowVis[drawType], bitIndex, 0xFFFFFFF);
    }
}

bool __cdecl R_IsEntityVisibleToPrimaryLight(
                int localClientNum,
                unsigned int entityNum,
                unsigned int primaryLightIndex)
{
    unsigned int bitIndex; // [esp+Ch] [ebp-4h]

    bitIndex = R_GetPrimaryLightEntityShadowBit(localClientNum, entityNum, primaryLightIndex);
    return Com_BitCheckAssert(rgp.world->primaryLightEntityShadowVis, bitIndex, 0xFFFFFFF);
}

bool __cdecl R_IsDynEntVisibleToPrimaryLight(
                unsigned int dynEntId,
                DynEntityDrawType drawType,
                unsigned int primaryLightIndex)
{
    unsigned int bitIndex; // [esp+Ch] [ebp-4h]

    bitIndex = R_GetPrimaryLightDynEntShadowBit(dynEntId, primaryLightIndex);
    return Com_BitCheckAssert(rgp.world->primaryLightDynEntShadowVis[drawType], bitIndex, 0xFFFFFFF);
}

int __cdecl R_IsEntityVisibleToAnyShadowedPrimaryLight(const GfxViewInfo *viewInfo, unsigned int entityNum)
{
    unsigned int baseBitIndex; // [esp+0h] [ebp-10h]
    unsigned int relevantPrimaryLightCount; // [esp+4h] [ebp-Ch]
    unsigned int ignoredPrimaryLightCount; // [esp+8h] [ebp-8h]
    unsigned int spotShadowIndex; // [esp+Ch] [ebp-4h]

    ignoredPrimaryLightCount = rgp.world->sunPrimaryLightIndex + 1;
    relevantPrimaryLightCount = rgp.world->primaryLightCount - ignoredPrimaryLightCount;
    baseBitIndex = relevantPrimaryLightCount * (entityNum + gfxCfg.entCount * viewInfo->localClientNum)
                             - ignoredPrimaryLightCount;
    if ( relevantPrimaryLightCount + 1 < frontEndDataOut->spotShadowCount
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_primarylights.cpp",
                    469,
                    0,
                    "relevantPrimaryLightCount + GFX_MAX_EMISSIVE_SPOT_LIGHTS >= frontEndDataOut->spotShadowCount\n\t%i, %i",
                    relevantPrimaryLightCount + 1,
                    frontEndDataOut->spotShadowCount) )
    {
        __debugbreak();
    }
    for ( spotShadowIndex = 0; spotShadowIndex < frontEndDataOut->spotShadowCount; ++spotShadowIndex )
    {
        if ( R_IsEntityVisibleToShadowedPrimaryLight(
                     baseBitIndex,
                     frontEndDataOut->spotShadows[spotShadowIndex].shadowableLightIndex) )
        {
            return 1;
        }
    }
    return 0;
}

bool __cdecl R_IsEntityVisibleToShadowedPrimaryLight(unsigned int baseBitIndex, unsigned int shadowableLightIndex)
{
    return R_IsPrimaryLight(shadowableLightIndex)
            && Com_BitCheckAssert(rgp.world->primaryLightEntityShadowVis, shadowableLightIndex + baseBitIndex, 0xFFFFFFF);
}

int __cdecl R_IsDynEntVisibleToAnyShadowedPrimaryLight(
                const GfxViewInfo *viewInfo,
                unsigned int dynEntId,
                DynEntityDrawType drawType)
{
    unsigned int relevantPrimaryLightCount; // [esp+4h] [ebp-Ch]
    unsigned int ignoredPrimaryLightCount; // [esp+8h] [ebp-8h]
    unsigned int spotShadowIndex; // [esp+Ch] [ebp-4h]

    ignoredPrimaryLightCount = rgp.world->sunPrimaryLightIndex + 1;
    relevantPrimaryLightCount = rgp.world->primaryLightCount - ignoredPrimaryLightCount;
    if ( relevantPrimaryLightCount + 1 < frontEndDataOut->spotShadowCount
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_primarylights.cpp",
                    507,
                    0,
                    "relevantPrimaryLightCount + GFX_MAX_EMISSIVE_SPOT_LIGHTS >= frontEndDataOut->spotShadowCount\n\t%i, %i",
                    relevantPrimaryLightCount + 1,
                    frontEndDataOut->spotShadowCount) )
    {
        __debugbreak();
    }
    for ( spotShadowIndex = 0; spotShadowIndex < frontEndDataOut->spotShadowCount; ++spotShadowIndex )
    {
        if ( R_IsDynEntVisibleToShadowedPrimaryLight(
                     relevantPrimaryLightCount * dynEntId - ignoredPrimaryLightCount,
                     drawType,
                     frontEndDataOut->spotShadows[spotShadowIndex].shadowableLightIndex) )
        {
            return 1;
        }
    }
    return 0;
}

bool __cdecl R_IsDynEntVisibleToShadowedPrimaryLight(
                unsigned int baseBitIndex,
                DynEntityDrawType drawType,
                unsigned int shadowableLightIndex)
{
    return R_IsPrimaryLight(shadowableLightIndex)
            && Com_BitCheckAssert(
                     rgp.world->primaryLightDynEntShadowVis[drawType],
                     shadowableLightIndex + baseBitIndex,
                     0xFFFFFFF);
}

unsigned int __cdecl R_GetNonSunPrimaryLightForBox(
                const GfxViewInfo *viewInfo,
                const float *boxMidPoint,
                const float *boxHalfSize)
{
    int bestPriority; // [esp+8Ch] [ebp-10h]
    unsigned int primaryLightIndex; // [esp+90h] [ebp-Ch]
    unsigned int bestIndex; // [esp+94h] [ebp-8h]
    const ComPrimaryLight *light; // [esp+98h] [ebp-4h]

    bestIndex = 0;
    bestPriority = 0x80000000;
    for ( primaryLightIndex = rgp.world->sunPrimaryLightIndex + 1;
                primaryLightIndex < rgp.world->primaryLightCount;
                ++primaryLightIndex )
    {
        light = Com_GetPrimaryLight(primaryLightIndex);
        if ( light->type != 2
            && light->type != 3
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_primarylights.cpp",
                        530,
                        0,
                        "%s\n\t(light->type) = %i",
                        "(light->type == GFX_LIGHT_TYPE_SPOT || light->type == GFX_LIGHT_TYPE_OMNI)",
                        light->type) )
        {
            __debugbreak();
        }
        if ( !Com_CullBoxFromPrimaryLight(light, boxMidPoint, boxHalfSize)
            && !R_CullBoxFromLightRegion(&rgp.world->lightRegion[primaryLightIndex], light->origin, boxMidPoint, boxHalfSize)
            && light->priority > bestPriority )
        {
            bestIndex = primaryLightIndex;
            bestPriority = light->priority;
        }
    }
    return bestIndex;
}

unsigned int __cdecl R_GetNonSunPrimaryLightForSphere(const GfxViewInfo *viewInfo, const float *origin, float radius)
{
    int bestPriority; // [esp+80h] [ebp-10h]
    unsigned int primaryLightIndex; // [esp+84h] [ebp-Ch]
    unsigned int bestIndex; // [esp+88h] [ebp-8h]
    const ComPrimaryLight *light; // [esp+8Ch] [ebp-4h]

    bestIndex = 0;
    bestPriority = 0x80000000;
    for ( primaryLightIndex = rgp.world->sunPrimaryLightIndex + 1;
                primaryLightIndex < rgp.world->primaryLightCount;
                ++primaryLightIndex )
    {
        light = Com_GetPrimaryLight(primaryLightIndex);
        if ( light->type != 2
            && light->type != 3
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_primarylights.cpp",
                        557,
                        0,
                        "%s\n\t(light->type) = %i",
                        "(light->type == GFX_LIGHT_TYPE_SPOT || light->type == GFX_LIGHT_TYPE_OMNI)",
                        light->type) )
        {
            __debugbreak();
        }
        if ( !Com_CullSphereFromPrimaryLight(light, origin, radius)
            && !R_CullSphereFromLightRegion(&rgp.world->lightRegion[primaryLightIndex], light->origin, origin, radius)
            && light->priority > bestPriority )
        {
            bestIndex = primaryLightIndex;
            bestPriority = light->priority;
        }
    }
    return bestIndex;
}

char __cdecl R_CullSphereFromLightRegion(
                const GfxLightRegion *region,
                const float *lightOrigin,
                const float *origin,
                float radius)
{
    unsigned int hullIter; // [esp+60h] [ebp-10h]
    float localOrigin[3]; // [esp+64h] [ebp-Ch] BYREF

    if ( !region->hullCount )
        return 0;
    localOrigin[0] = *origin - *lightOrigin;
    localOrigin[1] = origin[1] - lightOrigin[1];
    localOrigin[2] = origin[2] - lightOrigin[2];
    for ( hullIter = 0; hullIter < region->hullCount; ++hullIter )
    {
        if ( !R_CullSphereFromLightRegionHull(&region->hulls[hullIter], localOrigin, radius) )
            return 0;
    }
    return 1;
}

char __cdecl R_CullSphereFromLightRegionHull(const GfxLightRegionHull *hull, const float *origin, float radius)
{
    unsigned int axisIter; // [esp+58h] [ebp-4h]

    if ( fabs(*origin - hull->kdopMidPoint[0]) >= (float)(radius + hull->kdopHalfSize[0]) )
        return 1;
    if ( fabs(origin[1] - hull->kdopMidPoint[1]) >= (float)(radius + hull->kdopHalfSize[1]) )
        return 1;
    if ( fabs(origin[2] - hull->kdopMidPoint[2]) >= (float)(radius + hull->kdopHalfSize[2]) )
        return 1;
    if ( fabs((float)(*origin + origin[1]) - hull->kdopMidPoint[3]) >= (float)(radius + hull->kdopHalfSize[3]) )
        return 1;
    if ( fabs((float)(*origin - origin[1]) - hull->kdopMidPoint[4]) >= (float)(radius + hull->kdopHalfSize[4]) )
        return 1;
    if ( fabs((float)(*origin + origin[2]) - hull->kdopMidPoint[5]) >= (float)(radius + hull->kdopHalfSize[5]) )
        return 1;
    if ( fabs((float)(*origin - origin[2]) - hull->kdopMidPoint[6]) >= (float)(radius + hull->kdopHalfSize[6]) )
        return 1;
    if ( fabs((float)(origin[1] + origin[2]) - hull->kdopMidPoint[7]) >= (float)(radius + hull->kdopHalfSize[7]) )
        return 1;
    if ( fabs((float)(origin[1] - origin[2]) - hull->kdopMidPoint[8]) >= (float)(radius + hull->kdopHalfSize[8]) )
        return 1;
    for ( axisIter = 0; axisIter < hull->axisCount; ++axisIter )
    {
        if ( fabs(((((*origin * hull->axis[axisIter].dir[0]) + (float)(origin[1] * hull->axis[axisIter].dir[1])) + (float)(origin[2] * hull->axis[axisIter].dir[2])) - hull->axis[axisIter].midPoint)) >= (float)(radius + hull->axis[axisIter].halfSize) )
            return 1;
    }
    return 0;
}

bool __cdecl Com_CullSphereFromPrimaryLight(const ComPrimaryLight *light, const float *origin, float radius)
{
    float lightRadius; // [esp+18h] [ebp-4h]

    lightRadius = light->radius;
    if ( light->type == 2 )
        lightRadius = lightRadius / light->cosHalfFovOuter;
    if ( Vec3DistanceSq(light->origin, origin) >= (radius + lightRadius) * (radius + lightRadius) )
        return 1;
    if ( light->type == 2 && light->cosHalfFovExpanded >= 0.0 )
        return CullSphereFromCone(light->origin, light->dir, light->cosHalfFovExpanded, origin, radius);
    return 0;
}

void __cdecl R_ShowPrimaryLightDebugLine(const float *lineStartPosition, unsigned int primaryLightIndex, int context)
{
    const ComPrimaryLight *L; // [esp+4h] [ebp-3Ch]
    float samplePos2Eye_4; // [esp+Ch] [ebp-34h]
    float samplePos2Eye_8; // [esp+10h] [ebp-30h]
    float mins[3]; // [esp+14h] [ebp-2Ch] BYREF
    float ext[3]; // [esp+20h] [ebp-20h]
    float maxs[3]; // [esp+2Ch] [ebp-14h] BYREF
    const float (*color)[4]; // [esp+38h] [ebp-8h]
    bool drawLine; // [esp+3Fh] [ebp-1h]

    if ( context == r_debugShowPrimaryLightLines->current.integer )
    {
        samplePos2Eye_4 = frontEndDataOut->viewParms[0].origin[1] - lineStartPosition[1];
        samplePos2Eye_8 = frontEndDataOut->viewParms[0].origin[2] - lineStartPosition[2];
        if ( (float)((float)((float)((float)(frontEndDataOut->viewParms[0].origin[0] - *lineStartPosition)
                                                             * (float)(frontEndDataOut->viewParms[0].origin[0] - *lineStartPosition))
                                             + (float)(samplePos2Eye_4 * samplePos2Eye_4))
                             + (float)(samplePos2Eye_8 * samplePos2Eye_8)) <= 810000.0 )
        {
            drawLine = 0;
            L = Com_GetPrimaryLight(primaryLightIndex);
            if ( primaryLightIndex )
            {
                if ( primaryLightIndex == 1 )
                {
                    color = (const float (*)[4])colorYellow;
                }
                else if ( primaryLightIndex == 255 )
                {
                    color = (const float (*)[4])colorGreen;
                }
                else
                {
                    color = (const float (*)[4])colorMdGrey;
                    drawLine = 1;
                }
            }
            else
            {
                color = (const float (*)[4])colorRed;
            }
            ext[0] = 0.75f;
            ext[1] = 0.75f;
            ext[2] = 0.75f;
            maxs[0] = *lineStartPosition + 0.75;
            maxs[1] = lineStartPosition[1] + 0.75;
            maxs[2] = lineStartPosition[2] + 0.75;
            mins[0] = *lineStartPosition - 0.75;
            mins[1] = lineStartPosition[1] - 0.75;
            mins[2] = lineStartPosition[2] - 0.75;
            R_AddDebugBox(&frontEndDataOut->debugGlobals, mins, maxs, (const float *)color);
            if ( drawLine )
                R_AddDebugLine(&frontEndDataOut->debugGlobals, lineStartPosition, L->origin, (const float *)color, 1);
        }
    }
}

