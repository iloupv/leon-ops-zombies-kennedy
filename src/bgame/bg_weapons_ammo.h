#pragma once
#include "bg_local.h"

void __cdecl BG_SetupWeaponDefAmmoIndexes(unsigned int weapIndex, WeaponDef *weapDef, WeaponVariantDef *weapVarDef);
void __cdecl BG_SetupWeaponDefSharedAmmoIndexes(unsigned int weapIndex, WeaponDef *weapDef);
void __cdecl BG_SetupWeaponDefClipIndexes(WeaponDef *weapDef, WeaponVariantDef *weapVarDef);
void __cdecl BG_ClearWeaponDefAmmo();
int __cdecl BG_GetSharedAmmoCapSize(unsigned int capIndex);
AmmoClip *__cdecl BG_GetAmmoClip(playerState_s *ps, int clipIndex);
void __cdecl BG_AddAmmoToClip(playerState_s *ps, int clipIndex, int amount);
AmmoClip *__cdecl BG_AddAmmoClip(playerState_s *ps, int clipIndex);
AmmoClip *__cdecl BG_GetFreeAmmoClip(playerState_s *ps);
void __cdecl BG_SetAmmoInClip(playerState_s *ps, int clipIndex, int amount);
AmmoPool *__cdecl BG_GetAmmoPool(playerState_s *ps, int ammoIndex);
void __cdecl BG_AddAmmoToPool(playerState_s *ps, unsigned int weaponIndex, int amount);
AmmoPool *__cdecl BG_GetFreeAmmoPool(playerState_s *ps);
void __cdecl BG_SetAmmoInPool(playerState_s *ps, unsigned int weaponIndex, int amount);
int __cdecl BG_GetAmmoPlayerMax(const playerState_s *ps, unsigned int weaponIndex, unsigned int weaponIndexToSkip);
int __cdecl BG_GetMaxAmmo(unsigned int weaponIndex);
char __cdecl ValueInArray(const int *weaponArray, int value);
void __cdecl AddValueToArray(int *weaponArray, int value);
int __cdecl BG_GetMaxPickupableAmmo(const playerState_s *ps, unsigned int weaponIndex);
int __cdecl BG_GetTotalAmmoReserve(const playerState_s *ps, unsigned int weaponIndex);
int __cdecl BG_ClipForWeapon(unsigned int weapon);
int __cdecl BG_AmmoForWeapon(unsigned int weapon);
bool __cdecl BG_WeaponIsClipOnly(unsigned int weapon);
int __cdecl BG_WeaponAmmo(const playerState_s *ps, unsigned int weapon);
void __cdecl PM_ReloadClip(playerState_s *ps);
void __cdecl PM_WeaponUseAmmo(playerState_s *ps, unsigned int wp, int amount);
int __cdecl PM_WeaponAmmoAvailable(playerState_s *ps);
bool __cdecl PM_WeaponClipEmpty(playerState_s *ps);
