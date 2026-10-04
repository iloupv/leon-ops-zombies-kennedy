#pragma once
#include <qcommon/msg_mp.h>
#include <cgame/cg_local.h>

struct CenterPrint // sizeof=0x408
{                                       // XREF: .data:s_centerPrint/r
    int time;                           // XREF: CG_ClearCenterPrint(int)+C/w
    char text[1024];
    int priority;
};

struct ScreenBurn // sizeof=0xC
{                                       // XREF: .data:s_screenBurn/r
                                        // .data:s_screenElectrified/r
    int startTime;
    int endTime;
    int localClientNum;
};


struct clientInfo_t;
struct rectDef_s;
struct Font_s;
struct weaponInfo_s;

void __cdecl CG_PriorityCenterPrint(int localClientNum, const char *str, int priority);
void __cdecl CG_DisplayPlayerCard(
                int localClientNum,
                const clientInfo_t *attackerCI,
                const clientInfo_t *victimCI,
                bool wasKilled);
void __cdecl CG_ClearCenterPrint(int localClientNum);
void __cdecl CG_DrawCenterString(
                int localClientNum,
                const rectDef_s *rect,
                Font_s *font,
                float fontscale,
                float *color,
                int textStyle);
void __cdecl CG_Electrified(int localClientNum, int durationMS);
void __cdecl CG_ClearElectrified(int localClientNum);
void __cdecl CG_Burn(int localClientNum, int durationMS);
void __cdecl CG_ClearBurn(int localClientNum);
bool __cdecl IsHardcoreMode(int localClientNum);
bool __cdecl CG_AreAllPlayerNamesVisible();
void __cdecl CG_Draw2D(int localClientNum);
void __cdecl CG_Draw2DInternal(int localClientNum);
void __cdecl CG_DrawChatMessages(int localClientNum);
void __cdecl CG_CheckTimedMenus(int localClientNum);
void __cdecl CG_CheckForPlayerInput(int localClientNum);
bool __cdecl CG_CheckPlayerMovement(usercmd_s oldCmd, usercmd_s newCmd);
int __cdecl CG_CheckPlayerStanceChange(
                int localClientNum,
                bitarray<51> *newButton_bits,
                bitarray<51> *changedButton_bits);
int __cdecl CG_CheckPlayerWeaponUsage(int localClientNum, bitarray<51> *button_bits);
bool __cdecl CG_CheckPlayerTryReload(int localClientNum, bitarray<51> *button_bits);
bool __cdecl CG_CheckPlayerFireNonTurret(int localClientNum, bitarray<51> *button_bits);
int __cdecl CG_CheckPlayerOffHandUsage(int localClientNum, bitarray<51> *button_bits);
int __cdecl CG_CheckPlayerMiscInput(bitarray<51> *button_bits);
void __cdecl CG_CheckHudHealthDisplay(int localClientNum);
void __cdecl CG_CheckHudAmmoDisplay(int localClientNum);
void __cdecl CG_CheckHudCompassDisplay(int localClientNum);
void __cdecl CG_CheckHudStanceDisplay(int localClientNum);
void __cdecl CG_CheckHudSprintDisplay(int localClientNum);
void __cdecl CG_CheckHudOffHandDisplay(int localClientNum);
void __cdecl CG_CheckHudObjectiveDisplay(int localClientNum);
void __cdecl CG_CheckVehicleControlsDisplay(int localClientNum);
void __cdecl CG_ScreenBurn(int localClientNum);
void __cdecl CG_ScreenElectrified(int localClientNum);
void __cdecl CG_Transported(int localClientNum, int durationMS);
void __cdecl CG_ClearTransported(int localClientNum);
void __cdecl CG_ScreenTransported(int localClientNum);
void __cdecl CG_DrawMiniConsole(int localClientNum);
void __cdecl CG_DrawErrorMessages(int localClientNum);
void __cdecl CG_DrawSay(int localClientNum);
void __cdecl CG_DrawVote(int localClientNum);
char __cdecl CG_DrawGenericOverlay(int SortIndex, int localClientNum);
void __cdecl DrawIntermission(int localClientNum);
void __cdecl CG_DrawSpectatorMessage(int localClientNum);
const char *__cdecl CG_GetBoundSpectatorCommand(int localClientNum, const char **choices, int choiceCount);
void __cdecl CG_UpdatePlayerNames(int localClientNum);
void __cdecl CG_UpdatePlayerNamesInternal(int localClientNum);
void __cdecl DrawViewmodelInfo(int localClientNum);
weaponInfo_s *__cdecl CG_GetLocalClientWeaponInfo(int localClientNum, int weaponIndex);
void __cdecl CG_DrawActive(int localClientNum);
void __cdecl CG_DrawMissileCam(int localClientNum);
void __cdecl CG_AddSceneTracerBeams(int localClientNum);
void __cdecl CG_GenerateSceneCodeMeshes(int localClientNum);
void __cdecl CG_GenerateSceneVerts(int localClientNum);

extern const char *WeaponStateNames[50];