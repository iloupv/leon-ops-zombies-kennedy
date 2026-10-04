#pragma once
#include "r_xsurface_load_obj.h"

struct XVertexInfo_s // sizeof=0x40
{                                       // XREF: XModelReadSurface(XModel *,uchar const * *,void * (*)(int),XSurface *,int,char const *)+154A/o
                                        // XVertexBuffer/r ...
    float normal[3];
    unsigned __int8 color[4];
    float binormal[3];
    float texCoordX;
    float tangent[3];
    float texCoordY;
    float offset[3];
    unsigned __int8 numWeights;
    unsigned __int8 pad;
    __int16 boneOffset;
};

struct XVertexBuffer // sizeof=0x44
{
    XVertexInfo_s v;
    XBlendLoadInfo w[1];
};

int __cdecl XSurfaceGetNumVerts(const XSurface *surface);
int __cdecl XSurfaceGetNumTris(const XSurface *surface);
void __cdecl XSurfaceGetVerts(const XSurface *surf, float *pVert, float *pTexCoord, float *pNormal);
void __cdecl XSurfaceTransfer(
                const XVertexBuffer *surfVerts,
                GfxPackedVertex *verts0,
                GfxPackedVertex *verts1,
                int vertCount);
void __cdecl XSurfaceTransferGetTexCoordRange(const XVertexInfo_s *v, int vertCount, float *texCoordAv);
