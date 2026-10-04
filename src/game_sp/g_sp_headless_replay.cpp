// k1: headless player replay (TEST SWITCH bo1_testclient_replay, harness only). The zombie test client plays a
// retail recording's own usercmds (retail research extracted/probe/sessions/<name>/cmds_raw.tsv, decoded and exported by
// tools/k1_replay_export.mjs) so the zombies chase the recorded player and their paths can be compared with the
// recording's (tools/measure_five.mjs). Every recorded cmd is run through SV_ClientThink at its own serverTime
// (the recording's clients sent one per 8 ms; the test client otherwise sends one per server frame), with the
// recorded forward/right move, world view angles and stance/sprint/jump buttons. Pmove, collision, scripts and
// damage decide the rest. Drift: at the start of each server frame the playerState is compared with the recorded
// predicted playerState at the same commandTime; beyond bo1_testclient_replay_snap units its origin and velocity
// are set to the recorded ones (counted and logged per frame as "bo1_replay:"), so the zombies see the recorded
// player. The recording has no fire / ADS / melee input in session2, so none is sent.
//
// Clocks: the recording's level time is commandTime + clockOffset (level.tsv); the replay puts the recording's
// round-1 first spawn (16900 ms in session2) on KB's, the same first-spawn alignment measure_five.mjs uses for the paths.
// k1 chunk 12: KB's first spawn is not a constant (10350 in k1-c9, 10400 in k1-c10 / k1-c12), and the old fixed 10300
// put the replayed player 100 ms ahead of the zombies chasing it (796 turned two frames early at rec 57900). The
// align is now taken at run time from the level notify "begin_spawning" (maps/_zombiemode.gsc flag_set), which leads
// the round-1 first spawn by a script-fixed 8250 ms (MEASURED: 4 of 4 KB runs, k1-c9-c1x, k1-c9-measure, k1-c10-a,
// k1-c12-a). bo1_testclient_replay_align > 0 still forces a fixed value.
#include "g_sp_headless_move.h"
#include <Windows.h>
#include "g_sp_levelstart.h"
#include <client_mp/g_client_mp.h>
#include <game_mp/g_main_mp.h>
#include <client_mp/sv_client_mp.h>
#include <server_mp/sv_main_mp.h>
#include <win32/win_main.h>
#include <universal/dvar.h>
#include <qcommon/common.h>
#include <bgame/bg_misc.h>
#include <bgame/bg_local.h>
#include <bgame/bg_weapons_def.h>
#include <universal/com_math.h>
#include <stdio.h>
#include <vector>

struct ReplayCmd
{
    int t;              // recording commandTime
    int forward, right;
    unsigned int buttons; // the recorded usercmd's first button word (same bit order as KB's bitarray word 0)
    float angles[3];    // world view angles
    int weapon;         // a1 chunk 4: the recording's SP weapon index (0 = not exported); the local route only
    unsigned int buttons1; // a1 chunk 4: the second button word (bit 44 = dive to prone); the local route only
    int gun[2];         // a1 chunk 10: the recorded cmd's gun angles (SP usercmd +0x20 / +0x22 shorts)
    bool hasGun;
    int snap;           // a1 chunk 12: the level.time (recording clock) the cmd ran at in SP, from the 's' lines; 0 = unknown
};

struct ReplayPs
{
    int t;
    float origin[3], velocity[3];
    float viewheight;
};

static const dvar_t *g_replayPath;
static const dvar_t *g_replayAlign;
static const dvar_t *g_replaySnap;
static const dvar_t *g_replayGod;
static const dvar_t *g_replayGodEnd;
static const dvar_t *g_replayWeapons;
static const dvar_t *g_replayButtons;
static bool s_replayGodSet;
static bool s_replayLoaded, s_replayFailed;
static int s_replayClockOffset, s_replayFirstSpawn, s_replayDown, s_replayEnd;
static std::vector<ReplayCmd> s_replayCmds;
static std::vector<ReplayPs> s_replayPs;
static std::vector<std::pair<int, int>> s_replaySnapClock; // a1 chunk 12: level.tsv (commandTime, snapServerTime) samples
static size_t s_replayNext;
static int s_replaySnaps, s_replayFrames;
static int s_replayAlignLevel = -1; // KB level time of the round-1 first spawn, from "begin_spawning" (-1 = not seen)
static const int REPLAY_BEGIN_SPAWNING_TO_FIRST_SPAWN = 8250; // MEASURED, see the clock note above
static int SP_ReplayAlign();

// SP/MP button indices replayed: 1 sprint, 8 prone, 9 crouch, 10 jump, 12 stance key held (PM_UpdateStance
// only uses it to skip the "can't stand" event). session2 also holds index 13 with sprint and one index 29 tap;
// they are not movement and are left out.
static const unsigned int REPLAY_BUTTON_MASK = (0x80000000u >> 1) | (0x80000000u >> 8) | (0x80000000u >> 9)
    | (0x80000000u >> 10) | (0x80000000u >> 12);
// The weapon buttons the -Client route (and the bot route with bo1_testclient_replay_buttons) adds; see below.
static const unsigned int REPLAY_LOCAL_BUTTON_MASK = REPLAY_BUTTON_MASK | 0x80000000u | (0x80000000u >> 2)
    | (0x80000000u >> 3) | (0x80000000u >> 4) | (0x80000000u >> 11) | (0x80000000u >> 13);

static void SP_HeadlessReplayRegister()
{
    if (g_replayPath || !Sys_IsHeadless())
        return;
    g_replayPath = _Dvar_RegisterString("bo1_testclient_replay", "", 0,
        "TEST SWITCH (headless test client only): replay this exported recording's usercmds (tools/k1_replay_export.mjs)");
    g_replayAlign = _Dvar_RegisterInt("bo1_testclient_replay_align", 0, 0, 0x7FFFFFFF, 0,
        "TEST SWITCH: KB level time the recording's first round-1 spawn is aligned to (ms); 0 = begin_spawning + 8250");
    g_replaySnap = _Dvar_RegisterFloat("bo1_testclient_replay_snap", 4.0f, 0.0f, 100000.0f, 0,
        "TEST SWITCH: drift (units) beyond which the replayed player is put back on the recorded state");
    g_replayGod = _Dvar_RegisterBool("bo1_testclient_replay_god", true, 0,
        "TEST SWITCH: the replayed player keeps the retail god-mode bit until the recording's own down (then clears it)");
    g_replayGodEnd = _Dvar_RegisterInt("bo1_testclient_replay_godend", 0, 0, 0x7FFFFFFF, 0,
        "TEST SWITCH: recording level time (ms) the god-mode bit is cleared at; 0 = the recording's own down");
    g_replayWeapons = _Dvar_RegisterString("bo1_testclient_replay_weapons", "", 0,
        "TEST SWITCH (-Client replay): '<SP weapon index>:<weapon name>,...' - the recording's weapon switches to replay");
    g_replayButtons = _Dvar_RegisterBool("bo1_testclient_replay_buttons", false, 0,
        "TEST SWITCH (bot replay): also send the recording's weapon buttons, dive-to-prone and weapon switches");
}

// a1 chunk 12: the level.time each recorded cmd ran at in SP. SP's server thread (Com_ServerLoop 0x0087E2A0) takes a
// client packet (its cmds run at the current level.time) and only then runs the due G_RunFrame(svs.time), so a cmd runs
// at the level.time of the snapshot the client held when it built it: level.tsv's snapServerTime at that commandTime
// (100 Hz samples; the cmds are 125 Hz). A cmd between two samples on different snapshots takes the nearer sample's
// (APPROXIMATED: the frame change's own moment is not recorded). The old rule (commandTime + the median clockOffset 35,
// run before the frame at or after it) ran session4's shots 1, 2 and 4 one frame early: shot 1 (cmd 39894, snap
// 39950) broke its glass in frame 40000, shot 2 (40047, snap 40100) in 40150, one frame after the damage frame.
// a1 chunk 13: the held snapshot is only a lower bound - SP's server is often a frame past it when the cmd is built.
// tools/k1_replay_export.mjs --snapclock now writes one 's' line per cmd with its MEASURED level (max(held snapshot,
// floor((ct + 65) / 50) * 50)); session4 shots 3 and 5 ran one frame after their snapshot (see the exporter).
static int SP_ReplaySnapClock()
{
    int count = 0, prev = 0;
    const size_t n = s_replaySnapClock.size();
    for (ReplayCmd &c : s_replayCmds)
    {
        c.snap = 0;
        if (!n)
            continue;
        size_t lo = 0, hi = n; // first sample with commandTime >= c.t
        while (lo < hi)
        {
            const size_t mid = (lo + hi) / 2;
            if (s_replaySnapClock[mid].first < c.t)
                lo = mid + 1;
            else
                hi = mid;
        }
        const std::pair<int, int> *after = lo < n ? &s_replaySnapClock[lo] : nullptr;
        const std::pair<int, int> *before = lo ? &s_replaySnapClock[lo - 1] : nullptr;
        int snap = 0;
        if (after && after->first == c.t)
            snap = after->second;
        else if (before && after && after->first - before->first <= 50)
            snap = before->second == after->second || c.t - before->first < after->first - c.t ? before->second : after->second;
        if (snap - c.t < 0 || snap - c.t > 100)
            snap = 0;
        if (snap && snap < prev)
            snap = prev;
        if (snap)
        {
            c.snap = snap;
            prev = snap;
            ++count;
        }
    }
    return count;
}

static bool SP_HeadlessReplayLoad()
{
    if (s_replayLoaded || s_replayFailed)
        return s_replayLoaded;
    FILE *f = fopen(g_replayPath->current.string, "rb");
    if (!f)
    {
        s_replayFailed = true;
        Com_Printf(16, "bo1_replay: cannot open '%s'\n", g_replayPath->current.string);
        return false;
    }
    char line[512];
    bool clock = false;
    while (fgets(line, sizeof(line), f))
    {
        if (line[0] == 'c' && line[1] == ' ')
        {
            ReplayCmd c = {};
            const int n = sscanf(line + 2, "%d %d %d %u %f %f %f %d %u %d %d", &c.t, &c.forward, &c.right, &c.buttons,
                &c.angles[0], &c.angles[1], &c.angles[2], &c.weapon, &c.buttons1, &c.gun[0], &c.gun[1]);
            c.hasGun = n == 11;
            if (n >= 7)
                s_replayCmds.push_back(c);
        }
        else if (line[0] == 'p' && line[1] == ' ')
        {
            ReplayPs p = {};
            if (sscanf(line + 2, "%d %f %f %f %f %f %f %f", &p.t, &p.origin[0], &p.origin[1], &p.origin[2],
                &p.velocity[0], &p.velocity[1], &p.velocity[2], &p.viewheight) == 8)
                s_replayPs.push_back(p);
        }
        else if (line[0] == 's' && line[1] == ' ')
        {
            int ct, st;
            if (sscanf(line + 2, "%d %d", &ct, &st) == 2)
                s_replaySnapClock.emplace_back(ct, st);
        }
        else if (!strncmp(line, "clock ", 6))
            clock = sscanf(line + 6, "%d %d %d %d", &s_replayClockOffset, &s_replayFirstSpawn, &s_replayDown,
                &s_replayEnd) == 4;
    }
    fclose(f);
    const int snapped = SP_ReplaySnapClock();
    s_replayLoaded = clock && !s_replayCmds.empty() && !s_replayPs.empty();
    s_replayFailed = !s_replayLoaded;
    Com_Printf(16, "bo1_replay: loaded '%s' cmds %u ps %u clockOffset %d firstSpawn %d down %d end %d align %d ok %d snapclock %u/%d\n",
        g_replayPath->current.string, (unsigned int)s_replayCmds.size(), (unsigned int)s_replayPs.size(),
        s_replayClockOffset, s_replayFirstSpawn, s_replayDown, s_replayEnd, SP_ReplayAlign(),
        s_replayLoaded ? 1 : 0, (unsigned int)s_replaySnapClock.size(), snapped);
    return s_replayLoaded;
}

// recording commandTime -> KB level time (svs.time and level.time are one clock: G_RunFrame(svs.time))
static int SP_ReplayAlign()
{
    if (g_replayAlign->current.integer > 0)
        return g_replayAlign->current.integer;
    return s_replayAlignLevel >= 0 ? s_replayAlignLevel : 10400; // not reached: begin_spawning is at ~2150
}

static int SP_ReplayToLevel(int t)
{
    return t + s_replayClockOffset - s_replayFirstSpawn + SP_ReplayAlign();
}

// The server frame (svs.time) whose bot update runs the recorded cmd: the frame after the level.time it ran at in SP
// (SP_ReplaySnapClock), or without that data the first frame at or after SP_ReplayToLevel(t).
static int SP_ReplayRunAt(const ReplayCmd &c)
{
    if (c.snap)
        return c.snap - s_replayFirstSpawn + SP_ReplayAlign() + 50;
    return SP_ReplayToLevel(c.t);
}

void G_SP_HeadlessReplayLevelNotify(const char *name)
{
    SP_HeadlessReplayRegister();
    if (!g_replayPath || !*g_replayPath->current.string || strcmp(name, "begin_spawning"))
        return;
    s_replayAlignLevel = level.time + REPLAY_BEGIN_SPAWNING_TO_FIRST_SPAWN;
    Com_Printf(16, "bo1_replay: begin_spawning at %d, recording first spawn aligned to %d (align dvar %d)\n",
        level.time, s_replayAlignLevel, g_replayAlign->current.integer);
}

// The recorded predicted playerState at recording commandTime t (linear between samples up to 40 ms apart).
static bool SP_ReplayStateAt(int t, float *origin, float *velocity, float *viewheight)
{
    size_t lo = 0, hi = s_replayPs.size();
    while (lo < hi)
    {
        const size_t mid = (lo + hi) / 2;
        if (s_replayPs[mid].t < t)
            lo = mid + 1;
        else
            hi = mid;
    }
    if (lo < s_replayPs.size() && s_replayPs[lo].t == t)
    {
        Vec3Copy(s_replayPs[lo].origin, origin);
        Vec3Copy(s_replayPs[lo].velocity, velocity);
        *viewheight = s_replayPs[lo].viewheight;
        return true;
    }
    if (lo == 0 || lo >= s_replayPs.size())
        return false;
    const ReplayPs &a = s_replayPs[lo - 1], &b = s_replayPs[lo];
    if (b.t - a.t > 40)
        return false;
    const float frac = (float)(t - a.t) / (float)(b.t - a.t);
    Vec3Lerp(a.origin, b.origin, frac, origin);
    Vec3Lerp(a.velocity, b.velocity, frac, velocity);
    *viewheight = a.viewheight + (b.viewheight - a.viewheight) * frac;
    return true;
}

static void SP_ReplayLocalWeaponMap();
static int SP_ReplayLocalWeapon(int sp);

// a1 chunk 9: the bot route's weapon selection (bo1_testclient_replay_buttons), kept like a client's cmd weapon
// (cgameUserCmdWeapon): the script's selects ('a' server command, G_SP_HeadlessReplaySelectWeapon) and the recording's
// weapon changes (the -Client route's rule: the mapped weapon once the playerState holds it, within 1 s) set it; a
// weapon the player no longer holds falls back to ps->weapon. It must stay in the cmd while PM_Weapon drops the old
// weapon (ps->weapon changes only at PM_FinishWeaponChange), which the frame-start ps->weapon copy would undo.
static int s_botRecWeapon = -1, s_botPendingWeapon, s_botPendingUntil, s_botSelWeapon;
static bool s_botWeaponMap;
// a1 chunk 9: the spread seed. SP FireWeapon 0x004CFC90 (called from ClientEvents 0x00551F50 with the client's last
// cmd serverTime while g_antilag is on, the retail pool's value) passes that time to FireBullets 0x004625C0, which seeds
// each pellet's Bullet_Endpos with shotIndex + time - KB's Bullet_Fire does the same. The replayed cmds run at
// SP_ReplayToLevel(recorded serverTime), so without this every recorded shot would draw another spread direction.
int G_SP_HeadlessReplayShotTime(const gentity_s *attacker, int gameTime)
{
    if (!g_replayButtons || !g_replayButtons->current.enabled || !s_replayLoaded || !attacker || !attacker->client
        || attacker->s.number < 0 || attacker->s.number >= 64 || !svs.clients[attacker->s.number].bIsTestClient)
        return gameTime;
    return gameTime - SP_ReplayToLevel(0);
}

void G_SP_HeadlessReplaySelectWeapon(int weapon)
{
    s_botSelWeapon = weapon;
    s_botPendingWeapon = 0;
}

static void SP_ReplayBotWeapon(const ReplayCmd &rc, const playerState_s *ps, int serverTime, usercmd_s *cmd)
{
    if (!g_replayButtons || !g_replayButtons->current.enabled)
        return;
    if (!s_botWeaponMap)
    {
        SP_ReplayLocalWeaponMap();
        s_botWeaponMap = true;
    }
    if (rc.weapon && rc.weapon != s_botRecWeapon)
    {
        if (s_botRecWeapon >= 0)
        {
            s_botPendingWeapon = SP_ReplayLocalWeapon(rc.weapon);
            s_botPendingUntil = serverTime + 1000;
        }
        s_botRecWeapon = rc.weapon;
    }
    if (s_botPendingWeapon && serverTime > s_botPendingUntil)
        s_botPendingWeapon = 0;
    if (s_botPendingWeapon && BG_PlayerHasWeapon(ps, s_botPendingWeapon))
    {
        s_botSelWeapon = s_botPendingWeapon;
        s_botPendingWeapon = 0;
    }
    if (!s_botSelWeapon || !BG_PlayerHasWeapon(ps, s_botSelWeapon))
        s_botSelWeapon = ps->weapon;
    if (s_botSelWeapon)
        cmd->weapon = s_botSelWeapon;
}

// a1 chunk 10: the replayed cmds' gun angles by cmd serverTime (bo1_testclient_replay_buttons). SP ClientThink_real
// 0x0069D450 sets client fGunPitch / fGunYaw = SHORT2ANGLE(usercmd +0x20 / +0x22) after ClientEvents: the client's own
// gun angles (cg view + weapon sway / bob / its recoil kick), which the NEXT shot fires along (FireWeapon 0x004CFC90 ->
// Weapon_GetThrowInfo 0x00670B70: fGunPitch / fGunYaw unless player_topDownCamMode). KB's usercmd_s (MP) has no gun
// angles and ClientThink_real builds them on the server from G_random recoil draws (BG_WeaponFireRecoil via
// G_PlayerEvent), which differ run to run and from the recording.
struct ReplayGun { int serverTime; int gun[2]; };
static ReplayGun s_replayGun[32];
static unsigned int s_replayGunNext;

bool G_SP_HeadlessReplayGunAngles(const gentity_s *ent, int cmdServerTime, float *pitch, float *yaw)
{
    if (!g_replayButtons || !g_replayButtons->current.enabled || !s_replayLoaded || !ent || !ent->client
        || ent->s.number < 0 || ent->s.number >= 64 || !svs.clients[ent->s.number].bIsTestClient)
        return false;
    for (const ReplayGun &g : s_replayGun)
    {
        if (g.serverTime != cmdServerTime || !cmdServerTime)
            continue;
        *pitch = (float)(short)g.gun[0] * 0.0054931641f;
        *yaw = (float)(short)g.gun[1] * 0.0054931641f;
        return true;
    }
    return false;
}

static void SP_ReplayFillCmd(const ReplayCmd &rc, const playerState_s *ps, int serverTime, usercmd_s *cmd)
{
    if (rc.hasGun)
    {
        ReplayGun &g = s_replayGun[s_replayGunNext++ % (sizeof(s_replayGun) / sizeof(s_replayGun[0]))];
        g.serverTime = serverTime;
        g.gun[0] = rc.gun[0];
        g.gun[1] = rc.gun[1];
    }
    cmd->serverTime = serverTime;
    cmd->forwardmove = (char)rc.forward;
    cmd->rightmove = (char)rc.right;
    // a1 chunk 9: bo1_testclient_replay_buttons (session4 parity: the user shoots, knifes, buys) sends the -Client
    // route's buttons from the bot route too; each recorded cmd runs at its own serverTime here, so no OR-ing.
    const bool all = g_replayButtons && g_replayButtons->current.enabled;
    cmd->button_bits.array[0] = rc.buttons & (all ? REPLAY_LOCAL_BUTTON_MASK : REPLAY_BUTTON_MASK);
    cmd->button_bits.array[1] = all ? rc.buttons1 & (0x80000000u >> (44 - 32)) : 0;
    SP_ReplayBotWeapon(rc, ps, serverTime, cmd);
    for (int i = 0; i < 3; ++i)
        cmd->angles[i] = (int)((rc.angles[i] - ps->delta_angles[i]) * 65536.0f / 360.0f) & 0xFFFF;
}

static bool SP_ReplayPlayerPlaying(const client_t *client)
{
    const gentity_s *player = client->gentity;
    return client->header.state == CS_ACTIVE && player && player->client && player->health > 0
        && player->client->sess.sessionState == SESS_STATE_PLAYING;
}

static void SP_ReplayGodBit(gentity_s *player)
{
    // k1 chunk 8: the recorded player was not downed before the recording's down (session2: 70231 = KB 63631), but
    // KB's zombies can reach the replayed player earlier while their paths still differ; the retail scripts then
    // end the solo game (player_damage_override -> end_game) and zombie_game_over_death kills every zombie with
    // DoDamage(health + 666), which cut every track at ~47-52 s. Up to the recorded down the replayed player keeps
    // FL_GODMODE, the bit the retail `god` cheat sets (Cmd_God_f; G_Damage returns for it before the damage callback,
    // SP 0x00540916), so the zombie tracks run to the compare window. The bit is cleared once, at the recorded down.
    // k1 c17: bo1_testclient_replay_godend moves the clear earlier, so the zombies' last hits before the recorded
    // down can land (the recording's player took them; god absorbed them and put KB's down 1.27 s late in seed 2).
    const int godEnd = g_replayGodEnd->current.integer > 0 ? g_replayGodEnd->current.integer : s_replayDown;
    if (g_replayGod->current.enabled && s_replayDown >= 0)
    {
        if (svs.time < SP_ReplayToLevel(godEnd - s_replayClockOffset))
        {
            player->flags |= 1;
            s_replayGodSet = true;
        }
        else if (s_replayGodSet)
        {
            player->flags &= ~1;
            s_replayGodSet = false;
            Com_Printf(16, "bo1_replay: recorded down reached at %d, god bit cleared\n", svs.time);
        }
    }
}

// Drift check against the recorded state at the commandTime the playerState has reached.
static int SP_ReplayDrift(playerState_s *ps, float *err, bool *snapped, float *recViewheight)
{
    const int recNow = ps->commandTime - SP_ReplayToLevel(0);
    float recOrigin[3], recVelocity[3];
    *recViewheight = 0.0f;
    *err = -1.0f;
    *snapped = false;
    if (recNow <= s_replayEnd - s_replayClockOffset && SP_ReplayStateAt(recNow, recOrigin, recVelocity, recViewheight))
    {
        *err = Vec3Distance(ps->origin, recOrigin);
        if (*err > g_replaySnap->current.value)
        {
            Vec3Copy(recOrigin, ps->origin);
            Vec3Copy(recVelocity, ps->velocity);
            *snapped = true;
            ++s_replaySnaps;
        }
    }
    ++s_replayFrames;
    return recNow;
}

bool G_SP_HeadlessReplayCommand(client_t *client, usercmd_s *cmd)
{
    SP_HeadlessReplayRegister();
    if (!g_replayPath || !*g_replayPath->current.string || !G_SP_IsZombieMode() || !client->bIsTestClient)
        return false;
    if (!SP_HeadlessReplayLoad())
        return false;
    gentity_s *player = client->gentity;
    *cmd = client->lastUsercmd;
    cmd->serverTime = svs.time;
    cmd->forwardmove = cmd->rightmove = 0;
    cmd->button_bits.array[0] = cmd->button_bits.array[1] = 0;
    if (!SP_ReplayPlayerPlaying(client))
        return true;
    playerState_s *ps = &player->client->ps;
    cmd->weapon = ps->weapon;
    for (int i = 0; !cmd->weapon && i < 15; ++i)
        cmd->weapon = ps->heldWeapons[i].weapon;
    SP_ReplayGodBit(player);

    // Before the recording's first cmd: stand facing the recorded start.
    if (svs.time < SP_ReplayToLevel(s_replayCmds.front().t))
    {
        SP_ReplayFillCmd(s_replayCmds.front(), ps, svs.time, cmd);
        cmd->forwardmove = cmd->rightmove = 0;
        cmd->button_bits.array[0] = 0;
        return true;
    }

    float err, recViewheight;
    bool snapped;
    const int recNow = SP_ReplayDrift(ps, &err, &snapped, &recViewheight);

    // Every recorded cmd up to this server time; the last one is left in *cmd for SV_UpdateBots' SV_ClientThink.
    int ran = 0;
    const ReplayCmd *last = nullptr;
    while (s_replayNext < s_replayCmds.size() && SP_ReplayRunAt(s_replayCmds[s_replayNext]) <= svs.time)
    {
        const ReplayCmd &rc = s_replayCmds[s_replayNext++];
        const int serverTime = SP_ReplayToLevel(rc.t);
        if (serverTime <= ps->commandTime)
            continue;
        if (last)
        {
            usercmd_s run = *cmd;
            SP_ReplayFillCmd(*last, ps, SP_ReplayToLevel(last->t), &run);
            SV_ClientThink(client, &run);
            ++ran;
        }
        last = &rc;
    }
    if (last)
    {
        SP_ReplayFillCmd(*last, ps, SP_ReplayToLevel(last->t), cmd);
        ++ran;
    }
    else if (s_replayNext > 0 && s_replayNext < s_replayCmds.size())
    {
        // A gap in the recorded cmd ring (up to 161 ms in session2): hold the previous cmd, stopping short of
        // the next recorded one so it still runs.
        const int nextTime = SP_ReplayToLevel(s_replayCmds[s_replayNext].t);
        SP_ReplayFillCmd(s_replayCmds[s_replayNext - 1], ps, svs.time < nextTime ? svs.time : nextTime - 1, cmd);
        // a1 c10 APPROXIMATED: the ring gap's unrecorded cmds carried their own gun angles, and the next recorded cmd
        // fires along the last of them (SP sets fGunPitch / fGunYaw after ClientEvents). The next recorded cmd's gun
        // angles (8 ms later) stand in for them, not the held cmd's (session4's first shot at 39927 followed a 143 ms
        // gap: the held 39784 still carried the previous shot's kick, 1.2 deg off).
        const ReplayCmd &next = s_replayCmds[s_replayNext];
        ReplayGun &g = s_replayGun[(s_replayGunNext - 1) % (sizeof(s_replayGun) / sizeof(s_replayGun[0]))];
        if (next.hasGun && s_replayCmds[s_replayNext - 1].hasGun)
        {
            g.gun[0] = next.gun[0];
            g.gun[1] = next.gun[1];
        }
    }
    else
    {
        // After the recording: stand still with the last view.
        SP_ReplayFillCmd(s_replayCmds.back(), ps, svs.time, cmd);
        cmd->forwardmove = cmd->rightmove = 0;
        cmd->button_bits.array[0] &= (0x80000000u >> 8) | (0x80000000u >> 9);
    }
    Com_Printf(16, "bo1_replay: t=%d ct=%d rec=%d cmds=%d err=%.2f snap=%d snaps=%d/%d o=%.1f,%.1f,%.1f vh=%.1f rvh=%.1f pmf=0x%x hp=%d\n",
        svs.time, ps->commandTime, recNow, ran, err, snapped ? 1 : 0, s_replaySnaps, s_replayFrames, ps->origin[0],
        ps->origin[1], ps->origin[2], ps->viewHeightCurrent, recViewheight, ps->pm_flags, player->health);
    return true;
}

// a1 chunk 4: the replay through a -Client run's LOCAL client, so the player's own client-side sounds (footsteps,
// sprint, land, weapon, hurt: all cgame, local client only) play for the recorded session. The server frame keeps
// the bot route's god bit and drift snap for the local player (G_SP_HeadlessReplayLocalFrame); the client thread's
// CL_CreateCmd takes the recorded cmd at its own serverTime (G_SP_HeadlessReplayLocalCommand), so the usercmds
// reach the server through the ordinary client path (one per client frame: 8 ms at -ClientMaxFps 125, the
// recording's rate). Buttons of every recorded cmd since the previous local cmd are ORed in, so a one-cmd tap is not
// lost to a frame boundary. The local route also sends the recording's weapon buttons (the bot route does not):
// 0 attack, 2 melee, 3 use, 4 reload, 11 ADS, 13 hold breath (the sprint key in session4) - the button bits of
// CL_CmdButtons (cl_input_mp.cpp). Weapon switches: the recorded weapon index is SP's; bo1_testclient_replay_weapons
// names them ("3:m1911_zm,38:rottweil72_zm" for session4 - MEASURED from its sounds: 3 fires wpn_colt45_fire_plr, 38
// appears with the rottweil wall buy's cha_ching / wpn_rottweil72_1straise_plr). A change of the recorded weapon is a
// weapon key press: it selects the mapped weapon once the predicted playerState holds it (within 1 s), and never
// fights a script SwitchToWeapon in between. From the second button word only bit 44 is sent: the dive-to-prone
// press (CL_AddCurrentStanceToCmd's CL_STANCE_DIVE_TO_PRONE; Dtp_IsDtp tests it; session4 holds it for 2 cmds per dive).
static SRWLOCK s_localLock = SRWLOCK_INIT;
static bool s_localValid, s_localPlaying;
static int s_localToLevel0;
static float s_localDelta[3];
static int s_localLastServerTime = -1; // client thread only
static int s_localWeaponSp[16], s_localWeaponKb[16], s_localWeaponCount; // set once before s_localValid
static int s_localRecWeapon = -1, s_localPendingWeapon, s_localPendingUntil; // client thread only

static void SP_ReplayLocalWeaponMap()
{
    char name[64];
    int sp, used = 0;
    const char *p = g_replayWeapons->current.string;
    while (s_localWeaponCount < 16 && sscanf(p, " %d:%63[^,]%n", &sp, name, &used) == 2)
    {
        p += used;
        if (*p == ',')
            ++p;
        const int kb = BG_FindWeaponIndexForName(name);
        Com_Printf(16, "bo1_replay: local weapon map %d -> %s (%d)\n", sp, name, kb);
        if (kb)
        {
            s_localWeaponSp[s_localWeaponCount] = sp;
            s_localWeaponKb[s_localWeaponCount++] = kb;
        }
    }
}

static int SP_ReplayLocalWeapon(int sp)
{
    for (int i = 0; i < s_localWeaponCount; ++i)
        if (s_localWeaponSp[i] == sp)
            return s_localWeaponKb[i];
    return 0;
}

bool G_SP_HeadlessReplayLocalFrame(client_t *client)
{
    SP_HeadlessReplayRegister();
    if (!g_replayPath || !*g_replayPath->current.string || !G_SP_IsZombieMode() || client->bIsTestClient)
        return false;
    if (!SP_HeadlessReplayLoad())
        return false;
    gentity_s *player = client->gentity;
    const bool playing = SP_ReplayPlayerPlaying(client);
    float err = -1.0f, recViewheight = 0.0f;
    bool snapped = false;
    int recNow = 0;
    if (playing)
    {
        playerState_s *ps = &player->client->ps;
        SP_ReplayGodBit(player);
        if (ps->commandTime >= SP_ReplayToLevel(s_replayCmds.front().t))
            recNow = SP_ReplayDrift(ps, &err, &snapped, &recViewheight);
    }
    AcquireSRWLockExclusive(&s_localLock);
    if (!s_localValid)
        SP_ReplayLocalWeaponMap();
    s_localValid = true;
    s_localPlaying = playing;
    s_localToLevel0 = SP_ReplayToLevel(0);
    if (playing)
        Vec3Copy(player->client->ps.delta_angles, s_localDelta);
    ReleaseSRWLockExclusive(&s_localLock);
    if (playing)
    {
        const playerState_s *ps = &player->client->ps;
        Com_Printf(16, "bo1_replay: local t=%d ct=%d rec=%d err=%.2f snap=%d snaps=%d/%d o=%.1f,%.1f,%.1f vh=%.1f rvh=%.1f pmf=0x%x hp=%d\n",
            svs.time, ps->commandTime, recNow, err, snapped ? 1 : 0, s_replaySnaps, s_replayFrames, ps->origin[0],
            ps->origin[1], ps->origin[2], ps->viewHeightCurrent, recViewheight, ps->pm_flags, player->health);
    }
    return true;
}

// Index of the first recorded cmd after recording time t.
static size_t SP_ReplayUpperBound(int t)
{
    size_t lo = 0, hi = s_replayCmds.size();
    while (lo < hi)
    {
        const size_t mid = (lo + hi) / 2;
        if (s_replayCmds[mid].t <= t)
            lo = mid + 1;
        else
            hi = mid;
    }
    return lo;
}

bool G_SP_HeadlessReplayLocalCommand(usercmd_s *cmd, const playerState_s *predictedPs)
{
    AcquireSRWLockShared(&s_localLock);
    const bool valid = s_localValid, playing = s_localPlaying;
    const int toLevel0 = s_localToLevel0;
    float delta[3];
    Vec3Copy(s_localDelta, delta);
    ReleaseSRWLockShared(&s_localLock);
    if (!valid)
        return false;
    cmd->forwardmove = cmd->rightmove = 0;
    cmd->button_bits.array[0] = cmd->button_bits.array[1] = 0;
    if (!playing)
        return true;
    const int recTime = cmd->serverTime - toLevel0;
    const size_t next = SP_ReplayUpperBound(recTime);
    const ReplayCmd &rc = next ? s_replayCmds[next - 1] : s_replayCmds.front();
    for (int i = 0; i < 3; ++i)
        cmd->angles[i] = (int)((rc.angles[i] - delta[i]) * 65536.0f / 360.0f) & 0xFFFF;
    if (!next)
    {
        s_localLastServerTime = cmd->serverTime; // before the recording: stand facing the recorded start
        return true;
    }
    if (next >= s_replayCmds.size() && recTime > rc.t + 100)
    {
        // After the recording: stand still with the last view and stance.
        cmd->button_bits.array[0] = rc.buttons & ((0x80000000u >> 8) | (0x80000000u >> 9));
        s_localLastServerTime = cmd->serverTime;
        return true;
    }
    if (rc.weapon && rc.weapon != s_localRecWeapon)
    {
        if (s_localRecWeapon >= 0)
        {
            s_localPendingWeapon = SP_ReplayLocalWeapon(rc.weapon);
            s_localPendingUntil = cmd->serverTime + 1000;
        }
        s_localRecWeapon = rc.weapon;
    }
    if (s_localPendingWeapon && cmd->serverTime > s_localPendingUntil)
        s_localPendingWeapon = 0;
    if (s_localPendingWeapon && predictedPs && BG_PlayerHasWeapon(predictedPs, s_localPendingWeapon))
    {
        if (predictedPs->weapon != s_localPendingWeapon)
            cmd->weapon = s_localPendingWeapon;
        s_localPendingWeapon = 0;
    }
    cmd->forwardmove = (char)rc.forward;
    cmd->rightmove = (char)rc.right;
    unsigned int buttons = rc.buttons, buttons1 = rc.buttons1;
    const int since = s_localLastServerTime - toLevel0;
    for (size_t i = next - 1; i > 0 && s_replayCmds[i - 1].t > since && since >= 0; --i)
    {
        buttons |= s_replayCmds[i - 1].buttons;
        buttons1 |= s_replayCmds[i - 1].buttons1;
    }
    cmd->button_bits.array[0] = buttons & REPLAY_LOCAL_BUTTON_MASK;
    cmd->button_bits.array[1] = buttons1 & (0x80000000u >> (44 - 32));
    s_localLastServerTime = cmd->serverTime;
    return true;
}
