#pragma once
#include <bgame/bg_local.h>
#include <xanim/dobj.h>

void __cdecl BG_Player_DoControllers(const CEntPlayerInfo *player, const DObj *obj, int *partBits);
void __cdecl CG_Destructible_DoControllers(const cpose_t *pose, const DObj *obj, int *partBits);
void __cdecl CG_Actor_DoControllers(const cpose_t *pose, const DObj *obj, int *partBits);
void __cdecl CG_DoControllers(const cpose_t *pose, const DObj *obj, int *partBits);
void __cdecl CG_Player_DoControllers(const cpose_t *pose, const DObj *obj, int *partBits);
void    CG_Vehicle_DoControllers(const cpose_t *pose, const DObj *obj, int *partBits);
void __cdecl CG_mg42_DoControllers(const cpose_t *pose, const DObj *obj, int *partBits);
void __cdecl CG_DoBaseOriginController(const cpose_t *pose, const DObj *obj, int *setPartBits);
void __cdecl MatrixTransformVectorQuatTrans(const float *in, const DObjAnimMat *mat, float *out);
void __cdecl QuatMultiplyEquals(const float *in, float *inout);
void __cdecl CG_RuntimeXModel_Pose(const cpose_t *pose, DObj *obj);
void __cdecl CG_RuntimeXModel_PlayerHit(int localClientNum, int yawByte, int pitchByte); // mod
void __cdecl CG_RuntimeXModel_PlayerState(int localClientNum, const struct playerState_s *ps); // mod
bool __cdecl CG_RuntimeXModel_KnifeOut(); // mod
bool __cdecl CG_RuntimeXModel_ClipOut(); // mod: the local player's magazine out, through RE4's reload
bool __cdecl CG_RuntimeXModel_GrenadeOut(); // mod
// mod (co-op): another player's weapon event (CG_EntityEvent), for his runtime model's actions
void __cdecl CG_RuntimeXModel_RemoteEvent(int localClientNum, int clientNum, int event, unsigned int weapon);
bool __cdecl CG_RuntimeXModel_RemoteLaserOn(int localClientNum, int clientNum);
DObjAnimMat *__cdecl CG_DObjCalcPose(const cpose_t *remote_pose, DObj *obj, int *partBits);
