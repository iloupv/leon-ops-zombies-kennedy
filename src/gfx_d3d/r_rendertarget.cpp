#include "r_rendertarget.h"
#include "r_dvars.h"
#include "r_init.h"
#include "rb_backend.h"
#include "r_ui3d.h"
#include "r_image_load_common.h"
#include "rb_logfile.h"
#include <qcommon/threads.h>
#include "rb_resource.h"

ExtraCamSettings g_extraCamSettings;

const char *s_renderTargetNames[44] =
{
  "R_RENDERTARGET_NULL",
  "R_RENDERTARGET_SAVED_SCREEN",
  "R_RENDERTARGET_FRAME_BUFFER",
  "R_RENDERTARGET_SCENE",
  "R_RENDERTARGET_SCENE_NULLCOLOR",
  "R_RENDERTARGET_RESOLVED_POST_SUN",
  "R_RENDERTARGET_RESOLVED_SCENE",
  "R_RENDERTARGET_FLOAT_Z",
  "R_RENDERTARGET_PINGPONG_0",
  "R_RENDERTARGET_PINGPONG_1",
  "R_RENDERTARGET_POST_EFFECT_SRC",
  "R_RENDERTARGET_POST_EFFECT_GODRAYS",
  "R_RENDERTARGET_POST_EFFECT_0",
  "R_RENDERTARGET_POST_EFFECT_1",
  "R_RENDERTARGET_SHADOWMAP_SUN",
  "R_RENDERTARGET_SHADOWMAP_SPOT",
  "R_RENDERTARGET_16BIT_SYSTEM",
  "R_RENDERTARGET_8BIT_SYSTEM",
  "R_RENDERTARGET_8BIT_SWAPCHAIN_BACKBUFFER",
  "R_RENDERTARGET_SEETHRU_DECAL",
  "R_RENDERTARGET_UI3D",
  "R_RENDERTARGET_UI3D_PING_PONG",
  "R_RENDERTARGET_MISSILE_CAM",
  "R_RENDERTARGET_FLOAT_Z_MISSILE_CAM",
  "R_RENDERTARGET_COMPOSITE",
  "R_RENDERTARGET_BLOOM_MIP1",
  "R_RENDERTARGET_BLOOM_MIP2",
  "R_RENDERTARGET_BLOOM_MIP3",
  "R_RENDERTARGET_BLOOM_MIP3_PING",
  "R_RENDERTARGET_BLOOM_MIP3_PONG",
  "R_RENDERTARGET_BLOOM_STREAK",
  "R_RENDERTARGET_SHADOWMAP_SUN_HI_FULL",
  "R_RENDERTARGET_SHADOWMAP_SUN_HI_PARTITION_0",
  "R_RENDERTARGET_SHADOWMAP_SUN_HI_PARTITION_1",
  "R_RENDERTARGET_SHADOWMAP_SUN_LO_FULL",
  "R_RENDERTARGET_SHADOWMAP_SUN_LO_PARTITION_0",
  "R_RENDERTARGET_SHADOWMAP_SUN_LO_PARTITION_1",
  "R_RENDERTARGET_SHADOWMAP_SPOT_FULL",
  "R_RENDERTARGET_SHADOWMAP_SPOT_0",
  "R_RENDERTARGET_SHADOWMAP_SPOT_1",
  "R_RENDERTARGET_SHADOWMAP_SPOT_2",
  "R_RENDERTARGET_SHADOWMAP_SPOT_3",
  "R_RENDERTARGET_SHADOWMAP_SPOT_HI_0",
  "R_RENDERTARGET_SHADOWMAP_SPOT_HI_1"
};




int __cdecl R_GetDepthStencilFormat(_D3DFORMAT renderTargetFormat)
{
    if ( R_IsDepthStencilFormatOk(renderTargetFormat, D3DFMT_D24FS8) )
        return 83;
    else
        return 75;
}

bool __cdecl R_IsDepthStencilFormatOk(_D3DFORMAT renderTargetFormat, _D3DFORMAT depthStencilFormat)
{
    return dx.d3d9->CheckDeviceFormat(dx.adapterIndex,
                     D3DDEVTYPE_HAL,
                     D3DFMT_X8R8G8B8,
                     2,
                     D3DRTYPE_SURFACE,
                     depthStencilFormat) >= 0
            && dx.d3d9->CheckDepthStencilMatch(
                     dx.adapterIndex,
                     D3DDEVTYPE_HAL,
                     D3DFMT_X8R8G8B8,
                     renderTargetFormat,
                     depthStencilFormat) >= 0;
}

void __cdecl R_InitRenderTargets()
{
    R_InitRenderTargets_PC();
}

void R_InitRenderTargets_PC()
{
    _D3DFORMAT backBufferFormat; // [esp+0h] [ebp-4h]

    backBufferFormat = R_InitFrameBufferRenderTarget();
    if (r_reflectionProbeGenerate->current.enabled)
    {
        R_InitFullscreenRenderTargetImage(
            14,
            FULLSCREEN_SCENE,
            0,
            D3DFMT_A16B16G16R16F,
            RENDERTARGET_USAGE_RENDER,
            &gfxRenderTargets[R_RENDERTARGET_16BIT_SYSTEM]);
        R_InitFullscreenRenderTargetImage(
            15,
            FULLSCREEN_SCENE,
            0,
            D3DFMT_A8R8G8B8,
            RENDERTARGET_USAGE_RENDER,
            &gfxRenderTargets[R_RENDERTARGET_8BIT_SYSTEM]);
        R_InitFullscreenRenderTargetImage(
            13,
            FULLSCREEN_SCENE,
            0,
            D3DFMT_A16B16G16R16F,
            RENDERTARGET_USAGE_RENDER,
            &gfxRenderTargets[R_RENDERTARGET_FRAME_BUFFER]);
        R_ShareRenderTarget(R_RENDERTARGET_FRAME_BUFFER, R_RENDERTARGET_SCENE);
    }
    if (!g_allocateMinimalResources)
    {
        if (r_floatz->current.enabled)
        {
            R_InitIntZRenderTarget_PC();
            if (!dx.supportsIntZ)
                R_InitFullscreenRenderTargetImage(
                    2,
                    FULLSCREEN_SCENE,
                    0,
                    D3DFMT_R32F,
                    RENDERTARGET_USAGE_RENDER,
                    &gfxRenderTargets[R_RENDERTARGET_FLOAT_Z]);
        }
        R_ShareRenderTarget(R_RENDERTARGET_RESOLVED_SCENE, R_RENDERTARGET_RESOLVED_POST_SUN);
        R_InitFullscreenRenderTargetImage(
            10,
            FULLSCREEN_SCENE,
            0,
            backBufferFormat,
            RENDERTARGET_USAGE_RENDER,
            &gfxRenderTargets[R_RENDERTARGET_SAVED_SCREEN]);
        R_InitSceneNullRenderTarget_PC();
        dx.sunShadowmapSize = 1024;
        if (dx.nvStereoHandle)
        {
            if (vidConfig.displayWidth <= 0x400)
                dx.sunShadowmapSize = 512;
        }
        R_InitShadowmapRenderTarget(0, dx.sunShadowmapSize, 2u, &gfxRenderTargets[R_RENDERTARGET_SHADOWMAP_SUN], SHADOWMAP_RT_TILE_VERTICAL);
        R_InitShadowmapRenderTarget(1, 0x200u, 4u, &gfxRenderTargets[R_RENDERTARGET_SHADOWMAP_SPOT], SHADOWMAP_RT_TILE_VERTICAL);
        R_InitFullscreenRenderTargetImage(
            5,
            FULLSCREEN_SCENE,
            2,
            backBufferFormat,
            RENDERTARGET_USAGE_RENDER,
            &gfxRenderTargets[R_RENDERTARGET_POST_EFFECT_0]);
        R_InitFullscreenRenderTargetImage(
            6,
            FULLSCREEN_SCENE,
            2,
            backBufferFormat,
            RENDERTARGET_USAGE_RENDER,
            &gfxRenderTargets[R_RENDERTARGET_POST_EFFECT_1]);
        R_InitFullscreenRenderTargetImage(
            3,
            FULLSCREEN_SCENE,
            2,
            backBufferFormat,
            RENDERTARGET_USAGE_RENDER,
            &gfxRenderTargets[R_RENDERTARGET_POST_EFFECT_SRC]);
        R_InitFullscreenRenderTargetImage(
            4,
            FULLSCREEN_SCENE,
            2,
            backBufferFormat,
            RENDERTARGET_USAGE_RENDER,
            &gfxRenderTargets[R_RENDERTARGET_POST_EFFECT_GODRAYS]);
        R_InitFullscreenRenderTargetImage(
            7,
            FULLSCREEN_SCENE,
            2,
            backBufferFormat,
            RENDERTARGET_USAGE_RENDER,
            &gfxRenderTargets[R_RENDERTARGET_PINGPONG_0]);
        R_InitFullscreenRenderTargetImage(
            8,
            FULLSCREEN_SCENE,
            2,
            backBufferFormat,
            RENDERTARGET_USAGE_RENDER,
            &gfxRenderTargets[R_RENDERTARGET_PINGPONG_1]);
        R_InitFullscreenRenderTargetImage(
            23,
            FULLSCREEN_SCENE,
            1,
            D3DFMT_A16B16G16R16,
            RENDERTARGET_USAGE_RENDER,
            &gfxRenderTargets[R_RENDERTARGET_BLOOM_MIP1]);
        R_InitFullscreenRenderTargetImage(
            24,
            FULLSCREEN_SCENE,
            2,
            D3DFMT_A16B16G16R16,
            RENDERTARGET_USAGE_RENDER,
            &gfxRenderTargets[R_RENDERTARGET_BLOOM_MIP2]);
        R_InitFullscreenRenderTargetImage(
            25,
            FULLSCREEN_SCENE,
            3,
            D3DFMT_A16B16G16R16,
            RENDERTARGET_USAGE_RENDER,
            &gfxRenderTargets[R_RENDERTARGET_BLOOM_MIP3]);
        R_InitFullscreenRenderTargetImage(
            26,
            FULLSCREEN_SCENE,
            3,
            D3DFMT_A16B16G16R16,
            RENDERTARGET_USAGE_RENDER,
            &gfxRenderTargets[R_RENDERTARGET_BLOOM_MIP3_PING]);
        R_InitFullscreenRenderTargetImage(
            27,
            FULLSCREEN_SCENE,
            3,
            D3DFMT_A16B16G16R16,
            RENDERTARGET_USAGE_RENDER,
            &gfxRenderTargets[R_RENDERTARGET_BLOOM_MIP3_PONG]);
        R_InitFullscreenRenderTargetImage(
            28,
            FULLSCREEN_SCENE,
            3,
            D3DFMT_A8R8G8B8,
            RENDERTARGET_USAGE_RENDER,
            &gfxRenderTargets[R_RENDERTARGET_BLOOM_STREAK]);
        R_InitRenderTargetImage_Composite(22, &gfxRenderTargets[R_RENDERTARGET_COMPOSITE]);
        R_UI3D_CheckRenderTarget();
        R_CheckExtraCamTargets();
    }
}

void __cdecl R_ShareRenderTarget(GfxRenderTargetId idFrom, GfxRenderTargetId idTo)
{
    AssertUninitializedRenderTarget(&gfxRenderTargets[idTo]);

    gfxRenderTargets[idTo] = gfxRenderTargets[idFrom];

    if (gfxRenderTargets[idTo].surface.color)
        gfxRenderTargets[idTo].surface.color->AddRef();

    if (gfxRenderTargets[idTo].surface.depthStencil)
        gfxRenderTargets[idTo].surface.depthStencil->AddRef();
}

void __cdecl AssertUninitializedRenderTarget(const GfxRenderTarget *renderTarget)
{
    if ( !renderTarget
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_rendertarget.cpp", 783, 0, "%s", "renderTarget") )
    {
        __debugbreak();
    }
    if ( renderTarget->image
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_rendertarget.cpp",
                    784,
                    0,
                    "%s",
                    "renderTarget->image == NULL") )
    {
        __debugbreak();
    }
    if ( renderTarget->surface.color
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_rendertarget.cpp",
                    786,
                    0,
                    "%s",
                    "renderTarget->surface.color == NULL") )
    {
        __debugbreak();
    }
    if ( renderTarget->surface.depthStencil
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_rendertarget.cpp",
                    787,
                    0,
                    "%s",
                    "renderTarget->surface.depthStencil == NULL") )
    {
        __debugbreak();
    }
    if ( renderTarget->width
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_rendertarget.cpp",
                    789,
                    0,
                    "%s",
                    "renderTarget->width == 0") )
    {
        __debugbreak();
    }
    if ( renderTarget->height )
    {
        if ( !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_rendertarget.cpp",
                        790,
                        0,
                        "%s",
                        "renderTarget->height == 0") )
            __debugbreak();
    }
}

void __cdecl R_InitFullscreenRenderTargetImage(
                int imageProgType,
                FullscreenType screenType,
                int picmip,
                _D3DFORMAT format,
                RenderTargetUsage usage,
                GfxRenderTarget *renderTarget)
{
    unsigned __int16 v6; // [esp+0h] [ebp-20h]
    unsigned __int16 v7; // [esp+4h] [ebp-1Ch]
    int fullscreenWidth; // [esp+18h] [ebp-8h] BYREF
    int fullscreenHeight; // [esp+1Ch] [ebp-4h] BYREF

    fullscreenWidth = 0;
    fullscreenHeight = 0;
    R_GetFullScreenRes(screenType, &fullscreenWidth, &fullscreenHeight);
    if ( fullscreenWidth >> picmip > 1 )
        v7 = fullscreenWidth >> picmip;
    else
        v7 = 1;
    if ( fullscreenHeight >> picmip > 1 )
        v6 = fullscreenHeight >> picmip;
    else
        v6 = 1;
    R_InitRenderTargetImage(imageProgType, v7, v6, format, usage, renderTarget);
    if ( usage == RENDERTARGET_USAGE_RENDER_SHARE_SCENE )
    {
        if ( !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_rendertarget.cpp",
                        1479,
                        0,
                        "RENDERTARGET_USAGE_RENDER_SHARE_SCENE only implemented for XBOX") )
            __debugbreak();
    }
    else if ( usage == RENDERTARGET_USAGE_RENDER )
    {
        renderTarget->surface.depthStencil = R_AssignSingleSampleDepthStencilSurface();
    }
    Image_TrackFullscreenTexture(renderTarget->image, fullscreenWidth, fullscreenHeight, picmip, format);
}

void __cdecl R_GetFullScreenRes(FullscreenType screenType, int *fullscreenWidth, int *fullscreenHeight)
{
    unsigned int sceneHeight; // [esp+0h] [ebp-8h]
    unsigned int sceneWidth; // [esp+4h] [ebp-4h]

    if ( (unsigned int)screenType > FULLSCREEN_SCENE
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_rendertarget.cpp",
                    1108,
                    0,
                    "%s\n\t(screenType) = %i",
                    "(screenType == FULLSCREEN_DISPLAY || screenType == FULLSCREEN_MIXED || screenType == FULLSCREEN_SCENE)",
                    screenType) )
    {
        __debugbreak();
    }
    if ( screenType )
        sceneWidth = vidConfig.sceneWidth;
    else
        sceneWidth = vidConfig.displayWidth;
    *fullscreenWidth = sceneWidth;
    if ( screenType == FULLSCREEN_SCENE )
        sceneHeight = vidConfig.sceneHeight;
    else
        sceneHeight = vidConfig.displayHeight;
    *fullscreenHeight = sceneHeight;
}

IDirect3DSurface9 *R_AssignSingleSampleDepthStencilSurface()
{
    const char *v2; // eax
    int hr; // [esp+4h] [ebp-Ch]
    int depthStencilWidth; // [esp+8h] [ebp-8h] BYREF
    int depthStencilHeight; // [esp+Ch] [ebp-4h] BYREF

    if (!dx.singleSampleDepthStencilSurface && dx.multiSampleType == D3DMULTISAMPLE_NONE)
        dx.singleSampleDepthStencilSurface = gfxRenderTargets[R_RENDERTARGET_FRAME_BUFFER].surface.depthStencil;
    if (dx.singleSampleDepthStencilSurface)
    {
        dx.singleSampleDepthStencilSurface->AddRef();
        return dx.singleSampleDepthStencilSurface;
    }
    else
    {
        R_GetFrameBufferDepthStencilRes(&depthStencilWidth, &depthStencilHeight);
        if (r_logFile && r_logFile->current.integer)
            RB_LogPrint(
                "dx.device->CreateDepthStencilSurface( depthStencilWidth, depthStencilHeight, dx.depthStencilFormat, D3DMULTISAMP"
                "LE_NONE, 0, 0, &dx.singleSampleDepthStencilSurface, 0 )\n");
        hr = dx.device->CreateDepthStencilSurface(
            depthStencilWidth,
            depthStencilHeight,
            dx.depthStencilFormat,
            D3DMULTISAMPLE_NONE,
            0,
            0,
            &dx.singleSampleDepthStencilSurface,
            0);
        if (hr < 0)
        {
            ++g_disableRendering;
            v2 = R_ErrorDescription(hr);
            Com_Error(
                ERR_FATAL,
                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_rendertarget.cpp (%i) dx.device->CreateDepthStencilSurface( depthS"
                "tencilWidth, depthStencilHeight, dx.depthStencilFormat, D3DMULTISAMPLE_NONE, 0, 0, &dx.singleSampleDepthStencilS"
                "urface, 0 ) failed: %s\n",
                1156,
                v2);
        }
        if (!dx.singleSampleDepthStencilSurface
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_rendertarget.cpp",
                1171,
                0,
                "%s",
                "dx.singleSampleDepthStencilSurface"))
        {
            __debugbreak();
        }
        return dx.singleSampleDepthStencilSurface;
    }
}

void __cdecl R_GetFrameBufferDepthStencilRes(int *depthStencilWidth, int *depthStencilHeight)
{
    *depthStencilWidth = vidConfig.displayWidth;
    *depthStencilHeight = vidConfig.displayHeight;
}

void __cdecl R_InitRenderTargetImage(
                int imageProgType,
                unsigned __int16 width,
                unsigned __int16 height,
                _D3DFORMAT format,
                RenderTargetUsage usage,
                GfxRenderTarget *renderTarget)
{
    unsigned int renderTargetId; // [esp+0h] [ebp-4h]

    AssertUninitializedRenderTarget(renderTarget);
    if ( !width
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_rendertarget.cpp",
                    1332,
                    0,
                    "%s\n\t(width) = %i",
                    "(width > 0)",
                    0) )
    {
        __debugbreak();
    }
    if ( !height
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_rendertarget.cpp",
                    1333,
                    0,
                    "%s\n\t(height) = %i",
                    "(height > 0)",
                    0) )
    {
        __debugbreak();
    }
    renderTargetId = renderTarget - gfxRenderTargets;
    if ( renderTargetId >= 0x2C
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_rendertarget.cpp",
                    1336,
                    0,
                    "renderTargetId doesn't index R_RENDERTARGET_COUNT\n\t%i not in [0, %i)",
                    renderTargetId,
                    44) )
    {
        __debugbreak();
    }
    renderTarget->image = Image_AllocProg(imageProgType, 6u, 0);
    if ( !renderTarget->image
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_rendertarget.cpp",
                    1339,
                    1,
                    "%s",
                    "renderTarget->image") )
    {
        __debugbreak();
    }
    Image_SetupRenderTarget(renderTarget->image, width, height, format);
    if ( usage )
        R_AssignImageToRenderTargetColor(renderTarget, renderTarget->image);
    else
        R_AssignImageToRenderTargetDepthStencil(renderTarget, renderTarget->image);
    renderTarget->width = width;
    renderTarget->height = height;
}

void __cdecl R_AssignImageToRenderTargetDepthStencil(GfxRenderTarget *renderTarget, GfxImage *image)
{
    renderTarget->surface.depthStencil = Image_GetSurface(image);
}

void __cdecl R_AssignImageToRenderTargetColor(GfxRenderTarget *renderTarget, GfxImage *image)
{
    renderTarget->surface.color = Image_GetSurface(image);
}

void __cdecl R_InitRenderTargetImage_Composite(int imageProgType, GfxRenderTarget *renderTarget)
{
    R_InitRenderTargetImage(imageProgType, 0x100u, 0x100u, D3DFMT_A8R8G8B8, RENDERTARGET_USAGE_RENDER, renderTarget);
}

void __cdecl R_InitShadowmapRenderTarget(
                int imageProgType,
                unsigned __int16 tileRes,
                unsigned __int16 tileRowCount,
                GfxRenderTarget *renderTarget,
                ShadowmapRTTiling tilingMethond)
{
    const char *v5; // eax
    const char *v6; // eax
    int v8; // [esp+4h] [ebp-1Ch]
    int hr; // [esp+Ch] [ebp-14h]
    unsigned __int16 totalHeight; // [esp+14h] [ebp-Ch]
    RenderTargetUsage usage; // [esp+18h] [ebp-8h]
    unsigned __int16 totalWidth; // [esp+1Ch] [ebp-4h]

    AssertUninitializedRenderTarget(renderTarget);
    if ( tilingMethond )
    {
        totalWidth = tileRowCount / 2 * tileRes;
        totalHeight = totalWidth;
    }
    else
    {
        totalWidth = tileRes;
        totalHeight = tileRowCount * tileRes;
    }
    if ( ((totalWidth - 1) & totalWidth) != 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_rendertarget.cpp",
                    1674,
                    0,
                    "%s\n\t(totalWidth) = %i",
                    "((((totalWidth) & ((totalWidth) - 1)) == 0))",
                    totalWidth) )
    {
        __debugbreak();
    }
    if ( ((totalHeight - 1) & totalHeight) != 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_rendertarget.cpp",
                    1675,
                    0,
                    "%s\n\t(totalHeight) = %i",
                    "((((totalHeight) & ((totalHeight) - 1)) == 0))",
                    totalHeight) )
    {
        __debugbreak();
    }
    usage = (RenderTargetUsage)(gfxMetrics.shadowmapBuildTechType != 2);
    R_InitAndTrackRenderTargetImage(
        imageProgType,
        totalWidth,
        totalHeight,
        gfxMetrics.shadowmapFormatPrimary,
        usage,
        renderTarget);
    if ( usage )
    {
        if ( r_logFile && r_logFile->current.integer )
            RB_LogPrint(
                "dx.device->CreateDepthStencilSurface( totalWidth, totalHeight, gfxMetrics.shadowmapFormatSecondary, D3DMULTISAMP"
                "LE_NONE, 0, 0, &renderTarget->surface.depthStencil, 0 )\n");


        //v8 = ((int (__thiscall *)(IDirect3DDevice9 *, IDirect3DDevice9 *, unsigned int, unsigned int, _D3DFORMAT, unsigned int, unsigned int, unsigned int, IDirect3DSurface9 **, unsigned int))dx.device->CreateDepthStencilSurface)(
        //             dx.device,
        //             dx.device,
        //             totalWidth,
        //             totalHeight,
        //             gfxMetrics.shadowmapFormatSecondary,
        //             0,
        //             0,
        //             0,
        //             &renderTarget->surface.depthStencil,
        //             0);

        v8 = dx.device->CreateDepthStencilSurface(totalWidth, totalHeight, gfxMetrics.shadowmapFormatSecondary, D3DMULTISAMPLE_NONE, 0, 0, &renderTarget->surface.depthStencil, 0);
        if ( v8 < 0 )
        {
            ++g_disableRendering;
            v6 = R_ErrorDescription(v8);
            Com_Error(
                ERR_FATAL,
                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_rendertarget.cpp (%i) dx.device->CreateDepthStencilSurface( totalW"
                "idth, totalHeight, gfxMetrics.shadowmapFormatSecondary, D3DMULTISAMPLE_NONE, 0, 0, &renderTarget->surface.depthS"
                "tencil, 0 ) failed: %s\n",
                1688,
                v6);
        }
    }
    else
    {
        if ( r_logFile && r_logFile->current.integer )
            RB_LogPrint(
                "dx.device->CreateRenderTarget( totalWidth, totalHeight, gfxMetrics.shadowmapFormatSecondary, D3DMULTISAMPLE_NONE"
                ", 0, 0, &renderTarget->surface.color, 0 )\n");
        //hr = ((int (__thiscall *)(IDirect3DDevice9 *, IDirect3DDevice9 *, unsigned int, unsigned int, _D3DFORMAT, unsigned int, unsigned int, unsigned int, GfxRenderTargetSurface *, unsigned int))
        // dx.device->CreateRenderTarget)(
        //             dx.device,
        //             dx.device,
        //             totalWidth,
        //             totalHeight,
        //             gfxMetrics.shadowmapFormatSecondary,
        //             0,
        //             0,
        //             0,
        //             &renderTarget->surface,
        //             0);

        hr = dx.device->CreateRenderTarget(totalWidth, totalHeight, gfxMetrics.shadowmapFormatSecondary, D3DMULTISAMPLE_NONE, 0, 0, &renderTarget->surface.color, 0);
        if ( hr < 0 )
        {
            ++g_disableRendering;
            v5 = R_ErrorDescription(hr);
            Com_Error(
                ERR_FATAL,
                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_rendertarget.cpp (%i) dx.device->CreateRenderTarget( totalWidth, t"
                "otalHeight, gfxMetrics.shadowmapFormatSecondary, D3DMULTISAMPLE_NONE, 0, 0, &renderTarget->surface.color, 0 ) failed: %s\n",
                1682,
                v5);
        }
    }
}

void __cdecl R_InitAndTrackRenderTargetImage(
                int imageProgType,
                unsigned __int16 width,
                unsigned __int16 height,
                _D3DFORMAT format,
                RenderTargetUsage usage,
                GfxRenderTarget *renderTarget)
{
    R_InitRenderTargetImage(imageProgType, width, height, format, usage, renderTarget);
    Image_TrackTexture(renderTarget->image, 3, format, width, height, 1);
}

_D3DFORMAT __cdecl R_InitFrameBufferRenderTarget()
{
    const char *v0; // eax
    const char *v1; // eax
    const char *v2; // eax
    const char *v3; // eax
    _D3DSURFACE_DESC surfaceDesc; // [esp+0h] [ebp-20h] BYREF

    if ( r_reflectionProbeGenerate->current.enabled )
    {
        R_InitFrameBufferRenderTarget_Win32(&gfxRenderTargets[R_RENDERTARGET_8BIT_SWAPCHAIN_BACKBUFFER]);
        v0 = R_DescribeFormat(D3DFMT_A8R8G8B8);
        Com_Printf(8, "Requested frame buffer to be %s\n", v0);
        gfxRenderTargets[R_RENDERTARGET_8BIT_SWAPCHAIN_BACKBUFFER].surface.color->GetDesc(&surfaceDesc);
        if ( surfaceDesc.Format == D3DFMT_UNKNOWN
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_rendertarget.cpp",
                        2017,
                        0,
                        "%s",
                        "surfaceDesc.Format != D3DFMT_UNKNOWN") )
        {
            __debugbreak();
        }
        v1 = R_DescribeFormat(surfaceDesc.Format);
        Com_Printf(8, "DirectX returned a frame buffer that is %s\n", v1);
        if ( !g_allocateMinimalResources )
            R_InitFullscreenRenderTargetImage(
                9,
                FULLSCREEN_SCENE,
                0,
                D3DFMT_A16B16G16R16F,
                RENDERTARGET_USAGE_RENDER,
                &gfxRenderTargets[R_RENDERTARGET_RESOLVED_SCENE]);
    }
    else
    {
        R_InitFrameBufferRenderTarget_Win32(&gfxRenderTargets[R_RENDERTARGET_FRAME_BUFFER]);
        R_ShareRenderTarget(R_RENDERTARGET_FRAME_BUFFER, R_RENDERTARGET_SCENE);
        v2 = R_DescribeFormat(D3DFMT_A8R8G8B8);
        Com_Printf(8, "Requested frame buffer to be %s\n", v2);
        gfxRenderTargets[R_RENDERTARGET_FRAME_BUFFER].surface.color->GetDesc(&surfaceDesc);
        if ( surfaceDesc.Format == D3DFMT_UNKNOWN
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_rendertarget.cpp",
                        2031,
                        0,
                        "%s",
                        "surfaceDesc.Format != D3DFMT_UNKNOWN") )
        {
            __debugbreak();
        }
        v3 = R_DescribeFormat(surfaceDesc.Format);
        Com_Printf(8, "DirectX returned a frame buffer that is %s\n", v3);
        if ( !g_allocateMinimalResources )
            R_InitFullscreenRenderTargetImage(
                9,
                FULLSCREEN_SCENE,
                0,
                surfaceDesc.Format,
                RENDERTARGET_USAGE_RENDER,
                &gfxRenderTargets[R_RENDERTARGET_RESOLVED_SCENE]);
    }
    return surfaceDesc.Format;
}

const char *__cdecl R_DescribeFormat(_D3DFORMAT format)
{
    const char *result; // eax

    switch ( format )
    {
        case D3DFMT_A8R8G8B8:
        case D3DFMT_A8B8G8R8:
            result = "24-bit color with 8-bit alpha";
            break;
        case D3DFMT_R5G6B5:
            result = "16-bit color";
            break;
        case D3DFMT_A1R5G5B5:
            result = "15-bit color with 1-bit alpha";
            break;
        case D3DFMT_D16_LOCKABLE:
        case D3DFMT_D16:
            result = "16-bit depth without stencil";
            break;
        case D3DFMT_D15S1:
            result = "15-bit depth with 1-bit stencil";
            break;
        case D3DFMT_D24S8:
            result = "24-bit depth with 8-bit stencil";
            break;
        case D3DFMT_D24X8:
            result = "24-bit depth without stencil";
            break;
        default:
            result = va("unknown format 0x%08x", format);
            break;
    }
    return result;
}

void __cdecl R_InitFrameBufferRenderTarget_Win32(GfxRenderTarget *renderTarget)
{
    const char *v1; // eax
    const char *v2; // eax
    const char *v3; // eax
    int v5; // [esp+4h] [ebp-1Ch]
    int depthStencilWidth; // [esp+8h] [ebp-18h] BYREF
    int depthStencilHeight; // [esp+Ch] [ebp-14h] BYREF
    HRESULT v9; // [esp+14h] [ebp-Ch]
    HRESULT hr; // [esp+1Ch] [ebp-4h]

    if ( !renderTarget
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_rendertarget.cpp",
                    1799,
                    0,
                    "%s",
                    "renderTarget") )
    {
        __debugbreak();
    }
    renderTarget->width = vidConfig.displayWidth;
    renderTarget->height = vidConfig.displayHeight;
    if ( r_logFile && r_logFile->current.integer )
        RB_LogPrint("dx.device->GetSwapChain( 0, &dx.windows[0].swapChain )\n");

    hr = dx.device->GetSwapChain(0, &dx.windows[0].swapChain);
    //hr = ((int (__thiscall *)(IDirect3DDevice9 *, IDirect3DDevice9 *, unsigned int, int *))dx.device->GetSwapChain)(
    //             dx.device,
    //             dx.device,
    //             0,
    //             &dx.windows[0].width);
    if ( hr < 0 )
    {
        ++g_disableRendering;
        v1 = R_ErrorDescription(hr);
        Com_Error(
            ERR_FATAL,
            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_rendertarget.cpp (%i) dx.device->GetSwapChain( 0, &dx.windows[0].swa"
            "pChain ) failed: %s\n",
            1803,
            v1);
    }
    if ( r_logFile && r_logFile->current.integer )
        RB_LogPrint("dx.device->GetBackBuffer( 0, 0, D3DBACKBUFFER_TYPE_MONO, &renderTarget->surface.color )\n");

    v9 = dx.device->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &renderTarget->surface.color);
    //v9 = ((int (__thiscall *)(IDirect3DDevice9 *, IDirect3DDevice9 *, unsigned int, unsigned int, unsigned int, GfxRenderTargetSurface *))dx.device->GetBackBuffer)(
    //             dx.device,
    //             dx.device,
    //             0,
    //             0,
    //             0,
    //             &renderTarget->surface);
    if ( v9 < 0 )
    {
        ++g_disableRendering;
        v2 = R_ErrorDescription(v9);
        Com_Error(
            ERR_FATAL,
            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_rendertarget.cpp (%i) dx.device->GetBackBuffer( 0, 0, D3DBACKBUFFER_"
            "TYPE_MONO, &renderTarget->surface.color ) failed: %s\n",
            1808,
            v2);
    }
    if ( !renderTarget->surface.color
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_rendertarget.cpp",
                    1809,
                    0,
                    "%s",
                    "renderTarget->surface.color") )
    {
        __debugbreak();
    }
    if ( g_allocateMinimalResources )
    {
        renderTarget->surface.depthStencil = 0;
    }
    else
    {
        R_GetFrameBufferDepthStencilRes(&depthStencilWidth, &depthStencilHeight);
        if ( r_logFile && r_logFile->current.integer )
            RB_LogPrint(
                "dx.device->CreateDepthStencilSurface( depthStencilWidth, depthStencilHeight, dx.depthStencilFormat, dx.multiSamp"
                "leType, dx.multiSampleQuality, 0, &renderTarget->surface.depthStencil, 0 )\n");
        //v5 = ((int (__thiscall *)(IDirect3DDevice9 *, IDirect3DDevice9 *, int, int, _D3DFORMAT, _D3DMULTISAMPLE_TYPE, unsigned int, unsigned int, IDirect3DSurface9 **, unsigned int))dx.device->CreateDepthStencilSurface)(
        //             dx.device,
        //             dx.device,
        //             depthStencilWidth,
        //             depthStencilHeight,
        //             dx.depthStencilFormat,
        //             dx.multiSampleType,
        //             dx.multiSampleQuality,
        //             0,
        //             &renderTarget->surface.depthStencil,
        //             0);

        v5 = dx.device->CreateDepthStencilSurface(depthStencilWidth, depthStencilHeight, dx.depthStencilFormat, dx.multiSampleType, dx.multiSampleQuality, 0, &renderTarget->surface.depthStencil, 0);
        if ( v5 < 0 )
        {
            ++g_disableRendering;
            v3 = R_ErrorDescription(v5);
            Com_Error(
                ERR_FATAL,
                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_rendertarget.cpp (%i) dx.device->CreateDepthStencilSurface( depthS"
                "tencilWidth, depthStencilHeight, dx.depthStencilFormat, dx.multiSampleType, dx.multiSampleQuality, 0, &renderTar"
                "get->surface.depthStencil, 0 ) failed: %s\n",
                1821,
                v3);
        }
    }
}

void R_InitSceneNullRenderTarget_PC()
{
    IDirect3DSurface9 *nullTextureSurface; // [esp+4h] [ebp-Ch] BYREF
    HRESULT hr; // [esp+8h] [ebp-8h]
    _D3DFORMAT adapterFormat; // [esp+Ch] [ebp-4h]

    adapterFormat = D3DFMT_X8R8G8B8;
    nullTextureSurface = 0;
    dx.supportsSceneNullRenderTarget = 0;
    if ( dx.vendorId != 4098 && r_allow_null_rt->current.enabled )
    {
        hr = dx.d3d9->CheckDeviceFormat(
                     dx.adapterIndex,
                     D3DDEVTYPE_HAL,
                     D3DFMT_X8R8G8B8,
                     1u,
                     D3DRTYPE_SURFACE,
                     (_D3DFORMAT)1280070990);
        if ( !hr )
        {
            hr = dx.d3d9->CheckDepthStencilMatch(
                         dx.adapterIndex,
                         D3DDEVTYPE_HAL,
                         D3DFMT_X8R8G8B8,
                         (_D3DFORMAT)1280070990,
                         dx.depthStencilFormat);
            if ( !hr )
            {
                hr = dx.device->CreateRenderTarget(
                            gfxRenderTargets[R_RENDERTARGET_SCENE].width,
                            gfxRenderTargets[R_RENDERTARGET_SCENE].height,
                             (_D3DFORMAT)1280070990,
                             dx.multiSampleType,
                             dx.multiSampleQuality,
                             0,
                             &nullTextureSurface,
                             0);
                if ( !hr )
                {
                    dx.supportsSceneNullRenderTarget = 1;
                    R_ShareRenderTarget_ReplaceColor(3u, 4u, nullTextureSurface);
                    nullTextureSurface->Release();
                }
            }
        }
    }
}

void R_ShareRenderTarget_ReplaceColor(
    unsigned __int8 idFrom,
    unsigned __int8 idTo,
    IDirect3DSurface9 *colorSurface)
{
    GfxRenderTarget *v4; // edx
    GfxRenderTarget *v5; // eax
    GfxRenderTarget *renderTarget; // [esp+0h] [ebp-4h]

    renderTarget = &gfxRenderTargets[idTo];
    if (!renderTarget
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_rendertarget.cpp",
            4421,
            0,
            "%s",
            "renderTarget"))
    {
        __debugbreak();
    }
    if (renderTarget->image
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_rendertarget.cpp",
            4422,
            0,
            "%s",
            "renderTarget->image == NULL"))
    {
        __debugbreak();
    }
    if (renderTarget->surface.color
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_rendertarget.cpp",
            4424,
            0,
            "%s",
            "renderTarget->surface.color == NULL"))
    {
        __debugbreak();
    }
    if (renderTarget->surface.depthStencil
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_rendertarget.cpp",
            4425,
            0,
            "%s",
            "renderTarget->surface.depthStencil == NULL"))
    {
        __debugbreak();
    }
    if (renderTarget->width
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_rendertarget.cpp",
            4427,
            0,
            "%s",
            "renderTarget->width == 0"))
    {
        __debugbreak();
    }
    if (renderTarget->height
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_rendertarget.cpp",
            4428,
            0,
            "%s",
            "renderTarget->height == 0"))
    {
        __debugbreak();
    }
    v4 = &gfxRenderTargets[idFrom];
    v5 = &gfxRenderTargets[idTo];
    v5->image = v4->image;
    v5->surface.color = v4->surface.color;
    v5->surface.depthStencil = v4->surface.depthStencil;
    *(_DWORD *)&v5->width = *(_DWORD *)&v4->width;
    *(_DWORD *)&v5->cleared = *(_DWORD *)&v4->cleared;
    gfxRenderTargets[idTo].surface.color = colorSurface;
    gfxRenderTargets[idTo].surface.color->AddRef();
    if (gfxRenderTargets[idTo].surface.depthStencil)
        gfxRenderTargets[idTo].surface.depthStencil->AddRef();
}

void R_InitIntZRenderTarget_PC()
{
    GfxImage *image; // [esp+0h] [ebp-518h]
    NV_DISPLAY_DRIVER_VERSION driverVersion; // [esp+8h] [ebp-510h] BYREF
    unsigned __int8 stereoEnabled; // [esp+9Bh] [ebp-47Dh] BYREF
    NVDX_ObjectHandle__ *hNvDepthTexture; // [esp+9Ch] [ebp-47Ch] BYREF
    unsigned int acceptableDriver; // [esp+A0h] [ebp-478h]
    unsigned int mipCount; // [esp+A4h] [ebp-474h]
    _D3DADAPTER_IDENTIFIER9 adapterId; // [esp+A8h] [ebp-470h] BYREF
    unsigned int width; // [esp+4FCh] [ebp-1Ch]
    unsigned int height; // [esp+500h] [ebp-18h]
    _D3DFORMAT FOURCC_INTZ; // [esp+504h] [ebp-14h]
    unsigned int usage; // [esp+508h] [ebp-10h]
    _D3DFORMAT FOURCC_RESZ; // [esp+50Ch] [ebp-Ch]
    IDirect3DTexture9 *depthTexture; // [esp+510h] [ebp-8h] BYREF
    _D3DPOOL memPool; // [esp+514h] [ebp-4h]

    FOURCC_INTZ = (_D3DFORMAT)1515474505;
    FOURCC_RESZ = (_D3DFORMAT)1515406674;
    width = vidConfig.sceneWidth;
    height = vidConfig.sceneHeight;
    usage = 2;
    memPool = D3DPOOL_DEFAULT;
    mipCount = 1;
    depthTexture = 0;
    dx.supportsIntZ = 0;

    if ( !r_allow_intz->current.enabled || dx.d3d9->GetAdapterIdentifier(dx.adapterIndex, 0, &adapterId) )
        return;

    // BO1TODO: fix this INTZ thing, see if we can enable it on modern gpu's without nvapi (I believe the below code fails)
    // Update: this library works now along with the other fixes, seems to add about 10%~ performance
#ifdef BO1_NVAPI_INTZ
    if ( adapterId.VendorId == 4318 && dx.nvInitialized )
    {
        acceptableDriver = 728896;
        if ( adapterId.DriverVersion.LowPart < 0xB1F40 )
            return;
        stereoEnabled = 0;
        if ( !NvAPI_Stereo_IsEnabled(&stereoEnabled) )
        {
            if ( stereoEnabled )
            {
                driverVersion.version = 65676;
                if ( NvAPI_GetDisplayDriverVersion(0, &driverVersion) || driverVersion.drvVersion < 0x65BD )
                    return;
            }
        }
        if ( dx.device->CreateTexture(width, height, 1u, 2u, FOURCC_INTZ, D3DPOOL_DEFAULT, &depthTexture, 0) )
            return;
        if ( NvAPI_D3D9_GetTextureHandle(depthTexture, &hNvDepthTexture) )
        {
            depthTexture->Release();
            return;
        }
        if ( NvAPI_D3D9_StretchRect(dx.device, hNvDepthTexture, 0, hNvDepthTexture, 0, D3DTEXF_POINT) )
        {
            //((void (__thiscall *)(IDirect3DTexture9 *, IDirect3DTexture9 *))depthTexture->Release)(depthTexture, depthTexture);
            depthTexture->Release();
            return;
        }
        dx.nvDepthBufferHandle = 0;
        dx.nvFloatZBufferHandle = 0;
        dx.supportsIntZ = 1;
    }

    if ( adapterId.VendorId != 4098 && adapterId.VendorId != 32902 )
        goto LABEL_22;
#endif

    if ( !dx.d3d9->CheckDeviceFormat(dx.adapterIndex, D3DDEVTYPE_HAL, D3DFMT_X8R8G8B8, 2, D3DRTYPE_TEXTURE, FOURCC_INTZ)
        && !dx.d3d9->CheckDeviceFormat(dx.adapterIndex, D3DDEVTYPE_HAL, D3DFMT_X8R8G8B8, 1, D3DRTYPE_SURFACE, FOURCC_RESZ) )
    {
        if ( !dx.device->CreateTexture(
                        width,
                        height,
                        1,
                        2,
                        FOURCC_INTZ,
                        D3DPOOL_DEFAULT,
                        &depthTexture,
                        0) )
        {
            dx.supportsIntZ = 1;
LABEL_22:
            if ( dx.supportsIntZ )
            {
                image = Image_AllocProg(2, 6u, 0);
                image->texture.basemap = depthTexture;
                image->width = width;
                image->height = height;
                image->depth = 1;
                image->mapType = 3;
                gfxRenderTargets[R_RENDERTARGET_FLOAT_Z].image = image;
                gfxRenderTargets[R_RENDERTARGET_FLOAT_Z].width = width;
                gfxRenderTargets[R_RENDERTARGET_FLOAT_Z].height = height;
                Com_Printf(8, "INTZ format supported\n");
            }
        }
    }
}

void __cdecl R_ShutdownRenderTargets()
{
    int renderTargetId; // [esp+0h] [ebp-4h]

    for ( renderTargetId = 0; renderTargetId < 44; ++renderTargetId )
        R_ShutDownSingleRenderTarget(&gfxRenderTargets[renderTargetId]);
    dx.singleSampleDepthStencilSurface = 0;
}

void __cdecl R_ShutDownSingleRenderTarget(GfxRenderTarget *rt)
{
    if ( Sys_IsRenderThread() )
    {
        if ( rt->surface.color )
            rt->surface.color->Release();
        if ( rt->surface.depthStencil )
            rt->surface.depthStencil->Release();
    }
    else
    {
        if ( rt->surface.color )
            RB_Resource_Release(rt->surface.color);
        if ( rt->surface.depthStencil )
            RB_Resource_Release(rt->surface.depthStencil);
    }
    if ( rt->image )
        Image_Release(rt->image);
    rt->image = 0;
    rt->surface.color = 0;
    rt->surface.depthStencil = 0;
    *(unsigned int *)&rt->width = 0;
    *(unsigned int *)&rt->cleared = 0;
}

const char *__cdecl R_RenderTargetName(unsigned __int8 renderTargetId)
{
    return s_renderTargetNames[renderTargetId];
}

void __cdecl R_CheckExtraCamTargets()
{
    if ( g_extraCamSettings.enabled && !gfxRenderTargets[R_RENDERTARGET_MISSILE_CAM].image )
        R_InitExtraCamRenderTargets(g_extraCamSettings.width, g_extraCamSettings.height, g_extraCamSettings.location);
}

void __cdecl R_InitExtraCamRenderTargets(unsigned int width, unsigned int height, int location)
{
    _D3DSURFACE_DESC surfaceDesc; // [esp+0h] [ebp-24h] BYREF
    _D3DFORMAT backBufferFormat; // [esp+20h] [ebp-4h]

    g_extraCamSettings.enabled = 1;
    g_extraCamSettings.width = width;
    g_extraCamSettings.height = height;
    g_extraCamSettings.location = location;
    gfxRenderTargets[R_RENDERTARGET_FRAME_BUFFER].surface.color->GetDesc(&surfaceDesc);
    backBufferFormat = surfaceDesc.Format;
    R_InitSizedRenderTargetImage(
        20,
        width,
        height,
        0,
        surfaceDesc.Format,
        RENDERTARGET_USAGE_RENDER,
        &gfxRenderTargets[R_RENDERTARGET_MISSILE_CAM]);
    R_InitSizedRenderTargetImage(21, width, height, 0, D3DFMT_R32F, RENDERTARGET_USAGE_RENDER, &gfxRenderTargets[R_RENDERTARGET_FLOAT_Z_MISSILE_CAM]);
}

void R_InitSizedRenderTargetImage(
    int imageProgType,
    int fullscreenWidth,
    int fullscreenHeight,
    char picmip,
    _D3DFORMAT format,
    RenderTargetUsage usage,
    GfxRenderTarget *renderTarget)
{
    const char *v8; // eax
    unsigned __int16 v9; // [esp+0h] [ebp-20h]
    unsigned __int16 v10; // [esp+4h] [ebp-1Ch]
    int hr; // [esp+14h] [ebp-Ch]

    if (fullscreenWidth >> picmip > 1)
        v10 = fullscreenWidth >> picmip;
    else
        v10 = 1;
    if (fullscreenHeight >> picmip > 1)
        v9 = fullscreenHeight >> picmip;
    else
        v9 = 1;
    R_InitRenderTargetImage(imageProgType, v10, v9, format, usage, renderTarget);
    if (usage == RENDERTARGET_USAGE_RENDER_SHARE_SCENE)
    {
        if (!gfxRenderTargets[R_RENDERTARGET_SCENE].surface.depthStencil
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_rendertarget.cpp",
                1390,
                0,
                "%s",
                "gfxRenderTargets[R_RENDERTARGET_SCENE].surface.depthStencil"))
        {
            __debugbreak();
        }
        renderTarget->surface.depthStencil = gfxRenderTargets[R_RENDERTARGET_SCENE].surface.depthStencil;
        renderTarget->surface.depthStencil->AddRef();
        if (!gfxRenderTargets[R_RENDERTARGET_SCENE].surface.color
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_rendertarget.cpp",
                1394,
                0,
                "%s",
                "gfxRenderTargets[R_RENDERTARGET_SCENE].surface.color"))
        {
            __debugbreak();
        }
        renderTarget->surface.color = gfxRenderTargets[R_RENDERTARGET_SCENE].surface.color;
        renderTarget->surface.color->AddRef();
    }
    else if (usage == RENDERTARGET_USAGE_RENDER)
    {
        if (r_logFile && r_logFile->current.integer)
            RB_LogPrint(
                "dx.device->CreateDepthStencilSurface( fullscreenWidth, fullscreenHeight, dx.depthStencilFormat, D3DMULTISAMPLE_N"
                "ONE, 0, 0, &renderTarget->surface.depthStencil, 0 )\n");
        hr = dx.device->CreateDepthStencilSurface(
            fullscreenWidth,
            fullscreenHeight,
            dx.depthStencilFormat,
            D3DMULTISAMPLE_NONE,
            0,
            0,
            &renderTarget->surface.depthStencil,
            0);
        if (hr < 0)
        {
            ++g_disableRendering;
            v8 = R_ErrorDescription(hr);
            Com_Error(
                ERR_FATAL,
                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_rendertarget.cpp (%i) dx.device->CreateDepthStencilSurface( fullsc"
                "reenWidth, fullscreenHeight, dx.depthStencilFormat, D3DMULTISAMPLE_NONE, 0, 0, &renderTarget->surface.depthStenc"
                "il, 0 ) failed: %s\n",
                1436,
                v8);
        }
    }
}

void __cdecl R_ShutdownExtraCamRenderTargets()
{
    g_extraCamSettings.enabled = 0;
    R_ShutDownSingleRenderTarget(&gfxRenderTargets[R_RENDERTARGET_MISSILE_CAM]);
    R_ShutDownSingleRenderTarget(&gfxRenderTargets[R_RENDERTARGET_FLOAT_Z_MISSILE_CAM]);
}

void R_InitUI3DRenderTarget(
    unsigned __int16 width,
    unsigned __int16 height,
    int location,
    bool allocatePingPongBuffer)
{

    R_InitSizedRenderTargetImage(
        18,
        width,
        height,
        0,
        D3DFMT_A8R8G8B8,
        RENDERTARGET_USAGE_RENDER,
        &gfxRenderTargets[R_RENDERTARGET_UI3D]);
    if (allocatePingPongBuffer)
        R_InitSizedRenderTargetImage(
            19,
            width,
            height,
            0,
            D3DFMT_A8R8G8B8,
            RENDERTARGET_USAGE_RENDER,
            &gfxRenderTargets[R_RENDERTARGET_UI3D_PING_PONG]);
}

void __cdecl R_ShutdownUI3DRenderTarget(bool freePingPongBuffer)
{
    if (freePingPongBuffer)
        R_ShutDownSingleRenderTarget(&gfxRenderTargets[R_RENDERTARGET_UI3D_PING_PONG]);
    R_ShutDownSingleRenderTarget(&gfxRenderTargets[R_RENDERTARGET_UI3D]);
}
