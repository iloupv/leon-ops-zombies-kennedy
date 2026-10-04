#include <qcommon/cm_mapkit.h>
#include <database/db_registry.h>
#include "g_sp_levelstart.h"

#include <cstring>
#include <cfloat>

#include <clientscript/cscr_animtree.h>
#include <clientscript/cscr_main.h>
#include <clientscript/cscr_stringlist.h>
#include <clientscript/cscr_vm.h>
#include <game/g_load_utils.h>
#include <game_mp/actor_mp.h>
#include <game_sp/actor_sp_ext.h>
#include <game_mp/g_cmds_mp.h>
#include <game_mp/g_main_mp.h>
#include <game_mp/g_scr_main_mp.h>
#include <game_mp/g_spawn_mp.h>
#include <qcommon/cmd.h>
#include <qcommon/common.h>
#include <server_mp/sv_init_mp.h>
#include <universal/com_memory.h>
#include <universal/dvar.h>
#include <universal/q_shared.h>

scr_data_sp_t g_scr_data_sp;
static XAnim_s *g_actorAnimsSP; // SP .data 0x01C79B60

bool G_SP_IsZombieMode()
{
    return zombiemode && zombiemode->current.enabled;
}

bool G_SP_IsSPLevel()
{
    return Com_IsSPLevel();
}

void G_SP_RegisterLevelDvars()
{
    const bool spLevel = G_SP_IsSPLevel(); // SP registers these for every level
    // zombies: g_speed = 190, INT_MIN..INT_MAX, flags 0x3000 (SP 0x007E0E58..0x007E0E76).
    // MP registers the same value/domain with flags 0. Reregistration ORs flags, so
    // restore the MP bits explicitly when leaving a zombie map in this process.
    const_cast<dvar_s *>(g_speed)->flags = (g_speed->flags & ~0x3000) | (spLevel ? 0x3000 : 0);
    // zombies: ai_corpseCount 0..32 on an SP level (SP 0x007E0F9D), 0..8 on MP. Reregistration keeps the first
    // domain, so set it here; clamp a value that the MP domain cannot hold.
    const_cast<dvar_s *>(ai_corpseCount)->domain.integer.max = Com_GetMaxActorCorpses();
    if (ai_corpseCount->current.integer > ai_corpseCount->domain.integer.max)
        Dvar_SetInt(const_cast<dvar_s *>(ai_corpseCount), ai_corpseCount->domain.integer.max);
    // zombies: sv_maxRate defaults to 7000 in SP (0..25000, flags 5; SP 0x006986D8), 5000 in MP. Loopback and LAN
    // clients are never rate limited (SV_SendClientMessages), so only a remote co-op client sees it. Swap the default
    // per level; a value the player set (anything but the other exe's default) is kept.
    if (sv_maxRate)
    {
        dvar_s *maxRate = const_cast<dvar_s *>(sv_maxRate);
        const int want = spLevel ? 7000 : 5000;
        maxRate->reset.integer = want;
        if (maxRate->current.integer == (spLevel ? 5000 : 7000))
            Dvar_SetInt(maxRate, want);
    }
    if (!spLevel)
        return;

    // zombies: SP registration calls; defaults/ranges/flags read from the exe, not live values.
    _Dvar_RegisterFloat("ai_accuracyDistScale", 1.0f, FLT_MIN, FLT_MAX, 0x3080, "AI accuracy distance scale"); // SP 0x007E15F0
    // zombies: run animation scripts must yield a frame, not repeatedly wait(0) (SP 0x007E1C10).
    _Dvar_RegisterFloat("ai_runAnimUpdateFrequency", 0.05f, 0.05f, 0.2f, 0x4080, "Run animation update interval");
    _Dvar_RegisterFloat("player_meleeDamageMultiplier", 1.0f, 0.0f, 1000.0f, 0x3000, "Player melee damage multiplier"); // SP 0x007E0D53
    _Dvar_RegisterFloat("player_radiusDamageMultiplier", 1.0f, 0.0f, 1000.0f, 0x3000, "Player radius damage multiplier"); // SP 0x007E0D1B
    _Dvar_RegisterBool("sv_saveOnStartMap", true, 0x1004, "Save when starting the map"); // SP 0x00698480
    _Dvar_RegisterString("ui_campaign", "american", 0x1000, "Current campaign"); // SP 0x007E0C46
    _Dvar_RegisterInt("g_player_maxhealth", 100, 10, 2000, 0x2000, "Player maximum health"); // SP 0x007E0CB7
    _Dvar_RegisterFloat("player_damageMultiplier", 1.0f, 0.0f, 1000.0f, 0x3000, "Player damage multiplier"); // SP 0x007E0CEB
    _Dvar_RegisterInt("player_deathInvulnerableTime", 1000, 0, 0x7FFFFFFF, 0x3080, "Player death invulnerability time"); // SP 0x007E185C
    // zombies: SP 0x006982DF..0x00698364, read by G_SP_UpdateSkillDvars (SP 0x00640590).
    _Dvar_RegisterInt("player_healthEasy", 500, 10, 2000, 0x2000, "");
    _Dvar_RegisterInt("player_healthMedium", 275, 10, 2000, 0x2000, "");
    _Dvar_RegisterInt("player_healthHard", 165, 10, 2000, 0x2000, "");
    _Dvar_RegisterInt("player_healthFu", 115, 10, 2000, 0x2000, "");
}

void G_SP_UpdateSkillDvars()
{
    // zombies: SP 0x00640590. G_LoadGame calls it after g_gameskill is restored (SP 0x007ECE0C); SP's
    // G_InitGame also calls it when the game is not a restart (SP 0x0051E9A5, KB G_InitGame).
    Dvar_SetInt((dvar_s *)Dvar_FindVar("g_player_maxhealth"), 100);
    if (!I_stricmp(Dvar_GetString("g_gametype"), "vs"))  // SP 0x0063E630
        return;
    const char *health;
    int invulnerableTime;
    switch (Dvar_GetInt("g_gameskill"))
    {
    case 0: health = "player_healthEasy"; invulnerableTime = 4000; break;
    case 1: health = "player_healthMedium"; invulnerableTime = 1000; break;
    case 2: health = "player_healthHard"; invulnerableTime = 100; break;
    default: health = "player_healthFu"; invulnerableTime = 100; break;
    }
    Dvar_SetFloat((dvar_s *)Dvar_FindVar("player_damageMultiplier"), 100.0f / (float)Dvar_GetInt(health));
    Dvar_SetInt((dvar_s *)Dvar_FindVar("player_deathInvulnerableTime"), invulnerableTime);
}

// zombies: SP loads each script twice - a load pass that queues handles (GScr_LoadScriptAndLabel
// 0x007EE310, a print when missing) and a set pass that reads them back in the same order
// (GScr_SetScriptAndLabel 0x007EEB50, Com_Error(ERR_DROP) when a required one is missing). KB
// resolves the handle during the load (GScr_LoadScriptAndLabel in g_scr_main_mp.cpp), so the two
// passes collapse into one call; bEnforceExists is the set pass's requirement.
static int GScr_SP_LoadCallback(scriptInstance_t inst, const char *filename, const char *label, int bEnforceExists)
{
    return GScr_LoadScriptAndLabel(inst, filename, label, bEnforceExists);
}

// zombies: GScr_LoadConsts, SP 0x006124B0. Every CodeCallback_* comes from maps/_callbacksetup (not
// maps/mp/gametypes/_callbacksetup); there is no VehicleRadiusDamage, no UpdateSpawnPoints and no
// pregame; the gametype script is optional. The handles that KB's scr_data_t has a slot for go
// there, so KB's Scr_PlayerConnect / Scr_ActorDamage / ... call the SP functions unchanged.
static void GScr_SP_LoadConsts(scriptInstance_t inst)
{
    const char *cb = "maps/_callbacksetup";
    char filename[64];
    const dvar_s *ui_gametype;

    g_scr_data_sp.saveRestored = GScr_SP_LoadCallback(inst, cb, "CodeCallback_SaveRestored", 1);
    // SP loads StartGameType (0x01C79B34) but the exe never reads the handle: it is not called.
    g_scr_data.gametype.startupgametype = GScr_SP_LoadCallback(inst, cb, "CodeCallback_StartGameType", 1);
    g_scr_data.gametype.playerconnect = GScr_SP_LoadCallback(inst, cb, "CodeCallback_PlayerConnect", 1);
    g_scr_data.gametype.playerdisconnect = GScr_SP_LoadCallback(inst, cb, "CodeCallback_PlayerDisconnect", 1);
    g_scr_data.gametype.actordamage = GScr_SP_LoadCallback(inst, cb, "CodeCallback_ActorDamage", 1);
    g_scr_data.gametype.playerdamage = GScr_SP_LoadCallback(inst, cb, "CodeCallback_PlayerDamage", 1);
    g_scr_data.gametype.playerkilled = GScr_SP_LoadCallback(inst, cb, "CodeCallback_PlayerKilled", 1);
    g_scr_data.gametype.actorkilled = GScr_SP_LoadCallback(inst, cb, "CodeCallback_ActorKilled", 1);
    // SP 0x01C79B50: loaded, never read by the exe either.
    g_scr_data.gametype.playerrevive = GScr_SP_LoadCallback(inst, cb, "CodeCallback_PlayerRevive", 1);
    g_scr_data.gametype.playerlaststand = GScr_SP_LoadCallback(inst, cb, "CodeCallback_PlayerLastStand", 1);
    g_scr_data.levelnotify = GScr_SP_LoadCallback(inst, cb, "CodeCallback_LevelNotify", 1);
    g_scr_data.gametype.vehicledamage = GScr_SP_LoadCallback(inst, cb, "CodeCallback_VehicleDamage", 1);
    g_scr_data_sp.actorShouldReact = GScr_SP_LoadCallback(inst, cb, "CodeCallback_ActorShouldReact", 1);
    g_scr_data.faceeventnotify = GScr_SP_LoadCallback(inst, cb, "CodeCallback_FaceEventNotify", 0);
    g_scr_data_sp.disconnectedDuringLoad = GScr_SP_LoadCallback(inst, cb, "CodeCallback_DisconnectedDuringLoad", 1);
    g_scr_data.destructible_callback = GScr_SP_LoadCallback(inst, "maps/_destructible", "CodeCallback_DestructibleEvent", 1);

    // SP: `if (ui_gametype) handle = maps/gametypes/<ui_gametype>::init` with no existence check
    // (label string .rdata 0x00A0FF44). ui_gametype is "" in a zombies session (dvars-live.json), so
    // the SP exe finds no script there; an empty name is skipped here rather than looking up
    // "maps/gametypes/".
    g_scr_data_sp.gametypeMain = 0;
    ui_gametype = Dvar_FindVar("ui_gametype");
    if ( ui_gametype && *ui_gametype->current.string )
    {
        Com_sprintf(filename, sizeof(filename), "maps/gametypes/%s", ui_gametype->current.string);
        g_scr_data_sp.gametypeMain = GScr_SP_LoadCallback(inst, filename, "init", 0);
    }

    g_scr_data_sp.menuMessage = GScr_SP_LoadCallback(inst, cb, "CodeCallback_MenuMessage", 1);
    g_scr_data_sp.dec20Message = GScr_SP_LoadCallback(inst, cb, "CodeCallback_Dec20Message", 1);
    g_scr_data.glassSmash = GScr_SP_LoadCallback(inst, cb, "CodeCallback_GlassSmash", 0);

    // MP-only slots: nothing in SP fills them.
    g_scr_data.gametype.main = 0;
    g_scr_data.gametype.vehicleradiusdamage = 0;
    g_scr_data.gametype.votecalled = 0;
    g_scr_data.gametype.playervote = 0;
    g_scr_data.updatespawnpoints = 0;
    g_scr_data.pregamescript = 0;
}

// zombies: GScr_SetSingleAnimScript, SP 0x005836B0: "animscripts/%s"::main, required; the name is
// kept as a script string.
static void GScr_SP_LoadSingleAnimScript(scriptInstance_t inst, scr_animscript_t *pAnim, const char *name)
{
    char filename[64];

    Com_sprintf(filename, sizeof(filename), "animscripts/%s", name);
    pAnim->name = GScr_AllocString(name);
    pAnim->func = GScr_LoadScriptAndLabel(inst, filename, "main", 1);
}

// zombies: SP 0x005836B0, the human list's per-entry set: a missing script or label is only printed.
static void GScr_SP_LoadSingleAnimScriptOptional(scriptInstance_t inst, scr_animscript_t *pAnim, const char *name)
{
    char filename[64];

    Com_sprintf(filename, sizeof(filename), "animscripts/%s", name);
    pAnim->func = GScr_LoadScriptAndLabel(inst, filename, "main", 0);
    if ( !pAnim->func )
        Com_PrintError(1, "Could not find label '%s' in script '%s'\n", "main", filename);
    pAnim->name = GScr_AllocString(name);
}

// zombies: GScr_LoadZombieAnimScripts, SP 0x007EE8B0 (load) / 0x007EF090 (set): the zombie
// species' animscripts, then animscripts/zombie_scripted::init into the scripted-init handle.
// Use member names: SP's 0x4c0 list inserts cover_pillar after cover_left; KB does not.
static void GScr_SP_LoadZombieAnimScripts(scriptInstance_t inst)
{
    AnimScriptList *list = &g_scr_data_sp.zombieAnim;

    GScr_SP_LoadSingleAnimScript(inst, &list->combat, "zombie_combat");
    GScr_SP_LoadSingleAnimScript(inst, &list->death, "zombie_death");
    GScr_SP_LoadSingleAnimScript(inst, &list->init, "zombie_init");
    GScr_SP_LoadSingleAnimScript(inst, &list->pain, "zombie_pain");
    GScr_SP_LoadSingleAnimScript(inst, &list->move, "zombie_move");
    GScr_SP_LoadSingleAnimScript(inst, &list->scripted, "zombie_scripted");
    GScr_SP_LoadSingleAnimScript(inst, &list->stop, "zombie_stop");
    // SP G_ScriptedAnim_Start (0x00624f30, call 0x0062530c) executes this with eight
    // arguments for animscripted on an actor. It is not an actor-spawn init hook.
    g_scr_data_sp.scriptedInit = GScr_LoadScriptAndLabel(inst, "animscripts/zombie_scripted", "init", 1);
}

// zombies: SP 0x007EE3B0 (load) / 0x007EEDA0 (set): the human species' animscripts, loaded on every SP level
// that is not zombiemode (the SP front end). The set pass (0x005836B0 per entry) only prints a missing script
// on channel 1, so nothing here is required. SP's list (base 0x01C79B64, 8 bytes per entry) has cover_pillar
// after cover_left; KB's AnimScriptList has no slot for it, so it is loaded (the print) and not stored.
// Then animscripts/scripted::init into the scripted-init handle (0x01C79B2C, the slot zombie_scripted::init
// uses in zombiemode) and animscripts/init_mode_sp::init (0x01C7AE74).
static void GScr_SP_LoadHumanAnimScripts(scriptInstance_t inst)
{
    AnimScriptList *list = Actor_SP_GetAnimScriptStorage(AI_SPECIES_HUMAN);
    scr_animscript_t coverPillar;

    GScr_SP_LoadSingleAnimScriptOptional(inst, &list->combat, "combat");
    GScr_SP_LoadSingleAnimScriptOptional(inst, &list->concealment_crouch, "concealment_crouch");
    GScr_SP_LoadSingleAnimScriptOptional(inst, &list->concealment_prone, "concealment_prone");
    GScr_SP_LoadSingleAnimScriptOptional(inst, &list->concealment_stand, "concealment_stand");
    GScr_SP_LoadSingleAnimScriptOptional(inst, &list->cover_arrival, "cover_arrival");
    GScr_SP_LoadSingleAnimScriptOptional(inst, &list->cover_crouch, "cover_crouch");
    GScr_SP_LoadSingleAnimScriptOptional(inst, &list->cover_left, "cover_left");
    GScr_SP_LoadSingleAnimScriptOptional(inst, &coverPillar, "cover_pillar");
    GScr_SP_LoadSingleAnimScriptOptional(inst, &list->cover_prone, "cover_prone");
    GScr_SP_LoadSingleAnimScriptOptional(inst, &list->cover_right, "cover_right");
    GScr_SP_LoadSingleAnimScriptOptional(inst, &list->cover_stand, "cover_stand");
    GScr_SP_LoadSingleAnimScriptOptional(inst, &list->cover_wide_left, "cover_wide_left");
    GScr_SP_LoadSingleAnimScriptOptional(inst, &list->cover_wide_right, "cover_wide_right");
    GScr_SP_LoadSingleAnimScriptOptional(inst, &list->death, "death");
    GScr_SP_LoadSingleAnimScriptOptional(inst, &list->grenade_return_throw, "grenade_return_throw");
    GScr_SP_LoadSingleAnimScriptOptional(inst, &list->init, "init");
    GScr_SP_LoadSingleAnimScriptOptional(inst, &list->pain, "pain");
    GScr_SP_LoadSingleAnimScriptOptional(inst, &list->react, "react");
    GScr_SP_LoadSingleAnimScriptOptional(inst, &list->move, "move");
    GScr_SP_LoadSingleAnimScriptOptional(inst, &list->scripted, "scripted");
    GScr_SP_LoadSingleAnimScriptOptional(inst, &list->stop, "stop");
    GScr_SP_LoadSingleAnimScriptOptional(inst, &list->grenade_cower, "grenade_cower");
    GScr_SP_LoadSingleAnimScriptOptional(inst, &list->flashed, "flashed");
    g_scr_data_sp.scriptedInit = GScr_LoadScriptAndLabel(inst, "animscripts/scripted", "init", 0);
    g_scr_data_sp.initModeSp = GScr_LoadScriptAndLabel(inst, "animscripts/init_mode_sp", "init", 0);
}

// zombies: SP 0x007EE9E0 (load) / 0x007EF190 (set): the zombie dog species' animscripts.
static void GScr_SP_LoadZombieDogAnimScripts(scriptInstance_t inst)
{
    AnimScriptList *list = &g_scr_data_sp.zombieDogAnim;

    GScr_SP_LoadSingleAnimScript(inst, &list->combat, "zombie_dog_combat");
    GScr_SP_LoadSingleAnimScript(inst, &list->death, "zombie_dog_death");
    GScr_SP_LoadSingleAnimScript(inst, &list->init, "zombie_dog_init");
    GScr_SP_LoadSingleAnimScript(inst, &list->pain, "zombie_dog_pain");
    GScr_SP_LoadSingleAnimScript(inst, &list->move, "zombie_dog_move");
    GScr_SP_LoadSingleAnimScript(inst, &list->scripted, "zombie_dog_scripted");
    GScr_SP_LoadSingleAnimScript(inst, &list->stop, "zombie_dog_stop");
    GScr_SP_LoadSingleAnimScript(inst, &list->flashed, "zombie_dog_flashed");
    GScr_SP_LoadSingleAnimScript(inst, &list->turn, "zombie_dog_turn");
}

static void *__cdecl GScr_SP_HunkNameAlloc(int size)
{
    return Hunk_AllocLow(size, "GScr_AnimscriptAlloc", 5);
}

// zombies: SP 0x005EFBB0 (load) / 0x008071F0 (set): walk the map's entity string. Each new
// actor_<aitype> classname loads aitype/<aitype>::{main, precache, spawner} (all required) into an
// AITypeScript kept as hunk data type 0 under the aitype name, which is where KB's
// Actor_FinishSpawning (actor_mp.cpp) looks it up. Each node_negotiation_begin loads
// animscripts/traverse/<animscript>::main as hunk data type 1, the table KB's
// GScr_SetScriptsForPathNode and the pathnode loader read. The human/dog branch (non-zombiemode
// "dog"/"hound" aitypes load the dog animscripts) and the misc_turret / misc_mg42 branch (the turret
// weapon's animscript) are not ported: Five has neither.
static void GScr_SP_LoadEntityScripts(scriptInstance_t inst)
{
    static SpawnVar spawnVar;
    const char *classname;
    const char *animscript;
    char filename[64];
    AITypeScript *typeScript;
    int func;

    while ( G_ParseSpawnVars(&spawnVar) )
    {
        G_SpawnString(&spawnVar, "classname", "", &classname);
        if ( !strncmp(classname, "actor_", 6) )
        {
            const char *aitype = classname + 6;

            if ( Hunk_FindDataForFile(0, aitype) )
                continue;
            typeScript = (AITypeScript *)Hunk_AllocLow(sizeof(AITypeScript), "GScr_SP_LoadEntityScripts", 5);
            Com_sprintf(filename, sizeof(filename), "aitype/%s", aitype);
            typeScript->main = GScr_LoadScriptAndLabel(inst, filename, "main", 1);
            typeScript->precache = GScr_LoadScriptAndLabel(inst, filename, "precache", 1);
            typeScript->spawner = GScr_LoadScriptAndLabel(inst, filename, "spawner", 1);
            Hunk_SetDataForFile(0, aitype, typeScript, GScr_SP_HunkNameAlloc);
        }
        else if ( !I_stricmp(classname, "node_negotiation_begin") )
        {
            G_SpawnString(&spawnVar, "animscript", "", &animscript);
            if ( !*animscript || Hunk_FindDataForFile(1, animscript) )
                continue;
            Com_sprintf(filename, sizeof(filename), "animscripts/traverse/%s", animscript);
            func = GScr_LoadScriptAndLabel(inst, filename, "main", 1);
            Hunk_SetDataForFile(1, animscript, (void *)func, GScr_SP_HunkNameAlloc);
        }
        else if ( !I_stricmp(classname, "misc_mg42") || !I_stricmp(classname, "misc_turret") )
        {
            Com_PrintWarning(24, "zombies: turret animscripts are not ported (%s)\n", classname);
        }
    }
    G_ResetEntityParsePoint();
}

// zombies: GScr_LoadScripts, SP 0x00580370, after codescripts/delete and codescripts/struct.
void GScr_SP_LoadScripts(scriptInstance_t inst)
{
    char filename[64];
    const char *mapname;

    memset(&g_scr_data_sp, 0, sizeof(g_scr_data_sp));

    GScr_SP_LoadConsts(inst);

    // SP: zombiemode loads the zombie and zombie dog lists (0x007EE8B0, 0x007EE9E0) instead of the
    // human list (0x007EE3B0, the 23 human scripts, animscripts/scripted and animscripts/init_mode_sp);
    // the test is zombiemode itself (SP GScr_LoadScripts 0x00580370). The SP front end takes the human list.
    if ( G_SP_IsZombieMode() )
    {
        GScr_SP_LoadZombieAnimScripts(inst);
        GScr_SP_LoadZombieDogAnimScripts(inst);
    }
    else
    {
        GScr_SP_LoadHumanAnimScripts(inst);
    }

    // SP: maps/<mapname>::main, optional at load (GScr_SetLevelMainHandle 0x007EEBB0 leaves the
    // handle 0 when the label is missing, and for a mapname starting with "mp/" or "mp\").
    mapname = Dvar_GetString("mapname");
    g_scr_data.levelscript = 0;
    if ( I_strncmp(mapname, "mp/", 3) && I_strncmp(mapname, "mp\\", 3) )
    {
        Com_sprintf(filename, sizeof(filename), "maps/%s", mapname);
        if ( CM_Mapkit_Empty() ) // mod: mapkit empty base - the layout's level main (mods/mapkit), not the retail map's
            Com_sprintf(filename, sizeof(filename), "maps/mapkit/mapkit_main");
        g_scr_data.levelscript = GScr_LoadScriptAndLabel(inst, filename, "main", 0);
    }

    // mod: with bo1_mod_zones (fs_game mods only), maps/mod_zones/<zone>::main of each extra zone when the mod has
    // that script; Scr_LoadLevel runs it after the level main. Scripts that reference another map's scripts (e.g.
    // zombie_temple's maps/_zombiemode_ai_napalm) are reached only from there, so the mod still compiles without the zone.
    extern int g_modZoneLevelScripts[BO1_MOD_ZONES_MAX];
    for ( int z = 0; z < BO1_MOD_ZONES_MAX; ++z )
    {
        char zone[64];
        g_modZoneLevelScripts[z] = 0;
        if ( DB_GetModExtraZoneName(z, zone, sizeof(zone)) )
        {
            Com_sprintf(filename, sizeof(filename), "maps/mod_zones/%s", zone);
            g_modZoneLevelScripts[z] = GScr_LoadScriptAndLabel(inst, filename, "main", 0);
            Com_Printf(24, "mod: zone script %s::main handle %d\n", filename, g_modZoneLevelScripts[z]);
        }
    }

    GScr_SP_LoadEntityScripts(inst);
}

// zombies: SP 0x0049CC70, first loop. Runs in G_InitGame after Path_InitPaths, before the actors'
// aitype main (Actor_FinishSpawningAll) and before the level main. SP eType 0x10 / 0x11 are KB's
// ET_ACTOR / ET_ACTOR_SPAWNER.
void G_SP_RunAITypeLoadScripts()
{
    int entnum;
    gentity_s *ent;
    const char *classname;
    AITypeScript *typeScript;
    unsigned int hThread;

    for ( entnum = 0; entnum < G_EntEnd(); entnum = G_EntNext(entnum) )
    {
        ent = &g_entities[entnum];
        if ( !ent->r.inuse )
            continue;
        if ( ent->s.eType != ET_ACTOR && ent->s.eType != ET_ACTOR_SPAWNER )
            continue;
        classname = SL_ConvertToString(ent->classname, SCRIPTINSTANCE_SERVER);
        typeScript = (AITypeScript *)Hunk_FindDataForFile(0, classname + 6);
        if ( !typeScript )
        {
            Com_Error(ERR_DROP, "zombies: no aitype script loaded for '%s'", classname);
            return;
        }
        if ( ent->s.eType == ET_ACTOR_SPAWNER )
        {
            hThread = Scr_ExecEntThread(ent, typeScript->spawner, 0);
            Scr_FreeThread(hThread, SCRIPTINSTANCE_SERVER);
        }
        if ( typeScript->precache )
        {
            hThread = Scr_ExecThread(SCRIPTINSTANCE_SERVER, typeScript->precache, 0);
            Scr_FreeThread(hThread, SCRIPTINSTANCE_SERVER);
            typeScript->precache = 0;
        }
    }
}

// zombies: Scr_StartLevelMain, SP 0x004B7F80.
void Scr_SP_StartLevelMain()
{
    unsigned int hThread;

    if ( !g_scr_data.levelscript )
        return;
    if ( g_scr_data_sp.gametypeMain )
    {
        hThread = Scr_ExecThread(SCRIPTINSTANCE_SERVER, g_scr_data_sp.gametypeMain, 0);
        Scr_FreeThread(hThread, SCRIPTINSTANCE_SERVER);
    }
    hThread = Scr_ExecThread(SCRIPTINSTANCE_SERVER, g_scr_data.levelscript, 0);
    Scr_FreeThread(hThread, SCRIPTINSTANCE_SERVER);
    // mod: then the bo1_mod_zones zone scripts (G_SP_LoadLevelScripts), as Scr_LoadLevel does on the MP path; zombie
    // levels start here, not in Scr_LoadLevel, so without this the zone scripts never ran (L33: napalm module silent)
    extern int g_modZoneLevelScripts[BO1_MOD_ZONES_MAX];
    for ( int z = 0; z < BO1_MOD_ZONES_MAX; ++z )
    {
        if ( g_modZoneLevelScripts[z] )
        {
            hThread = Scr_ExecThread(SCRIPTINSTANCE_SERVER, g_modZoneLevelScripts[z], 0);
            Scr_FreeThread(hThread, SCRIPTINSTANCE_SERVER);
        }
    }
}

// zombies: SP 0x0054BBB0, run by the "mlvl" client command (SP ClientCommand 0x004AF8C0) that the front end's
// "uiscript sendMenuNotify" sends (KB UI_RunMenuScript): exactly three arguments, else a print (SP .rdata
// 0x009BBC24); then CodeCallback_MenuMessage(argv 1, argv 2) with no return value kept.
void G_SP_Cmd_MenuLevel_f()
{
    char arg[1024];
    unsigned int hThread;
    const int argc = SV_Cmd_Argc();

    if ( argc != 3 )
    {
        Com_Printf(15, "Wrong number of args. Found %i expecting: 3\n", argc);
        return;
    }
    SV_Cmd_ArgvBuffer(2, arg, sizeof(arg));
    Scr_AddString(arg, SCRIPTINSTANCE_SERVER);
    // SP 0x0054BBEC: argv 1, or "" (0x009DD354) when argc <= 1 (unreachable after the check above).
    SV_Cmd_ArgvBuffer(1, arg, sizeof(arg));
    Scr_AddString(arg, SCRIPTINSTANCE_SERVER);
    hThread = Scr_ExecThread(SCRIPTINSTANCE_SERVER, g_scr_data_sp.menuMessage, 2);
    Scr_FreeThread(hThread, SCRIPTINSTANCE_SERVER);
}

// zombies: SP 0x005687F0.
void G_SP_FindActorAnimTree()
{
    g_actorAnimsSP = Scr_FindAnimTree(SCRIPTINSTANCE_SERVER, "generic_human").anims;
    if ( !g_actorAnimsSP )
        Com_Error(ERR_DROP, "Could not find animation tree '%s'", "generic_human");
}

XAnim_s *G_SP_GetActorAnims()
{
    return g_actorAnimsSP;
}
