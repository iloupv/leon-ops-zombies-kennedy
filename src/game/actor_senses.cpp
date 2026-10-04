#include "actor_senses.h"
#include <game_mp/g_main_mp.h>
#include <server/sv_world.h>
#include <game_mp/actor_mp.h>
#include <game_mp/g_spawn_mp.h>
#include <clientscript/scr_const.h>
#include <game_mp/g_utils_mp.h>
#include "bullet.h"
#include <qcommon/dobj_management.h>

colgeom_visitor_inlined_t<200> *g_visitor;
float g_viewPos[3];

char __fastcall Actor_CanSeePointFrom(
                actor_s *self,
                const float *vStart,
                const float *vEnd,
                float fMaxDistSqrd,
                int ignoreEntityNum)
{
    float fDistSqrd; // [esp+10h] [ebp-10h]
    float vDelta_4; // [esp+18h] [ebp-8h]
    float vDelta_8; // [esp+1Ch] [ebp-4h]

    if ( fMaxDistSqrd < 0.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_senses.cpp",
                    122,
                    0,
                    "%s\n\t(fMaxDistSqrd) = %g",
                    "(fMaxDistSqrd >= 0)",
                    fMaxDistSqrd) )
    {
        __debugbreak();
    }
    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_senses.cpp", 123, 0, "%s", "self") )
        __debugbreak();
    if ( !self->ent
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_senses.cpp", 124, 0, "%s", "self->ent") )
    {
        __debugbreak();
    }
    if ( fMaxDistSqrd == 0.0 )
        return Actor_SightTrace(self, vStart, vEnd, ignoreEntityNum);
    vDelta_4 = vEnd[1] - vStart[1];
    vDelta_8 = vEnd[2] - vStart[2];
    fDistSqrd = (float)((float)((float)(*vEnd - *vStart) * (float)(*vEnd - *vStart)) + (float)(vDelta_4 * vDelta_4))
                        + (float)(vDelta_8 * vDelta_8);
    if ( fDistSqrd <= fMaxDistSqrd && fDistSqrd <= level.fFogOpaqueDistSqrd )
        return Actor_SightTrace(self, vStart, vEnd, ignoreEntityNum);
    else
        return 0;
}

char __fastcall Actor_SightTrace(actor_s *self, const float *start, const float *end, int passEntNum)
{
    float value; // [esp+24h] [ebp-50h]
    float foliageIgnorePos[3]; // [esp+38h] [ebp-3Ch] BYREF
    col_context_t context; // [esp+44h] [ebp-30h] BYREF
    int hitNum; // [esp+70h] [ebp-4h] BYREF

    PROF_SCOPED("Actor_SightTrace");

    //col_context_t::col_context_t(&context);
    ++self->iTraceCount;
    if ( g_visitor )
    {
        context.prims = g_visitor->prims;
        context.nprims = g_visitor->nprims;
    }
    context.passEntityNum0 = self->ent->s.number;
    context.passEntityNum1 = passEntNum;
    if ( self->ignoreCloseFoliage )
    {
        foliageIgnorePos[0] = *end - *start;
        foliageIgnorePos[1] = end[1] - start[1];
        foliageIgnorePos[2] = end[2] - start[2];
        Vec3Normalize(foliageIgnorePos);
        value = ai_foliageIngoreDist->current.value;
        foliageIgnorePos[0] = (float)(value * foliageIgnorePos[0]) + *start;
        foliageIgnorePos[1] = (float)(value * foliageIgnorePos[1]) + start[1];
        foliageIgnorePos[2] = (float)(value * foliageIgnorePos[2]) + start[2];
        context.mask = 0x2809803;
        SV_SightTracePoint(&hitNum, foliageIgnorePos, end, &context);
        if ( !hitNum )
        {
            context.mask = 0x2809801;
            SV_SightTracePoint(&hitNum, start, foliageIgnorePos, &context);
        }
    }
    else
    {
        if ( ai_foliageIngoreDist->current.value * ai_foliageIngoreDist->current.value <= Vec3DistanceSq(start, end) )
            context.mask = 0x2809803;
        else
            context.mask = 0x2809801;
        SV_SightTracePoint(&hitNum, start, end, &context);
    }
    if ( hitNum )
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

char __fastcall Actor_CanSeePointEx(
                actor_s *self,
                const float *vPoint,
                float fovDot,
                float fMaxDistSqrd,
                int ignoreEntityNum)
{
    float vViewPos[3]; // [esp+18h] [ebp-Ch] BYREF

    if ( fovDot < 0.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_senses.cpp",
                    247,
                    0,
                    "%s\n\t(fovDot) = %g",
                    "(fovDot >= 0)",
                    fovDot) )
    {
        __debugbreak();
    }
    if ( fMaxDistSqrd < 0.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_senses.cpp",
                    248,
                    0,
                    "%s\n\t(fMaxDistSqrd) = %g",
                    "(fMaxDistSqrd >= 0)",
                    fMaxDistSqrd) )
    {
        __debugbreak();
    }
    if ( g_visitor )
    {
        vViewPos[0] = g_viewPos[0];
        vViewPos[1] = g_viewPos[1];
        vViewPos[2] = g_viewPos[2];
    }
    else
    {
        Actor_GetEyePosition(self, vViewPos);
    }
    return Actor_CanSeePointExInternal(self, vPoint, fovDot, fMaxDistSqrd, ignoreEntityNum, vViewPos);
}

char __fastcall Actor_CanSeePointExInternal(
                actor_s *self,
                const float *vPoint,
                float fovDot,
                float fMaxDistSqrd,
                int ignoreEntityNum,
                float *vViewPos)
{
    if ( fovDot < 0.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_senses.cpp",
                    218,
                    0,
                    "%s\n\t(fovDot) = %g",
                    "(fovDot >= 0)",
                    fovDot) )
    {
        __debugbreak();
    }
    if ( fMaxDistSqrd < 0.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_senses.cpp",
                    219,
                    0,
                    "%s\n\t(fMaxDistSqrd) = %g",
                    "(fMaxDistSqrd >= 0)",
                    fMaxDistSqrd) )
    {
        __debugbreak();
    }
    if ( PointInFovAndRange(self, vViewPos, vPoint, fovDot, fMaxDistSqrd) )
        return Actor_SightTrace(self, vViewPos, vPoint, ignoreEntityNum);
    else
        return 0;
}

bool __fastcall PointInFovAndRange(
                actor_s *self,
                const float *vEyePos,
                const float *vPoint,
                float fovDot,
                float fMaxDistSqrd)
{
    float fDistSqrd; // [esp+8h] [ebp-20h]
    float vForward[3]; // [esp+Ch] [ebp-1Ch] BYREF
    float vDelta[3]; // [esp+18h] [ebp-10h]
    float fDot; // [esp+24h] [ebp-4h]

    vDelta[0] = *vPoint - *vEyePos;
    vDelta[1] = vPoint[1] - vEyePos[1];
    vDelta[2] = vPoint[2] - vEyePos[2];
    fDistSqrd = (float)((float)(vDelta[0] * vDelta[0]) + (float)(vDelta[1] * vDelta[1])) + (float)(vDelta[2] * vDelta[2]);
    if ( fMaxDistSqrd != 0.0 && (fDistSqrd > fMaxDistSqrd || fDistSqrd > level.fFogOpaqueDistSqrd) )
        return 0;
    if ( fovDot == 0.0 )
        return 1;
    Actor_GetEyeDirection(self, vForward);
    fDot = (float)((float)(vDelta[0] * vForward[0]) + (float)(vDelta[1] * vForward[1])) + (float)(vDelta[2] * vForward[2]);
    return fDot >= 0.0 && (float)((float)(fDistSqrd * fovDot) * fovDot) <= (float)(fDot * fDot);
}

char __fastcall Actor_CanSeeEntity(actor_s *self, const gentity_s *ent)
{
    return Actor_CanSeeEntityEx(self, ent, self->fovDot, self->fMaxSightDistSqrd);
}

void __fastcall Actor_UpdateVisCache(actor_s *self, const gentity_s *ent, sentient_info_t *pInfo, bool bVisible)
{
    bool wasVisible; // [esp+Bh] [ebp-1h]

    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_senses.cpp", 289, 0, "%s", "self") )
        __debugbreak();
    if ( !ent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_senses.cpp", 290, 0, "%s", "ent") )
        __debugbreak();
    if ( !pInfo && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_senses.cpp", 291, 0, "%s", "pInfo") )
        __debugbreak();
    wasVisible = pInfo->VisCache.bVisible;
    VisCache_Update(&pInfo->VisCache, bVisible);
    if ( bVisible )
    {
        if ( !wasVisible )
        {
            if ( g_dumpAIEvents->current.integer == self->ent->s.number )
                Com_Printf(18, "%d ^3 visible^7:    entity^5 %d ^7at time^5 %d\n", self->ent->s.number, ent->s.number, level.time);
            if ( ent == Actor_GetTargetEntity(self) )
                Scr_Notify(self->ent, scr_const.enemy_visible, 0);
        }
        pInfo->attackTime = 0;
        Actor_UpdateLastKnownPos(self, ent->sentient);
    }
}

char __fastcall Actor_CanSeeEntityEx(actor_s *self, const gentity_s *ent, float fovDot, float fMaxDistSqrd)
{
    sentient_s *TargetSentient; // eax
    sentient_s *v5; // eax
    float fMaxSightDistSqrd; // [esp+14h] [ebp-54h]
    bool v8; // [esp+18h] [ebp-50h]
    int v9; // [esp+1Ch] [ebp-4Ch]
    bool v10; // [esp+20h] [ebp-48h]
    float v11; // [esp+24h] [ebp-44h]
    char bVisible; // [esp+37h] [ebp-31h]
    float vViewPos[3]; // [esp+3Ch] [ebp-2Ch] BYREF
    sentient_s *sentient; // [esp+48h] [ebp-20h]
    bool bOtherVisible; // [esp+4Fh] [ebp-19h]
    sentient_info_t *pInfo; // [esp+50h] [ebp-18h]
    float fovDotUse; // [esp+54h] [ebp-14h]
    float vDestPos[3]; // [esp+58h] [ebp-10h] BYREF
    int bCacheable; // [esp+64h] [ebp-4h]

    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_senses.cpp", 325, 0, "%s", "self") )
        __debugbreak();
    if ( !self->sentient
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_senses.cpp", 326, 0, "%s", "self->sentient") )
    {
        __debugbreak();
    }
    if ( !ent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_senses.cpp", 327, 0, "%s", "ent") )
        __debugbreak();
    if ( fovDot < 0.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_senses.cpp",
                    342,
                    0,
                    "%s\n\t(fovDot) = %g",
                    "(fovDot >= 0)",
                    fovDot) )
    {
        __debugbreak();
    }
    if ( fMaxDistSqrd < 0.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_senses.cpp",
                    343,
                    0,
                    "%s\n\t(fMaxDistSqrd) = %g",
                    "(fMaxDistSqrd >= 0)",
                    fMaxDistSqrd) )
    {
        __debugbreak();
    }
    sentient = ent->sentient;
    if ( sentient )
    {
        pInfo = &self->sentientInfo[sentient - level.sentients];
        Sentient_GetEyePosition(sentient, vDestPos);
        Actor_GetEyePosition(self, vViewPos);
        fovDotUse = fovDot;
        TargetSentient = Actor_GetTargetSentient(self);
        if ( TargetSentient == sentient
            //|| SentientHandle::isDefined(&self->pFavoriteEnemy)
            || self->pFavoriteEnemy.isDefined()
            //&& (v5 = SentientHandle::sentient(&self->pFavoriteEnemy), v5 == sentient) )
            && (v5 = self->pFavoriteEnemy.sentient(), v5 == sentient))
        {
            fovDotUse = 0.0f;
        }
        if ( (float)(fMaxDistSqrd - (float)(sentient->maxVisibleDist * sentient->maxVisibleDist)) < 0.0 )
            v11 = fMaxDistSqrd;
        else
            v11 = sentient->maxVisibleDist * sentient->maxVisibleDist;
        fMaxDistSqrd = v11;
        bVisible = Actor_CanSeePointExInternal(self, vDestPos, fovDotUse, v11, ent->s.number, vViewPos);
    }
    else
    {
        if ( !G_DObjGetWorldTagPos(ent, scr_const.tag_eye, vDestPos) )
            G_EntityCentroid(ent, vDestPos);
        pInfo = 0;
        bVisible = Actor_CanSeePointEx(self, vDestPos, fovDot, fMaxDistSqrd, ent->s.number);
    }
    if ( bVisible )
    {
        v10 = fovDot >= self->fovDot && self->fMaxSightDistSqrd >= fMaxDistSqrd;
        v9 = v10;
    }
    else
    {
        v8 = self->fovDot >= fovDot && fMaxDistSqrd >= self->fMaxSightDistSqrd;
        v9 = v8;
    }
    bCacheable = v9;
    if ( v9 && sentient )
        Actor_UpdateVisCache(self, ent, pInfo, bVisible);
    if ( !bVisible )
        return 0;
    if ( ent->actor )
    {
        bOtherVisible = 1;
        if ( !ent->sentient
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_senses.cpp", 408, 0, "%s", "ent->sentient") )
        {
            __debugbreak();
        }
        if ( (float)(ent->actor->fMaxSightDistSqrd - (float)(self->sentient->maxVisibleDist * self->sentient->maxVisibleDist)) < 0.0 )
            fMaxSightDistSqrd = ent->actor->fMaxSightDistSqrd;
        else
            fMaxSightDistSqrd = self->sentient->maxVisibleDist * self->sentient->maxVisibleDist;
        if ( !PointInFovAndRange(ent->actor, vDestPos, vViewPos, ent->actor->fovDot, fMaxSightDistSqrd) )
            bOtherVisible = 0;
        pInfo = &ent->actor->sentientInfo[self->sentient - level.sentients];
        Actor_UpdateVisCache(ent->actor, self->ent, pInfo, bOtherVisible);
    }
    return 1;
}

char __fastcall Actor_CanSeeSentient(actor_s *self, sentient_s *sentient, int iMaxLatency)
{
    if ( self->sentientInfo[sentient - level.sentients].VisCache.iLastUpdateTime
        && iMaxLatency + self->sentientInfo[sentient - level.sentients].VisCache.iLastUpdateTime >= level.time )
    {
        return self->sentientInfo[sentient - level.sentients].VisCache.bVisible;
    }
    else
    {
        return Actor_CanSeeEntity(self, sentient->ent);
    }
}

char __fastcall Actor_CanSeeEnemy(actor_s *self)
{
    const gentity_s *v2; // eax
    sentient_s *enemy; // [esp+4h] [ebp-4h]

    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_senses.cpp", 456, 0, "%s", "self") )
        __debugbreak();
    if ( !self->sentient
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_senses.cpp", 457, 0, "%s", "self->sentient") )
    {
        __debugbreak();
    }
    //if ( !EntHandle::isDefined(&self->sentient->targetEnt)
    if ( !self->sentient->targetEnt.isDefined()
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_senses.cpp",
                    458,
                    0,
                    "%s",
                    "self->sentient->targetEnt.isDefined()") )
    {
        __debugbreak();
    }
    enemy = Actor_GetTargetSentient(self);
    if ( enemy )
        return Actor_CanSeeSentient(self, enemy, 250);
    //v2 = EntHandle::ent(&self->sentient->targetEnt);
    v2 = self->sentient->targetEnt.ent();
    return Actor_CanSeeEntity(self, v2);
}

struct sentient_sort_t // sizeof=0x8
{                                       // XREF: ?Actor_UpdateSight@@YIXPAUactor_s@@@Z/r
    sentient_s *sentient;               // XREF: Actor_UpdateSight(actor_s *)+324/w
                                        // Actor_UpdateSight(actor_s *)+3DF/r
    float fMetric;                      // XREF: Actor_UpdateSight(actor_s *)+314/w
};

void __fastcall Actor_UpdateSight(actor_s *self)
{
    double v1; // st7
    float v2; // [esp+4h] [ebp-208h]
    int v4; // [esp+38h] [ebp-1D4h]
    float *currentOrigin; // [esp+40h] [ebp-1CCh]
    float fDistSqrd; // [esp+58h] [ebp-1B4h]
    int iOldTraceCount; // [esp+64h] [ebp-1A8h]
    sentient_s *sentient; // [esp+6Ch] [ebp-1A0h]
    int iTeamFlags; // [esp+70h] [ebp-19Ch]
    int i; // [esp+74h] [ebp-198h]
    float v[3]; // [esp+78h] [ebp-194h] BYREF
    sentient_sort_t check[MAX_SENTIENTS_CAP]; // [esp+84h] [ebp-188h] BYREF
    int iCheckCount; // [esp+208h] [ebp-4h]

    PROF_SCOPED("Actor_UpdateSight");

    iassert(self);
    iassert(self->sentient);

    iTeamFlags = 1 << Sentient_EnemyTeam(self->sentient->eTeam);

    {
        PROF_SCOPED("sight 1");
        iCheckCount = 0;
        for ( sentient = Sentient_FirstSentient(iTeamFlags); sentient; sentient = Sentient_NextSentient(sentient, iTeamFlags) )
        {
            if ( !sentient->ent
                && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_senses.cpp", 712, 0, "%s", "sentient->ent") )
            {
                __debugbreak();
            }
            if ( sentient->ent->s.number != self->ent->s.number
                //&& (!EntHandle::isDefined(&self->sentient->scriptOwner)
                && (!self->sentient->scriptOwner.isDefined()
                 //|| sentient->ent != EntHandle::ent(&self->sentient->scriptOwner)) )
                 || sentient->ent != self->sentient->scriptOwner.ent()) )
            {
                Sentient_GetOrigin(sentient, v);
                currentOrigin = self->ent->r.currentOrigin;
                v[0] = v[0] - *currentOrigin;
                v[1] = v[1] - currentOrigin[1];
                v[2] = v[2] - currentOrigin[2];
                fDistSqrd = (float)((float)(v[0] * v[0]) + (float)(v[1] * v[1])) + (float)(v[2] * v[2]);
                iassert(!IS_NAN(fDistSqrd));
                if ( fDistSqrd != 0.0 )
                {
                    v4 = level.time - self->sentientInfo[sentient - level.sentients].VisCache.iLastUpdateTime - 100;
                    v2 = (float)(v4 & ~(v4 >> 31));
                    v1 = I_rsqrt(fDistSqrd);
                    check[iCheckCount].fMetric = v1 * v2;
                    check[iCheckCount++].sentient = sentient;
                }
            }
        }
        if ( iCheckCount > 1 )
            qsort(check, iCheckCount, 8u, (int (__cdecl *)(const void *, const void *))compare_sentient_sort);
    }
    {
        PROF_SCOPED("sight 2");
        iOldTraceCount = self->iTraceCount;
        for (i = 0; i < iCheckCount; ++i)
        {
            Actor_CanSeeSentient(self, check[i].sentient, 0);
            if (self->iTraceCount != iOldTraceCount)
                break;
        }
    }
}

int __cdecl compare_sentient_sort(unsigned int *pe1, unsigned int *pe2)
{
    return pe2[1] - pe1[1];
}

void __fastcall Actor_UpdateLastKnownPos(actor_s *self, sentient_s *other)
{
    sentient_info_t *pInfo; // [esp+8h] [ebp-4h]

    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_senses.cpp", 776, 0, "%s", "self") )
        __debugbreak();
    if ( !other && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_senses.cpp", 777, 0, "%s", "other") )
        __debugbreak();
    if ( (other < level.sentients || other >= &level.sentients[MAX_SENTIENTS])
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_senses.cpp",
                    778,
                    0,
                    "%s",
                    "other >= level.sentients && other < level.sentients + MAX_SENTIENTS") )
    {
        __debugbreak();
    }
    if ( other != &level.sentients[other - level.sentients]
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_senses.cpp",
                    779,
                    0,
                    "%s",
                    "other == level.sentients + (other - level.sentients)") )
    {
        __debugbreak();
    }
    pInfo = &self->sentientInfo[other - level.sentients];
    self->sentientInfo[other - level.sentients].lastKnownPosTime = level.time;
    { extern int g_modSentientKnownMax[]; int &m = g_modSentientKnownMax[other - level.sentients]; if (m < level.time) m = level.time; } // mod (L55): actor_mp.cpp bound
    Sentient_GetOrigin(other, pInfo->vLastKnownPos);
    pInfo->pLastKnownNode = other->pNearestNode;
}

void __fastcall Actor_UpdateLastEnemySightPos(actor_s *self)
{
    sentient_s *enemy; // [esp+28h] [ebp-8h]

    PROF_SCOPED("Actor_UpdateLastEnemySightPos");

    enemy = Actor_GetTargetSentient(self);
    if ( enemy )
    {
        if ( self->sentientInfo[enemy - level.sentients].VisCache.bVisible )
        {
            if ( self->sentientInfo[enemy - level.sentients].VisCache.iLastVisTime == level.time )
            {
                self->lastEnemySightPosValid = 1;
                Sentient_GetEyePosition(enemy, self->lastEnemySightPos);
            }
        }
    }
}

void __fastcall Actor_GetEyePosition(actor_s *self, float *vEyePosOut)
{
    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_senses.cpp", 871, 0, "%s", "self") )
        __debugbreak();
    if ( !vEyePosOut
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_senses.cpp", 872, 0, "%s", "vEyePosOut") )
    {
        __debugbreak();
    }
    Actor_UpdateEyeInformation(self);
    *vEyePosOut = self->eyeInfo.pos[0];
    vEyePosOut[1] = self->eyeInfo.pos[1];
    vEyePosOut[2] = self->eyeInfo.pos[2];
}

void __fastcall Actor_UpdateEyeInformation(actor_s *self)
{
    DObj *ServerDObj; // eax
    const char *Name; // eax
    float *currentOrigin; // [esp+10h] [ebp-4Ch]
    float tagMat[4][3]; // [esp+2Ch] [ebp-30h] BYREF

    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_senses.cpp", 829, 0, "%s", "self") )
        __debugbreak();
    if ( !self->ent
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_senses.cpp", 830, 0, "%s", "self->ent") )
    {
        __debugbreak();
    }
    if ( self->eyeInfo.time != level.time )
    {
        self->eyeInfo.time = level.time;
        if ( G_DObjGetWorldTagMatrix(self->ent, scr_const.tag_eye, tagMat) )
        {
            self->eyeInfo.pos[0] = tagMat[3][0];
            self->eyeInfo.pos[1] = tagMat[3][1];
            self->eyeInfo.pos[2] = tagMat[3][2];
            *(_QWORD *)self->eyeInfo.dir = *(_QWORD *)&tagMat[0][0];
            Vec2Normalize(self->eyeInfo.dir);
            self->eyeInfo.dir[2] = 0.0f;
        }
        else
        {
            if ( Com_GetServerDObj(self->ent->s.number) )
            {
                ServerDObj = Com_GetServerDObj(self->ent->s.number);
                Name = DObjGetName(ServerDObj);
                Com_Printf(18, "Actor_UpdateEyeInformation: Actor dobj doesn't have TAG_EYE.(%s)\n", Name);
            }
            currentOrigin = self->ent->r.currentOrigin;
            self->eyeInfo.pos[0] = *currentOrigin;
            self->eyeInfo.pos[1] = currentOrigin[1];
            self->eyeInfo.pos[2] = currentOrigin[2];
            self->eyeInfo.pos[2] = self->eyeInfo.pos[2] + 64.0;
            self->eyeInfo.dir[0] = self->vLookForward[0];
            self->eyeInfo.dir[1] = self->vLookForward[1];
            self->eyeInfo.dir[2] = self->vLookForward[2];
        }
    }
}

void __fastcall Actor_GetDebugEyePosition(actor_s *self, float *vEyePosOut)
{
    float *currentOrigin; // [esp+8h] [ebp-Ch]
    int i; // [esp+10h] [ebp-4h]

    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_senses.cpp", 896, 0, "%s", "self") )
        __debugbreak();
    if ( !vEyePosOut
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_senses.cpp", 897, 0, "%s", "vEyePosOut") )
    {
        __debugbreak();
    }
    *vEyePosOut = self->eyeInfo.pos[0];
    vEyePosOut[1] = self->eyeInfo.pos[1];
    vEyePosOut[2] = self->eyeInfo.pos[2];
    for ( i = 0; i < 3; ++i )
    {
        if ( (float)(self->ent->r.absmin[i] - 32.0) > vEyePosOut[i]
            || vEyePosOut[i] > (float)(self->ent->r.absmax[i] + 32.0) )
        {
            currentOrigin = self->ent->r.currentOrigin;
            *vEyePosOut = *currentOrigin;
            vEyePosOut[1] = currentOrigin[1];
            vEyePosOut[2] = currentOrigin[2];
            vEyePosOut[2] = vEyePosOut[2] + 56.0;
            return;
        }
    }
}

void __fastcall Actor_GetEyeDirection(actor_s *self, float *vEyeDir)
{
    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_senses.cpp", 922, 0, "%s", "self") )
        __debugbreak();
    if ( !vEyeDir
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_senses.cpp", 923, 0, "%s", "vEyeDir") )
    {
        __debugbreak();
    }
    Actor_UpdateEyeInformation(self);
    *vEyeDir = self->eyeInfo.dir[0];
    vEyeDir[1] = self->eyeInfo.dir[1];
    vEyeDir[2] = self->eyeInfo.dir[2];
}

void __fastcall Actor_GetEyeOffset(actor_s *self, float *vEyePosOut)
{
    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_senses.cpp", 939, 0, "%s", "self") )
        __debugbreak();
    if ( !vEyePosOut
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_senses.cpp", 940, 0, "%s", "vEyePosOut") )
    {
        __debugbreak();
    }
    *vEyePosOut = 0.0f;
    vEyePosOut[1] = 0.0f;
    vEyePosOut[2] = ACTOR_EYE_OFFSET;
}

int __fastcall Actor_GetMuzzleInfo(actor_s *self, float *vOrigin, float *vForward)
{
    float tagMat[4][3]; // [esp+1Ch] [ebp-30h] BYREF

    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_senses.cpp", 957, 0, "%s", "self") )
        __debugbreak();
    if ( self->muzzleInfo.time != level.time )
    {
        if ( !G_DObjGetWorldTagMatrix(self->ent, scr_const.tag_flash, tagMat) )
            return 0;
        self->muzzleInfo.time = level.time;
        self->muzzleInfo.pos[0] = tagMat[3][0];
        self->muzzleInfo.pos[1] = tagMat[3][1];
        self->muzzleInfo.pos[2] = tagMat[3][2];
        *(_QWORD *)self->muzzleInfo.dir = *(_QWORD *)&tagMat[0][0];
        self->muzzleInfo.dir[2] = tagMat[0][2];
    }
    if ( !vOrigin
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_senses.cpp", 976, 0, "%s", "vOrigin") )
    {
        __debugbreak();
    }
    *vOrigin = self->muzzleInfo.pos[0];
    vOrigin[1] = self->muzzleInfo.pos[1];
    vOrigin[2] = self->muzzleInfo.pos[2];
    if ( vForward )
    {
        *vForward = self->muzzleInfo.dir[0];
        vForward[1] = self->muzzleInfo.dir[1];
        vForward[2] = self->muzzleInfo.dir[2];
    }
    return 1;
}

