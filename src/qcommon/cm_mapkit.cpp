#include "cm_mapkit.h"
#include "cm_trace.h"
#include "cm_load.h"
#include "common.h"
#include <universal/com_files.h>
#include <universal/dvar.h>

#include <stdlib.h>
#include <string.h>
#include <math.h>

// mod: mapkit layout loader + collision for the layout's boxes (see cm_mapkit.h).

namespace
{
    enum { MKJ_NULL, MKJ_BOOL, MKJ_NUM, MKJ_STR, MKJ_ARR, MKJ_OBJ };

    struct MkJson
    {
        int type;
        double num;
        const char *str;
        const char *key;
        int first;
        int next;
        int count;
    };

    const int MKJ_MAX_NODES = 8192;
    const int MKJ_STR_BYTES = 128 * 1024;
    const int MK_MAX_BOXES = 512;

    MkJson s_json[MKJ_MAX_NODES];
    int s_jsonCount;
    char s_strBuf[MKJ_STR_BYTES];
    int s_strUsed;
    const char *s_parseError;

    MapkitBox s_boxes[MK_MAX_BOXES];
    cbrush_t s_brushes[MK_MAX_BOXES];
    int s_boxCount;
    int s_editsNode = -1;

    // mapkit-fix-4g: every room wall gap (world space), for the kit window frames (Mapkit_AddWindowFrames)
    struct MkGap { int axis; float slab[2]; float along[2]; float z[2]; };
    enum { MK_MAX_GAPS = 128 };
    MkGap s_gaps[MK_MAX_GAPS];
    int s_gapCount;
    char s_layoutName[64];

    // ---- tiny JSON reader (objects, arrays, strings, numbers, true/false/null) ----
    void MkJ_SkipWs(const char *&p)
    {
        for (;;)
        {
            while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n')
                ++p;
            if (p[0] == '/' && p[1] == '/')
            {
                while (*p && *p != '\n')
                    ++p;
                continue;
            }
            return;
        }
    }

    int MkJ_NewNode(int type)
    {
        if (s_jsonCount >= MKJ_MAX_NODES)
        {
            s_parseError = "too many values";
            return -1;
        }
        MkJson *n = &s_json[s_jsonCount];
        memset(n, 0, sizeof(*n));
        n->type = type;
        n->first = -1;
        n->next = -1;
        return s_jsonCount++;
    }

    const char *MkJ_ParseString(const char *&p)
    {
        ++p; // opening quote
        char *out = &s_strBuf[s_strUsed];
        while (*p && *p != '"')
        {
            char c = *p++;
            if (c == '\\' && *p)
            {
                c = *p++;
                if (c == 'n')
                    c = '\n';
                else if (c == 't')
                    c = '\t';
            }
            if (s_strUsed >= MKJ_STR_BYTES - 1)
            {
                s_parseError = "strings too long";
                return "";
            }
            s_strBuf[s_strUsed++] = c;
        }
        if (*p != '"')
        {
            s_parseError = "unterminated string";
            return "";
        }
        ++p;
        s_strBuf[s_strUsed++] = 0;
        return out;
    }

    int MkJ_ParseValue(const char *&p)
    {
        MkJ_SkipWs(p);
        if (s_parseError)
            return -1;
        if (*p == '{' || *p == '[')
        {
            const bool isObj = *p == '{';
            const char close = isObj ? '}' : ']';
            int node = MkJ_NewNode(isObj ? MKJ_OBJ : MKJ_ARR);
            if (node < 0)
                return -1;
            ++p;
            int last = -1;
            MkJ_SkipWs(p);
            if (*p == close)
            {
                ++p;
                return node;
            }
            for (;;)
            {
                const char *key = 0;
                MkJ_SkipWs(p);
                if (isObj)
                {
                    if (*p != '"')
                    {
                        s_parseError = "expected a key";
                        return -1;
                    }
                    key = MkJ_ParseString(p);
                    MkJ_SkipWs(p);
                    if (*p != ':')
                    {
                        s_parseError = "expected ':'";
                        return -1;
                    }
                    ++p;
                }
                int child = MkJ_ParseValue(p);
                if (child < 0)
                    return -1;
                s_json[child].key = key;
                if (last < 0)
                    s_json[node].first = child;
                else
                    s_json[last].next = child;
                last = child;
                ++s_json[node].count;
                MkJ_SkipWs(p);
                if (*p == ',')
                {
                    ++p;
                    continue;
                }
                if (*p == close)
                {
                    ++p;
                    return node;
                }
                s_parseError = isObj ? "expected ',' or '}'" : "expected ',' or ']'";
                return -1;
            }
        }
        if (*p == '"')
        {
            int node = MkJ_NewNode(MKJ_STR);
            if (node >= 0)
                s_json[node].str = MkJ_ParseString(p);
            return node;
        }
        if (!strncmp(p, "true", 4) || !strncmp(p, "false", 5))
        {
            int node = MkJ_NewNode(MKJ_BOOL);
            if (node >= 0)
                s_json[node].num = *p == 't' ? 1.0 : 0.0;
            p += *p == 't' ? 4 : 5;
            return node;
        }
        if (!strncmp(p, "null", 4))
        {
            p += 4;
            return MkJ_NewNode(MKJ_NULL);
        }
        char *end = 0;
        double v = strtod(p, &end);
        if (end == p)
        {
            s_parseError = "unexpected character";
            return -1;
        }
        p = end;
        int node = MkJ_NewNode(MKJ_NUM);
        if (node >= 0)
            s_json[node].num = v;
        return node;
    }

    int MkJ_Child(int node, const char *key)
    {
        if (node < 0)
            return -1;
        const MkJson *n = &s_json[node];
        if (n->type == MKJ_ARR)
        {
            char *end = 0;
            long idx = strtol(key, &end, 10);
            if (end == key)
                return -1;
            int c = n->first;
            for (; c >= 0 && idx > 0; --idx)
                c = s_json[c].next;
            return c;
        }
        if (n->type != MKJ_OBJ)
            return -1;
        for (int c = n->first; c >= 0; c = s_json[c].next)
        {
            if (s_json[c].key && !strcmp(s_json[c].key, key))
                return c;
        }
        return -1;
    }

    // dotted path lookup: "gaps.0.side"
    int MkJ_Path(int node, const char *path)
    {
        char part[64];
        while (node >= 0 && *path)
        {
            int len = 0;
            while (path[len] && path[len] != '.' && len < 63)
            {
                part[len] = path[len];
                ++len;
            }
            part[len] = 0;
            node = MkJ_Child(node, part);
            path += len;
            if (*path == '.')
                ++path;
        }
        return node;
    }

    bool MkJ_Vec3(int node, float *out)
    {
        if (node < 0 || s_json[node].type != MKJ_ARR || s_json[node].count != 3)
            return false;
        int c = s_json[node].first;
        for (int i = 0; i < 3; ++i, c = s_json[c].next)
        {
            if (s_json[c].type != MKJ_NUM)
                return false;
            out[i] = (float)s_json[c].num;
        }
        return true;
    }

    const char *MkJ_Str(int node, const char *key, const char *def)
    {
        int c = MkJ_Child(node, key);
        return c >= 0 && s_json[c].type == MKJ_STR ? s_json[c].str : def;
    }

    float MkJ_Num(int node, const char *key, float def)
    {
        int c = MkJ_Child(node, key);
        return c >= 0 && (s_json[c].type == MKJ_NUM || s_json[c].type == MKJ_BOOL) ? (float)s_json[c].num : def;
    }

    int Mapkit_Contents(const char *name)
    {
        if (!strcmp(name, "none"))
            return 0;
        return 1; // CONTENTS_SOLID
    }

    void Mapkit_AddBox(const float *mins, const float *maxs, const char *material, int contents, float texScale, int edit)
    {
        if (s_boxCount >= MK_MAX_BOXES)
        {
            Com_PrintWarning(1, "mapkit: more than %d boxes, the rest are dropped\n", MK_MAX_BOXES);
            return;
        }
        if (mins[0] >= maxs[0] || mins[1] >= maxs[1] || mins[2] >= maxs[2])
            return;
        MapkitBox *box = &s_boxes[s_boxCount];
        memset(box, 0, sizeof(*box));
        for (int i = 0; i < 3; ++i)
        {
            box->mins[i] = mins[i];
            box->maxs[i] = maxs[i];
        }
        box->contents = contents;
        box->editIndex = edit;
        box->texScale = texScale > 1.0f ? texScale : 128.0f;
        for (int i = 0; i < 3; ++i)
            box->lightOrigin[i] = (mins[i] + maxs[i]) * 0.5f;
        strncpy(box->material, material, sizeof(box->material) - 1);

        // an axial box brush: the same fill as CM_TempBoxModel (no sides, no verts)
        cbrush_t *brush = &s_brushes[s_boxCount];
        memset(brush, 0, sizeof(*brush));
        for (int i = 0; i < 3; ++i)
        {
            brush->mins[i] = mins[i];
            brush->maxs[i] = maxs[i];
            brush->axial_cflags[0][i] = contents;
            brush->axial_cflags[1][i] = contents;
        }
        brush->contents = contents;
        ++s_boxCount;
    }

    // a room: inner mins/maxs, walls of "wall" thickness around it, "open" sides left out, "gaps" cut in walls
    void Mapkit_AddRoom(int edit, int node)
    {
        float mins[3], maxs[3];
        if (!MkJ_Vec3(MkJ_Child(node, "mins"), mins) || !MkJ_Vec3(MkJ_Child(node, "maxs"), maxs))
        {
            Com_PrintWarning(1, "mapkit: room edit %d needs mins and maxs\n", edit);
            return;
        }
        const float t = MkJ_Num(node, "wall", 8.0f);
        const float texScale = MkJ_Num(node, "texScale", 128.0f);
        const char *wallMat = MkJ_Str(node, "wallMaterial", "mc/pent_art_wall_wood07_dark");
        const char *floorMat = MkJ_Str(node, "floorMaterial", wallMat);
        const char *ceilMat = MkJ_Str(node, "ceilingMaterial", wallMat);
        const int contents = Mapkit_Contents(MkJ_Str(node, "contents", "solid"));
        const int openNode = MkJ_Child(node, "open");
        const int firstBox = s_boxCount;
        const int gapsNode = MkJ_Child(node, "gaps");

        float lo[3], hi[3];
        lo[0] = mins[0] - t; lo[1] = mins[1] - t; lo[2] = mins[2] - t;
        hi[0] = maxs[0] + t; hi[1] = maxs[1] + t; hi[2] = mins[2];
        if (MkJ_Num(node, "floor", 1.0f) != 0.0f)
            Mapkit_AddBox(lo, hi, floorMat, contents, texScale, edit);
        lo[2] = maxs[2];
        hi[2] = maxs[2] + t;
        if (MkJ_Num(node, "ceiling", 1.0f) != 0.0f)
            Mapkit_AddBox(lo, hi, ceilMat, contents, texScale, edit);

        static const char *sides[4] = { "-x", "+x", "-y", "+y" };
        bool openSide[4] = { false, false, false, false };
        for (int s = 0; s < 4; ++s)
        {
            for (int c = openNode >= 0 ? s_json[openNode].first : -1; c >= 0; c = s_json[c].next)
            {
                if (s_json[c].type == MKJ_STR && !strcmp(s_json[c].str, sides[s]))
                    openSide[s] = true;
            }
        }
        for (int s = 0; s < 4; ++s)
        {
            if (openSide[s])
                continue;
            const int axis = s < 2 ? 0 : 1;   // the wall's normal axis
            const int along = axis ^ 1;       // the axis the wall runs along
            float wlo[3], whi[3];
            wlo[axis] = (s & 1) ? maxs[axis] : mins[axis] - t;
            whi[axis] = (s & 1) ? maxs[axis] + t : mins[axis];
            // mapkit-fix-3c: the corner extension only where the neighbouring side has a wall to meet. At an open side
            // it ran t units into the neighbour's shared wall (MAPKIT.md shared-wall rule), and its end face, coplanar
            // with that wall's inner face, z-fought: the thin red strip on mood_example's window wall (3b handoff).
            wlo[along] = mins[along] - (openSide[along * 2] ? 0.0f : t);
            whi[along] = maxs[along] + (openSide[along * 2 + 1] ? 0.0f : t);
            wlo[2] = mins[2];
            whi[2] = maxs[2];

            // gaps on this side, sorted by their start along the wall
            float gapA[8][2], gapZ[8][2];
            int gapCount = 0;
            for (int g = gapsNode >= 0 ? s_json[gapsNode].first : -1; g >= 0 && gapCount < 8; g = s_json[g].next)
            {
                if (strcmp(MkJ_Str(g, "side", ""), sides[s]))
                    continue;
                int an = MkJ_Child(g, "along"), zn = MkJ_Child(g, "z");
                if (an < 0 || zn < 0 || s_json[an].count != 2 || s_json[zn].count != 2)
                    continue;
                gapA[gapCount][0] = (float)s_json[s_json[an].first].num;
                gapA[gapCount][1] = (float)s_json[s_json[s_json[an].first].next].num;
                gapZ[gapCount][0] = (float)s_json[s_json[zn].first].num;
                gapZ[gapCount][1] = (float)s_json[s_json[s_json[zn].first].next].num;
                for (int k = gapCount; k > 0 && gapA[k][0] < gapA[k - 1][0]; --k)
                {
                    float a0 = gapA[k][0], a1 = gapA[k][1], z0 = gapZ[k][0], z1 = gapZ[k][1];
                    gapA[k][0] = gapA[k - 1][0]; gapA[k][1] = gapA[k - 1][1];
                    gapZ[k][0] = gapZ[k - 1][0]; gapZ[k][1] = gapZ[k - 1][1];
                    gapA[k - 1][0] = a0; gapA[k - 1][1] = a1; gapZ[k - 1][0] = z0; gapZ[k - 1][1] = z1;
                }
                ++gapCount;
            }
            float cursor = wlo[along];
            for (int g = 0; g < gapCount; ++g)
            {
                if (s_gapCount < MK_MAX_GAPS)
                {
                    MkGap &gp = s_gaps[s_gapCount++];
                    gp.axis = axis;
                    gp.slab[0] = wlo[axis], gp.slab[1] = whi[axis];
                    gp.along[0] = gapA[g][0], gp.along[1] = gapA[g][1];
                    gp.z[0] = gapZ[g][0], gp.z[1] = gapZ[g][1];
                }
                float plo[3] = { wlo[0], wlo[1], wlo[2] }, phi[3] = { whi[0], whi[1], whi[2] };
                plo[along] = cursor;
                phi[along] = gapA[g][0];
                Mapkit_AddBox(plo, phi, wallMat, contents, texScale, edit);       // full wall before the gap
                plo[along] = gapA[g][0];
                phi[along] = gapA[g][1];
                phi[2] = gapZ[g][0];
                Mapkit_AddBox(plo, phi, wallMat, contents, texScale, edit);       // sill
                plo[2] = gapZ[g][1];
                phi[2] = whi[2];
                Mapkit_AddBox(plo, phi, wallMat, contents, texScale, edit);       // lintel
                if (gapA[g][1] > cursor)
                    cursor = gapA[g][1];
            }
            float plo[3] = { wlo[0], wlo[1], wlo[2] };
            plo[along] = cursor;
            Mapkit_AddBox(plo, whi, wallMat, contents, texScale, edit);
        }
        // the room's walls, floor and ceiling take their lighting from inside the room, 16 u in from the face nearest
        // the box: a wall's own centre can sit in retail solid, where the light grid is black (run mk9: the east
        // wall drew black while a zombie in front of it was lit)
        for (int b = firstBox; b < s_boxCount; ++b)
        {
            for (int i = 0; i < 3; ++i)
            {
                const float lo = mins[i] + 16.0f < maxs[i] - 16.0f ? mins[i] + 16.0f : (mins[i] + maxs[i]) * 0.5f;
                const float hi = mins[i] + 16.0f < maxs[i] - 16.0f ? maxs[i] - 16.0f : (mins[i] + maxs[i]) * 0.5f;
                const float c = s_boxes[b].lightOrigin[i];
                s_boxes[b].lightOrigin[i] = c < lo ? lo : (c > hi ? hi : c);
            }
        }
    }
}

// mod: mapkit empty base - the level's entity string is built from the layout (CM_EntityString returns this):
// the retail worldspawn block first (SP_worldspawn requires it; it keeps the base map's sun/fog keys), then the
// entities of the layout's edits. No retail entity (spawners, windows, triggers, script models, fx) is kept.
//   spawn { origin, yaw, zone }  4 initial_spawn_points + 4 respawn structs + a player_respawn_point for the zone
//                                + an info_player_start
//   ent   { keys: { classname, targetname, ... } }  one entity/struct verbatim; vector values become "x y z"
namespace
{
    char *s_mkEnts;
    size_t s_mkEntsLen;
    size_t s_mkEntsCap;
    bool s_mkEntsBuilt; // built once per map load (CM_Mapkit_Load clears it)

    void MkE_Add(const char *text)
    {
        const size_t n = strlen(text);
        if (s_mkEntsLen + n + 1 > s_mkEntsCap)
        {
            s_mkEntsCap = (s_mkEntsLen + n + 1) * 2 + 4096;
            s_mkEnts = (char *)realloc(s_mkEnts, s_mkEntsCap);
        }
        memcpy(s_mkEnts + s_mkEntsLen, text, n + 1);
        s_mkEntsLen += n;
    }

    void MkE_Key(const char *key, const char *value)
    {
        MkE_Add("\"");
        MkE_Add(key);
        MkE_Add("\" \"");
        MkE_Add(value);
        MkE_Add("\"\n");
    }

    void MkE_KeyVec(const char *key, const float *v)
    {
        char buf[96];
        Com_sprintf(buf, sizeof(buf), "%g %g %g", v[0], v[1], v[2]);
        MkE_Key(key, buf);
    }

    void MkE_Struct(const char *targetname, const float *origin, float yaw, const char *extraKey, const char *extraValue)
    {
        const float angles[3] = { 0.0f, yaw, 0.0f };
        MkE_Add("{\n\"classname\" \"script_struct\"\n");
        MkE_Key("targetname", targetname);
        MkE_KeyVec("origin", origin);
        MkE_KeyVec("angles", angles);
        if (extraKey)
            MkE_Key(extraKey, extraValue);
        MkE_Add("}\n");
    }

    void MkE_Spawn(int e, int c)
    {
        float origin[3];
        if (!MkJ_Vec3(MkJ_Child(c, "origin"), origin))
        {
            Com_PrintWarning(1, "mapkit: spawn edit %d needs an origin\n", e);
            return;
        }
        const float yaw = MkJ_Num(c, "yaw", 0.0f);
        const char *zone = MkJ_Str(c, "zone", "mapkit_zone");
        // the four players stand 40 u apart on a line across the facing (retail Five: script_int 1..4 + a character)
        static const char *characters[4] = { "jfk", "nixon", "castro", "mcnamara" };
        const float rad = yaw * 0.017453292f;
        const float side[2] = { -sinf(rad), cosf(rad) };
        for (int i = 0; i < 4; ++i)
        {
            const float off = (i - 1.5f) * 40.0f;
            const float p[3] = { origin[0] + side[0] * off, origin[1] + side[1] * off, origin[2] };
            char idx[8];
            Com_sprintf(idx, sizeof(idx), "%d", i + 1);
            const float angles[3] = { 0.0f, yaw, 0.0f };
            MkE_Add("{\n\"classname\" \"script_struct\"\n\"targetname\" \"initial_spawn_points\"\n");
            MkE_KeyVec("origin", p);
            MkE_KeyVec("angles", angles);
            MkE_Key("script_int", idx);
            MkE_Key("script_string", characters[i]);
            MkE_Add("}\n");
            // respawn structs: player_respawn_point targets them, indexed by player number (_zombiemode)
            MkE_Struct("mapkit_respawn_points", p, yaw, "script_int", idx);
        }
        MkE_Add("{\n\"classname\" \"script_struct\"\n\"targetname\" \"player_respawn_point\"\n\"target\" \"mapkit_respawn_points\"\n\"radius\" \"500\"\n");
        MkE_KeyVec("origin", origin);
        MkE_Key("script_noteworthy", zone);
        MkE_Add("}\n{\n\"classname\" \"info_player_start\"\n");
        MkE_KeyVec("origin", origin);
        const float angles[3] = { 0.0f, yaw, 0.0f };
        MkE_KeyVec("angles", angles);
        MkE_Add("}\n");
    }

    void MkE_Ent(int e, int c)
    {
        const int keys = MkJ_Child(c, "keys");
        if (keys < 0 || s_json[keys].type != MKJ_OBJ || !*MkJ_Str(keys, "classname", ""))
        {
            Com_PrintWarning(1, "mapkit: ent edit %d needs keys with a classname\n", e);
            return;
        }
        MkE_Add("{\n");
        for (int k = s_json[keys].first; k >= 0; k = s_json[k].next)
        {
            char buf[128];
            float v[3];
            if (s_json[k].type == MKJ_STR)
                MkE_Key(s_json[k].key, s_json[k].str);
            else if (s_json[k].type == MKJ_NUM || s_json[k].type == MKJ_BOOL)
            {
                Com_sprintf(buf, sizeof(buf), "%g", s_json[k].num);
                MkE_Key(s_json[k].key, buf);
            }
            else if (MkJ_Vec3(k, v))
                MkE_KeyVec(s_json[k].key, v);
        }
        MkE_Add("}\n");
    }
}

// ---- empty base: layout brush models (zone volumes, wall-buy triggers, "bmodel" ents), windows, wall buys, spawners ----
namespace
{
    const int MK_MAX_BMODELS = 256;
    const int MK_MAX_EDITS = 1024;
    cmodel_t *s_bmCmodels; // cm arrays grown by the layout's brush models (0 = cm holds the retail arrays)
    cbrush_t *s_bmBrushes;
    cLeafBrushNode_s *s_bmNodes;
    unsigned __int16 s_bmLeafBrush[MK_MAX_BMODELS];
    cmodel_t *s_retailCmodels;
    unsigned int s_retailNumSub;
    cbrush_t *s_retailBrushes;
    unsigned int s_retailNumBrushes;
    cLeafBrushNode_s *s_retailNodes;
    unsigned int s_retailNodeCount;
    int s_editBModel[MK_MAX_EDITS]; // edit -> cmodel index ("*N"), -1 = none
    int s_editUseBModel[MK_MAX_EDITS]; // blockers have a separate touch volume reaching both sides
    float s_editBCenter[MK_MAX_EDITS][3]; // the entity origin (the brush is built around it)

    int MkE_EditNode(int edit)
    {
        int c = s_editsNode >= 0 ? s_json[s_editsNode].first : -1;
        for (int e = 0; c >= 0 && e < edit; ++e)
            c = s_json[c].next;
        return c;
    }

    // the wall buy's use trigger: a 64-unit cube 24 units in front of the weapon model (retail Five: brush *601)
    bool MkE_WallBuyBox(int c, float *mins, float *maxs)
    {
        float origin[3];
        if (!MkJ_Vec3(MkJ_Child(c, "origin"), origin))
            return false;
        const float rad = MkJ_Num(c, "yaw", 0.0f) * 0.017453292f;
        const float center[3] = { origin[0] + cosf(rad) * 24.0f, origin[1] + sinf(rad) * 24.0f, origin[2] };
        for (int k = 0; k < 3; ++k)
        {
            mins[k] = center[k] - 32.0f;
            maxs[k] = center[k] + 32.0f;
        }
        return true;
    }

    // window: Five's window pf82 (entities.txt:55-192) re-placed. Layout "origin" = where retail (-917, 1911, 16)
    // goes (the middle of the wall at floor height), "yaw" = the direction into the room (retail 90).
    // MEASURED: the reference floor z 16 is a choice (pf82's nodes are at z 47, its repair struct at 24).
    const float MK_WIN_REF[3] = { -917.0f, 1911.0f, 16.0f };

    void MkE_WindowXf(int c, const float *retail, float *out)
    {
        float origin[3];
        if (!MkJ_Vec3(MkJ_Child(c, "origin"), origin))
            origin[0] = origin[1] = origin[2] = 0.0f;
        const float rad = (MkJ_Num(c, "yaw", 90.0f) - 90.0f) * 0.017453292f;
        const float d[3] = { retail[0] - MK_WIN_REF[0], retail[1] - MK_WIN_REF[1], retail[2] - MK_WIN_REF[2] };
        out[0] = origin[0] + cosf(rad) * d[0] - sinf(rad) * d[1];
        out[1] = origin[1] + sinf(rad) * d[0] + cosf(rad) * d[1];
        out[2] = origin[2] + d[2];
    }
}

// grows cm.cmodels / cm.brushes / cm.leafbrushNodes by one axial box brush model per layout zone, wall buy and
// "bmodel" ent (empty base only). The retail arrays are kept and restored first, so every load starts from them.
void CM_Mapkit_ExtendSubmodels()
{
    if (!s_bmCmodels || cm.cmodels != s_bmCmodels)
    {
        s_retailCmodels = cm.cmodels;
        s_retailNumSub = cm.numSubModels;
        s_retailBrushes = cm.brushes;
        s_retailNumBrushes = cm.numBrushes;
        s_retailNodes = cm.leafbrushNodes;
        s_retailNodeCount = cm.leafbrushNodesCount;
    }
    cm.cmodels = s_retailCmodels;
    cm.numSubModels = s_retailNumSub;
    cm.brushes = s_retailBrushes;
    cm.numBrushes = (unsigned __int16)s_retailNumBrushes;
    cm.leafbrushNodes = s_retailNodes;
    cm.leafbrushNodesCount = s_retailNodeCount;
    free(s_bmCmodels);
    _aligned_free(s_bmBrushes);
    free(s_bmNodes);
    s_bmCmodels = 0;
    s_bmBrushes = 0;
    s_bmNodes = 0;
    for (int i = 0; i < MK_MAX_EDITS; ++i)
    {
        s_editBModel[i] = -1;
        s_editUseBModel[i] = -1;
    }
    if (!CM_Mapkit_Empty())
        return;

    float bmins[MK_MAX_BMODELS][3], bmaxs[MK_MAX_BMODELS][3];
    int bedit[MK_MAX_BMODELS];
    bool buse[MK_MAX_BMODELS] = {};
    int count = 0;
    int e = 0;
    for (int c = s_json[s_editsNode].first; c >= 0 && e < MK_MAX_EDITS && count < MK_MAX_BMODELS; c = s_json[c].next, ++e)
    {
        const char *kind = MkJ_Str(c, "kind", "");
        bool ok = false;
        if (!strcmp(kind, "zone") || !strcmp(kind, "door") || !strcmp(kind, "debris"))
            ok = MkJ_Vec3(MkJ_Child(c, "mins"), bmins[count]) && MkJ_Vec3(MkJ_Child(c, "maxs"), bmaxs[count]);
        else if (!strcmp(kind, "mysterybox"))
        {
            float o[3];
            ok = MkJ_Vec3(MkJ_Child(c, "origin"), o);
            if (ok)
                for (int k = 0; k < 3; ++k)
                {
                    bmins[count][k] = o[k] - (k == 2 ? 0.0f : 40.0f);
                    bmaxs[count][k] = o[k] + (k == 2 ? 64.0f : 40.0f);
                }
        }
        else if (!strcmp(kind, "wallbuy"))
            ok = MkE_WallBuyBox(c, bmins[count], bmaxs[count]);
        else if (!strcmp(kind, "ent"))
        {
            const int bm = MkJ_Child(c, "bmodel");
            ok = bm >= 0 && MkJ_Vec3(MkJ_Child(bm, "mins"), bmins[count]) && MkJ_Vec3(MkJ_Child(bm, "maxs"), bmaxs[count]);
        }
        if (ok)
        {
            bedit[count++] = e;
            if ((!strcmp(kind, "door") || !strcmp(kind, "debris")) && count < MK_MAX_BMODELS)
            {
                for (int k = 0; k < 3; ++k)
                {
                    bmins[count][k] = bmins[count - 1][k] - (k == 2 ? 0.0f : 48.0f);
                    bmaxs[count][k] = bmaxs[count - 1][k] + (k == 2 ? 0.0f : 48.0f);
                }
                buse[count] = true;
                bedit[count++] = e;
            }
        }
    }
    if (!count)
        return;
    if (s_retailNumSub + count >= 1024 || s_retailNumBrushes + count > 0xFFFF)
    {
        Com_PrintWarning(1, "mapkit: %d layout brush models do not fit (retail %u models, %u brushes)\n", count,
            s_retailNumSub, s_retailNumBrushes);
        return;
    }
    s_bmCmodels = (cmodel_t *)malloc((s_retailNumSub + count) * sizeof(cmodel_t));
    s_bmBrushes = (cbrush_t *)_aligned_malloc((s_retailNumBrushes + count) * sizeof(cbrush_t), 16);
    s_bmNodes = (cLeafBrushNode_s *)malloc((s_retailNodeCount + count) * sizeof(cLeafBrushNode_s));
    memcpy(s_bmCmodels, s_retailCmodels, s_retailNumSub * sizeof(cmodel_t));
    memcpy(s_bmBrushes, s_retailBrushes, s_retailNumBrushes * sizeof(cbrush_t));
    memcpy(s_bmNodes, s_retailNodes, s_retailNodeCount * sizeof(cLeafBrushNode_s));
    for (int i = 0; i < count; ++i)
    {
        float *center = s_editBCenter[bedit[i]];
        float lo[3], hi[3];
        for (int k = 0; k < 3; ++k)
        {
            center[k] = (bmins[i][k] + bmaxs[i][k]) * 0.5f;
            lo[k] = bmins[i][k] - center[k];
            hi[k] = bmaxs[i][k] - center[k];
        }
        // the brush in model space around the entity origin: the same fill as a layout box (Mapkit_AddBox)
        cbrush_t *brush = &s_bmBrushes[s_retailNumBrushes + i];
        memset(brush, 0, sizeof(*brush));
        for (int k = 0; k < 3; ++k)
        {
            brush->mins[k] = lo[k];
            brush->maxs[k] = hi[k];
            brush->axial_cflags[0][k] = 1;
            brush->axial_cflags[1][k] = 1;
        }
        brush->contents = 1;
        s_bmLeafBrush[i] = (unsigned __int16)(s_retailNumBrushes + i);
        cLeafBrushNode_s *node = &s_bmNodes[s_retailNodeCount + i];
        memset(node, 0, sizeof(*node));
        node->leafBrushCount = 1;
        node->contents = 1;
        node->data.leaf.brushes = &s_bmLeafBrush[i];
        cmodel_t *model = &s_bmCmodels[s_retailNumSub + i];
        memset(model, 0, sizeof(*model));
        for (int k = 0; k < 3; ++k)
        {
            model->mins[k] = model->leaf.mins[k] = lo[k];
            model->maxs[k] = model->leaf.maxs[k] = hi[k];
        }
        model->radius = sqrtf(hi[0] * hi[0] + hi[1] * hi[1] + hi[2] * hi[2]);
        model->leaf.brushContents = 1;
        model->leaf.leafBrushNode = (int)(s_retailNodeCount + i);
        if (buse[i])
            s_editUseBModel[bedit[i]] = (int)(s_retailNumSub + i);
        else
            s_editBModel[bedit[i]] = (int)(s_retailNumSub + i);
    }
    cm.cmodels = s_bmCmodels;
    cm.numSubModels = s_retailNumSub + count;
    cm.brushes = s_bmBrushes;
    cm.numBrushes = (unsigned __int16)(s_retailNumBrushes + count);
    cm.leafbrushNodes = s_bmNodes;
    cm.leafbrushNodesCount = s_retailNodeCount + count;
    Com_Printf(1, "mapkit: %d layout brush models *%u..*%u (retail *13 bounds %g %g %g .. %g %g %g)\n", count,
        s_retailNumSub, s_retailNumSub + count - 1, s_retailCmodels[13].mins[0], s_retailCmodels[13].mins[1],
        s_retailCmodels[13].mins[2], s_retailCmodels[13].maxs[0], s_retailCmodels[13].maxs[1], s_retailCmodels[13].maxs[2]);
}

// mod: "stairs" edit: mins/maxs bound the whole flight, "up" (+x,-x,+y,-y) is the rising direction, the top tread is
// at maxs.z. Steps = round(height / rise) (rise default 12); step i is a solid box from mins.z up to its tread.
// Engine limits (G_Mapkit_LinkPathnodes -> Path_CanLinkNodes): a rise over 18 fails the step-up (g_apl.stepheight 18)
// and a tread under 32 puts the next riser inside the 31-wide node drop hull.
static int CM_Mapkit_StairsInfo(int c, float *mins, float *maxs, int *axis, int *sign)
{
    if (c < 0 || strcmp(MkJ_Str(c, "kind", ""), "stairs") || !MkJ_Vec3(MkJ_Child(c, "mins"), mins)
        || !MkJ_Vec3(MkJ_Child(c, "maxs"), maxs) || mins[0] >= maxs[0] || mins[1] >= maxs[1] || mins[2] >= maxs[2])
        return 0;
    const char *up = MkJ_Str(c, "up", "+x");
    *axis = (up[1] == 'y') ? 1 : 0;
    *sign = (up[0] == '-') ? -1 : 1;
    const float rise = MkJ_Num(c, "rise", 12.0f);
    int n = (int)floorf((maxs[2] - mins[2]) / (rise > 1.0f ? rise : 1.0f) + 0.5f);
    return n < 1 ? 1 : (n > 32 ? 32 : n);
}

bool CM_Mapkit_StairsStep(int edit, int step, float *outMins, float *outMaxs)
{
    float mins[3], maxs[3];
    int axis, sign;
    const int n = CM_Mapkit_StairsInfo(MkE_EditNode(edit), mins, maxs, &axis, &sign);
    if (step < 0 || step >= n)
        return false;
    const float tread = (maxs[axis] - mins[axis]) / n;
    for (int k = 0; k < 3; ++k)
    {
        outMins[k] = mins[k];
        outMaxs[k] = maxs[k];
    }
    outMins[axis] = sign > 0 ? mins[axis] + tread * step : maxs[axis] - tread * (step + 1);
    outMaxs[axis] = outMins[axis] + tread;
    outMaxs[2] = mins[2] + (maxs[2] - mins[2]) * (step + 1) / n;
    return true;
}

// Evenly spaced positions 24 inside [lo, hi] at most `spacing` apart (at least one, centred when narrow).
static int CM_Mapkit_GridAxis(float lo, float hi, float spacing, float *out, int max)
{
    const float a = lo + 24.0f, b = hi - 24.0f;
    if (b <= a || spacing <= 0.0f)
    {
        out[0] = (lo + hi) * 0.5f;
        return 1;
    }
    int n = (int)floorf((b - a) / spacing) + 1;
    if (a + spacing * (n - 1) < b - 0.5f)
        ++n;
    if (n > max)
        n = max;
    for (int i = 0; i < n; ++i)
        out[i] = n == 1 ? (a + b) * 0.5f : a + (b - a) * i / (n - 1);
    return n;
}

// mod: purchase use triggers as spawned (wall buy: MkE_WallBuyBox 64-cube; perk: _mapkit.gsc trigger_radius_use r44 12 in
// front, z +48..+118, r56 centred when yaw is absent, also pap and powerswitch; box: 80x80x64). Intersecting triggers send the use press to the
// wrong purchase (mapmaker-2 D4: AK74u beside Juggernog). Same test as tools/mapkit/doctor-static.mjs triggerGap.
struct MkTrigger { int edit; bool circle; float c[2], r, mins[3], maxs[3]; };

bool CM_Mapkit_ResolvedSpot(int edit, float *origin, float *yaw);

static bool CM_Mapkit_PurchaseTrigger(int edit, MkTrigger *t)
{
    const int c = MkE_EditNode(edit);
    float o[3];
    if (c < 0 || !MkJ_Vec3(MkJ_Child(c, "origin"), o))
        return false;
    const char *kind = MkJ_Str(c, "kind", "");
    memset(t, 0, sizeof(*t));
    t->edit = edit;
    if (!strcmp(kind, "wallbuy"))
        return MkE_WallBuyBox(c, t->mins, t->maxs);
    if (!strcmp(kind, "mysterybox"))
    {
        for (int k = 0; k < 3; ++k)
        {
            t->mins[k] = o[k] - (k == 2 ? 0.0f : 40.0f);
            t->maxs[k] = o[k] + (k == 2 ? 64.0f : 40.0f);
        }
        return true;
    }
    if (strcmp(kind, "perk") && strcmp(kind, "pap") && strcmp(kind, "powerswitch")) // mapkit-fix-3: same trigger as a perk
        return false;
    float spotYaw = 0.0f;
    const bool snapped = CM_Mapkit_ResolvedSpot(edit, o, &spotYaw); // mapkit-fix-4b: snap:"wall" as placed
    const bool hasYaw = snapped || MkJ_Child(c, "yaw") >= 0;
    const float rad = (snapped ? spotYaw : MkJ_Num(c, "yaw", 0.0f)) * 0.017453292f, off = hasYaw ? 12.0f : 0.0f;
    t->circle = true;
    t->c[0] = o[0] + cosf(rad) * off;
    t->c[1] = o[1] + sinf(rad) * off;
    t->r = hasYaw ? 44.0f : 56.0f;
    t->mins[0] = t->maxs[0] = t->c[0];
    t->mins[1] = t->maxs[1] = t->c[1];
    t->mins[2] = o[2] + 48.0f;
    t->maxs[2] = o[2] + 118.0f;
    return true;
}

static float CM_Mapkit_TriggerGap(const MkTrigger *a, const MkTrigger *b)
{
    if (a->mins[2] > b->maxs[2] || b->mins[2] > a->maxs[2])
        return 1.0e9f;
    if (a->circle && b->circle)
        return sqrtf((a->c[0] - b->c[0]) * (a->c[0] - b->c[0]) + (a->c[1] - b->c[1]) * (a->c[1] - b->c[1])) - a->r - b->r;
    if (a->circle || b->circle)
    {
        const MkTrigger *c = a->circle ? a : b, *q = a->circle ? b : a;
        float d[2], inside = 1.0e9f;
        for (int k = 0; k < 2; ++k)
        {
            d[k] = fmaxf(fmaxf(q->mins[k] - c->c[k], 0.0f), c->c[k] - q->maxs[k]);
            inside = fminf(inside, fminf(c->c[k] - q->mins[k], q->maxs[k] - c->c[k]));
        }
        return (d[0] > 0.0f || d[1] > 0.0f) ? sqrtf(d[0] * d[0] + d[1] * d[1]) - c->r : -c->r - inside;
    }
    float g[2];
    for (int k = 0; k < 2; ++k)
        g[k] = fmaxf(a->mins[k] - b->maxs[k], b->mins[k] - a->maxs[k]);
    if (g[0] < 0.0f && g[1] < 0.0f)
        return fmaxf(g[0], g[1]);
    return sqrtf(fmaxf(g[0], 0.0f) * fmaxf(g[0], 0.0f) + fmaxf(g[1], 0.0f) * fmaxf(g[1], 0.0f));
}

static void CM_Mapkit_WarnPurchaseOverlap()
{
    MkTrigger t[64];
    int n = 0;
    for (int e = 0; e < CM_Mapkit_EditCount() && n < 64; ++e)
        if (CM_Mapkit_PurchaseTrigger(e, &t[n]))
            ++n;
    for (int i = 0; i < n; ++i)
        for (int j = i + 1; j < n; ++j)
        {
            const float gap = CM_Mapkit_TriggerGap(&t[i], &t[j]);
            if (gap < 0.0f)
                Com_PrintWarning(1, "mapkit: purchase triggers of edit %d and edit %d intersect by %.0f units; a use press can "
                    "buy the wrong one - move them at least %.0f units further apart\n", t[i].edit, t[j].edit, -gap, ceilf(-gap));
        }
}

// mapkit-fix-4b: machine clips. Retail Five's perk machines, Pack-a-Punch and box stand in baked CLIP brushes (player clip +
// monster clip: cm_noperks.cpp removes them); a script_model's own contents never stop a player or an actor. Every perk / pap
// / powerswitch / mysterybox edit gets a hidden axial box of contents 0x8030200 around the machine's model bounds (rotated by
// the model yaw, then the covering AABB). It is added before G_InitGame, so the node pass cuts the links through it
// (Mapkit_GridInSolid / Mapkit_LinkNearBox, Path_CanLinkNodes mask 0x2820011 & 0x20000). Use traces (0x11) and BulletTrace
// (0x280E033) pass through it. snap:"wall" is resolved here, by _mapkit.gsc mapkit_wall_spot's rule, so the clip and the
// machine stand on the same spot: _mapkit.gsc reads the result as fields "_spot" / "_spotyaw".
namespace
{
    const int MK_MAX_SPOTS = 256;
    struct MkSpot { bool set, clip; float origin[3]; float yaw; float cmins[3], cmaxs[3]; };
    MkSpot s_spots[MK_MAX_SPOTS];

    float MkM_FloorZ(const float *o)
    {
        // _mapkit.gsc mapkit_floor_z: BulletTrace from z+48 down to z-256
        const float start[3] = { o[0], o[1], o[2] + 48.0f };
        const float end[3] = { o[0], o[1], o[2] - 256.0f };
        col_context_t context(0x280E033);
        trace_t tr;
        CM_BoxTrace(&tr, start, end, vec3_origin, vec3_origin, 0x280E033, &context);
        return start[2] + (end[2] - start[2]) * tr.fraction;
    }

    // _mapkit.gsc mapkit_wall_spot: 8 yaws at floor+40, 200 units, the nearest hit, backed off by depth, re-floored
    bool MkM_WallSpot(const float *anchor, float depth, MkSpot *s)
    {
        const float start[3] = { anchor[0], anchor[1], MkM_FloorZ(anchor) + 40.0f };
        float bestD = 99999.0f, best[3], bestDir[2];
        bool found = false;
        for (int a = 0; a < 360; a += 45)
        {
            const float dir[2] = { cosf(a * 0.017453292f), sinf(a * 0.017453292f) };
            const float end[3] = { start[0] + dir[0] * 200.0f, start[1] + dir[1] * 200.0f, start[2] };
            col_context_t context(0x280E033);
            trace_t tr;
            CM_BoxTrace(&tr, start, end, vec3_origin, vec3_origin, 0x280E033, &context);
            if (tr.fraction < 1.0f && tr.fraction * 200.0f < bestD)
            {
                bestD = tr.fraction * 200.0f;
                for (int k = 0; k < 3; ++k)
                    best[k] = start[k] + (end[k] - start[k]) * tr.fraction;
                bestDir[0] = dir[0];
                bestDir[1] = dir[1];
                found = true;
            }
        }
        if (!found)
            return false;
        const float o[3] = { best[0] - bestDir[0] * depth, best[1] - bestDir[1] * depth, best[2] };
        s->origin[0] = o[0];
        s->origin[1] = o[1];
        s->origin[2] = MkM_FloorZ(o);
        s->yaw = atan2f(-bestDir[1], -bestDir[0]) * 57.29578f; // VectorToAngles(out)[1]: 0..360
        if (s->yaw < 0.0f)
            s->yaw += 360.0f;
        s->yaw = floorf(s->yaw + 0.5f);
        s->set = true;
        return true;
    }

    // model-local clip bounds (x, y, z from the edit origin; the model faces local -y): Five's machine xmodel bounds
    struct MkClip { const char *key; float mins[3], maxs[3]; float modelYaw; };
    const MkClip s_machineClips[] = {
        { "specialty_armorvest", { -16.0f, -18.0f, -4.0f }, { 16.0f, 19.0f, 101.0f }, 90.0f },
        { "specialty_fastreload", { -27.5f, -18.0f, -4.0f }, { 28.5f, 19.0f, 105.5f }, 90.0f },
        { "specialty_rof", { -28.0f, -18.0f, -4.0f }, { 27.0f, 13.5f, 83.5f }, 90.0f },
        { "specialty_quickrevive", { -25.0f, -24.0f, -4.0f }, { 25.0f, 12.0f, 86.0f }, 90.0f },
        { "pap", { -43.0f, -27.0f, -4.0f }, { 43.0f, 23.0f, 87.0f }, 90.0f },
        // MEASURED/approximated: the panel is wall-flat at spot+48 (4 off the wall); a 32-wide slab from the wall out 12
        { "powerswitch", { -16.0f, -12.0f, 24.0f }, { 16.0f, 4.0f, 104.0f }, 90.0f },
        // box model at origin+14, angles yaw-90
        { "mysterybox", { -50.0f, -18.0f, -10.0f }, { 50.0f, 18.0f, 145.0f }, -90.0f },
    };
}

void CM_Mapkit_AddMachineClips()
{
    memset(s_spots, 0, sizeof(s_spots));
    if (!CM_Mapkit_Active())
        return;
    int added = 0;
    for (int e = 0; e < CM_Mapkit_EditCount() && e < MK_MAX_SPOTS; ++e)
    {
        const int c = MkE_EditNode(e);
        float o[3];
        if (c < 0 || !MkJ_Vec3(MkJ_Child(c, "origin"), o))
            continue;
        const char *kind = MkJ_Str(c, "kind", "");
        const char *key = !strcmp(kind, "perk") ? MkJ_Str(c, "perk", "") : kind;
        const MkClip *clip = 0;
        for (int i = 0; i < (int)(sizeof(s_machineClips) / sizeof(s_machineClips[0])); ++i)
            if (!strcmp(s_machineClips[i].key, key))
                clip = &s_machineClips[i];
        if (!clip)
            continue;
        float yaw = MkJ_Num(c, "yaw", !strcmp(kind, "mysterybox") ? 270.0f : 0.0f);
        if (strcmp(kind, "mysterybox") && !strcmp(MkJ_Str(c, "snap", ""), "wall")
            && MkM_WallSpot(o, !strcmp(kind, "powerswitch") ? 4.0f : 20.0f, &s_spots[e]))
        {
            o[0] = s_spots[e].origin[0];
            o[1] = s_spots[e].origin[1];
            o[2] = s_spots[e].origin[2];
            yaw = s_spots[e].yaw;
        }
        const float m = (yaw + clip->modelYaw) * 0.017453292f, cm_ = cosf(m), sm = sinf(m);
        float mins[3] = { 1.0e9f, 1.0e9f, o[2] + clip->mins[2] }, maxs[3] = { -1.0e9f, -1.0e9f, o[2] + clip->maxs[2] };
        for (int k = 0; k < 4; ++k)
        {
            const float x = (k & 1) ? clip->maxs[0] : clip->mins[0], y = (k & 2) ? clip->maxs[1] : clip->mins[1];
            const float wx = o[0] + x * cm_ - y * sm, wy = o[1] + x * sm + y * cm_;
            mins[0] = fminf(mins[0], wx);
            maxs[0] = fmaxf(maxs[0], wx);
            mins[1] = fminf(mins[1], wy);
            maxs[1] = fmaxf(maxs[1], wy);
        }
        // mod: noperks 1 (stacked with mapkit, fs_mods): the layout's perk machines are not placed
        // (mods/mapkit/maps/mapkit/_mapkit.gsc main_end), so no clip where they would stand - as cm_noperks.cpp removes retail machines' clip.
        // Pack-a-Punch stays, as on retail maps.
        const dvar_s *noperks = Dvar_FindVar("noperks");
        if (!strcmp(kind, "perk") && noperks && noperks->current.enabled)
            continue;
        const int before = s_boxCount;
        Mapkit_AddBox(mins, maxs, "mc/pent_art_wall_wood07_dark", 0x8030200, 128.0f, e);
        if (s_boxCount > before)
        {
            s_boxes[before].hidden = true;
            s_spots[e].clip = true;
            for (int k = 0; k < 3; ++k)
            {
                s_spots[e].cmins[k] = mins[k];
                s_spots[e].cmaxs[k] = maxs[k];
            }
            ++added;
        }
    }
    CM_Mapkit_WarnPurchaseOverlap();
    if (added)
        Com_Printf(1, "mapkit: %d machine clips (player + monster clip)\n", added);
}

bool CM_Mapkit_MachineClip(int edit, float *mins, float *maxs)
{
    if (edit < 0 || edit >= MK_MAX_SPOTS || !s_spots[edit].clip)
        return false;
    for (int k = 0; k < 3; ++k)
    {
        mins[k] = s_spots[edit].cmins[k];
        maxs[k] = s_spots[edit].cmaxs[k];
    }
    return true;
}

bool CM_Mapkit_ResolvedSpot(int edit, float *origin, float *yaw)
{
    if (edit < 0 || edit >= MK_MAX_SPOTS || !s_spots[edit].set)
        return false;
    for (int k = 0; k < 3; ++k)
        origin[k] = s_spots[edit].origin[k];
    *yaw = s_spots[edit].yaw;
    return true;
}

// Walking-surface nodes (feet Z) generated by an edit: every stairs tread, and the top of a box with "walkable":1.
// "nodeSpacing" (default 56) spaces them; g_mapkit_path adds them like explicit "node" edits.
int CM_Mapkit_WalkNodes(int edit, float (*out)[3], int max)
{
    const int c = MkE_EditNode(edit);
    if (c < 0)
        return 0;
    const float spacing = MkJ_Num(c, "nodeSpacing", 56.0f);
    float xs[16], ys[16], mins[3], maxs[3];
    int n = 0;
    if (!strcmp(MkJ_Str(c, "kind", ""), "stairs"))
    {
        float lo[3], hi[3];
        for (int s = 0; CM_Mapkit_StairsStep(edit, s, lo, hi); ++s)
        {
            int axis, sign;
            CM_Mapkit_StairsInfo(c, mins, maxs, &axis, &sign);
            const int across = axis ^ 1;
            const int k = CM_Mapkit_GridAxis(lo[across], hi[across], spacing, xs, 16);
            for (int i = 0; i < k && n < max; ++i, ++n)
            {
                out[n][axis] = (lo[axis] + hi[axis]) * 0.5f;
                out[n][across] = xs[i];
                out[n][2] = hi[2];
            }
        }
        return n;
    }
    if (strcmp(MkJ_Str(c, "kind", ""), "box") || MkJ_Num(c, "walkable", 0.0f) == 0.0f
        || !MkJ_Vec3(MkJ_Child(c, "mins"), mins) || !MkJ_Vec3(MkJ_Child(c, "maxs"), maxs))
        return 0;
    const int nx = CM_Mapkit_GridAxis(mins[0], maxs[0], spacing, xs, 16);
    const int ny = CM_Mapkit_GridAxis(mins[1], maxs[1], spacing, ys, 16);
    for (int i = 0; i < nx; ++i)
        for (int j = 0; j < ny && n < max; ++j, ++n)
        {
            out[n][0] = xs[i];
            out[n][1] = ys[j];
            out[n][2] = maxs[2];
        }
    return n;
}

bool CM_Mapkit_WindowNodes(int edit, float *begin, float *end, float *yaw)
{
    const int c = MkE_EditNode(edit);
    if (c < 0 || strcmp(MkJ_Str(c, "kind", ""), "window"))
        return false;
    // a retail-overlay window (spawn/target, no origin) reuses the retail entrance and its nodes: MkE_WindowXf would
    // put this pair at the world origin (mapkit_test: "Pathnode (Begin) at (0 -43 31) is floating", node_clearance FAIL)
    if (MkJ_Child(c, "origin") < 0)
        return false;
    static const float retailBegin[3] = { -917.0f, 1868.0f, 47.0f }; // pf82 node_negotiation_begin (E:165)
    static const float retailEnd[3] = { -917.0f, 1948.0f, 47.0f };   // pf82 node_negotiation_end (E:172)
    MkE_WindowXf(c, retailBegin, begin);
    MkE_WindowXf(c, retailEnd, end);
    *yaw = MkJ_Num(c, "yaw", 90.0f);
    return true;
}

namespace
{
    int s_mkWindowEdits; // window edits in the layout (set before the entity loop)

    void MkE_Model(const char *model, int index)
    {
        char buf[16];
        Com_sprintf(buf, sizeof(buf), "*%d", index);
        MkE_Key(model, buf);
    }

    // zone: the info_volume the zone manager tests players against (zone_manager.gsc:179-196; Five E:4893)
    void MkE_Zone(int e, int c)
    {
        if (e >= MK_MAX_EDITS || s_editBModel[e] < 0)
            return;
        const char *name = MkJ_Str(c, "name", "mapkit_zone");
        char buf[96];
        MkE_Add("{\n\"classname\" \"info_volume\"\n");
        MkE_Model("model", s_editBModel[e]);
        MkE_KeyVec("origin", s_editBCenter[e]);
        MkE_Key("targetname", name);
        Com_sprintf(buf, sizeof(buf), "%s_spawners", name);
        MkE_Key("target", buf);
        MkE_Key("script_noteworthy", "player_volume");
        MkE_Add("}\n");
    }

    // wall buy: trigger_use + the chalk/weapon model it targets (weapons.gsc:356-374; Five m14 E:17207-17221)
    void MkE_WallBuy(int e, int c)
    {
        float origin[3];
        if (e >= MK_MAX_EDITS || s_editBModel[e] < 0 || !MkJ_Vec3(MkJ_Child(c, "origin"), origin))
            return;
        char target[32];
        Com_sprintf(target, sizeof(target), "mkbuy%d", e);
        MkE_Add("{\n\"classname\" \"trigger_use\"\n");
        MkE_Model("model", s_editBModel[e]);
        MkE_KeyVec("origin", s_editBCenter[e]);
        MkE_Key("targetname", "weapon_upgrade");
        MkE_Key("zombie_weapon_upgrade", MkJ_Str(c, "weapon", "m14_zm"));
        MkE_Key("target", target);
        MkE_Add("}\n{\n\"classname\" \"script_model\"\n");
        MkE_Key("model", MkJ_Str(c, "model", "t5_weapon_m14_world"));
        MkE_KeyVec("origin", origin);
        const float angles[3] = { 0.0f, MkJ_Num(c, "yaw", 0.0f) - 90.0f, 0.0f };
        MkE_KeyVec("angles", angles);
        MkE_Key("targetname", target);
        MkE_Add("}\n");
    }

    // Layout adapter for retail _zombiemode_blockers::door_init / debris_init.
    // The hidden brush owns collision and DisconnectPaths; the kit draws its authored bounds until purchase.
    void MkE_Blocker(int e, int c)
    {
        if (e >= MK_MAX_EDITS || s_editBModel[e] < 0 || s_editUseBModel[e] < 0)
            return;
        char target[32], flag[32], cost[24];
        Com_sprintf(target, sizeof(target), "mkdoor%d", e);
        Com_sprintf(flag, sizeof(flag), "mkdoor%d_open", e);
        Com_sprintf(cost, sizeof(cost), "%g", MkJ_Num(c, "cost", 750));
        MkE_Add("{\n\"classname\" \"trigger_use_touch\"\n");
        MkE_Model("model", s_editUseBModel[e]);
        MkE_KeyVec("origin", s_editBCenter[e]);
        MkE_Key("targetname", !strcmp(MkJ_Str(c, "kind", ""), "door") ? "zombie_door" : "zombie_debris");
        MkE_Key("target", target);
        MkE_Key("script_flag", flag);
        MkE_Key("zombie_cost", cost);
        // mapkit-fix-3: "power": true on a door = retail electric door (_zombiemode_blockers door_init / door_think):
        // electric_buyable_door waits for flag power_on and is then bought; with "cost": 0 it is electric_door, which
        // opens by itself at power on. Retail debris_init has no power case, so debris ignores the field.
        if (!strcmp(MkJ_Str(c, "kind", ""), "door") && MkJ_Num(c, "power", 0.0f) != 0.0f)
            MkE_Key("script_noteworthy", MkJ_Num(c, "cost", 750) > 0.0f ? "electric_buyable_door" : "electric_door");
        MkE_Add("}\n{\n\"classname\" \"script_brushmodel\"\n");
        MkE_Model("model", s_editBModel[e]);
        MkE_KeyVec("origin", s_editBCenter[e]);
        MkE_Key("targetname", target);
        MkE_Key("script_noteworthy", "clip");
        MkE_Key("script_string", "clip");
        MkE_Key("spawnflags", "1"); // retail DYNAMICPATH: DisconnectPaths / ConnectPaths
        MkE_Add("}\n");
    }

    // Retail Five chest chain: trigger -> lid -> weapon origin -> box (entities *423 / level1_chest_*).
    void MkE_MysteryBox(int e, int c)
    {
        float o[3];
        if (e >= MK_MAX_EDITS || s_editBModel[e] < 0 || !MkJ_Vec3(MkJ_Child(c, "origin"), o))
            return;
        const float yaw = MkJ_Num(c, "yaw", 270);
        const float radians = yaw * 0.01745329252f;
        float angles[3] = { 0, yaw - 90, 0 };
        char lid[32], org[32], box[32], note[48], cost[24];
        Com_sprintf(lid, sizeof(lid), "mkchest%d_lid", e);
        Com_sprintf(org, sizeof(org), "mkchest%d_org", e);
        Com_sprintf(box, sizeof(box), "mkchest%d_box", e);
        Com_sprintf(note, sizeof(note), "mkchest%d%s", e, MkJ_Num(c, "start", 1) ? "_start_chest" : "");
        Com_sprintf(cost, sizeof(cost), "%g", MkJ_Num(c, "cost", 950));
        MkE_Add("{\n\"classname\" \"trigger_use_touch\"\n");
        MkE_Model("model", s_editBModel[e]);
        MkE_KeyVec("origin", s_editBCenter[e]);
        MkE_Key("targetname", "treasure_chest_use");
        MkE_Key("target", lid);
        MkE_Key("script_noteworthy", note);
        MkE_Key("zombie_cost", cost);
        MkE_Add("}\n");
        const char *names[] = { lid, org, box };
        for (int p = 0; p < 3; ++p)
        {
            float pos[3] = { o[0], o[1], o[2] + (p == 0 ? 31.0f : p == 1 ? 16.0f : 14.0f) };
            if (!p)
            {
                pos[0] -= cosf(radians) * 12;
                pos[1] -= sinf(radians) * 12;
            }
            angles[1] = p == 1 ? yaw : yaw - 90;
            MkE_Add("{\n");
            MkE_Key("classname", p == 1 ? "script_origin" : "script_model");
            if (p != 1)
                MkE_Key("model", p == 0 ? "zombie_treasure_box_lid" : "zombie_treasure_box");
            MkE_KeyVec("origin", pos);
            MkE_KeyVec("angles", angles);
            MkE_Key("targetname", names[p]);
            if (p < 2)
                MkE_Key("target", names[p + 1]);
            MkE_Add("}\n");
        }
    }

    // spawner: Five's office worker (E:9836-9849); no target = the closest exterior_goal (spawner.gsc:506-531)
    void MkE_Spawner(int e, int c)
    {
        float origin[3];
        if (!MkJ_Vec3(MkJ_Child(c, "origin"), origin))
        {
            Com_PrintWarning(1, "mapkit: spawner edit %d needs an origin\n", e);
            return;
        }
        char buf[96];
        const float angles[3] = { 0.0f, MkJ_Num(c, "yaw", 0.0f), 0.0f };
        MkE_Add("{\n");
        MkE_Key("classname", MkJ_Str(c, "classname", "actor_zombie_usa_zombie_officeworker"));
        MkE_Key("model", MkJ_Str(c, "model", "c_usa_pent_warroom_worker_body"));
        MkE_KeyVec("origin", origin);
        MkE_KeyVec("angles", angles);
        MkE_Key("spawnflags", "3");
        MkE_Key("count", "9999");
        MkE_Key("script_forcespawn", "1");
        MkE_Key("script_noteworthy", "zombie_spawner");
        MkE_Key("script_disable_bleeder", "1");
        // zombies: a layout without windows has no exterior_goal, and zombie_think's barrier search reads
        // nodes[0] of an empty level.exterior_goals (spawner.gsc:506-512, "undefined is not a field object",
        // stress_arena live doctor: 84 script errors). Retail's own way to skip the barrier is the spawner's
        // script_string "zombie_chaser" (should_skip_teardown, spawner.gsc:398-411 -> find_flesh directly).
        if (!s_mkWindowEdits)
            MkE_Key("script_string", "zombie_chaser");
        Com_sprintf(buf, sizeof(buf), "%s_spawners", MkJ_Str(c, "zone", "mapkit_zone"));
        MkE_Key("targetname", buf);
        MkE_Add("}\n");
    }

    // riser (mapkit-fix-4c): a ground riser, the retail way (Five's labs_zone2 risers): a spawner with
    // script_string "riser" (spawner.gsc:427 -> do_zombie_rise) in the zone, and a script_struct "<zone>_spawners_rise"
    // (zone_init: zone.rise_locations = GetStructArray(volume.target + "_rise"), zone_manager.gsc:231) at the spot.
    // script_noteworthy "find_flesh" on the spot = retail's "riser who spawns in the playable area" (should_skip_teardown,
    // spawner.gsc:398-402): after the climb-out it goes for the player, no window. Rise anims/fx are Five's own.
    void MkE_Riser(int e, int c)
    {
        float origin[3];
        if (!MkJ_Vec3(MkJ_Child(c, "origin"), origin))
        {
            Com_PrintWarning(1, "mapkit: riser edit %d needs an origin\n", e);
            return;
        }
        char buf[96];
        const float angles[3] = { 0.0f, MkJ_Num(c, "yaw", 0.0f), 0.0f };
        const char *zone = MkJ_Str(c, "zone", "mapkit_zone");
        MkE_Add("{\n");
        MkE_Key("classname", MkJ_Str(c, "classname", "actor_zombie_usa_zombie_officeworker"));
        MkE_Key("model", MkJ_Str(c, "model", "c_usa_pent_warroom_worker_body"));
        MkE_KeyVec("origin", origin);
        MkE_KeyVec("angles", angles);
        MkE_Key("spawnflags", "3");
        MkE_Key("count", "9999");
        MkE_Key("script_forcespawn", "1");
        MkE_Key("script_noteworthy", "zombie_spawner");
        MkE_Key("script_disable_bleeder", "1");
        MkE_Key("script_string", "riser");
        Com_sprintf(buf, sizeof(buf), "%s_spawners", zone);
        MkE_Key("targetname", buf);
        MkE_Add("}\n{\n\"classname\" \"script_struct\"\n\"script_noteworthy\" \"find_flesh\"\n");
        Com_sprintf(buf, sizeof(buf), "%s_spawners_rise", zone);
        MkE_Key("targetname", buf);
        MkE_KeyVec("origin", origin);
        MkE_KeyVec("angles", angles);
        MkE_Add("}\n");
    }

    // window: pf82's clip, 6 boards, 6 unbroken sections, repair struct and exterior_goal, moved by MkE_WindowXf.
    // The brush models stay retail (*7..*19: drawn and collided by the base map's own brush models).
    void MkE_Window(int e, int c)
    {
        struct Piece { int model; float o[3]; const char *param; const char *noteworthy; int section; };
        static const Piece pieces[] = {
            { 7, { -900, 1909, 104 }, "unbroken_section", 0, 2118 }, { 8, { -923, 1909, 104 }, "unbroken_section", 0, 2117 },
            { 9, { -944, 1909, 106 }, "unbroken_section", 0, 2119 }, { 10, { -942, 1909, 75 }, "unbroken_section", 0, 2116 },
            { 11, { -921, 1909, 78 }, "unbroken_section", 0, 2121 }, { 12, { -897, 1909, 78 }, "unbroken_section", 0, 2120 },
            { 13, { -917, 1911, 68 }, 0, "clip", 0 },
            { 14, { -914, 1907, 99 }, "repair_board", "1", 2116 }, { 15, { -926, 1909, 70 }, "repair_board", "2", 2117 },
            { 16, { -900, 1909, 73 }, "repair_board", "3", 2118 }, { 17, { -916, 1914, 58 }, "repair_board", "6", 2121 },
            { 18, { -918, 1914, 85 }, "repair_board", "5", 2120 }, { 19, { -918, 1914, 97 }, "repair_board", "4", 2119 },
        };
        const float yaw = MkJ_Num(c, "yaw", 90.0f);
        const float bmAngles[3] = { 0.0f, yaw - 90.0f, 0.0f };
        char name[32], buf[48];
        float p[3];
        Com_sprintf(name, sizeof(name), "mkwin%d", e);
        // mapkit-fix-4c: "open": true = an unboardable entrance (a gap zombies climb through, as retail maps' open
        // windows). Retail's own rule: an exterior_goal with no target has no barrier (blocker_init returns,
        // blockers.gsc:1165; tear_into_building returns, spawner.gsc:769) - so no boards, no clip, no repair struct,
        // no rebuild. The traverse pair (g_mapkit_path.cpp) and the zone's _barriers label stay.
        const bool open = MkJ_Num(c, "open", 0.0f) != 0.0f;
        for (int i = 0; !open && i < (int)(sizeof(pieces) / sizeof(pieces[0])); ++i)
        {
            const Piece &pc = pieces[i];
            MkE_Add("{\n\"classname\" \"script_brushmodel\"\n");
            MkE_Model("model", pc.model);
            MkE_WindowXf(c, pc.o, p);
            MkE_KeyVec("origin", p);
            MkE_KeyVec("angles", bmAngles);
            if (pc.param && !strcmp(pc.param, "unbroken_section"))
            {
                Com_sprintf(buf, sizeof(buf), "%s_sec%d", name, pc.section);
                MkE_Key("targetname", buf);
                MkE_Key("script_parameters", pc.param);
            }
            else
            {
                MkE_Key("targetname", name);
                MkE_Key("script_noteworthy", pc.noteworthy);
                if (pc.param)
                {
                    MkE_Key("script_parameters", pc.param);
                    MkE_Key("script_team", "new_barricade");
                    Com_sprintf(buf, sizeof(buf), "%s_sec%d", name, pc.section);
                    MkE_Key("target", buf);
                }
                else
                    MkE_Key("spawnflags", "1");
            }
            MkE_Add("}\n");
        }
        static const float repair[3] = { -917.0f, 1915.0f, 24.0f }; // E:178
        static const float goal[3] = { -917.0f, 1865.0f, 43.0f };   // E:186
        const float angles[3] = { 0.0f, yaw, 0.0f };
        if (!open)
        {
            MkE_WindowXf(c, repair, p);
            MkE_Add("{\n\"classname\" \"script_struct\"\n\"radius\" \"36\"\n\"height\" \"64\"\n");
            MkE_Key("targetname", name);
            MkE_KeyVec("origin", p);
            MkE_KeyVec("angles", angles);
            MkE_Add("}\n");
        }
        MkE_Add("{\n\"classname\" \"script_struct\"\n\"targetname\" \"exterior_goal\"\n");
        if (!open)
            MkE_Key("target", name);
        // Retail struct_class_init indexes script_noteworthy once. Adding it later in start_zones
        // let deactivate_initial_barrier_goals lower this goal by 10000, but enable_zone could not
        // find it in GetStructArray(<zone>_barriers, script_noteworthy) to restore its origin.
        // Match mapkit_window_zone's point 48 units into the room, 32 above the floor.
        float inside[3];
        if (MkJ_Vec3(MkJ_Child(c, "origin"), inside))
        {
            inside[0] += cosf(yaw * 0.017453292f) * 48.0f;
            inside[1] += sinf(yaw * 0.017453292f) * 48.0f;
            inside[2] += 32.0f;
            for (int zc = s_json[s_editsNode].first; zc >= 0; zc = s_json[zc].next)
            {
                float lo[3], hi[3];
                if (strcmp(MkJ_Str(zc, "kind", ""), "zone") || !MkJ_Vec3(MkJ_Child(zc, "mins"), lo)
                    || !MkJ_Vec3(MkJ_Child(zc, "maxs"), hi))
                    continue;
                if (inside[0] >= lo[0] && inside[0] <= hi[0] && inside[1] >= lo[1] && inside[1] <= hi[1]
                    && inside[2] >= lo[2] && inside[2] <= hi[2])
                {
                    char zoneLabel[128];
                    Com_sprintf(zoneLabel, sizeof(zoneLabel), "%s_barriers", MkJ_Str(zc, "name", "mapkit_zone"));
                    MkE_Key("script_noteworthy", zoneLabel);
                    break;
                }
            }
        }
        MkE_WindowXf(c, goal, p);
        MkE_KeyVec("origin", p);
        MkE_KeyVec("angles", angles);
        MkE_Add("}\n");
    }
}

const char *CM_Mapkit_EntityString(const char *retail)
{
    if (s_mkEntsBuilt)
        return s_mkEnts;
    s_mkEntsBuilt = true;
    s_mkEntsLen = 0;
    MkE_Add("");
    // the retail worldspawn: the first { ... } block
    const char *open = strchr(retail, '{');
    const char *close = open ? strchr(open, '}') : 0;
    if (close)
    {
        const size_t n = close - open + 1;
        char *ws = (char *)malloc(n + 2);
        memcpy(ws, open, n);
        ws[n] = '\n';
        ws[n + 1] = 0;
        MkE_Add(ws);
        free(ws);
    }
    else
    {
        MkE_Add("{\n\"classname\" \"worldspawn\"\n}\n");
    }
    s_mkWindowEdits = 0;
    for (int c = s_json[s_editsNode].first; c >= 0; c = s_json[c].next)
    {
        if (!strcmp(MkJ_Str(c, "kind", ""), "window"))
            ++s_mkWindowEdits;
    }
    if (!s_mkWindowEdits)
        Com_Printf(1, "mapkit: no window edits: spawners are zombie_chaser (straight to find_flesh, no barrier)\n");
    int e = 0;
    int spawns = 0;
    for (int c = s_json[s_editsNode].first; c >= 0; c = s_json[c].next, ++e)
    {
        const char *kind = MkJ_Str(c, "kind", "");
        if (!strcmp(kind, "spawn"))
        {
            MkE_Spawn(e, c);
            ++spawns;
        }
        else if (!strcmp(kind, "ent"))
        {
            MkE_Ent(e, c);
            if (e < MK_MAX_EDITS && s_editBModel[e] >= 0 && s_mkEntsLen >= 2)
            {
                // "bmodel": the entity gets a layout brush model around its box centre
                s_mkEntsLen -= 2;
                s_mkEnts[s_mkEntsLen] = 0;
                MkE_Model("model", s_editBModel[e]);
                MkE_KeyVec("origin", s_editBCenter[e]);
                MkE_Add("}\n");
            }
        }
        else if (!strcmp(kind, "zone"))
            MkE_Zone(e, c);
        else if (!strcmp(kind, "wallbuy"))
            MkE_WallBuy(e, c);
        else if (!strcmp(kind, "door") || !strcmp(kind, "debris"))
            MkE_Blocker(e, c);
        else if (!strcmp(kind, "mysterybox"))
            MkE_MysteryBox(e, c);
        else if (!strcmp(kind, "spawner"))
            MkE_Spawner(e, c);
        else if (!strcmp(kind, "riser"))
            MkE_Riser(e, c);
        else if (!strcmp(kind, "window"))
            MkE_Window(e, c);
    }
    if (!spawns)
        Com_PrintWarning(1, "mapkit: empty base without a spawn edit: players have no spawn point\n");
    Com_Printf(1, "mapkit: entity string built from the layout (%d bytes, %d spawn edits; retail entities off)\n",
        (int)s_mkEntsLen, spawns);
    return s_mkEnts;
}

bool CM_Mapkit_Active()
{
    return s_editsNode >= 0;
}

// mod: "empty": true - the base map's fastfile supplies assets only; its world (draw surfaces, sky, collision,
// static models, pathnodes) is gated off everywhere CM_Mapkit_Empty() is tested, and the layout is the whole map
static bool s_empty;
bool CM_Mapkit_Empty()
{
    return s_empty && s_editsNode >= 0;
}

const char *CM_Mapkit_LayoutName()
{
    return s_layoutName;
}

// mapkit-fix-3 mood lighting: "light" edits tint the flat model lighting of the empty base (R_CalcModelLighting)
// per area. Tint 1 = the kit's flat grey 128; the last light containing a point wins. Kit feature, not from the exe.
#define MK_MAX_LIGHTS 64
struct MkLight
{
    int edit;
    float mins[3];
    float maxs[3];
    float rgb[3];
    volatile int uses; // renderer lighting samples that took this tint
};
static MkLight s_lights[MK_MAX_LIGHTS];
static int s_lightCount;

struct MkLightPreset
{
    const char *name;
    float rgb[3];
    float intensity;
};
static const MkLightPreset s_lightPresets[] = {
    { "dim bunker", { 1.0f, 0.82f, 0.62f }, 0.55f },
    { "red alarm", { 1.0f, 0.2f, 0.14f }, 1.0f },
    { "cold lab", { 0.72f, 0.88f, 1.0f }, 1.15f },
};

static void Mapkit_AddLights()
{
    s_lightCount = 0;
    int e = 0;
    for (int c = s_json[s_editsNode].first; c >= 0; c = s_json[c].next, ++e)
    {
        if (strcmp(MkJ_Str(c, "kind", ""), "light"))
            continue;
        if (s_lightCount >= MK_MAX_LIGHTS)
        {
            Com_PrintWarning(1, "mapkit: light edit %d: more than %d lights, ignored\n", e, MK_MAX_LIGHTS);
            continue;
        }
        MkLight *l = &s_lights[s_lightCount];
        const char *room = MkJ_Str(c, "room", "");
        bool area = false;
        if (*room)
        {
            // the named room/zone/box edit's bounds
            for (int r = s_json[s_editsNode].first; r >= 0 && !area; r = s_json[r].next)
                if (r != c && !strcmp(MkJ_Str(r, "name", ""), room))
                    area = MkJ_Vec3(MkJ_Child(r, "mins"), l->mins) && MkJ_Vec3(MkJ_Child(r, "maxs"), l->maxs);
        }
        else
            area = MkJ_Vec3(MkJ_Child(c, "mins"), l->mins) && MkJ_Vec3(MkJ_Child(c, "maxs"), l->maxs);
        if (!area)
        {
            Com_PrintWarning(1, "mapkit: light edit %d needs mins/maxs or a room naming an edit with bounds\n", e);
            continue;
        }
        const char *presetName = MkJ_Str(c, "preset", "");
        const MkLightPreset *preset = 0;
        for (int p = 0; p < (int)(sizeof(s_lightPresets) / sizeof(s_lightPresets[0])); ++p)
            if (!strcmp(presetName, s_lightPresets[p].name))
                preset = &s_lightPresets[p];
        if (*presetName && !preset)
            Com_PrintWarning(1, "mapkit: light edit %d: unknown preset '%s' (dim bunker, red alarm, cold lab)\n", e, presetName);
        float color[3] = { 1.0f, 1.0f, 1.0f };
        if (preset)
            color[0] = preset->rgb[0], color[1] = preset->rgb[1], color[2] = preset->rgb[2];
        MkJ_Vec3(MkJ_Child(c, "color"), color);
        const float intensity = MkJ_Num(c, "intensity", preset ? preset->intensity : 1.0f);
        for (int i = 0; i < 3; ++i)
        {
            const float v = color[i] * intensity;
            l->rgb[i] = v < 0.0f ? 0.0f : (v > 2.0f ? 2.0f : v);
        }
        l->edit = e;
        l->uses = 0;
        Com_Printf(1, "mapkit: light edit %d tint %.2f %.2f %.2f over (%g %g %g)-(%g %g %g)\n", e, l->rgb[0], l->rgb[1], l->rgb[2],
            l->mins[0], l->mins[1], l->mins[2], l->maxs[0], l->maxs[1], l->maxs[2]);
        ++s_lightCount;
    }
}

int CM_Mapkit_LightAt(const float *p, float *rgb)
{
    for (int k = s_lightCount - 1; k >= 0; --k)
    {
        MkLight *l = &s_lights[k];
        if (p[0] >= l->mins[0] && p[0] <= l->maxs[0] && p[1] >= l->mins[1] && p[1] <= l->maxs[1] && p[2] >= l->mins[2] && p[2] <= l->maxs[2])
        {
            rgb[0] = l->rgb[0], rgb[1] = l->rgb[1], rgb[2] = l->rgb[2];
            return k;
        }
    }
    return -1;
}

void CM_Mapkit_LightUsed(int light)
{
    if (light >= 0 && light < s_lightCount)
        ++s_lights[light].uses;
}

int CM_Mapkit_LightUses(int edit)
{
    for (int k = 0; k < s_lightCount; ++k)
        if (s_lights[k].edit == edit)
            return s_lights[k].uses;
    return -1;
}

namespace
{
    // mapkit-fix-4g: a frame around every boarded kit window, as retail's door frame around pf82-style panes: the kit
    // cuts a bare wall gap and the re-placed pf82 panels float in it with their ragged edges showing
    // (notes/mapkit-fix-4f). Four non-solid boxes (contents 0, like door/debris boxes) fill the gap from its edges to
    // FRAME_OVERLAP inside the panels' outline, lap FRAME_LIP onto both wall faces and stand FRAME_PROUD off them.
    // The outline is the union of the unbroken sections' brush models (*7..*12) in retail coordinates, moved like
    // every window piece (MkE_WindowXf). MEASURED: lip / proud / overlap are chosen from the photos, not retail values.
    const float FRAME_LIP = 4.0f, FRAME_PROUD = 1.5f, FRAME_OVERLAP = 8.0f;

    bool Mapkit_PanelOutline(int c, float *lo, float *hi) // lo/hi: [along, z] in world space, along = the wall's axis
    {
        static const float sectionOrigin[6][3] = {
            { -900, 1909, 104 }, { -923, 1909, 104 }, { -944, 1909, 106 }, { -942, 1909, 75 }, { -921, 1909, 78 }, { -897, 1909, 78 } };
        if (!cm.cmodels || cm.numSubModels <= 12)
            return false;
        float rlo[3] = { 1e9f, 1e9f, 1e9f }, rhi[3] = { -1e9f, -1e9f, -1e9f };
        for (int k = 0; k < 6; ++k)
        {
            const cmodel_t *m = &cm.cmodels[7 + k];
            const float *o = sectionOrigin[k];
            // bounds around the entity origin (model space) unless they already contain it (world space)
            const bool world = o[0] >= m->mins[0] - 1 && o[0] <= m->maxs[0] + 1 && o[2] >= m->mins[2] - 1 && o[2] <= m->maxs[2] + 1;
            for (int i = 0; i < 3; ++i)
            {
                const float a = m->mins[i] + (world ? 0.0f : o[i]), b = m->maxs[i] + (world ? 0.0f : o[i]);
                rlo[i] = a < rlo[i] ? a : rlo[i];
                rhi[i] = b > rhi[i] ? b : rhi[i];
            }
        }
        float p0[3], p1[3];
        MkE_WindowXf(c, rlo, p0);
        MkE_WindowXf(c, rhi, p1);
        const float yaw = MkJ_Num(c, "yaw", 90.0f);
        const int along = (fabsf(sinf(yaw * 0.017453292f)) > 0.5f) ? 0 : 1; // yaw 90/270: the wall runs along x
        lo[0] = fminf(p0[along], p1[along]), hi[0] = fmaxf(p0[along], p1[along]);
        lo[1] = fminf(p0[2], p1[2]), hi[1] = fmaxf(p0[2], p1[2]);
        return true;
    }

    void Mapkit_AddWindowFrames()
    {
        int e = 0;
        for (int c = s_json[s_editsNode].first; c >= 0; c = s_json[c].next, ++e)
        {
            float o[3];
            if (strcmp(MkJ_Str(c, "kind", ""), "window") || MkJ_Num(c, "open", 0.0f) != 0.0f || !MkJ_Vec3(MkJ_Child(c, "origin"), o))
                continue;
            const MkGap *gap = 0;
            for (int g = 0; g < s_gapCount && !gap; ++g)
            {
                const MkGap &gp = s_gaps[g];
                const int along = gp.axis ^ 1;
                if (o[gp.axis] >= gp.slab[0] - 1 && o[gp.axis] <= gp.slab[1] + 1 && o[along] > gp.along[0] && o[along] < gp.along[1])
                    gap = &gp;
            }
            if (!gap)
            {
                Com_Printf(1, "mapkit: window edit %d is not in a room gap: no frame\n", e);
                continue;
            }
            const int axis = gap->axis, along = axis ^ 1;
            float inLo[2] = { gap->along[0], gap->z[0] }, inHi[2] = { gap->along[1], gap->z[1] };
            float pLo[2], pHi[2];
            if (Mapkit_PanelOutline(c, pLo, pHi))
            {
                for (int i = 0; i < 2; ++i)
                {
                    inLo[i] = fmaxf(inLo[i], fminf(pLo[i] + FRAME_OVERLAP, inHi[i]));
                    inHi[i] = fminf(inHi[i], fmaxf(pHi[i] - FRAME_OVERLAP, inLo[i]));
                }
            }
            const float outLo[2] = { gap->along[0] - FRAME_LIP, gap->z[0] - FRAME_LIP }, outHi[2] = { gap->along[1] + FRAME_LIP, gap->z[1] + FRAME_LIP };
            // jambs full height, sill and head between them
            const float rect[4][4] = {
                { outLo[0], outLo[1], inLo[0], outHi[1] }, { inHi[0], outLo[1], outHi[0], outHi[1] },
                { inLo[0], outLo[1], inHi[0], inLo[1] }, { inLo[0], inHi[1], inHi[0], outHi[1] } };
            const float yaw = MkJ_Num(c, "yaw", 90.0f) * 0.017453292f;
            const float light[3] = { o[0] + cosf(yaw) * 16.0f, o[1] + sinf(yaw) * 16.0f, (gap->z[0] + gap->z[1]) * 0.5f };
            const char *mat = MkJ_Str(c, "frameMaterial", "mc/pent_art_wall_wood07_dark");
            for (int r = 0; r < 4; ++r)
            {
                float mins[3], maxs[3];
                mins[axis] = gap->slab[0] - FRAME_PROUD, maxs[axis] = gap->slab[1] + FRAME_PROUD;
                mins[along] = rect[r][0], maxs[along] = rect[r][2];
                mins[2] = rect[r][1], maxs[2] = rect[r][3];
                if (maxs[along] - mins[along] < 0.5f || maxs[2] - mins[2] < 0.5f)
                    continue;
                const int before = s_boxCount;
                Mapkit_AddBox(mins, maxs, mat, 0, 128.0f, e);
                if (s_boxCount > before)
                    s_boxes[before].lightOrigin[0] = light[0], s_boxes[before].lightOrigin[1] = light[1], s_boxes[before].lightOrigin[2] = light[2];
            }
            Com_Printf(1, "mapkit: window edit %d frame: gap along %g..%g z %g..%g, opening %g..%g z %g..%g\n", e,
                gap->along[0], gap->along[1], gap->z[0], gap->z[1], inLo[0], inHi[0], inLo[1], inHi[1]);
        }
    }
}

void CM_Mapkit_Load(const char *mapName)
{
    s_lightCount = 0;
    s_jsonCount = 0;
    s_strUsed = 0;
    s_parseError = 0;
    s_boxCount = 0;
    s_gapCount = 0;
    s_editsNode = -1;
    s_layoutName[0] = 0;
    s_empty = false;
    s_mkEntsBuilt = false;

    const dvar_s *layout = _Dvar_RegisterString("bo1_mod_mapkit", (char *)"", 0,
        "mod: mapkit layout name (mods/mapkit/layouts/<name>.json: a base map + edits); read when a map loads, empty = off");
    if (!layout || !layout->current.string || !*layout->current.string)
        return;

    char path[128];
    Com_sprintf(path, sizeof(path), "layouts/%s.json", layout->current.string);
    void *buf = 0;
    int len = FS_ReadFile(path, &buf);
    if (len <= 0 || !buf)
    {
        Com_PrintWarning(1, "mapkit: cannot read %s (run with +set fs_game mods/mapkit)\n", path);
        return;
    }
    char *text = (char *)malloc(len + 1);
    memcpy(text, buf, len);
    text[len] = 0;
    FS_FreeFile(buf);

    const char *p = text;
    int root = MkJ_ParseValue(p);
    free(text);
    if (s_parseError || root < 0 || s_json[root].type != MKJ_OBJ)
    {
        Com_PrintWarning(1, "mapkit: %s: parse error: %s\n", path, s_parseError ? s_parseError : "the root is not an object");
        s_jsonCount = 0;
        return;
    }
    const char *base = MkJ_Str(root, "base", "");
    if (!*base || !strstr(mapName, base))
    {
        Com_Printf(1, "mapkit: layout %s is for base '%s', not %s: not applied\n", path, base, mapName);
        return;
    }
    strncpy(s_layoutName, layout->current.string, sizeof(s_layoutName) - 1);
    s_empty = MkJ_Num(root, "empty", 0.0f) != 0.0f;
    s_editsNode = MkJ_Child(root, "edits");
    if (s_editsNode >= 0 && s_json[s_editsNode].type != MKJ_ARR)
        s_editsNode = -1;
    if (s_editsNode < 0)
    {
        Com_PrintWarning(1, "mapkit: %s has no edits array\n", path);
        return;
    }
    int e = 0;
    for (int c = s_json[s_editsNode].first; c >= 0; c = s_json[c].next, ++e)
    {
        const char *kind = MkJ_Str(c, "kind", "");
        if (!strcmp(kind, "box") || (s_empty && (!strcmp(kind, "door") || !strcmp(kind, "debris"))))
        {
            float mins[3], maxs[3];
            if (MkJ_Vec3(MkJ_Child(c, "mins"), mins) && MkJ_Vec3(MkJ_Child(c, "maxs"), maxs))
                Mapkit_AddBox(mins, maxs, MkJ_Str(c, "material", "mc/pent_art_wall_wood07_dark"),
                    strcmp(kind, "box") ? 0 : Mapkit_Contents(MkJ_Str(c, "contents", "solid")), MkJ_Num(c, "texScale", 128.0f), e);
            else
                Com_PrintWarning(1, "mapkit: box edit %d needs mins and maxs\n", e);
        }
        else if (!strcmp(kind, "room"))
        {
            Mapkit_AddRoom(e, c);
        }
        else if (!strcmp(kind, "stairs"))
        {
            float lo[3], hi[3];
            int s = 0;
            for (; CM_Mapkit_StairsStep(e, s, lo, hi); ++s)
                Mapkit_AddBox(lo, hi, MkJ_Str(c, "material", "mc/rus_metal_panel03"), Mapkit_Contents("solid"),
                    MkJ_Num(c, "texScale", 128.0f), e);
            if (!s)
                Com_PrintWarning(1, "mapkit: stairs edit %d needs mins and maxs (maxs.z = top tread)\n", e);
            // lo/hi = the top step: its box spans mins.z..maxs.z, so the rise per step is that height / s
            const int axis = MkJ_Str(c, "up", "+x")[1] == 'y' ? 1 : 0;
            if (s && (hi[2] - lo[2]) / s > 18.0f)
                Com_PrintWarning(1, "mapkit: stairs edit %d rises %g per step (max 18); zombies cannot climb it\n", e, (hi[2] - lo[2]) / s);
            if (s && hi[axis] - lo[axis] < 32.0f)
                Com_PrintWarning(1, "mapkit: stairs edit %d treads are %g deep (min 32, use 40+); nodes on them go bad\n", e, hi[axis] - lo[axis]);
        }
    }
    Mapkit_AddWindowFrames();
    Mapkit_AddLights();
    // mapkit-fix-4b: CM_Mapkit_WarnPurchaseOverlap runs in CM_Mapkit_AddMachineClips (after snap:"wall" is resolved)
    Com_Printf(1, "mapkit: layout %s on %s: %d edits, %d boxes\n", s_layoutName, mapName, e, s_boxCount);
}

int CM_Mapkit_BoxCount()
{
    return s_boxCount;
}

const MapkitBox *CM_Mapkit_GetBox(int index)
{
    return index >= 0 && index < s_boxCount ? &s_boxes[index] : 0;
}

bool CM_Mapkit_IsSubmodel(unsigned int index)
{
    return CM_Mapkit_Empty() && s_bmCmodels && cm.cmodels == s_bmCmodels
        && index >= s_retailNumSub && index < cm.numSubModels;
}

void CM_Mapkit_SetVisible(int edit, bool visible)
{
    for (int i = 0; i < s_boxCount; ++i)
        if (s_boxes[i].editIndex == edit)
            s_boxes[i].hidden = !visible;
}

static bool CM_Mapkit_BoxTouches(const traceWork_t *tw, const cbrush_t *b)
{
    for (int i = 0; i < 3; ++i)
    {
        if (tw->bounds[0].vec.v[i] - tw->size.vec.v[i] > b->maxs[i] + 1.0f
            || tw->bounds[1].vec.v[i] + tw->size.vec.v[i] < b->mins[i] - 1.0f)
            return false;
    }
    return true;
}

// zombies: called at the end of the world (model 0) branch of CM_Trace, like CM_TraceThroughTempBrush
void CM_Mapkit_TraceThrough(const traceWork_t *tw, trace_t *trace)
{
    for (int i = 0; i < s_boxCount && trace->fraction != 0.0f; ++i)
    {
        const cbrush_t *b = &s_brushes[i];
        if ((tw->contents & b->contents) != 0 && CM_Mapkit_BoxTouches(tw, b))
            CM_TraceThroughBrush(tw, b, trace);
    }
}

void CM_Mapkit_TestIn(const traceWork_t *tw, trace_t *trace)
{
    for (int i = 0; i < s_boxCount && !trace->allsolid; ++i)
    {
        const cbrush_t *b = &s_brushes[i];
        if ((tw->contents & b->contents) != 0 && CM_Mapkit_BoxTouches(tw, b))
            CM_TestBoxInBrush(tw, b, trace);
    }
}

int CM_Mapkit_PointContents(const float *p)
{
    int contents = 0;
    for (int i = 0; i < s_boxCount; ++i)
    {
        const cbrush_t *b = &s_brushes[i];
        if (p[0] > b->mins[0] && p[0] < b->maxs[0] && p[1] > b->mins[1] && p[1] < b->maxs[1]
            && p[2] > b->mins[2] && p[2] < b->maxs[2])
            contents |= b->contents;
    }
    return contents;
}

const cbrush_t *CM_Mapkit_GetBrush(int index)
{
    return &s_brushes[index];
}

bool CM_Mapkit_IsBrush(const void *brush)
{
    return brush >= (const void *)s_brushes && brush < (const void *)(s_brushes + s_boxCount);
}

// returns a hit number past cm.numBrushes (the sight-trace caches only reuse hits below it)
int CM_Mapkit_SightTrace(const traceWork_t *tw, trace_t *trace)
{
    for (int i = 0; i < s_boxCount; ++i)
    {
        const cbrush_t *b = &s_brushes[i];
        if ((tw->contents & b->contents) != 0 && CM_Mapkit_BoxTouches(tw, b)
            && CM_SightTraceThroughBrush(tw, b, cm.numBrushes, trace))
            return cm.numBrushes + 1;
    }
    return 0;
}

int CM_Mapkit_EditCount()
{
    return s_editsNode >= 0 ? s_json[s_editsNode].count : 0;
}

int CM_Mapkit_GetField(int edit, const char *key, const char **str, float *vec)
{
    int node = MkJ_Child(s_editsNode, va("%d", edit));
    if (node < 0 || !key)
        return MKF_NONE;
    if (!strcmp(key, "_clipmins") || !strcmp(key, "_clipmaxs")) // mapkit-fix-4b: a machine edit's clip box
    {
        float mins[3], maxs[3];
        if (!CM_Mapkit_MachineClip(edit, mins, maxs))
            return MKF_NONE;
        for (int k = 0; k < 3; ++k)
            vec[k] = key[6] == 'i' ? mins[k] : maxs[k];
        return MKF_VECTOR;
    }
    if (!strcmp(key, "_spot") || !strcmp(key, "_spotyaw")) // mapkit-fix-4b: snap:"wall" as resolved by the machine clips
    {
        float o[3], yaw;
        if (!CM_Mapkit_ResolvedSpot(edit, o, &yaw))
            return MKF_NONE;
        if (key[5])
        {
            vec[0] = yaw;
            return MKF_NUMBER;
        }
        vec[0] = o[0];
        vec[1] = o[1];
        vec[2] = o[2];
        return MKF_VECTOR;
    }
    if (*key == '#')
    {
        node = *++key ? MkJ_Path(node, key) : node;
        if (node < 0 || (s_json[node].type != MKJ_ARR && s_json[node].type != MKJ_OBJ))
            return MKF_NONE;
        vec[0] = (float)s_json[node].count;
        return MKF_NUMBER;
    }
    node = MkJ_Path(node, key);
    if (node < 0)
        return MKF_NONE;
    switch (s_json[node].type)
    {
    case MKJ_STR:
        if (str)
            *str = s_json[node].str;
        return MKF_STRING;
    case MKJ_NUM:
    case MKJ_BOOL:
        vec[0] = (float)s_json[node].num;
        return MKF_NUMBER;
    case MKJ_ARR:
        return MkJ_Vec3(node, vec) ? MKF_VECTOR : MKF_NONE;
    default:
        return MKF_NONE;
    }
}
