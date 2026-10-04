#pragma once
#include "actor.h"

char __fastcall Actor_CanSeePointFrom(
                actor_s *self,
                const float *vStart,
                const float *vEnd,
                float fMaxDistSqrd,
                int ignoreEntityNum);
char __fastcall Actor_SightTrace(actor_s *self, const float *start, const float *end, int passEntNum);
char __fastcall Actor_CanSeePointEx(
                actor_s *self,
                const float *vPoint,
                float fovDot,
                float fMaxDistSqrd,
                int ignoreEntityNum);
char __fastcall Actor_CanSeePointExInternal(
                actor_s *self,
                const float *vPoint,
                float fovDot,
                float fMaxDistSqrd,
                int ignoreEntityNum,
                float *vViewPos);
bool __fastcall PointInFovAndRange(
                actor_s *self,
                const float *vEyePos,
                const float *vPoint,
                float fovDot,
                float fMaxDistSqrd);
char __fastcall Actor_CanSeeEntity(actor_s *self, const gentity_s *ent);
void __fastcall Actor_UpdateVisCache(actor_s *self, const gentity_s *ent, sentient_info_t *pInfo, bool bVisible);
char __fastcall Actor_CanSeeEntityEx(actor_s *self, const gentity_s *ent, float fovDot, float fMaxDistSqrd);
char __fastcall Actor_CanSeeSentient(actor_s *self, sentient_s *sentient, int iMaxLatency);
char __fastcall Actor_CanSeeEnemy(actor_s *self);
void __fastcall Actor_UpdateSight(actor_s *self);
int __cdecl compare_sentient_sort(unsigned int *pe1, unsigned int *pe2);
void __fastcall Actor_UpdateLastKnownPos(actor_s *self, sentient_s *other);
void __fastcall Actor_UpdateLastEnemySightPos(actor_s *self);
void __fastcall Actor_GetEyePosition(actor_s *self, float *vEyePosOut);
void __fastcall Actor_UpdateEyeInformation(actor_s *self);
void __fastcall Actor_GetDebugEyePosition(actor_s *self, float *vEyePosOut);
void __fastcall Actor_GetEyeDirection(actor_s *self, float *vEyeDir);
void __fastcall Actor_GetEyeOffset(actor_s *self, float *vEyePosOut);
int __fastcall Actor_GetMuzzleInfo(actor_s *self, float *vOrigin, float *vForward);
