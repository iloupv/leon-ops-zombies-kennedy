#pragma once
#include "rb_state.h"
#include "r_rendercmds.h"
#include "rb_tess.h"
#include "r_draw_cmdbuf.h"

struct GfxStaticModelDrawStream // sizeof=0x34
{                                       // XREF: ?R_DrawStaticModelSurfLit@@YAXPBIUGfxCmdBufContext@@1PBUGfxDrawSurfListInfo@@@Z/r
                                        // ?R_DrawStaticModelSurf@@YAXPBIUGfxCmdBufContext@@PBUGfxDrawSurfListInfo@@@Z/r ...
    const unsigned int *primDrawSurfPos;
                                        // XREF: R_DrawStaticModelSurfLit(uint const *,GfxCmdBufContext,GfxCmdBufContext,GfxDrawSurfListInfo const *)+2C/w
                                        // R_DrawStaticModelSurf(uint const *,GfxCmdBufContext,GfxDrawSurfListInfo const *)+2C/w ...
    const GfxTexture *reflectionProbeTexture;
                                        // XREF: R_DrawStaticModelSurfLit(uint const *,GfxCmdBufContext,GfxCmdBufContext,GfxDrawSurfListInfo const *)+38/w
                                        // R_DrawStaticModelSurf(uint const *,GfxCmdBufContext,GfxDrawSurfListInfo const *)+38/w ...
    unsigned int customSamplerFlags;    // XREF: R_DrawStaticModelSurfLit(uint const *,GfxCmdBufContext,GfxCmdBufContext,GfxDrawSurfListInfo const *)+48/w
                                        // R_DrawStaticModelSurf(uint const *,GfxCmdBufContext,GfxDrawSurfListInfo const *)+48/w ...
    GfxFrameStats *frameStats;          // XREF: R_DrawStaticModelSurfLit(uint const *,GfxCmdBufContext,GfxCmdBufContext,GfxDrawSurfListInfo const *)+66/w
                                        // R_DrawStaticModelSurf(uint const *,GfxCmdBufContext,GfxDrawSurfListInfo const *)+66/w ...
    GfxPrimStats *primStats;            // XREF: R_DrawStaticModelSurfLit(uint const *,GfxCmdBufContext,GfxCmdBufContext,GfxDrawSurfListInfo const *)+72/w
                                        // R_DrawStaticModelSurf(uint const *,GfxCmdBufContext,GfxDrawSurfListInfo const *)+72/w ...
    int usingCrossFade;                 // XREF: R_DrawStaticModelSurfLit(uint const *,GfxCmdBufContext,GfxCmdBufContext,GfxDrawSurfListInfo const *)+5A/w
                                        // R_DrawStaticModelSurf(uint const *,GfxCmdBufContext,GfxDrawSurfListInfo const *)+5A/w ...
    XSurface *localSurf;
    unsigned int smodelCount;
    const unsigned __int16 *smodelList;
    unsigned int reflectionProbeIndex;
    unsigned __int32 viewInfoIndex : 2;   // XREF: R_DrawStaticModelSurfLit(uint const *,GfxCmdBufContext,GfxCmdBufContext,GfxDrawSurfListInfo const *)+20/r
                                          // R_DrawStaticModelSurfLit(uint const *,GfxCmdBufContext,GfxCmdBufContext,GfxDrawSurfListInfo const *)+26/w ...
    unsigned __int32 which_lod : 2;
    unsigned __int32 pad : 28;
    const DynSModelClientView *dynSModelView;
                                        // XREF: R_DrawStaticModelSurfLit(uint const *,GfxCmdBufContext,GfxCmdBufContext,GfxDrawSurfListInfo const *)+6/w
                                        // R_DrawStaticModelSurfLit(uint const *,GfxCmdBufContext,GfxCmdBufContext,GfxDrawSurfListInfo const *)+90/w ...
    const DynSModelGfxState *dynSModelState;
                                        // XREF: R_DrawStaticModelSurfLit(uint const *,GfxCmdBufContext,GfxCmdBufContext,GfxDrawSurfListInfo const *)+D/w
                                        // R_DrawStaticModelSurfLit(uint const *,GfxCmdBufContext,GfxCmdBufContext,GfxDrawSurfListInfo const *)+9F/w ...
};

union GfxStaticModelPreTessSurf // sizeof=0x4
{                                       // XREF: R_PreTessStaticModelCachedList+15F/w
                                        // R_PreTessStaticModelCachedList+165/w ...
    //GfxStaticModelPreTessSurf::<unnamed_type_fields> fields;
    struct
    {
        unsigned __int8 surfIndex;
        unsigned __int8 lod;
        unsigned __int16 cachedIndex;
    } fields;
    unsigned int packed;
};

void __cdecl R_DrawStaticModelSurfLit(
                const unsigned int *primDrawSurfPos,
                GfxCmdBufContext context,
                GfxCmdBufContext prepassContext,
                const GfxDrawSurfListInfo *info);
int __cdecl R_GetNextStaticModelSurf(GfxStaticModelDrawStream *drawStream, XSurface **outSurf);
void __cdecl R_DrawStaticModelSurf(
                const unsigned int *primDrawSurfPos,
                GfxCmdBufContext context,
                const GfxDrawSurfListInfo *info);
void __cdecl R_DrawStaticModelDrawSurfNonOptimized(GfxStaticModelDrawStream *drawStream, GfxCmdBufContext context);
void __cdecl R_SetStaticModelVertexBuffer(GfxCmdBufPrimState *primState, XSurface *xsurf);
void __cdecl R_SetStaticModelIndexBuffer(GfxCmdBufPrimState *primState, XSurface *xsurf);
void __cdecl R_SetStaticModelPrimArgs(const XSurface *xsurf, GfxDrawPrimArgs *args);
void __cdecl R_SetStaticModelVertexPrimArgs(const XSurface *xsurf, GfxDrawPrimArgs *args);
void __cdecl R_DrawStaticModelDrawSurfPlacement(
                const GfxStaticModelDrawInst *smodelDrawInst,
                GfxCmdBufSourceState *source);
void __cdecl R_DrawStaticModelDrawSurfLightingNonOptimized(
                GfxStaticModelDrawStream *drawStream,
                GfxCmdBufContext context);
void __cdecl R_DrawStaticModelSkinnedSurfLit(
                const unsigned int *primDrawSurfPos,
                GfxCmdBufContext context,
                const GfxDrawSurfListInfo *info);
void __cdecl R_DrawStaticModelsSkinnedDrawSurfLighting(GfxStaticModelDrawStream *drawStream, GfxCmdBufContext context);
void __cdecl R_DrawStaticModelSkinnedDrawSurfLighting(
                int smodelIndex,
                const GfxStaticModelDrawInst *smodelDrawInst,
                unsigned __int16 lightingHandle,
                GfxDrawPrimArgs *args,
                GfxCmdBufContext context,
                const DynSModelClientView *dynSModelView,
                const DynSModelGfxState *dynSModelState);
void __cdecl R_SetStaticModelSkinnedPrimArgs(GfxCmdBufPrimState *state, const XSurface *xsurf, GfxDrawPrimArgs *args);
void __cdecl R_DrawStaticModelSkinnedSurf(
                const unsigned int *primDrawSurfPos,
                GfxCmdBufContext context,
                const GfxDrawSurfListInfo *info);
void __cdecl R_DrawStaticModelsSkinnedDrawSurf(GfxStaticModelDrawStream *drawStream, GfxCmdBufContext context);
void __cdecl R_DrawStaticModelSkinnedDrawSurf(
                int smodelIndex,
                const GfxStaticModelDrawInst *smodelDrawInst,
                GfxDrawPrimArgs *args,
                GfxCmdBufContext context,
                const DynSModelClientView *dynSModelView,
                const DynSModelGfxState *dynSModelState);
void __cdecl R_DrawStaticModelCachedSurfLit(const unsigned int *primDrawSurfPos, GfxCmdBufContext context);
int __cdecl R_GetNextStaticModelCachedSurf(GfxStaticModelDrawStream *drawStream);
void __cdecl R_DrawStaticModelsCachedDrawSurfLighting(GfxStaticModelDrawStream *drawStream, GfxCmdBufContext context);
XSurface *__cdecl R_GetCurrentStaticModelCachedSurf(
                GfxStaticModelDrawStream *drawStream,
                unsigned int *reflectionProbeIndex);
void __cdecl R_SetStaticModelCachedPrimArgs(const XSurface *xsurf, GfxDrawPrimArgs *args);
void __cdecl R_SetStaticModelCachedBuffer(GfxCmdBufState *state, unsigned int cachedIndex);
void __cdecl R_DrawStaticModelCachedSurf(const unsigned int *primDrawSurfPos, GfxCmdBufContext context);
void __cdecl R_DrawStaticModelsCachedDrawSurf(GfxStaticModelDrawStream *drawStream, GfxCmdBufContext context);
void __cdecl R_DrawStaticModelPreTessSurfLit(const unsigned int *primDrawSurfPos, GfxCmdBufContext context);
int __cdecl R_ReadStaticModelPreTessDrawSurf(
                GfxReadCmdBuf *readCmdBuf,
                GfxStaticModelPreTessSurf *pretessSurf,
                unsigned int *firstIndex,
                unsigned int *count);
void __cdecl R_DrawStaticModelsPreTessDrawSurfLighting(
                GfxStaticModelPreTessSurf pretessSurf,
                unsigned int firstIndex,
                unsigned int count,
                GfxCmdBufContext context);
const GfxStaticModelDrawInst *__cdecl R_SetupCachedSModelSurface(
                GfxCmdBufState *state,
                unsigned int cachedIndex,
                unsigned int lod,
                unsigned int surfIndex,
                unsigned int count,
                GfxDrawPrimArgs *args,
                unsigned int *baseIndex);
void __cdecl R_DrawStaticModelPreTessSurf(const unsigned int *primDrawSurfPos, GfxCmdBufContext context);
void __cdecl R_DrawStaticModelsPreTessDrawSurf(
                GfxStaticModelPreTessSurf pretessSurf,
                unsigned int firstIndex,
                unsigned int count,
                GfxCmdBufContext context);
