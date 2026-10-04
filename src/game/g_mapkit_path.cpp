// mod: mapkit pathnodes (bo1_mod_mapkit, off by default = nothing here runs). Adds the layout's pathnodes to the
// fastfile's PathData after load and links them into the map's graph with the game's own link test (Path_CanLinkNodes,
// actor physics through the world traces, which include the mapkit boxes), and cuts retail links that the new boxes
// block. Two hooks in G_InitGame (first load only):
//   G_Mapkit_AppendPathnodes: after Path_PreSpawnInitPaths, before G_DropPathnodesToFloor (which drops the new nodes /
//     marks them bad like retail ones). nodes/basenodes move to static arrays (fastfile arrays can't grow; everything
//     refers to nodes by index), the new nodes go into the kd tree leaves, pathVis rows are re-strided.
//   G_Mapkit_LinkPathnodes: after G_UpdateTrackExtraNodes, before Path_InitPaths (which validates every node).
// Node sources: "room" edits get a grid ("nodeSpacing", default 56; 0 = none) 24 units inside the walls; "node" edits
// give one node at "origin".
#include "pathnode.h"
#include "pathnode_load_obj.h"
#include "g_bsp.h"
#include <qcommon/cm_mapkit.h>
#include <game_mp/g_main_mp.h>
#include <qcommon/common.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <clientscript/cscr_stringlist.h>
#include <universal/com_memory.h>

static pathnode_t s_mkNodes[0x2000];
static pathbasenode_t s_mkBaseNodes[0x2000];
static pathlink_s s_mkLinks[0x8000];
static unsigned __int16 s_mkLeafNodes[0x4000];
static unsigned char *s_mkVis;
static int s_mkLinkUsed, s_mkLeafUsed;
static int s_mkFirstAdded = -1, s_mkAddedCount;

int G_Mapkit_FirstAddedNode() { return s_mkFirstAdded; }
int G_Mapkit_AddedNodeCount() { return s_mkAddedCount; }

// a layout node: a plain pathnode, or one end of a negotiation (traverse) pair
struct MkNodeSrc
{
    float origin[3];
    nodeType type;
    float yaw;
    char target[32];
    char targetname[32];
    const char *animscript;
};
static MkNodeSrc s_mkSrc[0x2000];
static int s_mkGridOmitted;

// Automatic room grids avoid static geometry with the standing actor hull. Explicit nodes
// still go through the engine's bad-node checks, as do candidates over missing floors.
static bool Mapkit_GridInSolid(const float *p)
{
    for (int i = 0; i < CM_Mapkit_BoxCount(); ++i)
    {
        const MapkitBox *box = CM_Mapkit_GetBox(i);
        if (box->contents && p[0] + 16 > box->mins[0] && p[0] - 16 < box->maxs[0]
            && p[1] + 16 > box->mins[1] && p[1] - 16 < box->maxs[1]
            && p[2] + 56 > box->mins[2] && p[2] - 15.75f < box->maxs[2])
            return true;
    }
    return false;
}

// mapkit-fix-4d: stairs treads and walkable box tops sat inside the perk / pap / powerswitch / box clips added in
// mapkit-fix-4b (raised_example: Speed Cola on the platform -> check node_clearance FAIL 2 bad nodes). Those automatic
// surface nodes are omitted like grid candidates; only machine clips are tested (the next stair tread must not count).
static bool Mapkit_InMachineClip(const float *p)
{
    for (int i = 0; i < CM_Mapkit_BoxCount(); ++i)
    {
        const MapkitBox *box = CM_Mapkit_GetBox(i);
        if (box->hidden && box->contents == 0x8030200 && p[0] + 16 > box->mins[0] && p[0] - 16 < box->maxs[0]
            && p[1] + 16 > box->mins[1] && p[1] - 16 < box->maxs[1]
            && p[2] + 56 > box->mins[2] && p[2] - 15.75f < box->maxs[2])
            return true;
    }
    return false;
}

static void Mapkit_SetNode(MkNodeSrc *src, const float *origin, nodeType type)
{
    memset(src, 0, sizeof(*src));
    src->origin[0] = origin[0]; src->origin[1] = origin[1]; src->origin[2] = origin[2];
    src->type = type;
}

static int Mapkit_CollectNodes(MkNodeSrc *out, int max)
{
    int n = 0;
    s_mkGridOmitted = 0;
    for (int e = 0; e < CM_Mapkit_EditCount(); ++e)
    {
        const char *kind = 0;
        float v[3], mins[3], maxs[3];
        if (CM_Mapkit_GetField(e, "kind", &kind, v) != MKF_STRING)
            continue;
        if (!strcmp(kind, "node") && CM_Mapkit_GetField(e, "origin", 0, v) == MKF_VECTOR && n < max)
        {
            v[2] += 16.0f;
            Mapkit_SetNode(&out[n++], v, NODE_PATHNODE);
        }
        // stairs treads and "walkable" box tops: surface nodes, handled like explicit node edits
        float walk[64][3];
        for (int w = 0, count = CM_Mapkit_WalkNodes(e, walk, 64); w < count && n < max; ++w)
        {
            walk[w][2] += 16.0f;
            if (Mapkit_InMachineClip(walk[w]))
            {
                ++s_mkGridOmitted;
                continue;
            }
            Mapkit_SetNode(&out[n++], walk[w], NODE_PATHNODE);
        }
        // a window's traverse pair: begin outside (target -> end), end inside; the link is made by Path_CanLinkNodes
        // (Path_IsNegotiationLink: begin.target == end.targetname)
        float begin[3], end[3], yaw;
        if (!strcmp(kind, "window") && n + 2 <= max && CM_Mapkit_WindowNodes(e, begin, end, &yaw))
        {
            MkNodeSrc *b = &out[n++];
            MkNodeSrc *en = &out[n++];
            Mapkit_SetNode(b, begin, NODE_NEGOTIATION_BEGIN);
            Mapkit_SetNode(en, end, NODE_NEGOTIATION_END);
            b->yaw = en->yaw = yaw;
            Com_sprintf(b->target, sizeof(b->target), "mkwin%d_end", e);
            Com_sprintf(en->targetname, sizeof(en->targetname), "mkwin%d_end", e);
            b->animscript = "zombie_mantle_over_40"; // MEASURED: 30 of Five's 38 begin nodes (pf82 E:165-171)
        }
        if (strcmp(kind, "room") || CM_Mapkit_GetField(e, "mins", 0, mins) != MKF_VECTOR || CM_Mapkit_GetField(e, "maxs", 0, maxs) != MKF_VECTOR)
            continue;
        float spacing = 56.0f;
        if (CM_Mapkit_GetField(e, "nodeSpacing", 0, v) == MKF_NUMBER)
            spacing = v[0];
        if (spacing <= 0.0f)
            continue;
        int count[2];
        float step[2];
        for (int k = 0; k < 2; ++k)
        {
            const float span = maxs[k] - mins[k] - 48.0f;
            count[k] = span > 0.0f ? (int)(span / spacing) + 1 : 1;
            step[k] = count[k] > 1 ? span / (float)(count[k] - 1) : 0.0f;
        }
        for (int y = 0; y < count[1]; ++y)
            for (int x = 0; x < count[0] && n < max; ++x)
            {
                const float p[3] = { count[0] > 1 ? mins[0] + 24.0f + step[0] * x : (mins[0] + maxs[0]) * 0.5f,
                    count[1] > 1 ? mins[1] + 24.0f + step[1] * y : (mins[1] + maxs[1]) * 0.5f, mins[2] + 16.0f };
                if (Mapkit_GridInSolid(p))
                    ++s_mkGridOmitted;
                else
                    Mapkit_SetNode(&out[n++], p, NODE_PATHNODE);
            }
    }
    return n;
}

// the kd tree leaves hold node indices; a new node joins the leaf its origin falls in (leaf lists move to s_mkLeafNodes)
static void Mapkit_TreeInsert(pathnode_tree_t *tree, unsigned __int16 nodeIndex, const float *origin)
{
    while (tree && tree->axis >= 0)
        tree = origin[tree->axis] < tree->dist ? tree->u.child[0] : tree->u.child[1];
    if (!tree || s_mkLeafUsed + tree->u.s.nodeCount + 1 > (int)(sizeof(s_mkLeafNodes) / sizeof(s_mkLeafNodes[0])))
        return;
    unsigned __int16 *list = &s_mkLeafNodes[s_mkLeafUsed];
    memcpy(list, tree->u.s.nodes, tree->u.s.nodeCount * sizeof(unsigned __int16));
    list[tree->u.s.nodeCount] = nodeIndex;
    s_mkLeafUsed += tree->u.s.nodeCount + 1;
    tree->u.s.nodes = list;
    ++tree->u.s.nodeCount;
}

void G_Mapkit_AppendPathnodes()
{
    s_mkFirstAdded = -1;
    s_mkAddedCount = 0;
    s_mkLinkUsed = 0;
    s_mkLeafUsed = 0;
    if (!CM_Mapkit_Active() || !gameWorldCurrent)
        return;
    PathData *path = &gameWorldCurrent->path;
    int n = Mapkit_CollectNodes(s_mkSrc, 0x2000);
    // empty base: the retail nodes are dropped (their world is off); the layout's nodes are the whole graph
    const bool empty = CM_Mapkit_Empty();
    const unsigned int oldCount = empty ? 0 : path->nodeCount;
    if (!n || (!empty && oldCount != g_path.actualNodeCount) || oldCount + n > 0x2000 || path->nodes == s_mkNodes)
    {
        if (n)
            Com_PrintWarning(18, "mapkit: %d layout pathnodes not added (nodes %u, matched %u)\n", n, oldCount, g_path.actualNodeCount);
        return;
    }
    // zombies: pathVis rows are strided by actualNodeCount (pathnode.cpp NodeVisCacheEntry / ExpandedNodeVisCacheEntry):
    // move bit (row r, col c) from c + N0*r to c + N1*r; pairs with a new node stay 0 (not visible)
    const unsigned int newCount = oldCount + n;
    const unsigned int visBytes = (newCount * (newCount - 1) + 7) >> 3;
    unsigned char *vis = (unsigned char *)calloc(visBytes, 1);
    if (!vis)
        return;
    if (path->pathVis && !empty)
        for (unsigned int r = 0; r + 1 < oldCount; ++r)
            for (unsigned int c = 0; c < oldCount; ++c)
            {
                const unsigned int from = c + oldCount * r, to = c + newCount * r;
                if (path->pathVis[from >> 3] & (1 << (from & 7)))
                    vis[to >> 3] |= (unsigned char)(1 << (to & 7));
            }
    if (s_mkVis)
        free(s_mkVis);
    s_mkVis = vis;
    path->pathVis = vis;
    path->visBytes = visBytes;

    memcpy(s_mkNodes, path->nodes, oldCount * sizeof(pathnode_t));
    memcpy(s_mkBaseNodes, path->basenodes, oldCount * sizeof(pathbasenode_t));
    for (int i = 0; i < n; ++i)
    {
        pathnode_t *node = &s_mkNodes[oldCount + i];
        memset(node, 0, sizeof(*node));
        const MkNodeSrc *src = &s_mkSrc[i];
        node->constant.type = src->type;
        node->constant.vOrigin[0] = src->origin[0];
        node->constant.vOrigin[1] = src->origin[1];
        node->constant.vOrigin[2] = src->origin[2];
        node->constant.fAngle = src->yaw;
        node->constant.forward[0] = cosf(src->yaw * 0.017453292f);
        node->constant.forward[1] = sinf(src->yaw * 0.017453292f);
        if (src->target[0])
            node->constant.target = (unsigned __int16)SL_GetString((char *)src->target, 1u, SCRIPTINSTANCE_SERVER);
        if (src->targetname[0])
            node->constant.targetname = (unsigned __int16)SL_GetString((char *)src->targetname, 1u, SCRIPTINSTANCE_SERVER);
        if (src->animscript)
        {
            // GScr_SetScriptsForPathNodes ran before this (GScr_LoadScripts): resolve the traverse script here, as
            // pathnode.cpp does for loaded nodes (a missing script makes the node bad)
            node->constant.animscript = (unsigned __int16)SL_GetString((char *)src->animscript, 1u, SCRIPTINSTANCE_SERVER);
            node->constant.animscriptfunc = (int)Hunk_FindDataForFile(1, src->animscript);
            if (!node->constant.animscriptfunc)
            {
                Com_PrintWarning(18, "mapkit: traverse node %d: animscript '%s' is not loaded\n", i, src->animscript);
                node->constant.type = NODE_BADNODE;
            }
        }
        node->constant.wOverlapNode[0] = -1;
        node->constant.wOverlapNode[1] = -1;
        node->constant.wChainId = -1;
        node->constant.wChainParent = -1;
        Path_InitNodeDynamic(node);
        G_InitPathBaseNode(&s_mkBaseNodes[oldCount + i], node);
        if (!empty)
            Mapkit_TreeInsert(path->nodeTree, (unsigned __int16)(oldCount + i), node->constant.vOrigin);
    }
    // the node array first: Path_BuildNodeBsp_r reads origins through gameWorldCurrent->path.nodes (built before
    // this, the empty base's tree split the layout's node indices by the RETAIL nodes' origins: goal lookups away
    // from the retail layout found no candidates, e.g. a player at a wall buy was unreachable)
    path->nodes = s_mkNodes;
    path->basenodes = s_mkBaseNodes;
    path->nodeCount = newCount;
    g_path.actualNodeCount = newCount;
    if (empty)
    {
        // the node kd tree from zero (retail non-fastfile path, Path_MakePathDataPermanent); leaves point into s_mkIndirect
        static unsigned __int16 s_mkIndirect[0x2000];
        for (int i = 0; i < n; ++i)
            s_mkIndirect[i] = (unsigned __int16)i;
        path->nodeTreeCount = 0;
        path->nodeTree = Path_BuildNodeBsp_r(s_mkIndirect, n);
        path->chainNodeCount = 0;
        path->chainNodeForNode = 0;
        path->nodeForChainNode = 0;
    }
    s_mkFirstAdded = (int)oldCount;
    s_mkAddedCount = n;
    Com_Printf(18, "mapkit: added %d pathnodes (%u..%u)\n", n, oldCount, newCount - 1);
}

static bool Mapkit_LinkNearBox(const float *a, const float *b)
{
    float lo[3], hi[3];
    for (int k = 0; k < 3; ++k)
    {
        lo[k] = (a[k] < b[k] ? a[k] : b[k]) - (k == 2 ? -8.0f : 16.0f);
        hi[k] = (a[k] > b[k] ? a[k] : b[k]) + (k == 2 ? 64.0f : 16.0f);
    }
    for (int i = 0; i < CM_Mapkit_BoxCount(); ++i)
    {
        const MapkitBox *box = CM_Mapkit_GetBox(i);
        if (!box->contents) // decorative boxes and dynamic blockers do not permanently cut links
            continue;
        if (box->mins[0] < hi[0] && box->maxs[0] > lo[0] && box->mins[1] < hi[1] && box->maxs[1] > lo[1]
            && box->mins[2] < hi[2] && box->maxs[2] > lo[2])
            return true;
    }
    return false;
}

// give a node a new link list (pool copy) of `count` links
static pathlink_s *Mapkit_AllocLinks(int count)
{
    if (s_mkLinkUsed + count > (int)(sizeof(s_mkLinks) / sizeof(s_mkLinks[0])))
        return 0;
    pathlink_s *links = &s_mkLinks[s_mkLinkUsed];
    s_mkLinkUsed += count;
    return links;
}

static void Mapkit_AddLink(pathnode_t *from, unsigned int toIndex, float dist, int negotiation)
{
    for (int l = 0; l < from->constant.totalLinkCount; ++l)
        if (from->constant.Links[l].nodeNum == toIndex)
            return;
    const int count = from->constant.totalLinkCount;
    pathlink_s *links = Mapkit_AllocLinks(count + 1);
    if (!links)
        return;
    if (count)
        memcpy(links, from->constant.Links, count * sizeof(pathlink_s));
    memset(&links[count], 0, sizeof(pathlink_s));
    links[count].fDist = dist;
    links[count].nodeNum = (unsigned __int16)toIndex;
    links[count].negotiationLink = (unsigned __int8)(negotiation != 0);
    from->constant.Links = links;
    from->constant.totalLinkCount = (unsigned __int16)(count + 1);
    from->dynamic.wLinkCount = (__int16)(count + 1);
}

void G_Mapkit_LinkPathnodes()
{
    if (s_mkFirstAdded < 0 || !gameWorldCurrent)
        return;
    PathData *path = &gameWorldCurrent->path;
    const unsigned int first = (unsigned int)s_mkFirstAdded;
    // zombies: SP ConnectPaths (0x0044a140) only restores saved disconnected links.
    // Build topology without dynamic blockers, as Path_ConnectPaths does; the later
    // Path_AutoDisconnectPaths pass closes their links until the door is purchased.
    int oldContents[MAX_GENTITIES_SV];
    for (int i = 0; i < level.num_entities; ++i)
    {
        oldContents[i] = g_entities[i].r.contents;
        if (g_entities[i].r.inuse && Path_IsDynamicBlockingEntity(&g_entities[i]))
            g_entities[i].r.contents = 0;
    }
    int cut = 0, linked = 0, bad = 0;
    float dist;
    int negotiation;
    // 1. cut retail links that a mapkit box now blocks (the game's own link test fails); traversal links are kept
    for (unsigned int i = 0; i < first; ++i)
    {
        pathnode_t *node = &path->nodes[i];
        const int count = node->constant.totalLinkCount;
        if (!count || node->dynamic.wLinkCount != count)
            continue;
        pathlink_s *keep = 0;
        int kept = 0;
        for (int l = 0; l < count; ++l)
        {
            const pathlink_s *link = &node->constant.Links[l];
            pathnode_t *to = &path->nodes[link->nodeNum];
            bool drop = !link->negotiationLink && Mapkit_LinkNearBox(node->constant.vOrigin, to->constant.vOrigin)
                && !Path_CanLinkNodes(node, to, &dist, &negotiation);
            if (drop && !keep)
            {
                keep = Mapkit_AllocLinks(count);
                if (!keep)
                    break;
                memcpy(keep, node->constant.Links, l * sizeof(pathlink_s));
                kept = l;
            }
            if (drop)
                ++cut;
            else if (keep)
                keep[kept++] = *link;
        }
        if (keep)
        {
            node->constant.Links = keep;
            node->constant.totalLinkCount = (unsigned __int16)kept;
            node->dynamic.wLinkCount = (__int16)kept;
        }
    }
    // 2. link each new node both ways to every usable node within 256 units horizontally / 128 vertically
    for (unsigned int i = first; i < path->nodeCount; ++i)
    {
        pathnode_t *node = &path->nodes[i];
        if (node->constant.type == NODE_BADNODE)
        {
            ++bad;
            continue;
        }
        pathlink_s tmp[256];
        int found = 0;
        for (unsigned int j = 0; j < path->nodeCount; ++j)
        {
            pathnode_t *other = &path->nodes[j];
            if (j == i || other->constant.type == NODE_BADNODE)
                continue;
            const float dx = other->constant.vOrigin[0] - node->constant.vOrigin[0];
            const float dy = other->constant.vOrigin[1] - node->constant.vOrigin[1];
            const float dz = other->constant.vOrigin[2] - node->constant.vOrigin[2];
            if (dx * dx + dy * dy > 256.0f * 256.0f || fabsf(dz) > 128.0f)
                continue;
            if (Path_CanLinkNodes(node, other, &dist, &negotiation))
            {
                if (first == 0 && found < 256)
                {
                    // graph from zero (empty base): collect, then one pool block per node (no per-link copies)
                    memset(&tmp[found], 0, sizeof(tmp[found]));
                    tmp[found].fDist = dist;
                    tmp[found].nodeNum = (unsigned __int16)j;
                    tmp[found].negotiationLink = (unsigned __int8)(negotiation != 0);
                    ++found;
                }
                else if (first)
                    Mapkit_AddLink(node, j, dist, negotiation);
                ++linked;
            }
            if (j < first && Path_CanLinkNodes(other, node, &dist, &negotiation))
            {
                Mapkit_AddLink(other, i, dist, negotiation);
                ++linked;
            }
        }
        if (found)
        {
            pathlink_s *links = Mapkit_AllocLinks(found);
            if (links)
            {
                memcpy(links, tmp, found * sizeof(pathlink_s));
                node->constant.Links = links;
                node->constant.totalLinkCount = (unsigned __int16)found;
                node->dynamic.wLinkCount = (__int16)found;
            }
        }
    }
    if (first == 0 && path->pathVis)
    {
        // empty base: node visibility from zero (sight traces hit only the mapkit boxes; retail Path_BuildNodeVis)
        memset(path->pathVis, 0, path->visBytes);
        Path_BuildNodeVis(path->pathVis, path->visBytes);
    }
    for (int i = 0; i < level.num_entities; ++i)
        g_entities[i].r.contents = oldContents[i];
    G_LogPrintf("mapkit: check node_clearance %s %d bad nodes; %d automatic candidates omitted at solids\n",
        bad ? "FAIL" : "PASS", bad, s_mkGridOmitted);
    Com_Printf(18, "mapkit: pathnodes %u..%u: %d links added, %d retail links cut, %d new nodes bad (in solid / no floor)\n",
        first, path->nodeCount - 1, linked, cut, bad);
    // self-test: every layout node must be found by the goal-node lookup an actor uses (Path_FindPathFrom: radius 192,
    // 64 candidates, height check) at its own spot; a node the lookup misses is a spot no zombie can path to
    int found = 0, missed = 0, firstMiss = -1;
    for (unsigned int i = first; i < path->nodeCount; ++i)
    {
        pathnode_t *node = &path->nodes[i];
        if (node->constant.type == NODE_BADNODE || node->constant.type == NODE_NEGOTIATION_BEGIN
            || node->constant.type == NODE_NEGOTIATION_END)
            continue;
        pathsort_t cands[64];
        int count = 0;
        if (Path_NearestNode(node->constant.vOrigin, cands, -2, 192.0f, &count, 64, NEAREST_NODE_DO_HEIGHT_CHECK))
            ++found;
        else if (++missed == 1)
            firstMiss = (int)i;
    }
    const char *verdict = missed ? "FAIL" : "PASS";
    // G_LogPrintf mirrors mapkit checks to the console and includes this native check in the DONE totals.
    G_LogPrintf("mapkit: check pathnodes %s goal lookup finds %d/%d layout nodes (first miss %d)\n", verdict, found, found + missed, firstMiss);
}

// Path_Shutdown clears every node's target/targetname; retail nodes get them back from their node_* entity-string
// entries, layout nodes have none: put the traverse names back on a restart (init_traverse reads self.target)
void G_Mapkit_RestorePathnodeStrings()
{
    if (s_mkFirstAdded < 0 || !gameWorldCurrent || gameWorldCurrent->path.nodes != s_mkNodes)
        return;
    for (int i = 0; i < s_mkAddedCount; ++i)
    {
        pathnode_t *node = &s_mkNodes[s_mkFirstAdded + i];
        const MkNodeSrc *src = &s_mkSrc[i];
        if (src->target[0])
            node->constant.target = (unsigned __int16)SL_GetString((char *)src->target, 1u, SCRIPTINSTANCE_SERVER);
        if (src->targetname[0])
            node->constant.targetname = (unsigned __int16)SL_GetString((char *)src->targetname, 1u, SCRIPTINSTANCE_SERVER);
    }
}
