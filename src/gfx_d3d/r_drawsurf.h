#pragma once
#include "r_marks.h"
#include "r_bsp.h"

struct GfxPackedVertex;
struct GfxWorldVertex;
struct r_double_index_t;

void __cdecl R_SortDrawSurfs(GfxDrawSurf *drawSurfList, int surfCount);
void __cdecl R_ReverseSortDrawSurfs(GfxDrawSurf *drawSurfList, int surfCount);
void __cdecl R_SortWorldSurfaces();
GfxWorld *R_SetPrimaryLightShadowSurfaces();
GfxDrawSurf __cdecl R_GetWorldDrawSurf(GfxSurface *worldSurf);
void __cdecl R_AddCodeMeshDrawSurf(
                Material *material,
                r_double_index_t *indices,
                unsigned int indexCount,
                unsigned int argOffset,
                unsigned int argCount,
                const char *fxName,
                unsigned int region);
GfxDrawSurf *__cdecl R_AllocFxDrawSurf(unsigned int region);
unsigned __int8 __cdecl R_GetMaterialSortKey(const Material *material);
GfxDrawSurf __cdecl R_GetMaterialInfoPacked(const Material *material);
void __cdecl R_AddRopeCodeMeshDrawSurf(
                const Material *material,
                r_double_index_t *indices,
                unsigned int indexCount,
                unsigned int argOffset,
                unsigned int argCount,
                unsigned __int8 primaryLightIndex,
                unsigned int drawsurfRegion);
void __cdecl R_AddGlassDrawSurf(
                Material *material,
                unsigned __int16 *indices,
                unsigned int indexCount,
                unsigned int lightHandle,
                unsigned __int8 primaryLightIndex,
                char reflectionProbeIndex,
                unsigned int drawsurfRegion);
void __cdecl R_AddMarkMeshDrawSurf(
                Material *material,
                const GfxMarkContext *context,
                unsigned __int16 *indices,
                unsigned int indexCount,
                const float *hitNormal,
                char visLightsMask);
char __cdecl R_AddParticleCloudDrawSurf(volatile unsigned int cloudIndex, Material *material);
void __cdecl R_BeginCodeMeshVerts();
void __cdecl R_EndCodeMeshVerts();
void __cdecl R_BeginMarkMeshVerts();
void __cdecl R_EndMarkMeshVerts();
char __cdecl R_ReserveCodeMeshIndices(int indexCount, r_double_index_t **indicesOut);
char __cdecl R_ReserveCodeMeshVerts(int vertCount, unsigned __int16 *baseVertex);
char __cdecl R_ReserveCodeMeshArgs(int argCount, unsigned int *argOffsetOut);
char __cdecl R_ReserveMarkMeshIndices(int indexCount, r_double_index_t **indicesOut);
char __cdecl R_ReserveMarkMeshVerts(int vertCount, unsigned __int16 *baseVertex);
float (*__cdecl R_GetCodeMeshArgs(unsigned int argOffset))[4];
GfxPackedVertex *__cdecl R_GetCodeMeshVerts(unsigned __int16 baseVertex);
GfxWorldVertex *__cdecl R_GetMarkMeshVerts(unsigned __int16 baseVertex);
