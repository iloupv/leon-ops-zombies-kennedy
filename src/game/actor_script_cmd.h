#pragma once
#include <clientscript/cscr_variable.h>
#include "actor.h"

enum DEBUGMAYMOVE_LIFT_ENUM : __int32
{                                       // XREF: DEBUGMAYMOVE/r
    DEBUGMAYMOVE_NOT_LIFTED = 0x0,
    DEBUGMAYMOVE_LIFTED     = 0x1,
};

struct entityHandler_t // sizeof=0x30
{                                       // XREF: .data:entityHandler_t * entityHandlers/r
    void(__cdecl *think)(gentity_s *); // XREF: G_RunThink(gentity_s *)+41/r
    void(__cdecl *reached)(gentity_s *);
    void(__cdecl *blocked)(gentity_s *, gentity_s *);
    void(__cdecl *touch)(gentity_s *, gentity_s *, int);
    void(__cdecl *use)(gentity_s *, gentity_s *, gentity_s *);
    void(__cdecl *pain)(gentity_s *, gentity_s *, int, const float *, const int, const float *, const hitLocation_t, const int);
    void(__cdecl *react)(gentity_s *, gentity_s *, const float *);
    void(__cdecl *die)(gentity_s *, gentity_s *, gentity_s *, int, int, const int, const float *, const hitLocation_t, int);
    void(__cdecl *entinfo)(gentity_s *, float *);
    void(__cdecl *controller)(const gentity_s *, int *);
    // G_DObjCalcBone(gentity_s const *,int)+56/r
    int methodOfDeath;                  // XREF: G_ExplodeMissile(gentity_s *)+C03/r
    int splashMethodOfDeath;            // XREF: GetSplashMethodOfDeath+B5/r
};

void __cdecl ActorCmd_StartCoverArrival(scr_entref_t entref);
actor_s *__fastcall Actor_Get(scr_entref_t entref);
void __fastcall Actor_StartArrivalState(actor_s *self, ai_state_t newState);
void __cdecl ActorCmd_StartTraverseArrival(scr_entref_t entref);
void __cdecl ActorCmd_Melee(scr_entref_t entref);
void __cdecl ActorCmd_ReacquireStep(scr_entref_t entref);
void __cdecl ActorCmd_FindReacquireNode(scr_entref_t entref);
void __cdecl ActorCmd_GetReacquireNode(scr_entref_t entref);
void __cdecl ActorCmd_UseReacquireNode(scr_entref_t entref);
void __cdecl ActorCmd_FindReacquireDirectPath(scr_entref_t entref);
void __cdecl ActorCmd_FindReacquireProximatePath(scr_entref_t entref);
void __cdecl ActorCmd_TrimPathToAttack(scr_entref_t entref);
void __cdecl ActorCmd_ReacquireMove(scr_entref_t entref);
void __cdecl ActorCmd_FlagEnemyUnattackable(scr_entref_t entref);
void __cdecl ActorCmd_ClearPitchOrient(scr_entref_t entref);
void __cdecl ActorCmd_SetLookAtAnimNodes(scr_entref_t entref);
void __cdecl ActorCmd_SetLookAt(scr_entref_t entref);
void __cdecl ActorCmd_SetLookAtYawLimits(scr_entref_t entref);
void __cdecl ActorCmd_StopLookAt(scr_entref_t entref);
void __cdecl ActorCmd_CanSee(scr_entref_t entref);
void __cdecl ActorCmd_MayMoveToPoint(scr_entref_t entref);
void __cdecl DEBUGMAYMOVE(
                const float *vec1,
                const float *vec2,
                const float *color,
                DEBUGMAYMOVE_LIFT_ENUM liftBehavior);
int __cdecl MayMove_TraceCheck(actor_s *self, float *vStart, float *vEnd, int allowStartSolid, int checkDrop);
void __cdecl ActorCmd_MayMoveFromPointToPoint(scr_entref_t entref);
void __cdecl ActorCmd_Teleport(scr_entref_t entref);
void __cdecl ActorCmd_ForceTeleport(scr_entref_t entref);
void __cdecl ActorCmd_WithinApproxPathDist(scr_entref_t entref);
void __cdecl ActorCmd_IsPathDirect(scr_entref_t entref);
void __cdecl ActorCmd_AllowedStances(scr_entref_t entref);
void __cdecl ActorCmd_IsStanceAllowed(scr_entref_t entref);
void __cdecl ActorCmd_Knockback(scr_entref_t entref);
void __cdecl ActorCmd_TraverseMode(scr_entref_t entref);
void __cdecl ActorCmd_AnimMode(scr_entref_t entref);
void __cdecl ActorCmd_OrientMode(scr_entref_t entref);
void __cdecl ActorCmd_GetMotionAngle(scr_entref_t entref);
void __cdecl ActorCmd_GetAnglesToLikelyEnemyPath(scr_entref_t entref);
void __cdecl ActorCmd_LerpPosition(scr_entref_t entref);
void __cdecl ActorCmd_PredictOriginAndAngles(scr_entref_t entref);
void __cdecl ActorCmd_PredictAnim(scr_entref_t entref);
void __cdecl ActorCmd_GetHitEntType(scr_entref_t entref);
void __cdecl Actor_GetEntType(int entnum);
void __cdecl ActorCmd_GetHitYaw(scr_entref_t entref);
void __cdecl ActorCmd_GetGroundEntType(scr_entref_t entref);
void __cdecl ActorCmd_IsDeflected(scr_entref_t entref);
void __cdecl ScrCmd_animcustom(scr_entref_t entref);
void __cdecl ScrCmd_CanAttackEnemyNode(scr_entref_t entref);
void ScrCmd_GetPathLength(scr_entref_t entref); // ADD
void ScrCmd_CalcPathLength(scr_entref_t entref); // ADD
void __cdecl ScrCmd_GetNegotiationStartNode(scr_entref_t entref);
void __cdecl ScrCmd_GetNegotiationEndNode(scr_entref_t entref);
void __cdecl ActorCmd_CheckProne(scr_entref_t entref);
void __cdecl ActorCmd_PushPlayer(scr_entref_t entref);
void __cdecl ActorCmd_SetGoalNode(scr_entref_t entref);
void __fastcall Actor_SetScriptGoalPos(actor_s *self, const float *vGoalPos, pathnode_t *node, float *ang);
void __cdecl ActorCmd_SetGoalPos(scr_entref_t entref);
void __cdecl ActorCmd_SetGoalEntity(scr_entref_t entref);
void __fastcall Actor_SetScriptGoalEntity(actor_s *self, gentity_s *pGoalEnt);
void __cdecl ActorCmd_SetGoalVolume(scr_entref_t entref);
void __fastcall Actor_SetScriptGoalVolume(actor_s *self, gentity_s *volume);
void __cdecl ActorCmd_GetGoalVolume(scr_entref_t entref);
void __cdecl ActorCmd_ClearGoalVolume(scr_entref_t entref);
void __cdecl ActorCmd_SetFixedNodeSafeVolume(scr_entref_t entref);
void __cdecl ActorCmd_GetFixedNodeSafeVolume(scr_entref_t entref);
void __cdecl ActorCmd_ClearFixedNodeSafeVolume(scr_entref_t entref);
void __cdecl ActorCmd_IsInGoal(scr_entref_t entref);
void __cdecl ActorCmd_SetOverrideRunToPos(scr_entref_t entref);
void __cdecl ActorCmd_ClearOverrideRunToPos(scr_entref_t entref);
void __cdecl ActorCmd_NearNode(scr_entref_t entref);
void __cdecl ActorCmd_ClearEnemy(scr_entref_t entref);
void __cdecl ActorCmd_SetEntityTarget(scr_entref_t entref);
void __cdecl ActorCmd_ClearEntityTarget(scr_entref_t entref);
void __cdecl ActorCmd_ClearEntityOwner(scr_entref_t entref);
void __cdecl ActorCmd_SetEntityOwner(scr_entref_t entref);
void __cdecl ActorCmd_SetPotentialThreat(scr_entref_t entref);
void __cdecl ActorCmd_ClearPotentialThreat(scr_entref_t entref);
void __cdecl ActorCmd_GetPerfectInfo(scr_entref_t entref);
void __cdecl ActorCmd_SetFlashBanged(scr_entref_t entref);
void __cdecl ActorCmd_SetFlashbangImmunity(scr_entref_t entref);
void __cdecl ActorCmd_GetFlashBangedStrength(scr_entref_t entref);
void __cdecl ActorCmd_IsKnownEnemyInRadius(scr_entref_t entref);
void __cdecl ActorCmd_IsKnownEnemyInVolume(scr_entref_t entref);
void __cdecl ActorCmd_SetTalkToSpecies(scr_entref_t entref);
void __cdecl ActorCmd_GetDeltaTurnYaw(scr_entref_t entref);
void __cdecl ActorCmd_SetAnimState(scr_entref_t entref);
void __cdecl ActorCmd_SetAimAnimWeights(scr_entref_t entref);
void __cdecl ActorCmd_finishActorDamage(scr_entref_t entref);
//void (__cdecl *__cdecl Actor_GetMethod(const char **pName))(scr_entref_t);
void(__cdecl *__cdecl Actor_GetMethod(const char **pName))(scr_entref_t);


extern entityHandler_t entityHandlers[31];