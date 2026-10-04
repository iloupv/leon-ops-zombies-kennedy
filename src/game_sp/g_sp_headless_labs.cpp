// TEST SWITCH (headless only): bo1_testclient_labs.
//
// Five's special enemies live on the labs floor: the nova crawler (quad) spawners belong to the zones
// labs_hallway1/2 and the thief spawns in the power room, after the power switch there is used
// (maps\_zombiemode_ai_thief::thief_round_tracker waits for flag "power_on"). A player reaches the labs
// by two elevator rides: elevator2 (floor 1 -> war room) and elevator1 (war room -> labs). Driving the
// elevators in general is lane f1's harness work. For the special-enemy runs this switch moves the
// headless fighting test client once, early in the level, into the car of "elevator1" (standing inside
// its "elevator1_up_riders" trigger), so only the first ride is skipped. From then on the harness acts
// like a player and the scripts decide everything: it presses use on the car's buy trigger
// (maps\zombie_pentagon_elevators::elevator_buy_think, 250 points), the car carries it down
// (elevator_move_to sets flag "labs_enabled"; the car's elevator1_down_riders trigger enables zone
// labs_hallway1), then it walks to the map's "use_elec_switch" trigger and presses use
// (maps\zombie_pentagon::electric_switch sets power_on).
//
// Before the move it buys the solo Quick Revive on the spawn floor (as the default harness goal does) and
// waits for the 250 points of the ride; after the power it buys the labs AK74u (and its ammo) at the wall,
// like a player; see SP_LabsWeaponGoal.
// Never writes script state, score, health or ammunition. Registered only in headless runs.
#include "g_sp_headless_labs.h"
#include "g_sp_levelstart.h"
#include <game_mp/g_main_mp.h>
#include <game_mp/g_misc_mp.h>
#include <game_mp/g_utils_mp.h>
#include <win32/win_main.h>
#include <universal/dvar.h>
#include <qcommon/common.h>
#include <server/sv_world.h>
#include <qcommon/cm_trace.h>
#include <clientscript/scr_const.h>
#include <clientscript/cscr_stringlist.h>
#include <clientscript/cscr_compiler.h>
#include <game/actor_navigation.h>
#include <game/actor.h>
#include <game_mp/actor_mp.h>
#include <bgame/bg_local.h>
#include <bgame/bg_weapons.h>
#include <bgame/bg_weapons_def.h>
#include <bgame/bg_misc.h>
#include <bgame/bg_mantle.h>
#include "g_sp_player_state.h"
#include <cstring>

static const dvar_s *g_headlessLabs;
static bool g_labsMoved;

void G_SP_HeadlessLabsInit()
{
    g_labsMoved = false;
    if (Sys_IsHeadless())
        g_headlessLabs = _Dvar_RegisterBool("bo1_testclient_labs", false, 0,
            "TEST SWITCH (headless test client only): move the fighting test client next to Five's labs elevator "
            "(into the elevator1 car, riders trigger) early in the level; it then rides elevator1 down and uses use_elec_switch");
}

bool G_SP_HeadlessLabsActive()
{
    return Sys_IsHeadless() && G_SP_IsZombieMode() && g_headlessLabs && g_headlessLabs->current.enabled;
}

static bool SP_LabsHasTargetname(const gentity_s *ent, const char *name)
{
    return ent->r.inuse && ent->targetname
        && !strcmp(SL_ConvertToString(ent->targetname, SCRIPTINSTANCE_SERVER), name);
}

static gentity_s *SP_LabsFindTargetname(const char *name)
{
    for (int i = 0; i < level.num_entities; ++i)
    {
        if (SP_LabsHasTargetname(&g_entities[i], name))
            return &g_entities[i];
    }
    return nullptr;
}

static void SP_LabsCenter(const gentity_s *ent, float *center)
{
    for (int i = 0; i < 3; ++i)
        center[i] = 0.5f * (ent->r.absmin[i] + ent->r.absmax[i]);
}

// Floor point under a trigger's center where a standing player hull fits.
static bool SP_LabsStandPoint(gentity_s *player, const gentity_s *trigger, float *point)
{
    float center[3], down[3];
    SP_LabsCenter(trigger, center);
    Vec3Copy(center, down);
    down[2] -= 256.0f;
    trace_t trace;
    col_context_t context;
    G_TraceCapsule(&trace, center, vec3_origin, vec3_origin, down, player->s.number, player->clipmask, &context);
    if (trace.startsolid || trace.fraction >= 1.0f)
        return false;
    Vec3Lerp(center, down, trace.fraction, point);
    point[2] += 1.0f;
    float up[3] = { point[0], point[1], point[2] + 1.0f };
    G_TraceCapsule(&trace, point, playerMins, playerMaxs, up, player->s.number, player->clipmask, &context);
    return !trace.startsolid && !trace.allsolid;
}

static gentity_s *SP_LabsFindPiece(const char *owner, int classname)
{
    const gentity_s *car = SP_LabsFindTargetname(owner);
    if (!car || !car->target)
        return nullptr;
    const char *pieces = SL_ConvertToString(car->target, SCRIPTINSTANCE_SERVER);
    for (int i = 0; i < level.num_entities; ++i)
    {
        gentity_s *ent = &g_entities[i];
        if (ent->classname == classname && SP_LabsHasTargetname(ent, pieces))
            return ent;
    }
    return nullptr;
}

// Once: stand the player in elevator1's car, inside its elevator1_up_riders trigger.
static void SP_LabsMove(gentity_s *player)
{
    g_labsMoved = true;
    const gentity_s *riders = SP_LabsFindTargetname("elevator1_up_riders");
    float point[3];
    if (!riders || !SP_LabsStandPoint(player, riders, point))
    {
        Com_Printf(16, "bo1_labs: elevator1_up_riders missing or no standing room; test client not moved\n");
        return;
    }
    float angles[3] = { 0.0f, player->client->ps.viewangles[1], 0.0f };
    TeleportPlayer(player, point, angles);
    Com_Printf(16, "bo1_labs: test client moved into elevator1 at %.1f %.1f %.1f (trigger ent %d) at %d\n",
        point[0], point[1], point[2], riders->s.number, level.time);
}

// The trigger_use that targets `target` (a purchase: wall weapon, perk machine).
static gentity_s *SP_LabsFindTrigger(const char *target)
{
    for (int i = 0; i < level.num_entities; ++i)
    {
        gentity_s *ent = &g_entities[i];
        if (ent->r.inuse && ent->classname == scr_const.trigger_use && ent->target
            && !strcmp(SL_ConvertToString(ent->target, SCRIPTINSTANCE_SERVER), target))
            return ent;
    }
    return nullptr;
}

// After the power: the labs' AK74u wall buy (weapon_upgrade trigger "ak74u_zm", which targets
// pf884_auto37), so the test client can hold the labs through rounds 5-8 where the thief round is
// drawn. Harness policy only: walk there and press use when the score could pay for the gun (1200) or,
// once it is held and empty, its ammo (600); maps\_zombiemode_weapons::weapon_spawn_think decides.
static gentity_s *SP_LabsWeaponGoal(gentity_s *player)
{
    const unsigned int weapon = BG_FindWeaponIndexForName("ak74u_zm"); // lookup only, never registers
    const playerState_s *ps = &player->client->ps;
    const int score = player->client->sess.cs.score.score;
    if (!weapon)
        return nullptr;
    if (BG_PlayerHasWeapon(ps, weapon))
    {
        if (score < 600 || BG_GetAmmoInClip(ps, weapon) + BG_GetAmmoNotInClip(ps, weapon) > 0)
            return nullptr;
    }
    else if (score < 1200)
    {
        return nullptr;
    }
    return SP_LabsFindTrigger("pf884_auto37");
}

gentity_s *G_SP_HeadlessLabsGoal(gentity_s *player)
{
    // electric_switch deletes use_elec_switch once used, so no trigger = power is on.
    gentity_s *power = SP_LabsFindTargetname("use_elec_switch");
    if (!power)
        return SP_LabsWeaponGoal(player);
    if (!g_labsMoved)
    {
        // First, on the spawn floor, the solo Quick Revive (500) like the default harness goal, then the
        // move once the elevator1 ride (250) is affordable.
        if (!G_SP_HasPerk(player->client, "specialty_quickrevive"))
            return player->client->sess.cs.score.score >= 500 ? SP_LabsFindTrigger("vending_revive") : nullptr;
        if (player->client->sess.cs.score.score >= 250)
            SP_LabsMove(player);
        return nullptr;
    }
    float powerCenter[3];
    SP_LabsCenter(power, powerCenter);
    // On the switch's floor: walk to it and press use.
    if (fabsf(player->r.currentOrigin[2] - powerCenter[2]) < 128.0f)
        return power;
    // Still upstairs: press use on the car's buy trigger while it is in reach (elevator_buy_think moves
    // it away with trigger_off while the car travels).
    gentity_s *buy = SP_LabsFindPiece("elevator1", scr_const.trigger_use);
    if (!buy)
        return nullptr;
    float buyCenter[3];
    SP_LabsCenter(buy, buyCenter);
    return Vec3DistanceSq(buyCenter, player->r.currentOrigin) < 200.0f * 200.0f ? buy : nullptr;
}

// The Pentagon Thief (aitype actor_zombie_electrician, maps\_zombiemode_ai_thief) while alive, for the
// labs switch's fight policy: once it has stolen a weapon it runs through its portals and leaves after
// about 40 s, and thief_return_loot (the GiveWeapon with get_pack_a_punch_weapon_options) only runs when a
// player kills it first. Harness policy only: with a loaded weapon the test client shoots the thief before
// other zombies and walks toward it when it is out of sight; the scripts and the damage code decide the rest.
gentity_s *G_SP_HeadlessLabsThief(gentity_s *player)
{
    if (!G_SP_HeadlessLabsActive() || !level.actors)
        return nullptr;
    const playerState_s *ps = &player->client->ps;
    const unsigned int weapon = ps->weapon;
    if (!weapon || BG_GetAmmoInClip(ps, weapon) + BG_GetAmmoNotInClip(ps, weapon) <= 0)
        return nullptr;
    for (int i = 0; i < MAX_ACTORS; ++i)
    {
        actor_s *actor = &level.actors[i];
        gentity_s *ent = actor->ent;
        if (actor->inuse && ent && ent->health > 0 && actor->Physics.bIsAlive && ent->classname
            && !strcmp(SL_ConvertToString(ent->classname, SCRIPTINSTANCE_SERVER), "actor_zombie_electrician"))
            return ent;
    }
    return nullptr;
}

// TEST SWITCH (headless test client only): bo1_testclient_round N.
//
// Rounds 13-15 take about 15 min of fighting from mutator_quickStart's round 5, longer than the test
// client survives. The retail way to start later is maps\_zombiemode::difficulty_init: mutator_quickStart
// sets level.first_round = false and level.round_number = 5. This switch writes the SAME variable,
// level.round_number = N, once, when round_start sets flag "begin_spawning" (flag_set notifies level).
// That is after difficulty_init and before round_start threads round_think, so round_think and
// round_spawning (ai_calculate_health( level.round_number ), zombie_total) start from round N exactly as
// they start from quickStart's 5. Use it together with mutator_quickStart 1 (first_round false).
// Only in headless runs with bo1_testclient 1; writes nothing else.
static const dvar_s *g_headlessRound;
static unsigned int g_roundNumberKey;
// L26 TEST SWITCH (headless test client only): bo1_testclient_dogs 1 makes every round after round 1 a dog round.
// It is the body of maps\_zombiemode_ai_dogs' /# force_dogs #/ devgui dvar (developer_script only in retail):
// level.next_dog_round = level.round_number, written on the level notify "between_round_over" (sent after
// round_number++), before dog_round_tracker's waittill wakes and compares the two.
static const dvar_s *g_headlessDogs;
static unsigned int g_nextDogRoundKey;

void G_SP_HeadlessRoundFields(scriptInstance_t inst)
{
    if (inst != SCRIPTINSTANCE_SERVER || !Sys_IsHeadless())
        return;
    // k1 harness: loading client scripts must not erase the server canonical key.
    g_roundNumberKey = 0;
    g_nextDogRoundKey = 0;
    if (!g_headlessDogs)
        g_headlessDogs = _Dvar_RegisterBool("bo1_testclient_dogs", false, 0,
            "TEST SWITCH (headless test client only): force_dogs - level.next_dog_round = level.round_number at "
            "between_round_over");
    if (!g_headlessRound)
        g_headlessRound = _Dvar_RegisterInt("bo1_testclient_round", 0, 0, 100, 0,
            "TEST SWITCH (headless test client only): set level.round_number to N when round_start sets "
            "flag begin_spawning (use with mutator_quickStart 1); 0 = off");
    // Compiler identifiers are canonical IDs; save the mapping before Scr_EndLoadScripts releases them.
    unsigned int string = SL_FindString("round_number", inst);
    g_roundNumberKey = string ? gScrCompilePub[inst].canonicalStrings[string] : 0;
    string = SL_FindString("next_dog_round", inst);
    g_nextDogRoundKey = string ? gScrCompilePub[inst].canonicalStrings[string] : 0;
}

static void G_SP_HeadlessDogsNotify(scriptInstance_t inst, unsigned int owner, unsigned int name)
{
    if (!g_headlessDogs || !g_headlessDogs->current.enabled || !g_nextDogRoundKey || !g_roundNumberKey
        || !name || strcmp(SL_ConvertToString(name, inst), "between_round_over"))
        return;
    unsigned int roundId = FindVariable(inst, owner, g_roundNumberKey);
    unsigned int dogId = FindVariable(inst, owner, g_nextDogRoundKey);
    if (!roundId || !dogId || GetValueType(inst, roundId) != VAR_INTEGER || GetValueType(inst, dogId) != VAR_INTEGER)
        return;
    const int round = GetVariableValueAddress(inst, roundId)->u.intValue;
    VariableUnion &value = GetVariableValueAddress(inst, dogId)->u;
    Com_Printf(16, "TEST SWITCH bo1_testclient_dogs: level.next_dog_round %d -> %d\n", value.intValue, round);
    value.intValue = round;
}

void G_SP_HeadlessRoundNotify(scriptInstance_t inst, unsigned int owner, unsigned int name)
{
    if (inst == SCRIPTINSTANCE_SERVER && owner == gScrVarPub[inst].levelId && Sys_IsHeadless() && G_SP_IsZombieMode()
        && Dvar_GetBool("bo1_testclient"))
        G_SP_HeadlessDogsNotify(inst, owner, name);
    if (inst != SCRIPTINSTANCE_SERVER || owner != gScrVarPub[inst].levelId || !g_roundNumberKey
        || !g_headlessRound || g_headlessRound->current.integer <= 0 || !Sys_IsHeadless()
        || !G_SP_IsZombieMode() || !Dvar_GetBool("bo1_testclient"))
        return;
    if (!name || strcmp(SL_ConvertToString(name, inst), "begin_spawning"))
        return;
    unsigned int id = FindVariable(inst, owner, g_roundNumberKey);
    if (!id || GetValueType(inst, id) != VAR_INTEGER)
        return;
    VariableUnion &value = GetVariableValueAddress(inst, id)->u;
    Com_Printf(16, "TEST SWITCH bo1_testclient_round: level.round_number %d -> %d\n",
        value.intValue, g_headlessRound->current.integer);
    value.intValue = g_headlessRound->current.integer;
}
