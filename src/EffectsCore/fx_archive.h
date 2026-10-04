#pragma once
#include <gfx_d3d/fxprimitives.h>

FxEffectContainer *__cdecl FX_EffectFromHandle(FxSystem *system, unsigned __int16 handle);
FxPool<FxElem,FxElemContainer> *__cdecl FX_ElemFromHandle(FxSystem *system, unsigned __int16 handle);
FxElemVisuals __cdecl FX_GetElemVisuals(const FxElemDef *elemDef, int randomSeed);

