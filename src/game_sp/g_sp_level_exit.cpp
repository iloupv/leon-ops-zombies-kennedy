#include "g_sp_level_exit.h"
// zombies: lane x2 - SP level exit (changelevel / missionsuccess / forcelevelend and the per-frame
// check that acts on them). Ported from the SP exe; addresses in the comments.
#include <game_mp/g_main_mp.h>
#include <game_mp/g_utils_mp.h>
#include <clientscript/cscr_vm.h>
#include <clientscript/scr_const.h>
#include <server/sv_game.h>
#include <qcommon/cmd.h>
#include <universal/dvar.h>
#include <win32/win_shared.h>
#include <cstring>
#include <cfloat>
#include <cmath>

sp_level_exit_t g_spLevelExit;
const dvar_s *g_reloading;
const dvar_s *g_changelevel_time;

void G_SP_ResetLevelExit()
{
    memset(&g_spLevelExit, 0, sizeof(g_spLevelExit));
    // zombies: SP G_RegisterDvars registrations (0x007E0C8B g_reloading int 0, 0..4, flags 0x40;
    // 0x007E167F g_changelevel_time float 0, 0..FLT_MAX, flags 0x2000; both without a description).
    g_reloading = _Dvar_RegisterInt("g_reloading", 0, 0, 4, 0x40, "");
    g_changelevel_time = _Dvar_RegisterFloat("g_changelevel_time", 0.0f, 0.0f, FLT_MAX, 0x2000, "");
    // zombies: SP G_InitGame clears it for the new level (SP 0x0051E9CE).
    Dvar_SetInt((dvar_s *)g_reloading, 0);
}

// zombies: SP 0x0049B530, the next map name (64 chars).
static void G_SP_SetNextMap(const char *mapname)
{
    I_strncpyz(g_spLevelExit.nextMap, mapname, sizeof(g_spLevelExit.nextMap));
}

// zombies: SP 0x007E30B0, the exit starts: fade/music commands to the clients, then the deadline.
static void G_SP_BeginLevelExit()
{
    gentity_s *player = G_Find(0, offsetof(gentity_s, classname), scr_const.player);
    if (player->health <= 0 || g_reloading->current.integer)
        return;
    int delay = g_spLevelExit.changeLevelDelay;
    if (delay)
    {
        // SP sends 'U' (0x55) "1 250 <delay+750>"; KB's cgame has no consumer yet.
        SV_GameSendServerCommand(-1, SV_CMD_RELIABLE, va("%c 1 %i %i", 'U', 250, delay + 750));
        delay = g_spLevelExit.changeLevelDelay;
    }
    if (!g_spLevelExit.missionSuccess)
    {
        // SP sends 'q' (0x71) "0 <delay+1000>"; KB's cgame has no consumer yet.
        SV_GameSendServerCommand(-1, SV_CMD_RELIABLE, va("%c 0 %i\n", 'q', delay + 1000));
        delay = g_spLevelExit.changeLevelDelay;
    }
    g_spLevelExit.exitTime = level.time + delay + 1000;
    Dvar_SetInt((dvar_s *)g_reloading, 4);
}

// zombies: SP 0x00510690, the exit itself. SP runs "spmap"/"spdevmap"; KB (the MP exe) has no SP
// map commands, so the same choice runs its "map"/"devmap".
static void G_SP_ExitLevel()
{
    if (!g_spLevelExit.nextMap[0])
    {
        Cbuf_AddText(0, "disconnect\n");
        return;
    }
    if (Dvar_GetBool("sv_cheats"))
        Cbuf_AddText(0, va("devmap %s\n", g_spLevelExit.nextMap));
    else
        Cbuf_AddText(0, va("map %s\n", g_spLevelExit.nextMap));
}

// zombies: SP 0x0041DE40, from G_RunFrame (SP 0x0045AAF7) when g_gametype is not "vs".
void G_SP_CheckLevelExit()
{
    if (g_gametype && !I_stricmp(g_gametype->current.string, "vs"))
        return;
    if (g_spLevelExit.changeLevel)
    {
        g_spLevelExit.changeLevel = 0;
        G_SP_BeginLevelExit();
    }
    if (g_spLevelExit.exitTime && g_spLevelExit.exitTime < level.time)
    {
        g_spLevelExit.exitTime = 0;
        if (g_spLevelExit.missionSuccess)
        {
            g_spLevelExit.missionSuccess = 0;
            if (g_spLevelExit.cinematic[0])
            {
                Cbuf_AddText(0, va("cinematic %s 0\n", g_spLevelExit.cinematic));
                g_spLevelExit.cinematic[0] = 0;
            }
            else
            {
                // SP '1' (0x31); KB's cgame has no consumer yet.
                SV_GameSendServerCommand(-1, SV_CMD_RELIABLE, va("%c", '1'));
                g_spLevelExit.cinematic[0] = 0;
            }
        }
        else if (g_spLevelExit.loadGameContinue)
        {
            g_spLevelExit.loadGameContinue = 0;
            Cbuf_AddText(0, "loadgame_continue\n");
        }
        else
        {
            G_SP_ExitLevel();
        }
    }
    if (g_spLevelExit.loadGameTime && (int)(g_spLevelExit.loadGameTime - Sys_Milliseconds()) < 0)
    {
        g_spLevelExit.loadGameTime = 0;
        Cbuf_AddText(0, "loadgame_continue\n");
    }
}

// SP: seconds * 1000.0f stored as a float, + 9.313225746154785e-10 (double 0x009D3F88), then fistp,
// which rounds to nearest in the default FPU mode (lrint), not a C truncation.
static int G_SP_MsecFromSeconds(float seconds)
{
    float msec = seconds * 1000.0f;
    return (int)lrint((double)msec + 9.313225746154785e-10);
}

// zombies: changelevel (SP 0x007FBBB0). changelevel(mapname [, persistent [, exitTime seconds]]).
void G_SP_ChangeLevel()
{
    gentity_s *player = G_Find(0, offsetof(gentity_s, classname), scr_const.player);
    if (player->health <= 0 || g_reloading->current.integer)
        return;
    unsigned int numParam = Scr_GetNumParam(SCRIPTINSTANCE_SERVER);
    if (numParam != 1)
    {
        if (numParam != 2)
        {
            g_spLevelExit.changeLevelDelay = G_SP_MsecFromSeconds((float)Scr_GetFloat(2, SCRIPTINSTANCE_SERVER));
            if (g_spLevelExit.changeLevelDelay < 0)
                // SP passes parameter index 1 here although the time is the third argument.
                Scr_ParamError(1, "exitTime cannot be negative", SCRIPTINSTANCE_SERVER);
        }
        g_spLevelExit.savePersist = Scr_GetInt(1, SCRIPTINSTANCE_SERVER);
    }
    const char *mapname = Scr_GetString(0, SCRIPTINSTANCE_SERVER);
    if (Scr_GetNumParam(SCRIPTINSTANCE_SERVER) < 3 && g_changelevel_time->current.value >= 0.0f)
        g_spLevelExit.changeLevelDelay = G_SP_MsecFromSeconds(g_changelevel_time->current.value);
    g_spLevelExit.changeLevel = 1;
    G_SP_SetNextMap(mapname);
}
