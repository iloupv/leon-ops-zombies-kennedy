#pragma once
#include <d3d9.h>
#include "r_gfx.h"
#include "rb_state.h"

enum FullscreenType : __int32
{                                       // XREF: R_InitFullscreenRenderTargetImage/r
                                        // R_GetFullScreenRes/r ...
    FULLSCREEN_DISPLAY = 0x0,
    FULLSCREEN_MIXED   = 0x1,
    FULLSCREEN_SCENE   = 0x2,
};

enum RenderTargetUsage : __int32
{                                       // XREF: R_InitFullscreenRenderTargetImage/r
                                        // R_InitRenderTargetImage/r ...
    RENDERTARGET_USAGE_DEPTH   = 0x0,
    RENDERTARGET_USAGE_RENDER  = 0x1,
    RENDERTARGET_USAGE_RENDER_SHARE_SCENE = 0x2,
    RENDERTARGET_USAGE_TEXTURE = 0x3,
};

enum ShadowmapRTTiling : __int32
{                                       // XREF: R_InitShadowmapRenderTarget/r
    SHADOWMAP_RT_TILE_VERTICAL = 0x0,
    SHADOWMAP_RT_TILE_GRID     = 0x1,
};

struct GfxRenderTargetSurface // sizeof=0x8
{                                       // XREF: GfxRenderTarget/r
    IDirect3DSurface9 *color;           // XREF: yuv_encode_frame(void)+3F/r
    IDirect3DSurface9 *depthStencil;    // XREF: R_Resolve(GfxCmdBufContext,GfxImage *)+1C8/r
};

struct __declspec(align(4)) GfxRenderTarget // sizeof=0x14
{                                       // XREF: RB_SaveScreen_BlendBlurred(GfxBlendSaveScreenBlurredParam const *,GfxViewInfo const *):loc_AB807E/r
                                        // RB_SaveScreen_BlendFlashed(GfxBlendSaveScreenFlashedParam const *,GfxViewInfo const *):loc_AB8345/r ...
    GfxImage *image;                    // XREF: R_GenerateSortedDrawSurfs+689/r
                                        // R_GenerateSortedDrawSurfs+715/r ...
    GfxRenderTargetSurface surface;     // XREF: yuv_encode_frame(void)+3F/r
                                        // R_SetDepthOfField+762/r ...
    unsigned __int16 width;             // XREF: RB_StandardDrawCommandsCommon+198/r
                                        // R_UI3D_CheckRenderTarget(void):loc_A8B0BD/r ...
    unsigned __int16 height;            // XREF: RB_StandardDrawCommandsCommon+1A3/r
                                        // RB_UI3D_RenderToTexture(void const *,GfxUI3DBackend const *,GfxCmdBufInput const *)+3F2/r ...
    bool cleared;                       // XREF: RB_BeginFrame(void const *)+58/w
                                        // R_ClearRenderTargetForMultiGpu(GfxCmdBufContext,uchar)+1B/r ...
    // padding byte
    // padding byte
    // padding byte
};

struct ExtraCamSettings // sizeof=0x10
{                                       // XREF: .data:ExtraCamSettings g_extraCamSettings/r
    bool enabled;                       // XREF: R_CheckExtraCamTargets(void)+3/r
                                        // R_InitExtraCamRenderTargets(uint,uint,int)+6/w ...
    // padding byte
    // padding byte
    // padding byte
    int width;                          // XREF: R_CheckExtraCamTargets(void)+27/r
                                        // R_InitExtraCamRenderTargets(uint,uint,int)+10/w
    int height;                         // XREF: R_CheckExtraCamTargets(void)+20/r
                                        // R_InitExtraCamRenderTargets(uint,uint,int)+18/w
    int location;                       // XREF: R_CheckExtraCamTargets(void):loc_AF9C79/r
                                        // R_InitExtraCamRenderTargets(uint,uint,int)+21/w
};

struct GfxImage;

int __cdecl R_GetDepthStencilFormat(_D3DFORMAT renderTargetFormat);
bool __cdecl R_IsDepthStencilFormatOk(_D3DFORMAT renderTargetFormat, _D3DFORMAT depthStencilFormat);
void __cdecl R_InitRenderTargets();
void R_InitRenderTargets_PC();
void __cdecl R_ShareRenderTarget(GfxRenderTargetId idFrom, GfxRenderTargetId idTo);
void __cdecl AssertUninitializedRenderTarget(const GfxRenderTarget *renderTarget);
void __cdecl R_InitFullscreenRenderTargetImage(
                int imageProgType,
                FullscreenType screenType,
                int picmip,
                _D3DFORMAT format,
                RenderTargetUsage usage,
                GfxRenderTarget *renderTarget);
void __cdecl R_GetFullScreenRes(FullscreenType screenType, int *fullscreenWidth, int *fullscreenHeight);
IDirect3DSurface9 *__cdecl R_AssignSingleSampleDepthStencilSurface();
void __cdecl R_GetFrameBufferDepthStencilRes(int *depthStencilWidth, int *depthStencilHeight);
void __cdecl R_InitRenderTargetImage(
                int imageProgType,
                unsigned __int16 width,
                unsigned __int16 height,
                _D3DFORMAT format,
                RenderTargetUsage usage,
                GfxRenderTarget *renderTarget);
void __cdecl R_AssignImageToRenderTargetDepthStencil(GfxRenderTarget *renderTarget, GfxImage *image);
void __cdecl R_AssignImageToRenderTargetColor(GfxRenderTarget *renderTarget, GfxImage *image);
void __cdecl R_InitRenderTargetImage_Composite(int imageProgType, GfxRenderTarget *renderTarget);
void __cdecl R_InitShadowmapRenderTarget(
                int imageProgType,
                unsigned __int16 tileRes,
                unsigned __int16 tileRowCount,
                GfxRenderTarget *renderTarget,
                ShadowmapRTTiling tilingMethond);
void __cdecl R_InitAndTrackRenderTargetImage(
                int imageProgType,
                unsigned __int16 width,
                unsigned __int16 height,
                _D3DFORMAT format,
                RenderTargetUsage usage,
                GfxRenderTarget *renderTarget);
_D3DFORMAT __cdecl R_InitFrameBufferRenderTarget();
const char *__cdecl R_DescribeFormat(_D3DFORMAT format);
void __cdecl R_InitFrameBufferRenderTarget_Win32(GfxRenderTarget *renderTarget);
void R_InitSceneNullRenderTarget_PC();
void __cdecl R_ShareRenderTarget_ReplaceColor(
                unsigned __int8 idFrom,
                unsigned __int8 idTo,
                IDirect3DSurface9 *colorSurface);
void R_InitIntZRenderTarget_PC();
void __cdecl R_ShutdownRenderTargets();
void __cdecl R_ShutDownSingleRenderTarget(GfxRenderTarget *rt);
const char *__cdecl R_RenderTargetName(unsigned __int8 renderTargetId);
void __cdecl R_CheckExtraCamTargets();
void __cdecl R_InitExtraCamRenderTargets(unsigned int width, unsigned int height, int location);
void __cdecl R_InitSizedRenderTargetImage(
                int imageProgType,
                int fullscreenWidth,
                int fullscreenHeight,
                char picmip,
                _D3DFORMAT format,
                RenderTargetUsage usage,
                GfxRenderTarget *renderTarget);
void __cdecl R_ShutdownExtraCamRenderTargets();
void __cdecl R_InitUI3DRenderTarget(
                unsigned __int16 width,
                unsigned __int16 height,
                int location,
                bool allocatePingPongBuffer);
void __cdecl R_ShutdownUI3DRenderTarget(bool freePingPongBuffer);

