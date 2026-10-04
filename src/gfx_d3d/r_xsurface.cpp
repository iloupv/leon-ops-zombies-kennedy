#include "r_xsurface.h"

void __cdecl XSurfaceTransfer_Position_GfxPackedVertex_(GfxPackedVertex *out, const XVertexInfo_s *v)
{
    out->xyz[0] = v->offset[0];
    out->xyz[1] = v->offset[1];
    out->xyz[2] = v->offset[2];
}

void __cdecl XSurfaceTransfer_BinormalSign_GfxPackedVertex_(GfxPackedVertex *out, const XVertexInfo_s *v)
{
    float v2; // [esp+0h] [ebp-14h]
    float binormal[3]; // [esp+8h] [ebp-Ch] BYREF

    Vec3Cross(v->normal, v->tangent, binormal);
    if ((float)((float)((float)(binormal[0] * v->binormal[0]) + (float)(binormal[1] * v->binormal[1]))
        + (float)(binormal[2] * v->binormal[2])) < 0.0)
        v2 = -1.0f;
    else
        v2 = 1.0f;
    out->binormalSign = v2;
}

void __cdecl XSurfaceTransfer_NormalTangent_GfxPackedVertex_(GfxPackedVertex *out, const XVertexInfo_s *v)
{
    out->normal = Vec3PackUnitVec(v->normal);
    out->tangent = Vec3PackUnitVec(v->tangent);
}

void __cdecl XSurfaceTransfer_Texcoord_GfxPackedVertex_(
    GfxPackedVertex *out,
    const XVertexInfo_s *v,
    const float *texCoordAv)
{
    float texCoord[2]; // [esp+4h] [ebp-8h] BYREF

    texCoord[0] = v->texCoordX - *texCoordAv;
    texCoord[1] = v->texCoordY - texCoordAv[1];
    out->texCoord = Vec2PackTexCoords(texCoord);
}

void __cdecl XSurfaceTransfer_Color_GfxPackedVertex_(GfxPackedVertex *out, const XVertexInfo_s *v)
{
    out->color.packed = *(_DWORD *)v->color;
}

int __cdecl XSurfaceGetNumVerts(const XSurface *surface)
{
    if ( !surface
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_xsurface.cpp", 37, 0, "%s", "surface") )
    {
        __debugbreak();
    }
    return surface->vertCount;
}

int __cdecl XSurfaceGetNumTris(const XSurface *surface)
{
    if ( !surface
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_xsurface.cpp", 43, 0, "%s", "surface") )
    {
        __debugbreak();
    }
    return surface->triCount;
}

void __cdecl XSurfaceGetVerts(const XSurface *surf, float *pVert, float *pTexCoord, float *pNormal)
{
    PackedTexCoords *packedTexCoord; // [esp+0h] [ebp-1Ch]
    int render_count; // [esp+4h] [ebp-18h]
    const PackedUnitVec *packedNormal; // [esp+8h] [ebp-14h]
    GfxPackedVertex *verts0; // [esp+Ch] [ebp-10h]

    verts0 = surf->verts0;
    packedNormal = &verts0->normal;
    packedTexCoord = &verts0->texCoord;
    for ( render_count = surf->vertCount; render_count; --render_count )
    {
        if ( pNormal )
        {
            Vec3UnpackUnitVec(*packedNormal, pNormal);
            packedNormal += 8;
            pNormal += 3;
        }
        if ( pTexCoord )
        {
            Vec2UnpackTexCoords(*packedTexCoord, pTexCoord);
            packedTexCoord += 8;
            pTexCoord += 2;
        }
        *pVert = verts0->xyz[0];
        pVert[1] = verts0->xyz[1];
        pVert[2] = verts0->xyz[2];
        ++verts0;
        pVert += 3;
    }
}

void __cdecl XSurfaceTransfer(
                const XVertexBuffer *surfVerts,
                GfxPackedVertex *verts0,
                GfxPackedVertex *verts1,
                int vertCount)
{
    float texCoordAv[2]; // [esp+0h] [ebp-10h] BYREF
    int vertIndex; // [esp+8h] [ebp-8h]
    const XVertexInfo_s *v; // [esp+Ch] [ebp-4h]

    if ( !vertCount
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_xsurface.cpp", 277, 0, "%s", "vertCount") )
    {
        __debugbreak();
    }
    v = &surfVerts->v;
    XSurfaceTransferGetTexCoordRange(&surfVerts->v, vertCount, texCoordAv);
    for ( vertIndex = 0; vertIndex < vertCount; ++vertIndex )
    {
        XSurfaceTransfer_Position_GfxPackedVertex_(&verts0[vertIndex], v);
        XSurfaceTransfer_BinormalSign_GfxPackedVertex_(&verts0[vertIndex], v);
        XSurfaceTransfer_NormalTangent_GfxPackedVertex_(&verts1[vertIndex], v);
        XSurfaceTransfer_Texcoord_GfxPackedVertex_(&verts1[vertIndex], v, texCoordAv);
        XSurfaceTransfer_Color_GfxPackedVertex_(&verts1[vertIndex], v);
        v = (const XVertexInfo_s *)((char *)v + 4 * v->numWeights + 64);
    }
}

void __cdecl XSurfaceTransferGetTexCoordRange(const XVertexInfo_s *v, int vertCount, float *texCoordAv)
{
    float v3; // [esp+8h] [ebp-1Ch]
    float v4; // [esp+Ch] [ebp-18h]
    int vertIndex; // [esp+1Ch] [ebp-8h]
    char texCoordUnitRange; // [esp+20h] [ebp-4h]
    char texCoordUnitRange_1; // [esp+21h] [ebp-3h]

    *texCoordAv = 0.0f;
    texCoordAv[1] = 0.0f;
    texCoordUnitRange = 1;
    texCoordUnitRange_1 = 1;
    for ( vertIndex = 0; vertIndex < vertCount; ++vertIndex )
    {
        *texCoordAv = *texCoordAv + v->texCoordX;
        texCoordAv[1] = texCoordAv[1] + v->texCoordY;
        if ( v->texCoordX < 0.0 || v->texCoordX > 1.0 )
            texCoordUnitRange = 0;
        if ( v->texCoordY < 0.0 || v->texCoordY > 1.0 )
            texCoordUnitRange_1 = 0;
    }
    *texCoordAv = (float)(1.0 / (float)vertCount) * *texCoordAv;
    texCoordAv[1] = (float)(1.0 / (float)vertCount) * texCoordAv[1];
    if ( texCoordUnitRange )
    {
        *texCoordAv = 0.0f;
    }
    else
    {
        v4 = floor((float)(*texCoordAv + 0.5));
        *texCoordAv = v4;
    }
    if ( texCoordUnitRange_1 )
    {
        texCoordAv[1] = 0.0f;
    }
    else
    {
        v3 = floor((float)(texCoordAv[1] + 0.5));
        texCoordAv[1] = v3;
    }
}

