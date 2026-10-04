#pragma once

#include <gfx_d3d/fxprimitives.h>
#include "fx_marks.h"

struct FxProfileEntry // sizeof=0x1C
{                                       // XREF: ?FX_DrawProfile@@YAXHP6AXPBDQAM@Z1@Z/r
    const FxEffectDef *effectDef;
    int effectCount;
    int activeElemCount;
    int pendingElemCount;
    int trailCount;
    int activeTrailElemCount;
    int pendingTrailElemCount;
};

void __cdecl FX_DrawProfile(int clientIndex, void (__cdecl *drawFunc)(char *), float *profilePos);
FxProfileEntry *__cdecl FX_GetProfileEntry(const FxEffectDef *effectDef, FxProfileEntry *entryPool, int *entryCount);
void __cdecl FX_ProfileSingleEffect(FxSystem *system, const FxEffect *effect, FxProfileEntry *entry);
int __cdecl FX_CompareProfileEntries(const FxProfileEntry *e0, const FxProfileEntry *e1);
double __cdecl FX_GetProfileEntryCost(const FxProfileEntry *entry);
void __cdecl FX_DrawPriorityDebug(int clientIndex, void (__cdecl *drawFunc)(char *), float *profilePos);
int __cdecl FX_ComparePriorityDebugEntries(unsigned int *e0, unsigned int *e1);
void __cdecl FX_DrawMarkProfile(unsigned int clientIndex, void (__cdecl *drawFunc)(char *), float *profilePos);
void __cdecl FX_DrawMarkProfile_MarkPrint(
                FxMarksSystem *marksSystem,
                unsigned __int16 head,
                const char *name,
                int index,
                void (__cdecl *drawFunc)(const char *, float *),
                float *profilePos);
const char *__cdecl typeAsString(unsigned __int8 type);
