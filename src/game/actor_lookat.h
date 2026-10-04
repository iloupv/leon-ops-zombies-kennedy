#pragma once
#include "actor.h"

void __cdecl Actor_InitLookAt(actor_s *self);
void __cdecl Actor_SetLookAtAnimNodes(
                actor_s *self,
                unsigned __int16 animStraight,
                unsigned __int16 animLeft,
                unsigned __int16 animRight);
void __cdecl Actor_SetLookAt(actor_s *self, const float *vPosition, float fTurnAccel);
void __cdecl Actor_SetLookAtYawLimits(actor_s *self, float fAnimYawLimit, float fYawLimit, float fBlendTime);
double __cdecl Actor_CurrentLookAtAnimYawMax(actor_s *self);
double __cdecl Actor_CurrentLookAtYawMax(actor_s *self);
void __cdecl Actor_StopLookAt(actor_s *self, float fTurnAccel);
void __cdecl Actor_UpdateLookAt(actor_s *self);
