#pragma once
#include "actor.h"
#include <bgame/bg_weapons.h>

void __cdecl Actor_FillWeaponParms(actor_s *self, weaponParms *wp);
void __cdecl Actor_InitWeaponParms(actor_s *self, unsigned int weapon, weaponParms *wp);
gentity_s *__cdecl Actor_Melee(actor_s *self, const float *direction);
