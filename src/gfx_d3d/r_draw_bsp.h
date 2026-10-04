#pragma once
#include "rb_state.h"
#include "r_draw_cmdbuf.h"
#include "r_pretess.h"

struct GfxTrianglesDrawStream // sizeof=0x20
{                                       // XREF: ?R_DrawBspDrawSurfsLit@@YAXPBIUGfxCmdBufContext@@1PBUGfxDrawSurfListInfo@@@Z/r
                                        // ?R_DrawBspDrawSurfs@@YAXPBIPAUGfxCmdBufState@@PBUGfxDrawSurfListInfo@@@Z/r
    GfxTexture *whiteTexture;           // XREF: R_DrawBspDrawSurfsLit(uint const *,GfxCmdBufContext,GfxCmdBufContext,GfxDrawSurfListInfo const *)+B2/w
    const unsigned int *primDrawSurfPos;
                                        // XREF: R_DrawBspDrawSurfsLit(uint const *,GfxCmdBufContext,GfxCmdBufContext,GfxDrawSurfListInfo const *)+B8/w
                                        // R_DrawBspDrawSurfs(uint const *,GfxCmdBufState *,GfxDrawSurfListInfo const *)+9/w
    const GfxTexture *reflectionProbeTexture;
                                        // XREF: R_DrawBspDrawSurfsLit(uint const *,GfxCmdBufContext,GfxCmdBufContext,GfxDrawSurfListInfo const *)+85/w
                                        // R_DrawBspDrawSurfsLit(uint const *,GfxCmdBufContext,GfxCmdBufContext,GfxDrawSurfListInfo const *)+10E/r
    const GfxTexture *lightmapPrimaryTexture;
                                        // XREF: R_DrawBspDrawSurfsLit(uint const *,GfxCmdBufContext,GfxCmdBufContext,GfxDrawSurfListInfo const *)+91/w
                                        // R_DrawBspDrawSurfsLit(uint const *,GfxCmdBufContext,GfxCmdBufContext,GfxDrawSurfListInfo const *)+11A/r
    const GfxTexture *lightmapSecondaryTexture;
                                        // XREF: R_DrawBspDrawSurfsLit(uint const *,GfxCmdBufContext,GfxCmdBufContext,GfxDrawSurfListInfo const *)+9D/w
                                        // R_DrawBspDrawSurfsLit(uint const *,GfxCmdBufContext,GfxCmdBufContext,GfxDrawSurfListInfo const *)+126/r
    const GfxTexture *lightmapSecondaryTextureB;
                                        // XREF: R_DrawBspDrawSurfsLit(uint const *,GfxCmdBufContext,GfxCmdBufContext,GfxDrawSurfListInfo const *)+A9/w
                                        // R_DrawBspDrawSurfsLit(uint const *,GfxCmdBufContext,GfxCmdBufContext,GfxDrawSurfListInfo const *)+132/r
    unsigned int customSamplerFlags;    // XREF: R_DrawBspDrawSurfsLit(uint const *,GfxCmdBufContext,GfxCmdBufContext,GfxDrawSurfListInfo const *)+C2/w
    int hasSunDirChanged;               // XREF: R_DrawBspDrawSurfsLit(uint const *,GfxCmdBufContext,GfxCmdBufContext,GfxDrawSurfListInfo const *)+D4/w
};

void __cdecl R_SetStreamsForBspSurface(GfxCmdBufPrimState *state, const srfTriangles_t *tris);
void __cdecl R_DrawBspDrawSurfsLit(
                const unsigned int *primDrawSurfPos,
                GfxCmdBufContext context,
                GfxCmdBufContext prepassContext);
void __cdecl R_DrawTrianglesLit(
                GfxTrianglesDrawStream *drawStream,
                GfxCmdBufPrimState *primState,
                GfxCmdBufPrimState *prepassPrimState);
void __cdecl R_DrawBspTris(GfxCmdBufPrimState *state, const srfTriangles_t *tris, unsigned int triCount);
int __cdecl R_ReadBspDrawSurfs(
                const unsigned int **primDrawSurfPos,
                const unsigned __int16 **list,
                unsigned int *count);
void __cdecl R_DrawBspDrawSurfsLitPreTess(const unsigned int *primDrawSurfPos, GfxCmdBufContext context);
void __cdecl R_DrawPreTessTris(
                GfxCmdBufPrimState *state,
                const srfTriangles_t *tris,
                unsigned int baseIndex,
                unsigned int triCount);
int __cdecl R_ReadBspPreTessDrawSurfs(
                GfxReadCmdBuf *cmdBuf,
                const GfxBspPreTessDrawSurf **list,
                unsigned int *count,
                unsigned int *baseIndex);
void __cdecl R_DrawBspDrawSurfsPreTess(const unsigned int *primDrawSurfPos, GfxCmdBufContext context);
void __cdecl R_DrawBspDrawSurfs(const unsigned int *primDrawSurfPos, GfxCmdBufState *state);
void __cdecl R_DrawTriangles(GfxTrianglesDrawStream *drawStream, GfxCmdBufPrimState *state);
