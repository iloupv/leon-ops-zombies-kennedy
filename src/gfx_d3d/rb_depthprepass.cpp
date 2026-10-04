#include "rb_depthprepass.h"
#include "r_dvars.h"
#include "r_state_utils.h"
#include "r_state.h"
#include "r_adszscale.h"
#include "rb_draw3d.h"
#include "rb_backend.h"
#include "r_foliage.h"
#include "r_wind.h"

void    R_DepthPrepass(
                GfxRenderTargetId renderTargetId,
                const GfxViewInfo *viewInfo,
                GfxCmdBuf *cmdBuf)
{
    GfxCmdBufSourceState source; // [sp+50h] [-F00h] BYREF

    if ( !r_reflectionProbeGenerate->current.enabled )
    {
        R_InitCmdBufSourceState(&source, &viewInfo->input, 1);
        R_SetRenderTargetSize(&source, renderTargetId);
        R_SetViewportStruct(&source, &viewInfo->cullViewInfo.sceneViewport);
        R_SetADSZScaleConstants(viewInfo->localClientNum, &source);
        R_DrawCall(
            R_DepthPrepassCallback,
            viewInfo,
            &source,
            viewInfo,
            (GfxDrawSurfListInfo *)0,
            &viewInfo->cullViewInfo.viewParms,
            cmdBuf,
            0);
    }
}

void __cdecl R_DepthPrepassCallback(const void *userData, GfxCmdBufContext context, GfxCmdBufContext prepassContext)
{
    int v2; // [esp+44h] [ebp-64h]
    jqBatchGroup *i; // [esp+48h] [ebp-60h]
    GfxDrawSurfListInfo info; // [esp+50h] [ebp-58h] BYREF
    unsigned __int8 baseTechType; // [esp+A3h] [ebp-5h]
    const GfxViewInfo *viewInfo; // [esp+A4h] [ebp-4h]

    viewInfo = (const GfxViewInfo *)userData;

    if ((viewInfo->sceneComposition.renderingMode & 7) == 0)
    {
        R_HW_EnableScissor(
            context.state->prim.device,
            viewInfo->cullViewInfo.scissorViewport.x,
            viewInfo->cullViewInfo.scissorViewport.y,
            viewInfo->cullViewInfo.scissorViewport.width,
            viewInfo->cullViewInfo.scissorViewport.height);
    }

    if ( viewInfo->renderingFloatZ )
    {
        if ( viewInfo->isMissileCamera )
        {
            R_SetRenderTarget(context, R_RENDERTARGET_FLOAT_Z_MISSILE_CAM);
        }
        else
        {
            if (!gfxRenderTargets[viewInfo->isMissileCamera ? R_RENDERTARGET_FLOAT_Z_MISSILE_CAM : R_RENDERTARGET_FLOAT_Z].surface.color
                && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_depthprepass.cpp",
                    213,
                    0,
                    "%s",
                    "R_HaveFloatZ(viewInfo->isMissileCamera)"))
            {
                __debugbreak();
            }
            R_SetRenderTarget(context, R_RENDERTARGET_FLOAT_Z);
        }
        baseTechType = 1;
        R_DrawQuadMesh(context, rgp.shadowClearMaterial, &viewInfo->fullSceneViewMesh->meshData);
        R_SetCodeConstant(context.source, CONST_SRC_CODE_DEPTH_FROM_CLIP, 0.0f, 0.0f, 0.0f, 1.0f);
    }
    else
    {
        R_SetRenderTarget(context, dx.supportsSceneNullRenderTarget ? R_RENDERTARGET_SCENE_NULLCOLOR : R_RENDERTARGET_SCENE);
        baseTechType = 0;
    }

    R_SetWindShaderConstants(context.source);

    info = viewInfo->drawList[DRAWLIST_DECAL];
    info.baseTechType = baseTechType;

    {
        PROF_SCOPED("zprepass decalInfo");
        R_DrawSurfs(context, 0, &info);
    }
    
    info = viewInfo->drawList[DRAWLIST_LIT];
    info.baseTechType = baseTechType;

    {
        PROF_SCOPED("zprepass litInfo");
        R_DrawSurfs(context, 0, &info);
    }
    
    if ( (viewInfo->sceneComposition.renderingMode & 7) == 0 )
        R_HW_DisableScissor(context.state->prim.device);
}

