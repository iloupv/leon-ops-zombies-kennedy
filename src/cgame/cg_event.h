#pragma once
#include <gfx_d3d/fxprimitives.h>
#include <bgame/bg_local.h>
#include <cgame_mp/cg_local_mp.h>
#include <bgame/bg_misc.h>
#include <bgame/bg_weapons.h>

void __cdecl CG_SetWetness(int localClientNum, int entNum, float wetness, int invert);
void __cdecl CG_ExplosionEvent(
                int localClientNum,
                float *origin,
                float damageInner,
                float damageOuter,
                float radius,
                int mod);
int __cdecl CG_GetBoneIndex(
                int localClientNum,
                unsigned int dobjHandle,
                unsigned int boneName,
                unsigned __int8 *boneIndex);
unsigned int __cdecl CG_PlayBoltedEffect(
                int localClientNum,
                const FxEffectDef *fxDef,
                int dobjHandle,
                unsigned int boneName);
void __cdecl CG_PlayFxOnTag(int localClientNum, centity_s *cent, int eventParm);
void __cdecl CG_DoFootstepEvent(
                int localClientNum,
                centity_s *cent,
                bool isPlayerView,
                int event,
                int surfaceType,
                bool quiet);
void __cdecl CG_DoJumpEvent(int localClientNum, centity_s *cent, bool isPlayerView, int surfaceType, bool quiet);
void __cdecl CG_DoLandEvent(
                int localClientNum,
                centity_s *cent,
                bool isPlayerView,
                int surfaceType,
                bool quiet,
                bool damagePlayer);
void __cdecl CG_DoFoliageEvent(int localClientNum, centity_s *cent, bool isPlayerView, bool quiet);
void __cdecl CG_EntityEvent(int localClientNum, centity_s *cent, int event);
bool __cdecl Com_IsRagdollTrajectory(const trajectory_t *trajectory);
void __cdecl CG_Obituary(int localClientNum, const entityState_s *ent);
void __cdecl CG_ObituaryRevive(int localClientNum, const entityState_s *ent);
void __cdecl CG_DirectionalHitIndicator(int localClientNum, const entityState_s *ent);
void __cdecl CG_ItemPickup(int localClientNum, int weapIndex);
void __cdecl CG_PlayFx(int localClientNum, centity_s *cent, const float *angles);
void __cdecl CG_SetInvalidCmdHint(cg_s *cgameGlob, InvalidCmdHintType hintType);
void __cdecl CG_SetInvalidStanceHint(cg_s *cgameGlob, invalid_cmd_hint_t type);
void __cdecl CG_StopWeaponSound(
                int localClientNum,
                bool isPlayerView,
                const WeaponDef *weaponDef,
                int entitynum,
                weaponstate_t weaponstate);
void __cdecl CG_PhysLaunch(int localClientNum, centity_s *cent, const entityState_s *es);
void __cdecl CG_CheckEvents(int localClientNum, centity_s *cent);
void __cdecl CG_CheckEntEvents(int localClientNum, centity_s *cent);
bool __cdecl CG_CheckIfDualWieldEmpty(playerState_s *ps, unsigned int weapIndex);
