#pragma once

#include <qcommon/bitarray.h>

struct pmove_t;
struct gjkcc_input_t;
struct trace_t;
struct pml_t;
struct playerState_s;
struct usercmd_s;
struct clientInfo_t;
struct col_context_t;

enum scriptAnimMoveTypes_t : __int32;

#define MASK_PLAYER_FULL 0x3818813

enum PmStanceFrontBack : __int32
{                                       // XREF: PM_Footsteps/r
                                        // PM_GetBobMove/r ...
    PM_STANCE_STAND           = 0x0,
    PM_STANCE_PRONE           = 0x1,
    PM_STANCE_CROUCH          = 0x2,
    PM_STANCE_BACKWARD_FIRST  = 0x3,
    PM_STANCE_BACKWARD_RUN    = 0x3,
    PM_STANCE_BACKWARD_PRONE  = 0x4,
    PM_STANCE_BACKWARD_CROUCH = 0x5,
    NUM_PM_STANCE_FRONTBACK   = 0x6,
};

struct viewLerpWaypoint_s // sizeof=0xC
{                                       // XREF: .data:viewLerp_StandCrouch/r
    int iFrac;
    float fViewHeight;
    int iOffset;
};


void __cdecl setup_gjkcc_input(pmove_t *pm, gjkcc_input_t *gjkcc_in);
void __cdecl PM_trace(
                pmove_t *pm,
                trace_t *results,
                const float *start,
                const float *mins,
                const float *maxs,
                const float *end,
                int passEntityNum,
                int contentMask);
void __cdecl PM_playerTrace(
                pmove_t *pm,
                trace_t *results,
                float *start,
                float *mins,
                float *maxs,
                float *end,
                int passEntityNum,
                int contentMask);
void __cdecl PM_AddEvent(playerState_s *ps, unsigned int newEvent);
void __cdecl PM_AddTouchEnt(pmove_t *pm, int entityNum);
void __cdecl PM_AddTouchGlass(pmove_t *pm, unsigned int glassId);
void __cdecl PM_ClipVelocity(const float *in, const float *normal, float *out);
void __cdecl PM_ProjectVelocity(const float *velIn, const float *normal, float *velOut);
int __cdecl PM_GetEffectiveStance(const playerState_s *ps);
int __cdecl PM_GetSprintLeft(const playerState_s *ps, int gametime);
int __cdecl PM_GetSprintLeftLastTime(const playerState_s *ps);
bool __cdecl PM_IsSprinting(const playerState_s *ps);
bool __cdecl PM_CanStand(playerState_s *ps, pmove_t *pm);
bool __cdecl is_not_penetrating(pmove_t *pm, float *start, float *mins, float *maxs, float *end, int contentMask);
double __cdecl PM_DamageScale_Walk(int damage_timer);
unsigned int __cdecl PM_GroundSurfaceType(playerState_s *ps, pml_t *pml);
int __cdecl PM_GetWaterLevel(const playerState_s *ps, float waterHeight);
int __cdecl PM_GetViewHeightLerpTime(const playerState_s *ps, int iTarget, int bDown);
bool __cdecl PlayerProneAllowed(pmove_t *pm);
void __cdecl PM_FootstepEvent(pmove_t *pm, pml_t *pml, int iOldBobCycle, int iNewBobCycle, int bFootStep);
int __cdecl PM_FootstepType(pmove_t *pm);
bool __cdecl PM_ShouldMakeFootsteps(pmove_t *pm);
void __cdecl PM_UpdateLean(
                playerState_s *ps,
                float msec,
                usercmd_s *cmd,
                void (__cdecl *capsuleTrace)(trace_t *, const float *, const float *, const float *, const float *, int, int, col_context_t *));
void __cdecl PM_UpdateViewAngles_RangeLimited(playerState_s *ps);
void __cdecl PM_UpdateViewAngles(
                int localClientNum,
                playerState_s *ps,
                float msec,
                usercmd_s *cmd,
                unsigned __int8 handler);
void __cdecl PM_UpdateViewLockedEnt(playerState_s *ps, usercmd_s *cmd, unsigned __int8 handler);
void __cdecl PM_UpdateViewAngles_Clamp(playerState_s *ps, usercmd_s *cmd, unsigned __int8 handler);
void __cdecl PM_UpdateViewAngles_LadderClamp(playerState_s *ps);
void __cdecl PM_UpdateViewAngles_Prone(
                playerState_s *ps,
                float msec,
                usercmd_s *cmd,
                unsigned __int8 handler,
                float oldViewYaw);
int __cdecl BG_CheckProneTurned(playerState_s *ps, float newProneYaw, unsigned __int8 handler);
void __cdecl PM_UpdateViewAngles_ProneYawClamp(
                playerState_s *ps,
                float delta,
                int proneBlocked,
                float oldViewYaw,
                float newViewYaw);
void __cdecl PM_UpdateViewAngles_PronePitchClamp(playerState_s *ps);
void __cdecl PM_UpdatePronePitch(pmove_t *pm, pml_t *pml);
void __cdecl PM_SetProneMovementOverride(playerState_s *ps);
void __cdecl PM_MeleeChargeStart(pmove_t *pm, pml_t *pml);
void __cdecl PM_MeleeChargeClear(playerState_s *ps);
bool __cdecl PM_MeleeChargeIsValid(pmove_t *pm, pml_t *pml);
bool __cdecl PM_IsInAir(pmove_t *pm);
void __cdecl PM_CalcMeleeChargeTime(playerState_s *ps, int *outTime, float *outChargeVel);
void __cdecl Pmove_1(pmove_t *pm);
void __cdecl PmoveSingle(pmove_t *pm);
void __cdecl PM_UpdateVisionAnims(pmove_t *pm, pml_t *pml);
void __cdecl PM_UpdateScriptedAnim(pmove_t *pm, pml_t *pml);
void __cdecl PM_UpdateSprint(pmove_t *pm, const pml_t *pml);
void __cdecl PM_StartSprint(playerState_s *ps, pmove_t *pm, const pml_t *pml, int sprintLeft);
void __cdecl PM_EndSprint(playerState_s *ps, pmove_t *pm);
bool __cdecl PM_SprintStartInterferingButtons(const playerState_s *ps, int forwardSpeed, bitarray<51> *button_bits);
bool __cdecl PM_SprintEndingButtons(const pmove_t *pm);
bool __cdecl PM_CanCrouch(playerState_s *ps, pmove_t *pm);
void __cdecl PM_FlyMove(pmove_t *pm, pml_t *pml);
void __cdecl PM_Friction(playerState_s *ps, pml_t *pml, pmove_t *pm);
void __cdecl PM_Accelerate(playerState_s *ps, const pml_t *pml, float *wishdir, float wishspeed, float accel);
double __cdecl PM_MoveScale(playerState_s *ps, float fmove, float rmove, float umove);
double __cdecl PM_CmdScale(playerState_s *ps, usercmd_s *cmd);
void __cdecl PM_AirMove(pmove_t *pm, pml_t *pml);
void __cdecl PM_DoSlideAdjustments(playerState_s *ps, const pml_t *pml);
void __cdecl PM_SetMovementDir(pmove_t *pm, pml_t *pml);
void __cdecl PM_SwimMove(pmove_t *pm, pml_t *pml);
void __cdecl PM_WalkMove(pmove_t *pm, pml_t *pml);
double __cdecl PM_CmdScale_Walk(pmove_t *pm, usercmd_s *cmd);
double __cdecl PM_CmdScaleForStance(const pmove_t *pm);
void __cdecl PM_DeadMove(playerState_s *ps, pml_t *pml);
void __cdecl PM_NoclipMove(pmove_t *pm, pml_t *pml);
void __cdecl PM_UFOMove(pmove_t *pm, pml_t *pml);
void __cdecl PM_GroundTrace(pmove_t *pm, pml_t *pml);
void __cdecl PM_CrashLand(pmove_t *pm, pml_t *pml);
int __cdecl PM_LightLandingForSurface();
int __cdecl PM_MediumLandingForSurface();
unsigned int __cdecl PM_HardLandingForSurface(playerState_s *ps, pml_t *pml);
unsigned int __cdecl PM_DamageLandingForSurface(playerState_s *ps, pml_t *pml);
int __cdecl PM_CorrectAllSolid(pmove_t *pm, pml_t *pml, trace_t *trace);
void __cdecl PM_GroundTraceMissed(pmove_t *pm, pml_t *pml);
void __cdecl PM_SetWaterLevel(pmove_t *pm);
double __cdecl PM_GetViewHeightLerp(const pmove_t *pm, int iFromHeight, int iToHeight);
bool __cdecl PM_IsPlayerFrozenByWeapon(const playerState_s *ps);
void __cdecl PM_CheckDuck(pmove_t *pm, pml_t *pml);
void __cdecl PM_ViewHeightAdjust(pmove_t *pm, pml_t *pml);
double __cdecl PM_ViewHeightTableLerp(int iFrac, viewLerpWaypoint_s *pTable, float *pfPosOfs);
void __cdecl PM_Footsteps_NotMoving(pmove_t *pm, int stance, bool allow_flinch);
int __cdecl PM_Footsteps_TurnAnim(clientInfo_t *ci);
scriptAnimMoveTypes_t __cdecl PM_GetNotMovingAnim(int turnAdjust);
bool __cdecl PM_ShouldFlinch(playerState_s *ps);
void __cdecl PM_VehicleDrive(pmove_t *pm, pml_t *pml);
void __cdecl PM_Footsteps(pmove_t *pm, pml_t *pml);
int __cdecl PM_GetStanceEx(int stance, int backward);
char __cdecl PM_Footstep_LadderMove(pmove_t *pm, pml_t *pml);
double __cdecl PM_GetMaxSpeed(pmove_t *pm, int walking, int sprinting);
double __cdecl PM_GetBobMove(
                pmove_t *pm,
                PmStanceFrontBack stance,
                float xyspeed,
                float fMaxSpeed,
                int walking,
                int sprinting);
void __cdecl PM_Footstep_NotTryingToMove(pmove_t *pm);
int __cdecl BG_CalcBob(pmove_t *pm, pml_t *pml, int old, float bobmove);
void __cdecl PM_ApplyMovementAnimations(
                pmove_t *pm,
                pml_t *pml,
                PmStanceFrontBack stanceFrontBack,
                int walking,
                int sprinting);
int __cdecl PM_GetMoveAnim(playerState_s *ps, pml_t *pml, PmStanceFrontBack stance, int walking, int sprinting);
void __cdecl PM_ApplyLegAnimations(pmove_t *pm, pml_t *pml, int iStance, PmStanceFrontBack stanceFrontBack);
double __cdecl PM_CalcPlayerPitch(playerState_s *ps, pml_t *pml);
void __cdecl PM_FoliageSounds(pmove_t *pm);
void __cdecl PM_DropTimers(playerState_s *ps, pml_t *pml);
void __cdecl PM_UpdatePlayerWalkingFlag(pmove_t *pm);
void __cdecl PM_ClearLadderFlag(playerState_s *ps);
void __cdecl PM_CheckLadderMove(pmove_t *pm, pml_t *pml);
void __cdecl PM_SetLadderFlag(playerState_s *ps);
void __cdecl PM_LadderMove(pmove_t *pm, pml_t *pml);
void __cdecl PM_CheckMeleeCharge(pmove_t *pm, pml_t *pml);
void __cdecl PM_MeleeChargeMove(pmove_t *pm, pml_t *pml);
void __cdecl TurretNVGTrigger(pmove_t *pm);
void __cdecl PM_UpdatePush(pmove_t *pm, pml_t *pml);
void __cdecl set_stance(pmove_t *pm);
void __cdecl Pmove(pmove_t *pm);
