#pragma once

struct cbrushside_t;
struct winding_t;
struct cbrush_t;
struct cplane_s;

struct __declspec(align(4)) ShowCollisionBrushPt // sizeof=0x14
{                                       // XREF: ?CM_ShowSingleBrushCollision@@YAXPBUcbrush_t@@QBMP6AXHPAY02M1@Z@Z/r
    float xyz[3];
    __int16 sideIndex[3];
    // padding byte
    // padding byte
};

void __cdecl CM_GetPlaneVec4Form(
                const cbrushside_t *sides,
                const float (*axialPlanes)[4],
                int index,
                float *expandedPlane);
void __cdecl CM_ShowSingleBrushCollision(
                const cbrush_t *brush,
                const float *color,
                void (__cdecl *drawCollisionPoly)(int, float (*)[3], const float *));
void __cdecl CM_BuildAxialPlanes(const cbrush_t *brush, float (*axialPlanes)[4]);
int __cdecl CM_ForEachBrushPlaneIntersection(
                const cbrush_t *brush,
                const float (*axialPlanes)[4],
                ShowCollisionBrushPt *brushPts);
int __cdecl CM_AddSimpleBrushPoint(
                const cbrush_t *brush,
                const float (*axialPlanes)[4],
                const __int16 *sideIndices,
                const float *xyz,
                int ptCount,
                ShowCollisionBrushPt *brushPts);
char __cdecl CM_BuildBrushWindingForSide(
                winding_t *winding,
                float *planeNormal,
                int sideIndex,
                const ShowCollisionBrushPt *pts,
                int ptCount);
int __cdecl CM_GetXyzList(int sideIndex, const ShowCollisionBrushPt *pts, int ptCount, float (*xyz)[3], int xyzLimit);
int __cdecl CM_PointInList(const float *point, const float (*xyzList)[3], int xyzCount);
void __cdecl CM_PickProjectionAxes(const float *normal, int *i, int *j);
void __cdecl CM_AddExteriorPointToWindingProjected(winding_t *w, float *pt, int i, int j);
double __cdecl CM_SignedAreaForPointsProjected(const float *pt0, const float *pt1, const float *pt2, int i, int j);
void __cdecl CM_AddColinearExteriorPointToWindingProjected(
                winding_t *w,
                float *pt,
                int i,
                int j,
                int index0,
                int index1);
double __cdecl CM_RepresentativeTriangleFromWinding(const winding_t *w, const float *normal, int *i0, int *i1, int *i2);
void __cdecl CM_ReverseWinding(winding_t *w);
void __cdecl CM_ShowBrushCollision(
                int contentMask,
                cplane_s *frustumPlanes,
                int frustumPlaneCount,
                void (__cdecl *drawCollisionPoly)(int, float (*)[3], const float *));
void __cdecl CM_GetShowCollisionColor(float *colorFloat, char colorCounter);
