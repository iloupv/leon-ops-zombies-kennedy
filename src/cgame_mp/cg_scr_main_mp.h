#pragma once
#include <clientscript/cscr_variable.h>
#include <game/teams.h>
#include "cg_ents_mp.h"

struct cscr_mp_data_t // sizeof=0x10
{                                       // XREF: .data:cscr_mp_data_t cg_scr_mp_data/r
    int dogstep;                        // XREF: CScr_SetUniqueClientScripts(ScriptFunctions *)+225/w
    int airsupport;                     // XREF: CScr_SetUniqueClientScripts(ScriptFunctions *)+27C/w
    int demo_jump;                      // XREF: CScr_SetUniqueClientScripts(ScriptFunctions *)+299/w
    int demo_playerSwitch;              // XREF: CScr_SetUniqueClientScripts(ScriptFunctions *)+2B6/w
};

// zombies: the SP client script handles that cscr_data_t has no slot for. SP keeps them in separate globals
// (.data 0x02FF838C-0x02FF83A0), written by CScr_SetUniqueClientScripts_SP (SP 0x00408EE0).
struct cscr_sp_data_t
{
    int playAIFootstep;                 // SP 0x02FF838C  clientscripts/_footsteps::playAIFootstep
    int activateExploder;               // SP 0x02FF8390  clientscripts/_callbacks::callback_activate_exploder
    int deactivateExploder;             // SP 0x02FF8394  clientscripts/_callbacks::callback_deactivate_exploder
    int zombieEyeCallback;              // SP 0x02FF8398  clientscripts/_zombiemode (or _zombietron)::zombie_eye_callback
    int weaponDeathEffects;             // SP 0x02FF839C  CodeCallback_PlayWeaponDeathEffects
    int weaponDamageEffects;            // SP 0x02FF83A0  CodeCallback_PlayWeaponDamageEffects
};
extern cscr_sp_data_t cg_scr_sp_data;

struct cached_tag_mat_t;
struct cent_field_s;
struct centity_s;

void __cdecl CScrCmd_Earthquake(scr_entref_t entref);
unsigned int __cdecl CScr_SpawnFXInternal(int localClientNum, int fxId, float (*axis)[3], float *pos, int time);
void CScr_DeleteFX();
void CScr_SpawnFX();
void CScr_PlayFXOnTag();
void CScr_PlayViewmodelFX();
void __cdecl CScr_IsDemoPlaying();
void __cdecl CScr_IsSpectating();
void __cdecl CScrCmd_IsBurning(scr_entref_t entref);
void __cdecl CPlayerCmd_HasPerk(scr_entref_t entref);
void __cdecl CScr_GetVehicleHealth(scr_entref_t entref);
void __cdecl CScr_GetLeftTreadHealth(scr_entref_t entref);
void __cdecl CScr_GetHeliDamageState(scr_entref_t entref);
void __cdecl CScr_GetRightTreadHealth(scr_entref_t entref);
void __cdecl CScr_GetInKillcam(scr_entref_t entref);
void __cdecl CScr_GetAnimState(scr_entref_t entref);
void __cdecl CScr_GetAnimStateCategory(scr_entref_t entref);
void CScr_GetTotalAmmo();
void CScr_GetCurrentWeapon();
void CScr_GetCurrentWeaponIncludingMelee();
void CScr_HasWeapon();
void CScr_SetLocalRadarEnabled();
void CScr_SetLocalRadarPosition();
void CScr_SetExtraCamEntity();
void CScr_SetExtraCamActive();
void CScr_GetExtraCamStatic();
void CScr_SetExtraCamStatic();
void CScr_SetExtraCamOrigin();
void CScr_SetExtraCamAngles();
void CScr_IsCameraSpikeToggled();
// ADD
void CScr_SetClientVolumetricFog();
void CScr_SwitchToServerVolumetricFog();
void CScr_SwitchToClientVolumetricFog();
void CScr_IsInHelicopter();
// END
void CScr_GetGridFromPos();
void CScr_CompassScale();
void CScr_ResetCompassScale();
void __cdecl CScr_GetLocalPlayerTeam();
void __cdecl CScr_AddTeamName(team_t team);
void (__cdecl *__cdecl CScr_GetFunctionProjectSpecific(const char **pName, int *type))();
void __cdecl CScrCmd_GetOwner(scr_entref_t entref);
void __cdecl CScr_GetTagOrigin(scr_entref_t entref);
int __cdecl CScr_UpdateTagInternal(centity_s *ent, unsigned int tagName, cached_tag_mat_t *cachedTag);
void __cdecl CScr_GetTagAngles(scr_entref_t entref);
void __cdecl CScrCmd_ShellShock(scr_entref_t entref);
void __cdecl CScr_SetEnemyGlobalScrambler(scr_entref_t entref);
void __cdecl CScr_SetEnemyScramblerAmount(scr_entref_t entref);
void __cdecl CScr_SetFriendlyScramblerAmount(scr_entref_t entref);
void __cdecl CScr_GetFriendlyScramblerAmount(scr_entref_t entref);
void __cdecl CScr_GetEnemyScramblerAmount(scr_entref_t entref);
void __cdecl CScr_IsScrambled(scr_entref_t entref);
void __cdecl CScr_SetNearestEnemyScrambler(scr_entref_t entref);
void __cdecl CScr_ClearNearestEnemyScrambler(scr_entref_t entref);
void __cdecl CScr_AddFriendlyScrambler(scr_entref_t entref);
void __cdecl CScr_RemoveFriendlyScrambler(scr_entref_t entref);
void __cdecl CScr_RemoveAllFriendlyScramblers(scr_entref_t entref);
void __cdecl CScr_HasTacticalMaskOverlay(scr_entref_t entref);
void __cdecl CScr_GetStance(scr_entref_t entref);
void __cdecl CScr_SetFlagAsAway(scr_entref_t entref);
void __cdecl CScr_GetParentEntity(scr_entref_t entref);
void(__cdecl *__cdecl CScr_GetMethodProjectSpecific(const char **pName, int *type))(scr_entref_t);
void __cdecl CScr_SetUniqueClientScripts(ScriptFunctions *functions);
void __cdecl CG_SendSwimNotify(int localClientNum, unsigned int clientNum, int start);
void __cdecl CScr_GetEntityByIndex(centity_s *cent, const cent_field_s *pField);
int __cdecl GetField(const int *i, int size);
void __cdecl CScr_GetTeamName(centity_s *cent, const cent_field_s *pField);
team_t __cdecl GetTeam(centity_s *cent);
unsigned __int16 __cdecl CScr_GetFootTag(eFoot foot);
void __cdecl CScr_PlayDogstepSound(int localClientNum, centity_s *cent, eFoot foot);

extern cscr_mp_data_t cg_scr_mp_data;