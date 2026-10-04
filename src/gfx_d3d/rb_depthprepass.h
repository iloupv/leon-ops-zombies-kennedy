#pragma once
#include "r_rendercmds.h"
#include "rb_state.h"

void    R_DepthPrepass(
                GfxRenderTargetId renderTargetId,
                const GfxViewInfo *viewInfo,
                GfxCmdBuf *cmdBuf);
void __cdecl R_DepthPrepassCallback(const void *userData, GfxCmdBufContext context, GfxCmdBufContext prepassContext);
