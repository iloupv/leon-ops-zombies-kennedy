#pragma once
#include <cgame_mp/cg_local_mp.h>
#include <bgame/bg_weapons.h>

void __cdecl CG_CalcCrosshairPosition(const cg_s *cgameGlob, float *x, float *y);
char __cdecl CG_GetWeapReticleZoom(const cg_s *cgameGlob, float *zoom);
void __cdecl CG_DrawNightVisionOverlay(int localClientNum);
bool __cdecl CG_UsingLowResViewPort(int localClientNum);
bool __cdecl CG_ColorCrosshairs(int time, int startMsec, int lastMsec, int fadeInMsec, int fadeOutMsec);
bool __cdecl ShouldShowCrosshairOnVehicle(const cg_s *cgameGlob, const WeaponDef *weapDef);
bool __cdecl ShouldDrawCrosshair(const cg_s *cgameGlob, const playerState_s *ps);
void __cdecl CG_DrawCrosshair(int localClientNum);
void __cdecl CG_DrawAdsOverlay(int localClientNum, int weaponIndex, const float *color, const float *crosshairPos);
void __cdecl CG_DrawFrameOverlay(
                float innerLeft,
                float innerRight,
                float innerTop,
                float innerBottom,
                float centerW,
                const float *color,
                Material *material);
double __cdecl CG_DrawWeapReticle(int localClientNum);
void __cdecl CG_CalcCrosshairColor(int localClientNum, float alpha, float *color);
void __cdecl CG_DrawTurretCrossHair(int localClientNum);
void __cdecl CG_DrawVehicleCrossHair(int localClientNum);
double __cdecl CG_GetVehicleCrossHairAlpha(int localClientNum);
bool __cdecl AllowedToDrawCrosshair(int localClientNum, const playerState_s *predictedPlayerState);
bool __cdecl CG_IsReticleTurnedOff(int localClientNum);
void __cdecl CG_DrawAdsAimIndicator(
                int localClientNum,
                const WeaponDef *weapDef,
                const float *color,
                float centerX,
                float centerY,
                float transScale);
void __cdecl CG_TransitionToAds(
                const cg_s *cgameGlob,
                const WeaponDef *weapDef,
                float posLerp,
                float *transScale,
                float *transShift);
void __cdecl CG_DrawReticleCenter(
                int localClientNum,
                const WeaponDef *weapDef,
                const float *color,
                float centerX,
                float centerY,
                float centerW,
                float transScale);
void __cdecl CG_DrawReticleSides(
                int localClientNum,
                unsigned int weaponIndex,
                const float *baseColor,
                float centerX,
                float centerY,
                float centerW,
                float transScale);
void __cdecl CG_CalcReticleSpread(
                const cg_s *cgameGlob,
                const WeaponDef *weapDef,
                const float *drawSize,
                float transScale,
                float *spread);
void __cdecl CG_CalcReticleColor(const float *baseColor, float alpha, float aimSpreadScale, float *reticleColor);
void __cdecl CG_CalcReticleImageOffset(const float *drawSize, float *imageTexelOffset);
