#pragma once
#include <bgame/bg_weapons.h>

char __cdecl G_ParseWeaponAccurayGraphs(WeaponDef *weaponDef);
char __cdecl G_ParseWeaponAccurayGraphInternal(
                WeaponDef *weaponDef,
                const char *dirName,
                const char *graphName,
                float (*knots)[2],
                int *knotCount);
char __cdecl G_ParseAIWeaponAccurayGraphFile(
                const char *buffer,
                const char *fileName,
                float (*knots)[2],
                int *knotCount);
