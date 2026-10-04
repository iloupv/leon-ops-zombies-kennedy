// zombies: client ends of SP server commands that BO1Zombies's MP CG_DeployServerCommand did not
// handle. Each is the body of a case of SP's CG_DeployServerCommand (SP 0x0078daf0; its jump table is
// indexed by the command letter through the byte table at 0x0078e930). They are called only from
// zombiemode-gated cases in cg_servercmds_mp.cpp.
#include "cg_sp_client_ext.h"
#include <cgame_mp/cg_local_mp.h>
#include <qcommon/cmd.h>
#include <qcommon/common.h>
#include <qcommon/com_clients.h>
#include <bgame/bg_misc.h>
#include <bgame/bg_weapons.h>
#include <DynEntity/DynEntity_client.h>
#include <gfx_d3d/r_cinematic.h>
#include <live/live_steam.h>
#include <universal/q_shared.h>
#include <cgame/cg_scr_main.h>
#include <clientscript/cscr_vm.h>
#include <clientscript/scr_const.h>
#include <cgame_mp/cg_scr_main_mp.h>
#include <client/splitscreen.h>
#include <sound/snd_public_async.h>
#include <win32/win_shared.h>
#include <client_mp/cl_cgame_mp.h>
#include <ui/ui_main.h>
#include <cgame_mp/cg_draw_mp.h>
#include <physics/rope.h>
#include <ui_mp/ui_main_mp.h>

// zombies: 'F' "<name>" <ms> = SP CG_ConfigString_VisionSetLastStand (SP 0x005b5ce0). SP reads the
// same two tokens from its configstring 0xc27 under the key of its client number; KB's 0xc27 is a
// head icon, so the server (visionsetlaststand, x1) sends them to the keyed client as this command.
// The name is copied into the last-stand channel's name (SP cg+0xcc720, 64 bytes), and that copy is
// what the lerp loads.
void CG_SP_VisionSetLastStandCommand(int localClientNum)
{
    CgSPClientExt *ext = CG_SP_GetClientExt(localClientNum);
    char *name = ext->visionName[SP_VISIONSET_LASTSTAND - SP_VISIONSET_FIRST];
    I_strncpyz(name, Cmd_Argv(1), 64);
    int duration = atoi(Cmd_Argv(2));
    CG_SP_VisionSetStartLerp(localClientNum, SP_VISIONSET_LASTSTAND, VISIONSETLERP_TO_SMOOTH, name, duration);
}

// zombies: '}' hideviewmodel (SP 0x0078e512): set cg+0xa9d44, then SP 0x0052a0e0 on the predicted
// player state (PM_Weapon_StateEnd 0x00767130 = KB's PM_Weapon_Idle, as the server half uses).
void CG_SP_HideViewModelCommand(int localClientNum)
{
    cg_s *cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    CG_SP_GetClientExt(localClientNum)->hideViewModel = 1;
    PM_Weapon_Idle(&cgameGlob->predictedPlayerState);
}

// zombies: '{' showviewmodel (SP 0x0078e501): clear cg+0xa9d44.
void CG_SP_ShowViewModelCommand(int localClientNum)
{
    CG_SP_GetClientExt(localClientNum)->hideViewModel = 0;
}

// zombies: SP 0x0065b490. The refdef's double-vision record (SP cg+0x8c410 = KB refdef.doubleVision)
// gets a per-ms step toward the new target. SP writes local client 0's cg regardless of the argument.
void CG_SP_SetDoubleVision(int localClientNum, int duration, float value)
{
    GfxDoubleVision *dv = &CG_GetLocalClientGlobals(localClientNum)->refdef.doubleVision;
    dv->deltaPerMS = (value - dv->cur) / (float)duration;
    dv->targ = value;
}

// zombies: '4' <ms> <value> setdoublevision (SP 0x0078d850).
void CG_SP_DoubleVisionCommand(int localClientNum)
{
    int duration = atoi(Cmd_Argv(1));
    float value = (float)atof(Cmd_Argv(2));
    CG_SP_SetDoubleVision(localClientNum, duration, value);
}

// zombies: SP 0x00793b40, from CG_DrawActiveFrame (SP 0x005c3b9a) before the scene depth of field,
// when the demo movie camera is off. The current amount steps toward the target (clamped when it
// passes it); while it is above 0 a slowly wandering angle gives the direction and the blur is
// sin(time) scaled by the amount. The angle and its target are function statics in SP (0x00c23d20,
// 0x00c23d24; the target is seeded once with rand).
void CG_SP_UpdateDoubleVision(int localClientNum)
{
    static bool s_targetSeeded; // SP guard bit 1 of 0x00c23d28
    static float s_angle;       // SP 0x00c23d20
    static float s_targetAngle; // SP 0x00c23d24

    cg_s *cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    GfxDoubleVision *dv = &cgameGlob->refdef.doubleVision;
    float cur = dv->cur;
    float targ = dv->targ;
    dv->direction[0] = 0.0f;
    dv->direction[1] = 0.0f;
    dv->direction[2] = 0.0f;
    dv->motionBlurMagnitude = 0.0f;
    if (cur != targ)
    {
        float next = (float)cgameGlob->frametime * dv->deltaPerMS + cur;
        dv->cur = next;
        if (next != targ && (targ > cur) != (targ > next))
            dv->cur = targ;
    }
    if (dv->cur > 0.0f)
    {
        float frac = (float)cgameGlob->frametime * 0.001f;
        if (!s_targetSeeded)
        {
            s_targetSeeded = true;
            s_targetAngle = (float)rand() * 20.0f * 0.000030517578f - 10.0f;
        }
        float delta = s_targetAngle - s_angle;
        s_angle = delta * frac + s_angle;
        if (fabs(delta) < 0.1f)
        {
            float spread = (float)rand() * 2.0f * 0.000030517578f - 1.0f;
            float scale = (float)rand() * 2.0f * 0.000030517578f + 1.0f;
            s_targetAngle = scale * spread + s_angle;
        }
        dv->direction[0] = (float)sin(s_angle);
        dv->direction[1] = (float)cos(s_angle);
        dv->motionBlurMagnitude = (float)sin((float)cgameGlob->time * 0.001f) * dv->cur;
    }
}

// zombies: '7' <leaderboard> <values...> uploadscore (SP 0x0078e2b5 -> 0x00674c20). SP gathers the
// integer arguments and writes them to a DemonWare leaderboard for the local controller, only when
// that controller is signed in online (SP 0x00407020 = LiveSteam_IsClientSignedInOnline); SP
// 0x00674400 (always false) is the other early out. The write itself (the SP leaderboard table at
// 0x00a65220 and the dwWriteStats task 0x005cd5b0) is not ported: this build is offline, so the
// command is consumed and stops at the signed-in test (KB's test takes no controller).
void CG_SP_UploadScoreCommand(int localClientNum)
{
    if (!LiveSteam_IsClientSignedInOnline())
        return;
    // SP: LB_EndOngoingTasks, then dwWriteStats(leaderboard = atoi(argv 1), argv 2.. as the row) -
    // not ported (see above).
}

// zombies: '>' cleanupspawneddynents (SP 0x0078e6a2 -> 0x0061c7d0).
void CG_SP_CleanupSpawnedDynEntsCommand(int localClientNum)
{
    DynEntCl_SP_CleanupSpawnedDynEnts();
}

// zombies: '<' <name> <flags> start3dcinematic (SP 0x0078e6b2): local client 0 only.
void CG_SP_Start3DCinematicCommand(int localClientNum)
{
    if (localClientNum)
        return;
    R_Cinematic_StartPlayback_SP(Cmd_Argv(1), atoi(Cmd_Argv(2)), 0.0f);
    Dvar_SetBool((dvar_s *)cg_cinematicFullscreen, 0);
}

// zombies: '.' stop3dcinematic (SP 0x0078e73f): local client 0 only (the same body as the client
// script's stopbink, SP 0x00581060).
void CG_SP_Stop3DCinematicCommand(int localClientNum)
{
    if (localClientNum)
        return;
    R_Cinematic_StopPlayback();
    Dvar_SetBool((dvar_s *)cg_cinematicFullscreen, 1);
}

// zombies: ';' <pause> pause3dcinematic (SP 0x0078e6fb): local client 0 only; 1 pauses as a script
// pause, 0 lifts it.
void CG_SP_Pause3DCinematicCommand(int localClientNum)
{
    if (localClientNum)
        return;
    if (atoi(Cmd_Argv(1)))
        R_Cinematic_Pause_SP(true);
    else
        R_Cinematic_Unpause_SP(true);
}

// ---------------------------------------------------------------------------------------------
// entity events whose SP client end is a client-script callback

// zombies: SP CScr_SetUniqueClientScripts (SP 0x00408ee0) stores these two handles:
// clientscripts/_callbacks::CodeCallback_PlayWeaponDeathEffects (SP 0x02ff839c) and
// clientscripts/_callbacks::CodeCallback_GibEvent (SP 0x00c20834). BO1Zombies's port of that setter
// (CScr_SetUniqueClientScripts_SP, cg_scr_main_mp.cpp) assigns both in zombiemode.
int cg_spScrPlayWeaponDeathEffects;
int cg_spScrGibEvent;

// zombies: EV_PLAY_WEAPON_DEATH_EFFECTS (SP event 0xc1 -> 0x005953e0): the callback runs on the
// entity the server named, with (localClientNum, weapon name, parameter).
void CG_SP_PlayWeaponDeathEffects(int localClientNum, int entityNum, unsigned int weapon, unsigned int parm)
{
    centity_s *cent = CG_GetEntity(localClientNum, entityNum);
    if (!cg_spScrPlayWeaponDeathEffects || !cent)
        return;
    if (!weapon)
        return;
    if (weapon >= BG_GetNumWeapons())
    {
        Com_Error(ERR_DROP, "\x15" "CScr_PlayWeaponDeathEffects: ent->weapon >= BG_GetNumWeapons()");
        return;
    }
    Scr_AddInt(parm, SCRIPTINSTANCE_CLIENT);
    Scr_AddString((char *)BG_WeaponName(weapon), SCRIPTINSTANCE_CLIENT);
    Scr_AddInt(localClientNum, SCRIPTINSTANCE_CLIENT);
    unsigned short id = CScr_ExecEntThread(cent, cg_spScrPlayWeaponDeathEffects, 3);
    Scr_FreeThread(id, SCRIPTINSTANCE_CLIENT);
}

// zombies: EV_GIB (SP event 0xca -> CG_GibEntity 0x00593e40, local client 0 only): the low byte of
// the parameter is the gib tag mask (tags 0..7, passed as an array), the rest the gib type
// (1 "freeze", 2 "up", else "normal").
void CG_SP_GibEntity(int localClientNum, centity_s *cent, unsigned int parm)
{
    if (!cg_spScrGibEvent)
        return;
    unsigned int mask = parm & 0xFFFF;
    unsigned int type = parm >> 8;
    Scr_MakeArray(SCRIPTINSTANCE_CLIENT);
    for (int i = 0; i < 8; ++i)
    {
        if (mask & (1u << i))
        {
            Scr_AddInt(i, SCRIPTINSTANCE_CLIENT);
            Scr_AddArray(SCRIPTINSTANCE_CLIENT);
        }
    }
    if (type == 1)
        Scr_AddConstString(cscr_const.freeze, SCRIPTINSTANCE_CLIENT);
    else if (type == 2)
        Scr_AddConstString(cscr_const.up, SCRIPTINSTANCE_CLIENT);
    else
        Scr_AddConstString(cscr_const.normal, SCRIPTINSTANCE_CLIENT);
    Scr_AddInt(localClientNum, SCRIPTINSTANCE_CLIENT);
    unsigned short id = CScr_ExecEntThread(cent, cg_spScrGibEvent, 3);
    Scr_FreeThread(id, SCRIPTINSTANCE_CLIENT);
}

// ---------------------------------------------------------------------------------------------
// the level-exit / mission-failed commands (sent by missionfailed and the level exit, g_sp_*)

// zombies: 'q' <in> <ms> (SP 0x0078df07): SND_FadeOut (SP 0x00505ca0, command 0xb) when the first
// argument is below the exe's 1.5288e-05, else SND_FadeIn (SP 0x0061ad30, command 0xa). SP reads no
// other argument.
void CG_SP_SoundFadeCommand(int localClientNum)
{
    if ( 1.5287900e-05 > atof(Cmd_Argv(1)) )
        SND_FadeOut();
    else
        SND_FadeIn();
}

// zombies: SP 0x00486190. The code time scale is forced to 1 in online / system-link games.
static void CG_SP_SetCodeTimeScale(float timescale)
{
    if ( Dvar_GetBool("systemlink") || Dvar_GetBool("onlinegame") )
        Com_SetTimeScale(1.0f);
    else
        Com_SetTimeScale(timescale);
}

// zombies: 'X' <ms> <from> <to> (SP 0x0078d930 -> 0x005da340): start a real-time lerp of the code time
// scale (missionfailed sends "X <g_deathDelay> 1 0.1"). SP writes local client 0's cg whatever the
// argument, and not for the "vs" game type.
void CG_SP_TimeScaleLerpCommand(int localClientNum)
{
    int duration = atoi(Cmd_Argv(1));
    float from = (float)atof(Cmd_Argv(2));
    float to = (float)atof(Cmd_Argv(3));
    if ( !strcmp(Dvar_GetString("ui_gametype"), "vs") )
        return;
    CgSPClientExt *ext = CG_SP_GetClientExt(0);
    int now = Sys_Milliseconds();
    ext->timeScaleLerpStart = now;
    ext->timeScaleLerpFrom = from;
    ext->timeScaleLerpEnd = now + duration;
    ext->timeScaleLerpTo = to;
}

// zombies: SP 0x00772540, the first call of SP's CG_Draw2DInternal (0x00655a50): runs the 'X' lerp.
void CG_SP_UpdateTimeScaleLerp(int localClientNum)
{
    CgSPClientExt *ext = CG_SP_GetClientExt(localClientNum);
    int now = Sys_Milliseconds();
    if ( !ext->timeScaleLerpStart || ext->timeScaleLerpStart >= now )
        return;
    if ( now < ext->timeScaleLerpEnd )
    {
        float frac = (float)(now - ext->timeScaleLerpStart) / (float)(ext->timeScaleLerpEnd - ext->timeScaleLerpStart);
        CG_SP_SetCodeTimeScale((1.0f - frac) * ext->timeScaleLerpFrom + ext->timeScaleLerpTo * frac);
        return;
    }
    float to = ext->timeScaleLerpTo;
    ext->timeScaleLerpStart = 0;
    ext->timeScaleLerpEnd = 0;
    if ( ext->deadQuoteTime > 0 )
    {
        // NOT PORTED: SP 0x0041ef70, a fade to black over cg_deathScreenFadeOutTime * 1000 ms. Reached only
        // after 'Y' (the dead quote), which BO1Zombies's client does not handle yet.
    }
    CG_SP_SetCodeTimeScale(to);
}

// zombies: ']' <ms> (SP 0x0078e235 -> 0x0078d520): settransported. Non-zero starts the transporter overlay
// for ms (CG_Transported, SP 0x006745d0), zero clears it (CG_ClearTransported, SP 0x004650f0).
void CG_SP_TransportedCommand(int localClientNum)
{
    int time = atoi(Cmd_Argv(1));
    if ( time )
        CG_Transported(localClientNum, time);
    else
        CG_ClearTransported(localClientNum);
}

// zombies: '!' <on> <ms> (SP 0x0078e390 -> 0x00505cc0): setwatersheeting. SP reads argv 2 before argv 1.
// Non-zero turns the script water sheeting on (SP 0x005b0440), zero clears it (SP 0x004f1da0).
void CG_SP_WaterSheetingCommand(int localClientNum)
{
    int duration = atoi(Cmd_Argv(2));
    int on = atoi(Cmd_Argv(1));
    if ( on )
        CG_SetScriptWaterSheeting(localClientNum, duration);
    else
        CG_ClearScriptWaterSheeting(localClientNum);
}

// zombies: '#' <sub> <rope> ... (SP 0x0078e4d9 -> 0x00668700, sub-codes 0x4e..0x57; SP's numbering is not
// BO1Zombies's EV_ROPE_* enum). Only 0x53 (ropesetflag, 0x00668aea -> Rope_SetFlag 0x005c1290, no index check)
// is ported; the rest are NOT PORTED.
void CG_SP_RopeCommand(int localClientNum)
{
    int sub = atoi(Cmd_Argv(1));
    int rope = atoi(Cmd_Argv(2));
    if (sub == 0x53)
        Rope_SetFlag(rope, atoi(Cmd_Argv(3)), atoi(Cmd_Argv(4)));
    else
        Com_Printf(14, "rope server command 0x%x: not ported (SP 0x00668700)\n", sub);
}

// zombies: '@' <id> (SP 0x0078e464 -> 0x0050e6b0): activateclientexploder / deactivateclientexploder.
// A positive id runs clientscripts/_callbacks::callback_activate_exploder, a negative one
// callback_deactivate_exploder, with abs(id).
void CG_SP_ClientExploderCommand(int localClientNum)
{
    int id = atoi(Cmd_Argv(1));
    if (CG_SP_VisualsEnabled())
        Com_Printf(14, "BO1_VISUAL exploder time=%d client=%d id=%d\n",
            CG_GetLocalClientGlobals(localClientNum)->time, localClientNum, id);
    if ( !CL_LocalClient_IsActive(localClientNum) )
        return;
    if ( !cg_scr_sp_data.activateExploder || !cg_scr_sp_data.deactivateExploder )
        return;
    Scr_AddInt(abs(id), SCRIPTINSTANCE_CLIENT);
    unsigned short thread = Scr_ExecThread(
        SCRIPTINSTANCE_CLIENT,
        id > 0 ? cg_scr_sp_data.activateExploder : cg_scr_sp_data.deactivateExploder,
        1);
    Scr_FreeThread(thread, SCRIPTINSTANCE_CLIENT);
}

// zombies: 'b' <index> = openmainmenu's client end (SP 0x0078d0a0, case 'b' of SP 0x0078daf0). The index is a
// script menu configstring (SP 0x9c7 + index; KB's script menus start at 2548, as CG_OpenScriptMenu reads them).
// "main" is the SP main menu: UI_SetActiveMenu with com_desiredMenu when set, else the default menu screen
// (SP 0x00569ee0: its main menu outside a lobby = KB's UI_GetMenuScreen), then com_desiredMenu = 0. Any other
// name opens that menu (SP 0x005c8fe0 = UI_OpenMenu).
void CG_SP_OpenMainMenuCommand(int localClientNum)
{
    unsigned int menuIndex = atoi(Cmd_Argv(1));
    if (menuIndex > 31)
    {
        Com_PrintError(14, "Server tried to open a bad script menu index: %i\n", menuIndex);
        Cbuf_AddText(localClientNum, va("cmd mr %i bad\n", menuIndex));
        return;
    }
    const char *menuName = CL_GetConfigString(menuIndex + 2548);
    if (!*menuName)
    {
        Com_PrintError(14, "Server tried to open a non-loaded script menu index: %i\n", menuIndex);
        Cbuf_AddText(localClientNum, va("cmd mr %i bad\n", menuIndex));
        return;
    }
    if (Dvar_GetBool("bo1_headless_client"))
        Com_Printf(14, "bo1_menu: openmainmenu %u '%s'\n", menuIndex, menuName);
    if (!I_stricmp(menuName, "main"))
    {
        int menu = com_desiredMenu && com_desiredMenu->current.integer > 0 ? com_desiredMenu->current.integer
                                                                           : UI_GetMenuScreen();
        int opened = UI_SetActiveMenu(localClientNum, (uiMenuCommand_t)menu);
        if (Dvar_GetBool("bo1_headless_client"))
            Com_Printf(14, "bo1_menu: UI_SetActiveMenu(%d) = %d, top '%s'\n", menu, opened,
                UI_GetTopActiveMenuName(localClientNum) ? UI_GetTopActiveMenuName(localClientNum) : "");
        Dvar_SetInt((dvar_s *)com_desiredMenu, 0);
        return;
    }
    UI_OpenMenu(localClientNum, menuName);
}

// zombies: 'Z' <index> = closemainmenu's client end (SP 0x005089e0, case 'Z' of SP 0x0078daf0): a negative
// index closes every menu (SP 0x004f3540 = UI_CloseAll, see cg_sp_hud.cpp), else the named menu closes
// (SP 0x005d8df0 = UI_CloseMenu).
void CG_SP_CloseMainMenuCommand(int localClientNum)
{
    int menuIndex = atoi(Cmd_Argv(1));
    if (menuIndex >= 32)
    {
        Com_PrintError(14, "Server tried to open a bad script menu index: %i\n", menuIndex);
        return;
    }
    if (menuIndex < 0)
    {
        UI_CloseAll(localClientNum);
        return;
    }
    const char *menuName = CL_GetConfigString(menuIndex + 2548);
    if (*menuName)
        UI_CloseMenu(localClientNum, menuName);
}
