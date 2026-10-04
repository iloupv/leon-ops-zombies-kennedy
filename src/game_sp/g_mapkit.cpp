#include <qcommon/cm_mapkit.h>
#include <qcommon/cm_load.h>
#include <qcommon/common.h>
#include <game/g_bsp.h>
#include <game/pathnode.h>
#include <game_mp/g_main_mp.h>
#include <clientscript/cscr_vm.h>
#include <universal/com_files.h>
#include <universal/dvar.h>
#include <universal/com_memory.h>
#include <database/db_assetnames.h>
#include <bgame/bg_weapons.h>
#include <xanim/xmodel_utils.h>

#include <stdio.h>
#include <string.h>
#include <string>

static int s_checkPass, s_checkFail;

void G_Mapkit_ResetChecks()
{
    s_checkPass = s_checkFail = 0;
}

void G_Mapkit_RecordCheck(const char *message)
{
    if (!CM_Mapkit_Active())
        return;
    if (!strncmp(message, "mapkit: check ", 14))
    {
        if (strstr(message, " PASS "))
            ++s_checkPass;
        if (strstr(message, " FAIL "))
            ++s_checkFail;
        Com_Printf(18, "%s", message);
    }
    else if (!strncmp(message, "mapkit_selftest ", 16) || !strncmp(message, "mapkit_doctor ", 14))
        Com_Printf(18, "%s", message);
}

void G_Mapkit_f_selftestdone()
{
    G_LogPrintf("mapkit_selftest DONE pass %d fail %d\n", s_checkPass, s_checkFail);
}

static bool G_Mapkit_InBox(const float *p, const float *lo, const float *hi)
{
    return p[0] >= lo[0] && p[0] <= hi[0] && p[1] >= lo[1] && p[1] <= hi[1] && p[2] >= lo[2] && p[2] <= hi[2];
}

static bool G_Mapkit_InLoopArea(const float *p, const float lo[2][3], const float hi[2][3], const float *blo, const float *bhi)
{
    return G_Mapkit_InBox(p, lo[0], hi[0]) || G_Mapkit_InBox(p, lo[1], hi[1]) || G_Mapkit_InBox(p, blo, bhi);
}

// Test active links, not totalLinkCount (which includes links disabled by closed doors).
// Optional 3rd arg "loop": the blocker closes a loop (its `to` zone is already reachable another way), so the
// search is limited to nodes inside the from/to zones or within 96 units of the blocker bounds - the path
// THROUGH this doorway must be disconnected before and connected after the purchase.
void G_Mapkit_f_graphcheck()
{
    const int edit = Scr_GetInt(0, SCRIPTINSTANCE_SERVER);
    const bool expected = Scr_GetInt(1, SCRIPTINSTANCE_SERVER) != 0;
    const bool loop = Scr_GetNumParam(SCRIPTINSTANCE_SERVER) > 2 && Scr_GetInt(2, SCRIPTINSTANCE_SERVER) != 0;
    const char *from = "", *to = "", *name = "";
    float v[3], lo[2][3], hi[2][3], blo[3] = {}, bhi[3] = {};
    if (loop && (CM_Mapkit_GetField(edit, "mins", 0, blo) != MKF_VECTOR || CM_Mapkit_GetField(edit, "maxs", 0, bhi) != MKF_VECTOR))
        blo[0] = bhi[0] = blo[1] = bhi[1] = blo[2] = bhi[2] = 0.0f;
    for (int k = 0; loop && k < 3; ++k)
    {
        blo[k] -= 96.0f;
        bhi[k] += 96.0f;
    }
    CM_Mapkit_GetField(edit, "from", &from, v);
    CM_Mapkit_GetField(edit, "to", &to, v);
    CM_Mapkit_GetField(edit, "name", &name, v);
    bool zones[2] = { false, false };
    for (int i = 0; i < CM_Mapkit_EditCount(); ++i)
    {
        const char *kind = "", *zone = "";
        CM_Mapkit_GetField(i, "kind", &kind, v);
        CM_Mapkit_GetField(i, "name", &zone, v);
        if (strcmp(kind, "zone"))
            continue;
        for (int k = 0; k < 2; ++k)
            if (!strcmp(zone, k ? to : from))
                zones[k] = CM_Mapkit_GetField(i, "mins", 0, lo[k]) == MKF_VECTOR
                    && CM_Mapkit_GetField(i, "maxs", 0, hi[k]) == MKF_VECTOR;
    }
    PathData *path = gameWorldCurrent ? &gameWorldCurrent->path : 0;
    bool seen[0x2000] = {};
    unsigned short queue[0x2000];
    int head = 0, tail = 0, destinations = 0;
    bool connected = false;
    const bool valid = path && path->nodeCount <= 0x2000 && zones[0] && zones[1];
    if (valid)
    {
        for (unsigned int i = 0; i < path->nodeCount; ++i)
        {
            const pathnode_t *node = &path->nodes[i];
            if (node->constant.type == NODE_BADNODE)
                continue;
            for (int k = 0; k < 2; ++k)
            {
                const float *p = node->constant.vOrigin;
                if (p[0] >= lo[k][0] && p[0] <= hi[k][0] && p[1] >= lo[k][1] && p[1] <= hi[k][1]
                    && p[2] >= lo[k][2] && p[2] <= hi[k][2])
                {
                    if (k)
                        ++destinations;
                    else
                    {
                        seen[i] = true;
                        queue[tail++] = (unsigned short)i;
                    }
                }
            }
        }
        while (head < tail && !connected)
        {
            const pathnode_t *node = &path->nodes[queue[head++]];
            const float *p = node->constant.vOrigin;
            connected = p[0] >= lo[1][0] && p[0] <= hi[1][0] && p[1] >= lo[1][1] && p[1] <= hi[1][1]
                && p[2] >= lo[1][2] && p[2] <= hi[1][2];
            for (int l = 0; l < node->dynamic.wLinkCount; ++l)
            {
                const unsigned int next = node->constant.Links[l].nodeNum;
                if (next < path->nodeCount && !seen[next] && path->nodes[next].constant.type != NODE_BADNODE
                    && (!loop || G_Mapkit_InLoopArea(path->nodes[next].constant.vOrigin, lo, hi, blo, bhi)))
                {
                    seen[next] = true;
                    queue[tail++] = (unsigned short)next;
                }
            }
        }
    }
    G_LogPrintf("mapkit: check door_graph %s %s edit %d %s purchase: active graph %s %s -> %s%s\n", name,
        valid && tail && destinations && connected == expected ? "PASS" : "FAIL", edit,
        expected ? "after" : "before", connected ? "connected" : "disconnected", from, to,
        loop ? " (loop-closer: path through this doorway only)" : "");
}

// mod: mapkit GSC builtins (the runtime is mods/mapkit/maps/mapkit/_mapkit.gsc):
//   bo1_mapkit_editcount()                  number of edits in the loaded layout (0 = mapkit off)
//   bo1_mapkit_get(edit, key)               a field of an edit: string / float / vector, undefined if absent
//                                             (key may be dotted: "gaps.0.side"; "#gaps" = array length)
//   bo1_mapkit_layout()                     the loaded layout's name ("" = none)
//   bo1_mapkit_mark(kind, name, origin, r)  record a placement / zone / probe for the validator dump
//   bo1_mapkit_dump(file)                   write <fs_game>/<file>: boxes, marks, retail brushes and pathnodes
//                                             around the edits (tools/mapkit/topdown.mjs draws it)

namespace
{
    struct MapkitMark
    {
        char kind[24];
        char name[64];
        float origin[3];
        float radius;
    };
    const int MK_MAX_MARKS = 512;
    MapkitMark s_marks[MK_MAX_MARKS];
    int s_markCount;
    char s_markLayout[64];

    void Mapkit_ResetMarksIfNewLayout()
    {
        if (strcmp(s_markLayout, CM_Mapkit_LayoutName()))
        {
            s_markCount = 0;
            strncpy(s_markLayout, CM_Mapkit_LayoutName(), sizeof(s_markLayout) - 1);
        }
    }

    void Mapkit_AppendVec(std::string &out, const float *v)
    {
        char buf[96];
        sprintf_s(buf, "[%.1f,%.1f,%.1f]", v[0], v[1], v[2]);
        out += buf;
    }

    bool Mapkit_Overlaps(const float *mins, const float *maxs, const float *lo, const float *hi)
    {
        for (int i = 0; i < 3; ++i)
        {
            if (mins[i] > hi[i] || maxs[i] < lo[i])
                return false;
        }
        return true;
    }
}

void G_Mapkit_f_editcount()
{
    Scr_AddInt(CM_Mapkit_EditCount(), SCRIPTINSTANCE_SERVER);
}

void G_Mapkit_f_layout()
{
    Scr_AddString(CM_Mapkit_LayoutName(), SCRIPTINSTANCE_SERVER);
}

void G_Mapkit_f_get()
{
    const int edit = Scr_GetInt(0, SCRIPTINSTANCE_SERVER);
    const char *key = Scr_GetString(1, SCRIPTINSTANCE_SERVER);
    const char *str = 0;
    float vec[3] = { 0.0f, 0.0f, 0.0f };
    switch (CM_Mapkit_GetField(edit, key, &str, vec))
    {
    case MKF_STRING:
        Scr_AddString(str, SCRIPTINSTANCE_SERVER);
        break;
    case MKF_NUMBER:
        Scr_AddFloat(vec[0], SCRIPTINSTANCE_SERVER);
        break;
    case MKF_VECTOR:
        Scr_AddVector(vec, SCRIPTINSTANCE_SERVER);
        break;
    default:
        Scr_AddUndefined(SCRIPTINSTANCE_SERVER);
        break;
    }
}

void G_Mapkit_f_light()
{
    // mapkit-fix-3: the mood-light tint the renderer applies at a point (1 1 1 = the flat grey), undefined outside lights
    float p[3], rgb[3];
    Scr_GetVector(0, p, SCRIPTINSTANCE_SERVER);
    if (CM_Mapkit_LightAt(p, rgb) >= 0)
        Scr_AddVector(rgb, SCRIPTINSTANCE_SERVER);
    else
        Scr_AddUndefined(SCRIPTINSTANCE_SERVER);
}

void G_Mapkit_f_lightuses()
{
    Scr_AddInt(CM_Mapkit_LightUses(Scr_GetInt(0, SCRIPTINSTANCE_SERVER)), SCRIPTINSTANCE_SERVER);
}

void G_Mapkit_f_mark()
{
    Mapkit_ResetMarksIfNewLayout();
    if (s_markCount >= MK_MAX_MARKS)
        return;
    MapkitMark *m = &s_marks[s_markCount++];
    memset(m, 0, sizeof(*m));
    strncpy(m->kind, Scr_GetString(0, SCRIPTINSTANCE_SERVER), sizeof(m->kind) - 1);
    strncpy(m->name, Scr_GetString(1, SCRIPTINSTANCE_SERVER), sizeof(m->name) - 1);
    Scr_GetVector(2, m->origin, SCRIPTINSTANCE_SERVER);
    m->radius = Scr_GetNumParam(SCRIPTINSTANCE_SERVER) > 3 ? (float)Scr_GetFloat(3, SCRIPTINSTANCE_SERVER) : 0.0f;
}

void G_Mapkit_f_visible()
{
    CM_Mapkit_SetVisible(Scr_GetInt(0, SCRIPTINSTANCE_SERVER), Scr_GetInt(1, SCRIPTINSTANCE_SERVER) != 0);
}

void G_Mapkit_f_dump()
{
    Mapkit_ResetMarksIfNewLayout();
    const char *file = Scr_GetString(0, SCRIPTINSTANCE_SERVER);
    float lo[3] = { 1e9f, 1e9f, 1e9f }, hi[3] = { -1e9f, -1e9f, -1e9f };
    for (int i = 0; i < CM_Mapkit_BoxCount(); ++i)
    {
        const MapkitBox *b = CM_Mapkit_GetBox(i);
        for (int k = 0; k < 3; ++k)
        {
            lo[k] = b->mins[k] < lo[k] ? b->mins[k] : lo[k];
            hi[k] = b->maxs[k] > hi[k] ? b->maxs[k] : hi[k];
        }
    }
    for (int i = 0; i < s_markCount; ++i)
    {
        for (int k = 0; k < 3; ++k)
        {
            lo[k] = s_marks[i].origin[k] < lo[k] ? s_marks[i].origin[k] : lo[k];
            hi[k] = s_marks[i].origin[k] > hi[k] ? s_marks[i].origin[k] : hi[k];
        }
    }
    if (lo[0] > hi[0])
    {
        Com_PrintWarning(24, "mapkit: nothing to dump\n");
        return;
    }
    for (int k = 0; k < 3; ++k)
    {
        lo[k] -= k == 2 ? 64.0f : 256.0f;
        hi[k] += k == 2 ? 64.0f : 256.0f;
    }

    std::string out;
    out.reserve(256 * 1024);
    out += "{\"format\":\"mapkit-dump-v1\",\"layout\":\"";
    out += CM_Mapkit_LayoutName();
    out += CM_Mapkit_Empty() ? "\",\"empty\":true,\"bounds\":[" : "\",\"bounds\":[";
    Mapkit_AppendVec(out, lo);
    out += ",";
    Mapkit_AppendVec(out, hi);
    out += "],\n\"boxes\":[";
    char buf[256];
    for (int i = 0; i < CM_Mapkit_BoxCount(); ++i)
    {
        const MapkitBox *b = CM_Mapkit_GetBox(i);
        out += i ? ",\n{\"mins\":" : "\n{\"mins\":";
        Mapkit_AppendVec(out, b->mins);
        out += ",\"maxs\":";
        Mapkit_AppendVec(out, b->maxs);
        sprintf_s(buf, ",\"contents\":%d,\"edit\":%d,\"material\":\"%s\"}", b->contents, b->editIndex, b->material);
        out += buf;
    }
    out += "],\n\"marks\":[";
    for (int i = 0; i < s_markCount; ++i)
    {
        const MapkitMark *m = &s_marks[i];
        sprintf_s(buf, "%s\n{\"kind\":\"%s\",\"name\":\"%s\",\"radius\":%.1f,\"origin\":", i ? "," : "", m->kind, m->name, m->radius);
        out += buf;
        Mapkit_AppendVec(out, m->origin);
        out += "}";
    }
    out += "],\n\"brushes\":[";
    int n = 0;
    // empty base: the retail brushes are still in cm but none of them collide (the world is off): not drawn
    const unsigned int retailBrushes = CM_Mapkit_Empty() ? 0u : cm.numBrushes;
    for (unsigned int i = 0; i < retailBrushes; ++i)
    {
        const cbrush_t *b = &cm.brushes[i];
        if (!b->contents || !Mapkit_Overlaps(b->mins, b->maxs, lo, hi))
            continue;
        out += n++ ? ",\n{\"mins\":" : "\n{\"mins\":";
        Mapkit_AppendVec(out, b->mins);
        out += ",\"maxs\":";
        Mapkit_AppendVec(out, b->maxs);
        sprintf_s(buf, ",\"contents\":%d}", b->contents);
        out += buf;
    }
    out += "],\n\"nodes\":[";
    n = 0;
    const PathData *path = gameWorldCurrent ? &gameWorldCurrent->path : 0;
    for (unsigned int i = 0; path && i < path->nodeCount; ++i)
    {
        const pathnode_t *node = &path->nodes[i];
        const float *o = node->constant.vOrigin;
        if (o[0] < lo[0] || o[0] > hi[0] || o[1] < lo[1] || o[1] > hi[1] || o[2] < lo[2] || o[2] > hi[2])
            continue;
        const bool added = G_Mapkit_FirstAddedNode() >= 0 && i >= (unsigned int)G_Mapkit_FirstAddedNode();
        sprintf_s(buf, "%s\n{\"i\":%u,\"type\":%d,%s\"origin\":", n++ ? "," : "", i, (int)node->constant.type,
            added ? "\"added\":1," : "");
        out += buf;
        Mapkit_AppendVec(out, o);
        out += ",\"links\":[";
        for (unsigned int l = 0; l < node->dynamic.wLinkCount; ++l)
        {
            sprintf_s(buf, "%s%u", l ? "," : "", (unsigned int)node->constant.Links[l].nodeNum);
            out += buf;
        }
        out += "],\"traverses\":[";
        bool firstTraverse = true;
        for (unsigned int l = 0; l < node->dynamic.wLinkCount; ++l)
        {
            const pathlink_s &link = node->constant.Links[l];
            if (!link.negotiationLink)
                continue;
            sprintf_s(buf, "%s%u", firstTraverse ? "" : ",", (unsigned int)link.nodeNum);
            out += buf;
            firstTraverse = false;
        }
        out += "]}";
    }
    out += "]}\n";
    // FS_WriteFile asserts Sys_IsMainThread and the server runs on its own thread: plain stdio into fs_homepath/fs_game
    const dvar_s *home = Dvar_FindVar("fs_homepath");
    const dvar_s *game = Dvar_FindVar("fs_game");
    char osPath[512];
    sprintf_s(osPath, "%s/%s/%s", home && home->current.string ? home->current.string : ".",
        game && game->current.string && *game->current.string ? game->current.string : "main", file);
    FILE *f = 0;
    if (fopen_s(&f, osPath, "wb") || !f)
    {
        Com_PrintWarning(24, "mapkit: cannot write %s\n", osPath);
        return;
    }
    fwrite(out.c_str(), 1, out.size(), f);
    fclose(f);
    Com_Printf(24, "mapkit: dump %s: %d boxes, %d marks, %d retail nodes\n", file, CM_Mapkit_BoxCount(), s_markCount, n);
}

// zombies: mapkit asset catalog (tools/mapkit/catalog.mjs): prints every loaded weapon with its world model,
// every xmodel and every material, one "mapkit_catalog <type> ..." console line each, then an END line.
static int s_catalogCount[3];
static void Mapkit_CatalogWeapon(XAssetHeader header, void *)
{
    const WeaponVariantDef *v = header.weapon;
    const XModel *world = v && v->weapDef && v->weapDef->worldModel ? v->weapDef->worldModel[0] : NULL;
    Com_Printf(0, "mapkit_catalog weapon %s %s\n", v && v->szInternalName ? v->szInternalName : "-", world ? XModelGetName(world) : "-");
    ++s_catalogCount[0];
}
static void Mapkit_CatalogName(XAssetHeader header, void *data)
{
    const int type = *(const int *)data;
    Com_Printf(0, "mapkit_catalog %s %s\n", type == ASSET_TYPE_XMODEL ? "xmodel" : "material", DB_GetXAssetHeaderName(type, &header));
    ++s_catalogCount[type == ASSET_TYPE_XMODEL ? 1 : 2];
}
void G_Mapkit_f_catalog()
{
    s_catalogCount[0] = s_catalogCount[1] = s_catalogCount[2] = 0;
    DB_EnumXAssets(ASSET_TYPE_WEAPON, Mapkit_CatalogWeapon, NULL, false);
    int type = ASSET_TYPE_XMODEL;
    DB_EnumXAssets(ASSET_TYPE_XMODEL, Mapkit_CatalogName, &type, false);
    type = ASSET_TYPE_MATERIAL;
    DB_EnumXAssets(ASSET_TYPE_MATERIAL, Mapkit_CatalogName, &type, false);
    Com_Printf(0, "mapkit_catalog END weapons %d xmodels %d materials %d\n", s_catalogCount[0], s_catalogCount[1], s_catalogCount[2]);
}
