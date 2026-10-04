#include "rb_spotshadow.h"
#include "r_state.h"
#include "rb_backend.h"
#include "rb_pixelcost.h"
#include "r_cmdbuf.h"
#include "r_dvars.h"
#include "r_foliage.h"
#include "r_state_utils.h"
#include "r_wind.h"
#include "rb_draw3d.h"
#include "rb_shade.h"
#include "rb_sunshadow.h"

void __cdecl R_DrawSpotShadowMapCallback(const void *userData, GfxCmdBufContext context, GfxCmdBufContext prepassContext)
{
    const GfxDrawSurfListInfo *drawList; // [esp+Ch] [ebp-8h]
    const GfxSpotShadow *spotShadow; // [esp+10h] [ebp-4h]

    const SpotShadowMapCallbackUserData *data = (const SpotShadowMapCallbackUserData *)userData;

    spotShadow = data->shadow;
    drawList = data->drawList;

    R_SetRenderTarget(context, spotShadow->renderTargetId);
    if ( spotShadow->clearScreen )
        R_ClearScreen(context.state->prim.device, 3u, shadowmapClearColor, 1.0, 0, 0);
    if ( !gfxMetrics.hasHardwareShadowmap )
        R_DrawQuadMesh(context, rgp.shadowClearMaterial, spotShadow->clearMesh);
    R_DrawSurfs(context, 0, drawList);
}

void __cdecl RB_SpotShadowMaps(const GfxBackEndData *data, const GfxViewInfo *viewInfo)
{
    GfxCmdBuf cmdBuf; // [esp+1Ch] [ebp-4h] BYREF
    int savedregs; // [esp+20h] [ebp+0h] BYREF

    PROF_SCOPED("RB_SpotShadowMaps");

    if ( pixelCostMode )
    {
        return;
    }
    else
    {
        if ( !data
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_spotshadow.cpp", 302, 0, "%s", "data") )
        {
            __debugbreak();
        }
        R_InitContext(data, &cmdBuf);

        PROF_SCOPED("Spot Shadow Maps");
        R_DrawSpotShadowMapArray(viewInfo, &cmdBuf);
    }
}

void    R_DrawSpotShadowMapArray(const GfxViewInfo *viewInfo, GfxCmdBuf *cmdBuf)
{
    GfxCmdBufSourceState state; // [esp+30h] [ebp-1AACh] BYREF

    R_InitCmdBufSourceState(&state, &viewInfo->input, 0);
    R_SetWindShaderConstants(&state);

    const GfxBackEndData *data = viewInfo->input.data;

    for ( unsigned int i = 0; i < data->spotShadowCount; ++i )
    {
        const GfxSpotShadow *shadow = &data->spotShadows[i];
        const GfxDrawSurfListInfo *list = &viewInfo->drawList[DRAWLIST_SPOT_SHADOW_MAP0 + i];

        R_SetRenderTargetSize(&state, data->spotShadows[i].renderTargetId);
        R_UpdateCodeConstant(
            &state,
            CONST_SRC_CODE_SHADOWMAP_POLYGON_OFFSET,
            sm_polygonOffsetBias->current.value * 0.25,
            sm_polygonOffsetScale->current.value,
            0.0,
            0.0);
        R_SetViewportValues(&state, shadow->viewport.x, shadow->viewport.y, shadow->viewport.width, shadow->viewport.height);

        SpotShadowMapCallbackUserData userData;
        userData.shadow = shadow;
        userData.drawList = list;

        R_DrawCall(
            R_DrawSpotShadowMapCallback,
            &userData,
            &state,
            viewInfo,
            list,
            &shadow->shadowViewParms,
            cmdBuf,
            0);
    }
}

void __cdecl RB_DrawSpotShadowOverlay()
{
    float x; // [esp+0h] [ebp-6Ch]
    float t0; // [esp+40h] [ebp-2Ch]
    unsigned int spotShadowIndex; // [esp+60h] [ebp-Ch]
    float h; // [esp+64h] [ebp-8h]

    if (!backEndData->viewInfoCount
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_spotshadow.cpp",
            375,
            0,
            "%s",
            "backEndData->viewInfoCount > 0"))
    {
        __debugbreak();
    }
    if (backEndData->spotShadowCount)
    {
        h = (double)vidConfig.displayHeight * 0.25;
        gfxCmdBufSourceState.input.codeImageSamplerStates[TEXTURE_SRC_CODE_FEEDBACK] = 97;
        R_SetCodeImageTexture(&gfxCmdBufSourceState, 8u, gfxRenderTargets[R_RENDERTARGET_SHADOWMAP_SPOT].image);
        for (spotShadowIndex = 0; spotShadowIndex < backEndData->spotShadowCount; ++spotShadowIndex)
        {
            t0 = (double)spotShadowIndex * 0.25;
            RB_SetSpotShadowOverlayScaleAndBias(&backEndData->spotShadows[spotShadowIndex]);
            x = (double)spotShadowIndex * (h + 2.0) + 4.0;
            RB_DrawStretchPic(
                rgp.shadowOverlayMaterial,
                x,
                4.0,
                h,
                h,
                0.0,
                t0,
                1.0,
                t0 + 0.25,
                0xFFFFFFFF,
                GFX_PRIM_STATS_HUD);
            RB_EndTessSurface();
        }
        gfxCmdBufSourceState.input.codeImageSamplerStates[TEXTURE_SRC_CODE_FEEDBACK] = 98;
    }
}


void __cdecl RB_SetSpotShadowOverlayScaleAndBias(const GfxSpotShadow *spotShadow)
{
    float nearDepth; // [esp+20h] [ebp-1Ch] BYREF
    unsigned __int8 shadowmapBuildTechType; // [esp+27h] [ebp-15h]
    float zNear; // [esp+28h] [ebp-14h]
    float zFar; // [esp+30h] [ebp-Ch]
    float farDepth; // [esp+38h] [ebp-4h] BYREF

    RB_GetShadowOverlayDepthBounds(&nearDepth, &farDepth);
    zNear = (-(spotShadow->shadowViewParms.projectionMatrix.m[3][2])) / spotShadow->shadowViewParms.projectionMatrix.m[2][2];
    zFar = spotShadow->light->radius;
    shadowmapBuildTechType = gfxMetrics.shadowmapBuildTechType;
    if (gfxMetrics.shadowmapBuildTechType == 3)
        R_UpdateCodeConstant(
            &gfxCmdBufSourceState,
            CONST_SRC_CODE_FILTER_TAP_0,
            1.0 / (float)((float)(zFar - zNear) * (float)(farDepth - nearDepth)),
            (-(1.0 / (float)((float)(zFar - zNear) * (float)(farDepth - nearDepth))))
            * (float)((float)((float)(zFar - zNear) * nearDepth) + zNear),
            1.0,
            1.0);
    else
        R_UpdateCodeConstant(
            &gfxCmdBufSourceState,
            CONST_SRC_CODE_FILTER_TAP_0,
            1.0 / (float)(farDepth - nearDepth),
            (-(1.0 / (float)(farDepth - nearDepth))) * nearDepth,
            zNear,
            zFar);
}

