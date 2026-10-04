#pragma once
#include <gfx_d3d/fxprimitives.h>

FxEffectContainer *__cdecl UniqueHandleToEffect(int localClientNum, unsigned int h);
FxEffectContainer *__cdecl UniqueHandleToEffect(FxSystem *system, unsigned int handle);
void __cdecl UniqueHandleAssignHandle(FxSystem *system, FxEffectContainer *effect, FxEffectContainer *remoteEffect);
void __cdecl UniqueHandleRelease(FxSystem *system, FxEffect *effect);
void __cdecl UniqueHandleInit(FxUniqueHandleDb *uniqueHandleDb);
