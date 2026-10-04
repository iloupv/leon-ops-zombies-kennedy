#pragma once
#include <gfx_d3d/fxprimitives.h>

struct __declspec(align(4)) FxInsertSortElem // sizeof=0x14
{                                       // XREF: FX_SortSpriteElemIntoEffect/r
    int defSortOrder;                   // XREF: FX_SortSpriteElemIntoEffect+70/r
    float distToCamSq;
    int msecBegin;
    int defIndex;
    unsigned __int8 elemType;
    // padding byte
    // padding byte
    // padding byte
};

void __cdecl FX_SortEffects(FxSystem *system);
void __cdecl FX_WaitBeginIteratingOverEffects_Exclusive(FxSystem *system);
bool __cdecl FX_FirstEffectIsFurther(FxEffect *firstEffect, FxEffect *secondEffect);
int __cdecl FX_CalcRunnerParentSortOrder(FxEffect *effect);
void __cdecl FX_SortNewElemsInEffect(FxSystem *system, FxEffect *effect);
void __cdecl FX_SortSpriteElemIntoEffect(FxSystem *system, FxEffect *effect, FxElem *remoteElem, FxElem *elem);
int __cdecl FX_ElemToHandle(FxSystem *system, FxElem *elem);
void __cdecl FX_GetInsertSortElem(
                const FxSystem *system,
                const FxEffect *effect,
                const FxElem *elem,
                FxInsertSortElem *sortElem);
bool __cdecl FX_ExistingElemSortsBeforeNewElem(
                const FxSystem *system,
                const FxEffect *effect,
                const FxElem *remoteElem,
                const FxInsertSortElem *sortElemNew);
//int __cdecl FX_PoolToHandle_Generic<FxElem,FxElemContainer,2048>(
//                FxPool<FxElem,FxElemContainer> *poolArray,
//                FxElem *item_slim);
