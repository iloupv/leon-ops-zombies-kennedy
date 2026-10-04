#pragma once
#include "dobj.h"
#include <universal/com_math.h>

struct __declspec(align(8)) XAnimCalcAnimInfo // sizeof=0x7830
{
    DObjAnimMat rotTransArray[960];
    bitarray<160> animPartBits;
    bitarray<160> ignorePartBits;
    bool bLeftHandGripIKActive;
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte

    XAnimCalcAnimInfo();
};

struct XAnimInfo;
struct XAnimParts;
struct XAnimDeltaPart;

void    DObjCalcAnim(DObj *obj, int *partBits);
void __cdecl XAnimCalc(
                const DObj *obj,
                XAnimInfo *info,
                float weightScale,
                bool bClear,
                bool bNormQuat,
                XAnimCalcAnimInfo *animInfo,
                int rotTransArrayIndex,
                DObjAnimMat *rotTransArray);
bool __cdecl IsInfoAdditive(const XAnimInfo *info);
void __cdecl XAnimClearRotTransArray(const DObj *obj, DObjAnimMat *rotTransArray, XAnimCalcAnimInfo *info);
void __cdecl XAnimCalcLeaf(XAnimInfo *info, float weightScale, DObjAnimMat *rotTransArray, XAnimCalcAnimInfo *animInfo);
void __cdecl XAnimCalcLeafInternal(
                const XAnimParts *parts,
                unsigned __int16 animToModelIndex,
                float time,
                float weightScale,
                DObjAnimMat *rotTransArray,
                bitarray<160> *animPartBits,
                bitarray<160> *ignorePartBits);
void __cdecl XAnimCalcNonLoopEnd(
                const XAnimParts *parts,
                const unsigned __int8 *animToModel,
                float weightScale,
                DObjAnimMat *rotTransArray,
                bitarray<160> *ignorePartBits);
void __cdecl Vec4Mad(const float *start, float scale, const float *dir, float *result);
DObjAnimMat *__cdecl XAnimGetCalcBuffer(XAnimCalcAnimInfo *info, const DObj *obj, int *rotTransArrayIndex);
void __cdecl XAnimScaleRotTransArray(int numBones, const XAnimCalcAnimInfo *info, DObjAnimMat *rotTransArray);
void __cdecl XAnimNormalizeRotScaleTransArray(
                int numBones,
                const XAnimCalcAnimInfo *info,
                float weightScale,
                DObjAnimMat *rotTransArray);
void __cdecl XAnimMadRotTransArray(
                int numBones,
                const XAnimCalcAnimInfo *info,
                float weightScale,
                const DObjAnimMat *rotTrans,
                DObjAnimMat *totalRotTrans);
void __cdecl XAnimApplyAdditives(
                DObjAnimMat *rotTransArray,
                DObjAnimMat *additiveArray,
                float weight,
                int boneCount,
                XAnimCalcAnimInfo *info);
void __cdecl XAnim_CalcDeltaForTime(const XAnimParts *anim, float time, float *rotDelta, float4 *posDelta);
void __cdecl XAnim_CalcRotDeltaEntire(const XAnimDeltaPart *animDelta, float *rotDelta);
void __cdecl XAnim_CalcPosDeltaEntire(const XAnimDeltaPart *animDelta, float4 *posDelta);
//bool __thiscall bitarray<160>::testBit(bitarray<160> *this, unsigned int pos);
void __cdecl XAnimWeightedAccumLerpedTrans(
                const float4 *fromVec,
                const float4 *toVec,
                float keyFrameLerpFrac,
                float weightScale,
                int *dataInt,
                DObjAnimMat *totalRotTrans);
void __cdecl XAnimWeightedAccumTrans(float weightScale, float *dataInt, DObjAnimMat *totalRotTrans);
void __cdecl Short2LerpAsVec2(const __int16 *from, const __int16 *to, float frac, float *out);
