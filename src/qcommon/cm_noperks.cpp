// mod: noperks (dvar noperks 0/1, default 0 = retail, nothing here runs). The perk machines' collision is not in
// the scripts on most machines: it is baked into the bsp as world clip brushes (Five: Juggernog, Speed Cola, Double
// Tap; Quick Revive's clip is a script_brushmodel, the script deletes it). At map load every brush that lies wholly
// inside a box around a perk machine (a script_model targeted by a "zombie_vending" trigger in the map entities)
// gets contents 0, so traces, sight traces and point contents skip it: players and zombies walk through where the
// machine stood. Triggers (contents 0x40000000) are left alone. The count goes to dvar noperks_clip for the
// script's log line; the script half is maps/noperks/_noperks.gsc (mods/zinfo, copied into mods/horde).
#include "cm_mapkit.h"
#include "cm_trace.h"
#include "cm_load.h"
#include "cm_load_obj.h"
#include "common.h"
#include <universal/dvar.h>

#include <string.h>
#include <stdlib.h>

#define NP_MAX_NAMES 32
#define NP_MAX_EDITS 256

// the edit is made in the zone's own clipmap memory: remember it, and undo it before the next load of the same map
// (a devmap of the loaded map can keep the zone), so noperks 0 after noperks 1 gives the retail collision back
static cbrush_t *s_npBrushes;
static unsigned int s_npNumBrushes;
static char s_npMap[64];
static int s_npIndex[NP_MAX_EDITS];
static int s_npContents[NP_MAX_EDITS];
static int s_npCount;
// the removed machines' origins on the loaded map (0 with noperks 0), for CM_NoPerks_NearMachine
static float s_npOrigins[NP_MAX_NAMES][3];
static int s_npOriginCount;

bool CM_NoPerks_NearMachine(const float *origin, float radius)
{
    for (int m = 0; m < s_npOriginCount; ++m)
    {
        const float d[3] = { origin[0] - s_npOrigins[m][0], origin[1] - s_npOrigins[m][1], origin[2] - s_npOrigins[m][2] };
        if (d[0] * d[0] + d[1] * d[1] + d[2] * d[2] <= radius * radius)
            return true;
    }
    return false;
}

struct NpEnt
{
    char classname[64];
    char targetname[64];
    char target[64];
    float origin[3];
    bool hasOrigin;
};

static const char *Np_ReadQuoted(const char *p, char *out, int size)
{
    while (*p && *p != '"' && *p != '}')
        ++p;
    if (*p != '"')
        return 0;
    ++p;
    int n = 0;
    while (*p && *p != '"')
    {
        if (n < size - 1)
            out[n++] = *p;
        ++p;
    }
    out[n] = 0;
    return *p ? p + 1 : 0;
}

// next { ... } block of the entity string into e; returns the text after it, 0 at the end
static const char *Np_NextEnt(const char *p, NpEnt *e)
{
    memset(e, 0, sizeof(*e));
    while (*p && *p != '{')
        ++p;
    if (!*p)
        return 0;
    ++p;
    for (;;)
    {
        while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n')
            ++p;
        if (!*p)
            return 0;
        if (*p == '}')
            return p + 1;
        char key[64], value[256];
        p = Np_ReadQuoted(p, key, sizeof(key));
        if (!p)
            return 0;
        p = Np_ReadQuoted(p, value, sizeof(value));
        if (!p)
            return 0;
        if (!strcmp(key, "classname"))
            strncpy(e->classname, value, sizeof(e->classname) - 1);
        else if (!strcmp(key, "targetname"))
            strncpy(e->targetname, value, sizeof(e->targetname) - 1);
        else if (!strcmp(key, "target"))
            strncpy(e->target, value, sizeof(e->target) - 1);
        else if (!strcmp(key, "origin"))
        {
            e->hasOrigin = true;
            char *q = value;
            for (int k = 0; k < 3; ++k)
                e->origin[k] = (float)strtod(q, &q);
        }
    }
}

void CM_NoPerks_Apply(const char *name)
{
    // undo the last edit if this is the same clipmap memory of the same map
    if (s_npCount && s_npBrushes == cm.brushes && s_npNumBrushes == cm.numBrushes && !strcmp(s_npMap, name))
    {
        for (int i = 0; i < s_npCount; ++i)
            cm.brushes[s_npIndex[i]].contents = s_npContents[i];
    }
    s_npCount = 0;
    s_npBrushes = 0;
    s_npOriginCount = 0;

    const dvar_s *noperks = _Dvar_RegisterBool("noperks", 0, 0,
        "mod: no perk machines (their triggers, models, fx and collision are removed at map load); 0 = retail");
    const dvar_s *clipCount = _Dvar_RegisterInt("noperks_clip", 0, 0, 100000, 0,
        "mod: world clip brushes the noperks load removed on this map (read by maps/noperks/_noperks.gsc)");
    Dvar_SetInt((dvar_s *)clipCount, 0);
    if (!noperks || !noperks->current.enabled || !cm.mapEnts || !cm.mapEnts->entityString)
        return;

    // the perk triggers' targets
    char names[NP_MAX_NAMES][64];
    int nameCount = 0;
    NpEnt e;
    for (const char *p = cm.mapEnts->entityString; (p = Np_NextEnt(p, &e)) != 0;)
    {
        if (!strcmp(e.targetname, "zombie_vending") && e.target[0] && nameCount < NP_MAX_NAMES)
            strcpy(names[nameCount++], e.target);
    }
    // the machines: script_models with one of those targetnames
    float origins[NP_MAX_NAMES][3];
    int machineCount = 0;
    for (const char *p = cm.mapEnts->entityString; (p = Np_NextEnt(p, &e)) != 0;)
    {
        if (strcmp(e.classname, "script_model") || !e.hasOrigin || machineCount >= NP_MAX_NAMES)
            continue;
        for (int i = 0; i < nameCount; ++i)
        {
            if (!strcmp(e.targetname, names[i]))
            {
                origins[machineCount][0] = e.origin[0];
                origins[machineCount][1] = e.origin[1];
                origins[machineCount][2] = e.origin[2];
                ++machineCount;
                break;
            }
        }
    }

    memcpy(s_npOrigins, origins, sizeof(origins[0]) * machineCount);
    s_npOriginCount = machineCount;
    s_npBrushes = cm.brushes;
    s_npNumBrushes = cm.numBrushes;
    strncpy(s_npMap, name, sizeof(s_npMap) - 1);
    s_npMap[sizeof(s_npMap) - 1] = 0;
    for (int m = 0; m < machineCount; ++m)
    {
        // a machine is about 40 x 60 x 110 around its origin (on the floor); the search box is wider for any yaw
        const float *o = origins[m];
        const float lo[3] = { o[0] - 48.0f, o[1] - 48.0f, o[2] - 8.0f };
        const float hi[3] = { o[0] + 48.0f, o[1] + 48.0f, o[2] + 160.0f };
        // the machine's footprint: the union of the collision brushes (solid, player or monster clip) in the box that
        // stand on the machine's own column (contain the origin's x,y and overlap 8..64 up). The search box alone also
        // held walls behind a machine, a staircase's bullet clip on the far side of a wall and a shelf above one
        // (Der Riese); only brushes wholly inside the footprint (+1) go. No footprint (the collision is a
        // script_brushmodel the script deletes, or none): nothing is removed; the script's self-test checks either way.
        float flo[3] = { 0, 0, 0 }, fhi[3] = { 0, 0, 0 };
        bool haveFoot = false;
        for (unsigned int b = 0; b < cm.numBrushes; ++b)
        {
            const cbrush_t *brush = &cm.brushes[b];
            if (!(brush->contents & 0x30001) || (brush->contents & 0x40000000) != 0)
                continue;
            if (brush->mins[0] < lo[0] || brush->mins[1] < lo[1] || brush->mins[2] < lo[2]
                || brush->maxs[0] > hi[0] || brush->maxs[1] > hi[1] || brush->maxs[2] > hi[2])
                continue;
            if (brush->mins[0] > o[0] || brush->maxs[0] < o[0] || brush->mins[1] > o[1] || brush->maxs[1] < o[1]
                || brush->mins[2] >= o[2] + 64.0f || brush->maxs[2] <= o[2] + 8.0f)
                continue;
            for (int k = 0; k < 3; ++k)
            {
                flo[k] = haveFoot && flo[k] < brush->mins[k] ? flo[k] : brush->mins[k];
                fhi[k] = haveFoot && fhi[k] > brush->maxs[k] ? fhi[k] : brush->maxs[k];
            }
            haveFoot = true;
        }
        if (haveFoot)
            Com_Printf(1, "noperks:   footprint rel (%g %g %g)..(%g %g %g)\n", flo[0] - o[0], flo[1] - o[1],
                flo[2] - o[2], fhi[0] - o[0], fhi[1] - o[1], fhi[2] - o[2]);
        int removed = 0;
        for (unsigned int b = 0; b < cm.numBrushes && s_npCount < NP_MAX_EDITS; ++b)
        {
            cbrush_t *brush = &cm.brushes[b];
            if (!brush->contents || (brush->contents & 0x40000000) != 0)
                continue;
            if (brush->mins[0] < lo[0] || brush->mins[1] < lo[1] || brush->mins[2] < lo[2]
                || brush->maxs[0] > hi[0] || brush->maxs[1] > hi[1] || brush->maxs[2] > hi[2])
            {
                // kept (reaches out of the box: walls, floor, ceiling); listed when it touches the machine's own
                // column (origin +-16, 1..100 up) so a self-test FAIL can be read off the log
                if (brush->mins[0] < o[0] + 16.0f && brush->maxs[0] > o[0] - 16.0f
                    && brush->mins[1] < o[1] + 16.0f && brush->maxs[1] > o[1] - 16.0f
                    && brush->mins[2] < o[2] + 100.0f && brush->maxs[2] > o[2] + 1.0f)
                    Com_Printf(1, "noperks:   kept brush %u contents 0x%x (%g %g %g)..(%g %g %g)\n", b, brush->contents,
                        brush->mins[0], brush->mins[1], brush->mins[2], brush->maxs[0], brush->maxs[1], brush->maxs[2]);
                continue;
            }
            // listed relative to the machine origin, so the log shows which lie in the footprint
            const bool inFoot = haveFoot && brush->mins[0] >= flo[0] - 1.0f && brush->mins[1] >= flo[1] - 1.0f
                && brush->mins[2] >= flo[2] - 1.0f && brush->maxs[0] <= fhi[0] + 1.0f && brush->maxs[1] <= fhi[1] + 1.0f
                && brush->maxs[2] <= fhi[2] + 1.0f;
            Com_Printf(1, "noperks:   %s brush %u contents 0x%x rel (%g %g %g)..(%g %g %g)\n",
                inFoot ? "off" : "kept (outside footprint)", b, brush->contents,
                brush->mins[0] - o[0], brush->mins[1] - o[1], brush->mins[2] - o[2],
                brush->maxs[0] - o[0], brush->maxs[1] - o[1], brush->maxs[2] - o[2]);
            if (!inFoot)
                continue;
            s_npIndex[s_npCount] = (int)b;
            s_npContents[s_npCount] = brush->contents;
            ++s_npCount;
            brush->contents = 0;
            ++removed;
        }
        Com_Printf(1, "noperks: machine at (%g %g %g): %d clip brushes off\n", origins[m][0], origins[m][1],
            origins[m][2], removed);
    }
    Dvar_SetInt((dvar_s *)clipCount, s_npCount);
    Com_Printf(1, "noperks: %d perk triggers, %d machines, %d clip brushes off (%s)\n", nameCount, machineCount,
        s_npCount, name);
}
