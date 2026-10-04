// p1: headless movement lab (TEST SWITCH bo1_testclient_move, harness only). The zombie test client runs a fixed
// sequence of usercmds (walk / back / strafe / crouch / prone / jump / sprint to exhaustion) and every server frame
// prints its playerState as a "bo1_move:" line; tools/p1_recording_feel.mjs --log measures it the same way it
// measures the retail recordings (retail research extracted/probe/sessions). The movement lab writes usercmds;
// the separate bo1_testclient_fall lab first places the player on a collision-verified ledge, then jumps
// toward a lower floor using usercmds. Pmove, scripts and the game decide movement and damage.
#include "g_sp_headless_move.h"
#include "g_sp_levelstart.h"
#include "g_sp_player_state.h"
#include <client_mp/g_client_mp.h>
#include <game_mp/g_main_mp.h>
#include <client_mp/sv_client_mp.h>
#include <server_mp/sv_main_mp.h>
#include <win32/win_main.h>
#include <universal/dvar.h>
#include <qcommon/common.h>
#include <bgame/bg_misc.h>
#include <bgame/bg_perks.h>
#include <server_mp/sv_bot_mp.h>
#include <game/g_bsp.h>
#include <game/pathnode.h>
#include <game_mp/g_misc_mp.h>
#include <server/sv_world.h>
#include <qcommon/cm_trace.h>
#include <bgame/bg_local.h>
#include <bgame/bg_weapons_def.h>
#include <universal/com_math.h>
#include <bgame/bg_mantle.h>
#include <game/g_weapon.h>
#include <game_mp/g_client_script_cmd_mp.h>

extern bot_info_t botInfos[32]; // sv_bot_mp.cpp

static const dvar_t *g_headlessMove;
static const dvar_t *g_headlessCrowdMove;
static const dvar_t *g_headlessFall;
static const dvar_t *g_headlessFallDive;
static const dvar_t *g_headlessWeapon;
static const dvar_t *g_headlessWeapons;
static int g_moveStartTime = -1;
static int g_moveBaseYaw;
static int g_moveYawOffset;
static int g_moveLastTime;

struct MovePhase
{
    const char *name;
    int durationMs;
    int forward, right;
    int stanceBit;      // 8 prone, 9 crouch, -1 stand (CL_AddCurrentStanceToCmd's held stance bits)
    bool sprint;        // button bit 1
    int jumpPeriodMs;   // > 0: press jump (bit 10) for 100 ms every period
    float yawRate;      // degrees per second
    unsigned int disabled; // SP AllowStand/AllowCrouch/AllowProne bits held for the phase (0x400000/0x800000/0x1000000)
};

// Level-time schedule. The reverse phases walk the player back so the lab stays in the spawn room.
static const MovePhase g_movePhases[] = {
    { "idle", 1000, 0, 0, -1, false, 0, 0.0f },
    { "walk", 1500, 127, 0, -1, false, 0, 0.0f },
    { "back", 2000, -127, 0, -1, false, 0, 0.0f },
    { "strafeL", 1500, 0, -127, -1, false, 0, 0.0f },
    { "strafeR", 1500, 0, 127, -1, false, 0, 0.0f },
    { "crouchF", 2000, 127, 0, 9, false, 0, 0.0f },
    { "crouchB", 2000, -127, 0, 9, false, 0, 0.0f },
    { "stand", 1500, 0, 0, -1, false, 0, 0.0f },
    { "proneF", 3500, 127, 0, 8, false, 0, 0.0f },
    { "proneB", 3500, -127, 0, 8, false, 0, 0.0f },
    { "stand2", 2000, 0, 0, -1, false, 0, 0.0f },
    { "jump", 3000, 0, 0, -1, false, 1000, 0.0f },
    { "sprint", 9000, 127, 0, -1, true, 0, 90.0f },
    { "walk2", 1500, 127, 0, -1, false, 0, 90.0f },
    { "sprint2", 3000, 127, 0, -1, true, 0, 90.0f },
    { "sprintJump", 2000, 127, 0, -1, true, 1000, 90.0f },
    // stance permissions (SP PM_UpdateStance): stand refused -> crouch; crouch refused while crouched -> stand;
    // prone refused while standing -> stand; prone refused with stand refused -> crouch.
    { "noStand", 1500, 0, 0, -1, false, 0, 0.0f, 0x400000 },
    { "noCrouch", 1500, 0, 0, 9, false, 0, 0.0f, 0x800000 },
    { "noProne", 2000, 0, 0, 8, false, 0, 0.0f, 0x1000000 },
    { "noProneStand", 2000, 0, 0, 8, false, 0, 0.0f, 0x1400000 },
    // q1c16: prone turning in place (SP BG_CheckProneTurned / PM_UpdateProneAngles body length; pmf 0x1000 = blocked).
    { "proneIn", 1500, 0, 0, 8, false, 0, 0.0f },
    { "proneTurn", 6000, 0, 0, 8, false, 0, 120.0f },
    { "done", 1000, 0, 0, -1, false, 0, 0.0f },
};

void G_SP_HeadlessMoveRegister()
{
    if (!g_headlessCrowdMove && Sys_IsHeadless())
        g_headlessCrowdMove = _Dvar_RegisterBool("bo1_testclient_crowdmove", false, 0,
            "TEST SWITCH: continuously walk and turn the headless crowd client");
    if (!g_headlessMove)
        g_headlessMove = _Dvar_RegisterBool("bo1_testclient_move", false, 0,
            "Headless test harness: run the p1 movement lab on the test client and log its playerState");
    if (!g_headlessFall && Sys_IsHeadless())
        g_headlessFall = _Dvar_RegisterBool("bo1_testclient_fall", false, 0,
            "TEST SWITCH: find a Five ledge, place the test client on it, then jump off and log the fall");
    if (!g_headlessFallDive && Sys_IsHeadless())
        g_headlessFallDive = _Dvar_RegisterBool("bo1_testclient_falldive", false, 0,
            "TEST SWITCH (with bo1_testclient_fall): give PhD Flopper (specialty_flakjacket) and dive off the ledge instead of jumping");
    if (!g_headlessWeapon && Sys_IsHeadless())
        g_headlessWeapon = _Dvar_RegisterBool("bo1_testclient_weapon", false, 0,
            "TEST SWITCH: drive sprint/ADS/knife/fire/reload usercmds and capture weapon assets and timers");
    if (!g_headlessWeapons && Sys_IsHeadless())
        g_headlessWeapons = _Dvar_RegisterString("bo1_testclient_weapons", "", 0,
            "TEST SWITCH (with bo1_testclient_weapon): after its lab, give and cycle these weapons (comma or space separated), 32 s each");
}

// TEST HARNESS (w1): after the single-weapon lab, cycle through bo1_testclient_weapons (comma separated names).
// Each weapon is given once (the giveweapon builtin's G_GivePlayerWeapon + G_InitializeAmmo, the only state write),
// then driven by buttons alone: switch (first raise), ADS in/out, fire the clip empty once (held for full auto,
// pressed every other command otherwise), the empty reload, one shot, a partial reload, then a switch to the
// spawn pistol and back (drop and raise). Logs one bo1_weapcyc line per command.
// w1 c12: then sprint 1000 ms, release and fire at once (sprint-in, the sprint drop, the first shot after it), and walk
// back 2500 ms towards where the sprint started; one more shot and a partial reload (the first one can still be in an
// empty segmented reload at 13500).
static void SP_HeadlessWeaponCycle(gentity_s *player, usercmd_s *cmd, int elapsed)
{
    static const int cycleMs = 32000;
    static int lastCycle = -1, weaponIndex, prevWeapon, firstWeapon, awayWeapon;
    static bool pressedLast, emptied;
    playerState_s *ps = &player->client->ps;
    const int cycle = elapsed / cycleMs, c = elapsed % cycleMs;
    if (cycle != lastCycle)
    {
        lastCycle = cycle;
        prevWeapon = weaponIndex;
        weaponIndex = 0;
        char name[64];
        const char *list = g_headlessWeapons->current.string;
        for (int i = 0; i <= cycle && *list; ++i)
        {
            while (*list == ' ' || *list == ',')
                ++list;
            int n = 0;
            while (*list && *list != ' ' && *list != ',' && n < 63)
                name[n++] = *list++;
            name[n] = 0;
            if (i == cycle && n)
                weaponIndex = BG_FindWeaponIndexForName(name);
        }
        // Five's player_too_many_weapons_monitor takes a third primary, so the previous cycle's weapon goes first
        // (the takeweapon builtin's BG_TakePlayerWeapon); the away switch then returns to the spawn pistol.
        // w1 c12: from the second weapon on, the away switch goes to the first listed weapon (a primary: its full
        // drop and raise times, not the quick change to a pistol) and the spawn pistol is taken instead.
        if (cycle == 1 && prevWeapon && firstWeapon && prevWeapon != firstWeapon && weaponIndex != prevWeapon)
        {
            BG_TakePlayerWeapon(ps, firstWeapon);
            awayWeapon = prevWeapon;
        }
        else if (prevWeapon && prevWeapon != firstWeapon && prevWeapon != awayWeapon && weaponIndex != prevWeapon)
            BG_TakePlayerWeapon(ps, prevWeapon);
        if (!firstWeapon)
            firstWeapon = ps->weapon;
        if (weaponIndex && !BG_PlayerHasWeapon(ps, weaponIndex))
        {
            renderOptions_s options;
            options.i = 0;
            if (G_GivePlayerWeapon(ps, weaponIndex, 0, options))
                G_InitializeAmmo(player, weaponIndex, 0, 0);
        }
        const WeaponDef *def = weaponIndex ? BG_GetWeaponDef(weaponIndex) : nullptr;
        const WeaponVariantDef *variant = weaponIndex ? BG_GetWeaponVariantDef(weaponIndex) : nullptr;
        if (def)
            Com_Printf(16, "bo1_weapcyc_asset: w=%d name=%s has=%d fireType=%d fire=%d,%d lastFire=%d reload=%d,%d add=%d,%d raise=%d drop=%d quickRaise=%d,%d ads=%.6f,%.6f spread=%.3f,%.3f,%.3f add=%.3f decay=%.3f clip=%d\n",
                weaponIndex, variant->szInternalName, BG_PlayerHasWeapon(ps, weaponIndex), def->fireType, def->iFireTime,
                def->iFireDelay, def->iLastFireTime, variant->iReloadTime, variant->iReloadEmptyTime, def->iReloadAddTime,
                def->iReloadEmptyAddTime, def->iRaiseTime, def->iDropTime, def->quickRaiseTime, def->quickDropTime,
                variant->fOOPosAnimLength[0], variant->fOOPosAnimLength[1], def->fHipSpreadStandMin, def->hipSpreadStandMax,
                def->fAdsSpread, def->fHipSpreadFireAdd, def->fHipSpreadDecayRate, variant->iClipSize);
        if (def)
            Com_Printf(16, "bo1_weapcyc_asset2: w=%d name=%s first=%d sprint=%d,%d start=%d,%d end=%d emptyRaise=%d emptyDrop=%d quickDrop=%d rechamber=%d\n",
                weaponIndex, variant->szInternalName, def->iFirstRaiseTime, def->sprintInTime, def->sprintOutTime,
                def->iReloadStartTime, def->iReloadStartAddTime, def->iReloadEndTime, def->iEmptyRaiseTime,
                def->iEmptyDropTime, def->quickDropTime, def->iRechamberTime);
        else
            Com_Printf(16, "bo1_weapcyc_asset: cycle=%d no weapon\n", cycle);
    }
    if (!weaponIndex)
        return;
    const int clip = BG_GetAmmoInClip(ps, weaponIndex);
    const char *phase = "raise";
    bool fire = false;
    cmd->weapon = weaponIndex;
    if (c == 0)
        emptied = false;
    // Reload as the retail recording's driver does (drive.py engage): press it as soon as the clip reads 0.
    const bool reloadIfEmpty = !clip && BG_GetAmmoNotInClip(ps, weaponIndex) > 0 && !(ps->weaponstate >= 8 && ps->weaponstate <= 16);
    if (c >= 3000 && c < 4200)
    {
        phase = "ads";
        cmd->button_bits.setBit(11);
    }
    else if (c >= 4200 && c < 5400)
        phase = "adsOut";
    else if (c >= 5400 && c < 13400)
    {
        phase = "fire";
        if (!clip)
            emptied = true;
        if (!emptied && ps->weapon == (unsigned int)weaponIndex)
            fire = BG_GetWeaponDef(weaponIndex)->fireType == WEAPON_FIRETYPE_FULLAUTO || !pressedLast;
        else if (reloadIfEmpty)
            cmd->button_bits.setBit(4);
    }
    else if (c >= 13500 && c < 13600)
    {
        phase = "shot";
        fire = !pressedLast;
    }
    else if (c >= 14100 && c < 14200)
    {
        phase = "reload";
        cmd->button_bits.setBit(4);
    }
    else if (c >= 18000 && c < 20000 && (awayWeapon ? awayWeapon : firstWeapon) != weaponIndex
        && (awayWeapon || firstWeapon))
    {
        // A plain switch away (to the spawn pistol; w1 c12: to the first listed weapon from the second cycle on) and
        // back: this weapon's drop, then its raise.
        phase = "away";
        cmd->weapon = awayWeapon ? awayWeapon : firstWeapon;
    }
    else if (c >= 23000 && c < 24000)
    {
        phase = "sprint";
        cmd->button_bits.setBit(1);
        cmd->forwardmove = 127;
    }
    else if (c >= 24000 && c < 24600)
    {
        // Fire from the first command after the sprint ends (every other command, as for a semi-automatic).
        phase = "sprintFire";
        fire = !pressedLast && clip > 0;
    }
    else if (c >= 24600 && c < 27100)
    {
        phase = "back";
        cmd->forwardmove = -127;
    }
    else if (c >= 27500 && c < 27600)
    {
        phase = "shot2";
        fire = !pressedLast;
    }
    else if (c >= 28100 && c < 28200)
    {
        phase = "reload2";
        cmd->button_bits.setBit(4);
    }
    else if (c >= 13500)
        phase = "wait";
    if (fire)
        cmd->button_bits.setBit(0);
    pressedLast = fire;
    Com_Printf(16, "bo1_weapcyc: t=%d c=%d next=%s cmd=%d w=%d state=%d anim=%d time=%d delay=%d clip=%d stock=%d ads=%.6f spread=%.3f fire=%d\n",
        level.time, c, phase, ps->commandTime, ps->weapon, ps->weaponstate, ps->weapAnim, ps->weaponTime,
        ps->weaponDelay, clip, BG_GetAmmoNotInClip(ps, weaponIndex), ps->fWeaponPosFrac, ps->aimSpreadScale, fire ? 1 : 0);
}

// TEST HARNESS: only buttons/movement, no weapon-state, ammo, health or timer writes.
// Samples describe the completed commandTime; phase/buttons describe the NEXT command.
static void SP_HeadlessWeaponCommand(gentity_s *player, usercmd_s *cmd)
{
    static int startTime = -1;
    playerState_s *ps = &player->client->ps;
    if (ps->pm_flags & 0x800)
        return;
    if (startTime < 0)
    {
        startTime = level.time;
        const unsigned int weapons[] = {ps->weapon, (unsigned int)ps->meleeWeapon, (unsigned int)ps->offHandIndex};
        for (unsigned int weapon : weapons)
        {
            const WeaponDef *def = BG_GetWeaponDef(weapon);
            const WeaponVariantDef *variant = BG_GetWeaponVariantDef(weapon);
            Com_Printf(16, "bo1_weapon_asset: w=%u name=%s raise=%d drop=%d first=%d sprint=%d,%d reload=%d,%d,%d fire=%d,%d melee=%d,%d,%d,%d adsrate=%.9f,%.9f zoom=%.3f,%.3f,%.3f zoomfrac=%.3f,%.3f sway=%.3f,%.3f,%.3f,%.3f range=%.3f\n",
                weapon, variant->szInternalName, def->iRaiseTime, def->iDropTime, def->iFirstRaiseTime,
                def->sprintInTime, def->sprintOutTime, variant->iReloadTime, variant->iReloadEmptyTime, def->iReloadAddTime,
                def->iFireTime, def->iFireDelay, def->iMeleeTime, def->iMeleeDelay, def->meleeChargeTime,
                def->meleeChargeDelay, variant->fOOPosAnimLength[0], variant->fOOPosAnimLength[1],
                variant->fAdsZoomFov1, variant->fAdsZoomFov2, variant->fAdsZoomFov3,
                variant->fAdsZoomInFrac, variant->fAdsZoomOutFrac, def->swayMaxAngle, def->swayLerpSpeed,
                def->swayPitchScale, def->swayYawScale, def->meleeChargeRange);
        }
    }
    const int elapsed = level.time - startTime;
    if (elapsed > 14000)
    {
        if (g_headlessWeapons && *g_headlessWeapons->current.string)
            SP_HeadlessWeaponCycle(player, cmd, elapsed - 14000);
        return;
    }
    const char *phase = "idle";
    if (elapsed >= 2000 && elapsed < 3000)
    {
        phase = "ads";
        cmd->button_bits.setBit(11);
    }
    else if (elapsed >= 3000 && elapsed < 4000)
        phase = "adsOut";
    else if (elapsed >= 4000 && elapsed < 5500)
    {
        phase = "sprint";
        cmd->button_bits.setBit(1);
        cmd->forwardmove = 127;
    }
    else if (elapsed >= 5500 && elapsed < 6500)
        phase = "sprintOut";
    else if (elapsed >= 6500 && elapsed < 7500)
    {
        phase = "melee";
        if (elapsed < 6600)
            cmd->button_bits.setBit(2);
    }
    else if (elapsed >= 7500 && elapsed < 9000)
    {
        phase = "fire";
        if ((elapsed - 7500) % 300 < 100)
            cmd->button_bits.setBit(0);
    }
    else if (elapsed >= 9000 && elapsed < 12000)
    {
        phase = "reload";
        if (elapsed < 9100)
            cmd->button_bits.setBit(4);
    }
    Com_Printf(16, "bo1_weapon: t=%d elapsed=%d next=%s cmd=%d w=%d state=%d time=%d delay=%d ads=%.6f sprint=%d charge=%d,%d vel=%.3f,%.3f,%.3f health=%d\n",
        level.time, elapsed, phase, ps->commandTime, ps->weapon, ps->weaponstate, ps->weaponTime,
        ps->weaponDelay, ps->fWeaponPosFrac, (ps->pm_flags & 0x8000) != 0, ps->meleeChargeDist,
        ps->meleeChargeTime, ps->velocity[0], ps->velocity[1], ps->velocity[2], player->health);
}

// TEST HARNESS: search real map collision for a standing ledge and a lower floor. Only the initial
// placement teleports; the fall uses ordinary usercmds, gravity, collision and damage. No health writes.
static void SP_HeadlessFallCommand(gentity_s *player, usercmd_s *cmd)
{
    static int startTime = -1;
    static bool airborne, landed;
    static float heading, ledgeHeight, ledgeXY[2];
    static int sprintStart = -1, diveTime = -1;
    playerState_s *ps = &player->client->ps;
    const bool dive = g_headlessFallDive && g_headlessFallDive->current.enabled;
    if (ps->pm_flags & 0x800)
        return;
    // L39: Ascension's intro carries the player down linked to a mover (dive1: setup at time 50 was undone, the
    // player descended at -697 -118 with velocity 0 for 6 s); the dive lab starts once he stands free.
    if (dive && startTime == -1 && (ps->pm_type != 0 || level.time < 15000))
        return;
    if (startTime == -1)
    {
        startTime = -2; // A failed search is reported once, never replaced with a synthetic air drop.
        const float dirs[8][2] = {{1,0},{.70710678f,.70710678f},{0,1},{-.70710678f,.70710678f},
            {-1,0},{-.70710678f,-.70710678f},{0,-1},{.70710678f,-.70710678f}};
        for (unsigned int i = 0; i < gameWorldCurrent->path.nodeCount && startTime < 0; ++i)
        {
            const float *node = gameWorldCurrent->path.nodes[i].constant.vOrigin;
            float from[3] = {node[0], node[1], node[2] + 8.0f};
            float to[3] = {node[0], node[1], node[2] - 64.0f};
            trace_t trace;
            col_context_t context;
            G_TraceCapsule(&trace, from, playerMins, playerMaxs, to, player->s.number, player->clipmask, &context);
            if (trace.startsolid || trace.fraction == 1.0f || trace.normal.vec.v[2] < 0.7f)
                continue;
            float ledge[3];
            Vec3Lerp(from, to, trace.fraction, ledge);
            ledge[2] += 0.125f;
            for (int d = 0; d < 8; ++d)
            {
                float above[3] = {ledge[0], ledge[1], ledge[2] + 39.0f};
                G_TraceCapsule(&trace, ledge, playerMins, playerMaxs, above, player->s.number, player->clipmask, &context);
                if (trace.startsolid || trace.fraction != 1.0f)
                    continue;
                float edge[3] = {ledge[0] + dirs[d][0] * 192.0f, ledge[1] + dirs[d][1] * 192.0f, above[2]};
                G_TraceCapsule(&trace, above, playerMins, playerMaxs, edge, player->s.number, player->clipmask, &context);
                if (trace.startsolid || trace.fraction != 1.0f)
                    continue;
                float down[3] = {edge[0], edge[1], edge[2] - 320.0f};
                G_TraceCapsule(&trace, edge, playerMins, playerMaxs, down, player->s.number, player->clipmask, &context);
                const float drop = trace.fraction * 320.0f;
                if (trace.startsolid || drop < 150.0f || drop > 300.0f || trace.normal.vec.v[2] < 0.7f)
                    continue;
                // Reject stair/ramp routes: a far lower endpoint alone does not establish a free fall.
                bool stairs = false;
                for (int distance = 32; distance < 192; distance += 32)
                {
                    float probe[3] = {ledge[0]+dirs[d][0]*distance, ledge[1]+dirs[d][1]*distance, above[2]};
                    float bottom[3] = {probe[0], probe[1], probe[2]-320.0f};
                    G_TraceCapsule(&trace, probe, playerMins, playerMaxs, bottom, player->s.number, player->clipmask, &context);
                    const float depth = trace.fraction*320.0f-39.0f;
                    if (trace.startsolid || (depth > 18.0f && depth < 128.0f))
                        stairs = true;
                }
                if (stairs)
                    continue;
                heading = d * 45.0f;
                ledgeHeight = ledge[2];
                ledgeXY[0] = ledge[0];
                ledgeXY[1] = ledge[1];
                if (dive)
                {
                    // L39 harness: the perk bit the machine's give_perk sets (the SetPerk method's BG_SetPerk on ps
                    // and sess perks, g_client_script_cmd_mp.cpp:7496); the script's MOD_FALLING rule
                    // (_zombiemode.gsc:4652) reads HasPerk + self.divetoprone.
                    const unsigned int perk = BG_GetPerkIndexForName("specialty_flakjacket");
                    BG_SetPerk(ps->perks, perk);
                    BG_SetPerk(player->client->sess.cs.perks, perk);
                }
                float angles[3] = {0, heading, 0};
                TeleportPlayer(player, ledge, angles);
                startTime = level.time;
                Com_Printf(16, "bo1_fall: setup node=%u ledge=%.3f,%.3f,%.3f yaw=%.0f lower=%.3f,%.3f,%.3f drop=%.3f health=%d\n",
                    i, ledge[0], ledge[1], ledge[2], heading, edge[0], edge[1], edge[2]-drop, drop, player->health);
                break;
            }
        }
        if (startTime < 0)
            Com_Printf(16, "bo1_fall: no collision-verified ledge found\n");
        return;
    }
    if (startTime < 0 || level.time - startTime > 6000 || landed)
        return;
    const int elapsed = level.time - startTime;
    cmd->angles[1] = (int)((heading - ps->delta_angles[1]) * 65536.0f / 360.0f) & 0xFFFF;
    if (elapsed >= 500)
    {
        cmd->forwardmove = 127;
        if (dive)
        {
            // L39: sprint (bit 1) along the 192-unit runway, then dive to prone (bit 8 ends the sprint on this
            // command, bit 44 = the dive request: Dtp_IsDtp bg_dtp.cpp:421, 250 ms dtp_startup_delay) after 300 ms of sprint or 96
            // before the edge, so the dive carries the player off the ledge (dive2: the floor ended ~115 units out).
            const float rx = ps->origin[0] - ledgeXY[0], ry = ps->origin[1] - ledgeXY[1];
            if (sprintStart < 0 && (ps->pm_flags & 0x8000))
                sprintStart = level.time;
            if (diveTime < 0 && sprintStart >= 0 && (level.time - sprintStart >= 300 || rx * rx + ry * ry >= 96.0f * 96.0f))
                diveTime = level.time;
            if (diveTime < 0)
                cmd->button_bits.setBit(1);
            else if (level.time - diveTime < 100)
            {
                cmd->button_bits.setBit(8);
                cmd->button_bits.setBit(44);
            }
        }
        else if ((elapsed - 500) % 1000 < 100)
            cmd->button_bits.setBit(10);
        if (ps->groundEntityNum == 1023)
            airborne = true;
        else if (airborne && ps->origin[2] < ledgeHeight - 100.0f)
        {
            landed = true;
            cmd->forwardmove = 0;
        }
    }
    Com_Printf(16, "bo1_fall: t=%d elapsed=%d o=%.3f,%.3f,%.3f v=%.3f,%.3f,%.3f ground=%d health=%d landed=%d dtp=%d sprint=%d\n",
        level.time, elapsed, ps->origin[0], ps->origin[1], ps->origin[2], ps->velocity[0], ps->velocity[1],
        ps->velocity[2], ps->groundEntityNum, player->health, landed ? 1 : 0, (ps->pm_flags & 0x400000) != 0,
        (ps->pm_flags & 0x8000) != 0);
}

bool G_SP_HeadlessMoveCommand(client_t *client, usercmd_s *cmd)
{
    G_SP_HeadlessMoveRegister();
    // w1 c12: or the headless local client of a -Client run (g_sp_client.cpp calls this for the test client number).
    if (!Sys_IsHeadless() || !G_SP_IsZombieMode() || (!client->bIsTestClient && !Sys_IsHeadlessClient())
        || (!g_headlessMove->current.enabled && !(g_headlessCrowdMove && g_headlessCrowdMove->current.enabled)
            && !(g_headlessFall && g_headlessFall->current.enabled)
            && !(g_headlessWeapon && g_headlessWeapon->current.enabled)))
        return false;
    gentity_s *player = client->gentity;
    *cmd = client->lastUsercmd;
    cmd->serverTime = svs.time;
    cmd->forwardmove = cmd->rightmove = 0;
    cmd->button_bits.array[0] = cmd->button_bits.array[1] = 0;
    if (client->header.state != CS_ACTIVE || !player || !player->client || player->health <= 0
        || player->client->sess.sessionState != SESS_STATE_PLAYING)
        return true;
    playerState_s *ps = &player->client->ps;
    // Hold a gun as a player would: the script's SwitchToWeapon request (the bot weapon), else the held weapon, else
    // the first weapon in the inventory (the test client's first usercmd selects none). Sprint time and move speed
    // come from the held weapon's file.
    const unsigned int requested = (unsigned int)botInfos[client - svs.clients].weapon;
    cmd->weapon = requested && BG_PlayerHasWeapon(ps, requested) ? requested : ps->weapon;
    for (int i = 0; !cmd->weapon && i < 15; ++i)
        cmd->weapon = ps->heldWeapons[i].weapon;
    // mod: L54 crowd coverage, ordinary usercmd movement on the private desktop only.
    if (g_headlessCrowdMove && g_headlessCrowdMove->current.enabled)
    {
        if (!(ps->pm_flags & 0x800))
        {
            cmd->angles[0] = (int)(-ps->delta_angles[0] * 65536.0f / 360.0f) & 0xFFFF;
            cmd->angles[1] = (int)((level.time * 0.018f - ps->delta_angles[1]) * 65536.0f / 360.0f) & 0xFFFF;
            cmd->forwardmove = 80;
            cmd->rightmove = (level.time / 5000) % 2 ? 40 : -40;
        }
        static int lastReport;
        if (level.time < lastReport || level.time - lastReport >= 5000)
        {
            lastReport = level.time;
            Com_Printf(16, "bo1_crowdmove: t=%d origin=%.1f,%.1f,%.1f yaw=%.1f fov=%g\n",
                level.time, ps->origin[0], ps->origin[1], ps->origin[2], ps->viewangles[1],
                Dvar_GetFloat("cg_fov"));
        }
        return true;
    }
    if (g_headlessWeapon && g_headlessWeapon->current.enabled)
    {
        // w1: the weapon cycle stands among round-1 zombies for minutes; bo1_testclient_god keeps the retail
        // god bit (Cmd_God_f's FL_GODMODE) here too, exactly as the fight harness does (g_sp_client.cpp).
        const dvar_t *god = Dvar_FindVar("bo1_testclient_god");
        if (god && god->current.enabled)
            player->flags |= 1;
        SP_HeadlessWeaponCommand(player, cmd);
        return true;
    }
    if (g_headlessFall && g_headlessFall->current.enabled)
    {
        SP_HeadlessFallCommand(player, cmd);
        return true;
    }
    // Five holds the player frozen (pm_flags 0x800) through the intro; start once the controls are free.
    if (g_moveStartTime < 0 && !(ps->pm_flags & 0x800))
    {
        g_moveStartTime = level.time + 1000;
        // Face world yaw 270 (-y): the longest free run from the Five test client's spawn (-498, 2582).
        g_moveBaseYaw = cmd->angles[1] + (int)((270.0f - ps->viewangles[1]) * 65536.0f / 360.0f);
        g_moveYawOffset = 0;
        g_moveLastTime = level.time;
    }
    const int t = level.time - g_moveStartTime;
    if (g_moveStartTime < 0 || t < 0)
        return true;
    const MovePhase *phase = nullptr;
    int phaseStart = 0;
    for (const MovePhase &p : g_movePhases)
    {
        if (t < phaseStart + p.durationMs)
        {
            phase = &p;
            break;
        }
        phaseStart += p.durationMs;
    }
    if (!phase)
        return true;
    const int dt = level.time - g_moveLastTime;
    g_moveLastTime = level.time;
    g_moveYawOffset += (int)(phase->yawRate * (float)dt * 65536.0f / 360000.0f);
    cmd->angles[0] = client->lastUsercmd.angles[0];
    cmd->angles[1] = (g_moveBaseYaw + g_moveYawOffset) & 0xFFFF;
    cmd->forwardmove = (char)phase->forward;
    cmd->rightmove = (char)phase->right;
    if (phase->stanceBit >= 0)
        cmd->button_bits.setBit(phase->stanceBit);
    if (phase->sprint)
        cmd->button_bits.setBit(1);
    if (phase->jumpPeriodMs > 0 && (t - phaseStart) % phase->jumpPeriodMs < 100)
        cmd->button_bits.setBit(10);
    // The permission phases write the state AllowStand/AllowCrouch/AllowProne write (SP_AllowPermission).
    static bool s_movePermSet;
    SPPlayerBuiltinState &spState = G_SP_PlayerBuiltinState(player->s.number);
    if (phase->disabled || s_movePermSet)
    {
        spState.disabledActions = (spState.disabledActions & ~0x1C00000u) | phase->disabled;
        s_movePermSet = phase->disabled != 0;
    }

    Com_Printf(16, "bo1_move: t=%d ph=%s ox=%.3f oy=%.3f oz=%.3f vx=%.3f vy=%.3f vz=%.3f g=%d vh=%.3f fm=%d rm=%d sp=%d j=%d"
        " spr=%d speed=%d pmf=0x%x wst=%d health=%d st=%d ss=%d se=%d sl=%d w=%d yaw=%.2f pdir=%.2f\n",
        level.time, phase->name, ps->origin[0], ps->origin[1], ps->origin[2], ps->velocity[0], ps->velocity[1],
        ps->velocity[2], ps->groundEntityNum, ps->viewHeightCurrent, phase->forward, phase->right, phase->sprint ? 1 : 0,
        cmd->button_bits.testBit(10) ? 1 : 0, (ps->pm_flags & 0x8000) ? 1 : 0, ps->speed, ps->pm_flags,
        ps->weaponstate, player->health, cmd->serverTime, ps->sprintState.lastSprintStart, ps->sprintState.lastSprintEnd,
        ps->sprintState.sprintStartMaxLength, ps->weapon, ps->viewangles[1], ps->proneDirection);
    return true;
}

// p1 TEST SWITCH bo1_feeltrace (registered by the headless local client, cg_sp_feeltrace.cpp): while a player is
// knifing or lunging, log the server's playerState each frame with the nearest live actor. Read only.
void G_SP_FeelTraceServerFrame()
{
    static const dvar_t *feelTrace;
    if (!Sys_IsHeadlessClient() || !G_SP_IsZombieMode())
        return;
    if (!feelTrace && !(feelTrace = Dvar_FindVar("bo1_feeltrace")))
        return;
    if (!feelTrace->current.enabled)
        return;
    for (int i = 0; i < level.maxclients && i < 32; ++i)
    {
        const gentity_s *player = &g_entities[i];
        if (!player->r.inuse || !player->client)
            continue;
        const playerState_s *ps = &player->client->ps;
        if (!(ps->weaponstate >= 17 && ps->weaponstate <= 19) && !ps->meleeChargeTime && !ps->meleeChargeDist)
            continue;
        int nearest = -1;
        float nearestDist = 1.0e9f;
        for (int e = 0; e < level.num_entities; ++e)
        {
            const gentity_s *ent = &g_entities[e];
            if (!ent->r.inuse || !ent->actor || ent->health <= 0)
                continue;
            const float d = Vec3Distance(ent->r.currentOrigin, ps->origin);
            if (d < nearestDist)
            {
                nearestDist = d;
                nearest = e;
            }
        }
        Com_Printf(16, "bo1_feel_sv: t=%d c=%d st=%d wt=%d wd=%d charge=%d,%d,%.4f o=%.3f,%.3f,%.3f v=%.3f,%.3f,%.3f"
            " g=%d near=%d,%.3f,%d\n",
            level.time, i, ps->weaponstate, ps->weaponTime, ps->weaponDelay, ps->meleeChargeDist, ps->meleeChargeTime,
            ps->meleeChargeYaw, ps->origin[0], ps->origin[1], ps->origin[2], ps->velocity[0], ps->velocity[1],
            ps->velocity[2], ps->groundEntityNum, nearest, nearestDist, nearest >= 0 ? g_entities[nearest].health : 0);
    }
}

// k1 c17 TEST (headless test client only): SP has one local client, and it runs every client-dvar server command
// ('v'): maps/_zombiemode::onPlayerConnect_clientDvars -> SetClientDvars (SP G_m_setclientdvars 0x007d8620, the same
// "v <name> \"<value>\" ..." string as KB PlayerCmd_SetClientDvars) -> server command -> CG 'v' ->
// CG_SetClientDvarFromServer -> Dvar_SetFromStringByName. Some of those dvars are read by the server side in the same
// process: playerPushAmount "1" makes PM_UpdatePush (gjk_sentient_push) push the player away from AIs; the retail
// session's pool has it at 1 (retail research extracted/exe/dvars-live.json) while G_InitGame's Dvar_ResetDvars(0x1000)
// puts it back to its registered 0 each map. KB drops a test client's reliable commands (SV_AddServerCommand), so the
// headless player was never pushed: after the recorded down a zombie walking into the prone player climbed onto it
// (z 16 -> 46) instead of pushing it along (session2 rec 71250-74350: the recorded player moves 16 u/s with zero
// velocity and no input, 796 staying 32 u from it). The cg-only names CG_SetClientDvarFromServer keeps for itself are
// skipped (no cgame in a headless run).
void G_SP_HeadlessTestClientServerCommand(const client_t *client, const char *cmd)
{
    if (!client || !client->bIsTestClient || !cmd || !Sys_IsHeadless() || !G_SP_IsZombieMode())
        return;
    // a1 c9: G_SelectWeaponIndex's 'a <index>' (script SwitchToWeapon, the wall buy, last stand) is the local client's
    // weapon selection (CG 'a' -> CG_SelectWeaponIndex); the replayed test client keeps it as its cmd weapon.
    if (cmd[0] == 'a' && cmd[1] == ' ')
    {
        G_SP_HeadlessReplaySelectWeapon(atoi(cmd + 2));
        return;
    }
    if (cmd[0] != 'v' || (cmd[1] && cmd[1] != ' '))
        return;
    char name[150], value[1024];
    const char *p = cmd + 1;
    for (;;)
    {
        while (*p == ' ')
            ++p;
        if (!*p)
            return;
        unsigned int n = 0;
        while (*p && *p != ' ')
        {
            if (n + 1 < sizeof(name))
                name[n++] = *p;
            ++p;
        }
        name[n] = 0;
        while (*p == ' ')
            ++p;
        n = 0;
        if (*p == '"')
        {
            for (++p; *p && *p != '"'; ++p)
                if (n + 1 < sizeof(value))
                    value[n++] = *p;
            if (*p == '"')
                ++p;
        }
        else
        {
            for (; *p && *p != ' '; ++p)
                if (n + 1 < sizeof(value))
                    value[n++] = *p;
        }
        value[n] = 0;
        if (I_stricmp(name, "cg_objectiveText") && I_stricmp(name, "hud_drawHud") && I_stricmp(name, "g_scriptMainMenu"))
            Dvar_SetFromStringByName(name, value);
    }
}
