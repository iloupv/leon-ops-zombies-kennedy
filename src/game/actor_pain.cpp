#include "actor_pain.h"
#include <game_mp/g_main_mp.h>
#include <game_mp/actor_mp.h>
#include <game_mp/g_spawn_mp.h>
#include <clientscript/scr_const.h>
#include "actor_events.h"
#include "actor_orientation.h"
#include "actor_state.h"

bool __fastcall Actor_InPain(const actor_s *self)
{
    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_pain.cpp", 22, 0, "%s", "self") )
        __debugbreak();
    return self->eState[self->stateLevel] == AIS_PAIN;
}

bool __fastcall Actor_Pain_Start(actor_s *self, ai_state_t ePrevState)
{
    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_pain.cpp", 37, 0, "%s", "self") )
        __debugbreak();
    self->iPainTime = level.time;
    Actor_ClearPath(self);
    Actor_AnimPain(self);
    Scr_Notify(self->ent, scr_const.pain, 0);
    return 1;
}

void __fastcall Actor_Pain_Finish(actor_s *self, ai_state_t eNextState)
{
    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_pain.cpp", 61, 0, "%s", "self") )
        __debugbreak();
    if ( !self->ent
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_pain.cpp", 62, 0, "%s", "self->ent") )
    {
        __debugbreak();
    }
    //if ( EntHandle::isDefined(&self->sentient->lastAttacker) )
    if ( self->sentient->lastAttacker.isDefined() )
    {
        Actor_BroadcastTeamEvent(self->sentient, AI_EV_PAIN);
        //EntHandle::setEnt(&self->sentient->lastAttacker, 0);
        self->sentient->lastAttacker.setEnt(0);
    }
}

actor_think_result_t __fastcall Actor_Pain_Think(actor_s *self)
{
    PROF_SCOPED("painthink");

    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_pain.cpp", 83, 0, "%s", "self") )
        __debugbreak();
    if ( !self->ent
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_pain.cpp", 84, 0, "%s", "self->ent") )
    {
        __debugbreak();
    }
    self->pszDebugInfo = "pain";
    if ( level.time <= self->iPainTime || Actor_IsAnimScriptAlive(self) )
    {
        //if ( EntHandle::isDefined(&self->sentient->lastAttacker) && level.time - self->iPainTime >= 500 )
        if ( self->sentient->lastAttacker.isDefined() && level.time - self->iPainTime >= 500)
        {
            Actor_BroadcastTeamEvent(self->sentient, AI_EV_PAIN);
            //EntHandle::setEnt(&self->sentient->lastAttacker, 0);
            self->sentient->lastAttacker.setEnt(0);
        }
        Actor_PreThink(self);
        Actor_SetOrientMode(self, AI_ORIENT_DONT_CHANGE);
        Actor_AnimPain(self);
        Actor_PostThink(self);
        return ACTOR_THINK_DONE;
    }
    else
    {
        Actor_PopState(self);
        return ACTOR_THINK_REPEAT;
    }
}

