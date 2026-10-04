#include "r_extracam.h"
#include <database/db_registry.h>
#include <qcommon/common.h>
#include <universal/physicalmemory.h>
#include "r_init.h"
#include "r_rendertarget.h"
#include "rb_backend.h"
#include "r_dvars.h"

ExtraCamConfig g_extraCamConfig;

double __cdecl R_ExtraCam_AspectRatio()
{
    return g_extraCamConfig.aspectRatio;
}

void __cdecl R_ExtraCam_Init(int location)
{
    int extracamResolution; // [esp+0h] [ebp-14h]
    XGlobals *xGlobals; // [esp+Ch] [ebp-8h]
    eExtraCamResolution requestedRes; // [esp+10h] [ebp-4h]

    if ( !g_extraCamConfig.initialized )
    {
        requestedRes = EXTRACAM_RES_NORMAL;
        if ( useFastFile->current.enabled )
        {
            xGlobals = DB_FindXAssetHeader(ASSET_TYPE_XGLOBALS, (char*)"xGlobalsSingleton", 0, -1).xGlobals;
            if ( xGlobals )
                extracamResolution = xGlobals->extracamResolution;
            else
                extracamResolution = 1;
            requestedRes = (eExtraCamResolution)extracamResolution;
        }
        if ( requestedRes )
        {
            PMem_BeginAlloc("ExtraCamHeap", location, TRACK_MISC);
            CreateExtraCamRenderTargets(requestedRes, location);
            PMem_EndAlloc("ExtraCamHeap", location);
            g_extraCamConfig.initialized = 1;
        }
    }
}

void __cdecl CreateExtraCamRenderTargets(eExtraCamResolution resConfig, int location)
{
    unsigned int width; // [esp+44h] [ebp-Ch]
    unsigned int height; // [esp+48h] [ebp-8h]

    width = 0;
    height = 0;
    if ( resConfig == EXTRACAM_RES_NORMAL )
    {
        width = 256;
        height = 256;
    }
    else if ( resConfig == EXTRACAM_RES_HIGH )
    {
        width = (__int64)((double)vidConfig.sceneWidth * 0.60000002) & 0xFFFFFFF8;
        height = (__int64)((double)vidConfig.sceneHeight * 0.60000002) & 0xFFFFFFF8;
    }
    else if ( !Assert_MyHandler(
                             "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_extracam.cpp",
                             46,
                             0,
                             "invalid extra cam config") )
    {
        __debugbreak();
    }
    R_InitExtraCamRenderTargets(width, height, location);
    g_extraCamConfig.renderTargetWidth = width;
    g_extraCamConfig.renderTargetHeight = height;
    g_extraCamConfig.aspectRatio = (double)width / (double)height;
}

void __cdecl R_ExtraCam_Shutdown()
{
    if ( g_extraCamConfig.initialized )
    {
        R_ShutdownExtraCamRenderTargets();
        PMem_Free("ExtraCamHeap");
        g_extraCamConfig.initialized = 0;
    }
}

unsigned int __cdecl R_ExtraCam_RenderTargetWidth()
{
    return g_extraCamConfig.renderTargetWidth;
}

unsigned int __cdecl R_ExtraCam_RenderTargetHeight()
{
    return g_extraCamConfig.renderTargetHeight;
}

bool __cdecl R_ExtraCam_GfxResourcesReady()
{
    // KB fix: the decompiled "rt" was gfxRenderTargets[R_RENDERTARGET_MISSILE_CAM] (SP 0x006cec00 reads its image at 0x045eb3a0)
    return g_extraCamConfig.initialized && gfxRenderTargets[R_RENDERTARGET_MISSILE_CAM].image;
}

bool __cdecl R_ExtraCam_UseShadowmap()
{
    return r_extracam_shadowmap_enable->current.enabled;
}

