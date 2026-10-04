#include "g_sp_client.h"
#include <Windows.h>
#include "g_sp_levelstart.h"
#include "g_sp_ext.h"
#include <client_mp/g_client_mp.h>
#include <game_mp/g_main_mp.h>
#include <game_mp/g_scr_main_mp.h>
#include <cstring>
#include <client_mp/sv_client_mp.h>
#include <server_mp/sv_main_mp.h>
#include <win32/win_main.h>
#include <universal/dvar.h>
#include <qcommon/common.h>
#include <game/actor.h>
#include <game_mp/actor_mp.h>
#include <cmath>
#include <bgame/bg_weapons.h>
#include <bgame/bg_weapons_def.h>
#include <bgame/bg_misc.h>
#include <bgame/bg_animation.h>
#include <game_mp/g_utils_mp.h>
#include <game_mp/g_combat_mp.h>
#include <server/sv_world.h>
#include <qcommon/cm_trace.h>
#include <clientscript/scr_const.h>
#include <universal/com_math.h>
#include <game/actor_navigation.h>
#include <clientscript/cscr_stringlist.h>
#include "g_sp_player_state.h"
#include "g_sp_testplan.h"
#include "g_sp_headless_move.h"
#include "g_sp_headless_move.h"
#include <server_mp/sv_bot_mp.h>
#include <game/g_weapon.h>
#include <game_mp/g_client_script_cmd_mp.h>
#include "g_sp_headless_labs.h"
#include "g_sp_headless_throw.h"
#include "g_sp_playtrace.h"
#include <bgame/bg_animation.h>

extern bot_info_t botInfos[32]; // sv_bot_mp.cpp

static bool g_clientConnectPending[32];
static const dvar_s *g_headlessTestClient;
static const dvar_s *g_headlessFight;
static const dvar_s *g_headlessFightPassive;
static const dvar_s *g_headlessFightHoldFire;
static const dvar_s *g_headlessGod;
static const dvar_s *g_headlessRealWindow; // L50: bo1_testclient_realwindow (real window only, tools\realtest.ps1 -Fight)

// L50 TEST SWITCH: bo1_testclient_realwindow 1 in a real (not headless) window makes the test harness active there
// too: the bo1_testclient switches register, and the local client is the harness player exactly as in a headless
// -Client run (G_SP_HeadlessLocalClientCommand). Off by default: a normal game never registers or runs the harness.
bool G_SP_TestClientRealWindow()
{
    return !Sys_IsHeadless() && g_headlessRealWindow && g_headlessRealWindow->current.enabled;
}

// the harness runs: headless, or a real window with bo1_testclient_realwindow 1
static bool G_SP_TestHarness()
{
    return Sys_IsHeadless() || G_SP_TestClientRealWindow();
}

// the local client is the harness player: a headless -Client run, or a real window with bo1_testclient_realwindow 1
bool G_SP_TestLocalClient()
{
    return Sys_IsHeadlessClient() || G_SP_TestClientRealWindow();
}
static const dvar_s *g_headlessHurt;
static int s_headlessHurtAt = -1;
static const dvar_s *g_headlessHurtStep;
static int s_headlessHurtStepNext = -1;
static int s_headlessHurtStepDone;
static const dvar_s *g_headlessFightAimTag;
// j1 TEST SWITCHES (headless test client only, off by default): stance, a slow back-and-forth walk, a place
// to stand and a point to look at while passive. Evidence for "zombies do not attack me when I ...".
static const dvar_s *g_headlessStance;
static const dvar_s *g_headlessSlowWalk;
static const dvar_s *g_headlessGoto;
static const dvar_s *g_headlessLook;
static const dvar_s *g_headlessUse;
static const dvar_s *g_headlessPress; // a1 c11: window glass by bullet / knife / grenade
static const dvar_s *g_headlessFightWeapons;
static const dvar_s *g_headlessSelfBlast;
static const dvar_s *g_headlessSpareCrawlers;
static const dvar_s *g_headlessKnife;
static int g_fightReportTime;
static int g_fightReportHealth;
static bool g_testClientAttempted;
static int g_testClientNum = -1;
static int g_testClientFrames;
static int g_testClientActiveFrames;
static bool g_addingHeadlessTestClient;
// x6: headless -Client harness - the fight command computed on the server thread for the LOCAL client, applied by the
// client's own CL_CreateCmd (G_SP_HeadlessLocalClientCommand), so the rendered view is the harness player's.
static SRWLOCK g_localFightLock = SRWLOCK_INIT;
static usercmd_s g_localFightCmd;
static bool g_localFightValid;
static bool g_localFightWeaponOverride;
static int g_actorObservedEntity[MAX_ACTORS_CAP];
static int g_actorObservedUseCount[MAX_ACTORS_CAP];
static float g_actorObservedOrigin[MAX_ACTORS_CAP][3];
static int g_actorDeadSince[MAX_ACTORS_CAP]; // L9: level time the summary first saw this actor dead (0 = alive)
// L9 TEST SWITCH bo1_testclient_watchkill: after each of the first N deaths, hold fire and look at the death spot
// for 8 s of level time, with back-buffer screenshots (-Client) every 350 ms (20 shots). Evidence of the death pose,
// the ragdoll and the corpse that replaces the actor.
static const dvar_s *g_headlessWatchKill;
static const dvar_s *g_headlessFireRange;
static int g_watchKillCount;
static int g_watchKillStart;
static int g_watchKillEnt = -1;
static int g_watchKillShot;
static float g_watchKillPoint[3];
static int g_actorSpawnCount;
static int g_actorReportTime;
static path_t g_fightPath;
static int g_fightPathGoal = -1;
static int g_fightPathTime;
static int g_fightPathPoint;
static int g_fightAdsStart;
static int g_fightWeaponsStart;
static unsigned int g_fightWeaponsCurrent;
static unsigned int g_fightWeaponsAltParent; // w1 c8: the primary of an "alt:" cycle's alternate weapon
static int g_fightRetreatBlocked;
static int g_fightStrafeUntil;
static bool g_fightStrafeRight;

void G_SP_ResetClientConnects()
{
    memset(g_clientConnectPending, 0, sizeof(g_clientConnectPending));
    g_testClientAttempted = false;
    g_testClientNum = -1;
    g_testClientFrames = 0;
    g_testClientActiveFrames = 0;
    g_addingHeadlessTestClient = false;
    AcquireSRWLockExclusive(&g_localFightLock);
    g_localFightValid = false;
    ReleaseSRWLockExclusive(&g_localFightLock);
    memset(g_actorObservedEntity, -1, sizeof(g_actorObservedEntity));
    memset(g_actorObservedUseCount, 0, sizeof(g_actorObservedUseCount));
    memset(g_actorObservedOrigin, 0, sizeof(g_actorObservedOrigin));
    memset(g_actorDeadSince, 0, sizeof(g_actorDeadSince));
    g_watchKillCount = 0;
    g_watchKillStart = 0;
    g_watchKillEnt = -1;
    g_watchKillShot = 0;
    g_actorSpawnCount = 0;
    g_actorReportTime = 0;
    g_fightReportTime = 0;
    g_fightReportHealth = -1;
    g_fightPathGoal = -1;
    g_fightPathTime = 0;
    g_fightPathPoint = -1;
    g_fightAdsStart = 0;
    g_fightWeaponsStart = 0;
    g_fightWeaponsCurrent = 0;
    g_fightWeaponsAltParent = 0;
    g_fightRetreatBlocked = 0;
    g_fightStrafeUntil = 0;
    g_fightStrafeRight = false;
    if (!Sys_IsHeadless())
        g_headlessRealWindow = _Dvar_RegisterBool("bo1_testclient_realwindow", false, 0,
            "TEST SWITCH (real window: tools\\realtest.ps1 -Fight): the bo1_testclient harness drives the local player as in a headless -Client run");
    if (G_SP_TestHarness())
    {
        g_headlessTestClient = _Dvar_RegisterBool("bo1_testclient", false, 0,
            "Add one local test client after server initialization (headless only)");
        g_headlessFight = _Dvar_RegisterBool("bo1_testclient_fight", false, 0,
            "Aim and fire through the headless zombie test client's usercmds");
        g_headlessFightPassive = _Dvar_RegisterInt("bo1_testclient_passive", 0, 0, 0x7FFFFFFF, 0,
            "Harness evidence: from this level time (ms) on the fight command stops aiming, firing and buying (0 = never)");
        g_headlessFightHoldFire = _Dvar_RegisterInt("bo1_testclient_holdfire", 0, 0, 0x7FFFFFFF, 0,
            "Harness evidence: from this level time (ms) on the fight command only looks at the nearest zombie (0 = never)");
        g_headlessGod = _Dvar_RegisterBool("bo1_testclient_god", false, 0,
            "TEST SWITCH for long-game soaks: the headless test client keeps the retail god-mode flag (Cmd_God_f's bit)");
        g_headlessHurt = _Dvar_RegisterInt("bo1_testclient_hurt", 0, 0, 0x7FFFFFFF, 0,
            "TEST SWITCH (headless test client only): at this level time (ms), one G_Damage of health + 1000 as zombie_damage_failsafe's DoDamage (0 = never)");
        g_headlessHurtStep = _Dvar_RegisterString("bo1_testclient_hurtstep", "", 0,
            "TEST SWITCH (headless test client only): \"<start ms> <damage> <interval ms> <count>\" - count G_Damage calls of damage each, from level time start (empty = never)");
        g_headlessFightAimTag = _Dvar_RegisterString("bo1_testclient_aimtag", "", 0,
            "Harness evidence: the zombie bone the fight command aims at (empty = j_head), e.g. j_knee_le for leg gibs");
        g_headlessStance = _Dvar_RegisterInt("bo1_testclient_stance", 0, 0, 2, 0,
            "TEST SWITCH: the fight command holds this stance (0 stand, 1 crouch, 2 prone; usercmd bits 9 / 8)");
        g_headlessSlowWalk = _Dvar_RegisterInt("bo1_testclient_slowwalk", 0, 0, 127, 0,
            "TEST SWITCH: while passive, walk forward/back with this forwardmove (0 = stand), reversing every 1.5 s");
        g_headlessGoto = _Dvar_RegisterString("bo1_testclient_goto", "", 0,
            "TEST SWITCH: while passive, first walk to this 'x,y,z' (pathnode planner) and stay there");
        g_headlessLook = _Dvar_RegisterString("bo1_testclient_look", "", 0,
            "TEST SWITCH: while passive, look at this 'x,y,z' (screenshots of one window)");
        g_headlessPress = _Dvar_RegisterString("bo1_testclient_press", "", 0,
            "TEST SWITCH: while passive and at the goto point, 'bit,from,period,hold': hold usercmd button bit <bit> (CL_CmdButtons: 0 attack, 2 melee, 14 lethal grenade) for <hold> ms every <period> ms from level time <from>");
        g_headlessFightWeapons = _Dvar_RegisterString("bo1_testclient_fightweapons", "", 0,
            "TEST SWITCH (with bo1_testclient_fight): give and fight with these weapons in turn (comma separated, 20 s of level time each from the first fight command); ammo is refilled when the gun is empty");
        g_headlessSpareCrawlers = _Dvar_RegisterBool("bo1_testclient_sparecrawlers", false, 0,
            "TEST SWITCH (with bo1_testclient_fight): never target a crawler (script has_legs false), so one can be the round's last zombie");
        g_headlessSelfBlast = _Dvar_RegisterBool("bo1_testclient_selfblast", false, 0,
            "TEST SWITCH (with bo1_testclient_fight): fire an explosive gun at a target inside its own blast radius (splash damage to the player)");
        g_headlessKnife = _Dvar_RegisterString("bo1_testclient_knife", "", 0,
            "TEST SWITCH (with bo1_testclient_fight): knife every target in melee reach whatever the ammo; a melee weapon name (bowie_knife_zm) is given first as the bowie script does (give, take knife_zm)");
        g_headlessWatchKill = _Dvar_RegisterInt("bo1_testclient_watchkill", 0, 0, 100, 0,
            "TEST SWITCH (with bo1_testclient_fight): after each of the first N actor deaths hold fire and look at the death spot for 8 s (screenshots bo1_watch<n>_<k> on -Client runs)");
        g_headlessFireRange = _Dvar_RegisterFloat("bo1_testclient_firerange", 0.0f, 0.0f, 10000.0f, 0,
            "TEST SWITCH (with bo1_testclient_fight): hold fire until the target is within this distance (0 = any)");
        g_headlessUse = _Dvar_RegisterBool("bo1_testclient_use", false, 0,
            "TEST SWITCH: while passive and at the goto spot, hold the use button (usercmd bit 3; window board repair)");
    }
    else
    {
        // L43: the test switches register only when headless, so a normal game ignores them. This one also works in a
        // real window, for tools\realtest.ps1 (capacity runs in a real window must not end in bleed-out and a restart):
        // every playing client keeps the retail god-mode flag (G_SP_HeadlessTestClientFrame). Off by default.
        g_headlessGod = _Dvar_RegisterBool("bo1_testclient_god", false, 0,
            "TEST SWITCH (real window: tools\\realtest.ps1): every playing client keeps the retail god-mode flag (Cmd_God_f's bit)");
    }
    G_SP_HeadlessLabsInit(); // f2 test switch bo1_testclient_labs (headless only)
    G_SP_HeadlessThrowReset(); // w1 chunk 6 test switches (headless only)
    G_SP_HeadlessThrowRegister();
    G_SP_TestPlanRegister();
    G_SP_TestPlanReset();
}

// Harness purchase goal: the solo Quick Revive machine, the one purchase on the spawn floor that the
// scripts sell for the 500 a round-one player has (maps\_zombiemode_perks::vending_trigger_think:
// solo cost 500, no power wait). Found by its map key target "vending_revive". The scripts decide
// whether a press buys anything; the harness only walks there and presses use.
static gentity_s *SP_HeadlessPurchaseGoal(gentity_s *player)
{
    if (G_SP_HeadlessLabsActive()) // f2 test switch: labs floor, the power switch is the only goal
        return G_SP_HeadlessLabsGoal(player);
    if (player->client->sess.cs.score.score < 500 || G_SP_HasPerk(player->client, "specialty_quickrevive"))
        return nullptr;
    for ( int i = 0; i < G_EntEnd(); i = G_EntNext(i) )
    {
        gentity_s *ent = &g_entities[i];
        if (ent->r.inuse && ent->classname == scr_const.trigger_use && ent->target
            && !strcmp(SL_ConvertToString(ent->target, SCRIPTINSTANCE_SERVER), "vending_revive"))
            return ent;
    }
    return nullptr;
}

// Walk toward goal along the pathnode graph (Path_FindPath, the actors' own planner), re-planned
// every two seconds. Produces forward/right moves relative to the command's view yaw.
static bool SP_HeadlessWalk(gentity_s *player, const float *goal, int goalNum, usercmd_s *cmd, float viewYaw)
{
    const float *origin = player->r.currentOrigin;
    if (g_fightPathGoal != goalNum || level.time - g_fightPathTime >= 2000)
    {
        g_fightPathGoal = goalNum;
        g_fightPathTime = level.time;
        g_fightPath.wPathLen = 0;
        if (!Path_FindPath(&g_fightPath, player->sentient->eTeam, origin, goal, 0))
            g_fightPath.wPathLen = 0;
        g_fightPathPoint = g_fightPath.wPathLen - 1;
    }
    float point[3];
    // Path points run from the goal (index 0) back to the start.
    while (g_fightPathPoint >= 0)
    {
        const float *p = g_fightPath.pts[g_fightPathPoint].vOrigPoint;
        const float dx = p[0] - origin[0], dy = p[1] - origin[1];
        if (dx * dx + dy * dy > 24.0f * 24.0f)
            break;
        --g_fightPathPoint;
    }
    if (g_fightPathPoint >= 0)
        Vec3Copy(g_fightPath.pts[g_fightPathPoint].vOrigPoint, point);
    else
        Vec3Copy(goal, point);
    float dir[2] = { point[0] - origin[0], point[1] - origin[1] };
    const float length = sqrtf(dir[0] * dir[0] + dir[1] * dir[1]);
    if (length < 1.0f)
        return false;
    dir[0] /= length;
    dir[1] /= length;
    const float yaw = viewYaw * (3.14159265f / 180.0f);
    const float forward = dir[0] * cosf(yaw) + dir[1] * sinf(yaw);
    const float right = dir[0] * sinf(yaw) - dir[1] * cosf(yaw);
    cmd->forwardmove = (char)(int)(127.0f * forward);
    cmd->rightmove = (char)(int)(127.0f * right);
    return true;
}

bool G_SP_HeadlessWalkTo(gentity_s *player, const float *goal, int goalNum, usercmd_s *cmd)
{
    const float viewYaw = (float)(unsigned short)cmd->angles[1] / 182.04445f + player->client->ps.delta_angles[1];
    return SP_HeadlessWalk(player, goal, goalNum, cmd, viewYaw);
}

static bool SP_ParseVec3(const dvar_s *dvar, float *out)
{
    return dvar && dvar->current.string && sscanf_s(dvar->current.string, "%f%*[ ,]%f%*[ ,]%f", &out[0], &out[1], &out[2]) == 3;
}

// j1 test switches while passive: goto, then look and slow walk. Only usercmd fields change.
static void SP_HeadlessPassiveMove(gentity_s *player, playerState_s *ps, usercmd_s *cmd)
{
    float point[3];
    float viewYaw = (float)(unsigned short)cmd->angles[1] / 182.04445f + ps->delta_angles[1];
    if (SP_ParseVec3(g_headlessGoto, point))
    {
        const float dx = point[0] - player->r.currentOrigin[0], dy = point[1] - player->r.currentOrigin[1];
        if (dx * dx + dy * dy > 12.0f * 12.0f && SP_HeadlessWalk(player, point, -2, cmd, viewYaw))
            return;
    }
    if (SP_ParseVec3(g_headlessLook, point))
    {
        float eye[3], dir[3], angles[3];
        G_GetPlayerViewOrigin(ps, eye);
        Vec3Sub(point, eye, dir);
        vectoangles(dir, angles);
        for (int i = 0; i < 3; ++i)
            cmd->angles[i] = (unsigned short)(int)((angles[i] - ps->delta_angles[i]) * 182.04445f);
    }
    if (g_headlessUse && g_headlessUse->current.enabled)
        cmd->button_bits.setBit(3); // CL_CmdButtons: +activate (kb 22) -> bit 3
    int press[4];
    if (g_headlessPress && g_headlessPress->current.string
        && sscanf_s(g_headlessPress->current.string, "%d,%d,%d,%d", &press[0], &press[1], &press[2], &press[3]) == 4
        && press[0] >= 0 && press[0] < 64 && press[2] > 0 && level.time >= press[1]
        && (level.time - press[1]) % press[2] < press[3])
    {
        cmd->button_bits.setBit(press[0]);
    }
    if (g_headlessSlowWalk && g_headlessSlowWalk->current.integer > 0)
    {
        const int speed = g_headlessSlowWalk->current.integer;
        cmd->forwardmove = (char)((level.time / 1500) % 2 ? -speed : speed);
    }
}

bool G_SP_HeadlessFightCommand(client_t *client, usercmd_s *cmd)
{
    // mod (mapkit-fix-2): bo1_testclient_passive alone (no _fight) also drives the client, so the documented camera
    // controls (_goto/_look/_press) work in screenshot runs; only the passive stand/walk runs then, never the fight plan.
    const bool passiveOnly = g_headlessFightPassive && g_headlessFightPassive->current.integer > 0
        && level.time >= g_headlessFightPassive->current.integer && (!g_headlessFight || !g_headlessFight->current.enabled);
    if (!G_SP_TestHarness() || !G_SP_IsZombieMode() || ((!g_headlessFight || !g_headlessFight->current.enabled) && !passiveOnly)
        || !g_headlessTestClient || !g_headlessTestClient->current.enabled
        || (!client->bIsTestClient && !G_SP_TestLocalClient()) || client - svs.clients != g_testClientNum)
        return false;

    // Harness policy only: SV_UpdateBots sends this through its ordinary SV_ClientThink.
    // Never write player angles, ammunition, damage, score, or script state here.
    *cmd = client->lastUsercmd;
    cmd->serverTime = svs.time;
    cmd->forwardmove = cmd->rightmove = 0;
    cmd->button_bits.array[0] = cmd->button_bits.array[1] = 0;
    // j1 test switch: CL_CmdButtons' stance bits (cl_input_mp.cpp: 9 crouch, 8 prone).
    if (g_headlessStance && g_headlessStance->current.integer == 1)
        cmd->button_bits.setBit(9);
    else if (g_headlessStance && g_headlessStance->current.integer == 2)
        cmd->button_bits.setBit(8);
    G_SP_TestPlanQuitCheck(); // q1 c9: also in last stand, bleed-out and intermission
    gentity_s *player = client->gentity;
    if (client->header.state != CS_ACTIVE || !player || !player->client || player->health <= 0
        || player->client->sess.sessionState != SESS_STATE_PLAYING)
        return true;

    // k1 TEST SWITCH (headless soak only, off by default): bo1_testclient_god keeps FL_GODMODE, the bit the
    // retail `god` cheat toggles (Cmd_God_f). As in SP, G_Damage applies knockback and returns for it before
    // the damage callback (SP 0x00540916), so the damage scripts do not run. It lets a soak reach late rounds
    // to measure the engine; it is not evidence that the test client survives them.
    if (g_headlessGod && g_headlessGod->current.enabled)
        player->flags |= 1;
    // p1 TEST SWITCH (off by default): bo1_testclient_hurt fires the engine call maps\_zombiemode_spawner's
    // zombie_damage_failsafe makes (DoDamage(health + 1000, origin, undefined, undefined, "riflebullet") ->
    // ScrCmd_DoDamage -> G_Damage) once, so a short run can show what a god / non-god player does with it.
    if (g_headlessHurt && g_headlessHurt->current.integer > 0 && level.time >= g_headlessHurt->current.integer
        && (s_headlessHurtAt < 0 || level.time < s_headlessHurtAt))
    {
        s_headlessHurtAt = level.time;
        Com_Printf(16, "bo1_testclient: time %d TEST SWITCH hurt %d health %d god %d\n", level.time,
            player->health + 1000, player->health, player->flags & 1);
        // As the MP kill command (g_cmds_mp.cpp): outside G_RunFrame the thread's bgs is not the level's, and the
        // last-stand anim event (G_SP_PlayerEnterLastStand -> BG_AnimScriptEvent) reads it.
        bgs_t *const savedBgs = bgs;
        bgs = &level_bgs;
        G_Damage(player, nullptr, nullptr, nullptr, player->r.currentOrigin, player->health + 1000, 0,
            MOD_RIFLE_BULLET, -1, HITLOC_HEAD, 0, 0, 0);
        bgs = savedBgs;
        Com_Printf(16, "bo1_testclient: time %d TEST SWITCH hurt done health %d\n", level.time, player->health);
    }
    // x6 c29 TEST SWITCH (off by default): bo1_testclient_hurtstep takes the player down in steps (the same
    // G_Damage as above, a fixed amount each), so a capture can show the low-health presentation while alive.
    if (g_headlessHurtStep && *g_headlessHurtStep->current.string)
    {
        int start = 0, amount = 0, interval = 0, count = 0;
        if (sscanf(g_headlessHurtStep->current.string, "%d %d %d %d", &start, &amount, &interval, &count) == 4
            && amount > 0)
        {
            if (s_headlessHurtStepNext < 0 || level.time < s_headlessHurtStepNext - interval) // new map / restart
            {
                s_headlessHurtStepNext = start;
                s_headlessHurtStepDone = 0;
            }
            if (s_headlessHurtStepDone < count && level.time >= s_headlessHurtStepNext)
            {
                s_headlessHurtStepNext = level.time + interval;
                ++s_headlessHurtStepDone;
                const int before = player->health;
                bgs_t *const savedBgs = bgs;
                bgs = &level_bgs;
                G_Damage(player, nullptr, nullptr, nullptr, player->r.currentOrigin, amount, 0,
                    MOD_RIFLE_BULLET, -1, HITLOC_HEAD, 0, 0, 0);
                bgs = savedBgs;
                Com_Printf(16, "bo1_testclient: time %d TEST SWITCH hurtstep %d/%d damage %d health %d -> %d\n",
                    level.time, s_headlessHurtStepDone, count, amount, before, player->health);
            }
        }
    }

    playerState_s *ps = &player->client->ps;
    if (passiveOnly)
    {
        SP_HeadlessPassiveMove(player, ps, cmd);
        return true;
    }
    // j1: a held (cooking) frag lowers the gun first; the weapon-switch fallbacks below would skip the
    // plan's command for those frames and so release the button. Let the plan hold it alone.
    if (G_SP_TestPlanHoldingGrenade())
    {
        G_SP_TestPlanCommand(player, client, cmd, nullptr, -1.0f, false);
        return true;
    }
    // w1 TEST SWITCH bo1_testclient_knife <melee weapon>: give it once, as maps\_zombiemode_bowie does
    // (GiveWeapon sets the melee weapon for a WEAPCLASS_MELEE weapon; TakeWeapon knife_zm).
    const bool knifeAlways = g_headlessKnife && *g_headlessKnife->current.string;
    if (knifeAlways && strcmp(g_headlessKnife->current.string, "1"))
    {
        const unsigned int knife = BG_FindWeaponIndexForName(g_headlessKnife->current.string);
        if (knife && !BG_PlayerHasWeapon(ps, knife))
        {
            renderOptions_s options;
            options.i = 0;
            G_GivePlayerWeapon(ps, knife, 0, options);
            const unsigned int stock = BG_FindWeaponIndexForName("knife_zm");
            if (stock && stock != knife && BG_PlayerHasWeapon(ps, stock))
                BG_TakePlayerWeapon(ps, stock);
            Com_Printf(16, "bo1_fightweapon: time %d melee weapon %s index %u meleeWeapon %s\n", level.time,
                g_headlessKnife->current.string, knife, BG_WeaponName(ps->meleeWeapon));
        }
    }
    // A script SwitchToWeapon reaches a test client as its bot weapon request (SV_BotSwitchWeapon),
    // which Bot_UpdateWeapon puts in the usercmd; honour it the same way while the weapon is held.
    // The local client receives script SwitchToWeapon through its client command, not botInfos.
    // A stale bot request otherwise makes the rendered harness stop fighting after a wall buy.
    // w1 TEST SWITCH bo1_testclient_fightweapons: the projectile weapons fired at zombies (give = the giveweapon
    // builtin's G_GivePlayerWeapon + G_InitializeAmmo; the previous weapon is taken, Five keeps two primaries).
    // The list starts at the first command that has a target (round 1 zombies need ~30 s to be seen).
    if (g_headlessFightWeapons && *g_headlessFightWeapons->current.string && g_fightWeaponsStart)
    {
        static int s_fwCycle = -1;
        unsigned int &s_fwWeapon = g_fightWeaponsCurrent;
        if (!s_fwWeapon)
            s_fwCycle = -1;
        const int cycle = (level.time - g_fightWeaponsStart) / 20000;
        if (cycle != s_fwCycle)
        {
            s_fwCycle = cycle;
            char name[64] = {};
            const char *list = g_headlessFightWeapons->current.string;
            for (int i = 0; i <= cycle && *list; ++i)
            {
                while (*list == ' ' || *list == ',')
                    ++list;
                int n = 0;
                while (*list && *list != ' ' && *list != ',' && n < 63)
                    name[n++] = *list++;
                name[n] = 0;
                if (i < cycle)
                    name[0] = 0;
            }
            // w1 c6: "alt:<gun>" holds the gun's alternate weapon; offhand / item weapons are thrown with the gun kept
            // (G_SP_HeadlessThrowCycle, g_sp_headless_throw.cpp).
            static unsigned int s_fwGiven;
            unsigned int next = 0, hold = 0;
            const bool thrown = name[0] && G_SP_HeadlessThrowCycle(name, &next, &hold);
            if (thrown && !s_fwWeapon)
                s_fwWeapon = ps->weapon; // the held gun (a zero current weapon restarts the cycle list)
            if (next && !thrown)
            {
                if (s_fwWeapon && s_fwWeapon != hold && BG_PlayerHasWeapon(ps, s_fwWeapon))
                    BG_TakePlayerWeapon(ps, s_fwWeapon);
                if (s_fwGiven && s_fwGiven != next && BG_PlayerHasWeapon(ps, s_fwGiven))
                    BG_TakePlayerWeapon(ps, s_fwGiven);
                if (!BG_PlayerHasWeapon(ps, next))
                {
                    renderOptions_s options;
                    options.i = 0;
                    if (G_GivePlayerWeapon(ps, next, 0, options))
                        G_InitializeAmmo(player, next, 0, 0);
                }
                // The alt's ammo comes from the give above: SP G_InitializeAmmo (SP 0x007d6010) walks the alt chain from the
                // parent; started at a dual wield's alt (microwavegun_zm) the chain never returns to it (the r11 assert).
                s_fwGiven = next;
                s_fwWeapon = hold;
                g_fightWeaponsAltParent = hold != next ? next : 0;
                if (client->bIsTestClient)
                    botInfos[client - svs.clients].weapon = hold;
            }
            Com_Printf(16, "bo1_fightweapon: time %d cycle %d weapon %s index %u has %d hold %s thrown %d\n", level.time,
                cycle, name, next, next ? BG_PlayerHasWeapon(ps, next) : 0, hold ? BG_WeaponName(hold) : "", (int)thrown);
        }
        if (s_fwWeapon && ps->weapon == s_fwWeapon && ps->weaponstate == WEAPON_READY
            && !BG_GetAmmoInClip(ps, s_fwWeapon) && !BG_GetAmmoNotInClip(ps, s_fwWeapon)
            && !G_SP_HeadlessThrowKeepEmpty(player, s_fwWeapon))
            G_InitializeAmmo(player, g_fightWeaponsAltParent ? g_fightWeaponsAltParent : s_fwWeapon, 0, 1);
    }
    const unsigned int requested = client->bIsTestClient ? (unsigned int)botInfos[client - svs.clients].weapon : 0;
    cmd->weapon = requested && BG_PlayerHasWeapon(ps, requested) ? requested : ps->weapon;
    // w1 c8: an alternate-mode weapon (alt:m16_gl_upgraded_zm -> gl_m16_upgraded_zm) is only kept with the usercmd's
    // lastWeaponAltModeSwitch naming its primary (BG_GetValidPrimaryWeaponForAltMode; the client sends
    // cgameUserCmdLastWeaponForAlt), else PM_BeginWeaponChange drops it to weapon 0.
    if (cmd->weapon && BG_GetWeaponDef(cmd->weapon)->inventoryType == WEAPINVENTORY_ALTMODE && g_fightWeaponsAltParent)
        cmd->lastWeaponAltModeSwitch = g_fightWeaponsAltParent;
    // The local (-Client) player has no bot weapon request: the fight-weapons switch raises its weapon directly.
    if (!client->bIsTestClient && g_fightWeaponsCurrent && ps->weapon != g_fightWeaponsCurrent
        && BG_PlayerHasWeapon(ps, g_fightWeaponsCurrent) && ps->weaponstate == WEAPON_READY)
        cmd->weapon = g_fightWeaponsCurrent;
    // Harness: let the requested bottle / knuckle-crack weapon raise before choosing another gun.
    // PaP removes the current gun before SwitchToWeapon; an ammo fallback here interrupts its animation.
    if (cmd->weapon != ps->weapon)
        return true;
    // r1: not while the plan raises an empty gun on purpose (the box or a wall buy replaces it).
    // q1 c9: projectile guns too (q18a: an empty PaP'd crossbow stayed raised for a 45 s walk with a loaded m16 held;
    // the player knifed round-8 zombies and went down).
    if (!ps->weapon || (ps->weaponstate == WEAPON_READY
        && (BG_GetWeaponDef(ps->weapon)->weapType == WEAPTYPE_BULLET
            || (BG_GetWeaponDef(ps->weapon)->weapType == WEAPTYPE_PROJECTILE
                && BG_GetWeaponDef(ps->weapon)->inventoryType == WEAPINVENTORY_PRIMARY))
        && !BG_GetAmmoInClip(ps, ps->weapon) && !BG_GetAmmoNotInClip(ps, ps->weapon)
        && ps->weapon != G_SP_TestPlanHoldWeapon()))
    {
        // Harness only: TakeWeapon followed by GiveWeapon/SwitchToWeapon (wall buy or thief loot)
        // leaves empty hands until this command is processed. Preserve the script's valid choice
        // already copied into cmd->weapon instead of overwriting it with the first held gun.
        if (requested && requested != ps->weapon && BG_PlayerHasWeapon(ps, requested)
            && BG_GetAmmoInClip(ps, requested) + BG_GetAmmoNotInClip(ps, requested) > 0)
            return true;

        // Empty hands (the thief's TakeWeapon of the held gun) or an empty gun: switch to another held gun
        // with ammo, as a player would.
        // r1 c5 (r4c 377450-388200): a bullet gun first, then any other primary that is not an explosive
        // projectile (the freezegun, clip 0 reserve 12, stayed lowered while a dry m16 knifed 13 zombies).
        for (int i = 0; i < 30; ++i)
        {
            const unsigned int held = ps->heldWeapons[i % 15].weapon;
            const WeaponDef *def = held ? BG_GetWeaponDef(held) : nullptr;
            if (held && held != ps->weapon && def->inventoryType == WEAPINVENTORY_PRIMARY
                && (i < 15 ? def->weapType == WEAPTYPE_BULLET
                    : !(def->weapType == WEAPTYPE_PROJECTILE && def->iExplosionRadius > 0))
                && BG_GetAmmoInClip(ps, held) + BG_GetAmmoNotInClip(ps, held) > 0)
            {
                // The player's own choice replaces a stale script SwitchToWeapon request.
                botInfos[client - svs.clients].weapon = held;
                cmd->weapon = held;
                return true;
            }
        }
        // Nothing to switch to (Pack-a-Punch holds the gun while it upgrades): the plan goes on.
        if (!ps->weapon)
        {
            if (G_SP_TestPlanActive())
                G_SP_TestPlanCommand(player, client, cmd, nullptr, -1.0f, false);
            return true;
        }
    }
    // L9 TEST SWITCH bo1_testclient_watchkill: look at the last death spot, no fire, with screenshots.
    if (g_watchKillStart && level.time - g_watchKillStart < 8000)
    {
        // 20 shots, one every 350 ms of level time from +100 ms (death anim, ragdoll start, corpse swap)
        float eye[3], dir[3], angles[3];
        G_GetPlayerViewOrigin(ps, eye);
        Vec3Sub(g_watchKillPoint, eye, dir);
        vectoangles(dir, angles);
        for (int i = 0; i < 3; ++i)
            cmd->angles[i] = (unsigned short)(int)((angles[i] - ps->delta_angles[i]) * 182.04445f);
        cmd->weapon = ps->weapon;
        if (g_watchKillShot < 20 && level.time - g_watchKillStart >= 100 + 350 * g_watchKillShot)
        {
            char command[128];
            Com_sprintf(command, sizeof(command), "screenshotJpeg bo1_watch%d_%d\n", g_watchKillCount, g_watchKillShot);
            Cbuf_AddText(0, command);
            Com_Printf(16, "bo1_watchkill: time %d kill %d ent %d shot %d\n", level.time, g_watchKillCount, g_watchKillEnt,
                g_watchKillShot);
            ++g_watchKillShot;
        }
        return true;
    }
    // x6: bo1_testclient_passive - stand still from this level time on (evidence of the downed / last stand path)
    if (g_headlessFightPassive && g_headlessFightPassive->current.integer > 0
        && level.time >= g_headlessFightPassive->current.integer)
    {
        SP_HeadlessPassiveMove(player, ps, cmd);
        return true;
    }
    // w1 c6 test switches: ballistic knife retrieval, freeze-gun shatter shots, the lure tracker.
    if (g_fightWeaponsStart && G_SP_HeadlessThrowPreCommand(player, client, cmd, g_fightWeaponsCurrent))
        return true;
    const WeaponDef *weapon = BG_GetWeaponDef(ps->weapon);
    const int clip = BG_GetAmmoInClip(ps, ps->weapon);
    const int reserve = BG_GetAmmoNotInClip(ps, ps->weapon);
    // r1 c6: not while a use trigger is under the cursor. Player_UpdateActivate uses the hinted trigger on a latched
    // bit 3/4/5 before bit 5 means reload (SP Player_UpdateUse 0x00559690 the same), so the press buys what the
    // player stands at. r6h 623800: waiting out a thief round at the QR trap switch (hint ent 350), the empty-m14
    // reload bought trap_qr; the player stood on the edge of its volume and the trap downed it. The empty clip still
    // reloads by itself (PM_Weapon_CheckForReload, bg_weapons.cpp: clip 0 with a reserve -> PM_BeginWeaponReload).
    const bool useHint = ps->cursorHint && ps->cursorHintEntIndex != 1023;
    if (!clip && reserve > 0 && !useHint)
        cmd->button_bits.setBit(5); // reload, same usercmd bit as Bot_UpdateReload

    float eye[3], aim[3] = {}, nearest = 8192.0f * 8192.0f;
    G_GetPlayerViewOrigin(ps, eye);
    gentity_s *target = nullptr;
    // f2 labs test switch: the thief is shot before other zombies (see G_SP_HeadlessLabsThief).
    gentity_s *thief = G_SP_HeadlessLabsThief(player);
    if (!thief)
        thief = G_SP_TestPlanThief(player); // r1: the feature plan's thief round
    bool thiefVisible = false;
    float thiefDistance = 0.0f, thiefAim[3] = {};
    if (thief)
    {
        if (!G_DObjGetWorldTagPos(thief, scr_const.j_head, thiefAim))
        {
            Vec3Copy(thief->r.currentOrigin, thiefAim);
            thiefAim[2] += 40.0f;
        }
        trace_t trace;
        G_LocationalTrace(&trace, eye, thiefAim, player->s.number, 0x280E833, bulletPriorityMap, nullptr);
        thiefVisible = trace.fraction >= 1.0f || (trace.hitType == TRACE_HITTYPE_ENTITY && trace.hitId == thief->s.number);
        thiefDistance = Vec3DistanceSq(eye, thiefAim);
    }
    // x6: bo1_testclient_aimtag - aim at another bone (limb gib / crawler evidence); unknown names use j_head.
    unsigned int aimTag = scr_const.j_head;
    if (g_headlessFightAimTag && *g_headlessFightAimTag->current.string)
    {
        static int s_aimTagReported;
        unsigned int tag = SL_FindString(g_headlessFightAimTag->current.string, SCRIPTINSTANCE_SERVER);
        if (!tag)
            tag = SL_FindLowercaseString(g_headlessFightAimTag->current.string, SCRIPTINSTANCE_SERVER);
        if (tag)
            aimTag = tag;
        if (!s_aimTagReported)
        {
            s_aimTagReported = 1;
            Com_Printf(16, "bo1_testclient: aim tag '%s' -> %u (j_head %u)\n", g_headlessFightAimTag->current.string,
                tag, (unsigned int)scr_const.j_head);
        }
    }
    for (int i = 0; level.actors && i < MAX_ACTORS; ++i)
    {
        actor_s *actor = &level.actors[i];
        if (!actor->inuse || !actor->ent || actor->ent->health <= 0 || !actor->Physics.bIsAlive
            || !actor->sentient || actor->sentient->eTeam == player->sentient->eTeam)
            continue;
        gentity_s *ent = actor->ent;
        if (g_headlessSpareCrawlers && g_headlessSpareCrawlers->current.enabled && G_SP_TestPlanActorLegless(ent->s.number))
            continue;
        float point[3];
        if (!G_DObjGetWorldTagPos(ent, aimTag, point))
        {
            Vec3Copy(ent->r.currentOrigin, point);
            point[2] += 40.0f;
        }
        const float distance = Vec3DistanceSq(eye, point);
        if (distance >= nearest)
            continue;
        trace_t trace;
        G_LocationalTrace(&trace, eye, point, player->s.number, 0x280E833, bulletPriorityMap, nullptr);
        if (trace.fraction < 1.0f && (trace.hitType != TRACE_HITTYPE_ENTITY || trace.hitId != ent->s.number))
            continue;
        nearest = distance;
        target = ent;
        Vec3Copy(point, aim);
    }
    const float meleeReach = player_meleeRange->current.value + 16.0f;
    if (thiefVisible && target != thief && (!target || nearest > meleeReach * meleeReach))
    {
        target = thief;
        nearest = thiefDistance;
        Vec3Copy(thiefAim, aim);
    }
    {
        // x6: the server look-at crosshair bits (8 friendly, 0x10 enemy, 0x200000; written by
        // Player_UpdateLookAtEntity_SP last frame) and its look-at entity, logged on change.
        static unsigned int s_lastLookAt = ~0u;
        static int s_lastLookAtEnt = -2;
        const unsigned int lookAt = ps->weapFlags & 0x200018u;
        const gclientSpExt &ext = G_ClientSpExt(player->client);
        const int lookAtEnt = ext.lookatent.isDefined() ? ext.lookatent.entnum() : -1;
        if (lookAt != s_lastLookAt || lookAtEnt != s_lastLookAtEnt)
        {
            s_lastLookAt = lookAt;
            s_lastLookAtEnt = lookAtEnt;
            Com_Printf(16, "bo1_lookat: time %d weapFlags 0x%x lookatent %d target %d dist %.0f weapon %s range %.0f\n",
                level.time, lookAt, lookAtEnt, target ? target->s.number : -1, target ? sqrtf(nearest) : -1.0f,
                BG_WeaponName(ps->weapon), weapon ? weapon->enemyCrosshairRange : -1.0f);
        }
    }
    // L20 harness policy (G_SP_TestPlanMayRetreat): 0 = the plan holds its spot, 1 = back away only while the gun
    // cannot fire, 2 = back away as without a plan.
    const int retreatMode = G_SP_TestPlanActive() ? G_SP_TestPlanMayRetreat(player) : 2;
    const bool reloading = ps->weaponstate >= WEAPON_RELOADING_RIGHT && ps->weaponstate <= WEAPON_RELOAD_QUICK_EMPTY;
    const bool cannotFire = reloading || (clip <= 0 && reserve > 0);
    // L20: a zombie inside 200 units and the held gun empty or reloading: raise another primary with a loaded clip,
    // as a player would (a switch is quicker than the reload). b111 761750: rpk_upgraded_zm reload (2750 ms) with a
    // zombie at 43 units and a loaded m16 held; a112m 122500: the box's m72_law reloading at 155 -> 25 units beside
    // the m16; x632m2 116550: the box's ballistic knife (clip 0) at 28 units beside the m16. Not an explosive gun
    // inside its own blast, not the gun a plan step holds (box / wall replacement), not in last stand, not with the
    // w1 fight-weapons test switch.
    if (target && cannotFire && nearest < 200.0f * 200.0f && retreatMode && !player->client->lastStand
        && !g_fightWeaponsCurrent && ps->weapon != G_SP_TestPlanHoldWeapon())
    {
        for (int i = 0; i < 15; ++i)
        {
            const unsigned int held = ps->heldWeapons[i].weapon;
            const WeaponDef *def = held ? BG_GetWeaponDef(held) : nullptr;
            if (!held || held == ps->weapon || def->inventoryType != WEAPINVENTORY_PRIMARY
                || BG_GetAmmoInClip(ps, held) <= 0
                || (def->weapType == WEAPTYPE_PROJECTILE && def->iExplosionRadius > 0
                    && sqrtf(nearest) < (float)def->iExplosionRadius + 64.0f))
                continue;
            botInfos[client - svs.clients].weapon = held;
            cmd->weapon = held;
            cmd->button_bits.resetBit(5);
            break;
        }
    }
    bool melee = false;
    if (!target)
        g_fightAdsStart = 0;
    if (target && !g_fightWeaponsStart)
    {
        g_fightWeaponsStart = level.time;
        G_SP_HeadlessThrowStart(player); // w1 c6 TEST SWITCH bo1_testclient_scriptgive
    }
    if (target)
    {
        float dir[3], angles[3];
        Vec3Sub(aim, eye, dir);
        vectoangles(dir, angles);
        for (int i = 0; i < 3; ++i)
            cmd->angles[i] = (unsigned short)(int)((angles[i] - ps->delta_angles[i]) * 182.04445f);
        // x6: bo1_testclient_holdfire - watch the nearest zombie without knifing or firing (animation shots)
        if (g_headlessFightHoldFire && g_headlessFightHoldFire->current.integer > 0
            && level.time >= g_headlessFightHoldFire->current.integer)
            return true;
        // L9 TEST SWITCH bo1_testclient_firerange: only look at a target farther than this (close-range kills).
        if (g_headlessFireRange && g_headlessFireRange->current.value > 0.0f
            && nearest > g_headlessFireRange->current.value * g_headlessFireRange->current.value)
            return true;
        // w1 c6: a thrown fight weapon's cycle (frag, monkey, claymore) throws / places instead of firing.
        if (G_SP_HeadlessThrowCommand(player, client, cmd, target, sqrtf(nearest), g_fightWeaponsCurrent))
            return true;
        // A target inside the melee reach is knifed, as a player would: the melee button is an
        // edge (PM_Weapon's !oldcmd test), so press it only on a released frame. Pmove, the melee
        // trace, the knife's damage and the damage scripts decide whether anything is hit.
        // k1: only when the held gun has no ammunition at all. From round ~10 the knife (150) is a few
        // percent of a zombie's health; standing still to knife was what downed the test client at
        // round 12 (f2-report, 175600-180600: melee 1, move 0 0, AK 13+120). With a loaded or reloading
        // gun it now fires from the hip at close range and backs away (below).
        if (sqrtf(nearest) <= player_meleeRange->current.value + 16.0f && (knifeAlways || (clip <= 0 && reserve <= 0)))
        {
            melee = true;
            if (!client->lastUsercmd.button_bits.testBit(2))
                cmd->button_bits.setBit(2);
        }
        else
        {
            // w1 TEST SWITCH bo1_testclient_hipfire: no ADS (a -Client screenshot sees the projectile leave the gun).
            const bool hip = Dvar_GetBool("bo1_testclient_hipfire");
            if (!hip)
                cmd->button_bits.setBit(11); // ADS
            if (!g_fightAdsStart)
                g_fightAdsStart = level.time;
            // Fire once aimed down the sights, or after 500 ms of holding ADS when the scripts
            // disallow it (AllowAds(false), e.g. while a perk bottle is drunk).
            const bool aimed = hip || ps->fWeaponPosFrac >= 1.0f || level.time - g_fightAdsStart >= 500;
            // r1: an explosive gun is not fired at a target inside its own blast (r2c: the box's upgraded m72_law
            // fired at 150 units downed the player, mod 6 damage 356); raise another loaded gun, as a player would.
            const bool selfBlast = !(g_headlessSelfBlast && g_headlessSelfBlast->current.enabled)
                && weapon->weapType == WEAPTYPE_PROJECTILE && weapon->iExplosionRadius > 0
                && sqrtf(nearest) < (float)weapon->iExplosionRadius + 64.0f;
            if (selfBlast && ps->weaponstate == WEAPON_READY)
            {
                for (int i = 0; i < 15; ++i)
                {
                    const unsigned int held = ps->heldWeapons[i].weapon;
                    if (!held || held == ps->weapon || BG_GetWeaponDef(held)->inventoryType != WEAPINVENTORY_PRIMARY
                        || (BG_GetWeaponDef(held)->weapType == WEAPTYPE_PROJECTILE && BG_GetWeaponDef(held)->iExplosionRadius > 0)
                        || BG_GetAmmoInClip(ps, held) + BG_GetAmmoNotInClip(ps, held) <= 0)
                        continue;
                    botInfos[client - svs.clients].weapon = held;
                    cmd->weapon = held;
                    break;
                }
            }
            // Release a single-shot trigger between shots; the weapon code owns fire rate/reload.
            if (clip > 0 && aimed && !ps->weaponTime && !selfBlast
                && (!weapon->fireType || !client->lastUsercmd.button_bits.testBit(0)))
                cmd->button_bits.setBit(0);
        }
    }

    // L20: top up a clip under 30% while no zombie is inside 600 units (the far one is not shot meanwhile), so the
    // reload does not come with zombies close. b111 749750-759750: rpk_upgraded_zm clip 7 of 125 for 10 s with the
    // nearest zombie at 280-1384 units, then 1 -> 0 and the 2750 ms reload with one at 43 units. Not over a use hint
    // (r1 c6, above), not while a plan step holds still (retreatMode 0: measured fire, views, rides, thief rounds).
    if (clip > 0 && reserve > 0 && !useHint && !reloading && retreatMode && !player->client->lastStand
        && cmd->weapon == ps->weapon && ps->weapon != G_SP_TestPlanHoldWeapon()
        && (float)clip < 0.3f * (float)BG_GetClipSize(ps->weapon) && (!target || nearest > 600.0f * 600.0f))
    {
        cmd->button_bits.resetBit(0);
        cmd->button_bits.setBit(5);
    }

    // A feature plan (bo1_testclient_plan) replaces the solo Quick Revive goal below.
    bool plan = G_SP_TestPlanActive();
    if (plan)
    {
        const bool commanded = G_SP_TestPlanCommand(player, client, cmd, target, target ? sqrtf(nearest) : -1.0f, melee);
        // L20: a dry player's wall buy (the plan presses use instead of knifing, g_sp_testplan.cpp dryWallBuy).
        if (melee && cmd->button_bits.testBit(3))
            cmd->button_bits.resetBit(2);
        // TEST SWITCH: a completed equipment plan can hand back to the labs fight policy. Keep the
        // plan's power-up pickup movement when it has a drop to collect; purchases and damage still
        // go through the scripts and normal usercmds. Plans without bo1_testclient_labs are unchanged.
        if (G_SP_HeadlessLabsActive() && G_SP_TestPlanComplete() && !commanded)
            plan = false;
    }

    // Purchases: walk to the goal while fighting, then face it and press use (an edge, like the
    // melee button). Never while knifing.
    float goalDistance = -1.0f;
    gentity_s *goal = melee || plan ? nullptr : SP_HeadlessPurchaseGoal(player);
    if (goal)
    {
        float center[3];
        for (int i = 0; i < 3; ++i)
            center[i] = 0.5f * (goal->r.absmin[i] + goal->r.absmax[i]);
        float toGoal[3];
        Vec3Sub(center, eye, toGoal);
        goalDistance = sqrtf(toGoal[0] * toGoal[0] + toGoal[1] * toGoal[1]);
        if (goalDistance > 64.0f)
        {
            float viewYaw = (float)(unsigned short)cmd->angles[1] / 182.04445f + ps->delta_angles[1];
            if (!target)
            {
                // Nothing to shoot: face the way the walk goes.
                float angles[3];
                vectoangles(toGoal, angles);
                cmd->angles[1] = (unsigned short)(int)((angles[1] - ps->delta_angles[1]) * 182.04445f);
                viewYaw = angles[1];
            }
            float ground[3] = { center[0], center[1], player->r.currentOrigin[2] };
            SP_HeadlessWalk(player, ground, goal->s.number, cmd, viewYaw);
        }
        else if (!target || nearest > 256.0f * 256.0f)
        {
            float angles[3];
            vectoangles(toGoal, angles);
            for (int i = 0; i < 2; ++i)
                cmd->angles[i] = (unsigned short)(int)((angles[i] - ps->delta_angles[i]) * 182.04445f);
            cmd->button_bits.resetBit(0);
            cmd->button_bits.resetBit(11);
            if (!client->lastUsercmd.button_bits.testBit(3))
                cmd->button_bits.setBit(3); // +activate
        }
    }
    else if (thief && !thiefVisible && !melee && !plan)
    {
        // f2 labs test switch: the thief is out of sight - walk toward it along the pathnode graph.
        float toThief[3];
        Vec3Sub(thief->r.currentOrigin, player->r.currentOrigin, toThief);
        float viewYaw = (float)(unsigned short)cmd->angles[1] / 182.04445f + ps->delta_angles[1];
        if (!target)
        {
            float angles[3];
            vectoangles(toThief, angles);
            cmd->angles[1] = (unsigned short)(int)((angles[1] - ps->delta_angles[1]) * 182.04445f);
            viewYaw = angles[1];
        }
        SP_HeadlessWalk(player, thief->r.currentOrigin, thief->s.number, cmd, viewYaw);
    }

    // k1 harness policy: back away from a close zombie while shooting, as a player would, instead of standing
    // in its swipes. Only when nothing else moves the test client (a plan, a purchase walk, the thief chase)
    // and not while a feature plan holds it in place (an elevator ride, a paid weapon's pickup window).
    // When the backward move is stopped for 200 ms (a wall), it adds a 1 s strafe, alternating sides.
    // L20: inside a plan too, over its movement, where G_SP_TestPlanMayRetreat allows it (2: always; 1: only while the
    // gun cannot fire - the plan's walk resumes once it can).
    const bool planRetreat = plan && (retreatMode == 2 || (retreatMode == 1 && cannotFire));
    if (target && (!plan || planRetreat) && !melee && !player->client->lastStand && nearest < 128.0f * 128.0f
        && (planRetreat || (!cmd->forwardmove && !cmd->rightmove)))
    {
        if (planRetreat)
        {
            // Face the zombie again (a plan walk or use may have turned the view): "back" is away from it.
            float dir[3], angles[3];
            Vec3Sub(aim, eye, dir);
            vectoangles(dir, angles);
            for (int i = 0; i < 3; ++i)
                cmd->angles[i] = (unsigned short)(int)((angles[i] - ps->delta_angles[i]) * 182.04445f);
            cmd->forwardmove = cmd->rightmove = 0;
            cmd->button_bits.resetBit(3);
        }
        cmd->forwardmove = -127;
        const float speed = sqrtf(ps->velocity[0] * ps->velocity[0] + ps->velocity[1] * ps->velocity[1]);
        if (level.time < g_fightStrafeUntil)
            cmd->rightmove = g_fightStrafeRight ? 127 : -127;
        else if (speed >= 40.0f)
            g_fightRetreatBlocked = 0;
        else if (!g_fightRetreatBlocked)
            g_fightRetreatBlocked = level.time;
        else if (level.time - g_fightRetreatBlocked >= 200)
        {
            g_fightRetreatBlocked = 0;
            g_fightStrafeRight = !g_fightStrafeRight;
            g_fightStrafeUntil = level.time + 1000;
        }
    }
    else
        g_fightRetreatBlocked = 0;

    // KB measurement (not SP): every change of the test client's health (damage and regen shape).
    if (player->health != g_fightReportHealth)
    {
        Com_Printf(16, "bo1_playerhealth: time %d health %d\n", level.time, player->health);
        g_fightReportHealth = player->health;
    }
    if (level.time >= g_fightReportTime)
    {
        Com_Printf(16, "bo1_fight: time %d target %d distance %.1f weapon %u clip %d reserve %d fire %d reload %d melee %d goal %d goalDistance %.1f move %d %d score %d health %d origin %.1f %.1f %.1f\n",
            level.time, target ? target->s.number : -1, target ? sqrtf(nearest) : -1.0f,
            ps->weapon, clip, reserve, cmd->button_bits.testBit(0), cmd->button_bits.testBit(5), melee,
            goal ? goal->s.number : -1, goalDistance, cmd->forwardmove, cmd->rightmove,
            player->client->sess.cs.score.score, player->health,
            player->r.currentOrigin[0], player->r.currentOrigin[1], player->r.currentOrigin[2]);
        g_fightReportTime = level.time + 1000;
    }
    return true;
}

static void G_SP_HeadlessActorSummary()
{
    if (!G_SP_IsZombieMode() || !level.actors || g_testClientNum < 0)
        return;
    const bool report = level.time >= g_actorReportTime;
    int live = 0, moved = 0;
    float nearest = -1.0f;
    for (int i = 0; i < MAX_ACTORS; ++i)
    {
        actor_s *actor = &level.actors[i];
        if (!actor->inuse || !actor->ent)
        {
            g_actorObservedEntity[i] = -1;
            continue;
        }
        gentity_s *ent = actor->ent;
        ++live;
        if (g_actorObservedEntity[i] != ent->s.number || g_actorObservedUseCount[i] != ent->useCount)
        {
            g_actorObservedEntity[i] = ent->s.number;
            g_actorObservedUseCount[i] = ent->useCount;
            ++g_actorSpawnCount;
            memcpy(g_actorObservedOrigin[i], ent->r.currentOrigin, sizeof(g_actorObservedOrigin[i]));
            Com_Printf(16, "bo1_actor_spawn: time %d total %d actor %d ent %d species %d origin %.1f %.1f %.1f\n",
                level.time, g_actorSpawnCount, i, ent->s.number, actor->species,
                ent->r.currentOrigin[0], ent->r.currentOrigin[1], ent->r.currentOrigin[2]);
        }
        float distanceSq = 0, motionSq = 0;
        for (int j = 0; j < 3; ++j)
        {
            float delta = ent->r.currentOrigin[j] - g_entities[g_testClientNum].r.currentOrigin[j];
            distanceSq += delta * delta;
            delta = ent->r.currentOrigin[j] - g_actorObservedOrigin[i][j];
            motionSq += delta * delta;
        }
        const float distance = sqrtf(distanceSq);
        if (nearest < 0 || distance < nearest)
            nearest = distance;
        moved += motionSq > 0.01f;
        if (report)
        {
            Com_Printf(16, "bo1_actor: time %d actor %d ent %d state %d origin %.1f %.1f %.1f delta %.2f playerDist %.1f health %d\n",
                level.time, i, ent->s.number, actor->eState[actor->stateLevel],
                ent->r.currentOrigin[0], ent->r.currentOrigin[1], ent->r.currentOrigin[2], sqrtf(motionSq), distance, ent->health);
            memcpy(g_actorObservedOrigin[i], ent->r.currentOrigin, sizeof(g_actorObservedOrigin[i]));
        }
        // L9 evidence: every actor's animscript, leaf anims and trajectory type (dead ones until they become corpses;
        // "dead" = ms since first seen dead, 0 while alive).
        if (ent->health > 0)
            g_actorDeadSince[i] = 0;
        else if (!g_actorDeadSince[i])
        {
            g_actorDeadSince[i] = level.time;
            if (g_headlessWatchKill && g_watchKillCount < g_headlessWatchKill->current.integer
                && (!g_watchKillStart || level.time - g_watchKillStart >= 8000))
            {
                ++g_watchKillCount;
                g_watchKillStart = level.time;
                g_watchKillEnt = ent->s.number;
                g_watchKillShot = 0;
                Vec3Copy(ent->r.currentOrigin, g_watchKillPoint);
                g_watchKillPoint[2] += 20.0f;
                Com_Printf(16, "bo1_watchkill: time %d kill %d ent %d start origin %.1f %.1f %.1f\n", level.time,
                    g_watchKillCount, g_watchKillEnt, ent->r.currentOrigin[0], ent->r.currentOrigin[1], ent->r.currentOrigin[2]);
            }
        }
        if (report)
        {
            char state[512];
            G_SP_PlayTraceDeadActor(actor, g_actorDeadSince[i] ? g_actorDeadSince[i] : level.time, state, sizeof(state));
            Com_Printf(16, "%s: time %d {%s,\"g\":%.0f}\n", ent->health > 0 ? "bo1_liveactor" : "bo1_deadactor", level.time, state,
                actor->Physics.fGravity); // L42: g = Actor_DoMove_SP gravity (moon_gravity 136, else bg_gravity)
        }
    }
    if (report)
    {
        Com_Printf(16, "bo1_actors: time %d spawned %d live %d moved %d nearest %.1f playerHealth %d\n",
            level.time, g_actorSpawnCount, live, moved, nearest, g_entities[g_testClientNum].health);
        // L17 measurement (not SP): entities in use, to watch for leaks across rounds (actor corpses are the actors'
        // own entities since G_CorpseFromActor).
        int inuse = 0, actorEnts = 0, corpseEnts = 0;
        for (int e = 0; e < level.num_entities; ++e)
        {
            if (!g_entities[e].r.inuse)
                continue;
            ++inuse;
            actorEnts += g_entities[e].s.eType == ET_ACTOR;
            corpseEnts += g_entities[e].s.eType == ET_ACTOR_CORPSE;
        }
        Com_Printf(16, "bo1_entities: time %d inuse %d actors %d corpses %d num %d\n", level.time, inuse, actorEnts,
            corpseEnts, level.num_entities);
        g_actorReportTime = level.time + 1000;
    }
}

bool G_SP_DeferHeadlessTestClientBegin()
{
    return Sys_IsHeadless() && G_SP_IsZombieMode() && g_addingHeadlessTestClient;
}

void G_SP_HeadlessClientBegin(unsigned int clientNum)
{
    if (Sys_IsHeadless() && g_headlessTestClient && g_headlessTestClient->current.enabled)
        Com_Printf(16, "bo1_testclient: begin notify client %u time %d\n", clientNum, level.time);
}

void G_SP_HeadlessTestClientFrame(bool afterFrame)
{
    {
        // L43: bo1_testclient_god in a real window (no harness there): the player(s) keep FL_GODMODE, the bit the
        // retail `god` cheat toggles (Cmd_God_f); G_Damage returns for it before the damage callback (SP 0x00540916).
        // Set before and after G_RunFrame, as a spawn inside the frame rebuilds the entity's flags. L43 pass 6: headless
        // too, so god holds without bo1_testclient_fight (the stress arena's -Client player must outlive the hold).
        if (g_headlessGod && g_headlessGod->current.enabled && G_SP_IsZombieMode() && sv.state == SS_GAME
            && !level.initializing && com_sv_running->current.enabled)
        {
            for (int i = 0; i < com_maxclients->current.integer; ++i)
            {
                gentity_s *player = &g_entities[i];
                if (svs.clients[i].header.state == CS_ACTIVE && player->client && player->health > 0
                    && player->client->sess.sessionState == SESS_STATE_PLAYING)
                    player->flags |= 1;
            }
        }
    }
    if (!G_SP_TestHarness())
        return;
    if (!g_headlessTestClient || !g_headlessTestClient->current.enabled)
        return;
    if (sv.state != SS_GAME || level.initializing || !com_sv_running->current.enabled)
        return;
    if (!afterFrame)
    {
        // The zombie harness admits on the first server frame. SV_DirectConnect
        // excludes unused slots from its reconnect test only during this admission.
        if (!G_SP_IsSPLevel() && svs.time < 1000 * Dvar_GetInt("sv_reconnectlimit"))
            return;
        if (!g_testClientAttempted && G_SP_TestLocalClient())
        {
            // x6: with a local client (-Client) the local client IS the player: no second (bot) player joins, the
            // harness drives the local client's usercmds instead (G_SP_HeadlessLocalClientCommand).
            for (int i = 0; i < com_maxclients->current.integer; ++i)
            {
                if (svs.clients[i].header.state == CS_ACTIVE && !svs.clients[i].bIsTestClient)
                {
                    g_testClientAttempted = true;
                    g_testClientNum = i;
                    Com_Printf(16, "bo1_testclient: local client %d is the player at time %d\n", i, level.time);
                    break;
                }
            }
            return;
        }
        if (!g_testClientAttempted)
        {
            g_testClientAttempted = true;
            Com_Printf(16, "bo1_testclient: SV_AddTestClient at time %d\n", level.time);
            g_addingHeadlessTestClient = true;
            gentity_s *ent = SV_AddTestClient();
            g_addingHeadlessTestClient = false;
            if (ent)
                g_testClientNum = ent->s.number;
            Com_Printf(16, "bo1_testclient: admitted client %d\n", g_testClientNum);
        }
        else if (g_testClientNum >= 0 && g_testClientFrames >= 1
            && svs.clients[g_testClientNum].header.state == CS_CLIENTLOADING)
        {
            // Simulate a client finishing its map load. SP's first_player_connect
            // waits until frame end before notifying "connecting". The bot's
            // usual immediate EnterWorld makes difficulty_init run before that.
            // Keep the real CS_CLIENTLOADING -> CS_ACTIVE / ClientBegin path.
            usercmd_s cmd = {};
            SV_ClientEnterWorld(&svs.clients[g_testClientNum], &cmd);
        }
        return;
    }
    if (g_testClientNum < 0)
        return;
    if (Sys_IsHeadlessClient() && G_SP_HeadlessReplayLocalFrame(&svs.clients[g_testClientNum]))
    {
        // a1 chunk 4: bo1_testclient_replay drives the local client (G_SP_HeadlessReplayLocalCommand), not the fight.
        AcquireSRWLockExclusive(&g_localFightLock);
        g_localFightValid = false;
        ReleaseSRWLockExclusive(&g_localFightLock);
    }
    else if (G_SP_TestLocalClient())
    {
        usercmd_s cmd;
        // w1 c12: the p1 / w1 labs (bo1_testclient_move / _fall / _weapon) drive the local client too, for -Client runs.
        const bool lab = G_SP_HeadlessMoveCommand(&svs.clients[g_testClientNum], &cmd);
        const bool valid = lab || G_SP_HeadlessFightCommand(&svs.clients[g_testClientNum], &cmd);
        AcquireSRWLockExclusive(&g_localFightLock);
        g_localFightValid = valid;
        if (valid)
        {
            g_localFightCmd = cmd;
            // Only the harness's empty-ammo fallback overrides the local client's weapon choice.
            // Its ordinary choice must keep honoring script SwitchToWeapon while it is in flight.
            // w1 c12: a lab's weapon choice always stands (the local client's own cmd weapon is its last selection).
            g_localFightWeaponOverride = lab || cmd.weapon != g_entities[g_testClientNum].client->ps.weapon;
        }
        ReleaseSRWLockExclusive(&g_localFightLock);
    }
    G_SP_HeadlessActorSummary();
    ++g_testClientFrames;
    if (svs.clients[g_testClientNum].header.state == CS_ACTIVE)
        ++g_testClientActiveFrames;
    if (g_testClientFrames > 10 && g_testClientFrames % 20)
        return;
    int connected = 0, expected = 0;
    for (int i = 0; i < com_maxclients->current.integer; ++i)
    {
        connected += svs.clients[i].header.state == CS_ACTIVE;
        expected += svs.clients[i].header.state > CS_ZOMBIE;
    }
    gentity_s *ent = &g_entities[g_testClientNum];
    Com_Printf(16, "bo1_testclient: frame %d activeFrame %d time %d client %d state %d connected %d expected %d all_players_are_connected %d session %d team %d sentientTeam %d score %d health %d\n",
        g_testClientFrames, g_testClientActiveFrames, level.time, g_testClientNum, svs.clients[g_testClientNum].header.state,
        connected, expected, Dvar_GetInt("all_players_are_connected"), ent->client->sess.connected,
        ent->client->sess.cs.team, ent->sentient ? ent->sentient->eTeam : -1,
        ent->client->sess.cs.score.score, ent->health);
}

// x6: the client side of the -Client harness: CL_CreateCmd replaces its (empty: no input when headless) aim, moves
// and buttons with the last fight command the server frame computed for this client. Harness only.
bool G_SP_HeadlessLocalClientCommand(usercmd_s *cmd, const playerState_s *predictedPs)
{
    if (!G_SP_TestLocalClient())
        return false;
    if (G_SP_HeadlessReplayLocalCommand(cmd, predictedPs))
        return true;
    AcquireSRWLockShared(&g_localFightLock);
    const bool valid = g_localFightValid;
    if (valid)
    {
        // L10: not while frozen - a player's mouse does not turn the view then (CL_MouseMove skips it on the
        // snapshot's pm_flags 0x800, SP 0x00881930), so the harness does not either (the thief grab's scripted turn).
        if ((predictedPs->pm_flags & 0x800) == 0)
            for (int i = 0; i < 3; ++i)
                cmd->angles[i] = g_localFightCmd.angles[i];
        cmd->forwardmove = g_localFightCmd.forwardmove;
        cmd->rightmove = g_localFightCmd.rightmove;
        cmd->button_bits.array[0] = g_localFightCmd.button_bits.array[0];
        cmd->button_bits.array[1] = g_localFightCmd.button_bits.array[1];
        if (g_localFightWeaponOverride)
            cmd->weapon = g_localFightCmd.weapon;
    }
    ReleaseSRWLockShared(&g_localFightLock);
    return valid;
}

void G_SP_SetClientConnectPending(unsigned int clientNum, bool pending)
{
    iassert(clientNum < 32);
    g_clientConnectPending[clientNum] = pending;
}

void SV_RunClientConnectCallbacks()
{
    if (!G_SP_IsSPLevel())
        return;

    // zombies: pending connect callbacks run before G_RunFrame (SP 0x004EF8F0).
    // Clear first: script may disconnect the client. Replay begin after connect
    // installs listeners when SV_ClientEnterWorld already made it connected.
    for (unsigned int i = 0; i < level.maxclients; ++i)
    {
        if (!g_clientConnectPending[i])
            continue;
        g_clientConnectPending[i] = false;
        gclient_s *client = &level.clients[i];
        if (client->sess.connected == CON_CONNECTING || client->sess.connected == CON_CONNECTED)
            Scr_PlayerConnect(&g_entities[i]);
        if (client->sess.connected == CON_CONNECTED)
            ClientBegin(i);
    }
}

