#include "actor_exposed.h"
#include <game_mp/actor_mp.h>
#include <game_mp/g_main_mp.h>
#include "actor_senses.h"
#include "actor_state.h"
#include <game_mp/g_utils_mp.h>
#include "actor_navigation.h"
#include "actor_orientation.h"
#include "actor_events.h"
#include <cgame_mp/cg_local_mp.h>

void __fastcall Actor_Exposed_FindReacquireNode(actor_s *self)
{
    if ( Actor_GetTargetEntity(self) && self->eState[self->stateLevel] == AIS_EXPOSED )
        self->iPotentialReacquireNodeCount = 0;
    else
        self->iPotentialReacquireNodeCount = 0;
}

pathnode_t *__fastcall Actor_Exposed_GetReacquireNode(actor_s *self)
{
    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_exposed.cpp", 320, 0, "%s", "self") )
        __debugbreak();
    if ( !self->sentient
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_exposed.cpp", 321, 0, "%s", "self->sentient") )
    {
        __debugbreak();
    }
    if ( Actor_GetTargetEntity(self) && self->eState[self->stateLevel] == AIS_EXPOSED )
    {
        if ( self->iPotentialReacquireNodeCount )
        {
            if ( Actor_HasPath(self) || level.time >= self->iTeamMoveWaitTime )
            {
                return self->pPotentialReacquireNode[--self->iPotentialReacquireNodeCount];
            }
            else
            {
                self->iPotentialReacquireNodeCount = 0;
                return 0;
            }
        }
        else
        {
            return 0;
        }
    }
    else
    {
        self->iPotentialReacquireNodeCount = 0;
        return 0;
    }
}

char __fastcall Actor_Exposed_UseReacquireNode(actor_s *self, pathnode_t *pNode)
{
    float vPoint[3]; // [esp+14h] [ebp-1Ch] BYREF
    gentity_s *targetEnt; // [esp+20h] [ebp-10h]
    float vFrom[3]; // [esp+24h] [ebp-Ch] BYREF

    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_exposed.cpp", 355, 0, "%s", "self") )
        __debugbreak();
    if ( !self->sentient
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_exposed.cpp", 356, 0, "%s", "self->sentient") )
    {
        __debugbreak();
    }
    if ( Actor_KeepClaimedNode(self)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_exposed.cpp",
                    357,
                    0,
                    "%s",
                    "!Actor_KeepClaimedNode( self )") )
    {
        __debugbreak();
    }
    targetEnt = Actor_GetTargetEntity(self);
    if ( !targetEnt || self->eState[self->stateLevel] != AIS_EXPOSED )
        return 0;
    if ( !Path_CanClaimNode(pNode, self->sentient) )
        return 0;
    vFrom[0] = pNode->constant.vOrigin[0];
    vFrom[1] = pNode->constant.vOrigin[1];
    vFrom[2] = pNode->constant.vOrigin[2];
    vFrom[2] = vFrom[2] + 64.0;
    Actor_GetTargetLookPosition(self, vPoint);
    if ( !Actor_CanSeePointFrom(self, vFrom, vPoint, 0.0, targetEnt->s.number) )
        return 0;
    if ( !Actor_FindPathToNode(self, pNode, 1) )
        return 0;
    Sentient_ClaimNode(self->sentient, pNode);
    self->iPotentialReacquireNodeCount = 0;
    Actor_SetSubState(self, STATE_EXPOSED_REACQUIRE_MOVE);
    return 1;
}

char __fastcall Actor_Exposed_ReacquireStepMove(actor_s *self, float fDist)
{
    float *v4; // ecx
    float *currentOrigin; // [esp+28h] [ebp-80h]
    float fScale; // [esp+30h] [ebp-78h]
    float vStepDir[3]; // [esp+34h] [ebp-74h] BYREF
    float fYaw; // [esp+40h] [ebp-68h]
    float vTraceEndPos[3]; // [esp+44h] [ebp-64h] BYREF
    float stepheight; // [esp+50h] [ebp-58h]
    gentity_s *targetEnt; // [esp+54h] [ebp-54h]
    float vStartPos[3]; // [esp+58h] [ebp-50h] BYREF
    float forward[3]; // [esp+64h] [ebp-44h] BYREF
    float vEyePos[3]; // [esp+70h] [ebp-38h] BYREF
    int iOrder; // [esp+7Ch] [ebp-2Ch]
    int i; // [esp+80h] [ebp-28h]
    float vCheckPos[3]; // [esp+84h] [ebp-24h] BYREF
    float vEnemyPos[3]; // [esp+90h] [ebp-18h] BYREF
    float vMovePos[3]; // [esp+9Ch] [ebp-Ch] BYREF

    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_exposed.cpp", 401, 0, "%s", "self") )
        __debugbreak();
    if ( !self->sentient
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_exposed.cpp", 402, 0, "%s", "self->sentient") )
    {
        __debugbreak();
    }
    targetEnt = Actor_GetTargetEntity(self);
    if ( !targetEnt || self->eState[self->stateLevel] != AIS_EXPOSED )
        return 0;
    fYaw = self->fDesiredBodyYaw + 90.0;
    Actor_GetTargetLookPosition(self, vEnemyPos);
    YawVectors(fYaw, vStepDir, 0);
    forward[0] = targetEnt->r.currentOrigin[0];
    forward[1] = targetEnt->r.currentOrigin[1];
    forward[2] = targetEnt->r.currentOrigin[2];
    currentOrigin = self->ent->r.currentOrigin;
    forward[0] = forward[0] - *currentOrigin;
    forward[1] = forward[1] - currentOrigin[1];
    forward[2] = forward[2] - currentOrigin[2];
    if ( (float)((float)(forward[0] * forward[0]) + (float)(forward[1] * forward[1])) <= 1.0 )
        return 0;
    Vec2Normalize(forward);
    forward[2] = 0.0f;
    vStepDir[0] = forward[1];
    vStepDir[1] = -forward[0];
    vStepDir[2] = 0.0f;
    Actor_GetEyePosition(self, vEyePos);
    v4 = self->ent->r.currentOrigin;
    vStartPos[0] = *v4;
    vStartPos[1] = v4[1];
    vStartPos[2] = v4[2];
    if ( vStepDir[2] != 0.0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_exposed.cpp", 448, 0, "%s", "!vStepDir[2]") )
    {
        __debugbreak();
    }
    iOrder = G_rand() & 1;
    for ( i = 0; i < 2; ++i )
    {
        fScale = factor[iOrder ^ i] * fDist;
        vCheckPos[0] = (float)(fScale * vStepDir[0]) + vEyePos[0];
        vCheckPos[1] = (float)(fScale * vStepDir[1]) + vEyePos[1];
        vCheckPos[2] = vEyePos[2];
        if ( Actor_CanSeePointFrom(self, vCheckPos, vEnemyPos, self->fMaxSightDistSqrd, targetEnt->s.number) )
        {
            vMovePos[0] = (float)(fScale * vStepDir[0]) + vStartPos[0];
            vMovePos[1] = (float)(fScale * vStepDir[1]) + vStartPos[1];
            vMovePos[2] = vStartPos[2];
            stepheight = self->Physics.prone ? 10.0f : 18.0f;
            if ( Path_PredictionTrace(vStartPos, vMovePos, 1023, self->Physics.iTraceMask | 4, vTraceEndPos, stepheight, 1)
                && Actor_PointAtGoal(vTraceEndPos, &self->codeGoal) )
            {
                Actor_FindPath(self, vTraceEndPos, 0, 0);
                if ( Actor_HasPath(self) )
                {
                    Actor_SetSubState(self, STATE_EXPOSED_REACQUIRE_MOVE);
                    Actor_SetOrientMode(self, AI_ORIENT_TO_ENEMY);
                    return 1;
                }
            }
        }
    }
    return 0;
}

void __fastcall Actor_Exposed_FindReacquireDirectPath(actor_s *self, bool ignoreSuppression)
{
    sentient_s *enemy; // [esp+8h] [ebp-10h]
    float vEnemyPos[3]; // [esp+Ch] [ebp-Ch] BYREF

    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_exposed.cpp", 512, 0, "%s", "self") )
        __debugbreak();
    if ( !self->sentient
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_exposed.cpp", 513, 0, "%s", "self->sentient") )
    {
        __debugbreak();
    }
    if ( Actor_GetTargetEntity(self) && self->eState[self->stateLevel] == AIS_EXPOSED )
    {
        Actor_GetTargetPosition(self, vEnemyPos);
        enemy = Actor_GetTargetSentient(self);
        if ( enemy )
            Actor_FindPathToSentient(self, enemy, !ignoreSuppression);
        else
            Actor_FindPath(self, vEnemyPos, 1, ignoreSuppression);
        if ( !Actor_PointAtGoal(vEnemyPos, &self->codeGoal) )
            Actor_ClipPathToGoal(self);
        Actor_BeginTrimPath(self);
    }
    else
    {
        Actor_ClearPath(self);
    }
}

void __fastcall Actor_Exposed_FindReacquireProximatePath(actor_s *self, bool ignoreSuppression)
{
    float fWithinDistSqrd; // [esp+1Ch] [ebp-10h]
    float vEnemyPos[3]; // [esp+20h] [ebp-Ch] BYREF

    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_exposed.cpp", 549, 0, "%s", "self") )
        __debugbreak();
    if ( !self->sentient
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_exposed.cpp", 550, 0, "%s", "self->sentient") )
    {
        __debugbreak();
    }
    if ( Actor_GetTargetEntity(self) && self->eState[self->stateLevel] == AIS_EXPOSED )
    {
        Actor_GetTargetPosition(self, vEnemyPos);
        fWithinDistSqrd = Vec3DistanceSq(self->ent->r.currentOrigin, vEnemyPos) * 0.25;
        if ( fWithinDistSqrd > (float)(self->fMaxSightDistSqrd * 0.0625) )
            fWithinDistSqrd = self->fMaxSightDistSqrd * 0.0625;
        Actor_FindPathInGoalWithLOS(self, vEnemyPos, fWithinDistSqrd, ignoreSuppression);
        Actor_BeginTrimPath(self);
    }
    else
    {
        Actor_ClearPath(self);
    }
}

char __fastcall Actor_Exposed_StartReacquireMove(actor_s *self)
{
    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_exposed.cpp", 585, 0, "%s", "self") )
        __debugbreak();
    if ( !self->sentient
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_exposed.cpp", 586, 0, "%s", "self->sentient") )
    {
        __debugbreak();
    }
    self->TrimInfo.iIndex = 0;
    self->TrimInfo.iDelta = 0;
    if ( !Actor_GetTargetEntity(self) || self->eState[self->stateLevel] != AIS_EXPOSED )
        return 0;
    if ( !Actor_HasPath(self) )
        return 0;
    Actor_SetSubState(self, STATE_EXPOSED_REACQUIRE_MOVE);
    return 1;
}

void __fastcall Actor_Exposed_FlashBanged(actor_s *self)
{
    PROF_SCOPED("flashbanged");

    if ( !self->flashBanged
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_exposed.cpp",
                    766,
                    0,
                    "%s",
                    "self->flashBanged") )
    {
        __debugbreak();
    }
    Actor_SetAnimScript(
        self,
        &g_animScriptTable[self->species]->flashed,
        AI_MOVE_STOP,
        AI_ANIM_USE_BOTH_DELTAS,
        AI_ANIM_FUNCTION_STOP);
    Actor_SetOrientMode(self, AI_ORIENT_DONT_CHANGE);
}

int __cdecl Path_IsValidClaimNode(const pathnode_t *node)
{
    if ( !node && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\game\\pathnode.h", 224, 0, "%s", "node") )
        __debugbreak();
    return node->constant.spawnflags & 0x8000;
}

void __fastcall Actor_Exposed_Touch(actor_s *self, gentity_s *pOther)
{
    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_exposed.cpp", 920, 0, "%s", "self") )
        __debugbreak();
    if ( !self->sentient
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_exposed.cpp", 921, 0, "%s", "self->sentient") )
    {
        __debugbreak();
    }
    if ( !pOther
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_exposed.cpp", 922, 0, "%s", "pOther") )
    {
        __debugbreak();
    }
    if ( pOther->sentient )
        Actor_GetPerfectInfo(self, pOther->sentient);
}

