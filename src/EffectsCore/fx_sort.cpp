#include "fx_sort.h"
#include "fx_archive.h"
#include "fx_update_util.h"

void __cdecl FX_SortEffects(FxSystem *system)
{
    FxEffectContainer *firstEffect; // [esp+14h] [ebp-1024h]
    float v2[1024]; // [esp+18h] [ebp-1020h]
    int v3; // [esp+1018h] [ebp-20h]
    unsigned __int16 v4; // [esp+101Ch] [ebp-1Ch]
    float *p2; // [esp+1020h] [ebp-18h]
    volatile int j; // [esp+1024h] [ebp-14h]
    int v7; // [esp+1028h] [ebp-10h]
    FxEffect *secondEffect; // [esp+102Ch] [ebp-Ch]
    float v9; // [esp+1030h] [ebp-8h]
    volatile int i; // [esp+1034h] [ebp-4h]

    if ( !system
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_sort.cpp", 100, 0, "%s", "system") )
    {
        __debugbreak();
    }
    p2 = (float *)system;
    FX_WaitBeginIteratingOverEffects_Exclusive(system);
    for ( i = system->shared->firstActiveEffect; i != system->shared->firstNewEffect; ++i )
    {
        v4 = system->shared->allEffectHandles[i & 0x3FF];
        firstEffect = FX_EffectFromHandle(system, v4);
        v9 = Vec3DistanceSq(firstEffect->effect.frameNow.origin, p2);
        for ( j = i; j != system->shared->firstActiveEffect; --j )
        {
            v3 = ((_WORD)j - 1) & 0x3FF;
            if ( v2[v3] >= (float)(v9 * 0.99989998) )
            {
                if ( v2[v3] >= (float)(v9 * 1.0001) )
                    break;
                secondEffect = (FxEffect *)FX_EffectFromHandle(system, system->shared->allEffectHandles[v3]);
                if ( secondEffect->owner != firstEffect->effect.owner
                    || !FX_FirstEffectIsFurther(&firstEffect->effect, secondEffect) )
                {
                    break;
                }
            }
            v7 = j & 0x3FF;
            v2[v7] = v2[v3];
            system->shared->allEffectHandles[v7] = system->shared->allEffectHandles[v3];
        }
        v7 = j & 0x3FF;
        v2[v7] = v9;
        system->shared->allEffectHandles[v7] = v4;
    }
    system->shared->iteratorCount = 0;
}

void __cdecl FX_WaitBeginIteratingOverEffects_Exclusive(FxSystem *system)
{
    volatile unsigned int *p_iteratorCount; // [esp+0h] [ebp-4h]

    if ( system->isArchiving
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\effectscore\\fx_system.h",
                    672,
                    0,
                    "%s",
                    "!system->isArchiving") )
    {
        __debugbreak();
    }
    p_iteratorCount = &system->shared->iteratorCount;
    do
    {
        while ( *p_iteratorCount )
            ;
    }
    while ( _InterlockedCompareExchange(p_iteratorCount, -1, 0) );
}

bool __cdecl FX_FirstEffectIsFurther(FxEffect *firstEffect, FxEffect *secondEffect)
{
    //if ( (unsigned __int8)*(unsigned int *)&firstEffect->boltAndSortOrder.0 == 255
    //    && (unsigned __int8)*(unsigned int *)&secondEffect->boltAndSortOrder.0 == 255 )
    //{
    //    return 0;
    //}
    //if ( (unsigned __int8)*(unsigned int *)&firstEffect->boltAndSortOrder.0 == 255 )
    //    firstEffect->boltAndSortOrder.0 = ($88D60DA77564256B22C3F95D9DD45A24)((unsigned __int8)FX_CalcRunnerParentSortOrder(firstEffect)
    //                                                                                                                                            | *(unsigned int *)&firstEffect->boltAndSortOrder.0
    //                                                                                                                                            & 0xFFFFFF00);
    //if ( (unsigned __int8)*(unsigned int *)&secondEffect->boltAndSortOrder.0 == 255 )
    //    secondEffect->boltAndSortOrder.0 = ($88D60DA77564256B22C3F95D9DD45A24)((unsigned __int8)FX_CalcRunnerParentSortOrder(secondEffect)
    //                                                                                                                                             | *(unsigned int *)&secondEffect->boltAndSortOrder.0
    //                                                                                                                                             & 0xFFFFFF00);
    //return (unsigned __int8)*(unsigned int *)&firstEffect->boltAndSortOrder.0 < (unsigned int)(unsigned __int8)*(unsigned int *)&secondEffect->boltAndSortOrder.0;

    if (firstEffect->boltAndSortOrder.sortOrder == 255
        && secondEffect->boltAndSortOrder.sortOrder == 255)
    {
        return 0;
    }
    if (firstEffect->boltAndSortOrder.sortOrder == 255)
        firstEffect->boltAndSortOrder.sortOrder = FX_CalcRunnerParentSortOrder(firstEffect);
    if (secondEffect->boltAndSortOrder.sortOrder == 255)
        secondEffect->boltAndSortOrder.sortOrder = FX_CalcRunnerParentSortOrder(secondEffect);
    return firstEffect->boltAndSortOrder.sortOrder < secondEffect->boltAndSortOrder.sortOrder;
}

int __cdecl FX_CalcRunnerParentSortOrder(FxEffect *effect)
{
    int v3; // [esp+8h] [ebp-24h]
    const FxEffectDef *def; // [esp+14h] [ebp-18h]
    int totalNonRunnerElemDefs; // [esp+18h] [ebp-14h]
    const FxElemDef *elemDef; // [esp+1Ch] [ebp-10h]
    int totalSortOrder; // [esp+24h] [ebp-8h]
    int elemDefIndex; // [esp+28h] [ebp-4h]

    if ( !effect
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_sort.cpp", 43, 0, "%s", "effect") )
    {
        __debugbreak();
    }
    def = effect->def;
    if ( !effect->def
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_sort.cpp", 46, 0, "%s", "def") )
    {
        __debugbreak();
    }
    totalSortOrder = 0;
    totalNonRunnerElemDefs = 0;
    for ( elemDefIndex = 0;
                elemDefIndex < def->elemDefCountLooping + def->elemDefCountOneShot + def->elemDefCountEmission;
                ++elemDefIndex )
    {
        elemDef = &effect->def->elemDefsEA[elemDefIndex];
        if ( elemDef->elemType != 12 )
        {
            totalSortOrder += elemDef->sortOrder;
            ++totalNonRunnerElemDefs;
        }
    }
    if ( totalNonRunnerElemDefs <= 0 )
        return 0;
    if ( totalSortOrder / totalNonRunnerElemDefs < 254 )
        v3 = totalSortOrder / totalNonRunnerElemDefs;
    else
        v3 = 254;
    if ( v3 > 0 )
        return v3;
    else
        return 0;
}

void __cdecl FX_SortNewElemsInEffect(FxSystem *system, FxEffect *effect)
{
    FxPool<FxElem,FxElemContainer> *v2; // [esp+14h] [ebp-18h]
    unsigned __int16 elemHandle; // [esp+1Ch] [ebp-10h]
    unsigned __int16 elemHandlea; // [esp+1Ch] [ebp-10h]
    unsigned __int16 stopElemHandle; // [esp+20h] [ebp-Ch]
    FxPool<FxElem,FxElemContainer> *remoteElem; // [esp+24h] [ebp-8h]

    elemHandle = effect->firstElemHandle[0];
    stopElemHandle = effect->firstSortedElemHandle;
    if ( elemHandle != stopElemHandle )
    {
        effect->firstElemHandle[0] = stopElemHandle;
        if ( stopElemHandle != 0xFFFF )
            FX_ElemFromHandle(system, stopElemHandle)->item.shared.prevElemHandleInEffect = -1;
        do
        {
            v2 = FX_ElemFromHandle(system, elemHandle);
            elemHandle = v2->item.shared.nextElemHandleInEffect;
            FX_SortSpriteElemIntoEffect(system, effect, (FxElem *)v2, (FxElem *)v2);
        }
        while ( elemHandle != stopElemHandle );
        effect->firstSortedElemHandle = effect->firstElemHandle[0];
        for ( elemHandlea = effect->firstElemHandle[0];
                    elemHandlea != 0xFFFF;
                    elemHandlea = remoteElem->item.shared.nextElemHandleInEffect )
        {
            remoteElem = FX_ElemFromHandle(system, elemHandlea);
            if ( effect->def->elemDefsEA[remoteElem->item.elem.defIndex].elemType > 5u
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_sort.cpp",
                            303,
                            0,
                            "FX_GetEffectElemDef( effect, elem->defIndex )->elemType <= FX_ELEM_TYPE_LAST_SPRITE\n\t%i, %i",
                            effect->def->elemDefsEA[remoteElem->item.elem.defIndex].elemType,
                            5) )
            {
                __debugbreak();
            }
        }
    }
}

void __cdecl FX_SortSpriteElemIntoEffect(FxSystem *system, FxEffect *effect, FxElem *remoteElem, FxElem *elem)
{
    unsigned __int16 nextElemHandle; // [esp+74h] [ebp-2Ch]
    FxInsertSortElem sortElem; // [esp+78h] [ebp-28h] BYREF
    unsigned __int16 elemHandle; // [esp+8Ch] [ebp-14h]
    FxElem *nextElem; // [esp+90h] [ebp-10h]
    unsigned __int16 *prevNextElemHandle; // [esp+94h] [ebp-Ch]
    FxElem *prevElem; // [esp+98h] [ebp-8h]
    unsigned __int16 prevElemHandle; // [esp+9Ch] [ebp-4h]

    nextElem = 0;
    prevElem = 0;
    prevNextElemHandle = 0;
    elemHandle = FX_ElemToHandle(system, remoteElem);
    prevElemHandle = -1;
    nextElemHandle = effect->firstElemHandle[0];
    if ( nextElemHandle != 0xFFFF )
    {
        FX_GetInsertSortElem(system, effect, elem, &sortElem);
        if ( sortElem.defSortOrder < 0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_sort.cpp",
                        224,
                        0,
                        "%s",
                        "sortElem.defSortOrder >= 0") )
        {
            __debugbreak();
        }
        do
        {
            nextElem = (FxElem *)FX_ElemFromHandle(system, nextElemHandle);
            if ( !FX_ExistingElemSortsBeforeNewElem(system, effect, nextElem, &sortElem) )
                break;
            prevElem = nextElem;
            prevElemHandle = nextElemHandle;
            prevNextElemHandle = (unsigned __int16 *)&nextElem[1].defIndex;
            nextElemHandle = *(_WORD *)&nextElem[1].defIndex;
        }
        while ( nextElemHandle != 0xFFFF );
    }
    *(_WORD *)&remoteElem[1].defIndex = nextElemHandle;
    *(_WORD *)&remoteElem[1].atRestFraction = prevElemHandle;
    if ( elemHandle == nextElemHandle
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_sort.cpp",
                    242,
                    0,
                    "%s",
                    "elemHandle != nextElemHandle") )
    {
        __debugbreak();
    }
    if ( prevNextElemHandle )
        *prevNextElemHandle = elemHandle;
    else
        effect->firstElemHandle[0] = elemHandle;
    if ( nextElem )
    {
        if ( nextElemHandle != 0xFFFF )
            *(_WORD *)&nextElem[1].atRestFraction = elemHandle;
    }
}

int __cdecl FX_ElemToHandle(FxSystem *system, FxElem *elem)
{
    if ( !system
        && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\effectscore\\fx_system.h", 472, 0, "%s", "system") )
    {
        __debugbreak();
    }
    return FX_PoolToHandle_Generic<FxElem,FxElemContainer,FX_ELEM_POOL_MAX>(system->elems, elem);
}

void __cdecl FX_GetInsertSortElem(
                const FxSystem *system,
                const FxEffect *effect,
                const FxElem *elem,
                FxInsertSortElem *sortElem)
{
    float posWorld[3]; // [esp+14h] [ebp-44h] BYREF
    const FxElemDef *elemDef; // [esp+20h] [ebp-38h]
    int randomSeed; // [esp+24h] [ebp-34h]
    orientation_t orient; // [esp+28h] [ebp-30h] BYREF

    sortElem->msecBegin = elem->msecBegin;
    sortElem->defIndex = elem->defIndex;
    elemDef = &effect->def->elemDefsEA[elem->defIndex];
    sortElem->elemType = elemDef->elemType;
    if ( elemDef->elemType > 5u
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_sort.cpp",
                    161,
                    0,
                    "%s\n\t(elemDef->elemType) = %i",
                    "(elemDef->elemType <= FX_ELEM_TYPE_LAST_SPRITE)",
                    elemDef->elemType) )
    {
        __debugbreak();
    }
    sortElem->defSortOrder = elemDef->sortOrder;
    randomSeed = (296 * elem->sequence + elem->msecBegin + (unsigned int)effect->randomSeed) % 0x1DF;
    FX_GetOrientation(elemDef, &effect->frameAtSpawn, &effect->frameNow, randomSeed, &orient);
    FX_OrientationPosToWorldPos(&orient, elem->origin, posWorld);
    sortElem->distToCamSq = Vec3DistanceSq(posWorld, system->cameraPrev.origin);
}

bool __cdecl FX_ExistingElemSortsBeforeNewElem(
                const FxSystem *system,
                const FxEffect *effect,
                const FxElem *remoteElem,
                const FxInsertSortElem *sortElemNew)
{
    float posWorld[3]; // [esp+18h] [ebp-4Ch] BYREF
    float distToCamSq; // [esp+24h] [ebp-40h]
    const FxElemDef *elemDef; // [esp+28h] [ebp-3Ch]
    int randomSeed; // [esp+2Ch] [ebp-38h]
    orientation_t orient; // [esp+30h] [ebp-34h] BYREF
    const FxElem *elem; // [esp+60h] [ebp-4h]

    elem = remoteElem;
    elemDef = &effect->def->elemDefsEA[remoteElem->defIndex];
    if ( elemDef->elemType == 5
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_sort.cpp",
                    185,
                    0,
                    "%s",
                    "elemDef->elemType != FX_ELEM_TYPE_TRAIL") )
    {
        __debugbreak();
    }
    if ( elemDef->elemType > 5u )
        return 1;
    if ( !elemDef->visualCount )
        return 1;
    if ( elemDef->sortOrder < sortElemNew->defSortOrder )
        return 1;
    if ( elemDef->sortOrder > sortElemNew->defSortOrder )
        return 0;
    randomSeed = (296 * elem->sequence + elem->msecBegin + (unsigned int)effect->randomSeed) % 0x1DF;
    FX_GetOrientation(elemDef, &effect->frameAtSpawn, &effect->frameNow, randomSeed, &orient);
    FX_OrientationPosToWorldPos(&orient, elem->origin, posWorld);
    distToCamSq = Vec3DistanceSq(posWorld, system->cameraPrev.origin);
    return distToCamSq > sortElemNew->distToCamSq;
}

//int __cdecl FX_PoolToHandle_Generic<FxElem,FxElemContainer,FX_ELEM_POOL_MAX>(
//                FxPool<FxElem,FxElemContainer> *poolArray,
//                FxElem *item_slim)
//{
//    const char *v2; // eax
//
//    if ( !item_slim || item_slim < (FxElem *)poolArray || item_slim >= (FxElem *)&poolArray[2048] )
//    {
//        v2 = va("%p %p", poolArray, item_slim);
//        if ( !Assert_MyHandler(
//                        "c:\\projects_pc\\cod\\codsrc\\src\\effectscore\\fx_system.h",
//                        457,
//                        0,
//                        "%s\n\t%s",
//                        "item && item >= &poolArray[0].item && item < &poolArray[LIMIT].item",
//                        v2) )
//            __debugbreak();
//    }
//    return ((char *)item_slim - (char *)poolArray) / 4;
//}

