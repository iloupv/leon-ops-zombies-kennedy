#pragma once

// zombies: SP "continue from the last save" chain. After game over the level exit (g_sp_level_exit.cpp,
// SP 0x0041DE40) queues "loadgame_continue"; its handler (SP 0x00545CE0) stages the save name, and on
// the next server frame SV_Frame (SP 0x00618D71) hands a staged name to the loader (SP 0x00479C00).

struct sp_loadgame_t
{
    int loadArg;         // SP 0x027F7060: argument for the pending in-place load
    int loadPending;     // SP 0x027F7064: an in-place load was requested ("loadgame" family)
    char saveName[128];  // SP 0x027F7068: staged save name, cleared by the loader
    int restartCount;    // SP 0x027F76A8: fast restarts by 0x0087D030 (0 after a pending load); saved by G_SaveState
};

extern sp_loadgame_t g_spLoadGame;

// SP 0x02865E80: the loaded map's BSP checksum (stored by SV_SpawnServer, SP 0x005C31AB). A memory
// save only matches the map it was taken on.
extern int g_spSaveMapChecksum;

// SP 0x004E9C10: fs_game, arcademode, onlinegame, zombiemode or credits_active use the fixed save
// name "dummy" instead of sv_lastSaveGame.
bool SP_UseDummySaveName();
// SP 0x00599F10: the committed memory save matches this name (or any name when the dummy name is in
// use) and was taken on the map with this checksum.
bool SP_MemorySaveMatches(const char *name, int mapChecksum);
// SP 0x00484D50 "SaveExists": a readable save file with a supported SAVE_VERSION.
bool SP_SaveExists(const char *name);
// SP 0x00545CE0, console command "loadgame_continue".
void SP_LoadGameContinue_f();
// mod: bo1_mod_quickrestart; the reload key after game over and the command bo1_quickrestart.
const struct dvar_s *SP_QuickRestartDvar();
bool SP_QuickRestart(bool verbose);
// SP 0x00512700: forget any staged or pending load.
void SP_ClearLoadGame();
// TEMPORARY (not SP): true once after SP_TemporaryReloadMap queued its fast_restart.
bool SP_TakeTemporaryFastRestart();
void SP_RegisterLoadGameCommands();
// SP 0x00479C00, run by SV_Frame (SP 0x00618D7A) while the server runs: a staged save is loaded.
// Nonzero skips the server frame.
int SP_LoadGame_RunLoader();
// SP 0x005A1130 -> 0x00534B40 -> 0x0046BD80, after G_InitGame(loadGame).
// False until all payload sections and the post-load steps have been ported.
bool G_SP_LoadGameAfterInit(struct SPSaveGame *save);
