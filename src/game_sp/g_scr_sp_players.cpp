#include "g_sp_vision.h"
#include "g_scr_sp_players_x1.h"
// SP builtin table, lane C: players / session (getplayers, numremoteclients, getnum{connected,expected}players, ...).
// Searched after BO1Zombies's own tables and before the generated not-ported table (see
// src/game_sp/scr_sp_tables.h). Add a row per real port, keep the end marker last, then rerun
// `python tools/gen_sp_builtins.py` so the name's not-ported stub is dropped.
#include "scr_sp_tables.h"
#include "g_sp_player_state.h"
#include <clientscript/cscr_vm.h>
#include <clientscript/cscr_stringlist.h>
#include <game_mp/g_main_mp.h>
#include <game_mp/g_active_mp.h>
#include <game_mp/g_spawn_mp.h>
#include <server/server.h>
#include <server_mp/sv_main_mp.h>
#include <server_mp/sv_init_mp.h>
#include <universal/dvar.h>
#include <bgame/bg_animation.h>
#include <bgame/bg_mantle.h>
#include <cgame_mp/cg_predict_mp.h>
#include <clientscript/scr_const.h>
#include <game_mp/g_utils_mp.h>
#include <server/sv_game.h>
#include <universal/com_math.h>
#include <cstring>
#include "g_sp_levelstart.h"
#include "g_sp_savegame.h"
#include "g_sp_level_exit.h"
#include "g_sp_testplan.h"
#include "scr_sp_debug.h"
#include <qcommon/cmd.h>
#include <win32/win_shared.h>

void G_SP_ResetMissionFailed()
{
    if (G_SP_IsSPLevel())
    {
        // zombies: SP registration 0x007E1C10. g_reloading, its level reset and the continuation
        // timer belong to the level-exit state (g_sp_level_exit.cpp, SP 0x0041DE40).
        _Dvar_RegisterInt("g_deathDelay", 4000, 0, 0x7FFFFFFF, 1, "Delay before continuing after death");
    }
}

static void G_f_missionfailed()
{
    // zombies: missionfailed (SP 0x007FBD00), player lookup at 0x0054EC60.
    // Brief suggested map restart; exe queues loadgame_continue after its death fade.
    SV_GameSendServerCommand(-1, SV_CMD_RELIABLE, "v hud_missionFailed 1");
    if (!G_Find(nullptr, offsetof(gentity_s, classname), scr_const.player))
        return;

    // zombies: G_MissionFailed death-fade setup (SP 0x00503650).
    if (!g_reloading->current.integer)
    {
        Dvar_SetInt(const_cast<dvar_s *>(g_reloading), 1);
        const int delay = Dvar_GetInt("g_deathDelay");
        // SP 0x00503682 stores the deadline in the level-exit state (0x01C07010); G_SP_CheckLevelExit
        // (SP 0x0041DEF0 branch) queues loadgame_continue when it passes.
        g_spLevelExit.loadGameTime = Sys_Milliseconds() + delay + 1000;
        const dvar_s *quote = Dvar_FindVar("ui_deadquote");
        SV_GameSendServerCommand(-1, SV_CMD_RELIABLE, va("q 0 %i", delay));
        SV_GameSendServerCommand(-1, SV_CMD_RELIABLE, "A 0 0 0 0");
        SV_GameSendServerCommand(-1, SV_CMD_RELIABLE, va("A %i %f 1 2", delay, 32.0));
        SV_GameSendServerCommand(-1, SV_CMD_RELIABLE, va("X %i %f %f", delay, 1.0, 0.10000000149011612));
        SV_GameSendServerCommand(-1, SV_CMD_RELIABLE, "4 0 0");
        SV_GameSendServerCommand(-1, SV_CMD_RELIABLE, "0 0");
        if (quote)
            SV_GameSendServerCommand(-1, SV_CMD_RELIABLE, va("Y 1 %s ", quote->current.string));
    }
    // SP 0x007FBD3A: after G_MissionFailed, whether or not it was already reloading.
    g_spLevelExit.loadGameContinue = 1;
}

// zombies: getplayers (SP 0x007f0a10). The exported C is absent; checked against exe bytes.
static void G_f_getplayers()
{
    bool filterTeam = false;
    int team = TEAM_ALLIES;
    if (Scr_GetNumParam(SCRIPTINSTANCE_SERVER))
    {
        unsigned int teamString = Scr_GetConstString(0, SCRIPTINSTANCE_SERVER);
        const char *name = SL_ConvertToString(teamString, SCRIPTINSTANCE_SERVER);
        filterTeam = true;
        if (!strcmp(name, "allies"))
            team = TEAM_ALLIES;
        else if (!strcmp(name, "axis"))
            team = TEAM_AXIS;
        else if (!strcmp(name, "neutral"))
            team = -1; // KB has no neutral client team; do not confuse SP 3 with MP spectator.
        else if (!strcmp(name, "spectator"))
            team = TEAM_SPECTATOR;
        else if (!strcmp(name, "all"))
            filterTeam = false;
        else
        {
            Scr_Error(va("Unknown team %s passed to getplayers()", Scr_GetString(0, SCRIPTINSTANCE_SERVER)), false);
            return;
        }
    }
    Scr_MakeArray(SCRIPTINSTANCE_SERVER);
    for (int i = 0; i < sv_maxclients->current.integer; ++i)
    {
        gentity_s *ent = G_GetPlayer(i);
        if (!ent->client || ent->client->sess.connected != CON_CONNECTED)
            continue;
        if (filterTeam && ent->client->sess.cs.team != team)
            continue;
        Scr_AddEntity(ent, SCRIPTINSTANCE_SERVER);
        Scr_AddArray(SCRIPTINSTANCE_SERVER);
    }
}

// zombies: getnumconnectedplayers (SP 0x0068e8b0).
static void G_f_getnumconnectedplayers()
{
    int count = 0;
    for (int i = 0; i < sv_maxclients->current.integer; ++i)
    {
        // SP state 4 is KB's CS_ACTIVE (5); SP substate (client_t +4) 10 is the connect state the client
        // reports in its packets (connectState, set by SV_PacketEvent): CA_ACTIVE once the client has
        // parsed its first snapshot, so a client counts one frame after it entered the world
        // (bots: SV_SendClientMessages sets it at the end of that frame).
        const client_t *cl = &svs.clients[i];
        if (cl->header.state == CS_ACTIVE && cl->connectState == 10 /*CA_ACTIVE*/)
            ++count;
    }
    Scr_AddInt(count, SCRIPTINSTANCE_SERVER);
}

static bool SP_IsFrontendMap()
{
    // SP 0x00684eb0: this is a frontend-map test, not a zombiemode test.
    const char *map = Dvar_GetString("mapname");
    return !I_strnicmp(map, "menu_", 5) || !I_stricmp(map, "frontend");
}

// zombies: getnumexpectedplayers (SP 0x005e6b20).
static void G_f_getnumexpectedplayers()
{
    if (!SP_IsFrontendMap() && (Dvar_GetBool("onlinegame") || Dvar_GetBool("systemlink")))
    {
        // SP uses max(Party_CountMembers(&g_partyData), 1) here. KB has no party implementation.
        Scr_Error("getnumexpectedplayers: SP party count is not ported (SP 0x005ae190)", false);
        return;
    }
    int count = 0;
    for (int i = 0; i < sv_maxclients->current.integer; ++i)
    {
        if (svs.clients[i].header.state > CS_ZOMBIE)
            ++count;
    }
    Scr_AddInt(count, SCRIPTINSTANCE_SERVER);
}

// zombies: numremoteclients (SP 0x00642850).
static void G_f_numremoteclients()
{
    int count = 0;
    if (Dvar_GetBool("onlinegame") || Dvar_GetBool("systemlink"))
    {
        // SP: max(Party_CountMembers(&g_partyData) (SP 0x005ae190), 1) - 1. KB has no party (no remote
        // members can exist), so the party is the local player alone: 0, as SP returns for a solo party.
        // The SP front end's Zombies menu sets onlinegame 1 while the level's spawn threads call this.
        const int partyMembers = 1;
        count = (partyMembers > 1 ? partyMembers : 1) - 1;
    }
    Scr_AddInt(count, SCRIPTINSTANCE_SERVER);
}

// zombies: issaverecentlyloaded (SP 0x007fb020). KB has no SP save loading; solo starts fresh.
static void G_f_issaverecentlyloaded()
{
    Scr_AddBool(false, SCRIPTINSTANCE_SERVER);
}

// zombies: setsaveddvar (SP 0x007f06a0). Checked against exe bytes (exported C absent).
static void G_f_setsaveddvar()
{
    const char *name = Scr_GetString(0, SCRIPTINSTANCE_SERVER);
    char message[1024];
    const char *value;
    if (Scr_GetType(1, SCRIPTINSTANCE_SERVER) == VAR_ISTRING)
    {
        Scr_ConstructMessageString(1, Scr_GetNumParam(SCRIPTINSTANCE_SERVER) - 1, "Dvar Value", message, sizeof(message));
        value = message;
    }
    else
        value = Scr_GetString(1, SCRIPTINSTANCE_SERVER);

    char clean[1024] = {};
    // Preserve SP's quote replacement, bounded to the actual buffer (the exe's bound is 0x4000).
    for (unsigned int i = 0; i < sizeof(clean) - 1 && value[i]; ++i)
        clean[i] = value[i] == '"' ? '\'' : value[i];
    if (!Dvar_IsValidName(name))
    {
        Scr_Error(va("Dvar %s has an invalid dvar name", name), false);
        return;
    }
    const dvar_s *dvar = Dvar_FindVar(name);
    if (!dvar)
    {
        Scr_Error(va("SetSavedDvar(): The dvar \"%s\" does not exist.", name), false);
        return;
    }
    if (!(dvar->flags & 0x1000)) // DVAR_SAVED, same bit in SP and KB.
    {
        Scr_Error("SetSavedDvar can only be called on dvars with the SAVED flag set", false);
        return;
    }
    Dvar_SetFromStringByNameFromSource(name, clean, DVAR_SOURCE_SCRIPT, 0);
}

static gentity_s *SP_PlayerEntity(scr_entref_t entref)
{
    if (entref.classnum)
    {
        Scr_ObjectError("not an entity", SCRIPTINSTANCE_SERVER);
        return nullptr;
    }
    iassert(entref.entnum < g_scrEntNumLimit);
    gentity_s *ent = &g_entities[entref.entnum];
    if (!ent->client)
    {
        Scr_ObjectError(va("entity %i is not a player", entref.entnum), SCRIPTINSTANCE_SERVER);
        return nullptr;
    }
    return ent;
}

static void SP_AllowPermission(scr_entref_t entref, unsigned int mask)
{
    gentity_s *ent = SP_PlayerEntity(entref);
    if (!ent)
        return;
    SPPlayerBuiltinState &state = G_SP_PlayerBuiltinState(ent->s.number);
    if (Scr_GetInt(0, SCRIPTINSTANCE_SERVER))
    {
        state.disabledActions &= ~mask;
        return;
    }
    state.disabledActions |= mask;
}

static void G_m_getcurrentweaponclipammo(scr_entref_t entref)
{
    // zombies: current weapon clip query, no script arguments (SP 0x005D8FF0).
    gentity_s *ent = SP_PlayerEntity(entref);
    if (!ent)
        return;
    playerState_s *ps = &ent->client->ps;
    Scr_AddInt(ps->weapon ? BG_GetAmmoInClip(ps, ps->weapon) : 0, SCRIPTINSTANCE_SERVER);
}

// zombies: settransported <seconds> (SP 0x007fa6a0). No server state: sends the client ']' <ms> (the transporter
// overlay, CG_SP_TransportedCommand). ms is truncated (cvttss2si), the time is checked before the entity.
static void G_m_settransported(scr_entref_t entref)
{
    int time = (int)(Scr_GetFloat(0, SCRIPTINSTANCE_SERVER) * 1000.0f);
    if (time < 0)
        Scr_ParamError(1, "Time must be positive", SCRIPTINSTANCE_SERVER);
    if (entref.classnum)
    {
        Scr_ObjectError("not an entity", SCRIPTINSTANCE_SERVER);
        return;
    }
    iassert(entref.entnum < g_scrEntNumLimit);
    gentity_s *ent = &g_entities[entref.entnum];
    if (!ent->r.inuse || !ent->client)
    {
        Scr_Error("settransported() called on an invalid client entity.\n", SCRIPTINSTANCE_SERVER);
        return;
    }
    SV_GameSendServerCommand(entref.entnum, SV_CMD_RELIABLE, va("%c %i", ']', time));
}

// zombies: setwatersheeting <on> [seconds] (SP 0x00416190). No server state: sends the client '!' <on> <ms>
// (CG_SP_WaterSheetingCommand). SP checks only eType == ET_PLAYER (no inuse / client check) and sends to the
// entity's clientNum byte (ent+0xd2); for a player entity that is its entnum. ms is 0 without a second arg.
static void G_m_setwatersheeting(scr_entref_t entref)
{
    if (entref.classnum)
        Scr_ObjectError("not an entity", SCRIPTINSTANCE_SERVER);
    iassert(entref.entnum < MAX_GENTITIES);
    gentity_s *ent = &g_entities[entref.entnum];
    if (ent->s.eType != ET_PLAYER)
        Scr_Error("SetWaterSheeting called on an ent that's not a player.", SCRIPTINSTANCE_SERVER);
    int on = Scr_GetInt(0, SCRIPTINSTANCE_SERVER);
    int duration = 0;
    if (Scr_GetNumParam(SCRIPTINSTANCE_SERVER) == 2)
        duration = (int)(Scr_GetFloat(1, SCRIPTINSTANCE_SERVER) * 1000.0f);
    SV_GameSendServerCommand(entref.entnum, SV_CMD_RELIABLE, va("%c %i %i", '!', on, duration));
}

// zombies: allowprone (SP 0x006121a0). Storage port; movement consumers still need a port.
static void G_m_allowprone(scr_entref_t entref)
{
    SP_AllowPermission(entref, SP_DISABLE_PRONE);
}

// zombies: allowcrouch (SP 0x00501ae0).
static void G_m_allowcrouch(scr_entref_t entref)
{
    SP_AllowPermission(entref, SP_DISABLE_CROUCH);
}

// zombies: allowlean (SP 0x00599ae0).
static void G_m_allowlean(scr_entref_t entref)
{
    SP_AllowPermission(entref, SP_DISABLE_LEAN);
}

// zombies: allowmelee (SP 0x00489fb0).
static void G_m_allowmelee(scr_entref_t entref)
{
    SP_AllowPermission(entref, SP_DISABLE_MELEE);
}

// zombies: allowstand (SP 0x00521050).
static void G_m_allowstand(scr_entref_t entref)
{
    SP_AllowPermission(entref, SP_DISABLE_STAND);
}

// zombies: savegame (SP 0x007fad20). Queues through G_SaveGame (SP 0x0043c850, g_sp_savegame.cpp);
// the queued request reaches SP's serializer failure return because KB has no SP save serializer.
static void G_f_savegame()
{
    const char *map = Dvar_GetString("mapname");
    if (SP_IsFrontendMap())
        return;
    if (g_entities[0].health < 1)
        Scr_Error("Attempting to save while dead\n", false);
    const char *name = Scr_GetString(0, SCRIPTINSTANCE_SERVER);
    if (!name[0])
        name = "auto";
    int count = Scr_GetNumParam(SCRIPTINSTANCE_SERVER);
    const char *description = count > 1 ? Scr_GetIString(1, SCRIPTINSTANCE_SERVER) : "";
    const char *screenshot = count > 2 ? Scr_GetString(2, SCRIPTINSTANCE_SERVER) : "$default";
    bool suppress = count > 3 && Scr_GetInt(3, SCRIPTINSTANCE_SERVER) != 0;
    char filename[64];
    if (!I_stricmp(name, "levelstart"))
        Com_sprintf(filename, sizeof(filename), "%s", map);
    else
        Com_sprintf(filename, sizeof(filename), "%s-%s", map, name);
    // SP clears the memory-commit flag here (0x004ed9e0); see g_sp_savegame.cpp for why KB stores none.
    // SP passes auto save type 0, flags 2 (commit to memory) and a no-op callback (0x00651a30).
    const int result = G_SP_SaveGame(filename, description, screenshot, 0, 2, suppress, 0, nullptr, 0);
    if (result == SP_SAVE_SAME_FRAME)
        Scr_Error("Attempting to save multiple times in a single frame.\n", false);
    else if (result == SP_SAVE_RELOADING)
        Scr_Error("Attempting to save during a restart.\n", false);
}

// zombies: getdifficulty (SP 0x007f07e0), names at SP 0x00b75ecc.
static void G_f_getdifficulty()
{
    // SP registration: default 1, range 0..3, flags 0x1064 (including SAVED and LATCH).
    const dvar_s *skill = _Dvar_RegisterInt("g_gameskill", 1, 0, 3, 0x1064, "Game difficulty");
    static const char *names[] = { "easy", "medium", "hard", "fu" };
    Scr_AddString(names[skill->current.integer], SCRIPTINSTANCE_SERVER);
}

// zombies: enablehealthshield (SP 0x007d8340). Damage-system consumer still needs a port.
static void G_m_enablehealthshield(scr_entref_t entref)
{
    gentity_s *ent = SP_PlayerEntity(entref);
    if (!ent)
        return;
    if (Scr_GetInt(0, SCRIPTINSTANCE_SERVER))
    {
        G_SP_PlayerBuiltinState(ent->s.number).healthShield = true;
        return;
    }
    G_SP_PlayerBuiltinState(ent->s.number).healthShield = false;
}

// zombies: playerknockback (SP 0x007d7300). SP's 0x20 is KB's FL_NO_KNOCKBACK (0x8).
static void G_m_playerknockback(scr_entref_t entref)
{
    gentity_s *ent = SP_PlayerEntity(entref);
    if (!ent)
        return;
    if (Scr_GetInt(0, SCRIPTINSTANCE_SERVER) != 1)
    {
        ent->flags |= FL_NO_KNOCKBACK;
        return;
    }
    ent->flags &= ~FL_NO_KNOCKBACK;
}

// zombies: reviveplayer (SP 0x007db090). Exported C absent; statement order checked in exe bytes.
static void G_m_reviveplayer(scr_entref_t entref)
{
    gentity_s *ent = SP_PlayerEntity(entref);
    if (!ent)
        return;
    gclient_s *client = ent->client;
    client->ps.damageTimer = 0;
    client->ps.damageDuration = 0;
    pmove_t pm(&g_pmove[client->ps.clientNum]);
    client->damage_fromWorld = 0;
    client->revive = 0;
    if (pm.cmd.button_bits.testBit(9))
        BG_AnimScriptEvent(&g_pmove[client->ps.clientNum], ANIM_ET_LASTSTAND_TO_CROUCH, 0, 1);
    else if (pm.cmd.button_bits.testBit(8))
        BG_AnimScriptEvent(&g_pmove[client->ps.clientNum], ANIM_ET_LASTSTAND_TO_PRONE, 0, 1);
    else
        BG_AnimScriptEvent(&g_pmove[client->ps.clientNum], ANIM_ET_LASTSTAND_TO_STAND, 0, 1);
    client->lastStand = 0;
    client->lastStandTime = 0;
    client->ps.lastStandPrevWeapon = 0;
    // SP clientState +0x40 is lastStandStartTime and +0x44 beingRevived (SP clientState netfields,
    // .rdata 0x00a5c0f8); the earlier port cleared lastDamageTime (+0x3c) by mistake.
    G_GetClientState(ent->s.number)->lastStandStartTime = 0;
    G_SP_PlayerX1State(ent->s.number).beingRevived = false;
    ent->health = client->sess.maxHealth;
    ent->maxHealth = client->sess.maxHealth; // SP gentity +0x188
    client->damage_blood = 0;
    client->ps.stats[0] = ent->health;
}

// zombies: playerlinkto (SP 0x007f2880).
static void G_m_playerlinkto(scr_entref_t entref)
{
    gentity_s *ent = GetEntity(entref);
    if (Scr_GetType(0, SCRIPTINSTANCE_SERVER) != VAR_POINTER || Scr_GetPointerType(0, SCRIPTINSTANCE_SERVER) != VAR_ENTITY)
        Scr_ParamError(0, "not an entity", SCRIPTINSTANCE_SERVER);
    if (!ent->client)
        Scr_ObjectError("not a player entity", SCRIPTINSTANCE_SERVER);
    gentity_s *parent = Scr_GetEntity(0);
    int count = Scr_GetNumParam(SCRIPTINSTANCE_SERVER);
    unsigned int tag = 0;
    if (count > 1 && Scr_GetType(1, SCRIPTINSTANCE_SERVER))
    {
        tag = Scr_GetConstLowercaseString(1, SCRIPTINSTANCE_SERVER);
        if (tag == scr_const._)
            tag = 0;
    }
    float fraction = count > 2 ? (float)Scr_GetFloat(2, SCRIPTINSTANCE_SERVER) : 0.0f;
    float arcs[4];
    for (int i = 0; i < 4; ++i)
    {
        float arc = count > i + 3 ? (float)Scr_GetFloat(i + 3, SCRIPTINSTANCE_SERVER) : 180.0f;
        arcs[i] = arc;
        if (arc >= 180.0f)
            arcs[i] = 180.0f;
        if (arc <= 0.0f)
            arcs[i] = 0.0f;
    }
    gclient_s *client = ent->client;
    G_UpdateViewAngleClamp(client, parent->r.currentAngles);
    if (count > 7 && Scr_GetInt(7, SCRIPTINSTANCE_SERVER))
        client->ps.linkFlags |= 2;
    else
        client->ps.linkFlags &= ~2;

    // SP 0x00620c00 (player link helper); use KB's link machinery and its pm_type transition.
    client->linkAnglesFrac = fraction;
    client->linkAnglesLocked = false;
    client->ps.linkFlags &= ~1;
    // SP also clears playerlinktoabsolute's pm_flags 0x04000000 (read by G_SetPlayerFixedLink on SP levels).
    client->ps.pm_flags &= ~0x4000000;
    client->prevLinkAnglesSet = false;
    client->linkAnglesMinClamp[1] = -arcs[0];
    client->linkAnglesMaxClamp[1] = arcs[1];
    client->linkAnglesMinClamp[0] = -arcs[2];
    client->linkAnglesMaxClamp[0] = arcs[3];
    if (G_EntLinkTo(ent, parent, tag))
    {
        if (!(client->ps.linkFlags & 2))
        {
            Vec3Clear(client->ps.linkAngles);
            return;
        }
        float axis[4][3];
        G_CalcTagParentAxis(ent, axis);
        AxisToAngles(axis, client->ps.linkAngles);
        return;
    }
    Scr_Error("failed to link entity", false);
}

// zombies: disablegrenadesuicide (SP 0x00804d40). ClientEvents consumer still needs SP 0x00552237.
static void G_f_disablegrenadesuicide()
{
    G_SP_DisableGrenadeSuicide();
}

static int SP_BlurClient(scr_entref_t entref)
{
    if (entref.classnum)
        Scr_ObjectError("not an entity", SCRIPTINSTANCE_SERVER);
    else
    {
        iassert(entref.entnum < g_scrEntNumLimit);
        gentity_s *ent = &g_entities[entref.entnum];
        if (ent->r.inuse && ent->client)
            return ent->s.number;
    }
    // Both SP blur methods use this exact error.
    Scr_Error("setblur() called on an invalid client entity.\n", false);
    return -1;
}

// zombies: setblur (SP 0x007f9f90). Lane F must consume A <milliseconds> <radius> 0 1.
static void G_m_setblur(scr_entref_t entref)
{
    float blur = (float)Scr_GetFloat(0, SCRIPTINSTANCE_SERVER);
    float time = (float)Scr_GetFloat(1, SCRIPTINSTANCE_SERVER);
    if (time < 0.0f)
        Scr_ParamError(1, "Time must be positive", SCRIPTINSTANCE_SERVER);
    if (blur < 0.0f)
        Scr_ParamError(0, "Blur value must be greater than 0", SCRIPTINSTANCE_SERVER);
    int clientNum = SP_BlurClient(entref);
    if (G_SP_TestPlanActive())
    {
        // KB measurement (not SP): who blurs the test client (Nova-6 gas: _zombiemode_ai_quad::quad_gas_area_of_effect).
        Com_Printf(16, "bo1_effect: time %d setblur client %d blur %.2f time %.2f at %s\n", level.time, clientNum, blur,
            time, Scr_SP_DescribeCodePos(SCRIPTINSTANCE_SERVER, gScrVmPub[SCRIPTINSTANCE_SERVER].function_frame->fs.pos));
    }
    SV_GameSendServerCommand(clientNum, SV_CMD_RELIABLE, va("%c %i %f %i %i", 'A', (int)(time * 1000.0f), blur, 0, 1));
}

// zombies: startfadingblur (SP 0x007fa080). Lane F must consume | <milliseconds> <radius>.
static void G_m_startfadingblur(scr_entref_t entref)
{
    float blur = (float)Scr_GetFloat(0, SCRIPTINSTANCE_SERVER);
    float time = (float)Scr_GetFloat(1, SCRIPTINSTANCE_SERVER);
    if (time < 0.0f)
        Scr_ParamError(1, "Time must be positive", SCRIPTINSTANCE_SERVER);
    if (blur < 0.0f)
        Scr_ParamError(0, "Blur value must be greater than 0", SCRIPTINSTANCE_SERVER);
    int clientNum = SP_BlurClient(entref);
    SV_GameSendServerCommand(clientNum, SV_CMD_RELIABLE, va("%c %i %f", '|', (int)(time * 1000.0f), blur));
}

// zombies: retail IsGodMode accepts an entity and tests both god/demigod flags (SP 0x007F00F0).
// The exported builtin metadata reports codeSize 1; the exe has this complete 79-byte body.
static void G_f_isgodmode()
{
    if (Scr_GetType(0, SCRIPTINSTANCE_SERVER) == VAR_POINTER
        && Scr_GetPointerType(0, SCRIPTINSTANCE_SERVER) == VAR_ENTITY
        && (Scr_GetEntity(0)->flags & 3))
    {
        Scr_AddBool(true, SCRIPTINSTANCE_SERVER);
        return;
    }
    Scr_AddBool(false, SCRIPTINSTANCE_SERVER);
}

const BuiltinFunctionDef g_sp_players_functions[] =
{
    { "isgodmode", G_f_isgodmode, 0 },
    { "getplayers", G_f_getplayers, 0 },
    { "getnumconnectedplayers", G_f_getnumconnectedplayers, 0 },
    { "getnumexpectedplayers", G_f_getnumexpectedplayers, 0 },
    { "numremoteclients", G_f_numremoteclients, 0 },
    { "issaverecentlyloaded", G_f_issaverecentlyloaded, 0 },
    { "setsaveddvar", G_f_setsaveddvar, 0 },
    { "savegame", G_f_savegame, 0 },
    { "missionfailed", G_f_missionfailed, 0 },
    { "getdifficulty", G_f_getdifficulty, 0 },
    { "disablegrenadesuicide", G_f_disablegrenadesuicide, 0 },
    // lane x1 (g_scr_sp_players_x1.cpp)
    { "refreshhudammocounter", GScrSP_refreshhudammocounter, 0 },
    { "reportclientdisconnected", GScrSP_reportclientdisconnected, 0 },
    { "playerpositionvalid", GScrSP_playerpositionvalid, 0 },
    { "getpersistentprofilevar", GScrSP_getpersistentprofilevar, 0 },
    { "setpersistentprofilevar", GScrSP_setpersistentprofilevar, 0 },
    { "hascollectible", GScrSP_hascollectible, 0 },
    { "iscoopepd", GScrSP_iscoopepd, 0 },
    { "getweaponclipmodel", GScrSP_getweaponclipmodel, 0 },
    { "weaponfightdist", GScrSP_weaponfightdist, 0 },
    { "weaponmaxdist", GScrSP_weaponmaxdist, 0 },
    { nullptr, nullptr, 0 } // end marker, keep last
};
const unsigned int g_sp_players_function_count = ARRAY_COUNT(g_sp_players_functions) - 1;

const BuiltinMethodDef g_sp_players_methods[] =
{
    { "getcurrentweaponclipammo", G_m_getcurrentweaponclipammo, 0 },
    { "visionsetnaked", G_SP_PlayerVisionSetNaked, 0 },
    { "getvisionsetnaked", G_SP_GetVisionSetNaked, 0 },
    { "allowprone", G_m_allowprone, 0 },
    { "settransported", G_m_settransported, 0 },
    { "setwatersheeting", G_m_setwatersheeting, 0 },
    { "allowcrouch", G_m_allowcrouch, 0 },
    { "allowlean", G_m_allowlean, 0 },
    { "allowmelee", G_m_allowmelee, 0 },
    { "allowstand", G_m_allowstand, 0 },
    { "enablehealthshield", G_m_enablehealthshield, 0 },
    { "playerknockback", G_m_playerknockback, 0 },
    { "reviveplayer", G_m_reviveplayer, 0 },
    { "playerlinkto", G_m_playerlinkto, 0 },
    { "setblur", G_m_setblur, 0 },
    { "startfadingblur", G_m_startfadingblur, 0 },
    // lane x1 (g_scr_sp_players_x1.cpp)
    { "startrevive", PlayerCmdSP_startrevive, 0 },
    { "stoprevive", PlayerCmdSP_stoprevive, 0 },
    { "visionsetlaststand", PlayerCmdSP_visionsetlaststand, 0 },
    { "setmaxhealth", PlayerCmdSP_setmaxhealth, 0 },
    { "hideviewmodel", PlayerCmdSP_hideviewmodel, 0 },
    { "showviewmodel", PlayerCmdSP_showviewmodel, 0 },
    { "reloadbuttonpressed", PlayerCmdSP_reloadbuttonpressed, 0 },
    { "allowpickupweapons", PlayerCmdSP_allowpickupweapons, 0 },
    { "setautopickup", PlayerCmdSP_setautopickup, 0 },
    { "getweaponmuzzlepoint", PlayerCmdSP_getweaponmuzzlepoint, 0 },
    { "getweaponforwarddir", PlayerCmdSP_getweaponforwarddir, 0 },
    { "getweaponrenderoptions", PlayerCmdSP_getweaponrenderoptions, 0 },
    { "updateweaponoptions", PlayerCmdSP_updateweaponoptions, 0 },
    { "disableweaponfire", PlayerCmdSP_disableweaponfire, 0 },
    { "enableweaponfire", PlayerCmdSP_enableweaponfire, 0 },
    { "disableweaponreload", PlayerCmdSP_disableweaponreload, 0 },
    { "enableweaponreload", PlayerCmdSP_enableweaponreload, 0 },
    { "setloweredweapon", PlayerCmdSP_setloweredweapon, 0 },
    { "playersetgroundreferenceent", PlayerCmdSP_playersetgroundreferenceent, 0 },
    { "playerlinktoabsolute", PlayerCmdSP_playerlinktoabsolute, 0 },
    { "setdoublevision", PlayerCmdSP_setdoublevision, 0 },
    { "getplayerviewheight", PlayerCmdSP_getplayerviewheight, 0 },
    { "islookingat", PlayerCmdSP_islookingat, 0 },
    { "resetadswidthandlerp", PlayerCmdSP_resetadswidthandlerp, 0 },
    { "setadswidthandlerp", PlayerCmdSP_setadswidthandlerp, 0 },
    { "uploadscore", PlayerCmdSP_uploadscore, 0 },
    { "setvolfog", PlayerCmdSP_setvolfog, 0 },
    { "setshadowhint", PlayerCmdSP_setshadowhint, 0 },
    { "startcameratween", PlayerCmdSP_startcameratween, 0 },
    { "getnormalizedmovement", PlayerCmdSP_getnormalizedmovement, 0 },
    { "getnormalizedcameramovement", PlayerCmdSP_getnormalizedcameramovement, 0 },
    { "dropweapon", ActorCmdSP_dropweapon, 0 },
    { "enterprone", ActorCmdSP_enterprone, 0 },
    { "exitprone", ActorCmdSP_exitprone, 0 },
    { "changefontscaleovertime", HECmdSP_changefontscaleovertime, 0 },
    { nullptr, nullptr, 0 } // end marker, keep last
};
const unsigned int g_sp_players_method_count = ARRAY_COUNT(g_sp_players_methods) - 1;
