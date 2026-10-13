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
// specular black); a texture marked alpha masked clones an alpha-tested lit material instead.
//
// .rlm (little-endian; written by the converter): 'RLM1', u32 version 1, u32 texture count, u32 surface count;
// texture: char name[32], u16 width, u16 height, u8 mip count, u8 alpha masked, u16 0, every mip's BGRA8
// pixels, largest first; surface: u32 texture, u32 vertex count, u32 triangle count, vertex count x {f32 pos[3]
// (inches, Z up), f32 normal[3], f32 uv[2]}, triangle count x 3 u16 indices, zero pad to 4.
//
// Version 2 adds a skeleton and skinned vertices, and optionally, after the surfaces, baked animations: 'ANM2',
// u32 count, per animation char name[32], u32 frame count, f32 frames per second, u32 bone count (the model's),
// f32 move speed (inches per second at its own rate, 0: in place), frame count x bone count x {f32 local rotation
// quaternion[4], f32 translation offset from the rest pose[3]}. In place of an XAnim tree, every DObj of the
// model plays the locomotion cycle that fits how its entity moves (R_RuntimeXModel_PoseDObj): "idle", "walk",
// "run" or "back", at the rate matching its speed, crossfaded.

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
#include <universal/com_math.h>
#include <universal/com_math_anglevectors.h>
#include <cgame_mp/cg_pose_mp.h>
#include <win32/win_shared.h>
#include <malloc.h>
#include <string.h>
#include <math.h>
#include <intrin.h>
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
    char name[32];
    int frameCount;
    float fps;
    int boneCount;
    float moveSpeed;     // inches per second at the cycle's own rate
    const float *frames; // frameCount x boneCount x {quat[4], trans offset[3]}
    const unsigned char *upperBody; // per bone: 1 from j_spinelower up (the aim poses take these), 0 below
    int spineBone;       // j_spinelower (the aim correction turns it), -1: none
};
static std::vector<RtAnim> s_anims; // this level's
static const XModel *s_laserModels[2]; // this level's laser sight beam segment and dot (R_RuntimeXModel_Laser)

// Per entity locomotion state (by DObj entnum): the measured ground speed and the playing / fading-out cycles.
struct RtAnimState
{
    const DObj *obj; // the DObj the state belongs to (a new one restarts it)
    int lastTime;
    float lastOrigin[3];
    float vel[2];      // smoothed ground velocity, inches per second
    float speed;       // its length
    float moveRel;     // its direction relative to the facing, degrees (0: forward, +-180: backward)
    float bodyYaw;     // the body's turn toward the motion (a strafing player), degrees
    int cur, prev;     // s_anims indices (-1: none)
    float curPhase, prevPhase; // frames
    float prevRate;
    float blend;       // 0: prev only .. 1: cur only
    float slowTime;    // seconds the motion has been (nearly) still
};
static const int RT_MAX_ANIM_ENTS = 2048;
static RtAnimState s_animStates[RT_MAX_ANIM_ENTS];
static volatile long s_animStateLock; // the renderer's worker threads pose DObjs too

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
    memset(s_animStates, 0, sizeof(s_animStates));
    s_laserModels[0] = s_laserModels[1] = 0;
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

// masked: the texture's alpha is a cut-out mask (hair strands, lashes)
static GfxImage *RtCreateImage(RtReader &r, const char *modelName, int zoneIndex, bool gpu, bool *masked)
{
    char texName[33] = {};
    if ( const void *n = r.take(32) )
        memcpy(texName, n, 32);
    unsigned int width = r.u16(), height = r.u16(), mips = r.u8();
    *masked = r.u8() != 0;
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

// Template materials: a loaded lit model material of the plainest technique set available (colour, normal and
// specular maps; no weapon reflection or colour detail), preferring the earlier names. "r0" sets are opaque, "t0"
// ones alpha-test the colour map (the perk machines' mc_l_sm_t0c0n0s0 is in common_zombie, so on every level).
static const char *const s_opaqueTechsets[] = { "mc_l_sm_r0c0n0s0", "mc_l_sm_r0c0n0s0_hero", "mc_l_sm_r0c0n0s0x0",
                                                 "mc_l_sm_r0c0s0", "mc_l_sm_r0c0" };
static const char *const s_alphaTestTechsets[] = { "mc_l_sm_t0c0n0s0", "mc_l_sm_t0c0n0s0x0", "mc_l_sm_t0c0s0",
                                                    "mc_l_sm_t0c0" };

struct RtTemplateSearch
{
    const char *const *techsets;
    int count;
    int best; // index into techsets of the found one
    const Material *found;
};

static void RtFindTemplate(XAssetHeader header, void *data)
{
    RtTemplateSearch *search = (RtTemplateSearch *)data;
    const Material *m = header.material;
    if ( !m || !m->techniqueSet || !m->techniqueSet->name || !m->stateBitsTable )
        return;
    for ( int t = 0; t < search->count && t < search->best; ++t )
    {
        if ( strcmp(m->techniqueSet->name, search->techsets[t]) )
            continue;
        for ( int i = 0; i < m->textureCount; ++i )
        {
            if ( m->textureTable[i].nameHash == R_HashString("colorMap") )
            {
                search->best = t;
                search->found = m;
                return;
            }
        }
        return;
    }
}

static const Material *RtFindTemplateMaterial(const char *const *techsets, int count)
{
    RtTemplateSearch search = { techsets, count, count, 0 };
    DB_EnumXAssets_FastFile(ASSET_TYPE_MATERIAL, RtFindTemplate, &search, false);
    return search.found;
}

// Culling stays the template's: drawn two-sided, the back of a hair card is lit as if it faced the camera (the lit
// shaders do not flip the normal), which turns whole cards white under real lighting; RE4's hair shell is built to
// be drawn one-sided.
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
        else if ( table[i].semantic == 8 ) // specular: none (it also gates the probe's reflection)
            table[i].u.image = rgp.blackImage;
    }
    material.textureTable = table;
    // the template's environment reflection (envMapParms: the reflection probe's min / max by the view's angle, its
    // falloff and the sun's specular) is its own surface's (the zombies templates are polished steel and the like):
    // Leon's cloth, leather and skin take none, or they look wet
    if ( templ->constantCount && templ->constantTable )
    {
        MaterialConstantDef *constants = (MaterialConstantDef *)RtAlloc(sizeof(MaterialConstantDef) * templ->constantCount);
        for ( int i = 0; i < templ->constantCount; ++i )
        {
            constants[i] = templ->constantTable[i];
            if ( !strncmp(constants[i].name, "envMapParms", 11) )
                constants[i].literal[3] = 0.0f;
        }
        material.constantTable = constants;
    }
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

// builds a model from .rlm data (a file's, or one made in memory); returns the DB's copy
static const XModel *RtBuildModel(const char *name, const void *data, int size, int zoneIndex, const Material *templ,
                                  const Material *atestTempl, bool gpu)
{
    RtReader r{ (const unsigned __int8 *)data, (const unsigned __int8 *)data, (const unsigned __int8 *)data + size };
    const void *magic = r.take(4);
    unsigned int version = r.u32(), texCount = r.u32(), surfCount = r.u32();
    unsigned int boneCount = version >= 2 ? r.u32() : 1;
    if ( !r.ok || memcmp(magic, "RLM1", 4) || (version != 1 && version != 2) || !surfCount || surfCount > 255 || !boneCount
         || boneCount >= 160 )
    {
        Com_PrintWarning(0, "runtime model '%s': not a version 1 or 2 .rlm file\n", name);
        return 0;
    }

    std::vector<Material *> materials;
    for ( unsigned int t = 0; t < texCount && r.ok; ++t )
    {
        bool masked = false;
        GfxImage *image = RtCreateImage(r, name, zoneIndex, gpu, &masked);
        const Material *from = masked && atestTempl ? atestTempl : templ;
        materials.push_back(image ? RtCreateMaterial(from, image, va("%s_mtl%u", name, t), zoneIndex) : 0);
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
        return 0;
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
    // the hit locations (hitLocation_t) of the bones named as BO1's, as a BO1 character's: a server trace (an AI's
    // melee, a bullet: DObjTraceline) only tests bones whose location ranks, so an unclassified body is never hit; the
    // other bones (fingers, hair, RE4's extra ones) take their parent's
    static const struct { const char *bone; unsigned char loc; } s_hitLocations[] = {
        { "j_mainroot", HITLOC_TORSO_LWR }, { "j_spinelower", HITLOC_TORSO_LWR }, { "j_spine4", HITLOC_TORSO_UPR },
        { "j_neck", HITLOC_NECK }, { "j_head", HITLOC_HEAD },
        { "j_clavicle_ri", HITLOC_TORSO_UPR }, { "j_shoulder_ri", HITLOC_R_ARM_UPR }, { "j_elbow_ri", HITLOC_R_ARM_LWR },
        { "j_wristtwist_ri", HITLOC_R_ARM_LWR }, { "j_wrist_ri", HITLOC_R_HAND },
        { "j_clavicle_le", HITLOC_TORSO_UPR }, { "j_shoulder_le", HITLOC_L_ARM_UPR }, { "j_elbow_le", HITLOC_L_ARM_LWR },
        { "j_wristtwist_le", HITLOC_L_ARM_LWR }, { "j_wrist_le", HITLOC_L_HAND },
        { "j_hip_ri", HITLOC_R_LEG_UPR }, { "j_knee_ri", HITLOC_R_LEG_LWR }, { "j_ankle_ri", HITLOC_R_FOOT },
        { "j_ball_ri", HITLOC_R_FOOT },
        { "j_hip_le", HITLOC_L_LEG_UPR }, { "j_knee_le", HITLOC_L_LEG_LWR }, { "j_ankle_le", HITLOC_L_FOOT },
        { "j_ball_le", HITLOC_L_FOOT },
    };
    model.partClassification = (unsigned __int8 *)RtAlloc(boneCount);
    for ( unsigned int b = 0; b < boneCount; ++b )
    {
        for ( const auto &hit : s_hitLocations )
        {
            if ( boneName[b] == hit.bone )
                model.partClassification[b] = hit.loc;
        }
    }
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
    if ( r.ok && version >= 2 && r.end - r.p >= 8 && !memcmp(r.p, "ANM2", 4) )
    {
        r.take(4);
        const unsigned int animCount = r.u32();
        for ( unsigned int a = 0; a < animCount && r.ok; ++a )
        {
            RtAnim anim = {};
            if ( const void *n = r.take(32) )
                memcpy(anim.name, n, sizeof(anim.name) - 1);
            anim.frameCount = (int)r.u32();
            const void *fps = r.take(4);
            anim.fps = fps ? *(const float *)fps : 0.0f;
            anim.boneCount = (int)r.u32();
            const void *speed = r.take(4);
            anim.moveSpeed = speed ? *(const float *)speed : 0.0f;
            if ( anim.frameCount <= 0 || anim.frameCount > 100000 || anim.boneCount != (int)boneCount || !(anim.fps > 0.0f)
                 || !(anim.moveSpeed >= 0.0f) )
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
    if ( !r.ok )
    {
        Com_PrintWarning(0, "runtime model '%s': truncated or invalid .rlm file\n", name);
        return 0;
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
    // the upper body: j_spinelower and everything under it (the aim poses' bones; the legs keep the cycle)
    unsigned char *upperBody = (unsigned char *)RtAlloc((int)boneCount);
    for ( unsigned int b = 0; b < boneCount; ++b )
    {
        for ( int a = (int)b; a >= 0 && !upperBody[b]; a = parent[a] )
            upperBody[b] = boneName[a] == "j_spinelower";
    }
    int spineBone = -1;
    for ( unsigned int b = 0; b < boneCount; ++b )
    {
        if ( boneName[b] == "j_spinelower" )
            spineBone = (int)b;
    }
    for ( RtAnim &anim : anims )
    {
        anim.model = linked;
        anim.upperBody = upperBody;
        anim.spineBone = spineBone;
        s_anims.push_back(anim);
    }
    Com_Printf(0, "runtime model '%s': %u surfaces, %d vertices, %d triangles, %u textures, %u bones, %u animations"
        " (level zone %d)\n", name, surfCount, baseVert, baseTri, texCount, boneCount, (unsigned int)anims.size(), zoneIndex);
    return linked;
}

static void RtLoadModel(const char *name, int zoneIndex, const Material *templ, const Material *atestTempl, bool gpu)
{
    void *file = 0;
    int size = FS_ReadFile(va("rtmodels/%s.rlm", name), &file);
    if ( size <= 0 || !file )
    {
        Com_PrintWarning(0, "runtime model '%s': rtmodels/%s.rlm not found\n", name, name);
        return;
    }
    RtBuildModel(name, file, size, zoneIndex, templ, atestTempl, gpu);
    FS_FreeFile(file);
}

// ---- the laser sight's models (the over-the-shoulder aim, cg_view_mp.cpp) ---------------------------------------
// Made here, not converted: a red beam segment (16 units along +X, two crossed thin quads, both faces) and a dot
// (three crossed quads), on an unlit effect material, so the laser glows in the dark as RE4's does.
static const char *const s_effectTechsets[] = { "mc_effect", "mc_effect_zfeather", "mc_unlit_alphatest" };

static void RtPut(std::vector<unsigned char> &out, const void *p, size_t n)
{
    out.insert(out.end(), (const unsigned char *)p, (const unsigned char *)p + n);
}

static void RtPutU32(std::vector<unsigned char> &out, unsigned int v)
{
    RtPut(out, &v, 4);
}

// the laser's glow at texture coordinates (u, v) in 0..1: radial (the dot: a white-hot core, a red bloom fading
// out) or across v (the beam: bright along its middle, soft at its edges); BGRA
static void RtLaserTexel(bool radial, float u, float v, unsigned char *bgra)
{
    float d = radial ? sqrtf((u - 0.5f) * (u - 0.5f) + (v - 0.5f) * (v - 0.5f)) * 2.0f : fabsf(v - 0.5f) * 2.0f;
    d = d > 1.0f ? 1.0f : d;
    const float core = radial ? (d < 0.18f ? 1.0f - d / 0.18f : 0.0f) : (d < 0.35f ? 1.0f - d / 0.35f : 0.0f);
    float glow = 1.0f - d;
    glow = radial ? glow * glow * glow : glow * glow; // the bloom falls off fast
    const float a = core > glow ? core : glow;
    const float hot = radial ? 200.0f : 20.0f; // the dot's core white-hot; the beam's stays red (thinned far out)
    bgra[0] = (unsigned char)(40.0f + hot * core); // B
    bgra[1] = (unsigned char)(40.0f + hot * core); // G
    bgra[2] = 255;                                    // R
    bgra[3] = (unsigned char)(255.0f * a * (radial ? 1.0f : 0.25f)); // the beam dull, the dot bright
}

// a version 1 .rlm with one glow texture (radial: the dot's, else the beam's) and one surface of quads (4 corners
// each, drawn both ways)
static std::vector<unsigned char> RtMakeQuadModel(const float (*corners)[3], int quadCount, bool radial)
{
    std::vector<unsigned char> out;
    RtPut(out, "RLM1", 4);
    RtPutU32(out, 1);
    RtPutU32(out, 1);
    RtPutU32(out, 1);
    char texName[32] = {};
    strcpy(texName, radial ? "laser_dot" : "laser_beam");
    RtPut(out, texName, 32);
    const unsigned short size = 32;
    RtPut(out, &size, 2);
    RtPut(out, &size, 2);
    const unsigned char mips = 6, masked = 0;
    RtPut(out, &mips, 1);
    RtPut(out, &masked, 1);
    const unsigned short pad = 0;
    RtPut(out, &pad, 2);
    for ( int m = 0, w = 32; m < 6; ++m, w /= 2 )
    {
        for ( int y = 0; y < w; ++y )
        {
            for ( int x = 0; x < w; ++x )
            {
                unsigned char bgra[4];
                RtLaserTexel(radial, ((float)x + 0.5f) / (float)w, ((float)y + 0.5f) / (float)w, bgra);
                RtPut(out, bgra, 4);
            }
        }
    }
    RtPutU32(out, 0);
    RtPutU32(out, (unsigned int)quadCount * 4);
    RtPutU32(out, (unsigned int)quadCount * 4);
    for ( int q = 0; q < quadCount; ++q )
    {
        const float *c = corners[4 * q];
        float e1[3], e2[3], n[3];
        for ( int k = 0; k < 3; ++k )
        {
            e1[k] = corners[4 * q + 1][k] - c[k];
            e2[k] = corners[4 * q + 3][k] - c[k];
        }
        n[0] = e1[1] * e2[2] - e1[2] * e2[1];
        n[1] = e1[2] * e2[0] - e1[0] * e2[2];
        n[2] = e1[0] * e2[1] - e1[1] * e2[0];
        const float len = sqrtf(n[0] * n[0] + n[1] * n[1] + n[2] * n[2]);
        for ( int k = 0; k < 3; ++k )
            n[k] = len > 0.0f ? n[k] / len : 0.0f;
        const float uv[4][2] = { { 0, 0 }, { 1, 0 }, { 1, 1 }, { 0, 1 } };
        for ( int v = 0; v < 4; ++v )
        {
            RtPut(out, corners[4 * q + v], 12);
            RtPut(out, n, 12);
            RtPut(out, uv[v], 8);
        }
    }
    for ( int q = 0; q < quadCount; ++q )
    {
        const unsigned short b = (unsigned short)(4 * q);
        const unsigned short tris[12] = { b, (unsigned short)(b + 1), (unsigned short)(b + 2),
                                          b, (unsigned short)(b + 2), (unsigned short)(b + 3),
                                          b, (unsigned short)(b + 2), (unsigned short)(b + 1),
                                          b, (unsigned short)(b + 3), (unsigned short)(b + 2) };
        RtPut(out, tris, sizeof(tris));
    }
    while ( out.size() % 4 )
        out.push_back(0);
    return out;
}

static void RtBuildLaserModels(int zoneIndex, bool gpu)
{
    s_laserModels[0] = s_laserModels[1] = 0;
    const Material *effect = RtFindTemplateMaterial(s_effectTechsets, ARRAYSIZE(s_effectTechsets));
    if ( !effect )
    {
        Com_PrintWarning(0, "runtime models: no unlit effect material; no laser sight\n");
        return;
    }
    // the beam's glow is soft at its edges; the dot is one quad facing +X (CG_ShoulderLaser turns it to the camera
    // and scales it, as RE4's drawPoint), its radial glow round with a bloom
    const float w = 0.3f, L = 16.0f, d = 2.0f;
    const float beam[4][4][3] = {
        { { 0, -w, 0 }, { L, -w, 0 }, { L, w, 0 }, { 0, w, 0 } },
        { { 0, 0, -w }, { L, 0, -w }, { L, 0, w }, { 0, 0, w } },
        { { 0, -w * 0.5f, -w * 0.5f }, { L, -w * 0.5f, -w * 0.5f }, { L, w * 0.5f, w * 0.5f }, { 0, w * 0.5f, w * 0.5f } },
        { { 0, -w * 0.5f, w * 0.5f }, { L, -w * 0.5f, w * 0.5f }, { L, w * 0.5f, -w * 0.5f }, { 0, w * 0.5f, -w * 0.5f } } };
    const float dot[1][4][3] = { { { 0, -d, -d }, { 0, d, -d }, { 0, d, d }, { 0, -d, d } } };
    std::vector<unsigned char> data = RtMakeQuadModel(&beam[0][0], 4, false);
    s_laserModels[0] = RtBuildModel("leon_laser", data.data(), (int)data.size(), zoneIndex, effect, 0, gpu);
    data = RtMakeQuadModel(&dot[0][0], 1, true);
    s_laserModels[1] = RtBuildModel("leon_laser_dot", data.data(), (int)data.size(), zoneIndex, effect, 0, gpu);
    Com_Printf(0, "runtime models: laser sight on '%s' (%s)\n", effect->info.name, effect->techniqueSet->name);
}

// the laser sight's beam segment (16 units along +X) or dot, or 0 (no runtime models, or no effect material)
const XModel *__cdecl R_RuntimeXModel_Laser(bool dot)
{
    return s_laserModels[dot ? 1 : 0];
}

bool __cdecl R_RuntimeXModel_DvarOn(const char *name)
{
    const dvar_s *dvar = Dvar_FindVar(name);
    if ( !dvar )
        return false;
    if ( dvar->type == DVAR_TYPE_BOOL )
        return dvar->current.enabled;
    if ( dvar->type == DVAR_TYPE_STRING )
        return dvar->current.string && atoi(dvar->current.string) != 0;
    return dvar->current.integer != 0;
}

bool __cdecl R_RuntimeXModel_RootYaw(const char *name, float time, float *yaw, float *duration)
{
    for ( const RtAnim &anim : s_anims )
    {
        if ( strcmp(anim.name, name) || anim.frameCount < 2 || anim.fps <= 0.0f )
            continue;
        *duration = (float)(anim.frameCount - 1) / anim.fps;
        float phase = time * anim.fps;
        phase = phase < 0.0f ? 0.0f : (phase > (float)(anim.frameCount - 1) ? (float)(anim.frameCount - 1) : phase);
        const int f0 = (int)phase, f1 = f0 + 1 < anim.frameCount ? f0 + 1 : f0;
        // bone 0's rotation (about up): the frame's yaw, unwrapped toward the previous one across +-180
        const float *q0 = anim.frames + (size_t)f0 * anim.boneCount * 7, *q1 = anim.frames + (size_t)f1 * anim.boneCount * 7;
        const float y0 = 2.0f * atan2f(q0[2], q0[3]) * 57.29578f;
        float y1 = 2.0f * atan2f(q1[2], q1[3]) * 57.29578f;
        y1 = y0 + AngleNormalize180(y1 - y0);
        *yaw = y0 + (y1 - y0) * (phase - (float)f0);
        return true;
    }
    return false;
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

    // a character's cloth material first (the zombies' gear: the characters' own lit shader, matte where a prop's is
    // shiny), else a plain lit prop's
    static const char *const s_clothTechsets[] = { "mc_l_sm_r0c0n0s0_clothcharr_hero", "mc_l_sm_r0c0n0s0_clothcharr" };
    const Material *templ = RtFindTemplateMaterial(s_clothTechsets, ARRAYSIZE(s_clothTechsets));
    if ( !templ )
        templ = RtFindTemplateMaterial(s_opaqueTechsets, ARRAYSIZE(s_opaqueTechsets));
    if ( !templ )
        templ = RtTemplateMaterial(); // a weapon's: lit, but with its reflection and colour detail
    if ( !templ )
    {
        Com_PrintWarning(0, "runtime models: no template material on this level\n");
        return;
    }
    const Material *atestTempl = RtFindTemplateMaterial(s_alphaTestTechsets, ARRAYSIZE(s_alphaTestTechsets));
    Com_Printf(0, "runtime models: templates '%s' (%s), alpha-tested '%s' (%s)\n", templ->info.name,
        templ->techniqueSet->name, atestTempl ? atestTempl->info.name : "none: masked textures draw opaque",
        atestTempl ? atestTempl->techniqueSet->name : "-");
    RtFreeAll(); // the previous level's arrays; its zone (and the GPU side) is gone
    const bool gpu = !r_loadForRenderer || r_loadForRenderer->current.enabled;
    I_strncpyz(names, bo1_rtmodels->current.string, sizeof(names));
    for ( char *tok = strtok(names, " ,"); tok; tok = strtok(0, " ,") )
        RtLoadModel(tok, zoneIndex, templ, atestTempl, gpu);
    RtBuildLaserModels(zoneIndex, gpu);
    if ( gpu )
        RB_Resource_Flush();
}

// the model's animation `name` (0: its first), an s_anims index or -1
static int RtFindAnim(const XModel *model, const char *name)
{
    for ( int i = 0; i < (int)s_anims.size(); ++i )
    {
        if ( s_anims[i].model == model && (!name || !strcmp(s_anims[i].name, name)) )
            return i;
    }
    return -1;
}

// A motion of a set (the converter's weapon sets: "<set>:<name>", e.g. "shotgun:aim"), or the default set's
// (unprefixed: the handgun's) when the set has none or no set is given.
static int RtFindSetAnim(const XModel *model, const char *set, const char *name)
{
    if ( set && *set )
    {
        char full[64];
        Com_sprintf(full, sizeof(full), "%s:%s", set, name);
        const int index = RtFindAnim(model, full);
        if ( index >= 0 )
            return index;
    }
    return RtFindAnim(model, name);
}

// a motion's name without its set
static const char *RtAnimBaseName(const RtAnim &anim)
{
    const char *colon = strchr(anim.name, ':');
    return colon ? colon + 1 : anim.name;
}

// R_AddDObjToScene: a one-model DObj without an XAnim tree normally draws as a scene model in the model's base
// pose; one with a baked animation takes the posed (scene DObj) path, which calls R_RuntimeXModel_PoseDObj.
bool __cdecl R_RuntimeXModel_IsAnimated(const XModel *model)
{
    return !s_anims.empty() && RtFindAnim(model, 0) >= 0;
}

static float RtWrapPhase(float phase, int frameCount)
{
    phase = fmodf(phase, (float)frameCount);
    return phase < 0.0f ? phase + (float)frameCount : phase;
}

// bone b of a cycle at `phase` (frames), the neighbouring frames blended
static void RtSampleBone(const RtAnim &anim, float phase, int b, float *quat, float *trans)
{
    int f0 = (int)phase;
    if ( f0 >= anim.frameCount )
        f0 = anim.frameCount - 1;
    const int f1 = (f0 + 1) % anim.frameCount;
    const float frac = phase - (float)f0;
    const float *a = &anim.frames[7 * (f0 * anim.boneCount + b)];
    const float *c = &anim.frames[7 * (f1 * anim.boneCount + b)];
    const float sign = a[0] * c[0] + a[1] * c[1] + a[2] * c[2] + a[3] * c[3] < 0.0f ? -1.0f : 1.0f;
    for ( int k = 0; k < 4; ++k )
        quat[k] = a[k] + (sign * c[k] - a[k]) * frac;
    for ( int k = 0; k < 3; ++k )
        trans[k] = a[4 + k] + (c[4 + k] - a[4 + k]) * frac;
}

// out = a * b (Hamilton; x, y, z, w): b turned by a
static void RtQuatMul(const float *a, const float *b, float *out)
{
    out[0] = a[3] * b[0] + a[0] * b[3] + a[1] * b[2] - a[2] * b[1];
    out[1] = a[3] * b[1] - a[0] * b[2] + a[1] * b[3] + a[2] * b[0];
    out[2] = a[3] * b[2] + a[0] * b[1] - a[1] * b[0] + a[2] * b[3];
    out[3] = a[3] * b[3] - a[0] * b[0] - a[1] * b[1] - a[2] * b[2];
}

// a = a + (b - a) * t for a bone pose: the rotation by the shorter way (normalised later), the translation linearly
static void RtBlendPose(float *quat, float *trans, const float *bq, const float *bt, float t)
{
    const float sign = quat[0] * bq[0] + quat[1] * bq[1] + quat[2] * bq[2] + quat[3] * bq[3] < 0.0f ? -1.0f : 1.0f;
    float lenSq = 0.0f;
    for ( int k = 0; k < 4; ++k )
    {
        quat[k] += (sign * bq[k] - quat[k]) * t;
        lenSq += quat[k] * quat[k];
    }
    const float inv = lenSq > 0.0f ? 1.0f / sqrtf(lenSq) : 0.0f;
    for ( int k = 0; k < 4; ++k )
        quat[k] *= inv;
    for ( int k = 0; k < 3; ++k )
        trans[k] += (bt[k] - trans[k]) * t;
}

// playback rate of a cycle at a ground speed: a moving cycle's feet keep pace with the entity
static float RtCycleRate(const RtAnim &anim, float speed)
{
    if ( anim.moveSpeed <= 0.0f )
        return 1.0f;
    const float rate = speed / anim.moveSpeed;
    return rate < 0.5f ? 0.5f : (rate > 2.0f ? 2.0f : rate);
}

// The cycle for the entity's motion: idle when (nearly) still, back when moving against its facing, run past the
// geometric mean of the walk and run speeds, else walk; hysteresis keeps it from flickering at the thresholds.
static int RtChooseCycle(const XModel *model, const RtAnimState &st, bool turnToMotion, const char *set)
{
    const int idle = RtFindSetAnim(model, set, "idle"), walk = RtFindSetAnim(model, set, "walk");
    const int run = RtFindSetAnim(model, set, "run"), back = RtFindSetAnim(model, set, "back");
    const int still = idle >= 0 ? idle : RtFindAnim(model, 0);
    const bool moving = st.cur >= 0 && strcmp(RtAnimBaseName(s_anims[st.cur]), "idle");
    if ( st.speed < (moving ? 6.0f : 12.0f) || (walk < 0 && run < 0) )
        return still;
    // under tank controls (no sideways motion) anything against the facing is the back step, with a wide margin: a
    // turn while stepping back leaves the measured motion trailing the facing, and must not flip it to a walk
    const float backIn = turnToMotion ? 115.0f : 90.0f, backOut = turnToMotion ? 100.0f : 45.0f;
    const bool backNow = st.cur >= 0 && !strcmp(RtAnimBaseName(s_anims[st.cur]), "back");
    if ( back >= 0 && fabsf(st.moveRel) > (backNow ? backOut : backIn) )
        return back;
    if ( walk < 0 )
        return run;
    if ( run < 0 )
        return walk;
    const bool runNow = st.cur >= 0 && !strcmp(RtAnimBaseName(s_anims[st.cur]), "run");
    const float runCut = sqrtf(s_anims[walk].moveSpeed * s_anims[run].moveSpeed) * (runNow ? 0.9f : 1.1f);
    return st.speed > runCut ? run : walk;
}

// Advances the entity's locomotion state to timeMs (once per game time; the pose functions run several times a
// frame) and returns a copy.
static RtAnimState RtUpdateAnimState(const DObj *obj, const XModel *model, const float *origin, const float *angles,
                                     int timeMs, bool turnToMotion, const char *set)
{
    while ( _InterlockedCompareExchange(&s_animStateLock, 1, 0) )
        _mm_pause();
    RtAnimState &st = s_animStates[obj->entnum % RT_MAX_ANIM_ENTS];
    // a new entity (or a stale state) restarts; the same one's DObj rebuilt (a grenade attached: the player's dobjDirty)
    // keeps its motion going
    if ( st.cur < 0 || st.cur >= (int)s_anims.size() || s_anims[st.cur].model != model || timeMs < st.lastTime
         || (st.obj != obj && timeMs - st.lastTime > 500) )
    {
        memset(&st, 0, sizeof(st));
        st.obj = obj;
        st.lastTime = timeMs;
        for ( int k = 0; k < 3; ++k )
            st.lastOrigin[k] = origin[k];
        const int idle = RtFindSetAnim(model, set, "idle");
        st.cur = idle >= 0 ? idle : RtFindAnim(model, 0);
        st.prev = -1;
        st.blend = 1.0f;
    }
    else if ( timeMs != st.lastTime )
    {
        const float dt = (float)(timeMs - st.lastTime) * 0.001f;
        if ( dt > 0.0f && dt <= 0.5f )
        {
            float vx = (origin[0] - st.lastOrigin[0]) / dt, vy = (origin[1] - st.lastOrigin[1]) / dt;
            if ( vx * vx + vy * vy > 2000.0f * 2000.0f ) // a teleport
                vx = vy = 0.0f;
            const float k = dt * 10.0f < 1.0f ? dt * 10.0f : 1.0f;
            st.vel[0] += (vx - st.vel[0]) * k;
            st.vel[1] += (vy - st.vel[1]) * k;
            st.speed = sqrtf(st.vel[0] * st.vel[0] + st.vel[1] * st.vel[1]);
            if ( st.speed > 1.0f )
                st.moveRel = AngleNormalize180(atan2f(st.vel[1], st.vel[0]) * 57.29578f - angles[1]);

            int target = RtChooseCycle(model, st, turnToMotion, set);
            // a reversal (back step <-> walk) passes through standing still for a moment: the cycle goes to the
            // idle only once he has stood for 0.15 s, so it goes straight from one step to the other, in stride
            st.slowTime = st.speed < 6.0f ? st.slowTime + dt : 0.0f;
            if ( target >= 0 && st.cur >= 0 && target != st.cur && !strcmp(RtAnimBaseName(s_anims[target]), "idle")
                 && strcmp(RtAnimBaseName(s_anims[st.cur]), "idle") && st.slowTime < 0.15f )
                target = st.cur;
            if ( target != st.cur && target >= 0 )
            {
                const RtAnim &from = s_anims[st.cur], &to = s_anims[target];
                st.prev = st.cur;
                st.prevPhase = st.curPhase;
                st.prevRate = RtCycleRate(from, st.speed);
                // walk <-> run keep the stride's phase (the feet stay in step); others start at the top
                const bool stride = from.moveSpeed > 0.0f && to.moveSpeed > 0.0f;
                st.curPhase = stride ? st.curPhase / (float)from.frameCount * (float)to.frameCount : 0.0f;
                st.cur = target;
                st.blend = 0.0f;
            }
            // the body turns toward a sideways motion (backward: the back step, turned the other way); not for a
            // player under RE4's tank controls, who never moves sideways (the noise in his measured motion twisted
            // him on the back step)
            float bodyTarget = 0.0f;
            if ( turnToMotion && s_anims[st.cur].moveSpeed > 0.0f )
            {
                const bool backStep = !strcmp(RtAnimBaseName(s_anims[st.cur]), "back");
                bodyTarget = backStep ? AngleNormalize180(st.moveRel - 180.0f) : st.moveRel;
                const float limit = backStep ? 70.0f : 100.0f;
                bodyTarget = bodyTarget > limit ? limit : (bodyTarget < -limit ? -limit : bodyTarget);
            }
            st.bodyYaw += (bodyTarget - st.bodyYaw) * (dt * 8.0f < 1.0f ? dt * 8.0f : 1.0f);
            const RtAnim &cur = s_anims[st.cur];
            st.curPhase = RtWrapPhase(st.curPhase + dt * cur.fps * RtCycleRate(cur, st.speed), cur.frameCount);
            if ( st.prev >= 0 )
            {
                const RtAnim &prev = s_anims[st.prev];
                st.prevPhase = RtWrapPhase(st.prevPhase + dt * prev.fps * st.prevRate, prev.frameCount);
            }
            st.blend += dt / 0.2f; // the crossfade time
            if ( st.blend >= 1.0f )
            {
                st.blend = 1.0f;
                st.prev = -1;
            }
        }
        st.lastTime = timeMs;
        for ( int k = 0; k < 3; ++k )
            st.lastOrigin[k] = origin[k];
    }
    st.obj = obj;
    const RtAnimState copy = st;
    _InterlockedExchange(&s_animStateLock, 0);
    return copy;
}

// Called by CG_DObjCalcPose / CG_DObjCalcBone before DObjCalcSkel, with the entity's pose origin and angles. A DObj
// whose model has baked animations gets every bone's local pose from its locomotion cycle (crossfaded from the
// previous one), marked as animated so DObjCalcAnim leaves the bones alone; DObjCalcSkel then builds the skeleton as
// for an XAnim pose (the rotation absolute, the translation added to the model's rest offset).
float __cdecl R_RuntimeXModel_PoseDObj(DObj *obj, const float *origin, const float *angles, int timeMs,
                                      bool isPlayer, float aimFrac, float aimPitch, RtAction action, float actionTime,
                                      float actionDuration, const float *aimCorrection, bool turnToMotion,
                                      const char *animSet)
{
    if ( s_anims.empty() || !obj->numModels || !obj->skel.mat )
        return 0.0f;
    const XModel *model = obj->localModels[0];
    if ( RtFindAnim(model, 0) < 0 )
        return 0.0f;
    const RtAnimState st = RtUpdateAnimState(obj, model, origin, angles, timeMs, turnToMotion, animSet);
    if ( st.cur < 0 )
        return 0.0f;
    const RtAnim &cur = s_anims[st.cur];
    const RtAnim *prev = st.prev >= 0 && st.blend < 1.0f ? &s_anims[st.prev] : 0;
    // aiming (a player's ADS): the upper body takes the aim pose, level blended toward up or down by the view pitch
    // (RE4 blends its three by the aim pitch, cMot3), in by the ADS fraction
    const int aimLevel = aimFrac > 0.0f ? RtFindSetAnim(model, animSet, "aim") : -1;
    int aimPitched = -1;
    float aimPitchBlend = 0.0f, aimPhase = 0.0f;
    if ( aimLevel >= 0 )
    {
        aimPitchBlend = -aimPitch / 60.0f; // view pitch is positive looking down; the up / down poses are ~60 degrees
        if ( aimPitchBlend > 1.0f )
            aimPitchBlend = 1.0f;
        if ( aimPitchBlend < -1.0f )
            aimPitchBlend = -1.0f;
        aimPitched = RtFindSetAnim(model, animSet, aimPitchBlend >= 0.0f ? "aim_up" : "aim_down");
        aimPitchBlend = fabsf(aimPitchBlend);
        const RtAnim &level = s_anims[aimLevel];
        aimPhase = RtWrapPhase((float)(timeMs % 3600000) * 0.001f * level.fps, level.frameCount);
        if ( aimFrac > 1.0f )
            aimFrac = 1.0f;
    }
    // the action: a shot plays RE4's fire motion (level blended toward up / down by the pitch, as the aim) from the
    // shot at its own rate; a reload RE4's reload stretched over BO1's; both fade in and out on the upper body
    int actionAnim = -1, actionPitched = -1;
    float actionPhase = 0.0f, actionWeight = 0.0f, actionPitchBlend = 0.0f;
    if ( action == RT_ACTION_FIRE || action == RT_ACTION_KNIFE || action == RT_ACTION_THROW || action == RT_ACTION_KICK )
    {
        // a shot, a knife slash or a grenade throw, from its start at the motion's own rate; the shot and the slash
        // blend toward their up / down versions by the pitch
        const char *level = action == RT_ACTION_FIRE ? "fire"
                            : (action == RT_ACTION_KNIFE ? "knife" : (action == RT_ACTION_KICK ? "kick" : "throw"));
        actionAnim = RtFindSetAnim(model, animSet, level);
        if ( actionAnim >= 0 )
        {
            const RtAnim &motion = s_anims[actionAnim];
            actionPhase = actionTime * motion.fps;
            if ( actionPhase >= (float)(motion.frameCount - 1) )
                actionAnim = -1;
            else
            {
                const float left = (float)(motion.frameCount - 1) - actionPhase;
                actionWeight = left < 4.0f ? left / 4.0f : 1.0f; // the last frames blend back into the aim
                if ( action != RT_ACTION_FIRE && actionPhase < 2.0f )
                    actionWeight *= actionPhase / 2.0f; // the slash and the throw come in over 2 frames
                if ( action != RT_ACTION_THROW && action != RT_ACTION_KICK )
                {
                    float r = -aimPitch / 60.0f;
                    r = r > 1.0f ? 1.0f : (r < -1.0f ? -1.0f : r);
                    const bool up = r >= 0.0f;
                    actionPitched = RtFindSetAnim(model, animSet, action == RT_ACTION_FIRE ? (up ? "fire_up" : "fire_down")
                                                                                           : (up ? "knife_up" : "knife_down"));
                    actionPitchBlend = fabsf(r);
                }
            }
        }
    }
    else if ( action == RT_ACTION_HIT_FRONT || action == RT_ACTION_HIT_BACK )
    {
        // a hit: from it at the motion's own rate, in over 2 frames, back into the locomotion over the last 6
        actionAnim = RtFindSetAnim(model, animSet, action == RT_ACTION_HIT_FRONT ? "hit_front" : "hit_back");
        if ( actionAnim >= 0 )
        {
            const RtAnim &hit = s_anims[actionAnim];
            actionPhase = actionTime * hit.fps;
            const float left = (float)(hit.frameCount - 1) - actionPhase;
            if ( left <= 0.0f )
                actionAnim = -1;
            else
                actionWeight = (left < 6.0f ? left / 6.0f : 1.0f) * (actionPhase < 2.0f ? actionPhase / 2.0f : 1.0f);
        }
    }
    else if ( action == RT_ACTION_DEATH )
    {
        // down: the fall, then its last frame for as long as he stays down; in over 0.2 s
        actionAnim = RtFindSetAnim(model, animSet, "death");
        if ( actionAnim >= 0 )
        {
            const RtAnim &death = s_anims[actionAnim];
            actionPhase = actionTime * death.fps;
            if ( actionPhase > (float)(death.frameCount - 1) )
                actionPhase = (float)(death.frameCount - 1);
            actionWeight = actionTime < 0.2f ? actionTime / 0.2f : 1.0f;
        }
    }
    else if ( action == RT_ACTION_TURN )
    {
        // the quick turn: the whole body, at its own rate (the view turns along its root yaw: CL_LeonQuickTurn); the
        // last 0.15 s blend back into the locomotion
        actionAnim = RtFindSetAnim(model, animSet, "turn180");
        if ( actionAnim >= 0 )
        {
            const RtAnim &turn = s_anims[actionAnim];
            const float length = (float)(turn.frameCount - 1) / turn.fps;
            actionPhase = actionTime * turn.fps;
            if ( actionTime >= length )
                actionAnim = -1;
            else
                actionWeight = length - actionTime < 0.15f ? (length - actionTime) / 0.15f : 1.0f;
        }
    }
    else if ( action == RT_ACTION_RELOAD && actionDuration > 0.0f )
    {
        actionAnim = RtFindSetAnim(model, animSet, "reload");
        if ( actionAnim >= 0 )
        {
            const RtAnim &reload = s_anims[actionAnim];
            float t = actionTime / actionDuration;
            t = t < 0.0f ? 0.0f : (t > 1.0f ? 1.0f : t);
            actionPhase = t * (float)(reload.frameCount - 1);
            const float fade = 0.15f / actionDuration; // 0.15 s in and out
            actionWeight = t < fade ? t / fade : (1.0f - t < fade ? (1.0f - t) / fade : 1.0f);
        }
    }
    int numBones = cur.boneCount < model->numBones ? cur.boneCount : model->numBones;
    if ( numBones > obj->numBones )
        numBones = obj->numBones;
    for ( int b = 0; b < numBones; ++b )
    {
        DObjAnimMat *mat = &obj->skel.mat[b];
        // The root: the skeleton is in world space, CG_DoBaseOriginController (in CG_DoControllers, before this) puts
        // the entity's origin and angles on it, composed with a controller's root. A player's is rebuilt the same way
        // with the body's turn toward the motion in place of CoD's leg yaw (BG_Player_DoControllers' tag_origin, for
        // CoD's leg animations); another entity's stays as set. Not once the skeleton is built from it.
        if ( b == 0 && !(obj->skel.partBits.skel[0] & 0x80000000)
             && (isPlayer || !(obj->skel.partBits.anim[0] & 0x80000000)) )
        {
            const float half = st.bodyYaw * (aimLevel >= 0 ? 1.0f - aimFrac : 1.0f) * 0.5f * 0.017453292f; // aiming: facing the aim
            float quat[4] = { 0.0f, 0.0f, sinf(half), cosf(half) };
            if ( isPlayer )
            {
                float baseQuat[4];
                AnglesToQuat(angles, baseQuat);
                QuatMultiplyEquals(baseQuat, quat); // as CG_DoBaseOriginController composes a controller's root
            }
            for ( int k = 0; k < 4; ++k )
                mat->quat[k] = quat[k];
            for ( int k = 0; k < 3; ++k )
                mat->trans[k] = isPlayer ? origin[k] : 0.0f;
            mat->transWeight = 2.0f;
            obj->skel.partBits.anim[0] |= 0x80000000;
            continue;
        }
        if ( obj->skel.partBits.anim[b >> 5] & (0x80000000 >> (b & 31)) )
            continue; // posed already this frame (an earlier CG_DObjCalcBone / CG_DObjCalcPose)
        RtSampleBone(cur, st.curPhase, b, mat->quat, mat->trans);
        if ( prev )
        {
            float quat[4], trans[3];
            RtSampleBone(*prev, st.prevPhase, b, quat, trans);
            const float sign = quat[0] * mat->quat[0] + quat[1] * mat->quat[1] + quat[2] * mat->quat[2]
                + quat[3] * mat->quat[3] < 0.0f ? -1.0f : 1.0f;
            for ( int k = 0; k < 4; ++k )
                mat->quat[k] = sign * quat[k] + (mat->quat[k] - sign * quat[k]) * st.blend;
            for ( int k = 0; k < 3; ++k )
                mat->trans[k] = trans[k] + (mat->trans[k] - trans[k]) * st.blend;
        }
        if ( aimLevel >= 0 && cur.upperBody && cur.upperBody[b] )
        {
            float quat[4], trans[3];
            RtSampleBone(s_anims[aimLevel], aimPhase, b, quat, trans);
            if ( aimPitched >= 0 && aimPitchBlend > 0.0f )
            {
                float pq[4], pt[3];
                const RtAnim &pitched = s_anims[aimPitched];
                RtSampleBone(pitched, RtWrapPhase(aimPhase, pitched.frameCount), b, pq, pt);
                RtBlendPose(quat, trans, pq, pt, aimPitchBlend);
            }
            RtBlendPose(mat->quat, mat->trans, quat, trans, aimFrac);
        }
        if ( actionAnim >= 0 && actionWeight > 0.0f
             && (action == RT_ACTION_TURN || action == RT_ACTION_HIT_FRONT || action == RT_ACTION_HIT_BACK
                 || action == RT_ACTION_KICK
                 || action == RT_ACTION_DEATH || (cur.upperBody && cur.upperBody[b])) )
        {
            float quat[4], trans[3];
            RtSampleBone(s_anims[actionAnim], actionPhase, b, quat, trans);
            if ( actionPitched >= 0 && actionPitchBlend > 0.0f )
            {
                float pq[4], pt[3];
                RtSampleBone(s_anims[actionPitched], actionPhase, b, pq, pt);
                RtBlendPose(quat, trans, pq, pt, actionPitchBlend);
            }
            RtBlendPose(mat->quat, mat->trans, quat, trans, actionWeight);
        }
        if ( b == cur.spineBone && aimCorrection && aimLevel >= 0 && (aimCorrection[0] != 0.0f || aimCorrection[1] != 0.0f) )
        {
            // the aim correction: the upper body turned (yaw about up, pitch about the left axis, in its parent's
            // frame) so the gun's barrel points where the shots land; in by the aim
            const float yaw = aimCorrection[1] * aimFrac * 0.5f * 0.017453292f;
            const float pitch = aimCorrection[0] * aimFrac * 0.5f * 0.017453292f;
            const float qy[4] = { 0.0f, 0.0f, sinf(yaw), cosf(yaw) };
            const float qp[4] = { 0.0f, sinf(pitch), 0.0f, cosf(pitch) };
            float qc[4];
            RtQuatMul(qy, qp, qc);
            float out[4];
            RtQuatMul(qc, mat->quat, out);
            for ( int k = 0; k < 4; ++k )
                mat->quat[k] = out[k];
        }
        const float lenSq = mat->quat[0] * mat->quat[0] + mat->quat[1] * mat->quat[1] + mat->quat[2] * mat->quat[2]
            + mat->quat[3] * mat->quat[3];
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
        obj->skel.partBits.anim[b >> 5] |= 0x80000000 >> (b & 31);
    }
    return action == RT_ACTION_FIRE && actionAnim >= 0 ? actionWeight : 0.0f;
}
