#include "r_meshdata.h"
#include "rb_backend.h"
#include "rb_tess.h"
#include "r_state_utils.h"
#include "r_state.h"
#include "r_shade.h"
#include "rb_stats.h"

GfxMeshGlobals gfxMeshGlob;

void __cdecl R_BeginMeshVerts(GfxMeshData *mesh)
{
    if ( mesh->vb.verts
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_meshdata.cpp",
                    27,
                    0,
                    "%s",
                    "mesh->vb.verts == NULL") )
    {
        __debugbreak();
    }
    mesh->vb.verts = (unsigned __int8 *)R_LockVertexBuffer(
                                                                                mesh->vb.buffer,
                                                                                0,
                                                                                mesh->vb.total,
                                                                                mesh->vb.used != 0 ? 4096 : 0x2000);
}

void __cdecl R_EndMeshVerts(GfxMeshData *mesh)
{
    if ( mesh->vb.verts )
    {
        R_UnlockVertexBuffer(mesh->vb.buffer);
        mesh->vb.verts = 0;
    }
}

char __cdecl R_ReserveMeshIndices(GfxMeshData *mesh, int indexCount, r_double_index_t **indicesOut)
{
    unsigned int usedCodeMeshIndexCount; // [esp+0h] [ebp-4h]

    if ( indexCount < 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_meshdata.cpp",
                    65,
                    0,
                    "%s\n\t(indexCount) = %i",
                    "(indexCount >= 0)",
                    indexCount) )
    {
        __debugbreak();
    }
    if ( (indexCount & 1) != 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_meshdata.cpp",
                    66,
                    0,
                    "%s\n\t(indexCount) = %i",
                    "(!(indexCount & 1))",
                    indexCount) )
    {
        __debugbreak();
    }
    if ( !indicesOut
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_meshdata.cpp", 67, 0, "%s", "indicesOut") )
    {
        __debugbreak();
    }
    if ( mesh->indexCount > mesh->totalIndexCount
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_meshdata.cpp",
                    68,
                    0,
                    "%s",
                    "mesh->indexCount <= mesh->totalIndexCount") )
    {
        __debugbreak();
    }
    usedCodeMeshIndexCount = mesh->indexCount;
    if ( indexCount + usedCodeMeshIndexCount > mesh->totalIndexCount )
        return 0;
    mesh->indexCount = indexCount + usedCodeMeshIndexCount;
    *indicesOut = (r_double_index_t *)&mesh->indices[usedCodeMeshIndexCount];
    if ( ((unsigned int)*indicesOut & 3) != 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_meshdata.cpp",
                    84,
                    0,
                    "%s\n\t((uint)(*indicesOut)) = %i",
                    "(!((uint)(*indicesOut) & 3))",
                    *indicesOut) )
    {
        __debugbreak();
    }
    return 1;
}

char __cdecl R_ReserveMeshVerts(GfxMeshData *mesh, int vertCount, unsigned __int16 *baseVertex)
{
    GfxVertexBufferState *codeMeshVb; // [esp+4h] [ebp-Ch]
    unsigned int usedCodeMeshVertBytes; // [esp+8h] [ebp-8h]

    if ( vertCount < 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_meshdata.cpp",
                    96,
                    0,
                    "%s\n\t(vertCount) = %i",
                    "(vertCount >= 0)",
                    vertCount) )
    {
        __debugbreak();
    }
    if ( !baseVertex
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_meshdata.cpp", 97, 0, "%s", "baseVertex") )
    {
        __debugbreak();
    }
    codeMeshVb = &mesh->vb;
    if ( !mesh->vb.verts )
        return 0;
    usedCodeMeshVertBytes = codeMeshVb->used;
    codeMeshVb->used += mesh->vertSize * vertCount;
    if ( mesh->vb.used > mesh->vb.total )
        return 0;
    *baseVertex = usedCodeMeshVertBytes / mesh->vertSize;
    return 1;
}

unsigned __int8 *__cdecl R_GetMeshVerts(GfxMeshData *mesh, unsigned __int16 baseVertex)
{
    if ( mesh->vb.verts )
        return &mesh->vb.verts[mesh->vertSize * baseVertex];
    else
        return 0;
}

void __cdecl R_ResetMesh(GfxMeshData *mesh)
{
    mesh->vb.used = 0;
    mesh->indexCount = 0;
}

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
                unsigned int color)
{
    GfxVertex *verts; // [esp+14h] [ebp-Ch]
    //r_double_index_t *indices; // [esp+18h] [ebp-8h]
    int *indices; // [esp+18h] [ebp-8h]

    if ( !mesh && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_meshdata.cpp", 147, 0, "%s", "mesh") )
        __debugbreak();
    if ( mesh->vertSize != 32
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_meshdata.cpp",
                    148,
                    0,
                    "mesh->vertSize == sizeof( GfxVertex )\n\t%i, %i",
                    mesh->vertSize,
                    32) )
    {
        __debugbreak();
    }
    if ( mesh->vb.total != 128
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_meshdata.cpp",
                    149,
                    0,
                    "mesh->vb.total == sizeof( GfxVertex ) * 4\n\t%i, %i",
                    mesh->vb.total,
                    128) )
    {
        __debugbreak();
    }
    if ( mesh->totalIndexCount != 6
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_meshdata.cpp",
                    150,
                    0,
                    "mesh->totalIndexCount == 6\n\t%i, %i",
                    mesh->totalIndexCount,
                    6) )
    {
        __debugbreak();
    }
    R_BeginMeshVerts(mesh);
    indices = (int *)mesh->indices;
    *indices = 3;
    indices[1] = 131074;
    indices[2] = 0x10000;
    verts = (GfxVertex *)mesh->vb.verts;
    R_SetVertex2d(verts, x, y, s0, t0, color);
    R_SetVertex2d(verts + 1, x + w, y, s1, t0, color);
    R_SetVertex2d(verts + 2, x + w, y + h, s1, t1, color);
    R_SetVertex2d(verts + 3, x, y + h, s0, t1, color);
    R_EndMeshVerts(mesh);
    mesh->indexCount = 6;
    mesh->vb.used = 128;
}

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
                unsigned int color)
{
    if ( !quadMesh
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_meshdata.cpp", 187, 0, "%s", "quadMesh") )
    {
        __debugbreak();
    }
    quadMesh->x = x;
    quadMesh->y = y;
    quadMesh->width = w;
    quadMesh->height = h;
    R_SetQuadMeshData(&quadMesh->meshData, x, y, w, h, s0, t0, s1, t1, color);
}

void __cdecl R_DrawQuadMesh(GfxCmdBufContext context, const Material *material, GfxMeshData *quadMesh)
{
    GfxViewport viewport; // [esp+0h] [ebp-1Ch] BYREF
    GfxDrawPrimArgs args; // [esp+10h] [ebp-Ch] BYREF

    if (R_BeginMaterial(context.state, material, 4u))
    {
        context.state->prim.vertDeclType = VERTDECL_GENERIC;
        R_Set2D(context.source);
        if (context.source->viewportIsDirty)
        {
            R_GetViewport(context.source, &viewport);
            R_SetViewport(context.state, &viewport);
            R_UpdateViewport(context.source, &viewport);
        }
        R_SetupPass(context, 0);
        R_UpdateVertexDecl(context.state);
        R_SetupPassCriticalPixelShaderArgs(context);
        R_SetMeshStream(context.state, quadMesh);
        args.vertexCount = 4;
        args.triCount = 2;
        args.baseIndex = R_SetIndexData(&context.state->prim, (unsigned __int8 *)quadMesh->indices, 2);
        R_SetupPassPerObjectArgs(context);
        R_SetupPassPerPrimArgs(context);
        R_TrackPrims(&context.state->prim, GFX_PRIM_STATS_CODE);
        R_DrawIndexedPrimitive(&context.state->prim, &args);
        context.state->prim.primStats = 0;
    }
}

