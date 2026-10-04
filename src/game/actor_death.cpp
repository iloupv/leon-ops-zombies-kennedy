#include "actor_death.h"
#include <game_mp/actor_mp.h>
#include <game_mp/g_main_mp.h>
#include <cgame/cg_event.h>
#include "actor_corpse.h"
#include "actor_state.h"
#include "actor_orientation.h"
#include <server/sv_world.h>
#include "actor_events.h"

bool __fastcall Actor_Death_Start(actor_s *self, ai_state_t ePrevState)
{
    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_death.cpp", 32, 0, "%s", "self") )
        __debugbreak();
    if ( !self->ent
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_death.cpp", 33, 0, "%s", "self->ent") )
    {
        __debugbreak();
    }
    if ( self->ent->health > 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_death.cpp",
                    34,
                    0,
                    "%s\n\t(self->ent->health) = %i",
                    "(self->ent->health <= 0)",
                    self->ent->health) )
    {
        __debugbreak();
    }
    Actor_ClearKeepClaimedNode(self);
    Sentient_ClaimNode(self->sentient, 0);
    if ( self->eAnimMode != AI_ANIM_NOPHYSICS
        && !self->ent->tagInfo
        && !BG_ActorIsProne(&self->ProneInfo, level.time)
        && !Com_IsRagdollTrajectory(&self->ent->s.lerp.pos) )
    {
        self->ProneInfo.iProneTime = level.time;
        self->ProneInfo.prone = 1;
        self->ProneInfo.bCorpseOrientation = 1;
        self->ProneInfo.iProneTrans = 500;
        self->ProneInfo.fTorsoPitch = 0.0f;
        self->ProneInfo.fWaistPitch = 0.0f;
        Actor_OrientCorpseToGround(self->ent, 0);
    }
    Actor_SetSubState(self, STATE_DEATH_PRECLEANUP);
    return 1;
}

actor_think_result_t __fastcall Actor_Death_Think(actor_s *self)
{
    int savedregs; // [esp+20h] [ebp+0h] BYREF

    PROF_SCOPED("deaththink");

    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_death.cpp", 95, 0, "%s", "self") )
        __debugbreak();
    self->pszDebugInfo = "death";
    if ( self->eSubState[self->stateLevel] == STATE_DEATH_PRECLEANUP && level.time - self->iStateTime >= 800 )
    {
        Actor_Death_Cleanup(self);
        Actor_SetSubState(self, STATE_DEATH_POSTCLEANUP);
    }
    if ( self->pAnimScriptFunc != &g_animScriptTable[self->species]->death || Actor_IsAnimScriptAlive(self) )
    {
        Actor_SetOrientMode(self, AI_ORIENT_DONT_CHANGE);
        Actor_AnimDeath(self);
        Actor_PostThink(self);
        if ( level.time >= self->iStateTime + 500 )
        {
            self->ent->r.contents = self->deathContents;
            SV_LinkEntity(self->ent);
        }
        return ACTOR_THINK_DONE;
    }
    else
    {
        if ( self->eSubState[self->stateLevel] == STATE_DEATH_PRECLEANUP )
            Actor_Death_Cleanup(self);
        return ACTOR_THINK_MOVE_TO_BODY_QUEUE;
    }
}

void __fastcall Actor_Death_Cleanup(actor_s *self)
{
    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_death.cpp", 65, 0, "%s", "self") )
        __debugbreak();
    if ( !self->sentient
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_death.cpp", 66, 0, "%s", "self->sentient") )
    {
        __debugbreak();
    }
    Actor_BroadcastTeamEvent(self->sentient, AI_EV_DEATH);
    Sentient_Dissociate(self->sentient);
    if ( !self->sentient
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_death.cpp", 77, 0, "%s", "self->sentient") )
    {
        __debugbreak();
    }
    if ( self->Physics.bIsAlive
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_death.cpp",
                    78,
                    0,
                    "%s",
                    "!self->Physics.bIsAlive") )
    {
        __debugbreak();
    }
    Actor_ClearPath(self);
}

