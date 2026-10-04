#include "actor_state.h"
#include <game_mp/g_main_mp.h>
#include <game_mp/actor_mp.h>

const ai_state_transition_t g_eSimplificationRules[4][4] =
{
  {
    AIS_TRANSITION_CANONICAL,
    AIS_TRANSITION_CANONICAL,
    AIS_TRANSITION_CANONICAL,
    AIS_TRANSITION_CANONICAL
  },
  {
    AIS_TRANSITION_CANONICAL,
    AIS_TRANSITION_SET,
    AIS_TRANSITION_CANONICAL,
    AIS_TRANSITION_POP
  },
  {
    AIS_TRANSITION_CANONICAL,
    AIS_TRANSITION_SET,
    AIS_TRANSITION_CANONICAL,
    AIS_TRANSITION_NONE
  },
  {
    AIS_TRANSITION_CANONICAL,
    AIS_TRANSITION_CANONICAL,
    AIS_TRANSITION_SET,
    AIS_TRANSITION_CANONICAL
  }
};

const ai_state_t g_eSupercededStates[5][5] =
{
  { AIS_PAIN, AIS_REACT, AIS_CUSTOMANIM, AIS_INVALID, AIS_INVALID },
  { AIS_REACT, AIS_CUSTOMANIM, AIS_INVALID, AIS_INVALID, AIS_INVALID },
  { AIS_PAIN, AIS_REACT, AIS_SCRIPTEDANIM, AIS_CUSTOMANIM, AIS_NEGOTIATION },
  { AIS_PAIN, AIS_REACT, AIS_SCRIPTEDANIM, AIS_CUSTOMANIM, AIS_NEGOTIATION },
  { AIS_PAIN, AIS_REACT, AIS_SCRIPTEDANIM, AIS_CUSTOMANIM, AIS_NEGOTIATION }
};

const ai_state_t g_eSupercedingStates[5][5] =
{
  { AIS_SCRIPTEDANIM, AIS_NEGOTIATION, AIS_INVALID, AIS_INVALID, AIS_INVALID },
  { AIS_PAIN, AIS_SCRIPTEDANIM, AIS_NEGOTIATION, AIS_INVALID, AIS_INVALID },
  { AIS_INVALID, AIS_INVALID, AIS_INVALID, AIS_INVALID, AIS_INVALID },
  { AIS_NEGOTIATION, AIS_INVALID, AIS_INVALID, AIS_INVALID, AIS_INVALID },
  { AIS_SCRIPTEDANIM, AIS_INVALID, AIS_INVALID, AIS_INVALID, AIS_INVALID }
};


void __cdecl Actor_SetDefaultState(actor_s *actor)
{
    if ( actor->stateLevel
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_state.cpp",
                    129,
                    0,
                    "%s",
                    "actor->stateLevel == 0") )
    {
        __debugbreak();
    }
    if ( actor->simulatedStateLevel
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_state.cpp",
                    130,
                    0,
                    "%s",
                    "actor->simulatedStateLevel == 0") )
    {
        __debugbreak();
    }
    if ( actor->transitionCount
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_state.cpp",
                    131,
                    0,
                    "%s",
                    "actor->transitionCount == 0") )
    {
        __debugbreak();
    }
    actor->eState[0] = AIS_EXPOSED;
    actor->eSimulatedState[0] = AIS_EXPOSED;
}

void __fastcall Actor_ThinkStateTransitions(actor_s *self)
{
    const char *v1; // eax
    ai_state_transition_t eTransition; // [esp+4h] [ebp-10h]
    unsigned int i; // [esp+10h] [ebp-4h]
    unsigned int ia; // [esp+10h] [ebp-4h]

    if ( !self->transitionCount
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_state.cpp",
                    287,
                    0,
                    "%s",
                    "self->transitionCount > 0") )
    {
        __debugbreak();
    }
    for ( i = 0; i < self->transitionCount; ++i )
    {
        if ( self->stateLevel >= 6
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_state.cpp",
                        294,
                        0,
                        "self->stateLevel doesn't index ARRAY_COUNT( self->eState )\n\t%i not in [0, %i)",
                        self->stateLevel,
                        6) )
        {
            __debugbreak();
        }
        if ( (self->eState[self->stateLevel] <= AIS_INVALID || self->eState[self->stateLevel] >= AIS_COUNT)
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_state.cpp",
                        295,
                        0,
                        "%s\n\t(self->eState[self->stateLevel]) = %i",
                        "(self->eState[self->stateLevel] > AIS_INVALID && self->eState[self->stateLevel] < AIS_COUNT)",
                        self->eState[self->stateLevel]) )
        {
            __debugbreak();
        }
        eTransition = self->StateTransitions[i].eTransition;
        switch ( eTransition )
        {
            case AIS_TRANSITION_SET:
                Actor_FinishState(self, self->StateTransitions[i].eState);
                if ( !(unsigned __int8)Actor_StartState(self, self->StateTransitions[i].eState) )
                    Actor_StartDefaultState(self);
                break;
            case AIS_TRANSITION_PUSH:
                if ( self->stateLevel + 1 >= 6
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_state.cpp",
                                307,
                                0,
                                "%s",
                                "self->stateLevel + 1 < ARRAY_COUNT( self->eState )") )
                {
                    __debugbreak();
                }
                Actor_SuspendState(self, self->StateTransitions[i].eState);
                ++self->stateLevel;
                if ( !(unsigned __int8)Actor_StartState(self, self->StateTransitions[i].eState) )
                    Actor_ResumePopedState(self, self->StateTransitions[i].eState);
                break;
            case AIS_TRANSITION_POP:
                if ( !self->stateLevel
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_state.cpp",
                                316,
                                0,
                                "%s",
                                "self->stateLevel > 0") )
                {
                    __debugbreak();
                }
                Actor_FinishState(self, *((ai_state_t *)&self->legsAnimDuration + self->stateLevel));
                Actor_ResumePopedState(self, self->eState[self->stateLevel]);
                break;
            default:
                v1 = va("invalid state transition %i", self->StateTransitions[i].eTransition);
                if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_state.cpp", 322, 0, v1) )
                    __debugbreak();
                break;
        }
    }
    self->transitionCount = 0;
    self->simulatedStateLevel = self->stateLevel;
    for ( ia = 0; ia <= self->stateLevel; ++ia )
        self->eSimulatedState[ia] = self->eState[ia];
}

int __fastcall Actor_StartState(actor_s *self, ai_state_t eStartedState)
{
    if ( !self->stateLevel )
        self->iStateTime = level.time;
    if ( !AIFuncTable[self->species][eStartedState].pfnStart
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_state.cpp",
                    150,
                    0,
                    "%s\n\t(eStartedState) = %i",
                    "(AIFuncTable[self->species][eStartedState].pfnStart)",
                    eStartedState) )
    {
        __debugbreak();
    }
    self->eState[self->stateLevel] = eStartedState;
    return ((int (__fastcall *)(actor_s *, ai_state_t))AIFuncTable[self->species][eStartedState].pfnStart)(
                     self,
                     self->eState[self->stateLevel]);
}

void __fastcall Actor_FinishState(actor_s *self, ai_state_t eNextState)
{
    ai_state_t eCurState; // [esp+Ch] [ebp-4h]

    eCurState = self->eState[self->stateLevel];
    if ( !AIFuncTable[self->species][eCurState].pfnFinish
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_state.cpp",
                    169,
                    0,
                    "%s\n\t(eCurState) = %i",
                    "(AIFuncTable[self->species][eCurState].pfnFinish)",
                    eCurState) )
    {
        __debugbreak();
    }
    AIFuncTable[self->species][eCurState].pfnFinish(self, eNextState);
}

void __fastcall Actor_SuspendState(actor_s *self, ai_state_t eNextState)
{
    ai_state_t eCurState; // [esp+Ch] [ebp-4h]

    eCurState = self->eState[self->stateLevel];
    if ( !AIFuncTable[self->species][eCurState].pfnSuspend
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_state.cpp",
                    187,
                    0,
                    "%s\n\t(eCurState) = %i",
                    "(AIFuncTable[self->species][eCurState].pfnSuspend)",
                    eCurState) )
    {
        __debugbreak();
    }
    AIFuncTable[self->species][eCurState].pfnSuspend(self, eNextState);
}

void __cdecl Actor_StartDefaultState(actor_s *self)
{
    if ( !(unsigned __int8)Actor_StartState(self, AIS_EXPOSED)
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_state.cpp", 247, 0, "%s", "startSuccess") )
    {
        __debugbreak();
    }
}

void __cdecl Actor_ResumePopedState(actor_s *self, ai_state_t state)
{
    char startSuccess; // [esp+3h] [ebp-1h]

    while ( self->stateLevel )
    {
        --self->stateLevel;
        startSuccess = Actor_ResumeState(self, state);
        state = self->eState[self->stateLevel];
        if ( startSuccess )
            return;
    }
    Actor_StartDefaultState(self);
}

char __fastcall Actor_ResumeState(actor_s *self, ai_state_t ePrevState)
{
    ai_state_t NextPopedState; // eax
    ai_state_t eCurState; // [esp+10h] [ebp-4h]

    eCurState = self->eState[self->stateLevel];
    if ( !AIFuncTable[self->species][eCurState].pfnResume
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_state.cpp",
                    224,
                    0,
                    "%s\n\t(eCurState) = %i",
                    "(AIFuncTable[self->species][eCurState].pfnResume)",
                    eCurState) )
    {
        __debugbreak();
    }
    if ( AIFuncTable[self->species][eCurState].pfnResume(self, ePrevState) )
        return 1;
    NextPopedState = (ai_state_t)Actor_GetNextPopedState(self);
    Actor_FinishState(self, NextPopedState);
    return 0;
}

int __cdecl Actor_GetNextPopedState(actor_s *self)
{
    if ( self->stateLevel )
        return *(&self->legsAnimDuration + self->stateLevel);
    else
        return 1;
}

void __fastcall Actor_SetSubState(actor_s *self, ai_substate_t eSubState)
{
    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_state.cpp", 347, 0, "%s", "self") )
        __debugbreak();
    self->eSubState[self->stateLevel] = eSubState;
}

int __fastcall Actor_IsStateOnStack(const actor_s *self, ai_state_t eState)
{
    unsigned int i; // [esp+8h] [ebp-4h]

    for ( i = 0; i <= self->simulatedStateLevel; ++i )
    {
        if ( self->eSimulatedState[i] == eState )
            return 1;
    }
    return 0;
}

void __fastcall Actor_SetState(actor_s *self, ai_state_t eState)
{
    const char *v2; // eax

    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_state.cpp", 527, 0, "%s", "self") )
        __debugbreak();
    if ( self->transitionCount >= 0xD
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_state.cpp",
                    528,
                    0,
                    "self->transitionCount doesn't index ARRAY_COUNT( self->StateTransitions )\n\t%i not in [0, %i)",
                    self->transitionCount,
                    13) )
    {
        __debugbreak();
    }
    if ( self->simulatedStateLevel )
    {
        v2 = va(
                     "s level %i stack %i %i %i %i %i %i, level %i stack %i %i %i %i %i %i",
                     self->simulatedStateLevel,
                     self->eSimulatedState[0],
                     self->eSimulatedState[1],
                     self->eSimulatedState[2],
                     self->eSimulatedState[3],
                     self->eSimulatedState[4],
                     self->eSimulatedState[5],
                     self->stateLevel,
                     self->eState[0],
                     self->eState[1],
                     self->eState[2],
                     self->eState[3],
                     self->eState[4],
                     self->eState[5]);
        if ( !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_state.cpp",
                        529,
                        0,
                        "%s\n\t%s",
                        "self->simulatedStateLevel == 0",
                        v2) )
            __debugbreak();
    }
    self->StateTransitions[self->transitionCount].eTransition = AIS_TRANSITION_SET;
    self->StateTransitions[self->transitionCount++].eState = eState;
    Actor_SimplifyStateTransitions(self);
    self->eSimulatedState[self->simulatedStateLevel] = eState;
    Actor_OnStateChange(self);
}

void __fastcall Actor_SimplifyStateTransitions(actor_s *self)
{
    ai_state_transition_t eCmd1; // [esp+8h] [ebp-Ch]
    ai_state_transition_t eCmd2; // [esp+Ch] [ebp-8h]
    ai_state_transition_t eTransition; // [esp+10h] [ebp-4h]

    while ( self->transitionCount >= 2 )
    {
        eCmd1 = (ai_state_transition_t)self->eSubState[2 * self->transitionCount + 5];
        eCmd2 = (ai_state_transition_t )(*(&self->iStateTime + 2 * self->transitionCount));
        if ( (unsigned int)eCmd1 >= 4
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_state.cpp",
                        418,
                        0,
                        "eCmd1 doesn't index ARRAY_COUNT( g_eSimplificationRules )\n\t%i not in [0, %i)",
                        eCmd1,
                        4) )
        {
            __debugbreak();
        }
        if ( (unsigned int)eCmd2 >= 4
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_state.cpp",
                        419,
                        0,
                        "eCmd2 doesn't index ARRAY_COUNT( g_eSimplificationRules[0] )\n\t%i not in [0, %i)",
                        eCmd2,
                        4) )
        {
            __debugbreak();
        }
        if ( !self->Physics.bIsAlive )
        {
            Com_PrintWarning(18, "WARNING: ignoring AI state transition on entnum %d (actor is dead)\n", self->ent->s.number);
            self->transitionCount = 1;
            return;
        }
        eTransition = g_eSimplificationRules[eCmd1][eCmd2];
        if ( eTransition == AIS_TRANSITION_CANONICAL )
            return;
        if ( eTransition )
        {
            self->eSubState[2 * self->transitionCount + 5] = (ai_substate_t )(eTransition);
            self->eSubState[2 * self->transitionCount + 6] = (ai_substate_t )(*(&self->preThinkTime + 2 * self->transitionCount));
            --self->transitionCount;
        }
        else
        {
            self->transitionCount -= 2;
        }
    }
}

void __fastcall Actor_OnStateChange(actor_s *self)
{
    Actor_ClearArrivalPos(self);
}

int __fastcall Actor_PushState(actor_s *self, ai_state_t eState)
{
    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_state.cpp", 554, 0, "%s", "self") )
        __debugbreak();
    if ( (eState < AIS_PAIN || eState > AIS_NEGOTIATION)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_state.cpp",
                    555,
                    0,
                    "eState not in [AIS_PUSHABLE_FIRST, AIS_PUSHABLE_LAST]\n\t%i not in [%i, %i]",
                    eState,
                    7,
                    11) )
    {
        __debugbreak();
    }
    if ( !Actor_AllowedToPushState(self, eState) )
        return 0;
    Actor_PrepareToPushState(self, eState);
    if ( self->simulatedStateLevel >= 5
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_state.cpp",
                    562,
                    0,
                    "self->simulatedStateLevel doesn't index ARRAY_COUNT( self->eState ) - 1\n\t%i not in [0, %i)",
                    self->simulatedStateLevel,
                    5) )
    {
        __debugbreak();
    }
    self->StateTransitions[self->transitionCount].eTransition = AIS_TRANSITION_PUSH;
    self->StateTransitions[self->transitionCount++].eState = eState;
    Actor_SimplifyStateTransitions(self);
    self->eSimulatedState[++self->simulatedStateLevel] = eState;
    Actor_OnStateChange(self);
    return 1;
}

int __fastcall Actor_AllowedToPushState(actor_s *self, int eState)
{
    unsigned int j; // [esp+Ch] [ebp-8h]
    unsigned int i; // [esp+10h] [ebp-4h]

    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_state.cpp", 458, 0, "%s", "self") )
        __debugbreak();
    if ( (eState < 7 || eState > 11)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_state.cpp",
                    459,
                    0,
                    "%s",
                    "eState >= AIS_PUSHABLE_FIRST && eState <= AIS_PUSHABLE_LAST") )
    {
        __debugbreak();
    }
    for ( i = 1; i <= self->simulatedStateLevel; ++i )
    {
        for ( j = 0; j < 5; ++j )
        {
            if ( self->eSimulatedState[i] == g_eSupercedingStates[eState - 7][j] )
                return 0;
        }
    }
    return 1;
}

void __fastcall Actor_PrepareToPushState(actor_s *self, int eState)
{
    unsigned int j; // [esp+Ch] [ebp-8h]
    unsigned int i; // [esp+10h] [ebp-4h]

    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_state.cpp", 484, 0, "%s", "self") )
        __debugbreak();
    if ( (eState < 7 || eState > 11)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_state.cpp",
                    485,
                    0,
                    "%s",
                    "eState >= AIS_PUSHABLE_FIRST && eState <= AIS_PUSHABLE_LAST") )
    {
        __debugbreak();
    }
    i = 1;
LABEL_9:
    if ( i <= self->simulatedStateLevel )
    {
        for ( j = 0; ; ++j )
        {
            if ( j >= 5 )
            {
                ++i;
                goto LABEL_9;
            }
            if ( self->eSimulatedState[i] == g_eSupercededStates[eState - 7][j] )
                break;
        }
        do
            Actor_PopState(self);
        while ( i <= self->simulatedStateLevel );
    }
}

void __fastcall Actor_PopState(actor_s *self)
{
    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_state.cpp", 588, 0, "%s", "self") )
        __debugbreak();
    if ( self->transitionCount >= 0xD
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_state.cpp",
                    589,
                    0,
                    "self->transitionCount doesn't index ARRAY_COUNT( self->StateTransitions )\n\t%i not in [0, %i)",
                    self->transitionCount,
                    13) )
    {
        __debugbreak();
    }
    if ( !self->simulatedStateLevel
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_state.cpp",
                    590,
                    0,
                    "%s",
                    "self->simulatedStateLevel > 0") )
    {
        __debugbreak();
    }
    self->StateTransitions[self->transitionCount].eTransition = AIS_TRANSITION_POP;
    self->StateTransitions[self->transitionCount++].eState = AIS_INVALID;
    Actor_SimplifyStateTransitions(self);
    --self->simulatedStateLevel;
}

void __fastcall Actor_ForceState(actor_s *self, ai_state_t eState)
{
    while ( self->stateLevel )
    {
        if ( !AIFuncTable[self->species][self->eState[self->stateLevel]].pfnFinish
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_state.cpp",
                        614,
                        0,
                        "%s",
                        "AIFuncTable[self->species][self->eState[self->stateLevel]].pfnFinish") )
        {
            __debugbreak();
        }
        AIFuncTable[self->species][self->eState[self->stateLevel]].pfnFinish(self, eState);
        --self->stateLevel;
    }
    self->simulatedStateLevel = 0;
    self->transitionCount = 0;
    Actor_SetState(self, eState);
}

