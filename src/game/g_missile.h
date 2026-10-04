#pragma once
#include <qcommon/cm_trace.h>
#include <bgame/bg_weapons.h>
#include <qcommon/common.h>

struct AttractorRepulsor_t // sizeof=0x1C
{                                       // XREF: $60CB2C8C864AAD4BAC72BAB25E42491F/r
    bool inUse;                         // XREF: Missile_FreeAttractorRefs(gentity_s *)+22/r
                                        // Scr_MissileCreateAttractorEnt(void)+AF/w ...
    bool isAttractor;
    // padding byte
    // padding byte
    int entnum;
    float origin[3];
    float strength;
    float maxDist;
};

struct AttractorGlob //$60CB2C8C864AAD4BAC72BAB25E42491F // sizeof=0x380
{                                       // XREF: .data:attrGlob/r
    AttractorRepulsor_t attractors[32]; // XREF: Missile_FreeAttractorRefs(gentity_s *)+22/r
};
extern AttractorGlob attrGlob;

void __cdecl G_RegisterMissileDvars();
void __cdecl G_RegisterMissileDebugDvars();
bool __cdecl collide_missle_entity_filter(int entnum);
void __cdecl G_MissileTrace(
                trace_t *results,
                const float *start,
                const float *end,
                int passEntityNum,
                int contentmask,
                unsigned int weapon);
void __cdecl G_TimedObjectThink(gentity_s *ent);
// local variable allocation has failed, the output may be wrong!
void    G_ExplodeMissile(gentity_s *ent);
int __cdecl ForcedDud(gentity_s *ent);
int __cdecl GetSplashMethodOfDeath(gentity_s *ent);
void __cdecl G_UnlinkPlayerToRocket(gentity_s *ent);
void __cdecl Missile_InitAttractors();
void __cdecl Missile_FreeAttractorRefs(gentity_s *ent);
void __cdecl Scr_MissileCreateAttractorEnt();
unsigned int __cdecl Missile_GetFreeAttractor();
void __cdecl Scr_MissileCreateAttractorOrigin();
void __cdecl Scr_MissileCreateRepulsorEnt();
void __cdecl Scr_MissileCreateRepulsorOrigin();
void __cdecl Scr_MissileDeleteAttractor();
void __cdecl G_MakeMissilePickupItem(gentity_s *ent);
void __cdecl G_RunMissileInternal(gentity_s *ent);
void    MissileImpact(gentity_s *ent, trace_t *trace, float *dir, float *endpos);
bool __cdecl CheckCrumpleMissile(gentity_s *ent, trace_t *trace);
// local variable allocation has failed, the output may be wrong!
bool    BounceMissile(gentity_s *ent, trace_t *trace);
void __cdecl MissileLandAngles(gentity_s *ent, trace_t *trace, float *vAngles, int bForceAlign, int bKeepSpeed);
void __cdecl MissileLandAnglesFlat(gentity_s *ent, trace_t *trace, float *angles);
void __cdecl MissileLandAnglesFlatMaintainingDirection(gentity_s *ent, trace_t *trace, float *angles);
void __cdecl CheckGrenadeDanger(gentity_s *grenadeEnt);
char __cdecl GrenadeBounceVelocity(
                const float *preBounceVelocity,
                float dot,
                const float *normal,
                int surfType,
                const WeaponDef *weapDef,
                trajectory_t *pos,
                float *rollSlideSpeed,
                float *rollSlideDir,
                bool isDud);
bool __cdecl JavelinProjectile(gentity_s *ent, const WeaponDef *weapDef);
bool __cdecl isDud(gentity_s *ent, const WeaponDef *weapDef);
bool __cdecl GrenadeDud(gentity_s *ent, const WeaponDef *weapDef);
bool __cdecl JavelinDud(gentity_s *ent, const WeaponDef *weapDef);
void __cdecl AttachBoltGrenade(gentity_s *ent, gentity_s *hitEnt, trace_t *trace, hitLocation_t hitLocation);
void __cdecl AttachMissileToEntity(
                gentity_s *missile,
                unsigned int entnum,
                unsigned int boneName,
                int hitLocation,
                float *velocity);
double __cdecl GetAttachedMissileOffset(int hitlocation);
int __cdecl StickMissile(
                gentity_s *ent,
                gentity_s *other,
                const WeaponDef *weapDef,
                trace_t *trace,
                hitLocation_t hitLocation,
                float *velocity);
bool __cdecl isBounceProjectile(gentity_s *ent);
void __cdecl createRetrieveableProjectile(
                gentity_s *ent,
                gentity_s *other,
                const WeaponDef *weapDef,
                trace_t *trace,
                hitLocation_t hitLocation,
                float *dir,
                float *endpos);
void __cdecl Missile_PenetrateGlass(
                trace_t *results,
                gentity_s *ent,
                const float *start,
                const float *end,
                int damage,
                bool predicted);
void __cdecl DrawMissileDebug(float *start, float *end);
void __cdecl RunMissile_Destabilize(gentity_s *missile);
double __cdecl RunMissile_GetPerturbation(float destabilizationCurvatureMax);
void    Missile_ApplyAttractorsRepulsors(gentity_s *missile);
void __cdecl RunMissile_CreateWaterSplash(const gentity_s *missile, const float *hitPos, const trace_t *trace);
void __cdecl RunMissile_BroadcastActorEvents(gentity_s *missile);
void __cdecl MissileTrajectory(gentity_s *ent, float *result);
bool __cdecl MissileIsReadyForSteering(gentity_s *ent);
void __cdecl MissileTrajectoryClientControlled(gentity_s *ent, float *result);
void __cdecl GuidedMissileSteering(gentity_s *ent);
char __cdecl IsMissileLockedOn(gentity_s *ent);
void __cdecl GetTargetPosition(gentity_s *ent, float *result);
void __cdecl MissileTVGuidedHorzSteerToTarget(
                gentity_s *ent,
                const float *currentRight,
                const float *toTargetRelative,
                float *steer,
                float accel);
void __cdecl MissileHorzSteerToTarget(
                gentity_s *ent,
                const float *currentRight,
                const float *toTargetRelative,
                float currentHorzSpeed,
                float *steer,
                bool tvGuided);
void __cdecl MissileVerticalSteering(
                gentity_s *ent,
                const float *toTargetRelative,
                float currentHorzSpeed,
                float *steer);
void __cdecl MissileVerticalSteerToTarget(
                gentity_s *ent,
                const float *toTargetHorzRelDir,
                float horzDistToTarg,
                float vertDistToTarg,
                float currentHorzSpeed,
                float *steer);
void __cdecl JavelinSteering(gentity_s *ent, const WeaponDef *weapDef);
void __cdecl JavelinClimbOffset(gentity_s *ent, float *targetPos);
void __cdecl JavelinRotateVelocity(gentity_s *ent, const float *currentVel, const float *targetDir, float *resultVel);
double __cdecl JavelinRotateDir(gentity_s *ent, const float *currentDir, const float *targetDir, float *resultDir);
double __cdecl JavelinMaxDPS(gentity_s *ent);
void __cdecl VecToQuat(const float *vec, float *quat);
double __cdecl JavelinClimbCeiling(gentity_s *ent);
bool __cdecl JavelinClimbEnd(gentity_s *ent, const float *targetPos);
char    JavelinClimbExceededAngle(gentity_s *ent, const float *targetPos);
char __cdecl JavelinClimbWithinDistance(gentity_s *ent, const float *targetPos);
bool __cdecl JavelinClimbIsAboveCeiling(gentity_s *ent, const float *targetPos);
bool __cdecl CheckForMissileClientControlledDetonation(gentity_s *ent);
char __cdecl UpdateGuidedMissileFuelTime(gentity_s *ent, bool initTimer);
unsigned __int8 __cdecl G_GetGrenadeTrType(gentity_s *grenade);
void __cdecl G_RunMissile(gentity_s *ent);
int __cdecl G_PredictMissile(gentity_s *ent, int duration, float *vLandPos, int allowBounce, int *timeAtRest);
void __cdecl DrawMissilePredictDebug(float *start, float *end);
void __cdecl PredictBounceMissile(
                gentity_s *ent,
                trajectory_t *pos,
                trace_t *trace,
                int time,
                int velocityTime,
                float *origin,
                float *endpos);
void __cdecl G_InitGrenadeEntity(gentity_s *parent, gentity_s *grenade);
void __cdecl G_InitGrenadeMovement(
                gentity_s *grenade,
                const float *start,
                const float *dir,
                int rotate,
                WeapRotateType rotateType);
gentity_s *__cdecl G_FireGrenade(
                gentity_s *parent,
                float *start,
                float *dir,
                int grenadeWPID,
                unsigned __int8 grenModel,
                int rotate,
                int time);
int __cdecl CalcMissileNoDrawTime(float speed);
void __cdecl InitGrenadeTimer(const gentity_s *parent, gentity_s *grenade, const WeaponDef *weapDef, int time);
gentity_s *__cdecl G_FireRocket(
                gentity_s *parent,
                unsigned int weaponIndex,
                float *start,
                float *dir,
                const float *gunVel,
                gentity_s *target,
                const float *targetOffset);
void __cdecl G_LinkPlayerToRocket(gentity_s *ent, gentity_s *player);
void __cdecl InitRocketTimer(gentity_s *bolt, const WeaponDef *weapDef);
gentity_s *__cdecl G_DropBomb(
                gentity_s *parent,
                unsigned int weaponIndex,
                float *start,
                float *dir,
                const float *gunVel,
                gentity_s *target,
                const float *targetOffset);
