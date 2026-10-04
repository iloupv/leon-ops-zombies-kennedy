#pragma once
#include <ui/ui_main.h>
#include <bgame/bg_weapons.h>
#include <universal/dvar.h>

struct cg_s;
struct entityState_s;
struct centity_s;

void __cdecl CG_OffhandRegisterDvars();
void __cdecl CG_DrawOffHandIcon(
                int localClientNum,
                const rectDef_s *rect,
                const float *color,
                Material *material,
                OffhandSlot weaponType);
int __cdecl GetBestOffhand(const playerState_s *predictedPlayerState, int offhandSlot);
bool __cdecl IsOffHandDisplayVisible(const cg_s *cgameGlob);
int __cdecl CalcOffHandAmmo(const playerState_s *predictedPlayerState, int weaponType);
void __cdecl CG_DrawOffHandHighlight(
                int localClientNum,
                const rectDef_s *rect,
                const float *color,
                Material *material,
                OffhandSlot weaponType);
void __cdecl OffHandFlash(const cg_s *cgameGlob, const float *base_color, float *out_color);
void __cdecl CG_DrawOffHandAmmo(
                int localClientNum,
                const rectDef_s *rect,
                Font_s *font,
                float scale,
                const float *color,
                int textStyle,
                OffhandSlot weaponType);
void __cdecl CG_DrawOffHandName(
                int localClientNum,
                const rectDef_s *rect,
                Font_s *font,
                float scale,
                const float *color,
                int textStyle,
                OffhandSlot weaponType);
void __cdecl CG_SwitchOffHandCmd(int localClientNum);
void __cdecl CG_PrepOffHand(int localClientNum, const entityState_s *ent, unsigned int weaponIndex, bool isPlayerView);
void __cdecl CG_UseOffHand(int localClientNum, const centity_s *cent, unsigned int weaponIndex, bool isPlayerView);
void __cdecl CG_SetEquippedOffHand(int localClientNum, int offHandIndex);
//double __cdecl _Pow_int<float>(float _X, int _Y);


extern const dvar_t *hud_flash_time_offhand;
extern const dvar_t *hud_flash_period_offhand;