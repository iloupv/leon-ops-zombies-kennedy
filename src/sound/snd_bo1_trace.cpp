#include <database/db_registry.h>
#include "snd_bo1_trace.h"

#include <cstdarg>
#include <cstring>
#include "snd.h"
#include "snd_bank.h"
#include "snd_utils.h"
#include "snd_globals.h"
#include <qcommon/common.h>
#include <qcommon/threads.h>
#include <universal/dvar.h>
#include <win32/win_common.h>
#include <win32/win_shared.h>
#include <Windows.h>

// zombies (a1): see snd_bo1_trace.h. Called from the main (client) and server threads, serialised by one lock;
// nothing here feeds back into game or sound state.

extern SndBank *g_snd_banks[SND_MAX_BANKS];
extern unsigned int g_snd_bankCount;

namespace
{
const dvar_s *s_audio;
const dvar_s *s_nodevLength; // bo1_snd_nodevice_length (see the header)
const dvar_s *s_voices; // zombies (L16): bo1_audio_voices
bool s_suppress; // main thread (CG_PumpEntityLoopSound) only
SRWLOCK s_lock = SRWLOCK_INIT;

struct Lock
{
    Lock() { AcquireSRWLockExclusive(&s_lock); }
    ~Lock() { ReleaseSRWLockExclusive(&s_lock); }
};

// hash -> name for requests made by name (SND_FindAliasId), so missing aliases print their name
struct NameSlot
{
    unsigned int id;
    char name[64];
};
NameSlot s_names[8192];

struct LoopSlot
{
    unsigned int id;
    int ent;
    unsigned int lastMs;
};
LoopSlot s_loops[512];

const char *NameOf(unsigned int id, const snd_alias_list_t *list, char *buf, size_t size)
{
    if (list && list->name)
        return list->name;
    for (unsigned int i = 0, h = id & 8191; i < 8192; ++i, h = (h + 1) & 8191)
    {
        if (!s_names[h].id)
            break;
        if (s_names[h].id == id)
            return s_names[h].name;
    }
    _snprintf(buf, size, "#%08x", id);
    buf[size - 1] = 0;
    return buf;
}

int s_svTime; // level.time of the last server frame (G_RunFrame)
int s_clTime; // cg time of the last client frame (CG_DrawActiveFrame)

// " lt=<level ms>[ 2d|3d][ nofile]": the retail recorder (retail research engine/probe/record_session.py, snd.tsv) samples
// the playing CHANNELS with the recording's levelTime; an alias whose head has no SoundFile never takes a channel there.
// " dmax=<distReverbMax>" on 3d aliases: SND_PlaySoundAlias refuses a positioned play (and every per-frame loop
// emitter update) while the nearest listener is farther than that (SP SND_PlayAliasList 0x005bb9a0, alias+0x3c).
// Headless runs have no listener, so tools/audio_parity.py applies that cull offline with the player's eye.
void Shape(char *buf, size_t size, const char *side, const snd_alias_list_t *list)
{
    const snd_alias_t *head = list && list->count ? list->head : 0;
    char dmax[24] = "";
    if (head && (head->flags & 2))
        _snprintf(dmax, sizeof(dmax), " dmax=%d", (int)head->distReverbMax);
    dmax[sizeof(dmax) - 1] = 0;
    _snprintf(buf, size, " lt=%d%s%s%s", side[0] == 's' ? s_svTime : s_clTime,
        !head ? "" : (head->flags & 2) ? " 3d" : " 2d", dmax, head && !head->soundFile ? " nofile" : "");
    buf[size - 1] = 0;
}

void Print(const char *side, const char *api, unsigned int id, int ent, const float *origin, const char *extra)
{
    const snd_alias_list_t *list = id ? SND_BankAliasLookup(id) : 0;
    const char *outcome = !id ? "no_alias"
        : (!list || !list->count) ? "alias_missing"
        : SND_Active() ? "queued" : "resolved_no_device";
    char buf[16];
    char shape[64];
    const float zero[3] = { 0.0f, 0.0f, 0.0f };
    if (!origin)
        origin = zero;
    Shape(shape, sizeof(shape), side, list);
    Com_Printf(16, "BO1_AUDIO %s %s alias=%s ent=%d org=(%.0f %.0f %.0f) t=%u -> %s%s%s%s\n",
        side, api, NameOf(id, list, buf, sizeof(buf)), ent, origin[0], origin[1], origin[2], Sys_Milliseconds(),
        outcome, extra ? " " : "", extra ? extra : "", shape);
    // The secondary alias chain a client play starts with it (snd.cpp SND_PlaySoundAlias's
    // list->head->secondaryname recursion; SP SND_PlayAliasList 0x005BB9A0 + SND_CheckSecondaryAlias 0x008AFBC0,
    // at most 10 links). Logged as "secondary" lines so the census names what the retail channel array would hold.
    if (side[0] != 'c' || !list || !list->count)
        return;
    const snd_alias_t *link = list->head;
    for (int depth = 0; depth < 10 && link && link->secondaryname; ++depth)
    {
        const snd_alias_list_t *next = SND_BankAliasLookup(SND_HashName(link->secondaryname));
        Shape(shape, sizeof(shape), side, next);
        Com_Printf(16, "BO1_AUDIO %s secondary alias=%s ent=%d org=(%.0f %.0f %.0f) t=%u -> %s of=%s%s\n",
            side, link->secondaryname, ent, origin[0], origin[1], origin[2], Sys_Milliseconds(),
            next && next->count ? "resolved_no_device" : "alias_missing", link->name ? link->name : "?", shape);
        link = next && next->count ? next->head : 0;
    }
}
}

int SND_BO1TraceVoiceInterval()
{
    return s_voices ? s_voices->current.integer : 0;
}

void SND_BO1TraceInit()
{
    if (!s_audio)
        s_audio = _Dvar_RegisterBool("bo1_audio", false, 0,
            "Log every sound request (alias, entity, origin, outcome) - measurement only");
    if (!s_voices)
        s_voices = _Dvar_RegisterInt("bo1_audio_voices", 0, 0, 100000, 0,
            "Log each live voice's position, listener, falloff, levels and pan at most every N ms of sound time - measurement only");
    if (!s_nodevLength)
        s_nodevLength = _Dvar_RegisterBool("bo1_snd_nodevice_length", false, 0,
            "Without a sound device, report a notify sound's length as its voice would (headless measurement)");
}

bool SND_BO1NoDeviceLengthEnabled()
{
    return s_nodevLength && s_nodevLength->current.enabled;
}

bool SND_BO1NoDeviceAliasExists(const char *name)
{
    if (!name || !*name || SND_Active() || !SND_BO1NoDeviceLengthEnabled())
        return false;
    const snd_alias_list_t *list = SND_BankAliasLookup(SND_HashName(name));
    return list && list->count;
}

void SND_BO1TraceSetTime(bool client, int ms)
{
    (client ? s_clTime : s_svTime) = ms;
}

bool SND_BO1TraceEnabled()
{
    return s_audio && s_audio->current.enabled;
}

void SND_BO1TraceName(unsigned int id, const char *name)
{
    if (!id || !name || !SND_BO1TraceEnabled())
        return;
    Lock lock;
    for (unsigned int i = 0, h = id & 8191; i < 8192; ++i, h = (h + 1) & 8191)
    {
        if (s_names[h].id == id)
            return;
        if (!s_names[h].id)
        {
            I_strncpyz(s_names[h].name, name, sizeof(s_names[h].name));
            s_names[h].id = id;
            return;
        }
    }
}

void SND_BO1Trace(const char *side, const char *api, unsigned int id, int ent, const float *origin, const char *extra)
{
    if ((s_suppress && Sys_IsMainThread()) || !SND_BO1TraceEnabled())
        return;
    Lock lock;
    Print(side, api, id, ent, origin, extra);
}

void SND_BO1TraceLoop(const char *side, unsigned int id, int ent, const float *origin)
{
    if (!id || !SND_BO1TraceEnabled())
        return;
    Lock lock;
    const unsigned int now = Sys_Milliseconds();
    LoopSlot *free = 0, *oldest = &s_loops[0];
    for (LoopSlot &slot : s_loops)
    {
        if (slot.id == id && slot.ent == ent)
        {
            const bool restarted = now - slot.lastMs >= 1000;
            slot.lastMs = now;
            if (restarted)
                Print(side, "loop_ent", id, ent, origin, 0);
            return;
        }
        if (!slot.id && !free)
            free = &slot;
        if (slot.lastMs < oldest->lastMs)
            oldest = &slot;
    }
    LoopSlot *slot = free ? free : oldest;
    slot->id = id;
    slot->ent = ent;
    slot->lastMs = now;
    Print(side, "loop_ent", id, ent, origin, 0);
}

void SND_BO1TraceSuppress(bool suppress)
{
    s_suppress = suppress;
}

bool SND_BO1TraceSuppressed()
{
    return s_suppress;
}

void SND_BO1TraceBanks(const char *when)
{
    if (!SND_BO1TraceEnabled())
        return;
    unsigned int total = 0;
    for (unsigned int i = 0; i < g_snd_bankCount && i < SND_MAX_BANKS; ++i)
    {
        const SndBank *bank = g_snd_banks[i];
        if (!bank)
            continue;
        Com_Printf(16, "BO1_AUDIO bank %s: %u aliases, %u radverbs, %u snapshots\n",
            bank->name ? bank->name : "?", bank->aliasCount, bank->radverbCount, bank->snapshotCount);
        total += bank->aliasCount;
    }
    // L14: each sound group's attenuation columns and the product along its parent chain (the factor SND_GetBaseLevel
    // multiplies into every voice of the group); used = the column this build reads (SND_GroupAttenuationRaw).
    // Without a sound device SND_Init leaves g_snd.global_constants null; the asset is loaded with the zone anyway.
    const SndDriverGlobals *globals = g_snd.global_constants ? g_snd.global_constants
        : DB_FindXAssetHeader(ASSET_TYPE_SNDDRIVER_GLOBALS, (char *)"singleton", false, 0).sndDriverGlobals;
    const unsigned int groupCount = globals ? globals->groupCount : 0;
    for (unsigned int g = 0; g < groupCount; ++g)
    {
        float chainSp = 1.0f, chainMp = 1.0f, chainUsed = 1.0f;
        int p = (int)g;
        for (int i = 0; p >= 0 && (unsigned int)p < groupCount && i < 0x64; ++i)
        {
            const snd_group *pg = &globals->groups[p];
            chainSp *= (float)pg->attenuationSp / 65535.0f;
            chainMp *= (float)pg->attenuationMp / 65535.0f;
            chainUsed *= (float)SND_GroupAttenuationRaw(pg) / 65535.0f;
            p = pg->parentIndex;
        }
        const snd_group *grp = &globals->groups[g];
        Com_Printf(16, "BO1_AUDIO group %u %s parent=%s sp=%.3f mp=%.3f chainSp=%.3f chainMp=%.3f used=%.3f%s\n", g,
            grp->name, grp->parentName[0] ? grp->parentName : "-", grp->attenuationSp / 65535.0f,
            grp->attenuationMp / 65535.0f, chainSp, chainMp, chainUsed, chainSp != chainMp ? " DIFF" : "");
    }
    Com_Printf(16, "BO1_AUDIO banks (%s): %u banks, %u aliases, sound %s\n",
        when, g_snd_bankCount, total, SND_Active() ? "active" : "inactive (no device)");
}

void SND_BO1TraceText(const char *fmt, ...)
{
    if (!SND_BO1TraceEnabled())
        return;
    char text[512];
    va_list args;
    va_start(args, fmt);
    _vsnprintf(text, sizeof(text) - 1, fmt, args);
    va_end(args);
    text[sizeof(text) - 1] = 0;
    Com_Printf(16, "BO1_AUDIO %s t=%u\n", text, Sys_Milliseconds());
}
