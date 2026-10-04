#include "actor_zombie_exposed.h"
#include "actor_exposed.h"
#include "actor_state.h"
#include <game_sp/actor_sp_ext.h>
#include <game_sp/g_sp_playtrace.h>
#include <game_sp/g_sp_measure.h>
#include <game_mp/g_main_mp.h>
#include <game_mp/g_utils_mp.h>
#include <game_mp/g_spawn_mp.h>
#include <clientscript/cscr_vm.h>
#include <clientscript/cscr_variable.h>
#include <clientscript/cscr_stringlist.h>
#include <qcommon/common.h>
#include <bgame/bg_misc.h>
#include <universal/dvar.h>
#include "actor_animapi.h"
#include "actor_corpse.h"
#include "actor_navigation.h"
#include "actor_senses.h"
#include "actor_team_move.h"
#include "actor_events.h"
#include <physics/phys_main.h>
#include <game_mp/actor_mp.h>
#include <clientscript/scr_const.h>
#include "actor_threat.h"
#include "actor_script_cmd.h"
#include "turret.h"
#include <qcommon/cm_trace.h>
#include <server/sv_game.h>
#include <qcommon/dobj_management.h>
#include <server/sv_world.h>
#include <universal/com_math_anglevectors.h>
#include "g_missile.h"
#include <bgame/bg_weapons_def.h>
#include <EffectsCore/fx_system.h>
#include <cgame/cg_drawtools.h>
#include <xanim/xmodel_utils.h>

// Additional SP -> KB mappings used below (SP and KB are not a constant displacement):
// +b94 eState[] (+b98 is eState[1]), +bcc preThinkTime, +cd0 CodeOrient, +ce0 ScriptOrient,
// +dcc pAnimScriptFunc, +ddc moveMode, +ddd safeToChangeScript, +dde bUseGoalWeight,
// +de0 eAnimMode; +15bc Path, +193c Path.wPathLen, +1948 Path.vFinalGoal,
// +1978 Path.flags, +197c Path.iPathTime, +1994 Path.iPathEndTime,
// +19a4 fWalkDist, +19bc pathWaitTime, +19c0 iTeamMoveWaitTime, +19c4 iTeamMoveDodgeTime,
// +19c8/+19cc pPileUpActor/pPileUpEnt, +19dc/+19dd keepClaimedNode/keepClaimedNodeInGoal,
// +19e0 mayMoveTime, +19e4 prevMoveDir, +19f4 exposedStartTime; +19fc codeGoal, +1a24 codeGoalSrc,
// +1a28 scriptGoal, +1a50 scriptGoalEnt, +1a58 pathEnemyFightDist,
// +1a60 useEnemyGoal, +1a62 goalPosChanged, +1a63 commitToFixedNode, +1a65 fixedNode,
// +1a74 bNotifyTurnDone (orientation turnNotify, not the nearby cover-node count),
// +1a7c arrivalInfo.arrivalNotifyRequested,
// +1a80 arrivalInfo.animscriptOverrideRunTo, +1a84 arrivalInfo.animscriptOverrideRunToPos,
// +1aec/+1af0 fovDot/fMaxSightDistSqrd, +1af8 sentientInfo[] (SP stride 0x28),
// +2098 pFavoriteEnemy, +2148 threatUpdateTime, +214c hasThreateningEnemy,
// +215c pGrenade, +21bc badPlaceAwareness, +21d6 pCloseEnt, +2230 yawVeloc,
// +2238 flashBanged, +2248 side-table linkYawOnly, +2254 pszDebugInfo. sentient+30/+34/+5c -> syncedMeleeEnt,
// targetEnt/pClaimedNode. SP sentient stride 0x8c is NOT KB's stride 0x90.

static void Actor_UpdateGoalPath_SP(actor_s *self);
void Actor_FindPathToGoalDirect_SP(actor_s *self);
static bool Actor_FindPath_SP(actor_s *self, const float *goal, int allowNegotiation, bool ignoreSuppression);
static bool Actor_GetNodeAnim_SP(actor_s *self, pathnode_t *node, bool checkEnemy, scr_animscript_t **script);
bool FUN_005ae040(actor_s *self, pathnode_t *node);
static float FUN_007bfe20(actor_s *self, pathnode_t *node);
static unsigned int g_exposedNoTargetCount; // SP DAT_01a48ca0, no KB statistics consumer yet
extern colgeom_visitor_inlined_t<200> *g_visitor;
extern float g_viewPos[3];

static const dvar_t *ai_coverScore_coverType, *ai_coverScore_engagement, *ai_coverScore_nodeAngle;
static const dvar_t *ai_coverScore_playerLos, *ai_coverScore_priority, *ai_coverScore_targetDir;
static const dvar_t *ai_coverScore_visibility, *ai_coverScore_flanking;
static const dvar_t *ai_coverFlankMaxAngle, *ai_coverFlankDistToCover, *ai_coverFlankCheckRad;
static const dvar_t *ai_showSuppression, *ai_debugGrenadeHintArc;
static float g_grenadeHints[512][3]; // SP DAT_01a4ede0, count DAT_01c07064
static unsigned int g_grenadeHintCount;
static float g_grenadeHintSortOrigin[3]; // SP DAT_01a4edd0

// zombies: SP_info_grenade_hint (SP 0x00441ff0)
void __cdecl SP_info_grenade_hint_SP(gentity_s *ent, SpawnVar *)
{
    if ( g_grenadeHintCount > 0x1ff )
        Com_Error(ERR_DROP, "\x15MAX_GRENADE_HINTS (%i) exceeded\n", 512);
    // SP entity+0x11c/+0x120/+0x124 -> r.currentOrigin[0..2], not KB raw offsets.
    g_grenadeHints[g_grenadeHintCount][0] = ent->r.currentOrigin[0];
    g_grenadeHints[g_grenadeHintCount][1] = ent->r.currentOrigin[1];
    unsigned int index = g_grenadeHintCount;
    ++g_grenadeHintCount;
    g_grenadeHints[index][2] = ent->r.currentOrigin[2];
    G_FreeEntity(ent); // SP 0x00438a70; the map marker has no remaining entity lifetime.
}

// zombies: AI_RegisterDvars_A cover-score subset (SP 0x007e0c00)
void Actor_SP_RegisterExposedDvars()
{
    ai_coverScore_coverType = _Dvar_RegisterFloat("ai_coverScore_coverType", 2.0f, 0.0f, 50.0f, 0x2080u, "");
    ai_coverScore_engagement = _Dvar_RegisterFloat("ai_coverScore_engagement", 4.0f, 0.0f, 50.0f, 0x2080u, "");
    ai_coverScore_nodeAngle = _Dvar_RegisterFloat("ai_coverScore_nodeAngle", 4.0f, 0.0f, 50.0f, 0x2080u, "");
    ai_coverScore_playerLos = _Dvar_RegisterFloat("ai_coverScore_playerLos", 8.0f, 0.0f, 50.0f, 0x2080u, "");
    ai_coverScore_priority = _Dvar_RegisterFloat("ai_coverScore_priority", 8.0f, 0.0f, 50.0f, 0x2080u, "");
    ai_coverScore_targetDir = _Dvar_RegisterFloat("ai_coverScore_targetDir", 4.0f, 0.0f, 50.0f, 0x2080u, "");
    ai_coverScore_visibility = _Dvar_RegisterFloat("ai_coverScore_visibility", 8.0f, 0.0f, 50.0f, 0x2080u, "");
    ai_coverScore_flanking = _Dvar_RegisterFloat("ai_coverScore_flanking", 0.0f, 0.0f, 50.0f, 0x2080u, "");
    ai_coverFlankMaxAngle = _Dvar_RegisterFloat("ai_coverFlankMaxAngle", 70.0f, 0.0f, 360.0f, 0x80u, "");
    ai_coverFlankDistToCover = _Dvar_RegisterFloat("ai_coverFlankDistToCover", 180.0f, 0.0f, 360.0f, 0x80u, "");
    ai_coverFlankCheckRad = _Dvar_RegisterFloat("ai_coverFlankCheckRad", 6.0f, 0.0f, 60.0f, 0x80u, "");
    // zombies: AI_RegisterDvars_B (SP 0x007e1c10).
    ai_showSuppression = _Dvar_RegisterInt("ai_showSuppression", -1, -1, 1023, 0x80u, "");
    ai_debugGrenadeHintArc = _Dvar_RegisterFloat("ai_debugGrenadeHintArc", 150.0f, 0.0f, 180.0f, 0x80u, "");
    g_grenadeHintCount = 0;
}

// zombies: Actor_UpdateEyeInformation (SP 0x007cf350)
static void Actor_UpdateEyeInformation_SP(actor_s *self)
{
    // SP +d30..d48 -> eyeInfo.time/pos/dir, +cac..cb4 -> vLookForward.
    if ( self->eyeInfo.time == level.time )
        return;
    bool useTag = true;
    if ( zombiemode->current.enabled )
        useTag = !ai_useCheapSight->current.enabled;
    self->eyeInfo.time = level.time;
    if ( useTag )
    {
        float matrix[4][3];
        if ( G_DObjGetWorldTagMatrix(self->ent, scr_const.tag_eye, matrix) ) // 0x00489730
        {
            Vec3Copy(matrix[3], self->eyeInfo.pos);
            self->eyeInfo.dir[0] = matrix[0][0];
            self->eyeInfo.dir[1] = matrix[0][1];
            float length = sqrtf(self->eyeInfo.dir[0] * self->eyeInfo.dir[0]
                + self->eyeInfo.dir[1] * self->eyeInfo.dir[1]);
            if ( -length >= 0.0f )
                length = 1.0f;
            self->eyeInfo.dir[0] *= 1.0f / length;
            self->eyeInfo.dir[1] *= 1.0f / length;
            self->eyeInfo.dir[2] = 0.0f;
            return;
        }
        if ( Com_GetServerDObj(self->ent->s.number) )
            Com_Printf(18, "Actor_UpdateEyeInformation: Actor dobj doesn't have TAG_EYE.(%s)\n",
                DObjGetName(Com_GetServerDObj(self->ent->s.number)));
    }
    Vec3Copy(self->ent->r.currentOrigin, self->eyeInfo.pos);
    self->eyeInfo.pos[2] += 64.0f;
    Vec3Copy(self->vLookForward, self->eyeInfo.dir);
}

// zombies: Actor_IsUsingTurret (SP 0x00473160); also used by the x3 AI builtins.
bool Actor_IsUsingTurret_SP(actor_s *self)
{
    // SP actor+21a8 -> side turret; entity+134 -> r.ownerNum.
    gentity_s *turret = Actor_SP_Ext(self)->turret;
    return turret && turret->r.ownerNum.isDefined() && turret->r.ownerNum.ent() == self->ent;
}

// zombies: turret_CanTargetSentient (SP 0x0042f450)
bool turret_CanTargetSentient_SP(gentity_s *self, sentient_s *enemy,
    float *target, float *source, float *localAngles)
{
    // MP predicts a client's position here; SP always obtains its eye position.
    if ( enemy->ent->actor )
        Actor_UpdateEyeInformation_SP(enemy->ent->actor);
    Sentient_GetEyePosition(enemy, target);
    float muzzle[3], delta[3], angles[3];
    if ( !G_DObjGetWorldTagPos(self, scr_const.tag_flash, muzzle) ) // SP 0x004ff790
        return false;
    Vec3Sub(target, muzzle, delta);
    // SP entity+14c -> pTurretInfo; +128/+12c -> currentAngles pitch/yaw.
    TurretInfo *turret = self->pTurretInfo;
    vectosignedangles(delta, angles); // SP 0x005c2940
    localAngles[1] = AngleNormalize180(angles[1] - self->r.currentAngles[1]);
    if ( localAngles[1] > turret->arcmax[1] )
        return false;
    if ( localAngles[1] < turret->arcmin[1] )
        return false;
    Vec3Copy(muzzle, source);
    bool tooLow;
    if ( turret->state == 1 )
    {
        localAngles[0] = AngleDelta(angles[0], self->r.currentAngles[0]); // SP 0x0057ab60
        if ( localAngles[0] <= turret->arcmax[0] )
        {
            if ( localAngles[0] >= turret->arcmin[0] )
                return true;
            tooLow = false;
        }
        else
            tooLow = true;
        Vec3Sub(enemy->ent->r.currentOrigin, muzzle, delta);
        delta[2] += 2.0f;
        localAngles[0] = AngleDelta(vectosignedpitch(delta), self->r.currentAngles[0]); // SP 0x00653290
        if ( localAngles[0] > turret->arcmax[0] )
            goto above_arc;
        if ( localAngles[0] >= turret->arcmin[0] )
            goto use_feet;
    }
    else
    {
        Vec3Sub(enemy->ent->r.currentOrigin, muzzle, delta);
        delta[2] += 2.0f;
        localAngles[0] = AngleDelta(vectosignedpitch(delta), self->r.currentAngles[0]);
        if ( localAngles[0] <= turret->arcmax[0] )
        {
            if ( localAngles[0] >= turret->arcmin[0] )
                goto use_feet;
            tooLow = false;
        }
        else
            tooLow = true;
        localAngles[0] = AngleDelta(angles[0], self->r.currentAngles[0]);
        if ( localAngles[0] > turret->arcmax[0] )
            goto above_arc;
        if ( localAngles[0] >= turret->arcmin[0] )
            return true;
    }
    if ( !tooLow )
        return false;
    goto use_level;
above_arc:
    if ( tooLow )
        return false;
use_level:
    localAngles[0] = 0.0f;
    target[2] = muzzle[2];
    return true;
use_feet:
    Vec3Copy(enemy->ent->r.currentOrigin, target);
    target[2] += 2.0f;
    return true;
}

// zombies: PointInFovAndRange (SP 0x007cf4f0)
static bool PointInFovAndRange_SP(actor_s *self, const float *eye, const float *point,
    float fovDot, float maxDistSq)
{
    float delta[3];
    Vec3Sub(point, eye, delta);
    float distSq = delta[0] * delta[0] + delta[2] * delta[2] + delta[1] * delta[1];
    if ( maxDistSq != 0.0f && (maxDistSq < distSq || level.fFogOpaqueDistSqrd < distSq) )
        return false;
    if ( fovDot != 0.0f )
    {
        Actor_UpdateEyeInformation_SP(self);
        float dot = self->eyeInfo.dir[0] * delta[0] + self->eyeInfo.dir[2] * delta[2]
            + self->eyeInfo.dir[1] * delta[1];
        if ( dot < 0.0f )
            return false;
        if ( dot * dot < distSq * fovDot * fovDot )
            return false;
    }
    return true;
}

// zombies: FX_GetServerVisibility (SP 0x0044f050)
float FX_GetServerVisibility_SP(const float *start, const float *end)
{
    // SP thunk 0x005d8b90 forwards here. DAT_01a48bb0 -> fx_serverVisClient;
    // 0x00665a30 is KB FX_GetClientVisibility, including the FX_VIS critical section
    // (SP 0x005fea60/0x0056d400), packed blocker radius/visibility and minimum distance.
    // SP always has an FX system. A KB dedicated server has no client FX allocation;
    // that represents the same empty-blocker case, whose native result is 1.
    if ( !FX_GetSystemRemote(fx_serverVisClient) )
        return 1.0f;
    return (float)FX_GetClientVisibility(fx_serverVisClient, start, end);
}

// zombies: Actor_SightTrace (SP 0x007cf070)
static bool Actor_SightTrace_SP(actor_s *self, const float *start, const float *end, int passEntNum)
{
    col_context_t context; // SP 0x004b3e90
    ++self->iTraceCount; // SP actor+ca0
    sentient_s *sentient = level.gentities[passEntNum].sentient;
    // SP reuses the end-pointer stack slot as an initial hint for non-sentients.
    // Zero means no hint; CM_SightTracePoint uses valid hints only as a brush fast path.
    int hitNum = 0;
    if ( sentient )
        hitNum = Actor_SP_Ext(self)->sightTraceHitNum[sentient - level.sentients]; // SP +31f8
    if ( g_visitor )
    {
        context.prims = g_visitor->prims;
        context.nprims = g_visitor->nprims;
    }
    context.passEntityNum0 = self->ent->s.number;
    context.passEntityNum1 = passEntNum;
    float foliagePos[3];
    const float *traceEnd = end;
    // SP masks 0x280d803/1 include SP actor contents 0x4000; KB actors use 0x8000.
    if ( !self->ignoreCloseFoliage ) // SP +1af4
    {
        float delta[3];
        Vec3Sub(end, start, delta);
        float distSq = delta[0] * delta[0] + delta[2] * delta[2] + delta[1] * delta[1];
        context.mask = ai_foliageIngoreDist->current.value * ai_foliageIngoreDist->current.value <= distSq
            ? 0x2809803 : 0x2809801;
    }
    else
    {
        float delta[3];
        Vec3Sub(end, start, delta);
        float length = sqrtf(delta[2] * delta[2] + delta[0] * delta[0] + delta[1] * delta[1]);
        if ( -length >= 0.0f )
            length = 1.0f;
        float invLength = 1.0f / length;
        float distance = ai_foliageIngoreDist->current.value;
        foliagePos[0] = distance * invLength * delta[0] + start[0];
        foliagePos[1] = distance * invLength * delta[1] + start[1];
        foliagePos[2] = delta[2] * invLength * distance + start[2];
        context.mask = 0x2809803;
        SV_SightTracePoint(&hitNum, foliagePos, end, &context); // SP 0x005f4330, same KB implementation
        if ( hitNum )
            goto trace_result;
        context.mask = 0x2809801;
        traceEnd = foliagePos;
    }
    SV_SightTracePoint(&hitNum, start, traceEnd, &context);
trace_result:
    if ( sentient )
        Actor_SP_Ext(self)->sightTraceHitNum[sentient - level.sentients] = (unsigned short)hitNum;
    if ( !hitNum )
    {
        if ( FX_GetServerVisibility_SP(start, end) >= 0.2f )
            return true;
    }
    return false;
}

// zombies: Actor_CanSeePointEx (SP 0x005e5250)
bool Actor_CanSeePointEx_SP(actor_s *self, const float *point, float fovDot,
    float maxDistSq, int ignoreEntityNum)
{
    float viewPos[3], turretAngles[2];
    if ( !Actor_IsUsingTurret_SP(self)
        || !turret_CanTargetPoint(Actor_SP_Ext(self)->turret, point, viewPos, turretAngles) ) // SP 0x00464f00
    {
        if ( !g_visitor )
        {
            Actor_UpdateEyeInformation_SP(self);
            Vec3Copy(self->eyeInfo.pos, viewPos);
        }
        else
            Vec3Copy(g_viewPos, viewPos);
    }
    if ( PointInFovAndRange_SP(self, viewPos, point, fovDot, maxDistSq) )
        return Actor_SightTrace_SP(self, viewPos, point, ignoreEntityNum);
    return false;
}

// zombies: Actor_UpdateVisCache (SP 0x00556cf0)
static void Actor_UpdateVisCache_SP(actor_s *self, const gentity_s *ent, sentient_info_t *info, bool visible)
{
    bool wasVisible = info->VisCache.bVisible;
    VisCache_Update(&info->VisCache, visible); // SP 0x004f5440
    if ( visible )
    {
        if ( !wasVisible )
        {
            if ( g_dumpAIEvents->current.integer == self->ent->s.number )
                Com_Printf(18, "%d ^3 visible^7:  entity^5 %d ^7at time^5 %d\n",
                    self->ent->s.number, ent->s.number, level.time); // SP 0x0043bf30
            if ( ent == Actor_GetTargetEntity(self) )
                Scr_Notify(self->ent, scr_const.enemy_visible, 0);
        }
        if ( !Actor_IsUsingTurret_SP(self) )
            info->attackTime = 0;
        // SP inlines lastKnownPosTime, Sentient_GetOrigin (0x005f2350), pLastKnownNode.
        Actor_UpdateLastKnownPos(self, ent->sentient);
    }
}

// zombies: FUN_00629a20 / Actor_CanSeeEntityEx (SP 0x00629a20)
static bool Actor_CanSeeEntityEx_SP(actor_s *self, const gentity_s *ent, float fovDot, float maxDistSq)
{
    sentient_s *sentient = ent->sentient;
    sentient_info_t *info = NULL;
    float targetPos[3], viewPos[3];
    bool visible;
    if ( !sentient )
    {
        if ( !G_DObjGetWorldTagPos(ent, scr_const.tag_eye, targetPos) ) // 0x004ff790
            G_EntityCentroid(ent, targetPos); // 0x00474ea0, confirmed by rebuild getcentroid port
        // SP's point test has a turret-origin branch absent from MP Actor_CanSeePointEx.
        visible = Actor_CanSeePointEx_SP(self, targetPos, fovDot, maxDistSq, ent->s.number);
    }
    else
    {
        info = &self->sentientInfo[sentient - level.sentients];
        // Prime the SP eye cache before KB's otherwise-identical sentient-eye dispatcher.
        if ( ent->actor )
            Actor_UpdateEyeInformation_SP(ent->actor);
        Sentient_GetEyePosition(sentient, targetPos); // 0x004fbe10
        if ( !Actor_IsUsingTurret_SP(self) )
        {
            Actor_UpdateEyeInformation_SP(self);
            Vec3Copy(self->eyeInfo.pos, viewPos);
        }
        else
        {
            float turretAngles[2];
            gentity_s *turret = Actor_SP_Ext(self)->turret; // SP +21a8
            if ( !turret_CanTargetPoint(turret, targetPos, viewPos, turretAngles)
                && !turret_CanTargetSentient_SP(turret, sentient, targetPos, viewPos, turretAngles) )
            {
                if ( level.time - info->lastKnownPosTime >= 1000
                    && Vec2DistanceSq(self->ent->r.currentOrigin, targetPos) >= 262144.0f )
                    return false;
                Actor_UpdateEyeInformation_SP(self);
                Actor_GetEyePosition(self, viewPos); // 0x00628af0, cache already updated
            }
        }
        float fovUse = fovDot;
        if ( Actor_GetTargetSentient(self) == sentient
            || (self->pFavoriteEnemy.isDefined() && self->pFavoriteEnemy.sentient() == sentient) )
            fovUse = 0.0f;
        float visibleDistSq = sentient->maxVisibleDist * sentient->maxVisibleDist;
        if ( maxDistSq - visibleDistSq >= 0.0f )
            maxDistSq = visibleDistSq;
        visible = PointInFovAndRange_SP(self, viewPos, targetPos, fovUse, maxDistSq);
        if ( visible )
            visible = Actor_SightTrace_SP(self, viewPos, targetPos, ent->s.number);
    }
    if ( !visible )
    {
        if ( self->fovDot < fovDot )
            return false;
        if ( maxDistSq < self->fMaxSightDistSqrd )
            return false;
    }
    else if ( fovDot < self->fovDot || self->fMaxSightDistSqrd < maxDistSq )
        goto visibility_result;
    if ( sentient )
        // MP resets attackTime even on a turret; SP's cache writer has a turret test.
        Actor_UpdateVisCache_SP(self, ent, info, visible);
visibility_result:
    if ( !visible )
        return false;
    if ( ent->actor && !Actor_IsUsingTurret_SP(self)
        && !Actor_IsUsingTurret_SP(ent->actor) )
    {
        float otherMaxDistSq = ent->actor->fMaxSightDistSqrd;
        float visibleDistSq = self->sentient->maxVisibleDist * self->sentient->maxVisibleDist;
        if ( otherMaxDistSq - visibleDistSq >= 0.0f )
            otherMaxDistSq = visibleDistSq;
        bool otherVisible = PointInFovAndRange_SP(ent->actor, targetPos, viewPos,
            ent->actor->fovDot, otherMaxDistSq);
        Actor_UpdateVisCache_SP(ent->actor, self->ent,
            &ent->actor->sentientInfo[self->sentient - level.sentients], otherVisible);
    }
    return true;
}

// zombies: Actor_CanSeeSentient (SP 0x0045f800)
static bool Actor_CanSeeSentient_SP(actor_s *self, sentient_s *sentient, int maxLatency)
{
    vis_cache_t *cache = &self->sentientInfo[sentient - level.sentients].VisCache;
    if ( cache->iLastUpdateTime && cache->iLastUpdateTime + maxLatency >= level.time )
        return cache->bVisible;
    return Actor_CanSeeEntityEx_SP(self, sentient->ent, self->fovDot, self->fMaxSightDistSqrd);
}

// zombies: FUN_00558c70 (SP 0x00558c70)
static void FUN_00558c70(gentity_s *ent, int count, const int *touchEnts)
{
    // MP ClientImpacts has different callback order and a pmove/glass interface.
    for ( int i = 0; i < count; ++i )
    {
        int j;
        for ( j = 0; j < i && touchEnts[j] != touchEnts[i]; ++j )
            ;
        if ( j != i )
            continue;
        gentity_s *other = &g_entities[touchEnts[i]];
        if ( Scr_IsSystemActive(1, SCRIPTINSTANCE_SERVER) ) // 0x00431a60
        {
            Scr_AddEntity(other, SCRIPTINSTANCE_SERVER); // 0x005dfc20
            Scr_Notify(ent, scr_const.touch, 1); // 0x00518010
            Scr_AddEntity(ent, SCRIPTINSTANCE_SERVER);
            Scr_Notify(other, scr_const.touch, 1);
        }
        if ( entityHandlers[ent->handler].touch )
            entityHandlers[ent->handler].touch(ent, other, 1);
        if ( entityHandlers[other->handler].touch )
            entityHandlers[other->handler].touch(other, ent, 1);
    }
}

// zombies: FUN_00627530 (SP 0x00627530)
static void FUN_00627530(actor_s *self)
{
    path_t *path = &self->Path;
    pathnode_t *node;
    float goal[3];
    if ( !Path_HasNegotiationNode(path) )
    {
        node = self->sentient->pClaimedNode;
        Vec3Copy(path->vFinalGoal, goal);
    }
    else
    {
        node = Path_GetNegotiationNode(path); // 0x00466830 -> 0x00419990
        Vec3Copy(node->constant.vOrigin, goal);
    }
    // SP actor+e54 -> Physics.iTraceMask, Path+386 -> wNegotiationStartNode.
    int mask = self->Physics.iTraceMask;
    goal[2] += 18.0f;
    int i = path->wNegotiationStartNode + 1;
    for ( ; i < path->wPathLen; ++i )
    {
        const float *point = path->pts[i].vOrigPoint;
        float dx = goal[0] - point[0];
        float dy = goal[1] - point[1];
        if ( i > path->wNegotiationStartNode + 1 && dy * dy + dx * dx > 250000.0f )
            break;
        if ( node->constant.forward[1] * dy + node->constant.forward[0] * dx >= 0.0f )
        {
            float start[3] = { point[0], point[1], point[2] + 18.0f };
            trace_t trace = {};
            col_context_t context; // 0x004b3e90, same default initializer
            G_TraceCapsule(&trace, start, vec3_origin, vec3_origin, goal,
                self->ent->s.number, mask, &context); // 0x004ee360
            if ( trace.startsolid || trace.fraction < 1.0f )
                break;
        }
    }
    if ( i == path->wNegotiationStartNode + 1 )
        return;
    float dx = goal[0] - (path->lookaheadDir[0] * path->fLookaheadDist + self->ent->r.currentOrigin[0]);
    float dy = goal[1] - (path->lookaheadDir[1] * path->fLookaheadDist + self->ent->r.currentOrigin[1]);
    float direction[3];
    if ( dy * dy + dx * dx >= 225.0f )
    {
        direction[0] = goal[0] - path->pts[i - 1].vOrigPoint[0];
        direction[1] = goal[1] - path->pts[i - 1].vOrigPoint[1];
    }
    else
    {
        direction[0] = path->lookaheadDir[0];
        direction[1] = path->lookaheadDir[1];
    }
    float length = sqrtf(direction[1] * direction[1] + direction[0] * direction[0]);
    if ( -length >= 0.0f )
        length = 1.0f;
    direction[0] *= 1.0f / length;
    direction[1] *= 1.0f / length;
    direction[2] = 0.0f;
    Scr_AddVector(direction, SCRIPTINSTANCE_SERVER); // 0x00432940
    // SP scr_const 0x023a586c is corner_approach; KB's scr_const has no member.
    unsigned int notify = SL_GetString("corner_approach", 0, SCRIPTINSTANCE_SERVER);
    Scr_Notify(self->ent, notify, 1);
    SL_RemoveRefToString(SCRIPTINSTANCE_SERVER, notify);
}

// zombies: FUN_0049b0e0 (SP 0x0049b0e0); also canshoot (x3).
bool FUN_0049b0e0(actor_s *self, const float *targetPos, const float *muzzlePos)
{
    // SP level+fFogOpaqueDistSqrd at 0x01c05334, confirmed by G_SetFogForClient.
    if ( level.fFogOpaqueDistSqrd < Vec3DistanceSq(muzzlePos, targetPos) )
        return false;
    trace_t trace = {};
    col_context_t context; // 0x004b3e90
    G_TraceCapsule(&trace, muzzlePos, vec3_origin, vec3_origin, targetPos,
        self->ent->s.number, 0x0200C861, &context); // SP callsite 0x0049b168
    if ( trace.fraction != 1.0f )
    {
        unsigned int hit = Trace_GetEntityHitId(&trace); // 0x00699730
        if ( !self->sentient->targetEnt.isDefined() || self->sentient->targetEnt.entnum() != hit )
        {
            sentient_s *sentient = g_entities[hit].sentient;
            return sentient && sentient->eTeam != self->sentient->eTeam;
        }
    }
    return true;
}

// zombies: FUN_0059baa0 (SP 0x0059baa0)
static bool FUN_0059baa0(actor_s *self)
{
    float targetPos[3], muzzlePos[3];
    // 0x0061a730 matches Actor_GetTargetLookPosition (sentient eye / entity centroid).
    gentity_s *target = Actor_GetTargetEntity(self);
    if ( target->actor && target->sentient )
        Actor_UpdateEyeInformation_SP(target->actor);
    Actor_GetTargetLookPosition(self, targetPos);
    if ( !Actor_GetMuzzleInfo(self, muzzlePos, NULL) ) // 0x004dfe10
        return false;
    return FUN_0049b0e0(self, targetPos, muzzlePos);
}

// zombies: FUN_005e9b70 (SP 0x005e9b70)
static bool FUN_005e9b70(const float *position, float buffer, const badplace_arc_t *arc)
{
    float dx = position[0] - arc->origin[0];
    float dy = position[1] - arc->origin[1];
    float distance = sqrtf(dx * dx + dy * dy);
    float divisor = distance;
    if ( -distance >= 0.0f )
        divisor = 1.0f;
    dx *= 1.0f / divisor;
    dy *= 1.0f / divisor;
    if ( (distance - buffer) * (distance - buffer) <= arc->radius * arc->radius )
    {
        if ( arc->origin[2] - arc->halfheight <= position[2]
            && position[2] <= arc->origin[2] + arc->halfheight )
        {
            float yaw = 0.0f;
            if ( dy != 0.0f || dx != 0.0f )
            {
                yaw = (float)atan2((double)dy, (double)dx) * 57.295776f; // CRT 0x00967b8a
                if ( yaw < 0.0f )
                    yaw += 360.0f;
            }
            yaw = AngleNormalize360(yaw); // 0x0047fae0
            if ( arc->angle1 <= arc->angle0 )
            {
                if ( yaw < arc->angle1 || arc->angle0 < yaw )
                    return true;
            }
            else if ( yaw < arc->angle1 && arc->angle0 < yaw )
                return true;
        }
    }
    return false;
}

// zombies: FUN_005a29f0 (SP 0x005a29f0)
static bool FUN_005a29f0(const float *position)
{
    // SP g_badplaces at 0x01a4c088; MP Actor_IsInAnyBadPlace additionally rolls
    // awareness and uses entity bounds. This SP position query does neither.
    extern badplace_t g_badplaces[256];
    for ( int i = 0; i < 256; ++i )
    {
        badplace_t *place = &g_badplaces[i];
        switch ( place->type )
        {
        case 1: case 3: case 4:
            if ( FUN_005e9b70(position, 15.0f, &place->parms.arc) )
                return true;
            break;
        case 2:
            if ( SV_EntityContact(position, position, place->parms.brush.volume) ) // 0x0059a900
                return true;
            break;
        }
    }
    return false;
}

// zombies: FUN_004ac4e0 (SP 0x004ac4e0)
static bool FUN_004ac4e0(const float *point, const actor_goal_s *goal, float buffer)
{
    // SP goal+0x18/+0x1c -> radius/height, not the MP struct's byte offsets.
    float dz = point[2] - goal->pos[2];
    return dz * dz <= goal->height * goal->height
        && Vec2DistanceSq(point, goal->pos) <= (goal->radius + buffer) * (goal->radius + buffer);
}

// zombies: FUN_00550b20 (SP 0x00550b20)
static int FUN_00550b20(actor_s *self, float (*normal)[2], float *distance)
{
    // SP +20c8/+20e0/+20f8/+2110 are record time; +8 normal, +10 dist.
    actor_sp_ext_t *ext = Actor_SP_Ext(self);
    int count = 0;
    for ( int i = 0; i < 4; ++i )
    {
        if ( ext->suppression[i].time )
        {
            normal[count][0] = ext->suppression[i].normal[0];
            normal[count][1] = ext->suppression[i].normal[1];
            distance[count++] = ext->suppression[i].dist;
        }
    }
    return count; // EAX in the exe, lost by the decompiler's void prototype.
}

// zombies: FUN_0060c1e0 (SP 0x0060c1e0)
static bool FUN_0060c1e0(actor_s *self)
{
    // SP +2134 -> suppressionStartTime (spawn writer 0x004eff50).
    if ( self->suppressionStartTime < 1 )
    {
        for ( int i = 0; i < 4; ++i )
            if ( Actor_SP_Ext(self)->suppression[i].time )
                return true;
        return false;
    }
    return true;
}

// zombies: FUN_007cf300 (SP 0x007cf300)
static bool FUN_007cf300(actor_s *self)
{
    pathnode_t *claimed = self->sentient->pClaimedNode;
    if ( !claimed )
        return false;
    // 0x00537ae0 takes a sentient, despite one export naming it Actor_NearestNode.
    pathnode_t *enemyNode = Sentient_NearestNode(Actor_GetTargetSentient(self));
    return enemyNode && Path_ExpandedNodeVisible(claimed, enemyNode); // 0x004e9120
}

// zombies: FUN_007d2e30 (SP 0x007d2e30)
static bool FUN_007d2e30(actor_s *self)
{
    if ( Actor_HasPath(self) && level.time - self->Path.iPathTime < 2000 )
        return !Path_DistanceGreaterThan(&self->Path, 60.0f); // 0x0063fe80
    return false;
}

// zombies: FUN_007bb620 (SP 0x007bb620)
static bool FUN_007bb620(sentient_s *enemy, actor_s *self)
{
    // SP +1a58 pathEnemyFightDist, +1a18 codeGoal.height.
    return Vec2DistanceSq(enemy->ent->r.currentOrigin, self->ent->r.currentOrigin)
            < self->pathEnemyFightDist * self->pathEnemyFightDist
        && fabs(enemy->ent->r.currentOrigin[2] - self->ent->r.currentOrigin[2]) <= self->codeGoal.height;
}

// zombies: FUN_007bb6a0 (SP 0x007bb6a0)
static bool FUN_007bb6a0(actor_s *self, const float *enemyPos, const float *lookaheadEnd, bool hadPath)
{
    // SP +1a54 -> pathEnemyLookahead; +1944 -> Path.lookaheadNextNode,
    // +1954/+1968/+1970 -> lookaheadDir/fLookaheadDist/fLookaheadDistToNextNode.
    float radius = self->pathEnemyLookahead;
    if ( !hadPath )
        radius += 256.0f;
    float radiusSq = radius * radius;
    if ( Vec2DistanceSq(enemyPos, self->ent->r.currentOrigin) < radiusSq )
        return true;
    float dx = lookaheadEnd[0] - enemyPos[0];
    float dy = lookaheadEnd[1] - enemyPos[1];
    float along = self->Path.lookaheadDir[0] * dx + self->Path.lookaheadDir[1] * dy;
    if ( along < self->Path.fLookaheadDist )
    {
        float d = self->Path.lookaheadDir[1] * dx - self->Path.lookaheadDir[0] * dy;
        if ( (along > 0.0f ? d * d : dy * dy + dx * dx) < radiusSq )
            return true;
    }
    int index = self->Path.lookaheadNextNode;
    const pathpoint_t *point = &self->Path.pts[index];
    dx = point->vOrigPoint[0] - enemyPos[0];
    dy = point->vOrigPoint[1] - enemyPos[1];
    along = point->fDir2D[0] * dx + point->fDir2D[1] * dy;
    if ( along < self->Path.fLookaheadDistToNextNode )
    {
        float d = point->fDir2D[1] * dx - point->fDir2D[0] * dy;
        if ( (along > 0.0f ? d * d : dy * dy + dx * dx) < radiusSq )
            return true;
    }
    float distance = self->Path.fLookaheadDistToNextNode + self->Path.fLookaheadDist;
    while ( distance < self->pathEnemyFightDist )
    {
        if ( --index < 0 )
            break;
        point = &self->Path.pts[index];
        dx = point->vOrigPoint[0] - enemyPos[0];
        dy = point->vOrigPoint[1] - enemyPos[1];
        along = point->fDir2D[0] * dx + point->fDir2D[1] * dy;
        if ( along < point->fOrigLength )
        {
            float d = point->fDir2D[1] * dx - point->fDir2D[0] * dy;
            if ( (along > 0.0f ? d * d : dy * dy + dx * dx) < radiusSq )
                return true;
        }
        distance += point->fOrigLength;
    }
    return false;
}

// zombies: FUN_007bb900 (SP 0x007bb900)
static bool FUN_007bb900(actor_s *self)
{
    // SP +19f8 -> exposedDuration.
    if ( self->eState[self->stateLevel] == AIS_EXPOSED
        && self->eSubState[self->stateLevel] == STATE_EXPOSED_COMBAT )
        return level.time < self->exposedStartTime + self->exposedDuration;
    return false;
}

// zombies: FUN_006067c0 (SP 0x006067c0)
static bool FUN_006067c0(actor_s *self)
{
    gentity_s *target = Actor_GetTargetEntity(self);
    if ( !target )
        return false;
    if ( target->sentient && level.time - self->sentientInfo[target->sentient - level.sentients]
            .lastKnownPosTime > 10000 )
        return false;
    float radius = 64.0f;
    if ( 64.0f - self->codeGoal.radius >= 0.0f )
        radius = self->codeGoal.radius;
    // DAT_01c79b64 is the HUMAN combat entry, not this species' combat entry.
    if ( self->pAnimScriptFunc == &Actor_SP_GetAnimScriptStorage(AI_SPECIES_HUMAN)->combat )
        return Actor_PointNearPoint(self->ent->r.currentOrigin, self->codeGoal.pos, radius);
    return false;
}

// zombies: FUN_00506e40 (SP 0x00506e40)
static bool FUN_00506e40(actor_s *self)
{
    AnimScriptList *anims = Actor_SP_GetAnimScriptStorage(AI_SPECIES_HUMAN);
    scr_animscript_t *script = self->pAnimScriptFunc;
    // SP also compares HUMAN cover_pillar. That missing slot cannot be installed in KB;
    // FUN_007bfa30 diagnoses an attempt to select it before storing a script pointer.
    return script == &anims->cover_left || script == &anims->cover_right
        || script == &anims->cover_stand || script == &anims->cover_crouch
        || script == &anims->cover_wide_left || script == &anims->cover_wide_right;
}

// zombies: Actor_OrientPitchToGround (SP 0x0063ced0)
void Actor_OrientPitchToGround_SP(gentity_s *ent, bool lerp)
{
    actor_prone_info_s *prone = &ent->actor->ProneInfo;
    if ( !prone->orientPitch )
        return;
    // Use KB's contents numbering for SP's 0xfdff3fff body-plant mask.
    int mask = ent->clipmask & 0xFDFF7FFF;
    if ( !lerp )
    {
        Actor_GetBodyPlantAngles(ent->s.number, mask, ent->r.currentOrigin,
            ent->r.currentAngles[1], &prone->fTorsoPitch, NULL, &prone->fBodyHeight);
        return;
    }
    float pitch, height;
    Actor_GetBodyPlantAngles(ent->s.number, mask, ent->r.currentOrigin,
        ent->r.currentAngles[1], &pitch, NULL, &height);
    float delta = AngleNormalize180(pitch - prone->fTorsoPitch);
    if ( fabs(delta) > 6.0f ) // MP lerps by 12 degrees.
        pitch = prone->fTorsoPitch + (delta < 0.0f ? -6.0f : 6.0f);
    prone->fTorsoPitch = pitch;
    if ( fabs(height - prone->fBodyHeight) > 0.6f )
        height = prone->fBodyHeight + (height - prone->fBodyHeight < 0.0f ? -0.6f : 0.6f);
    prone->fBodyHeight = height;
    if ( zombiemode->current.enabled && ent->actor->species == AI_SPECIES_ZOMBIE_DOG )
        ent->s.lerp.u.actor.proneInfo.fBodyPitch = (int)floor(pitch * 182.04445f + 0.5f);
    if ( Actor_SP_Ext(ent->actor)->allowPitchAngle )
        ent->s.lerp.u.actor.proneInfo.fBodyPitch = (int)floor(prone->fTorsoPitch * 182.04445f + 0.5f);
}

// zombies: Actor_PreThink (SP 0x0062bbf0)
void Actor_PreThink_SP(actor_s *self)
{
    if ( self->preThinkTime == level.time )
        return;
    self->preThinkTime = level.time;
    if ( self->flashBanged )
        return;
    if ( self->pFavoriteEnemy.isDefined() )
        Actor_GetPerfectInfo(self, self->pFavoriteEnemy.sentient());
    if ( self->eSubState[self->stateLevel] == STATE_EXPOSED_COMBAT && !Actor_GetTargetEntity(self) )
    {
        // SP increments DAT_01a48ca0 (SP-only statistics) before stamping threatUpdateTime.
        ++g_exposedNoTargetCount;
        self->threatUpdateTime = level.time;
    }
    Actor_UpdateSight(self); // 0x00552810
    Actor_UpdateThreat(self); // 0x00565100
    Actor_UpdateLastEnemySightPos(self); // 0x00658c50
}

// zombies: Actor_CanSeeEnemy (SP 0x0049d490)
static bool Actor_CanSeeEnemy_SP(actor_s *self)
{
    sentient_s *enemy = Actor_GetTargetSentient(self);
    if ( enemy )
        // Refresh through the SP trace/cache path, including its turret test.
        return Actor_CanSeeSentient_SP(self, enemy, 250);
    return Actor_CanSeeEntityEx_SP(self, Actor_GetTargetEntity(self), self->fovDot,
        self->fMaxSightDistSqrd) != 0; // 0x00629a20; MP calls Actor_CanSeeEntity here.
}

// zombies: FUN_005eeb10 (SP 0x005eeb10)
static bool FUN_005eeb10(actor_s *self, const gentity_s *ent)
{
    return Actor_CanSeeEntityEx_SP(self, ent, self->fovDot, self->fMaxSightDistSqrd);
}

// zombies: FUN_0061c020 (SP 0x0061c020)
static bool FUN_0061c020(actor_s *self, bool checkEnemy)
{
    sentient_s *enemy = Actor_GetTargetSentient(self);
    if ( !enemy )
        return Actor_CanSeeEntityEx_SP(self, Actor_GetTargetEntity(self), self->fovDot, self->fMaxSightDistSqrd);
    if ( checkEnemy && FUN_007cf300(self) )
        return true;
    int lastSeen = self->sentientInfo[enemy - level.sentients].VisCache.iLastVisTime;
    return lastSeen && level.time - lastSeen < 10000;
}

// zombies: FUN_007d2d80 (SP 0x007d2d80)
static void FUN_007d2d80(actor_s *self)
{
    if ( !Actor_GetTargetEntity(self) )
    {
        Actor_SetSubState(self, STATE_EXPOSED_NONCOMBAT);
        return;
    }
    ai_substate_t state = self->eSubState[self->stateLevel];
    if ( state == STATE_EXPOSED_NONCOMBAT )
    {
        if ( FUN_0061c020(self, true) )
        {
            Actor_SetSubState(self, STATE_EXPOSED_COMBAT);
            return;
        }
    }
    else if ( state == STATE_EXPOSED_REACQUIRE_MOVE )
    {
        if ( !Actor_HasPath(self) || self->pPileUpActor || self->pCloseEnt.isDefined() )
            Actor_SetSubState(self, STATE_EXPOSED_COMBAT);
        if ( Actor_CanSeeEnemy_SP(self) && FUN_0059baa0(self) )
        {
            Actor_SetSubState(self, STATE_EXPOSED_COMBAT);
            self->exposedStartTime = level.time;
        }
    }
}

// zombies: FUN_007d30d0 (SP 0x007d30d0)
static bool FUN_007d30d0(actor_s *self)
{
    if ( self->pPileUpActor || self->pCloseEnt.isDefined() )
        return true;
    // Misnamed Actor_IsInGoal in the export: ECX is Path.vFinalGoal, EDX is codeGoal.
    if ( !Actor_PointAtGoal(self->Path.vFinalGoal, &self->codeGoal) )
        return false;
    if ( self->sentient->pClaimedNode && !Actor_KeepClaimedNode(self) )
        return Actor_PointAt(self->ent->r.currentOrigin, self->sentient->pClaimedNode->constant.vOrigin);
    float buffer = self->codeGoal.radius * 0.5f;
    if ( buffer - 64.0f >= 0.0f )
        buffer = 64.0f;
    // Arguments in the exe are origin, codeGoal, -buffer.
    return FUN_004ac4e0(self->ent->r.currentOrigin, &self->codeGoal, -buffer);
}

// zombies: FUN_007c0e30 (SP 0x007c0e30)
static void FUN_007c0e30(actor_s *self)
{
    if ( !self->useEnemyGoal && self->eAnimMode != AI_ANIM_MOVE_CODE
        && self->eAnimMode != AI_ANIM_USE_ANGLE_DELTAS && self->mayMoveTime + 1000 <= level.time )
    {
        float radius = self->codeGoal.radius - 30.0f;
        if ( !Actor_KeepClaimedNode(self) && (radius <= 0.0f
            || radius * radius <= Vec2DistanceSq(self->codeGoal.pos, self->ent->r.currentOrigin)) )
        {
            self->moveMode = AI_MOVE_STOP;
            self->eAnimMode = AI_ANIM_MOVE_CODE;
        }
    }
}

// zombies: Actor_GetStopAnim (SP 0x005de4b0)
static scr_animscript_t *Actor_GetStopAnim_SP(actor_s *self)
{
    if ( self->species != AI_SPECIES_HUMAN )
        return &g_animScriptTable[self->species]->stop;
    pathnode_t *node = self->sentient->pClaimedNode;
    scr_animscript_t *script;
    if ( (!self->fixedNode || node == self->codeGoal.node) && node && Actor_IsNearClaimedNode(self)
        && Actor_GetNodeAnim_SP(self, node, false, &script) )
    {
        if ( script )
            return script;
        if ( !self->fixedNode )
            Actor_NodeClaimRevoked(self, 5000); // EDX=0x1388 at 0x005de513
    }
    return &g_animScriptTable[self->species]->stop;
}

// zombies: Actor_AnimCombat (SP 0x00426710)
static void Actor_AnimCombat_SP(actor_s *self)
{
    Actor_CheckCollisions(self);
    Actor_ClearPileUp(self);
    self->bUseGoalWeight = false;
    pathnode_t *node = self->sentient->pClaimedNode;
    if ( self->fixedNode )
    {
        if ( node != self->codeGoal.node )
            node = NULL;
        if ( self->fixedNode && level.time <= self->exposedStartTime + 2000 )
            goto set_script;
    }
    if ( node && self->species == AI_SPECIES_HUMAN && Actor_IsNearClaimedNode(self) )
    {
        scr_animscript_t *script;
        if ( Actor_GetNodeAnim_SP(self, node, false, &script) )
        {
            if ( script )
            {
                Actor_SetAnimScript(self, script, AI_MOVE_STOP, AI_ANIM_MOVE_CODE, AI_ANIM_FUNCTION_STOP);
                return;
            }
            if ( !self->fixedNode )
                Actor_NodeClaimRevoked(self, 5000);
        }
    }
    if ( self->fixedNode && self->pAnimScriptFunc != &g_animScriptTable[self->species]->combat )
        self->exposedStartTime = level.time;
set_script:
    Actor_SetAnimScript(self, &g_animScriptTable[self->species]->combat, AI_MOVE_STOP,
        self->pCloseEnt.isDefined() ? AI_ANIM_MOVE_CODE : AI_ANIM_USE_BOTH_DELTAS, AI_ANIM_FUNCTION_COMBAT);
}

// zombies: FUN_00492930 (SP 0x00492930)
static bool FUN_00492930(actor_s *self)
{
    if ( self->arrivalInfo.animscriptOverrideRunTo )
        return false;
    // FUN_004bdfd0 = Path_HasNegotiationNode; FUN_0068c780 = exact three-float equality.
    if ( Path_HasNegotiationNode(&self->Path) )
        return true;
    pathnode_t *node = self->sentient->pClaimedNode;
    return node && ((1u << (node->constant.type & 31)) & 0x83ffc)
        && Actor_HasPath(self) && Vec3Compare(self->Path.vFinalGoal, node->constant.vOrigin);
}

// zombies: Actor_MoveAlongPathWithTeam (SP 0x00464590)
void Actor_MoveAlongPathWithTeam_SP(actor_s *self, bool run, bool useInterval, bool goalPileUp)
{
    bool wasMoving;
    ai_teammove_t result;
    if ( self->eAnimMode == AI_ANIM_USE_BOTH_DELTAS_ZONLY_PHYSICS )
    {
        wasMoving = false;
        result = AI_TEAMMOVE_TRAVEL;
    }
    else
    {
        wasMoving = Actor_IsMoving(self);
        if ( !wasMoving )
        {
            self->moveMode = AI_MOVE_STOP;
            self->eAnimMode = AI_ANIM_MOVE_CODE;
            // SP reads eState[stateLevel+1], matching KB (SP 0x004645c4).
            if ( self->stateLevel < 6 && self->eState[self->stateLevel + 1] != AIS_NEGOTIATION )
                self->prevMoveDir[0] = self->prevMoveDir[1] = 0.0f;
        }
        result = Actor_GetTeamMoveStatus(self, useInterval, goalPileUp);
        G_SP_PlayTraceTeamMove(self, (int)result);
    }
    if ( result == AI_TEAMMOVE_TRAVEL )
    {
        if ( run )
        {
            Actor_AnimTryRun(self);
            goto moved;
        }
    }
    else
    {
        if ( result == AI_TEAMMOVE_WAIT )
        {
            Actor_AnimStop(self, &g_animScriptTable[self->species]->stop);
            self->arrivalInfo.animscriptOverrideRunTo = 0;
            return;
        }
        if ( result != AI_TEAMMOVE_SLOW_DOWN )
            return;
    }
    Actor_AnimTryWalk(self);
moved:
    if ( (self->goalPosChanged && wasMoving) || self->arrivalInfo.arrivalNotifyRequested )
    {
        if ( FUN_00492930(self) )
            FUN_00627530(self);
        self->arrivalInfo.arrivalNotifyRequested = 0;
    }
    if ( !wasMoving && Actor_IsMoving(self) )
    {
        Actor_ClearMoveHistory(self);
        self->yawVeloc = 0.0f;
        self->ent->flags &= 0xE7FFFFFF; // SP entity flags 8/16 are KB 0x08000000/0x10000000.
        self->Path.iPathEndTime = 0;
    }
}

// zombies: FUN_007c0ef0 (SP 0x007c0ef0)
static void FUN_007c0ef0(actor_s *self)
{
    Actor_AnimCombat_SP(self);
    sentient_s *enemy = Actor_GetTargetSentient(self);
    if ( Actor_IsAtGoal(self) && enemy )
    {
        FUN_007c0e30(self);
        int lastSeen = self->sentientInfo[enemy - level.sentients].VisCache.iLastVisTime;
        pathnode_t *node = self->sentient->pClaimedNode;
        if ( (!lastSeen || level.time - lastSeen >= 10000) && node && (node->constant.spawnflags & 0x8000) )
        {
            scr_animscript_t *script;
            if ( !Actor_GetNodeAnim_SP(self, node, true, &script) || script )
            {
                Actor_SetOrientMode(self, AI_ORIENT_TO_GOAL);
                return;
            }
        }
    }
    Actor_SetOrientMode(self, AI_ORIENT_TO_ENEMY);
}

// zombies: FUN_007d2e70 (SP 0x007d2e70)
static void FUN_007d2e70(actor_s *self)
{
    pathnode_t *node = self->sentient->pClaimedNode;
    Actor_UpdateGoalPath_SP(self);
    if ( Actor_HasPath(self) )
    {
        self->pszDebugInfo = "noncombat_move";
        // The C export loses the float comparison passed as argument 2 (0x007d2e91..2f18).
        float dx = self->ent->r.currentOrigin[0] - self->Path.vFinalGoal[0];
        float dy = self->ent->r.currentOrigin[1] - self->Path.vFinalGoal[1];
        float dz = self->ent->r.currentOrigin[2] - self->Path.vFinalGoal[2];
        bool run = dz * dz + dx * dx + dy * dy >= self->fWalkDist * self->fWalkDist;
        Actor_MoveAlongPathWithTeam_SP(self, run, true, true);
        ai_orient_mode_t mode = (ai_orient_mode_t)ai_moveOrientMode->current.integer;
        if ( mode == AI_ORIENT_INVALID )
        {
            if ( node && (node->constant.spawnflags & 0x8000)
                && FUN_007d2e30(self) && !self->arrivalInfo.animscriptOverrideRunTo )
                mode = AI_ORIENT_TO_GOAL;
            else
                mode = AI_ORIENT_TO_MOTION;
        }
        if ( Actor_IsAtGoal(self) )
        {
            if ( node && (node->constant.spawnflags & 0x8000) )
            {
                Actor_SetOrientMode(self, mode);
                return;
            }
            Actor_SetOrientMode(self, AI_ORIENT_TO_ENEMY_OR_MOTION);
            return;
        }
        Actor_SetOrientMode(self, mode);
        return;
    }
    self->pszDebugInfo = "noncombat_stop";
    if ( !Actor_IsAtGoal(self) )
    {
        if ( !Actor_GetTargetEntity(self) )
        {
            Actor_AnimStop(self, Actor_GetStopAnim_SP(self));
            Actor_SetOrientMode(self, AI_ORIENT_TO_ENEMY);
            return;
        }
        Actor_AnimCombat_SP(self);
        Actor_SetOrientMode(self, AI_ORIENT_TO_ENEMY);
        return;
    }
    if ( node && (node->constant.spawnflags & 0x8000) )
    {
        Actor_AnimStop(self, Actor_GetStopAnim_SP(self));
        Actor_SetOrientMode(self, AI_ORIENT_TO_GOAL);
        return;
    }
    FUN_007c0e30(self);
    if ( !Actor_GetTargetEntity(self) )
        Actor_AnimStop(self, Actor_GetStopAnim_SP(self));
    else
        Actor_AnimCombat_SP(self);
    if ( self->codeGoal.ang[1] != 0.0f )
    {
        self->bNotifyTurnDone = 1;
        self->ScriptOrient.eMode = AI_ORIENT_DONT_CHANGE;
        Actor_SetDesiredAngles(&self->ScriptOrient, 0.0f, self->codeGoal.ang[1]);
        self->codeGoal.ang[1] = 0.0f;
        self->scriptGoal.ang[1] = 0.0f;
        return;
    }
    Actor_SetOrientMode(self, AI_ORIENT_TO_ENEMY);
}

// zombies: Actor_PhysicsMove / Actor_DoMove (SP 0x00575220)
static void Actor_DoMove_SP(actor_s *self)
{
    unsigned short oldGround = self->Physics.groundEntNum;
    // mod (L50d): a large horde can push a code-walking zombie off an exterior platform while
    // it is still approaching its window. The brief said through the floor; the traces show
    // an unsupported ledge (Kino near -430,-1903), not a missed floor collision.
    // SP ground checks use a 0.25-unit lift / 18-unit step (0x007C7D20), and failed crowd
    // moves restore PhysicsInputs and origin (0x007BC6C0). Apply that rollback to unsupported
    // crowd steps too. Scripted traverses, falling actors, deaths and retail-sized games retain SP movement.
    const bool guardLedge = zombiemode->current.enabled && Dvar_GetInt("horde_max_zombies") > 24
        && self->Physics.bIsAlive && self->ent->health > 0 && self->eAnimMode == AI_ANIM_MOVE_CODE
        && oldGround != ENTITYNUM_NONE
        && (self->Physics.ePhysicsType == AIPHYS_NORMAL_ABSOLUTE || self->Physics.ePhysicsType == AIPHYS_NORMAL_RELATIVE);
    PhysicsInputs ledgeInputs;
    if ( guardLedge )
        Actor_PhysicsBackupInputs(self, &ledgeInputs);
    if ( self->Physics.ePhysicsType != AIPHYS_NORMAL_ABSOLUTE )
    {
        float forward[3], right[3], wish[3];
        YawVectors(self->fDesiredBodyYaw, forward, right);
        Vec3Copy(self->Physics.vWishDelta, wish);
        Vec3Scale(forward, wish[0], self->Physics.vWishDelta);
        self->Physics.vWishDelta[0] += right[0] * -wish[1];
        self->Physics.vWishDelta[1] += right[1] * -wish[1];
        self->Physics.vWishDelta[2] += right[2] * -wish[1];
        self->Physics.vWishDelta[2] += wish[2];
    }
    // SP entity+0x160 is script_noteworthy (SP field table 0x00a55668: model+154, classname+15c, script_linkname+15e,
    // script_noteworthy+160, target+162, targetname+164); zombie_moon_gravity.gsc sets script_noteworthy "moon_gravity".
    // DAT_023a5884 is "moon_gravity", DAT_0247fde8 is zombietron.
    if ( !zombiemode->current.enabled || zombietron->current.enabled || !self->ent->script_noteworthy
        || strcmp(SL_ConvertToString(self->ent->script_noteworthy, SCRIPTINSTANCE_SERVER), "moon_gravity") )
        self->Physics.fGravity = bg_gravity->current.value;
    else
        self->Physics.fGravity = bg_moonGravity->current.value;

    // zombies: mod field noactorcollision - drop a close actor this actor must not steer around (or that must not
    // steer around it), so it keeps walking its path through the crowd. Never set by retail scripts.
    if ( self->pCloseEnt.isDefined() && self->pCloseEnt.ent()->actor
        && ( self->noActorCollision || self->pCloseEnt.ent()->actor->noActorCollision ) )
        self->pCloseEnt.setEnt(0);

    int success;
    if ( self->eAnimMode != AI_ANIM_MOVE_CODE || !self->moveMode
        || !Actor_HasPath(self) || self->pCloseEnt.isDefined() )
    {
        if ( !self->pCloseEnt.isDefined() || self->Physics.ePhysicsType == AIPHYS_NOCLIP || !self->pushable )
        {
            self->ent->flags &= 0xE7FFFFFF;
            Vec3Copy(self->ent->r.currentOrigin, self->Physics.vOrigin);
            success = Actor_Physics(&self->Physics);
            if ( success )
                goto moved;
        }
        else
        {
            success = Actor_PhysicsMoveAway(self);
            if ( success )
                goto moved;
            if ( self->eAnimMode == AI_ANIM_MOVE_CODE && Actor_HasPath(self) )
                goto dodge;
        }
        {
            pathsort_t nodes[64];
            int count = Path_NodesInCylinder(self->ent->r.currentOrigin, 384.0f, 128.0f, nodes, 64, -1);
            float bestDist = FLT_MAX;
            pathnode_t *closest = NULL;
            // The export unrolls this by four; retain its <= tie order.
            for ( int i = 0; i < count; ++i )
            {
                float delta[3];
                Vec3Sub(nodes[i].node->constant.vOrigin, self->ent->r.currentOrigin, delta);
                float dist = delta[2] * delta[2] + delta[1] * delta[1] + delta[0] * delta[0];
                if ( dist <= bestDist )
                {
                    closest = nodes[i].node;
                    bestDist = dist;
                }
            }
            if ( closest )
            {
                float height = closest->constant.vOrigin[2] - self->ent->r.currentOrigin[2];
                if ( height <= 8.0f )
                {
                    if ( height < 0.0f )
                        height = height >= -18.0f ? 0.0f : -8.0f;
                }
                else
                    height = 8.0f;
                Vec3Copy(self->ent->r.currentOrigin, self->Physics.vOrigin);
                self->Physics.vOrigin[2] += height;
                self->Physics.vVelocity[2] = 0.0f;
            }
        }
        goto moved;
    }
dodge:
    success = Actor_PhysicsAndDodge(self);
    if ( success )
    {
        if ( self->Path.lookaheadDir[2] <= 4.0f || self->Physics.vWishDelta[2] <= 0.0f
            || self->ent->r.currentOrigin[2] < self->Physics.vOrigin[2] )
        {
            if ( self->sentient->bNearestNodeBad )
            {
                Sentient_InvalidateNearestNode(self->sentient);
                Sentient_NearestNode(self->sentient);
                if ( self->sentient->bNearestNodeBad )
                    success = 0;
            }
        }
        else
            success = 0;
    }
    // MP does this on every collision backend. SP restricts it to zombie legacy collision.
    if ( phys_ai_collision_mode->current.integer == 0 && zombiemode->current.enabled && !success )
    {
        Vec3Add(self->Physics.vWishDelta, self->ent->r.currentOrigin, self->Physics.vOrigin);
        self->Physics.vVelocity[2] = 0.0f;
        self->Path.wDodgeEntity = ENTITYNUM_NONE;
        if ( self->Path.fLookaheadAmount < 64.0f )
            self->Path.fLookaheadAmount = 64.0f;
    }
moved:
    if ( guardLedge && (self->Physics.vOrigin[0] != self->ent->r.currentOrigin[0]
        || self->Physics.vOrigin[1] != self->ent->r.currentOrigin[1]) )
    {
        const float step = self->Physics.prone ? 10.0f : 18.0f;
        float start[3] = { self->Physics.vOrigin[0], self->Physics.vOrigin[1], self->ent->r.currentOrigin[2] + step };
        float end[3] = { start[0], start[1], self->Physics.vOrigin[2] - step };
        trace_t support;
        col_context_t context;
        // A neighbouring zombie is not a floor. Use a fresh trace, not the movement's cached contact plane.
        G_TraceCapsule(&support, start, self->Physics.vMins, self->Physics.vMaxs, end,
            self->ent->s.number, self->Physics.iTraceMask & ~0x02000000, &context);
        if ( support.fraction == 1.0f && !support.startsolid )
        {
            static int logged;
            if ( Dvar_GetBool("bo1_falltrace") && logged++ < 32 )
                Com_Printf(18, "BO1_LEDGE time %i ent %i from %.1f %.1f %.1f rejected %.1f %.1f %.1f\n",
                    level.time, self->ent->s.number, self->ent->r.currentOrigin[0], self->ent->r.currentOrigin[1], self->ent->r.currentOrigin[2],
                    self->Physics.vOrigin[0], self->Physics.vOrigin[1], self->Physics.vOrigin[2]);
            Actor_PhysicsRestoreInputs(self, &ledgeInputs);
            Vec3Copy(self->ent->r.currentOrigin, self->Physics.vOrigin);
        }
    }
    Vec3Copy(self->Physics.vOrigin, self->ent->r.currentOrigin);
    self->Physics.ePhysicsType = AIPHYS_BAD;
    self->ent->s.groundEntityNum = self->Physics.groundEntNum;
    if ( oldGround != self->Physics.groundEntNum )
        Scr_Notify(self->ent, scr_const.groundEntChanged, 0);
}

// zombies: Actor_Move / Actor_UpdateOriginAndAngles (SP 0x00535680)
void Actor_UpdateOriginAndAngles_SP(actor_s *self)
{
    gentity_s *ent = self->ent;
    if ( self->eAnimMode == AI_ANIM_NOPHYSICS )
        return;
    if ( ent->tagInfo )
    {
        G_SetFixedLink(ent, Actor_SP_Ext(self)->linkYawOnly != 0);
        Actor_ClearPath(self);
        Actor_UpdateAnglesAndDelta(self);
        // SP +df8/+dfc/+e00 and +e18/+e1c/+e20 are Physics velocity and wish delta.
        Vec3Clear(self->Physics.vVelocity);
        Vec3Clear(self->Physics.vWishDelta);
        G_CalcTagAxis(ent, 1);
        // SP entity+48/+24 are apos.trDelta/pos.trDelta. Linked snapshots carry
        // the relative transform in the trajectory delta fields, not the current pose.
        AxisToAngles(ent->tagInfo->axis, ent->s.lerp.apos.trDelta);
        Vec3Copy(ent->tagInfo->axis[3], ent->s.lerp.pos.trDelta);
        return;
    }
    Actor_UpdateAnglesAndDelta(self);
    Actor_DoMove_SP(self);
    if ( level.gentities[self->Physics.iHitEntnum].sentient && !self->noDodgeMove )
        Actor_TeamMoveBlocked(self);
    // SP passes ent, Physics.iNumTouch (+e5c), Physics.iTouchEnts (+e60).
    // SP dispatches script touch notifications and both entity callbacks.
    FUN_00558c70(ent, self->Physics.iNumTouch, self->Physics.iTouchEnts);
}

// zombies: Actor_PostThink / Actor_MoveAndTouch (SP 0x0065a350)
void Actor_PostThink_SP(actor_s *self)
{
    Actor_UpdateOriginAndAngles_SP(self);
    // Unlike KB Actor_PostThink, SP does not call BG_Dog_UpdateAnimationState here.
    if ( self->eAnimMode != AI_ANIM_MOVE_CODE )
    {
        if ( ai_showPaths->current.integer )
            Path_DebugDraw(&self->Path, self->ent->r.currentOrigin, 1, self->ent->s.number);
        if ( Actor_HasPath(self) )
            Path_UpdateLookahead_NonCodeMove(&self->Path, self->sentient->oldOrigin, self->ent->r.currentOrigin);
    }
    Actor_CheckNodeClaim(self);
    Actor_CheckClearNodeClaimCloseEnt(self);
    G_SP_MeasureActorTrace(self); // a1 c15 TEST (bo1_measure_actortrace_from / _to)
}

// zombies: Actor_Zombie_Exposed_Think (SP 0x005591f0)
actor_think_result_t __fastcall Actor_Zombie_Exposed_Think(actor_s *self)
{
    self->pszDebugInfo = "exposed";
    if ( Actor_SP_Ext(self)->allowPitchAngle )
        Actor_OrientPitchToGround_SP(self->ent, true);
    Actor_PreThink_SP(self);
    FUN_007d2d80(self);
    ai_substate_t state = self->eSubState[self->stateLevel];
    if ( state == STATE_EXPOSED_COMBAT )
    {
        self->pszDebugInfo = "exposed_combat";
        Actor_UpdateGoalPath_SP(self);
        if ( Actor_HasPath(self) && Actor_CanSeeEnemy_SP(self) && FUN_007d30d0(self) )
        {
            G_SP_PlayTracePathClear(self, self->pCloseEnt.isDefined() ? 1 : self->pPileUpActor ? 2 : 3);
            Actor_ClearPath(self);
        }
        if ( Actor_HasPath(self) && self->safeToChangeScript )
        {
            Actor_SetOrientMode(self, AI_ORIENT_TO_ENEMY_OR_MOTION);
            Actor_MoveAlongPathWithTeam_SP(self, true, true, true);
            Actor_PostThink_SP(self);
            return ACTOR_THINK_DONE;
        }
        FUN_007c0ef0(self);
    }
    else
    {
        if ( state == STATE_EXPOSED_NONCOMBAT )
        {
            FUN_007d2e70(self);
            Actor_PostThink_SP(self);
            return ACTOR_THINK_DONE;
        }
        if ( state == STATE_EXPOSED_REACQUIRE_MOVE )
        {
            self->pszDebugInfo = "exposed_reacquire_move";
            Actor_SetOrientMode(self, AI_ORIENT_TO_ENEMY_OR_MOTION_SIDESTEP);
            Actor_MoveAlongPathWithTeam_SP(self, true, false, true);
            Actor_PostThink_SP(self);
            return ACTOR_THINK_DONE;
        }
    }
    Actor_PostThink_SP(self);
    return ACTOR_THINK_DONE;
}

// zombies: FUN_007c0fb0 (SP 0x007c0fb0, register arg esi = actor): the human exposed substate update,
// the counterpart of the zombie's FUN_007d2d80 with the flashbanged substate. SP actor+0x2238 is flashBanged.
static void FUN_007c0fb0(actor_s *self)
{
    if ( self->flashBanged )
    {
        Actor_SetSubState(self, STATE_EXPOSED_FLASHBANGED);
        return;
    }
    if ( !Actor_GetTargetEntity(self) ) // SP 0x00635a60
    {
        Actor_SetSubState(self, STATE_EXPOSED_NONCOMBAT);
        return;
    }
    switch ( self->eSubState[self->stateLevel] )
    {
    case STATE_EXPOSED_NONCOMBAT:
        break;
    case STATE_EXPOSED_REACQUIRE_MOVE:
        if ( !Actor_HasPath(self) || self->pPileUpActor || self->pCloseEnt.isDefined() )
            Actor_SetSubState(self, STATE_EXPOSED_COMBAT);
        if ( Actor_CanSeeEnemy_SP(self) && FUN_0059baa0(self) )
        {
            Actor_SetSubState(self, STATE_EXPOSED_COMBAT);
            self->exposedStartTime = level.time;
        }
        return;
    case STATE_EXPOSED_FLASHBANGED:
        if ( !self->flashBanged )
            Actor_SetSubState(self, STATE_EXPOSED_NONCOMBAT);
        break;
    default:
        return;
    }
    if ( FUN_0061c020(self, true) )
        Actor_SetSubState(self, STATE_EXPOSED_COMBAT);
}

// zombies: the human species' exposed think (SP 0x004eec10, AIFuncTable species 0 row 1). Before the
// zombie think's body it leaves exposed for a live grenade (actor+0x215c pGrenade), a bad place
// (+0x21b8 aiBadPlace) or a turret nobody is on (+0x21a8 turret, entity+0x158 active), each unless
// flashbanged (+0x2238) where SP tests it; then the human substate update and no safeToChangeScript
// test before moving along the path.
actor_think_result_t __fastcall Actor_Human_Exposed_Think(actor_s *self)
{
    self->pszDebugInfo = "exposed";
    if ( Actor_SP_Ext(self)->allowPitchAngle )
        Actor_OrientPitchToGround_SP(self->ent, true);
    if ( self->pGrenade.isDefined() && !self->flashBanged )
    {
        Actor_SetState(self, AIS_GRENADE_RESPONSE); // SP 0x004dceb0
        return ACTOR_THINK_REPEAT;
    }
    if ( self->aiBadPlace && !self->flashBanged )
    {
        Actor_SetState(self, AIS_BADPLACE_FLEE);
        return ACTOR_THINK_REPEAT;
    }
    if ( Actor_SP_Ext(self)->turret )
    {
        if ( !Actor_SP_Ext(self)->turret->active )
        {
            Actor_SetState(self, AIS_TURRET);
            return ACTOR_THINK_REPEAT;
        }
        Actor_SP_Ext(self)->turret = NULL;
    }
    Actor_PreThink_SP(self);
    FUN_007c0fb0(self);
    switch ( self->eSubState[self->stateLevel] )
    {
    case STATE_EXPOSED_COMBAT:
        self->pszDebugInfo = "exposed_combat";
        Actor_UpdateGoalPath_SP(self);
        if ( Actor_HasPath(self) && Actor_CanSeeEnemy_SP(self) && FUN_007d30d0(self) )
        {
            G_SP_PlayTracePathClear(self, self->pCloseEnt.isDefined() ? 1 : self->pPileUpActor ? 2 : 3);
            Actor_ClearPath(self);
        }
        if ( Actor_HasPath(self) )
        {
            Actor_SetOrientMode(self, AI_ORIENT_TO_ENEMY_OR_MOTION);
            Actor_MoveAlongPathWithTeam_SP(self, true, true, true);
            Actor_PostThink_SP(self);
            return ACTOR_THINK_DONE;
        }
        FUN_007c0ef0(self);
        break;
    case STATE_EXPOSED_NONCOMBAT:
        FUN_007d2e70(self);
        break;
    case STATE_EXPOSED_REACQUIRE_MOVE:
        self->pszDebugInfo = "exposed_reacquire_move";
        Actor_SetOrientMode(self, AI_ORIENT_TO_ENEMY_OR_MOTION_SIDESTEP);
        Actor_MoveAlongPathWithTeam_SP(self, true, false, true);
        break;
    case STATE_EXPOSED_FLASHBANGED:
        self->pszDebugInfo = "exposed_flashbanged";
        Actor_Exposed_FlashBanged(self); // SP 0x0058c140
        break;
    default:
        break;
    }
    Actor_PostThink_SP(self);
    return ACTOR_THINK_DONE;
}

// zombies: FUN_007bfa30 (SP 0x007bfa30)
static bool FUN_007bfa30(actor_s *self, pathnode_t *node, scr_animscript_t **script)
{
    // SP addresses 0x01c79b64 onward are the HUMAN anim list, not the actor's species list.
    AnimScriptList *anims = Actor_SP_GetAnimScriptStorage(AI_SPECIES_HUMAN);
    switch ( node->constant.type )
    {
    case 2: *script = &anims->cover_stand; return true;
    case 3: case 4: case 19: *script = &anims->cover_crouch; return true;
    case 5: *script = &anims->cover_prone; return true;
    case 6: *script = &anims->cover_right; return true;
    case 7: *script = &anims->cover_left; return true;
    case 8: *script = &anims->cover_wide_right; return true;
    case 9: *script = &anims->cover_wide_left; return true;
    case 10:
        // SP cover_pillar is inserted after cover_left; KB AnimScriptList has no slot.
        Com_Error(ERR_DROP, "zombie exposed: SP human cover_pillar animscript storage/load not ported");
        return false;
    case 11: *script = &anims->concealment_stand; return true;
    case 12: *script = &anims->concealment_crouch; return true;
    case 13: *script = &anims->concealment_prone; return true;
    case 20:
        if ( (node->constant.spawnflags & 0x80) && (!Actor_GetTargetEntity(self) || !self->hasThreateningEnemy) )
        {
            *script = &anims->stop;
            return true;
        }
        *script = &anims->combat;
        return true;
    default:
        *script = NULL;
        return (node->constant.spawnflags & 0x8000) != 0;
    }
}

struct NodeEngagementRecord
{
    float minDistanceSq;
    float minYaw, maxYaw;
    float additionalBounds[4]; // SP slots 3..6; this call tree reads only slots 0..2.
};

// zombies: FUN_004334d0 (SP 0x004334d0)
static void FUN_004334d0(const pathnode_t *node, float *minimum, float *maximum)
{
    // SP node+60 -> dynamic.turretEntNumber; entity+14c -> pTurretInfo;
    // turret+38/+40 -> arcmin[1]/arcmax[1].
    if ( node->dynamic.turretEntNumber < 0 )
    {
        *minimum = -45.0f;
        *maximum = 45.0f;
        return;
    }
    const TurretInfo *turret = g_entities[node->dynamic.turretEntNumber].pTurretInfo;
    *minimum = AngleNormalize180(turret->arcmin[1]);
    *maximum = AngleNormalize180(turret->arcmax[1]);
}

// zombies: FUN_004d8ae0 (SP 0x004d8ae0)
static void FUN_004d8ae0(NodeEngagementRecord *record, const pathnode_t *node)
{
    switch ( node->constant.type )
    {
    case 2: case 3: case 10: case 11: case 12:
        record->minYaw = -45.0f; record->maxYaw = 45.0f;
        record->additionalBounds[2] = -35.0f; record->additionalBounds[3] = 35.0f;
        record->additionalBounds[0] = -25.0f; record->additionalBounds[1] = 25.0f;
        record->minDistanceSq = 81225.0f;
        return;
    case 4:
        record->minYaw = -30.0f; record->maxYaw = 10.0f;
        record->additionalBounds[2] = -20.0f; record->additionalBounds[3] = 5.0f;
        record->additionalBounds[0] = -15.0f; record->additionalBounds[1] = 1.0f;
        record->minDistanceSq = 81225.0f;
        return;
    case 5: case 13:
        record->minYaw = -30.0f; record->maxYaw = 30.0f;
        record->additionalBounds[2] = -20.0f; record->additionalBounds[3] = 20.0f;
        record->additionalBounds[0] = -15.0f; record->additionalBounds[1] = 15.0f;
        record->minDistanceSq = node->constant.type == 5 ? 640000.0f : 81225.0f;
        return;
    case 6: case 8:
        record->minYaw = -60.0f; record->maxYaw = 14.0f;
        record->additionalBounds[2] = -35.0f; record->additionalBounds[3] = 5.0f;
        record->additionalBounds[0] = -10.0f; record->additionalBounds[1] = -1.0f;
        record->minDistanceSq = 81225.0f;
        return;
    case 7: case 9:
        record->minYaw = -12.0f; record->maxYaw = 60.0f;
        record->additionalBounds[2] = -5.0f; record->additionalBounds[3] = 35.0f;
        record->additionalBounds[0] = 1.0f; record->additionalBounds[1] = 10.0f;
        record->minDistanceSq = 81225.0f;
        return;
    case 19:
        FUN_004334d0(node, &record->minYaw, &record->maxYaw);
        record->additionalBounds[2] = record->minYaw;
        record->additionalBounds[0] = record->minYaw;
        record->additionalBounds[3] = record->maxYaw;
        record->additionalBounds[1] = record->maxYaw;
        record->minDistanceSq = 81225.0f;
        return;
    default:
        record->minYaw = -55.0f; record->maxYaw = 55.0f;
        record->additionalBounds[2] = -35.0f; record->additionalBounds[3] = 35.0f;
        record->additionalBounds[0] = -25.0f; record->additionalBounds[1] = 25.0f;
        record->minDistanceSq = 0.0f;
        return;
    }
}

// zombies: FUN_004dd0b0 (SP 0x004dd0b0)
static bool FUN_004dd0b0(const float *position, const pathnode_t *node, const NodeEngagementRecord *record)
{
    // SP node+14/+18 -> vOrigin[0/1], +20 -> fAngle. 0x00967b8a is CRT atan2.
    float yaw = (float)atan2((double)(position[1] - node->constant.vOrigin[1]),
        (double)(position[0] - node->constant.vOrigin[0])) * 57.295776f;
    yaw = AngleNormalize180(yaw - node->constant.fAngle);
    return record->minYaw <= yaw && yaw <= record->maxYaw;
}

// zombies: FUN_00565c70 (SP 0x00565c70)
static bool FUN_00565c70(const float *position, const pathnode_t *node, const NodeEngagementRecord *record)
{
    if ( Vec2DistanceSq(node->constant.vOrigin, position) < record->minDistanceSq )
        return false;
    return FUN_004dd0b0(position, node, record);
}

// zombies: PredictBounceMissile (SP 0x007e68f0)
static void PredictBounceMissile_SP(gentity_s *ent, trajectory_t *pos, trace_t *trace,
    int time, int velocityTime, const float *origin, float *endpos)
{
    const WeaponDef *weapon = BG_GetWeaponDef(ent->s.weapon); // SP 0x00425770
    unsigned int surface = (trace->sflags >> 20) & 0x3f;
    float velocity[3];
    BG_EvaluateTrajectoryDelta(pos, velocityTime, velocity); // SP 0x00635050
    float dot = trace->normal.vec.v[1] * velocity[1] + trace->normal.vec.v[2] * velocity[2]
        + trace->normal.vec.v[0] * velocity[0];
    float reflection = dot * -2.0f;
    for ( int i = 0; i < 3; ++i )
        pos->trDelta[i] = trace->normal.vec.v[i] * reflection + velocity[i];
    if ( ent->s.lerp.eFlags & 0x1000000 )
    {
        float speed = sqrtf(velocity[1] * velocity[1] + velocity[2] * velocity[2] + velocity[0] * velocity[0]);
        if ( speed > 0.0f && dot <= 0.0f )
        {
            // SP WeaponDef+650/+654 -> parallelBounce/perpendicularBounce arrays.
            float parallel = weapon->parallelBounce[surface];
            float factor = (weapon->perpendicularBounce[surface] - parallel) * (-1.0f / speed) * dot + parallel;
            for ( int i = 0; i < 3; ++i )
                pos->trDelta[i] *= factor;
        }
        if ( weapon->stickiness == WEAPSTICKINESS_ALL || weapon->stickiness == WEAPSTICKINESS_ALL_NO_SENTIENTS
            || (trace->normal.vec.v[2] > 0.7f && (weapon->stickiness == WEAPSTICKINESS_GROUND
                || weapon->stickiness == WEAPSTICKINESS_GROUND_WITH_YAW || Vec3Length(pos->trDelta) < 20.0f)) )
        {
            Vec3Copy(endpos, pos->trBase);
            pos->trType = TR_STATIONARY;
            pos->trTime = 0;
            pos->trDuration = 0;
            Vec3Clear(pos->trDelta);
            // SP notifies stationary (scr_const slot 0x023a5606); MP omits this.
            Scr_AddVector(trace->normal.vec.v, SCRIPTINSTANCE_SERVER); // SP 0x00432940
            Scr_AddVector(endpos, SCRIPTINSTANCE_SERVER);
            Scr_Notify(ent, scr_const.stationary, 2);
            return;
        }
    }
    float offsetZ = trace->normal.vec.v[2] * 0.1f;
    if ( offsetZ > 0.0f )
        offsetZ = 0.0f;
    pos->trBase[0] = origin[0] + trace->normal.vec.v[0] * 0.1f;
    pos->trBase[1] = origin[1] + trace->normal.vec.v[1] * 0.1f;
    pos->trBase[2] = origin[2] + offsetZ;
    pos->trTime = time;
}

// zombies: G_PredictMissile (SP 0x00524d10)
static int G_PredictMissile_SP(gentity_s *ent, int duration, float *landPos, int allowBounce, int *timeAtRest)
{
    // SP entity+0c -> s.lerp.pos; +134 -> ownerNum; +174/+180 -> clipmask/nextthink.
    // Use KB sizes for the trajectory and temporary entity copy (SP entity stride is 0x34c).
    trajectory_t pos = ent->s.lerp.pos;
    trace_t trace = {};
    float org[3], origin[3], endpos[3], traceStart[3];
    BG_EvaluateTrajectory(&pos, level.time - 50, org); // SP 0x00621e00
    gentity_s backup = *ent;
    *timeAtRest = ent->nextthink;
    const WeaponDef *weapon = BG_GetWeaponDef(ent->s.weapon);
    Vec3Clear(landPos);
    int time;
    for ( time = level.time; time < level.time + duration; time += 50 )
    {
        BG_EvaluateTrajectory(&pos, time, origin);
        int passEntityNum = ent->r.ownerNum.isDefined() ? ent->r.ownerNum.entnum() : ENTITYNUM_NONE;
        G_MissileTrace(&trace, org, origin, passEntityNum, ent->clipmask, ent->s.weapon); // SP 0x005e50e0
        if ( !trace.startsolid )
        {
            if ( (trace.sflags & 0x3f00000) == 0x900000 )
                // The predicted=true branch matches SP 0x007e47a0, including contents save/restore.
                Missile_PenetrateGlass(&trace, ent, org, origin, weapon->damage, true);
            Vec3Lerp(org, origin, trace.fraction, org);
            Vec3Copy(org, endpos);
            if ( weapon->stickiness == WEAPSTICKINESS_ALL || weapon->stickiness == WEAPSTICKINESS_ALL_NO_SENTIENTS )
            {
                if ( trace.fraction < 1.0f )
                {
                    for ( int i = 0; i < 3; ++i )
                    {
                        traceStart[i] = trace.normal.vec.v[i] * 0.13500001f + org[i];
                        origin[i] = org[i] - trace.normal.vec.v[i] * 1.5f;
                    }
                    passEntityNum = ent->r.ownerNum.isDefined() ? ent->r.ownerNum.entnum() : ENTITYNUM_NONE;
                    G_MissileTrace(&trace, traceStart, origin, passEntityNum, ent->clipmask, ent->s.weapon);
                    Vec3Lerp(traceStart, origin, trace.fraction, endpos);
                    if ( trace.fraction == 1.0f )
                        continue;
                    if ( Trace_GetEntityHitId(&trace) == ENTITYNUM_WORLD ) // SP 0x00699730
                    {
                        for ( int i = 0; i < 3; ++i )
                            org[i] = (endpos[i] - origin[i]) + endpos[i];
                    }
                }
            }
            else if ( trace.fraction == 1.0f || (trace.fraction < 1.0f && trace.normal.vec.v[2] > 0.7f) )
            {
                Vec3Copy(org, traceStart);
                Vec3Copy(org, origin);
                traceStart[2] += 0.13500001f;
                origin[2] -= 1.5f;
                passEntityNum = ent->r.ownerNum.isDefined() ? ent->r.ownerNum.entnum() : ENTITYNUM_NONE;
                G_MissileTrace(&trace, traceStart, origin, passEntityNum, ent->clipmask, ent->s.weapon);
                Vec3Lerp(traceStart, origin, trace.fraction, endpos);
                if ( trace.fraction == 1.0f )
                    continue;
                float correction = (endpos[2] + 1.5f) - org[2];
                org[2] = endpos[2] + 1.5f;
                pos.trBase[2] += correction;
                org[0] = endpos[0];
                org[1] = endpos[1];
            }
            if ( trace.fraction != 1.0f )
            {
                if ( trace.sflags & 4 )
                    goto prediction_failed;
                if ( allowBounce && (ent->s.lerp.eFlags & 0x1000000) )
                {
                    PredictBounceMissile_SP(ent, &pos, &trace, time,
                        time - (int)(trace.fraction * -50.0f) - 50, org, endpos);
                    pos.trTime = time;
                    if ( pos.trType != TR_STATIONARY )
                        continue;
                }
                *timeAtRest = time;
                break;
            }
        }
        else
        {
            if ( time != level.time )
                goto prediction_failed;
            Vec3Copy(origin, org);
        }
    }
    Vec3Copy(org, landPos);
    *ent = backup;
    if ( allowBounce && (ent->s.lerp.eFlags & 0x1000000) )
        return ent->nextthink;
    return time;
prediction_failed:
    *ent = backup;
    return 0;
}

static struct
{
    actor_grenade_landing_sp_t landing;
    int birthTime;
    int useCount;
} g_grenadeLandingCache[MAX_GENTITIES_SV];

void Actor_SP_ResetGrenadeLandingCaches()
{
    memset(g_grenadeLandingCache, 0, sizeof(g_grenadeLandingCache));
}

actor_grenade_landing_sp_t *Actor_SP_GrenadeLandingCache(const gentity_s *grenade)
{
    auto *slot = &g_grenadeLandingCache[grenade->s.number];
    if ( slot->birthTime != grenade->birthTime || slot->useCount != grenade->useCount )
    {
        memset(&slot->landing, 0, sizeof(slot->landing));
        slot->birthTime = grenade->birthTime;
        slot->useCount = grenade->useCount;
    }
    return &slot->landing;
}

// zombies: FUN_007c21d0 / grenade predicted-landing writer (SP 0x007c21d0)
void Actor_SP_PredictGrenadeLanding(gentity_s *grenade)
{
    actor_grenade_landing_sp_t *cache = Actor_SP_GrenadeLandingCache(grenade);
    if ( cache->position[0] == 0.0f && cache->position[1] == 0.0f && cache->position[2] == 0.0f )
    {
        int clipmask = grenade->clipmask;
        // SP clears 0x0200c000. Map SP actor contents 0x4000 to KB's 0x8000
        // (P6a); keep the other cleared bits, yielding 0xfdff7fff.
        grenade->clipmask &= 0xfdff7fff;
        float landPos[3];
        int time;
        int result = G_PredictMissile_SP(grenade, grenade->nextthink - level.time, landPos, 1, &time);
        cache->time = time; // SP entity+220
        cache->populated = true;
        if ( !result )
        {
            Vec3Copy(grenade->r.currentOrigin, cache->position); // SP +214/+218/+21c
            grenade->clipmask = clipmask;
            return;
        }
        Vec3Copy(landPos, cache->position);
        grenade->clipmask = clipmask;
    }
}

// zombies: CanDamage (SP 0x005394d0)
static bool CanDamage_SP(gentity_s *target, const float *targetOrigin, gentity_s *inflictor,
    const float *center, float radius, float coneCos, const float *coneDirection,
    float height, bool useEyeMidpoint, int contentMask)
{
    // SP entity+100 -> r.contents, +124 -> currentOrigin.z, +13c/+144 -> client/sentient.
    float width = 15.0f;
    float delta[3];
    Vec3Sub(center, targetOrigin, delta);
    if ( !(target->r.contents & 0x405c0008)
        && radius * radius <= delta[0] * delta[0] + delta[2] * delta[2] + delta[1] * delta[1] )
        return false;
    float originZ = target->r.currentOrigin[2];
    col_context_t context(contentMask); // SP 0x0059bf50
    context.init_locational(target->s.number, inflictor ? inflictor->s.number : ENTITYNUM_NONE); // SP 0x004af580
    int hitNum = -1;
    float dest[5][3];
    if ( !target->sentient )
    {
        float maxs[3];
        if ( target->classname == scr_const.script_model && target->model )
        {
            float mins[3];
            DObjPhysicsGetBounds(Com_GetServerDObj(target->s.number), mins, maxs); // SP 0x0047e340 / 0x005f9d10
            // SP 0x005b12a0 is G_EntityCentroid with explicit model bounds. Reuse
            // KB's rotation/translation on a local bounds view, without changing the entity.
            gentity_s boundsView = *target;
            Vec3Copy(mins, boundsView.r.mins);
            Vec3Copy(maxs, boundsView.r.maxs);
            G_EntityCentroid(&boundsView, dest[0]);
            Vec3Add(target->r.currentOrigin, maxs, maxs);
        }
        else
        {
            G_EntityCentroid(target, dest[0]);
            Vec3Copy(target->r.absmax, maxs);
        }
        float forward[3], right[3], up[3];
        Vec3Sub(center, dest[0], forward);
        Vec3Normalize(forward);
        right[0] = -forward[1];
        right[1] = forward[0];
        right[2] = 0.0f;
        Vec3Normalize(right);
        Vec3Cross(forward, right, up);
        float corner[3];
        Vec3Sub(maxs, dest[0], corner);
        float radiusRight = fabsf(corner[0] * right[0]) + fabsf(corner[1] * right[1]);
        float radiusUp = fabsf(up[1] * corner[1]) + fabsf(up[0] * corner[0]) + fabsf(corner[2] * up[2]);
        Vec3Scale(right, radiusRight, right);
        Vec3Scale(up, radiusUp, up);
        // SP 0x00539d24..0x00539ddf keeps center-right in registers: the C
        // export's reassignment of local_94/local_90 does not overwrite dest[0].
        for ( int i = 0; i < 3; ++i )
        {
            dest[1][i] = (dest[0][i] + right[i]) + up[i];
            dest[2][i] = (dest[0][i] - right[i]) + up[i];
            dest[3][i] = (dest[0][i] + right[i]) - up[i];
            dest[4][i] = (dest[0][i] - right[i]) - up[i];
        }
    }
    else
    {
        if ( target->client )
            width = 8.0f;
        float x = center[0] - targetOrigin[0];
        float y = center[1] - targetOrigin[1];
        float length = sqrtf(x * x + y * y);
        if ( -length >= 0.0f )
            length = 1.0f;
        float invLength = 1.0f / length;
        float right[3] = { -(invLength * y), invLength * x, invLength * 0.0f };
        float eye[3];
        if ( height == 0.0f )
        {
            if ( target->actor )
                Actor_UpdateEyeInformation_SP(target->actor);
            Sentient_GetEyePosition(target->sentient, eye);
            height = eye[2] - originZ;
        }
        height *= 0.5f;
        if ( !useEyeMidpoint )
        {
            Vec3Copy(targetOrigin, dest[0]);
            dest[0][2] += height;
        }
        else
        {
            for ( int i = 0; i < 3; ++i )
                dest[0][i] = (eye[i] + targetOrigin[i]) * 0.5f;
        }
        for ( int i = 0; i < 3; ++i )
        {
            dest[1][i] = right[i] * width + dest[0][i];
            dest[2][i] = dest[1][i];
            dest[3][i] = -width * right[i] + dest[0][i];
            dest[4][i] = dest[3][i];
        }
        dest[1][2] += height;
        dest[2][2] -= height;
        dest[3][2] += height;
        dest[4][2] -= height;
    }
    if ( radius_damage_debug->current.enabled )
    {
        for ( int i = 0; i < 5; ++i )
        {
            const float *color = colorWhite;
            if ( coneCos != -1.0f && coneDirection )
            {
                Vec3Sub(dest[i], center, delta);
                Vec3Normalize(delta);
                if ( coneCos > Vec3Dot(coneDirection, delta) )
                {
                    CG_DebugLine(center, dest[i], colorOrange, 1, 200); // SP thunk 0x004c9af0
                    continue;
                }
            }
            if ( !SV_SightTracePoint(&hitNum, center, dest[i], &context) )
                color = colorRed;
            CG_DebugLine(center, dest[i], color, 1, 200);
        }
    }
    for ( int i = 0; i < 5; ++i )
    {
        if ( coneCos != -1.0f && coneDirection )
        {
            Vec3Sub(dest[i], center, delta);
            Vec3Normalize(delta);
            if ( coneCos > Vec3Dot(coneDirection, delta) )
                continue;
        }
        if ( SV_SightTracePoint(&hitNum, center, dest[i], &context) )
            return true;
    }
    return false;
}

// zombies: FUN_0041ea90 (SP 0x0041ea90)
static bool FUN_0041ea90(actor_s *self, const float *origin)
{
    gentity_s *grenade = self->pGrenade.ent();
    const WeaponDef *weapon = BG_GetWeaponDef(grenade->s.weapon);
    if ( weapon->projExplosion == 2 ) // SP WeaponDef+5ec -> projExplosion (smoke)
        return true;
    return !CanDamage_SP(self->ent, origin, grenade, Actor_SP_GrenadeLandingCache(grenade)->position,
        (float)weapon->iExplosionRadius * 1.1f, 1.0f, NULL, 0.0f, true, 0x802013);
}

// zombies: FUN_004b0e10 (SP 0x004b0e10)
static bool FUN_004b0e10(actor_s *self)
{
    if ( self->grenadeAwareness == 0.0f ) // SP actor+2150
        return false;
    if ( Actor_GetTargetEntity(self) && self->grenadeAwareness * 32767.0f < (float)G_rand() ) // SP 0x005a3b20
        return false;
    return true;
}

// zombies: FUN_006938c0 (SP 0x006938c0)
static team_t FUN_006938c0(team_t team)
{
    static const team_t enemyTeams[6] = { TEAM_FREE, TEAM_ALLIES, TEAM_AXIS, TEAM_FREE, TEAM_FREE, TEAM_FREE };
    return enemyTeams[team];
}

// zombies: FUN_007c24d0 (SP 0x007c24d0)
static bool FUN_007c24d0(actor_s *self, gentity_s *grenade)
{
    if ( !Actor_HasPath(self) || Actor_PointAtGoal(self->ent->r.currentOrigin, &self->scriptGoal) )
        return false;
    const float *landing = Actor_SP_GrenadeLandingCache(grenade)->position;
    float x = landing[0] - self->ent->r.currentOrigin[0];
    float y = landing[1] - self->ent->r.currentOrigin[1];
    float length = sqrtf(x * x + y * y);
    if ( -length >= 0.0f )
        length = 1.0f;
    float invLength = 1.0f / length;
    float dot = self->Path.lookaheadDir[1] * invLength * y + self->Path.lookaheadDir[0] * invLength * x;
    if ( dot < 0.0f )
        return true;
    // SP entity+17c -> parent, not r.ownerNum; parent retains the original thrower.
    if ( grenade->parent.isDefined() && grenade->parent.ent()->sentient
        && grenade->parent.ent()->sentient->eTeam != FUN_006938c0(self->sentient->eTeam) )
        return false;
    if ( dot > 0.7f )
        return Vec3DistanceSq(landing, self->ent->r.currentOrigin) > 100.0f; // SP 0x005da2a0 returns SQUARED distance
    return false;
}

// zombies: Actor_EventGrenadePing (SP 0x00441310)
void Actor_EventGrenadePing_SP(actor_s *self, gentity_s *grenade)
{
    if ( self->flashBanged )
        return;
    if ( self->eState[self->stateLevel] == AIS_GRENADE_RESPONSE && self->eSubState[self->stateLevel] == STATE_GRENADE_THROWBACK )
        return;
    if ( self->pGrenade.isDefined() )
    {
        const float *position = Actor_SP_GrenadeLandingCache(self->pGrenade.ent())->position;
        if ( position[0] != 0.0f || position[1] != 0.0f || position[2] != 0.0f )
            return;
    }
    // SP WeaponDef+571 -> bNoPing (weapon parse row offset 0x655 includes the 0xe4 full-def prefix).
    if ( BG_GetWeaponDef(grenade->s.weapon)->bNoPing || !FUN_004b0e10(self) )
        return;
    self->pGrenade.setEnt(grenade); // SP 0x005f3c20
    if ( grenade->parent.isDefined() && grenade->parent.ent()->sentient
        && grenade->parent.ent()->sentient->eTeam == FUN_006938c0(self->sentient->eTeam) )
    {
        Scr_AddEntity(grenade, SCRIPTINSTANCE_SERVER);
        Scr_Notify(self->ent, scr_const.grenadedanger, 1); // SP slot 0x023a55fa = "grenade danger"
    }
    Actor_SP_PredictGrenadeLanding(grenade);
    if ( !FUN_007c24d0(self, grenade) )
    {
        float origin[3];
        Sentient_GetOrigin(self->sentient, origin);
        if ( !FUN_0041ea90(self, origin) )
            return;
    }
    self->pGrenade.setEnt(NULL);
}

// zombies: FUN_004b03e0 (SP 0x004b03e0)
static bool FUN_004b03e0(const path_t *path, const float *origin, const float *point, float radiusSq)
{
    // SP path+398/+39c/+3ac/+3b4/+388 -> lookaheadDir/fLookaheadDist/
    // fLookaheadDistToNextNode/lookaheadNextNode; pathpoint stride 0x1c -> pts[].
    float length = path->fLookaheadDist;
    float x = point[0] - (path->lookaheadDir[0] * length + origin[0]);
    float y = point[1] - (path->lookaheadDir[1] * length + origin[1]);
    float cross = y * path->lookaheadDir[0] - path->lookaheadDir[1] * x;
    if ( cross * cross < radiusSq )
    {
        float dot = path->lookaheadDir[1] * y + path->lookaheadDir[0] * x;
        if ( dot < 0.0f )
        {
            if ( x * x + y * y < radiusSq )
                return true;
        }
        else
        {
            if ( dot <= length )
                return true;
            y = origin[1] - point[1];
            x = origin[0] - point[0];
            if ( y * y + x * x < radiusSq )
                return true;
        }
    }
    length = path->fLookaheadDistToNextNode;
    if ( length != 0.0f )
    {
        const pathpoint_t *pt = &path->pts[path->lookaheadNextNode];
        y = point[1] - pt->vOrigPoint[1];
        x = point[0] - pt->vOrigPoint[0];
        cross = y * pt->fDir2D[0] - pt->fDir2D[1] * x;
        if ( cross * cross < radiusSq )
        {
            float dot = pt->fDir2D[1] * y + pt->fDir2D[0] * x;
            if ( dot >= 0.0f )
            {
                if ( dot <= length )
                    return true;
                x = pt->fDir2D[0] * length + x;
                y = pt->fDir2D[1] * length + y;
                if ( x * x + y * y < radiusSq )
                    return true;
            }
            else if ( x * x + y * y < radiusSq )
                return true;
        }
    }
    for ( int i = path->lookaheadNextNode - 1; i >= 0; --i )
    {
        const pathpoint_t *pt = &path->pts[i];
        y = point[1] - pt->vOrigPoint[1];
        x = point[0] - pt->vOrigPoint[0];
        cross = pt->fDir2D[0] * y - pt->fDir2D[1] * x;
        if ( cross * cross < radiusSq )
        {
            float dot = pt->fDir2D[0] * x + pt->fDir2D[1] * y;
            if ( dot >= 0.0f )
            {
                if ( dot <= pt->fOrigLength )
                    return true;
                x = pt->fDir2D[0] * pt->fOrigLength + x;
                y = pt->fDir2D[1] * pt->fOrigLength + y;
            }
            if ( x * x + y * y < radiusSq )
                return true;
        }
    }
    return false;
}

// zombies: FUN_004179f0 (SP 0x004179f0)
static bool FUN_004179f0(actor_s *self, pathnode_t *node, bool useSuppression, bool ignoreBadPlaces)
{
    if ( !Path_Exists(&self->Path) ) // SP 0x00453550
    {
        if ( Actor_PointAt(self->ent->r.currentOrigin, node->constant.vOrigin) ) // SP 2-unit XY tolerance
            return true;
    }
    else
    {
        if ( !Path_NeedsReevaluation(&self->Path) && Actor_PointAt(self->Path.vFinalGoal, node->constant.vOrigin) )
            return true;
        Actor_ClearPath(self); // SP 0x00485d60 / 0x005a03e0 / actor+19c4 = 0
    }
    Sentient_InvalidateNearestNode(self->sentient); // SP 0x004e6ad0
    float normal[4][2], distance[4];
    int count = 0;
    pathnode_t *from;
    if ( !useSuppression )
        from = Sentient_NearestNode(self->sentient);
    else
    {
        count = FUN_00550b20(self, normal, distance);
        from = Sentient_NearestNodeSuppressed(self->sentient, normal, distance, count);
    }
    if ( from )
    {
        pathsort_t nodes[64];
        int nodeCount;
        if ( node->constant.spawnflags & 1 )
        {
            node = Path_NearestNode(node->constant.vOrigin, nodes, -2, 192.0f, &nodeCount, 64, NEAREST_NODE_DO_HEIGHT_CHECK);
            if ( !node )
                return false;
        }
        if ( count )
            Path_FindPathFromToNotCrossPlanes(&self->Path, self->sentient->eTeam, from,
                self->ent->r.currentOrigin, node, node->constant.vOrigin, normal, distance, count, 1, ignoreBadPlaces);
        else
            Path_FindPathFromTo(&self->Path, self->sentient->eTeam, from,
                self->ent->r.currentOrigin, node, node->constant.vOrigin, 1, ignoreBadPlaces);
        return Path_Exists(&self->Path);
    }
    return false;
}

// zombies: FUN_005f3580 (SP 0x005f3580)
static bool FUN_005f3580(actor_s *self, pathnode_t *node, bool useSuppression)
{
    if ( self->badPlaceAwareness == 0.0f )
        return FUN_004179f0(self, node, useSuppression, true);
    return FUN_004179f0(self, node, useSuppression, false);
}

struct flee_search_sp_t
{
    float origin[3], radius, radiusSq, startDistSq, bestDistSq;
    pathnode_t *bestNode;
    int planeCount;
    float (*normal)[2];
    float *distance;
};

// zombies: Path_AStarAlgorithm flee variants (SP 0x007c65b0)
static bool FUN_007c65b0(path_t *path, team_t team, const float *start, pathnode_t *from,
    flee_search_sp_t *search, int allowNegotiation, bool withPlanes)
{
    // 0x007c6880 has the same open-list search with a plane filter before the
    // negotiation test. SP node+64..78 -> transient searchFrame/next/prev/parent/cost/heuristic.
    Vec3Clear(g_pathAttemptGoalPos);
    pathnode_t top;
    from->transient.iSearchFrame = ++level.iSearchFrame;
    from->transient.pParent = &top;
    from->transient.pNextOpen = NULL;
    from->transient.pPrevOpen = &top;
    from->transient.fCost = 0.0f;
    top.transient.pNextOpen = from;
    do
    {
        pathnode_t *current = top.transient.pNextOpen;
        float delta[3];
        Vec3Sub(current->constant.vOrigin, search->origin, delta);
        float distSq = withPlanes ? delta[1] * delta[1] + delta[0] * delta[0] + delta[2] * delta[2]
            : delta[0] * delta[0] + delta[1] * delta[1] + delta[2] * delta[2];
        if ( search->radiusSq <= distSq )
        {
            if ( !path )
                return true;
            return Path_GeneratePath(path, team, start, NULL, from, current, 0, allowNegotiation) != 0;
        }
        if ( search->bestDistSq < distSq )
        {
            search->bestDistSq = distSq;
            search->bestNode = current;
        }
        top.transient.pNextOpen = current->transient.pNextOpen;
        if ( top.transient.pNextOpen )
            top.transient.pNextOpen->transient.pPrevOpen = &top;
        for ( int i = 0; i < current->dynamic.wLinkCount; ++i )
        {
            const pathlink_s *link = &current->constant.Links[i];
            if ( link->ubBadPlaceCount[team] ) // both native callers pass ignoreBadPlaces=0
                continue;
            pathnode_t *next = Path_ConvertIndexToNode(link->nodeNum);
            if ( withPlanes )
            {
                int plane = 0;
                for ( ; plane < search->planeCount; ++plane )
                {
                    if ( search->distance[plane] < search->normal[plane][1] * next->constant.vOrigin[1]
                        + next->constant.vOrigin[0] * search->normal[plane][0] )
                        break;
                }
                if ( plane != search->planeCount )
                    continue;
            }
            if ( current->constant.type == NODE_NEGOTIATION_BEGIN && next->constant.type == NODE_NEGOTIATION_END
                && (current->dynamic.wOverlapCount || next->dynamic.wOverlapCount) )
                continue;
            float cost;
            if ( next->transient.iSearchFrame == level.iSearchFrame )
            {
                cost = link->fDist + current->transient.fCost;
                if ( next->transient.fCost <= cost )
                    continue;
                if ( next->transient.pPrevOpen )
                {
                    next->transient.pPrevOpen->transient.pNextOpen = next->transient.pNextOpen;
                    if ( next->transient.pNextOpen )
                        next->transient.pNextOpen->transient.pPrevOpen = next->transient.pPrevOpen;
                }
            }
            else
            {
                next->transient.iSearchFrame = level.iSearchFrame;
                Vec3Sub(next->constant.vOrigin, search->origin, delta);
                next->transient.fHeuristic = search->radius - sqrtf(delta[0] * delta[0] + delta[2] * delta[2] + delta[1] * delta[1]);
                cost = link->fDist + current->transient.fCost;
            }
            next->transient.pParent = current;
            next->transient.fCost = cost;
            pathnode_t *insert = &top;
            while ( insert->transient.pNextOpen && insert->transient.pNextOpen->transient.fHeuristic
                + insert->transient.pNextOpen->transient.fCost < next->transient.fHeuristic + cost )
                insert = insert->transient.pNextOpen;
            next->transient.pPrevOpen = insert;
            next->transient.pNextOpen = insert->transient.pNextOpen;
            insert->transient.pNextOpen = next;
            if ( next->transient.pNextOpen )
                next->transient.pNextOpen->transient.pPrevOpen = next;
        }
        current->transient.pPrevOpen = NULL;
        if ( !top.transient.pNextOpen )
            return false;
    } while ( true );
}

// zombies: Path_AStarAlgorithm flee not crossing planes (SP 0x007c6880)
static bool FUN_007c6880(path_t *path, team_t team, const float *start, pathnode_t *from,
    flee_search_sp_t *search, int allowNegotiation)
{
    return FUN_007c65b0(path, team, start, from, search, allowNegotiation, true);
}

// zombies: FUN_00568a70 (SP 0x00568a70)
static bool FUN_00568a70(path_t *path, team_t team, pathnode_t *from, const float *start,
    const float *danger, float radius, float (*normal)[2], float *distance, int count, int allowNegotiation)
{
    flee_search_sp_t search = {};
    Vec3Copy(danger, search.origin);
    search.radius = radius;
    search.radiusSq = radius * radius;
    search.bestDistSq = -1.0f;
    search.planeCount = count;
    search.normal = normal;
    search.distance = distance;
    for ( int i = 0; i < count; ++i )
        if ( distance[i] < normal[i][0] * from->constant.vOrigin[0] + normal[i][1] * from->constant.vOrigin[1] )
            return false;
    if ( FUN_007c6880(path, team, start, from, &search, allowNegotiation) )
        return true;
    // C export loses bestNode; SP 0x00568b60 compares it with from, then tests visibility.
    if ( search.bestNode != from && !Path_NodesVisible(search.bestNode, from) )
    {
        if ( !path )
            return true;
        return Path_GeneratePath(path, team, start, NULL, from, search.bestNode, 0, allowNegotiation) != 0;
    }
    return false;
}

// zombies: FUN_0064e3e0 (SP 0x0064e3e0)
static bool FUN_0064e3e0(path_t *path, team_t team, pathnode_t *from, const float *start,
    const float *danger, float radius, int allowNegotiation)
{
    flee_search_sp_t search = {};
    Vec3Copy(danger, search.origin);
    search.radius = radius;
    search.radiusSq = radius * radius;
    search.startDistSq = Vec3DistanceSq(danger, start);
    search.bestDistSq = -1.0f;
    if ( !FUN_007c65b0(path, team, start, from, &search, allowNegotiation, false) )
    {
        if ( search.bestNode == from )
            return false;
        if ( path )
            return Path_GeneratePath(path, team, start, NULL, from, search.bestNode, 0, allowNegotiation) != 0;
    }
    return true;
}

// zombies: FUN_00501a30 (SP 0x00501a30)
static void FUN_00501a30(path_t *path, team_t team, const float *start, const float *danger,
    float radius, float (*normal)[2], float *distance, int count, int allowNegotiation)
{
    pathsort_t nodes[64];
    int nodeCount;
    pathnode_t *from = Path_NearestNodeNotCrossPlanes(start, nodes, -2, 192.0f,
        normal, distance, count, &nodeCount, 64, NEAREST_NODE_DO_HEIGHT_CHECK);
    if ( !from )
        return;
    FUN_00568a70(path, team, from, start, danger, radius, normal, distance, count, allowNegotiation);
}

// zombies: FUN_0053ec00 (SP 0x0053ec00)
static void FUN_0053ec00(path_t *path, team_t team, const float *start, const float *danger,
    float radius, int allowNegotiation)
{
    pathsort_t nodes[64];
    int nodeCount;
    pathnode_t *from = Path_NearestNode(start, nodes, -2, 192.0f, &nodeCount, 64, NEAREST_NODE_DO_HEIGHT_CHECK);
    if ( !from )
        return;
    FUN_0064e3e0(path, team, from, start, danger, radius, allowNegotiation);
}

// zombies: FUN_00599e50 (SP 0x00599e50)
static void FUN_00599e50(actor_s *self, const float *danger, float radius,
    const float *direction, float planeDistance, bool useSuppression, int allowNegotiation)
{
    Actor_ClearPath(self);
    float normal[5][2], distance[5];
    int count = useSuppression ? FUN_00550b20(self, normal, distance) : 0;
    if ( planeDistance != 0.0f )
    {
        distance[count] = planeDistance;
        normal[count][0] = direction[0];
        normal[count][1] = direction[1];
        ++count;
    }
    FUN_00501a30(&self->Path, self->sentient->eTeam, self->ent->r.currentOrigin,
        danger, radius, normal, distance, count, allowNegotiation);
}

// zombies: FUN_0049ce70 (SP 0x0049ce70)
static bool FUN_0049ce70(actor_s *self)
{
    return self->suppressionStartTime > 0; // SP actor+2134
}

// zombies: FUN_00523360 (SP 0x00523360)
static void FUN_00523360(actor_s *self, const float *danger, float radius, int allowNegotiation)
{
    Actor_ClearPath(self);
    FUN_0053ec00(&self->Path, self->sentient->eTeam, self->ent->r.currentOrigin, danger, radius, allowNegotiation);
}

// zombies: FUN_00479f60 (SP 0x00479f60)
static void FUN_00479f60(const float *normal, float distance, const float *origin,
    const float *color, float size, int depthTest, int duration)
{
    float x = normal[0], y = normal[1];
    float length = sqrtf(x * x + y * y);
    float safeLength = -length >= 0.0f ? 1.0f : length;
    float offset = (distance - (origin[0] * x + origin[1] * y)) / length;
    float half = size * 0.5f;
    y *= 1.0f / safeLength;
    x *= 1.0f / safeLength;
    float center[3] = { x * offset + origin[0], y * offset + origin[1], origin[2] + half };
    float end[3] = { -offset * x + center[0], -offset * y + center[1], center[2] };
    CG_DebugLine(center, end, color, depthTest, duration); // SP 0x004c9af0
    float left[3] = { half * -y + center[0], half * x + center[1], origin[2] };
    float right[3] = { size * -0.5f * -y + center[0], size * -0.5f * x + center[1], origin[2] };
    CG_DebugLine(left, right, color, depthTest, duration);
    left[2] += size;
    right[2] += size;
    CG_DebugLine(left, right, color, depthTest, duration);
    Vec3Copy(left, end);
    end[2] -= size;
    CG_DebugLine(left, end, color, depthTest, duration);
    Vec3Copy(right, end);
    end[2] -= size;
    CG_DebugLine(right, end, color, depthTest, duration);
}

// zombies: FUN_007c22d0 (SP 0x007c22d0)
static float FUN_007c22d0(actor_s *self, float *direction)
{
    float distance = 0.0f; // XMM0 return omitted by C export.
    gentity_s *target = Actor_GetTargetEntity(self);
    if ( !target )
        return distance;
    direction[0] = target->r.currentOrigin[0] - self->ent->r.currentOrigin[0];
    direction[1] = target->r.currentOrigin[1] - self->ent->r.currentOrigin[1];
    const float *position = Actor_SP_GrenadeLandingCache(self->pGrenade.ent())->position;
    float distSq = direction[0] * direction[0] + direction[1] * direction[1];
    if ( distSq < direction[0] * (target->r.currentOrigin[0] - position[0])
        + direction[1] * (target->r.currentOrigin[1] - position[1]) )
        position = self->ent->r.currentOrigin;
    float dot = position[1] * direction[1] + position[0] * direction[0];
    distance = sqrtf(distSq) * 15.0f + dot;
    if ( distance < dot )
    {
        direction[0] = -direction[0];
        direction[1] = -direction[1];
        distance = -distance;
    }
    if ( (ai_debugEntIndex->current.integer == self->ent->s.number && ai_showSuppression->current.integer > 0)
        || (ai_debugEntIndex->current.integer != self->ent->s.number && ai_showSuppression->current.integer == self->ent->s.number) )
        FUN_00479f60(direction, distance, self->ent->r.currentOrigin, colorOrange, 100.0f, 0, 100);
    return distance;
}

// zombies: FUN_007c2430 (SP 0x007c2430)
static void FUN_007c2430(const float *target, float *goal, actor_s *self)
{
    const float *landing = Actor_SP_GrenadeLandingCache(self->pGrenade.ent())->position;
    float x = landing[0] - target[0], y = landing[1] - target[1];
    float length = sqrtf(x * x + y * y);
    if ( -length >= 0.0f )
        length = 1.0f;
    goal[0] = (1.0f / length) * x * 29.5f + landing[0];
    goal[1] = (1.0f / length) * y * 29.5f + landing[1];
    goal[2] = landing[2];
}

// zombies: FUN_007c0040 (SP 0x007c0040)
static float FUN_007c0040(actor_s *self, pathnode_t *node)
{
    // The C export loses all score arithmetic; XMM0 sequence is 0x007c0044..19a.
    float delta[3];
    Vec3Sub(self->ent->r.currentOrigin, node->constant.vOrigin, delta);
    float distSq = delta[0] * delta[0] + delta[1] * delta[1] + delta[2] * delta[2];
    float clamped = 1440000.0f;
    if ( distSq - 1440000.0f < 0.0f )
        clamped = distSq;
    if ( -distSq >= 0.0f )
        clamped = 0.0f;
    float distance = (1.0f - clamped * 6.9444445e-7f) * ai_coverScore_distance->current.value;
    float visible = 1.0f;
    sentient_s *enemy = Actor_GetTargetSentient(self);
    if ( enemy )
    {
        pathnode_t *enemyNode = Sentient_NearestNode(enemy);
        if ( enemyNode && !Path_NodesVisible(node, enemyNode) )
            visible = 0.0f;
    }
    float score = ai_coverScore_visibility->current.value * visible + distance;
    float coverType = ((1 << node->constant.type) & 0x3800) ? 0.0f : 1.0f;
    score = ai_coverScore_coverType->current.value * coverType + score;
    score = FUN_007bfe20(self, node) * ai_coverScore_nodeAngle->current.value + score;
    float priority = (node->constant.spawnflags & 0x40) ? 1.0f : 0.0f;
    return ai_coverScore_priority->current.value * priority + score;
}

// zombies: FUN_004a26c0 (SP 0x004a26c0)
static pathnode_t *FUN_004a26c0(actor_s *self, const float *danger, float radius)
{
    Actor_HasPath(self);
    pathsort_t nodes[256];
    int count = Path_NodesInCylinder(self->ent->r.currentOrigin, 512.0f, 80.0f, nodes, 256, 0x83ffc);
    float bestScore = 0.0f;
    pathnode_t *best = NULL;
    if ( count < 1 )
        return NULL;
    for ( int i = 0; i < count; ++i )
    {
        pathnode_t *node = nodes[i].node;
        float x = danger[0] - node->constant.vOrigin[0], y = danger[1] - node->constant.vOrigin[1];
        if ( !(radius * radius < y * y + x * x) )
        {
            float height = ((1 << node->constant.type) & 0x2020) || (node->constant.spawnflags & 8) ? 20.0f : 36.0f;
            if ( CanDamage_SP(self->ent, node->constant.vOrigin, NULL, danger, radius, 1.0f, NULL, height, false, 0x802013) )
                continue;
        }
        if ( FUN_005ae040(self, node) )
        {
            float score = FUN_007c0040(self, node);
            if ( bestScore < score )
            {
                best = node;
                bestScore = score;
            }
        }
    }
    return best;
}

// zombies: G_DObjGetWorldTagPosCheck (SP 0x00648690)
static void FUN_00648690(gentity_s *ent, unsigned int tag, float *position)
{
    if ( !G_DObjGetWorldTagPos(ent, tag, position) )
    {
        DObj *obj = Com_GetServerDObj(ent->s.number);
        if ( obj && obj->numModels && DObjGetName(obj) )
        {
            Com_Error(ERR_DROP, "Missing tag [%s] on entity [%d] (%s)\n",
                SL_ConvertToString(tag, SCRIPTINSTANCE_SERVER), ent->s.number, DObjGetName(obj));
            return;
        }
        Com_Error(ERR_DROP, "Missing tag [%s] on entity [%d]\n",
            SL_ConvertToString(tag, SCRIPTINSTANCE_SERVER), ent->s.number);
    }
}

// zombies: FUN_004187b0 (SP 0x004187b0)
void FUN_004187b0(actor_s *self)
{
    gentity_s *grenade = self->pGrenade.ent();
    const WeaponDef *weapon = BG_GetWeaponDef(grenade->s.weapon);
    const char *modelName = XModelGetName(weapon->worldModel[grenade->s.weaponModel]); // SP +30c; 0x005f4600
    G_EntDetach(self->ent, modelName, scr_const.tag_inhand); // SP 0x00644d30
    float position[3];
    FUN_00648690(self->ent, scr_const.tag_inhand, position); // SP 0x00648690
    Vec3Copy(position, grenade->r.currentOrigin);
    Vec3Copy(position, grenade->s.lerp.pos.trBase);
    Vec3Clear(grenade->s.lerp.pos.trDelta);
    actor_grenade_landing_sp_t *cache = Actor_SP_GrenadeLandingCache(grenade);
    cache->throwbackOwner = NULL;
    grenade->r.svFlags &= ~1u; // SP entity+da -> r.svFlags, SVF_NOCLIENT.
    Vec3Clear(cache->position);
}

// zombies: G_InitGrenadeEntity actor branch (SP 0x00613620)
void G_InitActorGrenadeEntity_SP(gentity_s *parent, gentity_s *grenade)
{
    // Only called by 0x005655c0 with an actor entity: native client branches cannot execute.
    iassert(parent->actor && !parent->client);
    const WeaponDef *weapon = BG_GetWeaponDef(grenade->s.weapon);
    grenade->s.eType = ET_MISSILE;
    grenade->s.lerp.eFlags = 0x1000000;
    grenade->s.lerp.u.actor.actorNum = level.time;
    grenade->s.lerp.u.actor.team = weapon->aiFuseTime;
    grenade->r.contents = 0x2100;
    for ( int i = 0; i < 3; ++i )
        grenade->r.mins[i] = -1.5f;
    for ( int i = 0; i < 3; ++i )
        grenade->r.maxs[i] = 1.5f;
    Actor_SP_GrenadeLandingCache(grenade)->originalThrowbackOwner = NULL; // SP entity+27c, SP-only EntHandle
    grenade->r.ownerNum.setEnt(parent);
    grenade->parent.setEnt(parent);
    grenade->s.lerp.u.loopFx.period = parent->s.clientNum;
    grenade->clipmask = weapon->plantable ? 0x281a091 : 0x280a091; // SP 0x281e091/0x280e091, actor mask mapping
    if ( weapon->stickiness == WEAPSTICKINESS_GROUND_WITH_YAW )
        grenade->clipmask &= ~0x2000000;
    grenade->handler = 10; // SP handler 14 -> KB missile handler 10, not a numeric struct mapping.
    G_BroadcastEntity(grenade);
    grenade->r.svFlags = 4;
    if ( weapon->bThrowBack )
        G_MakeMissilePickupItem(grenade);
    grenade->missile.team = parent->sentient ? parent->sentient->eTeam : TEAM_FREE;
}

// zombies: G_InitGrenadeMovement actor branch (SP 0x005f0900)
static void G_InitActorGrenadeMovement_SP(gentity_s *grenade)
{
    // 0x005655c0 passes currentOrigin, zero velocity, rotate=1, rotateType=0.
    // Parent was just set to an actor, so SP's player-only alternate gravity branch is false.
    grenade->item[1].ammoCount = 0;
    grenade->s.lerp.pos.trType = G_GetGrenadeTrType(grenade);
    grenade->s.lerp.pos.trTime = level.time;
    Vec3Copy(grenade->r.currentOrigin, grenade->s.lerp.pos.trBase);
    Vec3Clear(grenade->s.lerp.pos.trDelta);
    vectoangles(vec3_origin, grenade->r.currentAngles);
    grenade->s.lerp.apos.trType = TR_LINEAR;
    grenade->s.lerp.apos.trTime = level.time;
    Vec3Copy(grenade->r.currentAngles, grenade->s.lerp.apos.trBase);
    grenade->s.lerp.apos.trBase[0] = AngleNormalize360(grenade->s.lerp.apos.trBase[0] - 120.0f);
    // Preserve SP RNG order; KB's generic movement initializer draws these in reverse order.
    int sign = G_irand(0, 2);
    grenade->s.lerp.apos.trDelta[0] = (float)(G_flrand(340.0f, 800.0f) * (sign * 2 - 1));
    grenade->s.lerp.apos.trDelta[1] = 0.0f;
    sign = G_irand(0, 2);
    grenade->s.lerp.apos.trDelta[2] = (float)(G_flrand(180.0f, 540.0f) * (sign * 2 - 1));
    Vec3Copy(grenade->s.lerp.apos.trBase, grenade->r.currentAngles);
}

// zombies: FUN_005655c0 (SP 0x005655c0)
static void FUN_005655c0(actor_s *self)
{
    if ( self->pGrenade.isDefined() )
    {
        gentity_s *grenade = self->pGrenade.ent();
        actor_grenade_landing_sp_t *cache = Actor_SP_GrenadeLandingCache(grenade);
        if ( cache->throwbackOwner == self->ent )
        {
            if ( grenade->r.svFlags & 1 )
            {
                FUN_004187b0(self);
                G_InitActorGrenadeEntity_SP(self->ent, grenade);
                G_InitActorGrenadeMovement_SP(grenade);
                SV_LinkEntity(grenade);
            }
            cache->throwbackOwner = NULL;
        }
    }
}

// zombies: FUN_007c29e0 (SP 0x007c29e0)
static bool FUN_007c29e0(actor_s *self, const float *start, const float *end, const float *velocity)
{
    float mins[3] = { -1.0f, -1.0f, -1.0f }, maxs[3] = { 1.0f, 1.0f, 1.0f };
    col_context_t context(0x280b091); // SP 0x280f091, mapped actor contents
    context.passEntityNum0 = self->ent->s.number;
    float gravity = bg_gravity->current.value;
    float peakTime, halfRiseTime, halfFallTime;
    if ( velocity[2] <= 0.0f )
    {
        float discriminant = (start[2] - end[2]) * gravity * 2.0f + velocity[2] * velocity[2];
        peakTime = 0.0f;
        halfRiseTime = 0.0f;
        if ( discriminant <= 0.0f )
            return false;
        halfFallTime = (sqrtf(discriminant) + velocity[2]) / gravity;
    }
    else
    {
        float invGravity = 1.0f / gravity;
        peakTime = invGravity * velocity[2];
        halfFallTime = (start[2] - end[2]) * invGravity * 2.0f + peakTime * peakTime;
        halfRiseTime = peakTime * 0.5f;
        if ( halfFallTime <= 0.0f )
            return false;
        halfFallTime = sqrtf(halfFallTime);
    }
    halfFallTime *= 0.5f;
    float midFallTime = halfFallTime + peakTime;
    if ( fabsf(velocity[0] * (midFallTime + halfFallTime) + start[0] - end[0]) > 0.1f )
        return false;
    if ( fabsf(velocity[1] * (midFallTime + halfFallTime) + start[1] - end[1]) > 0.1f )
        return false;
    int hit = 0;
    float peak[3], mid[3];
    if ( peakTime <= 0.0f )
        Vec3Copy(start, peak);
    else
    {
        maxs[2] = halfRiseTime * halfRiseTime * gravity * 0.125f + 1.0f;
        mid[0] = velocity[0] * halfRiseTime + start[0];
        mid[1] = velocity[1] * halfRiseTime + start[1];
        mid[2] = halfRiseTime * halfRiseTime * gravity * 1.5f + start[2];
        SV_SightTraceCapsule(&hit, start, mins, maxs, mid, &context); // SP 0x00449a70
        if ( hit )
            return false;
        peak[0] = velocity[0] * peakTime + start[0];
        peak[1] = velocity[1] * peakTime + start[1];
        peak[2] = gravity * peakTime * peakTime * 0.5f + start[2];
        SV_SightTraceCapsule(&hit, mid, mins, maxs, peak, &context);
        if ( hit )
            return false;
    }
    maxs[2] = halfFallTime * halfFallTime * gravity * 0.125f + 1.0f;
    mid[0] = velocity[0] * midFallTime + start[0];
    mid[1] = velocity[1] * midFallTime + start[1];
    mid[2] = (velocity[2] - midFallTime * gravity * 0.5f) * midFallTime + start[2];
    SV_SightTraceCapsule(&hit, peak, mins, maxs, mid, &context);
    if ( hit )
        return false;
    trace_t trace = {};
    G_TraceCapsule(&trace, mid, mins, maxs, end, self->ent->s.number, 0x280b091, &context);
    if ( trace.fraction == 1.0f )
        return true;
    unsigned int entityNum = Trace_GetEntityHitId(&trace);
    if ( entityNum == ENTITYNUM_WORLD )
    {
        float impact[3];
        for ( int i = 0; i < 3; ++i )
            impact[i] = (end[i] - mid[i]) * trace.fraction + mid[i];
        double distSq = Vec3DistanceSq(end, impact);
        if ( distSq < 16.0 )
            return true;
        if ( (float)distSq < 900.0f && trace.normal.vec.v[2] > 0.5f )
            return true;
    }
    else
    {
        sentient_s *hitSentient = g_entities[entityNum].sentient;
        if ( hitSentient && hitSentient->eTeam == FUN_006938c0(self->sentient->eTeam) )
            return true;
    }
    return false;
}

// zombies: FUN_00645d10 (SP 0x00645d10)
static bool FUN_00645d10(actor_s *self, const float *start, const float *end, bool maxTime, float *velocity)
{
    float x = end[0] - start[0], y = end[1] - start[1], z = end[2] - start[2];
    float distSq = y * y + x * x + z * z;
    if ( distSq < 1.0f )
        return false;
    float gravity = bg_gravity->current.value;
    float energy = 810000.0f - gravity * z;
    float discriminant = energy * energy - gravity * gravity * distSq;
    if ( discriminant < 0.0f )
        return false;
    discriminant = sqrtf(discriminant);
    if ( !maxTime )
        discriminant = energy - discriminant;
    else
        discriminant += energy;
    float time = sqrtf(discriminant * 2.0f) / gravity;
    float invTime = 1.0f / time;
    velocity[0] = invTime * x;
    velocity[1] = invTime * y;
    velocity[2] = invTime * z + gravity * time * 0.5f;
    return FUN_007c29e0(self, start, end, velocity);
}

// zombies: FUN_00602770 (SP 0x00602770)
static bool FUN_00602770(actor_s *self, const float *start, const float *end, float *velocity)
{
    float x = end[0] - start[0], y = end[1] - start[1], z = end[2] - start[2];
    float gravity = bg_gravity->current.value;
    float length = sqrtf(y * y + x * x + z * z);
    if ( 810000.0f < (z + length) * gravity )
        return false;
    float time = sqrtf((length / gravity) * 2.0f);
    float invTime = 1.0f / time;
    velocity[0] = invTime * x;
    velocity[1] = invTime * y;
    velocity[2] = invTime * z + time * gravity * 0.5f;
    return FUN_007c29e0(self, start, end, velocity);
}

// zombies: FUN_0045a1a0 (SP 0x0045a1a0)
static bool FUN_0045a1a0(actor_s *self, const float *start, const float *end, float *velocity)
{
    float y = end[1] - start[1], z = end[2] - start[2], x = end[0] - start[0];
    float gravity = bg_gravity->current.value;
    float time = sqrtf(sqrtf(z * z + y * y + x * x) * gravity * 2.0f) / gravity;
    float invTime = 1.0f / time;
    velocity[1] = invTime * y;
    velocity[0] = invTime * x;
    velocity[2] = gravity * time * 0.5f + invTime * z;
    return FUN_007c29e0(self, start, end, velocity);
}

// zombies: FUN_005024e0 (SP 0x005024e0)
static bool FUN_005024e0(actor_s *self, const float *target, int weaponIndex)
{
    float radius = (float)BG_GetWeaponDef(weaponIndex)->iExplosionRadius * 1.1f;
    int teams = ~(1 << FUN_006938c0(self->sentient->eTeam));
    for ( sentient_s *other = Sentient_FirstSentient(teams); other; other = Sentient_NextSentient(other, teams) )
    {
        float origin[3];
        Sentient_GetOrigin(other, origin);
        float x = target[0] - origin[0], y = target[1] - origin[1], z = target[2] - origin[2];
        // Native compares squared distance with radius*1.1, not its square.
        if ( z * z + x * x + y * y <= radius )
            return false;
    }
    return true;
}

// zombies: FUN_0045f730 (SP 0x0045f730)
static void FUN_0045f730(actor_s *self, const float *origin, const float *offset, float *position)
{
    float axis[3][3];
    AnglesToAxis(self->ent->r.currentAngles, axis); // SP 0x00442990
    for ( int i = 0; i < 3; ++i )
        position[i] = origin[i] + offset[0] * axis[0][i] + offset[1] * axis[1][i] + offset[2] * axis[2][i];
}

// zombies: FUN_00478fb0 (SP 0x00478fb0)
static void FUN_00478fb0(const float *start, const float *target, float *end, int weaponIndex)
{
    const WeaponDef *weapon = BG_GetWeaponDef(weaponIndex);
    if ( weapon->bProjImpactExplode ) // SP WeaponDef+62c
    {
        Vec3Copy(target, end);
        return;
    }
    float below[3] = { target[0], target[1], target[2] - 1.0f };
    float above[3] = { target[0], target[1], target[2] + 1.0f };
    trace_t trace = {};
    G_MissileTrace(&trace, above, below, ENTITYNUM_NONE, 0x811, weaponIndex);
    int surface = trace.fraction == 1.0f ? 0 : (trace.sflags >> 20) & 0x3f;
    float parallel = weapon->parallelBounce[surface];
    float factor = 1.0f - ((weapon->perpendicularBounce[surface] - parallel) * 0.9397f + parallel);
    for ( int i = 0; i < 3; ++i )
        end[i] = start[i] + (target[i] - start[i]) * factor;
}

// zombies: FUN_004bb030 (SP 0x004bb030)
static bool FUN_004bb030(const float *start, const float *target, unsigned int method, float *hint)
{
    float cosine = (float)cos((double)((ai_debugGrenadeHintArc->current.value * 0.5f + 180.0f) * 0.017453292f));
    unsigned int best = ~0u;
    float bestDistance = FLT_MAX;
    for ( unsigned int i = 0; i < g_grenadeHintCount; ++i )
    {
        const float *candidate = g_grenadeHints[i];
        if ( target[2] <= candidate[2] + 10.0f )
        {
            float x = candidate[0] - target[0], y = candidate[1] - target[1];
            float length = sqrtf(x * x + y * y);
            float safeLength = -length >= 0.0f ? 1.0f : length;
            if ( length <= 192.0f )
            {
                float sx = candidate[0] - start[0], sy = candidate[1] - start[1];
                float startLength = sqrtf(sy * sy + sx * sx);
                if ( -startLength >= 0.0f )
                    startLength = 1.0f;
                if ( (1.0f / startLength) * sx * (1.0f / safeLength) * x
                    + sy * (1.0f / startLength) * (1.0f / safeLength) * y <= cosine && length < bestDistance )
                {
                    best = i;
                    bestDistance = length;
                }
            }
        }
    }
    if ( best != ~0u )
    {
        Vec3Copy(g_grenadeHints[best], hint);
        return true;
    }
    return false;
}

// zombies: FUN_0053ac00 (SP 0x0053ac00)
bool FUN_0053ac00(actor_s *self, const float *origin, const float *offset, const float *target,
    unsigned int method, float *tossPosition, float *velocity, float randomRange, bool throwback)
{
    trace_t trace = {};
    if ( bg_gravity->current.value <= 0.0f || (self->pGrenade.isDefined() && !throwback) )
        return false;
    if ( !FUN_005024e0(self, target, self->iGrenadeWeaponIndex) ) // SP actor+2160 -> iGrenadeWeaponIndex
        return false;
    float randomized[3];
    Vec3Copy(target, randomized);
    if ( randomRange != 0.0f )
    {
        randomized[0] = (float)G_flrand(randomized[0] - randomRange, randomized[0] + randomRange);
        randomized[1] = (float)G_flrand(target[1] - randomRange, target[1] + randomRange);
    }
    FUN_0045f730(self, origin, offset, tossPosition);
    G_MissileTrace(&trace, tossPosition, randomized, ENTITYNUM_NONE, 0x811, self->iGrenadeWeaponIndex);
    float end[3], hint[3];
    if ( trace.fraction == 1.0f )
        FUN_00478fb0(tossPosition, randomized, end, self->iGrenadeWeaponIndex);
    else if ( !g_grenadeHintCount || !FUN_004bb030(tossPosition, randomized, method, hint) )
        Vec3Copy(randomized, end);
    else
    {
        end[0] = (float)G_flrand(hint[0] - 15.0f, 15.0f + hint[0]);
        end[1] = (float)G_flrand(hint[1] - 15.0f, 15.0f + hint[1]);
        end[2] = (float)G_flrand(hint[2] - 15.0f, 15.0f + hint[2]);
    }
    float x = end[0] - tossPosition[0], y = end[1] - tossPosition[1], z = end[2] - tossPosition[2];
    if ( z * z + x * x + y * y == 0.0f )
        return false;
    const char *name = SL_ConvertToString(method, SCRIPTINSTANCE_SERVER);
    if ( !strcmp(name, "min energy") )
        return FUN_00602770(self, tossPosition, end, velocity);
    if ( !strcmp(name, "min time") )
        return FUN_00645d10(self, tossPosition, end, false, velocity);
    if ( !strcmp(name, "max time") )
        return FUN_00645d10(self, tossPosition, end, true, velocity);
    if ( strcmp(name, "infinite energy") )
    {
        Scr_Error(va("checkGrenadeThrow: method must be 'min energy', 'min time', or 'max time' - value passed in was %s", name), 0);
        return false;
    }
    return FUN_0045a1a0(self, tossPosition, end, velocity);
}

// zombies: FUN_007c30b0 (SP 0x007c30b0)
static bool FUN_007c30b0(actor_s *self, const float *origin, const float *target, float *tossPosition, float *velocity)
{
    float offset[3] = { 0.0f, 0.0f, 40.0f };
    const char *methods[] = { "min energy", "min time", "max time" }; // SP slots 023a5880/5882/587c
    for ( int i = 0; i < 3; ++i )
    {
        Scr_SetStringFromCharString(&self->GrenadeTossMethod, (char *)methods[i], SCRIPTINSTANCE_SERVER); // SP 0x00406bd0
        if ( FUN_0053ac00(self, origin, offset, target, self->GrenadeTossMethod, tossPosition, velocity, 0.0f, true) )
            return true;
    }
    return false;
}

// zombies: FUN_007c2130 (SP 0x007c2130)
static int __cdecl FUN_007c2130(const void *left, const void *right)
{
    const float *a = (const float *)left;
    const float *b = (const float *)right;
    // Preserve SP's x/z/y and z/x/y summation orders, respectively.
    float distanceA = (a[0] - g_grenadeHintSortOrigin[0]) * (a[0] - g_grenadeHintSortOrigin[0])
        + (a[2] - g_grenadeHintSortOrigin[2]) * (a[2] - g_grenadeHintSortOrigin[2])
        + (a[1] - g_grenadeHintSortOrigin[1]) * (a[1] - g_grenadeHintSortOrigin[1]);
    float distanceB = (b[2] - g_grenadeHintSortOrigin[2]) * (b[2] - g_grenadeHintSortOrigin[2])
        + (b[0] - g_grenadeHintSortOrigin[0]) * (b[0] - g_grenadeHintSortOrigin[0])
        + (b[1] - g_grenadeHintSortOrigin[1]) * (b[1] - g_grenadeHintSortOrigin[1]);
    if ( distanceA < distanceB )
        return -1;
    if ( distanceB < distanceA )
        return 1;
    return 0;
}

// zombies: FUN_007c2f20 (SP 0x007c2f20)
static bool FUN_007c2f20(actor_s *self, const float *start, float *velocity)
{
    if ( g_grenadeHintCount )
    {
        Vec3Copy(start, g_grenadeHintSortOrigin);
        qsort(g_grenadeHints, g_grenadeHintCount, sizeof(g_grenadeHints[0]), FUN_007c2130);
        for ( unsigned int i = 0; i < g_grenadeHintCount; ++i )
        {
            const float *hint = g_grenadeHints[i];
            float x = hint[0] - start[0], y = hint[1] - start[1], z = hint[2] - start[2];
            float distSq = y * y + x * x + z * z;
            if ( distSq >= 1.0f )
            {
                float gravity = bg_gravity->current.value;
                float energy = 810000.0f - gravity * z;
                float discriminant = energy * energy - gravity * gravity * distSq;
                if ( discriminant >= 0.0f )
                {
                    float time = sqrtf((energy - sqrtf(discriminant)) * 2.0f) / gravity;
                    float invTime = 1.0f / time;
                    velocity[0] = invTime * x;
                    velocity[1] = invTime * y;
                    velocity[2] = invTime * z + gravity * time * 0.5f;
                    if ( FUN_007c29e0(self, start, hint, velocity) )
                        return true;
                }
            }
        }
    }
    return false;
}

// zombies: FUN_007c31b0 (SP 0x007c31b0)
static bool FUN_007c31b0(actor_s *self)
{
    gentity_s *grenade = self->pGrenade.ent();
    if ( !grenade->parent.isDefined() || !grenade->parent.ent()->sentient )
        return false;
    if ( grenade->parent.ent()->sentient->eTeam != FUN_006938c0(self->sentient->eTeam) )
        return false;
    if ( BG_GetWeaponDef(grenade->s.weapon)->weapType == WEAPTYPE_PROJECTILE ) // SP WeaponDef+1c
        return false;
    actor_grenade_landing_sp_t *landing = Actor_SP_GrenadeLandingCache(grenade);
    float grenadePos[3], targetPos[3], goalPos[3], velocity[3];
    float tossInfo[3]; // SP reuses its enemy-team table as this position output.
    Vec3Copy(landing->position, grenadePos);
    if ( sqrtf(Vec2DistanceSq(landing->position, self->ent->r.currentOrigin)) > 150.0f )
        return false;
    Sentient_GetOrigin(grenade->parent.ent()->sentient, targetPos);
    bool nearGrenade = Actor_PointAt(self->ent->r.currentOrigin, landing->position); // SP 0x004b77d0: 2-unit XY, 80-unit Z
    if ( !nearGrenade )
        FUN_007c2430(targetPos, goalPos, self);
    else
        Vec3Copy(landing->position, goalPos);
    if ( !FUN_007c30b0(self, grenadePos, targetPos, tossInfo, velocity) )
    {
        sentient_s *enemy = Actor_GetTargetSentient(self);
        if ( enemy )
        {
            Sentient_GetOrigin(enemy, targetPos);
            if ( FUN_007c30b0(self, grenadePos, targetPos, tossInfo, velocity) )
            {
                if ( !nearGrenade )
                    FUN_007c2430(targetPos, goalPos, self);
                goto find_path;
            }
        }
        self->bGrenadeTargetValid = false; // SP actor+216c
        if ( !FUN_007c2f20(self, grenadePos, velocity) )
            return false;
    }
find_path:
    Actor_FindPath_SP(self, goalPos, 0, true);
    if ( !Actor_HasPath(self) )
        return false;
    if ( self->goalPosChanged )
        Scr_Notify(self->ent, scr_const.goal_changed, 0);
    if ( !Actor_GetTargetEntity(self) )
    {
        Actor_GetPerfectInfo(self, grenade->parent.ent()->sentient); // SP thunk 0x00698f10: time/origin/nearest-node stores
        Sentient_SetEnemy(self->sentient, grenade->parent.ent(), 1); // SP 0x004bf0b0
    }
    // SP +2168/+2174/+2180/+218c -> toss validity/position/target/velocity in KB.
    self->bGrenadeTossValid = true;
    Vec3Copy(grenadePos, self->vGrenadeTossPos);
    Vec3Copy(velocity, self->vGrenadeTossVel);
    if ( self->bGrenadeTargetValid )
        Vec3Copy(targetPos, self->vGrenadeTargetPos);
    if ( landing->throwbackOwner != self->ent )
    {
        grenade->nextthink += 1000;
        landing->throwbackOwner = self->ent;
        Actor_SetSubState(self, STATE_GRENADE_ACQUIRE);
    }
    return true;
}

// zombies: FUN_007c34f0 (SP 0x007c34f0)
static void FUN_007c34f0(actor_s *self, bool forceRepath)
{
    gentity_s *grenade = self->pGrenade.ent();
    bool ownGrenade = grenade->parent.isDefined() && grenade->parent.ent() == self->ent;
    FUN_005655c0(self);
    if ( !Actor_GetTargetEntity(self) && grenade->parent.isDefined() )
    {
        gentity_s *parent = grenade->parent.ent();
        if ( parent->sentient && parent->sentient != self->sentient
            && parent->sentient->eTeam != self->sentient->eTeam && !Actor_CheckIgnore(parent->sentient, self->sentient) )
        {
            Actor_GetPerfectInfo(self, parent->sentient); // SP thunk 0x00698f10
            Sentient_SetEnemy(self->sentient, parent, 1);
        }
    }
    float radius = (float)BG_GetWeaponDef(grenade->s.weapon)->iExplosionRadius * 1.1f;
    const float *landing = Actor_SP_GrenadeLandingCache(grenade)->position;
    if ( !forceRepath && Actor_HasPath(self)
        && !FUN_004b03e0(&self->Path, self->ent->r.currentOrigin, landing, radius * radius) )
    {
        Actor_SetSubState(self, STATE_GRENADE_FLEE);
        return;
    }
    if ( ownGrenade )
    {
        Actor_UpdateGoalPath_SP(self);
        if ( Actor_HasPath(self) && FUN_0041ea90(self, self->Path.vFinalGoal) )
        {
            Actor_SetSubState(self, STATE_GRENADE_FLEE);
            return;
        }
    }
    else
    {
        pathnode_t *node = FUN_004a26c0(self, landing, radius);
        if ( node && FUN_005f3580(self, node, 1) )
        {
            Actor_ClearKeepClaimedNode(self); // SP 0x004d01c0
            Sentient_ClaimNode(self->sentient, node);
            Actor_SetSubState(self, STATE_GRENADE_TAKECOVER);
            return;
        }
    }
    float direction[2];
    float distance = FUN_007c22d0(self, direction);
    FUN_00599e50(self, landing, radius, direction, distance, 1, 1);
    if ( Actor_HasPath(self) )
    {
        Actor_SetSubState(self, STATE_GRENADE_FLEE);
        return;
    }
    if ( FUN_0049ce70(self) )
    {
        FUN_00599e50(self, landing, radius, direction, distance, 0, 1);
        if ( Actor_HasPath(self) )
        {
            Actor_SetSubState(self, STATE_GRENADE_FLEE);
            return;
        }
    }
    FUN_00523360(self, landing, radius, 1);
    Actor_SetSubState(self, Actor_HasPath(self) ? STATE_GRENADE_FLEE : STATE_GRENADE_COWER);
}

// zombies: FUN_005af1f0 (SP 0x005af1f0)
void Actor_GrenadeBounce_SP(gentity_s *grenade, gentity_s *hitEnt)
{
    // SP r.contents mask 0x0200c000 -> KB 0x02008000 (actor contents mapping).
    if ( !(hitEnt->r.contents & 0x02008000) )
        return;
    actor_grenade_landing_sp_t *landing = Actor_SP_GrenadeLandingCache(grenade);
    Vec3Clear(landing->position);
    Actor_SP_PredictGrenadeLanding(grenade);
    for ( actor_s *self = Actor_FirstActor(-1); self; self = Actor_NextActor(self, -1) ) // SP 0x00564a50 / 0x00428d20
    {
        if ( !self->pGrenade.isDefined() || self->pGrenade.ent() != grenade
            || self->eSimulatedState[self->simulatedStateLevel] != AIS_GRENADE_RESPONSE ) // SP +c3c/+c54
            continue;
        if ( Vec2DistanceSq(grenade->r.currentOrigin, self->ent->r.currentOrigin) <= 22500.0f && FUN_007c31b0(self) )
            continue;
        if ( landing->throwbackOwner == self->ent && FUN_0041ea90(self, self->ent->r.currentOrigin) )
        {
            landing->throwbackOwner = NULL;
            self->pGrenade.setEnt(NULL);
            Actor_SetState(self, AIS_EXPOSED); // SP 0x004dceb0; existing state queue/simplify/arrival-clear sequence
        }
        else
            FUN_007c34f0(self, true);
    }
}

// zombies: FUN_007bfb80 (SP 0x007bfb80)
static bool FUN_007bfb80(actor_s *self, pathnode_t *node, bool checkEnemy)
{
    NodeEngagementRecord record;
    FUN_004d8ae0(&record, node);
    gentity_s *target = Actor_GetTargetEntity(self);
    sentient_s *enemy = Actor_GetTargetSentient(self);
    if ( !target && !enemy )
    {
        if ( !checkEnemy || !self->pGrenade.isDefined() )
            return true;
        // SP's grenade event/bounce paths populate this before the exposed reader.
        actor_grenade_landing_sp_t *landing = Actor_SP_GrenadeLandingCache(self->pGrenade.ent());
        if ( Vec2DistanceSq(node->constant.vOrigin, landing->position) < record.minDistanceSq )
            return false;
        return FUN_004dd0b0(landing->position, node, &record);
    }
    if ( enemy )
    {
        if ( enemy->ent->flags & FL_NOTARGET )
            return true;
        if ( Actor_CheckIgnore(self->sentient, enemy) ) // SP 0x0052f5c0, identical threat-bias query
            return true;
        if ( self->sentientInfo[enemy - level.sentients].lastKnownPosTime < 1 )
            return true;
        if ( !checkEnemy && !Actor_CanSeeEnemy_SP(self) )
            return true;
        if ( FUN_00565c70(enemy->ent->r.currentOrigin, node, &record) )
            return true;
        return FUN_00565c70(self->sentientInfo[enemy - level.sentients].vLastKnownPos, node, &record);
    }
    if ( !checkEnemy && !FUN_005eeb10(self, target) )
        return true;
    if ( Vec2DistanceSq(node->constant.vOrigin, target->r.currentOrigin) < record.minDistanceSq )
        return false;
    return FUN_004dd0b0(target->r.currentOrigin, node, &record);
}

// zombies: FUN_004e2da0 (SP 0x004e2da0)
static bool Actor_GetNodeAnim_SP(actor_s *self, pathnode_t *node, bool checkEnemy, scr_animscript_t **script)
{
    if ( !FUN_007bfa30(self, node, script) )
        return false;
    if ( FUN_007bfb80(self, node, checkEnemy) )
        return *script != NULL;
    *script = NULL;
    return true;
}

// zombies: FUN_005ae040 (SP 0x005ae040)
bool FUN_005ae040(actor_s *self, pathnode_t *node)
{
    if ( Path_CanClaimNode(node, self->sentient) )
    {
        sentient_s *enemy = Actor_GetTargetSentient(self);
        scr_animscript_t *script;
        if ( !enemy )
        {
            if ( FUN_007bfa30(self, node, &script) && !script )
                return false;
            return true;
        }
        Sentient_NearestNode(enemy); // SP evaluates this for its cache side effect.
        gentity_s *target = Actor_GetTargetEntity(self);
        if ( !target || node->constant.forward[1] * (target->r.currentOrigin[1] - node->constant.vOrigin[1])
            + node->constant.forward[0] * (target->r.currentOrigin[0] - node->constant.vOrigin[0]) >= 0.0f )
        {
            if ( !Actor_GetNodeAnim_SP(self, node, true, &script) )
                return true;
            if ( script )
                return true;
        }
    }
    return false;
}

// zombies: FUN_007bfd40 (SP 0x007bfd40)
static float FUN_007bfd40(actor_s *self, pathnode_t *node)
{
    // The C export loses the XMM0 return. 0x007bfdc3..0x007bfe15 supplies the ramps.
    gentity_s *target = Actor_GetTargetEntity(self);
    if ( target )
    {
        float delta[3];
        Vec3Sub(target->r.currentOrigin, node->constant.vOrigin, delta);
        float distance = sqrtf(delta[0] * delta[0] + delta[2] * delta[2] + delta[1] * delta[1]);
        actor_sp_ext_t *ext = Actor_SP_Ext(self); // SP +1ad8/+1adc/+1ae0/+1ae4, existing engagement side fields
        if ( ext->engageMinFalloffDist <= distance && distance <= ext->engageMaxFalloffDist )
        {
            if ( ext->engageMinDist <= distance )
            {
                if ( distance <= ext->engageMaxDist )
                    return 1.0f;
                if ( distance <= ext->engageMaxFalloffDist )
                    return (ext->engageMaxFalloffDist - distance) / (ext->engageMaxFalloffDist - ext->engageMaxDist);
            }
            else if ( ext->engageMinFalloffDist <= distance )
                return 1.0f - (ext->engageMinDist - distance) / (ext->engageMinDist - ext->engageMinFalloffDist);
        }
    }
    return 0.0f;
}

// zombies: FUN_00683580 (SP 0x00683580)
static bool FUN_00683580(const potential_threat_t *threat, float *direction)
{
    // SP potential_threat+0/+4/+8 -> isEnabled/direction[0]/direction[1].
    if ( threat->isEnabled )
    {
        direction[0] = threat->direction[0];
        direction[1] = threat->direction[1];
    }
    return threat->isEnabled;
}

// zombies: FUN_0052c880 (SP 0x0052c880)
static bool FUN_0052c880(const float *origin, const float *cover, float maxAngle, float distance, float radius)
{
    col_context_t context; // SP 0x004b3e90
    float start[3] = { origin[0], origin[1], origin[2] };
    float x = cover[0] - start[0];
    float y = cover[1] - origin[1];
    float z = cover[2] - origin[2];
    float length = sqrtf(y * y + x * x + z * z);
    if ( length > 0.1f )
    {
        float invLength = 1.0f / length;
        if ( distance - length >= 0.0f )
            distance = length;
        float end[3] = { x * invLength * distance + start[0],
            y * invLength * distance + start[1], invLength * z * distance + origin[2] + 15.0f };
        start[2] = origin[2] + 15.0f; // SP DAT_00b77908
        float mins[3] = { -radius, -radius, -radius };
        float maxs[3] = { radius, radius, radius };
        trace_t trace = {};
        G_TraceCapsule(&trace, start, mins, maxs, end, ENTITYNUM_NONE, 1, &context); // SP 0x004ee360
        if ( trace.fraction < 1.0f )
        {
            if ( trace.normal.vec.v[2] * trace.normal.vec.v[2] + trace.normal.vec.v[1] * trace.normal.vec.v[1]
                + trace.normal.vec.v[0] * trace.normal.vec.v[0] <= 0.0f )
                return true;
            float dot = trace.normal.vec.v[1] * y * invLength + trace.normal.vec.v[2] * invLength * z
                + trace.normal.vec.v[0] * x * invLength;
            float cosine = (float)cos((double)(maxAngle * 0.017453292f)); // SP CRT 0x009667a1
            if ( cosine < -dot )
                return true;
            return false;
        }
    }
    return false;
}

// zombies: FUN_007bfe20 (SP 0x007bfe20)
static float FUN_007bfe20(actor_s *self, pathnode_t *node)
{
    float threatDir[2];
    bool hasThreat = FUN_00683580(&self->potentialThreat, threatDir); // SP +213c
    NodeEngagementRecord record;
    FUN_004d8ae0(&record, node);
    gentity_s *target = Actor_GetTargetEntity(self);
    float targetPos[3];
    if ( !target )
    {
        if ( !hasThreat )
            return 0.0f;
    }
    else if ( Actor_CanSeeEnemy_SP(self) || !hasThreat )
    {
        if ( !FUN_004dd0b0(target->r.currentOrigin, node, &record) )
            return 0.0f;
        return 1.0f;
    }
    targetPos[0] = node->constant.vOrigin[0] + threatDir[0];
    targetPos[1] = node->constant.vOrigin[1] + threatDir[1];
    targetPos[2] = node->constant.vOrigin[2];
    if ( !FUN_004dd0b0(targetPos, node, &record) )
        return 0.0f;
    return 1.0f;
}

// zombies: FUN_007bfed0 (SP 0x007bfed0)
static float FUN_007bfed0(actor_s *self, pathnode_t *node)
{
    gentity_s *target = Actor_GetTargetEntity(self);
    if ( !target )
        return 0.0f;
    if ( Actor_PointNearNode(self->ent->r.currentOrigin, node) ) // SP 0x004ac020
        return 1.0f;
    // XMM0 result omitted from C: (normalized enemy-from-node dot node-from-self + 1) / 2.
    float enemyX = target->r.currentOrigin[0] - node->constant.vOrigin[0];
    float enemyY = target->r.currentOrigin[1] - node->constant.vOrigin[1];
    float length = sqrtf(enemyX * enemyX + enemyY * enemyY);
    if ( -length >= 0.0f )
        length = 1.0f;
    float invLength = 1.0f / length;
    enemyX *= invLength;
    enemyY *= invLength;
    float nodeX = node->constant.vOrigin[0] - self->ent->r.currentOrigin[0];
    float nodeY = node->constant.vOrigin[1] - self->ent->r.currentOrigin[1];
    length = sqrtf(nodeY * nodeY + nodeX * nodeX);
    if ( -length >= 0.0f )
        length = 1.0f;
    invLength = 1.0f / length;
    return (nodeY * invLength * enemyY + invLength * nodeX * enemyX + 1.0f) * 0.5f;
}

// zombies: FUN_007bffe0 (SP 0x007bffe0)
static float FUN_007bffe0(actor_s *self, pathnode_t *node)
{
    gentity_s *target = Actor_GetTargetEntity(self);
    if ( target && !FUN_0052c880(target->r.currentOrigin, node->constant.vOrigin,
        ai_coverFlankMaxAngle->current.value, ai_coverFlankDistToCover->current.value, ai_coverFlankCheckRad->current.value) )
        return 1.0f;
    return 0.0f;
}

// zombies: FUN_004698d0 (SP 0x004698d0)
static float FUN_004698d0(actor_s *self, pathnode_t *node)
{
    float nodeAngle = FUN_007bfe20(self, node);
    float delta[3];
    Vec3Sub(self->ent->r.currentOrigin, node->constant.vOrigin, delta);
    float distSq = delta[2] * delta[2] + delta[0] * delta[0] + delta[1] * delta[1];
    float clamped = 1440000.0f;
    if ( distSq - 1440000.0f < 0.0f )
        clamped = distSq;
    if ( -distSq >= 0.0f )
        clamped = 0.0f;
    float distance = (1.0f - clamped * 6.9444445e-7f) * ai_coverScore_distance->current.value;
    float score = FUN_007bfd40(self, node) * ai_coverScore_engagement->current.value + distance;
    float visible = 1.0f;
    sentient_s *enemy = Actor_GetTargetSentient(self); // SP 0x004fd9b0
    if ( enemy )
    {
        pathnode_t *enemyNode = Sentient_NearestNode(enemy); // SP 0x00537ae0
        if ( enemyNode && !Path_NodesVisible(node, enemyNode) ) // SP 0x004a32d0
            visible = 0.0f;
    }
    score = ai_coverScore_visibility->current.value * visible + score;
    float coverType = ((1 << node->constant.type) & 0x3800) ? 0.0f : 1.0f;
    score = ai_coverScore_coverType->current.value * coverType + score;
    score += ai_coverScore_nodeAngle->current.value * nodeAngle;
    score = FUN_007bfed0(self, node) * ai_coverScore_targetDir->current.value + score;
    // SP node+58 -> dynamic.inPlayerLOSTime; SP team 2 -> TEAM_ALLIES.
    float playerLos = node->dynamic.inPlayerLOSTime < level.time || self->sentient->eTeam != TEAM_ALLIES ? 1.0f : 0.0f;
    score = ai_coverScore_playerLos->current.value * playerLos + score;
    float priority = (node->constant.spawnflags & 0x40) ? 1.0f : 0.0f;
    score = ai_coverScore_priority->current.value * priority + score;
    if ( nodeAngle > 0.0f && ai_coverScore_flanking->current.value > 0.0f )
        score = FUN_007bffe0(self, node) * ai_coverScore_flanking->current.value + score;
    return score;
}

// zombies: FUN_007c01a0 (SP 0x007c01a0)
static int FUN_007c01a0(actor_s *self, pathsort_t *nodes, int count, gentity_s *volume)
{
    int i = 0;
    while ( i < count )
    {
        pathnode_t *node = nodes[i].node;
        if ( !FUN_005ae040(self, node)
            || (volume && !SV_EntityContact(node->constant.vOrigin, node->constant.vOrigin, volume)) ) // SP 0x0059a900
        {
            --count;
            nodes[i] = nodes[count];
        }
        else
        {
            float delta[3];
            Vec3Sub(self->ent->r.currentOrigin, node->constant.vOrigin, delta);
            float distSq = delta[0] * delta[0] + delta[2] * delta[2] + delta[1] * delta[1];
            float clamped = 1440000.0f;
            if ( distSq - 1440000.0f < 0.0f )
                clamped = distSq;
            if ( -distSq >= 0.0f )
                clamped = 0.0f;
            nodes[i].distMetric = (1.0f - clamped * 6.9444445e-7f) * ai_coverScore_distance->current.value;
            nodes[i].metric = FUN_004698d0(self, node);
            ++i;
        }
    }
    return count;
}

// zombies: FUN_007c02d0 (SP 0x007c02d0)
static int __cdecl FUN_007c02d0(const void *left, const void *right)
{
    float delta = ((const pathsort_t *)left)->metric - ((const pathsort_t *)right)->metric;
    if ( delta < 0.0f )
        return -1;
    if ( delta > 0.0f )
        return 1;
    return 0;
}

// zombies: FUN_004e8c20 (SP 0x004e8c20)
static int FUN_004e8c20(actor_s *self, pathnode_t **outNodes, int requestedCount)
{
    pathsort_t nodes[256];
    int count = Path_NodesInCylinder(self->codeGoal.pos, self->codeGoal.radius, self->codeGoal.height,
        nodes, ARRAY_COUNT(nodes), 0x83ffc); // SP 0x00626af0
    count = FUN_007c01a0(self, nodes, count, self->codeGoal.volume);
    self->numCoverNodesInGoal = count; // SP actor+1aa4 -> KB numCoverNodesInGoal
    for ( int i = 0; i < count; ++i )
        nodes[i].metric -= nodes[i].distMetric;
    qsort(nodes, count, sizeof(pathsort_t), FUN_007c02d0); // SP CRT 0x00967090
    for ( int i = 0; i < count; ++i )
    {
        if ( nodes[i].node != self->sentient->pClaimedNode || i == count - 1 )
            nodes[i].metric += nodes[i].distMetric;
    }
    qsort(nodes, count, sizeof(pathsort_t), FUN_007c02d0);
    int first = 0;
    if ( requestedCount < count )
        first = count - requestedCount;
    int written = 0;
    for ( int i = first; i < count; ++i )
        outNodes[written++] = nodes[i].node;
    return written; // EAX at 0x004e8e4e; C export incorrectly declares void.
}

// zombies: FUN_005b5540 (SP 0x005b5540); also findbestcovernode (x3).
pathnode_t *FUN_005b5540(actor_s *self)
{
    // The selector returns a count in EAX and writes the node through EDX.
    pathnode_t *node = NULL;
    if ( !FUN_004e8c20(self, &node, 1) )
        return NULL;
    return node;
}

// zombies: FUN_007bb500 (SP 0x007bb500)
static pathnode_t *FUN_007bb500(actor_s *self)
{
    pathnode_t *node = self->sentient->pClaimedNode;
    bool claimedInGoal;
    if ( !node || !Actor_PointAtGoal(node->constant.vOrigin, &self->codeGoal)
        || (self->codeGoal.volume && !SV_EntityContact(node->constant.vOrigin,
            node->constant.vOrigin, self->codeGoal.volume)) )
        claimedInGoal = false;
    else
    {
        claimedInGoal = true;
        if ( FUN_005ae040(self, node) )
            return node;
    }
    if ( !Actor_GetTargetEntity(self) && self->codeGoal.node )
    {
        if ( FUN_005ae040(self, self->codeGoal.node)
            && Actor_PointAtGoal(self->codeGoal.node->constant.vOrigin, &self->codeGoal) )
            return self->codeGoal.node;
    }
    else
    {
        node = FUN_005b5540(self);
        if ( node )
            return node;
        if ( claimedInGoal )
            return self->sentient->pClaimedNode;
    }
    return NULL;
}

// zombies: FUN_007bcbd0 (SP 0x007bcbd0)
static bool FUN_007bcbd0(actor_s *self)
{
    pathnode_t *node = self->codeGoal.node;
    if ( !Actor_HasPath(self) )
    {
        if ( Actor_PointNear(self->ent->r.currentOrigin, self->codeGoal.pos) )
        {
            self->commitToFixedNode = false;
            return true;
        }
    }
    else
    {
        if ( Actor_PointNear(self->Path.vFinalGoal, self->codeGoal.pos) )
            return true;
        if ( self->arrivalInfo.animscriptOverrideRunTo
            && Actor_PointNear(self->Path.vFinalGoal, self->arrivalInfo.animscriptOverrideRunToPos) )
            return true;
    }
    if ( node && !Path_CanClaimNode(node, self->sentient) )
    {
        // SP node+44 -> dynamic.pOwner, a SentientHandle, not an entity index.
        if ( node->dynamic.pOwner.isDefined() )
        {
            Scr_AddEntity(node->dynamic.pOwner.sentient()->ent, SCRIPTINSTANCE_SERVER);
            Scr_Notify(self->ent, scr_const.node_taken, 1);
        }
        return false;
    }
    // 0x0044ff30 matches KB Actor_IsKnownEnemyInRegion, with the engine's sentient
    // capacity (36 SP / 48 KB) and fixedNodeSafeVolumeRadiusSq / dying-actor checks.
    gentity_s *enemy = Actor_IsKnownEnemyInRegion(self,
        self->fixedNodeSafeVolume.isDefined() ? self->fixedNodeSafeVolume.ent() : NULL,
        self->codeGoal.pos, self->fixedNodeSafeRadius);
    if ( !enemy )
        return true;
    Scr_AddEntity(enemy, SCRIPTINSTANCE_SERVER);
    Scr_Notify(self->ent, scr_const.node_not_safe, 1);
    return false;
}

static path_t g_exposedPathBackup;

// zombies: FUN_005c80b0 (SP 0x005c80b0)
static void FUN_005c80b0(const path_t *path)
{
    g_exposedPathBackup = *path; // SP memcpy(1000), same path_t layout in KB.
}

// zombies: FUN_005f2300 (SP 0x005f2300)
static void FUN_005f2300(path_t *path)
{
    *path = g_exposedPathBackup;
    if ( path->wPathLen > 0 && path->wNegotiationStartNode > 0 )
        ++Path_ConvertIndexToNode(path->pts[path->wNegotiationStartNode].iNodeNum)->dynamic.userCount;
}

// zombies: FUN_007bda90 (SP 0x007bda90)
static void FUN_007bda90(actor_s *self)
{
    FUN_005c80b0(&self->Path);
    if ( !Actor_FindPath_SP(self, self->arrivalInfo.animscriptOverrideRunToPos, 1, false) )
    {
        if ( self->Path.wPathLen )
            Actor_ClearPath(self); // AddTrimmedAmount, Path_Clear, iTeamMoveDodgeTime = 0
        FUN_005f2300(&self->Path);
        self->arrivalInfo.animscriptOverrideRunTo = 0;
        self->arrivalInfo.arrivalNotifyRequested = 0;
    }
}

// zombies: Actor_TransferGoal / Actor_UpdateGoalPos (SP 0x0040aaa0)
void Actor_UpdateGoalPos_SP(actor_s *self)
{
    float previous[3];
    Vec3Copy(self->codeGoal.pos, previous);
    if ( !self->useEnemyGoal )
    {
        if ( !self->scriptGoalEnt.isDefined() )
        {
            if ( enable_moving_paths->current.integer == 1 && self->scriptGoal.node )
                Vec3Copy(self->scriptGoal.node->constant.vOrigin, self->scriptGoal.pos);
            Vec3Copy(self->scriptGoal.pos, self->codeGoal.pos);
            Vec3Copy(self->scriptGoal.ang, self->codeGoal.ang); // MP omits goal angles.
            self->codeGoalSrc = AI_GOAL_SRC_SCRIPT_GOAL;
            self->codeGoal.node = self->scriptGoal.node;
            self->codeGoal.volume = self->scriptGoal.volume;
        }
        else
        {
            self->codeGoal.node = NULL;
            self->codeGoal.volume = NULL;
            Vec3Copy(self->scriptGoalEnt.ent()->r.currentOrigin, self->codeGoal.pos);
            self->codeGoalSrc = AI_GOAL_SRC_SCRIPT_ENTITY_GOAL;
        }
        float radius = self->scriptGoal.radius;
        if ( self->sentient && self->sentient->bInMeleeCharge && radius > 64.0f )
            radius = 64.0f;
        if ( radius < 4.0f )
            radius = 4.0f;
        self->codeGoal.radius = radius;
        float height = self->scriptGoal.height;
        if ( height < 80.0f )
            height = 80.0f;
        self->codeGoal.height = height;
    }
    else
    {
        sentient_s *enemy = Actor_GetTargetSentient(self);
        Vec3Copy(self->sentientInfo[enemy - level.sentients].vLastKnownPos, self->codeGoal.pos);
        float radius = self->pathEnemyFightDist;
        self->codeGoalSrc = AI_GOAL_SRC_ENEMY;
        self->codeGoal.node = NULL;
        self->codeGoal.volume = NULL;
        if ( radius < 4.0f )
            radius = 4.0f;
        self->codeGoal.radius = radius;
    }
    if ( self->arrivalInfo.animscriptOverrideRunTo && !Vec3Compare(self->codeGoal.pos, previous) )
        self->arrivalInfo.animscriptOverrideRunTo = 0;
}

// zombies: FUN_0050de30 (SP 0x0050de30)
static bool FUN_0050de30(actor_s *self, bool hadPath)
{
    sentient_s *enemy = Actor_GetTargetSentient(self);
    if ( !enemy )
        return false;
    if ( !hadPath && FUN_007cf300(self) )
        return true;
    int lastKnown = self->sentientInfo[enemy - level.sentients].lastKnownPosTime;
    return lastKnown && level.time - lastKnown < 10000;
}

// zombies: Actor_FindPath (SP 0x005266a0)
static bool Actor_FindPath_SP(actor_s *self, const float *goal, int allowNegotiation, bool ignoreSuppression)
{
    int planeCount = 0;
    // SP 0x00550b20 fills the suppression planes when suppression is enabled.
    float normal[4][2] = {};
    float distance[4] = {};
    if ( self->ent->tagInfo )
    {
        Actor_ClearPath(self);
        return true;
    }
    if ( !Actor_HasPath(self) )
    {
        if ( Actor_PointAt(self->ent->r.currentOrigin, goal) )
            return true;
    }
    else
    {
        if ( !Path_NeedsReevaluation(&self->Path) && Actor_PointAt(self->Path.vFinalGoal, goal) )
            return true;
        Actor_ClearPath(self);
    }
    if ( !ignoreSuppression )
        planeCount = FUN_00550b20(self, normal, distance);
    Sentient_InvalidateNearestNode(self->sentient);
    if ( !planeCount )
    {
        pathnode_t *from = Sentient_NearestNode(self->sentient);
        if ( from )
        {
            Path_FindPathFrom(&self->Path, self->sentient->eTeam, from,
                self->ent->r.currentOrigin, goal, allowNegotiation, self->badPlaceAwareness == 0.0f);
            return Actor_HasPath(self);
        }
    }
    else
    {
        pathnode_t *from = Sentient_NearestNodeSuppressed(self->sentient, normal, distance, planeCount);
        if ( from )
        {
            Path_FindPathFromNotCrossPlanes(&self->Path, self->sentient->eTeam, from,
                self->ent->r.currentOrigin, goal, normal, distance, planeCount,
                allowNegotiation, self->badPlaceAwareness == 0.0f);
            return Actor_HasPath(self);
        }
    }
    return false;
}

// zombies: Actor_PathToGoal / Actor_FindPathToGoalDirectInternal (SP 0x007bd390)
static bool Actor_FindPathToGoalDirectInternal_SP(actor_s *self)
{
    if ( !Actor_HasPath(self) )
    {
        if ( self->meleeAttackDist == 0.0f && Actor_PointAtGoal(self->ent->r.currentOrigin, &self->codeGoal) )
            return true;
    }
    else
    {
        if ( !Path_NeedsReevaluation(&self->Path) && Actor_PointAtGoal(self->Path.vFinalGoal, &self->codeGoal) )
            return true;
        Actor_ClearPath(self);
    }
    float sideMove = fabsf(self->sideMove);
    if ( self->codeGoal.radius - 15.0f < sideMove )
        sideMove = self->codeGoal.radius - 15.0f;
    if ( sideMove <= 0.0f )
        return Actor_FindPath_SP(self, self->codeGoal.pos, 1, false);
    float normal[4][2] = {};
    float distance[4] = {};
    int planeCount = FUN_00550b20(self, normal, distance);
    Sentient_InvalidateNearestNode(self->sentient);
    pathnode_t *from;
    pathnode_t *to;
    pathsort_t nodes[64];
    int nodeCount;
    if ( !planeCount )
    {
        from = Sentient_NearestNode(self->sentient);
        if ( !from )
            return false;
        to = Path_NearestNode(self->codeGoal.pos, nodes, -2, 192.0f, &nodeCount, 64, NEAREST_NODE_DO_HEIGHT_CHECK);
    }
    else
    {
        from = Sentient_NearestNodeSuppressed(self->sentient, normal, distance, planeCount);
        if ( !from )
            return false;
        to = Path_NearestNodeNotCrossPlanes(self->codeGoal.pos, nodes, -2, 192.0f,
            normal, distance, planeCount, &nodeCount, 64, NEAREST_NODE_DO_HEIGHT_CHECK);
    }
    if ( !to )
        return false;
    if ( self->sideMove < 0.0f )
        sideMove = -sideMove;
    float newGoal[3] = {
        self->Path.lookaheadDir[1] * sideMove + self->codeGoal.pos[0],
        -self->Path.lookaheadDir[0] * sideMove + self->codeGoal.pos[1], self->codeGoal.pos[2] };
    team_t team = self->sentient->eTeam;
    float stepheight = self->Physics.prone ? 10.0f : 18.0f;
    pathnode_t *close = Path_FindCloseNode(team, to, newGoal, 1);
    float goal[3];
    Path_PredictionTrace(close->constant.vOrigin, newGoal, ENTITYNUM_NONE, 0x820011, goal, stepheight, 1);
    pathnode_t *end = close;
    if ( !Actor_PointAtGoal(goal, &self->codeGoal) )
    {
        if ( !Actor_PointAtGoal(close->constant.vOrigin, &self->codeGoal) )
        {
            Vec3Copy(self->codeGoal.pos, goal);
            end = to;
        }
        else
            Vec3Copy(close->constant.vOrigin, goal);
    }
    if ( !planeCount )
    {
        Path_FindPathFromTo(&self->Path, team, from, self->ent->r.currentOrigin,
            end, goal, 1, self->badPlaceAwareness == 0.0f);
        return Actor_HasPath(self);
    }
    Path_FindPathFromToNotCrossPlanes(&self->Path, team, from, self->ent->r.currentOrigin,
        end, goal, normal, distance, planeCount, 1, self->badPlaceAwareness == 0.0f);
    return Actor_HasPath(self);
}

// zombies: Actor_FindPathToGoalDirect (SP 0x005fc8b0)
void Actor_FindPathToGoalDirect_SP(actor_s *self)
{
    if ( !Actor_HasPath(self) && level.time < self->pathWaitTime )
    {
        Actor_ClearPileUp(self);
        return;
    }
    bool validPath = Actor_FindPathToGoalDirectInternal_SP(self);
    if ( !Actor_HasPath(self) )
    {
        Actor_ClearPileUp(self);
        if ( !validPath )
            Actor_HandleInvalidPath(self); // SP Actor_BadPath, KB's diagnostic/notify helper
    }
}

// zombies: FUN_0046c830 (SP 0x0046c830)
bool FUN_0046c830(actor_s *self, pathnode_t *node)
{
    bool hadPath = Actor_HasPath(self);
    if ( !hadPath && level.time < self->pathWaitTime )
    {
        Actor_ClearPileUp(self);
        return false;
    }
    bool result = Actor_FindPath_SP(self, node->constant.vOrigin, 1, false);
    if ( !Actor_HasPath(self) )
    {
        Actor_ClearPileUp(self);
        if ( !result )
            Actor_HandleInvalidPath(self); // SP Actor_BadPath; KB name differs.
        return result;
    }
    if ( !hadPath && !((1u << (node->constant.type & 31)) & 0x3c0)
        && Actor_PointNearNode(self->ent->r.currentOrigin, node)
        && Actor_PointAtGoal(self->ent->r.currentOrigin, &self->codeGoal) )
    {
        Actor_ClearPath(self);
        Actor_ClearPileUp(self);
    }
    return true;
}

// zombies: Actor_HandleInvalidPath (SP 0x007bd830)
static int s_hipBranch;
static bool Actor_HandleInvalidPath_SP(actor_s *self, bool canUseEnemyGoal, pathnode_t *node, bool hadPath)
{
    sentient_s *enemy = Actor_GetTargetSentient(self);
    if ( !enemy )
        return false;
    if ( Actor_HasPath(self) && FUN_007bb620(enemy, self) )
    {
        if ( node && Actor_PointNearNode(self->ent->r.currentOrigin, node) )
            return false;
        if ( self->arrivalInfo.animscriptOverrideRunTo
            && Actor_PointNear(self->ent->r.currentOrigin, self->arrivalInfo.animscriptOverrideRunToPos) ) // 0x00555b90
            return false;
        if ( Actor_CanSeeEnemy_SP(self) )
        {
            float lookaheadEnd[2] = {
                self->Path.lookaheadDir[0] * self->Path.fLookaheadDist + self->ent->r.currentOrigin[0],
                self->Path.lookaheadDir[1] * self->Path.fLookaheadDist + self->ent->r.currentOrigin[1] };
            float dx = self->Path.lookaheadDir[0] * self->Path.fLookaheadDist
                + self->ent->r.currentOrigin[0] - enemy->ent->r.currentOrigin[0];
            float dy = self->Path.lookaheadDir[1] * self->Path.fLookaheadDist
                + self->ent->r.currentOrigin[1] - enemy->ent->r.currentOrigin[1];
            if ( self->Path.lookaheadDir[1] * dy + self->Path.lookaheadDir[0] * dx > 0.0f
                && dy * dy + dx * dx < self->Path.fLookaheadDist * self->Path.fLookaheadDist )
            {
                G_SP_PlayTraceInvalidPath(self, enemy, 1, hadPath);
                if ( canUseEnemyGoal && !self->useEnemyGoal )
                {
                    self->useEnemyGoal = true;
                    Actor_UpdateGoalPos_SP(self);
                }
                s_hipBranch = 1; return true;
            }
            // zombies: the exe tests the actor's own origin here, not the enemy's (SP 0x007bd98d: ECX still holds
            // self->ent->r.currentOrigin from 0x007bd8e7, EDX = &codeGoal). With the enemy's origin every zombie
            // within pathEnemyFightDist of a player at its goal lost its path, so the back row stopped at ~58 u.
            if ( Actor_PointAtGoal(self->ent->r.currentOrigin, &self->codeGoal) && Actor_CanSeeEnemy_SP(self) )
            {
                // 0x005b8190 is Vec2Distance; 0x0063fe80 is Path_DistanceGreaterThan.
                if ( !node || Path_DistanceGreaterThan(&self->Path,
                    sqrtf((float)Vec2DistanceSq(enemy->ent->r.currentOrigin, node->constant.vOrigin))) )
                    { G_SP_PlayTraceInvalidPath(self, enemy, 2, hadPath); s_hipBranch = 2; return true; }
            }
            if ( !canUseEnemyGoal || self->useEnemyGoal
                || Actor_PointAtGoal(self->ent->r.currentOrigin, &self->codeGoal)
                || !FUN_007bb6a0(self, enemy->ent->r.currentOrigin, lookaheadEnd, hadPath) )
                return false;
            self->useEnemyGoal = true;
            Actor_UpdateGoalPos_SP(self);
            if ( self->sentient->pClaimedNode
                && Actor_PointAtGoal(self->sentient->pClaimedNode->constant.vOrigin, &self->codeGoal) )
                return node != self->sentient->pClaimedNode;
            if ( node && Actor_PointAtGoal(node->constant.vOrigin, &self->codeGoal) )
                return false;
            s_hipBranch = 3; return true;
        }
    }
    return false;
}

// zombies: FUN_00667c80 (SP 0x00667c80)
static void FUN_00667c80(actor_s *self)
{
    self->useEnemyGoal = false;
    float previous[3];
    Vec3Copy(self->codeGoal.pos, previous);
    Actor_UpdateGoalPos_SP(self);
    self->goalPosChanged = !Vec3Compare(self->codeGoal.pos, previous);
    if ( self->goalPosChanged )
    {
        Scr_Notify(self->ent, scr_const.goal_changed, 0);
        self->commitToFixedNode = false;
    }
    if ( FUN_006067c0(self) )
        return;
    if ( Actor_KeepClaimedNode(self) )
    {
        if ( !Actor_HasPath(self) )
            return;
        Actor_ClearPath(self);
        Actor_TeamMoveBlocked(self); // SP 0x00545b60, level.time+500
        return;
    }
    if ( FUN_00506e40(self) )
        self->Path.flags |= 0x100;
    pathnode_t *node = self->codeGoal.node;
    pathnode_t *oldClaim = self->sentient->pClaimedNode;
    if ( !FUN_007bcbd0(self) && !self->commitToFixedNode )
    {
        Actor_ClearPath(self);
        Actor_TeamMoveBlocked(self);
        return;
    }
    if ( !self->arrivalInfo.animscriptOverrideRunTo )
    {
        if ( !node )
        {
            Actor_FindPathToGoalDirect_SP(self);
            if ( Actor_HasPath(self) && !self->commitToFixedNode )
            {
                self->commitToFixedNode = true;
                Scr_Notify(self->ent, scr_const.start_move, 0); // SP scr_const 0x023a58dc
            }
        }
        else if ( FUN_0046c830(self, node) )
        {
            if ( !self->commitToFixedNode && Actor_HasPath(self) )
            {
                self->commitToFixedNode = true;
                Scr_Notify(self->ent, scr_const.start_move, 0);
            }
            if ( Path_CanClaimNode(node, self->sentient) )
                Sentient_ClaimNode(self->sentient, node);
            goto notify_goal;
        }
        else
            Actor_TeamMoveBlocked(self);
        if ( Actor_HasPath(self) )
            Sentient_ClaimNode(self->sentient, node);
    }
    else
    {
        FUN_007bda90(self);
        if ( node ? Path_CanClaimNode(node, self->sentient) : Actor_HasPath(self) )
            Sentient_ClaimNode(self->sentient, node);
    }
notify_goal:
    if ( !self->goalPosChanged )
    {
        self->goalPosChanged = self->sentient->pClaimedNode && oldClaim != self->sentient->pClaimedNode;
        if ( self->goalPosChanged )
        {
            Scr_Notify(self->ent, scr_const.goal_changed, 0);
            self->commitToFixedNode = false;
        }
    }
}

// zombies: Actor_UpdateGoalPath (SP 0x00488d70)
static void Actor_UpdateGoalPath_SP(actor_s *self)
{
    int oldOverride = self->arrivalInfo.animscriptOverrideRunTo;
    pathnode_t *node = NULL;
    if ( self->ent->tagInfo )
    {
        if ( self->Path.wPathLen == 0 )
            return;
        Actor_ClearPath(self); // SP 0x00485d60 + 0x005a03e0 + dodge-time clear
        return;
    }
    bool hadPath = Actor_HasPath(self);
    if ( !hadPath && level.time < self->iTeamMoveWaitTime )
        return;
    if ( self->badPlaceAwareness > 0.0f && FUN_005a29f0(self->codeGoal.pos) )
    {
        if ( !Actor_HasPath(self) )
            return;
        Actor_ClearPath(self);
        Actor_TeamMoveBlocked(self);
        return;
    }
    if ( self->fixedNode )
    {
        FUN_00667c80(self);
        return;
    }
    bool canUseEnemyGoal = FUN_0050de30(self, hadPath);
    if ( !canUseEnemyGoal )
        self->useEnemyGoal = false;
    float previous[3];
    Vec3Copy(self->codeGoal.pos, previous);
    Actor_UpdateGoalPos_SP(self);
    self->goalPosChanged = !Vec3Compare(self->codeGoal.pos, previous);
    if ( self->goalPosChanged )
        Scr_Notify(self->ent, scr_const.goal_changed, 0);
    if ( Actor_KeepClaimedNode(self) )
    {
        if ( !Actor_HasPath(self) )
            return;
        Actor_ClearPath(self);
        Actor_TeamMoveBlocked(self);
        return;
    }
    if ( self->species == AI_SPECIES_HUMAN )
    {
        if ( FUN_007bb900(self) && !self->arrivalInfo.animscriptOverrideRunTo )
            return;
        if ( FUN_00506e40(self) )
            self->Path.flags |= 0x100;
        node = FUN_007bb500(self);
    }
    pathnode_t *oldClaim = self->sentient->pClaimedNode;
    if ( node && oldOverride && !self->arrivalInfo.animscriptOverrideRunTo )
    {
        if ( oldClaim == node && Actor_HasPath(self)
            && Vec3DistanceSq(self->Path.vFinalGoal, self->arrivalInfo.animscriptOverrideRunToPos) < 1.0f )
            self->arrivalInfo.animscriptOverrideRunTo = 1;
    }
    if ( self->arrivalInfo.animscriptOverrideRunTo )
        FUN_007bda90(self);
    else if ( !node )
    {
        if ( !self->useEnemyGoal )
            Actor_FindPathToGoalDirect_SP(self);
        else
        {
            self->useEnemyGoal = false;
            Actor_UpdateGoalPos_SP(self);
            if ( Actor_HasPath(self) )
            {
                Actor_ClearPath(self);
                Actor_TeamMoveBlocked(self);
            }
        }
    }
    else if ( !FUN_0046c830(self, node) )
    {
        if ( !FUN_0060c1e0(self) )
            Path_MarkNodeInvalid(node, self->sentient->eTeam);
        Actor_TeamMoveBlocked(self);
        node = NULL;
    }
    else if ( !Actor_HasPath(self) && self->useEnemyGoal )
    {
        if ( !Actor_PointAtGoal(self->ent->r.currentOrigin, &self->codeGoal)
            || !Actor_PointNearNode(self->ent->r.currentOrigin, node) )
        {
            self->useEnemyGoal = false;
            Actor_UpdateGoalPos_SP(self);
        }
        Actor_TeamMoveBlocked(self);
    }
    if ( Actor_HandleInvalidPath_SP(self, canUseEnemyGoal, node, hadPath) )
    {
        G_SP_PlayTracePathClear(self, (hadPath ? 40 : 140) + s_hipBranch); s_hipBranch = 0;
        Actor_ClearPath(self);
        Actor_TeamMoveBlocked(self);
        return;
    }
    if ( node || Actor_HasPath(self) )
        Sentient_ClaimNode(self->sentient, node);
    if ( self->goalPosChanged )
        return;
    self->goalPosChanged = self->sentient->pClaimedNode && oldClaim != self->sentient->pClaimedNode;
    if ( self->goalPosChanged )
        Scr_Notify(self->ent, scr_const.goal_changed, 0);
}

// SP offsets are mapped by meaning, never added to a KB actor pointer:
// +224c -> Actor_SP_Ext::allowPitchAngle (no KB actor_s member).
// +d19/+d1a -> ProneInfo.orientPitch/prone; +d1c/+d20 -> iProneTime/iProneTrans;
// +d24/+d28 -> ProneInfo.fBodyHeight/fTorsoPitch. Compare Actor_Dog_Exposed_Start.
// +bac/+bc4 -> eSubState[]/stateLevel (Actor_SetSubState, SP 0x00405550).

// zombies: Actor_Exposed_Start (SP 0x0062a110)
bool __fastcall Actor_Exposed_Start_SP(actor_s *self, ai_state_t ePrevState)
{
    if ( Actor_SP_Ext(self)->allowPitchAngle )
    {
        self->ProneInfo.prone = true;
        self->ProneInfo.orientPitch = true;
        self->ProneInfo.iProneTime = level.time;
        self->ProneInfo.iProneTrans = 500;
    }
    Actor_SetSubState(self, STATE_EXPOSED_COMBAT);
    return true;
}

// zombies: Actor_Exposed_Finish (SP 0x005f8970)
void __fastcall Actor_Exposed_Finish_SP(actor_s *self, ai_state_t eNextState)
{
    if ( Actor_SP_Ext(self)->allowPitchAngle )
    {
        self->ProneInfo.prone = false;
        self->ProneInfo.orientPitch = false;
        self->ProneInfo.fTorsoPitch = 0.0f;
    }
}

// zombies: Actor_Exposed_Resume (SP 0x004544c0)
bool __fastcall Actor_Exposed_Resume_SP(actor_s *self, ai_state_t ePrevState)
{
    // The export says void; 0x004544c7 sets AL=1 before either return path.
    if ( Actor_SP_Ext(self)->allowPitchAngle )
    {
        self->ProneInfo.prone = true;
        self->ProneInfo.orientPitch = true;
        self->ProneInfo.iProneTime = level.time;
        self->ProneInfo.iProneTrans = 500;
    }
    return true;
}

// zombies: G_m_allowpitchangle (SP 0x00444de0)
void __cdecl ActorCmd_AllowPitchAngle_SP(scr_entref_t entref)
{
    if ( !zombiemode->current.enabled )
        return; // MP's existing builtin is METHOD_NULLSUB.
    if ( !entref.classnum && g_entities[entref.entnum].actor )
    {
        actor_s *self = g_entities[entref.entnum].actor;
        if ( !Scr_GetNumParam(SCRIPTINSTANCE_SERVER) )
        {
            Scr_Error("AllowPitchAngle() called without an argument (0 or 1)\n", SCRIPTINSTANCE_SERVER);
            return;
        }
        int allow = Scr_GetInt(0, SCRIPTINSTANCE_SERVER);
        Actor_SP_Ext(self)->allowPitchAngle = allow != 0;
        // SP event 0xc9 is KB EV_ALLOWPITCH. SP entity+bc/+10 are eventParm/pos.trTime.
        // KB's client currently ignores this event; the server flag is still required.
        gentity_s *event = G_TempEntity(vec3_origin, EV_ALLOWPITCH);
        event->s.eventParm = allow;
        event->s.lerp.pos.trTime = self->ent->s.number;
        return;
    }
    Scr_ObjectError("not an actor", SCRIPTINSTANCE_SERVER);
    Scr_Error("AllowPitchAngle() called on non-actor\n", SCRIPTINSTANCE_SERVER);
}
