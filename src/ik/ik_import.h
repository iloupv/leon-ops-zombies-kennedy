#pragma once
#include "ik.h"
#include <bgame/bg_weapons.h>

struct DObjAnimMat;

void __cdecl IKImport_InitVars(IKSystem *ikSystem);
bool __cdecl IKImport_GetVar_IK_Enable();
bool __cdecl IKImport_GetVar_IK_Pelvis_Test();
bool __cdecl IKImport_GetVar_IK_Foot_Test();
bool __cdecl IKImport_GetVar_IK_Hand_Test();
bool __cdecl IKImport_GetVar_IK_Left_Hand_Lerp_Test();
bool __cdecl IKImport_GetVar_IK_Right_Hand_Lerp_Test();
int __cdecl IKImport_GetVar_IK_Debug();
int __cdecl IKImport_GetVar_IK_Hand_Tuning();
void __cdecl IKImport_GetVar_IK_Hand_Offset_Vec(float *offset);
void __cdecl IKImport_GetVar_IK_Hand_Rotation_Vec(float *rotation);
int __cdecl IKImport_GetVar_IK_Hand_Tuning(IKState *ikState);
void __cdecl IKImport_GetBoneStrings(char **outBoneStrings);
char __cdecl IKImport_GetBoneLookups(IKState *ikState, IKBoneNames ikBoneName);
char __cdecl IKImport_CalcAnimatedIKBonesBits(IKState *ikState, unsigned __int8 *model);
void __cdecl IKImport_SetIKPartBits(DObj *obj, int *partBits);
void __cdecl IKImport_RecurseSetIKPartBits(const DObj *obj, int boneIndex, int *partBits);
char __cdecl IKImport_CheckIKPartBits(IKState *ikState);
void __cdecl IKImport_AccumulateParentObjBones(
                IKState *ikState,
                const unsigned __int8 *parentList,
                int numRootBones,
                float *transArray,
                int objBoneIndex,
                DObjAnimMat *matArray,
                DObjAnimMat *localMat);
void __cdecl IKImport_GetBoneMatrixArrayModelBones();
void __cdecl IKImport_GetBoneMatrixArrayLocalBones(IKState *ikState);
void __cdecl IKImport_GetBoneMatrixArray(IKState *ikState, bool isLocalBones);
void __cdecl IKImport_ApplyIKToSkeletonModelBones();
void    IKImport_ApplyIKToSkeletonLocalBones(IKState *ikState);
void __cdecl IKImport_ApplyIKToSkeleton(IKState *ikState, bool isLocalBones);
IKState *__cdecl IKImport_FetchDObjIKState(const unsigned __int8 *model);
void __cdecl IKImport_SetDObjIKState(DObj *model, IKState *ikState);
bool __cdecl IKImport_IsIKEntity_Internal(unsigned __int8 *model);
bool __cdecl IKImport_IsIKEntity(unsigned __int8 *model);
bool __cdecl IKImport_BypassTerrainMapping(IKState *ikState);
bool __cdecl IKImport_ApplyTerrainMapping(IKState *ikState);
bool __cdecl IKImport_ApplyLeftHandIK(IKState *ikState);
bool __cdecl IKImport_ApplyRightHandIK(IKState *ikState);
void    IKImport_GetEntityXform(IKState *ikState);
void __cdecl IKImport_UpdateCollisionCache(IKState *ikState);
bool __cdecl IKImport_TraceBox(
                IKState *ikState,
                float *startPos,
                float *endPos,
                float boxRadius,
                float *tracePos,
                float *outNormal,
                int hintIndexListItem,
                float *outTraceFraction,
                bool bIgnoreStartSolid);
void __cdecl IKImport_DrawAxisOrigin(float (*mat44)[4], float colorScale);
void __cdecl IKImport_DrawLine(float *start, float *end, int colorType);
void IKImport_DebugPrint(const char *fmt, ...);
struct clientInfo_t *__cdecl IKImport_GetClientInfo(IKState *ikState);
void    IKImport_DrawDebugSkeleton(IKState *ikState);
int __cdecl IKImport_GetFrameNum();
bool __cdecl IKImport_IsMainThread();
int __cdecl IKImport_GetLocalClientIndex(IKState *ikState);
int __cdecl IKImport_GetLocalClientIndexForModel(unsigned __int8 *model);
bool __cdecl IKImport_IsServerModel(unsigned __int8 *model);
bool __cdecl IKImport_IsLocalClient(IKState *ikState);
bool __cdecl IKImport_IsLeftHandGripIKActive(IKState *ikState);
bool __cdecl IKImport_IsClient(IKState *ikState);
LerpEntityState *__cdecl IKImport_GetLerpEntityState(IKState *ikState, int inEntityNum);
gentity_s *__cdecl IKImport_GetEntityState(IKState *ikState, int inEntityNum);
char __cdecl IKImport_ConditionLayerTest(
                bool condition,
                IKLayerNames thisLayer,
                const IKLayerNames *layers,
                int arraySize);
unsigned int __cdecl IKImport_GetWeaponIndex(IKState *ikState);
const WeaponVariantDef *__cdecl IKImport_GetWeaponVariantDef(IKState *ikState);
const WeaponDef *__cdecl IKImport_GetWeaponDef(IKState *ikState);
bool __cdecl IKImport_IsVehicleRemoteControl(IKState *ikState);
double __cdecl IKImport_GetLayerLerp(IKState *ikState, IKLayerNames layerName, float layerLerpTime);
double __cdecl IKImport_SmoothLerpAdjust(float in);
double __cdecl IKImport_GetLayerLerp(IKState *ikState, IKLayerNames layerIndex);
double __cdecl IKImport_GetGunnerCrouch(IKState *ikState);
bool __cdecl IKImport_IsMountedTurret(IKState *ikState);
bool __cdecl IKImport_IsFiring(IKState *ikState);
bool __cdecl IKImport_IsCrouching(IKState *ikState);
bool __cdecl IKImport_IsProne(IKState *ikState);
char __cdecl IKImport_GetBoneControllers(IKState *ikState, float *outAngles);
void __cdecl IKImport_GetVelocity(IKState *ikState, float *velocity);
bool __cdecl IKImport_IsSpectating(IKState *ikState);
bool __cdecl IKImport_IsMoving(IKState *ikState);
int __cdecl IKImport_GetIKPriority();
void __cdecl IKImport_SetWeaponOffsetType(IKState *ikState);
void __cdecl IKImport_UpdateWeaponClass(IKState *ikState);
int __cdecl IKImport_GetActorNum(unsigned __int8 *model);
bool __cdecl IKImport_IsValidEntityType(const unsigned __int8 *model);
void __cdecl IKImport_GetPitchWeaponOffset(IKState *ikState, bool bIsPitchDirectionUp, float *weapOfsOut);
void __cdecl IKImport_InitCollisionCache(IKState *ikState);
void    IKImport_Profiler(IKState *ikState);
double __cdecl IKImport_GetMaxLayerLerp(IKState *ikState);
