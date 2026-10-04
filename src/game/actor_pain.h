#pragma once
#include "actor.h"

bool __fastcall Actor_InPain(const actor_s *self);
bool __fastcall Actor_Pain_Start(actor_s *self, ai_state_t ePrevState);
void __fastcall Actor_Pain_Finish(actor_s *self, ai_state_t eNextState);
actor_think_result_t __fastcall Actor_Pain_Think(actor_s *self);
