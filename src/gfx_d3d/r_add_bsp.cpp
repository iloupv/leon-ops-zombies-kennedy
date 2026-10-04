#include "r_add_bsp.h"
#include "r_add_cmdbuf.h"
#include "r_dvars.h"
#include <qcommon/threads.h>
#include "r_light.h"
#include "r_add_staticmodel.h"
#include "r_pretess.h"
#include "r_skybox.h"
#include <physics/rope.h>
#include "r_drawsurf.h"
#include "r_world_lod.h"

void __cdecl R_InitBspDrawSurf(GfxSModelDrawSurfLightingData *surfData)
{
    R_InitDelayedCmdBuf(&surfData->delayedCmdBuf);
}

void __cdecl R_AddBspDrawSurfs(
                GfxDrawSurf drawSurf,
                unsigned __int8 *list,
                unsigned int count,
                GfxBspDrawSurfData *surfData)
{
    bool v4; // [esp+13h] [ebp-1h]

    iassert(drawSurf.fields.surfType == SF_TRIANGLES);
    
    v4 = Sys_QueryD3DDeviceOKEvent() && r_pretess->current.enabled;
    if ( !v4 || !R_PreTessBspDrawSurfs(drawSurf, (const unsigned __int16 *)list, count, surfData) )
    {
        if ( R_AllocDrawSurf(&surfData->delayedCmdBuf, drawSurf, &surfData->drawSurfList, ((count + 1) >> 1) + 1) )
        {
            R_WritePrimDrawSurfInt(&surfData->delayedCmdBuf, count);
            R_WritePrimDrawSurfData(&surfData->delayedCmdBuf, list, (count + 1) >> 1);
        }
    }
}

char __cdecl R_PreTessBspDrawSurfs(
                GfxDrawSurf drawSurf,
                const unsigned __int16 *list,
                unsigned int count,
                GfxBspDrawSurfData *surfData)
{
    unsigned int simplifiedCount; // [esp+8h] [ebp-230h]
    unsigned __int16 surfIndex; // [esp+Ch] [ebp-22Ch]
    const GfxSurface *tris; // [esp+10h] [ebp-228h]
    unsigned int copyIndex; // [esp+14h] [ebp-224h]
    GfxBspPreTessDrawSurf simplifiedList[128]; // [esp+18h] [ebp-220h] BYREF
    unsigned int lmapIndex; // [esp+218h] [ebp-20h]
    unsigned __int16 *preTessIndices; // [esp+21Ch] [ebp-1Ch]
    unsigned int firstIndex; // [esp+220h] [ebp-18h]
    unsigned int triCount; // [esp+224h] [ebp-14h]
    unsigned int reflectionProbeIndex; // [esp+228h] [ebp-10h]
    const GfxSurface *surf; // [esp+22Ch] [ebp-Ch]
    unsigned int surfIter; // [esp+230h] [ebp-8h]
    int baseVertex; // [esp+234h] [ebp-4h]

    triCount = 0;
    for ( surfIter = 0; surfIter < count; ++surfIter )
    {
        bcassert(list[surfIter], rgp.world->surfaceCount);
        triCount += rgp.world->dpvs.surfaces[list[surfIter]].tris.triCount;
    }

    preTessIndices = R_AllocPreTessIndices(3 * triCount);

    if ( !preTessIndices )
        return 0;

    copyIndex = 0;
    simplifiedCount = 0;
    baseVertex = 0x7FFFFFFF;
    lmapIndex = 31;
    reflectionProbeIndex = 255;
    for ( surfIter = 0; surfIter < count; ++surfIter )
    {
        surfIndex = list[surfIter];
        surf = &rgp.world->dpvs.surfaces[surfIndex];
        tris = surf;
        if ( baseVertex != surf->tris.firstVertex
            || lmapIndex != surf->lightmapIndex
            || reflectionProbeIndex != surf->reflectionProbeIndex )
        {
            baseVertex = surf->tris.firstVertex;
            lmapIndex = surf->lightmapIndex;
            reflectionProbeIndex = surf->reflectionProbeIndex;
            simplifiedList[simplifiedCount].baseSurfIndex = surfIndex;
            simplifiedList[simplifiedCount++].totalTriCount = 0;
        }
        Com_Memcpy(&preTessIndices[copyIndex], &rgp.world->draw.indices[tris->tris.baseIndex], 6 * tris->tris.triCount);
        copyIndex += 3 * tris->tris.triCount;
        iassert(simplifiedCount); // kcod4

        //*((_WORD *)&copyIndex + 2 * simplifiedCount + 1) = truncate_cast<unsigned short>(tris->tris.triCount + *((unsigned __int16 *)&copyIndex + 2 * simplifiedCount + 1));
        simplifiedList[simplifiedCount - 1].totalTriCount = truncate_cast<unsigned short>(tris->tris.triCount + simplifiedList[simplifiedCount - 1].totalTriCount);
    }

    HIDWORD(drawSurf.packed) = HIDWORD(drawSurf.packed) & 0xFF87FFFF | 0x80000;

    if ( R_AllocDrawSurf(&surfData->delayedCmdBuf, drawSurf, &surfData->drawSurfList, simplifiedCount + 2) )
    {
        firstIndex = preTessIndices - gfxBuf.preTessIndexBuffer->indices;
        bcassert(firstIndex, R_MAX_PRETESS_INDICES);

        R_WritePrimDrawSurfInt(&surfData->delayedCmdBuf, simplifiedCount);
        R_WritePrimDrawSurfInt(&surfData->delayedCmdBuf, firstIndex);
        R_WritePrimDrawSurfData(&surfData->delayedCmdBuf, (unsigned __int8 *)simplifiedList, simplifiedCount);
    }
    return 1;
}

void __cdecl R_AddAllBspDrawSurfacesCamera(int localClientNum, const GfxLight *visibleLights, int visibleLightCount)
{
    R_AddAllBspDrawSurfacesRangeCamera(
        g_worldDpvs->litSurfsBegin,
        g_worldDpvs->litSurfsEnd,
        0,
        0x2000u,
        localClientNum,
        visibleLights,
        visibleLightCount);
    R_AddAllBspDrawSurfacesRangeCamera(
        g_worldDpvs->decalSurfsBegin,
        g_worldDpvs->decalSurfsEnd,
        3u,
        0x200u,
        localClientNum,
        visibleLights,
        visibleLightCount);
}

void __cdecl R_AddAllBspDrawSurfacesRangeCamera(
                unsigned int beginSurface,
                unsigned int endSurface,
                unsigned int stage,
                unsigned int maxDrawSurfCount,
                int localClientNum,
                const GfxLight *visibleLights,
                int visibleLightCount)
{
    int packed_high; // ecx
    unsigned __int64 v8; // rax
    unsigned __int64 v9; // rax
    unsigned __int64 v10; // rax
    const GfxLight *light; // [esp+1Ch] [ebp-170h]
    int visLightIndex; // [esp+20h] [ebp-16Ch]
    unsigned int visLightsMask; // [esp+24h] [ebp-168h]
    int fade; // [esp+28h] [ebp-164h] BYREF
    unsigned __int8 surfaceFlags; // [esp+2Fh] [ebp-15Dh]
    const GfxSurface *surface; // [esp+30h] [ebp-15Ch]
    unsigned __int16 triSurfList[128]; // [esp+34h] [ebp-158h] BYREF
    int debugFastSunShadow; // [esp+13Ch] [ebp-50h]
    unsigned int *surfaceCastsSunShadow; // [esp+140h] [ebp-4Ch]
    GfxDrawSurf drawSurf; // [esp+144h] [ebp-48h]
    GfxDrawSurf prevDrawSurf; // [esp+14Ch] [ebp-40h]
    unsigned int sortedSurfIndex; // [esp+158h] [ebp-34h]
    unsigned __int8 *surfaceVisData; // [esp+15Ch] [ebp-30h]
    unsigned int triSurfCount; // [esp+160h] [ebp-2Ch]
    GfxDrawSurf *surfaceMaterials; // [esp+164h] [ebp-28h]
    float (*dynamicSpotLightPlanes)[4]; // [esp+168h] [ebp-24h]
    GfxBspDrawSurfData surfData; // [esp+16Ch] [ebp-20h] BYREF
    bool hasSkyBoxModel; // [esp+18Bh] [ebp-1h]

    iassert(rgp.world);
    bcassert(beginSurface, rgp.world->models[0].surfaceCount + 1);
    bcassert(endSurface, rgp.world->models[0].surfaceCount + 1);


    surfaceVisData = rgp.world->dpvs.surfaceVisData[0];
    dynamicSpotLightPlanes = scene.dynamicSpotLightPlanes;
    R_InitBspDrawSurf((GfxSModelDrawSurfLightingData *)&surfData);
    surfData.drawSurfList.current = scene.drawSurfs[stage];

    iassert((int)maxDrawSurfCount == scene.maxDrawSurfCount[stage]);

    surfaceMaterials = rgp.world->dpvs.surfaceMaterials;
    surfData.drawSurfList.end = &surfData.drawSurfList.current[maxDrawSurfCount];
    prevDrawSurf.packed = -1;
    triSurfCount = 0;
    debugFastSunShadow = sm_debugFastSunShadow->current.color[0];
    surfaceCastsSunShadow = rgp.world->dpvs.surfaceCastsSunShadow;
    hasSkyBoxModel = R_MapGetSkyboxModel() != 0;
    for ( sortedSurfIndex = beginSurface; sortedSurfIndex < endSurface; ++sortedSurfIndex )
    {
        if ( surfaceVisData[sortedSurfIndex]
            && (!debugFastSunShadow || (surfaceCastsSunShadow[sortedSurfIndex >> 5] & (1 << (sortedSurfIndex & 0x1F))) != 0) )
        {
            surfaceFlags = GetSurfaceFlags(sortedSurfIndex);
            if ( (surfaceFlags & 8) == 0 )
            {
                //packed_high = HIDWORD(surfaceMaterials[sortedSurfIndex].packed);
                drawSurf.packed = surfaceMaterials[sortedSurfIndex].packed;
                //HIDWORD(drawSurf.packed) = packed_high;
                if ( !hasSkyBoxModel || (surfaceFlags & 4) == 0 )
                {
                    fade = 0;
                    if ( R_WorldLod_GetFadeForDrawSurf(sortedSurfIndex, &fade, localClientNum) )
                    {
                        visLightsMask = 0;
                        surface = &rgp.world->dpvs.surfaces[sortedSurfIndex];
                        for ( visLightIndex = 0; visLightIndex < visibleLightCount; ++visLightIndex )
                        {
                            light = &visibleLights[visLightIndex];
                            if ( light->radius * light->radius >= PointToBoxDistSq(
                                                                                                            light->origin,
                                                                                                            surface->bounds[0],
                                                                                                            surface->bounds[1]) )
                            {
                                if ( light->type == 3 )
                                {
                                    visLightsMask |= 1u;
                                }
                                else if ( BoxInPlanes(dynamicSpotLightPlanes, 6u, surface->bounds[0], surface->bounds[1]) )
                                {
                                    visLightsMask |= 2u;
                                }
                            }
                        }
                        v8 = (unsigned __int64)(visLightsMask & 1) << 29;
                        *(unsigned int *)&drawSurf.fields = v8 | *(unsigned int *)&drawSurf.fields & 0xDFFFFFFF;
                        HIDWORD(drawSurf.packed) |= HIDWORD(v8);
                        v9 = (unsigned __int64)((visLightsMask & 2) != 0) << 30;
                        *(unsigned int *)&drawSurf.fields = v9 | *(unsigned int *)&drawSurf.fields & 0xBFFFFFFF;
                        HIDWORD(drawSurf.packed) |= HIDWORD(v9);
                        v10 = (unsigned __int64)(fade & 0xF) << 16;
                        *(unsigned int *)&drawSurf.fields = v10 | *(unsigned int *)&drawSurf.fields & 0xFFF0FFFF;
                        HIDWORD(drawSurf.packed) |= HIDWORD(v10);
                        if ( fade > 0 )
                        {
                            HIDWORD(drawSurf.packed) = HIDWORD(drawSurf.packed) & 0xFE7FFFFF | 0x1000000;
                            HIDWORD(drawSurf.packed) = HIDWORD(drawSurf.packed) & 0x3FFFFFF | 0xAC000000;
                        }
                        if ( drawSurf.packed != prevDrawSurf.packed )
                        {
                            if ( triSurfCount )
                                R_AddBspDrawSurfs(prevDrawSurf, (unsigned __int8 *)triSurfList, triSurfCount, &surfData);
                            Com_BitSetAssert(scene.shadowableLightIsUsed, drawSurf.fields.primaryLightIndex, 128);
                            prevDrawSurf.fields = drawSurf.fields;
                            triSurfCount = 0;
                        }
                        triSurfList[triSurfCount] = sortedSurfIndex;
                        if ( triSurfList[triSurfCount] != sortedSurfIndex
                            && !Assert_MyHandler(
                                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_add_bsp.cpp",
                                        526,
                                        0,
                                        "triSurfList[triSurfCount] == sortedSurfIndex\n\t%i, %i",
                                        triSurfList[triSurfCount],
                                        sortedSurfIndex) )
                        {
                            __debugbreak();
                        }
                        if ( ++triSurfCount >= 0x80 )
                        {
                            R_AddBspDrawSurfs(drawSurf, (unsigned __int8 *)triSurfList, triSurfCount, &surfData);
                            triSurfCount = 0;
                        }
                    }
                }
            }
        }
    }
    if ( triSurfCount )
        R_AddBspDrawSurfs(prevDrawSurf, (unsigned __int8 *)triSurfList, triSurfCount, &surfData);
    R_EndCmdBuf(&surfData.delayedCmdBuf);
    scene.drawSurfCount[stage] = surfData.drawSurfList.current - scene.drawSurfs[stage];
    R_SortDrawSurfs(scene.drawSurfs[stage], scene.drawSurfCount[stage]);
}

unsigned __int8 __cdecl GetSurfaceFlags(unsigned int surfIndex)
{
    return g_worldDpvs->surfaces[surfIndex].flags;
}

char __cdecl R_WorldLod_GetFadeForDrawSurf(int surface, int *fade, int localClientNum)
{
    int retVal; // [esp+0h] [ebp-4h]

    retVal = R_WorldLod_GetLodVal(surface, localClientNum);
    if ( retVal )
    {
        if ( retVal == 255 )
        {
            *fade = 0;
            return 1;
        }
        else
        {
            *fade = retVal >> 4;
            if ( !*fade )
                *fade = 1;
            return 1;
        }
    }
    else
    {
        *fade = 0;
        return 0;
    }
}

int __cdecl R_WorldLod_GetLodVal(int surfIndex, int localClientNum)
{
    return s_lodVals[surfIndex + rgp.world->surfaceCount * localClientNum];
}

void __cdecl R_AddAllBspDrawSurfacesCameraNonlit(
                unsigned int beginSurface,
                unsigned int endSurface,
                unsigned int stage)
{
    int packed_high; // ecx
    unsigned __int16 triSurfList[128]; // [esp+0h] [ebp-148h] BYREF
    GfxDrawSurf drawSurf; // [esp+108h] [ebp-40h]
    GfxDrawSurf prevDrawSurf; // [esp+110h] [ebp-38h]
    unsigned int sortedSurfIndex; // [esp+118h] [ebp-30h]
    unsigned __int8 *surfaceVisData; // [esp+11Ch] [ebp-2Ch]
    unsigned int triSurfCount; // [esp+120h] [ebp-28h]
    GfxDrawSurf *surfaceMaterials; // [esp+124h] [ebp-24h]
    GfxBspDrawSurfData surfData; // [esp+128h] [ebp-20h] BYREF
    int drawSurfCount; // [esp+144h] [ebp-4h]

    if ( !rgp.world
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_add_bsp.cpp", 643, 0, "%s", "rgp.world") )
    {
        __debugbreak();
    }
    surfaceVisData = rgp.world->dpvs.surfaceVisData[0];
    surfaceMaterials = rgp.world->dpvs.surfaceMaterials;
    R_InitBspDrawSurf((GfxSModelDrawSurfLightingData *)&surfData);
    surfData.drawSurfList.current = scene.drawSurfs[stage];
    surfData.drawSurfList.end = &scene.drawSurfs[stage][scene.maxDrawSurfCount[stage]];
    prevDrawSurf.packed = -1;
    triSurfCount = 0;
    for ( sortedSurfIndex = beginSurface; sortedSurfIndex < endSurface; ++sortedSurfIndex )
    {
        if ( surfaceVisData[sortedSurfIndex] )
        {
            packed_high = HIDWORD(surfaceMaterials[sortedSurfIndex].packed);
            drawSurf.packed = surfaceMaterials[sortedSurfIndex].packed;
            HIDWORD(drawSurf.packed) = packed_high;
            if ( drawSurf.packed != prevDrawSurf.packed )
            {
                if ( triSurfCount )
                    R_AddBspDrawSurfs(prevDrawSurf, (unsigned __int8 *)triSurfList, triSurfCount, &surfData);
                prevDrawSurf.fields = drawSurf.fields;
                triSurfCount = 0;
            }
            triSurfList[triSurfCount] = sortedSurfIndex;
            if ( triSurfList[triSurfCount] != sortedSurfIndex
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_add_bsp.cpp",
                            676,
                            0,
                            "triSurfList[triSurfCount] == sortedSurfIndex\n\t%i, %i",
                            triSurfList[triSurfCount],
                            sortedSurfIndex) )
            {
                __debugbreak();
            }
            if ( ++triSurfCount >= 0x80 )
            {
                R_AddBspDrawSurfs(drawSurf, (unsigned __int8 *)triSurfList, triSurfCount, &surfData);
                triSurfCount = 0;
            }
        }
    }
    if ( triSurfCount )
        R_AddBspDrawSurfs(prevDrawSurf, (unsigned __int8 *)triSurfList, triSurfCount, &surfData);
    R_EndCmdBuf(&surfData.delayedCmdBuf);
    drawSurfCount = surfData.drawSurfList.current - scene.drawSurfs[stage];
    scene.drawSurfCount[stage] = drawSurfCount;
}

void __cdecl R_AddAllBspDrawSurfacesSunShadow(int localClientNum)
{
    if ( rgp.world->dpvs.litSurfsEnd != rgp.world->dpvs.decalSurfsBegin
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_add_bsp.cpp",
                    898,
                    0,
                    "%s",
                    "rgp.world->dpvs.litSurfsEnd == rgp.world->dpvs.decalSurfsBegin") )
    {
        __debugbreak();
    }
    if ( rgp.world->dpvs.decalSurfsEnd != rgp.world->dpvs.emissiveSurfsBegin
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_add_bsp.cpp",
                    899,
                    0,
                    "%s",
                    "rgp.world->dpvs.decalSurfsEnd == rgp.world->dpvs.emissiveSurfsBegin") )
    {
        __debugbreak();
    }
    R_AddAllBspDrawSurfacesRangeSunShadow(
        0,
        rgp.world->dpvs.litSurfsBegin,
        rgp.world->dpvs.emissiveSurfsEnd,
        0x1000u,
        localClientNum);
    R_AddAllBspDrawSurfacesRangeSunShadow(
        1u,
        rgp.world->dpvs.litSurfsBegin,
        rgp.world->dpvs.emissiveSurfsEnd,
        0x2000u,
        localClientNum);
}

void __cdecl R_AddAllBspDrawSurfacesRangeSunShadow(
                unsigned int partitionIndex,
                unsigned int beginSurface,
                unsigned int endSurface,
                unsigned int maxDrawSurfCount,
                int localClientNum)
{
    int v5; // edx
    int packed_high; // ecx
    unsigned __int16 triSurfList[128]; // [esp+Ch] [ebp-158h] BYREF
    unsigned int *surfaceCastsSunShadow; // [esp+110h] [ebp-54h]
    GfxDrawSurf drawSurf; // [esp+114h] [ebp-50h]
    GfxDrawSurf prevDrawSurf; // [esp+11Ch] [ebp-48h]
    unsigned int stage; // [esp+128h] [ebp-3Ch]
    unsigned int sortedSurfIndex; // [esp+12Ch] [ebp-38h]
    unsigned __int8 *surfaceVisData; // [esp+130h] [ebp-34h]
    int hasApproxSunDirChanged; // [esp+134h] [ebp-30h]
    unsigned int triSurfCount; // [esp+138h] [ebp-2Ch]
    GfxDrawSurf *surfaceMaterials; // [esp+13Ch] [ebp-28h]
    int fastSunShadow; // [esp+140h] [ebp-24h]
    GfxBspDrawSurfData surfData; // [esp+144h] [ebp-20h] BYREF
    int skipMaterial; // [esp+160h] [ebp-4h]

    if ( !rgp.world
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_add_bsp.cpp", 724, 0, "%s", "rgp.world") )
    {
        __debugbreak();
    }
    if ( beginSurface >= rgp.world->models->surfaceCount + 1
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_add_bsp.cpp",
                    725,
                    0,
                    "beginSurface doesn't index rgp.world->models[0].surfaceCount + 1\n\t%i not in [0, %i)",
                    beginSurface,
                    rgp.world->models->surfaceCount + 1) )
    {
        __debugbreak();
    }
    if ( endSurface >= rgp.world->models->surfaceCount + 1
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_add_bsp.cpp",
                    726,
                    0,
                    "endSurface doesn't index rgp.world->models[0].surfaceCount + 1\n\t%i not in [0, %i)",
                    endSurface,
                    rgp.world->models->surfaceCount + 1) )
    {
        __debugbreak();
    }
    surfaceVisData = rgp.world->dpvs.surfaceVisData[partitionIndex + 1];
    R_InitBspDrawSurf((GfxSModelDrawSurfLightingData *)&surfData);
    stage = 4 * partitionIndex + 19;
    surfData.drawSurfList.current = scene.drawSurfs[stage];
    if ( maxDrawSurfCount != scene.maxDrawSurfCount[stage]
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_add_bsp.cpp",
                    743,
                    0,
                    "%s",
                    "(int)maxDrawSurfCount == scene.maxDrawSurfCount[stage]") )
    {
        __debugbreak();
    }
    surfaceMaterials = rgp.world->dpvs.surfaceMaterials;
    surfData.drawSurfList.end = &surfData.drawSurfList.current[maxDrawSurfCount];
    prevDrawSurf.packed = -1;
    skipMaterial = 0;
    triSurfCount = 0;
    surfaceCastsSunShadow = rgp.world->dpvs.surfaceCastsSunShadow;
    fastSunShadow = sm_fastSunShadow->current.color[0];
    hasApproxSunDirChanged = frontEndDataOut->hasApproxSunDirChanged;
    if ( !hasApproxSunDirChanged && fastSunShadow )
    {
        for ( sortedSurfIndex = beginSurface; sortedSurfIndex < endSurface; ++sortedSurfIndex )
        {
            if ( surfaceVisData[sortedSurfIndex]
                && (surfaceCastsSunShadow[sortedSurfIndex >> 5] & (1 << (sortedSurfIndex & 0x1F))) != 0
                && R_WorldLod_GetLodVal(sortedSurfIndex, localClientNum) == 255 )
            {
                //packed_high = HIDWORD(surfaceMaterials[sortedSurfIndex].packed);
                //*(unsigned int *)&drawSurf.fields = surfaceMaterials[sortedSurfIndex].fields;
                drawSurf.packed = surfaceMaterials[sortedSurfIndex].packed;
                //HIDWORD(drawSurf.packed) = packed_high;
                if ( drawSurf.packed != prevDrawSurf.packed )
                {
                    if ( triSurfCount )
                        R_AddBspDrawSurfs(prevDrawSurf, (unsigned char*)triSurfList, triSurfCount, &surfData);
                    prevDrawSurf.fields = drawSurf.fields;
                    triSurfCount = 0;
                }
                triSurfList[triSurfCount] = sortedSurfIndex;
                if ( triSurfList[triSurfCount] != sortedSurfIndex
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_add_bsp.cpp",
                                854,
                                0,
                                "triSurfList[triSurfCount] == sortedSurfIndex\n\t%i, %i",
                                triSurfList[triSurfCount],
                                sortedSurfIndex) )
                {
                    __debugbreak();
                }
                if ( ++triSurfCount >= 0x80 )
                {
                    R_AddBspDrawSurfs(drawSurf, (unsigned char *)triSurfList, triSurfCount, &surfData);
                    triSurfCount = 0;
                }
            }
        }
    }
    else
    {
        for ( sortedSurfIndex = beginSurface; sortedSurfIndex < endSurface; ++sortedSurfIndex )
        {
            if ( surfaceVisData[sortedSurfIndex] && R_WorldLod_GetLodVal(sortedSurfIndex, localClientNum) == 255 )
            {
                //v5 = HIDWORD(surfaceMaterials[sortedSurfIndex].packed);
                //*(unsigned int *)&drawSurf.fields = surfaceMaterials[sortedSurfIndex].fields;
                drawSurf.packed = surfaceMaterials[sortedSurfIndex].packed;
                //HIDWORD(drawSurf.packed) = v5;
                if ( drawSurf.packed != prevDrawSurf.packed )
                {
                    if ( triSurfCount )
                    {
                        if ( skipMaterial
                            && !Assert_MyHandler(
                                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_add_bsp.cpp",
                                        789,
                                        0,
                                        "%s",
                                        "!skipMaterial") )
                        {
                            __debugbreak();
                        }
                        R_AddBspDrawSurfs(prevDrawSurf, (unsigned char *)triSurfList, triSurfCount, &surfData);
                    }
                    prevDrawSurf.fields = drawSurf.fields;
                    skipMaterial = (drawSurf.packed & 0x100000) == 0;
                    triSurfCount = 0;
                }
                if ( !skipMaterial )
                {
                    triSurfList[triSurfCount] = sortedSurfIndex;
                    if ( triSurfList[triSurfCount] != sortedSurfIndex
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_add_bsp.cpp",
                                    801,
                                    0,
                                    "triSurfList[triSurfCount] == sortedSurfIndex\n\t%i, %i",
                                    triSurfList[triSurfCount],
                                    sortedSurfIndex) )
                    {
                        __debugbreak();
                    }
                    if ( ++triSurfCount >= 0x80 )
                    {
                        R_AddBspDrawSurfs(drawSurf, (unsigned char *)triSurfList, triSurfCount, &surfData);
                        triSurfCount = 0;
                    }
                }
            }
        }
    }
    if ( triSurfCount )
    {
        if ( skipMaterial
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_add_bsp.cpp", 867, 0, "%s", "!skipMaterial") )
        {
            __debugbreak();
        }
        R_AddBspDrawSurfs(prevDrawSurf, (unsigned char *)triSurfList, triSurfCount, &surfData);
    }
    R_EndCmdBuf(&surfData.delayedCmdBuf);
    scene.drawSurfCount[stage] = surfData.drawSurfList.current - scene.drawSurfs[stage];
}

void __cdecl R_AddAllBspDrawSurfacesSpotShadow(unsigned int spotShadowIndex, unsigned int primaryLightIndex)
{
    int packed_high; // edx
    unsigned __int16 triSurfList[128]; // [esp+0h] [ebp-158h] BYREF
    GfxDrawSurf drawSurf; // [esp+108h] [ebp-50h]
    GfxShadowGeometry *shadowGeom; // [esp+114h] [ebp-44h]
    GfxDrawSurf prevDrawSurf; // [esp+118h] [ebp-40h]
    unsigned int stage; // [esp+120h] [ebp-38h]
    unsigned int sortedSurfIndex; // [esp+124h] [ebp-34h]
    unsigned int triSurfCount; // [esp+128h] [ebp-30h]
    GfxDrawSurf *drawSurfs; // [esp+12Ch] [ebp-2Ch]
    unsigned int surfIter; // [esp+130h] [ebp-28h]
    GfxDrawSurf *surfaceMaterials; // [esp+134h] [ebp-24h]
    GfxBspDrawSurfData surfData; // [esp+138h] [ebp-20h] BYREF
    int drawSurfCount; // [esp+154h] [ebp-4h]

    if ( !rgp.world
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_add_bsp.cpp", 970, 0, "%s", "rgp.world") )
    {
        __debugbreak();
    }
    surfaceMaterials = rgp.world->dpvs.surfaceMaterials;
    stage = 3 * spotShadowIndex + 27;
    R_InitBspDrawSurf((GfxSModelDrawSurfLightingData *)&surfData);
    drawSurfs = &scene.drawSurfs[stage][scene.drawSurfCount[stage]];
    surfData.drawSurfList.current = drawSurfs;
    surfData.drawSurfList.end = &scene.drawSurfs[stage][scene.maxDrawSurfCount[stage]];
    prevDrawSurf.packed = -1;
    triSurfCount = 0;
    shadowGeom = &rgp.world->shadowGeom[primaryLightIndex];
    for ( surfIter = 0; surfIter < shadowGeom->surfaceCount; ++surfIter )
    {
        sortedSurfIndex = shadowGeom->sortedSurfIndex[surfIter];
        //packed_high = HIDWORD(surfaceMaterials[sortedSurfIndex].packed);
        //*(unsigned int *)&drawSurf.fields = surfaceMaterials[sortedSurfIndex].fields;
        drawSurf.packed = surfaceMaterials[sortedSurfIndex].packed;
        //HIDWORD(drawSurf.packed) = packed_high;
        if ( drawSurf.packed != prevDrawSurf.packed )
        {
            if ( triSurfCount )
                R_AddBspDrawSurfs(prevDrawSurf, (unsigned __int8 *)triSurfList, triSurfCount, &surfData);
            prevDrawSurf.fields = drawSurf.fields;
            triSurfCount = 0;
        }
        triSurfList[triSurfCount] = sortedSurfIndex;
        if ( triSurfList[triSurfCount] != sortedSurfIndex
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_add_bsp.cpp",
                        999,
                        0,
                        "triSurfList[triSurfCount] == sortedSurfIndex\n\t%i, %i",
                        triSurfList[triSurfCount],
                        sortedSurfIndex) )
        {
            __debugbreak();
        }
        if ( ++triSurfCount >= 0x80 )
        {
            R_AddBspDrawSurfs(drawSurf, (unsigned __int8 *)triSurfList, triSurfCount, &surfData);
            triSurfCount = 0;
        }
    }
    if ( triSurfCount )
        R_AddBspDrawSurfs(prevDrawSurf, (unsigned __int8 *)triSurfList, triSurfCount, &surfData);
    R_EndCmdBuf(&surfData.delayedCmdBuf);
    drawSurfCount = surfData.drawSurfList.current - drawSurfs;
    scene.drawSurfCount[stage] += drawSurfCount;
}

