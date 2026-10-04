#include "r_draw_bsp.h"
#include "rb_tess.h"
#include "r_vertexstream2.h"
#include "r_state.h"
#include "rb_shade.h"
#include "r_dvars.h"
#include "r_shade.h"

int g_layerDataStride[18] = { 0, 0, 0, 8, 12, 16, 20, 24, 24, 28, 32, 32, 36, 40, 0, 0, 16, 0 };
int g_stream2Stride[18] = { 0, 0, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 0, 0, 0, 0 };


void __cdecl R_SetStreamsForBspSurface(GfxCmdBufPrimState *state, const srfTriangles_t *tris)
{
    int stream2ByteOffset; // [esp+0h] [ebp-34h]
    IDirect3DVertexBuffer9 *v3; // [esp+4h] [ebp-30h]
    int vertexLayerData; // [esp+8h] [ebp-2Ch]
    IDirect3DVertexBuffer9 *layerVb; // [esp+Ch] [ebp-28h]
    int v6; // [esp+10h] [ebp-24h]
    IDirect3DVertexBuffer9 *worldVb; // [esp+14h] [ebp-20h]
    int v8; // [esp+18h] [ebp-1Ch]
    IDirect3DVertexBuffer9 *v9; // [esp+1Ch] [ebp-18h]
    int vertexOffset; // [esp+20h] [ebp-14h]
    IDirect3DVertexBuffer9 *vb; // [esp+24h] [ebp-10h]
    unsigned int layerDataStride; // [esp+28h] [ebp-Ch]
    unsigned int stream2Stride; // [esp+2Ch] [ebp-8h]
    unsigned int stream2Stridea; // [esp+2Ch] [ebp-8h]

    layerDataStride = g_layerDataStride[state->vertDeclType];
    if ( layerDataStride )
    {
        if ( tris->stream2ByteOffset < 0 )
        {
            R_SetDoubleStreamSource(
                state,
                g_worldDraw->vd.worldVb,
                44 * tris->firstVertex,
                0x2Cu,
                g_worldDraw->vld.layerVb,
                tris->vertexLayerData,
                layerDataStride);
        }
        else
        {
            stream2Stridea = g_stream2Stride[state->vertDeclType];
            stream2ByteOffset = tris->stream2ByteOffset;
            v3 = *R_VertexStream2_GetVB();
            vertexLayerData = tris->vertexLayerData;
            layerVb = g_worldDraw->vld.layerVb;
            v6 = 44 * tris->firstVertex;
            worldVb = g_worldDraw->vd.worldVb;
            if ( state->streams[0].vb != worldVb || state->streams[0].offset != v6 || state->streams[0].stride != 44 )
                R_ChangeStreamSource(state, 0, worldVb, v6, 0x2Cu);
            if ( state->streams[1].vb != layerVb
                || state->streams[1].offset != vertexLayerData
                || state->streams[1].stride != layerDataStride )
            {
                R_ChangeStreamSource(state, 1u, layerVb, vertexLayerData, layerDataStride);
            }
            if ( state->streams[2].vb != v3
                || state->streams[2].offset != stream2ByteOffset
                || state->streams[2].stride != stream2Stridea )
            {
                R_ChangeStreamSource(state, 2u, v3, stream2ByteOffset, stream2Stridea);
            }
        }
    }
    else if ( tris->stream2ByteOffset < 0 )
    {
        R_SetStreamSource(state, g_worldDraw->vd.worldVb, 44 * tris->firstVertex, 0x2Cu);
    }
    else
    {
        stream2Stride = g_stream2Stride[state->vertDeclType];
        v8 = tris->stream2ByteOffset;
        v9 = *R_VertexStream2_GetVB();
        vertexOffset = 44 * tris->firstVertex;
        vb = g_worldDraw->vd.worldVb;
        if ( state->streams[0].vb != vb || state->streams[0].offset != vertexOffset || state->streams[0].stride != 44 )
            R_ChangeStreamSource(state, 0, vb, vertexOffset, 0x2Cu);
        if ( state->streams[1].vb || state->streams[1].offset || state->streams[1].stride )
            R_ChangeStreamSource(state, 1u, 0, 0, 0);
        if ( state->streams[2].vb != v9 || state->streams[2].offset != v8 || state->streams[2].stride != stream2Stride )
            R_ChangeStreamSource(state, 2u, v9, v8, stream2Stride);
    }
}

void __cdecl R_DrawBspDrawSurfsLit(
                const unsigned int *primDrawSurfPos,
                GfxCmdBufContext context,
                GfxCmdBufContext prepassContext)
{
    GfxTrianglesDrawStream drawStream; // [esp+4h] [ebp-28h] BYREF
    const MaterialPass *pass; // [esp+24h] [ebp-8h]
    unsigned int customSamplerFlags; // [esp+28h] [ebp-4h]

    pass = context.state->pass;
    customSamplerFlags = pass->customSamplerFlags;
    if ( (customSamplerFlags & 1) != 0 )
        R_SetSamplerState(context.state, 0xFu, 0x72u);
    if ( (customSamplerFlags & 2) != 0 )
        R_SetSamplerState(context.state, 0xCu, 0x62u);
    if ( (customSamplerFlags & 4) != 0 )
        R_SetSamplerState(context.state, 0xDu, 0x62u);
    if ( (customSamplerFlags & 8) != 0 )
        R_SetSamplerState(context.state, 0xEu, 0x62u);
    drawStream.reflectionProbeTexture = context.state->samplerTexture[15];
    drawStream.lightmapPrimaryTexture = context.state->samplerTexture[12];
    drawStream.lightmapSecondaryTexture = context.state->samplerTexture[13];
    drawStream.lightmapSecondaryTextureB = context.state->samplerTexture[14];
    drawStream.whiteTexture = &rgp.whiteImage->texture;
    drawStream.primDrawSurfPos = primDrawSurfPos;
    drawStream.customSamplerFlags = pass->customSamplerFlags;
    drawStream.hasSunDirChanged = context.source->input.data->prim.hasSunDirChanged;
    if ( prepassContext.state )
        R_DrawTrianglesLit(&drawStream, &context.state->prim, &prepassContext.state->prim);
    else
        R_DrawTrianglesLit(&drawStream, &context.state->prim, 0);
    context.state->samplerTexture[15] = drawStream.reflectionProbeTexture;
    context.state->samplerTexture[12] = drawStream.lightmapPrimaryTexture;
    context.state->samplerTexture[13] = drawStream.lightmapSecondaryTexture;
    context.state->samplerTexture[14] = drawStream.lightmapSecondaryTextureB;
}

void __cdecl R_DrawTrianglesLit(
                GfxTrianglesDrawStream *drawStream,
                GfxCmdBufPrimState *primState,
                GfxCmdBufPrimState *prepassPrimState)
{
    GfxTexture *whiteTexture; // [esp+0h] [ebp-ACh]
    GfxImage *blackImage; // [esp+8h] [ebp-A4h]
    const GfxSurface *bspSurf; // [esp+34h] [ebp-78h]
    int baseIndex; // [esp+38h] [ebp-74h]
    unsigned int surfIndex; // [esp+3Ch] [ebp-70h]
    const srfTriangles_t *prevTris; // [esp+48h] [ebp-64h]
    unsigned int lightmapSecondaryFlag; // [esp+4Ch] [ebp-60h]
    GfxImage *newLightmapSecondaryTextureB; // [esp+50h] [ebp-5Ch]
    unsigned int reflectionProbeFlag; // [esp+54h] [ebp-58h]
    const GfxTexture *lightmapPrimaryTexture; // [esp+58h] [ebp-54h]
    const unsigned __int16 *list; // [esp+5Ch] [ebp-50h] BYREF
    int triCount; // [esp+60h] [ebp-4Ch]
    unsigned int reflectionProbeIndex; // [esp+64h] [ebp-48h]
    const GfxTexture *reflectionProbeTexture; // [esp+68h] [ebp-44h]
    const GfxTexture *newLightmapPrimaryTexture; // [esp+6Ch] [ebp-40h]
    GfxTexture *reflectionProbeTextures; // [esp+70h] [ebp-3Ch]
    GfxTexture *conflictTex; // [esp+74h] [ebp-38h]
    unsigned int index; // [esp+78h] [ebp-34h]
    unsigned int lightmapIndex; // [esp+7Ch] [ebp-30h]
    const GfxTexture *lightmapSecondaryTexture; // [esp+80h] [ebp-2Ch]
    const GfxTexture *newLightmapSecondaryTexture; // [esp+84h] [ebp-28h]
    const GfxTexture *lightmapSecondaryTextureB; // [esp+88h] [ebp-24h]
    IDirect3DDevice9 *device; // [esp+8Ch] [ebp-20h]
    unsigned int lightmapPrimaryFlag; // [esp+90h] [ebp-1Ch]
    const GfxImage *overrideImage; // [esp+94h] [ebp-18h]
    unsigned int count; // [esp+98h] [ebp-14h] BYREF
    int baseVertex; // [esp+9Ch] [ebp-10h]
    const GfxTexture *newReflectionProbeTexture; // [esp+A0h] [ebp-Ch]
    int hasSunDirChanged; // [esp+A4h] [ebp-8h]
    bool override; // [esp+ABh] [ebp-1h]

    reflectionProbeIndex = 255;
    lightmapIndex = 31;
    prevTris = 0;
    triCount = 0;
    baseVertex = -1;
    baseIndex = 0;
    reflectionProbeTexture = drawStream->reflectionProbeTexture;
    lightmapPrimaryTexture = drawStream->lightmapPrimaryTexture;
    lightmapSecondaryTexture = drawStream->lightmapSecondaryTexture;
    lightmapSecondaryTextureB = drawStream->lightmapSecondaryTextureB;
    reflectionProbeFlag = drawStream->customSamplerFlags & 1;
    lightmapPrimaryFlag = drawStream->customSamplerFlags & 2;
    lightmapSecondaryFlag = drawStream->customSamplerFlags & 4;
    reflectionProbeTextures = g_worldDraw->reflectionProbeTextures;
    hasSunDirChanged = drawStream->hasSunDirChanged;
    overrideImage = 0;
    override = r_lightMap->current.integer != 1;
    if ( override )
        overrideImage = R_OverrideGrayscaleImage(r_lightMap);
    conflictTex = 0;
    if ( r_lightConflicts->current.enabled )
    {
        if ( (rg.frontEndFrameCount & 4) != 0 )
            blackImage = rgp.blackImage;
        else
            blackImage = rgp.whiteImage;
        conflictTex = &blackImage->texture;
    }
    device = primState->device;
    while ( R_ReadBspDrawSurfs(&drawStream->primDrawSurfPos, &list, &count) )
    {
        for ( index = 0; index < count; ++index )
        {
            surfIndex = list[index];
            if ( surfIndex >= rgp.world->surfaceCount
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_draw_bsp.cpp",
                            539,
                            0,
                            "surfIndex doesn't index rgp.world->surfaceCount\n\t%i not in [0, %i)",
                            surfIndex,
                            rgp.world->surfaceCount) )
            {
                __debugbreak();
            }
            bspSurf = &rgp.world->dpvs.surfaces[surfIndex];
            _mm_prefetch((const char *)&rgp.world->dpvs.surfaces[list[index + 1]].lightmapIndex, 1);
            if ( r_lightConflicts->current.color[0]
                 | (reflectionProbeIndex - bspSurf->reflectionProbeIndex)
                 | (lightmapIndex - bspSurf->lightmapIndex) )
            {
                if ( prevTris )
                    R_DrawBspTris(primState, prevTris, triCount);
                prevTris = &bspSurf->tris;
                triCount = 0;
                baseIndex = bspSurf->tris.baseIndex;
                if ( baseVertex != bspSurf->tris.firstVertex )
                {
                    baseVertex = bspSurf->tris.firstVertex;
                    R_SetStreamsForBspSurface(primState, &bspSurf->tris);
                }
                reflectionProbeIndex = bspSurf->reflectionProbeIndex;
                lightmapIndex = bspSurf->lightmapIndex;
                if ( reflectionProbeFlag )
                {
                    if ( reflectionProbeIndex >= g_worldDraw->reflectionProbeCount
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_draw_bsp.cpp",
                                    582,
                                    0,
                                    "reflectionProbeIndex doesn't index g_worldDraw->reflectionProbeCount\n\t%i not in [0, %i)",
                                    reflectionProbeIndex,
                                    g_worldDraw->reflectionProbeCount) )
                    {
                        __debugbreak();
                    }
                    newReflectionProbeTexture = &reflectionProbeTextures[reflectionProbeIndex];
                    if ( reflectionProbeTexture != newReflectionProbeTexture )
                    {
                        reflectionProbeTexture = newReflectionProbeTexture;
                        R_HW_SetSamplerTexture(device, 0xFu, newReflectionProbeTexture);
                    }
                }
                if ( lightmapIndex == 31 )
                {
                    if ( lightmapPrimaryFlag
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_draw_bsp.cpp",
                                    673,
                                    0,
                                    "%s\n\t(bspSurf->material->info.name) = %s",
                                    "(!lightmapPrimaryFlag)",
                                    bspSurf->material->info.name) )
                    {
                        __debugbreak();
                    }
                    if ( lightmapSecondaryFlag
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_draw_bsp.cpp",
                                    675,
                                    0,
                                    "%s\n\t(bspSurf->material->info.name) = %s",
                                    "(!lightmapSecondaryFlag)",
                                    bspSurf->material->info.name) )
                    {
                        __debugbreak();
                    }
                }
                else
                {
                    if ( lightmapIndex >= g_worldDraw->lightmapCount
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_draw_bsp.cpp",
                                    593,
                                    0,
                                    "lightmapIndex doesn't index g_worldDraw->lightmapCount\n\t%i not in [0, %i)",
                                    lightmapIndex,
                                    g_worldDraw->lightmapCount) )
                    {
                        __debugbreak();
                    }
                    if ( lightmapPrimaryFlag )
                    {
                        if ( override )
                        {
                            newLightmapPrimaryTexture = &overrideImage->texture;
                        }
                        else
                        {
                            if ( hasSunDirChanged )
                                whiteTexture = drawStream->whiteTexture;
                            else
                                whiteTexture = &g_worldDraw->lightmapPrimaryTextures[lightmapIndex];
                            newLightmapPrimaryTexture = whiteTexture;
                        }
                        if ( conflictTex && (bspSurf->flags & 2) != 0 )
                            newLightmapPrimaryTexture = conflictTex;
                        if ( lightmapPrimaryTexture != newLightmapPrimaryTexture )
                        {
                            lightmapPrimaryTexture = newLightmapPrimaryTexture;
                            R_HW_SetSamplerTexture(device, 0xCu, newLightmapPrimaryTexture);
                        }
                    }
                    if ( lightmapSecondaryFlag )
                    {
                        if ( override )
                        {
                            R_OverrideGrayscaleImage(r_lightMap);
                            newLightmapSecondaryTexture = &rgp.r5g6b5Image->texture;
                            overrideImage = rgp.g16r16Image;
                            newLightmapSecondaryTextureB = rgp.g16r16Image;
                        }
                        else
                        {
                            newLightmapSecondaryTexture = &g_worldDraw->lightmapSecondaryTextures[lightmapIndex];
                            newLightmapSecondaryTextureB = (GfxImage *)&g_worldDraw->lightmapSecondaryTexturesB[lightmapIndex];
                        }
                        if ( conflictTex && (bspSurf->flags & 2) != 0 )
                        {
                            newLightmapSecondaryTexture = conflictTex;
                            newLightmapSecondaryTextureB = (GfxImage *)conflictTex;
                        }
                        if ( lightmapSecondaryTexture != newLightmapSecondaryTexture )
                        {
                            lightmapSecondaryTexture = newLightmapSecondaryTexture;
                            R_HW_SetSamplerTexture(device, 0xDu, newLightmapSecondaryTexture);
                        }
                        if ( lightmapSecondaryTextureB != (const GfxTexture *)newLightmapSecondaryTextureB )
                        {
                            lightmapSecondaryTextureB = &newLightmapSecondaryTextureB->texture;
                            R_HW_SetSamplerTexture(device, 0xEu, &newLightmapSecondaryTextureB->texture);
                        }
                    }
                }
            }
            else if ( baseVertex != bspSurf->tris.firstVertex || baseIndex + 3 * triCount != bspSurf->tris.baseIndex )
            {
                if ( prevTris )
                    R_DrawBspTris(primState, prevTris, triCount);
                prevTris = &bspSurf->tris;
                triCount = 0;
                baseIndex = bspSurf->tris.baseIndex;
                if ( baseVertex != bspSurf->tris.firstVertex )
                {
                    baseVertex = bspSurf->tris.firstVertex;
                    R_SetStreamsForBspSurface(primState, &bspSurf->tris);
                }
            }
            triCount += bspSurf->tris.triCount;
            if ( prepassPrimState )
            {
                if ( !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_draw_bsp.cpp",
                                733,
                                0,
                                "%s",
                                "!prepassPrimState") )
                    __debugbreak();
            }
        }
    }
    if ( prevTris )
        R_DrawBspTris(primState, prevTris, triCount);
    drawStream->reflectionProbeTexture = reflectionProbeTexture;
    drawStream->lightmapPrimaryTexture = lightmapPrimaryTexture;
    drawStream->lightmapSecondaryTexture = lightmapSecondaryTexture;
    drawStream->lightmapSecondaryTextureB = lightmapSecondaryTextureB;
}

void __cdecl R_DrawBspTris(GfxCmdBufPrimState *state, const srfTriangles_t *tris, unsigned int triCount)
{
    GfxDrawPrimArgs args; // [esp+0h] [ebp-Ch] BYREF

    args.vertexCount = tris->vertexCount;
    args.triCount = triCount;
    args.baseIndex = R_SetIndexData(state, (unsigned __int8 *)&g_worldDraw->indices[tris->baseIndex], triCount);
    R_DrawIndexedPrimitive(state, &args);
    if ( state->primStats )
    {
        state->frameStats.geoIndexCount += 3 * triCount;
        state->primStats->dynamicIndexCount += 3 * triCount;
    }
}

int __cdecl R_ReadBspDrawSurfs(
                const unsigned int **primDrawSurfPos,
                const unsigned __int16 **list,
                unsigned int *count)
{
    *count = *(*primDrawSurfPos)++;
    if ( !*count )
        return 0;
    *list = (const unsigned __int16 *)*primDrawSurfPos;
    *primDrawSurfPos += (*count + 1) >> 1;
    return 1;
}

void __cdecl R_DrawBspDrawSurfsLitPreTess(const unsigned int *primDrawSurfPos, GfxCmdBufContext context)
{
    unsigned int baseIndex; // [esp+0h] [ebp-28h] BYREF
    unsigned int surfIndex; // [esp+4h] [ebp-24h]
    GfxReadCmdBuf cmdBuf; // [esp+8h] [ebp-20h] BYREF
    const srfTriangles_t *tris; // [esp+Ch] [ebp-1Ch]
    const GfxBspPreTessDrawSurf *list; // [esp+10h] [ebp-18h] BYREF
    unsigned int reflectionProbeIndex; // [esp+14h] [ebp-14h]
    const GfxSurface *bspSurf; // [esp+18h] [ebp-10h]
    unsigned int index; // [esp+1Ch] [ebp-Ch]
    unsigned int lightmapIndex; // [esp+20h] [ebp-8h]
    unsigned int count; // [esp+24h] [ebp-4h] BYREF

    cmdBuf.primDrawSurfPos = primDrawSurfPos;
    while ( R_ReadBspPreTessDrawSurfs(&cmdBuf, &list, &count, &baseIndex) )
    {
        reflectionProbeIndex = 255;
        lightmapIndex = 31;
        for ( index = 0; index < count; ++index )
        {
            surfIndex = list[index].baseSurfIndex;
            if ( surfIndex >= rgp.world->surfaceCount
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_draw_bsp.cpp",
                            928,
                            0,
                            "surfIndex doesn't index rgp.world->surfaceCount\n\t%i not in [0, %i)",
                            surfIndex,
                            rgp.world->surfaceCount) )
            {
                __debugbreak();
            }
            bspSurf = &rgp.world->dpvs.surfaces[surfIndex];
            tris = &bspSurf->tris;
            if ( reflectionProbeIndex != bspSurf->reflectionProbeIndex || lightmapIndex != bspSurf->lightmapIndex )
            {
                reflectionProbeIndex = bspSurf->reflectionProbeIndex;
                lightmapIndex = bspSurf->lightmapIndex;
                R_SetReflectionProbe(context, reflectionProbeIndex);
                R_SetLightmap(context, lightmapIndex);
                R_SetupPassPerObjectArgs(context);
                R_SetupPassPerPrimArgs(context);
            }
            R_DrawPreTessTris(&context.state->prim, tris, baseIndex, list[index].totalTriCount);
            baseIndex += 3 * list[index].totalTriCount;
        }
    }
}

void __cdecl R_DrawPreTessTris(
                GfxCmdBufPrimState *state,
                const srfTriangles_t *tris,
                unsigned int baseIndex,
                unsigned int triCount)
{
    GfxDrawPrimArgs args; // [esp+0h] [ebp-Ch] BYREF

    R_SetStreamsForBspSurface(state, tris);
    args.vertexCount = tris->vertexCount;
    args.triCount = triCount;
    args.baseIndex = baseIndex;
    R_DrawIndexedPrimitive(state, &args);
    state->frameStats.geoIndexCount += 3 * triCount;
    if ( !state->primStats
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_draw_bsp.cpp", 393, 0, "%s", "state->primStats") )
    {
        __debugbreak();
    }
    state->primStats->dynamicIndexCount += 3 * triCount;
}

int __cdecl R_ReadBspPreTessDrawSurfs(
                GfxReadCmdBuf *cmdBuf,
                const GfxBspPreTessDrawSurf **list,
                unsigned int *count,
                unsigned int *baseIndex)
{
    *count = R_ReadPrimDrawSurfInt(cmdBuf);
    if ( !*count )
        return 0;
    *baseIndex = R_ReadPrimDrawSurfInt(cmdBuf);
    *list = (const GfxBspPreTessDrawSurf *)R_ReadPrimDrawSurfData(cmdBuf, *count);
    return 1;
}

void __cdecl R_DrawBspDrawSurfsPreTess(const unsigned int *primDrawSurfPos, GfxCmdBufContext context)
{
    unsigned int baseIndex; // [esp+0h] [ebp-2Ch] BYREF
    unsigned int surfIndex; // [esp+4h] [ebp-28h]
    GfxReadCmdBuf cmdBuf; // [esp+8h] [ebp-24h] BYREF
    const srfTriangles_t *tris; // [esp+Ch] [ebp-20h]
    const srfTriangles_t *prevTris; // [esp+10h] [ebp-1Ch]
    const GfxBspPreTessDrawSurf *list; // [esp+14h] [ebp-18h] BYREF
    unsigned int triCount; // [esp+18h] [ebp-14h]
    const GfxSurface *bspSurf; // [esp+1Ch] [ebp-10h]
    unsigned int index; // [esp+20h] [ebp-Ch]
    unsigned int count; // [esp+24h] [ebp-8h] BYREF
    int baseVertex; // [esp+28h] [ebp-4h]

    R_SetupPassPerObjectArgs(context);
    R_SetupPassPerPrimArgs(context);
    cmdBuf.primDrawSurfPos = primDrawSurfPos;
    while ( R_ReadBspPreTessDrawSurfs(&cmdBuf, &list, &count, &baseIndex) )
    {
        prevTris = 0;
        triCount = 0;
        baseVertex = -1;
        for ( index = 0; index < count; ++index )
        {
            surfIndex = list[index].baseSurfIndex;
            if ( surfIndex >= rgp.world->surfaceCount
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_draw_bsp.cpp",
                            981,
                            0,
                            "surfIndex doesn't index rgp.world->surfaceCount\n\t%i not in [0, %i)",
                            surfIndex,
                            rgp.world->surfaceCount) )
            {
                __debugbreak();
            }
            bspSurf = &rgp.world->dpvs.surfaces[surfIndex];
            tris = &bspSurf->tris;
            if ( baseVertex != bspSurf->tris.firstVertex )
            {
                if ( triCount )
                {
                    R_DrawPreTessTris(&context.state->prim, prevTris, baseIndex, triCount);
                    baseIndex += 3 * triCount;
                    triCount = 0;
                }
                prevTris = tris;
                baseVertex = tris->firstVertex;
            }
            triCount += list[index].totalTriCount;
        }
        R_DrawPreTessTris(&context.state->prim, prevTris, baseIndex, triCount);
    }
}

void __cdecl R_DrawBspDrawSurfs(const unsigned int *primDrawSurfPos, GfxCmdBufState *state)
{
    GfxTrianglesDrawStream drawStream; // [esp+0h] [ebp-20h] BYREF

    drawStream.primDrawSurfPos = primDrawSurfPos;
    R_DrawTriangles(&drawStream, &state->prim);
}

void __cdecl R_DrawTriangles(GfxTrianglesDrawStream *drawStream, GfxCmdBufPrimState *state)
{
    int baseIndex; // [esp+0h] [ebp-28h]
    const GfxSurface *tris; // [esp+8h] [ebp-20h]
    const srfTriangles_t *prevTris; // [esp+Ch] [ebp-1Ch]
    const unsigned __int16 *list; // [esp+10h] [ebp-18h] BYREF
    int triCount; // [esp+14h] [ebp-14h]
    const GfxSurface *bspSurf; // [esp+18h] [ebp-10h]
    unsigned int index; // [esp+1Ch] [ebp-Ch]
    unsigned int count; // [esp+20h] [ebp-8h] BYREF
    int baseVertex; // [esp+24h] [ebp-4h]

    prevTris = 0;
    triCount = 0;
    baseVertex = -1;
    baseIndex = 0;
    while ( R_ReadBspDrawSurfs(&drawStream->primDrawSurfPos, &list, &count) )
    {
        for ( index = 0; index < count; ++index )
        {
            bspSurf = &rgp.world->dpvs.surfaces[list[index]];
            tris = bspSurf;
            if ( baseVertex != bspSurf->tris.firstVertex || baseIndex + 3 * triCount != bspSurf->tris.baseIndex )
            {
                if ( prevTris )
                    R_DrawBspTris(state, prevTris, triCount);
                prevTris = &tris->tris;
                triCount = 0;
                baseIndex = tris->tris.baseIndex;
                if ( baseVertex != tris->tris.firstVertex )
                {
                    baseVertex = tris->tris.firstVertex;
                    R_SetStreamsForBspSurface(state, &tris->tris);
                }
            }
            triCount += tris->tris.triCount;
        }
    }
    if ( prevTris )
        R_DrawBspTris(state, prevTris, triCount);
}

