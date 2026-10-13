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
//
// Version 2 adds a skeleton and skinned vertices, and optionally, after the surfaces, baked animations: 'ANIM',
// u32 count, per animation char name[32], u32 frame count, f32 frames per second, u32 bone count (the model's),
// frame count x bone count x {f32 local rotation quaternion[4], f32 translation offset from the rest pose[3]}.
// The first one loops on every DObj of the model (R_RuntimeXModel_PoseDObj), in place of an XAnim tree.

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
#include <string>

extern XZoneName g_zoneNames[33];
extern unsigned int g_zoneIndex;

static const dvar_t *bo1_rtmodels;

// the template's material: in common_zombie, so on every zombies level
static const char *const s_templateModels[] = { "t5_weapon_m14_world", "defaultactor" };

static std::vector<void *> s_allocs; // this level's runtime model memory

struct RtAnim
{
    const XModel *model; // the DB pool copy DObjs point at
    int frameCount;
    float fps;
    int boneCount;
    const float *frames; // frameCount x boneCount x {quat[4], trans offset[3]}
};
static std::vector<RtAnim> s_anims; // this level's

static void *RtAlloc(int size)
{
    void *p = _aligned_malloc(size ? size : 16, 16);
    memset(p, 0, size ? size : 16);
    s_allocs.push_back(p);
    return p;
}

static void RtFreeAll()
{
    s_anims.clear();
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

// The vertex list's collision tree (impact marks: XSurfaceVisitTrianglesInAabb). The engine's builder
// (XModelReadSurface_BuildCollisionTree) does not return on these surfaces, so build the smallest valid tree:
// one root node over the whole surface whose children are leaves (childCount | 0x8000), one triangle per leaf.
// Queries then test every triangle, which is fine for the few marks a character takes.
static XSurfaceCollisionTree *RtBuildCollisionTree(const XSurface *surf)
{
    float lo[3] = { 1e9f, 1e9f, 1e9f }, hi[3] = { -1e9f, -1e9f, -1e9f };
    for ( unsigned int v = 0; v < surf->vertCount; ++v )
    {
        for ( int k = 0; k < 3; ++k )
        {
            const float x = surf->verts0[v].xyz[k];
            lo[k] = x < lo[k] ? x : lo[k];
            hi[k] = x > hi[k] ? x : hi[k];
        }
    }
    XSurfaceCollisionTree *tree = (XSurfaceCollisionTree *)RtAlloc(sizeof(XSurfaceCollisionTree));
    for ( int k = 0; k < 3; ++k )
    {
        tree->trans[k] = -lo[k];
        tree->scale[k] = hi[k] > lo[k] ? 65535.0f / (hi[k] - lo[k]) : 1.0f;
    }
    const unsigned int leafCount = surf->triCount < 0x7FFF ? surf->triCount : 0x7FFF;
    tree->nodeCount = 1;
    tree->nodes = (XSurfaceCollisionNode *)RtAlloc(sizeof(XSurfaceCollisionNode));
    for ( int k = 0; k < 3; ++k )
    {
        tree->nodes[0].aabb.mins[k] = 0;
        tree->nodes[0].aabb.maxs[k] = 0xFFFF;
    }
    tree->nodes[0].childBeginIndex = 0;
    tree->nodes[0].childCount = (unsigned __int16)(0x8000 | leafCount);
    tree->leafCount = leafCount;
    tree->leafs = (XSurfaceCollisionLeaf *)RtAlloc(sizeof(XSurfaceCollisionLeaf) * (leafCount ? leafCount : 1));
    for ( unsigned int t = 0; t < leafCount; ++t )
        tree->leafs[t].triangleBeginIndex = (unsigned __int16)t;
    return tree;
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
    surf->vertList->collisionTree = RtBuildCollisionTree(surf);
    surf->partBits[0] = (int)0x80000000; // bone 0
}

struct RtSkinVert // the .rlm v2 vertex
{
    float pos[3];
    float normal[3];
    float uv[2];
    unsigned __int16 bone[4];
    float weight[4];
};

static void RtFillVertexInfo(XVertexInfo_s &x, const float *pos, const float *normal, const float *uv)
{
    for ( int k = 0; k < 3; ++k )
    {
        x.offset[k] = pos[k];
        x.normal[k] = normal[k];
    }
    // a tangent frame around the normal (the materials use a flat normal map, so any frame will do)
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
    x.texCoordX = uv[0];
    x.texCoordY = uv[1];
    x.color[0] = x.color[1] = x.color[2] = x.color[3] = 0xFF;
    x.numWeights = 0;
    x.boneOffset = 0;
}

// A weighted (deformed) surface, skinned on the CPU from the DObj's bone matrices (R_SkinXSurfaceWeight*): the
// vertices sorted by extra-weight count (0..3: vertInfo.vertCount[k]), vertsBlend per vertex {bone0 << 6, then
// (bone << 6, weight * 65536) per extra weight} - bone 0's weight is the rest, and it alone turns the normal, so
// it is the heaviest. No rigid vertex lists, so no collision tree (marks skip deformed surfaces).
static void RtBuildSkinnedSurface(RtReader &r, XSurface *surf, int numBones, const float (*boneWorld)[3],
                                  float (*boneMins)[3], float (*boneMaxs)[3], float *mins, float *maxs)
{
    unsigned int vertCount = r.u32(), triCount = r.u32();
    const RtSkinVert *src = (const RtSkinVert *)r.take((size_t)vertCount * sizeof(RtSkinVert));
    const unsigned short *idx = (const unsigned short *)r.take((size_t)triCount * 6);
    while ( r.ok && ((r.p - r.base) & 3) )
        r.take(1);
    if ( !r.ok || !vertCount || vertCount > 0xFFFF || triCount > 0xFFFF )
    {
        r.ok = false;
        return;
    }

    // extra-weight count per vertex, then the new order (stable within a group)
    std::vector<int> group(vertCount), order, newIndex(vertCount);
    int groupCount[4] = {};
    for ( unsigned int v = 0; v < vertCount; ++v )
    {
        int n = 0;
        for ( int k = 1; k < 4; ++k )
            if ( src[v].weight[k] > 0.0f && src[v].bone[k] < numBones )
                n = k;
        group[v] = n;
        ++groupCount[n];
    }
    for ( int g = 0; g < 4; ++g )
        for ( unsigned int v = 0; v < vertCount; ++v )
            if ( group[v] == g )
            {
                newIndex[v] = (int)order.size();
                order.push_back((int)v);
            }

    XVertexInfo_s *info = (XVertexInfo_s *)_aligned_malloc(sizeof(XVertexInfo_s) * vertCount, 16);
    memset(info, 0, sizeof(XVertexInfo_s) * vertCount);
    size_t blendCount = 0;
    for ( int g = 0; g < 4; ++g )
        blendCount += (size_t)groupCount[g] * (1 + 2 * g);
    surf->vertInfo.vertsBlend = (unsigned __int16 *)RtAlloc((int)(blendCount * 2));
    unsigned __int16 *blend = surf->vertInfo.vertsBlend;
    for ( unsigned int n = 0; n < vertCount; ++n )
    {
        const RtSkinVert &v = src[order[n]];
        RtFillVertexInfo(info[n], v.pos, v.normal, v.uv);
        const int bone0 = v.bone[0] < numBones ? v.bone[0] : 0;
        *blend++ = (unsigned __int16)(bone0 << 6);
        surf->partBits[bone0 >> 5] |= (int)(0x80000000u >> (bone0 & 31));
        for ( int k = 1; k <= group[order[n]]; ++k )
        {
            const int b = v.bone[k];
            const float w = v.weight[k] * 65536.0f + 0.5f;
            *blend++ = (unsigned __int16)(b << 6);
            *blend++ = (unsigned __int16)(w > 65535.0f ? 65535.0f : w);
            surf->partBits[b >> 5] |= (int)(0x80000000u >> (b & 31));
        }
        for ( int k = 0; k < 3; ++k )
        {
            mins[k] = v.pos[k] < mins[k] ? v.pos[k] : mins[k];
            maxs[k] = v.pos[k] > maxs[k] ? v.pos[k] : maxs[k];
            const float local = v.pos[k] - boneWorld[bone0][k]; // bone space: the bind pose has no rotation
            boneMins[bone0][k] = local < boneMins[bone0][k] ? local : boneMins[bone0][k];
            boneMaxs[bone0][k] = local > boneMaxs[bone0][k] ? local : boneMaxs[bone0][k];
        }
    }
    for ( int g = 0; g < 4; ++g )
        surf->vertInfo.vertCount[g] = (__int16)groupCount[g];

    surf->tileMode = 0;
    surf->flags = 0x82; // XSURFACE_FLAG_DEFORMED | XSURFACE_FLAG_SKINNED
    surf->vertCount = (unsigned __int16)vertCount;
    surf->triCount = (unsigned __int16)triCount;
    surf->verts0 = (GfxPackedVertex *)RtAlloc(sizeof(GfxPackedVertex) * vertCount);
    XSurfaceTransfer((const XVertexBuffer *)info, surf->verts0, surf->verts0, (int)vertCount);
    _aligned_free(info);
    surf->triIndices = (unsigned __int16 *)RtAlloc((int)triCount * 6);
    for ( unsigned int i = 0; i < triCount * 3; ++i )
        surf->triIndices[i] = (unsigned __int16)newIndex[idx[i] < vertCount ? idx[i] : 0];
    surf->vertListCount = 0;
    surf->vertList = 0;
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
    unsigned int boneCount = version >= 2 ? r.u32() : 1;
    if ( !r.ok || memcmp(magic, "RLM1", 4) || (version != 1 && version != 2) || !surfCount || surfCount > 255 || !boneCount
         || boneCount >= 160 )
    {
        Com_PrintWarning(0, "runtime model '%s': not a version 1 or 2 .rlm file\n", name);
        FS_FreeFile(file);
        return;
    }

    std::vector<Material *> materials;
    for ( unsigned int t = 0; t < texCount && r.ok; ++t )
    {
        GfxImage *image = RtCreateImage(r, name, zoneIndex, gpu);
        materials.push_back(image ? RtCreateMaterial(templ, image, va("%s_mtl%u", name, t), zoneIndex) : 0);
    }

    // the skeleton (version 1: only tag_origin); roots first, parents before children
    std::vector<int> parent(boneCount, -1);
    std::vector<float> localTrans(boneCount * 3, 0.0f), localQuat(boneCount * 4, 0.0f);
    std::vector<std::string> boneName(boneCount, "tag_origin");
    localQuat[3] = 1.0f;
    for ( unsigned int b = 0; version >= 2 && b < boneCount && r.ok; ++b )
    {
        char boneStr[33] = {};
        if ( const void *n = r.take(32) )
            memcpy(boneStr, n, 32);
        boneName[b] = boneStr;
        parent[b] = (int)r.u32();
        const float *f = (const float *)r.take(7 * sizeof(float));
        for ( int k = 0; f && k < 3; ++k )
            localTrans[3 * b + k] = f[k];
        for ( int k = 0; f && k < 4; ++k )
            localQuat[4 * b + k] = f[3 + k];
    }
    int numRoot = 0;
    while ( numRoot < (int)boneCount && parent[numRoot] < 0 )
        ++numRoot;
    for ( unsigned int b = numRoot; b < boneCount; ++b )
        if ( parent[b] < 0 || parent[b] >= (int)b || (int)b - parent[b] > 255 )
            r.ok = false; // roots first, parents before children, within the u8 parent offset
    if ( !r.ok || !numRoot )
    {
        Com_PrintWarning(0, "runtime model '%s': bad skeleton\n", name);
        FS_FreeFile(file);
        return;
    }
    // bind pose: the vertices' rest pose, every bone unrotated at its accumulated offset
    std::vector<float> world(boneCount * 3, 0.0f);
    for ( unsigned int b = numRoot; b < boneCount; ++b )
        for ( int k = 0; k < 3; ++k )
            world[3 * b + k] = world[3 * parent[b] + k] + localTrans[3 * b + k];

    XModel model;
    memset(&model, 0, sizeof(model));
    model.name = RtString(name);
    model.numBones = (unsigned __int8)boneCount;
    model.numRootBones = (unsigned __int8)numRoot;
    model.numsurfs = (unsigned __int8)surfCount;
    model.boneNames = (unsigned __int16 *)RtAlloc(sizeof(unsigned __int16) * boneCount);
    for ( unsigned int b = 0; b < boneCount; ++b )
        model.boneNames[b] = (unsigned __int16)SL_GetString((char *)boneName[b].c_str(), 4, SCRIPTINSTANCE_SERVER);
    const int nonRoot = (int)boneCount - numRoot;
    if ( nonRoot )
    {
        // the local pose DObjCalcBaseAnim builds the skeleton from: parent offset (child - parent), the rotation in
        // s16 (x 32767) and the translation, per non-root bone
        model.parentList = (unsigned __int8 *)RtAlloc(nonRoot);
        model.quats = (__int16 *)RtAlloc(sizeof(__int16) * 4 * nonRoot);
        model.trans = (float *)RtAlloc(sizeof(float) * 3 * nonRoot);
        for ( int i = 0; i < nonRoot; ++i )
        {
            const int b = numRoot + i;
            model.parentList[i] = (unsigned __int8)(b - parent[b]);
            const float *q = &localQuat[4 * b];
            const float len = sqrtf(q[0] * q[0] + q[1] * q[1] + q[2] * q[2] + q[3] * q[3]);
            for ( int k = 0; k < 4; ++k )
            {
                const float c = len > 0.0f ? q[k] / len : (k == 3 ? 1.0f : 0.0f);
                model.quats[4 * i + k] = (__int16)floorf(c * 32767.0f + 0.5f);
            }
            for ( int k = 0; k < 3; ++k )
                model.trans[3 * i + k] = localTrans[3 * b + k];
        }
    }
    model.partClassification = (unsigned __int8 *)RtAlloc(boneCount);
    model.baseMat = (DObjAnimMat *)RtAlloc(sizeof(DObjAnimMat) * boneCount);
    for ( unsigned int b = 0; b < boneCount; ++b )
    {
        model.baseMat[b].quat[3] = 1.0f;
        model.baseMat[b].transWeight = 2.0f;
        for ( int k = 0; k < 3; ++k )
            model.baseMat[b].trans[k] = world[3 * b + k];
    }

    model.surfs = (XSurface *)RtAlloc(sizeof(XSurface) * surfCount);
    model.materialHandles = (Material **)RtAlloc(sizeof(Material *) * surfCount);
    float mins[3] = { 1e9f, 1e9f, 1e9f }, maxs[3] = { -1e9f, -1e9f, -1e9f };
    std::vector<float> boneMins(boneCount * 3, 1e9f), boneMaxs(boneCount * 3, -1e9f);
    int baseTri = 0, baseVert = 0;
    for ( unsigned int s = 0; s < surfCount && r.ok; ++s )
    {
        unsigned int tex = r.u32();
        XSurface *surf = &model.surfs[s];
        if ( version >= 2 )
        {
            RtBuildSkinnedSurface(r, surf, (int)boneCount, (const float (*)[3])world.data(), (float (*)[3])boneMins.data(),
                                  (float (*)[3])boneMaxs.data(), mins, maxs);
        }
        else
        {
            RtBuildSurface(r, surf, mins, maxs);
            for ( int k = 0; k < 3; ++k )
            {
                boneMins[k] = mins[k];
                boneMaxs[k] = maxs[k];
            }
        }
        surf->baseTriIndex = (unsigned __int16)baseTri;
        surf->baseVertIndex = (unsigned __int16)baseVert;
        baseTri += surf->triCount;
        baseVert += surf->vertCount;
        for ( int k = 0; k < 5; ++k )
            model.lodInfo[0].partBits[k] |= surf->partBits[k];
        if ( surf->flags & 0x80 )
            model.lodRampType = 1;
        model.materialHandles[s] = tex < materials.size() && materials[tex] ? materials[tex] : (Material *)templ;
    }
    std::vector<RtAnim> anims;
    if ( r.ok && version >= 2 && r.end - r.p >= 8 && !memcmp(r.p, "ANIM", 4) )
    {
        r.take(4);
        const unsigned int animCount = r.u32();
        for ( unsigned int a = 0; a < animCount && r.ok; ++a )
        {
            r.take(32);
            RtAnim anim = {};
            anim.frameCount = (int)r.u32();
            const void *fps = r.take(4);
            anim.fps = fps ? *(const float *)fps : 0.0f;
            anim.boneCount = (int)r.u32();
            if ( anim.frameCount <= 0 || anim.frameCount > 100000 || anim.boneCount != (int)boneCount || !(anim.fps > 0.0f) )
            {
                r.ok = false;
                break;
            }
            const int size = anim.frameCount * anim.boneCount * 7 * (int)sizeof(float);
            if ( const void *src = r.take(size) )
            {
                float *frames = (float *)RtAlloc(size);
                memcpy(frames, src, size);
                anim.frames = frames;
                anims.push_back(anim);
            }
        }
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
    model.numLods = 1;
    model.collLod = -1;
    model.boneInfo = (XBoneInfo *)RtAlloc(sizeof(XBoneInfo) * boneCount);
    for ( unsigned int b = 0; b < boneCount; ++b )
    {
        XBoneInfo *bi = &model.boneInfo[b];
        if ( boneMins[3 * b] > boneMaxs[3 * b] )
            continue; // no vertices: an empty box at the bone
        float radiusSq = 0.0f;
        for ( int k = 0; k < 3; ++k )
        {
            bi->bounds[0][k] = boneMins[3 * b + k];
            bi->bounds[1][k] = boneMaxs[3 * b + k];
            bi->offset[k] = 0.5f * (boneMins[3 * b + k] + boneMaxs[3 * b + k]);
            const float half = 0.5f * (boneMaxs[3 * b + k] - boneMins[3 * b + k]);
            radiusSq += half * half;
        }
        bi->radiusSquared = radiusSq;
    }
    float radiusSq = 0.0f;
    for ( int k = 0; k < 3; ++k )
    {
        model.mins[k] = mins[k];
        model.maxs[k] = maxs[k];
        const float half = 0.5f * (maxs[k] - mins[k]);
        radiusSq += half * half;
    }
    model.radius = sqrtf(radiusSq);
    model.streamInfo.highMipBounds = (XModelHighMipBounds *)RtAlloc(sizeof(XModelHighMipBounds) * surfCount);
    const XModel *linked = RtLink(ASSET_TYPE_XMODEL, (XAssetHeader)&model, zoneIndex).model;
    for ( RtAnim &anim : anims )
    {
        anim.model = linked;
        s_anims.push_back(anim);
    }
    Com_Printf(0, "runtime model '%s': %u surfaces, %d vertices, %d triangles, %u textures, %u bones, %u animations"
        " (level zone %d)\n", name, surfCount, baseVert, baseTri, texCount, boneCount, (unsigned int)anims.size(), zoneIndex);
}

// Called by CL_InitCGame once the level's fastfiles are queued, before CG_Init registers the models.
void __cdecl R_RuntimeXModel_LoadForLevel(const char *mapName)
{
    R_RuntimeXModel_RegisterDvars();
    if ( !bo1_rtmodels->current.string[0] || IsDedicatedServer() )
        return;
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

// Called by CG_DObjCalcPose before DObjCalcSkel. A DObj whose model has a baked animation gets every bone's local
// pose from it (the first animation, looping, neighbouring frames blended), marked as animated so DObjCalcAnim
// leaves the bones alone; DObjCalcSkel then builds the skeleton as for an XAnim pose (the rotation absolute, the
// translation added to the model's rest offset).
void __cdecl R_RuntimeXModel_PoseDObj(DObj *obj, int timeMs)
{
    if ( s_anims.empty() || !obj->numModels || !obj->skel.mat )
        return;
    const XModel *model = obj->localModels[0];
    const RtAnim *anim = 0;
    for ( const RtAnim &a : s_anims )
    {
        if ( a.model == model )
        {
            anim = &a;
            break;
        }
    }
    if ( !anim )
        return;
    const float t = fmodf((float)(timeMs % 3600000) * 0.001f * anim->fps, (float)anim->frameCount);
    const int f0 = (int)t < anim->frameCount ? (int)t : anim->frameCount - 1;
    const int f1 = (f0 + 1) % anim->frameCount;
    const float frac = t - (float)f0;
    int numBones = anim->boneCount < model->numBones ? anim->boneCount : model->numBones;
    if ( numBones > obj->numBones )
        numBones = obj->numBones;
    for ( int b = 0; b < numBones; ++b )
    {
        const float *a = &anim->frames[7 * (f0 * anim->boneCount + b)];
        const float *c = &anim->frames[7 * (f1 * anim->boneCount + b)];
        const float sign = a[0] * c[0] + a[1] * c[1] + a[2] * c[2] + a[3] * c[3] < 0.0f ? -1.0f : 1.0f;
        DObjAnimMat *mat = &obj->skel.mat[b];
        float lenSq = 0.0f;
        for ( int k = 0; k < 4; ++k )
        {
            mat->quat[k] = a[k] + (sign * c[k] - a[k]) * frac;
            lenSq += mat->quat[k] * mat->quat[k];
        }
        if ( lenSq > 0.0f )
        {
            const float inv = 1.0f / sqrtf(lenSq);
            for ( int k = 0; k < 4; ++k )
                mat->quat[k] *= inv;
        }
        else
        {
            mat->quat[0] = mat->quat[1] = mat->quat[2] = 0.0f;
            mat->quat[3] = 1.0f;
        }
        mat->transWeight = 2.0f;
        for ( int k = 0; k < 3; ++k )
            mat->trans[k] = a[4 + k] + (c[4 + k] - a[4 + k]) * frac;
        obj->skel.partBits.anim[b >> 5] |= 0x80000000 >> (b & 31);
    }
}
