#include "actor_generic.h"
#include <game_mp/g_main_mp.h>
#include <game_mp/actor_mp.h>

void __fastcall Actor_Generic_Suspend(actor_s *self, ai_state_t eNextState)
{
    PROF_SCOPED("genericsuspend");

    if ( self->stateLevel >= 6
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_generic.cpp",
                    30,
                    0,
                    "self->stateLevel doesn't index ARRAY_COUNT( self->eState )\n\t%i not in [0, %i)",
                    self->stateLevel,
                    6) )
    {
        __debugbreak();
    }
    if ( (self->eState[self->stateLevel] <= AIS_INVALID || self->eState[self->stateLevel] >= AIS_COUNT)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_generic.cpp",
                    31,
                    0,
                    "%s\n\t(self->eState[self->stateLevel]) = %i",
                    "(self->eState[self->stateLevel] > AIS_INVALID && self->eState[self->stateLevel] < AIS_COUNT)",
                    self->eState[self->stateLevel]) )
    {
        __debugbreak();
    }
    if ( !AIFuncTable[self->species][self->eState[self->stateLevel]].pfnFinish
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_generic.cpp",
                    32,
                    0,
                    "%s\n\t(self->eState[self->stateLevel]) = %i",
                    "(AIFuncTable[self->species][self->eState[self->stateLevel]].pfnFinish)",
                    self->eState[self->stateLevel]) )
    {
        __debugbreak();
    }
    AIFuncTable[self->species][self->eState[self->stateLevel]].pfnFinish(self, eNextState);
}

bool __fastcall Actor_Generic_Resume(actor_s *self, ai_state_t ePrevState)
{
    if ( self->stateLevel >= 5
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_generic.cpp",
                    47,
                    0,
                    "self->stateLevel doesn't index ARRAY_COUNT( self->eState ) - 1\n\t%i not in [0, %i)",
                    self->stateLevel,
                    5) )
    {
        __debugbreak();
    }
    if ( (self->eState[self->stateLevel] <= AIS_INVALID || self->eState[self->stateLevel] >= AIS_COUNT)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_generic.cpp",
                    48,
                    0,
                    "%s\n\t(self->eState[self->stateLevel]) = %i",
                    "(self->eState[self->stateLevel] > AIS_INVALID && self->eState[self->stateLevel] < AIS_COUNT)",
                    self->eState[self->stateLevel]) )
    {
        __debugbreak();
    }
    if ( !AIFuncTable[self->species][self->eState[self->stateLevel]].pfnStart
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_generic.cpp",
                    49,
                    0,
                    "%s\n\t(self->eState[self->stateLevel]) = %i",
                    "(AIFuncTable[self->species][self->eState[self->stateLevel]].pfnStart)",
                    self->eState[self->stateLevel]) )
    {
        __debugbreak();
    }
    return ((int (__fastcall *)(actor_s *, ai_state_t))AIFuncTable[self->species][self->eState[self->stateLevel]].pfnStart)(
                     self,
                     ePrevState);
}

//void __stdcall Actor_Generic_Pain(int iDamage, const float *vPoint, int iMod, const float *vDir, hitLocation_t hitLoc)
void __fastcall Actor_Generic_Pain(actor_s *, gentity_s *, int, const float *, const int, const float *, const hitLocation_t)
{
    ;
}

void __fastcall Actor_Generic_Touch(actor_s *self, gentity_s *pOther)
{
    if ( pOther->s.number == self->Physics.iHitEntnum )
    {
        if ( level.gentities[self->Physics.iHitEntnum].client )
            Actor_ClearPath(self);
    }
}

