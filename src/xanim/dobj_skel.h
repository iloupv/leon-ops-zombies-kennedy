#pragma once

struct XModel;
struct DObj;
struct DObjAnimMat;
struct DSkel;

char __cdecl DobjHasAnyPartBits(const int *partBits);
void __cdecl DObjCalcSkel(DObj *obj, int *partBits);
void __cdecl GetControlAndDuplicatePartBits(
                const DObj *obj,
                const int *partBits,
                const int *ignorePartBits,
                const int *savedDuplicatePartBits,
                int *calcPartBits,
                int *controlPartBits);
const unsigned __int8 *__cdecl CalcSkelDuplicateBones(
                const XModel *model,
                DSkel *skel,
                int minBoneIndex,
                const unsigned __int8 *pos);
void __cdecl CalcSkelRootBonesNoParentOrDuplicate(
                const XModel *model,
                DSkel *skel,
                int minBoneIndex,
                int *calcPartBits);
void __cdecl CalcSkelRootBonesWithParent(
                const XModel *model,
                DSkel *skel,
                unsigned int minBoneIndex,
                unsigned int modelParent,
                int *calcPartBits,
                const int *controlPartBits);
void __cdecl QuatMultiplyReverseInverse(const float *in1, const float *in2, float *out);
void __cdecl MatrixTransformVectorQuatTransEquals(const DObjAnimMat *in, float *inout);
void __cdecl QuatMultiplyReverseEquals(const float *in, float *inout);
void __cdecl CalcSkelNonRootBones(
                const XModel *model,
                DSkel *skel,
                int minBoneIndex,
                int *calcPartBits,
                const int *controlPartBits);
void __cdecl DObjCalcBaseSkel(const DObj *obj, DObjAnimMat *mat, int *partBits);
void __cdecl DObjCalcBaseAnim(const DObj *obj, DObjAnimMat *mat, int *partBits);
