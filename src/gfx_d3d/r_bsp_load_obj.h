#pragma once
#include "r_material.h"
#include <universal/aabbtree.h>
#include <qcommon/com_bsp_load_obj.h>

struct r_lightmapMerge_t // sizeof=0x14
{                                       // XREF: GfxBspLoad/r
    unsigned __int8 index;
    // padding byte
    // padding byte
    // padding byte
    float shift[2];
    float scale[2];
};

struct GfxBspLoad // sizeof=0x2A4
{                                       // XREF: r_globals_load_t/r
    unsigned int bspVersion;            // XREF: R_LoadWorldInternal(char const *)+4D/w
                                        // R_LoadWorldInternal(char const *)+26E/r ...
    const dmaterial_t *diskMaterials;   // XREF: R_GetBspMaterial(uint,GfxSurface *,GfxWorldVertex *)+4C/r
    unsigned int materialCount;
    float outdoorMins[3];               // XREF: R_LoadWorldInternal(char const *)+57B/o
    float outdoorMaxs[3];               // XREF: R_LoadWorldInternal(char const *)+576/o
    r_lightmapMerge_t lmapMergeInfo[32];
};

struct r_lightmapGroup_t // sizeof=0x8
{                                       // XREF: R_LoadLightmaps/r
    int wideCount;                      // XREF: R_LoadLightmaps:loc_A91766/r
    int highCount;                      // XREF: R_LoadLightmaps+122/r
};

struct DiskTriangleSoup // sizeof=0x1C
{
    unsigned __int16 materialIndex;
    unsigned __int8 lightmapIndex;
    unsigned __int8 reflectionProbeIndex;
    unsigned __int8 primaryLightIndex;
    bool hasPrimaryLightConflict;
    bool castsSunShadow;
    // padding byte
    __int16 castsShadow;
    // padding byte
    // padding byte
    int vertexLayerData;
    unsigned int firstVertex;
    unsigned __int16 vertexCount;
    unsigned __int16 indexCount;
    int firstIndex;
};

struct DiskTriangleSoup_Version8 // sizeof=0x10
{
    unsigned __int16 materialIndex;
    unsigned __int8 lightmapIndex;
    unsigned __int8 reflectionProbeIndex;
    int firstVertex;
    unsigned __int16 vertexCount;
    unsigned __int16 indexCount;
    int firstIndex;
};

struct DiskTriangleSoup_Version12 // sizeof=0x14
{
    unsigned __int16 materialIndex;
    unsigned __int8 lightmapIndex;
    unsigned __int8 reflectionProbeIndex;
    int vertexLayerData;
    int firstVertex;
    unsigned __int16 vertexCount;
    unsigned __int16 indexCount;
    int firstIndex;
};

struct AnnotatedLightGridPoint // sizeof=0xA
{                                       // XREF: ??$_Insertion_sort1@PAUAnnotatedLightGridPoint@@P6A_NABU1@0@ZU1@@std@@YAXPAUAnnotatedLightGridPoint@@0P6A_NABU1@1@Z0@Z/r
    unsigned __int16 pos[3];            // XREF: std::_Insertion_sort1<AnnotatedLightGridPoint *,bool (*)(AnnotatedLightGridPoint const &,AnnotatedLightGridPoint const &),AnnotatedLightGridPoint>(AnnotatedLightGridPoint *,AnnotatedLightGridPoint *,bool (*)(AnnotatedLightGridPoint const &,AnnotatedLightGridPoint const &),AnnotatedLightGridPoint *)+42/w
    GfxLightGridEntry entry;            // XREF: std::_Insertion_sort1<AnnotatedLightGridPoint *,bool (*)(AnnotatedLightGridPoint const &,AnnotatedLightGridPoint const &),AnnotatedLightGridPoint>(AnnotatedLightGridPoint *,AnnotatedLightGridPoint *,bool (*)(AnnotatedLightGridPoint const &,AnnotatedLightGridPoint const &),AnnotatedLightGridPoint *)+4F/w
};

struct Stream2Usage // sizeof=0x10
{                                       // XREF: R_CalculateVertexStream2Usage/r
    int firstVertex;                    // XREF: R_CalculateVertexStream2Usage+1B0/w
    int byteOffset;                     // XREF: R_CalculateVertexStream2Usage+1B3/w
    unsigned __int16 vertexCount;       // XREF: R_CalculateVertexStream2Usage+1BE/w
    // padding byte
    // padding byte
    Stream2Usage *next;                 // XREF: R_CalculateVertexStream2Usage+1C2/w
};

struct DiskGfxVertex // sizeof=0x44
{
    float xyz[3];
    float normal[3];
    unsigned __int8 color[4];
    float texCoord[2];
    float lmapCoord[2];
    float tangent[3];
    float binormal[3];
};

struct GfxStaticModelCombinedInst // sizeof=0x78
{                                       // XREF: ??$_Insertion_sort1@PAUGfxStaticModelCombinedInst@@P6A_NABU1@0@ZU1@@std@@YAXPAUGfxStaticModelCombinedInst@@0P6A_NABU1@1@Z0@Z/r
    GfxStaticModelDrawInst smodelDrawInst;
    GfxStaticModelInst smodelInst;
    int isDynamicModel;
};

struct mnode_load_t // sizeof=0x10
{
    int cellIndex;
    int planeIndex;
    unsigned int children[2];
};

struct GfxLightGridEntry_Version15 // sizeof=0x8
{
    unsigned int xyzHighBits;
    unsigned __int8 xyzLowBitsAndPrimaryVis;
    unsigned __int8 needsTrace;
    unsigned __int16 colorsIndex;
};

struct __declspec(align(4)) DiskGfxCell // sizeof=0x70
{
    float mins[3];
    float maxs[3];
    unsigned __int16 aabbTreeIndex;
    // padding byte
    // padding byte
    int firstPortal;
    int portalCount;
    int firstCullGroup;
    int cullGroupCount;
    unsigned __int8 reflectionProbeCount;
    unsigned __int8 reflectionProbes[64];
    // padding byte
    // padding byte
    // padding byte
};

struct DiskGfxCell_Version14 // sizeof=0x34
{
    float mins[3];
    float maxs[3];
    int aabbTreeIndex;
    int firstPortal;
    int portalCount;
    int firstCullGroup;
    int cullGroupCount;
    int unused0;
    int unused1;
};

struct DiskGfxCell_Version21 // sizeof=0x2C
{
    float mins[3];
    float maxs[3];
    unsigned __int16 aabbTreeIndex[2];
    int firstPortal;
    int portalCount;
    int firstCullGroup;
    int cullGroupCount;
};

struct DiskHeroOnlyLight // sizeof=0x38
{
    unsigned __int8 type;
    unsigned __int8 unused[3];
    float color[3];
    float dir[3];
    float origin[3];
    float radius;
    float cosHalfFovOuter;
    float cosHalfFovInner;
    int exponent;
};

struct DiskGfxOccluder // sizeof=0x40
{
    char name[16];
    float pts[4][3];
};

struct DiskLightRegion // sizeof=0x1
{
    unsigned __int8 hullCount;
};

struct INFO // sizeof=0x80
{
    char name[64];
    char origin[64];
};

struct DiskGfxAabbTree // sizeof=0xC
{
    unsigned int firstSurface;
    unsigned int surfaceCount;
    unsigned int childCount;
};

struct DiskGfxPortal // sizeof=0x10
{
    int planeIndex;
    int cellIndex;
    int firstPortalVertex;
    int portalVertexCount;
};

struct DiskGfxCullGroup // sizeof=0x20
{
    float mins[3];
    float maxs[3];
    unsigned int firstSurface;
    unsigned int surfaceCount;
};

struct DiskLeaf // sizeof=0x18
{
    int cluster;
    int firstCollAabbIndex;
    int collAabbCount;
    int firstLeafBrush;
    int numLeafBrushes;
    int cellNum;
};

const Material *__cdecl R_GetBspMaterial(unsigned int materialIndex);
void __cdecl R_CreateWorldVertexBuffer(IDirect3DVertexBuffer9 **vb, int *srcData, unsigned int sizeInBytes);
unsigned __int8 *__cdecl R_LoadSurfaceAlloc(unsigned int bytes);
char *__cdecl R_ParseSunLight(struct SunLightParseParams *params, char *text);
struct GfxWorld *__cdecl R_LoadWorldInternal(const char *name);
void __cdecl R_LoadLightmaps(GfxBspLoad *load);
void __cdecl R_CopyLightmap(
                const unsigned __int8 *srcImage,
                int srcWidth,
                int srcHeight,
                int bytesPerPixel,
                unsigned __int8 *dstImage,
                int tileX,
                int tileY,
                int tilesWide);
int __cdecl R_BuildLightmapMergability(GfxBspLoad *load, r_lightmapGroup_t *groupInfo, int *reorder, LumpType lumpType);
unsigned int __cdecl R_DetermineLightmapCoupling(GfxBspLoad *load, int (*coupling)[16], LumpType lumpType);
void __cdecl R_LoadTriangleSurfaces(unsigned int bspVersion, DiskTriangleSoup **diskSurfaces, unsigned int *surfCount);
const DiskTriangleSoup *__cdecl R_UpdateDiskSurfaces_Version14(DiskTriangleSoup *oldSurfs, int surfCount);
const DiskTriangleSoup *__cdecl R_UpdateDiskSurfaces_Version12(
                const DiskTriangleSoup_Version12 *oldSurfs,
                int surfCount);
const DiskTriangleSoup *__cdecl R_UpdateDiskSurfaces_Version8(const DiskTriangleSoup_Version8 *oldSurfs, int surfCount);
void __cdecl R_UnloadTriangleSurfaces(unsigned int bspVersion, DiskTriangleSoup *diskSurfaces);
unsigned int __cdecl colorRound8Bit(unsigned int a, unsigned int b);
void R_LoadLightGridRowData();
void __cdecl R_LoadLightGridPoints_Version15(unsigned int bspVersion);
int R_InitEmptyLightGrid();
bool __cdecl R_AnnotatedLightGridPointSortsBefore(const AnnotatedLightGridPoint &p0, const AnnotatedLightGridPoint &p1);
char __cdecl R_EncodeLightGrid_Version15(const AnnotatedLightGridPoint *pointsArray, unsigned int pointsArrayCount);
char __cdecl R_CompressLightGridRow_Version15(
                const AnnotatedLightGridPoint *pointsArray,
                unsigned int beginRow,
                unsigned int endRow,
                unsigned __int16 *zRangeGlobal);
void __cdecl R_EmitEmptyLightGridBlock_Version15(unsigned int emptyCount);
char __cdecl R_EmitLightGridBlock_Version15(
                const AnnotatedLightGridPoint *pointsArray,
                signed int runCount,
                const unsigned __int16 *zSubRange,
                const unsigned __int16 *zRangeGlobal,
                unsigned int beginBlock,
                unsigned int endBlock);
unsigned int R_EmitDefaultLightGridEntry_Version15();
void __cdecl R_EmitLightGridEntry_Version15(const AnnotatedLightGridPoint *point);
void R_LoadLightGridHeader();
void __cdecl R_AssertLightGridValid(const GfxLightGrid *lightGrid);
void __cdecl R_LoadLightGridColors(unsigned int bspVersion);
unsigned int __cdecl colorRound16Bit(unsigned int a, unsigned int b);
unsigned __int8 *R_LoadLightGridEntries();
void R_CreateDefaultProbes();
void R_CreateDefaultProbe();
void __cdecl R_LoadReflectionProbes(unsigned int bspVersion);
void R_CreateDummyProbe();
void __cdecl R_LoadHeroOnlyLights(unsigned int bspVersion);
void __cdecl R_BuildHeroLightTree(
                unsigned int nodeIndex,
                unsigned int firstLight,
                unsigned int lightCount,
                float *mins,
                float *maxs);
int __cdecl R_HeroLightSorter(float *a, float *b);
int __cdecl nextPowerOf2(int v);
unsigned int R_LoadCoronas();
char *R_LoadOutdoorBounds();
unsigned __int8 *R_LoadOccluders();
int R_LoadShadowMapVolumes();
int R_LoadExposureVolumes();
unsigned __int8 *R_LoadWorldLodData();
void __cdecl R_LoadMaterials(GfxBspLoad *load);
signed int R_SortSurfaces();
bool __cdecl R_CompareSurfaces(const GfxSurface &surf0, const GfxSurface &surf1);
unsigned int R_CalculateVertexStream2Usage();
Stream2Usage *__cdecl FindExistingUsage(Stream2Usage *list, int firstVertex);
Stream2Usage *__cdecl InsertNewUsage(Stream2Usage **list, const Stream2Usage *copy);
Stream2Usage *__cdecl AllocateUsage();
void __cdecl FreeUsageList(Stream2Usage **list);
void __cdecl FreeUsage(Stream2Usage *usage);
void __cdecl R_LoadSurfaces(GfxBspLoad *load);
void __cdecl R_FinalizeSurfVerts(
                const Material *material,
                GfxSurface *surface,
                const DiskGfxVertex *vertsDisk,
                const r_lightmapMerge_t *merge,
                GfxWorldVertex *vertsMem,
                unsigned int vertCount);
void __cdecl R_CalculateStreamingVolume(const Material *material, srfTriangles_t *tris, const DiskGfxVertex *vertsDisk);
void __cdecl R_SurfCalculateMagicPortalVerts(
                const Material *material,
                GfxSurface *surface,
                const DiskGfxVertex *vertsDisk,
                const r_lightmapMerge_t *merge,
                GfxWorldVertex *vertsMem);
void __cdecl R_CalculateOutdoorBounds(GfxBspLoad *load, const DiskTriangleSoup *diskSurfaces);
void __cdecl R_CalculateWorldBounds(float *mins, float *maxs);
void __cdecl R_MaterialUsage(Material *material, unsigned int firstVertex, int vertexCount, int surfPlusIndexSize);
void __cdecl R_SetSkyImage(const Material *skyMaterial);
void __cdecl R_ValidateSurfaceLightmapUsage(const GfxSurface *surface);
GfxBrushModel *R_LoadSubmodels();
void R_LoadSunSettings();
void __cdecl R_LoadPrimaryLights(unsigned int bspVersion);
void R_LoadLightRegions();
int R_PostLoadEntities();
bool __cdecl R_StaticModelCompare(
                const GfxStaticModelCombinedInst &smodelInst0,
                const GfxStaticModelCombinedInst &smodelInst1);
void __cdecl R_AabbTreeFixAABBSizes_r(GfxAabbTree *tree);
void __cdecl R_FixupGfxAabbTrees(GfxCell *cell);
int __cdecl R_AabbTreeChildrenCount_r(GfxAabbTree *tree);
GfxAabbTree *__cdecl R_AabbTreeMove_r(GfxAabbTree *tree, GfxAabbTree *newTree, GfxAabbTree *newChildren);
bool __cdecl IsDynamicModel(const XModel *const xm);
char __cdecl DynamicModelsSortedCorrectly(
                GfxStaticModelDrawInst *drawInsts,
                unsigned int count,
                unsigned int dynModelCount);
void __cdecl R_LoadEntities();
void __cdecl R_LoadMiscModel(char *(*spawnVars)[2], int spawnVarCount);
double __cdecl R_FloatForKey(const char *key, float defaultValue, char *(*spawnVars)[2], int spawnVarCount);
char *__cdecl R_ValueForKey(const char *key, char *(*spawnVars)[2], int spawnVarCount);
int __cdecl R_IntForKey(const char *key, int defaultValue, char *(*spawnVars)[2], int spawnVarCount);
bool __cdecl R_VectorForKey(const char *key, char *defaultString, char *(*spawnVars)[2], int spawnVarCount, float *v);
char __cdecl R_CheckValidStaticModel(char *(*spawnVars)[2], int spawnVarCount, XModel **model, float *origin);
unsigned int R_LoadAabbTrees();
int __cdecl R_FinishLoadingAabbTrees_r(GfxAabbTree *tree, int totalTreesUsed);
void __cdecl R_LoadCells(unsigned int bspVersion);
void R_LoadPortalVerts();
int R_LoadPortals();
unsigned int R_LoadCullGroups();
void R_LoadCullGroupIndices();
void __cdecl R_LoadNodesAndLeafs(unsigned int bspVersion);
void __cdecl R_SetParentAndCell_r(mnode_load_t *node);
unsigned int __cdecl R_CountNodes_r(mnode_load_t *node);
mnode_t *__cdecl R_SortNodes_r(mnode_load_t *node, mnode_t *out);
void __cdecl R_LoadStep(const char *description);
void __cdecl R_GenerateHighmipAabbs();
void __cdecl R_SetupStreamAabbNodes(GenericAabbTreeOptions *options, GfxStreamingAabbTree *destNodes, int count);
void __cdecl R_GenerateHighmipAabbs_AddWorldSurfaces(int *elemCount, int *elemArray, GenericAabbTreeOptions *options);
void __cdecl R_GenerateHighmipAabbs_AddStaticModelSurfaces(
                int *elemCount,
                int *elemArray,
                GenericAabbTreeOptions *options);
unsigned __int8 *R_LoadWorldRuntime();
unsigned __int8 *R_AllocPrimaryLightBuffers();
void R_LoadInitSkyIntensity();
unsigned int R_SetStaticModelReflectionProbes();
unsigned __int8 *R_AllocShadowGeometryHeaderMemory();
struct GfxShadowGeometry *R_InitShadowGeometryArrays();
void __cdecl R_ForEachShadowCastingSurfaceOnEachLight(void (__cdecl *Callback)(GfxWorld *, unsigned int, unsigned int));
void __cdecl R_IncrementShadowGeometryCount(GfxWorld *world, unsigned int primaryLightIndex);
void R_AddAllProbesToAllCells();
void __cdecl R_InterpretSunLightParseParamsIntoLights(SunLightParseParams *sunParse, GfxLight *sunLight);
void __cdecl R_SetUpSunLight(const float *sunColor, const float *sunDirection, GfxLight *light);
void __cdecl R_InitPrimaryLights(GfxLight *primaryLights);
void __cdecl R_AddShadowSurfaceToPrimaryLight(
                GfxWorld *world,
                unsigned int primaryLightIndex,
                unsigned int sortedSurfIndex);
void __cdecl R_ForEachPrimaryLightAffectingSurface(
                GfxWorld *world,
                const GfxSurface *surface,
                unsigned int sortedSurfIndex,
                void (__cdecl *Callback)(GfxWorld *, unsigned int, unsigned int));
