#pragma once
#include <xanim/dobj.h>
#include "ragdoll.h"

struct RagdollSortStruct // sizeof=0x8
{                                       // XREF: ?Ragdoll_ExplosionEvent@@YAXH_NQBMMM1M@Z/r
    float distSq;                       // XREF: Ragdoll_ExplosionEvent(int,bool,float const * const,float,float,float const * const,float)+1DB/w
    RagdollBody *body;                  // XREF: Ragdoll_ExplosionEvent(int,bool,float const * const,float,float,float const * const,float)+1EE/w
};

struct StateEnt // sizeof=0xC
{                                       // XREF: .data:stateEntries/r
    bool (__cdecl *enterFunc)(RagdollBody *, RagdollBodyState, RagdollBodyState);
    bool (__cdecl *exitFunc)(RagdollBody *, RagdollBodyState, RagdollBodyState);
    void (__cdecl *updateFunc)(RagdollBody *, int);
};

void __cdecl Ragdoll_AnimMatToMat43(const DObjAnimMat *mat, float (*out)[3]);
char __cdecl Ragdoll_RebindBody(int ragdollHandle);
char __cdecl Ragdoll_ValidateBodyObj(RagdollBody *body);
void __cdecl Ragdoll_SnapshotBaseLerpOffsets(RagdollBody *body);
void __cdecl Ragdoll_GetDObjBaseBoneMatrix(DObj *obj, unsigned __int8 boneIndex, DObjAnimMat *outMat);
bool __cdecl Ragdoll_ValidatePrecalcBoneDef(RagdollDef *def, BoneDef *bone);
void __cdecl Ragdoll_ExplosionEvent(
                int localClientNum,
                bool isCylinder,
                const float *origin,
                float innerRadius,
                float outerRadius,
                const float *impulse,
                float inScale);
bool __cdecl Ragdoll_BodyIdle(RagdollBody *body);
void __cdecl Ragdoll_GetTorsoPosition(RagdollBody *body, float *center);
void __cdecl Ragdoll_Launch(int localClientNum, int ragdollHandle, const float *force, hitLocation_t hitloc);
void __cdecl Ragdoll_LaunchUpdate(RagdollBody *body);
void __cdecl Ragdoll_Attach(int localClientNum, int ragdollHandle, int attachEnt, hitLocation_t hitloc);
void __cdecl Ragdoll_AttachUpdate(RagdollBody *body);
char __cdecl Ragdoll_EnterTunnelTest(RagdollBody *body);
void __cdecl Ragdoll_SnapshotBaseLerpBones(RagdollBody *body, BoneOrientation *snapshot);
DObjAnimMat *__cdecl Ragdoll_GetDObjLocalBoneMatrix(const cpose_t *pose, DObj *obj, unsigned __int8 boneIndex);
void __cdecl Ragdoll_SetCurrentPoseFromSnapshot(RagdollBody *body, BoneOrientation *snapshot);
char __cdecl Ragdoll_CreateBodyPhysics(RagdollBody *body);
char __cdecl Ragdoll_CreatePhysJoints(RagdollBody *body);
char __cdecl Ragdoll_CreatePhysJoint(RagdollBody *body, JointDef *jointDef, Joint *joint);
char __cdecl Ragdoll_CreatePhysObjs(RagdollBody *body);
char __cdecl Ragdoll_CreatePhysObj(RagdollBody *body, BoneDef *boneDef, Bone *bone);
char __cdecl Ragdoll_GetDObjBaseBoneOrigin(
                DObj *obj,
                const float *offset,
                const float (*axis)[3],
                unsigned __int8 boneIndex,
                float *origin);
char __cdecl Ragdoll_GetDObjBaseBoneOriginQuat(
                DObj *obj,
                const float *offset,
                const float (*axis)[3],
                unsigned __int8 boneIndex,
                float *origin,
                float *quat);
void __cdecl Ragdoll_PoseInvAxis(const cpose_t *pose, float (*invAxis)[3]);
void __cdecl Ragdoll_DestroyPhysJoints(RagdollBody *body);
void __cdecl Ragdoll_DestroyPhysObjs(RagdollBody *body);
void __cdecl Ragdoll_EstimateInitialVelocities(RagdollBody *body);
char    Ragdoll_TunnelTest(RagdollBody *body);
char __cdecl Ragdoll_BoneTrace(trace_t *trace, trace_t *revTrace, const float *start, const float *end);
void __cdecl Ragdoll_PrintTunnelFail(RagdollBody *body);
void __cdecl Ragdoll_UpdateVelocityCapture(RagdollBody *body, int __formal);
void __cdecl Ragdoll_SnapshotAnimOrientations(RagdollBody *body, BoneOrientation *snapshot);
char __cdecl Ragdoll_GetDObjWorldBoneOriginQuat(
                int localClientNum,
                const cpose_t *pose,
                DObj *obj,
                unsigned __int8 boneIndex,
                float *origin,
                float *quat);
bool __cdecl Ragdoll_EnterDead(RagdollBody *body, RagdollBodyState prevState, RagdollBodyState curState);
void __cdecl Ragdoll_RemoveBodyPhysics(RagdollBody *body);
bool __cdecl Ragdoll_ExitDead(RagdollBody *body, RagdollBodyState prevState, RagdollBodyState curState);
bool __cdecl Ragdoll_ExitDObjWait(RagdollBody *body, RagdollBodyState prevState, RagdollBodyState curState);
bool __cdecl Ragdoll_ExitIdle(RagdollBody *body, RagdollBodyState curState, RagdollBodyState newState);
bool __cdecl Ragdoll_EnterIdle(RagdollBody *body, RagdollBodyState prevState, RagdollBodyState curState);
void __cdecl Ragdoll_SnapshotBonePositions(RagdollBody *body, BoneOrientation *boneSnapshot);
void __cdecl Ragdoll_UpdateDObjWait(RagdollBody *body, int __formal);
bool __cdecl Ragdoll_EnterTunnelTest(RagdollBody *body, RagdollBodyState prevState, RagdollBodyState curState);
void __cdecl Ragdoll_UpdateRunning(RagdollBody *body, int msec);
void __cdecl Ragdoll_UpdateFriction(RagdollBody *body);
void    Ragdoll_DebugRender(RagdollBody *body);
bool __cdecl Ragdoll_CheckIdle(RagdollBody *body, int msec);
void __cdecl Ragdoll_FilterBonePositions(RagdollBody *body);
char __cdecl Ragdoll_BodyNewState(RagdollBody *body, RagdollBodyState state);
void __cdecl Ragdoll_BodyUpdate(int msec, RagdollBody *body);
void __cdecl Ragdoll_Update(int msec);
void __cdecl Ragdoll_RemoveConstraintsForRope(int rope_id);
int __cdecl Ragdoll_GetRBForBone(int entnum, unsigned int boneName);
