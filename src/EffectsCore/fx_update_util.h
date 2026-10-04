#pragma once
#include "fx_update.h"

void __cdecl FX_OffsetSpawnOrigin(
                const FxSpatialFrame *effectFrame,
                const FxElemDef *elemDef,
                int randomSeed,
                float *spawnOrigin);
void __cdecl FX_GetOriginForTrailElem(
                FxEffect *effect,
                const FxElemDef *elemDef,
                const FxSpatialFrame *effectFrameWhenPlayed,
                int randomSeed,
                float *outOrigin,
                float *outRight,
                float *outUp);
void __cdecl FX_GetSpawnOrigin(
                const FxSpatialFrame *effectFrame,
                const FxElemDef *elemDef,
                int randomSeed,
                float *spawnOrigin);
void __cdecl FX_TransformPosFromLocalToWorld(const FxSpatialFrame *frame, float *posLocal, float *posWorld);
void __cdecl FX_SpatialFrameToOrientation(const FxSpatialFrame *frame, orientation_t *orient);
void __cdecl FX_OrientationDirToWorldDir(const orientation_t *orient, const float *dir, float *out);
void __cdecl FX_GetOrientation(
                const FxElemDef *elemDef,
                const FxSpatialFrame *frameAtSpawn,
                const FxSpatialFrame *frameNow,
                int randomSeed,
                orientation_t *orient);
void __cdecl FX_GetVelocityAtTime(
                const FxElemDef *elemDef,
                int randomSeed,
                float msecLifeSpan,
                float msecElapsed,
                const orientation_t *orient,
                const float *baseVel,
                float *velocity);
void __cdecl FX_GetVelocityAtTimeInFrame(
                const FxElemVelStateInFrame *statePrev,
                const FxElemVelStateInFrame *stateNext,
                const float *rangeLerp,
                const float *weight,
                float *velocity);
void __cdecl FX_OrientationPosToWorldPos(const orientation_t *orient, const float *pos, float *out);
void __cdecl FX_OrientationPosFromWorldPos(const orientation_t *orient, const float *pos, float *out);
void __cdecl FX_AddVisBlocker(FxSystem *system, const float *posWorld, float radius, float opacity);
void __cdecl FX_ToggleVisBlockerFrame(FxSystem *system);
char __cdecl FX_CullSphere(const FxCamera *camera, unsigned int frustumPlaneCount, const float *posWorld, float radius);
void __cdecl FX_AnglesToOrientedAxis(float *anglesInRad, const orientation_t *orient, float (*axisOut)[3]);
void __cdecl FX_GetElemAxis(
                const FxElemDef *elemDef,
                int randomSeed,
                const orientation_t *orient,
                float msecElapsed,
                float (*axis)[3]);
int __cdecl FX_GetElemLifeSpanMsec(int elemRandomSeed, float elemWindInterpolate, const FxElemDef *elemDef);
