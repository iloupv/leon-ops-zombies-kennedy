#pragma once
#include <gfx_d3d/r_primarylights.h>

enum LumpType : __int32
{                                       // XREF: BspChunk/r
    LUMP_MATERIALS               = 0x0,
    LUMP_LIGHTBYTES              = 0x1,
    LUMP_LIGHTGRIDENTRIES        = 0x2,
    LUMP_LIGHTGRIDCOLORS         = 0x3,
    LUMP_PLANES                  = 0x4,
    LUMP_BRUSHSIDES              = 0x5,
    LUMP_BRUSHSIDEEDGECOUNTS     = 0x6,
    LUMP_BRUSHEDGES              = 0x7,
    LUMP_BRUSHES                 = 0x8,
    LUMP_TRIANGLES               = 0x9,
    LUMP_DRAWVERTS               = 0xA,
    LUMP_DRAWINDICES             = 0xB,
    LUMP_CULLGROUPS              = 0xC,
    LUMP_CULLGROUPINDICES        = 0xD,
    LUMP_OBSOLETE_1              = 0xE,
    LUMP_OBSOLETE_2              = 0xF,
    LUMP_OBSOLETE_3              = 0x10,
    LUMP_OBSOLETE_4              = 0x11,
    LUMP_OBSOLETE_5              = 0x12,
    LUMP_PORTALVERTS             = 0x13,
    LUMP_OBSOLETE_6              = 0x14,
    LUMP_UINDS                   = 0x15,
    LUMP_BRUSHVERTSCOUNTS        = 0x16,
    LUMP_BRUSHVERTS              = 0x17,
    LUMP_AABBTREES               = 0x18,
    LUMP_CELLS                   = 0x19,
    LUMP_PORTALS                 = 0x1A,
    LUMP_NODES                   = 0x1B,
    LUMP_LEAFS                   = 0x1C,
    LUMP_LEAFBRUSHES             = 0x1D,
    LUMP_LEAFSURFACES            = 0x1E,
    LUMP_COLLISIONVERTS          = 0x1F,
    LUMP_COLLISIONTRIS           = 0x20,
    LUMP_COLLISIONEDGEWALKABLE   = 0x21,
    LUMP_COLLISIONBORDERS        = 0x22,
    LUMP_COLLISIONPARTITIONS     = 0x23,
    LUMP_COLLISIONAABBS          = 0x24,
    LUMP_MODELS                  = 0x25,
    LUMP_VISIBILITY              = 0x26,
    LUMP_ENTITIES                = 0x27,
    LUMP_PATHCONNECTIONS         = 0x28,
    LUMP_REFLECTION_PROBES       = 0x29,
    LUMP_VERTEX_LAYER_DATA       = 0x2A,
    LUMP_PRIMARY_LIGHTS          = 0x2B,
    LUMP_LIGHTGRIDHEADER         = 0x2C,
    LUMP_LIGHTGRIDROWS           = 0x2D,
    LUMP_OBSOLETE_10             = 0x2E,
    LUMP_OBSOLETE_11             = 0x2F,
    LUMP_OBSOLETE_12             = 0x30,
    LUMP_OBSOLETE_13             = 0x31,
    LUMP_OBSOLETE_14             = 0x32,
    LUMP_OBSOLETE_15             = 0x33,
    LUMP_WATERHEADER             = 0x34,
    LUMP_WATERCELLS              = 0x35,
    LUMP_WATERCELLDATA           = 0x36,
    LUMP_BURNABLEHEADER          = 0x37,
    LUMP_BURNABLECELLS           = 0x38,
    LUMP_BURNABLECELLDATA        = 0x39,
    LUMP_SIMPLELIGHTMAPBYTES     = 0x3A,
    LUMP_LODCHAINS               = 0x3B,
    LUMP_LODINFOS                = 0x3C,
    LUMP_LODSURFACES             = 0x3D,
    LUMP_LIGHTREGIONS            = 0x3E,
    LUMP_LIGHTREGION_HULLS       = 0x3F,
    LUMP_LIGHTREGION_AXES        = 0x40,
    LUMP_WIILIGHTGRID            = 0x41,
    LUMP_LIGHTGRID2D_LIGHTS      = 0x42,
    LUMP_LIGHTGRID2D_INDICES     = 0x43,
    LUMP_LIGHTGRID2D_POINTS      = 0x44,
    LUMP_LIGHTGRID2D_CELLS       = 0x45,
    LUMP_LIGHT_CORONAS           = 0x46,
    LUMP_SHADOWMAP_VOLUMES       = 0x47,
    LUMP_SHADOWMAP_VOLUME_PLANES = 0x48,
    LUMP_EXPOSURE_VOLUMES        = 0x49,
    LUMP_EXPOSURE_VOLUME_PLANES  = 0x4A,
    LUMP_OCCLUDERS               = 0x4B,
    LUMP_OUTDOORBOUNDS           = 0x4C,
    LUMP_HERO_ONLY_LIGHTS        = 0x4D,
};

struct BspLump // sizeof=0x8
{                                       // XREF: OldBspHeader/r
    unsigned int length;                // XREF: Com_ReadLumpOutOfBsp+B3/r
    unsigned int offset;                // XREF: Com_ReadLumpOutOfBsp+B7/r
};

struct BspChunk // sizeof=0x8
{                                       // XREF: BspHeader/r
    LumpType type;                      // XREF: Com_ReadLumpOutOfBsp:loc_6CB05D/r
    unsigned int length;                // XREF: Com_ReadLumpOutOfBsp+13D/r
};

struct BspHeader // sizeof=0x32C
{                                       // XREF: ?Com_SaveLump@@YAXW4LumpType@@PBXIW4ComSaveLumpBehavior@@@Z/r
    unsigned int ident;                 // XREF: Com_SaveLump(LumpType,void const *,uint,ComSaveLumpBehavior):loc_6CB821/w
    unsigned int version;               // XREF: Com_SaveLump(LumpType,void const *,uint,ComSaveLumpBehavior)+BB/w
    unsigned int chunkCount;            // XREF: Com_SaveLump(LumpType,void const *,uint,ComSaveLumpBehavior)+C5/w
                                        // Com_SaveLump(LumpType,void const *,uint,ComSaveLumpBehavior)+14A/r ...
    BspChunk chunks[100];               // XREF: Com_SaveLump(LumpType,void const *,uint,ComSaveLumpBehavior)+153/w
};

enum ComSaveLumpBehavior : __int32
{                                       // XREF: ?Com_SaveLump@@YAXW4LumpType@@PBXIW4ComSaveLumpBehavior@@@Z/r
    COM_SAVE_LUMP_AND_CLOSE  = 0x0,
    COM_SAVE_LUMP_AND_REOPEN = 0x1,
};

struct DiskBurnableSample // sizeof=0x1
{                                       // XREF: DiskBurnableCell/r
    unsigned __int8 state;
};

struct ComBurnableSample;

struct DiskPrimaryLight_Version16 // sizeof=0x60
{
    unsigned __int16 falloffStart;
    unsigned __int8 falloffSizeLessOne;
    unsigned __int8 type;
    float color[3];
    float dir[3];
    float origin[3];
    float radius;
    float cosHalfFovOuter;
    float cosHalfFovInner;
    int exponent;
    char defName[40];
};

struct DiskWaterCell // sizeof=0x8
{
    __int16 waterheight;
    char flooroffset;
    unsigned __int8 shoreDist;
    unsigned __int8 color[4];
};

struct DiskBurnableBlock // sizeof=0x20
{
    DiskBurnableSample data[32];
};

struct DiskPrimaryLight // sizeof=0x10C
{
    unsigned __int8 type;
    unsigned __int8 canUseShadowMap;
    __int16 cullDist;
    char priority;
    unsigned __int8 _pad[3];
    float dir[3];
    float origin[3];
    float color[3];
    float specularcolor[3];
    float attenuation[3];
    float angle[3];
    float bouncecolor[3];
    float intensity;
    float cutOn;
    float radius;
    float nearEdge;
    float farEdge;
    float cosHalfFovOuter;
    float cosHalfFovInner;
    int exponent;
    float rotationLimit;
    float translationLimit;
    float roundness;
    float mipDistance;
    float aAbB[4];
    float cookieControl0[4];
    float cookieControl1[4];
    float cookieControl2[4];
    char defName[64];
};

bool __cdecl Com_IsBspLoaded();
char *__cdecl Com_GetBspLump(LumpType type, unsigned int elemSize, unsigned int *count);
char *__cdecl Com_GetBspLumpCountForVersion(int version);
char *__cdecl Com_ValidateBspLumpData(
                LumpType type,
                unsigned int offset,
                unsigned int length,
                unsigned int elemSize,
                unsigned int *count);
const void *__cdecl Com_LoadBspLump(char *mapname, LumpType type, unsigned int elemSize, unsigned int *count);
unsigned __int8 *__cdecl Com_ReadLumpOutOfBsp(int h, LumpType type, unsigned int elemSize, unsigned int *count);
unsigned __int8 *__cdecl Com_ReadLumpOutOfBspAtOffset(
                int h,
                LumpType type,
                unsigned int offset,
                unsigned int length,
                unsigned int elemSize,
                unsigned int *count);
void __cdecl Com_UnloadBspLump(LumpType type);
bool __cdecl Com_BspHasLump(LumpType type);
unsigned int __cdecl Com_GetBspVersion();
unsigned int __cdecl Com_GetBspChecksum();
bool __cdecl Com_BspError();
char __cdecl Com_CheckVersionLumpCountError(int version);
void __cdecl Com_LoadBsp(char *filename);
void __cdecl Com_UnloadBsp();
void __cdecl Com_CleanupBsp();
char *__cdecl Com_EntityString(int *numEntityChars);
void __cdecl Com_SaveLump(LumpType type, const void *newLump, unsigned int size, ComSaveLumpBehavior behavior);
const char *__cdecl Com_GetHunkStringCopy(char *string);
void __cdecl Com_LoadWaterData();
void __cdecl Byte4CopyBgraToVertexColor(const unsigned __int8 *rgbaFrom, unsigned __int8 *nativeTo);
void __cdecl Com_LoadBurnableData();
int __cdecl Com_BurnableCellSort(unsigned int *va, unsigned int *vb);
void __cdecl Com_PackBurnableSample(const DiskBurnableSample *in, ComBurnableSample *out);
void __cdecl Com_LoadWorld(char *name);
void __cdecl Com_LoadWorld_LoadObj(char *name);
ComPrimaryLight *Com_LoadPrimaryLights();
const char *__cdecl Com_GetLightDefName(
                char *defName,
                const ComPrimaryLight *primaryLights,
                unsigned int primaryLightCount);
ComPrimaryLight *Com_LoadPrimaryLights_Version14();
const DiskPrimaryLight_Version16 *Com_LoadPrimaryLights_Version16();
void __cdecl Com_LoadWorld_FastFile(const char *name);
void __cdecl Com_ShutdownWorld();
