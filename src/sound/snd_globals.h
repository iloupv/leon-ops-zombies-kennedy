#pragma once
#include "snd.h"

snd_category_t __cdecl SND_GroupCategory(unsigned int group);
unsigned __int16 __cdecl SND_GroupAttenuationRaw(const snd_group *g);
double __cdecl SND_GroupGetAttenuation(unsigned int group);
unsigned int __cdecl SND_GetGroupCount();
const snd_group *__cdecl SND_GetGroupByIndex(unsigned int index);
const snd_master *__cdecl SND_GetMaster(unsigned int id);
const snd_master *__cdecl SND_GetMasterByConfig();
const snd_master *__cdecl SND_GetMasterCurrent();
const snd_curve *__cdecl SND_GetCurveById(unsigned int id);
const snd_curve *__cdecl SND_GetCurveByIndex(unsigned int i);
const snd_pan *__cdecl SND_GetPanByIndex(unsigned int i);
unsigned int __cdecl SND_FindContextIndex(unsigned int type);
int __cdecl SND_FindContextValueIndex(unsigned int type, unsigned int value);
double __cdecl Snd_CurveEval(const snd_curve *curve, float x);
double __cdecl Snd_CurveEvalOverRange(const snd_curve *curve, float x, float minimum, float maximum);
