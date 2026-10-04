#pragma once

struct gentity_s;
struct SpawnVar;
void __cdecl SP_info_grenade_hint_SP(gentity_s *ent, SpawnVar *spawnVar);
// SP missile union entity+0x214: predicted landing position; +0x220: prediction time.
// No MP missile_ent_t member represents it. Level init resets all slots; the accessor
// checks birthTime/useCount on entity-slot reuse. SP's writer uses position == vec3_origin
// as its prediction sentinel (populated is only bookkeeping for integrations).
struct actor_grenade_landing_sp_t
{
    float position[3];
    int time;
    bool populated;
    gentity_s *throwbackOwner; // SP missile entity+0x210; no KB missile member
    gentity_s *originalThrowbackOwner; // SP entity+0x27c, cleared by the actor-only reinitializer
};
actor_grenade_landing_sp_t *Actor_SP_GrenadeLandingCache(const gentity_s *grenade);
void Actor_SP_ResetGrenadeLandingCaches();
void Actor_SP_PredictGrenadeLanding(gentity_s *grenade);
void Actor_SP_RegisterExposedDvars();
#include "actor.h"

bool __fastcall Actor_Exposed_Start_SP(actor_s *self, ai_state_t ePrevState);
void __fastcall Actor_Exposed_Finish_SP(actor_s *self, ai_state_t eNextState);
bool __fastcall Actor_Exposed_Resume_SP(actor_s *self, ai_state_t ePrevState);
actor_think_result_t __fastcall Actor_Zombie_Exposed_Think(actor_s *self);
actor_think_result_t __fastcall Actor_Human_Exposed_Think(actor_s *self); // SP 0x004eec10
void __cdecl ActorCmd_AllowPitchAngle_SP(scr_entref_t entref);
// zombies: SP 0x00535680, shared with the custom-anim think (actor_scripted.cpp).
void Actor_UpdateOriginAndAngles_SP(actor_s *self);
void Actor_EventGrenadePing_SP(actor_s *self, gentity_s *grenade);
void Actor_GrenadeBounce_SP(gentity_s *grenade, gentity_s *hitEnt);
// zombies: helpers shared with the SP AI builtins (src/game_sp/g_scr_sp_ai.cpp).
bool Actor_IsUsingTurret_SP(actor_s *self); // SP 0x00473160
bool FUN_0049b0e0(actor_s *self, const float *targetPos, const float *muzzlePos); // SP 0x0049b0e0, canshoot test
pathnode_t *FUN_005b5540(actor_s *self); // SP 0x005b5540, best cover node
bool FUN_005ae040(actor_s *self, pathnode_t *node); // SP 0x005ae040, cover node usable
bool FUN_0046c830(actor_s *self, pathnode_t *node); // SP 0x0046c830, path to node
void FUN_004187b0(actor_s *self); // SP 0x004187b0, detach held grenade
void G_InitActorGrenadeEntity_SP(gentity_s *parent, gentity_s *grenade); // SP 0x00613620 actor branch
bool FUN_0053ac00(actor_s *self, const float *origin, const float *offset, const float *target,
    unsigned int method, float *tossPosition, float *velocity, float randomRange, bool throwback); // SP 0x0053ac00
bool Actor_CanSeePointEx_SP(actor_s *self, const float *point, float fovDot, float maxDistSq,
    int ignoreEntityNum); // SP 0x005e5250
bool turret_CanTargetSentient_SP(gentity_s *self, sentient_s *enemy, float *target, float *source,
    float *localAngles); // SP 0x0042f450
float FX_GetServerVisibility_SP(const float *start, const float *end); // SP 0x0044f050
// zombies: SP think helpers shared with the zombie-dog exposed think (actor_zombie_dog_exposed.cpp).
void Actor_OrientPitchToGround_SP(gentity_s *ent, bool lerp); // SP 0x0063ced0
void Actor_PreThink_SP(actor_s *self); // SP 0x0062bbf0
void Actor_MoveAlongPathWithTeam_SP(actor_s *self, bool run, bool useInterval, bool goalPileUp); // SP 0x00464590
void Actor_PostThink_SP(actor_s *self); // SP 0x0065a350
void Actor_UpdateGoalPos_SP(actor_s *self); // SP 0x0040aaa0
void Actor_FindPathToGoalDirect_SP(actor_s *self); // SP 0x005fc8b0
