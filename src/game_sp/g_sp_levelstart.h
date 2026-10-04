#pragma once

// zombies: the SP game module's script load and level start (P3), used when zombiemode is set.
// KB's MP flow (g_scr_main_mp.cpp GScr_LoadScripts, g_main_mp.cpp G_InitGame) calls into this file
// instead of loading the MP gametype / callback / animscript files.

#include <clientscript/cscr_main.h>
#include <game/actor_animapi.h>

// SP-only script handles, and the SP animscript lists KB's scr_data_t has no room for. KB's struct
// keeps its MP layout; the handles SP shares with MP go into g_scr_data (see GScr_SP_LoadConsts).
struct scr_data_sp_t
{
    int saveRestored;           // SP .data 0x01C79B30 maps/_callbacksetup::CodeCallback_SaveRestored
    int actorShouldReact;       // SP 0x01C79B5C CodeCallback_ActorShouldReact
    int disconnectedDuringLoad; // SP 0x01C87508 CodeCallback_DisconnectedDuringLoad
    int menuMessage;            // SP 0x01C87514 CodeCallback_MenuMessage
    int dec20Message;           // SP 0x01C87518 CodeCallback_Dec20Message
    int gametypeMain;           // SP 0x01C8750C maps/gametypes/<ui_gametype>::init, run before the level main
    int scriptedInit;           // SP 0x01C79B2C animscripts/zombie_scripted::init (zombiemode), animscripts/scripted::init (else)
    int initModeSp;             // SP 0x01C7AE74 animscripts/init_mode_sp::init (not zombiemode)
    AnimScriptList zombieAnim;    // SP 0x01C7A4E4, species 2 (animscripts/zombie_*)
    AnimScriptList zombieDogAnim; // SP 0x01C7A9A4, species 3 (animscripts/zombie_dog_*)
};

extern scr_data_sp_t g_scr_data_sp;

// zombiemode is set from the map name for zombie_* maps (Com_LoadLevelFastFiles).
bool G_SP_IsZombieMode();
// zombies: the SP game module runs this level: zombiemode, or the SP front end ("frontend", menu_*) booted with
// bo1_zombies (Com_IsSPLevel). The SP exe runs its game paths for every level; zombie-only rules stay on
// G_SP_IsZombieMode.
bool G_SP_IsSPLevel();

// SP registrations needed by level scripts; also restores MP g_speed flags on mode changes.
void G_SP_RegisterLevelDvars();
// SP 0x00640590: g_player_maxhealth 100 and the g_gameskill health / invulnerability dvars.
void G_SP_UpdateSkillDvars();

// GScr_LoadScripts, SP 0x00580370, from GScr_LoadConsts on (the codescripts/delete and
// codescripts/struct handles are loaded by the caller exactly as in MP).
void GScr_SP_LoadScripts(scriptInstance_t inst);

// SP 0x0049CC70, first loop: for every actor / actor spawner entity, run its aitype's `spawner`
// function on spawners and each aitype's `precache` function once.
void G_SP_RunAITypeLoadScripts();

// SP 0x005687F0 (called from G_LoadScripts 0x007E2F20 after BG_LoadAnim): the "generic_human"
// animtree every SP actor uses (.data 0x01C79B60), required. G_SP_GetActorAnims returns it.
void G_SP_FindActorAnimTree();
struct XAnim_s *G_SP_GetActorAnims();

// Scr_StartLevelMain, SP 0x004B7F80: the gametype script (if any), then maps/<mapname>::main,
// each as a thread on level, inside G_InitGame.
void Scr_SP_StartLevelMain();
// zombies: the "mlvl <a> <b>" client command (SP Cmd_MenuLevel_f 0x0054BBB0) -> CodeCallback_MenuMessage(a, b).
void G_SP_Cmd_MenuLevel_f();
