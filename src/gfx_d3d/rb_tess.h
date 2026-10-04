#pragma once
#include "rb_state.h"
#include <xanim/xanim.h>
#include "rb_backend.h"
#include "r_dobj_skin.h"

struct GfxDrawPrimArgs // sizeof=0xC
{                                       // XREF: R_DrawBspTris/r
    int vertexCount;                    // XREF: R_DrawPointLitSurfs+88/w
    int triCount;                       // XREF: R_DrawPointLitSurfs+8F/w
    int baseIndex;                      // XREF: R_DrawPointLitSurfs+81/w
};

struct GfxModelWaterSurface // sizeof=0x2C
{
    IDirect3DIndexBuffer9 *ib;
    IDirect3DVertexBuffer9 *vb0;
    IDirect3DVertexBuffer9 *vb1;
    unsigned int baseVertex0;
    unsigned int baseVertex1;
    float interpolate;
    int vertCount;
    int triCount;
    float origin[3];
};

void __cdecl RB_ShowTess(GfxCmdBufContext context, const float *center, const char *tessName, const float *color);
void __cdecl R_SetVertexDeclTypeNormal(GfxCmdBufState *state, MaterialVertexDeclType vertDeclType);
void __cdecl R_SetVertexDeclTypeWorld(GfxCmdBufState *state);
void __cdecl R_SetVertexDeclTypeModelSkinned(const XSurface *surf, GfxCmdBufState *state);
void __cdecl R_SetObjectIdentityPlacement(GfxCmdBufSourceState *source);
unsigned int __cdecl R_TessCodeMeshList(const GfxDrawSurfListArgs *listArgs, GfxCmdBufContext prepassContext);
int __cdecl R_SetVertexDeclTypeNormal_Safe(GfxCmdBufState *state, MaterialVertexDeclType vertDeclType);
void __cdecl R_TessCodeMeshList_AddCodeMeshArgs(
                GfxCmdBufSourceState *source,
                const GfxBackEndData *data,
                const FxCodeMeshData *codeMesh);
unsigned int __cdecl R_TessRopeMeshList(const GfxDrawSurfListArgs *listArgs, GfxCmdBufContext prepassContext);
unsigned int __cdecl R_TessGlassMeshList(const GfxDrawSurfListArgs *listArgs, GfxCmdBufContext prepassContext);
unsigned int __cdecl R_TessMarkMeshList(const GfxDrawSurfListArgs *listArgs, GfxCmdBufContext prepassContext);
unsigned int __cdecl R_TessParticleCloudList(const GfxDrawSurfListArgs *listArgs, GfxCmdBufContext prepassContext);
void __cdecl R_SetParticleCloudConstants(GfxCmdBufSourceState *source, const GfxParticleCloud *cloud);
void __cdecl Byte4UnpackVertexColor(const unsigned __int8 *from, float *to);
void __cdecl RB_Vec3DirWorldToView(const GfxCmdBufSourceState *source, const float *worldDir, float *viewDir);
void __cdecl RB_CreateParticleCloud2dAxis(const GfxParticleCloud *cloud, const float *viewUp, float (*viewAxis)[2]);
void __cdecl R_DrawXModelSkinnedModelSurf(GfxCmdBufContext context, const GfxModelSkinnedSurface *modelSurf);
void __cdecl R_DrawXModelSkinnedCached(GfxCmdBufContext context, const GfxModelSkinnedSurface *modelSurf);
void __cdecl R_DrawXModelSkinnedUncached(GfxCmdBufContext context, XSurface *xsurf, GfxPackedVertex *skinnedVert);
unsigned int __cdecl R_TessXModelSkinnedDrawSurfList(
                const GfxDrawSurfListArgs *listArgs,
                GfxCmdBufContext prepassContext);
unsigned int __cdecl R_TessXModelRigidDrawSurfList(
                const GfxDrawSurfListArgs *listArgs,
                GfxCmdBufContext prepassContext);
unsigned int __cdecl R_TessXModelWaterList(const GfxDrawSurfListArgs *listArgs, GfxCmdBufContext prepassContext);
void __cdecl R_SetDoubleStreamSource(
                GfxCmdBufPrimState *primState,
                IDirect3DVertexBuffer9 *vb0,
                unsigned int vertexOffset0,
                unsigned int vertexStride0,
                IDirect3DVertexBuffer9 *vb1,
                unsigned int vertexOffset1,
                unsigned int vertexStride1);
unsigned int __cdecl R_TessXModelRigidSkinnedDrawSurfList(
                const GfxDrawSurfListArgs *listArgs,
                GfxCmdBufContext prepassContext);
unsigned int __cdecl R_TessStaticModelRigidDrawSurfList(
                const GfxDrawSurfListArgs *listArgs,
                GfxCmdBufContext prepassContext);
unsigned int __cdecl R_TessStaticModelSkinnedDrawSurfList(
                const GfxDrawSurfListArgs *listArgs,
                GfxCmdBufContext prepassContext);
unsigned int __cdecl R_TessStaticModelPreTessList(const GfxDrawSurfListArgs *listArgs, GfxCmdBufContext prepassContext);
unsigned int __cdecl R_TessStaticModelCachedList(const GfxDrawSurfListArgs *listArgs, GfxCmdBufContext prepassContext);
unsigned int __cdecl R_TessTrianglesList(const GfxDrawSurfListArgs *listArgs, GfxCmdBufContext prepassContext);
void __cdecl R_SetTerrainScorchTextures(GfxCmdBufSourceState *source, const Material *material);
unsigned int __cdecl R_TessTrianglesPreTessList(const GfxDrawSurfListArgs *listArgs, GfxCmdBufContext prepassContext);
unsigned int __cdecl R_TessBModel(const GfxDrawSurfListArgs *listArgs, GfxCmdBufContext prepassContext);
void __cdecl R_DrawBModelPrim(GfxCmdBufContext *prepassContext, GfxCmdBufContext *context, GfxDrawPrimArgs *args);
