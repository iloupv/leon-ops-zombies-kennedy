#pragma once
#include "actor.h"

bool __fastcall Actor_Dog_Exposed_Start(actor_s *self, ai_state_t ePrevState);
void __fastcall Actor_Dog_Exposed_Finish(actor_s *self, ai_state_t eNextState);
void __fastcall Actor_Dog_Exposed_Suspend(actor_s *self, ai_state_t eNextState);
bool __fastcall Actor_Dog_PointNearAttackPoint(const float *vPoint, const float *vGoalPos, float buffer);
double __fastcall Actor_Dog_GetDeltaTurnYaw(actor_s *self);
actor_think_result_t __fastcall Actor_Dog_Exposed_Think(actor_s *self);
bool __cdecl Actor_Dog_IsInSyncedMelee(actor_s *self, sentient_s *enemy);
void __cdecl Actor_Dog_Attack(actor_s *self);
void __fastcall Actor_FindPathToGoalNearestNode(actor_s *self);
char __fastcall Actor_Dog_ObstacleInPath(const actor_s *self);
bool __fastcall Actor_Dog_ShouldTurn(actor_s *self);
int __cdecl Actor_Dog_IsEnemyInAttackRange(actor_s *self, sentient_s *enemy, int *goalPosSet);
char __fastcall Actor_SetMeleeAttackSpot(actor_s *self, const float *enemyPosition, float *attackPosition);
void __fastcall Actor_UpdateMeleeGoalPos(actor_s *self, float *goalPos);
double __cdecl Actor_Dog_GetEnemyPos(actor_s *self, sentient_s *enemy, float *enemyPos);
