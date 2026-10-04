#pragma once
#include "r_dpvs_dynmodel.h"
#include "r_sky.h"
#include "r_world_lod.h"
#include "r_gfx.h"
#include "rb_light.h"

struct XModel;

struct GfxShadowGeometry // sizeof=0xC
{
    unsigned __int16 surfaceCount;
    unsigned __int16 smodelCount;
    unsigned __int16 *sortedSurfIndex;
    unsigned __int16 *smodelIndex;
};

struct GfxStreamingAabbTree // sizeof=0x20
{
    unsigned __int16 firstItem;
    unsigned __int16 itemCount;
    unsigned __int16 firstChild;
    unsigned __int16 childCount;
    float mins[3];
    float maxs[3];
};

struct GfxWorldStreamInfo // sizeof=0x10
{                                       // XREF: GfxWorld/r
    int aabbTreeCount;                  // XREF: R_GenerateHighmipAabbs+172/w
    GfxStreamingAabbTree *aabbTrees;    // XREF: R_GenerateHighmipAabbs+18E/w
                                        // R_GenerateHighmipAabbs+1D4/r
    int leafRefCount;                   // XREF: R_GenerateHighmipAabbs+196/w
    int *leafRefs;                      // XREF: R_GenerateHighmipAabbs+1B1/w
                                        // R_GenerateHighmipAabbs+1C1/r
};

struct GfxWorldSunColor // sizeof=0x6C
{                                       // XREF: SunLightParseParams/r
    unsigned int control;
    float angles[3];
    float ambientColor[4];
    float sunDiffuseColor[4];
    float sunSpecularColor[4];
    float skyColor[4];
    float groundColor[4];
    float exposure;
    float sunShadowSampleSizeNear;
    float skyboxHDRScale;
};

struct SunLightParseParams // sizeof=0xB4
{                                       // XREF: GfxWorld/r
    char name[64];
    float treeScatterIntensity;         // XREF: R_LoadSunSettings+1E/w
    float treeScatterAmount;            // XREF: R_LoadSunSettings+E/w
    GfxWorldSunColor sunSettings[1];
};

struct GfxLightCorona // sizeof=0x20
{
    float origin[3];
    float radius;
    float color[3];
    float intensity;
};

struct GfxShadowMapVolume // sizeof=0x10
{
    unsigned int control;
    unsigned int padding1;
    unsigned int padding2;
    unsigned int padding3;
};

struct GfxVolumePlane // sizeof=0x10
{
    float plane[4];
};

struct GfxExposureVolume // sizeof=0x18
{
    unsigned int control;
    float exposure;
    float luminanceIncreaseScale;
    float luminanceDecreaseScale;
    float featherRange;
    float featherAdjust;
};

struct GfxSkyDynamicIntensity // sizeof=0x10
{                                       // XREF: GfxWorld/r
    float angle0;                       // XREF: R_LoadInitSkyIntensity+B/w
    float angle1;                       // XREF: R_LoadInitSkyIntensity+1B/w
    float factor0;                      // XREF: R_LoadInitSkyIntensity+2B/w
    float factor1;                      // XREF: R_LoadInitSkyIntensity+3B/w
};

struct GfxStaticModelInst // sizeof=0x28
{                                       // XREF: GfxStaticModelCombinedInst/r
    float mins[3];
    float maxs[3];
    float lightingOrigin[3];
    GfxColor groundLighting;
};

struct GfxCullGroup // sizeof=0x20
{
    float mins[3];
    float maxs[3];
    int surfaceCount;
    int startSurfIndex;
};

struct GfxPackedPlacement // sizeof=0x34
{                                       // XREF: GfxStaticModelDrawInst/r
    float origin[3];
    float axis[3][3];
    float scale;
};

struct GfxStaticModelDrawInst // sizeof=0x4C
{                                       // XREF: GfxStaticModelCombinedInst/r
    float cullDist;
    GfxPackedPlacement placement;
    XModel *model;
    int flags;
    unsigned __int16 smodelCacheIndex[4];
    unsigned __int16 lightingHandle;
    unsigned __int8 reflectionProbeIndex;
    unsigned __int8 primaryLightIndex;
};

struct GfxWorldDpvsStatic // sizeof=0x70
{                                       // XREF: GfxWorld/r
    unsigned int smodelCount;           // XREF: R_PostLoadEntities+41/r
                                        // R_PostLoadEntities+72/r ...
    unsigned int dynamicSModelCount;    // XREF: R_PostLoadEntities+1CE/r
                                        // R_PostLoadEntities+225/w ...
    unsigned int staticSurfaceCount;    // XREF: R_LoadWorldRuntime+1BD/w
                                        // R_LoadWorldRuntime+1C2/r ...
    unsigned int litSurfsBegin;         // XREF: R_SortSurfaces+219/w
    unsigned int litSurfsEnd;           // XREF: R_SortSurfaces+2AF/w
    unsigned int decalSurfsBegin;       // XREF: R_SortSurfaces+2B8/w
    unsigned int decalSurfsEnd;         // XREF: R_SortSurfaces+351/w
    unsigned int emissiveSurfsBegin;    // XREF: R_SortSurfaces+35A/w
    unsigned int emissiveSurfsEnd;      // XREF: R_SortSurfaces+3B3/w
    unsigned int smodelVisDataCount;    // XREF: R_LoadWorldRuntime+31/w
                                        // R_LoadWorldRuntime+18C/r
    unsigned int surfaceVisDataCount;   // XREF: R_LoadWorldRuntime+49/w
                                        // R_LoadWorldRuntime+209/r
    unsigned __int8 *smodelVisData[3];  // XREF: R_LoadWorldRuntime+7F/w
    unsigned __int8 *surfaceVisData[3]; // XREF: R_LoadWorldRuntime+BD/w
    unsigned __int8 *smodelVisDataCameraSaved;
                                        // XREF: R_LoadWorldRuntime+13D/w
    unsigned __int8 *surfaceVisDataCameraSaved;
                                        // XREF: R_LoadWorldRuntime+176/w
    unsigned int *lodData;              // XREF: R_LoadWorldRuntime+1AE/w
    unsigned __int16 *sortedSurfIndex;  // XREF: R_SortSurfaces+AC/w
                                        // R_SortSurfaces+DE/r ...
    GfxStaticModelInst *smodelInsts;    // XREF: R_PostLoadEntities+A0/r
                                        // R_PostLoadEntities+1AA/r ...
    GfxSurface *surfaces;               // XREF: R_SortSurfaces+D2/r
                                        // R_SortSurfaces+134/r ...
    GfxCullGroup *cullGroups;           // XREF: R_LoadCullGroups+36/w
                                        // R_LoadCullGroupIndices+41/r
    GfxStaticModelDrawInst *smodelDrawInsts;
                                        // XREF: R_PostLoadEntities+84/r
                                        // R_PostLoadEntities+18A/r ...
    GfxDrawSurf *surfaceMaterials;      // XREF: R_LoadWorldRuntime+1F3/w
    unsigned int *surfaceCastsSunShadow; // XREF: R_LoadWorldRuntime+229/w
    volatile int usageCount;
};

struct GfxWorldDpvsPlanes // sizeof=0x10
{                                       // XREF: GfxWorld/r
    int cellCount;                      // XREF: R_PostLoadEntities+2C0/r
    struct cplane_s *planes;                   // XREF: R_LoadWorldInternal(char const *)+1AF/w
    unsigned __int16 *nodes;            // XREF: R_PostLoadEntities:loc_A9921B/r
    unsigned int *sceneEntCellBits;     // XREF: R_LoadWorldRuntime+2A8/w
};

struct BModelSurface // sizeof=0x14
{
    GfxScaledPlacement *placement;
    GfxSurface *surf;
    struct ShaderConstantSet *shaderConstSet;
    float bmodelBurnAmt;
    float bmodelFadeAmt;
};

struct GfxAabbTree // sizeof=0x28
{
    float mins[3];
    float maxs[3];
    unsigned __int16 childCount;
    unsigned __int16 surfaceCount;
    unsigned __int16 startSurfIndex;
    unsigned __int16 smodelIndexCount;
    unsigned __int16 *smodelIndexes;
    int childrenOffset;
};

struct GfxPortal;

struct GfxCell // sizeof=0x38
{
    float mins[3];
    float maxs[3];
    int aabbTreeCount;
    GfxAabbTree *aabbTree;
    int portalCount;
    GfxPortal *portals;
    int cullGroupCount;
    int *cullGroups;
    unsigned __int8 reflectionProbeCount;
    // padding byte
    // padding byte
    // padding byte
    unsigned __int8 *reflectionProbes;
};

struct GfxWorldVertexData // sizeof=0x8
{                                       // XREF: GfxWorldDraw/r
    GfxWorldVertex *vertices;           // XREF: R_LoadSurfaces+151/w
    IDirect3DVertexBuffer9 *worldVb;    // XREF: R_LoadSurfaces+C0F/o
};

struct GfxWorldVertexLayerData // sizeof=0x8
{                                       // XREF: GfxWorldDraw/r
    unsigned __int8 *data;              // XREF: R_LoadSurfaces+10B/w
    IDirect3DVertexBuffer9 *layerVb;    // XREF: R_LoadSurfaces+C29/o
};

union GfxTexture;
struct GfxReflectionProbe;
struct GfxWorldDraw // sizeof=0xC0
{                                       // XREF: GfxWorld/r
    unsigned int reflectionProbeCount;  // XREF: R_GetReflectionProbePosition(uint,float * const)+7/r
                                        // R_CreateDefaultProbes+3/w ...
    struct GfxReflectionProbe *reflectionProbes;
                                        // XREF: R_GetReflectionProbePosition(uint,float * const)+38/r
                                        // R_CreateDefaultProbes+1E/w ...
    union GfxTexture *reflectionProbeTextures;
                                        // XREF: R_CreateDefaultProbes+34/w
                                        // R_LoadReflectionProbes+6B/w ...
    int lightmapCount;                  // XREF: R_LoadLightmaps+107/w
                                        // R_LoadLightmaps+1190/w ...
    GfxLightmapArray *lightmaps;        // XREF: R_LoadLightmaps+241/w
                                        // R_LoadLightmaps+540/r ...
    GfxTexture *lightmapPrimaryTextures; // XREF: R_LoadLightmaps+11F3/w
    GfxTexture *lightmapSecondaryTextures; // XREF: R_LoadLightmaps+1210/w
    GfxTexture *lightmapSecondaryTexturesB;
                                        // XREF: R_LoadLightmaps+122E/w
    GfxImage *terrainScorchImages[31];
    unsigned int vertexCount;           // XREF: R_LoadSurfaces+B0/w
                                        // R_LoadSurfaces+4F2/r ...
    GfxWorldVertexData vd;              // XREF: R_LoadSurfaces+151/w
                                        // R_LoadSurfaces+C08/r ...
    unsigned int vertexLayerDataSize;   // XREF: R_LoadSurfaces+102/w
                                        // R_LoadSurfaces+C1C/r ...
    GfxWorldVertexLayerData vld;        // XREF: R_LoadSurfaces+10B/w
                                        // R_LoadSurfaces+C23/r ...
    unsigned int vertexStream2DataSize; // XREF: R_CalculateVertexStream2Usage+6/w
                                        // R_CalculateVertexStream2Usage+28E/w
    int indexCount;                     // XREF: R_LoadSurfaces+443/w
                                        // R_LoadSurfaces+475/r ...
    unsigned __int16 *indices;          // XREF: R_LoadSurfaces+4AC/w
                                        // R_FinalizeSurfVerts+176/r ...
};

struct GfxWaterBuffer // sizeof=0x8
{                                       // XREF: GfxWorld/r
    unsigned int bufferSize;
    float (*buffer)[4];
};

struct Occluder // sizeof=0x44
{
    unsigned int flags;
    char name[16];
    float points[4][3];
};

struct GfxOutdoorBounds // sizeof=0x18
{
    float bounds[2][3];
};

struct GfxLightRegion;
struct GfxWorld // sizeof=0x43C
{                                       // XREF: .data:GfxWorld s_world/r
    const char *name;                   // XREF: R_LoadWorldInternal(char const *)+92/w
                                        // R_LoadWorldInternal(char const *)+9D/r ...
    const char *baseName;               // XREF: R_LoadWorldInternal(char const *)+141/w
                                        // R_LoadWorldInternal(char const *)+14F/r
    int planeCount;                     // XREF: R_LoadWorldInternal(char const *)+1B9/w
    int nodeCount;                      // XREF: R_LoadNodesAndLeafs+20F/w
                                        // R_LoadNodesAndLeafs+21B/r ...
    int surfaceCount;                   // XREF: R_CalculateVertexStream2Usage+2C/r
                                        // R_CalculateVertexStream2Usage+23C/r ...
    GfxWorldStreamInfo streamInfo;      // XREF: R_GenerateHighmipAabbs+172/w
                                        // R_GenerateHighmipAabbs+18E/w ...
    int skySurfCount;                   // XREF: R_LoadSurfaces+7BD/w
                                        // R_LoadSurfaces+A28/r ...
    int *skyStartSurfs;                 // XREF: R_LoadSurfaces+B09/w
                                        // R_LoadSurfaces+B68/w ...
    GfxImage *skyImage;                 // XREF: R_LoadSurfaces:loc_A9789F/w
                                        // R_SetSkyImage+95/w
    unsigned __int8 skySamplerState;    // XREF: R_SetSkyImage+A0/w
    // padding byte
    // padding byte
    // padding byte
    const char *skyBoxModel;            // XREF: R_ParseSunLight(SunLightParseParams *,char const *)+6D1/w
                                        // R_ParseSunLight(SunLightParseParams *,char const *)+6E2/r
    SunLightParseParams sunParse;       // XREF: R_LoadSunSettings+E/w
                                        // R_LoadSunSettings+1E/w ...
    GfxLight *sunLight;                 // XREF: R_LoadSunSettings+5E/w
                                        // R_LoadSunSettings+63/r
    float sunColorFromBsp[3];
    unsigned int sunPrimaryLightIndex;  // XREF: R_LoadLightGridPoints_Version15:loc_A93602/r
                                        // R_LoadLightGridHeader:loc_A9480C/r ...
    unsigned int primaryLightCount;     // XREF: R_LoadPrimaryLights+16/w
                                        // R_LoadPrimaryLights+2A/w ...
    int cullGroupCount;                 // XREF: R_LoadCullGroups+3F/w
    unsigned int coronaCount;           // XREF: R_LoadCoronas+7/o
                                        // R_LoadCoronas+22/r ...
    GfxLightCorona *coronas;            // XREF: R_LoadCoronas+33/w
                                        // R_LoadCoronas+38/r
    unsigned int shadowMapVolumeCount;  // XREF: R_LoadShadowMapVolumes+7/o
                                        // R_LoadShadowMapVolumes+22/r ...
    GfxShadowMapVolume *shadowMapVolumes;
                                        // XREF: R_LoadShadowMapVolumes+33/w
                                        // R_LoadShadowMapVolumes+38/r
    unsigned int shadowMapVolumePlaneCount;
                                        // XREF: R_LoadShadowMapVolumes:loc_A95D76/o
                                        // R_LoadShadowMapVolumes+E1/r ...
    GfxVolumePlane *shadowMapVolumePlanes;
                                        // XREF: R_LoadShadowMapVolumes+F3/w
                                        // R_LoadShadowMapVolumes+F8/r
    unsigned int exposureVolumeCount;   // XREF: R_LoadExposureVolumes+7/o
                                        // R_LoadExposureVolumes+22/r ...
    GfxExposureVolume *exposureVolumes; // XREF: R_LoadExposureVolumes+33/w
                                        // R_LoadExposureVolumes+38/r
    unsigned int exposureVolumePlaneCount;
                                        // XREF: R_LoadExposureVolumes:loc_A95F41/o
                                        // R_LoadExposureVolumes+11C/r ...
    GfxVolumePlane *exposureVolumePlanes;
                                        // XREF: R_LoadExposureVolumes+12E/w
                                        // R_LoadExposureVolumes+133/r
    GfxSkyDynamicIntensity skyDynIntensity;
                                        // XREF: R_LoadInitSkyIntensity+B/w
                                        // R_LoadInitSkyIntensity+1B/w ...
    GfxWorldDpvsPlanes dpvsPlanes;      // XREF: R_LoadWorldInternal(char const *)+1AF/w
                                        // R_PostLoadEntities:loc_A9921B/r ...
    int cellBitsCount;                  // XREF: R_LoadCells+86/w
    GfxCell *cells;                     // XREF: R_PostLoadEntities+2CE/r
                                        // R_PostLoadEntities+306/r ...
    GfxWorldDraw draw;                  // XREF: R_GetReflectionProbePosition(uint,float * const)+7/r
                                        // R_GetReflectionProbePosition(uint,float * const)+38/r ...
    GfxLightGrid lightGrid;             // XREF: R_LoadLightGridRowData+4/o
                                        // R_LoadLightGridRowData+18/r ...
    int modelCount;                     // XREF: R_SortSurfaces+6/r
                                        // R_SortSurfaces+F/r ...
    GfxBrushModel *models;              // XREF: R_SortSurfaces:loc_A962AB/r
                                        // R_SortSurfaces+46/r ...
    float mins[3];                      // XREF: GlassRenderer::GlassRenderer(Glasses const *)+B40/o
                                        // GlassShard::Update(float)+1B0/r ...
    float maxs[3];                      // XREF: GlassRenderer::GlassRenderer(Glasses const *)+B66/o
                                        // R_LoadSurfaces:loc_A9796B/o
    unsigned int checksum;
    int materialMemoryCount;            // XREF: R_CreateMaterialList(void)+6/w
                                        // R_CreateMaterialList(void):loc_AB943F/r ...
    struct MaterialMemory *materialMemory;     // XREF: R_CreateMaterialList(void)+85/w
                                        // R_CreateMaterialList(void)+102/r
    sunflare_t sun;                     // XREF: R_LoadWorldInternal(char const *)+54E/o
    float outdoorLookupMatrix[4][4];
    GfxImage *outdoorImage;
    unsigned int *cellCasterBits;       // XREF: R_LoadWorldRuntime+26D/w
    struct GfxSceneDynModel *sceneDynModel;    // XREF: R_LoadWorldRuntime+30F/w
    struct GfxSceneDynBrush *sceneDynBrush;    // XREF: R_LoadWorldRuntime+32C/w
    unsigned int *primaryLightEntityShadowVis;
                                        // XREF: R_AllocPrimaryLightBuffers+7A/w
    unsigned int *primaryLightDynEntShadowVis[2];
                                        // XREF: R_AllocPrimaryLightBuffers+D1/w
                                        // R_AllocPrimaryLightBuffers+106/w
    unsigned __int8 *nonSunPrimaryLightForModelDynEnt;
                                        // XREF: R_AllocPrimaryLightBuffers+9C/w
    GfxShadowGeometry *shadowGeom;      // XREF: R_LoadMiscModel+D08/r
                                        // R_LoadMiscModel+D21/r ...
    struct GfxLightRegion *lightRegion;        // XREF: R_LoadLightRegions+20/w
                                        // R_LoadLightRegions+116/r ...
    GfxWorldDpvsStatic dpvs;            // XREF: R_SortSurfaces+AC/w
                                        // R_SortSurfaces+D2/r ...
    GfxWorldDpvsDynamic dpvsDyn;        // XREF: R_LoadWorldInternal(char const *)+1F3/w
                                        // R_LoadWorldInternal(char const *)+1FD/r ...
    unsigned int worldLodChainCount;    // XREF: R_LoadWorldLodData+7/o
                                        // R_LoadWorldLodData+21/r ...
    GfxWorldLodChain *worldLodChains;   // XREF: R_LoadWorldLodData:loc_A9602A/w
                                        // R_LoadWorldLodData+7E/w ...
    unsigned int worldLodInfoCount;     // XREF: R_LoadWorldLodData+48/w
                                        // R_LoadWorldLodData+83/o ...
    GfxWorldLodInfo *worldLodInfos;     // XREF: R_LoadWorldLodData+3E/w
                                        // R_LoadWorldLodData+B0/w ...
    unsigned int worldLodSurfaceCount;  // XREF: R_LoadWorldLodData+5C/w
                                        // R_LoadWorldLodData+B5/o ...
    unsigned int *worldLodSurfaces;     // XREF: R_LoadWorldLodData+52/w
                                        // R_LoadWorldLodData+E2/w ...
    float waterDirection;
    GfxWaterBuffer waterBuffers[2];
    const Material *waterMaterial;      // XREF: R_RenderWaterModel+7/r
                                        // R_RenderWaterModel+2BF/r
    const Material *coronaMaterial;
    const Material *ropeMaterial;
    unsigned int numOccluders;          // XREF: R_SetupWorldSurfacesDpvs(GfxViewParms const *,uint)+68C/r
                                        // R_EnableOccluder(char const *,bool)+1B/r ...
    Occluder *occluders;                // XREF: R_SetupWorldSurfacesDpvs(GfxViewParms const *,uint)+6A1/r
                                        // R_EnableOccluder(char const *,bool)+2D/r ...
    unsigned int numOutdoorBounds;      // XREF: R_LoadOutdoorBounds+6/o
                                        // R_LoadOutdoorBounds+1A/r ...
    GfxOutdoorBounds *outdoorBounds;    // XREF: R_LoadOutdoorBounds+3B/w
                                        // R_LoadOutdoorBounds+40/r ...
    unsigned int heroLightCount;        // XREF: R_LoadHeroOnlyLights+6/w
                                        // R_LoadHeroOnlyLights:loc_A95409/o ...
    unsigned int heroLightTreeCount;    // XREF: R_LoadHeroOnlyLights+10/w
                                        // R_LoadHeroOnlyLights+6C/w ...
    GfxHeroLight *heroLights;           // XREF: R_LoadHeroOnlyLights+1A/w
                                        // R_LoadHeroOnlyLights+8A/w ...
    GfxHeroLightTree *heroLightTree;    // XREF: R_LoadHeroOnlyLights+24/w
                                        // R_LoadHeroOnlyLights+1C7/w ...
};

struct MaterialUsage *__cdecl R_GetMaterialUsageData(Material *material);
void __cdecl R_CreateMaterialList();
void __cdecl R_ShutdownMaterialUsage();
void __cdecl R_InterpretSunLightParseParams(SunLightParseParams *sunParse);
void __cdecl R_UpdateLightsFromDvars();
void __cdecl R_CopyParseParamsFromDvars(SunLightParseParams *sunParse);
void __cdecl R_LoadWorld(char *name, int *checksum, int savegame, int location);
void __cdecl R_CopyParseParamsToDvars(const SunLightParseParams *sunParse, int savegame);
void R_InitDynamicData();
void __cdecl R_SetWorldPtr_LoadObj(const char *name);
void __cdecl R_SetWorldPtr_FastFile(const char *name);
void __cdecl R_ShutdownWorld();
void __cdecl R_ReleaseWorld();
void __cdecl R_ReloadWorld();
unsigned int __cdecl R_GetDebugReflectionProbeLocs(float (*locArray)[3], unsigned int maxCount);
void __cdecl R_BspGenerateReflections();
void __cdecl R_RegisterSkyboxModel(char *xmodelName);

extern GfxWorld s_world;