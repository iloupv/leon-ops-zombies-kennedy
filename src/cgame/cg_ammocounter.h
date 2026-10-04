#pragma once

#include <universal/dvar.h>
#include <qcommon/cmd.h>
#include <cgame_mp/cg_local_mp.h>
#include <gfx_d3d/r_font.h>
#include <bgame/bg_weapons_def.h>
#include <client/screen_placement.h>

void __cdecl CG_AmmoCounterRegisterDvars();
void __cdecl CG_DrawAmmoCounterSingle(int localClientNum, rectDef_s *rect, float *color, bool last);
unsigned int __cdecl GetWeaponIndex(const cg_s *cgameGlob);
void __cdecl CG_DrawPlayerWeaponAmmoStock(
                int localClientNum,
                const rectDef_s *rect,
                Font_s *font,
                float scale,
                float *color,
                Material *material,
                int textStyle);
unsigned int __cdecl ClipCounterWeapIdx(const cg_s *cgameGlob, unsigned int weapIndex);
unsigned int __cdecl GetWeaponAltIndex(const cg_s *cgameGlob, unsigned int weaponIndex);
double __cdecl AmmoCounterFadeAlpha(int localClientNum, cg_s *cgameGlob);
void __cdecl CG_DrawFuelTankPercentage(
                int localClientNum,
                const rectDef_s *rect,
                Font_s *font,
                float scale,
                float *color,
                Material *material,
                int textStyle,
                float text_x,
                float text_y);
void __cdecl CG_DrawPlayerWeaponAmmoClip(
                int localClientNum,
                const rectDef_s *rect,
                Font_s *font,
                float scale,
                float *color,
                Material *material,
                int textStyle,
                float text_x,
                float text_y,
                char textAlignMode);
void __cdecl AmmoColor(cg_s *cgameGlob, float *color, unsigned int weapIndex);
void __cdecl CG_DrawPlayerWeaponAmmoClipDualWield(
                int localClientNum,
                const rectDef_s *rect,
                Font_s *font,
                float scale,
                float *color,
                Material *material,
                int textStyle);
float __cdecl CG_GetHudAlphaDPad(int localClientNum);
double __cdecl DpadFadeAlpha(int localClientNum, cg_s *cgameGlob);
bool __cdecl ActionSlotIsActive(int localClientNum, unsigned int slotIdx);
float __cdecl CG_GetHudAlphaAmmoCounter(int localClientNum);
char __cdecl CG_ActionSlotIsUsable(int localClientNum, unsigned int slotIdx);
void __cdecl CG_DrawPlayerActionSlotDpad(
                int localClientNum,
                const rectDef_s *rect,
                const float *color,
                Material *material);
void __cdecl CG_DrawPlayerActionSlotBack(
                int localClientNum,
                const rectDef_s *rect,
                unsigned int slotIdx,
                const float *color,
                Material *material);
void __cdecl CG_DrawPlayerActionSlotArrow(
                int localClientNum,
                const rectDef_s *rect,
                unsigned int slotIdx,
                float *color,
                Material *material);
void __cdecl CG_DrawPlayerActionSlot(
                int localClientNum,
                const rectDef_s *rect,
                unsigned int slotIdx,
                float *color,
                Font_s *textFont,
                float textScale,
                int textStyle);
void __cdecl DpadIconDims(
                const rectDef_s *rect,
                unsigned int slotIdx,
                const WeaponVariantDef *weapVariantDef,
                float *x,
                float *y,
                float *w,
                float *h);
void __cdecl DpadTextPos(
                int localClientNum,
                const rectDef_s *rect,
                unsigned int slotIdx,
                const WeaponVariantDef *weapVariantDef,
                float *x,
                float *y);
void __cdecl CG_DrawPlayerActionSlotBind(
                int localClientNum,
                const rectDef_s *rect,
                unsigned int slotIdx,
                float *color,
                Font_s *textFont,
                float textScale,
                int textStyle,
                int textAlignMode,
                itemDef_s *item,
                char *text);
void __cdecl CG_DrawPlayerWeaponBackground(
                int localClientNum,
                const rectDef_s *rect,
                const float *color,
                Material *material);
void __cdecl CG_DrawPlayerWeaponAmmoClipGraphic(int localClientNum, const rectDef_s *rect, const float *color);
void __cdecl GetBaseRectPos(int localClientNum, const rectDef_s *rect, float *base);
void __cdecl DrawClipAmmo(
                cg_s *cgameGlob,
                float *base,
                unsigned int weapIdx,
                const WeaponDef *weapDef,
                const WeaponVariantDef *weapVarDef,
                float *color);
void __cdecl DrawClipAmmoMagazine(
                cg_s *cgameGlob,
                const float *base,
                unsigned int weapIdx,
                const WeaponVariantDef *weapVarDef,
                float *color);
void __cdecl DrawClipAmmoShortMagazine(
                cg_s *cgameGlob,
                const float *base,
                unsigned int weapIdx,
                const WeaponVariantDef *weapVarDef,
                float *color);
void __cdecl DrawClipAmmoShotgunShells(
                cg_s *cgameGlob,
                const float *base,
                unsigned int weapIdx,
                const WeaponVariantDef *weapVarDef,
                float *color);
void __cdecl DrawClipAmmoRockets(
                cg_s *cgameGlob,
                const float *base,
                unsigned int weapIdx,
                const WeaponVariantDef *weapVarDef,
                float *color);
void __cdecl DrawClipAmmoBeltfed(
                cg_s *cgameGlob,
                float *base,
                unsigned int weapIdx,
                const WeaponDef *weapDef,
                const WeaponVariantDef *weapVarDef,
                float *color);
void __cdecl CG_DrawPlayerWeaponAmmoClipGraphicDualWield(int localClientNum, const rectDef_s *rect, const float *color);
void __cdecl CG_DrawPlayerWeaponIcon(int localClientNum, const rectDef_s *rect, const float *color);
void __cdecl DrawStretchPicGun(
                const ScreenPlacement *scrPlace,
                const rectDef_s *rect,
                const float *color,
                Material *material,
                weaponIconRatioType_t ratio);
void __cdecl CG_DrawPlayerWeaponLowAmmoWarning(
                int localClientNum,
                const rectDef_s *rect,
                Font_s *font,
                float textScale,
                int textStyle,
                float text_x,
                float text_y,
                int textAlignMode,
                Material *material);

extern const dvar_t *ammoCounterHide;
extern const dvar_t *actionSlotsHide;
extern const dvar_t *lowAmmoWarningColor1;
extern const dvar_t *lowAmmoWarningColor2;
extern const dvar_t *lowAmmoWarningPulseFreq;
extern const dvar_t *lowAmmoWarningPulseMax;
extern const dvar_t *lowAmmoWarningPulseMin;
extern const dvar_t *lowAmmoWarningNoReloadColor1;
extern const dvar_t *lowAmmoWarningNoReloadColor2;
extern const dvar_t *lowAmmoWarningNoAmmoColor1;
extern const dvar_t *lowAmmoWarningNoAmmoColor2;
extern const dvar_t *cg_fuelHudVersion;
extern const dvar_t *ui_right_ammo_width;
extern const dvar_t *ui_ammo_stock_width;