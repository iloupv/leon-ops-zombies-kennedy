#include "rb_sunshadow.h"
#include "rb_pixelcost.h"
#include "r_cmdbuf.h"
#include "r_draw_sunshadow.h"
#include "r_dvars.h"
#include "rb_backend.h"
#include "rb_draw3d.h"
#include "rb_shade.h"
#include "r_sunshadow.h"
#include "r_foliage.h"

GfxPointVertex g_overlayPoints[36];

void __cdecl RB_SunShadowMaps(const GfxBackEndData *data, const GfxViewInfo *viewInfo)
{
    GfxCmdBuf cmdBuf; // [esp+24h] [ebp-8h] BYREF
    int partitionIndex; // [esp+28h] [ebp-4h]
    int savedregs; // [esp+2Ch] [ebp+0h] BYREF

    PROF_SCOPED("RB_SunShadowMaps");

    if ( pixelCostMode )
    {
        return;
    }
    else
    {
        if ( !data
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_sunshadow.cpp", 130, 0, "%s", "data") )
        {
            __debugbreak();
        }
        R_InitContext(data, &cmdBuf);

        PROF_SCOPED("Sun Shadow Maps");
        for ( partitionIndex = 0; partitionIndex < 2; ++partitionIndex )
        {
            PROF_SCOPED_RUNTIME_NAME(va("Sun Shadow Map %d", partitionIndex));
            R_DrawSunShadowMap(viewInfo, partitionIndex, &cmdBuf);
        }
    }
}

void __cdecl RB_GetShadowOverlayDepthBounds(float *nearDepth, float *farDepth)
{
    *nearDepth = sm_showOverlayDepthBounds->current.value;
    *farDepth = sm_showOverlayDepthBounds->current.vector[1];
    if ( fabs(*farDepth - *nearDepth) < 0.0099999998 )
    {
        if ( *farDepth <= *nearDepth )
        {
            *farDepth = (float)((float)(*nearDepth + *farDepth) * 0.5) - 0.0049999999;
            *nearDepth = *farDepth + 0.0099999998;
        }
        else
        {
            *nearDepth = (float)((float)(*nearDepth + *farDepth) * 0.5) - 0.0049999999;
            *farDepth = *nearDepth + 0.0099999998;
        }
    }
}

void __cdecl RB_DrawSunShadowOverlay()
{
    float clipSpacePoints[9][2]; // [esp+48h] [ebp-A0h] BYREF
    int pointIsNear[9]; // [esp+94h] [ebp-54h] BYREF
    float shadowSampleSize; // [esp+B8h] [ebp-30h]
    const GfxViewInfo *viewInfo; // [esp+BCh] [ebp-2Ch]
    int pointIndexDst; // [esp+C0h] [ebp-28h]
    int pointIndexSrc; // [esp+C4h] [ebp-24h]
    int partitionIndex; // [esp+C8h] [ebp-20h]
    float y0; // [esp+CCh] [ebp-1Ch]
    float x; // [esp+D0h] [ebp-18h]
    float y; // [esp+D4h] [ebp-14h]
    float h; // [esp+D8h] [ebp-10h]
    const GfxSunShadow *sunShadow; // [esp+DCh] [ebp-Ch]
    const GfxSunShadowPartition *partition; // [esp+E0h] [ebp-8h]
    float w; // [esp+E4h] [ebp-4h]

    if (!backEndData->viewInfoCount
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_sunshadow.cpp",
            207,
            0,
            "%s",
            "backEndData->viewInfoCount > 0"))
    {
        __debugbreak();
    }
    viewInfo = backEndData->viewInfo;
    sunShadow = &backEndData->sunShadow;
    y0 = 4.0f;
    h = (double)vidConfig.displayHeight * 0.5;
    w = h;
    RB_SetSunShadowOverlayScaleAndBias();
    gfxCmdBufSourceState.input.codeImageSamplerStates[TEXTURE_SRC_CODE_FEEDBACK] = 97;
    R_SetCodeImageTexture(&gfxCmdBufSourceState, 8u, gfxRenderTargets[R_RENDERTARGET_SHADOWMAP_SUN].image);
    for (partitionIndex = 0; partitionIndex < 2; ++partitionIndex)
        RB_DrawStretchPic(
            rgp.shadowOverlayMaterial,
            (float)((float)partitionIndex * w) + 4.0,
            y0,
            w,
            h,
            0.0,
            (float)partitionIndex * 0.5,
            1.0,
            (float)((float)partitionIndex * 0.5) + 0.5,
            0xFFFFFFFF,
            GFX_PRIM_STATS_HUD);
    RB_EndTessSurface();
    gfxCmdBufSourceState.input.codeImageSamplerStates[TEXTURE_SRC_CODE_FEEDBACK] = 98;
    shadowSampleSize = sm_sunSampleSizeNear->current.value;
    pointIndexDst = 0;
    for (partitionIndex = 0; partitionIndex < 2; ++partitionIndex)
    {
        x = (float)((float)partitionIndex * w) + 4.0;
        y = y0;
        partition = &sunShadow->partition[partitionIndex];
        R_SunShadowMapBoundingPoly(
            &sunShadow->partition[partitionIndex].boundingPoly,
            shadowSampleSize,
            (float(*)[9][2])clipSpacePoints,
            pointIsNear,
            dx.sunShadowmapSize);
        for (pointIndexSrc = 0; pointIndexSrc < partition->boundingPoly.pointCount; ++pointIndexSrc)
        {
            if (!partitionIndex
                || !rg.sunShadowFull
                || pointIsNear[pointIndexSrc]
                || pointIsNear[(pointIndexSrc + 1) % partition->boundingPoly.pointCount])
            {
                RB_SunShadowOverlayPoint(clipSpacePoints[pointIndexSrc], x, y, w, h, g_overlayPoints[pointIndexDst].xyz);
                RB_SunShadowOverlayPoint(
                    clipSpacePoints[(pointIndexSrc + 1) % partition->boundingPoly.pointCount],
                    x,
                    y,
                    w,
                    h,
                    g_overlayPoints[pointIndexDst + 1].xyz);
                *(_DWORD *)g_overlayPoints[pointIndexDst].color = -16711936;
                *(_DWORD *)g_overlayPoints[pointIndexDst + 1].color = -16711936;
                pointIndexDst += 2;
            }
        }
        shadowSampleSize = shadowSampleSize * rg.sunShadowPartitionRatio;
    }
    if (pointIndexDst)
    {
        RB_DrawLines2D(pointIndexDst / 2, 1, g_overlayPoints);
        RB_EndTessSurface();
    }
}

void __cdecl RB_SunShadowOverlayPoint(const float *xy, float x0, float y0, float w, float h, float *point)
{
    *point = (float)((float)((float)(0.5 * *xy) + 0.5) * w) + x0;
    point[1] = (float)((float)(0.5 - (float)(0.5 * xy[1])) * h) + y0;
    point[2] = 0.0f;
}

int RB_SetSunShadowOverlayScaleAndBias()
{
  float nearDepth; // [esp+18h] [ebp-10h] BYREF
  float farDepth; // [esp+24h] [ebp-4h] BYREF

  RB_GetShadowOverlayDepthBounds(&nearDepth, &farDepth);
  return R_UpdateCodeConstant(
           &gfxCmdBufSourceState,
           CONST_SRC_CODE_FILTER_TAP_0,
           1.0 / (float)(farDepth - nearDepth),
           //COERCE_FLOAT(COERCE_UNSIGNED_INT(1.0 / (float)(farDepth - nearDepth)) ^ _mask__NegFloat_) * nearDepth,
           (-(1.0 / (float)(farDepth - nearDepth))) * nearDepth,
           1.0,
           1.0);
}

