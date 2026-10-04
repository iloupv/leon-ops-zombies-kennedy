#pragma once
#include <gfx_d3d/r_model_lod.h>
#include "xmodel.h"

struct Material;
struct PhysGeomList;
struct cpose_t;

int __cdecl XModelNumBones(const XModel *model);
const char *__cdecl XModelGetName(const XModel *model);
int __cdecl XModelGetSurfaces(const XModel *model, XSurface **surfaces, int submodel);
XSurface *__cdecl XModelGetSurface(const XModel *model, int lod, int surfIndex);
const XModelLodInfo *__cdecl XModelGetLodInfo(const XModel *model, int lod);
void __cdecl XModelSetSModelCacheForLod(
                XModel *model,
                unsigned int lod,
                unsigned int smcIndex,
                unsigned int smcAllocBits);
int __cdecl XModelGetStaticModelCacheVertCount(XModel *model, unsigned int lod);
void __cdecl XModelGetSurfaceStreamBounds(const XModel *model, int surfIndex, float *outMins, float *outMaxs);
unsigned int __cdecl XModelGetSurfCount(const XModel *model, int lod);
Material **__cdecl XModelGetSkins(const XModel *model, int lod);
XModelLodRampType __cdecl XModelGetLodRampType(const XModel *model);
int __cdecl XModelGetNumLods(const XModel *model);
double __cdecl XModelGetLodOutDist(const XModel *model);
int __cdecl XModelGetBoneIndex(const XModel *model, unsigned int name, unsigned int offset, unsigned __int8 *index);
int __cdecl XModelGetParentBoneName(const XModel *model, unsigned int name);
const DObjAnimMat *__cdecl XModelGetBasePose(const XModel *model);
double __cdecl XModelLodInfo_GetDist(const XModelLodInfo *lodInfo, int lodIndex);
int __cdecl XModelGetLodForDist(const XModel *model, float adjustedDist, float baseDist, bool noLodCullOut);
int __cdecl XModelGetLodForDist_Internal(const XModel *model, float dist);
void __cdecl XModelSetTestLods(unsigned int lodLevel, float dist);
double __cdecl XModelGetLodDist(const XModel *model, unsigned int lod);
int __cdecl XModelGetContents(const XModel *model);
bool __cdecl XModelHasCollmap(const XModel *model);
PhysGeomList *__cdecl XModelGetCollmap(const XModel *model, int collMapIndex);
PhysGeomList *__cdecl XModelGetCollmapForBoneIndex(const XModel *model, int boneIndex);
