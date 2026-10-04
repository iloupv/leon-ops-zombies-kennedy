// w1 chunk 6 TEST SWITCHES (headless fighting test client only, off by default). Harness policy, not game logic:
// it writes usercmd fields, requests weapons the way the script SwitchToWeapon reaches a test client
// (botInfos), and runs the scripts' own give functions. Damage, fuses, lures and pickups stay the scripts' and
// the engine's.
//
// - bo1_testclient_scriptgive "file::func,...": at the fight's first target, run these script functions on the
//   player once (Scr_ExecEntThread, no arguments), as the retail purchase paths do:
//   maps/_zombiemode_claymore::claymore_setup (the claymore wall buy) and
//   maps/_zombiemode_weap_cymbal_monkey::player_give_cymbal_monkey (the box's monkey).
// - bo1_testclient_fightweapons entries that are offhand / item weapons (frag_grenade_zm, zombie_cymbal_monkey,
//   claymore_zm) are thrown or placed during their 20 s cycle while the previous gun is kept and not fired:
//   an offhand grenade is thrown with its slot's button (CL_CmdButtons: bit 14 lethal, bit 15 tactical) held
//   300 ms at a zombie 200-900 units away, one every 4 s; the claymore is raised, fired at a zombie inside 450
//   units, and the gun is raised again. Empty throw weapons are refilled (G_InitializeAmmo, as the gun refill).
//   "alt:<gun>" gives <gun> and fights with its alternate weapon (m16_gl_upgraded_zm -> gl_m16_upgraded_zm).
// - bo1_testclient_retrieve 1: an empty ballistic knife is not refilled; the player walks to the nearest
//   trigger_radius_use (maps\_ballistic_knife::on_spawn_retrieve_trigger's pickup trigger) and holds use.
// - bo1_testclient_shatter <gun>: during a freeze gun cycle, a script-spawned trigger_damage (the freezegun
//   shatter trigger, maps\_zombiemode_weap_freezegun) in sight is shot with <gun>.
// - bo1_testclient_rebuild 1: walk into the nearest hinted trigger_radius (a torn barrier's rebuild trigger) and
//   hold use; bo1_rebuild lines report contact and the player's cursor hint entity.
// - Measurement (always, with a lure out): bo1_lure lines once a second for each live cymbal monkey /
//   upgraded crossbow bolt: live zombies and their distances to it and to the player.
#include "g_sp_headless_throw.h"
#include <game_mp/g_main_mp.h>
#include <game_mp/g_utils_mp.h>
#include <game_mp/g_spawn_mp.h>
#include <client_mp/sv_client_mp.h>
#include <server_mp/sv_main_mp.h>
#include <server_mp/sv_bot_mp.h>
#include <win32/win_main.h>
#include <universal/dvar.h>
#include <universal/com_math.h>
#include <qcommon/common.h>
#include <clientscript/cscr_vm.h>
#include <clientscript/cscr_stringlist.h>
#include <clientscript/cscr_variable.h>
#include <clientscript/cscr_compiler.h>
#include <clientscript/scr_const.h>
#include <bgame/bg_weapons.h>
#include <bgame/bg_weapons_def.h>
#include <bgame/bg_misc.h>
#include <game_mp/g_client_script_cmd_mp.h>
#include <server/sv_world.h>
#include <server/sv_game.h>
#include <bgame/bg_animation.h>
#include <qcommon/cm_trace.h>
#include <game/actor.h>
#include <game_mp/actor_mp.h>
#include <game/g_weapon.h>
#include <cmath>
#include <cstring>
#include <cstdio>

extern bot_info_t botInfos[32]; // sv_bot_mp.cpp

static const dvar_s *s_retrieveDvar;
static const dvar_s *s_shatterDvar;
static const dvar_s *s_rebuildDvar;
static const dvar_s *s_watchLureDvar;
static int s_giveFunc[4];
static int s_giveCount;
static bool s_given;
static int s_scriptAtFunc; // L27 TEST SWITCH bo1_testclient_scriptat
static int s_scriptAtMs;
static bool s_scriptAtDone;
static unsigned int s_throwWeapon;
static int s_throwStart;
static int s_lastThrow;
static int s_prevAmmo = -1;
static int s_lureReport;
static int s_knifeEmptySince;
static int s_knifeReport;
static int s_knifeAmmo = -1;
static int s_shatterReport;
static int s_rebuildReport;

void G_SP_HeadlessThrowRegister()
{
    if (!Sys_IsHeadless())
        return;
    _Dvar_RegisterString("bo1_testclient_scriptgive", "", 0,
        "TEST SWITCH (with bo1_testclient_fight): run these 'file::function' script functions on the player at the first target (comma separated)");
    s_retrieveDvar = _Dvar_RegisterBool("bo1_testclient_retrieve", false, 0,
        "TEST SWITCH (with bo1_testclient_fight): walk to an empty ballistic knife's pickup trigger and hold use instead of refilling it");
    s_shatterDvar = _Dvar_RegisterString("bo1_testclient_shatter", "", 0,
        "TEST SWITCH (with bo1_testclient_fight): in a freeze gun cycle, shoot a frozen zombie's shatter trigger with this gun");
    s_rebuildDvar = _Dvar_RegisterBool("bo1_testclient_rebuild", false, 0,
        "TEST SWITCH (with bo1_testclient_fight): walk into the nearest hinted trigger_radius (a torn barrier's rebuild trigger) and hold use");
    s_watchLureDvar = _Dvar_RegisterInt("bo1_testclient_watchlure", 0, 0, 1000, 0,
        "TEST SWITCH (with bo1_testclient_fight, -Client evidence): while a landed cymbal monkey model exists, walk to this many units of it and look at it instead of fighting");
    _Dvar_RegisterString("bo1_testclient_scriptat", "", 0,
        "TEST SWITCH (headless evidence runs): 'ms:file::function' - run this retail level script function once as a level thread at this level time (no arguments)");
}

void G_SP_HeadlessThrowReset()
{
    s_given = false;
    s_scriptAtDone = false;
    s_throwWeapon = 0;
    s_throwStart = 0;
    s_lastThrow = 0;
    s_prevAmmo = -1;
    s_lureReport = 0;
    s_knifeEmptySince = 0;
    s_knifeReport = 0;
    s_knifeAmmo = -1;
    s_shatterReport = 0;
    s_rebuildReport = 0;
}

// w1 c7: script fields read (find-only) for the bo1_lure_poi line. Compiler identifiers are canonical ids; saved
// before Scr_EndLoadScripts releases the table.
static const char *const s_lureNames[] = { "poi_active", "attract_to_origin", "attractor_positions", "attractor_array",
    "enemyoverride", "favoriteenemy", "claimed_attractor_positions", "ignoreall" };
static unsigned int s_lureKeys[_countof(s_lureNames)];

void G_SP_HeadlessThrowFields(scriptInstance_t inst)
{
    if (inst != SCRIPTINSTANCE_SERVER)
        return;
    for (int i = 0; i < _countof(s_lureNames); ++i)
    {
        const unsigned int string = SL_FindString(s_lureNames[i], inst);
        s_lureKeys[i] = string ? gScrCompilePub[inst].canonicalStrings[string] : 0;
    }
    s_scriptAtFunc = 0;
    const dvar_s *at = Sys_IsHeadless() ? Dvar_FindVar("bo1_testclient_scriptat") : nullptr;
    if (at && at->current.string && *at->current.string)
    {
        char item[256];
        strncpy_s(item, at->current.string, _TRUNCATE);
        char *colon = strchr(item, ':');
        char *sep = colon ? strstr(colon + 1, "::") : nullptr;
        if (sep)
        {
            *colon = 0;
            *sep = 0;
            s_scriptAtMs = atoi(item);
            s_scriptAtFunc = Scr_GetFunctionHandle(inst, colon + 1, sep + 2);
            Com_Printf(16, "bo1_scriptat: %s::%s at %d handle %d\n", colon + 1, sep + 2, s_scriptAtMs, s_scriptAtFunc);
        }
    }
    s_giveCount = 0;
    const dvar_s *dvar = Sys_IsHeadless() ? Dvar_FindVar("bo1_testclient_scriptgive") : nullptr;
    if (!dvar || !dvar->current.string || !*dvar->current.string)
        return;
    char list[256];
    strncpy_s(list, dvar->current.string, _TRUNCATE);
    char *context = nullptr;
    for (char *item = strtok_s(list, ", ", &context); item && s_giveCount < 4; item = strtok_s(nullptr, ", ", &context))
    {
        char *sep = strstr(item, "::");
        if (!sep)
            continue;
        *sep = 0;
        for (char *p = item; *p; ++p)
            if (*p == '\\')
                *p = '/';
        const int handle = Scr_GetFunctionHandle(inst, item, sep + 2);
        Com_Printf(16, "bo1_throw: scriptgive %s::%s handle %d\n", item, sep + 2, handle);
        if (handle)
            s_giveFunc[s_giveCount++] = handle;
    }
}

void G_SP_HeadlessScriptAtFrame()
{
    if (s_scriptAtDone || !s_scriptAtFunc || level.time < s_scriptAtMs)
        return;
    s_scriptAtDone = true;
    Scr_FreeThread(Scr_ExecThread(SCRIPTINSTANCE_SERVER, s_scriptAtFunc, 0), SCRIPTINSTANCE_SERVER);
    Com_Printf(16, "bo1_scriptat: time %d ran\n", level.time);
}

void G_SP_HeadlessThrowStart(gentity_s *player)
{
    if (s_given || !s_giveCount)
        return;
    s_given = true;
    for (int i = 0; i < s_giveCount; ++i)
        Scr_FreeThread(Scr_ExecEntThread(player, s_giveFunc[i], 0), SCRIPTINSTANCE_SERVER);
    Com_Printf(16, "bo1_throw: time %d scriptgive ran %d functions\n", level.time, s_giveCount);
}

static bool IsThrown(unsigned int weapon)
{
    const WeaponDef *def = weapon ? BG_GetWeaponDef(weapon) : nullptr;
    return def && (def->inventoryType == WEAPINVENTORY_OFFHAND || def->inventoryType == WEAPINVENTORY_ITEM);
}

bool G_SP_HeadlessThrowCycle(const char *name, unsigned int *give, unsigned int *hold)
{
    s_throwWeapon = 0;
    s_throwStart = 0;
    s_lastThrow = 0;
    s_prevAmmo = -1;
    if (!strncmp(name, "alt:", 4))
    {
        *give = BG_FindWeaponIndexForName(name + 4);
        const unsigned int alt = *give ? BG_GetWeaponVariantDef(*give)->altWeaponIndex : 0;
        *hold = alt ? alt : *give;
        return false;
    }
    *give = *hold = BG_FindWeaponIndexForName(name);
    if (!IsThrown(*give))
        return false;
    s_throwWeapon = *give;
    return true;
}

static int TotalAmmo(const playerState_s *ps, unsigned int weapon)
{
    return BG_GetAmmoInClip(ps, weapon) + BG_GetAmmoNotInClip(ps, weapon);
}

static unsigned int HeldGun(const playerState_s *ps, unsigned int gun)
{
    if (gun && BG_PlayerHasWeapon(ps, gun) && !IsThrown(gun))
        return gun;
    for (int i = 0; i < 15; ++i)
    {
        const unsigned int held = ps->heldWeapons[i].weapon;
        if (held && BG_GetWeaponDef(held)->weapType == WEAPTYPE_BULLET
            && BG_GetWeaponDef(held)->inventoryType == WEAPINVENTORY_PRIMARY)
            return held;
    }
    return 0;
}

static void Request(client_t *client, usercmd_s *cmd, unsigned int weapon)
{
    if (!weapon)
        return;
    if (client->bIsTestClient)
        botInfos[client - svs.clients].weapon = weapon;
    cmd->weapon = weapon;
}

static void AimAt(gentity_s *player, usercmd_s *cmd, const float *point)
{
    playerState_s *ps = &player->client->ps;
    float eye[3], dir[3], angles[3];
    G_GetPlayerViewOrigin(ps, eye);
    Vec3Sub(point, eye, dir);
    vectoangles(dir, angles);
    for (int i = 0; i < 2; ++i)
        cmd->angles[i] = (unsigned short)(int)((angles[i] - ps->delta_angles[i]) * 182.04445f);
}

static bool IsLure(const gentity_s *ent)
{
    if (!ent->r.inuse || ent->classname != scr_const.grenade || !ent->s.weapon)
        return false;
    const char *name = BG_WeaponName(ent->s.weapon);
    return !strcmp(name, "zombie_cymbal_monkey") || !strcmp(name, "explosive_bolt_upgraded_zm");
}

// find-only reads of a script field (never creates a field); -1 / 0 when undefined
static unsigned int LureVarType(unsigned int object, int key)
{
    const unsigned int id = object && s_lureKeys[key] ? FindVariable(SCRIPTINSTANCE_SERVER, object, s_lureKeys[key]) : 0;
    return id ? GetValueType(SCRIPTINSTANCE_SERVER, id) : 0;
}

static int LureVarInt(unsigned int object, int key)
{
    const unsigned int id = object && s_lureKeys[key] ? FindVariable(SCRIPTINSTANCE_SERVER, object, s_lureKeys[key]) : 0;
    if (!id)
        return -1;
    const unsigned int type = GetValueType(SCRIPTINSTANCE_SERVER, id);
    const VariableUnion &value = GetVariableValueAddress(SCRIPTINSTANCE_SERVER, id)->u;
    return type == VAR_INTEGER ? value.intValue : type == VAR_FLOAT ? (int)value.floatValue : -2 - (int)type;
}

static int LureArraySize(unsigned int object, int key)
{
    const unsigned int id = object && s_lureKeys[key] ? FindVariable(SCRIPTINSTANCE_SERVER, object, s_lureKeys[key]) : 0;
    if (!id || GetValueType(SCRIPTINSTANCE_SERVER, id) != VAR_POINTER)
        return -1;
    return (int)GetArraySize(SCRIPTINSTANCE_SERVER, GetVariableValueAddress(SCRIPTINSTANCE_SERVER, id)->u.pointerValue);
}

static void ReportLures(gentity_s *player)
{
    if (level.time - s_lureReport < 1000)
        return;
    s_lureReport = level.time;
    for (int i = 0; i < level.num_entities; ++i)
    {
        const gentity_s *lure = &g_entities[i];
        if (!IsLure(lure))
            continue;
        int n = 0, in128 = 0, in256 = 0;
        float sum = 0.0f, nearest = 1.0e9f, playerSum = 0.0f;
        for (int a = 0; level.actors && a < MAX_ACTORS; ++a)
        {
            const actor_s *actor = &level.actors[a];
            if (!actor->inuse || !actor->ent || actor->ent->health <= 0 || !actor->sentient
                || actor->sentient->eTeam == player->sentient->eTeam)
                continue;
            const float d = Vec3Distance(actor->ent->r.currentOrigin, lure->r.currentOrigin);
            ++n;
            in128 += d < 128.0f;
            in256 += d < 256.0f;
            sum += d;
            playerSum += Vec3Distance(actor->ent->r.currentOrigin, player->r.currentOrigin);
            if (d < nearest)
                nearest = d;
        }
        Com_Printf(16, "bo1_lure: time %d ent %d weapon %s age %d pos %.0f %.0f %.0f actors %d in128 %d in256 %d mean %.0f min %.0f pmean %.0f player %.0f\n",
            level.time, i, BG_WeaponName(lure->s.weapon), level.time - lure->item[0].index, lure->r.currentOrigin[0],
            lure->r.currentOrigin[1], lure->r.currentOrigin[2], n, in128, in256, n ? sum / n : -1.0f, n ? nearest : -1.0f,
            n ? playerSum / n : -1.0f, Vec3Distance(player->r.currentOrigin, lure->r.currentOrigin));
        // w1 c7: the script poi state (_zombiemode_utility create_zombie_point_of_interest) and what the zombies do
        // with it (find_flesh -> self.enemyoverride, zombie_follow_enemy -> SetGoalPos(enemyoverride[0])).
        const unsigned int object = FindEntityId(SCRIPTINSTANCE_SERVER, lure->s.number, 0, 0);
        int override_ = 0, scriptGoal256 = 0, codeGoal256 = 0, ignoreAll = 0;
        for (int a = 0; level.actors && a < MAX_ACTORS; ++a)
        {
            const actor_s *actor = &level.actors[a];
            if (!actor->inuse || !actor->ent || actor->ent->health <= 0 || !actor->sentient
                || actor->sentient->eTeam == player->sentient->eTeam)
                continue;
            const unsigned int actorObject = FindEntityId(SCRIPTINSTANCE_SERVER, actor->ent->s.number, 0, 0);
            override_ += LureVarType(actorObject, 4) != 0;
            ignoreAll += LureVarInt(actorObject, 7) != 0;
            scriptGoal256 += Vec3Distance(actor->scriptGoal.pos, lure->r.currentOrigin) < 256.0f;
            codeGoal256 += Vec3Distance(actor->codeGoal.pos, lure->r.currentOrigin) < 256.0f;
            static float s_prevPos[MAX_GENTITIES_SV][3];
            const int num = actor->ent->s.number;
            if (Vec3Distance(actor->codeGoal.pos, lure->r.currentOrigin) < 256.0f)
                Com_Printf(16, "bo1_lure_actor: time %d ent %d dist %.0f goaldist %.0f goalr %.0f moved %.0f pathlen %d pathflags %x move %d state %d anim %d script %d\n",
                    level.time, num, Vec3Distance(actor->ent->r.currentOrigin, lure->r.currentOrigin),
                    Vec3Distance(actor->ent->r.currentOrigin, actor->codeGoal.pos), actor->codeGoal.radius,
                    Vec3Distance(actor->ent->r.currentOrigin, s_prevPos[num]), actor->Path.wPathLen, actor->Path.flags,
                    actor->moveMode, actor->eState[actor->stateLevel], actor->eAnimMode, actor->scriptState);
            s_prevPos[num][0] = actor->ent->r.currentOrigin[0];
            s_prevPos[num][1] = actor->ent->r.currentOrigin[1];
            s_prevPos[num][2] = actor->ent->r.currentOrigin[2];
        }
        const unsigned short noteworthy = *(const unsigned short *)((const char *)lure + 362); // fields_1 script_noteworthy
        Com_Printf(16, "bo1_lure_poi: time %d ent %d noteworthy '%s' poi_active %d attract_to_origin %d positions %d attractors %d claimed %d zombies_override %d ignoreall %d scriptgoal256 %d codegoal256 %d\n",
            level.time, i, noteworthy ? SL_ConvertToString(noteworthy, SCRIPTINSTANCE_SERVER) : "", LureVarInt(object, 0),
            LureVarInt(object, 1), LureArraySize(object, 2), LureArraySize(object, 3), LureArraySize(object, 6), override_,
            ignoreAll, scriptGoal256, codeGoal256);
    }
}

static bool IsBallisticKnife(unsigned int weapon)
{
    return weapon && !strncmp(BG_WeaponName(weapon), "knife_ballistic", 15);
}

static gentity_s *NearestKnifeTrigger(gentity_s *player, float *distance)
{
    gentity_s *best = nullptr;
    *distance = 1500.0f;
    for (int i = 0; i < level.num_entities; ++i)
    {
        gentity_s *ent = &g_entities[i];
        if (!ent->r.inuse || ent->classname != scr_const.trigger_radius_use)
            continue;
        const float d = Vec3Distance(ent->r.currentOrigin, player->r.currentOrigin);
        if (d < *distance)
        {
            *distance = d;
            best = ent;
        }
    }
    return best;
}

bool G_SP_HeadlessThrowKeepEmpty(gentity_s *player, unsigned int weapon)
{
    if (!s_retrieveDvar || !s_retrieveDvar->current.enabled || !IsBallisticKnife(weapon))
        return false;
    float distance;
    return NearestKnifeTrigger(player, &distance) || !s_knifeEmptySince || level.time - s_knifeEmptySince < 5000;
}

static bool KnifeRetrieve(gentity_s *player, usercmd_s *cmd, unsigned int gun)
{
    playerState_s *ps = &player->client->ps;
    // w1 c7: the empty knife is switched away from (PM: no ammo), so follow the cycle's knife, not ps->weapon.
    const unsigned int knife = IsBallisticKnife(ps->weapon) ? ps->weapon
        : IsBallisticKnife(gun) && BG_PlayerHasWeapon(ps, gun) ? gun : 0;
    if (!s_retrieveDvar || !s_retrieveDvar->current.enabled || !knife)
        return false;
    const int ammo = TotalAmmo(ps, knife);
    if (s_knifeAmmo >= 0 && ammo != s_knifeAmmo)
        Com_Printf(16, "bo1_knife: time %d weapon %s ammo %d -> %d clip %d held %s\n", level.time, BG_WeaponName(knife),
            s_knifeAmmo, ammo, BG_GetAmmoInClip(ps, knife), BG_WeaponName(ps->weapon));
    s_knifeAmmo = ammo;
    if (ammo)
    {
        s_knifeEmptySince = 0;
        return false;
    }
    if (!s_knifeEmptySince)
        s_knifeEmptySince = level.time;
    float distance;
    gentity_s *trigger = NearestKnifeTrigger(player, &distance);
    if (level.time - s_knifeReport >= 500)
    {
        s_knifeReport = level.time;
        Com_Printf(16, "bo1_knife: time %d retrieve trigger %d dist %.0f pos %.0f %.0f %.0f hint %d/%d\n", level.time,
            trigger ? trigger->s.number : -1, trigger ? distance : -1.0f, trigger ? trigger->r.currentOrigin[0] : 0.0f,
            trigger ? trigger->r.currentOrigin[1] : 0.0f, trigger ? trigger->r.currentOrigin[2] : 0.0f, ps->cursorHint,
            ps->cursorHintEntIndex);
    }
    if (!trigger)
        return false;
    const float dx = trigger->r.currentOrigin[0] - player->r.currentOrigin[0];
    AimAt(player, cmd, trigger->r.currentOrigin); // the use candidate is scored by the view direction; walk after
    const float dy = trigger->r.currentOrigin[1] - player->r.currentOrigin[1];
    if (dx * dx + dy * dy > 40.0f * 40.0f)
        G_SP_HeadlessWalkTo(player, trigger->r.currentOrigin, 4000 + trigger->s.number, cmd);
    // Hold use 300 ms of every 500: the trigger's use is an edge, pick_up reads UseButtonPressed.
    if (level.time % 500 < 300)
        cmd->button_bits.setBit(3);
    return true;
}

static bool Shatter(gentity_s *player, client_t *client, usercmd_s *cmd, unsigned int gun)
{
    playerState_s *ps = &player->client->ps;
    if (!s_shatterDvar || !*s_shatterDvar->current.string || !gun || strncmp(BG_WeaponName(gun), "freezegun", 9))
        return false;
    const unsigned int shatterGun = BG_FindWeaponIndexForName(s_shatterDvar->current.string);
    if (!shatterGun)
        return false;
    if (!BG_PlayerHasWeapon(ps, shatterGun))
    {
        renderOptions_s options;
        options.i = 0;
        if (G_GivePlayerWeapon(ps, shatterGun, 0, options))
            G_InitializeAmmo(player, shatterGun, 0, 0);
    }
    float eye[3];
    G_GetPlayerViewOrigin(ps, eye);
    gentity_s *best = nullptr;
    float bestDistance = 1000.0f, point[3] = {};
    for (int i = 0; i < level.num_entities; ++i)
    {
        gentity_s *ent = &g_entities[i];
        if (!ent->r.inuse || ent->classname != scr_const.trigger_damage || ent->r.bmodel)
            continue;
        float center[3] = { ent->r.currentOrigin[0], ent->r.currentOrigin[1], ent->r.currentOrigin[2] + 30.0f };
        const float d = Vec3Distance(eye, center);
        if (d >= bestDistance)
            continue;
        trace_t trace;
        col_context_t context;
        G_TraceCapsule(&trace, eye, vec3_origin, vec3_origin, center, player->s.number, 1, &context); // world only
        if (trace.fraction < 1.0f)
            continue;
        best = ent;
        bestDistance = d;
        Vec3Copy(center, point);
    }
    if (!best)
    {
        if (ps->weapon == shatterGun)
            Request(client, cmd, gun);
        return false;
    }
    if (level.time - s_shatterReport >= 500)
    {
        s_shatterReport = level.time;
        Com_Printf(16, "bo1_shatter: time %d trigger %d dist %.0f weapon %s state %d\n", level.time, best->s.number,
            bestDistance, BG_WeaponName(ps->weapon), ps->weaponstate);
    }
    Request(client, cmd, shatterGun);
    AimAt(player, cmd, point);
    if (ps->weapon == shatterGun && ps->weaponstate == WEAPON_READY && !ps->weaponTime && BG_GetAmmoInClip(ps, shatterGun)
        && !client->lastUsercmd.button_bits.testBit(0))
        cmd->button_bits.setBit(0);
    if (ps->weapon == shatterGun && !TotalAmmo(ps, shatterGun))
        G_InitializeAmmo(player, shatterGun, 0, 1);
    return true;
}

// w1 c8: a torn barrier's rebuild trigger (maps\_zombiemode_blockers::blocker_trigger_think: trigger_radius,
// set_hint_string + HINT_NOICON; the repair itself runs on touch + UseButtonPressed). Walk into it and hold use;
// bo1_rebuild reports whether the engine made it the cursor hint entity (Player_GetUseList).
static bool Rebuild(gentity_s *player, usercmd_s *cmd)
{
    if (!s_rebuildDvar || !s_rebuildDvar->current.enabled)
        return false;
    playerState_s *ps = &player->client->ps;
    gentity_s *trigger = nullptr;
    float distance = 1500.0f;
    for (int i = 0; i < level.num_entities; ++i)
    {
        gentity_s *ent = &g_entities[i];
        if (!ent->r.inuse || ent->classname != scr_const.trigger_radius || ent->s.un3.item <= 0)
            continue;
        const float d = Vec3Distance(ent->r.currentOrigin, player->r.currentOrigin);
        if (d < distance)
        {
            distance = d;
            trigger = ent;
        }
    }
    if (!trigger)
        return false;
    const float touchMins[3] = { ps->origin[0] - 15.0f, ps->origin[1] - 15.0f, ps->origin[2] };
    const float touchMaxs[3] = { ps->origin[0] + 15.0f, ps->origin[1] + 15.0f, ps->origin[2] + 70.0f };
    const bool contact = SV_EntityContact(touchMins, touchMaxs, trigger);
    if (level.time - s_rebuildReport >= 500)
    {
        s_rebuildReport = level.time;
        Com_Printf(16, "bo1_rebuild: time %d trigger %d dist %.0f contact %d hint %d/%d isHint %d team %d/%d item %d score %d\n",
            level.time, trigger->s.number, distance, contact ? 1 : 0, ps->cursorHint, ps->cursorHintEntIndex,
            ps->cursorHint && ps->cursorHintEntIndex == trigger->s.number ? 1 : 0, trigger->team,
            level_bgs.clientinfo[ps->clientNum].team, trigger->s.un3.item, player->client->sess.cs.score.score);
    }
    AimAt(player, cmd, trigger->r.currentOrigin);
    if (!contact)
        G_SP_HeadlessWalkTo(player, trigger->r.currentOrigin, 5000 + trigger->s.number, cmd);
    cmd->button_bits.setBit(3); // blocker_trigger_think repairs while UseButtonPressed
    return true;
}

// L12 TEST SWITCH bo1_testclient_watchlure <units>: the landed monkey is the unlinked script model
// weapon_zombie_monkey_bomb with an anim tree (maps\_zombiemode_weap_cymbal_monkey::player_handle_cymbal_monkey).
static bool WatchLure(gentity_s *player, usercmd_s *cmd)
{
    if (!s_watchLureDvar || !s_watchLureDvar->current.integer)
        return false;
    for (int i = 0; i < level.num_entities; ++i)
    {
        gentity_s *ent = &g_entities[i];
        if (!ent->r.inuse || !ent->pAnimTree || ent->tagInfo || ent->actor || ent->client || !ent->model
            || strcmp(SL_ConvertToString(G_ModelName(ent->model), SCRIPTINSTANCE_SERVER), "weapon_zombie_monkey_bomb"))
            continue;
        const float point[3] = { ent->r.currentOrigin[0], ent->r.currentOrigin[1], ent->r.currentOrigin[2] + 6.0f };
        if (Vec3Distance(player->r.currentOrigin, ent->r.currentOrigin) > (float)s_watchLureDvar->current.integer)
            G_SP_HeadlessWalkTo(player, ent->r.currentOrigin, 6000 + i, cmd);
        AimAt(player, cmd, point);
        return true;
    }
    return false;
}

bool G_SP_HeadlessThrowPreCommand(gentity_s *player, client_t *client, usercmd_s *cmd, unsigned int gun)
{
    ReportLures(player);
    if (WatchLure(player, cmd))
        return true;
    if (Rebuild(player, cmd))
        return true;
    if (KnifeRetrieve(player, cmd, gun))
        return true;
    return Shatter(player, client, cmd, gun);
}

bool G_SP_HeadlessThrowCommand(gentity_s *player, client_t *client, usercmd_s *cmd, gentity_s *target, float distance,
    unsigned int gun)
{
    if (!s_throwWeapon)
        return false;
    playerState_s *ps = &player->client->ps;
    const WeaponDef *def = BG_GetWeaponDef(s_throwWeapon);
    cmd->button_bits.resetBit(0);
    cmd->button_bits.resetBit(11);
    if (!BG_PlayerHasWeapon(ps, s_throwWeapon))
        return true; // hold fire: the cycle measures the throw weapon only
    const int ammo = TotalAmmo(ps, s_throwWeapon);
    if (s_prevAmmo >= 0 && ammo < s_prevAmmo)
    {
        s_lastThrow = level.time;
        Com_Printf(16, "bo1_throw: time %d used %s ammo %d -> %d target %d dist %.0f\n", level.time,
            BG_WeaponName(s_throwWeapon), s_prevAmmo, ammo, target ? target->s.number : -1, distance);
    }
    s_prevAmmo = ammo;
    const unsigned int held = HeldGun(ps, gun);
    if (!ammo && level.time - s_lastThrow > 3000 && ps->weapon != s_throwWeapon && ps->weaponstate == WEAPON_READY)
    {
        G_InitializeAmmo(player, s_throwWeapon, 0, 1);
        s_prevAmmo = TotalAmmo(ps, s_throwWeapon);
        Com_Printf(16, "bo1_throw: time %d refill %s ammo %d\n", level.time, BG_WeaponName(s_throwWeapon), s_prevAmmo);
    }
    if (def->inventoryType == WEAPINVENTORY_OFFHAND)
    {
        const bool tactical = def->offhandSlot == OFFHAND_SLOT_TACTICAL_GRENADE;
        const int bit = tactical ? 15 : 14;
        // A lure (cymbal monkey) is thrown 25 degrees up along the most open of 16 directions (longest clear eye-level
        // trace, up to 1000 units), one per 10 s (its fuse is 8 s), so the zombies' walk from the player to it can
        // be measured (bo1_lure). The direction is kept from the press until the throw leaves the hand.
        static float s_throwYaw;
        if (tactical)
        {
            if (!s_throwStart || level.time - s_throwStart >= 4000)
            {
                float eye[3];
                G_GetPlayerViewOrigin(ps, eye);
                float best = -1.0f;
                for (int i = 0; i < 16; ++i)
                {
                    const float yaw = i * 22.5f, rad = yaw * (3.14159265f / 180.0f);
                    const float end[3] = { eye[0] + 1000.0f * cosf(rad), eye[1] + 1000.0f * sinf(rad), eye[2] };
                    trace_t trace;
                    col_context_t context;
                    G_TraceCapsule(&trace, eye, vec3_origin, vec3_origin, end, player->s.number, 1, &context);
                    if (trace.fraction > best)
                    {
                        best = trace.fraction;
                        s_throwYaw = yaw;
                    }
                }
            }
            cmd->angles[1] = (unsigned short)(int)((s_throwYaw - ps->delta_angles[1]) * 182.04445f);
            cmd->angles[0] = (unsigned short)(int)((-25.0f - ps->delta_angles[0]) * 182.04445f);
        }
        if (s_throwStart && level.time - s_throwStart < 300)
        {
            cmd->button_bits.setBit(bit);
            return true;
        }
        if (level.time % 2000 == 0)
            Com_Printf(16, "bo1_throw: time %d wait %s ammo %d dist %.0f state %d last %d\n", level.time,
                BG_WeaponName(s_throwWeapon), ammo, distance, ps->weaponstate, s_lastThrow);
        if (ammo > 0 && (tactical || (distance >= 200.0f && distance <= 900.0f))
            && level.time - s_lastThrow >= (tactical ? 10000 : 4000)
            && ps->weaponstate == WEAPON_READY && !client->lastUsercmd.button_bits.testBit(bit))
        {
            s_throwStart = s_lastThrow = level.time;
            cmd->button_bits.setBit(bit);
            Com_Printf(16, "bo1_throw: time %d press %s bit %d target %d dist %.0f ammo %d\n", level.time,
                BG_WeaponName(s_throwWeapon), bit, target->s.number, distance, ammo);
        }
        return true;
    }
    // Item (claymore): raise it and fire at a close zombie; after the ammo drops, raise the gun again.
    if (ammo > 0 && distance < 450.0f && (!s_lastThrow || level.time - s_lastThrow >= 6000))
    {
        Request(client, cmd, s_throwWeapon);
        if (ps->weapon == s_throwWeapon && ps->weaponstate == WEAPON_READY && !client->lastUsercmd.button_bits.testBit(0))
        {
            cmd->button_bits.setBit(0);
            Com_Printf(16, "bo1_throw: time %d press %s fire target %d dist %.0f ammo %d\n", level.time,
                BG_WeaponName(s_throwWeapon), target->s.number, distance, ammo);
        }
        return true;
    }
    if (ps->weapon == s_throwWeapon)
        Request(client, cmd, held);
    return true;
}
