#pragma once
#include "aim_assist.h"

AimTargetGlob *__cdecl AimTarget_GetGlobArray(int localClientNum);
void __cdecl AimTarget_Init(int localClientNum);
void AimTarget_RegisterDvars();
void __cdecl expandMins(float *mins, float *point);
void __cdecl expandMaxs(float *maxs, float *point);
void __cdecl AimTarget_ProcessEntityInternal(int localClientNum, const centity_s *ent);
bool __cdecl AimTarget_IsTargetValid(const cg_s *cgameGlob, const centity_s *targetEnt);
double __cdecl AimTarget_GetTargetRadius(const centity_s *targetEnt);
void __cdecl AimTarget_GetTargetBounds(const centity_s *targetEnt, float *mins, float *maxs);
int __cdecl AimTarget_GetTagUpdateInterval(const centity_s *ent);
void __cdecl AimTarget_GetTargetTop(const centity_s *targetEnt, float *top);
bool __cdecl AimTarget_IsTargetVisible(int localClientNum, const centity_s *targetEnt, unsigned int visBone);
void __cdecl AimTarget_GetTargetCenter(const centity_s *targetEnt, float *center);
void __cdecl AimTarget_CreateTarget(int localClientNum, const centity_s *targetEnt, AimTarget *target);
void __cdecl AimTarget_AddTargetToList(int localClientNum, const AimTarget *target);
int __cdecl AimTarget_CompareTargets(const AimTarget *targetA, const AimTarget *targetB);
bool __cdecl AimTarget_PlayerInValidState(const playerState_s *ps);
void __cdecl AimTarget_UpdateClientTargets(int localClientNum);
void __cdecl AimTarget_ClearTargetList(int localClientNum);
void __cdecl AimTarget_ProcessEntity(int localClientNum, const centity_s *ent);
void __cdecl AimTarget_GetClientTargetList(int localClientNum, AimTargetGlob **targetList, int *targetCount);
