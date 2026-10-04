#pragma once
#include <bgame/bg_public.h>
#include <bgame/bg_actor_prone.h>
#include "pathnode.h"
#include <clientscript/cscr_animtree.h>
#include "actor_physics.h"
#include "sentient.h"
#include "actor_threat.h"
#include "actor_orientation.h"

enum meansOfDeath_t : __int32
{                                       // XREF: ActorCmd_finishActorDamage/r
                                        // ?CMD_VEH_finishVehicleDamage@@YAXUscr_entref_t@@@Z/r ...
    MOD_UNKNOWN           = 0x0,
    MOD_PISTOL_BULLET     = 0x1,
    MOD_RIFLE_BULLET      = 0x2,
    MOD_GRENADE           = 0x3,
    MOD_GRENADE_SPLASH    = 0x4,
    MOD_PROJECTILE        = 0x5,
    MOD_PROJECTILE_SPLASH = 0x6,
    MOD_MELEE             = 0x7,
    MOD_BAYONET           = 0x8,
    MOD_HEAD_SHOT         = 0x9,
    MOD_CRUSH             = 0xA,
    MOD_TELEFRAG          = 0xB,
    MOD_FALLING           = 0xC,
    MOD_SUICIDE           = 0xD,
    MOD_TRIGGER_HURT      = 0xE,
    MOD_EXPLOSIVE         = 0xF,
    MOD_IMPACT            = 0x10,
    MOD_BURNED            = 0x11,
    MOD_HIT_BY_OBJECT     = 0x12,
    MOD_DROWN             = 0x13,
    MOD_GAS               = 0x14,
    MOD_NUM               = 0x15,
};

enum actor_think_result_t : __int32
{                                       // XREF: ?Actor_Think@@YAXPAUgentity_s@@@Z/r
                                        // Actor_CallThink/r
    ACTOR_THINK_DONE               = 0x0,
    ACTOR_THINK_REPEAT             = 0x1,
    ACTOR_THINK_MOVE_TO_BODY_QUEUE = 0x2,
};

// zombies: P6a - the SP exe's four species (MEASURED: the `type` setter 0x007c1100 loops 4 over
// .data 0x00b74f14, filled by the scr_const init at 0x005eba72 with human, dog, zombie, zombie_dog;
// AIFuncTable 0x00b750d8 has 4 rows). The MP exe had one species, the dog, numbered 0.
enum AISpecies : __int32
{                                       // XREF: actor_s/r
    AI_SPECIES_HUMAN      = 0x0,
    AI_SPECIES_DOG        = 0x1,
    AI_SPECIES_ZOMBIE     = 0x2,
    AI_SPECIES_ZOMBIE_DOG = 0x3,
    MAX_AI_SPECIES        = 0x4,
    AI_SPECIES_ALL        = 0x4,
};

// zombies: P6a - Actor_IsDogSpecies (SP 0x005886b0): species 1 || 3. Replaces the MP dog test,
// which the MP linker COMDAT-folded into a `return 1` (see Flame_GetLocalClientSourceRange).
inline bool Actor_IsDogSpecies(int species)
{
    return species == AI_SPECIES_DOG || species == AI_SPECIES_ZOMBIE_DOG;
}
inline AISpecies &operator++(AISpecies &t)
{
    t = static_cast<AISpecies>((static_cast<int>(t) + 1));
    return t;
}
inline AISpecies operator++(AISpecies &t, int)
{
    AISpecies old = t;
    t = static_cast<AISpecies>((static_cast<int>(t) + 1));
    return old;
}

enum ai_traverse_mode_t : __int32
{                                       // XREF: actor_s/r
    AI_TRAVERSE_INVALID   = 0x0,
    AI_TRAVERSE_GRAVITY   = 0x1,
    AI_TRAVERSE_NOGRAVITY = 0x2,
    AI_TRAVERSE_NOCLIP    = 0x3,
    AI_TRAVERSE_COUNT     = 0x4,
};

enum ai_animmode_t : __int32
{                                       // XREF: actor_s/r actor_s/r ...
    AI_ANIM_UNKNOWN                = 0x0,
    AI_ANIM_MOVE_CODE              = 0x1,
    AI_ANIM_USE_POS_DELTAS         = 0x2,
    AI_ANIM_USE_ANGLE_DELTAS       = 0x3,
    AI_ANIM_USE_BOTH_DELTAS        = 0x4,
    AI_ANIM_USE_BOTH_DELTAS_NOCLIP = 0x5,
    AI_ANIM_USE_BOTH_DELTAS_NOGRAVITY = 0x6,
    AI_ANIM_USE_BOTH_DELTAS_ZONLY_PHYSICS = 0x7,
    AI_ANIM_NOPHYSICS              = 0x8,
    AI_ANIM_POINT_RELATIVE         = 0x9,
};

enum aiGoalSources : __int32
{                                       // XREF: actor_s/r
    AI_GOAL_SRC_SCRIPT_GOAL        = 0x0,
    AI_GOAL_SRC_SCRIPT_ENTITY_GOAL = 0x1,
    AI_GOAL_SRC_FRIENDLY_CHAIN     = 0x2,
    AI_GOAL_SRC_ENEMY              = 0x3,
};

enum ai_state_t : __int32
{                                       // XREF: ai_transition_cmd_t/r
                                        // actor_s/r ...
    AIS_INVALID          = 0x0,         // XREF: .rdata:g_eSupercedingStates/s
                                        // .rdata:g_eSupercededStates/s
    AIS_KEEPCURRENT      = 0x0,
    AIS_EXPOSED          = 0x1,
    AIS_TURRET           = 0x2,
    AIS_GRENADE_RESPONSE = 0x3,
    AIS_BADPLACE_FLEE    = 0x4,
    AIS_COVERARRIVAL     = 0x5,
    AIS_DEATH            = 0x6,
    AIS_DEFAULT          = 0x1,
    AIS_SETABLE_FIRST    = 0x1,
    AIS_SETABLE_LAST     = 0x6,
    AIS_PAIN             = 0x7,         // XREF: .rdata:g_eSupercedingStates/s
                                        // .rdata:g_eSupercededStates/s
    AIS_REACT            = 0x8,         // XREF: .rdata:g_eSupercededStates/s
    AIS_SCRIPTEDANIM     = 0x9,         // XREF: .rdata:g_eSupercedingStates/s
                                        // .rdata:g_eSupercededStates/s
    AIS_CUSTOMANIM       = 0xA,         // XREF: .rdata:g_eSupercededStates/s
    AIS_NEGOTIATION      = 0xB,         // XREF: .rdata:g_eSupercedingStates/s
                                        // .rdata:g_eSupercededStates/s
    AIS_PUSHABLE_FIRST   = 0x7,
    AIS_PUSHABLE_LAST    = 0xB,
    AIS_COUNT            = 0xC,
};

enum ai_substate_t : __int32
{                                       // XREF: actor_s/r
                                        // ?Actor_SetSubState@@YIXPAUactor_s@@W4ai_substate_t@@@Z/r
    STATE_EXPOSED_COMBAT         = 0x64,
    STATE_EXPOSED_NONCOMBAT      = 0x65,
    STATE_EXPOSED_REACQUIRE_MOVE = 0x66,
    STATE_EXPOSED_FLASHBANGED    = 0x67,
    STATE_DEATH_PRECLEANUP       = 0xC8,
    STATE_DEATH_POSTCLEANUP      = 0xC9,
    STATE_GRENADE_FLEE           = 0x12C,
    STATE_GRENADE_TAKECOVER      = 0x12D,
    STATE_GRENADE_COWER          = 0x12E,
    STATE_GRENADE_COMBAT         = 0x12F,
    STATE_GRENADE_COVERATTACK    = 0x130,
    STATE_GRENADE_ACQUIRE        = 0x131,
    STATE_GRENADE_THROWBACK      = 0x132,
};

enum ai_state_transition_t : __int32
{                                       // XREF: ai_transition_cmd_t/r
                                        // Actor_SimplifyStateTransitions/r ...
    AIS_TRANSITION_CANONICAL = 0xFFFFFFFF,
                                        // XREF: .rdata:g_eSimplificationRules/s
    AIS_TRANSITION_NONE      = 0x0,     // XREF: .rdata:g_eSimplificationRules/s
    AIS_TRANSITION_SET       = 0x1,     // XREF: .rdata:g_eSimplificationRules/s
    AIS_TRANSITION_PUSH      = 0x2,
    AIS_TRANSITION_POP       = 0x3,     // XREF: .rdata:g_eSimplificationRules/s
};

struct ai_transition_cmd_t // sizeof=0x8
{                                       // XREF: actor_s/r
    ai_state_transition_t eTransition;
    ai_state_t eState;
};



struct ActorCachedInfo // sizeof=0x1C
{                                       // XREF: actor_s/r actor_s/r
    int time;
    float pos[3];
    float dir[3];
};

struct ActorLookAtInfo // sizeof=0x34
{                                       // XREF: actor_s/r
    float vLookAtPos[3];
    float fLookAtTurnAngle;
    float fLookAtTurnSpeed;
    float fLookAtTurnAccel;
    float fLookAtAnimYawLimit;
    float fLookAtYawLimit;
    unsigned __int16 animLookAtStraight;
    unsigned __int16 animLookAtLeft;
    unsigned __int16 animLookAtRight;
    bool bDoLookAt;
    bool bLookAtSetup;
    int iLookAtBlendEndTime;
    float fLookAtAnimBlendRate;
    float fLookAtLimitBlendRate;
};

struct actor_goal_s // sizeof=0x28
{                                       // XREF: actor_s/r actor_s/r ...
    float pos[3];
    float ang[3];
    float radius;
    float height;
    pathnode_t *node;
    gentity_s *volume;
};

struct ActorCoverArrivalInfo // sizeof=0x20
{                                       // XREF: actor_s/r
    int arrivalNotifyRequested;
    int animscriptOverrideRunTo;
    float animscriptOverrideRunToPos[3];
    float animscriptOverrideOriginError[3];
};

//enum $D416C61A81CE0211A2B0E6C3C6220A84 : __int32
enum ai_movemode_t : unsigned __int8 // not a real name
{
    AI_MOVE_STOP      = 0x0,
    AI_MOVE_STOP_SOON = 0x1,
    AI_MOVE_WALK      = 0x2,
    AI_MOVE_RUN       = 0x3,
};

// mod: actor_s keeps the retail 48 sentient_info_t in place; sentient numbers 48 and up (only with
// bo1_mod_maxactors > 32) live in a side table (Actor_SentientInfoExt, actor_mp.cpp). Same size/layout as [48].
sentient_info_t *__cdecl Actor_SentientInfoExt(const void *inlineArray, int index);
struct actor_sentient_info_array
{
    sentient_info_t inl[48];
    sentient_info_t &operator[](int i) { return i < 48 ? inl[i] : *Actor_SentientInfoExt(this, i); }
    const sentient_info_t &operator[](int i) const { return i < 48 ? inl[i] : *Actor_SentientInfoExt(this, i); }
};

struct actor_s // sizeof=0x2780
{                                       // XREF: .data:actor_s * g_actors/r
                                        // actor_array_t/r ...
    gentity_s *ent;
    sentient_s *sentient;
    AISpecies species;
    int legsAnim;
    int legsTimer;
    int legsAnimDuration;
    ai_state_t eState[6];
    ai_substate_t eSubState[6];
    unsigned int stateLevel;
    int iStateTime;
    int preThinkTime;
    ai_transition_cmd_t StateTransitions[13];
    unsigned int transitionCount;
    ai_state_t eSimulatedState[6];
    unsigned int simulatedStateLevel;
    int deathContents;
    int iPainTime;
    bool allowPain;
    bool allowDeath;
    bool delayedDeath;
    bool provideCoveringFire;
    int lastShotTime;
    unsigned __int16 weaponName;
    // padding byte
    // padding byte
    float fLookPitch;
    float fLookYaw;
    float vLookForward[3];
    float vLookRight[3];
    float vLookUp[3];
    ai_orient_t CodeOrient;
    ai_orient_t ScriptOrient;
    float fDesiredBodyYaw;
    float fInvProneAnimLowPitch;
    float fInvProneAnimHighPitch;
    float fProneLastDiff;
    int bProneOK;
    actor_prone_info_s ProneInfo;
    ActorCachedInfo eyeInfo;
    ActorCachedInfo muzzleInfo;
    ActorLookAtInfo lookAtInfo;
    float accuracy;
    float playerSightAccuracy;
    unsigned int missCount;
    unsigned int hitCount;
    float debugLastAccuracy;
    float debugWeaponAccuracy;
    int iTraceCount;
    int iDamageTaken;
    int iDamageYaw;
    float damageDir[3];
    unsigned __int16 damageHitLoc;
    unsigned __int16 damageWeapon;
    unsigned __int16 damageMod;
    // padding byte
    // padding byte
    ai_stance_e eAllowedStances;
    unsigned int AnimScriptHandle; // mod: was unsigned __int16 + 2 padding bytes (same offset and size); server thread ids pass 0xFFFF with bo1_mod_scriptvars 2+
    scr_animscript_t *pAnimScriptFunc;
    scr_animscript_t AnimScriptSpecific;
    ai_traverse_mode_t eTraverseMode;
    ai_movemode_t moveMode;
    bool safeToChangeScript;
    bool bUseGoalWeight;
    // padding byte
    ai_animmode_t eAnimMode;
    ai_animmode_t eScriptSetAnimMode;
    float fAnimTranslationScale;
    actor_physics_t Physics;
    path_t Path;
    float fWalkDist;
    path_trim_t TrimInfo;
    float fInterval;
    int pathWaitTime;
    int iTeamMoveWaitTime;
    int iTeamMoveDodgeTime;
    actor_s *pPileUpActor;
    gentity_s *pPileUpEnt;
    int bDontAvoidPlayer;
    float sideMove;
    unsigned __int8 keepClaimedNode;
    unsigned __int8 keepClaimedNodeInGoal;
    bool noDodgeMove;
    // padding byte
    int mayMoveTime;
    float prevMoveDir[2];
    float leanAmount;
    bool isFacingMotion;
    // padding byte
    // padding byte
    // padding byte
    int lastPathToEnemyTime;
    int bNotifyTurnDone;
    int exposedStartTime;
    int exposedDuration;
    actor_goal_s codeGoal;
    aiGoalSources codeGoalSrc;
    actor_goal_s scriptGoal;
    EntHandle scriptGoalEnt;
    float pathEnemyLookahead;
    float pathEnemyFightDist;
    float meleeAttackDist;
    bool useEnemyGoal;
    bool useMeleeAttackSpot;
    bool goalPosChanged;
    bool commitToFixedNode;
    bool ignoreForFixedNodeSafeCheck;
    bool fixedNode;
    // padding byte
    // padding byte
    float fixedNodeSafeRadius;
    float fixedNodeSafeVolumeRadiusSq;
    EntHandle fixedNodeSafeVolume;
    int bPacifist;
    int iPacifistWait;
    int numCoverNodesInGoal;
    int iPotentialCoverNodeCount;
    pathnode_t *pPotentialReacquireNode[10];
    int iPotentialReacquireNodeCount;
    ActorCoverArrivalInfo arrivalInfo;
    float fovDot;
    float fMaxSightDistSqrd;
    int ignoreCloseFoliage;
    actor_sentient_info_array sentientInfo; // mod: retail sentient_info_t sentientInfo[48]
    SentientHandle pFavoriteEnemy;
    int talkToSpecies;
    float lastEnemySightPos[3];
    bool lastEnemySightPosValid;
    // padding byte
    // padding byte
    // padding byte
    float anglesToLikelyEnemyPath[3];
    int faceLikelyEnemyPathNeedCheckTime;
    int faceLikelyEnemyPathNeedRecalculateTime;
    const pathnode_t *faceLikelyEnemyPathNode;
    int ignoreSuppression;
    int suppressionWait;
    int suppressionDuration;
    int suppressionStartTime;
    float suppressionMeter;
    potential_threat_t potentialThreat;
    int threatUpdateTime;
    int hasThreateningEnemy;
    float grenadeAwareness;
    int bThrowbackGrenades;
    EntHandle pGrenade;
    int iGrenadeWeaponIndex;
    unsigned __int16 GrenadeTossMethod;
    // padding byte
    // padding byte
    int bGrenadeTossValid;
    int bGrenadeTargetValid;
    int iGrenadeAmmo;
    float vGrenadeTossPos[3];
    float vGrenadeTargetPos[3];
    float vGrenadeTossVel[3];
    int bDropWeapon;
    int bDrawOnCompass;
    int bActivateCrosshair;
    bool ignoreTriggers;
    bool pushable;
    bool noActorCollision; // zombies: mod field "noactorcollision" (horde mod), was a padding byte; 0 = retail
    // padding byte
    int inuse;
    ai_badplace_t aiBadPlace;
    float badPlaceAwareness;
    float goodShootPos[3];
    int goodShootPosValid;
    unsigned __int16 scriptState;
    unsigned __int16 lastScriptState;
    unsigned __int16 stateChangeReason;
    EntHandle pCloseEnt;
    // padding byte
    // padding byte
    int moveHistoryIndex;
    float moveHistory[10][2];
    float yawVeloc;
    int flashBanged;
    float flashBangedStrength;
    int flashBangImmunity;
    const char *pszDebugInfo;
    pathnode_t *pPotentialCoverNode[1000];
    int ikPriority;
};

// zombies: P6a - SP's row is 8 pointers (0x20): the 8th is pfnReact
// `void (__fastcall *)(actor_s *, gentity_s *attacker, const float *dir)`, dispatched by the actor
// entity handler's `react` slot, Actor_React 0x004b40a0 (entityHandlers row .data 0x00b75650 +0x18).
// Only SP humans fill it (0x005d2db0, an empty `ret 4`); dog / zombie / zombie_dog rows are NULL.
// Not added here: the struct stays MP-sized (no layout changes) and nothing in KB calls it yet.
struct ai_funcs_t // sizeof=0x1C
{                                       // XREF: .rdata:AIDogFuncTable/r
    bool (__fastcall *pfnStart)(actor_s *, ai_state_t);
    void (__fastcall *pfnFinish)(actor_s *, ai_state_t);
    void (__fastcall *pfnSuspend)(actor_s *, ai_state_t);
    bool (__fastcall *pfnResume)(actor_s *, ai_state_t);
    actor_think_result_t (__fastcall *pfnThink)(actor_s *);
    void (__fastcall *pfnTouch)(actor_s *, gentity_s *);
    void (__fastcall *pfnPain)(actor_s *, gentity_s *, int, const float *, const int, const float *, const hitLocation_t);
};

constexpr float actorMins[3] = { -15.0, -15.0, 0.0 };
constexpr float actorMaxs[3] = { 15.0, 15.0, 48.0 };

constexpr float meleeAttackOffsets[4][2] = { { 1.0, 0.0 }, { 0.0, 1.0 }, { -1.0, 0.0 }, { 0.0, -1.0 } };
//constexpr float ACTOR_EYE_OFFSET = 64.0f; // in actor_mp.h


extern const unsigned __int16 *g_AISpeciesNames[MAX_AI_SPECIES];

extern const ai_funcs_t *AIFuncTable[MAX_AI_SPECIES];