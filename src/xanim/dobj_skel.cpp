#include <universal/profile.h>
#include "dobj_skel.h"
#include "dobj.h"
#include <universal/assertive.h>
#include "xanim_calc.h"
#include <clientscript/cscr_stringlist.h>
#include <ik/ik_import.h>
#include "xmodel.h"
#include <cgame_mp/cg_pose_mp.h>
#include <cgame/cg_drawtools.h>

char __cdecl DobjHasAnyPartBits(const int *partBits)
{
    int i; // [esp+0h] [ebp-4h]

    for ( i = 1; i < 5; ++i )
    {
        if ( partBits[i] )
            return 1;
    }
    return 0;
}

void __cdecl DObjCalcSkel(DObj *obj, int *partBits)
{
    const unsigned __int8 *pos; // [esp+34h] [ebp-8Ch]
    int j; // [esp+38h] [ebp-88h]
    const unsigned __int8 *modelParents; // [esp+3Ch] [ebp-84h]
    const unsigned __int8 *duplicateParts; // [esp+40h] [ebp-80h]
    unsigned int boneIndex; // [esp+44h] [ebp-7Ch]
    unsigned int boneIndexa; // [esp+44h] [ebp-7Ch]
    int controlPartBits[5]; // [esp+4Ch] [ebp-74h] BYREF
    int numModels; // [esp+60h] [ebp-60h]
    XModel *model; // [esp+64h] [ebp-5Ch]
    int calcPartBitsArchive[5]; // [esp+68h] [ebp-58h] BYREF
    unsigned int modelParent; // [esp+7Ch] [ebp-44h]
    bool bDoneIK; // [esp+83h] [ebp-3Dh]
    DSkel *skel; // [esp+84h] [ebp-3Ch]
    int calcPartBits[5]; // [esp+88h] [ebp-38h] BYREF
    int ignorePartBits[5]; // [esp+9Ch] [ebp-24h] BYREF
    int i; // [esp+B0h] [ebp-10h]
    XModel **models; // [esp+B4h] [ebp-Ch]
    bool bFinished; // [esp+BBh] [ebp-5h]
    const int *savedDuplicatePartBits; // [esp+BCh] [ebp-4h]
    int savedregs; // [esp+C0h] [ebp+0h] BYREF

    PROF_SCOPED("DObjCalcSkel");
    SVPROF_SCOPED(SVPROF_ANIM); // mod (L43): bo1_mod_svprof

    if ( !obj && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj_skel.cpp", 530, 0, "%s", "obj") )
        __debugbreak();
    skel = &obj->skel;
    if ( obj == (DObj *)-20
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj_skel.cpp", 533, 0, "%s", "skel") )
    {
        __debugbreak();
    }
    bFinished = 1;
    for ( i = 0; i < 5; ++i )
    {
        ignorePartBits[i] = skel->partBits.skel[i] | ~partBits[i];
        if ( ignorePartBits[i] != -1 )
            bFinished = 0;
    }
    if ( bFinished )
    {
        return;
    }
    else
    {
        DObjCalcAnim(obj, partBits);
        if ( !obj->duplicateParts
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj_skel.cpp",
                        578,
                        0,
                        "%s",
                        "obj->duplicateParts") )
        {
            __debugbreak();
        }
        savedDuplicatePartBits = (const int *)SL_ConvertToString(obj->duplicateParts, SCRIPTINSTANCE_SERVER);
        duplicateParts = (const unsigned __int8 *)(savedDuplicatePartBits + 5);
        GetControlAndDuplicatePartBits(obj, partBits, ignorePartBits, savedDuplicatePartBits, calcPartBits, controlPartBits);
        calcPartBitsArchive[0] = calcPartBits[0];
        calcPartBitsArchive[1] = calcPartBits[1];
        calcPartBitsArchive[2] = calcPartBits[2];
        calcPartBitsArchive[3] = calcPartBits[3];
        calcPartBitsArchive[4] = calcPartBits[4];
        numModels = obj->numModels;
        boneIndex = 0;
        pos = duplicateParts;
        models = obj->localModels;
        modelParents = (const unsigned __int8 *)&models[numModels];
        for ( j = 0; j < numModels; ++j )
        {
            model = models[j];
            modelParent = modelParents[j];
            pos = CalcSkelDuplicateBones(model, skel, boneIndex, pos);
            bDoneIK = 0;
            if ( !j && IKImport_IsIKEntity((unsigned __int8 *)obj) && !DobjHasAnyPartBits(controlPartBits) )
            {
                IK_UpdateEntity(obj->entnum - 1, (unsigned __int8 *)obj, 1, calcPartBitsArchive);
                bDoneIK = 1;
            }
            if ( modelParent == 255 )
                CalcSkelRootBonesNoParentOrDuplicate(model, skel, boneIndex, calcPartBits);
            else
                CalcSkelRootBonesWithParent(model, skel, boneIndex, modelParent, calcPartBits, controlPartBits);
            CalcSkelNonRootBones(model, skel, boneIndex + model->numRootBones, calcPartBits, controlPartBits);
            if ( !j && IKImport_IsIKEntity((unsigned __int8 *)obj) && !bDoneIK )
                IK_UpdateEntity(obj->entnum - 1, (unsigned __int8 *)obj, 0, calcPartBitsArchive);
            boneIndex += model->numBones;
        }
        if ( *pos && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj_skel.cpp", 693, 0, "%s", "!(*pos)") )
            __debugbreak();
        for ( boneIndexa = 0; boneIndexa < obj->numBones; ++boneIndexa )
        {
            if ( (skel->partBits.anim[boneIndexa >> 5] & (0x80000000 >> (boneIndexa & 0x1F))) != 0 )
            {
                if ( ((LODWORD(skel->mat[boneIndexa].quat[0]) & 0x7F800000) == 0x7F800000
                     || (LODWORD(skel->mat[boneIndexa].quat[1]) & 0x7F800000) == 0x7F800000
                     || (LODWORD(skel->mat[boneIndexa].quat[2]) & 0x7F800000) == 0x7F800000
                     || (LODWORD(skel->mat[boneIndexa].quat[3]) & 0x7F800000) == 0x7F800000)
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj_skel.cpp",
                                700,
                                0,
                                "%s",
                                "!IS_NAN((skel->localMat[boneIndex].quat)[0]) && !IS_NAN((skel->localMat[boneIndex].quat)[1]) && !IS_NAN("
                                "(skel->localMat[boneIndex].quat)[2]) && !IS_NAN((skel->localMat[boneIndex].quat)[3])") )
                {
                    __debugbreak();
                }
                if ( ((LODWORD(skel->mat[boneIndexa].trans[0]) & 0x7F800000) == 0x7F800000
                     || (LODWORD(skel->mat[boneIndexa].trans[1]) & 0x7F800000) == 0x7F800000
                     || (LODWORD(skel->mat[boneIndexa].trans[2]) & 0x7F800000) == 0x7F800000)
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj_skel.cpp",
                                701,
                                0,
                                "%s",
                                "!IS_NAN((skel->localMat[boneIndex].trans)[0]) && !IS_NAN((skel->localMat[boneIndex].trans)[1]) && !IS_NA"
                                "N((skel->localMat[boneIndex].trans)[2])") )
                {
                    __debugbreak();
                }
            }
        }
    }
}

void __cdecl GetControlAndDuplicatePartBits(
                const DObj *obj,
                const int *partBits,
                const int *ignorePartBits,
                const int *savedDuplicatePartBits,
                int *calcPartBits,
                int *controlPartBits)
{
    const char *v6; // eax
    int boneIndex; // [esp+4h] [ebp-14h]
    DSkel *skel; // [esp+8h] [ebp-10h]
    int i; // [esp+10h] [ebp-8h]
    unsigned int boneIndexLow; // [esp+14h] [ebp-4h]

    skel = (DSkel*)&obj->skel;
    if ( obj == (const DObj *)-20
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj_skel.cpp", 60, 0, "%s", "skel") )
    {
        __debugbreak();
    }
    for ( i = 0; i < 5; ++i )
    {
        skel->partBits.skel[i] |= partBits[i];
        controlPartBits[i] = skel->partBits.control[i];
        calcPartBits[i] = ~(savedDuplicatePartBits[i] | ignorePartBits[i]);
        if ( (savedDuplicatePartBits[i] & controlPartBits[i]) != 0 )
        {
            DObjDumpInfo(obj);
            for ( boneIndex = 0; boneIndex < obj->numBones; ++boneIndex )
            {
                boneIndexLow = 0x80000000 >> (boneIndex & 0x1F);
                if ( (boneIndexLow & controlPartBits[boneIndex >> 5]) != 0
                    && (boneIndexLow & savedDuplicatePartBits[boneIndex >> 5]) != 0 )
                {
                    break;
                }
            }
            v6 = va("control/meld conflict on bone %d - see the console log for details", boneIndex);
            if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj_skel.cpp", 86, 0, v6) )
                __debugbreak();
        }
    }
}

const unsigned __int8 *__cdecl CalcSkelDuplicateBones(
                const XModel *model,
                DSkel *skel,
                int minBoneIndex,
                const unsigned __int8 *pos)
{
    int boneIndex; // [esp+8h] [ebp-10h]
    DObjAnimMat *mat; // [esp+Ch] [ebp-Ch]
    int parentIndex; // [esp+10h] [ebp-8h]
    unsigned int maxBoneIndex; // [esp+14h] [ebp-4h]

    mat = skel->mat;
    maxBoneIndex = minBoneIndex + model->numBones;
    while ( 1 )
    {
        boneIndex = *pos - 1;
        if ( boneIndex >= maxBoneIndex )
            break;
        parentIndex = pos[1] - 1;
        if ( parentIndex >= boneIndex
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj_skel.cpp",
                        123,
                        0,
                        "%s",
                        "parentIndex < boneIndex") )
        {
            __debugbreak();
        }
        memcpy(&mat[boneIndex], &mat[parentIndex], sizeof(DObjAnimMat));
        pos += 2;
    }
    return pos;
}

void __cdecl CalcSkelRootBonesNoParentOrDuplicate(
                const XModel *model,
                DSkel *skel,
                int minBoneIndex,
                int *calcPartBits)
{
    DWORD v5; // eax
    int v6; // [esp+0h] [ebp-50h]
    float *v; // [esp+20h] [ebp-30h]
    float v8; // [esp+24h] [ebp-2Ch]
    int boneIndex; // [esp+2Ch] [ebp-24h]
    int maxBoneIndexHigh; // [esp+30h] [ebp-20h]
    unsigned int bits; // [esp+38h] [ebp-18h]
    DObjAnimMat *mat; // [esp+3Ch] [ebp-14h]
    int boneIndexHigh; // [esp+40h] [ebp-10h]
    int boneIndexLow; // [esp+44h] [ebp-Ch]
    int maxBoneIndexa; // [esp+4Ch] [ebp-4h]
    int maxBoneIndex; // [esp+4Ch] [ebp-4h]

    maxBoneIndexa = minBoneIndex + model->numRootBones;
    boneIndexHigh = minBoneIndex >> 5;
    maxBoneIndexHigh = (maxBoneIndexa - 1) >> 5;
    maxBoneIndex = maxBoneIndexa - 32 * (minBoneIndex >> 5);
    mat = skel->mat;
    while ( boneIndexHigh <= maxBoneIndexHigh )
    {
        bits = calcPartBits[boneIndexHigh];
        if ( maxBoneIndex > 32 )
            v6 = 32;
        else
            v6 = maxBoneIndex;
        while ( 1 )
        {
            if (!_BitScanReverse(&v5, bits))
                v5 = 63;// `CountLeadingZeros'::`2': : notFound;
            boneIndexLow = v5 ^ 0x1F;
            if ( (v5 ^ 0x1F) >= v6 )
                break;
            boneIndex = boneIndexLow + 32 * boneIndexHigh;
            if ( ((0x80000000 >> boneIndexLow) & bits) == 0
                && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj_skel.cpp", 182, 0, "%s", "bits & boneBit") )
            {
                __debugbreak();
            }
            bits &= ~(0x80000000 >> boneIndexLow);
            calcPartBits[boneIndexHigh] = bits;
            v = mat[boneIndex].quat;
            v8 = Vec4LengthSq(v);
            if ( v8 == 0.0 )
            {
                v[3] = 1.0f;
                v[7] = 2.0f;
            }
            else
            {
                v[7] = 2.0 / v8;
            }
            if ( ((LODWORD(mat[boneIndex].quat[0]) & 0x7F800000) == 0x7F800000
                 || (LODWORD(mat[boneIndex].quat[1]) & 0x7F800000) == 0x7F800000
                 || (LODWORD(mat[boneIndex].quat[2]) & 0x7F800000) == 0x7F800000
                 || (LODWORD(mat[boneIndex].quat[3]) & 0x7F800000) == 0x7F800000)
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj_skel.cpp",
                            189,
                            0,
                            "%s",
                            "!IS_NAN((mat[boneIndex].quat)[0]) && !IS_NAN((mat[boneIndex].quat)[1]) && !IS_NAN((mat[boneIndex].quat)[2]"
                            ") && !IS_NAN((mat[boneIndex].quat)[3])") )
            {
                __debugbreak();
            }
            if ( ((LODWORD(mat[boneIndex].trans[0]) & 0x7F800000) == 0x7F800000
                 || (LODWORD(mat[boneIndex].trans[1]) & 0x7F800000) == 0x7F800000
                 || (LODWORD(mat[boneIndex].trans[2]) & 0x7F800000) == 0x7F800000)
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj_skel.cpp",
                            190,
                            0,
                            "%s",
                            "!IS_NAN((mat[boneIndex].trans)[0]) && !IS_NAN((mat[boneIndex].trans)[1]) && !IS_NAN((mat[boneIndex].trans)[2])") )
            {
                __debugbreak();
            }
        }
        ++boneIndexHigh;
        maxBoneIndex -= 32;
    }
}

void __cdecl CalcSkelRootBonesWithParent(
                const XModel *model,
                DSkel *skel,
                unsigned int minBoneIndex,
                unsigned int modelParent,
                int *calcPartBits,
                const int *controlPartBits)
{
    DWORD v7; // eax
    unsigned int v8; // [esp+0h] [ebp-C8h]
    float v9; // [esp+3Ch] [ebp-8Ch]
    const DObjAnimMat *parentMat; // [esp+8Ch] [ebp-3Ch]
    DObjAnimMat *childMat; // [esp+90h] [ebp-38h]
    unsigned int boneIndex; // [esp+94h] [ebp-34h]
    unsigned int maxBoneIndexHigh; // [esp+98h] [ebp-30h]
    float quat[4]; // [esp+A0h] [ebp-28h] BYREF
    int bits; // [esp+B0h] [ebp-18h]
    const DObjAnimMat *mat; // [esp+B4h] [ebp-14h]
    unsigned int boneIndexHigh; // [esp+B8h] [ebp-10h]
    unsigned int boneIndexLow; // [esp+BCh] [ebp-Ch]
    int boneBit; // [esp+C0h] [ebp-8h]
    unsigned int maxBoneIndex; // [esp+C4h] [ebp-4h]

    maxBoneIndex = minBoneIndex + model->numRootBones;
    boneIndexHigh = minBoneIndex >> 5;
    maxBoneIndexHigh = (maxBoneIndex - 1) >> 5;
    maxBoneIndex -= 32 * (minBoneIndex >> 5);
    mat = skel->mat;
    parentMat = &mat[modelParent];
    while ( boneIndexHigh <= maxBoneIndexHigh )
    {
        bits = calcPartBits[boneIndexHigh];
        if ( (int)maxBoneIndex > 32 )
            v8 = 32;
        else
            v8 = maxBoneIndex;
        while ( 1 )
        {
            if (!_BitScanReverse(&v7, bits))
                v7 = 63;// `CountLeadingZeros'::`2': : notFound;
            boneIndexLow = v7 ^ 0x1F;
            if ( (v7 ^ 0x1Fu) >= v8 )
                break;
            boneIndex = boneIndexLow + 32 * boneIndexHigh;
            boneBit = 0x80000000 >> boneIndexLow;
            if ( ((0x80000000 >> boneIndexLow) & bits) == 0
                && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj_skel.cpp", 258, 0, "%s", "bits & boneBit") )
            {
                __debugbreak();
            }
            bits &= ~boneBit;
            calcPartBits[boneIndexHigh] = bits;
            if ( modelParent >= boneIndex
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj_skel.cpp",
                            263,
                            0,
                            "%s",
                            "modelParent < boneIndex") )
            {
                __debugbreak();
            }
            if ( (boneBit & skel->partBits.anim[boneIndexHigh]) == 0
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj_skel.cpp",
                            265,
                            0,
                            "%s",
                            "skel->partBits.anim[boneIndexHigh] & boneBit") )
            {
                __debugbreak();
            }
            if ( (skel->partBits.skel[modelParent >> 5] & (0x80000000 >> (modelParent & 0x1F))) == 0
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj_skel.cpp",
                            270,
                            0,
                            "%s",
                            "skel->partBits.skel[modelParent >> 5] & (HIGH_BIT >> (modelParent & 31))") )
            {
                __debugbreak();
            }
            if ( (skel->partBits.anim[modelParent >> 5] & (0x80000000 >> (modelParent & 0x1F))) == 0
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj_skel.cpp",
                            271,
                            0,
                            "%s",
                            "skel->partBits.anim[modelParent >> 5] & (HIGH_BIT >> (modelParent & 31))") )
            {
                __debugbreak();
            }
            childMat = &skel->mat[boneIndex];
            if ( (boneBit & controlPartBits[boneIndexHigh]) != 0 )
            {
                if ( skel->partBits.skel[0] >= 0
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj_skel.cpp",
                                289,
                                0,
                                "%s",
                                "skel->partBits.skel[0] & HIGH_BIT") )
                {
                    __debugbreak();
                }
                if ( skel->partBits.anim[0] >= 0
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj_skel.cpp",
                                290,
                                0,
                                "%s",
                                "skel->partBits.anim[0] & HIGH_BIT") )
                {
                    __debugbreak();
                }
                QuatMultiplyReverseInverse(mat->quat, parentMat->quat, quat);
                QuatMultiplyReverseEquals(quat, childMat->quat);
                QuatMultiplyEquals(mat->quat, childMat->quat);
            }
            else
            {
                QuatMultiplyEquals(parentMat->quat, childMat->quat);
            }
            if ( ((LODWORD(childMat->quat[0]) & 0x7F800000) == 0x7F800000
                 || (LODWORD(childMat->quat[1]) & 0x7F800000) == 0x7F800000
                 || (LODWORD(childMat->quat[2]) & 0x7F800000) == 0x7F800000
                 || (LODWORD(childMat->quat[3]) & 0x7F800000) == 0x7F800000)
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj_skel.cpp",
                            307,
                            0,
                            "%s",
                            "!IS_NAN((childMat->quat)[0]) && !IS_NAN((childMat->quat)[1]) && !IS_NAN((childMat->quat)[2]) && !IS_NAN((c"
                            "hildMat->quat)[3])") )
            {
                __debugbreak();
            }
            if ( ((LODWORD(childMat->trans[0]) & 0x7F800000) == 0x7F800000
                 || (LODWORD(childMat->trans[1]) & 0x7F800000) == 0x7F800000
                 || (LODWORD(childMat->trans[2]) & 0x7F800000) == 0x7F800000)
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj_skel.cpp",
                            308,
                            0,
                            "%s",
                            "!IS_NAN((childMat->trans)[0]) && !IS_NAN((childMat->trans)[1]) && !IS_NAN((childMat->trans)[2])") )
            {
                __debugbreak();
            }
            v9 = Vec4LengthSq(childMat->quat);
            if ( v9 == 0.0 )
            {
                childMat->quat[3] = 1.0f;
                childMat->transWeight = 2.0f;
            }
            else
            {
                childMat->transWeight = 2.0 / v9;
            }
            MatrixTransformVectorQuatTransEquals(parentMat, childMat->trans);
            if ( ((LODWORD(childMat->trans[0]) & 0x7F800000) == 0x7F800000
                 || (LODWORD(childMat->trans[1]) & 0x7F800000) == 0x7F800000
                 || (LODWORD(childMat->trans[2]) & 0x7F800000) == 0x7F800000)
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj_skel.cpp",
                            320,
                            0,
                            "%s",
                            "!IS_NAN((childMat->trans)[0]) && !IS_NAN((childMat->trans)[1]) && !IS_NAN((childMat->trans)[2])") )
            {
                __debugbreak();
            }
        }
        ++boneIndexHigh;
        maxBoneIndex -= 32;
    }
}

void __cdecl QuatMultiplyReverseInverse(const float *in1, const float *in2, float *out)
{
    *out = (float)((float)((float)(*in2 * in1[3]) - (float)(in2[3] * *in1)) - (float)(in2[2] * in1[1]))
             + (float)(in2[1] * in1[2]);
    out[1] = (float)((float)((float)(in2[1] * in1[3]) + (float)(in2[2] * *in1)) - (float)(in2[3] * in1[1]))
                 - (float)(*in2 * in1[2]);
    out[2] = (float)((float)((float)(in2[2] * in1[3]) - (float)(in2[1] * *in1)) + (float)(*in2 * in1[1]))
                 - (float)(in2[3] * in1[2]);
    out[3] = (float)((float)((float)(in2[3] * in1[3]) + (float)(*in2 * *in1)) + (float)(in2[1] * in1[1]))
                 + (float)(in2[2] * in1[2]);
}

void __cdecl MatrixTransformVectorQuatTransEquals(const DObjAnimMat *in, float *inout)
{
    float temp; // [esp+48h] [ebp-2Ch]
    float temp_4; // [esp+4Ch] [ebp-28h]
    float axis[3][3]; // [esp+50h] [ebp-24h] BYREF

    ConvertQuatToMat(in, axis);
    temp = (float)((float)((float)(*inout * axis[0][0]) + (float)(inout[1] * axis[1][0])) + (float)(inout[2] * axis[2][0]))
             + in->trans[0];
    temp_4 = (float)((float)((float)(*inout * axis[0][1]) + (float)(inout[1] * axis[1][1]))
                                 + (float)(inout[2] * axis[2][1]))
                 + in->trans[1];
    inout[2] = (float)((float)((float)(*inout * axis[0][2]) + (float)(inout[1] * axis[1][2]))
                                     + (float)(inout[2] * axis[2][2]))
                     + in->trans[2];
    *inout = temp;
    inout[1] = temp_4;
}

void __cdecl QuatMultiplyReverseEquals(const float *in, float *inout)
{
    float temp_4; // [esp+4h] [ebp-Ch]
    float temp_8; // [esp+8h] [ebp-8h]
    float temp_12; // [esp+Ch] [ebp-4h]

    temp_4 = (float)((float)((float)(in[1] * inout[3]) - (float)(in[2] * *inout)) + (float)(in[3] * inout[1]))
                 + (float)(*in * inout[2]);
    temp_8 = (float)((float)((float)(in[2] * inout[3]) + (float)(in[1] * *inout)) - (float)(*in * inout[1]))
                 + (float)(in[3] * inout[2]);
    temp_12 = (float)((float)((float)(in[3] * inout[3]) - (float)(*in * *inout)) - (float)(in[1] * inout[1]))
                    - (float)(in[2] * inout[2]);
    *inout = (float)((float)((float)(*in * inout[3]) + (float)(in[3] * *inout)) + (float)(in[2] * inout[1]))
                 - (float)(in[1] * inout[2]);
    inout[1] = temp_4;
    inout[2] = temp_8;
    inout[3] = temp_12;
}

void __cdecl CalcSkelNonRootBones(
                const XModel *model,
                DSkel *skel,
                int minBoneIndex,
                int *calcPartBits,
                const int *controlPartBits)
{
    DWORD v6; // eax
    int v7; // [esp+0h] [ebp-100h]
    float v8; // [esp+44h] [ebp-BCh]
    DObjAnimMat *childMat; // [esp+B4h] [ebp-4Ch]
    const DObjAnimMat *parentMat; // [esp+B8h] [ebp-48h]
    int boneIndex; // [esp+BCh] [ebp-44h]
    int maxBoneIndexHigh; // [esp+C0h] [ebp-40h]
    const float *trans; // [esp+C8h] [ebp-38h]
    float quat[4]; // [esp+CCh] [ebp-34h] BYREF
    int bits; // [esp+DCh] [ebp-24h]
    DObjAnimMat *mat; // [esp+E0h] [ebp-20h]
    int boneOffset; // [esp+E4h] [ebp-1Ch]
    const unsigned __int8 *parentList; // [esp+E8h] [ebp-18h]
    int parentOffset; // [esp+ECh] [ebp-14h]
    int boneIndexHigh; // [esp+F0h] [ebp-10h]
    int boneIndexLow; // [esp+F4h] [ebp-Ch]
    int boneBit; // [esp+F8h] [ebp-8h]
    int maxBoneIndex; // [esp+FCh] [ebp-4h]

    maxBoneIndex = minBoneIndex + model->numBones - model->numRootBones;
    boneIndexHigh = minBoneIndex >> 5;
    maxBoneIndexHigh = (maxBoneIndex - 1) >> 5;
    maxBoneIndex -= 32 * (minBoneIndex >> 5);
    mat = skel->mat;
    while ( boneIndexHigh <= maxBoneIndexHigh )
    {
        bits = calcPartBits[boneIndexHigh];
        if ( maxBoneIndex > 32 )
            v7 = 32;
        else
            v7 = maxBoneIndex;
        while ( 1 )
        {
            if (!_BitScanReverse(&v6, bits))
                v6 = 63;// `CountLeadingZeros'::`2': : notFound;
            boneIndexLow = v6 ^ 0x1F;
            if ( (v6 ^ 0x1F) >= v7 )
                break;
            boneIndex = boneIndexLow + 32 * boneIndexHigh;
            boneBit = 0x80000000 >> boneIndexLow;
            if ( ((0x80000000 >> boneIndexLow) & bits) == 0
                && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj_skel.cpp", 390, 0, "%s", "bits & boneBit") )
            {
                __debugbreak();
            }
            bits &= ~boneBit;
            calcPartBits[boneIndexHigh] = bits;
            childMat = &mat[boneIndex];
            boneOffset = boneIndex - minBoneIndex;
            parentList = &model->localParentList[boneIndex - minBoneIndex];
            parentOffset = *parentList;
            parentMat = &childMat[-parentOffset];
            if ( ((LODWORD(parentMat->quat[0]) & 0x7F800000) == 0x7F800000
                 || (LODWORD(parentMat->quat[1]) & 0x7F800000) == 0x7F800000
                 || (LODWORD(parentMat->quat[2]) & 0x7F800000) == 0x7F800000
                 || (LODWORD(parentMat->quat[3]) & 0x7F800000) == 0x7F800000)
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj_skel.cpp",
                            405,
                            0,
                            "%s",
                            "!IS_NAN((parentMat->quat)[0]) && !IS_NAN((parentMat->quat)[1]) && !IS_NAN((parentMat->quat)[2]) && !IS_NAN"
                            "((parentMat->quat)[3])") )
            {
                __debugbreak();
            }
            if ( ((LODWORD(parentMat->trans[0]) & 0x7F800000) == 0x7F800000
                 || (LODWORD(parentMat->trans[1]) & 0x7F800000) == 0x7F800000
                 || (LODWORD(parentMat->trans[2]) & 0x7F800000) == 0x7F800000)
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj_skel.cpp",
                            406,
                            0,
                            "%s",
                            "!IS_NAN((parentMat->trans)[0]) && !IS_NAN((parentMat->trans)[1]) && !IS_NAN((parentMat->trans)[2])") )
            {
                __debugbreak();
            }
            if ( (boneBit & skel->partBits.anim[boneIndexHigh]) == 0
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj_skel.cpp",
                            409,
                            0,
                            "%s",
                            "skel->partBits.anim[boneIndexHigh] & boneBit") )
            {
                __debugbreak();
            }
            if ( (skel->partBits.skel[(boneIndex - parentOffset) >> 5] & (0x80000000 >> ((boneIndex - parentOffset) & 0x1F))) == 0
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj_skel.cpp",
                            414,
                            0,
                            "%s",
                            "skel->partBits.skel[(boneIndex - parentOffset) >> 5] & (HIGH_BIT >> ((boneIndex - parentOffset) & 31))") )
            {
                __debugbreak();
            }
            if ( (skel->partBits.anim[(boneIndex - parentOffset) >> 5] & (0x80000000 >> ((boneIndex - parentOffset) & 0x1F))) == 0
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj_skel.cpp",
                            415,
                            0,
                            "%s",
                            "skel->partBits.anim[(boneIndex - parentOffset) >> 5] & (HIGH_BIT >> ((boneIndex - parentOffset) & 31))") )
            {
                __debugbreak();
            }
            if ( (boneBit & controlPartBits[boneIndexHigh]) != 0 )
            {
                if ( skel->partBits.skel[0] >= 0
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj_skel.cpp",
                                430,
                                0,
                                "%s",
                                "skel->partBits.skel[0] & HIGH_BIT") )
                {
                    __debugbreak();
                }
                if ( skel->partBits.anim[0] >= 0
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj_skel.cpp",
                                431,
                                0,
                                "%s",
                                "skel->partBits.anim[0] & HIGH_BIT") )
                {
                    __debugbreak();
                }
                QuatMultiplyReverseInverse(mat->quat, parentMat->quat, quat);
                QuatMultiplyReverseEquals(quat, childMat->quat);
                QuatMultiplyEquals(mat->quat, childMat->quat);
            }
            else
            {
                QuatMultiplyEquals(parentMat->quat, childMat->quat);
            }
            if ( ((LODWORD(childMat->quat[0]) & 0x7F800000) == 0x7F800000
                 || (LODWORD(childMat->quat[1]) & 0x7F800000) == 0x7F800000
                 || (LODWORD(childMat->quat[2]) & 0x7F800000) == 0x7F800000
                 || (LODWORD(childMat->quat[3]) & 0x7F800000) == 0x7F800000)
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj_skel.cpp",
                            448,
                            0,
                            "%s",
                            "!IS_NAN((childMat->quat)[0]) && !IS_NAN((childMat->quat)[1]) && !IS_NAN((childMat->quat)[2]) && !IS_NAN((c"
                            "hildMat->quat)[3])") )
            {
                __debugbreak();
            }
            if ( ((LODWORD(childMat->trans[0]) & 0x7F800000) == 0x7F800000
                 || (LODWORD(childMat->trans[1]) & 0x7F800000) == 0x7F800000
                 || (LODWORD(childMat->trans[2]) & 0x7F800000) == 0x7F800000)
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj_skel.cpp",
                            449,
                            0,
                            "%s",
                            "!IS_NAN((childMat->trans)[0]) && !IS_NAN((childMat->trans)[1]) && !IS_NAN((childMat->trans)[2])") )
            {
                __debugbreak();
            }
            v8 = Vec4LengthSq(childMat->quat);
            if ( v8 == 0.0 )
            {
                childMat->quat[3] = 1.0f;
                childMat->transWeight = 2.0f;
            }
            else
            {
                childMat->transWeight = 2.0 / v8;
            }
            trans = &model->localTrans[3 * boneOffset];
            childMat->trans[0] = childMat->trans[0] + *trans;
            childMat->trans[1] = childMat->trans[1] + trans[1];
            childMat->trans[2] = childMat->trans[2] + trans[2];
            MatrixTransformVectorQuatTransEquals(parentMat, childMat->trans);
            if ( ((LODWORD(childMat->trans[0]) & 0x7F800000) == 0x7F800000
                 || (LODWORD(childMat->trans[1]) & 0x7F800000) == 0x7F800000
                 || (LODWORD(childMat->trans[2]) & 0x7F800000) == 0x7F800000)
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj_skel.cpp",
                            461,
                            0,
                            "%s",
                            "!IS_NAN((childMat->trans)[0]) && !IS_NAN((childMat->trans)[1]) && !IS_NAN((childMat->trans)[2])") )
            {
                __debugbreak();
            }
        }
        ++boneIndexHigh;
        maxBoneIndex -= 32;
    }
}

void __cdecl DObjCalcBaseSkel(const DObj *obj, DObjAnimMat *mat, int *partBits)
{
    const unsigned __int8 *pos; // [esp+8h] [ebp-B4h]
    int j; // [esp+Ch] [ebp-B0h]
    const unsigned __int8 *modelParents; // [esp+10h] [ebp-ACh]
    const unsigned __int8 *duplicateParts; // [esp+14h] [ebp-A8h]
    unsigned int boneIndex; // [esp+18h] [ebp-A4h]
    int controlPartBits[5]; // [esp+1Ch] [ebp-A0h] BYREF
    int numModels; // [esp+30h] [ebp-8Ch]
    XModel *model; // [esp+34h] [ebp-88h]
    unsigned int modelParent; // [esp+38h] [ebp-84h]
    DSkel skel; // [esp+3Ch] [ebp-80h] BYREF
    int calcPartBits[5]; // [esp+88h] [ebp-34h] BYREF
    int ignorePartBits[5]; // [esp+9Ch] [ebp-20h]
    int i; // [esp+B0h] [ebp-Ch]
    XModel **models; // [esp+B4h] [ebp-8h]
    const int *savedDuplicatePartBits; // [esp+B8h] [ebp-4h]

    if ( !obj && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj_skel.cpp", 792, 0, "%s", "obj") )
        __debugbreak();
    if ( !mat && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj_skel.cpp", 793, 0, "%s", "mat") )
        __debugbreak();
    skel.mat = mat;
    skel.timeStamp = 0;
    for ( i = 0; i < 5; ++i )
    {
        skel.partBits.skel[i] = partBits[i];
        skel.partBits.anim[i] = skel.partBits.skel[i];
        skel.partBits.control[i] = 0;
        ignorePartBits[i] = ~partBits[i];
    }
    DObjCalcBaseAnim(obj, mat, partBits);
    if ( !obj->duplicateParts
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj_skel.cpp", 811, 0, "%s", "obj->duplicateParts") )
    {
        __debugbreak();
    }
    savedDuplicatePartBits = (const int *)SL_ConvertToString(obj->duplicateParts, SCRIPTINSTANCE_SERVER);
    duplicateParts = (const unsigned __int8 *)(savedDuplicatePartBits + 5);
    for ( i = 0; i < 5; ++i )
    {
        controlPartBits[i] = 0;
        calcPartBits[i] = ~(savedDuplicatePartBits[i] | ignorePartBits[i]);
    }
    numModels = obj->numModels;
    boneIndex = 0;
    pos = duplicateParts;
    models = obj->localModels;
    modelParents = (const unsigned __int8 *)&models[numModels];
    for ( j = 0; j < numModels; ++j )
    {
        model = models[j];
        modelParent = modelParents[j];
        pos = CalcSkelDuplicateBones(model, &skel, boneIndex, pos);
        if ( modelParent == 255 )
            CalcSkelRootBonesNoParentOrDuplicate(model, &skel, boneIndex, calcPartBits);
        else
            CalcSkelRootBonesWithParent(model, &skel, boneIndex, modelParent, calcPartBits, controlPartBits);
        CalcSkelNonRootBones(model, &skel, boneIndex + model->numRootBones, calcPartBits, controlPartBits);
        boneIndex += model->numBones;
    }
}

void __cdecl DObjCalcBaseAnim(const DObj *obj, DObjAnimMat *mat, int *partBits)
{
    int j; // [esp+8h] [ebp-18h]
    int boneIndex; // [esp+Ch] [ebp-14h]
    XModel *model; // [esp+14h] [ebp-Ch]
    __int16 *quats; // [esp+18h] [ebp-8h]
    int i; // [esp+1Ch] [ebp-4h]
    int ia; // [esp+1Ch] [ebp-4h]

    if ( !obj && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj_skel.cpp", 722, 0, "%s", "obj") )
        __debugbreak();
    if ( !mat && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\dobj_skel.cpp", 723, 0, "%s", "mat") )
        __debugbreak();
    boneIndex = 0;
    for ( j = 0; j < obj->numModels; ++j )
    {
        model = obj->localModels[j];
        for ( i = model->numRootBones; i; --i )
        {
            mat->quat[0] = 0.0f;
            mat->quat[1] = 0.0f;
            mat->quat[2] = 0.0f;
            mat->quat[3] = 1.0f;
            mat->trans[0] = 0.0f;
            mat->trans[1] = 0.0f;
            mat->trans[2] = 0.0f;
            ++mat;
            ++boneIndex;
        }
        quats = model->localQuats;
        ia = model->numBones - model->numRootBones;
        while ( ia )
        {
            if ( (partBits[boneIndex >> 5] & (0x80000000 >> (boneIndex & 0x1F))) != 0 )
            {
                mat->quat[0] = (float)*quats * 0.000030518509;
                mat->quat[1] = (float)quats[1] * 0.000030518509;
                mat->quat[2] = (float)quats[2] * 0.000030518509;
                mat->quat[3] = (float)quats[3] * 0.000030518509;
                mat->trans[0] = 0.0f;
                mat->trans[1] = 0.0f;
                mat->trans[2] = 0.0f;
            }
            --ia;
            ++mat;
            ++boneIndex;
            quats += 4;
        }
    }
}

