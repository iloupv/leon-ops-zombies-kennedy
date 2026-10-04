#pragma once
#include "rb_state.h"
#include "r_rendercmds.h"

enum CloakPhaseID : __int32
{
    CLOAK_PHASE_PREPASS = 0x0,
    CLOAK_PHASE_CLOAKED = 0x1,
};

enum LitPhaseID : __int32
{
    LIT_PHASE_MAIN         = 0x0,
    LIT_PHASE_POST_RESOLVE = 0x1,
    LIT_PHASE_PUNCHTHROUGH = 0x2,
};

// local variable allocation has failed, the output may be wrong!
void    R_SetDrawLitConstants(
                GfxCmdBufSourceState *source,
                const GfxViewInfo *viewInfo,
                const GfxViewParms *viewParms);
void    R_DrawCloakHDR(const GfxViewInfo *viewInfo, GfxCmdBuf *cmdBuf, CloakPhaseID phase);
void __cdecl R_DrawCloakPrePassCallbackHDR(
                const void *userData,
                GfxCmdBufContext context,
                GfxCmdBufContext prepassContext);
void __cdecl R_DrawCloakPostEmissiveCallbackHDR(
                const void *userData,
                GfxCmdBufContext context,
                GfxCmdBufContext prepassContext);
void    R_DrawLit(
                const GfxViewInfo *viewInfo,
                GfxCmdBuf *cmdBuf,
                GfxCmdBuf *prepassCmdBuf,
                LitPhaseID phase);
void __cdecl R_DrawLitCallback(const void *userData, GfxCmdBufContext context, GfxCmdBufContext prepassContext);
void __cdecl R_DrawLitPostResolveCallback(const void *userData, GfxCmdBufContext context, GfxCmdBufContext prepassContext);
void    R_DrawDecal(const GfxViewInfo *viewInfo, GfxCmdBuf *cmdBuf, GfxCmdBuf *prepassCmdBuf);
void __cdecl R_DrawDecalCallback(const void *userData, GfxCmdBufContext context, GfxCmdBufContext prepassContext);
