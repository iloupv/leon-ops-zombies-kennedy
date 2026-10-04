#pragma once

enum eFoot : __int32
{
    FOOTSTEP_FRONTRIGHT = 0x0,
    FOOTSTEP_FRONTLEFT  = 0x1,
    FOOTSTEP_REARRIGHT  = 0x2,
    FOOTSTEP_REARLEFT   = 0x3,
    FOOTSTEP_COUNT      = 0x4,
};

struct DObj;
struct centity_s;
struct XAnimClientNotify;
struct cg_s;
struct GfxSceneEntity;
struct cpose_t;
struct ComPrimaryLight;
struct DObjAnimMat;
struct XAnim_s;
struct trajectory_t;
struct XModel;
struct DObjModel_s;
struct WeaponDef;
struct GfxLight;

struct DelayListInfo // sizeof=0xC
{                                       // XREF: ?CG_AddPacketEntities@@YAHH@Z/r
    centity_s *head;
    centity_s *latest;
    int size;
};

unsigned int __cdecl CG_GetShadowHintForRefEntity(__int16 eFlags);
void __cdecl CG_Player_PreControllers(DObj *obj, centity_s *cent);
void __cdecl CG_mg42_OverheatUpdate(int localClientNum, const DObj *obj, centity_s *cent);
void __cdecl CG_mg42_PreControllers(DObj *obj, centity_s *cent);
// local variable allocation has failed, the output may be wrong!
void    CG_UpdateBModelWorldBounds(int localClientNum, centity_s *cent, int forceFilter);
bool __cdecl VecLessThan(float *a, float *b);
double __cdecl CG_ScriptMover_GetBurnFraction(int localClientNum, centity_s *cent);
DObj *__cdecl CG_ScriptMover_GetDObj(int localClientNum, centity_s *cent);
// local variable allocation has failed, the output may be wrong!
void    CG_AdjustPositionForMover(
                int localClientNum,
                const float *in,
                int moverNum,
                int fromTime,
                int toTime,
                float *out,
                float *outDeltaAngles);
void __cdecl LerpAngleVector(float *from, const float *to, float frac, float *result);
bool __cdecl ShouldAdjustPositionForMover(const centity_s *cent);
void __cdecl CG_SetFrameInterpolation(int localClientNum);
void __cdecl CScr_GetFootColor(eFoot foot, float *color);
void __cdecl CG_PlayClientSoundNoteTracks(
                int localClientNum,
                int entnum,
                float *origin,
                const char *notifyName,
                bool isViewArms);
bool __cdecl CG_isSoundAlias(const char *notifyName);
void __cdecl CG_DebugDrawFootFalls(int localClientNum, const centity_s *cent, eFoot foot);
void __cdecl MatrixNegateXY(const float (*in)[3], float (*out)[3]);
void __cdecl MatrixSwapXYNegateX(const float (*in)[3], float (*out)[3]);
void __cdecl CG_DoFootsteps(int localClientNum, centity_s *cent);
void __cdecl CG_DoFootStep(int localClientNum, centity_s *cent, eFoot foot);
void __cdecl CG_ProcessClientNote(
                XAnimClientNotify *note,
                unsigned int entityNum,
                cg_s *cgameGlob,
                centity_s *cent,
                int localClientNum);
void __cdecl CScr_AddEntity(centity_s *cent, int clientNum);
void __cdecl CG_ProcessFakeEntClientNoteTracks(int localClientNum, int entityNum);
void __cdecl CG_AddPacketEntity(int localClientNum, unsigned int entnum);
void __cdecl CG_UpdateClientDobjPartBits(centity_s *cent, unsigned int entnum, int localClientNum);
int __cdecl CG_AddPacketEntities(int localClientNum);
void __cdecl CG_ProcessFakeEntity(int localClientNum, struct fake_centity_s *fakeEnt);
void __cdecl CG_General(int localClientNum, centity_s *cent);
void __cdecl CG_GetLightingOrigin(centity_s *cent, float *lightingOrigin);
void __cdecl CG_FakeEntityEffects(int localClientNum, centity_s *cent);
void __cdecl CG_ScriptMover(int localClientNum, centity_s *cent);
void __cdecl CG_LoopFx(int localClientNum, centity_s *cent);
void __cdecl CG_PrimaryLight(int localClientNum, centity_s *cent);
unsigned int __cdecl Com_GetPrimaryLightCount();
const ComPrimaryLight *__cdecl Com_GetPrimaryLight(unsigned int primaryLightIndex);
void __cdecl CG_ClampPrimaryLightOrigin(GfxLight *light, const ComPrimaryLight *refLight);
void __cdecl CG_ClampPrimaryLightDir(GfxLight *light, const ComPrimaryLight *refLight);
bool __cdecl Vec3IsNormalized(const float *v);
bool __cdecl CG_ShouldDelayEntityPacketPostPS(int localClientNum, centity_s *cent);
bool __cdecl CG_IsCarriedTurretEnt(int localClientNum, centity_s *cent);
void __cdecl UpdatePacketEnt(int localClientNum, int entnum, int timeNow, int *postPSEntNum, bool contextKey);
void __cdecl DelayListInit(DelayListInfo *listInfo);
void __cdecl DelayListAdd(DelayListInfo *listInfo, centity_s *cent);
bool __cdecl EntPacketUpdateNeedsDelayed(int localClientNum, centity_s *cent, bool contextKey);
void __cdecl UpdateDelayedPacketEnts(int localClientNum, DelayListInfo *delayList, int *postPSEntNum, bool contextKey);
void __cdecl DelayListRemove(DelayListInfo *listInfo, centity_s *cent, centity_s *centPrev);
void __cdecl CG_DObjUpdateInfo(const cg_s *cgameGlob, DObj *obj, int notifyFlags);
int __cdecl CG_DObjGetWorldBoneMatrix(const cpose_t *pose, DObj *obj, int boneIndex, float (*tagMat)[3], float *origin);
DObjAnimMat *__cdecl CG_DObjGetLocalBoneMatrix(const cpose_t *pose, DObj *obj, int boneIndex);
int __cdecl CG_DObjGetWorldTagMatrix(
                const cpose_t *pose,
                DObj *obj,
                unsigned int tagName,
                float (*tagMat)[3],
                float *origin);
DObjAnimMat *__cdecl CG_DObjGetLocalTagMatrix(const cpose_t *pose, DObj *obj, unsigned int tagName);
int __cdecl CG_DObjGetWorldTagPos(const cpose_t *pose, DObj *obj, unsigned int tagName, float *pos);
const cpose_t *__cdecl CG_GetPose(int localClientNum, unsigned int handle);
unsigned int __cdecl CG_StartFx(int localClientNum, centity_s *cent, int startAtTime);
void __cdecl CG_Fx(int localClientNum, centity_s *cent);
bool __cdecl IsLocalClientDriving(const cg_s *cgameGlob, int vehicleEntNum);
char __cdecl AnyLocalClientDriving(centity_s *cent);
bool __cdecl ShouldInterpolateFromNitrousVehicleLocally(const cg_s *cgameGlob, centity_s *cent);
bool __cdecl ShouldUpdateNitrousVehicleFromNetwork(const cg_s *cgameGlob);
void __cdecl CG_CreateRagdollObject(int localClientNum, centity_s *cent);
void __cdecl CG_CalcEntityLerpPositions(int localClientNum, centity_s *cent);
void __cdecl CG_InterpolateEntityPosition(cg_s *cgameGlob, centity_s *cent, int localClientNum);
struct fake_centity_s *__cdecl CG_GetFakeEntity(int localClientNum, int entityIndex);
void __cdecl CG_InterpolateVehicle(cg_s *cgameGlob, centity_s *cent, int curTime);
void __cdecl CG_LerpTrajectory(
                const cg_s *cgameGlob,
                trajectory_t *trFrom,
                trajectory_t *trTo,
                float lerp,
                trajectory_t *trResult,
                bool isAngle);
void __cdecl CG_InterpolateVehicleDriver(const cg_s *cgameGlob, centity_s *cent, int curTime);
void __cdecl CG_CalcEntityRagdollPositions(int localClientNum, centity_s *cent);
void __cdecl CG_UpdateRagdollPose(centity_s *cent);
void __cdecl CG_UpdateTags(centity_s *ent);
DObj *__cdecl CG_PreProcess_GetDObj(int localClientNum, int entIndex, int entType, XModel *model, XModel *model2);
XAnim_s *__cdecl CG_GetAnimations(int localClientNum, int entIndex, int entType);
XAnim_s *__cdecl CG_GetMG42Anims(centity_s *cent);
unsigned int __cdecl CG_AddClientScriptAttachedModel(
                centity_s *cent,
                DObjModel_s *dobjModels,
                unsigned int numModels,
                int localClientNum);
unsigned int __cdecl CG_AddVehicleAttachedModel(
                centity_s *cent,
                DObjModel_s *dobjModels,
                unsigned int numModels,
                int localClientNum);
unsigned int __cdecl CG_AddScriptMoverAttachedModel(
                centity_s *cent,
                DObjModel_s *dobjModels,
                unsigned int numModels,
                int localClientNum);
void __cdecl CG_DObjCalcBone(const cpose_t *pose, DObj *obj, int boneIndex);
void __cdecl CG_ClearUnion(int localClientNum, centity_s *cent);
void __cdecl CG_SetUnionType(int localClientNum, centity_s *cent);
void __cdecl CG_UpdatePoseUnion(int localClientNum, centity_s *cent);
void __cdecl CG_ProcessEntity(int localClientNum, centity_s *cent);
void __cdecl CG_Item(int localClientNum, centity_s *cent);
void __cdecl CG_EntityEffects(int localClientNum, centity_s *cent);
void __cdecl CG_mg42(int localClientNum, centity_s *cent);
void __cdecl CG_Missile(int localClientNum, centity_s *cent);
void __cdecl SetBallisticAngles(int localClientNum, centity_s *cent);
bool __cdecl IsWeaponRetrieveable(
                int localClientNum,
                const centity_s *cent,
                const WeaponDef *weapDef,
                unsigned int weapIndex);
void __cdecl CG_Vehicle(int localClientNum, centity_s *cent);
void __cdecl CG_ClientFlagCallback(int localClientNum, centity_s *cent);
void __cdecl CG_AddPacketFxEntity(int localClientNum, int entnum);
void __cdecl CG_ProcessFxEntity(int localClientNum, centity_s *cent);
void __cdecl CG_CalcFxEntityLerpPositions(centity_s *cent);
void __cdecl CG_GetPoseOrigin(const cpose_t *pose, float *origin);
void __cdecl CG_GetPoseAxis(const cpose_t *pose, float (*axis)[3]);
void __cdecl CG_GetPoseQuat(const cpose_t *pose, float *quat);
float *__cdecl CG_GetEntityOrigin(int localClientNum, unsigned int entnum);
struct GfxSkinCacheEntry *__cdecl CG_GetSkinCacheEntry(const cpose_t *pose);
void __cdecl CG_PredictiveSkinCEntity(GfxSceneEntity *sceneEnt);
void __cdecl CG_SetOrigin(centity_s *ent, const float *origin);
void __cdecl CG_SetAngle(centity_s *ent, const float *angle);
int __cdecl CG_WhatModelShouldLocalPlayerSee(
                int localClientNum,
                const cg_s *cgameGlob,
                const centity_s *cent,
                int centTeam,
                int friendlyModel,
                int enemyModel);
int __cdecl CG_GetDriverClientNumFromVehicle(int localClientNum, const centity_s *vehicle);
void __cdecl CG_ClientFlagResetAll(centity_s *cent);
void __cdecl CG_ClientFlagSet(centity_s *cent, unsigned int flagNum);
void __cdecl CG_ClientFlagClear(centity_s *cent, unsigned int flagNum);
bool __cdecl CG_ClientFlagIsActive(const centity_s *cent, unsigned int flagNum);
bool __cdecl CG_ClientFlagIsSet(const centity_s *cent, unsigned int flagNum);
char *__cdecl CG_AllocAnimTree(int size);

extern int g_processEvents;
extern const unsigned short *controller_names[6];