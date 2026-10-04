// zombies: lane x3 - the zombie_dog (species 3) exposed think, ported from the SP exe bytes
// (BlackOps.exe, SP 0x004d5950, the state-1 think of AIZombieDogFuncTable 0x00a51ce8).
// It is the MP dog think's shape with SP differences: an omniscient last-known-position refresh of
// every enemy-team sentient before the prethink, a zombietron gate on the enemy goal, no obstacle/jump
// branch (SP's list has no jump script; +0xb8 is turn), SP's attack-range / melee-spot helpers (actor
// trace mask, 72-high hull, no best-fraction fallback), and the SP pathing/think helpers from P6c.
#include "actor_zombie_dog_exposed.h"
#include "actor_zombie_exposed.h"
#include "actor_dog_exposed.h"
#include "actor_exposed.h"
#include "actor_animapi.h"
#include "actor_orientation.h"
#include "actor_senses.h"
#include "actor_navigation.h"
#include "sentient.h"
#include "pathnode.h"
#include "g_debug.h"
#include <game_sp/actor_sp_ext.h>
#include <game_mp/actor_mp.h>
#include <game_mp/g_main_mp.h>
#include <bgame/bg_dog.h>
#include <qcommon/common.h>
#include <universal/com_math.h>
#include <cmath>

extern float dog_max_attack_height; // actor_dog_exposed.cpp, SP .data 0x00b75224 (80)

// SP .rdata 0x00a50c9c.. / 0x00a50ca8..: the SP actor capsule, 72 high (MP 48).
static const float s_zombieDogMeleeMins[3] = { -15.0f, -15.0f, 18.0f };
static const float s_zombieDogMaxs[3] = { 15.0f, 15.0f, 72.0f };
static const float s_zombieDogDropAmount = 90.0f; // SP .data 0x00b75228

// zombies: Actor_SetMeleeAttackSpot (SP 0x007d2540). As MP, except: all three traces use the actor's
// trace mask (SP actor+0xe54 -> Physics.iTraceMask) and the 72-high hull, and a spot whose line to the
// enemy is blocked is never kept as a best-fraction fallback - no free spot fails.
static bool Actor_SetMeleeAttackSpot_SP(actor_s *self, const float *enemyPosition, float *attackPosition)
{
    sentient_s *enemy = Actor_GetTargetSentient(self);
    float dirFromEnemy[2];
    dirFromEnemy[0] = self->ent->r.currentOrigin[0] - enemyPosition[0];
    dirFromEnemy[1] = self->ent->r.currentOrigin[1] - enemyPosition[1];
    Vec2Normalize(dirFromEnemy);
    float dotProducts[4];
    int indices[4];
    for (int i = 0; i < 4; ++i)
    {
        dotProducts[i] = dirFromEnemy[1] * meleeAttackOffsets[i][1] + dirFromEnemy[0] * meleeAttackOffsets[i][0];
        indices[i] = i;
    }
    for (int i = 1; i < 4; ++i)
    {
        int currentIndex = indices[i];
        float currentValue = dotProducts[currentIndex];
        int j;
        for (j = i - 1; j >= 0 && dotProducts[indices[j]] <= currentValue; --j)
            indices[j + 1] = indices[j];
        indices[j + 1] = currentIndex;
    }
    for (int i = 0; i < 4; ++i)
    {
        if (enemy->meleeAttackerSpot[i] == self->ent->s.number)
        {
            enemy->meleeAttackerSpot[i] = 0;
            break;
        }
    }
    attackPosition[2] = enemyPosition[2];
    trace_t trace;
    col_context_t context;
    int i;
    for (i = 0; i < 4; ++i)
    {
        int currentOccupier = enemy->meleeAttackerSpot[indices[i]];
        const float *offset = meleeAttackOffsets[indices[i]];
        attackPosition[0] = offset[0] * self->meleeAttackDist + enemyPosition[0];
        attackPosition[1] = offset[1] * self->meleeAttackDist + enemyPosition[1];
        if (currentOccupier > 0)
        {
            const float *occupier = g_entities[currentOccupier].r.currentOrigin;
            float oy = occupier[1] - attackPosition[1], ox = occupier[0] - attackPosition[0];
            float sy = self->ent->r.currentOrigin[1] - attackPosition[1];
            float sx = self->ent->r.currentOrigin[0] - attackPosition[0];
            if (sy * sy + sx * sx >= oy * oy + ox * ox)
                continue;
        }
        G_TraceCapsule(&trace, attackPosition, s_zombieDogMeleeMins, s_zombieDogMaxs, attackPosition,
            enemy->ent->s.number, self->Physics.iTraceMask, &context);
        if (!trace.startsolid && !trace.allsolid)
            G_TraceCapsule(&trace, attackPosition, s_zombieDogMeleeMins, s_zombieDogMaxs, enemyPosition,
                enemy->ent->s.number, self->Physics.iTraceMask, &context);
        if (trace.fraction != 1.0f || trace.startsolid || trace.allsolid)
        {
            if (ai_debugMeleeAttackSpots->current.enabled)
                G_DebugLine(attackPosition, enemyPosition, colorRed, 0);
            continue;
        }
        float dropPosition[3];
        dropPosition[0] = attackPosition[0];
        dropPosition[1] = attackPosition[1];
        dropPosition[2] = attackPosition[2] - s_zombieDogDropAmount;
        G_TraceCapsule(&trace, attackPosition, s_zombieDogMeleeMins, s_zombieDogMaxs, dropPosition,
            enemy->ent->s.number, self->Physics.iTraceMask, &context);
        if (trace.fraction < 1.0f && !trace.allsolid && !trace.startsolid)
        {
            attackPosition[2] = attackPosition[2] - s_zombieDogDropAmount * trace.fraction + 1.0f;
            enemy->meleeAttackerSpot[indices[i]] = self->ent->s.number;
            break;
        }
        if (ai_debugMeleeAttackSpots->current.enabled)
            G_DebugLine(attackPosition, dropPosition, colorRed, 0);
    }
    if (i == 4)
        return false;
    if (ai_debugMeleeAttackSpots->current.enabled && enemy)
    {
        G_DebugCircle(attackPosition, 15.0f, colorYellow, 0, 1, 0);
        G_AddDebugString(attackPosition, colorYellow, 0.7f, va("%i", self->ent->s.number), 0);
        G_DebugLine(attackPosition, enemyPosition, colorGreen, 0);
    }
    return true;
}

// zombies: Actor_Dog_IsEnemyInAttackRange (SP 0x007d2aa0). SP sets useEnemyGoal/useMeleeAttackSpot
// on entry (MP clears them and sets them only in range); the rest is MP's with the SP melee spot and
// the 72-high hull. Actor_Dog_GetEnemyPos (SP 0x007d2410) matches KB's MP body.
static int Actor_ZombieDog_IsEnemyInAttackRange_SP(actor_s *self, sentient_s *enemy, int *goalPosSet)
{
    float enemyPos[3], attackPos[3];
    float bufferedAttackDist = (float)Actor_Dog_GetEnemyPos(self, enemy, enemyPos);
    *goalPosSet = 0;
    self->useEnemyGoal = 1;
    self->useMeleeAttackSpot = 1;
    const float *origin = self->ent->r.currentOrigin;
    int enemyInAttackRange = Actor_Dog_PointNearAttackPoint(origin, enemyPos, bufferedAttackDist);
    if (Actor_SetMeleeAttackSpot_SP(self, enemyPos, attackPos))
    {
        if (enemyInAttackRange)
        {
            float enemyToAttackSpot[2], enemyToMe[2];
            enemyToAttackSpot[0] = attackPos[0] - enemyPos[0];
            enemyToAttackSpot[1] = attackPos[1] - enemyPos[1];
            Vec2Normalize(enemyToAttackSpot);
            enemyToMe[0] = self->ent->r.currentOrigin[0] - enemyPos[0];
            enemyToMe[1] = self->ent->r.currentOrigin[1] - enemyPos[1];
            Vec2Normalize(enemyToMe);
            enemyInAttackRange = enemyToMe[1] * enemyToAttackSpot[1] + enemyToMe[0] * enemyToAttackSpot[0] > 0.707f;
        }
        // SP inlines Actor_UpdateMeleeGoalPos (codeGoalSrc 3, node/volume cleared, radius 0x005a65d0).
        Actor_UpdateMeleeGoalPos(self, attackPos);
        *goalPosSet = 1;
        return enemyInAttackRange;
    }
    if (!enemyInAttackRange)
        return 0;
    trace_t trace;
    col_context_t context;
    G_TraceCapsule(&trace, self->ent->r.currentOrigin, s_zombieDogMeleeMins, s_zombieDogMaxs, enemyPos,
        enemy->ent->s.number, 0x2820011, &context);
    if (trace.hitType == TRACE_HITTYPE_ENTITY && trace.hitId == enemy->ent->s.number)
        return 1;
    if (trace.fraction < 1.0f || trace.startsolid || trace.allsolid)
        return 0;
    return enemyInAttackRange;
}

// zombies: Actor_FindPathToGoalNearestNode (SP 0x007d2330). As MP, but SP first invalidates the
// actor's nearest-node cache (sentient+0x70 -> bNearestNodeValid) and falls back to SP's direct path.
static void Actor_ZombieDog_FindPathToGoalNearestNode_SP(actor_s *self)
{
    self->sentient->bNearestNodeValid = 0;
    float targetOrigin[3];
    targetOrigin[0] = self->codeGoal.pos[0];
    targetOrigin[1] = self->codeGoal.pos[1];
    targetOrigin[2] = self->codeGoal.pos[2];
    if (self->codeGoalSrc == AI_GOAL_SRC_ENEMY)
        targetOrigin[2] = targetOrigin[2] + 64.0f;
    pathnode_t *nodeTo;
    sentient_s *enemy = Actor_GetTargetSentient(self);
    if (enemy)
        nodeTo = Sentient_NearestNode(enemy); // SP 0x00537ae0
    else
    {
        pathsort_t nodes[64];
        int nodeCount;
        nodeTo = Path_NearestNode(targetOrigin, nodes, -2, self->codeGoal.radius, &nodeCount, 64,
            NEAREST_NODE_DO_HEIGHT_CHECK); // SP 0x00583d10
    }
    if (!nodeTo)
    {
        Actor_FindPathToGoalDirect_SP(self);
        return;
    }
    pathnode_t *nodeFrom = Sentient_NearestNode(self->sentient);
    if (nodeFrom)
        Path_FindPathGetCloseAsPossible(&self->Path, self->sentient->eTeam, nodeFrom, self->ent->r.currentOrigin,
            nodeTo, nodeTo->constant.vOrigin, 1); // SP 0x00642390
}

// zombies: SP 0x007d24e0 - the zombie dog's turn test (MP Actor_Dog_ShouldTurn against the
// zombie_dog list; SP 0x0057bd10 is KB's Actor_Dog_GetDeltaTurnYaw).
static bool Actor_ZombieDog_ShouldTurn_SP(actor_s *self)
{
    AnimScriptList *anims = Actor_SP_GetAnimScriptStorage(AI_SPECIES_ZOMBIE_DOG); // SP 0x01c7a9a4
    if (self->pAnimScriptFunc == &anims->turn && Actor_IsAnimScriptAlive(self) && !self->safeToChangeScript)
        return false;
    if (dog_turn_min_goal_dist->current.value > self->Path.fLookaheadDist) // SP 0x00bcd180
        return false;
    return fabs(Actor_Dog_GetDeltaTurnYaw(self)) > dog_turn90_angle->current.value; // SP 0x00bcd034
}

// zombies: zombie_dog exposed think (SP 0x004d5950).
actor_think_result_t __fastcall Actor_ZombieDog_Exposed_Think(actor_s *self)
{
    AnimScriptList *anims = Actor_SP_GetAnimScriptStorage(AI_SPECIES_ZOMBIE_DOG); // SP 0x01c7a9a4
    int enemyInAttackRange = 0;
    int goalPosSet = 0;
    bool noDodge = true;
    self->pszDebugInfo = "exposed";
    if (self->preThinkTime != level.time) // SP actor+0xbcc
    {
        // Zombie dogs always know where every enemy-team sentient is (SP 0x0048ebe0 -> 0x00698f10 is
        // Actor_UpdateLastKnownPos). SP team table: axis -> allies, allies -> axis.
        static const int enemyTeam[6] = { 0, 2, 1, 0, 0, 0 };
        int team = enemyTeam[self->sentient->eTeam];
        if (team)
        {
            int teamFlags = 1 << team;
            for (sentient_s *other = Sentient_FirstSentient(teamFlags); other; other = Sentient_NextSentient(other, teamFlags))
                Actor_UpdateLastKnownPos(self, other); // SP 0x004deaf0 / 0x005e4100 iterate
        }
    }
    Actor_PreThink_SP(self);
    sentient_s *enemy = Actor_GetTargetSentient(self);
    int attackScriptRunning = self->pAnimScriptFunc == &anims->combat && Actor_IsAnimScriptAlive(self)
        && !self->safeToChangeScript;
    if (!attackScriptRunning)
    {
        Actor_OrientPitchToGround_SP(self->ent, true);
        if (enemy)
            enemyInAttackRange = Actor_ZombieDog_IsEnemyInAttackRange_SP(self, enemy, &goalPosSet);
    }
    // SP 0x0247fde8 is the zombietron dvar (registered at 0x0082bcfa).
    if (!zombietron->current.enabled || !enemy || !Actor_PointAtGoal(enemy->ent->r.currentOrigin, &self->codeGoal))
    {
        self->useEnemyGoal = 0;
        self->useMeleeAttackSpot = 0;
    }
    if (!goalPosSet)
        Actor_UpdateGoalPos_SP(self);
    if (Actor_HasPath(self))
    {
        float dy = self->Path.vFinalGoal[1] - self->codeGoal.pos[1];
        float dx = self->Path.vFinalGoal[0] - self->codeGoal.pos[0];
        if (dy * dy + dx * dx > 225.0f)
            Actor_ClearPath(self);
    }
    if (enemy && !self->useEnemyGoal)
    {
        if (!Actor_HasPath(self))
        {
            Actor_ZombieDog_FindPathToGoalNearestNode_SP(self);
            noDodge = false;
        }
    }
    else
        Actor_FindPathToGoalDirect_SP(self);
    if (!Actor_HasPath(self))
    {
        if (self->useMeleeAttackSpot)
        {
            Actor_UpdateGoalPos_SP(self);
            Actor_FindPathToGoalDirect_SP(self);
        }
        if (enemy && !Actor_HasPath(self))
        {
            Actor_ZombieDog_FindPathToGoalNearestNode_SP(self);
            noDodge = false;
        }
    }
    self->noDodgeMove = noDodge;
    if (self->flashBanged && !(enemy && enemy->syncedMeleeEnt.isDefined() && enemy->syncedMeleeEnt.ent() == self->ent))
    {
        Actor_Exposed_FlashBanged(self); // SP 0x0058c140
        Actor_PostThink_SP(self);
        return ACTOR_THINK_DONE;
    }
    if (attackScriptRunning || enemyInAttackRange)
    {
        if (self->sentient->targetEnt.isDefined())
        {
            if (self->pAnimScriptFunc == &anims->combat && !Actor_IsAnimScriptAlive(self))
                Actor_KillAnimScript(self);
            Actor_SetAnimScript(self, &g_animScriptTable[self->species]->combat, AI_MOVE_STOP,
                AI_ANIM_MOVE_CODE, AI_ANIM_FUNCTION_STOP);
        }
    }
    else if (Actor_HasPath(self))
    {
        if (Actor_IsMoving(self) && Actor_ZombieDog_ShouldTurn_SP(self))
        {
            Actor_SetOrientMode(self, AI_ORIENT_TO_MOTION);
            Actor_SetAnimScript(self, &g_animScriptTable[self->species]->turn, AI_MOVE_RUN, AI_ANIM_MOVE_CODE,
                AI_ANIM_FUNCTION_MOVE);
        }
        else if (self->pAnimScriptFunc != &anims->turn || !Actor_IsAnimScriptAlive(self) || self->safeToChangeScript)
        {
            Actor_SetOrientMode(self, AI_ORIENT_TO_MOTION);
            Actor_MoveAlongPathWithTeam_SP(self, true, false, false);
        }
    }
    else
    {
        self->useMeleeAttackSpot = 0;
        Actor_AnimStop(self, &g_animScriptTable[self->species]->stop);
    }
    Actor_PostThink_SP(self);
    return ACTOR_THINK_DONE;
}
