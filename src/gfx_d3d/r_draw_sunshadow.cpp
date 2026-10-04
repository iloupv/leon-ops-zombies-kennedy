#include "r_draw_sunshadow.h"
#include "r_state_utils.h"
#include "r_wind.h"
#include "r_state.h"
#include "r_foliage.h"
#include "r_dvars.h"
#include "rb_draw3d.h"
#include "rb_spotshadow.h"

void    R_DrawSunShadowMap(
                const GfxViewInfo *viewInfo,
                unsigned int partitionIndex,
                GfxCmdBuf *cmdBuf)
{
    float x; // [esp+20h] [ebp-1ABCh]
    const GfxBackEndData *data; // [esp+2Ch] [ebp-1AB0h]
    GfxCmdBufSourceState state; // [esp+30h] [ebp-1AACh] BYREF

    R_InitCmdBufSourceState(&state, &viewInfo->input, 0);
    R_SetWindShaderConstants(&state);
    R_SetRenderTargetSize(&state, R_RENDERTARGET_SHADOWMAP_SUN);

    data = viewInfo->input.data;
    x = data->sunShadow.partition[partitionIndex].shadowViewParms.projectionMatrix.m[2][2];

    R_UpdateCodeConstant(
        &state,
        CONST_SRC_CODE_SHADOWMAP_POLYGON_OFFSET,
        (sm_polygonOffsetBias->current.value * 0.25) * x,
        sm_polygonOffsetScale->current.value,
        0.0,
        0.0);

    R_SetViewportValues(&state, 0, partitionIndex * dx.sunShadowmapSize, dx.sunShadowmapSize, dx.sunShadowmapSize);

    SunShadowMapCallbackUserData userData;
    userData.partition = &data->sunShadow.partition[partitionIndex];
    userData.drawList = &viewInfo->drawList[DRAWLIST_SUN_SHADOW_MAP0 + partitionIndex];
    
    R_DrawCall(
        R_DrawSunShadowMapCallback, 
        &userData,
        &state, 
        viewInfo, 
        (GfxDrawSurfListInfo*)&viewInfo->drawList[DRAWLIST_SUN_SHADOW_MAP0 + partitionIndex], // de-const cast
        &data->sunShadow.partition[partitionIndex].shadowViewParms,
        cmdBuf, 
        0);
}

void __cdecl R_DrawSunShadowMapCallback(const void *userData, GfxCmdBufContext context, GfxCmdBufContext prepassContext)
{
    const GfxDrawSurfListInfo *drawList; // [esp+34h] [ebp-8h]
    const GfxSunShadowPartition *partition; // [esp+38h] [ebp-4h]

    const SunShadowMapCallbackUserData *data = (const SunShadowMapCallbackUserData *)userData;

    partition = data->partition;
    drawList = data->drawList;

    R_SetRenderTarget(context, R_RENDERTARGET_SHADOWMAP_SUN);
    if ( !partition->partitionIndex )
        R_ClearScreen(context.state->prim.device, 3u, shadowmapClearColor, 1.0, 0, 0);
    R_HW_EnableScissor(
        context.state->prim.device,
        partition->viewport.x,
        partition->viewport.y + partition->partitionIndex * dx.sunShadowmapSize,
        partition->viewport.width,
        partition->viewport.height);
    R_DrawSurfs(context, 0, drawList);
    R_HW_DisableScissor(context.state->prim.device);
}

