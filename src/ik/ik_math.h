#pragma once
#include "ik.h"

void __cdecl ikMatrixIdentity44(float (*out)[4]);
void __cdecl ikMatrixSet44(float (*out)[4], const float *origin, const float (*axis)[3], float scale);
void __cdecl ikMatrixTransformVector34(const float *in1, const float (*in2)[4], float *out);
void __cdecl ikQuatTransToMatrix44(float *quat, const float *trans, float (*out)[4]);
void __cdecl ikMatrix44ToQuatTrans(float (*mat)[4], float *quat, float *trans);
void __cdecl ikAxisToQuat(const float (*mat)[3], float *out);
void __cdecl ikMat44Set(float (*mat)[4], const float *v1, const float *v2, const float *v3, const float *pos);
// local variable allocation has failed, the output may be wrong!
void    ikCalcBoneModelMatrix_r(
                IKState *ikState,
                int boneNum,
                float *inMatArray,
                float (*out)[4]);
void __cdecl ikCalcBoneModelMatrix(IKState *ikState, int boneNum, float *inMatArray, float (*out)[4]);
void __cdecl ikSolve2D(
                const float *BaseJoint,
                const float *ModelTarget,
                float UpperIKc,
                float LowerIKc,
                float UpperIKInvc,
                float LowerIKInvc,
                float *ModelTargetDir,
                float *SinUpper,
                float *CosUpper,
                float *SinLower,
                float *CosLower);
// local variable allocation has failed, the output may be wrong!
void    ikMap2DTo3D(
                float UpperLimbLength,
                float SinUpper,
                float CosUpper,
                float SinLower,
                float CosLower,
                const float *ModelBaseJoint,
                const float *ModelTargetDir,
                const float *ModelMidJointDir,
                float sinTwist,
                float cosTwist,
                float (*UpperLocalToModel)[4],
                float (*LowerLocalToModel)[4]);
// local variable allocation has failed, the output may be wrong!
void    ikSolveLegJoint(
                IKState *ikState,
                IKJointBones *jointBones,
                IKJointVars *jointVars,
                float *footPos,
                float (*outParentMat)[4],
                float (*outJointMat)[4],
                bool flipHack);
// local variable allocation has failed, the output may be wrong!
void    ikSolveArmJoint(
                IKState *ikState,
                IKJointBones *jointBones,
                IKJointVars *jointVars,
                float *handPos,
                float (*outParentMat)[4],
                float (*outJointMat)[4],
                bool flipHack);
void __cdecl IK_FlipHack(float (*mat)[4]);
void __cdecl ikNormalizedMatrixAssert_func(float (*mat)[4]);
// local variable allocation has failed, the output may be wrong!
void    ikRotateBone(IKState *ikState, IKBoneNames boneName, float *rot, bool local);
void __cdecl ikMatrixLerp44(float (*from)[4], float (*to)[4], float lerp, float (*res)[4]);

extern float fMaxFootReachSubtract;
extern float fMaxHandReachSubtract;