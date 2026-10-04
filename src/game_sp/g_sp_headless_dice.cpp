// k1: pinned script dice (TEST SWITCH bo1_testclient_dice <file>, headless harness only, off by default; no
// effect on normal play: the builtins only ask when the dvar is set in a headless run).
//
// Why: a retail recording has no RNG state and KB seeds from the wall clock, so the script draws that decide a
// round-1 zombie's route (which spawner, which walk anim, which attack spot) cannot be reproduced by seeding.
// Instead each such draw is identified by its CALL SITE (the script function the RandomInt / RandomIntRange
// builtin was called from, Scr_SP_CodePosFunction, needs bo1_scripterrors > 0) and its outcome is replaced by
// the recording's. The scripts still make every decision; only the die is fixed. The rules are the retail research's
// recordingDiceHook (engine/tools/scripted-round.ts); the values come from its recording analysis, exported by
// tools/k1_dice_export.mjs:
//   spawn n x y z   maps/_zombiemode::round_spawning  RandomInt(level.enemy_spawns.size): the index whose spawner
//                   is at the n-th zombie's recorded spawner (n = zombies spawned so far this level);
//                   maps/_zombiemode_zone_manager::create_spawner_list RandomIntRange(0, zone.spawners.size), the
//                   random_spawners trim: the first spawner of `zone` that the recording never used (it is removed)
//   spot n i        maps/_zombiemode_spawner::get_attack_spot_index RandomInt(indexes.size): the position of i
//                   in the local `indexes` (the free spots)
//   walk n v        maps/_zombiemode_spawner::set_zombie_run_cycle RandomIntRange(1, 8): walk<v>
//   run n v / sprint n v   (a1 chunk 9, rounds 2+) the same site's RandomIntRange(1, 6) / (1, 4): run<v> / sprint<v>;
//                   and maps/_zombiemode_spawner::set_run_speed RandomIntRange(level.zombie_move_speed, +35) returns
//                   the lowest value of that move type's band (<= 35 walk, <= 70 run, else sprint) when it is in
//                   range (a walk line pins "walk" too; round 1's range 1..35 is walk only, so session2 is unchanged)
// Tear phase (k1 chunk 9, same rules as recordingDiceHook; c = tear cycles started so far = AnimScripted
// "tear_anim" calls on this actor):
//   chunk n c x y z maps/_zombiemode_utility::get_closest_2d RandomIntRange(0, temp_array.size): the entry of
//                   temp_array at that chunk origin (3-D, <= 1 u; two chunks share x,y 12 u apart in z)
//   taunt n c s     maps/_zombiemode_spawner::do_a_taunt RandomInt(100): the recorded idle after cycle c, spent as
//                   a budget: 0 (taunt) while >= 1.45 s (shortest taunt anim) is left, else 99 (no taunt) unless
//                   tear_into_building has nowhere to go (no "repaired" chunk and not all "destroyed"), then 0;
//                   the taunt anim (common_scripts/utility::random) is the one whose length is closest to what is left
//   tauntanim n c k name   (k1 chunk 10) the k-th taunt anim of that idle, pinned by name: while cycle c has named
//                   anims left do_a_taunt draws 0 and utility::random the named one; the budget above only covers
//                   idles without names and the forced taunts after the names run out
//   flight x y z k dist power   maps/_zombiemode_blockers::remove_chunk launch k of the chunk at map origin x y z:
//                   RandomInt(100) #1 = dist - 100, #2 = power - 200 (#3, the 60/40 spin, stays on the RNG)
//   mantle n anim   common_scripts/utility::random over DoTraverse's traverseAnim: the recorded crossing's anim
// Every other draw stays on the RNG (the builtin's own G_irand is still made first, so the RNG sequence of the
// unpinned draws does not shift). Each pinned draw prints a "bo1_dice:" line.
#include "g_sp_headless_dice.h"
#include "scr_sp_debug.h"
#include <clientscript/cscr_vm.h>
#include <clientscript/cscr_variable.h>
#include <clientscript/cscr_stringlist.h>
#include <clientscript/cscr_compiler.h>
#include <clientscript/cscr_animtree.h>
#include <xanim/xanim.h>
#include <game_mp/g_main_mp.h>
#include <win32/win_main.h>
#include <universal/dvar.h>
#include <qcommon/common.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <vector>
#include <string>

namespace
{
struct DiceVec
{
    float v[3];
};

struct DiceZombie
{
    bool hasSpawn, hasSpot, hasWalk;
    float spawn[3];
    int spot, walk;
    int moveType; // a1 c9: 0 = not pinned, 1 walk, 2 run, 3 sprint (the variant is `walk`)
    std::vector<DiceVec> chunks;   // chunk origin torn on cycle c
    std::vector<float> taunts;     // recorded idle (s) after cycle c
    std::vector<std::vector<std::string>> tauntAnims; // taunt anims of the idle after cycle c, in order
    char mantle[64];
};

struct DiceFlight
{
    float origin[3];
    int launch, dist, power;
};

struct DiceActorState
{
    int tearCycles = 0;
    std::vector<float> budget;     // taunt seconds left per cycle (-1 = not started)
    bool tauntPending = false;
    int tauntCycle = 0;
    float tauntWant = 0.0f;
    std::vector<int> tauntAnimsUsed; // named taunt anims spent per cycle
};

struct DiceChunkState
{
    int key = -2;                  // index into s_flightChunks (-1 = no recorded flight), -2 = not looked up
    int launches = 0;
    int draw = 0;                  // RandomInt(100) draws in the current remove_chunk call (0 dist, 1 power, 2 spin)
    int pendingFlight = -1;
};

const dvar_t *g_dicePath;
bool s_diceLoaded, s_diceFailed, s_diceNoMap;
std::vector<DiceZombie> s_dice;
std::vector<DiceFlight> s_flights;
std::vector<int> s_meleeDamage; // a1 c14: recorded damage of the player's k-th melee hit on an actor
int s_meleeHits;                  // melee hits on actors this level
std::vector<DiceActorState> s_actors;  // by spawn order n, this level
DiceChunkState s_chunks[MAX_GENTITIES_SV];
std::vector<int> s_spawnedEnts;   // actor entity numbers in spawn order, this level
unsigned int s_levelId;
int s_pinned;

const float DICE_SPAWNER_XY = 8.0f; // spawner entity origins, same map data on both sides
const float DICE_CHUNK_3D = 1.0f;   // chunk origins (map data)
const float DICE_MIN_TAUNT = 1.45f; // the shortest level._zombie_board_taunt anim (ai_zombie_taunts_7 1.47 s)

DiceZombie &DiceAt(int n)
{
    if ((int)s_dice.size() <= n) s_dice.resize(n + 1, DiceZombie());
    return s_dice[n];
}

bool DiceLoad()
{
    if (s_diceLoaded || s_diceFailed)
        return s_diceLoaded;
    FILE *f = fopen(g_dicePath->current.string, "rb");
    if (!f)
    {
        s_diceFailed = true;
        Com_Printf(16, "bo1_dice: cannot open '%s'\n", g_dicePath->current.string);
        return false;
    }
    char line[256], name[64];
    while (fgets(line, sizeof(line), f))
    {
        int n = -1, a = 0, k = 0;
        float x, y, z;
        if (sscanf(line, "spawn %d %f %f %f", &n, &x, &y, &z) == 4 && n >= 0 && n < 256)
        {
            if ((int)s_dice.size() <= n) s_dice.resize(n + 1, DiceZombie());
            s_dice[n].hasSpawn = true;
            s_dice[n].spawn[0] = x; s_dice[n].spawn[1] = y; s_dice[n].spawn[2] = z;
        }
        else if (sscanf(line, "spot %d %d", &n, &a) == 2 && n >= 0 && n < 256)
        {
            if ((int)s_dice.size() <= n) s_dice.resize(n + 1, DiceZombie());
            s_dice[n].hasSpot = true;
            s_dice[n].spot = a;
        }
        else if (sscanf(line, "walk %d %d", &n, &a) == 2 && n >= 0 && n < 256)
        {
            if ((int)s_dice.size() <= n) s_dice.resize(n + 1, DiceZombie());
            s_dice[n].hasWalk = true;
            s_dice[n].walk = a;
            s_dice[n].moveType = 1;
        }
        else if ((sscanf(line, "run %d %d", &n, &a) == 2 || sscanf(line, "sprint %d %d", &n, &a) == 2) && n >= 0 && n < 256)
        {
            if ((int)s_dice.size() <= n) s_dice.resize(n + 1, DiceZombie());
            s_dice[n].hasWalk = true;
            s_dice[n].walk = a;
            s_dice[n].moveType = line[0] == 'r' ? 2 : 3;
        }
        else if (sscanf(line, "chunk %d %d %f %f %f", &n, &a, &x, &y, &z) == 5 && n >= 0 && n < 256 && a >= 0 && a < 64)
        {
            DiceZombie &d = DiceAt(n);
            if ((int)d.chunks.size() <= a) d.chunks.resize(a + 1, DiceVec{ { NAN, NAN, NAN } });
            d.chunks[a].v[0] = x; d.chunks[a].v[1] = y; d.chunks[a].v[2] = z;
        }
        else if (sscanf(line, "taunt %d %d %f", &n, &a, &x) == 3 && n >= 0 && n < 256 && a >= 0 && a < 64)
        {
            DiceZombie &d = DiceAt(n);
            if ((int)d.taunts.size() <= a) d.taunts.resize(a + 1, 0.0f);
            d.taunts[a] = x;
        }
        else if (sscanf(line, "tauntanim %d %d %d %63s", &n, &a, &k, name) == 4 && n >= 0 && n < 256 && a >= 0 && a < 64
                 && k >= 0 && k < 16)
        {
            DiceZombie &d = DiceAt(n);
            if ((int)d.tauntAnims.size() <= a) d.tauntAnims.resize(a + 1);
            if ((int)d.tauntAnims[a].size() <= k) d.tauntAnims[a].resize(k + 1);
            d.tauntAnims[a][k] = name;
        }
        else if (sscanf(line, "melee %d %d %d", &n, &a, &k) == 3 && n >= 0 && n < 256)
        {
            if ((int)s_meleeDamage.size() <= n)
                s_meleeDamage.resize(n + 1, -1);
            s_meleeDamage[n] = a;
        }
        else if (sscanf(line, "mantle %d %63s", &n, name) == 2 && n >= 0 && n < 256)
            strcpy_s(DiceAt(n).mantle, name);
        else
        {
            DiceFlight fl;
            if (sscanf(line, "flight %f %f %f %d %d %d", &fl.origin[0], &fl.origin[1], &fl.origin[2], &fl.launch, &fl.dist,
                    &fl.power) == 6)
                s_flights.push_back(fl);
        }
    }
    fclose(f);
    s_diceLoaded = !s_dice.empty();
    s_diceFailed = !s_diceLoaded;
    Com_Printf(16, "bo1_dice: loaded '%s' zombies %u ok %d\n", g_dicePath->current.string, (unsigned int)s_dice.size(),
        s_diceLoaded ? 1 : 0);
    return s_diceLoaded;
}

bool DiceActive()
{
    if (!Sys_IsHeadless())
        return false;
    if (!g_dicePath)
        g_dicePath = _Dvar_RegisterString("bo1_testclient_dice", "", 0,
            "TEST SWITCH (headless only): pin the script dice that pick a zombie's spawner / walk anim / attack spot to "
            "this exported recording (tools/k1_dice_export.mjs); needs bo1_scripterrors > 0");
    if (!g_dicePath->current.string[0])
        return false;
    if (gScrVarPub[SCRIPTINSTANCE_SERVER].levelId != s_levelId)
    {
        s_levelId = gScrVarPub[SCRIPTINSTANCE_SERVER].levelId;
        s_spawnedEnts.clear();
        s_actors.clear();
        s_meleeHits = 0;
        for (DiceChunkState &c : s_chunks)
            c = DiceChunkState();
    }
    return DiceLoad();
}

// Compiler identifiers (locals and fields) are canonical ids, not SL string ids; G_SP_HeadlessDiceFields saves
// them before Scr_EndLoadScripts releases the table.
const char *const s_diceNames[] = { "enemy_spawns", "zone", "spawners", "indexes", "temp_array", "first_node",
    "barrier_chunks", "state", "chunk", "keys", "array" };
unsigned int s_diceKeys[_countof(s_diceNames)];

// value of the variable `name` under object `parent` (a local of the calling function when parent is its localId)
bool DiceField(unsigned int parent, const char *name, unsigned int *type, VariableUnion *value)
{
    unsigned int key = 0;
    for (int i = 0; i < _countof(s_diceNames); ++i)
        if (!strcmp(name, s_diceNames[i])) key = s_diceKeys[i];
    const unsigned int id = parent && key ? FindVariable(SCRIPTINSTANCE_SERVER, parent, key) : 0;
    if (!id)
        return false;
    *type = GetValueType(SCRIPTINSTANCE_SERVER, id);
    *value = GetVariableValueAddress(SCRIPTINSTANCE_SERVER, id)->u;
    return true;
}

// the object an array / struct / entity pointer variable refers to (0 if it is not a pointer)
unsigned int DiceObject(unsigned int parent, const char *name)
{
    unsigned int type;
    VariableUnion value;
    return DiceField(parent, name, &type, &value) && type == VAR_POINTER ? value.pointerValue : 0;
}

bool DiceElement(unsigned int array, int i, unsigned int *type, VariableUnion *value)
{
    const unsigned int id = FindArrayVariable(SCRIPTINSTANCE_SERVER, array, i);
    if (!id)
        return false;
    *type = GetValueType(SCRIPTINSTANCE_SERVER, id);
    *value = GetVariableValueAddress(SCRIPTINSTANCE_SERVER, id)->u;
    return true;
}

const gentity_s *DiceEntity(unsigned int object)
{
    if (!object || GetObjectType(SCRIPTINSTANCE_SERVER, object) != VAR_ENTITY)
        return nullptr;
    const scr_entref_t ref = Scr_GetEntityIdRef(SCRIPTINSTANCE_SERVER, object);
    return ref.classnum == 0 && ref.entnum < MAX_GENTITIES ? &g_entities[ref.entnum] : nullptr;
}

const gentity_s *DiceArrayEntity(unsigned int array, int i)
{
    unsigned int type;
    VariableUnion value;
    return DiceElement(array, i, &type, &value) && type == VAR_POINTER ? DiceEntity(value.pointerValue) : nullptr;
}

bool DiceNearXY(const float *a, const float *b)
{
    return hypotf(a[0] - b[0], a[1] - b[1]) <= DICE_SPAWNER_XY;
}

// n of the calling thread's self (an actor), or -1
int DiceSelfIndex(unsigned int localId)
{
    const gentity_s *self = DiceEntity(Scr_GetSelf(SCRIPTINSTANCE_SERVER, localId));
    if (!self)
        return -1;
    for (int n = (int)s_spawnedEnts.size() - 1; n >= 0; --n)
        if (s_spawnedEnts[n] == self->s.number)
            return n;
    return -1;
}

int DiceIndexOfEnt(int entnum)
{
    for (int n = (int)s_spawnedEnts.size() - 1; n >= 0; --n)
        if (s_spawnedEnts[n] == entnum)
            return n;
    return -1;
}

DiceActorState &DiceActor(int n)
{
    if ((int)s_actors.size() <= n) s_actors.resize(n + 1);
    return s_actors[n];
}

bool DiceNear3D(const float *a, const float *b)
{
    return sqrtf((a[0] - b[0]) * (a[0] - b[0]) + (a[1] - b[1]) * (a[1] - b[1]) + (a[2] - b[2]) * (a[2] - b[2]))
        <= DICE_CHUNK_3D;
}

// _zombiemode_spawner::tear_into_building: may the loop be told NOT to taunt? Only when it has somewhere to go: a
// chunk still "repaired" to pick, or every chunk "destroyed" (it returns). Otherwise it spins inside one frame until
// the taunt yields (the rebuild's tauntCanBeSkipped).
bool DiceTauntCanBeSkipped(const gentity_s *self)
{
    const unsigned int object = FindEntityId(SCRIPTINSTANCE_SERVER, self->s.number, 0, 0);
    const unsigned int node = object ? DiceObject(object, "first_node") : 0;
    const unsigned int chunks = node ? DiceObject(node, "barrier_chunks") : 0;
    const int size = chunks ? (int)GetArraySize(SCRIPTINSTANCE_SERVER, chunks) : 0;
    if (!size)
        return false;
    bool anyRepaired = false, allDestroyed = true;
    for (int i = 0; i < size; ++i)
    {
        unsigned int type, chunkObject = 0;
        VariableUnion value;
        if (DiceElement(chunks, i, &type, &value) && type == VAR_POINTER)
            chunkObject = value.pointerValue;
        const char *state = "";
        if (chunkObject && DiceField(chunkObject, "state", &type, &value) && type == VAR_STRING)
            state = SL_ConvertToString(value.stringValue, SCRIPTINSTANCE_SERVER);
        anyRepaired = anyRepaired || !strcmp(state, "repaired");
        allDestroyed = allDestroyed && !strcmp(state, "destroyed");
    }
    return anyRepaired || allDestroyed;
}

// the anim behind array[keys[i]] of common_scripts/utility::random (int keys only)
bool DiceRandomAnim(unsigned int localId, int i, const XAnim_s **anims, unsigned int *index)
{
    const unsigned int keys = DiceObject(localId, "keys"), array = DiceObject(localId, "array");
    unsigned int type;
    VariableUnion key, value;
    if (!keys || !array || !DiceElement(keys, i, &type, &key) || type != VAR_INTEGER)
        return false;
    if (!DiceElement(array, key.intValue, &type, &value) || type != VAR_ANIMATION)
        return false;
    const scr_anim_s anim = *(const scr_anim_s *)&value.intValue;
    *anims = Scr_GetAnims(anim.tree, SCRIPTINSTANCE_SERVER);
    *index = anim.index;
    return *anims && *index < (*anims)->size;
}

const char *DiceAnimName(const XAnim_s *anims, unsigned int index)
{
    const XAnimParts *parts = anims->entries[index].parts;
    return parts && parts->name ? parts->name : "";
}

int DicePinSite(const char *fn, int lo, int hi, unsigned int localId, int *n)
{
    *n = -1;
    if (!strcmp(fn, "maps/_zombiemode_utility::get_closest_2d") && lo == 0)
    {
        *n = DiceSelfIndex(localId);
        if (*n < 0 || *n >= (int)s_dice.size())
            return -1;
        const int c = DiceActor(*n).tearCycles;
        if (c >= (int)s_dice[*n].chunks.size() || isnan(s_dice[*n].chunks[c].v[0]))
            return -1;
        const unsigned int list = DiceObject(localId, "temp_array");
        if (!list || (int)GetArraySize(SCRIPTINSTANCE_SERVER, list) != hi)
            return -1;
        for (int i = 0; i < hi; ++i)
        {
            const gentity_s *chunk = DiceArrayEntity(list, i);
            if (chunk && DiceNear3D(chunk->r.currentOrigin, s_dice[*n].chunks[c].v))
                return i;
        }
        return -1; // the recorded chunk is gone here
    }
    if (!strcmp(fn, "maps/_zombiemode_spawner::do_a_taunt") && lo == 0 && hi == 100)
    {
        *n = DiceSelfIndex(localId);
        if (*n < 0 || *n >= (int)s_dice.size())
            return -1;
        DiceActorState &a = DiceActor(*n);
        const int c = a.tearCycles; // do_a_taunt runs right after cycle c
        if ((int)a.budget.size() <= c) a.budget.resize(c + 1, -1.0f);
        if (a.budget[c] < 0.0f)
            a.budget[c] = c < (int)s_dice[*n].taunts.size() ? s_dice[*n].taunts[c] : 0.0f;
        if ((int)a.tauntAnimsUsed.size() <= c) a.tauntAnimsUsed.resize(c + 1, 0);
        const int named = c < (int)s_dice[*n].tauntAnims.size() ? (int)s_dice[*n].tauntAnims[c].size() : 0;
        if (named && a.tauntAnimsUsed[c] >= named)
            a.budget[c] = 0.0f; // the named anims are the recorded idle: a forced taunt past them is the shortest
        const float left = a.budget[c];
        if (named)
        {
            if (a.tauntAnimsUsed[c] >= named && DiceTauntCanBeSkipped(&g_entities[s_spawnedEnts[*n]]))
                return 99;
        }
        else if (left < DICE_MIN_TAUNT && DiceTauntCanBeSkipped(&g_entities[s_spawnedEnts[*n]]))
            return 99; // 5 >= 99 is false: no taunt
        a.tauntPending = true;
        a.tauntCycle = c;
        a.tauntWant = left;
        return 0;
    }
    if (!strcmp(fn, "common_scripts/utility::random") && lo == 0)
    {
        *n = DiceSelfIndex(localId);
        if (*n < 0 || *n >= (int)s_dice.size())
            return -3;
        DiceActorState &a = DiceActor(*n);
        const XAnim_s *anims;
        unsigned int index;
        if (a.tauntPending && a.tauntCycle < (int)s_dice[*n].tauntAnims.size()
            && a.tauntAnimsUsed[a.tauntCycle] < (int)s_dice[*n].tauntAnims[a.tauntCycle].size())
        {
            const std::string &want = s_dice[*n].tauntAnims[a.tauntCycle][a.tauntAnimsUsed[a.tauntCycle]];
            for (int i = 0; i < hi; ++i)
            {
                if (!DiceRandomAnim(localId, i, &anims, &index))
                    return -3; // not the taunt list
                if (want == DiceAnimName(anims, index))
                {
                    a.tauntPending = false;
                    a.tauntAnimsUsed[a.tauntCycle]++;
                    return i;
                }
            }
            return -3; // the recorded anim is not offered
        }
        if (a.tauntPending)
        {
            int best = -1;
            float bestDelta = 1e9f, bestLength = 0.0f;
            for (int i = 0; i < hi; ++i)
            {
                if (!DiceRandomAnim(localId, i, &anims, &index))
                    return -3; // not the taunt list
                const float length = XAnimGetLength(anims, index);
                if (fabsf(length - a.tauntWant) < bestDelta)
                {
                    bestDelta = fabsf(length - a.tauntWant);
                    best = i;
                    bestLength = length;
                }
            }
            a.tauntPending = false;
            a.budget[a.tauntCycle] = a.tauntWant - bestLength > 0.0f ? a.tauntWant - bestLength : 0.0f;
            return best;
        }
        if (!s_dice[*n].mantle[0])
            return -3;
        for (int i = 0; i < hi; ++i)
            if (DiceRandomAnim(localId, i, &anims, &index) && !strcmp(DiceAnimName(anims, index), s_dice[*n].mantle))
                return i;
        return -3; // not the traverse list, or the recorded anim is not offered
    }
    if (!strcmp(fn, "maps/_zombiemode_blockers::remove_chunk") && lo == 0 && hi == 100)
    {
        const gentity_s *chunk = DiceEntity(DiceObject(localId, "chunk"));
        if (!chunk)
            return -1;
        DiceChunkState &st = s_chunks[chunk->s.number];
        const int draw = st.draw;
        st.draw = (st.draw + 1) % 3;
        if (draw == 0)
        {
            if (st.key == -2)
            {
                // the first launch finds the chunk at its map origin, which names it; later launches start
                // wherever the chunk has got to, so the identity is remembered
                st.key = -1;
                for (size_t i = 0; i < s_flights.size() && st.key < 0; ++i)
                    if (DiceNear3D(chunk->r.currentOrigin, s_flights[i].origin))
                        st.key = (int)i;
            }
            const int launch = st.launches++;
            st.pendingFlight = -1;
            if (st.key >= 0)
                for (size_t i = 0; i < s_flights.size(); ++i)
                    if (s_flights[i].launch == launch && DiceNear3D(s_flights[i].origin, s_flights[st.key].origin))
                        st.pendingFlight = (int)i;
            return st.pendingFlight >= 0 ? s_flights[st.pendingFlight].dist - 100 : -1;
        }
        if (draw == 1)
            return st.pendingFlight >= 0 ? s_flights[st.pendingFlight].power - 200 : -1;
        return -3; // the 60/40 spin: moves nothing, stays on the RNG
    }
    if (!strcmp(fn, "maps/_zombiemode::round_spawning") && lo == 0)
    {
        *n = (int)s_spawnedEnts.size();
        if (*n >= (int)s_dice.size() || !s_dice[*n].hasSpawn)
            return -1;
        const unsigned int list = DiceObject(gScrVarPub[SCRIPTINSTANCE_SERVER].levelId, "enemy_spawns");
        if (!list || (int)GetArraySize(SCRIPTINSTANCE_SERVER, list) != hi)
            return -1; // not the enemy_spawns draw (the RandomInt(100) special-spawn chances)
        for (int i = 0; i < hi; ++i)
        {
            const gentity_s *s = DiceArrayEntity(list, i);
            if (s && DiceNearXY(s->r.currentOrigin, s_dice[*n].spawn))
                return i;
        }
        return -1;
    }
    if (!strcmp(fn, "maps/_zombiemode_zone_manager::create_spawner_list") && lo == 0)
    {
        const unsigned int zone = DiceObject(localId, "zone");
        const unsigned int list = zone ? DiceObject(zone, "spawners") : 0;
        if (!list || (int)GetArraySize(SCRIPTINSTANCE_SERVER, list) != hi)
            return -1;
        for (int i = 0; i < hi; ++i)
        {
            const gentity_s *s = DiceArrayEntity(list, i);
            if (!s)
                continue;
            bool used = false;
            for (const DiceZombie &z : s_dice)
                used = used || (z.hasSpawn && DiceNearXY(s->r.currentOrigin, z.spawn));
            if (!used)
                return i;
        }
        return -1;
    }
    if (!strcmp(fn, "maps/_zombiemode_spawner::get_attack_spot_index") && lo == 0)
    {
        *n = DiceSelfIndex(localId);
        if (*n < 0 || *n >= (int)s_dice.size() || !s_dice[*n].hasSpot)
            return -1;
        const unsigned int indexes = DiceObject(localId, "indexes");
        if (!indexes)
            return -1;
        for (int k = 0; k < hi; ++k)
        {
            unsigned int type;
            VariableUnion value;
            if (DiceElement(indexes, k, &type, &value) && type == VAR_INTEGER && value.intValue == s_dice[*n].spot)
                return k;
        }
        return -1; // the recorded spot is taken here
    }
    if (!strcmp(fn, "maps/_zombiemode_spawner::set_zombie_run_cycle") && lo == 1 && (hi == 8 || hi == 6 || hi == 4))
    {
        *n = DiceSelfIndex(localId);
        if (*n < 0 || *n >= (int)s_dice.size() || !s_dice[*n].hasWalk || s_dice[*n].moveType != (hi == 8 ? 1 : hi == 6 ? 2 : 3))
            return -1;
        return s_dice[*n].walk >= lo && s_dice[*n].walk < hi ? s_dice[*n].walk : -1;
    }
    if (!strcmp(fn, "maps/_zombiemode_spawner::set_run_speed") && hi == lo + 35)
    {
        // a1 c9: the move type band's lowest value in [lo, hi) (walk <= 35 < run <= 70 < sprint)
        *n = DiceSelfIndex(localId);
        if (*n < 0 || *n >= (int)s_dice.size() || !s_dice[*n].moveType)
            return -1;
        const int bandLo = s_dice[*n].moveType == 1 ? 0 : s_dice[*n].moveType == 2 ? 36 : 71;
        const int bandHi = s_dice[*n].moveType == 1 ? 35 : s_dice[*n].moveType == 2 ? 70 : 0x7FFFFFFF;
        const int v = lo > bandLo ? lo : bandLo;
        return v < hi && v <= bandHi ? v : -1;
    }
    return -2; // not a pinned site
}
} // namespace

void G_SP_HeadlessDiceFields(int inst)
{
    if (inst != SCRIPTINSTANCE_SERVER)
        return;
    for (int i = 0; i < _countof(s_diceNames); ++i)
    {
        const unsigned int string = SL_FindString(s_diceNames[i], SCRIPTINSTANCE_SERVER);
        s_diceKeys[i] = string ? gScrCompilePub[SCRIPTINSTANCE_SERVER].canonicalStrings[string] : 0;
    }
}

void G_SP_HeadlessDiceAnimScripted(const gentity_s *ent, unsigned int notifyName)
{
    if (!ent || !notifyName || !DiceActive() || strcmp(SL_ConvertToString(notifyName, SCRIPTINSTANCE_SERVER), "tear_anim"))
        return;
    const int n = DiceIndexOfEnt(ent->s.number);
    if (n >= 0)
        ++DiceActor(n).tearCycles;
}

void G_SP_HeadlessDiceActorSpawned(const gentity_s *ent)
{
    if (ent && DiceActive())
        s_spawnedEnts.push_back(ent->s.number);
}

bool G_SP_HeadlessDicePin(int lo, int hi, int *value)
{
    if (!DiceActive() || !gScrVmPub[SCRIPTINSTANCE_SERVER].function_count)
        return false;
    const function_stack_t &fs = gScrVmPub[SCRIPTINSTANCE_SERVER].function_frame->fs;
    const char *fn = Scr_SP_CodePosFunction(SCRIPTINSTANCE_SERVER, fs.pos - 1);
    if (!fn)
    {
        if (!s_diceNoMap)
            Com_Printf(16, "bo1_dice: no call-site map (set bo1_scripterrors > 0); dice not pinned\n");
        s_diceNoMap = true;
        return false;
    }
    int n;
    const int pinned = DicePinSite(fn, lo, hi, fs.localId, &n);
    if (pinned == -2 || pinned == -3)
        return false;
    if (pinned < 0)
    {
        Com_Printf(16, "bo1_dice: time %d %s(%d,%d) n %d rng %d not pinned\n", level.time, fn, lo, hi, n, *value);
        return false;
    }
    ++s_pinned;
    Com_Printf(16, "bo1_dice: time %d %s(%d,%d) n %d rng %d -> %d pinned %d\n", level.time, fn, lo, hi, n, *value, pinned,
        s_pinned);
    *value = pinned;
    return true;
}

bool G_SP_HeadlessDiceMelee(int baseDamage, int targetEnt, int *roll)
{
    if (!DiceActive())
        return false;
    const int k = s_meleeHits++;
    if (k >= (int)s_meleeDamage.size() || s_meleeDamage[k] < 0)
        return false;
    const int want = s_meleeDamage[k] - baseDamage;
    if (want < 0 || want > 4)
    {
        Com_Printf(16, "bo1_dice: time %d melee %d target %d recorded %d base %d: not a roll, not pinned\n", level.time, k,
            targetEnt, s_meleeDamage[k], baseDamage);
        return false;
    }
    ++s_pinned;
    Com_Printf(16, "bo1_dice: time %d melee %d target %d rng %d -> %d pinned %d\n", level.time, k, targetEnt, *roll % 5, want,
        s_pinned);
    *roll = want;
    return true;
}

unsigned int G_SP_HeadlessSeed(unsigned int wallClockSeed)
{
    if (!Sys_IsHeadless())
        return wallClockSeed;
    static const dvar_t *seed;
    if (!seed)
        seed = _Dvar_RegisterInt("bo1_testclient_seed", 0, 0, 0x7FFFFFFF, 0,
            "TEST SWITCH (headless only): G_InitGame's random seed; 0 = the wall clock");
    if (!seed->current.integer)
        return wallClockSeed;
    Com_Printf(16, "bo1_seed: %d (wall clock %u)\n", seed->current.integer, wallClockSeed);
    return (unsigned int)seed->current.integer;
}
