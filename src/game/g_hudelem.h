#pragma once
#include <bgame/bg_public.h>
#include <game_mp/g_main_mp.h>
#include <clientscript/cscr_variable.h>

enum he_type_t : __int32
{                                       // XREF: HECmd_SetTimer_Internal/r
                                        // HECmd_SetClock_Internal/r
    HE_TYPE_FREE              = 0x0,
    HE_TYPE_TEXT              = 0x1,
    HE_TYPE_VALUE             = 0x2,
    HE_TYPE_PLAYERNAME        = 0x3,
    HE_TYPE_MAPNAME           = 0x4,
    HE_TYPE_GAMETYPE          = 0x5,
    HE_TYPE_WAR_GAME_DATA     = 0x6,
    HE_TYPE_SCORE             = 0x7,
    HE_TYPE_MATERIAL          = 0x8,
    HE_TYPE_TIMER_DOWN        = 0x9,
    HE_TYPE_TIMER_UP          = 0xA,
    HE_TYPE_TENTHS_TIMER_DOWN = 0xB,
    HE_TYPE_TENTHS_TIMER_UP   = 0xC,
    HE_TYPE_CLOCK_DOWN        = 0xD,
    HE_TYPE_CLOCK_UP          = 0xE,
    HE_TYPE_WAYPOINT          = 0xF,
    HE_TYPE_COUNT             = 0x10,
};

enum hudelem_update_t : __int32
{                                       // XREF: ?HudElem_UpdateClient@@YAXPAUgclient_s@@HW4hudelem_update_t@@@Z/r
                                        // ?HudElem_ClearClient@@YAXPAUgclient_s@@W4hudelem_update_t@@@Z/r
    HUDELEM_UPDATE_ARCHIVAL = 0x1,
    HUDELEM_UPDATE_CURRENT  = 0x2,
    HUDELEM_UPDATE_ARCHIVAL_AND_CURRENT = 0x3,
};

struct game_hudelem_s;
struct game_hudelem_field_t // sizeof=0x20
{                                       // XREF: .rdata:fields_0/r
    const char *name;
    int ofs;                            // XREF: HudElem_GetFlagForeground+9/r
                                        // HudElem_GetFontStyle3d+9/r ...
    int size;
    fieldtype_t type;
    int mask;
    int shift;
    void (__cdecl *setter)(game_hudelem_s *, int);
    void (__cdecl *getter)(game_hudelem_s *, int);
};

struct game_hudelem_s // sizeof=0x7C
{                                       // XREF: .data:game_hudelem_s * g_hudelems/r
    hudelem_s elem;                     // XREF: HudElem_Alloc(int,int)+25/r
                                        // HudElem_ClientDisconnect(gentity_s *)+25/r ...
    int clientNum;                      // XREF: HudElem_Alloc(int,int)+4D/w
                                        // HudElem_ClientDisconnect(gentity_s *)+3B/r
    int team;                           // XREF: HudElem_Alloc(int,int)+5C/w
    int archived;
};

game_hudelem_s *__cdecl HudElem_Alloc(int clientNum, int teamNum);
void __cdecl HudElem_SetDefaults(game_hudelem_s *hud);
void __cdecl HudElem_ClearTypeSettings(game_hudelem_s *hud);
game_hudelem_s *__cdecl HudElem_DebugAlloc(int clientNum);
void __cdecl HudElem_Free(game_hudelem_s *hud);
void __cdecl HudElem_ClientDisconnect(gentity_s *ent);
void __cdecl HudElem_DestroyAll();
void __cdecl HudElem_SetLocalizedString(game_hudelem_s *hud, int offset);
void __cdecl HudElem_SetFlagForeground(game_hudelem_s *hud, int offset);
int __cdecl GetField(char *i, int size);
void __cdecl HudElem_GetFlagForeground(game_hudelem_s *hud, int offset);
void __cdecl HudElem_SetFontStyle3d(game_hudelem_s *hud, int offset);
void __cdecl HudElem_GetFontStyle3d(game_hudelem_s *hud, int offset);
void __cdecl HudElem_SetFont3dUseGlowColor(game_hudelem_s *hud, int offset);
void __cdecl HudElem_GetFont3dUseGlowColor(game_hudelem_s *hud, int offset);
void __cdecl HudElem_SetFlagHideWhenDead(game_hudelem_s *hud, int offset);
void __cdecl HudElem_GetFlagHideWhenDead(game_hudelem_s *hud, int offset);
void __cdecl HudElem_SetFlagHideWhenInKillCam(game_hudelem_s *hud, int offset);
void __cdecl HudElem_GetFlagHideWhenInKillCam(game_hudelem_s *hud, int offset);
void __cdecl HudElem_SetFlagHideWhenInDemo(game_hudelem_s *hud, int offset);
void __cdecl HudElem_GetFlagHideWhenInDemo(game_hudelem_s *hud, int offset);
void __cdecl HudElem_SetFlagOverrideWhenInDemo(game_hudelem_s *hud, int offset);
void __cdecl HudElem_GetFlagOverrideWhenInDemo(game_hudelem_s *hud, int offset);
void __cdecl HudElem_SetFlagHideWhileRemoteControlling(game_hudelem_s *hud, int offset);
void __cdecl HudElem_GetFlagHideWhileRemoteControlling(game_hudelem_s *hud, int offset);
void __cdecl HudElem_SetFlagHideWhenInMenu(game_hudelem_s *hud, int offset);
void __cdecl HudElem_GetFlagHideWhenInMenu(game_hudelem_s *hud, int offset);
void __cdecl HudElem_SetFlagFadeWhenTargeted(game_hudelem_s *hud, int offset);
void __cdecl HudElem_GetFlagFadeWhenTargeted(game_hudelem_s *hud, int offset);
void __cdecl HudElem_SetBoolean(game_hudelem_s *hud, int offset);
void __cdecl HudElem_SetColor(game_hudelem_s *hud, int offset);
void __cdecl HudElem_GetColor(game_hudelem_s *hud, int offset);
void __cdecl HudElem_SetAlpha(game_hudelem_s *hud, int offset);
void __cdecl HudElem_GetAlpha(game_hudelem_s *hud, int offset);
void __cdecl HudElem_SetGlowColor(game_hudelem_s *hud, int offset);
void __cdecl HudElem_GetGlowColor(game_hudelem_s *hud, int offset);
void __cdecl HudElem_SetGlowAlpha(game_hudelem_s *hud, int offset);
void __cdecl HudElem_GetGlowAlpha(game_hudelem_s *hud, int offset);
void __cdecl HudElem_SetFontScale(game_hudelem_s *hud, int offset);
void __cdecl HudElem_SetFont(game_hudelem_s *hud, int offset);
void __cdecl HudElem_SetEnumString(
                game_hudelem_s *hud,
                const game_hudelem_field_t *f,
                const char **names,
                int nameCount);
void __cdecl HudElem_GetFont(game_hudelem_s *hud, int offset);
void __cdecl HudElem_GetEnumString(
                game_hudelem_s *hud,
                const game_hudelem_field_t *f,
                const char **names,
                int nameCount);
void __cdecl HudElem_SetAlignX(game_hudelem_s *hud, int offset);
void __cdecl HudElem_GetAlignX(game_hudelem_s *hud, int offset);
void __cdecl HudElem_SetAlignY(game_hudelem_s *hud, int offset);
void __cdecl HudElem_GetAlignY(game_hudelem_s *hud, int offset);
void __cdecl HudElem_SetHorzAlign(game_hudelem_s *hud, int offset);
void __cdecl HudElem_GetHorzAlign(game_hudelem_s *hud, int offset);
void __cdecl HudElem_SetVertAlign(game_hudelem_s *hud, int offset);
void __cdecl HudElem_GetVertAlign(game_hudelem_s *hud, int offset);
void __cdecl HudElem_SetUI3DWindow(game_hudelem_s *hud, int offset);
void __cdecl HudElem_GetUI3DWindow(game_hudelem_s *hud, int offset);
void __cdecl Scr_GetHudElemField(unsigned int entnum, unsigned int offset);
void __cdecl Scr_SetHudElemField(unsigned int entnum, unsigned int offset);
void __cdecl Scr_FreeHudElemConstStrings(game_hudelem_s *hud);
void __cdecl GScr_NewHudElem();
void __cdecl GScr_NewDebugHudElem();
void __cdecl GScr_NewClientHudElem();
void __cdecl GScr_NewScoreHudElem();
void __cdecl GScr_NewTeamHudElem();
void __cdecl GScr_AddFieldsForHudElems();
void __cdecl HECmd_SetText(scr_entref_t entref);
game_hudelem_s *__cdecl HECmd_GetHudElem(scr_entref_t entref);
void __cdecl HECmd_ClearAllTextAfterHudElem(scr_entref_t entref);
void __cdecl HECmd_SetMaterial(scr_entref_t entref);
void __cdecl HECmd_SetWarGameData(scr_entref_t entref);
void __cdecl HECmd_SetTargetEnt(scr_entref_t entref);
void __cdecl HECmd_ClearTargetEnt(scr_entref_t entref);
void __cdecl HECmd_SetTimer(scr_entref_t entref);
void __cdecl HECmd_SetTimer_Internal(scr_entref_t entref, he_type_t type, const char *cmdName);
void __cdecl HECmd_SetTimerUp(scr_entref_t entref);
void __cdecl HECmd_SetTenthsTimer(scr_entref_t entref);
void __cdecl HECmd_SetTenthsTimerUp(scr_entref_t entref);
void __cdecl HECmd_SetClock(scr_entref_t entref);
void __cdecl HECmd_SetClock_Internal(scr_entref_t entref, he_type_t type, const char *cmdName);
void __cdecl HECmd_SetClockUp(scr_entref_t entref);
void __cdecl HECmd_SetValue(scr_entref_t entref);
void __cdecl HECmd_SetWaypoint(scr_entref_t entref);
void __cdecl HECmd_FadeOverTime(scr_entref_t entref);
void __cdecl HECmd_ScaleOverTime(scr_entref_t entref);
void __cdecl HECmd_MoveOverTime(scr_entref_t entref);
void __cdecl HECmd_Reset(scr_entref_t entref);
void __cdecl HECmd_Destroy(scr_entref_t entref);
void __cdecl HECmd_SetPlayerNameString(scr_entref_t entref);
void __cdecl HECmd_SetGameTypeString(scr_entref_t entref);
void __cdecl HECmd_SetMapNameString(scr_entref_t entref);
void __cdecl HECmd_SetPulseFX(scr_entref_t entref);
int __cdecl GetIntGTZero(unsigned int index);
void __cdecl HECmd_SetCOD7DecodeFX(scr_entref_t entref);
void __cdecl HECmd_SetRedactFX(scr_entref_t entref);
void __cdecl HECmd_GetTextWidth(scr_entref_t entref);
void (__cdecl *__cdecl HudElem_GetMethod(const char **pName))(scr_entref_t);
void __cdecl HudElem_UpdateClient(gclient_s *client, int clientNum, hudelem_update_t which);
void __cdecl HudElem_ClearClient(gclient_s *client, hudelem_update_t which);
void __cdecl HudElem_ClearClientSingle(hudelem_s *elems, int max);
