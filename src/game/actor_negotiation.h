#pragma once
#include "actor.h"

bool __fastcall Actor_Negotiation_Start(actor_s *pSelf, ai_state_t ePrevState);
actor_think_result_t __fastcall Actor_Negotiation_Think(actor_s *pSelf);
