#include "g_sp_headless_throw.h"
#include "g_sp_headless_dice.h"
#include "g_sp_measure.h"
#include "g_sp_testplan.h"
#include "g_sp_headless_move.h"
#include "g_anim_commands_sp.h"

#include <clientscript/cscr_stringlist.h>
#include <clientscript/cscr_compiler.h>
#include <game_mp/g_main_mp.h>
#include <server_mp/sv_main_mp.h>
#include <qcommon/common.h>
#include <qcommon/threads.h>
#include <client/client.h>
#include <universal/dvar.h>
#include <win32/win_main.h>
#include <string>
#include <vector>
#include <algorithm>
#include <intrin.h>
#include <universal/profile.h>
#include <cmath>
#include <clientscript/scr_const.h>
#include <clientscript/cscr_vm.h>
#include <clientscript/cscr_animtree.h>
#include <EffectsCore/fx_system.h>
#include <game/sentient.h>
#include <xanim/xanim.h>
#include <game_mp/actor_mp.h>
#include <game_mp/g_utils_mp.h>
#include <psapi.h>
#include <universal/physicalmemory.h>
#include <universal/com_memory.h>

extern hunkUsed_t hunk_low; // k1: read-only allocator positions for the resources event (com_memory.cpp)
extern hunkUsed_t hunk_high;

extern unsigned int g_usedEntHandle;
extern unsigned int g_maxUsedEntHandle;

// Measurement only: these observers do not implement or replace SP game logic.
// JSON lines share console_mp.log's synchronous writes (headless.ps1 sets logfile 2).
namespace
{
const dvar_s *s_measure;
const dvar_s *s_interval;
const dvar_s *s_movers; // j1: per-frame script mover lines (board chunks), off by default
const dvar_s *s_animEnts; // L12: animated non-actor entities (script models), server and client, off by default
int s_nextAnimEnts;
const dvar_s *s_actorTraceFrom; // a1 c15: per-think actor path/orient/anim lines between these level times
const dvar_s *s_actorTraceTo;
int s_sequence, s_nextSample;
struct PlayerSample
{
    int frames, activeFrames, score, health, state;
};
PlayerSample s_players[32];
std::string s_levelValues[4];
const char *s_fieldNames[] = { "round_number", "zombie_total", "zombie_health", "first_round", "has_legs", "gibbed", "a", "gib_ref",
    "zombie_move_speed", // q1 c10: level.zombie_move_speed (int) and self.zombie_move_speed ("walk"/"run"/"sprint")
    "crouchrun_combatanim", "moveplaybackrate" }; // q1 c15: the crawler's crawl cycle (zombie_gib_on_damage) and its rate
unsigned int s_fieldKeys[_countof(s_fieldNames)];
bool s_triggerRadius[MAX_GENTITIES_SV]; // f2: trigger_radius entities seen in use (quad gas area)
// k1: slow-frame profile (QueryPerformanceCounter ticks) and resource record timer.
const int RESOURCE_MS = 10000;
const unsigned int SLOW_FRAME_MS = 50;
int s_nextResources;
bool s_profiling;
long long s_profileFrequency, s_profileLast;
long long s_phaseTicks[SP_PROFILE_COUNT];
long long s_slowEntityTicks, s_pathTicks, s_pathMaxTicks;
int s_slowEntity, s_pathSearches, s_entityThinks;

bool Enabled()
{
    return s_measure && s_measure->current.enabled && Sys_IsHeadless();
}

std::string Quote(const char *s)
{
    std::string out = "\"";
    for (; s && *s; ++s)
    {
        unsigned char c = (unsigned char)*s;
        if (c == '"' || c == '\\') { out += '\\'; out += c; }
        else if (c < 32) { char hex[7]; sprintf_s(hex, "\\u%04x", c); out += hex; }
        else out += c;
    }
    return out + '"';
}

const char *String(unsigned int id)
{
    return id ? SL_ConvertToString(id, SCRIPTINSTANCE_SERVER) : "";
}

// Find-only: never create fields, intern strings, call getters or borrow VM stack slots.
std::string Field(unsigned int object, const char *name)
{
    unsigned int key = 0;
    for (int i = 0; i < _countof(s_fieldNames); ++i)
        if (!strcmp(name, s_fieldNames[i])) key = s_fieldKeys[i];
    unsigned int id = object && key ? FindVariable(SCRIPTINSTANCE_SERVER, object, key) : 0;
    if (!id)
        return "null";
    unsigned int type = GetValueType(SCRIPTINSTANCE_SERVER, id);
    const VariableUnion &value = GetVariableValueAddress(SCRIPTINSTANCE_SERVER, id)->u;
    if (type == VAR_INTEGER)
        return std::to_string(value.intValue);
    if (type == VAR_STRING || type == VAR_ISTRING)
        return Quote(String(value.stringValue));
    if (type == VAR_FLOAT && std::isfinite(value.floatValue))
    {
        char number[32];
        sprintf_s(number, "%.9g", value.floatValue);
        return number;
    }
    if (type == VAR_ANIMATION)
    {
        // q1 c15: an animation field as {name, length s, root translation over the whole anim / length (u/s)}.
        const scr_anim_s anim = *(const scr_anim_s *)&value.intValue;
        const XAnim_s *anims = Scr_GetAnims(anim.tree, SCRIPTINSTANCE_SERVER);
        if (!anims || anim.index >= anims->size || !anims->entries[anim.index].parts)
            return "null";
        const char *name = anims->entries[anim.index].parts->name;
        float rot[2] = {}, trans[3] = {};
        XAnimGetRelDelta(anims, anim.index, rot, trans, 0.0f, 1.0f);
        const float length = (float)XAnimGetLength(anims, anim.index);
        char text[160];
        sprintf_s(text, "{\"name\":%s,\"len\":%.3f,\"speed\":%.2f}", Quote(name ? name : "").c_str(), length,
            length > 0.0f ? sqrtf(trans[0] * trans[0] + trans[1] * trans[1]) / length : 0.0f);
        return text;
    }
    return "null";
}

void Emit(const char *event, const std::string &fields)
{
    Com_Printf(16, "BO1_MEASURE {\"v\":1,\"seq\":%d,\"time\":%d,\"event\":\"%s\"%s}\n",
        ++s_sequence, level.time, event, fields.c_str());
}

double Us(long long ticks)
{
    return s_profileFrequency ? (double)ticks * 1000000.0 / (double)s_profileFrequency : 0.0;
}

// k1: pool and memory use, every RESOURCE_MS of level time. Read-only counters.
void EmitResources()
{
    PROCESS_MEMORY_COUNTERS_EX memory = {};
    memory.cb = sizeof(memory);
    K32GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS *)&memory, sizeof(memory));
    DWORD handles = 0;
    GetProcessHandleCount(GetCurrentProcess(), &handles);
    int entities = 0, actors = 0, sentients = 0;
    for (int i = 0; i < level.num_entities; ++i)
        if (g_entities[i].r.inuse) ++entities;
    for (int i = 0; level.actors && i < MAX_ACTORS; ++i)
        if (level.actors[i].inuse) ++actors;
    for (int i = 0; level.sentients && i < MAX_SENTIENTS; ++i)
        if (level.sentients[i].inuse) ++sentients;
    // Client FX pools (local client 0), read without locking: approximate while the client thread runs.
    int fxEffects = -1, fxElems = -1, fxTrailElems = -1;
    FxSystemContainer *fx = FX_GetSystem(0);
    if (fx && fx->system.isInitialized)
    {
        fxEffects = (int)(fx->shared.firstFreeEffect - fx->shared.firstActiveEffect);
        fxElems = (int)fx->shared.activeElemCount;
        fxTrailElems = (int)fx->shared.activeTrailElemCount;
    }
    int animReferenced, animProtected, animSnapshotTime;
    G_SP_GetAnimCommandUsage(&animReferenced, &animProtected, &animSnapshotTime);
    char fields[1024];
    sprintf_s(fields, ",\"working_set_kb\":%u,\"private_kb\":%u,\"peak_working_set_kb\":%u,\"handles\":%u,"
        "\"script_objects\":%u,\"script_values\":%u,\"script_threads\":%d,\"script_refs\":%d,"
        "\"entities\":%d,\"num_entities\":%d,\"max_entities\":%d,\"actors\":%d,\"max_actors\":%d,"
        "\"sentients\":%d,\"max_sentients\":%d,\"fx_effects\":%d,\"fx_elems\":%d,\"fx_trail_elems\":%d,"
        "\"ent_handles\":%u,\"peak_ent_handles\":%u,"
        "\"anim_referenced\":%d,\"anim_protected\":%d,\"anim_snapshot_time\":%d,"
        "\"pmem_low\":%u,\"pmem_high\":%u,\"hunk_low\":%d,\"hunk_high\":%d,\"hunk_temp\":%d,\"round\":",
        (unsigned int)(memory.WorkingSetSize / 1024), (unsigned int)(memory.PrivateUsage / 1024),
        (unsigned int)(memory.PeakWorkingSetSize / 1024), (unsigned int)handles,
        Scr_GetNumScriptVarsParent(SCRIPTINSTANCE_SERVER), Scr_GetNumScriptVarsChild(SCRIPTINSTANCE_SERVER),
        Scr_GetNumScriptThreads(SCRIPTINSTANCE_SERVER), gScrVarPub[SCRIPTINSTANCE_SERVER].totalObjectRefCount,
        entities, level.num_entities, MAX_GENTITIES, actors, MAX_ACTORS, sentients, MAX_SENTIENTS,
        fxEffects, fxElems, fxTrailElems, g_usedEntHandle, g_maxUsedEntHandle,
        animReferenced, animProtected, animSnapshotTime,
        g_mem.prim[0].pos, g_mem.prim[1].pos, hunk_low.permanent, hunk_high.permanent, hunk_low.temp + hunk_high.temp);
    Emit("resources", std::string(fields) + Field(gScrVarPub[SCRIPTINSTANCE_SERVER].levelId, "round_number"));
}
}

void G_SP_ProfileBegin()
{
    s_profiling = Enabled();
    if (!s_profiling)
        return;
    if (!s_profileFrequency)
    {
        LARGE_INTEGER frequency;
        QueryPerformanceFrequency(&frequency);
        s_profileFrequency = frequency.QuadPart;
    }
    memset(s_phaseTicks, 0, sizeof(s_phaseTicks));
    s_slowEntityTicks = s_pathTicks = s_pathMaxTicks = 0;
    s_slowEntity = -1;
    s_pathSearches = s_entityThinks = 0;
    s_profileLast = G_SP_ProfileNow();
}

long long G_SP_ProfileNow()
{
    if (!s_profiling)
        return 0;
    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    return now.QuadPart;
}

void G_SP_ProfileMark(int phase)
{
    if (!s_profiling)
        return;
    const long long now = G_SP_ProfileNow();
    s_phaseTicks[phase] += now - s_profileLast;
    s_profileLast = now;
}

void G_SP_ProfileEntity(const gentity_s *ent, long long start)
{
    if (!s_profiling)
        return;
    const long long ticks = G_SP_ProfileNow() - start;
    ++s_entityThinks;
    if (ticks > s_slowEntityTicks)
    {
        s_slowEntityTicks = ticks;
        s_slowEntity = ent->s.number;
    }
}

SPPathProfile::SPPathProfile() : start(G_SP_ProfileNow())
{
}

SPPathProfile::~SPPathProfile()
{
    if (!s_profiling)
        return;
    const long long ticks = G_SP_ProfileNow() - start;
    ++s_pathSearches;
    s_pathTicks += ticks;
    if (ticks > s_pathMaxTicks)
        s_pathMaxTicks = ticks;
}

void G_SP_MeasureFields(scriptInstance_t inst)
{
    G_SP_TestPlanFields(inst); // headless test-client plan (harness only)
    G_SP_HeadlessThrowFields(inst); // w1 c6 TEST SWITCH bo1_testclient_scriptgive
    G_SP_HeadlessDiceFields(inst); // k1 TEST SWITCH bo1_testclient_dice: canonical local / field names
    if (inst != SCRIPTINSTANCE_SERVER || !Enabled())
        return;
    // Compiler identifiers are canonical IDs, not SL string IDs. Save the mapping
    // before Scr_EndLoadScripts releases its strings; do not retain any VM refs.
    for (int i = 0; i < _countof(s_fieldNames); ++i)
    {
        unsigned int string = SL_FindString(s_fieldNames[i], inst);
        s_fieldKeys[i] = string ? gScrCompilePub[inst].canonicalStrings[string] : 0;
    }
}

bool G_SP_MeasureEnabled()
{
    return Enabled();
}

// L12 (TEST): bo1_measure_animents, read by the client trace too.
bool G_SP_MeasureAnimEntsOn()
{
    return Enabled() && s_animEnts && s_animEnts->current.enabled;
}

// L15 (TEST): an anim's share of the final pose - its weight over its siblings' weight sum, multiplied up to the
// root (how XAnim normalises child weights). -1 for an index outside the tree.
float G_SP_MeasureEffectiveWeight(const XAnimTree_s *tree, unsigned int anim)
{
    const XAnim_s *anims = XAnimGetAnims(tree);
    if (!anims || anim >= anims->size)
        return -1.0f;
    float result = 1.0f;
    for (int depth = 0; anim; ++depth)
    {
        const unsigned int parent = anims->entries[anim].parent;
        if (depth >= 32 || parent >= anims->size || parent == anim)
            return -1.0f;
        const unsigned int first = anims->entries[parent].animParent.children;
        const unsigned int count = anims->entries[parent].numAnims;
        if (first + count > anims->size)
            return -1.0f;
        float sum = 0.0f;
        for (unsigned int i = 0; i < count; ++i)
            sum += (float)XAnimGetWeight(tree, first + i);
        if (sum <= 0.0f)
            return 0.0f;
        result *= (float)XAnimGetWeight(tree, anim) / sum;
        anim = parent;
    }
    return result;
}

// L12 (TEST): "name:weight:time,..." for every leaf anim of the tree with a weight.
void G_SP_FormatWeightedAnims(const XAnimTree_s *tree, char *out, int size)
{
    int used = 0;
    out[0] = 0;
    const XAnim_s *anims = XAnimGetAnims(tree);
    const unsigned int count = anims ? XAnimGetAnimTreeSize(anims) : 0;
    for (unsigned int anim = 1; anim < count && used < size - 1; ++anim)
    {
        if (XAnimGetNumChildren(anims, anim) || !XAnimGetInfoIndex(tree, anim))
            continue;
        const float weight = (float)XAnimGetWeight(tree, anim);
        if (weight <= 0.0f)
            continue;
        const int n = _snprintf_s(out + used, size - used, _TRUNCATE, "%s%s:%.2f:%.3f", used ? "," : "",
            XAnimGetAnimName(anims, anim), weight, (float)XAnimGetTime(tree, anim));
        if (n < 0)
            break;
        used += n;
    }
    if (!used)
        strcpy_s(out, size, "-");
}

// a1 c15 (TEST): one actor's think result - origin, yaw, desired yaw, the path lookahead (the recording's actors.tsv
// path columns: wDodgeEntity, lookaheadDir, fLookaheadDist, flags, numIncreases / numReductions), the wish delta and
// the weighted leaf anims - to find where a zombie leaves its recorded track.
bool G_SP_MeasureActorTraceOn()
{
    return Enabled() && s_actorTraceFrom && s_actorTraceFrom->current.integer && level.time >= s_actorTraceFrom->current.integer
        && level.time <= s_actorTraceTo->current.integer;
}

void G_SP_MeasureActorTrace(const actor_s *actor)
{
    if (!G_SP_MeasureActorTraceOn() || !actor || !actor->ent)
        return;
    const gentity_s *ent = actor->ent;
    const path_t &p = actor->Path;
    char line[1400];
    int len = sprintf_s(line, "bo1_actortrace: time %d ent %d o %.9g %.9g %.9g yaw %.9g dyaw %.9g om %d am %d mm %d path %d/%d "
        "nn %d dodge %d/%d la %.5f %.5f dist %.3f amt %.3f tonext %.3f flags 0x%x inc %d red %d wish %.9g %.9g hit %d",
        level.time, ent->s.number, ent->r.currentOrigin[0], ent->r.currentOrigin[1], ent->r.currentOrigin[2],
        ent->r.currentAngles[1], actor->fDesiredBodyYaw, (int)actor->CodeOrient.eMode, (int)actor->eAnimMode,
        (int)actor->moveMode, (int)p.wPathLen, (int)p.wOrigPathLen, (int)p.lookaheadNextNode, (int)p.wDodgeCount,
        (int)p.wDodgeEntity, p.lookaheadDir[0], p.lookaheadDir[1], p.fLookaheadDist, p.fLookaheadAmount,
        p.fLookaheadDistToNextNode, p.flags, (int)p.numIncreases, (int)p.numReductions, actor->Physics.vWishDelta[0],
        actor->Physics.vWishDelta[1], actor->Physics.iHitEntnum);
    XAnimTree_s *tree = G_GetEntAnimTree(const_cast<gentity_s *>(ent));
    if (tree && tree->anims)
        for (unsigned int animIndex = 0; animIndex < tree->anims->size && len > 0 && len < 1200; ++animIndex)
        {
            const char *animName = XAnimGetAnimName(tree->anims, animIndex);
            const float weight = (float)XAnimGetWeight(tree, animIndex);
            if (animName[0] && weight > 0.0f)
                len += sprintf_s(line + len, sizeof(line) - len, " %s:%.5f:%.4f", animName,
                    (float)XAnimGetTime(tree, animIndex), weight);
        }
    Com_Printf(16, "%s\n", line);
}

// f2: headless measurement only. Include every completed server frame, even between origin samples.
// Times include synchronous measurement I/O inside the frame, but exclude this record's own write.
void G_SP_MeasureServerFrame(unsigned int totalMs, unsigned int gameMs, unsigned int beforeGameMs)
{
    if (!Enabled())
        return;
    int actors = 0, alive = 0;
    for (int i = 0; i < level.num_entities; ++i)
        if (g_entities[i].r.inuse && g_entities[i].actor)
        {
            ++actors;
            if (g_entities[i].health > 0) ++alive;
        }
    char fields[256];
    sprintf_s(fields, ",\"total_ms\":%u,\"game_ms\":%u,\"before_game_ms\":%u,\"after_game_ms\":%u,\"actors\":%d,\"alive\":%d,\"round\":",
        totalMs, gameMs, beforeGameMs, totalMs - gameMs - beforeGameMs, actors, alive);
    char paths[96];
    sprintf_s(paths, ",\"path_searches\":%d,\"path_us\":%.0f", s_pathSearches, Us(s_pathTicks));
    Emit("server_frame", std::string(fields) + Field(gScrVarPub[SCRIPTINSTANCE_SERVER].levelId, "round_number") + paths);
    if (totalMs < SLOW_FRAME_MS || !s_profiling)
        return;
    // k1: where a slow frame's G_RunFrame time went (microseconds per phase; see SPProfilePhase).
    std::string slow = ",\"total_ms\":" + std::to_string(totalMs) + ",\"phase_us\":[";
    for (int i = 0; i < SP_PROFILE_COUNT; ++i)
    {
        char number[24];
        sprintf_s(number, "%s%.0f", i ? "," : "", Us(s_phaseTicks[i]));
        slow += number;
    }
    const gentity_s *ent = s_slowEntity >= 0 ? &g_entities[s_slowEntity] : nullptr;
    char details[256];
    sprintf_s(details, "],\"entity_thinks\":%d,\"slowest_ent\":%d,\"slowest_ent_us\":%.0f,\"path_searches\":%d,\"path_us\":%.0f,\"path_max_us\":%.0f,\"slowest_classname\":",
        s_entityThinks, s_slowEntity, Us(s_slowEntityTicks), s_pathSearches, Us(s_pathTicks), Us(s_pathMaxTicks));
    Emit("slow_frame", slow + details + Quote(ent ? String(ent->classname) : ""));
}

void G_SP_MeasureInit()
{
    if (!Sys_IsHeadless())
        return;
    s_measure = _Dvar_RegisterBool("bo1_measure", false, 0, "Headless Five measurement JSON events");
    s_interval = _Dvar_RegisterInt("bo1_measure_ms", 50, 50, 60000, 0, "Actor origin sample interval in level milliseconds");
    s_movers = _Dvar_RegisterBool("bo1_measure_movers", false, 0,
        "With bo1_measure: a 'mover' line per moving/linked script mover per server frame, and the client's drawn brush-mover poses in bo1_cgmover.txt");
    s_animEnts = _Dvar_RegisterBool("bo1_measure_animents", false, 0,
        "With bo1_measure: every 250 ms a bo1_animent_sv line per animated non-actor entity (its weighted anims), "
        "bo1_animent_cl lines for the client's DObj of it, and bo1_animent_note per client notetrack on it");
    s_actorTraceFrom = _Dvar_RegisterInt("bo1_measure_actortrace_from", 0, 0, 0x7FFFFFFF, 0,
        "With bo1_measure: a 'bo1_actortrace' line per actor per think from this level time (0 = off)");
    s_actorTraceTo = _Dvar_RegisterInt("bo1_measure_actortrace_to", 0, 0, 0x7FFFFFFF, 0,
        "Last level time of bo1_measure_actortrace_from");
    if (!Enabled())
        return;
    s_sequence = s_nextSample = s_nextResources = 0;
    memset(s_players, 0, sizeof(s_players));
    memset(s_triggerRadius, 0, sizeof(s_triggerRadius));
    for (auto &value : s_levelValues) value.clear();
    Emit("init", ",\"sample_ms\":" + std::to_string(s_interval->current.integer));
}

void G_SP_MeasureActor(const char *event, const gentity_s *ent, const gentity_s *spawner)
{
    if (!Enabled())
        return;
    // Observe the retail script's crawler transition without forcing damage or gibs.
    const unsigned int object = FindEntityId(SCRIPTINSTANCE_SERVER, ent->s.number, 0, 0);
    unsigned int animObject = 0;
    const unsigned int animField = object && s_fieldKeys[6] ? FindVariable(SCRIPTINSTANCE_SERVER, object, s_fieldKeys[6]) : 0;
    if (animField && GetValueType(SCRIPTINSTANCE_SERVER, animField) == VAR_POINTER)
        animObject = GetVariableValueAddress(SCRIPTINSTANCE_SERVER, animField)->u.pointerValue;
    char numbers[320];
    sprintf_s(numbers, ",\"ent\":%d,\"species\":%d,\"origin\":[%.6f,%.6f,%.6f],\"yaw\":%.4f,\"health\":%d,\"round\":",
        ent->s.number, ent->actor ? (int)ent->actor->species : -1,
        ent->r.currentOrigin[0], ent->r.currentOrigin[1], ent->r.currentOrigin[2], ent->r.currentAngles[1], ent->health);
    Emit(event, std::string(numbers) + Field(gScrVarPub[SCRIPTINSTANCE_SERVER].levelId, "round_number")
        + ",\"zombie_total\":" + Field(gScrVarPub[SCRIPTINSTANCE_SERVER].levelId, "zombie_total")
        + ",\"aitype\":" + Quote(String(ent->classname))
        + ",\"spawner\":" + (spawner ? Quote(String(spawner->targetname)) : "null")
        + ",\"has_legs\":" + Field(object, "has_legs") + ",\"gibbed\":" + Field(object, "gibbed")
        + ",\"gib_ref\":" + Field(animObject, "gib_ref")
        // q1 c10: round rules per round (retail script values, read only)
        + ",\"max_health\":" + std::to_string(ent->maxHealth)
        + ",\"move_speed\":" + Field(object, "zombie_move_speed")
        + ",\"crawl\":" + Field(object, "crouchrun_combatanim") + ",\"moverate\":" + Field(object, "moveplaybackrate")
        + ",\"level_move_speed\":" + Field(gScrVarPub[SCRIPTINSTANCE_SERVER].levelId, "zombie_move_speed")
        + ",\"level_zombie_health\":" + Field(gScrVarPub[SCRIPTINSTANCE_SERVER].levelId, "zombie_health"));
}

void G_SP_MeasureAnimScripted(const gentity_s *ent, const XAnim_s *anims, unsigned int anim, unsigned int notifyName,
    const float *origin, const float *angles, float rate, float blendTime)
{
    if (!Enabled() || !anims || !anim)
        return;
    const XAnimParts *parts = anims->entries[anim].parts;
    char numbers[400];
    sprintf_s(numbers, ",\"ent\":%d,\"origin\":[%.3f,%.3f,%.3f],\"angles\":[%.3f,%.3f,%.3f],\"cur\":[%.3f,%.3f,%.3f],\"yaw\":%.3f,\"rate\":%.3f,\"blend\":%.3f,\"length\":%.4f",
        ent->s.number, origin[0], origin[1], origin[2], angles[0], angles[1], angles[2],
        ent->r.currentOrigin[0], ent->r.currentOrigin[1], ent->r.currentOrigin[2], ent->r.currentAngles[1],
        rate, blendTime, parts && parts->framerate > 0.0f ? (float)parts->numframes / parts->framerate : 0.0f);
    std::string notes = ",\"notes\":[";
    if (parts && parts->notify)
        for (unsigned int i = 0; i < parts->notifyCount; ++i)
        {
            char t[32];
            sprintf_s(t, "%s[%.4f,", i ? "," : "", parts->notify[i].time);
            notes += t + Quote(String(parts->notify[i].name)) + "]";
        }
    Emit("animscripted", std::string(numbers) + ",\"notify\":" + Quote(String(notifyName))
        + ",\"anim\":" + Quote(parts ? parts->name : "") + notes + "]");
}

void G_SP_MeasureFrame()
{
    if (!Enabled())
        return;
    unsigned int object = gScrVarPub[SCRIPTINSTANCE_SERVER].levelId;
    for (int i = 0; i < 4; ++i)
    {
        std::string value = Field(object, s_fieldNames[i]);
        if (value != s_levelValues[i])
        {
            Emit("level", ",\"name\":" + Quote(s_fieldNames[i]) + ",\"value\":" + value);
            s_levelValues[i] = value;
        }
    }
    for (int i = 0; i < level.maxclients && i < 32; ++i)
    {
        const gentity_s *ent = &g_entities[i];
        PlayerSample &old = s_players[i];
        if (!ent->r.inuse || !ent->client || ent->client->sess.connected == CON_DISCONNECTED)
        {
            old = {};
            continue;
        }
        int state = svs.clients[i].header.state;
        int score = ent->client->sess.cs.score.score;
        ++old.frames;
        if (state == CS_ACTIVE) ++old.activeFrames;
        if (old.frames == 1 || (state == CS_ACTIVE && old.activeFrames == 1)
            || old.score != score || old.health != ent->health || old.state != state)
        {
            char fields[320];
            sprintf_s(fields, ",\"ent\":%d,\"frame\":%d,\"active_frame\":%d,\"state\":%d,\"score\":%d,\"health\":%d,\"origin\":[%.6f,%.6f,%.6f]",
                i, old.frames, old.activeFrames, state, score, ent->health,
                ent->r.currentOrigin[0], ent->r.currentOrigin[1], ent->r.currentOrigin[2]);
            Emit("player", fields);
        }
        old.score = score; old.health = ent->health; old.state = state;
    }
    // f2: script-spawned trigger_radius areas (maps\_zombiemode_ai_quad::quad_gas_area_of_effect) come and go.
    for (int i = 0; i < MAX_GENTITIES; ++i)
    {
        const gentity_s *ent = &g_entities[i];
        const bool live = i < level.num_entities && ent->r.inuse && ent->classname == scr_const.trigger_radius;
        if (live == s_triggerRadius[i])
            continue;
        s_triggerRadius[i] = live;
        char fields[160];
        sprintf_s(fields, ",\"ent\":%d,\"origin\":[%.1f,%.1f,%.1f],\"radius\":%.1f", i,
            ent->r.currentOrigin[0], ent->r.currentOrigin[1], ent->r.currentOrigin[2], live ? ent->r.maxs[0] : 0.0f);
        Emit(live ? "trigger_radius_add" : "trigger_radius_remove", fields);
    }
    // j1: script movers while they move or ride a link (board chunks on remove_chunk's script_origin),
    // every server frame, to compare the board flight with the recording's per-frame mover rows.
    for (int i = 0; s_movers && s_movers->current.enabled && i < level.num_entities; ++i)
    {
        const gentity_s *ent = &g_entities[i];
        if (!ent->r.inuse || ent->s.eType != ET_SCRIPTMOVER)
            continue;
        const bool moving = ent->s.lerp.pos.trType != TR_STATIONARY || ent->s.lerp.apos.trType != TR_STATIONARY;
        if (!moving && !ent->tagInfo)
            continue;
        char fields[320];
        sprintf_s(fields, ",\"ent\":%d,\"origin\":[%.3f,%.3f,%.3f],\"angles\":[%.3f,%.3f,%.3f],\"tr\":[%d,%d],\"hidden\":%d,\"parent\":%d,\"model\":%d",
            i, ent->r.currentOrigin[0], ent->r.currentOrigin[1], ent->r.currentOrigin[2],
            ent->r.currentAngles[0], ent->r.currentAngles[1], ent->r.currentAngles[2],
            (int)ent->s.lerp.pos.trType, (int)ent->s.lerp.apos.trType, (ent->s.lerp.eFlags & 0x20) ? 1 : 0,
            ent->tagInfo && ent->tagInfo->parent ? ent->tagInfo->parent->s.number : -1, (int)ent->model);
        Emit("mover", std::string(fields) + ",\"cls\":" + Quote(String(ent->classname))
            + ",\"tn\":" + Quote(String(ent->targetname)));
    }
    // L12: animated script models (cymbal monkey, box, Pack-a-Punch ...): which anims the server tree plays.
    if (s_animEnts && s_animEnts->current.enabled && level.time >= s_nextAnimEnts)
    {
        s_nextAnimEnts = level.time + 250;
        for (int i = 0; i < level.num_entities; ++i)
        {
            const gentity_s *ent = &g_entities[i];
            if (!ent->r.inuse || ent->actor || ent->client || !ent->pAnimTree)
                continue;
            char anims[512];
            G_SP_FormatWeightedAnims(ent->pAnimTree, anims, sizeof(anims));
            Com_Printf(16, "bo1_animent_sv: time %d ent %d eType %d cls %s model %s origin %.0f %.0f %.0f hidden %d parent %d anims %s\n",
                level.time, i, (int)ent->s.eType, String(ent->classname),
                ent->model ? SL_ConvertToString(G_ModelName(ent->model), SCRIPTINSTANCE_SERVER) : "",
                ent->r.currentOrigin[0], ent->r.currentOrigin[1], ent->r.currentOrigin[2],
                (ent->s.lerp.eFlags & 0x20) ? 1 : 0,
                ent->tagInfo && ent->tagInfo->parent ? ent->tagInfo->parent->s.number : -1, anims);
        }
    }
    if (level.time >= s_nextResources)
    {
        s_nextResources = level.time + RESOURCE_MS;
        EmitResources();
    }
    if (level.time < s_nextSample)
        return;
    s_nextSample = level.time + s_interval->current.integer;
    Emit("frame", ",\"sv_running\":" + std::to_string(com_sv_running->current.enabled));
    for (int i = 0; i < level.num_entities; ++i)
        if (g_entities[i].r.inuse && g_entities[i].actor)
            G_SP_MeasureActor("actor_origin", &g_entities[i], nullptr);
}

void G_SP_MeasureNotify(scriptInstance_t inst, unsigned int owner, unsigned int name, const VariableValue *top)
{
    G_SP_TestPlanNotify(inst, owner, name, top); // headless test-client plan (harness only)
    if (inst == SCRIPTINSTANCE_SERVER && owner == gScrVarPub[inst].levelId && Sys_IsHeadless())
    {
        G_SP_HeadlessReplayLevelNotify(SL_ConvertToString(name, inst)); // replay clock (harness only)
        Sys_HeadlessLevelNotify(SL_ConvertToString(name, inst)); // L20: bo1_quitnotify (harness only)
    }
    if (inst != SCRIPTINSTANCE_SERVER || !Enabled())
        return;
    bool isLevel = owner == gScrVarPub[inst].levelId;
    int entnum = -1;
    if (!isLevel && GetObjectType(inst, owner) == VAR_ENTITY)
    {
        scr_entref_t ref = Scr_GetEntityIdRef(inst, owner);
        if (!ref.classnum) entnum = ref.entnum;
    }
    const char *notify = String(name);
    // All level notifies preserve gsc-report's first-occurrence digest. Actor/client
    // notifies include tear_anim("board"/"end"), begin, death and round transitions.
    if (!isLevel && (entnum < 0 || entnum >= level.num_entities
        || (!g_entities[entnum].actor && !g_entities[entnum].client)))
        return;
    std::string arg = "null";
    if (top->type == VAR_STRING || top->type == VAR_ISTRING) arg = Quote(String(top->u.stringValue));
    else if (top->type == VAR_INTEGER) arg = std::to_string(top->u.intValue);
    Emit("notify", ",\"scope\":" + Quote(isLevel ? "level" : "entity")
        + ",\"ent\":" + std::to_string(entnum) + ",\"name\":" + Quote(notify) + ",\"arg0\":" + arg
        + ",\"round\":" + Field(gScrVarPub[inst].levelId, "round_number"));
}

// k1: headless -Client frame cost. Main thread only (Com_Frame -> CL_Frame -> CL_CGameRendering).
namespace
{
std::vector<double> s_clientFrameMs, s_clientCgMs;
long long s_clientReportStart;
long long s_clientCgTicks;
long long s_clientCgCpuStart = -1, s_clientCgCpuTotal;
unsigned int s_clientCgHitchFrame; // k1: Sys_HitchWatchBegin's frame number (bo1_hitchwatch)
unsigned int s_clientCgCpuSamples;

// Measurement only: thread CPU excludes waits and time scheduled on other threads.
// Windows accounts it coarsely, so report a five-second mean rather than percentiles.
long long ThreadCpuTime()
{
    FILETIME created, exited, kernel, user;
    if (!GetThreadTimes(GetCurrentThread(), &created, &exited, &kernel, &user))
        return -1;
    return ((long long)kernel.dwHighDateTime << 32) + kernel.dwLowDateTime
        + ((long long)user.dwHighDateTime << 32) + user.dwLowDateTime;
}

double Percentile(std::vector<double> &v, double p)
{
    if (v.empty())
        return 0.0;
    std::sort(v.begin(), v.end());
    return v[(std::min)(v.size() - 1, (size_t)(p * v.size()))];
}

long long Frequency()
{
    static long long frequency;
    if (!frequency)
    {
        LARGE_INTEGER f;
        QueryPerformanceFrequency(&f);
        frequency = f.QuadPart;
    }
    return frequency;
}
}

long long G_SP_MeasureTicks()
{
    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    return now.QuadPart;
}

void G_SP_LoadMark(const char *label)
{
    static long long s_last;
    if (!Sys_FramePerfHitchMs())
        return;
    const long long now = G_SP_MeasureTicks();
    const double ms = s_last ? (double)(now - s_last) * 1000.0 / (double)Frequency() : 0.0;
    s_last = now;
    Com_Printf(16, "BO1_LOADMARK %s %.1f %s\n", label, ms, Sys_IsMainThread() ? "main" : "other");
}

long long G_SP_MeasureClientCgBegin()
{
    s_clientCgCpuStart = Sys_IsHeadlessClient() && Enabled() ? ThreadCpuTime() : -1;
    s_clientCgHitchFrame = Sys_HitchWatchBegin();
    return G_SP_MeasureTicks();
}

void G_SP_MeasureClientCg(long long start)
{
    const long long ticks = G_SP_MeasureTicks() - start;
    Sys_HitchWatchEnd();
    s_clientCgTicks += ticks;
    if (s_clientCgCpuStart >= 0)
    {
        const long long end = ThreadCpuTime();
        if (end >= s_clientCgCpuStart)
        {
            s_clientCgCpuTotal += end - s_clientCgCpuStart;
            ++s_clientCgCpuSamples;
            // k1: one line per long client frame - wall vs this thread's CPU says compute or wait; the frame number
            // matches the bo1_hitchwatch thread dumps (main\bo1_threads.log)
            const double wallMs = (double)ticks * 1000.0 / (double)Frequency();
            if (wallMs >= 60.0)
                Com_Printf(16, "BO1_CLIENTHITCH {\"level_time\":%d,\"frame\":%u,\"wall_ms\":%.2f,\"cpu_ms\":%.2f}\n",
                    level.time, s_clientCgHitchFrame, wallMs, (double)(end - s_clientCgCpuStart) * 0.0001);
        }
        s_clientCgCpuStart = -1;
    }
}

void G_SP_MeasureClientFrame(long long start)
{
    const long long now = G_SP_MeasureTicks();
    if (!Sys_IsHeadlessClient() || !Enabled())
    {
        s_clientCgTicks = 0;
        s_clientCgCpuTotal = 0;
        s_clientCgCpuSamples = 0;
        return;
    }
    const double toMs = 1000.0 / (double)Frequency();
    s_clientFrameMs.push_back((double)(now - start) * toMs);
    s_clientCgMs.push_back((double)s_clientCgTicks * toMs);
    s_clientCgTicks = 0;
    if (!s_clientReportStart)
        s_clientReportStart = now;
    const double elapsedMs = (double)(now - s_clientReportStart) * toMs;
    if (elapsedMs < 5000.0)
        return;
    const size_t frames = s_clientFrameMs.size();
    int alive = 0;
    for (int i = 0; i < level.num_entities; ++i)
        if (g_entities[i].r.inuse && g_entities[i].actor && g_entities[i].health > 0) ++alive;
    Com_Printf(16, "BO1_CLIENTPERF {\"level_time\":%d,\"frames\":%u,\"fps\":%.1f,\"cl_ms_median\":%.2f,\"cl_ms_p99\":%.2f,\"cl_ms_max\":%.2f,"
        "\"cg_ms_median\":%.2f,\"cg_ms_p99\":%.2f,\"cg_ms_max\":%.2f,"
        "\"cg_cpu_ms_mean\":%.2f,\"cg_cpu_samples\":%u,\"alive\":%d}\n",
        level.time, (unsigned int)frames, frames * 1000.0 / elapsedMs,
        Percentile(s_clientFrameMs, 0.5), Percentile(s_clientFrameMs, 0.99), Percentile(s_clientFrameMs, 1.0),
        Percentile(s_clientCgMs, 0.5), Percentile(s_clientCgMs, 0.99), Percentile(s_clientCgMs, 1.0),
        s_clientCgCpuSamples ? (double)s_clientCgCpuTotal * 0.0001 / s_clientCgCpuSamples : -1.0,
        s_clientCgCpuSamples, alive);
    s_clientCgCpuTotal = 0;
    s_clientCgCpuSamples = 0;
    s_clientFrameMs.clear();
    s_clientCgMs.clear();
    s_clientReportStart = now;
}

// p1: whole-frame timing at the player's settings (bo1_frameperf, measurement only; see g_sp_measure.h).
// Com_Frame calls G_SP_FramePerfMark at the end of its sleep loop (BEGIN), after SV_Frame, the pre-frame block,
// CL_Frame, SCR_UpdateScreen and R_WaitEndTime (END). The interval BEGIN(n-1)..BEGIN(n) is what the player sees;
// a long one is reported with frame n-1's phases and the zones (profile.h) collected since BEGIN(n-1).
namespace
{
long long s_fpMarks[FRAMEPERF_MARKS];
long long s_fpLastBegin;
bool s_fpHavePrev;
std::vector<double> s_fpIntervals;
long long s_fpReportStart;
unsigned int s_fpOver16, s_fpOver33, s_fpOverHitch, s_fpHitchLines;
double s_fpTscPerMs;
volatile long s_fpSvFrames, s_fpSvMaxUs, s_fpSvOver25; // server thread writes, main thread reads and clears
long s_fpFirst[BO1_FIRST_KINDS], s_fpFirstPrev[BO1_FIRST_KINDS], s_fpFirstWindow[BO1_FIRST_KINDS]; // first-use counts
char s_fpFirstNames[512], s_fpFirstNamesPrev[512];
int s_fpFirstDropped, s_fpFirstDroppedPrev;

// First-use counts of the interval that just ended (render thread, profile.h), the one before it and the 5 s window.
void FramePerfFirstUse()
{
    memcpy(s_fpFirstPrev, s_fpFirst, sizeof(s_fpFirst));
    memcpy(s_fpFirstNamesPrev, s_fpFirstNames, sizeof(s_fpFirstNames));
    s_fpFirstDroppedPrev = s_fpFirstDropped;
    for (int k = 0; k < BO1_FIRST_KINDS; ++k)
    {
        s_fpFirst[k] = _InterlockedExchange(&g_bo1FirstUse[k], 0);
        s_fpFirstWindow[k] += s_fpFirst[k];
    }
    s_fpFirstDropped = BO1_ProfFirstUseNames(s_fpFirstNames, sizeof(s_fpFirstNames));
}

// p1 chunk 25: render frames (universal/profile.h g_bo1RenderRing, written by RB_SwapBuffers) in order once their GPU
// time is known. A frame with a long swap interval, Present or GPU time gets a BO1_RENDERFRAME line with the two
// frames before it; every frame feeds the 5 s window totals of BO1_FRAMEPERF.
std::string FramePerfJsonText(const char *s);
long s_fpRenderNext = 1;
std::vector<double> s_fpGpuMs;
double s_fpPresentMax, s_fpGpuMax;
long s_fpRcWindow[BO1_RC_KINDS];
unsigned int s_fpAvailMin = 0xFFFFFFFF, s_fpAvailMax;
unsigned int s_fpRenderLines;

std::string FramePerfRenderBrief(long f)
{
    const BO1RenderFrame &e = g_bo1RenderRing[f % BO1_RENDER_RING];
    char item[160];
    if (f <= 0 || e.frame != f)
        return "null";
    sprintf_s(item, "[%.1f,%.1f,%.1f,%.1f,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld]", e.swapMs, e.presentMs, e.gpuMs, e.gpuEndDeltaMs,
        e.counts[BO1_RC_TEX_CREATE], e.counts[BO1_RC_TEX_UPLOAD_KB], e.counts[BO1_RC_TEX_RELEASE],
        e.counts[BO1_RC_STREAM_DONE], e.counts[BO1_RC_BUF_CREATE], e.counts[BO1_RC_BUF_LOCK], e.first[BO1_FIRST_PS],
        e.first[BO1_FIRST_COMBO]);
    return item;
}

void FramePerfRender(unsigned int hitchMs)
{
    for (;;)
    {
        const long f = s_fpRenderNext;
        BO1RenderFrame &e = g_bo1RenderRing[f % BO1_RENDER_RING];
        const long have = e.frame;
        if (have > f || (have && have < f - BO1_RENDER_RING)) // overwritten before it was read
        {
            ++s_fpRenderNext;
            continue;
        }
        if (have != f)
            return; // not written yet
        if (!e.state && g_bo1RenderRing[(f + 12) % BO1_RENDER_RING].frame != f + 12)
            return; // GPU time still pending (given up after 12 more frames)
        ++s_fpRenderNext;
        if (e.gpuMs >= 0.0f)
        {
            s_fpGpuMs.push_back(e.gpuMs);
            if (e.gpuMs > s_fpGpuMax) s_fpGpuMax = e.gpuMs;
        }
        if (e.presentMs > s_fpPresentMax) s_fpPresentMax = e.presentMs;
        for (int k = 0; k < BO1_RC_KINDS; ++k)
            s_fpRcWindow[k] += e.counts[k];
        if (e.availMB < s_fpAvailMin) s_fpAvailMin = e.availMB;
        if (e.availMB > s_fpAvailMax) s_fpAvailMax = e.availMB;
        const float half = (float)hitchMs * 0.5f;
        if ((e.swapMs < (float)hitchMs && e.presentMs < half && e.gpuMs < half && e.gpuEndDeltaMs < (float)hitchMs)
            || s_fpRenderLines >= 2000)
        {
            continue;
        }
        ++s_fpRenderLines;
        // counts: [tex create, tex create Ktexels, tex upload, tex upload KB, tex release, stream done, stream KB,
        // buf create, buf lock, buf lock KB]; prev: [swap, present, gpu, gpu_end_delta, tex create, upload KB,
        // tex release, stream done, buf create, buf lock, first ps, first combo] of frames f-1, f-2 (and next: f+1);
        // first: first uses [vs, ps, pair, pass, texture, combo] drawn in frame f
        const long *c = e.counts;
        Com_Printf(16, "BO1_RENDERFRAME {\"level_time\":%d,\"frame\":%ld,\"swap_ms\":%.1f,\"present_ms\":%.1f,\"gpu_ms\":%.1f,"
            "\"gpu_end_delta_ms\":%.1f,\"present_cpu_ms\":%.1f,\"present_proc_ms\":%.1f,\"present_main_ms\":%.1f,\"gpu_state\":%ld,\"avail_mb\":%u,\"counts\":[%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld],"
            "\"threads\":\"%s\",\"first\":[%ld,%ld,%ld,%ld,%ld,%ld],\"gpu_seg\":[%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f],\"prev\":[%s,%s],\"next\":%s}\n",
            level.time, f, e.swapMs, e.presentMs, e.gpuMs, e.gpuEndDeltaMs, e.presentCpuMs, e.presentProcMs, e.presentMainMs, e.state, e.availMB, c[0], c[1], c[2], c[3], c[4],
            c[5], c[6], c[7], c[8], c[9], FramePerfJsonText(e.threads).c_str(), e.first[0], e.first[1], e.first[2], e.first[3], e.first[4], e.first[5],
            e.gpuSeg[0], e.gpuSeg[1], e.gpuSeg[2], e.gpuSeg[3], e.gpuSeg[4], e.gpuSeg[5],
            e.gpuSeg[6], e.gpuSeg[7], FramePerfRenderBrief(f - 1).c_str(), FramePerfRenderBrief(f - 2).c_str(),
            FramePerfRenderBrief(f + 1).c_str());
    }
}

// JSON string body: names are material:technique, no quotes or backslashes expected; replace any to be safe.
std::string FramePerfJsonText(const char *s)
{
    std::string out(s);
    for (char &c : out)
        if (c == '"' || c == '\\' || (unsigned char)c < 32)
            c = '_';
    return out;
}

void FramePerfCalibrate()
{
    const long long f = Frequency();
    LARGE_INTEGER q0, q1;
    QueryPerformanceCounter(&q0);
    const unsigned long long t0 = __rdtsc();
    do
        QueryPerformanceCounter(&q1);
    while (q1.QuadPart - q0.QuadPart < f / 50);
    s_fpTscPerMs = (double)(__rdtsc() - t0) / ((double)(q1.QuadPart - q0.QuadPart) * 1000.0 / (double)f);
}

// Top zones by time: main thread (inclusive, outermost entry) and other threads, then clear every zone.
std::string FramePerfZones(bool report)
{
    std::vector<BO1ProfZone *> zones;
    for (BO1ProfZone *z = g_bo1ProfZones; z; z = z->next)
    {
        if (report && (z->mainTicks || z->otherTicks1k))
            zones.push_back(z);
    }
    std::string out;
    if (report)
    {
        char item[160];
        std::sort(zones.begin(), zones.end(), [](BO1ProfZone *a, BO1ProfZone *b) { return a->mainTicks > b->mainTicks; });
        out += ",\"main\":[";
        int n = 0;
        for (BO1ProfZone *z : zones)
        {
            const double ms = (double)z->mainTicks / s_fpTscPerMs;
            if (ms < 0.5 || n >= 14)
                break;
            sprintf_s(item, "%s[\"%s\",%.1f,%u]", n++ ? "," : "", z->name, ms, z->mainCalls);
            out += item;
        }
        std::sort(zones.begin(), zones.end(), [](BO1ProfZone *a, BO1ProfZone *b) { return a->otherTicks1k > b->otherTicks1k; });
        out += "],\"other\":[";
        n = 0;
        for (BO1ProfZone *z : zones)
        {
            const double ms = (double)z->otherTicks1k * 1024.0 / s_fpTscPerMs;
            if (ms < 1.0 || n >= 8)
                break;
            sprintf_s(item, "%s[\"%s\",%.1f]", n++ ? "," : "", z->name, ms);
            out += item;
        }
        out += "]";
    }
    for (BO1ProfZone *z = g_bo1ProfZones; z; z = z->next)
    {
        z->mainTicks = 0;
        z->mainCalls = 0;
        _InterlockedExchange(&z->otherTicks1k, 0);
    }
    return out;
}
}

void G_SP_FramePerfMark(int mark)
{
    const unsigned int hitchMs = Sys_FramePerfHitchMs();
    if (!hitchMs)
        return;
    const long long now = G_SP_MeasureTicks();
    if (mark != FRAMEPERF_BEGIN)
    {
        s_fpMarks[mark] = now;
        return;
    }
    s_fpMarks[FRAMEPERF_BEGIN] = s_fpLastBegin; // frame n-1: its BEGIN and (below) the marks it set
    s_fpLastBegin = now;
    if (!s_fpTscPerMs)
    {
        FramePerfCalibrate();
        g_bo1ProfMainThread = GetCurrentThreadId();
        g_bo1ProfOn = true;
        s_fpReportStart = now;
    }
    const double toMs = 1000.0 / (double)Frequency();
    FramePerfRender(hitchMs);
    if (s_fpHavePrev)
    {
        const double interval = (double)(now - s_fpMarks[FRAMEPERF_BEGIN]) * toMs;
        s_fpIntervals.push_back(interval);
        // L19: BO1_FRAMEFIRST - every frame of the first 30 s (real time) after client 0 first reaches CA_ACTIVE
        // (the streaming phase after spawn), pooled once: frames, p50 / p95 / p99 / max and counts over 20 / 33 ms.
        static std::vector<double> s_fpFirst30;
        static long long s_fpFirst30Start;
        static bool s_fpFirst30Done;
        if (!s_fpFirst30Done && CL_GetLocalClientConnectionState(0) == CA_ACTIVE)
        {
            if (!s_fpFirst30Start)
                s_fpFirst30Start = now;
            else
                s_fpFirst30.push_back(interval);
            if ((double)(now - s_fpFirst30Start) * toMs >= 30000.0)
            {
                s_fpFirst30Done = true;
                unsigned int over20 = 0, over33 = 0;
                for (double v : s_fpFirst30)
                {
                    if (v >= 20.0) ++over20;
                    if (v >= 33.3) ++over33;
                }
                Com_Printf(16, "BO1_FRAMEFIRST {\"level_time\":%d,\"frames\":%u,\"ms_p50\":%.2f,\"ms_p95\":%.2f,\"ms_p99\":%.2f,"
                    "\"ms_max\":%.1f,\"over20\":%u,\"over33\":%u}\n", level.time, (unsigned int)s_fpFirst30.size(),
                    Percentile(s_fpFirst30, 0.5), Percentile(s_fpFirst30, 0.95), Percentile(s_fpFirst30, 0.99),
                    Percentile(s_fpFirst30, 1.0), over20, over33);
            }
        }
        if (interval >= 16.7) ++s_fpOver16;
        if (interval >= 33.3) ++s_fpOver33;
        const bool hitch = interval >= (double)hitchMs;
        if (hitch) ++s_fpOverHitch;
        const std::string zones = FramePerfZones(hitch && s_fpHitchLines < 2000);
        FramePerfFirstUse();
        if (hitch && s_fpHitchLines < 2000)
        {
            ++s_fpHitchLines;
            const long long *p = s_fpMarks;
            // first / first_prev: [vs, ps, vs+ps pair, material pass, texture] first bound in this interval / the one before
            // (the render thread draws a frame while the main thread builds the next); names = material:technique
            Com_Printf(16, "BO1_FRAMEHITCH {\"level_time\":%d,\"interval_ms\":%.1f,\"sv_ms\":%.1f,\"pre_ms\":%.1f,\"cl_ms\":%.1f,"
                "\"scr_ms\":%.1f,\"spin_ms\":%.1f,\"rest_ms\":%.1f,\"first\":[%ld,%ld,%ld,%ld,%ld],\"first_prev\":[%ld,%ld,%ld,%ld,%ld],"
                "\"first_names\":\"%s\",\"first_names_prev\":\"%s\",\"names_dropped\":%d,\"combo\":[%ld,%ld]%s}\n",
                level.time, interval, (double)(p[FRAMEPERF_SV] - p[FRAMEPERF_BEGIN]) * toMs,
                (double)(p[FRAMEPERF_PRE] - p[FRAMEPERF_SV]) * toMs, (double)(p[FRAMEPERF_CL] - p[FRAMEPERF_PRE]) * toMs,
                (double)(p[FRAMEPERF_SCR] - p[FRAMEPERF_CL]) * toMs, (double)(p[FRAMEPERF_END] - p[FRAMEPERF_SCR]) * toMs,
                (double)(now - p[FRAMEPERF_END]) * toMs, s_fpFirst[0], s_fpFirst[1], s_fpFirst[2], s_fpFirst[3], s_fpFirst[4],
                s_fpFirstPrev[0], s_fpFirstPrev[1], s_fpFirstPrev[2], s_fpFirstPrev[3], s_fpFirstPrev[4],
                FramePerfJsonText(s_fpFirstNames).c_str(), FramePerfJsonText(s_fpFirstNamesPrev).c_str(),
                s_fpFirstDropped + s_fpFirstDroppedPrev, s_fpFirst[BO1_FIRST_COMBO], s_fpFirstPrev[BO1_FIRST_COMBO], zones.c_str());
        }
    }
    else
    {
        FramePerfZones(false);
        FramePerfFirstUse();
    }
    s_fpHavePrev = true;
    const double elapsedMs = (double)(now - s_fpReportStart) * toMs;
    if (elapsedMs < 5000.0)
        return;
    const size_t frames = s_fpIntervals.size();
    // machine load over the window (the PC may run other work): all CPUs busy % and this process's share, in % of all CPUs
    FILETIME idleF, kernelF, userF, createdF, exitedF, pKernelF, pUserF;
    GetSystemTimes(&idleF, &kernelF, &userF);
    GetProcessTimes(GetCurrentProcess(), &createdF, &exitedF, &pKernelF, &pUserF);
    auto u64 = [](const FILETIME &f) { return ((long long)f.dwHighDateTime << 32) + f.dwLowDateTime; };
    static long long s_idle, s_total, s_proc;
    const long long idle = u64(idleF), total = u64(kernelF) + u64(userF), proc = u64(pKernelF) + u64(pUserF);
    const double span = s_total ? (double)(total - s_total) : 0.0;
    const double sysBusy = span > 0.0 ? 100.0 * (1.0 - (double)(idle - s_idle) / span) : -1.0;
    const double procPct = span > 0.0 ? 100.0 * (double)(proc - s_proc) / span : -1.0;
    s_idle = idle;
    s_total = total;
    s_proc = proc;
    Com_Printf(16, "BO1_FRAMEPERF {\"level_time\":%d,\"frames\":%u,\"fps\":%.1f,\"ms_p50\":%.2f,\"ms_p99\":%.2f,\"ms_max\":%.1f,"
        "\"over16\":%u,\"over33\":%u,\"hitches\":%u,\"sv_frames\":%ld,\"sv_ms_max\":%.1f,\"sv_over25\":%ld,"
        "\"sys_busy_pct\":%.0f,\"proc_pct\":%.0f,\"first\":[%ld,%ld,%ld,%ld,%ld,%ld],\"gpu_ms_p50\":%.2f,\"gpu_ms_p99\":%.2f,"
        "\"gpu_ms_max\":%.1f,\"present_ms_max\":%.1f,\"avail_mb\":[%u,%u],\"rc\":[%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld]}\n",
        level.time, (unsigned int)frames, frames * 1000.0 / elapsedMs, Percentile(s_fpIntervals, 0.5),
        Percentile(s_fpIntervals, 0.99), Percentile(s_fpIntervals, 1.0), s_fpOver16, s_fpOver33, s_fpOverHitch,
        _InterlockedExchange(&s_fpSvFrames, 0), _InterlockedExchange(&s_fpSvMaxUs, 0) * 0.001, _InterlockedExchange(&s_fpSvOver25, 0),
        sysBusy, procPct, s_fpFirstWindow[0], s_fpFirstWindow[1], s_fpFirstWindow[2], s_fpFirstWindow[3], s_fpFirstWindow[4], s_fpFirstWindow[5],
        Percentile(s_fpGpuMs, 0.5), Percentile(s_fpGpuMs, 0.99), s_fpGpuMax, s_fpPresentMax,
        s_fpAvailMin == 0xFFFFFFFF ? 0 : s_fpAvailMin, s_fpAvailMax, s_fpRcWindow[0], s_fpRcWindow[1], s_fpRcWindow[2],
        s_fpRcWindow[3], s_fpRcWindow[4], s_fpRcWindow[5], s_fpRcWindow[6], s_fpRcWindow[7], s_fpRcWindow[8], s_fpRcWindow[9]);
    s_fpGpuMs.clear();
    s_fpGpuMax = s_fpPresentMax = 0.0;
    memset(s_fpRcWindow, 0, sizeof(s_fpRcWindow));
    s_fpAvailMin = 0xFFFFFFFF;
    s_fpAvailMax = 0;
    memset(s_fpFirstWindow, 0, sizeof(s_fpFirstWindow));
    // L8: run-level frame times, every window after the first (the first one holds the map load):
    // BO1_FRAMERUN frames / p50 / p95 / p99 / max since then, cumulative
    static std::vector<double> s_fpRun;
    static bool s_fpRunStarted;
    if (s_fpRunStarted)
    {
        s_fpRun.insert(s_fpRun.end(), s_fpIntervals.begin(), s_fpIntervals.end());
        std::vector<double> run(s_fpRun);
        Com_Printf(16, "BO1_FRAMERUN {\"level_time\":%d,\"frames\":%u,\"ms_p50\":%.2f,\"ms_p95\":%.2f,\"ms_p99\":%.2f,\"ms_max\":%.1f}\n",
            level.time, (unsigned int)run.size(), Percentile(run, 0.5), Percentile(run, 0.95), Percentile(run, 0.99),
            Percentile(run, 1.0));
    }
    s_fpRunStarted = true;
    s_fpIntervals.clear();
    s_fpOver16 = s_fpOver33 = s_fpOverHitch = 0;
    s_fpReportStart = now;
}

void G_SP_FramePerfServer(long long ticks)
{
    const unsigned int hitchMs = Sys_FramePerfHitchMs();
    if (!hitchMs || !s_fpTscPerMs)
        return;
    const double ms = (double)ticks * 1000.0 / (double)Frequency();
    _InterlockedIncrement(&s_fpSvFrames);
    const long us = (long)(ms * 1000.0);
    long old;
    while (us > (old = s_fpSvMaxUs) && _InterlockedCompareExchange(&s_fpSvMaxUs, us, old) != old)
        ;
    if (ms >= 25.0)
        _InterlockedIncrement(&s_fpSvOver25);
    if (ms >= (double)hitchMs)
        Com_Printf(16, "BO1_SVHITCH {\"level_time\":%d,\"ms\":%.1f}\n", level.time, ms);
}
