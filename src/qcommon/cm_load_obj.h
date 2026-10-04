#pragma once

struct MapEnts // sizeof=0xC
{                                       // XREF: XAssetPoolEntry<MapEnts>/r
    const char *name;
    char *entityString;
    int numEntityChars;
};

struct DiskCollBorder // sizeof=0x1C
{
    float distEq[3];
    float zBase;
    float zSlope;
    float start;
    float length;
};

struct DiskBrushModel // sizeof=0x2C
{
    float mins[3];
    float maxs[3];
    unsigned __int16 firstTriSoup;
    unsigned __int16 triSoupCount;
    int firstSurface;
    int numSurfaces;
    int firstBrush;
    int numBrushes;
};

struct dbrush_t // sizeof=0x4
{
    __int16 numSides;
    __int16 materialNum;
};

struct DiskLeaf_Version14 // sizeof=0x24
{
    int cluster;
    int unused0;
    int firstCollAabbIndex;
    int collAabbCount;
    int firstLeafBrush;
    int numLeafBrushes;
    int cellNum;
    int unused1;
    int unused2;
};

struct dbrushside_t // sizeof=0x8
{
    //dbrushside_t::<unnamed_type_u> u;
    union //dbrushside_t::<unnamed_type_u> // sizeof=0x4
    {                                       // XREF: dbrushside_t/r
        int planeNum;
        float bound;
    } u;
    int materialNum;
};

struct cLeafBrushNode_s;
struct cLeaf_s;
struct cplane_s;
struct SpawnVar;

void __cdecl CMod_LoadPlanes();
MapEnts *__cdecl MapEnts_VirtualLoad(char *name);
bool __cdecl MapEnts_CanPurgeEntity(
                const char *classname,
                void *userData,
                bool (__cdecl *HasKeyCallback)(void *, const char *));
MapEnts *__cdecl MapEnts_RealLoad(char *name);
MapEnts *__cdecl MapEnts_GetFromString(char *name, const char *entityString, int numEntityChars);
bool __cdecl CMod_HasSpawnString(const SpawnVar *userData, const char *key);
void __cdecl CM_LoadMapFromBsp(const char *name, bool usePvs);
void CMod_LoadMaterials();
unsigned int CMod_LoadNodes();
void CMod_LoadLeafSurfaces();
void CMod_LoadCollisionVerts();
void CMod_LoadCollisionTriangles();
void CMod_LoadPartitionIndices();
void CMod_LoadCollisionEdgeWalkable();
unsigned int CMod_LoadCollisionBorders();
unsigned int CMod_LoadCollisionPartitions();
MapEnts *CMod_LoadEntityString();
void CMod_LoadVisibility();
void __cdecl CMod_LoadBrushRelated(unsigned int version, bool usePvs);
unsigned int CMod_LoadSubmodels();
unsigned int CMod_LoadSubmodelBrushNodes();
void __cdecl CMod_PartionLeafBrushes(unsigned __int16 *leafBrushes, int numLeafBrushes, cLeaf_s *leaf);
cLeafBrushNode_s *__cdecl CMod_PartionLeafBrushes_r(
                unsigned __int16 *leafBrushes,
                int numLeafBrushes,
                const float *mins,
                const float *maxs);
cLeafBrushNode_s *__cdecl CMod_AllocLeafBrushNode();
double __cdecl CMod_GetPartitionScore(
                unsigned __int16 *leafBrushes,
                int numLeafBrushes,
                int axis,
                const float *mins,
                const float *maxs,
                float *dist);
int __cdecl CMod_GetLeafTerrainContents(cLeaf_s *leaf);
const unsigned __int8 *CMod_LoadBrushes();
void __cdecl CMod_LoadLeafs(bool usePvs);
void __cdecl CMod_LoadLeafs_Version14(bool usePvs);
cLeaf_s *CMod_LoadLeafBrushNodes();
unsigned int CMod_LoadLeafBrushNodes_Version14();
unsigned int CMod_LoadLeafBrushes();
unsigned int CMod_LoadCollisionAabbTrees();
unsigned __int16 *CM_InitBoxHull();
cplane_s *__cdecl CM_GetPlanes();
int __cdecl CM_GetPlaneCount();
char *__cdecl CM_EntityString();
