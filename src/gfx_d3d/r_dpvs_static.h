#pragma once
#include "r_dpvs.h"

struct DpvsStaticCellCmd;

struct GfxSurface;
union GfxDrawSurf;

struct DpvsPlanes // sizeof=0x8
{                                       // XREF: R_AddCellStaticSurfacesInFrustum/r
    const DpvsPlane *planes;            // XREF: R_AddCellStaticSurfacesInFrustum+AF/w
    int count;                          // XREF: R_AddCellStaticSurfacesInFrustum+4B/w
};

char __cdecl TestOccluders(const float (*bounds)[3], int numOccluders, float (*plane)[4]);
float (*__cdecl TestOccludersPartial(const float (*bounds)[3], int *numOccludersPtr, float (*plane)[4]))[4];
bool __cdecl R_CalcSurfaceNoDynamicShadow(unsigned int bits, GfxSurface *localSurfaces, GfxDrawSurf *localDrawSurfaces);
void __cdecl R_AddCellStaticSurfacesInFrustumCmd(DpvsStaticCellCmd *data);
int __cdecl R_AddCellStaticSurfacesInFrustum(DpvsStaticCellCmd *dpvsCell);
int __cdecl R_AddAabbTreeSurfacesInFrustum_r(
                const GfxAabbTree *tree,
                const DpvsClipPlanes *planes,
                int treeDepth,
                int numOccluders,
                float (*occluderPlanes)[4]);
double __cdecl R_DpvsPlaneMinSignedDistToBox(const DpvsPlane *plane, const float *minmax);
int __cdecl R_CullBoxLeaf(const float *minmax, DpvsClipPlanes *planes);
void __cdecl R_CopyClipPlanes(const DpvsPlane *in, DpvsPlane *out);
void __cdecl R_AddCellCullGroupsInFrustum(DpvsStaticCellCmd *dpvsCell);
void __cdecl R_AddSkySurfacesDpvs(const DpvsPlane *planes, int planeCount);
