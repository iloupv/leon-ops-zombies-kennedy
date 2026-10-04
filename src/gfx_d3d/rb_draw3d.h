#pragma once
#include "rb_state.h"
#include "r_rendercmds.h"

struct GfxPointLitSurfsInfo // sizeof=0x1C
{                                       // XREF: R_DrawPointLitSurfs/r
    const GfxViewInfo *viewInfo;        // XREF: R_DrawPointLitSurfs+9F/w
    GfxMeshData *clearQuadMesh;         // XREF: R_DrawPointLitSurfs+FD/w
                                        // R_DrawPointLitSurfs+81C/r
    const GfxDrawSurfListInfo *drawSurfInfo;
                                        // XREF: R_DrawPointLitSurfs+E7/w
    unsigned int x;                     // XREF: R_DrawPointLitSurfs+6CF/w
                                        // R_DrawPointLitSurfs+74F/r ...
    unsigned int y;                     // XREF: R_DrawPointLitSurfs+6F7/w
                                        // R_DrawPointLitSurfs+7AA/r ...
    unsigned int w;                     // XREF: R_DrawPointLitSurfs+755/w
    unsigned int h;                     // XREF: R_DrawPointLitSurfs+7B0/w
};

const float (*__cdecl R_GetCloudArea())[4];
void __cdecl R_CalcGameTimeVec(float gameTime, float *out);
void __cdecl R_ShowTris(GfxCmdBufContext context, const GfxDrawSurfListInfo *info);
void    R_DrawEmissive(const GfxViewInfo *viewInfo, GfxCmdBuf *cmdBuf);
void __cdecl R_DrawEmissiveCallback(const void *userData, GfxCmdBufContext context, GfxCmdBufContext prepassContext);
void __cdecl R_HW_DisableScissor(IDirect3DDevice9 *device);
void    R_DrawReflected(const GfxViewInfo *viewInfo, GfxCmdBuf *cmdBuf);
void __cdecl R_DrawReflectedCallback(const void *userData, GfxCmdBufContext context, GfxCmdBufContext prepassContext);
void __cdecl R_InitLocalCmdBufState(GfxCmdBufState *state);
bool __cdecl RB_ShouldDrawCoronas();
void __cdecl RB_Draw3DInternal(GfxViewInfo *viewInfo);
void __cdecl RB_FullbrightDrawCommands(const GfxViewInfo *viewInfo);
void __cdecl RB_EndSceneRendering(GfxCmdBufContext context, const GfxCmdBufInput *input, const GfxViewInfo *viewInfo);
void __cdecl R_SetAndClearSceneTarget(const GfxViewport *viewport, const GfxViewInfo *viewInfo);
void __cdecl R_ClearForFrameBuffer(IDirect3DDevice9 *device, const GfxViewport *viewport);
void __cdecl R_DrawFullbright(const GfxViewInfo *viewInfo, GfxCmdBufInput *input, GfxCmdBuf *cmdBuf);
void __cdecl R_DrawFullbrightLitCallback(const void *userData, GfxCmdBufContext context, GfxCmdBufContext prepassContext);
void __cdecl R_HW_EnableScissor(
                IDirect3DDevice9 *device,
                unsigned int x,
                unsigned int y,
                unsigned int w,
                unsigned int h);
void __cdecl R_DrawFullbrightDecalCallback(const void *userData, GfxCmdBufContext context, GfxCmdBufContext prepassContext);
void __cdecl R_DrawFullbrightEmissiveCallback(const void *userData, GfxCmdBufContext context, GfxCmdBufContext prepassContext);
void    R_DrawFullbrightOrDebugShader(
                void(__cdecl *callback)(const void *, GfxCmdBufContext, GfxCmdBufContext),
                const GfxViewInfo *viewInfo,
                const GfxDrawSurfListInfo *info,
                GfxCmdBuf *cmdBuf);
void __cdecl RB_DebugShaderDrawCommands(const GfxViewInfo *viewInfo);
void __cdecl R_DrawDebugShader(const GfxViewInfo *viewInfo, GfxCmdBuf *cmdBuf);
void __cdecl R_DrawDebugShaderLitCallback(const void *userData, GfxCmdBufContext context, GfxCmdBufContext prepassContext);
void __cdecl R_DrawDebugShaderDecalCallback(const void *userData, GfxCmdBufContext context, GfxCmdBufContext prepassContext);
void __cdecl R_DrawDebugShaderEmissiveCallback(const void *userData, GfxCmdBufContext context, GfxCmdBufContext prepassContext);
void __cdecl RB_StandardDrawCommands(GfxViewInfo *viewInfo);
void RB_SetFrameBufferAlpha();
void    R_DrawLights(const GfxViewInfo *viewInfo, GfxCmdBuf *cmdBuf);
void __cdecl R_DrawPointLitSurfs(GfxCmdBufSourceState *source, const GfxViewInfo *viewInfo, GfxCmdBuf *cmdBuf);
void __cdecl R_DrawPointLitSurfsCallback(const void *userData, GfxCmdBufContext context, GfxCmdBufContext prepassContext);
void __cdecl R_ResolveDistortion(const GfxViewInfo *viewInfo);
void __cdecl RB_StandardPostEffects(GfxViewInfo *viewInfo);
void __cdecl R_SetResolvedScene(GfxCmdBufContext context);
void __cdecl RB_ApplyLatePostEffects(const GfxViewInfo *viewInfo);
void RB_DrawDebugPostEffects();
void RB_ShowFbColorDebug_Screen();
void RB_ShowFbColorDebug_Feedback();
void RB_ShowFloatZDebug();
void __cdecl RB_Draw3DCommon();
void RB_FullbrightDrawCommandsCommon();
void __cdecl RB_FullbrightRenderCommands(GfxViewInfo *viewInfo);
GfxCmdBufSourceState *RB_DebugShaderDrawCommandsCommon();
void __cdecl RB_DebugShaderRenderCommands(GfxViewInfo *viewInfo);
void RB_StandardDrawCommandsCommon();
void __cdecl R_SetCodeImageTexture(GfxCmdBufSourceState *source, unsigned int codeTexture, const GfxImage *image);
void __cdecl RB_StandardRenderCommands(GfxViewInfo *viewInfo);
