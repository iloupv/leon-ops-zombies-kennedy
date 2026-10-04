#pragma once

struct XSurfaceCollisionTree;
struct trace_t;
struct DObj;
struct DObjAnimMat;
struct XSurface;

struct QueueElement // sizeof=0x8
{                                       // XREF: XSurfaceGetTriCandidatesLocals/r
                                        // XSurfaceGetTriCandidatesLocals/r ...
    unsigned int beginIndex;            // XREF: XSurfaceVisitTrianglesInAabb(XSurface const *,uint,float const * const,float const * const,bool (*)(void *,uchar const * *,uchar const * *),void *)+10E/w
    unsigned int count;                 // XREF: XSurfaceVisitTrianglesInAabb(XSurface const *,uint,float const * const,float const * const,bool (*)(void *,uchar const * *,uchar const * *),void *)+118/w
};

struct XSurfaceGetTriCandidatesLocals // sizeof=0x2AC
{                                       // XREF: ?XSurfaceVisitTrianglesInAabb@@YA_NPBUXSurface@@IQBM1P6A_NPAXPAPBE3@Z2@Z/r
    int mins[3];
    int maxs[3];                        // XREF: XSurfaceVisitTrianglesInAabb(XSurface const *,uint,float const * const,float const * const,bool (*)(void *,uchar const * *,uchar const * *),void *)+E9/o
    const XSurfaceCollisionTree *tree;  // XREF: XSurfaceVisitTrianglesInAabb(XSurface const *,uint,float const * const,float const * const,bool (*)(void *,uchar const * *,uchar const * *),void *)+5A/w
                                        // XSurfaceVisitTrianglesInAabb(XSurface const *,uint,float const * const,float const * const,bool (*)(void *,uchar const * *,uchar const * *),void *)+60/r ...
    const unsigned __int16 *inIndices;  // XREF: XSurfaceVisitTrianglesInAabb(XSurface const *,uint,float const * const,float const * const,bool (*)(void *,uchar const * *,uchar const * *),void *)+CB/w
    const unsigned __int8 *inVertices0; // XREF: XSurfaceVisitTrianglesInAabb(XSurface const *,uint,float const * const,float const * const,bool (*)(void *,uchar const * *,uchar const * *),void *)+D7/w
    const unsigned __int8 *inVertices1; // XREF: XSurfaceVisitTrianglesInAabb(XSurface const *,uint,float const * const,float const * const,bool (*)(void *,uchar const * *,uchar const * *),void *)+E3/w
    bool (__cdecl *visitorFunc)(void *, const unsigned __int8 **, const unsigned __int8 **);
                                        // XREF: XSurfaceVisitTrianglesInAabb(XSurface const *,uint,float const * const,float const * const,bool (*)(void *,uchar const * *,uchar const * *),void *)+B6/w
    void *visitorContext;               // XREF: XSurfaceVisitTrianglesInAabb(XSurface const *,uint,float const * const,float const * const,bool (*)(void *,uchar const * *,uchar const * *),void *)+BF/w
    unsigned __int16 vertexSize0;       // XREF: XSurfaceVisitTrianglesInAabb(XSurface const *,uint,float const * const,float const * const,bool (*)(void *,uchar const * *,uchar const * *),void *)+185/w
    unsigned __int16 surfaceFlags;      // XREF: XSurfaceVisitTrianglesInAabb(XSurface const *,uint,float const * const,float const * const,bool (*)(void *,uchar const * *,uchar const * *),void *)+179/w
    unsigned int nodeQueueBegin;        // XREF: XSurfaceVisitTrianglesInAabb(XSurface const *,uint,float const * const,float const * const,bool (*)(void *,uchar const * *,uchar const * *),void *)+122/w
                                        // XSurfaceVisitTrianglesInAabb(XSurface const *,uint,float const * const,float const * const,bool (*)(void *,uchar const * *,uchar const * *),void *):loc_8023BC/r
    unsigned int nodeQueueEnd;          // XREF: XSurfaceVisitTrianglesInAabb(XSurface const *,uint,float const * const,float const * const,bool (*)(void *,uchar const * *,uchar const * *),void *)+12C/w
                                        // XSurfaceVisitTrianglesInAabb(XSurface const *,uint,float const * const,float const * const,bool (*)(void *,uchar const * *,uchar const * *),void *)+192/r
    unsigned int leafQueueBegin;        // XREF: XSurfaceVisitTrianglesInAabb(XSurface const *,uint,float const * const,float const * const,bool (*)(void *,uchar const * *,uchar const * *),void *)+136/w
                                        // XSurfaceVisitTrianglesInAabb(XSurface const *,uint,float const * const,float const * const,bool (*)(void *,uchar const * *,uchar const * *),void *):loc_8023E9/r
    unsigned int leafQueueEnd;          // XREF: XSurfaceVisitTrianglesInAabb(XSurface const *,uint,float const * const,float const * const,bool (*)(void *,uchar const * *,uchar const * *),void *)+140/w
                                        // XSurfaceVisitTrianglesInAabb(XSurface const *,uint,float const * const,float const * const,bool (*)(void *,uchar const * *,uchar const * *),void *)+1BF/r
    unsigned int triangleQueueBegin;    // XREF: XSurfaceVisitTrianglesInAabb(XSurface const *,uint,float const * const,float const * const,bool (*)(void *,uchar const * *,uchar const * *),void *)+14A/w
                                        // XSurfaceVisitTrianglesInAabb(XSurface const *,uint,float const * const,float const * const,bool (*)(void *,uchar const * *,uchar const * *),void *):loc_802413/r
    unsigned int triangleQueueEnd;      // XREF: XSurfaceVisitTrianglesInAabb(XSurface const *,uint,float const * const,float const * const,bool (*)(void *,uchar const * *,uchar const * *),void *)+154/w
                                        // XSurfaceVisitTrianglesInAabb(XSurface const *,uint,float const * const,float const * const,bool (*)(void *,uchar const * *,uchar const * *),void *)+1E9/r
    unsigned int vertexQueueBegin;      // XREF: XSurfaceVisitTrianglesInAabb(XSurface const *,uint,float const * const,float const * const,bool (*)(void *,uchar const * *,uchar const * *),void *)+15E/w
                                        // XSurfaceVisitTrianglesInAabb(XSurface const *,uint,float const * const,float const * const,bool (*)(void *,uchar const * *,uchar const * *),void *):loc_80243D/r
    unsigned int vertexQueueEnd;        // XREF: XSurfaceVisitTrianglesInAabb(XSurface const *,uint,float const * const,float const * const,bool (*)(void *,uchar const * *,uchar const * *),void *)+168/w
                                        // XSurfaceVisitTrianglesInAabb(XSurface const *,uint,float const * const,float const * const,bool (*)(void *,uchar const * *,uchar const * *),void *)+213/r
    QueueElement nodeQueue[64];         // XREF: XSurfaceVisitTrianglesInAabb(XSurface const *,uint,float const * const,float const * const,bool (*)(void *,uchar const * *,uchar const * *),void *)+10E/w
                                        // XSurfaceVisitTrianglesInAabb(XSurface const *,uint,float const * const,float const * const,bool (*)(void *,uchar const * *,uchar const * *),void *)+118/w
    QueueElement leafQueue[4];
    QueueElement triangleQueue[4];
    unsigned __int16 vertexQueue[4][3];
};

struct XModelLodInfo // sizeof=0x20
{                                                                             // XREF: XModel/r
        float dist;
        unsigned __int16 numsurfs;
        unsigned __int16 surfIndex;
        int partBits[5];
        unsigned __int8 lod;
        unsigned __int8 smcIndexPlusOne;
        unsigned __int8 smcAllocBits;
        unsigned __int8 unused;
};

struct XModelNameMap // sizeof=0x4
{                                                                             // XREF: ?XAnimResetAnimMap@@YAXPBUDObj@@I@Z/r
    unsigned __int16 name;                            // XREF: XAnimResetAnimMap(DObj const *,uint)+10/w
    unsigned __int16 index;
};

struct XModelCollTri_s // sizeof=0x30
{                                                                             // XREF: XModelCollTri/r
        float plane[4];
        float svec[4];
        float tvec[4];
};

struct XModelCollSurf_s // sizeof=0x2C
{                                                                             // XREF: XModelCollSurf/r
        XModelCollTri_s *collTris;
        int numCollTris;
        float mins[3];
        float maxs[3];
        int boneIdx;
        int contents;
        int surfFlags;
};

struct XModelHighMipBounds // sizeof=0x10
{
        float center[3];
        float himipRadiusSq;
};

struct XModelStreamInfo // sizeof=0x4
{                                                                             // XREF: XModel/r
        XModelHighMipBounds *highMipBounds;
};

struct __declspec(align(4)) XBoneInfo // sizeof=0x2C
{                                                                             // XREF: XModelDefault/r
        float bounds[2][3];
        float offset[3];
        float radiusSquared;
        unsigned __int8 collmap;
        // padding byte
        // padding byte
        // padding byte
};

struct XModel // sizeof=0xFC
{                                                                             // XREF: XAssetPoolEntry<XModel>/r
        const char *name;
        unsigned __int8 numBones;
        unsigned __int8 numRootBones;
        unsigned __int8 numsurfs;
        unsigned __int8 lodRampType;
        //$C8C573B57ACA1D7542AD56C4163862EC ___u5;
        union //$C8C573B57ACA1D7542AD56C4163862EC // sizeof=0x4
        {                                                                             // XREF: XModel/r
                unsigned __int16 *localBoneNames;
                unsigned __int16 *boneNames;
        };
        //$37E13A30061EC97EDA76440B3BA5E97A ___u6;
        union //$37E13A30061EC97EDA76440B3BA5E97A // sizeof=0x4
        {                                                                             // XREF: XModel/r
                unsigned __int8 *localParentList;
                unsigned __int8 *parentList;
        };
        //$BF9640C0B3AB5E078C286DD9616EC22F ___u7;
        union //$BF9640C0B3AB5E078C286DD9616EC22F // sizeof=0x4
        {                                                                             // XREF: XModel/r
                __int16 *localQuats;
                __int16 *quats;
        };
        //$E1CD6A013C1D28F2956F4983A8D1052C ___u8;
        union //$E1CD6A013C1D28F2956F4983A8D1052C // sizeof=0x4
        {                                                                             // XREF: XModel/r
                float *localTrans;
                float *trans;
        };
        unsigned __int8 *partClassification;
        struct DObjAnimMat *baseMat;
        struct XSurface *surfs;
        struct Material **materialHandles;
        XModelLodInfo lodInfo[4];
        unsigned __int8 lodDistAutoGenerated;
        // padding byte
        // padding byte
        // padding byte
        XModelCollSurf_s *collSurfs;
        int numCollSurfs;
        int contents;
        //$AD77717FA1177FEDD3D106F4B139C8DB ___u18;
        union //$AD77717FA1177FEDD3D106F4B139C8DB // sizeof=0x4
        {                                                                             // XREF: XModel/r
                XBoneInfo *localBoneInfo;
                XBoneInfo *boneInfo;
        };
        float radius;
        float mins[3];
        float maxs[3];
        __int16 numLods;
        __int16 collLod;
        XModelStreamInfo streamInfo;
        int memUsage;
        int flags;
        bool bad;
        // padding byte
        // padding byte
        // padding byte
        struct PhysPreset *physPreset;
        unsigned __int8 numCollmaps;
        // padding byte
        // padding byte
        // padding byte
        struct Collmap *collmaps;
        struct PhysConstraints *physConstraints;
};

struct XModelPartsLoad // sizeof=0x1C
{                                                                             // XREF: XModelDefault/r
        unsigned __int8 numBones;
        unsigned __int8 numRootBones;
        // padding byte
        // padding byte
        unsigned __int16 *boneNames;
        unsigned __int8 *parentList;
        __int16 *quats;
        float *trans;
        unsigned __int8 *partClassification;
        DObjAnimMat *baseMat;
};

struct XModelSurfs // sizeof=0x18
{                                                                             // XREF: XModelLoadFile/r
        XSurface *surfs;                                        // XREF: XModelLoadFile+B47/r
        int partBits[5];                                        // XREF: XModelLoadFile+926/r
};

struct XModelPiece // sizeof=0x10
{
    XModel *model;
    float offset[3];
};

struct XModelPieces // sizeof=0xC
{                                       // XREF: XAssetPoolEntry<XModelPieces>/r
    const char *name;
    int numpieces;
    XModelPiece *pieces;
};

struct XModelDefault // sizeof=0x50
{                                       // XREF: .data:XModelDefault g_default/r
    unsigned __int16 boneNames[1];
    unsigned __int8 parentList[1];      // XREF: XModelCreateDefaultParts+17/o
    // padding byte
    XModelPartsLoad modelParts;         // XREF: XModelCreateDefaultParts+D/o
    XBoneInfo boneInfo;                 // XREF: XModelMakeDefault+84/o
    unsigned __int8 partClassification[1];
    // XREF: XModelCreateDefaultParts+4B/o
    // XModelCreateDefaultParts+52/w
// padding byte
    unsigned __int16 surfNames[1];
};

bool __cdecl XModelBad(const XModel *model);
void __cdecl XModelReleaseResources(XModel *model);
void __cdecl XModelPartsFree(XModelPartsLoad *modelParts);
XModelPartsLoad *__cdecl XModelPartsFindData(const char *name);
void __cdecl XModelPartsSetData(const char *name, XModelPartsLoad *modelParts, void *(__cdecl *Alloc)(int));
XModelSurfs *__cdecl XModelSurfsFindData(const char *name);
void __cdecl XModelSurfsSetData(const char *name, XModelSurfs *modelSurfs, void *(__cdecl *Alloc)(int));
XModel *__cdecl XModelPrecache(char *name, void *(__cdecl *Alloc)(int), void *(__cdecl *AllocColl)(int));
XModel *__cdecl XModelPrecache_LoadObj(char *name, void *(__cdecl *Alloc)(int), void *(__cdecl *AllocColl)(int));
XModel *__cdecl XModelDefaultModel(const char *name, void *(__cdecl *Alloc)(int));
XModel *__cdecl XModelCreateDefault(void *(__cdecl *Alloc)(int));
void __cdecl XModelMakeDefault(XModel *model);
XModelPartsLoad *__cdecl XModelCreateDefaultParts();
XModel *__cdecl XModelPrecache_FastFile(const char *name);
unsigned __int16 *__cdecl XModelBoneNames(XModel *model);
double __cdecl XModelGetRadius(const XModel *model);
void __cdecl XModelGetBounds(const XModel *model, float *mins, float *maxs);
int __cdecl XModelGetMemUsage(const XModel *model);
void __cdecl XModelRenderString(const float *pos, char *string);
int __cdecl XModelTraceLine(
                const XModel *model,
                trace_t *results,
                const float *localStart,
                const float *localEnd,
                int contentmask);
int __cdecl XModelTraceLineAnimated(
                const DObj *obj,
                unsigned int modelIndex,
                int baseBoneIndex,
                trace_t *results,
                const DObjAnimMat *boneMtxList,
                float *localStart,
                float *localEnd,
                int contentmask);
void __cdecl ConvertQuatToInverseMat(const DObjAnimMat *mat, float (*axis)[3]);
void __cdecl XModelTraceLineAnimatedPartBits(
                const DObj *obj,
                unsigned int modelIndex,
                int baseBoneIndex,
                int contentmask,
                int *partBits);
bool __cdecl XSurfaceVisitTrianglesInAabb(
                const XSurface *surface,
                unsigned int vertListIndex,
                const float *aabbMins,
                const float *aabbMaxs,
                bool (__cdecl *visitorFunc)(void *, const unsigned __int8 **, const unsigned __int8 **),
                void *visitorContext);
void __cdecl XSurfaceVisitTrianglesInAabb_ConvertAabb(
                const XSurfaceCollisionTree *tree,
                const float *aabbMins,
                const float *aabbMaxs,
                int *mins,
                int *maxs);
void __cdecl PrefetchArrayIndexed(char *mem, unsigned int elementCount, unsigned int index, unsigned int elementSize);
int __cdecl XSurfaceVisitTrianglesInAabb_ProcessVertices(XSurfaceGetTriCandidatesLocals *locals);
char __cdecl XSurfaceVisitTrianglesInAabb_ProcessTriangles(XSurfaceGetTriCandidatesLocals *locals);
char __cdecl XSurfaceVisitTrianglesInAabb_ProcessLeaf(XSurfaceGetTriCandidatesLocals *locals);
char __cdecl XSurfaceVisitTrianglesInAabb_ProcessNode(XSurfaceGetTriCandidatesLocals *locals);
