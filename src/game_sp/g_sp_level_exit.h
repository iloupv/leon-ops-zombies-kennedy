#pragma once

// zombies: SP level-exit state and its per-frame check (lane x2). KB's level_locals_t keeps its MP
// layout, so the SP fields live here. Reset per level from G_SP_ResetEntityBuiltins (zombiemode).
struct sp_level_exit_t
{
    int exitTime;          // SP 0x01C0700C: absolute level.time at which the level exits (0 = none)
    int loadGameTime;      // SP 0x01C07010: Sys_Milliseconds deadline for "loadgame_continue" (0 = none)
    int changeLevel;       // SP 0x01C07014: changelevel requested, the exit starts next frame
    int missionSuccess;    // SP 0x01C07018
    int loadGameContinue;  // SP 0x01C0701C: set by missionfailed (not this lane)
    int changeLevelDelay;  // SP 0x01C07020: exit delay in ms from changelevel's third argument
    char cinematic[64];    // SP 0x01C07024: only ever filled by a save restore in SP; empty otherwise
    int savePersist;       // SP 0x01C0532C: changelevel's persistent flag
    char nextMap[64];      // SP 0x01C018B0
    int forceLevelEnd;     // SP 0x01C08AC8: forcelevelend
};

extern sp_level_exit_t g_spLevelExit;

// Level reset plus the SP registrations of g_reloading / g_changelevel_time.
void G_SP_ResetLevelExit();
// SP 0x0041DE40, called from G_RunFrame (SP 0x0045AB15) unless g_gametype is "vs".
void G_SP_CheckLevelExit();
// SP 0x007FBBB0 / 0x007FBCD0 / 0x00804AB0 (the builtins read these, the table registers them).
void G_SP_ChangeLevel();

extern const struct dvar_s *g_reloading;
extern const struct dvar_s *g_changelevel_time;
