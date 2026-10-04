#pragma once
#include "r_rendercmds.h"

bool __cdecl RB_UsingDepthOfFieldFX(const GfxViewInfo *viewInfo);
bool __cdecl RB_UsingFlameFX(const GfxViewInfo *viewInfo);
bool __cdecl RB_UsingElectrifiedFX(const GfxViewInfo *viewInfo);
bool __cdecl RB_UsingTransportedFX(const GfxViewInfo *viewInfo);
bool __cdecl RB_UsingWaterSheetingFX(const GfxViewInfo *viewInfo);
bool __cdecl RB_UsingGenericFilter(const GfxViewInfo *viewInfo);
bool __cdecl RB_UsingReviveFX(const GfxViewInfo *viewInfo);
bool __cdecl RB_UsingBlur(const GfxViewInfo *viewInfo);
bool __cdecl RB_SetBloomConstants(const GfxViewInfo *viewInfo);
void __cdecl RB_BloomStreak(const GfxViewInfo *viewInfo, GfxRenderTargetId *srcRt, GfxRenderTargetId *dstRt);
void __cdecl RB_SetBlurConstants(float radius, float textureScaler);
void __cdecl RB_SetFilmCurveConstants(const GfxViewInfo *viewInfo);
// local variable allocation has failed, the output may be wrong!
void    RB_SetVisionSetColorCorrection(const GfxViewInfo *viewInfo);
void __cdecl RB_BloomLDR(const GfxViewInfo *viewInfo);
bool __cdecl R_UsingDoubleVision(const GfxViewInfo *viewInfo);
bool __cdecl RB_UsingColorManipulation(const GfxViewInfo *viewInfo);
void __cdecl RB_PoisonFX(const GfxViewInfo *viewInfo);
// local variable allocation has failed, the output may be wrong!
void    RB_GenericFilterFX(const GfxViewInfo *viewInfo);
void __cdecl RB_FlameFX(const GfxViewInfo *viewInfo);
void __cdecl RB_ElectrifiedFX(const GfxViewInfo *viewInfo);
void __cdecl RB_TransportedFX(const GfxViewInfo *viewInfo);
void __cdecl RB_WaterSheetingFX(const GfxViewInfo *viewInfo);
void __cdecl RB_ReviveFX(const GfxViewInfo *viewInfo);
void __cdecl RB_ProcessPostEffects(const GfxViewInfo *viewInfo);
bool __cdecl RB_UsingPoison(const GfxViewInfo *viewInfo);
bool __cdecl RB_UsingPostEffects(const GfxViewInfo *viewInfo);
bool __cdecl RB_UsingMergedPostEffects(const GfxViewInfo *viewInfo);
void RB_GetResolvedScene();
void    RB_ApplyDepthOfField(const GfxViewInfo *viewInfo);
void __cdecl RB_GetSceneDepthOfFieldEquation(
                float nearOutOfFocus,
                float nearInFocus,
                float farInFocus,
                float farOutOfFocus,
                float *dofEquation,
                float zNear);
void __cdecl RB_GetViewModelDepthOfFieldEquation(float outOfFocus, float inFocus, float *dofEquation);
double __cdecl RB_GetDepthOfFieldBlurFraction(const GfxViewInfo *viewInfo, float pixelRadiusAtSceneRes);
void __cdecl RB_GetDepthOfFieldInputImages(float radius);
double __cdecl RB_GetBlurRadius(float blurRadiusFromCode);
void __cdecl RB_BlurScreen(const GfxViewInfo *viewInfo, float blurRadius);
