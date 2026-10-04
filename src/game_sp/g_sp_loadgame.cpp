#include "g_sp_loadgame.h"
#include "g_sp_savegame.h"
#include "g_sp_levelstart.h"
#include "g_sp_level_exit.h"
#include <qcommon/cmd.h>
#include <qcommon/common.h>
#include <universal/com_files.h>
#include <universal/dvar.h>
#include <universal/q_shared.h>
#include <cstring>
#include <server/sv_game.h>
#include <server_mp/sv_main_mp.h>
#include <client_mp/sv_client_mp.h>
#include <qcommon/threads.h>
#include <clientscript/cscr_vm.h>
#include <clientscript/cscr_variable.h>
#include <win32/win_main.h>
#include <win32/win_gamerprofile.h>
#include <client/client.h>
#include <client/cl_keys.h>
#include <qcommon/com_clients.h>
#include <ui/ui_main.h>
#include <ui/ui_shared.h>
#include <game_mp/g_main_mp.h>
#include <win32/win_shared.h>

sp_loadgame_t g_spLoadGame;
int g_spSaveMapChecksum;

bool SP_UseDummySaveName()
{
    // zombies: SP 0x004E9C10.
    const dvar_s *fsGame = Dvar_FindVar("fs_game");
    if (fsGame && fsGame->current.string[0])
        return true;
    return Dvar_GetBool("arcademode") || Dvar_GetBool("onlinegame") || Dvar_GetBool("zombiemode")
        || Dvar_GetBool("credits_active");
}

bool SP_MemorySaveMatches(const char *name, int mapChecksum)
{
    // zombies: SP 0x00599F10 reads the committed memory save's header (buffer 0x01D04BA0 + 0x1004C:
    // +0x004 map checksum, +0x3E0 save name).
    const SPMemorySaveHeader *header = SP_GetCommittedMemorySave();
    if (!header)
        return false;
    if (SP_UseDummySaveName() && header->filename[0] && mapChecksum == header->mapChecksum)
        return true;
    if (!header->filename[0])
        return false;
    return !strncmp(name, header->filename, strlen(header->filename)) && mapChecksum == header->mapChecksum;
}

bool SP_SaveExists(const char *name)
{
    // zombies: SP 0x00484D50. The SP header is 0x498 bytes; only its first word, saveVersion, is read.
    enum { SP_SAVE_HEADER_SIZE = 0x498, SP_SAVE_VERSION = 0x134 };
    if (!name[0])
    {
        Com_Printf(0, "SaveExists(\"%s\"): returning false, !savename[0]\n", name);
        return false;
    }
    int f = 0;
    FS_FOpenFileRead(name, &f);
    if (!f)
        return false;
    unsigned char header[SP_SAVE_HEADER_SIZE];
    const unsigned int read = FS_Read(header, sizeof(header), f);
    FS_FCloseFile(f);
    if (read != sizeof(header))
        return false;
    int saveVersion;
    memcpy(&saveVersion, header, sizeof(saveVersion));
    if ((unsigned int)(saveVersion - 0x133) <= 1)
        return true;
    if (!saveVersion)
        Com_Printf(0, "SaveExists(\"%s\"): returning false, header.saveVersion == 0, likely doesn't exist\n", name);
    else
        Com_Printf(0, "SaveExists(\"%s\"): returning false, header.saveVersion %i != SAVE_VERSION (%i)\n", name,
            saveVersion, SP_SAVE_VERSION);
    return false;
}

void SP_ClearLoadGame()
{
    // zombies: SP 0x00512700.
    g_spLoadGame.saveName[0] = 0;
    g_spLoadGame.loadPending = 0;
    g_spLoadGame.loadArg = 0;
}

static bool s_spTemporaryFastRestart;

bool SP_TakeTemporaryFastRestart()
{
    const bool pending = s_spTemporaryFastRestart;
    s_spTemporaryFastRestart = false;
    return pending;
}

static bool SP_TemporaryReloadMap()
{
    // TEMPORARY (not SP): while the memory payload is incomplete, Restart starts a fresh
    // game on the current map. Replace with the SP 0x0087D030 restore when it is complete.
    // bo1_restart_full 0: MP-style fast restart of the same map (SV_MapRestart(1): the game VM
    // and scripts re-run from scratch, the zones stay loaded); 1: full `map` reload.
    if (!Dvar_GetBool("sv_running") || Dvar_GetBool("bo1_restore_partial"))
        return false;
    const char *mapName = Dvar_GetString("mapname");
    if (!mapName[0])
        return false;
    SP_ClearLoadGame();
    g_spLevelExit.loadGameTime = 0;
    g_spLevelExit.loadGameContinue = 0;
    if (!Dvar_GetBool("bo1_restart_full"))
    {
        Com_Printf(15, "loadgame: TEMPORARY fast map restart: %s\n", mapName);
        // The code time scale (missionfailed's 'X' lerp leaves 0.1) is reset to 1 by SP SV_SpawnServer
        // (SP 0x0050F05B) and CL_InitCGame (SP 0x0051B0B3); a fast restart runs neither.
        Com_SetTimeScale(1.0f);
        // SP's restart reloads through CL_InitCGame, whose CG_Init ends with Dvar_SetIntByName("cl_paused", 0)
        // (SP 0x0054C663..0x0054C66A). A fast restart runs no CG_Init, so Restart Level from the pause menu
        // (popup_restart_warning: fast_restart, then close pausedmenu while the client is not CA_ACTIVE) left
        // cl_paused / sv_paused 1 and the restarted level frozen (x6 c31: level.time stuck at 13850).
        Dvar_SetIntByName("cl_paused", 0);
        // G_InitGame(restart 1) skips SP's not-a-restart Dvar_ResetDvars(0x1000) (SP 0x0051E98A), which is
        // what clears missionfailed's hud_missionFailed; without it the zombie HUD menus stay hidden.
        // Same order as there: keep g_gameskill across the reset (SP 0x0051E977 / 0x0051E9B7).
        const dvar_s *gameskill = Dvar_FindVar("g_gameskill");
        const int savedGameskill = gameskill ? gameskill->current.integer : 0;
        Dvar_ResetDvars(0x1000u, DVAR_SOURCE_INTERNAL);
        if (gameskill)
            Dvar_SetInt(const_cast<dvar_s *>(gameskill), savedGameskill);
        s_spTemporaryFastRestart = true;
        Cbuf_AddText(0, "fast_restart\n");
        return true;
    }
    Com_Printf(15, "loadgame: TEMPORARY fresh map reload: %s\n", mapName);
    Cbuf_AddText(0, va(Dvar_GetBool("sv_cheats") ? "devmap %s\n" : "map %s\n", mapName));
    return true;
}

void SP_LoadGameContinue_f()
{
    // zombies: SP 0x00545CE0.
    if (!G_SP_IsSPLevel())
        return;
    Sys_HeadlessTimeline("loadgame_continue");
    if (SP_TemporaryReloadMap())
        return;
    const char *name;
    if (SP_UseDummySaveName())
    {
        name = "dummy";
    }
    else
    {
        const dvar_s *lastSave = Dvar_FindVar("sv_lastSaveGame");
        name = lastSave ? lastSave->current.string : "";
    }
    I_strncpyz(g_spLoadGame.saveName, name, sizeof(g_spLoadGame.saveName));
    if (!SP_MemorySaveMatches(g_spLoadGame.saveName, g_spSaveMapChecksum)
        && !SP_SaveExists(g_spLoadGame.saveName))
    {
        g_spLoadGame.saveName[0] = 0;
        g_spLoadGame.loadPending = 0;
        g_spLoadGame.loadArg = 0;
        Com_Error(ERR_DROP, "\x15Unable to find save.");
        return;
    }
    // SP 0x00545D60: with no server running (a load from the menus) the loader runs at once and its
    // failure is the same error; in a running game SV_Frame (SP 0x00618D71) runs it next frame.
    if (!Dvar_GetBool("sv_running") && !SP_LoadGame_RunLoader())
    {
        SP_ClearLoadGame();
        Com_Error(ERR_DROP, "Unable to find save.");
    }
}

static bool SP_GetSaveMapName(char *saveName, char *mapName)
{
    // zombies: SP 0x0087C7F0 (edi = saveName[128], esi = mapName[256]): takes the staged name.
    I_strncpyz(saveName, g_spLoadGame.saveName, 128);
    g_spLoadGame.saveName[0] = 0;
    if (SP_MemorySaveMatches(saveName, g_spSaveMapChecksum))
    {
        I_strncpyz(mapName, SP_GetSaveBuffer(1)->header.mapName, 256);  // SP 0x005411A0(1) + 0x20
        // SP 0x005F5B40 keeps the save name in 0x01C76F18 (not read by anything ported yet).
        return true;
    }
    // SP 0x0087C85C reads the header of a save file (0x0049D940). Not ported: zombiemode never writes
    // save files (its saves use flags 2, memory only).
    return false;
}

static bool G_SP_CheckLoadGame(SPSaveGame *save)
{
    // zombies: SP 0x0046BD80. The map-checksum argument is unused by this function in the exe.
    Com_Printf(10, "=== G_LoadGame ===\n");
    // Not ported: 0x006DA090 clears the SP asynchronous save message. KB has no save thread.
    const SPMemorySaveHeader *header = &save->header;
    if (header->usesScriptChecksum && !G_SP_SaveScriptChecksumMatches(save))
        Com_Error(ERR_DROP, "\x15G_LoadGame: savegame '%s' was saved with different script files\n%s",
            header->filename, header->buildNumber);
    if (header->saveCheckSum)
        Com_Error(ERR_DROP, "\x15The save file has become corrupted.\n");
    if (!G_SP_LoadGame(save))
        return false;
    // Not ported: the successful-load tail 0x0046BE1C..0x0046BE76 (finish reader, random seed,
    // client usercmd/delta angles and world entity). Keep the partial restore behind the fallback.
    return false;
}

bool G_SP_LoadGameAfterInit(SPSaveGame *save)
{
    // zombies: SP 0x005A1130 -> 0x00534B40. In SP loading state 2 means a non-null save.
    // The success tail (client reset commands, g_reloading, frame residual) must wait for the VM
    // and entity payloads; do not advertise a completed load for the partial diagnostic.
    if (save)
    {
        G_SP_ClearSaveGameQueue();  // SP 0x004EF140
        if (!G_SP_CheckLoadGame(save))
            return false;
    }
    return false;
}

static void SP_RestartFromSave()
{
    // Brief/notes put the random seed in G_InitGame argument 3; the exe passes the map checksum
    // there ([0x02865E80], SP 0x008339D7). SP's arguments are (randomSeed, restart, mapChecksum,
    // savePersist, loadGame, save**), unlike KB's MP signature. Do not pass the MP seed as checksum.
    // zombies: SP 0x0087D030(eax 1; 0, 1), same-map loadGame with scripts retained. Until the
    // payload is complete, the opt-in diagnostic traverses the real initialization/reader chain
    // and then drops. Default Restart temporarily reloads the map.
    if (SP_TemporaryReloadMap())
        return;
    SP_ClearLoadGame();
    if (Dvar_GetBool("bo1_restore_partial"))
    {
        Com_SyncThreads();
        if (!Dvar_GetBool("sv_running"))
        {
            Com_Printf(0, "Server is not running.\n");
            return;
        }
        // SP 0x0087D147..0x0087D18D resets the time scales, advances the low server-ID nibble,
        // and calls the retained-script restart. Client resets / SP gametype setup and the
        // successful-load suffix remain to port before this can replace the default fallback.
        if (const dvar_s *scale = Dvar_FindVar("com_timescale"))
            Dvar_Reset((dvar_s *)scale, DVAR_SOURCE_INTERNAL);
        if (const dvar_s *scale = Dvar_FindVar("timescale"))
            Dvar_Reset((dvar_s *)scale, DVAR_SOURCE_INTERNAL);
        sv_serverId_value = ((sv_serverId_value + 1) & 0xF) + (sv_serverId_value & 0xF0);
        SPSaveGame *save = nullptr;
        Com_Printf(15, "loadgame: partial restore diagnostic entering G_InitGame(loadGame)\n");
        SV_RestartGameProgs(0, true, &save);
        if (!G_SP_LoadGameAfterInit(save))
        {
            Com_Printf(15, "loadgame: partial restore stopped before missing actor/sentient/remaining payload and live install; retaining diagnostic popup\n");
        }
    }
    // TEMPORARY (not SP): until the restore works end to end, end like the no-save path of
    // loadgame_continue (SP 0x00545CE0) so game over shows the "Unable to find save." popup with its
    // Exit button instead of waiting in intermission. Remove when 0x0087D030 is ported.
    Com_Error(ERR_DROP, "\x15Unable to find save.");
}

int SP_LoadGame_RunLoader()
{
    // zombies: SP 0x00479C00. Branch (a), the pending in-place load of the "loadgame" commands
    // (0x027F7064), is not reachable: none of those commands is registered in KB.
    if (!G_SP_IsSPLevel())
        return 0;
    if (g_spLevelExit.forceLevelEnd || !g_spLoadGame.saveName[0])
        return 0;
    char saveName[128];
    char mapName[256];
    if (!SP_GetSaveMapName(saveName, mapName))
        return 0;
    if (Dvar_GetBool("sv_running") && !I_stricmp(mapName, Dvar_GetString("mapname")))
    {
        SP_RestartFromSave();
        return 1;
    }
    // SP queues the map by the save name (0x00479CDE / 0x00479CE9).
    Cbuf_AddText(0, va(Dvar_GetBool("sv_cheats") ? "devmap %s\n" : "map %s\n", saveName));
    return 1;
}

// ---------------------------------------------------------------------------------------------
// zombies: SP's save device commands (SP registers both at 0x004fd23a.., client Cbuf_AddServerText_f
// plus the server handler). The SP front end's main menu runs select_save_device.

static const dvar_t *sv_lastSaveGame;
static const dvar_t *sv_saveDeviceAvailable;
static const dvar_t *sv_saveGameAvailable;
static const dvar_t *sv_saveGameNotReadable;

// SP 0x005e6c00: the "readingsavedevice" menu over the UI key catcher, ui_readingSaveDevice set.
static void SP_OpenReadingSaveDevice(int localClientNum)
{
    Key_SetCatcher(localClientNum, 0x10);
    Menus_OpenByName(localClientNum, &UI_GetInfo(localClientNum)->uiDC, "readingsavedevice");
    Dvar_SetBool((dvar_s *)ui_readingSaveDevice, true);
}

// SP 0x004e1440: ui_readingSaveDevice cleared, the menu closed.
static void SP_CloseReadingSaveDevice(int localClientNum)
{
    Dvar_SetBool((dvar_s *)ui_readingSaveDevice, false);
    Menus_CloseByName(localClientNum, &UI_GetInfo(localClientNum)->uiDC, "readingsavedevice");
}

// SP 0x004b92a0, server command "select_save_device". The PC exe has no device selection: with a
// signed-in profile, outside a running game, it flags the device available and sv_saveGameAvailable
// = the last save exists (SP 0x00484D50).
static void SP_SelectSaveDevice_f()
{
    const char *lastSaveGame = sv_lastSaveGame->current.string;
    int controllerIndex = Com_LocalClient_GetControllerIndex(0);
    if (Dvar_GetBool("onlinegame") || Dvar_GetBool("systemlinkparty") || Dvar_GetBool("systemlink"))
        return;
    Dvar_SetBool((dvar_s *)sv_saveGameAvailable, false);
    if (controllerIndex < 0 || !GamerProfile_IsProfileLoggedIn(controllerIndex))
        return;
    // SP 0x00684eb0: the map is a menu_ map or the front end; SP 0x00526450: client 0 is CA_ACTIVE.
    const char *mapName = Dvar_GetString("mapname");
    bool frontEnd = !I_strncmp(mapName, "menu_", 5) || !I_stricmp(mapName, "frontend");
    if (!frontEnd && CL_GetLocalClientConnectionState(0) == CA_ACTIVE)
    {
        Com_Printf(0, "select_save_device is not available while a game is running - use force_select_save_device instead\n");
        return;
    }
    SP_OpenReadingSaveDevice(0);
    if (sv_saveDeviceAvailable->current.enabled)
        Dvar_SetBool((dvar_s *)sv_saveGameAvailable, SP_SaveExists(lastSaveGame));
    Dvar_SetBool((dvar_s *)sv_saveDeviceAvailable, true);
    Dvar_SetBool((dvar_s *)sv_saveGameAvailable, SP_SaveExists(lastSaveGame));
    SP_CloseReadingSaveDevice(0);
}

// SP 0x0057ff40, server command "force_select_save_device".
static void SP_ForceSelectSaveDevice_f()
{
    if (GamerProfile_IsProfileLoggedIn(Com_LocalClient_GetControllerIndex(0)))
    {
        SP_OpenReadingSaveDevice(0);
        SP_CloseReadingSaveDevice(0);
    }
}

static cmd_function_s SP_SelectSaveDevice_f_VAR;
static cmd_function_s SP_SelectSaveDevice_f_VAR_SERVER;
static cmd_function_s SP_ForceSelectSaveDevice_f_VAR;
static cmd_function_s SP_ForceSelectSaveDevice_f_VAR_SERVER;

// mod: bo1_mod_quickrestart 1 (the launcher's "R restarts after game over"; 0 = off = retail): once the game is
// over (a player in intermission, pm_type 5, from end_game; or missionfailed's g_reloading), the reload key
// (CL_KeyEvent) and the console command bo1_quickrestart both come here and restart the same map through the
// game-over Restart path above (loadgame_continue -> SP_TemporaryReloadMap -> fast_restart: +set dvars such as
// the horde flags, noperks, max zombies and the start round are kept). Returns true when it restarted.
const dvar_s *SP_QuickRestartDvar()
{
    static const dvar_s *bo1_mod_quickrestart;
    if (!bo1_mod_quickrestart)
        bo1_mod_quickrestart = _Dvar_RegisterBool("bo1_mod_quickrestart", false, 0,
            "mod: after game over the reload key (or bo1_quickrestart) restarts the map (launcher)");
    return bo1_mod_quickrestart;
}

bool SP_QuickRestart(bool verbose)
{
    static unsigned int lastRestartMs;
    if (!SP_QuickRestartDvar()->current.enabled || !G_SP_IsSPLevel() || !Dvar_GetBool("sv_running"))
    {
        if (verbose)
            Com_Printf(15, "bo1_quickrestart: off (bo1_mod_quickrestart 0, or no zombies level running)\n");
        return false;
    }
    bool over = g_reloading && g_reloading->current.integer != 0;
    for (int i = 0; i < 64 && !over; ++i)
    {
        if (g_entities[i].client && g_entities[i].client->ps.pm_type == 5) // PM_INTERMISSION: end_game
            over = true;
    }
    if (!over)
    {
        if (verbose)
            Com_Printf(15, "bo1_quickrestart: the game is not over\n");
        return false;
    }
    // one restart per key press: the old level's intermission state lasts until the queued fast_restart runs
    if (lastRestartMs && Sys_Milliseconds() - lastRestartMs < 2000)
        return true;
    lastRestartMs = Sys_Milliseconds();
    Com_Printf(15, "bo1_quickrestart: restarting %s (horde_max_zombies '%s' horde_start_round '%s' noperks '%s' fs_game '%s')\n",
        Dvar_GetString("mapname"), Dvar_GetVariantString("horde_max_zombies"), Dvar_GetVariantString("horde_start_round"),
        Dvar_GetVariantString("noperks"), Dvar_GetVariantString("fs_game"));
    SP_LoadGameContinue_f();
    return true;
}

static void SP_QuickRestart_f()
{
    SP_QuickRestart(true);
}

static cmd_function_s SP_LoadGameContinue_f_VAR;
static cmd_function_s SP_QuickRestart_f_VAR;

void SP_RegisterLoadGameCommands()
{
    // zombies: SP registers it with the server operator commands (SP 0x0057DDED).
    static bool registered;
    if (registered)
        return;
    registered = true;
    _Dvar_RegisterBool("bo1_restore_partial", false, 0,
        "Exercise the incomplete SP memory restore, then drop to the temporary popup");
    _Dvar_RegisterBool("bo1_restart_full", false, 0,
        "TEMPORARY game-over Restart: 1 = full map reload, 0 = fast restart keeping the zones");
    _Dvar_RegisterInt("bo1_resave_ms", 0, 0, 0x7FFFFFFF, 0,
        "KB diagnostic: queue one more level-start save when level time reaches this (0 = off)");
    Cmd_AddCommandInternal("loadgame_continue", SP_LoadGameContinue_f, &SP_LoadGameContinue_f_VAR);
    SP_QuickRestartDvar(); // mod
    Cmd_AddCommandInternal("bo1_quickrestart", SP_QuickRestart_f, &SP_QuickRestart_f_VAR); // mod
    if (!bo1_zombies->current.enabled)
        return;
    // zombies: SP SV init dvars (SP 0x00698450..0x006984cb).
    sv_lastSaveGame = _Dvar_RegisterString("sv_lastSaveGame", "", 1, "");
    sv_saveDeviceAvailable = _Dvar_RegisterBool("sv_saveDeviceAvailable", true, 0x44, "");
    sv_saveGameAvailable = _Dvar_RegisterBool("sv_saveGameAvailable", false, 0x44, "");
    sv_saveGameNotReadable = _Dvar_RegisterBool("sv_saveGameNotReadable", false, 0x44, "");
    Cmd_AddCommandInternal("select_save_device", Cbuf_AddServerText_f, &SP_SelectSaveDevice_f_VAR);
    Cmd_AddServerCommandInternal("select_save_device", SP_SelectSaveDevice_f, &SP_SelectSaveDevice_f_VAR_SERVER);
    Cmd_AddCommandInternal("force_select_save_device", Cbuf_AddServerText_f, &SP_ForceSelectSaveDevice_f_VAR);
    Cmd_AddServerCommandInternal("force_select_save_device", SP_ForceSelectSaveDevice_f,
        &SP_ForceSelectSaveDevice_f_VAR_SERVER);
}
