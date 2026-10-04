#pragma once
#include <cgame/cg_shellshock.h>
#include "bg_actor_prone.h"
#include <qcommon/ent.h>

struct gitem_s;
struct WeaponDef;
struct playerState_s;
struct entityState_s;
struct trajectory_t;
struct PlayerHeldWeapon;
struct XAnim_s;
struct hudelem_s;
union hudelem_color_t;

#define EVENT_PARM_MAX 2047

enum entity_event_t : __int32
{
    EV_NONE                        = 0x0,
    EV_FOLIAGE_SOUND               = 0x1,
    EV_STOP_WEAPON_SOUND           = 0x2,
    EV_STOP_SOUND_ALIAS            = 0x3,
    EV_SOUND_ALIAS                 = 0x4,
    EV_SOUND_ALIAS_NOTIFY          = 0x5,
    EV_SOUND_BATTLECHAT_ALIAS      = 0x6,
    EV_STOPSOUNDS                  = 0x7,
    EV_STANCE_FORCE_STAND          = 0x8,
    EV_STANCE_FORCE_CROUCH         = 0x9,
    EV_STANCE_FORCE_PRONE          = 0xA,
    EV_ITEM_PICKUP                 = 0xB,
    EV_AMMO_PICKUP                 = 0xC,
    EV_NOAMMO                      = 0xD,
    EV_NOAMMO_LEFT                 = 0xE,
    EV_EMPTYCLIP                   = 0xF,
    EV_EMPTY_OFFHAND               = 0x10,
    EV_RESET_ADS                   = 0x11,
    EV_RELOAD                      = 0x12,
    EV_RELOAD_FROM_EMPTY           = 0x13,
    EV_RELOAD_START                = 0x14,
    EV_RELOAD_END                  = 0x15,
    EV_RELOAD_START_NOTIFY         = 0x16,
    EV_RELOAD_ADDAMMO              = 0x17,
    EV_RAISE_WEAPON                = 0x18,
    EV_FIRST_RAISE_WEAPON          = 0x19,
    EV_PUTAWAY_WEAPON              = 0x1A,
    EV_WEAPON_ALT                  = 0x1B,
    EV_PULLBACK_WEAPON             = 0x1C,
    EV_FIRE_WEAPON                 = 0x1D,
    EV_FIRE_WEAPON_LASTSHOT        = 0x1E,
    EV_FIRE_WEAPON_LEFT            = 0x1F,
    EV_FIRE_WEAPON_LASTSHOT_LEFT   = 0x20,
    EV_RECHAMBER_WEAPON            = 0x21,
    EV_EJECT_BRASS                 = 0x22,
    EV_MELEE_SWIPE                 = 0x23,
    EV_FIRE_MELEE                  = 0x24,
    EV_WEAPON_DEPLOYING            = 0x25,
    EV_WEAPON_FINISH_DEPLOYING     = 0x26,
    EV_WEAPON_BREAKING_DOWN        = 0x27,
    EV_WEAPON_FINISH_BREAKING_DOWN = 0x28,
    EV_PREP_OFFHAND                = 0x29,
    EV_USE_OFFHAND                 = 0x2A,
    EV_SWITCH_OFFHAND              = 0x2B,
    EV_MELEE_HIT                   = 0x2C,
    EV_MELEE_MISS                  = 0x2D,
    EV_MELEE_BLOOD                 = 0x2E,
    EV_FIRE_WEAPON_MG42            = 0x2F,
    EV_FIRE_WEAPON_MG42A           = 0x30,
    EV_FIRE_QUADBARREL_1           = 0x31,
    EV_FIRE_QUADBARREL_2           = 0x32,
    EV_BULLET_HIT                  = 0x33,
    EV_BULLET_HIT_CLIENT_SMALL     = 0x34,
    EV_BULLET_HIT_CLIENT_LARGE     = 0x35,
    EV_DESTRUCTIBLE_BULLET_HIT     = 0x36,
    EV_DESTRUCTIBLE_EXPLOSION_HIT  = 0x37,
    EV_GRENADE_BOUNCE              = 0x38,
    EV_GRENADE_EXPLODE             = 0x39,
    EV_ROCKET_EXPLODE              = 0x3A,
    EV_ROCKET_EXPLODE_NOMARKS      = 0x3B,
    EV_FLASHBANG_EXPLODE           = 0x3C,
    EV_CUSTOM_EXPLODE              = 0x3D,
    EV_CUSTOM_EXPLODE_NOMARKS      = 0x3E,
    EV_CHANGE_TO_DUD               = 0x3F,
    EV_DUD_EXPLODE                 = 0x40,
    EV_DUD_IMPACT                  = 0x41,
    EV_FIRE_EXPLODE                = 0x42,
    EV_TIMED_FX                    = 0x43,
    EV_MOLOTOV_FLOAT               = 0x44,
    EV_FAKE_FIRE                   = 0x45,
    EV_PLAY_FX                     = 0x46,
    EV_PLAY_FX_ON_TAG              = 0x47,
    EV_PHYS_EXPLOSION_SPHERE       = 0x48,
    EV_PHYS_EXPLOSION_CYLINDER     = 0x49,
    EV_PHYS_EXPLOSION_JOLT         = 0x4A,
    EV_PHYS_LAUNCH                 = 0x4B,
    EV_CREATE_DYNENT               = 0x4C,
    EV_CREATE_ROPE                 = 0x4D,
    EV_BLOOD_IMPACTS               = 0x4E,
    EV_DETACH_ENTITY               = 0x4F,
    EV_DELETE_ROPE                 = 0x50,
    EV_ROPE_COLLIDE                = 0x51,
    EV_ROPE_SETFLAG                = 0x52,
    EV_ROPE_SETPARAM               = 0x53,
    EV_ROPE_ADDANCHOR              = 0x54,
    EV_ROPE_REMOVEANCHOR           = 0x55,
    EV_ROPE_MOVEANCHOR             = 0x56,
    EV_SETWETNESS                  = 0x57,
    EV_EARTHQUAKE                  = 0x58,
    EV_GRENADE_DROP                = 0x59,
    EV_GRENADE_SUICIDE             = 0x5A,
    EV_DETONATE                    = 0x5B,
    EV_NIGHTVISION_WEAR            = 0x5C,
    EV_NIGHTVISION_REMOVE          = 0x5D,
    EV_OBITUARY                    = 0x5E,
    EV_REVIVE_OBITUARY             = 0x5F,
    EV_DIRECTIONAL_HIT_INDICATOR   = 0x60,
    EV_ANIMATE_UI                  = 0x61,
    EV_NO_FRAG_GRENADE_HINT        = 0x62,
    EV_NO_SPECIAL_GRENADE_HINT     = 0x63,
    EV_TARGET_TOO_CLOSE_HINT       = 0x64,
    EV_TARGET_NOT_ENOUGH_CLEARANCE = 0x65,
    EV_LOCKON_REQUIRED_HINT        = 0x66,
    EV_FOOTSTEP_SPRINT             = 0x67,
    EV_FOOTSTEP_RUN                = 0x68,
    EV_FOOTSTEP_WALK               = 0x69,
    EV_FOOTSTEP_CROUCHRUN          = 0x6A,
    EV_FOOTSTEP_CROUCHWALK         = 0x6B,
    EV_FOOTSTEP_PRONE              = 0x6C,
    EV_MANTLE                      = 0x6D,
    EV_JUMP                        = 0x6E,
    EV_LANDING_FIRST               = 0x6F,
    EV_LANDING_LAST                = 0x8D,
    EV_LANDING_PAIN_FIRST          = 0x8E,
    EV_LANDING_PAIN_LAST           = 0xAC,
    EV_FIRE_VEHICLE_TURRET         = 0xAD,
    EV_FIRE_GUNNER_1               = 0xAE,
    EV_FIRE_GUNNER_2               = 0xAF,
    EV_FIRE_GUNNER_3               = 0xB0,
    EV_FIRE_GUNNER_4               = 0xB1,
    EV_FIRE_GUNNER_1A              = 0xB2,
    EV_FIRE_GUNNER_2A              = 0xB3,
    EV_FIRE_GUNNER_3A              = 0xB4,
    EV_FIRE_GUNNER_4A              = 0xB5,
    EV_START_CAMERA_TWEEN          = 0xB6,
    EV_DESTRUCTIBLE_DISABLE_PIECES = 0xB7,
    EV_FOOTPRINT                   = 0xB8,
    EV_CANNOTPLANT                 = 0xB9,
    EV_DTP_LAUNCH                  = 0xBA,
    EV_DTP_LAND                    = 0xBB,
    EV_SLIDE_START                 = 0xBC,
    EV_SLIDE_STOP                  = 0xBD,
    EV_SCOPE_ZOOM                  = 0xBE,
    EV_JAM_WEAPON                  = 0xBF,
    EV_STACKFIRE                   = 0xC0,
    EV_BOLT_IMPACT                 = 0xC1,
    EV_BOLT_IMPALE                 = 0xC2,
    EV_PLAY_WEAPON_DEATH_EFFECTS   = 0xC3,
    EV_PLAY_WEAPON_DAMAGE_EFFECTS  = 0xC4,
    EV_FACE_EVENT                  = 0xC5,
    EV_SETLOCALWIND                = 0xC6,
    EV_FLOAT_LONGER                = 0xC7,
    EV_FORCE_BUOYANCY              = 0xC8,
    EV_DISABLE_DEPTH_BUOYANCY_ADJUSTMENTS = 0xC9,
    EV_SCALE_BUOYANCY              = 0xCA,
    EV_ALLOWPITCH                  = 0xCB,
    EV_GIB                         = 0xCC,
    EV_MAX_EVENTS                  = 0xCD,
};

float BG_ZombieFloatDefault(const char *name, float mpDefault, float spDefault);
bool BG_ZombieBoolDefault(const char *name, bool mpDefault, bool spDefault);
int BG_ZombieIntDefault(const char *name, int mpDefault, int spDefault);
void BG_UpdateZombieDvarDefaults(bool spLevel);
void BG_SetZombieExternalDvars();
void __cdecl BG_RegisterDvars();
char *__cdecl BG_GetEntityTypeName(int eType);
const gitem_s *__cdecl BG_FindItemForWeapon(unsigned int weapon);
const gitem_s *__cdecl G_FindItem(char *pickupName);
bool __cdecl BG_PlayerTouchesItem(const playerState_s *ps, const entityState_s *item, int atTime);
bool __cdecl BG_PlayerCanPickUpWeaponType(const WeaponDef *weapDef, const playerState_s *ps);
bool __cdecl BG_CanItemBeGrabbed(const entityState_s *ent, const playerState_s *ps, int touched);
char __cdecl WeaponEntCanBeGrabbed(
                const entityState_s *weaponEntState,
                const playerState_s *ps,
                int touched,
                unsigned int weapIdx);
bool __cdecl BG_PlayerHasWeapon(const playerState_s *ps, unsigned int weaponIndex);
const PlayerHeldWeapon *__cdecl BG_GetHeldWeaponConst(const playerState_s *ps, unsigned int weaponIndex);
int __cdecl BG_GetHeldWeaponSlot(const playerState_s *ps, unsigned int weaponIndex);
char __cdecl HaveRoomForAmmo(const playerState_s *ps, unsigned int weaponIndex);
bool __cdecl BG_PlayerHasRoomForEntAllAmmoTypes(const entityState_s *ent, const playerState_s *ps);
void __cdecl BG_EvaluateTrajectory(const trajectory_t *tr, int atTime, float *result);
void __cdecl BG_EvaluateTrajectoryDelta(const trajectory_t *tr, int atTime, float *result);
bool __cdecl BG_ValidateOrigin(const float *pos, char xyBits, char zBits, const float *mapCenter);
bool __cdecl BG_ValidateOriginValue(float val, char bits, float mapCenterValue);
void __cdecl BG_AddPredictableEventToPlayerstate(unsigned int newEvent, unsigned int eventParm, playerState_s *ps);
void __cdecl BG_AddUnpredictableEventToPlayerstate(unsigned int newEvent, unsigned int eventParm, playerState_s *ps);
void __cdecl BG_AddEntityStateEvent(unsigned int newEvent, unsigned int eventParm, entityState_s *es, int eventTime);
void __cdecl BG_PlayerToEntitySetFlags(playerState_s *ps, entityState_s *s);
void __cdecl BG_PlayerToEntitySetPitchAngles(playerState_s *ps, entityState_s *s);
void __cdecl BG_PlayerStateToEntityState(playerState_s *ps, entityState_s *s, int snap, unsigned __int8 handler);
void __cdecl BG_PlayerToEntityProcessEvents(playerState_s *ps, entityState_s *s, unsigned __int8 handler);
void __cdecl BG_PlayerToEntityProcessEvents_Internal(
                entityState_s *s,
                unsigned __int8 handler,
                __int16 *eventSequence,
                __int16 *oldEventSequence,
                int *events,
                unsigned int *eventParms,
                unsigned int eventTime);
char __cdecl BG_PlayerToEntityShouldAddEvent(int event);
void __cdecl BG_PlayerToEntitySetMisc(playerState_s *ps, entityState_s *s);
unsigned __int8 __cdecl BG_GetPlayerWeaponModel(const playerState_s *ps, unsigned int weaponIndex);
renderOptions_s __cdecl BG_PlayerWeaponOptions(const playerState_s *ps, unsigned int weaponIndex);
void __cdecl BG_PlayerToEntitySetTrajectory(playerState_s *ps, entityState_s *s, int snap);
char    BG_CheckProne(
                const playerState_s *ps,
                int passEntityNum,
                const float *vPos,
                float fSize,
                float fHeight,
                float fYaw,
                float *pfTorsoPitch,
                float *pfWaistPitch,
                bool isAlreadyProne,
                bool isOnGround,
                bool groundIsWalkable,
                unsigned __int8 handler,
                proneCheckType_t proneCheckType,
                float prone_feet_dist);
void __cdecl BG_GetPlayerViewOrigin(const playerState_s *ps, float *origin, int time);
void __cdecl BG_GetPlayerViewDirection(const playerState_s *ps, float *forward, float *right, float *up);
void __cdecl BG_LerpHudColors(const hudelem_s *elem, int time, union hudelem_color_t *toColor);
int __cdecl BG_LoadShellShockDvars(const char *name);
void __cdecl BG_SetShellShockParmsFromDvars(shellshock_parms_t *parms);
int __cdecl BG_SaveShellShockDvars(const char *name);
shellshock_parms_t *__cdecl BG_GetShellshockParms(unsigned int index);
bool __cdecl BG_IsShellshockAnim(const playerState_s *ps);
void __cdecl BG_CreateXAnim(XAnim_s *anims, unsigned int animIndex, char *name);
inline void BG_CreateXAnim(XAnim_s *anims, unsigned int animIndex, const char *name)
{
    BG_CreateXAnim(anims, animIndex, (char *)name);
}
void __cdecl BG_CheckThread();
int __cdecl BG_GetMaxSprintTime(const playerState_s *ps);
bool __cdecl BG_ClipMoveToDobj(const entityState_s *es);
int __cdecl BG_GetFriendlyFireStatus();
const char *__cdecl BG_DisplayName(const struct clientInfo_t *ci, int type);
void __cdecl BG_ClipCameraToHeliPatch(float *origin, float *oldpos, float *velocity, float (*breachPos)[3]);


extern const dvar_s *bg_viewKickScale;
extern const dvar_s *bg_viewKickMax;
extern const dvar_s *bg_viewKickMin;
extern const dvar_s *bg_viewKickRandom;
extern const dvar_s *player_view_pitch_up;
extern const dvar_s *player_view_pitch_down;
extern const dvar_s *player_lean_shift;
extern const dvar_s *player_lean_shift_crouch;
extern const dvar_s *player_lean_rotate;
extern const dvar_s *player_lean_rotate_crouch;
extern const dvar_s *player_useSlopeAnims;
extern const dvar_s *bg_ladder_yawcap;
extern const dvar_s *bg_prone_yawcap;
extern const dvar_s *bg_foliagesnd_minspeed;
extern const dvar_s *bg_foliagesnd_maxspeed;
extern const dvar_s *bg_foliagesnd_slowinterval;
extern const dvar_s *bg_foliagesnd_fastinterval;
extern const dvar_s *bg_foliagesnd_resetinterval;
extern const dvar_s *bg_friendlyFire;
extern const dvar_s *bg_fallDamageMinHeight;
extern const dvar_s *bg_fallDamageMaxHeight;
extern const dvar_s *friction;
extern const dvar_s *stopspeed;
extern const dvar_s *cg_ufo_scaler;
extern const dvar_s *bg_swingSpeed;
extern const dvar_s *bg_proneSwingSpeed;
extern const dvar_s *bg_legYawTolerance;
extern const dvar_s *bg_legYawCrouchTolerance;
extern const dvar_s *bg_legYawProneTolerance;
extern const dvar_s *bg_viewBobAmplitudeBase;
extern const dvar_s *bg_viewBobAmplitudeSprinting;
extern const dvar_s *bg_viewBobAmplitudeDtp;
extern const dvar_s *bg_viewBobAmplitudeSwimming;
extern const dvar_s *bg_viewBobAmplitudeStanding;
extern const dvar_s *bg_viewBobAmplitudeStandingAds;
extern const dvar_s *bg_viewBobAmplitudeDucked;
extern const dvar_s *bg_viewBobAmplitudeDuckedAds;
extern const dvar_s *bg_viewBobAmplitudeProne;
extern const dvar_s *bg_viewBobAmplitudeRoll;
extern const dvar_s *bg_viewBobMax;
extern const dvar_s *bg_forceDurationOverride;
extern const dvar_s *bg_blendTimeOverride;
extern const dvar_s *bg_weaponBobAmplitudeBase;
extern const dvar_s *bg_weaponBobAmplitudeSprinting;
extern const dvar_s *bg_weaponBobAmplitudeDtp;
extern const dvar_s *bg_weaponBobAmplitudeSwimming;
extern const dvar_s *bg_weaponBobAmplitudeStanding;
extern const dvar_s *bg_weaponBobAmplitudeDucked;
extern const dvar_s *bg_weaponBobAmplitudeProne;
extern const dvar_s *bg_weaponBobAmplitudeRoll;
extern const dvar_s *bg_weaponBobMax;
extern const dvar_s *bg_weaponBobLag;
extern const dvar_s *bg_weaponBobFrequencySwimming;
extern const dvar_s *bg_weaponBobHeavyWeaponScalar;
extern const dvar_s *bg_forceExplosiveBullets;
extern const dvar_s *bg_aimSpreadMoveSpeedThreshold;
extern const dvar_s *bg_maxGrenadeIndicatorSpeed;
extern const dvar_s *player_breath_hold_time;
extern const dvar_s *player_breath_gasp_time;
extern const dvar_s *player_breath_fire_delay;
extern const dvar_s *player_breath_gasp_scale;
extern const dvar_s *player_breath_hold_lerp;
extern const dvar_s *player_breath_gasp_lerp;
extern const dvar_s *player_breath_snd_lerp;
extern const dvar_s *player_breath_snd_delay;
extern const dvar_s *player_scopeExitOnDamage;
extern const dvar_s *player_adsExitDelay;
extern const dvar_s *player_move_factor_on_torso;
extern const dvar_s *player_debugHealth;
extern const dvar_s *player_debugSprint;
extern const dvar_s *player_sustainAmmo;
extern const dvar_s *player_clipSizeMultiplier;
extern const dvar_s *player_lastStandBleedoutTime;
extern const dvar_s *player_lastStandBleedoutTimeNoRevive;
extern const dvar_s *revive_time_taken;
extern const dvar_s *player_lastStandHealthOverlayTime;
extern const dvar_s *player_reviveTriggerRadius;
extern const dvar_s *player_revivePlayerListCycleTime;
extern const dvar_s *bg_vsmode_hud;
extern const dvar_s *player_enableShuffleAnims;
extern const dvar_s *player_animRunThreshhold;
extern const dvar_s *player_animWalkThreshhold;
extern const dvar_s *player_runbkThreshhold;
extern const dvar_s *player_moveThreshhold;
extern const dvar_s *player_footstepsThreshhold;
extern const dvar_s *player_runThreshhold;
extern const dvar_s *player_knockbackMoveThreshhold;
extern const dvar_s *player_sprintThreshhold;
extern const dvar_s *player_waterSpeedScale;
extern const dvar_s *player_floatSpeed;
extern const dvar_s *player_disableWeaponsInWater;
extern const dvar_s *player_sliding_friction;
extern const dvar_s *player_sliding_wishspeed;
extern const dvar_s *player_sliding_velocity_cap;
extern const dvar_s *player_strafeSpeedScale;
extern const dvar_s *player_backSpeedScale;
extern const dvar_s *player_strafeAnimCosAngle;
extern const dvar_s *player_slopeAnimAngle;
extern const dvar_s *player_spectateSpeedScale;
extern const dvar_s *player_enduranceSpeedScale;
extern const dvar_s *player_sprintForwardMinimum;
extern const dvar_s *player_sprintSpeedScale;
extern const dvar_s *player_sprintUnlimited;
extern const dvar_s *player_sprintTime;
extern const dvar_s *player_sprintMinTime;
extern const dvar_s *player_sprintRechargePause;
extern const dvar_s *player_sprintStrafeSpeedScale;
extern const dvar_s *player_sprintCameraBob;
extern const dvar_s *player_turnRateScale;
extern const dvar_s *player_turnAnims;
extern const dvar_s *player_bayonetLaunchProof;
extern const dvar_s *player_bayonetLaunchDebugging;
extern const dvar_s *player_bayonetLaunchZCap;
extern const dvar_s *xanim_debug;
extern const dvar_s *animscript_debug;
extern const dvar_s *anim_debugSpeeds;
extern const dvar_s *animScript_listAnims;
extern const dvar_s *player_dmgtimer_timePerPoint;
extern const dvar_s *player_dmgtimer_maxTime;
extern const dvar_s *player_dmgtimer_minScale;
extern const dvar_s *player_dmgtimer_stumbleTime;
extern const dvar_s *player_dmgtimer_flinchTime;
extern const dvar_s *bg_shock_soundLoop;
extern const dvar_s *bg_shock_soundLoopSilent;
extern const dvar_s *bg_shock_soundEnd;
extern const dvar_s *bg_shock_soundEndAbort;
extern const dvar_s *bg_shock_screenType;
extern const dvar_s *bg_shock_screenBlurBlendTime;
extern const dvar_s *bg_shock_screenBlurBlendFadeTime;
extern const dvar_s *bg_shock_screenFlashWhiteFadeTime;
extern const dvar_s *bg_shock_screenFlashShotFadeTime;
extern const dvar_s *bg_shock_viewKickPeriod;
extern const dvar_s *bg_shock_viewKickRadius;
extern const dvar_s *bg_shock_viewKickFadeTime;
extern const dvar_s *bg_shock_sound;
extern const dvar_s *bg_shock_soundFadeInTime;
extern const dvar_s *bg_shock_soundFadeOutTime;
extern const dvar_s *bg_shock_soundLoopFadeTime;
extern const dvar_s *bg_shock_soundLoopEndDelay;
extern const dvar_s *bg_shock_soundRoomType;
extern const dvar_s *bg_shock_soundDryLevel;
extern const dvar_s *bg_shock_soundWetLevel;
extern const dvar_s *bg_shock_soundModEndDelay;
extern const dvar_s *bg_shock_soundSnapshot;
extern const dvar_s *bg_shock_lookControl;
extern const dvar_s *bg_shock_lookControl_maxpitchspeed;
extern const dvar_s *bg_shock_lookControl_maxyawspeed;
extern const dvar_s *bg_shock_lookControl_mousesensitivityscale;
extern const dvar_s *bg_shock_lookControl_fadeTime;
extern const dvar_s *bg_shock_movement;
extern const dvar_s *bg_shock_animation;
extern const dvar_s *bg_shock_visionset_name;
extern const dvar_s *bg_shock_visionset_inTime;
extern const dvar_s *bg_shock_visionset_outTime;
extern const dvar_s *player_meleeRange;
extern const dvar_s *player_meleeWidth;
extern const dvar_s *player_meleeHeight;
extern const dvar_s *player_bayonetRange;
extern const dvar_s *player_bayonetTargetDist;
extern const dvar_s *player_burstFireCooldown;
extern const dvar_s *bg_gravity;
extern const dvar_s *bg_lowGravity;
extern const dvar_s *player_swimTime;
extern const dvar_s *player_swimDamage;
extern const dvar_s *player_swimDamagerInterval;
extern const dvar_s *player_viewLockEnt;
extern const dvar_s *player_viewRateScale;
extern const dvar_s *player_topDownCamMode;
extern const dvar_s *player_topDownCamOffset;
extern const dvar_s *player_topDownCamAngles;
extern const dvar_s *player_topDownCamCenterPos;
extern const dvar_s *player_topDownCursorDist;
extern const dvar_s *player_topDownCursorPos;
extern const dvar_s *player_forceRedCrosshair;
extern const dvar_s *bullet_penetrationMinFxDist;
extern const dvar_s *cg_cinematicFullscreen;
extern const dvar_s *cg_debugMounting;
extern const dvar_s *player_AimBlend_Back_Low;
extern const dvar_s *player_AimBlend_Back_Mid;
extern const dvar_s *player_AimBlend_Back_Up;
extern const dvar_s *player_AimBlend_Pelvis;
extern const dvar_s *player_AimBlend_Neck;
extern const dvar_s *player_AimBlend_Head;
extern const dvar_s *player_AimBlend_Shoulder;
extern const dvar_s *dog_MeleeDamage;
extern const dvar_s *vehControlMode;
extern const dvar_s *vehRecenterDelay;
extern const dvar_s *vehLockTurretToPlayerView;
extern const dvar_s *vehLocationalVehicleSeatEntry;
extern const dvar_s *vehCameraTurretOffset;
extern const dvar_s *vehCameraTurretOffsetADS;
extern const dvar_s *vehicle_perk_boost_duration_seconds;
extern const dvar_s *vehicle_riding;
extern const dvar_s *vehicle_selfCollision;
extern const dvar_s *g_bDebugRenderPlayerCollision;
extern const dvar_s *bg_disableWeaponPlantingInWater;
extern const dvar_s *bg_plantInWaterDepth;
extern const dvar_s *bg_drawGrenadeInHand;
extern const dvar_s *bg_playStandToCrouchAnims;
extern const dvar_s *bg_maxWeaponAnimScale;
extern const dvar_s *bg_gunXOffset;
extern const dvar_s *bg_weaponleftbone;
extern const dvar_s *bg_weaponrightbone;
extern const dvar_s *bg_slopeFrames;
extern const dvar_s *debug_rope;
extern const dvar_s *vehicle_sounds_cutoff;
extern const dvar_s *footstep_sounds_cutoff;
extern const dvar_s *show_reticle_during_swimming;
extern const dvar_s *ragdoll_reactivation_cutoff;
extern const dvar_s *bg_teleportAlignTime;
extern const dvar_s *bg_enableIKActiveFix;
extern const dvar_s *waterbrush_entity;
extern const dvar_s *dive2swim;
extern const dvar_s *dive_recharge;
extern const dvar_s *playerPushAmount;
extern const dvar_s *bg_freeCamClipToHeliPatch;
extern const dvar_s *zero_idle_movement;

extern const char *eventnames[];
extern const char *entityTypeNames[21];