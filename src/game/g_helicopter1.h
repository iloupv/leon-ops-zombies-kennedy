#pragma once
#include <bgame/bg_public.h>
#include "g_scr_vehicle.h"

void __cdecl Helicopter_RegisterDvars();
void __cdecl HELI_CancelAIMove(gentity_s *ent);
void __cdecl VEH_CheckForPredictedCrash(gentity_s *ent);
void __cdecl VEH_CheckForCrash(gentity_s *ent, float *newPosition);
char __cdecl VEH_HandleHeliLockHeight(gentity_s *ent);
void __cdecl VEH_UpdateClientChopper(gentity_s *ent);
void __cdecl HELI_CalcAccel(gentity_s *ent, char *move, float *bodyAccel, float *rotAccel, float *viewangles);
void __cdecl HELI_CmdScale(char *move, float *outFracs);
void __cdecl HELI_UpdateJitter(VehicleJitter *jitter);
void __cdecl HELI_SoftenCollisions(gentity_s *ent, float *worldAccel);
bool __cdecl VEH_TestSlideMove(gentity_s *ent, float *outPos);
double __cdecl Vehicle_GetFakeLift(float *planeForward, float *planeUp);
double __cdecl Vehicle_AdjustRollForInputDeadZone(float rollPercent);
void __cdecl Vehicle_AddRotate(vehicle_physic_t *phys, float fRoll, float fPitch, float fYaw);
void __cdecl VEH_UpdateClientPlane(gentity_s *ent);

extern const dvar_t *vehHelicopterMaxAccelVertical;
extern const dvar_t *vehHelicopterLookaheadTime;
extern const dvar_t *vehHelicopterHoverSpeedThreshold;
extern const dvar_t *vehHelicopterRightStickDeadzone;
extern const dvar_t *vehHelicopterStrafeDeadzone;
extern const dvar_t *vehHelicopterScaleMovement;
extern const dvar_t *vehHelicopterYawAltitudeControls;
extern const dvar_t *vehHelicopterSoftCollisions;
extern const dvar_t *vehHelicopterDecelerationFwd;
extern const dvar_t *vehHelicopterDecelerationSide;
extern const dvar_t *vehHelicopterInvertUpDown;
extern const dvar_t *vehHelicopterYawOnLeftStick;
extern const dvar_t *vehHelicopterTiltFromControllerAxes;
extern const dvar_t *vehHelicopterTiltFromFwdAndYaw_VelAtMaxTilt;
extern const dvar_t *vehHelicopterTiltFromViewangles;
extern const dvar_t *vehHelicopterJitterJerkyness;
extern const dvar_t *vehHelicopterHeadSwayDontSwayTheTurret;
extern const dvar_t *vehHelicopterTiltMomentum;
extern const dvar_t *vehHelicopterAlwaysFaceCamera;
extern const dvar_t *vehHelicopterAlwaysFaceCameraRate;
extern const dvar_t *vehHelicopterPathTransitionTime;
extern const dvar_t *vehHelicopterFreeLook;
extern const dvar_t *vehHelicopterDefaultPitch;
extern const dvar_t *vehHelicopterRotDecel;
extern const dvar_t *vehHelicopterTiltFromFwdAndYaw;
extern const dvar_t *vehHelicopterMaxSpeedVertical;
extern const dvar_t *vehHelicopterboundsOn;
extern const dvar_t *vehHelicopterboundMapUpperLeftX;
extern const dvar_t *vehHelicopterboundMapUpperLeftY;
extern const dvar_t *vehHelicopterboundMapLowerRightX;
extern const dvar_t *vehHelicopterboundMapLowerRightY;
extern const dvar_t *vehHelicopterboundMapHeight;