#pragma once

#include "r_buffers.h"
#include "rb_state.h"

struct r_double_index_t;

struct GfxMeshData // sizeof=0x24
{                                       // XREF: .data:GfxMeshData * g_codeMesh/r
                                        // GfxQuadMeshData/r ...
    GfxMeshData *thisPtr;
    unsigned int indexCount;
    unsigned int totalIndexCount;
    unsigned __int16 *indices;
    GfxVertexBufferState vb;            // XREF: R_InitRenderBuffers(void)+5D/w
    // R_InitGlassRenderBuffers(int,int,int)+64/w ...
    unsigned int vertSize;
};

struct GfxQuadMeshData // sizeof=0x34
{                                       // XREF: GfxMeshGlobals/r
    float x;
    float y;
    float width;
    float height;
    GfxMeshData meshData;               // XREF: R_InitRenderBuffers(void)+193/o
    // R_ShutdownRenderBuffers(void)+16A/o
};

struct GfxMeshGlobals // sizeof=0x1A8
{                                       // XREF: .data:GfxMeshGlobals gfxMeshGlob/r
    GfxQuadMeshData fullSceneViewMesh[4];
    GfxMeshData spotShadowClearMeshData[4];
    GfxMeshData sunShadowClearMeshData[2];
};

void __cdecl R_BeginMeshVerts(GfxMeshData *mesh);
void __cdecl R_EndMeshVerts(GfxMeshData *mesh);
char __cdecl R_ReserveMeshIndices(GfxMeshData *mesh, int indexCount, r_double_index_t **indicesOut);
char __cdecl R_ReserveMeshVerts(GfxMeshData *mesh, int vertCount, unsigned __int16 *baseVertex);
unsigned __int8 *__cdecl R_GetMeshVerts(GfxMeshData *mesh, unsigned __int16 baseVertex);
void __cdecl R_ResetMesh(GfxMeshData *mesh);
void __cdecl R_SetQuadMeshData(
                GfxMeshData *mesh,
                float x,
                float y,
                float w,
                float h,
                float s0,
                float t0,
                float s1,
                float t1,
                unsigned int color);
void __cdecl R_SetQuadMesh(
                GfxQuadMeshData *quadMesh,
                float x,
                float y,
                float w,
                float h,
                float s0,
                float t0,
                float s1,
                float t1,
                unsigned int color);
void __cdecl R_DrawQuadMesh(GfxCmdBufContext context, const Material *material, GfxMeshData *quadMesh);

extern GfxMeshGlobals gfxMeshGlob;
