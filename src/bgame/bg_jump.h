#pragma once

#include <universal/dvar.h>
#include "bg_public.h"
#include "bg_weapons.h"


void __cdecl Jump_RegisterDvars();
void Jump_UpdateSlowdownDefault();
void __cdecl Jump_ClearState(playerState_s *ps);
char __cdecl Jump_GetStepHeight(playerState_s *ps, const float *origin, float *stepSize);
bool __cdecl Jump_IsPlayerAboveMax(playerState_s *ps);
void __cdecl Jump_ActivateSlowdown(playerState_s *ps);
void __cdecl Jump_ApplySlowdown(playerState_s *ps);
double __cdecl Jump_ReduceFriction(playerState_s *ps);
double __cdecl Jump_GetSlowdownFriction(playerState_s *ps);
void __cdecl Jump_ClampVelocity(playerState_s *ps, const float *origin);
char __cdecl Jump_Check(pmove_t *pm, pml_t *pml);
void __cdecl Jump_Start(pmove_t *pm, pml_t *pml, float height);
double __cdecl Jump_GetLandFactor(playerState_s *ps);
void __cdecl Jump_PushOffLadder(playerState_s *ps, pml_t *pml);
void __cdecl Jump_AddSurfaceEvent(playerState_s *ps, pml_t *pml);
//void __thiscall bitarray<51>::resetBit(bitarray<51> *this, unsigned int pos);

extern const dvar_t *jump_height;
extern const dvar_t *jump_stepSize;
extern const dvar_t *jump_slowdownEnable;
extern const dvar_t *jump_ladderPushVel;
extern const dvar_t *jump_spreadAdd;
