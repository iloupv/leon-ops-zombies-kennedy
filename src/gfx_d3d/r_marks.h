#pragma once
#include <xanim/dobj.h>
#include <bgame/bg_local.h>
#include "r_dpvs.h"

enum MarkFragmentsAgainstEnum : __int32
{                                       // XREF: MarkInfo/r
                                        // ?R_MarkFragments_Begin@@YAXPAUMarkInfo@@W4MarkFragmentsAgainstEnum@@QBMQAY02$$CBMM2PAUMaterial@@@Z/r ...
    MARK_FRAGMENTS_AGAINST_BRUSHES = 0x0,
    MARK_FRAGMENTS_AGAINST_MODELS  = 0x1,
};

struct FxMarkAlphaFade // sizeof=0x8
{                                       // XREF: FxMark/r
                                        // FX_CreateImpactMarkInternal/r
    int ageLimitMsec;                   // XREF: FX_CreateImpactMarkInternal+107/w
                                        // FX_CreateImpactMarkInternal+114/r ...
    int fadeStartAgeMsec;               // XREF: FX_CreateImpactMarkInternal+119/w
                                        // FX_CreateImpactMarkInternal+125/w
};

struct GfxMarkContext // sizeof=0x6
{                                       // XREF: FxTriGroup/r FxMark/r ...
    unsigned __int8 lmapIndex;          // XREF: R_MarkFragments_StaticModels+B3/w
    unsigned __int8 primaryLightIndex;  // XREF: R_MarkFragments_SceneDObjs+B5/w
                                        // R_MarkFragments_StaticModels+103/w
    unsigned __int8 reflectionProbeIndex;
                                        // XREF: std::_Insertion_sort1<FxMarkTri *,bool (*)(FxMarkTri const &,FxMarkTri const &),FxMarkTri>(FxMarkTri *,FxMarkTri *,bool (*)(FxMarkTri const &,FxMarkTri const &),FxMarkTri *)+4E/w
                                        // std::_Insertion_sort1<FxMarkTri *,bool (*)(FxMarkTri const &,FxMarkTri const &),FxMarkTri>(FxMarkTri *,FxMarkTri *,bool (*)(FxMarkTri const &,FxMarkTri const &),FxMarkTri *)+91/r ...
    unsigned __int8 modelTypeAndSurf;   // XREF: R_MarkFragments_WorldBrushes:loc_A53EA6/w
                                        // R_MarkFragments_EntBrushes+64/w ...
    unsigned __int16 modelIndex;        // XREF: R_MarkFragments_WorldBrushes+5C/w
                                        // R_MarkFragments_EntBrushes+6F/w ...
};

struct FxMarkTri // sizeof=0xC
{                                       // XREF: ??$_Insertion_sort1@PAUFxMarkTri@@P6A_NABU1@0@ZU1@@std@@YAXPAUFxMarkTri@@0P6A_NABU1@1@Z0@Z/r
                                        // ??$_Adjust_heap@PAUFxMarkTri@@HU1@P6A_NABU1@0@Z@std@@YAXPAUFxMarkTri@@HHU1@P6A_NABU1@2@Z@Z/r ...
    unsigned __int16 indices[3];        // XREF: std::_Insertion_sort1<FxMarkTri *,bool (*)(FxMarkTri const &,FxMarkTri const &),FxMarkTri>(FxMarkTri *,FxMarkTri *,bool (*)(FxMarkTri const &,FxMarkTri const &),FxMarkTri *)+42/w
                                        // std::_Insertion_sort1<FxMarkTri *,bool (*)(FxMarkTri const &,FxMarkTri const &),FxMarkTri>(FxMarkTri *,FxMarkTri *,bool (*)(FxMarkTri const &,FxMarkTri const &),FxMarkTri *)+48/w ...
    GfxMarkContext context;             // XREF: std::_Insertion_sort1<FxMarkTri *,bool (*)(FxMarkTri const &,FxMarkTri const &),FxMarkTri>(FxMarkTri *,FxMarkTri *,bool (*)(FxMarkTri const &,FxMarkTri const &),FxMarkTri *)+4E/w
                                        // std::_Insertion_sort1<FxMarkTri *,bool (*)(FxMarkTri const &,FxMarkTri const &),FxMarkTri>(FxMarkTri *,FxMarkTri *,bool (*)(FxMarkTri const &,FxMarkTri const &),FxMarkTri *)+91/r ...
};

struct __declspec(align(4)) MarkInfoCollidedDObj // sizeof=0xC
{                                       // XREF: MarkInfo/r
    DObj *dObj;
    cpose_t *pose;
    unsigned __int16 entnum;
    // padding byte
    // padding byte
};

struct MarkGenContext // sizeof=0x14
{
    int localClientNum;
    Material *material;
    float radius;
    const unsigned __int8 *nativeColor;
    const FxMarkAlphaFade *markAlpha;
};

struct MarkClipPlaneSet // sizeof=0xB4
{                                       // XREF: MarkInfo/r
                                        // R_MarkFragments_EntBrushes/r ...
    float planes[11][4];
    int planeCount;
};

struct FxMarkPoint;

struct __declspec(align(4)) MarkInfoCollidedBModel // sizeof=0xC
{                                       // XREF: MarkInfo/r
    GfxBrushModel *brushModel;
    cpose_t *pose;
    unsigned __int16 entnum;
    // padding byte
    // padding byte
};

struct FxWorldMarkPoint // sizeof=0x18
{                                       // XREF: R_MarkFragments_BrushSurface/r
    float xyz[3];
    float vertWeights[3];
};

struct FxModelMarkPoint // sizeof=0x18
{                                       // XREF: R_MarkModelCoreCallback_1_+12D/o
                                        // R_MarkModelCoreCallback_0_+128/o ...
    float xyz[3];                       // XREF: R_MarkModelCoreCallback_1_+126/o
                                        // R_MarkModelCoreCallback_0_+121/o
    float vertWeights[3];               // XREF: R_MarkModelCoreCallback_1_+BC/o
                                        // R_MarkModelCoreCallback_0_+B7/o
};

struct __declspec(align(4)) MarkInfo // sizeof=0x4AC
{                                       // XREF: FX_ImpactMark_Generate/r
    float origin[3];
    float localOrigin[3];
    float localTexCoordAxis[3];
    float localHitNormal[3];
    float axis[3][3];
    float radius;
    Material *material;
    int maxTris;
    FxMarkTri *tris;
    int maxPoints;
    FxMarkPoint *points;
    float mins[3];
    float maxs[3];
    MarkClipPlaneSet clipPlanes;
    float viewOffset[3];
    bool markHasLightmap;
    bool markHasReflection;
    // padding byte
    // padding byte
    MarkFragmentsAgainstEnum markAgainst;
    unsigned __int16 smodelsCollided[32];
    int smodelCollidedCount;
    MarkInfoCollidedDObj sceneDObjsCollided[32];
    int sceneDObjCollidedCount;
    MarkInfoCollidedBModel sceneBModelsCollided[32];
    int sceneBModelCollidedCount;
    int usedTriCount;
    int usedPointCount;
    void (__cdecl *callback)(void *, int, FxMarkTri *, int, FxMarkPoint *, const float *, const float *, const float *, const bool);
    void *callbackContext;
    bool isSeeThruDecal;                // XREF: FX_ImpactMark_Generate+B4/w
    // padding byte
    // padding byte
    // padding byte
};

struct MarkModelCoreContext // sizeof=0x1C
{                                       // XREF: R_MarkFragments_AnimatedXModel_VertList/r
    MarkInfo *markInfo;                 // XREF: R_MarkFragments_XModelSurface_Basic+1F0/w
    GfxMarkContext *markContext;        // XREF: R_MarkFragments_XModelSurface_Basic+1F9/w
    const float *markOrigin;            // XREF: R_MarkFragments_XModelSurface_Basic+202/w
    const float *markDir;               // XREF: R_MarkFragments_XModelSurface_Basic+20E/w
    const MarkClipPlaneSet *clipPlanes; // XREF: R_MarkFragments_XModelSurface_Basic+21D/w
    const float (*transformMatrix)[3];  // XREF: R_MarkFragments_XModelSurface_Basic+226/w
    const float (*transformNormalMatrix)[3];
};

struct GfxSurface;
struct GfxWorldVertex;

void    R_BoxSurfaces(
                const float *mins,
                const float *maxs,
                int (__cdecl **allowSurf)(int, void *),
                void *callbackContext,
                GfxSurface ***surfLists,
                unsigned int surfListSize,
                unsigned int *surfCounts,
                unsigned int listCount);
void __cdecl R_BoxSurfaces_r(
                mnode_t *node,
                const float *mins,
                const float *maxs,
                int (__cdecl **allowSurf)(int, void *),
                void *callbackContext,
                GfxSurface ***surfLists,
                unsigned int surfListSize,
                unsigned int *surfCounts,
                unsigned int listCount,
                unsigned __int8 *cellBits);
void __cdecl R_CellSurfaces(
                int cellIndex,
                const float *mins,
                const float *maxs,
                int (__cdecl *allowSurf)(int, void *),
                void *callbackContext,
                GfxSurface **surfList,
                unsigned int surfListSize,
                unsigned int *surfCount,
                unsigned __int8 *cellBits);
void __cdecl R_AABBTreeSurfaces_r(
                GfxAabbTree *tree,
                const float *mins,
                const float *maxs,
                int (__cdecl *allowSurf)(int, void *),
                void *callbackContext,
                GfxSurface **surfList,
                unsigned int surfListSize,
                unsigned int *surfCount);
void __cdecl R_AddSurfaceToList(GfxSurface *surf, GfxSurface **surfList, unsigned int *surfCount);
void __cdecl R_CellSurfacesTwoLists(
                int cellIndex,
                const float *mins,
                const float *maxs,
                int (__cdecl **allowSurf)(int, void *),
                void *callbackContext,
                GfxSurface ***surfLists,
                unsigned int surfListSize,
                unsigned int *surfCounts,
                unsigned __int8 *cellBits);
void __cdecl R_AABBTreeSurfacesTwoLists_r(
                GfxAabbTree *tree,
                const float *mins,
                const float *maxs,
                int (__cdecl **allowSurf)(int, void *),
                void *callbackContext,
                GfxSurface ***surfLists,
                unsigned int surfListSize,
                unsigned int *surfCounts);
int    R_BoxStaticModels(
                const float *mins,
                const float *maxs,
                int (__cdecl *allowSModel)(int),
                unsigned __int16 *smodelList,
                int smodelListSize);
void __cdecl R_BoxStaticModels_r(
                mnode_t *node,
                const float *mins,
                const float *maxs,
                int (__cdecl *allowSModel)(int),
                unsigned __int16 *smodelList,
                int smodelListSize,
                int *smodelCount,
                unsigned __int8 *cellBits);
void __cdecl R_CellStaticModels(
                int cellIndex,
                const float *mins,
                const float *maxs,
                int (__cdecl *allowSModel)(int),
                unsigned __int16 *smodelList,
                int smodelListSize,
                int *smodelCount,
                unsigned __int8 *cellBits);
void __cdecl R_AABBTreeStaticModels_r(
                GfxAabbTree *tree,
                const float *mins,
                const float *maxs,
                int (__cdecl *allowSModel)(int),
                unsigned __int16 *smodelList,
                int smodelListSize,
                int *smodelCount);
void __cdecl R_AddStaticModelToList(int smodelIndex, unsigned __int16 *smodelList, int *smodelCount);
void __cdecl R_MarkUtil_GetDObjAnimMatAndHideParts(
                DObj *dobj,
                const cpose_t *pose,
                const DObjAnimMat **outBoneMtxList,
                unsigned int *outHidePartBits);
void __cdecl R_MarkFragments_Begin(
                MarkInfo *markInfo,
                MarkFragmentsAgainstEnum markAgainst,
                const float *origin,
                const float (*axis)[3],
                float radius,
                const float *viewOffset,
                Material *material);
void __cdecl R_GetMarkFragmentBounds(
                const float *origin,
                const float (*axis)[3],
                float radius,
                float *mins,
                float *maxs);
void __cdecl R_GetMarkFragmentClipPlanes(
                const float *origin,
                const float (*axis)[3],
                float radius,
                MarkClipPlaneSet *clipPlanes);
char __cdecl R_MarkFragments_AddDObj(MarkInfo *markInfo, DObj *dObj, cpose_t *pose, unsigned __int16 entityIndex);
char __cdecl R_MarkFragments_AddBModel(
                MarkInfo *markInfo,
                GfxBrushModel *brushModel,
                cpose_t *pose,
                unsigned __int16 entityIndex);
void __cdecl R_MarkFragments_Go(
                MarkInfo *markInfo,
                void (__cdecl *callback)(void *, int, FxMarkTri *, int, FxMarkPoint *, const float *, const float *, const float *, const bool),
                void *callbackContext,
                int maxTris,
                FxMarkTri *tris,
                int maxPoints,
                FxMarkPoint *points);
bool __cdecl R_MarkFragments_Brushes(MarkInfo *markInfo);
char __cdecl R_MarkFragments_WorldBrushes(MarkInfo *markInfo);
bool __cdecl R_AllowMarks(int surfIndex, const Material *markMaterialAsVoid);
bool __cdecl R_Mark_MaterialAllowsMarks(const Material *markReceiverMaterialHandle, const Material *markMaterialHandle);
bool __cdecl R_MarkFragments_BrushSurface(
                MarkInfo *markInfo,
                GfxMarkContext *markContext,
                const MarkClipPlaneSet *clipPlanes,
                const float *markDir,
                const GfxSurface *surface,
                bool *anyMarks);
int __cdecl R_ChopWorldPolyBehindPlane(
                int inPointCount,
                FxWorldMarkPoint *inPoints,
                FxWorldMarkPoint *outPoints,
                const float *plane,
                float epsilon);
void __cdecl R_LerpModelMarkPoints(
                const FxModelMarkPoint *from,
                const FxModelMarkPoint *to,
                float lerp,
                FxModelMarkPoint *out);
bool __cdecl R_MarkFragment_IsTriangleRejected(
                const float *markNormal,
                const float *xyz0,
                const float *xyz1,
                const float *xyz2);
void __cdecl R_MarkFragment_SetupWorldClipPoints(
                const GfxWorldVertex *triVerts0,
                const unsigned __int16 *indices,
                FxWorldMarkPoint (*clipPoints)[14]);
char __cdecl R_MarkFragments_EntBrushes(MarkInfo *markInfo);
void __cdecl R_Mark_TransformClipPlanes(
                const MarkClipPlaneSet *inClipPlanes,
                float (*matrix)[3],
                MarkClipPlaneSet *outClipPlanes);
char __cdecl R_MarkFragments_Glass(MarkInfo *markInfo);
char __cdecl R_MarkFragments_Models(MarkInfo *markInfo);
char __cdecl R_MarkFragments_SceneDObjs(MarkInfo *markInfo);
char __cdecl R_MarkFragments_AnimatedXModel(
                MarkInfo *markInfo,
                const XModel *model,
                const unsigned int *hidePartBits,
                int boneIndex,
                const DObjAnimMat *boneMtxList,
                int boneCount,
                GfxMarkContext *markContext);
// local variable allocation has failed, the output may be wrong!
bool R_MarkFragments_AnimatedXModel_VertList(
                MarkInfo *markInfo,
                unsigned int vertListIndex,
                const DObjAnimMat *poseBone,
                const DObjAnimMat *baseBone,
                GfxMarkContext *markContext,
                XSurface *surface);
char __cdecl R_MarkFragments_StaticModels(MarkInfo *markInfo);
char __cdecl R_MarkFragments_EntirelyRigidXModel(
                MarkInfo *markInfo,
                const XModel *xmodel,
                const float (*modelAxis)[3],
                const float *modelOrigin,
                float modelScale,
                GfxMarkContext *markContext);
char __cdecl R_MarkFragments_XModelSurface_Basic(
                MarkInfo *markInfo,
                const XSurface *surface,
                const float (*modelAxis)[3],
                const float *modelOrigin,
                float modelScale,
                GfxMarkContext *markContext);
int __cdecl R_ChopPolyBehindPlane(
                int inPointCount,
                FxModelMarkPoint *inPoints,
                FxModelMarkPoint *outPoints,
                const float *plane);
