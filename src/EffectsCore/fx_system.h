#pragma once

#include <gfx_d3d/fxprimitives.h>

struct FxSystemContainer // sizeof=0xBA0
{
    FxSystem system;
    FxSystemShared shared;
};

// mod (L43): r_mod_fxElems - the elem pool is allocated at its largest (FX_ELEM_POOL_MAX) once, before the dvars exist;
// FX_ResetSystem links only fx_elemLimit of them into the free list (retail 2048). Handle = byte offset / HANDLE_SCALE
// = index * 12, so 4096 elems (max handle 49140) still fit the uint16 handles and their 0xFFFF sentinel.
extern int fx_elemLimit; // FX_ELEM_POOL_MAX: fxprimitives.h

struct FxSystemBuffers // sizeof=0x62480 retail (elems[2048]); larger with FX_ELEM_POOL_MAX
{
    FxEffectContainer effects[1024];
    FxPool<FxElem,FxElemContainer> elems[FX_ELEM_POOL_MAX];
    FxPool<FxTrail,FxTrail> trails[128];
    FxPool<FxTrailElem,FxTrailElem> trailElems[2048];
    FxVisState visState[2];
    FxUniqueHandleDb uniqueHandleDb;
    unsigned __int8 padBuffer[80];
};

struct HunkUser;

int __cdecl FX_AllocateClientMemory_SizeRequired(int maxLocalClients);
void __cdecl FX_AllocateClientMemory(HunkUser *hunk, int maxLocalClients);
void __cdecl FX_FreeClientMemory(HunkUser *hunk);
FxSystemContainer *__cdecl FX_GetSystem(int localClientNum);
FxSystemContainer *__cdecl FX_GetSystemRemote(int localClientNum);
FxSystemBuffers *__cdecl FX_GetSystemBuffers(int localClientNum);
void __cdecl FX_LinkSystemBuffers(FxSystem *system, FxSystemBuffers *systemBuffers);
void __cdecl FX_InitSystem(int localClientNum);
void __cdecl FX_ResetSystem(FxSystem *system);
int __cdecl FX_EffectToHandle(FxSystem *system, FxEffectContainer *effect);
void __cdecl FX_ShutdownSystem(int localClientNum);
void __cdecl FX_DelRefToEffect(FxSystem *system, FxEffectContainer *effect, unsigned int stackCount);
void __cdecl FX_EffectNoLongerReferenced(FxSystem *system, FxEffectContainer *remoteEffect, unsigned int stackCount);
void __cdecl FX_RunPrioritySorting(FxSystem *system);
void __cdecl FX_RunGarbageCollectionAndPrioritySort(FxSystem *system);
bool __cdecl FX_BeginIteratingOverEffects_Exclusive(FxSystem *system);
void __cdecl FX_RunGarbageCollection_FreeSpotLight(FxSystem *system, unsigned __int16 effectHandle);
void __cdecl FX_RunGarbageCollection_FreeTrails(FxSystem *system, FxEffect *effect);
void __cdecl FX_RemoveStaleAttachedEffects(FxSystem *system);
char __cdecl FX_EffectAffectsGameplay(const FxEffectDef *remoteEffectDef);
void __cdecl FX_SpawnEffect_AllocTrails(FxSystem *system, FxEffect *effect, FxEffect *remoteEffect);
int __cdecl FX_TrailToHandle(FxSystem *system, FxTrail *trail);
FxPool<FxTrail,FxTrail> *__cdecl FX_AllocTrail(FxEffect *remoteEffect, FxSystem *system);
void __cdecl FX_CalculatePackedLighting(FxEffect *effect, const float *origin);
bool __cdecl FX_NeedsPrioritySorting(const FxSystem *system);
void __cdecl Fx_ResetPriority(FxSystem *system);
void __cdecl FX_SetWarningPriority(FxSystem *system, unsigned __int8 priority);
void __cdecl FX_InvalidateFrameWarningPriority(FxSystem *system);
int __cdecl FX_GetFrameWarningPriority(const FxSystem *system);
void __cdecl FX_Priority_Begin(FxSystem *system);
unsigned int __cdecl FX_SpawnEffect(
                FxSystem *system,
                const FxEffectDef *remoteDef,
                int msecBegin,
                const float *origin,
                const float (*axis)[3],
                unsigned int dobjHandle,
                int boneIndex,
                int runnerSortOrder,
                unsigned __int16 owner,
                unsigned int markEntnum,
                const orientation_t *boneOffset);
void __cdecl FX_AddRefToEffect(FxSystem *__formal, FxEffect *effect);
char __cdecl FX_CullEffectForSpawn(const FxCamera *camera, const FxEffectDef *effectDef, const float *origin);
bool __cdecl FX_CullElemForSpawn(const FxCamera *camera, const FxElemDef *elemDef, const float *origin);
void __cdecl FX_SetEffectRandomSeed(FxEffect *effect, const FxEffectDef *remoteDef);
char __cdecl FX_IsSpotLightEffect(FxSystem *system, const FxEffectDef *def);
bool __cdecl FX_CanAllocSpotLightEffect(const FxSystem *system);
char __cdecl FX_SpawnEffect_AllocSpotLightEffect(FxSystem *system, FxEffect *effect, FxEffectContainer *remoteEffect);
unsigned int __cdecl FX_SpawnOrientedEffect(
                int localClientNum,
                const FxEffectDef *def,
                int msecBegin,
                const float *origin,
                const float (*axis)[3],
                unsigned int markEntnum);
void __cdecl FX_AssertAllocatedEffect(int localClientNum, unsigned int hEffect, const char *error_msg);
unsigned int __cdecl FX_PlayOrientedEffectWithMarkEntity(
                int localClientNum,
                const FxEffectDef *def,
                int startMsec,
                const float *origin,
                const float (*axis)[3],
                unsigned int markEntnum);
unsigned int __cdecl FX_PlayOrientedEffect(
                int localClientNum,
                const FxEffectDef *def,
                int startMsec,
                const float *origin,
                const float (*axis)[3]);
unsigned int __cdecl FX_PlayOrientedEffect(
                FxSystem *system,
                const FxEffectDef *def,
                int startMsec,
                const float *origin,
                const float (*axis)[3]);
unsigned int __cdecl FX_SpawnBoltedEffect(
                int localClientNum,
                const FxEffectDef *def,
                int msecBegin,
                unsigned int dobjHandle,
                unsigned int boneIndex,
                const float *origin,
                const float (*axis)[3]);
char __cdecl FX_NeedsBoltUpdate(const FxEffectDef *def);
unsigned int __cdecl FX_SpawnBoltedEffect(
                int localClientNum,
                const FxEffectDef *def,
                int msecBegin,
                unsigned int dobjHandle,
                int boneIndex);
unsigned int __cdecl FX_PlayBoltedEffect_DynEnt(
                int localClientNum,
                const FxEffectDef *def,
                int startMsec,
                int absDynEntId);
unsigned int __cdecl FX_PlayBoltedEffect(
                int localClientNum,
                const FxEffectDef *def,
                int startMsec,
                unsigned int dobjHandle,
                int boneIndex);
unsigned int __cdecl FX_PlayBoltedEffect(
                int localClientNum,
                const FxEffectDef *def,
                int startMsec,
                unsigned int dobjHandle,
                unsigned int boneIndex,
                const float *origin,
                const float (*axis)[3]);
void __cdecl FX_RetriggerEffect(int localClientNum, FxEffectContainer *effect, int msecBegin);
void __cdecl FX_GetTrailHandleList_Last(
                FxSystem *system,
                FxEffect *effect,
                unsigned __int16 *outHandleList,
                int *outTrailCount);
void __cdecl FX_RetriggerEffect(int localClientNum, unsigned int hEffect, int msecBegin);
void __cdecl FX_ThroughWithEffect(int localClientNum, unsigned int hEffect, bool forcekill);
void __cdecl FX_ThroughWithEffect(FxSystem *system, FxEffectContainer *effect, bool forcekill);
void __cdecl FX_StopEffect(FxSystem *system, FxEffectContainer *effect);
void __cdecl FX_StopEffectNonRecursive(FxSystem *system, FxEffectContainer *effect);
void __cdecl FX_KillEffect(FxSystem *system, FxEffectContainer *effect);
void __cdecl FX_RemoveAllEffectElems(FxSystem *system, FxEffectContainer *effect);
void __cdecl FX_KillEffectDef(int localClientNum, const FxEffectDef *def);
void __cdecl FX_KillAllEffects(int localClientNum);
int __cdecl FX_GetEffectPlayedTime(int localClientNum, unsigned int hEffect);
bool __cdecl FX_IsEffectDone(int localClientNum, unsigned int hEffect);
int __cdecl FX_MakePhysId(const FxElemDef *elemDef, const float *origin, const float *vel, int time, int sequence);
void __cdecl FX_SpawnTrailElem_NoCull(
                FxSystem *system,
                FxEffect *effect,
                FxEffect *remoteEffect,
                FxTrail *trail,
                const FxSpatialFrame *effectFrameWhenPlayed,
                int msecWhenPlayed,
                float distanceWhenPlayed);
void __cdecl FX_TrailElem_CompressBasis(const float (*inBasis)[3], char (*outBasis)[3]);
int __cdecl FX_TrailElemToHandle(FxSystem *system, FxTrailElem *trailElem);
FxPool<FxTrailElem,FxTrailElem> *__cdecl FX_AllocTrailElem(FxEffect *remoteEffect, FxSystem *system);
void __cdecl FX_SpawnTrailElem_Cull(
                FxSystem *system,
                FxEffect *effect,
                FxEffect *remoteEffect,
                FxTrail *trail,
                const FxSpatialFrame *effectFrameWhenPlayed,
                int msecWhenPlayed,
                float distanceWhenPlayed);
bool __cdecl FX_CullTrailElem(
                const FxCamera *camera,
                const FxElemDef *elemDef,
                const float *origin,
                unsigned __int8 sequence);
void __cdecl FX_SpawnSpotLightElem(FxSystem *system, FxElem *elem);
bool __cdecl FX_ImpactMarkWithinRange(const FxSystem *system, const float *markLocation);
void __cdecl FX_SpawnElem(
                FxSystem *system,
                FxEffect *effect,
                FxEffectContainer *remoteEffect,
                int elemDefIndex,
                FxSpatialFrame *effectFrameWhenPlayed,
                int msecWhenPlayed,
                float distanceWhenPlayed,
                int sequence);
FxPool<FxElem,FxElemContainer> *__cdecl FX_AllocElem(FxEffect *remoteEffect, FxSystem *system);
void __cdecl FX_SpawnRunner(
                FxSystem *system,
                FxEffect *effect,
                const FxElemDef *remoteElemDef,
                const FxSpatialFrame *effectFrameWhenPlayed,
                int randomSeed,
                int msecWhenPlayed);
void __cdecl FX_RandomlyRotateAxis(const float (*axisIn)[3], int randomSeed, float (*axisOut)[3]);
bool __cdecl FX_SpawnModelPhysics(
                FxSystem *system,
                FxEffect *effect,
                const FxElemDef *elemDef,
                int randomSeed,
                FxElem *elem,
                int sequence);
void __cdecl Vec4Scale(const float *v, float scale, float *result);
void __cdecl FX_GetOriginForElem(
                FxEffect *effect,
                const FxElemDef *elemDef,
                FxSpatialFrame *effectFrameWhenPlayed,
                int randomSeed,
                float *outOrigin);
void __cdecl FX_SpawnSound(
                int localClientNumber,
                FxEffect *effect,
                const FxElemDef *elemDef,
                const FxSpatialFrame *effectFrameWhenPlayed,
                int randomSeed);
unsigned __int8 __cdecl FX_ElemSpawnVisBits(const FxSystem *system, const FxElemDef *elemDef, const float *origin);
void __cdecl FX_FreeElem(
                FxSystem *system,
                unsigned __int16 elemHandle,
                FxEffect *effect,
                FxEffectContainer *remoteEffect,
                unsigned int elemClass);
void __cdecl FX_FreeTrailElem(
                FxSystem *system,
                unsigned __int16 trailElemHandle,
                FxEffect *effect,
                FxEffectContainer *remoteEffect,
                FxTrail *trail);
void __cdecl FX_FreeSpotLightElem(FxSystem *system, unsigned __int16 elemHandle, FxEffectContainer *effect);
double __cdecl FX_GetClientVisibility(int localClientNum, const float *start, const float *end);
bool __cdecl FX_ClientVisibilityTest(int localClientNum, const float *start, const float *end);
char __cdecl FX_GetActiveSpotLightBoltDobj(int localClientNum, int *dobjHandleOut);

//int __cdecl FX_PoolToHandle_Generic<FxTrailElem,FxTrailElem,2048>(
//                FxPool<FxTrailElem,FxTrailElem> *poolArray,
//                FxTrailElem *item_slim);
//int __cdecl FX_PoolToHandle_Generic<FxTrail,FxTrail,128>(FxPool<FxTrail,FxTrail> *poolArray, FxTrail *item_slim);
//

extern int fx_serverVisClient;