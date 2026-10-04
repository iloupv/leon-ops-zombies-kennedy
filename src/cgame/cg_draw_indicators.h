#pragma once
#include <cgame_mp/cg_local_mp.h>
#include <bgame/bg_weapons.h>

struct HudGrenade // sizeof=0x18
{                                       // XREF: .data:g_hudGrenades/r
    float origin[3];
    float predicted_dmg_ratio;          // XREF: CG_AddHudGrenade(cg_s const *,centity_s const *)+419/w
                                        // CG_DrawGrenadeIndicators(int)+1EB/r
    float predicted_time_ratio;         // XREF: CG_AddHudGrenade(cg_s const *,centity_s const *)+4D8/w
                                        // CG_AddHudGrenade(cg_s const *,centity_s const *)+4F3/w
    Material *material;                 // XREF: CG_AddHudGrenade(cg_s const *,centity_s const *)+42C/w
};

void __cdecl CG_DrawFlashDamage(const cg_s *cgameGlob);
void __cdecl CG_DrawDamageDirectionIndicators(int localClientNum);
void __cdecl CG_DamageShots_Note(int localClientNum, int yawByte, int pitchByte, int damage);
void __cdecl CG_DamageShots_Frame(int localClientNum);
void __cdecl CG_ClearHudGrenades();
double __cdecl CG_AddHudGrenade_PositionCheck(cg_s *cgameGlob, const centity_s *grenadeEnt, const WeaponDef *weapDef);
void __cdecl CG_GrenadeIndicator_GetPlayerPosition(const cg_s *cgameGlob, float *playerOrigin);
void __cdecl CG_AddHudGrenade(cg_s *cgameGlob, const centity_s *grenadeEnt);
void __cdecl CG_DrawGrenadeIndicators(int localClientNum);
void __cdecl CG_DrawWarningPointer(
                int localClientNum,
                float centerX,
                float centerY,
                const float *grenadeOffset,
                const float *color,
                float radiusOffset);
void __cdecl CG_DrawWarningIcon(
                int localClientNum,
                float centerX,
                float centerY,
                const float *grenadeOffset,
                const float *color,
                Material *material,
                float radiusOffset);
