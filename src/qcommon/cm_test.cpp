#include "cm_test.h"
#include "cm_mapkit.h"
#include "cm_trace.h"
#include "cm_load.h"
#include <universal/com_math_anglevectors.h>

int __cdecl CM_PointLeafnum_r(const float *p, int num)
{
    cNode_t *node; // [esp+0h] [ebp-Ch]
    cplane_s *plane; // [esp+4h] [ebp-8h]
    float d; // [esp+8h] [ebp-4h]

    while ( num >= 0 )
    {
        node = &cm.nodes[num];
        plane = node->plane;
        if ( node->plane->type >= 3u )
            d = (float)((float)((float)(plane->normal[0] * *p) + (float)(plane->normal[1] * p[1]))
                                + (float)(plane->normal[2] * p[2]))
                - plane->dist;
        else
            d = p[plane->type] - plane->dist;
        if ( d >= 0.0 )
            num = node->children[0];
        else
            num = node->children[1];
    }
    return -1 - num;
}

int __cdecl CM_PointLeafnum(const float *p)
{
    if ( !cm.numNodes
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_test.cpp", 38, 0, "%s", "cm.numNodes") )
    {
        __debugbreak();
    }
    return CM_PointLeafnum_r(p, 0);
}

void __cdecl CM_BoxLeafnums_r(leafList_s *ll, int nodenum)
{
    cNode_t *node; // [esp+0h] [ebp-Ch]
    int s; // [esp+8h] [ebp-4h]

    iassert(cm.nodes);
    iassert(ll);
    while (nodenum >= 0)
    {
        node = &cm.nodes[nodenum];
        s = BoxOnPlaneSide(ll->bounds[0], ll->bounds[1], node->plane);
        if (s == 1)
        {
            nodenum = node->children[0];
        }
        else
        {
            if (s != 2)
                CM_BoxLeafnums_r(ll, node->children[0]);
            nodenum = node->children[1];
        }
    }
    CM_StoreLeafs(ll, nodenum);
}

void __cdecl CM_StoreLeafs(leafList_s *ll, int nodenum)
{
    int leafNum; // [esp+0h] [ebp-4h]

    leafNum = -1 - nodenum;
    if ( *(unsigned int *)&cm.leafs[-1 - nodenum].cluster != -1 )
        ll->lastLeaf = leafNum;
    if ( ll->count < ll->maxcount )
    {
        if ( (unsigned __int16)leafNum != leafNum
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_test.cpp",
                        69,
                        0,
                        "%s",
                        "static_cast< ushort >( leafNum ) == leafNum") )
        {
            __debugbreak();
        }
        ll->list[ll->count++] = leafNum;
    }
    else
    {
        ll->overflowed = 1;
    }
}

int __cdecl CM_BoxLeafnums(const float *mins, const float *maxs, unsigned __int16 *list, int listsize, int *lastLeaf)
{
    leafList_s ll; // [esp+4h] [ebp-2Ch] BYREF

    *(_QWORD *)&ll.bounds[0][0] = *(_QWORD *)mins;
    ll.bounds[0][2] = mins[2];
    ll.bounds[1][0] = *maxs;
    ll.bounds[1][1] = maxs[1];
    ll.bounds[1][2] = maxs[2];
    ll.count = 0;
    ll.maxcount = listsize;
    ll.list = list;
    ll.lastLeaf = 0;
    ll.overflowed = 0;
    CM_BoxLeafnums_r(&ll, 0);
    *lastLeaf = ll.lastLeaf;
    return ll.count;
}

int __cdecl CM_PointContents(const float *p, unsigned int model)
{
    cLeaf_s *leaf; // [esp+4h] [ebp-10h]
    int i; // [esp+10h] [ebp-4h]

    if ( !cm.numNodes
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_test.cpp", 376, 0, "%s", "cm.numNodes") )
    {
        __debugbreak();
    }
    // mod: mapkit boxes are part of the world
    const int mapkitContents = model ? 0 : CM_Mapkit_PointContents(p);
    if ( !model && CM_Mapkit_Empty() ) // mod: mapkit empty base - no retail world contents
        return mapkitContents;
    if ( model )
        leaf = &CM_ClipHandleToModel(model)->leaf;
    else
        leaf = &cm.leafs[CM_PointLeafnum_r(p, 0)];
    if ( !leaf->leafBrushNode )
        return mapkitContents;
    for ( i = 0; i < 3; ++i )
    {
        if ( leaf->mins[i] >= p[i] )
            return mapkitContents;
        if ( p[i] >= leaf->maxs[i] )
            return mapkitContents;
    }
    return mapkitContents | CM_PointContentsLeafBrushNode_r(p, &cm.leafbrushNodes[leaf->leafBrushNode]);
}

int __cdecl CM_PointContentsLeafBrushNode_r(const float *p, cLeafBrushNode_s *node)
{
    cbrushside_t *side; // [esp+Ch] [ebp-18h]
    int contents; // [esp+10h] [ebp-14h]
    int k; // [esp+14h] [ebp-10h]
    cbrush_t *b; // [esp+18h] [ebp-Ch]
    int i; // [esp+1Ch] [ebp-8h]
    int ia; // [esp+1Ch] [ebp-8h]

    if ( !node && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_test.cpp", 315, 0, "%s", "node") )
        __debugbreak();
    contents = 0;
    while ( 1 )
    {
        if ( !node->leafBrushCount )
            goto LABEL_24;
        if ( node->leafBrushCount > 0 )
            break;
        contents |= CM_PointContentsLeafBrushNode_r(p, node + 1);
LABEL_24:
        node += node->data.children.childOffset[node->data.children.dist >= p[node->axis]];
    }
    for ( k = 0; k < node->leafBrushCount; ++k )
    {
        b = &cm.brushes[node->data.leaf.brushes[k]];
        for ( i = 0; i < 3; ++i )
        {
            if ( b->mins[i] > p[i] || p[i] > b->maxs[i] )
                goto miss;
        }
        side = b->sides;
        ia = b->numsides;
        if ( ia < 0 && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_test.cpp", 339, 0, "%s", "i >= 0") )
            __debugbreak();
        while ( ia )
        {
            if ( (float)((float)((float)(*p * side->plane->normal[0]) + (float)(p[1] * side->plane->normal[1]))
                                 + (float)(p[2] * side->plane->normal[2])) > side->plane->dist )
                goto miss;
            --ia;
            ++side;
        }
        contents |= b->contents;
miss:
        ;
    }
    return contents;
}

int __cdecl CM_TransformedPointContents(const float *p, unsigned int model, const float *origin, const float *angles)
{
    float temp; // [esp+8h] [ebp-3Ch]
    float temp_4; // [esp+Ch] [ebp-38h]
    float axis[3][3]; // [esp+14h] [ebp-30h] BYREF
    float p_l[3]; // [esp+38h] [ebp-Ch] BYREF

    p_l[0] = *p - *origin;
    p_l[1] = p[1] - origin[1];
    p_l[2] = p[2] - origin[2];
    if ( *angles != 0.0 || angles[1] != 0.0 || angles[2] != 0.0 )
    {
        AnglesToAxis(angles, axis);
        temp = p_l[0];
        temp_4 = p_l[1];
        p_l[0] = (float)((float)(p_l[0] * axis[0][0]) + (float)(p_l[1] * axis[0][1])) + (float)(p_l[2] * axis[0][2]);
        p_l[1] = (float)((float)(temp * axis[1][0]) + (float)(p_l[1] * axis[1][1])) + (float)(p_l[2] * axis[1][2]);
        p_l[2] = (float)((float)(temp * axis[2][0]) + (float)(temp_4 * axis[2][1])) + (float)(p_l[2] * axis[2][2]);
    }
    return CM_PointContents(p_l, model);
}

unsigned __int8 *__cdecl CM_ClusterPVS(int cluster)
{
    if ( !cm.vised )
        return cm.visibility;
    if ( (cluster < 0 || cluster >= cm.numClusters)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_test.cpp",
                    450,
                    0,
                    "%s\n\t(cluster) = %i",
                    "(cluster >= 0 && cluster < cm.numClusters)",
                    cluster) )
    {
        __debugbreak();
    }
    return &cm.visibility[cm.clusterBytes * cluster];
}

