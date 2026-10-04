#include "cm_mapkit.h"
#include "cm_load_obj.h"
#include <universal/com_math.h>
#include "com_bsp_load_obj.h"
#include "common.h"
#include "cm_load.h"
#include <game/g_load_utils.h>
#include "com_profilemapload.h"
#include <universal/mem_userhunk.h>
#include <clientscript/cscr_tempmemory.h>
#include <universal/com_files.h>
#include <database/db_registry.h>

const char *g_purgeableEnts[12] =
{
  "misc_model",
  "misc_prefab",
  "dyn_brushmodel",
  "dyn_model",
  "dyn_pointconstraint",
  "dyn_distanceconstraint",
  "dyn_hingeconstraint",
  "dyn_lightconstraint",
  "reflection_probe",
  "info_corona",
  "info_null",
  "func_group"
};

void __cdecl CMod_LoadPlanes()
{
    char v0; // [esp+4h] [ebp-28h]
    char v1; // [esp+8h] [ebp-24h]
    char v2; // [esp+Ch] [ebp-20h]
    cplane_s *out; // [esp+10h] [ebp-1Ch]
    unsigned int planeIter; // [esp+14h] [ebp-18h]
    unsigned __int8 bits; // [esp+1Fh] [ebp-Dh]
    char *in; // [esp+20h] [ebp-Ch]
    unsigned int axisIter; // [esp+24h] [ebp-8h]
    unsigned int count; // [esp+28h] [ebp-4h] BYREF

    in = Com_GetBspLump(LUMP_PLANES, 0x10u, &count);
    if ( !count )
        Com_Error(ERR_DROP, "Map with no planes");
    if ( count > 0x10000 )
        Com_Error(ERR_DROP, "Number of planes exceeds 65536");
    cm.planes = (cplane_s *)CM_Hunk_Alloc(20 * count, "CMod_LoadPlanes", 26);
    cm.planeCount = count;
    out = cm.planes;
    for ( planeIter = 0; planeIter < count; ++planeIter )
    {
        bits = 0;
        for ( axisIter = 0; axisIter < 3; ++axisIter )
        {
            out->normal[axisIter] = *(float *)&in[4 * axisIter];
            if ( out->normal[axisIter] < 0.0 )
                bits |= 1 << axisIter;
        }
        out->dist = *((float *)in + 3);
        if ( out->normal[0] == 1.0 )
        {
            v2 = 0;
        }
        else
        {
            if ( out->normal[1] == 1.0 )
            {
                v1 = 1;
            }
            else
            {
                if ( out->normal[2] == 1.0 )
                    v0 = 2;
                else
                    v0 = 3;
                v1 = v0;
            }
            v2 = v1;
        }
        out->type = v2;
        out->signbits = bits;
        in += 16;
        ++out;
    }
}

MapEnts *__cdecl MapEnts_VirtualLoad(char *name)
{
    return MapEnts_RealLoad(name);
}

bool __cdecl MapEnts_CanPurgeEntity(
                const char *classname,
                void *userData,
                bool (__cdecl *HasKeyCallback)(void *, const char *))
{
    unsigned int classnameIndex; // [esp+0h] [ebp-4h]

    for ( classnameIndex = 0; classnameIndex < 0xC; ++classnameIndex )
    {
        if ( !I_stricmp(classname, g_purgeableEnts[classnameIndex]) )
            return 1;
    }
    return !I_stricmp(classname, "light") && !HasKeyCallback(userData, "pl#");
}

MapEnts *__cdecl MapEnts_RealLoad(char *name)
{
    int numEntityChars; // [esp+0h] [ebp-8h] BYREF
    const char *entityString; // [esp+4h] [ebp-4h]

    entityString = Com_EntityString(&numEntityChars);
    return MapEnts_GetFromString(name, entityString, numEntityChars);
}

MapEnts *__cdecl MapEnts_GetFromString(char *name, const char *entityString, int numEntityChars)
{
    MapEnts *mapEnts; // [esp+10h] [ebp-A2Ch]
    SpawnVar spawnVar; // [esp+14h] [ebp-A28h] BYREF
    const char *end; // [esp+A28h] [ebp-14h]
    int size; // [esp+A2Ch] [ebp-10h]
    int nameLen; // [esp+A30h] [ebp-Ch]
    const char *classname; // [esp+A34h] [ebp-8h] BYREF
    const char *begin; // [esp+A38h] [ebp-4h]
    const char *entityStringa; // [esp+A48h] [ebp+Ch]

    mapEnts = (MapEnts *)CM_Hunk_Alloc(12, "CMod_LoadEntityString", 31);
    nameLen = strlen(name);
    mapEnts->name = (const char *)CM_Hunk_Alloc(nameLen + 1, "CMod_LoadEntityString", 31);
    memcpy((unsigned __int8 *)mapEnts->name, (unsigned __int8 *)name, nameLen + 1);
    entityStringa = Com_EntityString(&numEntityChars);
    mapEnts->entityString = (char *)CM_Hunk_Alloc(numEntityChars, "CMod_LoadEntityString", 31);
    mapEnts->numEntityChars = 0;
    G_SetEntityParsePoint(entityStringa);
    while (1)
    {
        begin = G_GetEntityParsePoint();
        if (!G_ParseSpawnVars(&spawnVar))
            break;
        G_SpawnString(&spawnVar, "classname", "", &classname);
        if (!MapEnts_CanPurgeEntity(classname, &spawnVar, (bool(*)(void*, const char*))CMod_HasSpawnString))
        {
            end = G_GetEntityParsePoint();
            size = end - begin;
            if (end - begin + mapEnts->numEntityChars > numEntityChars
                && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_load_obj.cpp",
                    1212,
                    0,
                    "%s",
                    "mapEnts->numEntityChars + size <= numEntityChars"))
            {
                __debugbreak();
            }
            memcpy((unsigned __int8 *)&mapEnts->entityString[mapEnts->numEntityChars], (unsigned __int8 *)begin, size);
            mapEnts->numEntityChars += size;
        }
    }
    if (mapEnts->numEntityChars >= numEntityChars
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_load_obj.cpp",
            1217,
            0,
            "%s",
            "mapEnts->numEntityChars < numEntityChars"))
    {
        __debugbreak();
    }
    mapEnts->entityString[mapEnts->numEntityChars++] = 0;
    return mapEnts;
}

bool __cdecl CMod_HasSpawnString(const SpawnVar *userData, const char *key)
{
    const char *value; // [esp+4h] [ebp-4h] BYREF

    return G_SpawnString(userData, key, "", &value) != 0;
}

void __cdecl CM_LoadMapFromBsp(const char *name, bool usePvs)
{
    char v2; // [esp+3h] [ebp-21h]
    char *v3; // [esp+8h] [ebp-1Ch]
    const char *v4; // [esp+Ch] [ebp-18h]
    unsigned int version; // [esp+20h] [ebp-4h]

    Com_Memset((unsigned int *)&cm, 0, 332);
    cm.name = (const char *)CM_Hunk_Alloc(strlen(name) + 1, "CM_LoadMapFromBsp", 26);
    v4 = name;
    v3 = (char *)cm.name;
    do
    {
        v2 = *v4;
        *v3++ = *v4++;
    }
    while ( v2 );
    version = Com_GetBspVersion();
    CMod_LoadMaterials();
    CMod_LoadPlanes();
    CMod_LoadBrushRelated(version, usePvs);
    CMod_LoadNodes();
    CMod_LoadLeafSurfaces();
    CMod_LoadCollisionVerts();
    CMod_LoadCollisionTriangles();
    CMod_LoadPartitionIndices();
    CMod_LoadCollisionEdgeWalkable();
    CMod_LoadCollisionBorders();
    CMod_LoadCollisionPartitions();
    if ( usePvs )
    {
        CMod_LoadVisibility();
    }
    else if ( Com_BspHasLump(LUMP_VISIBILITY) )
    {
        Com_Error(ERR_DROP, "In single player, do not compile the bsp with visibility");
    }
    ProfLoad_Begin("Load entity string");
    CMod_LoadEntityString();
    ProfLoad_End();
}

void CMod_LoadMaterials()
{
    const dmaterial_t *in; // [esp+8h] [ebp-Ch]
    unsigned int matIndex; // [esp+Ch] [ebp-8h]
    unsigned int count; // [esp+10h] [ebp-4h] BYREF

    in = (const dmaterial_t *)Com_GetBspLump(LUMP_MATERIALS, 0x48u, &count);
    if ( !count )
        Com_Error(ERR_DROP, "Map with no materials");
    cm.materials = (dmaterial_t *)(CM_Hunk_Alloc(72 * (count + 1), "CMod_LoadMaterials", 26) + 72);
    cm.numMaterials = count;
    Com_Memcpy(cm.materials, in, 72 * count);
    for ( matIndex = 0; matIndex < cm.numMaterials; ++matIndex )
        cm.materials[matIndex].contentFlags &= 0xDFFFFFFB;
}

unsigned int CMod_LoadNodes()
{
    unsigned int result; // eax
    int j; // [esp+0h] [ebp-1Ch]
    cNode_t *out; // [esp+4h] [ebp-18h]
    char *in; // [esp+Ch] [ebp-10h]
    unsigned int nodeIter; // [esp+10h] [ebp-Ch]
    int child; // [esp+14h] [ebp-8h]
    unsigned int count; // [esp+18h] [ebp-4h] BYREF

    in = Com_GetBspLump(LUMP_NODES, 0x24u, &count);
    if ( !count )
        Com_Error(ERR_DROP, "Map has no nodes");
    cm.nodes = (cNode_t *)CM_Hunk_Alloc(8 * count, "CMod_LoadNodes", 26);
    result = count;
    cm.numNodes = count;
    out = cm.nodes;
    nodeIter = 0;
    while ( nodeIter < count )
    {
        out->plane = &cm.planes[*(unsigned int *)in];
        for ( j = 0; j < 2; ++j )
        {
            child = *(unsigned int *)&in[4 * j + 4];
            out->children[j] = child;
            if ( out->children[j] != child )
                Com_Error(ERR_DROP, "CMod_LoadNodes: children exceeded");
        }
        ++nodeIter;
        result = (unsigned int)&out[1];
        ++out;
        in += 36;
    }
    return result;
}

void CMod_LoadLeafSurfaces()
{
    char *in; // [esp+8h] [ebp-8h]
    unsigned int count; // [esp+Ch] [ebp-4h] BYREF

    in = Com_GetBspLump(LUMP_LEAFSURFACES, 4u, &count);
    cm.leafsurfaces = (unsigned int *)CM_Hunk_Alloc(4 * count, "CMod_LoadLeafSurfaces", 29);
    cm.numLeafSurfaces = count;
    Com_Memcpy(cm.leafsurfaces, in, 4 * count);
}

void CMod_LoadCollisionVerts()
{
    char *in; // [esp+8h] [ebp-8h]
    unsigned int count; // [esp+Ch] [ebp-4h] BYREF

    in = Com_GetBspLump(LUMP_COLLISIONVERTS, 0xCu, &count);
    cm.verts = (float (*)[3])CM_Hunk_Alloc(12 * count, "CMod_LoadCollisionVerts", 29);
    cm.vertCount = count;
    Com_Memcpy(cm.verts, in, 12 * count);
}

void CMod_LoadCollisionTriangles()
{
    char *in; // [esp+8h] [ebp-8h]
    unsigned int count; // [esp+Ch] [ebp-4h] BYREF

    in = Com_GetBspLump(LUMP_COLLISIONTRIS, 2u, &count);
    cm.triIndices = (unsigned __int16 *)CM_Hunk_Alloc(2 * count, "CMod_LoadCollisionTriangles", 29);
    cm.triCount = count / 3;
    Com_Memcpy(cm.triIndices, in, 2 * count);
}

void CMod_LoadPartitionIndices()
{
    unsigned int size; // [esp+4h] [ebp-Ch]
    char *in; // [esp+8h] [ebp-8h]
    unsigned int count; // [esp+Ch] [ebp-4h] BYREF

    in = Com_GetBspLump(LUMP_UINDS, 2u, &count);
    size = 2 * count;
    cm.uinds = (unsigned __int16 *)CM_Hunk_Alloc(2 * count, "CMod_LoadPartitionIndices", 29);
    cm.nuinds = count;
    Com_Memcpy(cm.uinds, in, size);
}

void CMod_LoadCollisionEdgeWalkable()
{
    char *in; // [esp+0h] [ebp-8h]
    unsigned int count; // [esp+4h] [ebp-4h] BYREF

    in = Com_GetBspLump(LUMP_COLLISIONEDGEWALKABLE, 1u, &count);
    cm.triEdgeIsWalkable = CM_Hunk_Alloc(count, "CMod_LoadCollisionEdgeWalkable", 29);
    Com_Memcpy(cm.triEdgeIsWalkable, in, count);
}

unsigned int CMod_LoadCollisionBorders()
{
    unsigned int result; // eax
    CollisionBorder *out; // [esp+0h] [ebp-14h]
    const DiskCollBorder *in; // [esp+8h] [ebp-Ch]
    unsigned int index; // [esp+Ch] [ebp-8h]
    unsigned int count; // [esp+10h] [ebp-4h] BYREF

    in = (const DiskCollBorder *)Com_GetBspLump(LUMP_COLLISIONBORDERS, 0x1Cu, &count);
    cm.borders = (CollisionBorder *)CM_Hunk_Alloc(28 * count, "CMod_LoadCollisionBorders", 29);
    result = count;
    cm.borderCount = count;
    out = cm.borders;
    index = 0;
    while ( index < count )
    {
        out->distEq[0] = in->distEq[0];
        out->distEq[1] = in->distEq[1];
        out->distEq[2] = in->distEq[2];
        out->zBase = in->zBase;
        out->zSlope = in->zSlope;
        out->start = in->start;
        out->length = in->length;
        ++index;
        result = (unsigned int)&in[1];
        ++in;
        ++out;
    }
    return result;
}

unsigned int CMod_LoadCollisionPartitions()
{
    unsigned int result; // eax
    CollisionPartition *out; // [esp+0h] [ebp-14h]
    char *in; // [esp+8h] [ebp-Ch]
    unsigned int index; // [esp+Ch] [ebp-8h]
    unsigned int count; // [esp+10h] [ebp-4h] BYREF

    in = Com_GetBspLump(LUMP_COLLISIONPARTITIONS, 0x14u, &count);
    cm.partitions = (CollisionPartition *)CM_Hunk_Alloc(20 * count, "CMod_LoadCollisionPartitions", 29);
    result = count;
    cm.partitionCount = count;
    out = cm.partitions;
    index = 0;
    while ( index < count )
    {
        out->triCount = in[2];
        out->borderCount = in[3];
        out->firstTri = *((unsigned int *)in + 1);
        out->borders = &cm.borders[*((unsigned int *)in + 2)];
        out->nuinds = *((unsigned int *)in + 3);
        out->fuind = *((unsigned int *)in + 4);
        if ( out->firstTri + out->triCount > cm.triCount
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_load_obj.cpp",
                        1069,
                        0,
                        "%s",
                        "out->firstTri + out->triCount <= cm.triCount") )
        {
            __debugbreak();
        }
        if ( out->firstTri < 0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_load_obj.cpp",
                        1070,
                        0,
                        "%s",
                        "out->firstTri >= 0") )
        {
            __debugbreak();
        }
        ++index;
        result = (unsigned int)(in + 20);
        in += 20;
        ++out;
    }
    return result;
}

MapEnts *CMod_LoadEntityString()
{
    MapEnts *result; // eax

    result = MapEnts_VirtualLoad((char *)cm.name);
    cm.mapEnts = result;
    return result;
}

void CMod_LoadVisibility()
{
    const char *v0; // eax
    int *buf; // [esp+0h] [ebp-8h]
    unsigned int len; // [esp+4h] [ebp-4h] BYREF

    buf = (int *)Com_GetBspLump(LUMP_VISIBILITY, 1u, &len);
    if ( len )
    {
        cm.vised = 1;
        cm.numClusters = *buf;
        cm.clusterBytes = buf[1];
        if ( len != cm.clusterBytes * cm.numClusters + 8 )
        {
            v0 = va("%i != %i == %i * %i + %i", len, cm.clusterBytes * cm.numClusters + 8, cm.numClusters, cm.clusterBytes, 8);
            if ( !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_load_obj.cpp",
                            1260,
                            0,
                            "%s\n\t%s",
                            "static_cast< int >( len ) == cm.numClusters * cm.clusterBytes + VIS_HEADER",
                            v0) )
                __debugbreak();
        }
        cm.visibility = CM_Hunk_Alloc(len - 8, "CMod_LoadVisibility", 10);
        Com_Memcpy(cm.visibility, buf + 2, len - 8);
    }
    else
    {
        cm.clusterBytes = (int)((cm.numClusters + 63) & 0xFFFFFFC0) >> 3;
        cm.numClusters = 1;
        cm.visibility = CM_Hunk_Alloc(cm.clusterBytes, "CMod_LoadVisibility", 10);
        Com_Memset((unsigned int *)cm.visibility, 255, cm.clusterBytes);
    }
}

void __cdecl CMod_LoadBrushRelated(unsigned int version, bool usePvs)
{
    int leafbrushNodesCount; // [esp+0h] [ebp-10h]
    HunkUser *user; // [esp+8h] [ebp-8h]
    cLeafBrushNode_s *leafbrushNodes; // [esp+Ch] [ebp-4h]

    CMod_LoadBrushes();
    CMod_LoadLeafBrushes();
    CMod_LoadCollisionAabbTrees();
    if ( version > 0xE )
        CMod_LoadLeafs(usePvs);
    else
        CMod_LoadLeafs_Version14(usePvs);
    CMod_LoadSubmodels();
    user = Hunk_UserCreate(0x400000, HU_SCHEME_DEFAULT, 4u, 0, "CMod_LoadBrushRelated", 27);
    TempMemoryReset(user);
    cm.leafbrushNodes = (cLeafBrushNode_s *)(TempMallocAlignStrict(0) - 20);
    if ( version > 0xE )
        CMod_LoadLeafBrushNodes();
    else
        CMod_LoadLeafBrushNodes_Version14();
    CMod_LoadSubmodelBrushNodes();
    CM_InitBoxHull();
    ++cm.leafbrushNodes;
    leafbrushNodesCount = (TempMallocAlignStrict(0) - (char *)cm.leafbrushNodes) / 20;
    cm.leafbrushNodesCount = leafbrushNodesCount + 1;
    leafbrushNodes = (cLeafBrushNode_s *)CM_Hunk_Alloc(20 * (leafbrushNodesCount + 1), "CMod_LoadBrushRelated", 27);
    memcpy(&leafbrushNodes[1].axis, &cm.leafbrushNodes->axis, 20 * leafbrushNodesCount);
    cm.leafbrushNodes = leafbrushNodes;
    Hunk_UserDestroy(user);
}

unsigned int CMod_LoadSubmodels()
{
    unsigned int result; // eax
    float v1; // [esp+0h] [ebp-44h]
    int j; // [esp+18h] [ebp-2Ch]
    cmodel_t *out; // [esp+1Ch] [ebp-28h]
    char *in; // [esp+24h] [ebp-20h]
    unsigned int bmodelIndex; // [esp+28h] [ebp-1Ch]
    int firstCollAabbIndex; // [esp+2Ch] [ebp-18h]
    float extent[3]; // [esp+30h] [ebp-14h] BYREF
    unsigned int count; // [esp+3Ch] [ebp-8h] BYREF
    int collAabbCount; // [esp+40h] [ebp-4h]

    in = Com_GetBspLump(LUMP_MODELS, 0x2Cu, &count);
    if ( !count )
        Com_Error(ERR_DROP, "Map with no brush models (should at least have the world model)");
    cm.cmodels = (cmodel_t *)CM_Hunk_Alloc(72 * count, "CMod_LoadSubmodels", 27);
    cm.numSubModels = count;
    if ( count > 0x3FFF )
        Com_Error(ERR_DROP, "MAX_SUBMODELS exceeded");
    for ( bmodelIndex = 0; ; ++bmodelIndex )
    {
        result = bmodelIndex;
        if ( bmodelIndex >= count )
            break;
        out = &cm.cmodels[bmodelIndex];
        for ( j = 0; j < 3; ++j )
        {
            out->mins[j] = *(float *)&in[4 * j] - 1.0;
            out->maxs[j] = *(float *)&in[4 * j + 12] + 1.0;
            if ( (float)(fabs(out->mins[j])
                                 - fabs(out->maxs[j])) < 0.0 )
                v1 = fabs(out->maxs[j]);
            else
                v1 = fabs(out->mins[j]);
            extent[j] = v1;
        }
        out->radius = Vec3Length(extent);
        if ( bmodelIndex )
        {
            collAabbCount = *((unsigned int *)in + 8);
            out->leaf.collAabbCount = collAabbCount;
            if ( out->leaf.collAabbCount != collAabbCount )
                Com_Error(ERR_DROP, "CMod_LoadSubmodels: collAabbCount exceeded");
            firstCollAabbIndex = *((unsigned int *)in + 7);
            out->leaf.firstCollAabbIndex = firstCollAabbIndex;
            if ( out->leaf.firstCollAabbIndex != firstCollAabbIndex )
                Com_Error(ERR_DROP, "CMod_LoadSubmodels: firstCollAabbIndex exceeded");
        }
        in += 44;
    }
    return result;
}

unsigned int CMod_LoadSubmodelBrushNodes()
{
    unsigned int result; // eax
    int contents; // [esp+0h] [ebp-24h]
    cmodel_t *out; // [esp+4h] [ebp-20h]
    int numLeafBrushes; // [esp+8h] [ebp-1Ch]
    int leafBrushIndex; // [esp+Ch] [ebp-18h]
    char *in; // [esp+10h] [ebp-14h]
    const DiskBrushModel *ina; // [esp+10h] [ebp-14h]
    unsigned int bmodelIndex; // [esp+14h] [ebp-10h]
    unsigned __int16 *indexes; // [esp+18h] [ebp-Ch]
    unsigned int count; // [esp+1Ch] [ebp-8h] BYREF
    int firstBrush; // [esp+20h] [ebp-4h]

    in = Com_GetBspLump(LUMP_MODELS, 0x2Cu, &count);
    if ( count != cm.numSubModels
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_load_obj.cpp",
                    463,
                    0,
                    "count == cm.numSubModels\n\t%i, %i",
                    count,
                    cm.numSubModels) )
    {
        __debugbreak();
    }
    ina = (const DiskBrushModel *)(in + 44);
    for ( bmodelIndex = 1; ; ++bmodelIndex )
    {
        result = bmodelIndex;
        if ( bmodelIndex >= cm.numSubModels )
            break;
        out = &cm.cmodels[bmodelIndex];
        numLeafBrushes = ina->numBrushes;
        indexes = (unsigned __int16 *)CM_Hunk_Alloc(2 * numLeafBrushes, "CMod_LoadSubmodelBrushNodes", 27);
        contents = 0;
        for ( leafBrushIndex = 0; leafBrushIndex < numLeafBrushes; ++leafBrushIndex )
        {
            firstBrush = leafBrushIndex + ina->firstBrush;
            indexes[leafBrushIndex] = firstBrush;
            if ( indexes[leafBrushIndex] != firstBrush )
                Com_Error(ERR_DROP, "CMod_LoadSubmodelBrushNodes: leafBrushes exceeded");
            contents |= cm.brushes[firstBrush].contents;
        }
        out->leaf.brushContents = contents;
        out->leaf.terrainContents = CMod_GetLeafTerrainContents(&out->leaf);
        CMod_PartionLeafBrushes(indexes, numLeafBrushes, &out->leaf);
        ++ina;
    }
    return result;
}

void __cdecl CMod_PartionLeafBrushes(unsigned __int16 *leafBrushes, int numLeafBrushes, cLeaf_s *leaf)
{
    int j; // [esp+8h] [ebp-28h]
    int ja; // [esp+8h] [ebp-28h]
    float mins[3]; // [esp+Ch] [ebp-24h] BYREF
    int k; // [esp+18h] [ebp-18h]
    cbrush_t *b; // [esp+1Ch] [ebp-14h]
    float maxs[3]; // [esp+20h] [ebp-10h] BYREF
    int brushnum; // [esp+2Ch] [ebp-4h]

    if ( numLeafBrushes )
    {
        mins[0] = FLT_MAX;
        mins[1] = FLT_MAX;
        mins[2] = FLT_MAX;
        maxs[0] = -FLT_MAX;
        maxs[1] = -FLT_MAX;
        maxs[2] = -FLT_MAX;
        for ( k = 0; k < numLeafBrushes; ++k )
        {
            brushnum = leafBrushes[k];
            b = &cm.brushes[brushnum];
            for ( j = 0; j < 3; ++j )
            {
                if ( mins[j] > b->mins[j] )
                    mins[j] = b->mins[j];
                if ( b->maxs[j] > maxs[j] )
                    maxs[j] = b->maxs[j];
            }
        }
        leaf->mins[0] = mins[0];
        leaf->mins[1] = mins[1];
        leaf->mins[2] = mins[2];
        leaf->maxs[0] = maxs[0];
        leaf->maxs[1] = maxs[1];
        leaf->maxs[2] = maxs[2];
        for ( ja = 0; ja < 3; ++ja )
        {
            leaf->mins[ja] = leaf->mins[ja] - 0.125;
            leaf->maxs[ja] = leaf->maxs[ja] + 0.125;
        }
        CM_Hunk_CheckTempMemoryHighClear();
        leaf->leafBrushNode = CMod_PartionLeafBrushes_r(leafBrushes, numLeafBrushes, mins, maxs) - cm.leafbrushNodes;
        CM_Hunk_ClearTempMemoryHigh();
    }
    else if ( leaf->leafBrushNode )
    {
        if ( !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_load_obj.cpp",
                        391,
                        0,
                        "%s",
                        "!leaf->leafBrushNode") )
            __debugbreak();
    }
}

cLeafBrushNode_s *__cdecl CMod_PartionLeafBrushes_r(
                unsigned __int16 *leafBrushes,
                int numLeafBrushes,
                const float *mins,
                const float *maxs)
{
    float v5; // [esp+0h] [ebp-70h]
    float v6; // [esp+4h] [ebp-6Ch]
    cLeafBrushNode_s *node; // [esp+10h] [ebp-60h]
    int side; // [esp+14h] [ebp-5Ch]
    int nodeOffset; // [esp+18h] [ebp-58h]
    float testDist; // [esp+1Ch] [ebp-54h] BYREF
    int numLeafBrushesChild; // [esp+20h] [ebp-50h]
    unsigned __int8 testAxis; // [esp+27h] [ebp-49h]
    int k; // [esp+28h] [ebp-48h]
    float dist; // [esp+2Ch] [ebp-44h]
    float range; // [esp+30h] [ebp-40h]
    float childMaxs[3]; // [esp+34h] [ebp-3Ch] BYREF
    cbrush_t *b; // [esp+40h] [ebp-30h]
    float bestScore; // [esp+44h] [ebp-2Ch]
    int len; // [esp+48h] [ebp-28h]
    float childMins[3]; // [esp+4Ch] [ebp-24h] BYREF
    cLeafBrushNode_s *childNode; // [esp+58h] [ebp-18h]
    unsigned __int16 *leafBrushesCopy; // [esp+5Ch] [ebp-14h]
    cLeafBrushNode_s *returnNode; // [esp+60h] [ebp-10h]
    int axis; // [esp+64h] [ebp-Ch]
    int brushnum; // [esp+68h] [ebp-8h]
    float score; // [esp+6Ch] [ebp-4h]

    if ( !numLeafBrushes
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_load_obj.cpp", 234, 0, "%s", "numLeafBrushes") )
    {
        __debugbreak();
    }
    node = CMod_AllocLeafBrushNode();
    bestScore = 0.0f;
    axis = -1;
    dist = 0.0f;
    for ( testAxis = 0; testAxis < 3u; ++testAxis )
    {
        for ( k = 0; k < numLeafBrushes; ++k )
        {
            brushnum = leafBrushes[k];
            b = &cm.brushes[brushnum];
            testDist = b->mins[testAxis];
            score = CMod_GetPartitionScore(leafBrushes, numLeafBrushes, testAxis, mins, maxs, &testDist);
            if ( score > bestScore )
            {
                bestScore = score;
                axis = testAxis;
                dist = testDist;
            }
            testDist = b->maxs[testAxis];
            score = CMod_GetPartitionScore(leafBrushes, numLeafBrushes, testAxis, mins, maxs, &testDist);
            if ( score > bestScore )
            {
                bestScore = score;
                axis = testAxis;
                dist = testDist;
            }
        }
    }
    if ( axis >= 0 )
    {
        len = 2 * numLeafBrushes;
        leafBrushesCopy = (unsigned __int16 *)CM_Hunk_AllocateTempMemoryHigh(
                                                                                        2 * numLeafBrushes,
                                                                                        "CMod_PartionLeafBrushes_r");
        memcpy((unsigned __int8 *)leafBrushesCopy, (unsigned __int8 *)leafBrushes, len);
        numLeafBrushesChild = 0;
        for ( k = 0; k < numLeafBrushes; ++k )
        {
            brushnum = leafBrushesCopy[k];
            b = &cm.brushes[brushnum];
            if ( b->mins[axis] < dist && dist < b->maxs[axis] )
                leafBrushes[numLeafBrushesChild++] = brushnum;
        }
        if ( numLeafBrushesChild )
        {
            returnNode = CMod_PartionLeafBrushes_r(leafBrushes, numLeafBrushesChild, mins, maxs);
            if ( returnNode != &node[1]
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_load_obj.cpp",
                            309,
                            0,
                            "%s",
                            "returnNode == node + 1") )
            {
                __debugbreak();
            }
            node->leafBrushCount = -1;
            node->contents = returnNode->contents;
            leafBrushes += numLeafBrushesChild;
        }
        range = FLT_MAX;
        node->axis = axis;
        node->data.children.dist = dist;
        side = 0;
LABEL_36:
        if ( side > 1 )
        {
            node->data.children.range = range;
            return node;
        }
        numLeafBrushesChild = 0;
        for ( k = 0; ; ++k )
        {
            if ( k >= numLeafBrushes )
            {
                if ( !numLeafBrushesChild
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_load_obj.cpp",
                                348,
                                0,
                                "%s",
                                "numLeafBrushesChild") )
                {
                    __debugbreak();
                }
                childMins[0] = *mins;
                childMins[1] = mins[1];
                childMins[2] = mins[2];
                childMaxs[0] = *maxs;
                childMaxs[1] = maxs[1];
                childMaxs[2] = maxs[2];
                if ( side )
                    childMaxs[axis] = dist - range;
                else
                    childMins[axis] = dist + range;
                childNode = CMod_PartionLeafBrushes_r(leafBrushes, numLeafBrushesChild, childMins, childMaxs);
                nodeOffset = childNode - node;
                node->data.children.childOffset[side] = nodeOffset;
                if ( node->data.children.childOffset[side] != nodeOffset )
                    Com_Error(ERR_DROP, "CMod_PartionLeafBrushes_r: child exceeded");
                node->contents |= childNode->contents;
                leafBrushes += numLeafBrushesChild;
                ++side;
                goto LABEL_36;
            }
            brushnum = leafBrushesCopy[k];
            b = &cm.brushes[brushnum];
            if ( side )
            {
                if ( b->maxs[axis] > dist )
                    continue;
                if ( (float)((float)(dist - b->maxs[axis]) - range) < 0.0 )
                    v5 = dist - b->maxs[axis];
                else
                    v5 = range;
                range = v5;
            }
            else
            {
                if ( dist > b->mins[axis] )
                    continue;
                if ( (float)((float)(b->mins[axis] - dist) - range) < 0.0 )
                    v6 = b->mins[axis] - dist;
                else
                    v6 = range;
                range = v6;
            }
            leafBrushes[numLeafBrushesChild++] = brushnum;
        }
    }
    node->leafBrushCount = numLeafBrushes;
    if ( node->leafBrushCount != numLeafBrushes )
        Com_Error(ERR_DROP, "CMod_PartionLeafBrushes_r: leafBrushCount exceeded");
    for ( k = 0; k < numLeafBrushes; ++k )
    {
        brushnum = leafBrushes[k];
        b = &cm.brushes[brushnum];
        node->contents |= b->contents;
    }
    if ( !node->contents
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_load_obj.cpp", 282, 0, "%s", "node->contents") )
    {
        __debugbreak();
    }
    node->data.leaf.brushes = leafBrushes;
    return node;
}

cLeafBrushNode_s *__cdecl CMod_AllocLeafBrushNode()
{
    cLeafBrushNode_s *result; // eax

    result = (cLeafBrushNode_s *)TempMallocAlignStrict(20);
    *(unsigned int *)&result->axis = 0;
    result->contents = 0;
    result->data.leaf.brushes = 0;
    result->data.children.range = 0.0;
    *(unsigned int *)result->data.children.childOffset = 0;
    result->data.children.dist = -FLT_MAX;
    return result;
}

double __cdecl CMod_GetPartitionScore(
                unsigned __int16 *leafBrushes,
                int numLeafBrushes,
                int axis,
                const float *mins,
                const float *maxs,
                float *dist)
{
    float v8; // [esp+4h] [ebp-30h]
    int v9; // [esp+8h] [ebp-2Ch]
    float max; // [esp+14h] [ebp-20h]
    int rightBrushCount; // [esp+18h] [ebp-1Ch]
    int k; // [esp+1Ch] [ebp-18h]
    cbrush_t *b; // [esp+20h] [ebp-14h]
    float min; // [esp+24h] [ebp-10h]
    int leftBrushCount; // [esp+28h] [ebp-Ch]

    rightBrushCount = -1;
    leftBrushCount = -1;
    min = -FLT_MAX;
    max = FLT_MAX;
    for ( k = 0; k < numLeafBrushes; ++k )
    {
        b = &cm.brushes[leafBrushes[k]];
        if ( b->mins[axis] < *dist )
        {
            if ( *dist >= b->maxs[axis] )
            {
                ++leftBrushCount;
                if ( b->maxs[axis] > min )
                    min = b->maxs[axis];
            }
        }
        else
        {
            ++rightBrushCount;
            if ( max > b->mins[axis] )
                max = b->mins[axis];
        }
    }
    if ( *dist < min
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_load_obj.cpp", 195, 0, "%s", "min <= *dist") )
    {
        __debugbreak();
    }
    if ( max < *dist
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_load_obj.cpp", 196, 0, "%s", "*dist <= max") )
    {
        __debugbreak();
    }
    if ( rightBrushCount < leftBrushCount )
        v9 = rightBrushCount;
    else
        v9 = leftBrushCount;
    *dist = (float)(min + max) * 0.5;
    if ( v9 <= 0 )
    {
        return 0.0f;
    }
    else
    {
        if ( (float)((float)(maxs[axis] - min) - (float)(max - mins[axis])) < 0.0 )
            v8 = maxs[axis] - min;
        else
            v8 = max - mins[axis];
        return (float)((float)v9 * v8);
    }
}

int __cdecl CMod_GetLeafTerrainContents(cLeaf_s *leaf)
{
    int contents; // [esp+0h] [ebp-8h]
    int k; // [esp+4h] [ebp-4h]

    contents = 0;
    for ( k = 0; k < leaf->collAabbCount; ++k )
        contents |= cm.materials[cm.aabbTrees[k + leaf->firstCollAabbIndex].materialIndex].contentFlags;
    return contents;
}

const unsigned __int8 *CMod_LoadBrushes()
{
    const unsigned __int8 *result; // eax
    unsigned __int8 *v1; // [esp+4h] [ebp-60h]
    unsigned int nverts; // [esp+8h] [ebp-5Ch]
    unsigned __int8 *inVerts; // [esp+Ch] [ebp-58h]
    cbrushside_t *outSides; // [esp+10h] [ebp-54h]
    unsigned int vertsCount; // [esp+14h] [ebp-50h] BYREF
    cbrush_t *outBrush; // [esp+18h] [ebp-4Ch]
    const dbrush_t *inBrush; // [esp+1Ch] [ebp-48h]
    float (*outVerts)[3]; // [esp+20h] [ebp-44h]
    unsigned int allocSizeSides; // [esp+24h] [ebp-40h]
    unsigned int brushCount; // [esp+28h] [ebp-3Ch] BYREF
    float sign; // [esp+2Ch] [ebp-38h]
    const dbrushside_t *inSides; // [esp+30h] [ebp-34h]
    unsigned int brushVertsCounts; // [esp+34h] [ebp-30h] BYREF
    int countAllocatedBrushes; // [esp+38h] [ebp-2Ch]
    int allocSizeBrushVerts; // [esp+3Ch] [ebp-28h]
    unsigned int materialNum; // [esp+40h] [ebp-24h]
    unsigned int allocSizeBrushes; // [esp+44h] [ebp-20h]
    unsigned int axisIter; // [esp+48h] [ebp-1Ch]
    int index; // [esp+4Ch] [ebp-18h]
    const unsigned __int8 *inVertsCounts; // [esp+50h] [ebp-14h]
    int num; // [esp+54h] [ebp-10h]
    unsigned int sideIter; // [esp+58h] [ebp-Ch]
    unsigned int sidesCount; // [esp+5Ch] [ebp-8h] BYREF
    unsigned int brushIter; // [esp+60h] [ebp-4h]

    inBrush = (const dbrush_t *)Com_GetBspLump(LUMP_BRUSHES, 4u, &brushCount);
    inSides = (const dbrushside_t *)Com_GetBspLump(LUMP_BRUSHSIDES, 8u, &sidesCount);
    inVerts = (unsigned __int8 *)Com_GetBspLump(LUMP_BRUSHVERTS, 0xCu, &vertsCount);
    inVertsCounts = (const unsigned __int8 *)Com_GetBspLump(LUMP_BRUSHVERTSCOUNTS, 1u, &brushVertsCounts);
    if ( brushVertsCounts != brushCount )
        Com_Error(ERR_DROP, "CMod_LoadBrushes: bad number of brush verts counts: %i != %i", brushVertsCounts, brushCount);
    sidesCount -= 6 * brushCount;
    allocSizeSides = 12 * sidesCount;
    if ( sidesCount )
        v1 = CM_Hunk_Alloc(allocSizeSides, "CMod_LoadBrushSides", 27);
    else
        v1 = 0;
    cm.brushsides = (cbrushside_t *)v1;
    cm.numBrushSides = sidesCount;
    outSides = (cbrushside_t *)v1;
    countAllocatedBrushes = brushCount + 1;
    allocSizeBrushes = 96 * (brushCount + 1);
    cm.brushes = (cbrush_t *)CM_Hunk_Alloc(allocSizeBrushes, "CMod_LoadBrushes", 27);
    cm.numBrushes = brushCount;
    if ( (unsigned __int16)brushCount != brushCount )
        Com_Error(ERR_DROP, "CMod_LoadBrushes: cm.numBrushes exceeded");
    allocSizeBrushVerts = 12 * vertsCount;
    cm.brushVerts = (float (*)[3])CM_Hunk_Alloc(12 * vertsCount, "CMod_LoadBrushes", 27);
    memcpy((unsigned __int8 *)cm.brushVerts, inVerts, allocSizeBrushVerts);
    result = (const unsigned __int8 *)vertsCount;
    cm.numBrushVerts = vertsCount;
    outVerts = cm.brushVerts;
    outBrush = cm.brushes;
    brushIter = 0;
    while ( brushIter < brushCount )
    {
        outBrush->numsides = inBrush->numSides - 6;
        nverts = *inVertsCounts;
        outBrush->numverts = nverts;
        outBrush->verts = outVerts;
        outVerts += nverts;
        outBrush->sides = outBrush->numsides != 0 ? outSides : 0;
        for ( axisIter = 0; axisIter < 3; ++axisIter )
        {
            sign = -1.0f;
            index = 0;
            while ( index < 2 )
            {
                if ( index )
                    outBrush->maxs[axisIter] = inSides->u.bound;
                else
                    outBrush->mins[axisIter] = inSides->u.bound;
                outBrush->axial_cflags[index][axisIter] = cm.materials[inSides->materialNum].contentFlags;
                outBrush->axial_sflags[index++][axisIter] = cm.materials[inSides->materialNum].surfaceFlags;
                ++inSides;
                sign = 1.0f;
            }
        }
        sideIter = 0;
        while ( sideIter < outBrush->numsides )
        {
            num = inSides->u.planeNum;
            outSides->plane = &cm.planes[num];
            outSides->cflags = cm.materials[inSides->materialNum].contentFlags;
            outSides->sflags = cm.materials[inSides->materialNum].surfaceFlags;
            ++sideIter;
            ++inSides;
            ++outSides;
        }
        materialNum = inBrush->materialNum;
        outBrush->contents = cm.materials[materialNum].contentFlags;
        ++brushIter;
        ++outBrush;
        ++inBrush;
        result = ++inVertsCounts;
    }
    return result;
}

void __cdecl CMod_LoadLeafs(bool usePvs)
{
    cLeaf_s *out; // [esp+0h] [ebp-20h]
    int cluster; // [esp+4h] [ebp-1Ch]
    unsigned int leafIter; // [esp+8h] [ebp-18h]
    char *in; // [esp+10h] [ebp-10h]
    int firstCollAabbIndex; // [esp+14h] [ebp-Ch]
    unsigned int count; // [esp+18h] [ebp-8h] BYREF
    int collAabbCount; // [esp+1Ch] [ebp-4h]

    in = Com_GetBspLump(LUMP_LEAFS, 0x18u, &count);
    if ( !count )
        Com_Error(ERR_DROP, "Map with no leafs");
    cm.leafs = (cLeaf_s *)CM_Hunk_Alloc(44 * count, "CMod_LoadLeafs", 26);
    cm.numLeafs = count;
    cluster = 0;
    out = cm.leafs;
    for ( leafIter = 0; leafIter < count; ++leafIter )
    {
        if ( usePvs )
        {
            cluster = *(unsigned int *)in;
            out->cluster = *(unsigned int *)in;
            if ( out->cluster != cluster )
                Com_Error(ERR_DROP, "CMod_LoadLeafs: cluster exceeded");
        }
        firstCollAabbIndex = *((unsigned int *)in + 1);
        out->firstCollAabbIndex = firstCollAabbIndex;
        if ( out->firstCollAabbIndex != firstCollAabbIndex )
            Com_Error(ERR_DROP, "CMod_LoadLeafs: firstCollAabbIndex exceeded");
        collAabbCount = *((unsigned int *)in + 2);
        out->collAabbCount = collAabbCount;
        if ( out->collAabbCount != collAabbCount )
            Com_Error(ERR_DROP, "CMod_LoadLeafs: collAabbCount exceeded");
        if ( usePvs && cluster >= cm.numClusters )
            cm.numClusters = cluster + 1;
        in += 24;
        ++out;
    }
}

void __cdecl CMod_LoadLeafs_Version14(bool usePvs)
{
    cLeaf_s *out; // [esp+0h] [ebp-20h]
    int cluster; // [esp+4h] [ebp-1Ch]
    unsigned int leafIter; // [esp+8h] [ebp-18h]
    char *in; // [esp+10h] [ebp-10h]
    int firstCollAabbIndex; // [esp+14h] [ebp-Ch]
    unsigned int count; // [esp+18h] [ebp-8h] BYREF
    int collAabbCount; // [esp+1Ch] [ebp-4h]

    in = Com_GetBspLump(LUMP_LEAFS, 0x24u, &count);
    if ( !count )
        Com_Error(ERR_DROP, "Map with no leafs");
    cm.leafs = (cLeaf_s *)CM_Hunk_Alloc(44 * count, "CMod_LoadLeafs", 26);
    cm.numLeafs = count;
    cluster = 0;
    out = cm.leafs;
    for ( leafIter = 0; leafIter < count; ++leafIter )
    {
        if ( usePvs )
        {
            cluster = *(unsigned int *)in;
            out->cluster = *(unsigned int *)in;
            if ( out->cluster != cluster )
                Com_Error(ERR_DROP, "CMod_LoadLeafs: cluster exceeded");
        }
        firstCollAabbIndex = *((unsigned int *)in + 2);
        out->firstCollAabbIndex = firstCollAabbIndex;
        if ( out->firstCollAabbIndex != firstCollAabbIndex )
            Com_Error(ERR_DROP, "CMod_LoadLeafs: firstCollAabbIndex exceeded");
        collAabbCount = *((unsigned int *)in + 3);
        out->collAabbCount = collAabbCount;
        if ( out->collAabbCount != collAabbCount )
            Com_Error(ERR_DROP, "CMod_LoadLeafs: collAabbCount exceeded");
        if ( usePvs && cluster >= cm.numClusters )
            cm.numClusters = cluster + 1;
        in += 36;
        ++out;
    }
}

cLeaf_s *CMod_LoadLeafBrushNodes()
{
    cLeaf_s *result; // eax
    int contents; // [esp+0h] [ebp-24h]
    cLeaf_s *out; // [esp+4h] [ebp-20h]
    unsigned int numLeafBrushes; // [esp+8h] [ebp-1Ch]
    unsigned int leafIter; // [esp+Ch] [ebp-18h]
    char *in; // [esp+10h] [ebp-14h]
    int indexFirstLeafBrush; // [esp+14h] [ebp-10h]
    unsigned int count; // [esp+18h] [ebp-Ch] BYREF
    unsigned int brushIter; // [esp+1Ch] [ebp-8h]
    int brushnum; // [esp+20h] [ebp-4h]

    in = Com_GetBspLump(LUMP_LEAFS, 0x18u, &count);
    if ( count != cm.numLeafs
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_load_obj.cpp",
                    767,
                    0,
                    "count == cm.numLeafs\n\t%i, %i",
                    count,
                    cm.numLeafs) )
    {
        __debugbreak();
    }
    result = cm.leafs;
    out = cm.leafs;
    for ( leafIter = 0; leafIter < cm.numLeafs; ++leafIter )
    {
        numLeafBrushes = *((unsigned int *)in + 4);
        indexFirstLeafBrush = *((unsigned int *)in + 3);
        contents = 0;
        for ( brushIter = 0; brushIter < numLeafBrushes; ++brushIter )
        {
            brushnum = cm.leafbrushes[brushIter + indexFirstLeafBrush];
            contents |= cm.brushes[brushnum].contents;
        }
        out->brushContents = contents;
        out->terrainContents = CMod_GetLeafTerrainContents(out);
        CMod_PartionLeafBrushes(&cm.leafbrushes[indexFirstLeafBrush], numLeafBrushes, out);
        in += 24;
        result = ++out;
    }
    return result;
}

unsigned int CMod_LoadLeafBrushNodes_Version14()
{
    unsigned int result; // eax
    int contents; // [esp+0h] [ebp-24h]
    cLeaf_s *out; // [esp+4h] [ebp-20h]
    unsigned int numLeafBrushes; // [esp+8h] [ebp-1Ch]
    unsigned int leafIter; // [esp+Ch] [ebp-18h]
    DiskLeaf_Version14 *in; // [esp+10h] [ebp-14h]
    int indexFirstLeafBrush; // [esp+14h] [ebp-10h]
    unsigned int count; // [esp+18h] [ebp-Ch] BYREF
    unsigned int brushIter; // [esp+1Ch] [ebp-8h]
    int brushnum; // [esp+20h] [ebp-4h]

    in = (DiskLeaf_Version14 *)Com_GetBspLump(LUMP_LEAFS, 0x24u, &count);
    if ( count != cm.numLeafs
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_load_obj.cpp",
                    810,
                    0,
                    "count == cm.numLeafs\n\t%i, %i",
                    count,
                    cm.numLeafs) )
    {
        __debugbreak();
    }
    if ( !cm.numLeafs
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_load_obj.cpp", 812, 0, "%s", "cm.numLeafs > 0") )
    {
        __debugbreak();
    }
    out = cm.leafs;
    for ( leafIter = 0; ; ++leafIter )
    {
        result = leafIter;
        if ( leafIter >= cm.numLeafs )
            break;
        numLeafBrushes = in->numLeafBrushes;
        indexFirstLeafBrush = in->firstLeafBrush;
        contents = 0;
        for ( brushIter = 0; brushIter < numLeafBrushes; ++brushIter )
        {
            brushnum = cm.leafbrushes[brushIter + indexFirstLeafBrush];
            contents |= cm.brushes[brushnum].contents;
        }
        out->brushContents = contents;
        out->terrainContents = CMod_GetLeafTerrainContents(out);
        CMod_PartionLeafBrushes(&cm.leafbrushes[indexFirstLeafBrush], numLeafBrushes, out);
        ++in;
        ++out;
    }
    return result;
}

unsigned int CMod_LoadLeafBrushes()
{
    unsigned int result; // eax
    unsigned __int16 *out; // [esp+0h] [ebp-18h]
    unsigned int brushIndex; // [esp+4h] [ebp-14h]
    char *in; // [esp+Ch] [ebp-Ch]
    unsigned int iter; // [esp+10h] [ebp-8h]
    unsigned int count; // [esp+14h] [ebp-4h] BYREF

    in = Com_GetBspLump(LUMP_LEAFBRUSHES, 4u, &count);
    cm.leafbrushes = (unsigned __int16 *)CM_Hunk_Alloc(2 * count + 2, "CMod_LoadLeafBrushes", 27);
    cm.numLeafBrushes = count;
    out = cm.leafbrushes;
    for ( iter = 0; ; ++iter )
    {
        result = iter;
        if ( iter >= count )
            break;
        brushIndex = *(unsigned int *)in;
        *out = *(unsigned int *)in;
        if ( *out != brushIndex )
            Com_Error(ERR_DROP, "CMod_LoadLeafBrushes: brushIndex exceeded");
        if ( *out >= (int)cm.numBrushes
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_load_obj.cpp",
                        916,
                        0,
                        "%s\n\t(*out) = %i",
                        "(*out < cm.numBrushes)",
                        *out) )
        {
            __debugbreak();
        }
        in += 4;
        ++out;
    }
    return result;
}

unsigned int CMod_LoadCollisionAabbTrees()
{
    unsigned int result; // eax
    CollisionAabbTree *out; // [esp+0h] [ebp-14h]
    char *in; // [esp+8h] [ebp-Ch]
    unsigned int index; // [esp+Ch] [ebp-8h]
    unsigned int count; // [esp+10h] [ebp-4h] BYREF

    in = Com_GetBspLump(LUMP_COLLISIONAABBS, 0x20u, &count);
    cm.aabbTrees = (CollisionAabbTree *)CM_Hunk_Alloc(32 * count, "CMod_LoadCollisionAabbTrees", 29);
    result = count;
    cm.aabbTreeCount = count;
    out = cm.aabbTrees;
    index = 0;
    while ( index < count )
    {
        out->origin[0] = *(float *)in;
        out->origin[1] = *((float *)in + 1);
        out->origin[2] = *((float *)in + 2);
        out->halfSize[0] = *((float *)in + 3);
        out->halfSize[1] = *((float *)in + 4);
        out->halfSize[2] = *((float *)in + 5);
        out->materialIndex = *((_WORD *)in + 12);
        out->childCount = *((_WORD *)in + 13);
        out->u.firstChildIndex = *((unsigned int *)in + 7);
        ++index;
        result = (unsigned int)(in + 32);
        in += 32;
        ++out;
    }
    return result;
}

unsigned __int16 *CM_InitBoxHull()
{
    cLeafBrushNode_s *v0; // eax
    unsigned __int16 *result; // eax

    cm.box_brush = &cm.brushes[cm.numBrushes];
    cm.box_brush->numsides = 0;
    cm.box_brush->sides = 0;
    cm.box_brush->contents = -1;
    cm.box_model.leaf.brushContents = -1;
    cm.box_model.leaf.terrainContents = 0;
    cm.box_model.leaf.mins[0] = FLT_MAX;
    cm.box_model.leaf.mins[1] = FLT_MAX;
    cm.box_model.leaf.mins[2] = FLT_MAX;
    cm.box_model.leaf.maxs[0] = -FLT_MAX;
    cm.box_model.leaf.maxs[1] = -FLT_MAX;
    cm.box_model.leaf.maxs[2] = -FLT_MAX;
    cm.box_brush->axial_sflags[0][0] = -1;
    cm.box_brush->axial_sflags[0][1] = -1;
    cm.box_brush->axial_sflags[0][2] = -1;
    cm.box_brush->axial_sflags[1][0] = -1;
    cm.box_brush->axial_sflags[1][1] = -1;
    cm.box_brush->axial_sflags[1][2] = -1;
    cm.box_brush->axial_cflags[0][0] = -1;
    cm.box_brush->axial_cflags[0][1] = -1;
    cm.box_brush->axial_cflags[0][2] = -1;
    cm.box_brush->axial_cflags[1][0] = -1;
    cm.box_brush->axial_cflags[1][1] = -1;
    cm.box_brush->axial_cflags[1][2] = -1;
    v0 = CMod_AllocLeafBrushNode();
    cm.box_model.leaf.leafBrushNode = v0 - cm.leafbrushNodes;
    v0->leafBrushCount = 1;
    v0->data.leaf.brushes = &cm.leafbrushes[cm.numLeafBrushes];
    result = cm.leafbrushes;
    cm.leafbrushes[cm.numLeafBrushes] = cm.numBrushes;
    return result;
}

cplane_s *__cdecl CM_GetPlanes()
{
    if ( !cm.planes
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_load_obj.cpp", 1428, 0, "%s", "cm.planes") )
    {
        __debugbreak();
    }
    return cm.planes;
}

int __cdecl CM_GetPlaneCount()
{
    return cm.planeCount;
}

char *__cdecl CM_EntityString()
{
    if ( !cm.mapEnts
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_load_obj.cpp", 1451, 0, "%s", "cm.mapEnts") )
    {
        __debugbreak();
    }
    // mod: with fs_game set, for each zone in bo1_mod_zones (DB_GetModExtraZones) the text of
    // maps/<mapname>.<zone>.ents, if the mod has one, is appended to the level's entities for the server's spawn
    // parse (G_ResetEntityParsePoint): e.g. spawners for another map's AI. Without the file(s) the string is the level's.
    if ( CM_Mapkit_Empty() ) // mod: mapkit empty base - the layout supplies every entity
        return (char *)CM_Mapkit_EntityString(cm.mapEnts->entityString);
    static const char *modBase;
    static char *modString;
    if ( modBase != cm.mapEnts->entityString )
    {
        modBase = cm.mapEnts->entityString;
        if ( modString )
            free(modString);
        modString = NULL;
        const char *zones = DB_GetModExtraZones();
        const char *mapname = Dvar_GetString("mapname");
        if ( *zones && FS_ModsActive() && *mapname ) // mod: fs_game or fs_mods
        {
            size_t len = strlen(modBase);
            char zone[64];
            for ( const char *p = zones; *p; )
            {
                int n = 0;
                while ( *p == ' ' )
                    ++p;
                while ( *p && *p != ' ' && n < 63 )
                    zone[n++] = *p++;
                zone[n] = 0;
                if ( !n )
                    break;
                void *text;
                int size = FS_ReadFile(va("maps/%s.%s.ents", mapname, zone), &text);
                if ( size <= 0 )
                    continue;
                char *grown = (char *)realloc(modString, len + size + 2);
                if ( !modString )
                    memcpy(grown, modBase, len);
                modString = grown;
                modString[len++] = '\n';
                memcpy(&modString[len], text, size);
                len += size;
                modString[len] = 0;
                FS_FreeFile(text);
                Com_Printf(16, "mod: appended maps/%s.%s.ents (%d bytes) to the level entities\n", mapname, zone, size);
            }
        }
    }
    if ( modString )
        return modString;
    return cm.mapEnts->entityString;
}

