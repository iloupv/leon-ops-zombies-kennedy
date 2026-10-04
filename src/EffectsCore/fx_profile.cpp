#include "fx_profile.h"
#include "fx_system.h"
#include "fx_draw.h"
#include "fx_archive.h"

void __cdecl FX_DrawProfile(int clientIndex, void (__cdecl *drawFunc)(char *), float *profilePos)
{
    char *v3; // eax
    char *v4; // eax
    char *v5; // eax
    char *v6; // eax
    char *v7; // eax
    char *v8; // eax
    char *v9; // eax
    char *v10; // eax
    int v11; // [esp+2Ch] [ebp-701Ch]
    int j; // [esp+30h] [ebp-7018h]
    FxEffectContainer *effect; // [esp+34h] [ebp-7014h]
    int entryCount; // [esp+38h] [ebp-7010h] BYREF
    FxSystem *system; // [esp+3Ch] [ebp-700Ch]
    FxProfileEntry entryPool[1024]; // [esp+40h] [ebp-7008h] BYREF
    FxProfileEntry *entry; // [esp+7040h] [ebp-8h]
    volatile int i; // [esp+7044h] [ebp-4h]

    system = (FxSystem *)FX_GetSystem(clientIndex);
    if ( system )
    {
        FX_BeginIteratingOverEffects_Cooperative(system);
        entryCount = 0;
        for ( i = system->shared->firstActiveEffect; i != system->shared->firstNewEffect; ++i )
        {
            effect = FX_EffectFromHandle(system, system->shared->allEffectHandles[i & 0x3FF]);
            entry = FX_GetProfileEntry(effect->effect.def, entryPool, &entryCount);
            FX_ProfileSingleEffect(system, &effect->effect, entry);
        }
        if ( !_InterlockedDecrement(&system->shared->iteratorCount) )
            FX_RunGarbageCollectionAndPrioritySort(system);
        qsort(entryPool, entryCount, 0x1Cu, (int (__cdecl *)(const void *, const void *))FX_CompareProfileEntries);
        v11 = system->shared->firstNewEffect - system->shared->firstActiveEffect;
        v3 = va(
                     "%4i of %4i effect objects in use (%.0f%%; %i free)",
                     v11,
                     1024,
                     (float)((float)v11 * 0.09765625),
                     1024 - v11);
        drawFunc(v3);
        v4 = va(
                     "%4i of %4i trails in use (%.0f%%; %i free)",
                     system->shared->activeTrailCount,
                     128,
                     (float)((float)system->shared->activeTrailCount * 0.78125),
                     128 - system->shared->activeTrailCount);
        drawFunc(v4);
        v5 = va(
                     "%4i of %4i spot lights in use (%.0f%%; %i free)",
                     system->shared->activeSpotLightEffectCount,
                     1,
                     (float)((float)system->shared->activeSpotLightEffectCount * 100.0),
                     1 - system->shared->activeSpotLightEffectCount);
        drawFunc(v5);
        v6 = va(
                     "%4i of %4i effect elements in use (%.0f%%; %i free)",
                     system->shared->activeElemCount,
                     fx_elemLimit, // mod (L43): r_mod_fxElems (retail 2048)
                     (float)((float)system->shared->activeElemCount * 100.0f / (float)fx_elemLimit),
                     fx_elemLimit - system->shared->activeElemCount);
        drawFunc(v6);
        v7 = va(
                     "%4i of %4i trail elements in use (%.0f%%; %i free)",
                     system->shared->activeTrailElemCount,
                     2048,
                     (float)((float)system->shared->activeTrailElemCount * 0.048828125),
                     2048 - system->shared->activeTrailElemCount);
        drawFunc(v7);
        v8 = va(
                     "%4i of %4i cloud elements in use (%.0f%%; %i free)",
                     system->gfxCloudCount,
                     256,
                     (float)((float)system->gfxCloudCount * 0.390625),
                     256 - system->gfxCloudCount);
        drawFunc(v8);
        ((void (__cdecl *)(const char *, float *))drawFunc)("", profilePos);
        ((void (__cdecl *)(const char *, float *))drawFunc)(
            "Effects are sorted by percent of available resources used",
            profilePos);
        v9 = va(
                     "%4s    %4s (%4s/%4s)    %4s    %4s (%4s/%4s)    %s",
                     "objs",
                     "elem",
                     "actv",
                     "pend",
                     "trls",
                     "telm",
                     "actv",
                     "pend",
                     "effectname");
        drawFunc(v9);
        ((void (__cdecl *)(const char *, float *))drawFunc)(
            "-------------------------------------------------------------------",
            profilePos);
        for ( j = 0; j < 30 && j < entryCount; ++j )
        {
            entry = &entryPool[j];
            v10 = va(
                            "%4i    %4i (%4i/%4i)    %4i    %4i (%4i/%4i)    %s",
                            entry->effectCount,
                            entry->pendingElemCount + entry->activeElemCount,
                            entry->activeElemCount,
                            entry->pendingElemCount,
                            entry->trailCount,
                            entry->pendingTrailElemCount + entry->activeTrailElemCount,
                            entry->activeTrailElemCount,
                            entry->pendingTrailElemCount,
                            entry->effectDef->name);
            drawFunc(v10);
        }
    }
}

FxProfileEntry *__cdecl FX_GetProfileEntry(const FxEffectDef *effectDef, FxProfileEntry *entryPool, int *entryCount)
{
    int entryIndex; // [esp+0h] [ebp-4h]

    for ( entryIndex = 0; entryIndex < *entryCount; ++entryIndex )
    {
        if ( entryPool[entryIndex].effectDef == effectDef )
            return &entryPool[entryIndex];
    }
    if ( entryIndex != *entryCount
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_profile.cpp",
                    78,
                    0,
                    "entryIndex == *entryCount\n\t%i, %i",
                    entryIndex,
                    *entryCount) )
    {
        __debugbreak();
    }
    ++*entryCount;
    entryPool[entryIndex].effectDef = effectDef;
    entryPool[entryIndex].effectCount = 0;
    entryPool[entryIndex].activeElemCount = 0;
    entryPool[entryIndex].pendingElemCount = 0;
    entryPool[entryIndex].trailCount = 0;
    entryPool[entryIndex].activeTrailElemCount = 0;
    entryPool[entryIndex].pendingTrailElemCount = 0;
    return &entryPool[entryIndex];
}

void __cdecl FX_ProfileSingleEffect(FxSystem *system, const FxEffect *effect, FxProfileEntry *entry)
{
    unsigned __int16 elemHandle; // [esp+4h] [ebp-1Ch]
    unsigned __int16 trailHandle; // [esp+8h] [ebp-18h]
    FxPool<FxTrail,FxTrail> *trail; // [esp+Ch] [ebp-14h]
    FxPool<FxTrailElem,FxTrailElem> *trailElem; // [esp+10h] [ebp-10h]
    FxPool<FxElem,FxElemContainer> *elem; // [esp+14h] [ebp-Ch]
    unsigned int elemClass; // [esp+18h] [ebp-8h]
    unsigned __int16 trailElemHandle; // [esp+1Ch] [ebp-4h]

    ++entry->effectCount;
    for ( elemClass = 0; elemClass < 3; ++elemClass )
    {
        for ( elemHandle = effect->firstElemHandle[elemClass];
                    elemHandle != 0xFFFF;
                    elemHandle = elem->item.shared.nextElemHandleInEffect )
        {
            elem = FX_ElemFromHandle(system, elemHandle);
            if ( elem->item.elem.msecBegin > system->msecNow )
                ++entry->pendingElemCount;
            else
                ++entry->activeElemCount;
        }
    }
    for ( trailHandle = effect->firstTrailHandle; trailHandle != 0xFFFF; trailHandle = trail->item.nextTrailHandle )
    {
        trail = FX_TrailFromHandle(system, trailHandle);
        ++entry->trailCount;
        for ( trailElemHandle = trail->item.firstElemHandle;
                    trailElemHandle != 0xFFFF;
                    trailElemHandle = trailElem->item.nextTrailElemHandle )
        {
            trailElem = FX_TrailElemFromHandle(system, trailElemHandle);
            if ( trailElem->item.msecBegin > system->msecNow )
                ++entry->pendingTrailElemCount;
            else
                ++entry->activeTrailElemCount;
        }
    }
}

int __cdecl FX_CompareProfileEntries(const FxProfileEntry *e0, const FxProfileEntry *e1)
{
    float cost; // [esp+8h] [ebp-8h]
    float cost_4; // [esp+Ch] [ebp-4h]

    cost = FX_GetProfileEntryCost(e0);
    cost_4 = FX_GetProfileEntryCost(e1);
    if ( cost_4 > cost )
        return 1;
    if ( cost <= cost_4 )
        return 0;
    return -1;
}

double __cdecl FX_GetProfileEntryCost(const FxProfileEntry *entry)
{
    if ( (float)((float)((float)entry->effectCount * 0.0009765625)
                         - (float)((float)(entry->pendingElemCount + entry->activeElemCount) * 0.00048828125)) < 0.0 )
        return (float)((float)(entry->pendingElemCount + entry->activeElemCount) * 0.00048828125);
    else
        return (float)((float)entry->effectCount * 0.0009765625);
}

void __cdecl FX_DrawPriorityDebug(int clientIndex, void (__cdecl *drawFunc)(char *), float *profilePos)
{
    char *v3; // eax
    char *v4; // eax
    char *v5; // eax
    char *v6; // eax
    char *v7; // eax
    int v8; // [esp+10h] [ebp-204Ch]
    int k; // [esp+34h] [ebp-2028h]
    int j; // [esp+38h] [ebp-2024h]
    unsigned int *v11; // [esp+3Ch] [ebp-2020h]
    volatile int i; // [esp+40h] [ebp-201Ch]
    int v13; // [esp+44h] [ebp-2018h]
    FxEffectContainer *v14; // [esp+48h] [ebp-2014h]
    FxSystemContainer *system; // [esp+4Ch] [ebp-2010h]
    int num; // [esp+50h] [ebp-200Ch]
    unsigned int base[2049]; // [esp+54h] [ebp-2008h] BYREF
    int v18; // [esp+2058h] [ebp-4h]

    system = FX_GetSystem(clientIndex);
    if ( !system
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_profile.cpp", 257, 0, "%s", "system") )
    {
        __debugbreak();
    }
    FX_BeginIteratingOverEffects_Cooperative(&system->system);
    num = 0;
    for ( i = system->system.shared->firstActiveEffect; i != system->system.shared->firstNewEffect; ++i )
    {
        v14 = FX_EffectFromHandle(&system->system, system->system.shared->allEffectHandles[i & 0x3FF]);
        v11 = 0;
        for ( j = 0; j < num; ++j )
        {
            if ( (const FxEffectDef *)base[2 * j] == v14->effect.def )
            {
                v11 = &base[2 * j];
                break;
            }
        }
        if ( v11 )
        {
            ++v11[1];
        }
        else
        {
            if ( num >= 1024
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_profile.cpp",
                            280,
                            0,
                            "%s",
                            "entryCount < FX_EFFECT_LIMIT") )
            {
                __debugbreak();
            }
            base[2 * num] = (unsigned int)v14->effect.def;
            base[2 * num++ + 1] = 1;
        }
    }
    if ( !_InterlockedDecrement(&system->system.shared->iteratorCount) )
        FX_RunGarbageCollectionAndPrioritySort(&system->system);
    qsort(base, num, 8u, (int (__cdecl *)(const void *, const void *))FX_ComparePriorityDebugEntries);
    v18 = system->system.priorityInfo.warningHigh[system->system.priorityInfo.readIndex];
    v13 = system->system.shared->firstNewEffect - system->system.shared->firstActiveEffect;
    v3 = va("%4i of %4i effect objects in use (%.0f%%; %i free)", v13, 1024, (float)((float)v13 * 0.09765625), 1024 - v13);
    drawFunc(v3);
    v4 = va(
                 "%4i of %4i effect elements in use (%.0f%%; %i free)",
                 system->system.shared->activeElemCount,
                 fx_elemLimit, // mod (L43): r_mod_fxElems (retail 2048)
                 (float)((float)system->system.shared->activeElemCount * 100.0f / (float)fx_elemLimit),
                 fx_elemLimit - system->system.shared->activeElemCount);
    drawFunc(v4);
    v5 = va(
                 "priority warning high %d. Client %d, handles %d",
                 v18,
                 clientIndex,
                 system->system.uniqueHandleDb->numNodesInUse);
    drawFunc(v5);
    ((void (__cdecl *)(const char *, float *))drawFunc)("", profilePos);
    v6 = va("%4s     %4s     %s", "prio", "count", "effectname");
    drawFunc(v6);
    ((void (__cdecl *)(const char *, float *))drawFunc)(
        "-------------------------------------------------------------------",
        profilePos);
    if ( num > 40 )
        v8 = 40;
    else
        v8 = num;
    for ( k = 0; k < v8; ++k )
    {
        v7 = va("%4i     %4i     %s", *(unsigned __int8 *)(base[2 * k] + 5), base[2 * k + 1], *(const char **)base[2 * k]);
        drawFunc(v7);
    }
}

int __cdecl FX_ComparePriorityDebugEntries(unsigned int *e0, unsigned int *e1)
{
    int ap; // [esp+0h] [ebp-18h]
    int ac; // [esp+4h] [ebp-14h]
    int v5; // [esp+Ch] [ebp-Ch]
    int bc; // [esp+14h] [ebp-4h]

    ap = *(unsigned __int8 *)(*e0 + 5);
    v5 = *(unsigned __int8 *)(*e1 + 5);
    ac = e0[1];
    bc = e1[1];
    if ( ap > v5 )
        return -1;
    if ( ap < v5 )
        return 1;
    if ( ac <= bc )
        return ac < bc;
    return -1;
}

void __cdecl FX_DrawMarkProfile(unsigned int clientIndex, void (__cdecl *drawFunc)(char *), float *profilePos)
{
    const char *v3; // eax
    char *v4; // eax
    char *v5; // eax
    char *v6; // eax
    char *v7; // eax
    char *v8; // eax
    char *v9; // eax
    char *v10; // eax
    unsigned __int16 v11; // ax
    int modelIndex; // [esp-Ch] [ebp-60h]
    FxMarksSystem *marksSystem; // [esp+Ch] [ebp-48h]
    int worldBrushMarks; // [esp+10h] [ebp-44h]
    int freeMarks2; // [esp+1Ch] [ebp-38h]
    int entityIndex; // [esp+20h] [ebp-34h]
    int wastedPointGroups; // [esp+24h] [ebp-30h]
    FxTriGroupPool *triGroup; // [esp+28h] [ebp-2Ch]
    FxPointGroupPool *pointGroup; // [esp+2Ch] [ebp-28h]
    int entBrushMarks; // [esp+30h] [ebp-24h]
    int freeMarks; // [esp+34h] [ebp-20h]
    int wastedTriGroups; // [esp+38h] [ebp-1Ch]
    int freePointGroups; // [esp+3Ch] [ebp-18h]
    int entModelMarks; // [esp+40h] [ebp-14h]
    int worldModelMarks; // [esp+44h] [ebp-10h]
    unsigned __int16 markHandle; // [esp+48h] [ebp-Ch]
    int freeTriGroups; // [esp+4Ch] [ebp-8h]
    FxMark *markIter; // [esp+50h] [ebp-4h]
    FxMark *markItera; // [esp+50h] [ebp-4h]

    marksSystem = FX_GetMarksSystem(clientIndex);
    markHandle = marksSystem->firstFreeMarkHandle;
    freeMarks = 0;
    while ( markHandle != 0xFFFF )
    {
        ++freeMarks;
        markHandle = FX_MarkFromHandle(marksSystem, markHandle)->nextMark;
    }
    freeTriGroups = 0;
    for ( triGroup = marksSystem->firstFreeTriGroup; triGroup; triGroup = triGroup->nextFreeTriGroup )
        ++freeTriGroups;
    freePointGroups = 0;
    for ( pointGroup = marksSystem->firstFreePointGroup; pointGroup; pointGroup = pointGroup->nextFreePointGroup )
        ++freePointGroups;
    freeMarks2 = 0;
    worldBrushMarks = 0;
    entModelMarks = 0;
    entBrushMarks = 0;
    worldModelMarks = 0;
    wastedPointGroups = 0;
    wastedTriGroups = 0;
    for ( markIter = marksSystem->marks; markIter != (FxMark *)marksSystem->triGroups; ++markIter )
    {
        if ( markIter->frameCountDrawn == -1 )
        {
            ++freeMarks2;
        }
        else
        {
            wastedPointGroups += markIter->pointCount & 0x80000001;
            wastedTriGroups += markIter->triCount & 1;
            switch ( markIter->context.modelTypeAndSurf & 0xC0 )
            {
                case 0:
                    ++worldBrushMarks;
                    break;
                case 0x40:
                    ++worldModelMarks;
                    break;
                case 0x80:
                    ++entBrushMarks;
                    break;
                case 0xC0:
                    ++entModelMarks;
                    break;
                default:
                    continue;
            }
        }
    }
    if ( freeMarks != freeMarks2 )
    {
        v3 = va("%i %i", freeMarks, freeMarks2);
        if ( !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_profile.cpp",
                        493,
                        0,
                        "%s\n\t%s",
                        "freeMarks == freeMarks2",
                        v3) )
            __debugbreak();
    }
    v4 = va(
                 "%4i of %4i marks in use (%i alloced, %i freed)",
                 256 - freeMarks,
                 256,
                 marksSystem->allocedMarkCount,
                 marksSystem->freedMarkCount);
    drawFunc(v4);
    v5 = va("%4i of %4i triGroups in use (%4i wasted)", 1024 - freeTriGroups, 1024, wastedTriGroups);
    drawFunc(v5);
    v6 = va("%4i of %4i pointGroups in use (%4i wasted)", 1536 - freePointGroups, 1536, wastedPointGroups);
    drawFunc(v6);
    ((void (__cdecl *)(const char *, float *))drawFunc)(" ", profilePos);
    v7 = va("%4i World Brush Marks", worldBrushMarks);
    drawFunc(v7);
    v8 = va("%4i World Model Marks", worldModelMarks);
    drawFunc(v8);
    v9 = va("%4i Ent Brush Marks", entBrushMarks);
    drawFunc(v9);
    v10 = va("%4i Ent Model", entModelMarks);
    drawFunc(v10);
    FX_DrawMarkProfile_MarkPrint(
        marksSystem,
        marksSystem->firstActiveWorldMarkHandle,
        "world",
        0,
        (void (__cdecl *)(const char *, float *))drawFunc,
        profilePos);
    for ( entityIndex = 0; entityIndex != 1536; ++entityIndex )
        FX_DrawMarkProfile_MarkPrint(
            marksSystem,
            marksSystem->entFirstMarkHandles[entityIndex],
            "ent",
            entityIndex,
            (void (__cdecl *)(const char *, float *))drawFunc,
            profilePos);
    for ( markItera = marksSystem->marks; markItera != (FxMark *)marksSystem->triGroups; ++markItera )
    {
        if ( markItera->frameCountDrawn != -1
            && markItera->prevMark == 0xFFFF
            && (markItera->context.modelTypeAndSurf & 0xC0) == 0x40 )
        {
            modelIndex = markItera->context.modelIndex;
            v11 = FX_MarkToHandle(marksSystem, markItera);
            FX_DrawMarkProfile_MarkPrint(
                marksSystem,
                v11,
                "sm",
                modelIndex,
                (void (__cdecl *)(const char *, float *))drawFunc,
                profilePos);
        }
    }
}

void __cdecl FX_DrawMarkProfile_MarkPrint(
                FxMarksSystem *marksSystem,
                unsigned __int16 head,
                const char *name,
                int index,
                void (__cdecl *drawFunc)(const char *, float *),
                float *profilePos)
{
    const char *v6; // eax
    const char *v7; // eax
    const char *v8; // eax
    unsigned __int8 thisMarkType; // [esp+3h] [ebp-1Dh]
    FxMark *mark; // [esp+4h] [ebp-1Ch]
    int markCount; // [esp+8h] [ebp-18h]
    int triCount; // [esp+Ch] [ebp-14h]
    int pointCount; // [esp+10h] [ebp-10h]
    unsigned __int8 type; // [esp+17h] [ebp-9h]
    unsigned __int16 lastMarkHandle; // [esp+18h] [ebp-8h]

    markCount = 0;
    triCount = 0;
    pointCount = 0;
    type = 0;
    lastMarkHandle = -1;
    while ( head != 0xFFFF )
    {
        mark = FX_MarkFromHandle(marksSystem, head);
        if ( mark->frameCountDrawn == -1
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_profile.cpp",
                        394,
                        0,
                        "%s",
                        "mark->frameCountDrawn != FX_MARK_FREE") )
        {
            __debugbreak();
        }
        if ( mark->prevMark != lastMarkHandle
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_profile.cpp",
                        395,
                        0,
                        "%s",
                        "mark->prevMark == lastMarkHandle") )
        {
            __debugbreak();
        }
        triCount += mark->triCount;
        pointCount += mark->pointCount;
        thisMarkType = mark->context.modelTypeAndSurf & 0xC0;
        if ( !markCount )
            type = mark->context.modelTypeAndSurf & 0xC0;
        if ( type != thisMarkType )
        {
            v6 = va("%i %i", type, thisMarkType);
            if ( !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_profile.cpp",
                            401,
                            0,
                            "%s\n\t%s",
                            "type == thisMarkType",
                            v6) )
                __debugbreak();
        }
        ++markCount;
        lastMarkHandle = head;
        head = mark->nextMark;
    }
    if ( markCount )
    {
        v7 = typeAsString(type);
        v8 = va("%s%5i %s %i (%i pts, %i tris)", name, index, v7, markCount, pointCount, triCount);
        drawFunc(v8, profilePos);
    }
}

const char *__cdecl typeAsString(unsigned __int8 type)
{
    const char *result; // eax

    switch ( type )
    {
        case 0u:
            result = "wbrush";
            break;
        case 0x40u:
            result = "wmodel";
            break;
        case 0x80u:
            result = "ebrush";
            break;
        case 0xC0u:
            result = "emodel";
            break;
        default:
            if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_profile.cpp", 323, 0, "unpossible") )
                __debugbreak();
            result = "unknown";
            break;
    }
    return result;
}

