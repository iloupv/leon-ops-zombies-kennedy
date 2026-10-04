#include "cg_colltree.h"
#include <universal/assertive.h>
#include <cstring>
#include <qcommon/cm_load.h>
#include <cgame_mp/cg_local_mp.h>
#include <universal/dvar.h>
#include <intrin.h>
#include <windows.h>

CgEntCollNode cgEntCollNodes_data[1][MAX_CL_ENTNUM]; // mod (L43): retail [1536]

CgEntCollWorld cgEntCollWorld[1];
CgEntCollNode *cgEntCollNodes[1] = { cgEntCollNodes_data[0] };

// mod (L43): bo1_colltree_check 1/2 - verify the entity collision tree every 64th / every link op (the tree walked
// from sector 1: each listed node names its sector, lists ascend, no node twice; every node with a sector is listed;
// free sectors hold no entities). The first failure prints the ops since the last good check (op, entity, the node's
// sector before and after, the entity's eType/number, the caller as exe offset) once.
static const dvar_s *cg_collTreeCheck;
struct CgCollOp { unsigned short ent, before, after; unsigned char op, eType; int number; unsigned int caller; };
static CgCollOp s_collOps[256];
static unsigned int s_collOpCount, s_collOpsSinceCheck;
static bool s_collTreeReported;

static const char *CG_CheckEntityCollWorldFail(int localClientNum)
{
    static unsigned char seen[MAX_CL_ENTNUM];
    static unsigned short stack[1024];
    static char why[160];
    const CgEntCollWorld *world = &cgEntCollWorld[localClientNum];
    const CgEntCollNode *nodes = cgEntCollNodes[localClientNum];
    unsigned int depth = 0, visited = 0, i;
    memset(seen, 0, sizeof(seen));
    stack[depth++] = 1;
    while (depth)
    {
        unsigned short s = stack[--depth];
        if (s >= 1024 || ++visited > 1024)
            return snprintf(why, sizeof(why), "tree: sector %u / %u visited", s, visited), why;
        const CgEntCollSector *sector = &world->sectors[s];
        unsigned int prev = 0, count = 0;
        for (unsigned int li = sector->entListHead; li; li = nodes[li - 1].nextEntInSector)
        {
            if (li > MAX_CL_ENTNUM || li <= prev || ++count > MAX_CL_ENTNUM)
                return snprintf(why, sizeof(why), "sector %u list: index %u after %u", s, li, prev), why;
            if (seen[li - 1] || nodes[li - 1].sector != s)
                return snprintf(why, sizeof(why), "sector %u lists ent %u whose node says sector %u (seen %u)", s, li - 1,
                    nodes[li - 1].sector, seen[li - 1]), why;
            seen[li - 1] = 1;
            prev = li;
        }
        for (int c = 0; c < 2; ++c)
        {
            if (!sector->tree.child[c])
                continue;
            if (world->sectors[sector->tree.child[c]].tree.u.parent != s || depth >= 1024)
                return snprintf(why, sizeof(why), "sector %u child %u has parent %u", s, sector->tree.child[c],
                    world->sectors[sector->tree.child[c]].tree.u.parent), why;
            stack[depth++] = sector->tree.child[c];
        }
    }
    for (i = 0; i < MAX_CL_ENTNUM; ++i)
    {
        if (nodes[i].sector && !seen[i])
            return snprintf(why, sizeof(why), "ent %u: node says sector %u (entListHead %u) but no tree list holds it", i,
                nodes[i].sector, world->sectors[nodes[i].sector < 1024 ? nodes[i].sector : 0].entListHead), why;
    }
    for (unsigned int f = world->freeHead, n = 0; f; f = world->sectors[f].tree.u.parent)
    {
        if (f >= 1024 || ++n > 1024 || world->sectors[f].entListHead)
            return snprintf(why, sizeof(why), "free sector %u (entListHead %u)", f, f < 1024 ? world->sectors[f].entListHead : 0), why;
    }
    return 0;
}

static void CG_CollTreeOp(int localClientNum, unsigned int entIndex, int op, unsigned int caller)
{
    if (!cg_collTreeCheck || !cg_collTreeCheck->current.integer || s_collTreeReported)
        return;
    if (s_collOpCount)
    {
        CgCollOp *last = &s_collOps[(s_collOpCount - 1) & 255];
        last->after = cgEntCollNodes[localClientNum][last->ent].sector;
    }
    if (++s_collOpsSinceCheck >= (cg_collTreeCheck->current.integer >= 2 ? 1u : 64u))
    {
        const char *why = CG_CheckEntityCollWorldFail(localClientNum);
        if (why)
        {
            s_collTreeReported = true;
            Com_Printf(1, "BO1_COLLTREE FAIL: %s (checked before op %u)\n", why, s_collOpCount);
            unsigned int n = s_collOpsSinceCheck - 1 < 256 ? s_collOpsSinceCheck - 1 : 256;
            for (unsigned int k = s_collOpCount - n; k != s_collOpCount; ++k)
            {
                const CgCollOp *o = &s_collOps[k & 255];
                Com_Printf(1, "BO1_COLLTREE op %u: %s ent %u sector %u -> %u eType %u number %d caller +0x%X\n", k,
                    o->op ? "link" : "unlink", o->ent, o->before, o->after, o->eType, o->number, o->caller);
            }
            return;
        }
        s_collOpsSinceCheck = 0;
    }
    CgCollOp *o = &s_collOps[s_collOpCount++ & 255];
    const centity_s *cent = CG_GetEntity(localClientNum, entIndex);
    o->ent = (unsigned short)entIndex;
    o->op = (unsigned char)op;
    o->before = o->after = cgEntCollNodes[localClientNum][entIndex].sector;
    o->eType = (unsigned char)cent->nextState.eType;
    o->number = cent->nextState.number;
    o->caller = caller - (unsigned int)GetModuleHandleA(0);
}



void __cdecl CG_ClearEntityCollWorld(int localClientNum)
{
    unsigned __int16 sectorIndex; // [esp+18h] [ebp-8h]
    CgEntCollWorld *world; // [esp+1Ch] [ebp-4h]

    if ( localClientNum
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_colltree.cpp",
                    72,
                    0,
                    "localClientNum doesn't index MAX_LOCAL_CLIENTS\n\t%i not in [0, %i)",
                    localClientNum,
                    1) )
    {
        __debugbreak();
    }
    memset((unsigned __int8 *)&cgEntCollWorld[localClientNum], 0, sizeof(CgEntCollWorld));
    memset((unsigned __int8 *)cgEntCollNodes[localClientNum], 0, sizeof(cgEntCollNodes_data[0])); // mod (L43): retail 0xA800 = 1536 nodes
    cg_collTreeCheck = _Dvar_RegisterInt("bo1_colltree_check", 0, 0, 2, 0,
        "Verify the client entity collision tree every 64th (1) / every (2) link op; print the ops before the first failure");
    s_collOpCount = s_collOpsSinceCheck = 0;
    s_collTreeReported = false;
    world = &cgEntCollWorld[localClientNum];
    CM_ModelBounds(0, world->mins, world->maxs);
    world->freeHead = 2;
    for ( sectorIndex = 2; sectorIndex < 0x3FFu; ++sectorIndex )
        world->sectors[sectorIndex].tree.u.parent = sectorIndex + 1;
    world->sectors[1].tree.axis = (float)(world->maxs[1] - world->mins[1]) >= (float)(world->maxs[0] - world->mins[0]);
    world->sectors[1].tree.dist = (float)(world->maxs[world->sectors[1].tree.axis]
                                                                            + world->mins[world->sectors[1].tree.axis])
                                                            * 0.5;
}

void __cdecl CG_UnlinkEntityColl(int localClientNum, unsigned int entIndex)
{
    unsigned int v2; // [esp+0h] [ebp-30h]
    unsigned int v3; // [esp+8h] [ebp-28h]
    CgEntCollNode *node; // [esp+14h] [ebp-1Ch]
    CgEntCollSector *sector; // [esp+18h] [ebp-18h]
    CgEntCollNode *scan; // [esp+1Ch] [ebp-14h]
    unsigned __int16 parentSectorIndex; // [esp+20h] [ebp-10h]
    unsigned __int16 parentSectorIndexa; // [esp+20h] [ebp-10h]
    CgEntCollNode *next; // [esp+24h] [ebp-Ch]
    unsigned __int16 sectorIndex; // [esp+28h] [ebp-8h]
    CgEntCollWorld *world; // [esp+2Ch] [ebp-4h]

    CG_CollTreeOp(localClientNum, entIndex, 0, (unsigned int)_ReturnAddress()); // mod (L43)
    if ( localClientNum
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_colltree.cpp",
                    281,
                    0,
                    "localClientNum doesn't index MAX_LOCAL_CLIENTS\n\t%i not in [0, %i)",
                    localClientNum,
                    1) )
    {
        __debugbreak();
    }
    if ( entIndex >= MAX_CL_ENTNUM /* mod (L43): retail 0x600 */
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_colltree.cpp",
                    282,
                    0,
                    "entIndex doesn't index MAX_LOCAL_CENTITIES\n\t%i not in [0, %i)",
                    entIndex,
                    1536) )
    {
        __debugbreak();
    }
    world = &cgEntCollWorld[localClientNum];
    if ( localClientNum
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_colltree.h",
                    108,
                    0,
                    "localClientNum doesn't index MAX_LOCAL_CLIENTS\n\t%i not in [0, %i)",
                    localClientNum,
                    1) )
    {
        __debugbreak();
    }
    if ( entIndex >= MAX_CL_ENTNUM /* mod (L43): retail 0x600 */
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_colltree.h",
                    109,
                    0,
                    "entIndex doesn't index MAX_LOCAL_CENTITIES\n\t%i not in [0, %i)",
                    entIndex,
                    1536) )
    {
        __debugbreak();
    }
    node = &cgEntCollNodes[localClientNum][entIndex];
    sectorIndex = node->sector;
    if ( node->sector )
    {
        sector = &world->sectors[sectorIndex];
        node->sector = 0;
        if ( !world->sectors[sectorIndex].entListHead
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_colltree.cpp",
                        294,
                        0,
                        "%s",
                        "sector->entListHead") )
        {
            __debugbreak();
        }
        if ( world->sectors[sectorIndex].entListHead - 1 == entIndex )
        {
            world->sectors[sectorIndex].entListHead = node->nextEntInSector;
        }
        else
        {
            v3 = world->sectors[sectorIndex].entListHead - 1;
            if ( localClientNum
                && !Assert_MyHandler(
                            "c:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_colltree.h",
                            108,
                            0,
                            "localClientNum doesn't index MAX_LOCAL_CLIENTS\n\t%i not in [0, %i)",
                            localClientNum,
                            1) )
            {
                __debugbreak();
            }
            if ( v3 >= MAX_CL_ENTNUM // mod (L43): retail 0x600
                && !Assert_MyHandler(
                            "c:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_colltree.h",
                            109,
                            0,
                            "entIndex doesn't index MAX_LOCAL_CENTITIES\n\t%i not in [0, %i)",
                            v3,
                            1536) )
            {
                __debugbreak();
            }
            for ( scan = &cgEntCollNodes[localClientNum][v3]; ; scan = next )
            {
                v2 = scan->nextEntInSector - 1;
                if ( localClientNum
                    && !Assert_MyHandler(
                                "c:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_colltree.h",
                                108,
                                0,
                                "localClientNum doesn't index MAX_LOCAL_CLIENTS\n\t%i not in [0, %i)",
                                localClientNum,
                                1) )
                {
                    __debugbreak();
                }
                if ( v2 >= MAX_CL_ENTNUM // mod (L43): retail 0x600
                    && !Assert_MyHandler(
                                "c:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_colltree.h",
                                109,
                                0,
                                "entIndex doesn't index MAX_LOCAL_CENTITIES\n\t%i not in [0, %i)",
                                v2,
                                1536) )
                {
                    __debugbreak();
                }
                next = &cgEntCollNodes[localClientNum][v2];
                if ( next == node )
                    break;
                if ( !scan->nextEntInSector
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_colltree.cpp",
                                312,
                                0,
                                "%s",
                                "scan->nextEntInSector") )
                {
                    __debugbreak();
                }
            }
            scan->nextEntInSector = node->nextEntInSector;
        }
        while ( !sector->entListHead && !sector->tree.child[0] && !sector->tree.child[1] )
        {
            if ( !sector->tree.u.parent )
            {
                if ( sectorIndex != 1
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_colltree.cpp",
                                320,
                                0,
                                "%s",
                                "sectorIndex == CG_ENT_SECTOR_HEAD") )
                {
                    __debugbreak();
                }
                break;
            }
            parentSectorIndex = sector->tree.u.parent;
            sector->tree.u.parent = world->freeHead;
            world->freeHead = sectorIndex;
            sector = &world->sectors[parentSectorIndex];
            if ( world->sectors[parentSectorIndex].tree.child[0] == sectorIndex )
            {
                world->sectors[parentSectorIndex].tree.child[0] = 0;
            }
            else
            {
                if ( world->sectors[parentSectorIndex].tree.child[1] != sectorIndex
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_colltree.cpp",
                                337,
                                0,
                                "%s",
                                "sector->tree.child[1] == sectorIndex") )
                {
                    __debugbreak();
                }
                world->sectors[parentSectorIndex].tree.child[1] = 0;
            }
            sectorIndex = parentSectorIndex;
        }
        while ( 1 )
        {
            parentSectorIndexa = sector->tree.u.parent;
            if ( !parentSectorIndexa )
                break;
            sector = &world->sectors[parentSectorIndexa];
        }
    }
}

void __cdecl CG_LinkEntityColl(int localClientNum, unsigned int entIndex, const float *absMins, const float *absMaxs)
{
    CgEntCollNode *node; // [esp+18h] [ebp-28h]
    CgEntCollSector *sector; // [esp+1Ch] [ebp-24h]
    float dist; // [esp+20h] [ebp-20h]
    float mins[2]; // [esp+24h] [ebp-1Ch] BYREF
    unsigned __int16 sectorIndex; // [esp+2Ch] [ebp-14h]
    float maxs[2]; // [esp+30h] [ebp-10h] BYREF
    CgEntCollWorld *world; // [esp+38h] [ebp-8h]
    int axis; // [esp+3Ch] [ebp-4h]

    CG_CollTreeOp(localClientNum, entIndex, 1, (unsigned int)_ReturnAddress()); // mod (L43)
    if ( localClientNum
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_colltree.cpp",
                    371,
                    0,
                    "localClientNum doesn't index MAX_LOCAL_CLIENTS\n\t%i not in [0, %i)",
                    localClientNum,
                    1) )
    {
        __debugbreak();
    }
    if ( entIndex >= MAX_CL_ENTNUM /* mod (L43): retail 0x600 */
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_colltree.cpp",
                    372,
                    0,
                    "entIndex doesn't index MAX_LOCAL_CENTITIES\n\t%i not in [0, %i)",
                    entIndex,
                    1536) )
    {
        __debugbreak();
    }
    if ( !absMins
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_colltree.cpp", 373, 0, "%s", "absMins") )
    {
        __debugbreak();
    }
    if ( !absMaxs
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_colltree.cpp", 374, 0, "%s", "absMaxs") )
    {
        __debugbreak();
    }
    world = &cgEntCollWorld[localClientNum];
    if ( localClientNum
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_colltree.h",
                    108,
                    0,
                    "localClientNum doesn't index MAX_LOCAL_CLIENTS\n\t%i not in [0, %i)",
                    localClientNum,
                    1) )
    {
        __debugbreak();
    }
    if ( entIndex >= MAX_CL_ENTNUM /* mod (L43): retail 0x600 */
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_colltree.h",
                    109,
                    0,
                    "entIndex doesn't index MAX_LOCAL_CENTITIES\n\t%i not in [0, %i)",
                    entIndex,
                    1536) )
    {
        __debugbreak();
    }
    node = &cgEntCollNodes[localClientNum][entIndex];
    while ( 1 )
    {
        mins[0] = world->mins[0];
        mins[1] = world->mins[1];
        maxs[0] = world->maxs[0];
        maxs[1] = world->maxs[1];
        for ( sectorIndex = 1; ; sectorIndex = sector->tree.child[1] )
        {
            while ( 1 )
            {
                sector = &world->sectors[sectorIndex];
                axis = world->sectors[sectorIndex].tree.axis;
                dist = sector->tree.dist;
                if ( absMins[axis] <= sector->tree.dist )
                    break;
                mins[axis] = dist;
                if ( !sector->tree.child[0] )
                    goto LABEL_31;
                sectorIndex = sector->tree.child[0];
            }
            if ( dist <= absMaxs[axis] )
                break;
            maxs[axis] = dist;
            if ( !sector->tree.child[1] )
                goto LABEL_31;
        }
        if ( sectorIndex == node->sector )
        {
            node->linkMins[0] = *absMins;
            node->linkMins[1] = absMins[1];
            node->linkMins[2] = absMins[2];
            node->linkMaxs[0] = *absMaxs;
            node->linkMaxs[1] = absMaxs[1];
            node->linkMaxs[2] = absMaxs[2];
            return;
        }
LABEL_31:
        if ( !node->sector )
            break;
        if ( sectorIndex == node->sector )
            goto LABEL_36;
        CG_UnlinkEntityColl(localClientNum, entIndex);
    }
    CG_AddEntityToCollSector(localClientNum, entIndex, sectorIndex);
LABEL_36:
    node->linkMins[0] = *absMins;
    node->linkMins[1] = absMins[1];
    node->linkMins[2] = absMins[2];
    node->linkMaxs[0] = *absMaxs;
    node->linkMaxs[1] = absMaxs[1];
    node->linkMaxs[2] = absMaxs[2];
    CG_SortEntityCollSector(localClientNum, sectorIndex, mins, maxs);
}

void __cdecl CG_AddEntityToCollSector(int localClientNum, unsigned int entIndex, unsigned __int16 sectorIndex)
{
    unsigned int v3; // [esp+0h] [ebp-24h]
    CgEntCollNode *node; // [esp+Ch] [ebp-18h]
    unsigned __int16 *prevListIndex; // [esp+18h] [ebp-Ch]

    if ( localClientNum
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_colltree.cpp",
                    106,
                    0,
                    "localClientNum doesn't index MAX_LOCAL_CLIENTS\n\t%i not in [0, %i)",
                    localClientNum,
                    1) )
    {
        __debugbreak();
    }
    if ( entIndex >= MAX_CL_ENTNUM /* mod (L43): retail 0x600 */
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_colltree.cpp",
                    107,
                    0,
                    "entIndex doesn't index MAX_LOCAL_CENTITIES\n\t%i not in [0, %i)",
                    entIndex,
                    1536) )
    {
        __debugbreak();
    }
    if ( !sectorIndex
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_colltree.cpp", 108, 0, "%s", "sectorIndex") )
    {
        __debugbreak();
    }
    if ( sectorIndex >= 0x400u
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_colltree.cpp",
                    109,
                    0,
                    "%s\n\t(sectorIndex) = %i",
                    "(sectorIndex < 1024)",
                    sectorIndex) )
    {
        __debugbreak();
    }
    if ( localClientNum
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_colltree.h",
                    108,
                    0,
                    "localClientNum doesn't index MAX_LOCAL_CLIENTS\n\t%i not in [0, %i)",
                    localClientNum,
                    1) )
    {
        __debugbreak();
    }
    if ( entIndex >= MAX_CL_ENTNUM /* mod (L43): retail 0x600 */
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_colltree.h",
                    109,
                    0,
                    "entIndex doesn't index MAX_LOCAL_CENTITIES\n\t%i not in [0, %i)",
                    entIndex,
                    1536) )
    {
        __debugbreak();
    }
    node = &cgEntCollNodes[localClientNum][entIndex];
    for ( prevListIndex = &cgEntCollWorld[localClientNum].sectors[sectorIndex].entListHead;
                (unsigned int)*prevListIndex - 1 <= entIndex;
                prevListIndex = &cgEntCollNodes[localClientNum][v3].nextEntInSector )
    {
        v3 = *prevListIndex - 1;
        if ( localClientNum
            && !Assert_MyHandler(
                        "c:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_colltree.h",
                        108,
                        0,
                        "localClientNum doesn't index MAX_LOCAL_CLIENTS\n\t%i not in [0, %i)",
                        localClientNum,
                        1) )
        {
            __debugbreak();
        }
        if ( v3 >= MAX_CL_ENTNUM // mod (L43): retail 0x600
            && !Assert_MyHandler(
                        "c:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_colltree.h",
                        109,
                        0,
                        "entIndex doesn't index MAX_LOCAL_CENTITIES\n\t%i not in [0, %i)",
                        v3,
                        1536) )
        {
            __debugbreak();
        }
    }
    node->sector = sectorIndex;
    node->nextEntInSector = *prevListIndex;
    *prevListIndex = entIndex + 1;
}

void __cdecl CG_SortEntityCollSector(
                int localClientNum,
                unsigned __int16 sectorIndex,
                const float *mins,
                const float *maxs)
{
    unsigned int v4; // [esp+0h] [ebp-38h]
    unsigned int v5; // [esp+8h] [ebp-30h]
    CgEntCollNode *node; // [esp+14h] [ebp-24h]
    unsigned __int16 listIndex; // [esp+1Ch] [ebp-1Ch]
    float dist; // [esp+20h] [ebp-18h]
    CgEntCollNode *prevNode; // [esp+24h] [ebp-14h]
    CgEntCollWorld *world; // [esp+28h] [ebp-10h]
    unsigned int entIndex; // [esp+2Ch] [ebp-Ch]
    int axis; // [esp+30h] [ebp-8h]
    unsigned __int16 childSectorIndex; // [esp+34h] [ebp-4h]

    if ( localClientNum
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_colltree.cpp",
                    197,
                    0,
                    "localClientNum doesn't index MAX_LOCAL_CLIENTS\n\t%i not in [0, %i)",
                    localClientNum,
                    1) )
    {
        __debugbreak();
    }
    if ( !sectorIndex
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_colltree.cpp", 198, 0, "%s", "sectorIndex") )
    {
        __debugbreak();
    }
    if ( sectorIndex >= 0x400u
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_colltree.cpp",
                    199,
                    0,
                    "%s\n\t(sectorIndex) = %i",
                    "(sectorIndex < 1024)",
                    sectorIndex) )
    {
        __debugbreak();
    }
    if ( !mins && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_colltree.cpp", 200, 0, "%s", "mins") )
        __debugbreak();
    if ( !maxs && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_colltree.cpp", 201, 0, "%s", "maxs") )
        __debugbreak();
    world = &cgEntCollWorld[localClientNum];
    axis = world->sectors[sectorIndex].tree.axis;
    dist = world->sectors[sectorIndex].tree.dist;
    prevNode = 0;
    listIndex = world->sectors[sectorIndex].entListHead;
    while ( listIndex )
    {
        entIndex = listIndex - 1;
        if ( localClientNum
            && !Assert_MyHandler(
                        "c:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_colltree.h",
                        108,
                        0,
                        "localClientNum doesn't index MAX_LOCAL_CLIENTS\n\t%i not in [0, %i)",
                        localClientNum,
                        1) )
        {
            __debugbreak();
        }
        if ( entIndex >= MAX_CL_ENTNUM /* mod (L43): retail 0x600 */
            && !Assert_MyHandler(
                        "c:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_colltree.h",
                        109,
                        0,
                        "entIndex doesn't index MAX_LOCAL_CENTITIES\n\t%i not in [0, %i)",
                        entIndex,
                        1536) )
        {
            __debugbreak();
        }
        node = &cgEntCollNodes[localClientNum][entIndex];
        if ( node->linkMins[axis] <= dist )
        {
            if ( dist <= node->linkMaxs[axis] )
                goto skipEntity;
            childSectorIndex = cgEntCollWorld[localClientNum].sectors[sectorIndex].tree.child[1];
            if ( !childSectorIndex )
            {
                childSectorIndex = CG_AllocEntityCollSector(localClientNum, mins, maxs);
                if ( !childSectorIndex )
                    goto skipEntity;
                cgEntCollWorld[localClientNum].sectors[sectorIndex].tree.child[1] = childSectorIndex;
                world->sectors[childSectorIndex].tree.u.parent = sectorIndex;
            }
LABEL_35:
            listIndex = node->nextEntInSector;
            if ( !prevNode )
            {
                v5 = cgEntCollWorld[localClientNum].sectors[sectorIndex].entListHead - 1;
                if ( localClientNum
                    && !Assert_MyHandler(
                                "c:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_colltree.h",
                                108,
                                0,
                                "localClientNum doesn't index MAX_LOCAL_CLIENTS\n\t%i not in [0, %i)",
                                localClientNum,
                                1) )
                {
                    __debugbreak();
                }
                if ( v5 >= MAX_CL_ENTNUM // mod (L43): retail 0x600
                    && !Assert_MyHandler(
                                "c:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_colltree.h",
                                109,
                                0,
                                "entIndex doesn't index MAX_LOCAL_CENTITIES\n\t%i not in [0, %i)",
                                v5,
                                1536) )
                {
                    __debugbreak();
                }
                if ( &cgEntCollNodes[localClientNum][v5] != node
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_colltree.cpp",
                                253,
                                0,
                                "%s",
                                "prevNode || (CG_GetCollNode( localClientNum, sector->entListHead - 1 ) == node)") )
                {
                    __debugbreak();
                }
            }
            if ( prevNode )
            {
                v4 = prevNode->nextEntInSector - 1;
                if ( localClientNum
                    && !Assert_MyHandler(
                                "c:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_colltree.h",
                                108,
                                0,
                                "localClientNum doesn't index MAX_LOCAL_CLIENTS\n\t%i not in [0, %i)",
                                localClientNum,
                                1) )
                {
                    __debugbreak();
                }
                if ( v4 >= MAX_CL_ENTNUM // mod (L43): retail 0x600
                    && !Assert_MyHandler(
                                "c:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_colltree.h",
                                109,
                                0,
                                "entIndex doesn't index MAX_LOCAL_CENTITIES\n\t%i not in [0, %i)",
                                v4,
                                1536) )
                {
                    __debugbreak();
                }
                if ( &cgEntCollNodes[localClientNum][v4] != node
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_colltree.cpp",
                                254,
                                0,
                                "%s",
                                "!prevNode || (CG_GetCollNode( localClientNum, prevNode->nextEntInSector - 1) == node)") )
                {
                    __debugbreak();
                }
            }
            CG_AddEntityToCollSector(localClientNum, entIndex, childSectorIndex);
            if ( prevNode )
                prevNode->nextEntInSector = listIndex;
            else
                cgEntCollWorld[localClientNum].sectors[sectorIndex].entListHead = listIndex;
        }
        else
        {
            childSectorIndex = cgEntCollWorld[localClientNum].sectors[sectorIndex].tree.child[0];
            if ( childSectorIndex )
                goto LABEL_35;
            childSectorIndex = CG_AllocEntityCollSector(localClientNum, mins, maxs);
            if ( childSectorIndex )
            {
                cgEntCollWorld[localClientNum].sectors[sectorIndex].tree.child[0] = childSectorIndex;
                world->sectors[childSectorIndex].tree.u.parent = sectorIndex;
                goto LABEL_35;
            }
skipEntity:
            prevNode = node;
            listIndex = node->nextEntInSector;
        }
    }
}

unsigned __int16 __cdecl CG_AllocEntityCollSector(int localClientNum, const float *mins, const float *maxs)
{
    CgEntCollSector *sector; // [esp+8h] [ebp-18h]
    float size[2]; // [esp+Ch] [ebp-14h]
    unsigned __int16 sectorIndex; // [esp+14h] [ebp-Ch]
    CgEntCollWorld *world; // [esp+18h] [ebp-8h]
    unsigned __int16 axis; // [esp+1Ch] [ebp-4h]

    if ( localClientNum
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_colltree.cpp",
                    148,
                    0,
                    "localClientNum doesn't index MAX_LOCAL_CLIENTS\n\t%i not in [0, %i)",
                    localClientNum,
                    1) )
    {
        __debugbreak();
    }
    if ( !mins && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_colltree.cpp", 149, 0, "%s", "mins") )
        __debugbreak();
    if ( !maxs && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_colltree.cpp", 150, 0, "%s", "maxs") )
        __debugbreak();
    world = &cgEntCollWorld[localClientNum];
    sectorIndex = world->freeHead;
    if ( !sectorIndex )
        return 0;
    size[0] = *maxs - *mins;
    size[1] = maxs[1] - mins[1];
    axis = size[1] >= size[0];
    if ( size[size[1] >= size[0]] <= 512.0 )
        return 0;
    sector = &world->sectors[sectorIndex];
    if ( world->sectors[sectorIndex].entListHead
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_colltree.cpp",
                    166,
                    0,
                    "%s",
                    "!sector->entListHead") )
    {
        __debugbreak();
    }
    world->freeHead = sector->tree.u.parent;
    sector->tree.axis = axis;
    sector->tree.dist = (float)(maxs[axis] + mins[axis]) * 0.5;
    if ( sector->tree.child[0]
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_colltree.cpp",
                    173,
                    0,
                    "%s",
                    "!sector->tree.child[0]") )
    {
        __debugbreak();
    }
    if ( sector->tree.child[1] )
    {
        if ( !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_colltree.cpp",
                        174,
                        0,
                        "%s",
                        "!sector->tree.child[1]") )
            __debugbreak();
    }
    return sectorIndex;
}

