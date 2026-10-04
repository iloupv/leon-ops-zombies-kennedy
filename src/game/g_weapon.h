#pragma once
#include <bgame/bg_weapons.h>
#include <bgame/bg_public.h>
#include "game_public.h"

void __cdecl Weapon_SetWeaponParamsWeapon(weaponParms *wp, unsigned int weapon);
void __cdecl G_AntiLagRewindClientPos(int gameTime, AntilagClientStore *antilagStore);
void __cdecl G_AntiLag_RestoreClientPos(AntilagClientStore *antilagStore);
gentity_s *__cdecl Weapon_Melee(gentity_s *ent, weaponParms *wp, float range, float width, float height, int gametime);
gentity_s *__cdecl Weapon_Melee_internal(gentity_s *ent, weaponParms *wp, float range, float width, float height);
bool __cdecl Melee_Trace(
                gentity_s *ent,
                weaponParms *wp,
                int damage,
                float range,
                float width,
                float height,
                trace_t *traceResult,
                float *hitOrigin);
gentity_s *__cdecl Weapon_Throw_Grenade(gentity_s *ent, int grenType, unsigned __int8 grenModel, weaponParms *wp);
gentity_s *__cdecl Weapon_GrenadeLauncher_Fire(
                gentity_s *ent,
                int grenType,
                unsigned __int8 grenModel,
                weaponParms *wp);
gentity_s *Weapon_RocketLauncher_Fire(
                gentity_s *ent,
                unsigned int weaponIndex,
                float spread,
                weaponParms *wp,
                const float *gunVel,
                gentity_s *target,
                const float *targetOffset);
void __cdecl gunrandom(float *x, float *y);
gentity_s *__cdecl Weapon_BombDrop_Fire(
                gentity_s *ent,
                unsigned int weaponIndex,
                float spread,
                weaponParms *wp,
                const float *gunVel,
                gentity_s *target,
                const float *targetOffset);
void    Weapon_Overheat_Update(gentity_s *ent);
void __cdecl Weapon_Flamethrower_Update(gentity_s *ent, weaponParms *wp);
void __cdecl Weapon_Flamethrower_Fire(gentity_s *ent, weaponParms *wp);
void __cdecl Weapon_Napalm_Flame(gentity_s *ent, trace_t *trace, int count);
bool __cdecl LogAccuracyHit(gentity_s *target, gentity_s *attacker);
void __cdecl G_CalcMuzzlePoints(const gentity_s *ent, weaponParms *wp, int shotCount);
void __cdecl FireWeapon(gentity_s *ent, int gametime, int shotCount);
void __cdecl DeployWeapon(gentity_s *ent);
void __cdecl BreakDownWeapon(gentity_s *ent);
void __cdecl G_UseOffHand(gentity_s *ent);
void __cdecl FireWeaponMelee(gentity_s *ent, int gametime);
int __cdecl G_GivePlayerWeapon(
                playerState_s *pPS,
                int iWeaponIndex,
                unsigned __int8 altModelIndex,
                renderOptions_s weaponOptions);
void __cdecl G_SetupWeaponDef();
int __cdecl G_GetWeaponIndexForName(char *name);
void __cdecl G_SelectWeaponIndex(int clientNum, int iWeaponIndex);
void __cdecl G_SetEquippedOffHand(int clientNum, unsigned int offHandIndex);
unsigned int __cdecl G_PlayerTurretWeaponIdx(const playerState_s *ps);
int __cdecl G_GetPlayerWeapon(const playerState_s *ps, int localClientNum);
int __cdecl G_GetPlayerVehicleWeapon(const playerState_s *ps);
