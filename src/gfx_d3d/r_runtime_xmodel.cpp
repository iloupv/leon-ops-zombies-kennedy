// Runtime models: XModels built at level load from rtmodels/<name>.rlm (the bo1_rtmodels dvar lists the names)
// instead of from a fastfile. A mod uses this for models it converts on the player's PC from another game the
// player owns, so the mod never ships that game's assets.
//
// The model, its materials and its images are linked into the asset DB as the level zone's own assets
// (DB_LinkXAssetEntry with g_zoneIndex set to the level zone): GSC precache / setModel find them by name, the
// renderer's per-pool tables (material sort, streaming) see real pool entries, the level's
// R_FinishLoadingModels -> XModelOptimize makes the vertex / index buffers, and the zone unload releases the
// GPU side with the level. The CPU arrays are this file's, freed when the next level builds its models.
//
// Version 1 is rigid: one bone (tag_origin), every surface one vertex list on it. Materials are clones of a
// template material from an always-loaded zombies model with the colour map replaced (normal map flat,
// specular black).
//
// .rlm (little-endian; written by the converter): 'RLM1', u32 version 1, u32 texture count, u32 surface count;
// texture: char name[32], u16 width, u16 height, u8 mip count, u8 alpha masked, u16 0, every mip's BGRA8
// pixels, largest first; surface: u32 texture, u32 vertex count, u32 triangle count, vertex count x {f32 pos[3]
// (inches, Z up), f32 normal[3], f32 uv[2]}, triangle count x 3 u16 indices, zero pad to 4.

#include "r_runtime_xmodel.h"
#include "r_material.h"
#include "r_image.h"
#include "r_init.h"
#include "r_dvars.h"
#include "r_utils.h"
#include "r_xsurface.h"
#include "r_xsurface_load_obj.h"
#include "rb_resource.h"
#include <xanim/xmodel.h>
#include <xanim/xanim.h>
#include <xanim/dobj.h>
#include <database/database.h>
#include <database/db_registry.h>
#include <clientscript/cscr_main.h>
#include <clientscript/cscr_stringlist.h>
#include <qcommon/common.h>
#include <qcommon/threads.h>
#include <universal/com_files.h>
#include <universal/com_memory.h>
#include <universal/dvar.h>
#include <universal/q_shared.h>
#include <win32/win_shared.h>
#include <malloc.h>
#include <string.h>
#include <math.h>
#include <vector>

extern XZoneName g_zoneNames[33];
extern unsigned int g_zoneIndex;

static const dvar_t *bo1_rtmodels;

// the template's material: in common_zombie, so on every zombies level
static const char *const s_templateModels[] = { "t5_weapon_m14_world", "defaultactor" };

static std::vector<void *> s_allocs; // this level's runtime model memory

static void *RtAlloc(int size)
{
    void *p = _aligned_malloc(size ? size : 16, 16);
    memset(p, 0, size ? size : 16);
    s_allocs.push_back(p);
    return p;
}

static void RtFreeAll()
{
    for ( void *p : s_allocs )
        _aligned_free(p);
    s_allocs.clear();
}

static char *RtString(const char *s)
{
    char *p = (char *)RtAlloc((int)strlen(s) + 1);
    strcpy(p, s);
    return p;
}

void __cdecl R_RuntimeXModel_RegisterDvars()
{
    if ( !bo1_rtmodels )
        bo1_rtmodels = _Dvar_RegisterString("bo1_rtmodels", "", 0,
            "mod: runtime models built at level load from rtmodels/<name>.rlm (space-separated names)");
}

static int RtLevelZoneIndex(const char *mapName)
{
    for ( int i = 1; i < 33; ++i )
    {
        if ( g_zoneNames[i].name[0] && !I_stricmp(g_zoneNames[i].name, mapName) )
            return i;
    }
    return 0;
}

// links a header into the DB as the level zone's asset; returns the pool copy
static XAssetHeader RtLink(XAssetType type, XAssetHeader header, int zoneIndex)
{
    XAssetEntry entry;
    memset(&entry, 0, sizeof(entry));
    entry.asset.type = type;
    entry.asset.header = header;
    entry.zoneIndex = (unsigned __int8)zoneIndex;
    Sys_EnterCriticalSection(CRITSECT_DBHASH);
    unsigned int savedZone = g_zoneIndex;
    g_zoneIndex = zoneIndex;
    XAssetEntryPoolEntry *linked = DB_LinkXAssetEntry(&entry, 0);
    g_zoneIndex = savedZone;
    Sys_LeaveCriticalSection(CRITSECT_DBHASH);
    return linked->entry.asset.header;
}

struct RtReader
{
    const unsigned __int8 *base, *p, *end;
    bool ok = true;
    const void *take(size_t n)
    {
        if ( !ok || (size_t)(end - p) < n )
        {
            ok = false;
            return 0;
        }
        const void *r = p;
        p += n;
        return r;
    }
    unsigned int u32() { const void *r = take(4); return r ? *(const unsigned int *)r : 0; }
    unsigned short u16() { const void *r = take(2); return r ? *(const unsigned short *)r : 0; }
    unsigned char u8() { const void *r = take(1); return r ? *(const unsigned char *)r : 0; }
};

static Material *RtTemplateMaterial()
{
    for ( const char *name : s_templateModels )
    {
        if ( !DB_FindXAssetEntry(ASSET_TYPE_XMODEL, name) )
            continue;
        XModel *model = DB_FindXAssetHeader(ASSET_TYPE_XMODEL, (char *)name, 1, -1).model;
        if ( model && model->numsurfs && model->materialHandles && model->materialHandles[0] )
            return model->materialHandles[0];
    }
    return 0;
}

static GfxImage *RtCreateImage(RtReader &r, const char *modelName, int zoneIndex, bool gpu)
{
    char texName[33] = {};
    if ( const void *n = r.take(32) )
        memcpy(texName, n, 32);
    unsigned int width = r.u16(), height = r.u16(), mips = r.u8();
    r.u8(); // alpha masked: version 1 draws it opaque
    r.u16();
    size_t bytes = 0;
    for ( unsigned int m = 0; m < mips; ++m )
        bytes += (size_t)((width >> m) ? (width >> m) : 1) * ((height >> m) ? (height >> m) : 1) * 4;
    const void *pixels = r.take(bytes);
    if ( !r.ok || !width || !height || !mips )
        return 0;

    GfxImage image;
    memset(&image, 0, sizeof(image));
    image.mapType = 3; // MAPTYPE_2D
    image.semantic = 2; // colour map
    image.category = 3;
    image.noPicmip = 1;
    image.width = (unsigned __int16)width;
    image.height = (unsigned __int16)height;
    image.depth = 1;
    image.levelCount = (unsigned __int8)mips;
    image.name = RtString(va("%s/%s", modelName, texName));
    GfxImage *pooled = RtLink(ASSET_TYPE_IMAGE, (XAssetHeader)&image, zoneIndex).image;
    if ( gpu )
    {
        void *buffer = Z_VirtualAlloc((int)bytes, "Image_AllocTempMemory", 20);
        memcpy(buffer, pixels, bytes);
        RB_Resource_CreateTexture(pooled, buffer, (int)mips, 0, D3DFMT_A8R8G8B8);
    }
    return pooled;
}

static Material *RtCreateMaterial(const Material *templ, GfxImage *colorMap, const char *name, int zoneIndex)
{
    Material material;
    memcpy(&material, templ, sizeof(material));
    material.info.name = RtString(name);
    MaterialTextureDef *table = (MaterialTextureDef *)RtAlloc(sizeof(MaterialTextureDef) * (templ->textureCount ? templ->textureCount : 1));
    const unsigned int colorMapHash = R_HashString("colorMap");
    for ( int i = 0; i < templ->textureCount; ++i )
    {
        table[i] = templ->textureTable[i];
        if ( table[i].nameHash == colorMapHash )
            table[i].u.image = colorMap;
        else if ( table[i].semantic == 5 ) // normal map: flat
            table[i].u.image = rgp.identityNormalMapImage;
        else if ( table[i].semantic == 8 ) // specular: none
            table[i].u.image = rgp.blackImage;
    }
    material.textureTable = table;
    return RtLink(ASSET_TYPE_MATERIAL, (XAssetHeader)&material, zoneIndex).material;
}

static void RtBuildSurface(RtReader &r, XSurface *surf, float *mins, float *maxs)
{
    unsigned int vertCount = r.u32(), triCount = r.u32();
    const float *src = (const float *)r.take((size_t)vertCount * 32);
    const unsigned short *idx = (const unsigned short *)r.take((size_t)triCount * 6);
    while ( r.ok && ((r.p - r.base) & 3) )
        r.take(1);
    if ( !r.ok || !vertCount || vertCount > 0xFFFF || triCount > 0xFFFF )
    {
        r.ok = false;
        return;
    }

    XVertexInfo_s *info = (XVertexInfo_s *)_aligned_malloc(sizeof(XVertexInfo_s) * vertCount, 16);
    memset(info, 0, sizeof(XVertexInfo_s) * vertCount);
    for ( unsigned int v = 0; v < vertCount; ++v )
    {
        const float *f = src + 8 * v;
        XVertexInfo_s &x = info[v];
        for ( int k = 0; k < 3; ++k )
        {
            x.offset[k] = f[k];
            x.normal[k] = f[3 + k];
            mins[k] = f[k] < mins[k] ? f[k] : mins[k];
            maxs[k] = f[k] > maxs[k] ? f[k] : maxs[k];
        }
        // a tangent frame around the normal (version 1 uses a flat normal map, so any frame will do)
        float up[3] = { 0.0f, 0.0f, 1.0f };
        if ( fabsf(x.normal[2]) > 0.9f )
            up[0] = 1.0f, up[2] = 0.0f;
        float t[3] = { up[1] * x.normal[2] - up[2] * x.normal[1], up[2] * x.normal[0] - up[0] * x.normal[2],
                       up[0] * x.normal[1] - up[1] * x.normal[0] };
        float len = sqrtf(t[0] * t[0] + t[1] * t[1] + t[2] * t[2]);
        len = len > 0.0f ? 1.0f / len : 0.0f;
        for ( int k = 0; k < 3; ++k )
            x.tangent[k] = t[k] * len;
        x.binormal[0] = x.normal[1] * x.tangent[2] - x.normal[2] * x.tangent[1];
        x.binormal[1] = x.normal[2] * x.tangent[0] - x.normal[0] * x.tangent[2];
        x.binormal[2] = x.normal[0] * x.tangent[1] - x.normal[1] * x.tangent[0];
        x.texCoordX = f[6];
        x.texCoordY = f[7];
        x.color[0] = x.color[1] = x.color[2] = x.color[3] = 0xFF;
        x.numWeights = 0;
        x.boneOffset = 0;
    }

    surf->tileMode = 0;
    surf->flags = 0;
    surf->vertCount = (unsigned __int16)vertCount;
    surf->triCount = (unsigned __int16)triCount;
    surf->verts0 = (GfxPackedVertex *)RtAlloc(sizeof(GfxPackedVertex) * vertCount);
    XSurfaceTransfer((const XVertexBuffer *)info, surf->verts0, surf->verts0, (int)vertCount);
    _aligned_free(info);
    surf->triIndices = (unsigned __int16 *)RtAlloc((int)triCount * 6);
    memcpy(surf->triIndices, idx, (size_t)triCount * 6);
    surf->vertListCount = 1;
    surf->vertList = (XRigidVertList *)RtAlloc(sizeof(XRigidVertList));
    surf->vertList->boneOffset = 0; // bone 0 << 6
    surf->vertList->vertCount = (unsigned __int16)vertCount;
    surf->vertList->triOffset = 0;
    surf->vertList->triCount = (unsigned __int16)triCount;
    XModelReadSurface_BuildCollisionTree(surf, 0, RtAlloc);
    surf->partBits[0] = (int)0x80000000; // bone 0
}

static void RtLoadModel(const char *name, int zoneIndex, const Material *templ, bool gpu)
{
    void *file = 0;
    int size = FS_ReadFile(va("rtmodels/%s.rlm", name), &file);
    if ( size <= 0 || !file )
    {
        Com_PrintWarning(0, "runtime model '%s': rtmodels/%s.rlm not found\n", name, name);
        return;
    }
    RtReader r{ (const unsigned __int8 *)file, (const unsigned __int8 *)file, (const unsigned __int8 *)file + size };
    const void *magic = r.take(4);
    unsigned int version = r.u32(), texCount = r.u32(), surfCount = r.u32();
    if ( !r.ok || memcmp(magic, "RLM1", 4) || version != 1 || !surfCount || surfCount > 255 )
    {
        Com_PrintWarning(0, "runtime model '%s': not a version 1 .rlm file\n", name);
        FS_FreeFile(file);
        return;
    }

    std::vector<Material *> materials;
    for ( unsigned int t = 0; t < texCount && r.ok; ++t )
    {
        GfxImage *image = RtCreateImage(r, name, zoneIndex, gpu);
        materials.push_back(image ? RtCreateMaterial(templ, image, va("%s_mtl%u", name, t), zoneIndex) : 0);
    }

    XModel model;
    memset(&model, 0, sizeof(model));
    model.name = RtString(name);
    model.numBones = 1;
    model.numRootBones = 1;
    model.numsurfs = (unsigned __int8)surfCount;
    model.boneNames = (unsigned __int16 *)RtAlloc(sizeof(unsigned __int16));
    model.boneNames[0] = (unsigned __int16)SL_GetString((char *)"tag_origin", 4, SCRIPTINSTANCE_SERVER);
    model.partClassification = (unsigned __int8 *)RtAlloc(1);
    model.baseMat = (DObjAnimMat *)RtAlloc(sizeof(DObjAnimMat));
    model.baseMat->quat[3] = 1.0f;
    model.baseMat->transWeight = 2.0f;
    model.surfs = (XSurface *)RtAlloc(sizeof(XSurface) * surfCount);
    model.materialHandles = (Material **)RtAlloc(sizeof(Material *) * surfCount);
    float mins[3] = { 1e9f, 1e9f, 1e9f }, maxs[3] = { -1e9f, -1e9f, -1e9f };
    int baseTri = 0, baseVert = 0;
    for ( unsigned int s = 0; s < surfCount && r.ok; ++s )
    {
        unsigned int tex = r.u32();
        XSurface *surf = &model.surfs[s];
        RtBuildSurface(r, surf, mins, maxs);
        surf->baseTriIndex = (unsigned __int16)baseTri;
        surf->baseVertIndex = (unsigned __int16)baseVert;
        baseTri += surf->triCount;
        baseVert += surf->vertCount;
        model.materialHandles[s] = tex < materials.size() && materials[tex] ? materials[tex] : (Material *)templ;
    }
    FS_FreeFile(file);
    if ( !r.ok )
    {
        Com_PrintWarning(0, "runtime model '%s': truncated or invalid .rlm file\n", name);
        return;
    }

    model.lodInfo[0].dist = 0.0f; // never switched out
    model.lodInfo[0].numsurfs = (unsigned __int16)surfCount;
    model.lodInfo[0].surfIndex = 0;
    model.lodInfo[0].partBits[0] = (int)0x80000000;
    model.numLods = 1;
    model.collLod = -1;
    model.boneInfo = (XBoneInfo *)RtAlloc(sizeof(XBoneInfo));
    float radiusSq = 0.0f;
    for ( int k = 0; k < 3; ++k )
    {
        model.mins[k] = mins[k];
        model.maxs[k] = maxs[k];
        model.boneInfo->bounds[0][k] = mins[k];
        model.boneInfo->bounds[1][k] = maxs[k];
        model.boneInfo->offset[k] = 0.5f * (mins[k] + maxs[k]);
        float half = 0.5f * (maxs[k] - mins[k]);
        radiusSq += half * half;
    }
    model.boneInfo->radiusSquared = radiusSq;
    model.radius = sqrtf(radiusSq);
    model.streamInfo.highMipBounds = (XModelHighMipBounds *)RtAlloc(sizeof(XModelHighMipBounds) * surfCount);
    RtLink(ASSET_TYPE_XMODEL, (XAssetHeader)&model, zoneIndex);
    Com_Printf(0, "runtime model '%s': %u surfaces, %d vertices, %d triangles, %u textures (level zone %d)\n",
        name, surfCount, baseVert, baseTri, texCount, zoneIndex);
}

// Called by CL_InitCGame once the level's fastfiles are queued, before CG_Init registers the models.
void __cdecl R_RuntimeXModel_LoadForLevel(const char *mapName)
{
    R_RuntimeXModel_RegisterDvars();
    if ( !bo1_rtmodels->current.string[0] || IsDedicatedServer() )
        return;
    Com_Printf(0, "runtime models: level '%s', building '%s'\n", mapName, bo1_rtmodels->current.string);
    DB_SyncXAssets();
    const int zoneIndex = RtLevelZoneIndex(mapName);
    if ( !zoneIndex )
    {
        Com_PrintWarning(0, "runtime models: no level zone '%s'\n", mapName);
        return;
    }

    char names[1024];
    I_strncpyz(names, bo1_rtmodels->current.string, sizeof(names));
    // already built for this level (a restart keeps the zone)?
    bool built = true;
    for ( char *tok = strtok(names, " ,"); tok; tok = strtok(0, " ,") )
    {
        XAssetEntryPoolEntry *e = DB_FindXAssetEntry(ASSET_TYPE_XMODEL, tok);
        if ( !e || e->entry.zoneIndex != zoneIndex )
            built = false;
    }
    if ( built )
        return;

    const Material *templ = RtTemplateMaterial();
    if ( !templ )
    {
        Com_PrintWarning(0, "runtime models: no template material on this level\n");
        return;
    }
    RtFreeAll(); // the previous level's arrays; its zone (and the GPU side) is gone
    const bool gpu = !r_loadForRenderer || r_loadForRenderer->current.enabled;
    I_strncpyz(names, bo1_rtmodels->current.string, sizeof(names));
    for ( char *tok = strtok(names, " ,"); tok; tok = strtok(0, " ,") )
        RtLoadModel(tok, zoneIndex, templ, gpu);
    if ( gpu )
        RB_Resource_Flush();
}
