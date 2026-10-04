#pragma once
#include "actor.h"

void __fastcall Actor_Generic_Suspend(actor_s *self, ai_state_t eNextState);
bool __fastcall Actor_Generic_Resume(actor_s *self, ai_state_t ePrevState);
void __fastcall Actor_Generic_Pain(actor_s *, gentity_s *, int, const float *, const int, const float *, const hitLocation_t);
void __fastcall Actor_Generic_Touch(actor_s *self, gentity_s *pOther);
