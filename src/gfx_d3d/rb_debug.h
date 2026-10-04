#pragma once
#include "rb_backend.h"
#include "r_material.h"

int __cdecl RB_AddDebugLine(
                const float *start,
                const float *end,
                const float *color,
                bool depthTest,
                int vertCount,
                int vertLimit,
                GfxPointVertex *verts);
int __cdecl RB_EndDebugLines(int vertCount, const GfxPointVertex *verts);
void __cdecl RB_SetPolyVert(float *xyz, GfxColor color, int tessVertIndex);
void __cdecl RB_SetDebugBrushesAndPatchesCallback(void (__cdecl *callback)());
void __cdecl RB_DrawDebug(const GfxViewParms *viewParms);
const GfxBackEndData *RB_DrawDebugPolys();
void __cdecl RB_DrawPolyInteriors(PolySet *polySet);
const Material *__cdecl RB_DebugPolyGetMaterialByDepthTest(bool depthTest, bool blend);
void __cdecl RB_DrawPolyOutlines(PolySet *polySet);
void __cdecl RB_DrawDebugSpheres(trDebugSphere_t *spheres, int sphereCount);
void __cdecl RB_DrawDebugSphere(trDebugSphere_t *sphere);
void __cdecl RB_DrawDebugLines(trDebugLine_t *lines, int lineCount, GfxPointVertex *verts);
void __cdecl RB_DrawDebugStrings(trDebugString_t *strings, int stringCount);
void __cdecl RB_AddPlumeStrings(const GfxViewParms *viewParms);
void RB_DrawDebugBrushesAndPatches();
void __cdecl RB_ApplySunLight(const float (*verts)[3], const float *color, float *out_color);
void __cdecl RB_BeginCollisionPolygons(bool faceDepthTest, bool faceBlend);
void __cdecl RB_DrawCollisionPolygon(
                int vertCount,
                const float (*verts)[3],
                const float *faceColor,
                bool faceDepthTest,
                int debug_partition);
materialCommands_t *__cdecl RB_GetCmd();
void __cdecl RB_EndCollisionPolygons();
