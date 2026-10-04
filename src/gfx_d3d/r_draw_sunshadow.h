#pragma once
#include "r_rendercmds.h"
#include "rb_state.h"

struct SunShadowMapCallbackUserData
{
    const GfxSunShadowPartition *partition;
    const GfxDrawSurfListInfo *drawList;
};

void    R_DrawSunShadowMap(
                const GfxViewInfo *viewInfo,
                unsigned int partitionIndex,
                GfxCmdBuf *cmdBuf);
void __cdecl R_DrawSunShadowMapCallback(const void *userData, GfxCmdBufContext context, GfxCmdBufContext prepassContext);
