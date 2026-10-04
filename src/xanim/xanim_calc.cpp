#include "xanim_calc.h"
#include "xanim.h"
#include <demo/demo_common.h>
#include "xmodel.h"
#include <clientscript/cscr_stringlist.h>
#include <clientscript/scr_const.h>
#include <game_mp/g_spawnsystem_mp.h>

void    DObjCalcAnim(DObj *obj, int *partBits)
{
    void *v3; // esp
    const char *v4; // eax
    const char *v5; // eax
    float v6; // [esp+30h] [ebp-7904h]
    float v7; // [esp+34h] [ebp-7900h]
    float v8; // [esp+38h] [ebp-78FCh]
    __int16 *localQuats; // [esp+58h] [ebp-78DCh]
    int mm; // [esp+80h] [ebp-78B4h]
    int v11; // [esp+80h] [ebp-78B4h]
    XModel *v12; // [esp+84h] [ebp-78B0h]
    int kk; // [esp+88h] [ebp-78ACh]
    XModel **localModels; // [esp+8Ch] [ebp-78A8h]
    signed int v15; // [esp+90h] [ebp-78A4h]
    XAnimTree_s *localTree; // [esp+94h] [ebp-78A0h]
    int jj; // [esp+98h] [ebp-789Ch]
    int ii; // [esp+9Ch] [ebp-7898h]
    signed int n; // [esp+BCh] [ebp-7878h]
    DObjAnimMat *mat; // [esp+C0h] [ebp-7874h]
    DObjAnimMat *v21; // [esp+C0h] [ebp-7874h]
    char v22; // [esp+C7h] [ebp-786Dh]
    int m; // [esp+C8h] [ebp-786Ch]
    int k; // [esp+CCh] [ebp-7868h]
    int j; // [esp+D0h] [ebp-7864h]
    int i; // [esp+D4h] [ebp-7860h]
    DSkel *p_skel; // [esp+E0h] [ebp-7854h]
    XAnimInfo *AnimInfo; // [esp+E4h] [ebp-7850h]
    XAnimCalcAnimInfo v29; // [esp+E8h] [ebp-784Ch] BYREF
    //int v30; // [esp+7928h] [ebp-Ch]
    //void *v31; // [esp+792Ch] [ebp-8h]
    //void *retaddr; // [esp+7934h] [ebp+0h]

    //v30 = a1;
    //v31 = retaddr;
    //v3 = alloca(30988);
    //XAnimCalcAnimInfo::XAnimCalcAnimInfo(&v29);
    
    iassert(obj);

    p_skel = &obj->skel;
    //if ( obj == (const DObj *)-20
    //    && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim_calc.cpp", 2164, 0, "%s", "skel") )
    //{
    //    __debugbreak();
    //}
    for ( i = 0; i < 5; ++i )
        v29.animPartBits.array[i] = partBits[i];
    for ( j = 0; j < 5; ++j )
        v29.animPartBits.array[j] = ~v29.animPartBits.array[j];
    for ( k = 0; k < 5; ++k )
        v29.animPartBits.array[k] |= p_skel->partBits.anim[k];
    v29.bLeftHandGripIKActive = 0;
    for ( m = 0; m < 5; ++m )
    {
        if ( v29.animPartBits.array[m] != -1 )
        {
            v22 = 0;
            goto LABEL_22;
        }
    }
    v22 = 1;
LABEL_22:
    if ( !v22 )
    {
        mat = obj->skel.mat;
        for ( n = 0; n < obj->numBones; ++n )
        {
            //if ( bitarray<160>::testBit((bitarray<160> *)p_skel, n) )
            if (p_skel->partBits.anim.testBit(n))
            {
                if ( ((LODWORD(mat[n].quat[0]) & 0x7F800000) == 0x7F800000
                     || (LODWORD(mat[n].quat[1]) & 0x7F800000) == 0x7F800000
                     || (LODWORD(mat[n].quat[2]) & 0x7F800000) == 0x7F800000
                     || (LODWORD(mat[n].quat[3]) & 0x7F800000) == 0x7F800000)
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim_calc.cpp",
                                2197,
                                0,
                                "%s",
                                "!IS_NAN((mat[bone].quat)[0]) && !IS_NAN((mat[bone].quat)[1]) && !IS_NAN((mat[bone].quat)[2]) && !IS_NAN("
                                "(mat[bone].quat)[3])") )
                {
                    __debugbreak();
                }
                if ( ((LODWORD(mat[n].trans[0]) & 0x7F800000) == 0x7F800000
                     || (LODWORD(mat[n].trans[1]) & 0x7F800000) == 0x7F800000
                     || (LODWORD(mat[n].trans[2]) & 0x7F800000) == 0x7F800000)
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim_calc.cpp",
                                2198,
                                0,
                                "%s",
                                "!IS_NAN((mat[bone].trans)[0]) && !IS_NAN((mat[bone].trans)[1]) && !IS_NAN((mat[bone].trans)[2])") )
                {
                    __debugbreak();
                }
            }
        }
        for ( ii = 0; ii < 5; ++ii )
            p_skel->partBits.anim[ii] |= partBits[ii];
        for ( jj = 0; jj < 5; ++jj )
            v29.ignorePartBits.array[jj] = v29.animPartBits.array[jj];
        localTree = obj->localTree;
        if ( obj->localTree && localTree->children )
        {
            //bitarray<160>::setBit(&v29.ignorePartBits, 159);
            v29.ignorePartBits.setBit(159);
            AnimInfo = GetAnimInfo(localTree->children);
            XAnimCalc(obj, AnimInfo, 1.0, 1, 0, &v29, 0, obj->skel.mat);
        }
        if ( v29.bLeftHandGripIKActive )
            obj->flags |= 0x10u;
        else
            obj->flags &= ~0x10u;
        v15 = 0;
        v21 = obj->skel.mat;
        localModels = obj->localModels;
        for ( kk = 0; kk < obj->numModels; ++kk )
        {
            v12 = localModels[kk];
            for ( mm = v12->numRootBones; mm; --mm )
            {
                //if ( bitarray<160>::testBit(&v29.animPartBits, v15) )
                if ( v29.animPartBits.testBit(v15) )
                {
                    //if ( bitarray<160>::testBit((bitarray<160> *)p_skel, v15) )
                    if ( p_skel->partBits.anim.testBit(v15) )
                    {
                        if ( v15 >= obj->numBones )
                        {
                            v4 = va("model %d: '%s'", kk, localModels[kk]->name);
                            if ( !Assert_MyHandler(
                                            "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim_calc.cpp",
                                            2289,
                                            0,
                                            "%s\n\t%s",
                                            "boneIndex < obj->numBones",
                                            v4) )
                                __debugbreak();
                        }
                        if ( (LODWORD(v21->quat[0]) & 0x7F800000) == 0x7F800000
                            || (LODWORD(v21->quat[1]) & 0x7F800000) == 0x7F800000
                            || (LODWORD(v21->quat[2]) & 0x7F800000) == 0x7F800000
                            || (LODWORD(v21->quat[3]) & 0x7F800000) == 0x7F800000 )
                        {
                            v5 = va("model %d: '%s'", kk, localModels[kk]->name);
                            if ( !Assert_MyHandler(
                                            "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim_calc.cpp",
                                            2290,
                                            0,
                                            "%s\n\t%s",
                                            "(!IS_NAN((mat->quat)[0]) && !IS_NAN((mat->quat)[1]) && !IS_NAN((mat->quat)[2]) && !IS_NAN((mat->quat)[3]))",
                                            v5) )
                                __debugbreak();
                        }
                        if ( ((LODWORD(v21->trans[0]) & 0x7F800000) == 0x7F800000
                             || (LODWORD(v21->trans[1]) & 0x7F800000) == 0x7F800000
                             || (LODWORD(v21->trans[2]) & 0x7F800000) == 0x7F800000)
                            && !Assert_MyHandler(
                                        "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim_calc.cpp",
                                        2291,
                                        0,
                                        "%s",
                                        "!IS_NAN((mat->trans)[0]) && !IS_NAN((mat->trans)[1]) && !IS_NAN((mat->trans)[2])") )
                        {
                            __debugbreak();
                        }
                    }
                }
                else
                {
                    v21->quat[0] = 0.0f;
                    v21->quat[1] = 0.0f;
                    v21->quat[2] = 0.0f;
                    v21->quat[3] = 1.0f;
                    v21->trans[0] = 0.0f;
                    v21->trans[1] = 0.0f;
                    v21->trans[2] = 0.0f;
                    v21->transWeight = 0.0f;
                }
                ++v21;
                ++v15;
            }
            localQuats = v12->localQuats;
            v11 = v12->numBones - v12->numRootBones;
            while ( v11 )
            {
                //if ( bitarray<160>::testBit(&v29.animPartBits, v15) )
                if ( v29.animPartBits.testBit(v15) )
                {
                    //if ( bitarray<160>::testBit((bitarray<160> *)p_skel, v15) )
                    if ( p_skel->partBits.anim.testBit(v15) )
                    {
                        if ( ((LODWORD(v21->quat[0]) & 0x7F800000) == 0x7F800000
                             || (LODWORD(v21->quat[1]) & 0x7F800000) == 0x7F800000
                             || (LODWORD(v21->quat[2]) & 0x7F800000) == 0x7F800000
                             || (LODWORD(v21->quat[3]) & 0x7F800000) == 0x7F800000)
                            && !Assert_MyHandler(
                                        "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim_calc.cpp",
                                        2316,
                                        0,
                                        "%s",
                                        "!IS_NAN((mat->quat)[0]) && !IS_NAN((mat->quat)[1]) && !IS_NAN((mat->quat)[2]) && !IS_NAN((mat->quat)[3])") )
                        {
                            __debugbreak();
                        }
                        if ( ((LODWORD(v21->trans[0]) & 0x7F800000) == 0x7F800000
                             || (LODWORD(v21->trans[1]) & 0x7F800000) == 0x7F800000
                             || (LODWORD(v21->trans[2]) & 0x7F800000) == 0x7F800000)
                            && !Assert_MyHandler(
                                        "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim_calc.cpp",
                                        2317,
                                        0,
                                        "%s",
                                        "!IS_NAN((mat->trans)[0]) && !IS_NAN((mat->trans)[1]) && !IS_NAN((mat->trans)[2])") )
                        {
                            __debugbreak();
                        }
                    }
                }
                else
                {
                    v6 = 0.000030518509 * (float)localQuats[1];
                    v7 = 0.000030518509 * (float)localQuats[2];
                    v8 = 0.000030518509 * (float)localQuats[3];
                    v21->quat[0] = 0.000030518509 * (float)*localQuats;
                    v21->quat[1] = v6;
                    v21->quat[2] = v7;
                    v21->quat[3] = v8;
                    v21->trans[0] = 0.0f;
                    v21->trans[1] = 0.0f;
                    v21->trans[2] = 0.0f;
                    v21->transWeight = 0.0f;
                }
                --v11;
                ++v21;
                ++v15;
                localQuats += 4;
            }
        }
    }
}

void __cdecl XAnimCalc(
                const DObj *obj,
                XAnimInfo *info,
                float weightScale,
                bool bClear,
                bool bNormQuat,
                XAnimCalcAnimInfo *animInfo,
                int rotTransArrayIndex,
                DObjAnimMat *rotTransArray)
{
    XAnimInfo *firstInfo; // [esp+18h] [ebp-28h]
    XAnimInfo *secondInfo; // [esp+1Ch] [ebp-24h]
    XAnimInfo *secondInfoa; // [esp+1Ch] [ebp-24h]
    XAnimTree_s *tree; // [esp+20h] [ebp-20h]
    unsigned int secondInfoIndex; // [esp+24h] [ebp-1Ch]
    DObjAnimMat *calcBuffer; // [esp+28h] [ebp-18h]
    unsigned int firstInfoIndex; // [esp+2Ch] [ebp-14h]
    bool secondChildFound; // [esp+32h] [ebp-Eh]
    bool additiveChildExists; // [esp+33h] [ebp-Dh]
    int allocedCalcBuffer; // [esp+34h] [ebp-Ch]
    float weight; // [esp+38h] [ebp-8h]
    float weighta; // [esp+38h] [ebp-8h]
    float firstWeight; // [esp+3Ch] [ebp-4h]

    tree = obj->localTree;
    if ( !obj->localTree
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim_calc.cpp", 1853, 0, "%s", "tree") )
    {
        __debugbreak();
    }
    if ( !tree->anims
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim_calc.cpp", 1854, 0, "%s", "tree->anims") )
    {
        __debugbreak();
    }
    if ( info->animToModel )
    {
        if ( bClear )
            XAnimClearRotTransArray(obj, rotTransArray, animInfo);
        XAnimCalcLeaf(info, weightScale, rotTransArray, animInfo);
    }
    else
    {
        firstWeight = 0.0f;
        firstInfo = 0;
        for ( firstInfoIndex = info->children; firstInfoIndex; firstInfoIndex = firstInfo->next )
        {
            firstInfo = GetAnimInfo(firstInfoIndex);
            firstWeight = firstInfo->state.weight;
            if ( firstWeight != 0.0 )
            {
                if ( IsInfoAdditive(firstInfo) )
                    firstInfoIndex = 0;
                break;
            }
        }
        if ( firstInfoIndex )
        {
            if ( !firstInfo
                && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim_calc.cpp", 1940, 0, "%s", "firstInfo") )
            {
                __debugbreak();
            }
            secondChildFound = 0;
            calcBuffer = 0;
            allocedCalcBuffer = 0;
            for ( secondInfoIndex = firstInfo->next; secondInfoIndex; secondInfoIndex = secondInfo->next )
            {
                secondInfo = GetAnimInfo(secondInfoIndex);
                weight = secondInfo->state.weight;
                if ( weight != 0.0 )
                {
                    if ( IsInfoAdditive(secondInfo) )
                        break;
                    if ( !secondChildFound )
                    {
                        secondChildFound = 1;
                        if ( bClear )
                        {
                            calcBuffer = rotTransArray;
                            XAnimCalc(obj, firstInfo, firstWeight, 1, 1, animInfo, rotTransArrayIndex, rotTransArray);
                        }
                        else
                        {
                            calcBuffer = XAnimGetCalcBuffer(animInfo, obj, &rotTransArrayIndex);
                            if ( !calcBuffer )
                                return;
                            allocedCalcBuffer = 1;
                            XAnimCalc(obj, firstInfo, firstWeight, 1, 1, animInfo, rotTransArrayIndex, calcBuffer);
                        }
                    }
                    if ( !calcBuffer
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim_calc.cpp",
                                    2002,
                                    0,
                                    "%s",
                                    "calcBuffer") )
                    {
                        __debugbreak();
                    }
                    XAnimCalc(obj, secondInfo, weight, 0, 1, animInfo, rotTransArrayIndex, calcBuffer);
                }
            }
            if ( secondChildFound )
            {
                if ( !bNormQuat
                    && !bClear
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim_calc.cpp",
                                2013,
                                0,
                                "%s",
                                "bNormQuat || bClear") )
                {
                    __debugbreak();
                }
                if ( bNormQuat )
                {
                    if ( bClear )
                        XAnimNormalizeRotScaleTransArray(obj->numBones, animInfo, weightScale, rotTransArray);
                    else
                        XAnimMadRotTransArray(obj->numBones, animInfo, weightScale, calcBuffer, rotTransArray);
                }
                else
                {
                    XAnimScaleRotTransArray(obj->numBones, animInfo, rotTransArray);
                }
            }
            else
            {
                XAnimCalc(obj, firstInfo, weightScale, bClear, bNormQuat, animInfo, rotTransArrayIndex, rotTransArray);
            }
            if ( secondInfoIndex )
            {
                if ( allocedCalcBuffer || (calcBuffer = XAnimGetCalcBuffer(animInfo, obj, &rotTransArrayIndex)) != 0 )
                {
                    additiveChildExists = 0;
                    while ( secondInfoIndex )
                    {
                        secondInfoa = GetAnimInfo(secondInfoIndex);
                        if ( !IsInfoAdditive(secondInfoa)
                            && !Assert_MyHandler(
                                        "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim_calc.cpp",
                                        2079,
                                        0,
                                        "%s",
                                        "IsInfoAdditive( secondInfo )") )
                        {
                            __debugbreak();
                        }
                        weighta = secondInfoa->state.weight;
                        if ( weighta != 0.0 )
                        {
                            XAnimCalc(obj, secondInfoa, weighta, !additiveChildExists, 1, animInfo, rotTransArrayIndex, calcBuffer);
                            additiveChildExists = 1;
                        }
                        secondInfoIndex = secondInfoa->next;
                    }
                    if ( additiveChildExists )
                        XAnimApplyAdditives(rotTransArray, calcBuffer, weightScale, obj->numBones, animInfo);
                }
            }
        }
        else if ( bClear )
        {
            XAnimClearRotTransArray(obj, rotTransArray, animInfo);
        }
    }
}

bool __cdecl IsInfoAdditive(const XAnimInfo *info)
{
    if ( !info && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim_calc.cpp", 196, 0, "%s", "info") )
        __debugbreak();
    return !info->animToModel && (info->animParent.flags & 0x10) != 0;
}

void __cdecl XAnimClearRotTransArray(const DObj *obj, DObjAnimMat *rotTransArray, XAnimCalcAnimInfo *info)
{
    signed int modelPartIndex; // [esp+4h] [ebp-4h]

    for ( modelPartIndex = 0; modelPartIndex < obj->numBones; ++modelPartIndex )
    {
        //if ( !bitarray<160>::testBit(&info->ignorePartBits, modelPartIndex) )
        if ( !info->ignorePartBits.testBit(modelPartIndex) )
        {
            rotTransArray->quat[0] = 0.0f;
            rotTransArray->quat[1] = 0.0f;
            rotTransArray->quat[2] = 0.0f;
            rotTransArray->quat[3] = 0.0f;
            rotTransArray->transWeight = 0.0f;
            rotTransArray->trans[0] = 0.0f;
            rotTransArray->trans[1] = 0.0f;
            rotTransArray->trans[2] = 0.0f;
        }
        ++rotTransArray;
    }
}

void __cdecl XAnimCalcLeaf(XAnimInfo *info, float weightScale, DObjAnimMat *rotTransArray, XAnimCalcAnimInfo *animInfo)
{
    unsigned __int16 animToModel; // [esp+18h] [ebp-8h]
    XAnimParts *parts; // [esp+1Ch] [ebp-4h]

    parts = info->parts;
    animToModel = info->animToModel;
    if ( !parts && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim_calc.cpp", 1567, 0, "%s", "parts") )
        __debugbreak();
    if ( parts->bLeftHandGripIK )
        animInfo->bLeftHandGripIKActive = 1;
    XAnimCalcLeafInternal(
        parts,
        animToModel,
        info->state.currentAnimTime,
        weightScale,
        rotTransArray,
        &animInfo->animPartBits,
        &animInfo->ignorePartBits);
}

void __cdecl XAnim_GetTimeIndex_unsigned_short_(
        const XAnimTime *animTime,
        const unsigned __int16 *indices,
        int tableSize,
        int *keyFrameIndex,
        float *keyFrameLerpFrac)
{
  unsigned int low; // [esp+18h] [ebp-10h]
  unsigned int lowa; // [esp+18h] [ebp-10h]
  unsigned int frameIndex; // [esp+1Ch] [ebp-Ch]
  int index; // [esp+20h] [ebp-8h]
  int high; // [esp+24h] [ebp-4h]
  int higha; // [esp+24h] [ebp-4h]

  index = (int)(float)((float)tableSize * animTime->time);
  frameIndex = animTime->frameIndex;
  if ( frameIndex >= indices[index] )
  {
    if ( frameIndex >= indices[index + 1] )
    {
      lowa = index + 1;
      higha = tableSize;
      while ( frameIndex >= indices[++lowa] )
      {
        index = (higha + lowa) >> 1;
        if ( frameIndex >= indices[index] )
        {
          lowa = index + 1;
          if ( frameIndex < indices[index + 1] )
            goto LABEL_16;
        }
        else
        {
          higha = (higha + lowa) >> 1;
        }
      }
      index = lowa - 1;
    }
  }
  else
  {
    low = 0;
    high = (int)(float)((float)tableSize * animTime->time);
    while ( frameIndex < indices[--high] )
    {
      index = (high + low) >> 1;
      if ( frameIndex >= indices[index] )
      {
        low = index + 1;
        if ( frameIndex < indices[index + 1] )
          goto LABEL_16;
      }
      else
      {
        high = (high + low) >> 1;
      }
    }
    index = high;
  }
LABEL_16:
  if ( frameIndex < indices[index]
    && !Assert_MyHandler(
          "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim_calc.cpp",
          167,
          0,
          "%s",
          "frameIndex >= indices[index]") )
  {
    __debugbreak();
  }
  if ( frameIndex >= indices[index + 1]
    && !Assert_MyHandler(
          "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim_calc.cpp",
          168,
          0,
          "%s",
          "frameIndex < indices[index + 1]") )
  {
    __debugbreak();
  }
  *keyFrameLerpFrac = (float)(animTime->frameFrac - (float)indices[index])
                    / (float)(indices[index + 1] - indices[index]);
  *keyFrameIndex = index;
  if ( (*keyFrameLerpFrac < 0.0 || *keyFrameLerpFrac > 1.0)
    && !Assert_MyHandler(
          "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim_calc.cpp",
          176,
          1,
          "*keyFrameLerpFrac not in [0.0f, 1.0f]\n\t%g not in [%g, %g]",
          *keyFrameLerpFrac,
          0.0,
          1.0) )
  {
    __debugbreak();
  }
}

void __cdecl XAnimCalcParts_unsigned_short_(
    const XAnimParts *parts,
    const unsigned __int8 *animToModel,
    float time,
    float weightScale,
    DObjAnimMat *rotTransArray,
    bitarray<160> *ignorePartBits)
{
    int v7; // [esp+10h] [ebp-20Ch]
    int v8; // [esp+14h] [ebp-208h]
    int v9; // [esp+18h] [ebp-204h]
    int v10; // [esp+1Ch] [ebp-200h]
    XAnimNotifyInfo *notify; // [esp+20h] [ebp-1FCh]
    int *v12; // [esp+24h] [ebp-1F8h]
    __int16 *v13; // [esp+28h] [ebp-1F4h]
    unsigned __int8 *v14; // [esp+2Ch] [ebp-1F0h]
    char v15; // [esp+37h] [ebp-1E5h]
    int v16; // [esp+3Ch] [ebp-1E0h] BYREF
    int v17; // [esp+40h] [ebp-1DCh]
    int v18; // [esp+44h] [ebp-1D8h]
    unsigned __int16 *v19; // [esp+48h] [ebp-1D4h]
    unsigned __int8 *v20; // [esp+4Ch] [ebp-1D0h]
    char v21; // [esp+53h] [ebp-1C9h]
    unsigned int v22; // [esp+54h] [ebp-1C8h]
    int v23; // [esp+58h] [ebp-1C4h] BYREF
    int v24; // [esp+5Ch] [ebp-1C0h]
    int v25; // [esp+60h] [ebp-1BCh]
    unsigned __int16 *v26; // [esp+64h] [ebp-1B8h]
    DObjAnimMat *v27; // [esp+68h] [ebp-1B4h]
    float v28; // [esp+6Ch] [ebp-1B0h]
    DObjAnimMat *v29; // [esp+70h] [ebp-1ACh]
    float v30; // [esp+74h] [ebp-1A8h]
    float v31; // [esp+78h] [ebp-1A4h]
    float v32; // [esp+7Ch] [ebp-1A0h]
    float *quat; // [esp+80h] [ebp-19Ch]
    float dir[5]; // [esp+84h] [ebp-198h] BYREF
    char v35; // [esp+9Bh] [ebp-181h]
    unsigned int v36; // [esp+9Ch] [ebp-180h]
    int v37; // [esp+A0h] [ebp-17Ch] BYREF
    int v38; // [esp+A4h] [ebp-178h]
    int v39; // [esp+A8h] [ebp-174h]
    unsigned __int16 *v40; // [esp+ACh] [ebp-170h]
    float v41; // [esp+B0h] [ebp-16Ch]
    float scale; // [esp+B4h] [ebp-168h]
    float *start; // [esp+B8h] [ebp-164h]
    float result[5]; // [esp+BCh] [ebp-160h] BYREF
    char v45; // [esp+D3h] [ebp-149h]
    unsigned int v46; // [esp+D4h] [ebp-148h]
    int v47; // [esp+D8h] [ebp-144h] BYREF
    int v48; // [esp+DCh] [ebp-140h]
    int v49; // [esp+E0h] [ebp-13Ch]
    unsigned __int16 *v50; // [esp+E4h] [ebp-138h]
    int v51; // [esp+ECh] [ebp-130h] BYREF
    unsigned __int16 *v52; // [esp+F0h] [ebp-12Ch]
    float4 v53; // [esp+F4h] [ebp-128h] BYREF
    float v54; // [esp+104h] [ebp-118h] BYREF
    float4 v55; // [esp+108h] [ebp-114h] BYREF
    int v56; // [esp+11Ch] [ebp-100h] BYREF
    unsigned __int8 *v57; // [esp+120h] [ebp-FCh]
    float4 v58; // [esp+124h] [ebp-F8h] BYREF
    float v59; // [esp+134h] [ebp-E8h] BYREF
    float4 v60; // [esp+138h] [ebp-E4h] BYREF
    float v61; // [esp+148h] [ebp-D4h]
    float v62; // [esp+14Ch] [ebp-D0h]
    float v63; // [esp+150h] [ebp-CCh]
    float v64; // [esp+154h] [ebp-C8h]
    float4 frameVec; // [esp+158h] [ebp-C4h]
    int v66; // [esp+16Ch] [ebp-B0h] BYREF
    __int16 *v67; // [esp+170h] [ebp-ACh]
    float to[4]; // [esp+174h] [ebp-A8h] BYREF
    float frac; // [esp+184h] [ebp-98h] BYREF
    float from[4]; // [esp+188h] [ebp-94h] BYREF
    int keyFrameIndex; // [esp+19Ch] [ebp-80h] BYREF
    const __int16 *frame; // [esp+1A0h] [ebp-7Ch]
    float4 toVec; // [esp+1A4h] [ebp-78h] BYREF
    float keyFrameLerpFrac; // [esp+1B4h] [ebp-68h] BYREF
    float4 fromVec; // [esp+1B8h] [ebp-64h] BYREF
    const XAnimNotifyInfo *note; // [esp+1CCh] [ebp-50h]
    XAnimTime notifyTime; // [esp+1D0h] [ebp-4Ch] BYREF
    int iNote; // [esp+1DCh] [ebp-40h]
    __int16 *dataShort; // [esp+1E0h] [ebp-3Ch]
    XAnimTime animTime; // [esp+1E4h] [ebp-38h] BYREF
    unsigned int animPartIndex; // [esp+1F0h] [ebp-2Ch]
    unsigned __int8 *dataByte; // [esp+1F4h] [ebp-28h]
    int *randomDataInt; // [esp+1F8h] [ebp-24h]
    const XAnimNotifyInfo *notifies; // [esp+1FCh] [ebp-20h]
    unsigned int size; // [esp+200h] [ebp-1Ch]
    int *dataInt; // [esp+204h] [ebp-18h]
    __int16 *randomDataShort; // [esp+208h] [ebp-14h]
    unsigned __int8 *randomDataByte; // [esp+20Ch] [ebp-10h]
    unsigned int tableSize; // [esp+210h] [ebp-Ch]
    unsigned __int16 *indices; // [esp+214h] [ebp-8h]
    int modelPartIndex; // [esp+218h] [ebp-4h]

    if (!parts->numframes
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim_calc.cpp", 1000, 0, "%s", "parts->numframes"))
    {
        __debugbreak();
    }
    if (time < 0.0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim_calc.cpp", 1001, 0, "%s", "time >= 0"))
    {
        __debugbreak();
    }
    if (time >= 1.0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim_calc.cpp", 1002, 0, "%s", "time < 1.f"))
    {
        __debugbreak();
    }
    if (parts->dataByte)
        v14 = parts->dataByte;
    else
        v14 = 0;
    dataByte = v14;
    if (parts->dataShort)
        v13 = parts->dataShort;
    else
        v13 = 0;
    dataShort = v13;
    if (parts->dataInt)
        v12 = parts->dataInt;
    else
        v12 = 0;
    dataInt = v12;
    if (parts->notify)
        notify = parts->notify;
    else
        notify = 0;
    notifies = notify;
    randomDataByte = parts->randomDataByte;
    randomDataShort = parts->randomDataShort;
    randomDataInt = parts->randomDataInt;
    indices = parts->indices._2;
    animPartIndex = 0;
    size = parts->boneCount[0];
    while (animPartIndex < size)
    {
        modelPartIndex = animToModel[animPartIndex];
        if ((unsigned int)modelPartIndex >= 0xA0
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim_calc.cpp",
                1026,
                0,
                "modelPartIndex doesn't index DOBJ_MAX_PARTS\n\t%i not in [0, %i)",
                modelPartIndex,
                160))
        {
            __debugbreak();
        }
        //if (!ignorePartBits->testBit(modelPartIndex))
        if (!ignorePartBits->testBit(modelPartIndex))
            rotTransArray[modelPartIndex].quat[3] = rotTransArray[modelPartIndex].quat[3] + weightScale;
        ++animPartIndex;
    }
    XAnim_SetTime(time, parts->numframes, &animTime);
    for (iNote = 0; iNote < parts->notifyCount; ++iNote)
    {
        note = &notifies[iNote];
        if (note->name == scr_const.jumpcut)
        {
            XAnim_SetTime(note->time, parts->numframes, &notifyTime);
            if (notifyTime.frameIndex - 1 == animTime.frameIndex)
            {
                animTime.frameFrac = (float)animTime.frameIndex;
                break;
            }
        }
    }
    size += parts->boneCount[1];
    while (animPartIndex < size)
    {
        modelPartIndex = animToModel[animPartIndex];
        if ((unsigned int)modelPartIndex >= 0xA0
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim_calc.cpp",
                1222,
                0,
                "modelPartIndex doesn't index DOBJ_MAX_PARTS\n\t%i not in [0, %i)",
                modelPartIndex,
                160))
        {
            __debugbreak();
        }
        tableSize = (unsigned __int16)*dataShort++;
        if ((int)tableSize >= 64)
        {
            v50 = (unsigned __int16 *)dataShort;
            v48 = ((int)(tableSize - 1) >> 8) + 1;
            dataShort += v48 + 1;
            //if (ignorePartBits->testBit(modelPartIndex))
            if (ignorePartBits->testBit(modelPartIndex))
            {
                indices += tableSize + 1;
                v45 = 0;
                goto LABEL_54;
            }
            XAnim_GetTimeIndex_unsigned_short_(&animTime, v50, v48, &v47, &animTime.time);
            v47 <<= 8;
            v46 = tableSize - v47;
            if ((int)(tableSize - v47) > 256)
                v10 = 256;
            else
                v10 = v46;
            v49 = v10;
            v50 = &indices[v47];
            XAnim_GetTimeIndex_unsigned_short_(&animTime, v50, v10, &keyFrameIndex, &keyFrameLerpFrac);
            keyFrameIndex += v47;
            indices += tableSize + 1;
        }
        else
        {
            v50 = (unsigned __int16 *)dataShort;
            dataShort += tableSize + 1;
            //if (ignorePartBits->testBit(modelPartIndex))
            if (ignorePartBits->testBit(modelPartIndex))
            {
                v45 = 0;
                goto LABEL_54;
            }
            XAnim_GetTimeIndex_unsigned_short_(&animTime, v50, tableSize, &keyFrameIndex, &keyFrameLerpFrac);
        }
        v45 = 1;
    LABEL_54:
        if (v45)
        {
            frame = &randomDataShort[2 * keyFrameIndex];
            fromVec.v[0] = 0.0f;
            fromVec.v[1] = 0.0f;
            fromVec.v[2] = (float)*frame * 0.000030518509;
            fromVec.v[3] = (float)frame[1] * 0.000030518509;
            toVec.v[0] = 0.0f;
            toVec.v[1] = 0.0f;
            toVec.v[2] = (float)frame[2] * 0.000030518509;
            toVec.v[3] = (float)frame[3] * 0.000030518509;
            v41 = keyFrameLerpFrac;
            scale = weightScale;
            start = rotTransArray[modelPartIndex].quat;
            Vec4Lerp(fromVec.v, toVec.v, keyFrameLerpFrac, result);
            if ((float)((float)((float)((float)(result[0] * *start) + (float)(result[1] * start[1])) + (float)(result[2] * start[2])) + (float)(result[3] * start[3])) < 0.0)
                scale = -scale;
            Vec4Mad(start, scale, result, start);
        }
        ++animPartIndex;
        randomDataShort += 2 * tableSize + 2;
    }
    size += parts->boneCount[2];
    while (animPartIndex < size)
    {
        modelPartIndex = animToModel[animPartIndex];
        if ((unsigned int)modelPartIndex >= 0xA0
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim_calc.cpp",
                1270,
                0,
                "modelPartIndex doesn't index DOBJ_MAX_PARTS\n\t%i not in [0, %i)",
                modelPartIndex,
                160))
        {
            __debugbreak();
        }
        tableSize = (unsigned __int16)*dataShort++;
        if ((int)tableSize >= 64)
        {
            v40 = (unsigned __int16 *)dataShort;
            v38 = ((int)(tableSize - 1) >> 8) + 1;
            dataShort += v38 + 1;
            if (ignorePartBits->testBit(modelPartIndex))
            {
                indices += tableSize + 1;
                v35 = 0;
                goto LABEL_75;
            }
            XAnim_GetTimeIndex_unsigned_short_(&animTime, v40, v38, &v37, &animTime.time);
            v37 <<= 8;
            v36 = tableSize - v37;
            if ((int)(tableSize - v37) > 256)
                v9 = 256;
            else
                v9 = v36;
            v39 = v9;
            v40 = &indices[v37];
            XAnim_GetTimeIndex_unsigned_short_(&animTime, v40, v9, &v66, &frac);
            v66 += v37;
            indices += tableSize + 1;
        }
        else
        {
            v40 = (unsigned __int16 *)dataShort;
            dataShort += tableSize + 1;
            if (ignorePartBits->testBit(modelPartIndex))
            {
                v35 = 0;
                goto LABEL_75;
            }
            XAnim_GetTimeIndex_unsigned_short_(&animTime, v40, tableSize, &v66, &frac);
        }
        v35 = 1;
    LABEL_75:
        if (v35)
        {
            v67 = &randomDataShort[4 * v66];
            from[0] = (float)*v67 * 0.000030518509;
            from[1] = (float)v67[1] * 0.000030518509;
            from[2] = (float)v67[2] * 0.000030518509;
            from[3] = (float)v67[3] * 0.000030518509;
            to[0] = (float)v67[4] * 0.000030518509;
            to[1] = (float)v67[5] * 0.000030518509;
            to[2] = (float)v67[6] * 0.000030518509;
            to[3] = (float)v67[7] * 0.000030518509;
            v31 = frac;
            v32 = weightScale;
            quat = rotTransArray[modelPartIndex].quat;
            Vec4Lerp(from, to, frac, dir);
            if ((float)((float)((float)((float)(dir[0] * *quat) + (float)(dir[1] * quat[1])) + (float)(dir[2] * quat[2]))
                + (float)(dir[3] * quat[3])) < 0.0)
                v32 = -v32;
            Vec4Mad(quat, v32, dir, quat);
        }
        ++animPartIndex;
        randomDataShort += 4 * tableSize + 4;
    }
    size += parts->boneCount[3];
    while (animPartIndex < size)
    {
        modelPartIndex = animToModel[animPartIndex];
        if ((unsigned int)modelPartIndex >= 0xA0
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim_calc.cpp",
                1314,
                0,
                "modelPartIndex doesn't index DOBJ_MAX_PARTS\n\t%i not in [0, %i)",
                modelPartIndex,
                160))
        {
            __debugbreak();
        }
        if (!ignorePartBits->testBit(modelPartIndex))
        {
            frameVec.u[0] = 0.0f;
            frameVec.u[1] = 0.0f;
            frameVec.v[2] = (float)*dataShort * 0.000030518509;
            frameVec.v[3] = (float)dataShort[1] * 0.000030518509;
            v29 = &rotTransArray[modelPartIndex];
            v30 = weightScale;
            if ((float)((float)((float)((float)(0.0 * v29->quat[0]) + (float)(0.0 * v29->quat[1])) + (float)(frameVec.v[2] * v29->quat[2])) + (float)(frameVec.v[3] * v29->quat[3])) < 0.0)
                v30 = -v30;
            v29->quat[0] = (float)(v30 * frameVec.v[0]) + v29->quat[0];
            v29->quat[1] = (float)(v30 * frameVec.v[1]) + v29->quat[1];
            v29->quat[2] = (float)(v30 * frameVec.v[2]) + v29->quat[2];
            v29->quat[3] = (float)(v30 * frameVec.v[3]) + v29->quat[3];
        }
        ++animPartIndex;
        dataShort += 2;
    }
    size += parts->boneCount[4];
    while (animPartIndex < size)
    {
        modelPartIndex = animToModel[animPartIndex];
        if ((unsigned int)modelPartIndex >= 0xA0
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim_calc.cpp",
                1340,
                0,
                "modelPartIndex doesn't index DOBJ_MAX_PARTS\n\t%i not in [0, %i)",
                modelPartIndex,
                160))
        {
            __debugbreak();
        }
        if (!ignorePartBits->testBit(modelPartIndex))
        {
            v61 = (float)*dataShort * 0.000030518509;
            v62 = (float)dataShort[1] * 0.000030518509;
            v63 = (float)dataShort[2] * 0.000030518509;
            v64 = (float)dataShort[3] * 0.000030518509;
            v27 = &rotTransArray[modelPartIndex];
            v28 = weightScale;
            if ((float)((float)((float)((float)(v61 * v27->quat[0]) + (float)(v62 * v27->quat[1])) + (float)(v63 * v27->quat[2])) + (float)(v64 * v27->quat[3])) < 0.0)
                v28 = -v28;
            v27->quat[0] = (float)(v28 * v61) + v27->quat[0];
            v27->quat[1] = (float)(v28 * v62) + v27->quat[1];
            v27->quat[2] = (float)(v28 * v63) + v27->quat[2];
            v27->quat[3] = (float)(v28 * v64) + v27->quat[3];
        }
        ++animPartIndex;
        dataShort += 4;
    }
    animPartIndex = 0;
    size = parts->boneCount[5];
    while (animPartIndex < size)
    {
        modelPartIndex = animToModel[*dataByte++];
        if ((unsigned int)modelPartIndex >= 0xA0
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim_calc.cpp",
                1373,
                0,
                "modelPartIndex doesn't index DOBJ_MAX_PARTS\n\t%i not in [0, %i)",
                modelPartIndex,
                160))
        {
            __debugbreak();
        }
        tableSize = (unsigned __int16)*dataShort++;
        if ((int)tableSize >= 64)
        {
            v26 = (unsigned __int16 *)dataShort;
            v24 = ((int)(tableSize - 1) >> 8) + 1;
            dataShort += v24 + 1;
            if (ignorePartBits->testBit(modelPartIndex))
            {
                indices += tableSize + 1;
                v21 = 0;
                goto LABEL_116;
            }
            XAnim_GetTimeIndex_unsigned_short_(&animTime, v26, v24, &v23, &animTime.time);
            v23 <<= 8;
            v22 = tableSize - v23;
            if ((int)(tableSize - v23) > 256)
                v8 = 256;
            else
                v8 = v22;
            v25 = v8;
            v26 = &indices[v23];
            XAnim_GetTimeIndex_unsigned_short_(&animTime, v26, v8, &v56, &v59);
            v56 += v23;
            indices += tableSize + 1;
        }
        else
        {
            v26 = (unsigned __int16 *)dataShort;
            dataShort += tableSize + 1;
            if (ignorePartBits->testBit(modelPartIndex))
            {
                v21 = 0;
                goto LABEL_116;
            }
            XAnim_GetTimeIndex_unsigned_short_(&animTime, v26, tableSize, &v56, &v59);
        }
        v21 = 1;
    LABEL_116:
        if (v21)
        {
            v57 = &randomDataByte[3 * v56];
            v60.v[0] = (float)*v57;
            v60.v[1] = (float)v57[1];
            v60.v[2] = (float)v57[2];
            v60.u[3] = 0.0f;
            v20 = v57 + 3;
            v58.v[0] = (float)v57[3];
            v58.v[1] = (float)v57[4];
            v58.v[2] = (float)v57[5];
            v58.u[3] = 0.0f;
            XAnimWeightedAccumLerpedTrans(&v60, &v58, v59, weightScale, dataInt, &rotTransArray[modelPartIndex]);
        }
        ++animPartIndex;
        dataInt += 6;
        randomDataByte += 3 * tableSize + 3;
    }
    size += parts->boneCount[6];
    while (animPartIndex < size)
    {
        modelPartIndex = animToModel[*dataByte++];
        if ((unsigned int)modelPartIndex >= 0xA0
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim_calc.cpp",
                1406,
                0,
                "modelPartIndex doesn't index DOBJ_MAX_PARTS\n\t%i not in [0, %i)",
                modelPartIndex,
                160))
        {
            __debugbreak();
        }
        tableSize = (unsigned __int16)*dataShort++;
        if ((int)tableSize >= 64)
        {
            v19 = (unsigned __int16 *)dataShort;
            v17 = ((int)(tableSize - 1) >> 8) + 1;
            dataShort += v17 + 1;
            if (ignorePartBits->testBit(modelPartIndex))
            {
                indices += tableSize + 1;
                v15 = 0;
                goto LABEL_135;
            }
            XAnim_GetTimeIndex_unsigned_short_(&animTime, v19, v17, &v16, &animTime.time);
            v16 <<= 8;
            if ((int)(tableSize - v16) > 256)
                v7 = 256;
            else
                v7 = tableSize - v16;
            v18 = v7;
            v19 = &indices[v16];
            XAnim_GetTimeIndex_unsigned_short_(&animTime, v19, v7, &v51, &v54);
            v51 += v16;
            indices += tableSize + 1;
        }
        else
        {
            v19 = (unsigned __int16 *)dataShort;
            dataShort += tableSize + 1;
            if (ignorePartBits->testBit(modelPartIndex))
            {
                v15 = 0;
                goto LABEL_135;
            }
            XAnim_GetTimeIndex_unsigned_short_(&animTime, v19, tableSize, &v51, &v54);
        }
        v15 = 1;
    LABEL_135:
        if (v15)
        {
            v52 = (unsigned __int16 *)&randomDataShort[3 * v51];
            v55.v[0] = (float)*v52;
            v55.v[1] = (float)v52[1];
            v55.v[2] = (float)v52[2];
            v55.u[3] = 0.0f;
            v53.v[0] = (float)v52[3];
            v53.v[1] = (float)v52[4];
            v53.v[2] = (float)v52[5];
            v53.u[3] = 0.0f;
            XAnimWeightedAccumLerpedTrans(&v55, &v53, v54, weightScale, dataInt, &rotTransArray[modelPartIndex]);
        }
        ++animPartIndex;
        dataInt += 6;
        randomDataShort += 3 * tableSize + 3;
    }
    size += parts->boneCount[7];
    while (animPartIndex < size)
    {
        modelPartIndex = animToModel[*dataByte];
        if ((unsigned int)modelPartIndex >= 0xA0
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim_calc.cpp",
                1432,
                0,
                "modelPartIndex doesn't index DOBJ_MAX_PARTS\n\t%i not in [0, %i)",
                modelPartIndex,
                160))
        {
            __debugbreak();
        }
        if (!ignorePartBits->testBit(modelPartIndex))
            XAnimWeightedAccumTrans(weightScale, (float *)dataInt, &rotTransArray[modelPartIndex]);
        ++animPartIndex;
        ++dataByte;
        dataInt += 3;
    }
    size += parts->boneCount[8];
    while (animPartIndex < size)
    {
        modelPartIndex = animToModel[*dataByte];
        if (modelPartIndex >= 160
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim_calc.cpp",
                1446,
                0,
                "%s",
                "modelPartIndex < DOBJ_MAX_PARTS"))
        {
            __debugbreak();
        }
        if (!ignorePartBits->testBit(modelPartIndex))
            rotTransArray[modelPartIndex].transWeight = rotTransArray[modelPartIndex].transWeight + weightScale;
        ++animPartIndex;
        ++dataByte;
    }
}

void __cdecl XAnim_GetTimeIndex_unsigned_char_(
    const XAnimTime *animTime,
    const unsigned __int8 *indices,
    int tableSize,
    int *keyFrameIndex,
    float *keyFrameLerpFrac)
{
    unsigned int low; // [esp+18h] [ebp-10h]
    unsigned int lowa; // [esp+18h] [ebp-10h]
    unsigned int frameIndex; // [esp+1Ch] [ebp-Ch]
    int index; // [esp+20h] [ebp-8h]
    int high; // [esp+24h] [ebp-4h]
    int higha; // [esp+24h] [ebp-4h]

    index = (int)(float)((float)tableSize * animTime->time);
    frameIndex = animTime->frameIndex;
    if (frameIndex >= indices[index])
    {
        if (frameIndex >= indices[index + 1])
        {
            lowa = index + 1;
            higha = tableSize;
            while (frameIndex >= indices[++lowa])
            {
                index = (higha + lowa) >> 1;
                if (frameIndex >= indices[index])
                {
                    lowa = index + 1;
                    if (frameIndex < indices[index + 1])
                        goto LABEL_16;
                }
                else
                {
                    higha = (higha + lowa) >> 1;
                }
            }
            index = lowa - 1;
        }
    }
    else
    {
        low = 0;
        high = (int)(float)((float)tableSize * animTime->time);
        while (frameIndex < indices[--high])
        {
            index = (high + low) >> 1;
            if (frameIndex >= indices[index])
            {
                low = index + 1;
                if (frameIndex < indices[index + 1])
                    goto LABEL_16;
            }
            else
            {
                high = (high + low) >> 1;
            }
        }
        index = high;
    }
LABEL_16:
    if (frameIndex < indices[index]
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim_calc.cpp",
            167,
            0,
            "%s",
            "frameIndex >= indices[index]"))
    {
        __debugbreak();
    }
    if (frameIndex >= indices[index + 1]
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim_calc.cpp",
            168,
            0,
            "%s",
            "frameIndex < indices[index + 1]"))
    {
        __debugbreak();
    }
    *keyFrameLerpFrac = (float)(animTime->frameFrac - (float)indices[index])
        / (float)(indices[index + 1] - indices[index]);
    *keyFrameIndex = index;
    if ((*keyFrameLerpFrac < 0.0 || *keyFrameLerpFrac > 1.0)
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim_calc.cpp",
            176,
            1,
            "*keyFrameLerpFrac not in [0.0f, 1.0f]\n\t%g not in [%g, %g]",
            *keyFrameLerpFrac,
            0.0,
            1.0))
    {
        __debugbreak();
    }
}

void __cdecl XAnimCalcParts_unsigned_char_(
    const XAnimParts *parts,
    const unsigned __int8 *animToModel,
    float time,
    float weightScale,
    DObjAnimMat *rotTransArray,
    bitarray<160> *ignorePartBits)
{
    XAnimNotifyInfo *notify; // [esp+10h] [ebp-1BCh]
    int *v7; // [esp+14h] [ebp-1B8h]
    __int16 *v8; // [esp+18h] [ebp-1B4h]
    unsigned __int8 *v9; // [esp+1Ch] [ebp-1B0h]
    char v10; // [esp+27h] [ebp-1A5h]
    const unsigned __int8 *v11; // [esp+28h] [ebp-1A4h]
    char v12; // [esp+33h] [ebp-199h]
    const unsigned __int8 *v13; // [esp+34h] [ebp-198h]
    DObjAnimMat *v14; // [esp+38h] [ebp-194h]
    float v15; // [esp+3Ch] [ebp-190h]
    DObjAnimMat *v16; // [esp+40h] [ebp-18Ch]
    float v17; // [esp+44h] [ebp-188h]
    float v18; // [esp+4Ch] [ebp-180h]
    DObjAnimMat *v19; // [esp+50h] [ebp-17Ch]
    float dir[5]; // [esp+54h] [ebp-178h] BYREF
    char v21; // [esp+6Bh] [ebp-161h]
    unsigned __int8 *v22; // [esp+6Ch] [ebp-160h]
    float v23; // [esp+70h] [ebp-15Ch]
    float scale; // [esp+74h] [ebp-158h]
    float *start; // [esp+78h] [ebp-154h]
    float result[5]; // [esp+7Ch] [ebp-150h] BYREF
    char v27; // [esp+93h] [ebp-139h]
    unsigned __int8 *v28; // [esp+94h] [ebp-138h]
    int v29; // [esp+9Ch] [ebp-130h] BYREF
    unsigned __int16 *v30; // [esp+A0h] [ebp-12Ch]
    float4 v31; // [esp+A4h] [ebp-128h] BYREF
    float v32; // [esp+B4h] [ebp-118h] BYREF
    float4 v33; // [esp+B8h] [ebp-114h] BYREF
    int v34; // [esp+CCh] [ebp-100h] BYREF
    unsigned __int8 *v35; // [esp+D0h] [ebp-FCh]
    float4 v36; // [esp+D4h] [ebp-F8h] BYREF
    float v37; // [esp+E4h] [ebp-E8h] BYREF
    float4 v38; // [esp+E8h] [ebp-E4h] BYREF
    float v39; // [esp+F8h] [ebp-D4h]
    float v40; // [esp+FCh] [ebp-D0h]
    float v41; // [esp+100h] [ebp-CCh]
    float v42; // [esp+104h] [ebp-C8h]
    float4 frameVec; // [esp+108h] [ebp-C4h]
    int v44; // [esp+11Ch] [ebp-B0h] BYREF
    __int16 *v45; // [esp+120h] [ebp-ACh]
    float to[4]; // [esp+124h] [ebp-A8h] BYREF
    float frac; // [esp+134h] [ebp-98h] BYREF
    float from[4]; // [esp+138h] [ebp-94h] BYREF
    int keyFrameIndex; // [esp+14Ch] [ebp-80h] BYREF
    const __int16 *frame; // [esp+150h] [ebp-7Ch]
    float4 toVec; // [esp+154h] [ebp-78h] BYREF
    float keyFrameLerpFrac; // [esp+164h] [ebp-68h] BYREF
    float4 fromVec; // [esp+168h] [ebp-64h] BYREF
    const XAnimNotifyInfo *note; // [esp+17Ch] [ebp-50h]
    XAnimTime notifyTime; // [esp+180h] [ebp-4Ch] BYREF
    int iNote; // [esp+18Ch] [ebp-40h]
    __int16 *dataShort; // [esp+190h] [ebp-3Ch]
    XAnimTime animTime; // [esp+194h] [ebp-38h] BYREF
    unsigned int animPartIndex; // [esp+1A0h] [ebp-2Ch]
    unsigned __int8 *dataByte; // [esp+1A4h] [ebp-28h]
    int *randomDataInt; // [esp+1A8h] [ebp-24h]
    const XAnimNotifyInfo *notifies; // [esp+1ACh] [ebp-20h]
    unsigned int size; // [esp+1B0h] [ebp-1Ch]
    int *dataInt; // [esp+1B4h] [ebp-18h]
    __int16 *randomDataShort; // [esp+1B8h] [ebp-14h]
    unsigned __int8 *randomDataByte; // [esp+1BCh] [ebp-10h]
    unsigned int tableSize; // [esp+1C0h] [ebp-Ch]
    unsigned __int8 *indices; // [esp+1C4h] [ebp-8h]
    int modelPartIndex; // [esp+1C8h] [ebp-4h]

    if (!parts->numframes
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim_calc.cpp", 1000, 0, "%s", "parts->numframes"))
    {
        __debugbreak();
    }
    if (time < 0.0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim_calc.cpp", 1001, 0, "%s", "time >= 0"))
    {
        __debugbreak();
    }
    if (time >= 1.0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim_calc.cpp", 1002, 0, "%s", "time < 1.f"))
    {
        __debugbreak();
    }
    if (parts->dataByte)
        v9 = parts->dataByte;
    else
        v9 = 0;
    dataByte = v9;
    if (parts->dataShort)
        v8 = parts->dataShort;
    else
        v8 = 0;
    dataShort = v8;
    if (parts->dataInt)
        v7 = parts->dataInt;
    else
        v7 = 0;
    dataInt = v7;
    if (parts->notify)
        notify = parts->notify;
    else
        notify = 0;
    notifies = notify;
    randomDataByte = parts->randomDataByte;
    randomDataShort = parts->randomDataShort;
    randomDataInt = parts->randomDataInt;
    indices = parts->indices._1;
    animPartIndex = 0;
    size = parts->boneCount[0];
    while (animPartIndex < size)
    {
        modelPartIndex = animToModel[animPartIndex];
        if ((unsigned int)modelPartIndex >= 0xA0
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim_calc.cpp",
                1026,
                0,
                "modelPartIndex doesn't index DOBJ_MAX_PARTS\n\t%i not in [0, %i)",
                modelPartIndex,
                160))
        {
            __debugbreak();
        }
        if (!ignorePartBits->testBit(modelPartIndex))
            rotTransArray[modelPartIndex].quat[3] = rotTransArray[modelPartIndex].quat[3] + weightScale;
        ++animPartIndex;
    }
    XAnim_SetTime(time, parts->numframes, &animTime);
    for (iNote = 0; iNote < parts->notifyCount; ++iNote)
    {
        note = &notifies[iNote];
        if (note->name == scr_const.jumpcut)
        {
            XAnim_SetTime(note->time, parts->numframes, &notifyTime);
            if (notifyTime.frameIndex - 1 == animTime.frameIndex)
            {
                animTime.frameFrac = (float)animTime.frameIndex;
                break;
            }
        }
    }
    size += parts->boneCount[1];
    while (animPartIndex < size)
    {
        modelPartIndex = animToModel[animPartIndex];
        if ((unsigned int)modelPartIndex >= 0xA0
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim_calc.cpp",
                1222,
                0,
                "modelPartIndex doesn't index DOBJ_MAX_PARTS\n\t%i not in [0, %i)",
                modelPartIndex,
                160))
        {
            __debugbreak();
        }
        tableSize = (unsigned __int16)*dataShort++;
        v28 = dataByte;
        dataByte += tableSize + 1;
        if (ignorePartBits->testBit(modelPartIndex))
        {
            v27 = 0;
        }
        else
        {
            XAnim_GetTimeIndex_unsigned_char_(&animTime, v28, tableSize, &keyFrameIndex, &keyFrameLerpFrac);
            v27 = 1;
        }
        if (v27)
        {
            frame = &randomDataShort[2 * keyFrameIndex];
            fromVec.v[0] = 0.0f;
            fromVec.v[1] = 0.0f;
            fromVec.v[2] = (float)*frame * 0.000030518509;
            fromVec.v[3] = (float)frame[1] * 0.000030518509;
            toVec.v[0] = 0.0f;
            toVec.v[1] = 0.0f;
            toVec.v[2] = (float)frame[2] * 0.000030518509;
            toVec.v[3] = (float)frame[3] * 0.000030518509;
            v23 = keyFrameLerpFrac;
            scale = weightScale;
            start = rotTransArray[modelPartIndex].quat;
            Vec4Lerp(fromVec.v, toVec.v, keyFrameLerpFrac, result);
            if ((float)((float)((float)((float)(result[0] * *start) + (float)(result[1] * start[1]))
                + (float)(result[2] * start[2]))
                + (float)(result[3] * start[3])) < 0.0)
                scale = -scale;
            Vec4Mad(start, scale, result, start);
        }
        ++animPartIndex;
        randomDataShort += 2 * tableSize + 2;
    }
    size += parts->boneCount[2];
    while (animPartIndex < size)
    {
        modelPartIndex = animToModel[animPartIndex];
        if ((unsigned int)modelPartIndex >= 0xA0
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim_calc.cpp",
                1270,
                0,
                "modelPartIndex doesn't index DOBJ_MAX_PARTS\n\t%i not in [0, %i)",
                modelPartIndex,
                160))
        {
            __debugbreak();
        }
        tableSize = (unsigned __int16)*dataShort++;
        v22 = dataByte;
        dataByte += tableSize + 1;
        if (ignorePartBits->testBit(modelPartIndex))
        {
            v21 = 0;
        }
        else
        {
            XAnim_GetTimeIndex_unsigned_char_(&animTime, v22, tableSize, &v44, &frac);
            v21 = 1;
        }
        if (v21)
        {
            v45 = &randomDataShort[4 * v44];
            from[0] = (float)*v45 * 0.000030518509;
            from[1] = (float)v45[1] * 0.000030518509;
            from[2] = (float)v45[2] * 0.000030518509;
            from[3] = (float)v45[3] * 0.000030518509;
            to[0] = (float)v45[4] * 0.000030518509;
            to[1] = (float)v45[5] * 0.000030518509;
            to[2] = (float)v45[6] * 0.000030518509;
            to[3] = (float)v45[7] * 0.000030518509;
            v18 = weightScale;
            v19 = &rotTransArray[modelPartIndex];
            Vec4Lerp(from, to, frac, dir);
            if ((float)((float)((float)((float)(dir[0] * v19->quat[0]) + (float)(dir[1] * v19->quat[1])) + (float)(dir[2] * v19->quat[2])) + (float)(dir[3] * v19->quat[3])) < 0.0)
                v18 = -weightScale;
            Vec4Mad(v19->quat, v18, dir, v19->quat);
        }
        ++animPartIndex;
        randomDataShort += 4 * tableSize + 4;
    }
    size += parts->boneCount[3];
    while (animPartIndex < size)
    {
        modelPartIndex = animToModel[animPartIndex];
        if ((unsigned int)modelPartIndex >= 0xA0
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim_calc.cpp",
                1314,
                0,
                "modelPartIndex doesn't index DOBJ_MAX_PARTS\n\t%i not in [0, %i)",
                modelPartIndex,
                160))
        {
            __debugbreak();
        }
        if (!ignorePartBits->testBit(modelPartIndex))
        {
            frameVec.u[0] = 0.0f;
            frameVec.u[1] = 0.0f;
            frameVec.v[2] = (float)*dataShort * 0.000030518509;
            frameVec.v[3] = (float)dataShort[1] * 0.000030518509;
            v16 = &rotTransArray[modelPartIndex];
            v17 = weightScale;
            if ((float)((float)((float)((float)(0.0 * v16->quat[0]) + (float)(0.0 * v16->quat[1])) + (float)(frameVec.v[2] * v16->quat[2])) + (float)(frameVec.v[3] * v16->quat[3])) < 0.0)
                v17 = -weightScale;
            v16->quat[0] = (float)(v17 * frameVec.v[0]) + v16->quat[0];
            v16->quat[1] = (float)(v17 * frameVec.v[1]) + v16->quat[1];
            v16->quat[2] = (float)(v17 * frameVec.v[2]) + v16->quat[2];
            v16->quat[3] = (float)(v17 * frameVec.v[3]) + v16->quat[3];
        }
        ++animPartIndex;
        dataShort += 2;
    }
    size += parts->boneCount[4];
    while (animPartIndex < size)
    {
        modelPartIndex = animToModel[animPartIndex];
        if ((unsigned int)modelPartIndex >= 0xA0
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim_calc.cpp",
                1340,
                0,
                "modelPartIndex doesn't index DOBJ_MAX_PARTS\n\t%i not in [0, %i)",
                modelPartIndex,
                160))
        {
            __debugbreak();
        }
        if (!ignorePartBits->testBit(modelPartIndex))
        {
            v39 = (float)*dataShort * 0.000030518509;
            v40 = (float)dataShort[1] * 0.000030518509;
            v41 = (float)dataShort[2] * 0.000030518509;
            v42 = (float)dataShort[3] * 0.000030518509;
            v14 = &rotTransArray[modelPartIndex];
            v15 = weightScale;
            if ((float)((float)((float)((float)(v39 * v14->quat[0]) + (float)(v40 * v14->quat[1])) + (float)(v41 * v14->quat[2])) + (float)(v42 * v14->quat[3])) < 0.0)
                v15 = -weightScale;
            v14->quat[0] = (float)(v15 * v39) + v14->quat[0];
            v14->quat[1] = (float)(v15 * v40) + v14->quat[1];
            v14->quat[2] = (float)(v15 * v41) + v14->quat[2];
            v14->quat[3] = (float)(v15 * v42) + v14->quat[3];
        }
        ++animPartIndex;
        dataShort += 4;
    }
    animPartIndex = 0;
    size = parts->boneCount[5];
    while (animPartIndex < size)
    {
        modelPartIndex = animToModel[*dataByte++];
        if ((unsigned int)modelPartIndex >= 0xA0
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim_calc.cpp",
                1373,
                0,
                "modelPartIndex doesn't index DOBJ_MAX_PARTS\n\t%i not in [0, %i)",
                modelPartIndex,
                160))
        {
            __debugbreak();
        }
        tableSize = (unsigned __int16)*dataShort++;
        v13 = dataByte;
        dataByte += tableSize + 1;
        if (ignorePartBits->testBit(modelPartIndex))
        {
            v12 = 0;
        }
        else
        {
            XAnim_GetTimeIndex_unsigned_char_(&animTime, v13, tableSize, &v34, &v37);
            v12 = 1;
        }
        if (v12)
        {
            v35 = &randomDataByte[3 * v34];
            v38.v[0] = (float)*v35;
            v38.v[1] = (float)v35[1];
            v38.v[2] = (float)v35[2];
            v38.u[3] = 0.0f;
            v36.v[0] = (float)v35[3];
            v36.v[1] = (float)v35[4];
            v36.v[2] = (float)v35[5];
            v36.u[3] = 0.0f;
            XAnimWeightedAccumLerpedTrans(&v38, &v36, v37, weightScale, dataInt, &rotTransArray[modelPartIndex]);
        }
        ++animPartIndex;
        dataInt += 6;
        randomDataByte += 3 * tableSize + 3;
    }
    size += parts->boneCount[6];
    while (animPartIndex < size)
    {
        modelPartIndex = animToModel[*dataByte++];
        if ((unsigned int)modelPartIndex >= 0xA0
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim_calc.cpp",
                1406,
                0,
                "modelPartIndex doesn't index DOBJ_MAX_PARTS\n\t%i not in [0, %i)",
                modelPartIndex,
                160))
        {
            __debugbreak();
        }
        tableSize = (unsigned __int16)*dataShort++;
        v11 = dataByte;
        dataByte += tableSize + 1;
        if (ignorePartBits->testBit(modelPartIndex))
        {
            v10 = 0;
        }
        else
        {
            XAnim_GetTimeIndex_unsigned_char_(&animTime, v11, tableSize, &v29, &v32);
            v10 = 1;
        }
        if (v10)
        {
            v30 = (unsigned __int16 *)&randomDataShort[3 * v29];
            v33.v[0] = (float)*v30;
            v33.v[1] = (float)v30[1];
            v33.v[2] = (float)v30[2];
            v33.u[3] = 0.0f;
            v31.v[0] = (float)v30[3];
            v31.v[1] = (float)v30[4];
            v31.v[2] = (float)v30[5];
            v31.u[3] = 0.0f;
            XAnimWeightedAccumLerpedTrans(&v33, &v31, v32, weightScale, dataInt, &rotTransArray[modelPartIndex]);
        }
        ++animPartIndex;
        dataInt += 6;
        randomDataShort += 3 * tableSize + 3;
    }
    size += parts->boneCount[7];
    while (animPartIndex < size)
    {
        modelPartIndex = animToModel[*dataByte];
        if ((unsigned int)modelPartIndex >= 0xA0
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim_calc.cpp",
                1432,
                0,
                "modelPartIndex doesn't index DOBJ_MAX_PARTS\n\t%i not in [0, %i)",
                modelPartIndex,
                160))
        {
            __debugbreak();
        }
        if (!ignorePartBits->testBit(modelPartIndex))
            XAnimWeightedAccumTrans(weightScale, (float *)dataInt, &rotTransArray[modelPartIndex]);
        ++animPartIndex;
        ++dataByte;
        dataInt += 3;
    }
    size += parts->boneCount[8];
    while (animPartIndex < size)
    {
        modelPartIndex = animToModel[*dataByte];
        if (modelPartIndex >= 160
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim_calc.cpp",
                1446,
                0,
                "%s",
                "modelPartIndex < DOBJ_MAX_PARTS"))
        {
            __debugbreak();
        }
        if (!ignorePartBits->testBit(modelPartIndex))
            rotTransArray[modelPartIndex].transWeight = rotTransArray[modelPartIndex].transWeight + weightScale;
        ++animPartIndex;
        ++dataByte;
    }
}

void __cdecl XAnimCalcLeafInternal(
                const XAnimParts *parts,
                unsigned __int16 animToModelIndex,
                float time,
                float weightScale,
                DObjAnimMat *rotTransArray,
                bitarray<160> *animPartBits,
                bitarray<160> *ignorePartBits)
{
    const char *v7; // eax
    bool v8; // [esp+14h] [ebp-14h]
    const XAnimToXModel *animToModel; // [esp+20h] [ebp-8h]
    int i; // [esp+24h] [ebp-4h]

    if ( !animToModelIndex
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim_calc.cpp", 1481, 0, "%s", "animToModelIndex") )
    {
        __debugbreak();
    }
    animToModel = (const XAnimToXModel *)SL_ConvertToString(animToModelIndex, SCRIPTINSTANCE_SERVER);
    for ( i = 0; i < 5; ++i )
        animPartBits->array[i] |= animToModel->partBits.array[i] & ~ignorePartBits->array[i];
    if ( parts->bLoop )
        v8 = time < 1.0;
    else
        v8 = time <= 1.0;
    if ( !v8 )
    {
        v7 = va("time: %f, parts->bLoop: %d", time, parts->bLoop);
        if ( !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim_calc.cpp",
                        1493,
                        0,
                        "%s\n\t%s",
                        "parts->bLoop ? (time < 1.f) : (time <= 1.f)",
                        v7) )
            __debugbreak();
    }
    if ( time != 1.0 && parts->numframes )
    {
        if ( parts->numframes >= 0x100u )
            XAnimCalcParts_unsigned_short_(parts, animToModel->boneIndex, time, weightScale, rotTransArray, ignorePartBits);
        else
            XAnimCalcParts_unsigned_char_(parts, animToModel->boneIndex, time, weightScale, rotTransArray, ignorePartBits);
    }
    else
    {
        if ( parts->bLoop
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim_calc.cpp", 1504, 0, "%s", "!parts->bLoop") )
        {
            __debugbreak();
        }
        XAnimCalcNonLoopEnd(parts, animToModel->boneIndex, weightScale, rotTransArray, ignorePartBits);
    }
}

void __cdecl XAnimCalcNonLoopEnd(
                const XAnimParts *parts,
                const unsigned __int8 *animToModel,
                float weightScale,
                DObjAnimMat *rotTransArray,
                bitarray<160> *ignorePartBits)
{
    float *trans; // eax
    float *v6; // edx
    float *v7; // edx
    int *v8; // [esp+Ch] [ebp-198h]
    __int16 *v9; // [esp+10h] [ebp-194h]
    unsigned __int8 *v10; // [esp+14h] [ebp-190h]
    float v11; // [esp+2Ch] [ebp-178h]
    float v12; // [esp+34h] [ebp-170h]
    float v13; // [esp+3Ch] [ebp-168h]
    float scale; // [esp+48h] [ebp-15Ch]
    float v15; // [esp+64h] [ebp-140h]
    float v16; // [esp+68h] [ebp-13Ch]
    float v17; // [esp+6Ch] [ebp-138h]
    float v18; // [esp+9Ch] [ebp-108h]
    float v19; // [esp+A0h] [ebp-104h]
    float v20; // [esp+A4h] [ebp-100h]
    __int64 sizeVec; // [esp+CCh] [ebp-D8h]
    __int64 posVec; // [esp+F0h] [ebp-B4h]
    __int64 posVec_8; // [esp+F8h] [ebp-ACh]
    float dir4[4]; // [esp+120h] [ebp-84h] BYREF
    //float v25; // [esp+124h] [ebp-80h]
    //float v26; // [esp+128h] [ebp-7Ch]
    //float v27; // [esp+12Ch] [ebp-78h]
    float dir_2[4]; // [esp+130h] [ebp-74h] BYREF
    //float v29; // [esp+138h] [ebp-6Ch]
    //float v30; // [esp+13Ch] [ebp-68h]
    float dir[4]; // [esp+144h] [ebp-60h] BYREF
    //float v32; // [esp+148h] [ebp-5Ch]
    //float v33; // [esp+14Ch] [ebp-58h]
    //float v34; // [esp+150h] [ebp-54h]
    __int16 *v35; // [esp+154h] [ebp-50h]
    float4 frameVec; // [esp+15Ch] [ebp-48h] BYREF
    const __int16 *rotLastFrame; // [esp+170h] [ebp-34h]
    int useSmallIndices; // [esp+174h] [ebp-30h]
    __int16 *dataShort; // [esp+178h] [ebp-2Ch]
    DObjAnimMat *totalRotTrans; // [esp+17Ch] [ebp-28h]
    unsigned int animPartIndex; // [esp+180h] [ebp-24h]
    unsigned __int8 *dataByte; // [esp+184h] [ebp-20h]
    int *randomDataInt; // [esp+188h] [ebp-1Ch]
    unsigned int size; // [esp+18Ch] [ebp-18h]
    int *dataInt; // [esp+190h] [ebp-14h]
    __int16 *randomDataShort; // [esp+194h] [ebp-10h]
    unsigned __int8 *randomDataByte; // [esp+198h] [ebp-Ch]
    unsigned int tableSize; // [esp+19Ch] [ebp-8h]
    int modelPartIndex; // [esp+1A0h] [ebp-4h]

    if ( parts->bLoop
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim_calc.cpp", 420, 0, "%s", "!parts->bLoop") )
    {
        __debugbreak();
    }
    if ( parts->dataByte )
        v10 = parts->dataByte;
    else
        v10 = 0;
    dataByte = v10;
    if ( parts->dataShort )
        v9 = parts->dataShort;
    else
        v9 = 0;
    dataShort = v9;
    if ( parts->dataInt )
        v8 = parts->dataInt;
    else
        v8 = 0;
    dataInt = v8;
    randomDataByte = parts->randomDataByte;
    randomDataShort = parts->randomDataShort;
    randomDataInt = parts->randomDataInt;
    useSmallIndices = parts->numframes < 0x100u;
    animPartIndex = 0;
    size = parts->boneCount[0];
    while ( animPartIndex < size )
    {
        modelPartIndex = animToModel[animPartIndex];
        if ( (unsigned int)modelPartIndex >= 0xA0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim_calc.cpp",
                        438,
                        0,
                        "modelPartIndex doesn't index DOBJ_MAX_PARTS\n\t%i not in [0, %i)",
                        modelPartIndex,
                        160) )
        {
            __debugbreak();
        }
        if ( !ignorePartBits->testBit(modelPartIndex) )
        {
            totalRotTrans = &rotTransArray[modelPartIndex];
            totalRotTrans->quat[3] = totalRotTrans->quat[3] + weightScale;
        }
        ++animPartIndex;
    }
    size += parts->boneCount[1];
    while ( animPartIndex < size )
    {
        modelPartIndex = animToModel[animPartIndex];
        if ( (unsigned int)modelPartIndex >= 0xA0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim_calc.cpp",
                        589,
                        0,
                        "modelPartIndex doesn't index DOBJ_MAX_PARTS\n\t%i not in [0, %i)",
                        modelPartIndex,
                        160) )
        {
            __debugbreak();
        }
        tableSize = (unsigned __int16)*dataShort++;
        if ( useSmallIndices )
        {
            dataByte += tableSize + 1;
        }
        else if ( tableSize >= 0x40 )
        {
            dataShort += ((tableSize - 1) >> 8) + 2;
        }
        else
        {
            dataShort += tableSize + 1;
        }
        if ( !ignorePartBits->testBit(modelPartIndex) )
        {
            rotLastFrame = &randomDataShort[2 * tableSize];
            frameVec.u[0] = 0;
            frameVec.u[1] = 0;
            frameVec.v[2] = (float)*rotLastFrame * 0.000030518509;
            frameVec.v[3] = (float)rotLastFrame[1] * 0.000030518509;
            scale = weightScale;
            if ( (float)((float)((float)((float)(0.0 * rotTransArray[modelPartIndex].quat[0])
                                                                 + (float)(0.0 * rotTransArray[modelPartIndex].quat[1]))
                                                 + (float)(frameVec.v[2] * rotTransArray[modelPartIndex].quat[2]))
                                 + (float)(frameVec.v[3] * rotTransArray[modelPartIndex].quat[3])) < 0.0 )
                scale = -weightScale;
            Vec4Mad(rotTransArray[modelPartIndex].quat, scale, frameVec.v, rotTransArray[modelPartIndex].quat);
        }
        ++animPartIndex;
        randomDataShort += 2 * tableSize + 2;
    }
    size += parts->boneCount[2];
    while ( animPartIndex < size )
    {
        modelPartIndex = animToModel[animPartIndex];
        if ( (unsigned int)modelPartIndex >= 0xA0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim_calc.cpp",
                        625,
                        0,
                        "modelPartIndex doesn't index DOBJ_MAX_PARTS\n\t%i not in [0, %i)",
                        modelPartIndex,
                        160) )
        {
            __debugbreak();
        }
        tableSize = (unsigned __int16)*dataShort++;
        if ( useSmallIndices )
        {
            dataByte += tableSize + 1;
        }
        else if ( tableSize >= 0x40 )
        {
            dataShort += ((tableSize - 1) >> 8) + 2;
        }
        else
        {
            dataShort += tableSize + 1;
        }
        if ( !ignorePartBits->testBit(modelPartIndex) )
        {
            v35 = &randomDataShort[4 * tableSize];
            dir[0] = (float)*v35 * 0.000030518509;
            dir[1] = (float)v35[1] * 0.000030518509;
            dir[2] = (float)v35[2] * 0.000030518509;
            dir[3] = (float)v35[3] * 0.000030518509;
            v13 = weightScale;
            if ( (float)((float)((float)((float)(dir[0] * rotTransArray[modelPartIndex].quat[0])
                                                                 + (float)(dir[1] * rotTransArray[modelPartIndex].quat[1]))
                                                 + (float)(dir[2] * rotTransArray[modelPartIndex].quat[2]))
                                 + (float)(dir[3] * rotTransArray[modelPartIndex].quat[3])) < 0.0 )
                v13 = -weightScale;
            Vec4Mad(rotTransArray[modelPartIndex].quat, v13, dir, rotTransArray[modelPartIndex].quat);
        }
        ++animPartIndex;
        randomDataShort += 4 * tableSize + 4;
    }
    size += parts->boneCount[3];
    while ( animPartIndex < size )
    {
        modelPartIndex = animToModel[animPartIndex];
        if ( (unsigned int)modelPartIndex >= 0xA0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim_calc.cpp",
                        657,
                        0,
                        "modelPartIndex doesn't index DOBJ_MAX_PARTS\n\t%i not in [0, %i)",
                        modelPartIndex,
                        160) )
        {
            __debugbreak();
        }
        if ( !ignorePartBits->testBit(modelPartIndex) )
        {
            dir_2[0] = 0.0f;
            dir_2[1] = 0.0f;
            dir_2[2] = (float)*dataShort * 0.000030518509; // v29
            dir_2[3] = (float)dataShort[1] * 0.000030518509; // v30
            v12 = weightScale;
            if ( (float)((float)((float)((float)(0.0 * rotTransArray[modelPartIndex].quat[0])
                                                                 + (float)(0.0 * rotTransArray[modelPartIndex].quat[1]))
                                                 + (float)(dir[2] * rotTransArray[modelPartIndex].quat[2]))
                                 + (float)(dir[3] * rotTransArray[modelPartIndex].quat[3])) < 0.0 )
                v12 = -weightScale;
            Vec4Mad(rotTransArray[modelPartIndex].quat, v12, dir_2, rotTransArray[modelPartIndex].quat);
        }
        ++animPartIndex;
        dataShort += 2;
    }
    size += parts->boneCount[4];
    while ( animPartIndex < size )
    {
        modelPartIndex = animToModel[animPartIndex];
        if ( (unsigned int)modelPartIndex >= 0xA0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim_calc.cpp",
                        683,
                        0,
                        "modelPartIndex doesn't index DOBJ_MAX_PARTS\n\t%i not in [0, %i)",
                        modelPartIndex,
                        160) )
        {
            __debugbreak();
        }
        if ( !ignorePartBits->testBit(modelPartIndex) )
        {
            dir4[0] = (float)*dataShort * 0.000030518509;
            dir4[1] = (float)dataShort[1] * 0.000030518509;
            dir4[2] = (float)dataShort[2] * 0.000030518509;
            dir4[3] = (float)dataShort[3] * 0.000030518509;
            v11 = weightScale;
            if ( (float)((float)((float)((float)(dir4[0] * rotTransArray[modelPartIndex].quat[0])
                                                                 + (float)(dir4[1] * rotTransArray[modelPartIndex].quat[1]))
                                                 + (float)(dir4[2] * rotTransArray[modelPartIndex].quat[2]))
                                 + (float)(dir4[3] * rotTransArray[modelPartIndex].quat[3])) < 0.0 )
                v11 = -weightScale;
            Vec4Mad(rotTransArray[modelPartIndex].quat, v11, dir4, rotTransArray[modelPartIndex].quat);
        }
        ++animPartIndex;
        dataShort += 4;
    }
    animPartIndex = 0;
    size = parts->boneCount[5];
    while ( animPartIndex < size )
    {
        modelPartIndex = animToModel[*dataByte++];
        if ( (unsigned int)modelPartIndex >= 0xA0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim_calc.cpp",
                        720,
                        0,
                        "modelPartIndex doesn't index DOBJ_MAX_PARTS\n\t%i not in [0, %i)",
                        modelPartIndex,
                        160) )
        {
            __debugbreak();
        }
        tableSize = (unsigned __int16)*dataShort++;
        if ( useSmallIndices )
        {
            dataByte += tableSize + 1;
        }
        else if ( tableSize >= 0x40 )
        {
            dataShort += ((tableSize - 1) >> 8) + 2;
        }
        else
        {
            dataShort += tableSize + 1;
        }
        if ( !ignorePartBits->testBit(modelPartIndex) )
        {
            totalRotTrans = &rotTransArray[modelPartIndex];
            posVec = *(_QWORD *)totalRotTrans->trans;
            posVec_8 = *(_QWORD *)&totalRotTrans->trans[2];
            sizeVec = *(_QWORD *)(dataInt + 3);
            *(float *)&posVec = (float)(weightScale
                                                                * (float)((float)(*(float *)&sizeVec * (float)randomDataByte[3 * tableSize])
                                                                                + *(float *)dataInt))
                                                + *(float *)&posVec;
            *((float *)&posVec + 1) = (float)(weightScale
                                                                            * (float)((float)(*((float *)&sizeVec + 1)
                                                                                                            * (float)randomDataByte[3 * tableSize + 1])
                                                                                            + *((float *)dataInt + 1)))
                                                            + *((float *)&posVec + 1);
            *(float *)&posVec_8 = (float)(weightScale
                                                                    * (float)((float)(*((float *)dataInt + 5) * (float)randomDataByte[3 * tableSize + 2])
                                                                                    + *((float *)dataInt + 2)))
                                                    + *(float *)&posVec_8;
            trans = totalRotTrans->trans;
            *(_QWORD *)totalRotTrans->trans = posVec;
            *((unsigned int *)trans + 2) = posVec_8;
            trans[3] = (float)(weightScale * (float)((float)(0.0 * 0.0) + 0.0)) + *((float *)&posVec_8 + 1);
            totalRotTrans->transWeight = totalRotTrans->transWeight + weightScale;
        }
        ++animPartIndex;
        dataInt += 6;
        randomDataByte += 3 * tableSize + 3;
    }
    size += parts->boneCount[6];
    while ( animPartIndex < size )
    {
        modelPartIndex = animToModel[*dataByte++];
        if ( (unsigned int)modelPartIndex >= 0xA0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim_calc.cpp",
                        761,
                        0,
                        "modelPartIndex doesn't index DOBJ_MAX_PARTS\n\t%i not in [0, %i)",
                        modelPartIndex,
                        160) )
        {
            __debugbreak();
        }
        tableSize = (unsigned __int16)*dataShort++;
        if ( useSmallIndices )
        {
            dataByte += tableSize + 1;
        }
        else if ( tableSize >= 0x40 )
        {
            dataShort += ((tableSize - 1) >> 8) + 2;
        }
        else
        {
            dataShort += tableSize + 1;
        }
        if ( !ignorePartBits->testBit(modelPartIndex) )
        {
            totalRotTrans = &rotTransArray[modelPartIndex];
            v18 = (float)(weightScale
                                    * (float)((float)(*((float *)dataInt + 4) * (float)(unsigned __int16)randomDataShort[3 * tableSize + 1])
                                                    + *((float *)dataInt + 1)))
                    + totalRotTrans->trans[1];
            v19 = (float)(weightScale
                                    * (float)((float)(*((float *)dataInt + 5) * (float)(unsigned __int16)randomDataShort[3 * tableSize + 2])
                                                    + *((float *)dataInt + 2)))
                    + totalRotTrans->trans[2];
            v20 = (float)(weightScale * (float)((float)(0.0 * 0.0) + 0.0)) + totalRotTrans->transWeight;
            v6 = totalRotTrans->trans;
            totalRotTrans->trans[0] = (float)(weightScale
                                                                            * (float)((float)(*((float *)dataInt + 3)
                                                                                                            * (float)(unsigned __int16)randomDataShort[3 * tableSize])
                                                                                            + *(float *)dataInt))
                                                            + totalRotTrans->trans[0];
            v6[1] = v18;
            v6[2] = v19;
            v6[3] = v20;
            totalRotTrans->transWeight = totalRotTrans->transWeight + weightScale;
        }
        ++animPartIndex;
        dataInt += 6;
        randomDataShort += 3 * tableSize + 3;
    }
    size += parts->boneCount[7];
    while ( animPartIndex < size )
    {
        modelPartIndex = animToModel[*dataByte];
        if ( (unsigned int)modelPartIndex >= 0xA0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim_calc.cpp",
                        794,
                        0,
                        "modelPartIndex doesn't index DOBJ_MAX_PARTS\n\t%i not in [0, %i)",
                        modelPartIndex,
                        160) )
        {
            __debugbreak();
        }
        if ( !ignorePartBits->testBit(modelPartIndex) )
        {
            totalRotTrans = &rotTransArray[modelPartIndex];
            v15 = (float)(weightScale * *((float *)dataInt + 1)) + totalRotTrans->trans[1];
            v16 = (float)(weightScale * *((float *)dataInt + 2)) + totalRotTrans->trans[2];
            v17 = (float)(weightScale * 0.0) + totalRotTrans->transWeight;
            v7 = totalRotTrans->trans;
            totalRotTrans->trans[0] = (float)(weightScale * *(float *)dataInt) + totalRotTrans->trans[0];
            v7[1] = v15;
            v7[2] = v16;
            v7[3] = v17;
            totalRotTrans->transWeight = totalRotTrans->transWeight + weightScale;
        }
        ++animPartIndex;
        ++dataByte;
        dataInt += 3;
    }
    size += parts->boneCount[8];
    while ( animPartIndex < size )
    {
        modelPartIndex = animToModel[*dataByte];
        if ( (unsigned int)modelPartIndex >= 0xA0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim_calc.cpp",
                        814,
                        0,
                        "modelPartIndex doesn't index DOBJ_MAX_PARTS\n\t%i not in [0, %i)",
                        modelPartIndex,
                        160) )
        {
            __debugbreak();
        }
        if ( !ignorePartBits->testBit(modelPartIndex) )
        {
            totalRotTrans = &rotTransArray[modelPartIndex];
            totalRotTrans->transWeight = totalRotTrans->transWeight + weightScale;
        }
        ++animPartIndex;
        ++dataByte;
    }
}

void __cdecl Vec4Mad(const float *start, float scale, const float *dir, float *result)
{
    *result = (float)(scale * *dir) + *start;
    result[1] = (float)(scale * dir[1]) + start[1];
    result[2] = (float)(scale * dir[2]) + start[2];
    result[3] = (float)(scale * dir[3]) + start[3];
}

DObjAnimMat *__cdecl XAnimGetCalcBuffer(XAnimCalcAnimInfo *info, const DObj *obj, int *rotTransArrayIndex)
{
    DObjAnimMat *calcBuffer; // [esp+0h] [ebp-8h]
    int index; // [esp+4h] [ebp-4h]

    calcBuffer = &info->rotTransArray[*rotTransArrayIndex];
    index = *rotTransArrayIndex + obj->numBones;
    if ( index <= 960 )
    {
        *rotTransArrayIndex = index;
        return calcBuffer;
    }
    else
    {
        Com_PrintWarning(19, "max calculation buffer exceeded\n");
        DObjDisplayAnim(obj, "");
        return 0;
    }
}

void __cdecl XAnimScaleRotTransArray(int numBones, const XAnimCalcAnimInfo *info, DObjAnimMat *rotTransArray)
{
    float r; // [esp+8h] [ebp-8h]
    signed int i; // [esp+Ch] [ebp-4h]

    for ( i = 0; i < numBones; ++i )
    {
        if ( !info->ignorePartBits.testBit(i) && rotTransArray->transWeight != 0.0 )
        {
            r = 1.0 / rotTransArray->transWeight;
            rotTransArray->quat[0] = r * rotTransArray->quat[0];
            rotTransArray->quat[1] = r * rotTransArray->quat[1];
            rotTransArray->quat[2] = r * rotTransArray->quat[2];
            rotTransArray->quat[3] = r * rotTransArray->quat[3];
            rotTransArray->trans[0] = r * rotTransArray->trans[0];
            rotTransArray->trans[1] = r * rotTransArray->trans[1];
            rotTransArray->trans[2] = r * rotTransArray->trans[2];
        }
        ++rotTransArray;
    }
}

void __cdecl XAnimNormalizeRotScaleTransArray(
                int numBones,
                const XAnimCalcAnimInfo *info,
                float weightScale,
                DObjAnimMat *rotTransArray)
{
    float v4; // [esp+8h] [ebp-28h]
    float v5; // [esp+10h] [ebp-20h]
    float r; // [esp+28h] [ebp-8h]
    signed int i; // [esp+2Ch] [ebp-4h]

    for ( i = 0; i < numBones; ++i )
    {
        if ( !info->ignorePartBits.testBit(i) )
        {
            r = Vec4LengthSq(rotTransArray->quat);
            if ( r != 0.0 )
            {
                v5 = I_rsqrt(r) * weightScale;
                rotTransArray->quat[0] = v5 * rotTransArray->quat[0];
                rotTransArray->quat[1] = v5 * rotTransArray->quat[1];
                rotTransArray->quat[2] = v5 * rotTransArray->quat[2];
                rotTransArray->quat[3] = v5 * rotTransArray->quat[3];
            }
            if ( rotTransArray->transWeight != 0.0 )
            {
                v4 = weightScale / rotTransArray->transWeight;
                rotTransArray->trans[0] = v4 * rotTransArray->trans[0];
                rotTransArray->trans[1] = v4 * rotTransArray->trans[1];
                rotTransArray->trans[2] = v4 * rotTransArray->trans[2];
                rotTransArray->transWeight = weightScale;
            }
        }
        ++rotTransArray;
    }
}

void __cdecl XAnimMadRotTransArray(
                int numBones,
                const XAnimCalcAnimInfo *info,
                float weightScale,
                const DObjAnimMat *rotTrans,
                DObjAnimMat *totalRotTrans)
{
    float v5; // [esp+Ch] [ebp-3Ch]
    float v6; // [esp+14h] [ebp-34h]
    float r; // [esp+2Ch] [ebp-1Ch]
    signed int i; // [esp+30h] [ebp-18h]
    float addQuat; // [esp+38h] [ebp-10h]
    float addQuat_4; // [esp+3Ch] [ebp-Ch]
    float addQuat_8; // [esp+40h] [ebp-8h]
    float addQuat_12; // [esp+44h] [ebp-4h]

    for ( i = 0; i < numBones; ++i )
    {
        if ( !info->ignorePartBits.testBit(i) )
        {
            if ( (float)((float)((float)((float)(totalRotTrans->quat[0] * rotTrans->quat[0])
                                                                 + (float)(totalRotTrans->quat[1] * rotTrans->quat[1]))
                                                 + (float)(totalRotTrans->quat[2] * rotTrans->quat[2]))
                                 + (float)(totalRotTrans->quat[3] * rotTrans->quat[3])) >= 0.0 )
            {
                addQuat = rotTrans->quat[0];
                addQuat_4 = rotTrans->quat[1];
                addQuat_8 = rotTrans->quat[2];
                addQuat_12 = rotTrans->quat[3];
            }
            else
            {
                addQuat = -rotTrans->quat[0];
                addQuat_4 = -rotTrans->quat[1];
                addQuat_8 = -rotTrans->quat[2];
                addQuat_12 = -rotTrans->quat[3];
            }
            r = Vec4LengthSq(rotTrans->quat);
            if ( r != 0.0 )
            {
                v6 = I_rsqrt(r) * weightScale;
                totalRotTrans->quat[0] = (float)(v6 * addQuat) + totalRotTrans->quat[0];
                totalRotTrans->quat[1] = (float)(v6 * addQuat_4) + totalRotTrans->quat[1];
                totalRotTrans->quat[2] = (float)(v6 * addQuat_8) + totalRotTrans->quat[2];
                totalRotTrans->quat[3] = (float)(v6 * addQuat_12) + totalRotTrans->quat[3];
            }
            if ( rotTrans->transWeight != 0.0 )
            {
                v5 = weightScale / rotTrans->transWeight;
                totalRotTrans->trans[0] = (float)(v5 * rotTrans->trans[0]) + totalRotTrans->trans[0];
                totalRotTrans->trans[1] = (float)(v5 * rotTrans->trans[1]) + totalRotTrans->trans[1];
                totalRotTrans->trans[2] = (float)(v5 * rotTrans->trans[2]) + totalRotTrans->trans[2];
                totalRotTrans->transWeight = totalRotTrans->transWeight + weightScale;
            }
        }
        ++rotTrans;
        ++totalRotTrans;
    }
}

void __cdecl XAnimApplyAdditives(
                DObjAnimMat *rotTransArray,
                DObjAnimMat *additiveArray,
                float weight,
                int boneCount,
                XAnimCalcAnimInfo *info)
{
    DObjAnimMat *v5; // ecx
    float *trans; // [esp+0h] [ebp-40h]
    DObjAnimMat *v7; // [esp+18h] [ebp-28h]
    float r; // [esp+24h] [ebp-1Ch]
    float ra; // [esp+24h] [ebp-1Ch]
    float rb; // [esp+24h] [ebp-1Ch]
    signed int i; // [esp+28h] [ebp-18h]
    float rot[4]; // [esp+2Ch] [ebp-14h] BYREF
    const bitarray<160> *ignorePartBits; // [esp+3Ch] [ebp-4h]

    ignorePartBits = &info->ignorePartBits;
    for ( i = 0; i < boneCount; ++i )
    {
        if ( !ignorePartBits->testBit(i) )
        {
            r = Vec4LengthSq(additiveArray[i].quat);
            if ( r != 0.0 )
            {
                ra = sqrtf(r);
                v7 = &additiveArray[i];
                v7->quat[0] = (float)(1.0 / ra) * v7->quat[0];
                v7->quat[1] = (float)(1.0 / ra) * v7->quat[1];
                v7->quat[2] = (float)(1.0 / ra) * v7->quat[2];
                v7->quat[3] = (float)(1.0 / ra) * v7->quat[3];
                rb = ra * weight;
                v7->quat[0] = rb * v7->quat[0];
                v7->quat[1] = rb * v7->quat[1];
                v7->quat[2] = rb * v7->quat[2];
                v7->quat[3] = (float)((float)(1.0 - rb) * 1.0) + (float)(v7->quat[3] * rb);
                QuatMultiply(v7->quat, rotTransArray[i].quat, rot);
                rotTransArray[i].quat[0] = rot[0];
                v5 = &rotTransArray[i];
                v5->quat[1] = rot[1];
                v5->quat[2] = rot[2];
                v5->quat[3] = rot[3];
            }
            trans = rotTransArray[i].trans;
            *trans = (float)(weight * additiveArray[i].trans[0]) + *trans;
            trans[1] = (float)(weight * additiveArray[i].trans[1]) + rotTransArray[i].trans[1];
            trans[2] = (float)(weight * additiveArray[i].trans[2]) + rotTransArray[i].trans[2];
        }
    }
}

XAnimCalcAnimInfo::XAnimCalcAnimInfo()
{
    int j; // [esp+8h] [ebp-Ch]
    int i; // [esp+10h] [ebp-4h]

    for ( i = 0; i < 5; ++i )
        this->animPartBits.array[i] = 0;
    for ( j = 0; j < 5; ++j )
        this->ignorePartBits.array[j] = 0;
}

void __cdecl XAnim_CalcRotDeltaDuring_unsigned_char_(
    const XAnimDeltaPart *animDelta,
    float time,
    int frameCount,
    float *rotDelta)
{
    const XAnimDeltaPartQuatData *p_u; // [esp+10h] [ebp-1Ch]
    int keyFrameIndex; // [esp+14h] [ebp-18h] BYREF
    XAnimTime animTime; // [esp+18h] [ebp-14h] BYREF
    float keyFrameLerpFrac; // [esp+24h] [ebp-8h] BYREF
    const XAnimDeltaPartQuat *rotFrameDeltas; // [esp+28h] [ebp-4h]

    if ((!frameCount || time == 1.0)
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim_calc.cpp",
            2352,
            0,
            "%s",
            "frameCount && (time != 1.0f)"))
    {
        __debugbreak();
    }
    if (animDelta->quat)
    {
        rotFrameDeltas = animDelta->quat;
        if (rotFrameDeltas->size)
        {
            XAnim_SetTime(time, frameCount, &animTime);
            XAnim_GetTimeIndex_unsigned_char_(
                &animTime,
                rotFrameDeltas->u.frames.indices._1,
                rotFrameDeltas->size,
                &keyFrameIndex,
                &keyFrameLerpFrac);
            Short2LerpAsVec2(
                rotFrameDeltas->u.frames.frames[keyFrameIndex],
                rotFrameDeltas->u.frames.frames[keyFrameIndex + 1],
                keyFrameLerpFrac,
                rotDelta);
        }
        else
        {
            p_u = &rotFrameDeltas->u;
            *rotDelta = (float)rotFrameDeltas->u.frame0[0];
            rotDelta[1] = (float)p_u->frame0[1];
        }
    }
    else
    {
        *rotDelta = 0.0;
        rotDelta[1] = 32767.0f;
    }
}

void __cdecl XAnim_CalcRotDeltaDuring_unsigned_short_(
    const XAnimDeltaPart *animDelta,
    float time,
    int frameCount,
    float *rotDelta)
{
    const XAnimDeltaPartQuatData *p_u; // [esp+10h] [ebp-1Ch]
    int keyFrameIndex; // [esp+14h] [ebp-18h] BYREF
    XAnimTime animTime; // [esp+18h] [ebp-14h] BYREF
    float keyFrameLerpFrac; // [esp+24h] [ebp-8h] BYREF
    const XAnimDeltaPartQuat *rotFrameDeltas; // [esp+28h] [ebp-4h]

    if ((!frameCount || time == 1.0)
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim_calc.cpp",
            2352,
            0,
            "%s",
            "frameCount && (time != 1.0f)"))
    {
        __debugbreak();
    }
    if (animDelta->quat)
    {
        rotFrameDeltas = animDelta->quat;
        if (rotFrameDeltas->size)
        {
            XAnim_SetTime(time, frameCount, &animTime);
            XAnim_GetTimeIndex_unsigned_short_(
                &animTime,
                rotFrameDeltas->u.frames.indices._2,
                rotFrameDeltas->size,
                &keyFrameIndex,
                &keyFrameLerpFrac);
            Short2LerpAsVec2(
                rotFrameDeltas->u.frames.frames[keyFrameIndex],
                rotFrameDeltas->u.frames.frames[keyFrameIndex + 1],
                keyFrameLerpFrac,
                rotDelta);
        }
        else
        {
            p_u = &rotFrameDeltas->u;
            *rotDelta = (float)rotFrameDeltas->u.frame0[0];
            rotDelta[1] = (float)p_u->frame0[1];
        }
    }
    else
    {
        *rotDelta = 0.0;
        rotDelta[1] = 32767.0f;
    }
}

void __cdecl XAnim_CalcPosDeltaDuring_unsigned_short_(
    const XAnimDeltaPart *animDelta,
    float time,
    int frameCount,
    float4 *posDelta)
{
    unsigned __int8 *v4; // [esp+2Ch] [ebp-84h]
    unsigned __int8 *v5; // [esp+30h] [ebp-80h]
    unsigned __int8 *v6; // [esp+34h] [ebp-7Ch]
    unsigned __int8 *v7; // [esp+38h] [ebp-78h]
    const XAnimPartTransData *p_u; // [esp+3Ch] [ebp-74h]
    int keyFrameIndex; // [esp+40h] [ebp-70h] BYREF
    float4 sizeVec; // [esp+44h] [ebp-6Ch]
    float4 toVec; // [esp+54h] [ebp-5Ch]
    XAnimTime animTime; // [esp+64h] [ebp-4Ch] BYREF
    int nextKeyFrameIndex; // [esp+70h] [ebp-40h]
    float keyFrameLerpFrac; // [esp+74h] [ebp-3Ch] BYREF
    float4 fromVec; // [esp+78h] [ebp-38h]
    float4 lerp; // [esp+88h] [ebp-28h]
    float4 minsVec; // [esp+98h] [ebp-18h]
    const XAnimPartTrans *posFrameDeltas; // [esp+ACh] [ebp-4h]

    if ((!frameCount || time == 1.0)
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim_calc.cpp",
            2400,
            0,
            "%s",
            "frameCount && time != 1.0f"))
    {
        __debugbreak();
    }
    if (animDelta->trans)
    {
        posFrameDeltas = animDelta->trans;
        if (posFrameDeltas->size)
        {
            XAnim_SetTime(time, frameCount, &animTime);
            XAnim_GetTimeIndex_unsigned_short_(
                &animTime,
                posFrameDeltas->u.frames.indices._2,
                posFrameDeltas->size,
                &keyFrameIndex,
                &keyFrameLerpFrac);
            nextKeyFrameIndex = keyFrameIndex + 1;
            if (posFrameDeltas->smallTrans)
            {
                v7 = posFrameDeltas->u.frames.frames._1[keyFrameIndex];
                fromVec.v[0] = (float)*v7;
                fromVec.v[1] = (float)v7[1];
                fromVec.v[2] = (float)v7[2];
                fromVec.v[3] = 0.0;
                v6 = posFrameDeltas->u.frames.frames._1[nextKeyFrameIndex];
                toVec.v[0] = (float)*v6;
                toVec.v[1] = (float)v6[1];
                toVec.v[2] = (float)v6[2];
            }
            else
            {
                v5 = posFrameDeltas->u.frames.frames._1[2 * keyFrameIndex];
                fromVec.v[0] = (float)*(unsigned __int16 *)v5;
                fromVec.v[1] = (float)*((unsigned __int16 *)v5 + 1);
                fromVec.v[2] = (float)*((unsigned __int16 *)v5 + 2);
                fromVec.v[3] = 0.0;
                v4 = posFrameDeltas->u.frames.frames._1[2 * nextKeyFrameIndex];
                toVec.v[0] = (float)*(unsigned __int16 *)v4;
                toVec.v[1] = (float)*((unsigned __int16 *)v4 + 1);
                toVec.v[2] = (float)*((unsigned __int16 *)v4 + 2);
            }
            toVec.v[3] = 0.0;
            lerp.v[0] = (float)(keyFrameLerpFrac * (float)(toVec.v[0] - fromVec.v[0])) + fromVec.v[0];
            lerp.v[1] = (float)(keyFrameLerpFrac * (float)(toVec.v[1] - fromVec.v[1])) + fromVec.v[1];
            lerp.v[2] = (float)(keyFrameLerpFrac * (float)(toVec.v[2] - fromVec.v[2])) + fromVec.v[2];
            lerp.v[3] = (float)(keyFrameLerpFrac * (float)(0.0 - fromVec.v[3])) + fromVec.v[3];
            *(_QWORD *)minsVec.v = *(_QWORD *)posFrameDeltas->u.frames.mins;
            minsVec.u[2] = LODWORD(posFrameDeltas->u.frames.mins[2]);
            minsVec.v[3] = 0.0;
            *(_QWORD *)sizeVec.v = *(_QWORD *)&posFrameDeltas->u.frame0[3];
            sizeVec.u[2] = LODWORD(posFrameDeltas->u.frames.size[2]);
            sizeVec.u[3] = 0.0;
            posDelta->v[0] = (float)(sizeVec.v[0] * lerp.v[0]) + minsVec.v[0];
            posDelta->v[1] = (float)(sizeVec.v[1] * lerp.v[1]) + minsVec.v[1];
            posDelta->v[2] = (float)(sizeVec.v[2] * lerp.v[2]) + minsVec.v[2];
            posDelta->v[3] = (float)(sizeVec.v[3] * lerp.v[3]) + minsVec.v[3];
        }
        else
        {
            p_u = &posFrameDeltas->u;
            posDelta->v[0] = posFrameDeltas->u.frames.mins[0];
            posDelta->v[1] = p_u->frames.mins[1];
            posDelta->v[2] = p_u->frames.mins[2];
            posDelta->u[3] = 0.0;
        }
    }
    else
    {
        posDelta->u[0] = 0.0;
        posDelta->u[1] = 0.0;
        posDelta->u[2] = 0.0;
        posDelta->u[3] = 0.0;
    }
}

void __cdecl XAnim_CalcPosDeltaDuring_unsigned_char_(
    const XAnimDeltaPart *animDelta,
    float time,
    int frameCount,
    float4 *posDelta)
{
    unsigned __int8 *v4; // [esp+2Ch] [ebp-84h]
    unsigned __int8 *v5; // [esp+30h] [ebp-80h]
    unsigned __int8 *v6; // [esp+34h] [ebp-7Ch]
    unsigned __int8 *v7; // [esp+38h] [ebp-78h]
    const XAnimPartTransData *p_u; // [esp+3Ch] [ebp-74h]
    int keyFrameIndex; // [esp+40h] [ebp-70h] BYREF
    float4 sizeVec; // [esp+44h] [ebp-6Ch]
    float4 toVec; // [esp+54h] [ebp-5Ch]
    XAnimTime animTime; // [esp+64h] [ebp-4Ch] BYREF
    int nextKeyFrameIndex; // [esp+70h] [ebp-40h]
    float keyFrameLerpFrac; // [esp+74h] [ebp-3Ch] BYREF
    float4 fromVec; // [esp+78h] [ebp-38h]
    float4 lerp; // [esp+88h] [ebp-28h]
    float4 minsVec; // [esp+98h] [ebp-18h]
    const XAnimPartTrans *posFrameDeltas; // [esp+ACh] [ebp-4h]

    if ((!frameCount || time == 1.0)
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim_calc.cpp",
            2400,
            0,
            "%s",
            "frameCount && time != 1.0f"))
    {
        __debugbreak();
    }
    if (animDelta->trans)
    {
        posFrameDeltas = animDelta->trans;
        if (posFrameDeltas->size)
        {
            XAnim_SetTime(time, frameCount, &animTime);
            XAnim_GetTimeIndex_unsigned_char_(
                &animTime,
                posFrameDeltas->u.frames.indices._1,
                posFrameDeltas->size,
                &keyFrameIndex,
                &keyFrameLerpFrac);
            nextKeyFrameIndex = keyFrameIndex + 1;
            if (posFrameDeltas->smallTrans)
            {
                v7 = posFrameDeltas->u.frames.frames._1[keyFrameIndex];
                fromVec.v[0] = (float)*v7;
                fromVec.v[1] = (float)v7[1];
                fromVec.v[2] = (float)v7[2];
                fromVec.u[3] = 0.0;
                v6 = posFrameDeltas->u.frames.frames._1[nextKeyFrameIndex];
                toVec.v[0] = (float)*v6;
                toVec.v[1] = (float)v6[1];
                toVec.v[2] = (float)v6[2];
            }
            else
            {
                v5 = posFrameDeltas->u.frames.frames._1[2 * keyFrameIndex];
                fromVec.v[0] = (float)*(unsigned __int16 *)v5;
                fromVec.v[1] = (float)*((unsigned __int16 *)v5 + 1);
                fromVec.v[2] = (float)*((unsigned __int16 *)v5 + 2);
                fromVec.u[3] = 0.0;
                v4 = posFrameDeltas->u.frames.frames._1[2 * nextKeyFrameIndex];
                toVec.v[0] = (float)*(unsigned __int16 *)v4;
                toVec.v[1] = (float)*((unsigned __int16 *)v4 + 1);
                toVec.v[2] = (float)*((unsigned __int16 *)v4 + 2);
            }
            toVec.u[3] = 0.0;
            lerp.v[0] = (float)(keyFrameLerpFrac * (float)(toVec.v[0] - fromVec.v[0])) + fromVec.v[0];
            lerp.v[1] = (float)(keyFrameLerpFrac * (float)(toVec.v[1] - fromVec.v[1])) + fromVec.v[1];
            lerp.v[2] = (float)(keyFrameLerpFrac * (float)(toVec.v[2] - fromVec.v[2])) + fromVec.v[2];
            lerp.v[3] = (float)(keyFrameLerpFrac * (float)(0.0 - fromVec.v[3])) + fromVec.v[3];
            *(_QWORD *)minsVec.v = *(_QWORD *)posFrameDeltas->u.frames.mins;
            minsVec.u[2] = LODWORD(posFrameDeltas->u.frames.mins[2]);
            minsVec.u[3] = 0.0;
            *(_QWORD *)sizeVec.v = *(_QWORD *)&posFrameDeltas->u.frame0[3];
            sizeVec.u[2] = LODWORD(posFrameDeltas->u.frames.size[2]);
            sizeVec.u[3] = 0.0;
            posDelta->v[0] = (float)(sizeVec.v[0] * lerp.v[0]) + minsVec.v[0];
            posDelta->v[1] = (float)(sizeVec.v[1] * lerp.v[1]) + minsVec.v[1];
            posDelta->v[2] = (float)(sizeVec.v[2] * lerp.v[2]) + minsVec.v[2];
            posDelta->v[3] = (float)(sizeVec.v[3] * lerp.v[3]) + minsVec.v[3];
        }
        else
        {
            p_u = &posFrameDeltas->u;
            posDelta->v[0] = posFrameDeltas->u.frames.mins[0];
            posDelta->v[1] = p_u->frames.mins[1];
            posDelta->v[2] = p_u->frames.mins[2];
            posDelta->u[3] = 0.0;
        }
    }
    else
    {
        posDelta->u[0] = 0.0;
        posDelta->u[1] = 0.0;
        posDelta->u[2] = 0.0;
        posDelta->u[3] = 0.0;
    }
}

void __cdecl XAnim_CalcDeltaForTime(const XAnimParts *anim, float time, float *rotDelta, float4 *posDelta)
{
    int frameCount; // [esp+20h] [ebp-8h]
    XAnimDeltaPart *animDelta; // [esp+24h] [ebp-4h]

    if ( (time < 0.0 || time > 1.0)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xanim_calc.cpp",
                    2555,
                    0,
                    "time not in [0.0f, 1.0f]\n\t%g not in [%g, %g]",
                    time,
                    0.0,
                    1.0) )
    {
        __debugbreak();
    }
    animDelta = anim->deltaPart;
    frameCount = anim->numframes;
    if ( time == 1.0 || !anim->numframes )
    {
        XAnim_CalcRotDeltaEntire(animDelta, rotDelta);
        XAnim_CalcPosDeltaEntire(animDelta, posDelta);
    }
    else if ( anim->numframes >= 0x100u )
    {
        XAnim_CalcRotDeltaDuring_unsigned_short_(animDelta, time, frameCount, rotDelta);
        XAnim_CalcPosDeltaDuring_unsigned_short_(animDelta, time, frameCount, posDelta);
    }
    else
    {
        XAnim_CalcRotDeltaDuring_unsigned_char_(animDelta, time, frameCount, rotDelta);
        XAnim_CalcPosDeltaDuring_unsigned_char_(animDelta, time, frameCount, posDelta);
    }
}

void __cdecl XAnim_CalcRotDeltaEntire(const XAnimDeltaPart *animDelta, float *rotDelta)
{
    XAnimDeltaPartQuat *rotFrameDeltas; // [esp+0h] [ebp-8h]
    const __int16 *rotDeltaLastFrame; // [esp+4h] [ebp-4h]

    if ( animDelta->quat )
    {
        rotFrameDeltas = animDelta->quat;
        if ( rotFrameDeltas->size )
            rotDeltaLastFrame = rotFrameDeltas->u.frames.frames[rotFrameDeltas->size];
        else
            rotDeltaLastFrame = (const __int16 *)&rotFrameDeltas->u;
        *rotDelta = (float)*rotDeltaLastFrame;
        rotDelta[1] = (float)rotDeltaLastFrame[1];
    }
    else
    {
        *rotDelta = 0.0f;
        rotDelta[1] = 32767.0f;
    }
}

void __cdecl XAnim_CalcPosDeltaEntire(const XAnimDeltaPart *animDelta, float4 *posDelta)
{
    int v2; // edx
    unsigned __int16 *v3; // [esp+8h] [ebp-44h]
    unsigned __int8 *v4; // [esp+Ch] [ebp-40h]
    __int64 sizeVec; // [esp+14h] [ebp-38h]
    float sizeVec_8; // [esp+1Ch] [ebp-30h]
    float lerp[4]; // [esp+24h] [ebp-28h]
    float minsVec[2]; // [esp+38h] [ebp-14h]
    XAnimPartTrans *posFrameDeltas; // [esp+48h] [ebp-4h]

    if ( animDelta->trans )
    {
        posFrameDeltas = animDelta->trans;
        if ( animDelta->trans->size )
        {
            if ( posFrameDeltas->smallTrans )
            {
                v4 = posFrameDeltas->u.frames.frames._1[posFrameDeltas->size];
                lerp[0] = (float)*v4;
                lerp[1] = (float)v4[1];
                v2 = v4[2];
            }
            else
            {
                v3 = (unsigned __int16 *)posFrameDeltas->u.frames.frames._1[2 * posFrameDeltas->size];
                lerp[0] = (float)*v3;
                lerp[1] = (float)v3[1];
                v2 = v3[2];
            }
            minsVec[0] = posFrameDeltas->u.frames.mins[1];
            minsVec[1] = posFrameDeltas->u.frames.mins[2];
            sizeVec = *(_QWORD *)&posFrameDeltas->u.frame0[3];
            sizeVec_8 = posFrameDeltas->u.frames.size[2];
            posDelta->v[0] = (float)(*(float *)&sizeVec * lerp[0]) + posFrameDeltas->u.frames.mins[0];
            posDelta->v[1] = (float)(*((float *)&sizeVec + 1) * lerp[1]) + minsVec[0];
            posDelta->v[2] = (float)(sizeVec_8 * (float)v2) + minsVec[1];
            posDelta->v[3] = (float)(0.0 * 0.0) + 0.0;
        }
        else
        {
            posDelta->v[0] = posFrameDeltas->u.frames.mins[0];
            posDelta->v[1] = posFrameDeltas->u.frames.mins[1];
            posDelta->v[2] = posFrameDeltas->u.frames.mins[2];
            posDelta->u[3] = 0;
        }
    }
    else
    {
        posDelta->u[0] = 0;
        posDelta->u[1] = 0;
        posDelta->u[2] = 0;
        posDelta->u[3] = 0;
    }
}

//bool __thiscall bitarray<160>::testBit(bitarray<160> *this, unsigned int pos)
//{
//    if ( pos >= 0xA0
//        && !Assert_MyHandler(
//                    "c:\\projects_pc\\cod\\codsrc\\src\\universal\\../qcommon/bitarray.h",
//                    109,
//                    0,
//                    "%s",
//                    "pos < BIT_COUNT") )
//    {
//        __debugbreak();
//    }
//    return (this->array[pos >> 5] & (0x80000000 >> (pos & 0x1F))) != 0;
//}

void __cdecl XAnimWeightedAccumLerpedTrans(
                const float4 *fromVec,
                const float4 *toVec,
                float keyFrameLerpFrac,
                float weightScale,
                int *dataInt,
                DObjAnimMat *totalRotTrans)
{
    float frameVec_4; // [esp+18h] [ebp-18h]
    float frameVec_8; // [esp+1Ch] [ebp-14h]
    float lerp[3]; // [esp+20h] [ebp-10h] BYREF
    const float *minsVec; // [esp+2Ch] [ebp-4h]

    Vec3Lerp(fromVec->v, toVec->v, keyFrameLerpFrac, lerp);
    minsVec = (const float *)dataInt;
    frameVec_4 = (float)(*((float *)dataInt + 4) * lerp[1]) + *((float *)dataInt + 1);
    frameVec_8 = (float)(*((float *)dataInt + 5) * lerp[2]) + *((float *)dataInt + 2);
    totalRotTrans->trans[0] = (float)(weightScale * (float)((float)(*((float *)dataInt + 3) * lerp[0]) + *(float *)dataInt))
                                                    + totalRotTrans->trans[0];
    totalRotTrans->trans[1] = (float)(weightScale * frameVec_4) + totalRotTrans->trans[1];
    totalRotTrans->trans[2] = (float)(weightScale * frameVec_8) + totalRotTrans->trans[2];
    totalRotTrans->transWeight = totalRotTrans->transWeight + weightScale;
}

void __cdecl XAnimWeightedAccumTrans(float weightScale, float *dataInt, DObjAnimMat *totalRotTrans)
{
    totalRotTrans->trans[0] = (float)(weightScale * *dataInt) + totalRotTrans->trans[0];
    totalRotTrans->trans[1] = (float)(weightScale * dataInt[1]) + totalRotTrans->trans[1];
    totalRotTrans->trans[2] = (float)(weightScale * dataInt[2]) + totalRotTrans->trans[2];
    totalRotTrans->transWeight = totalRotTrans->transWeight + weightScale;
}

void __cdecl Short2LerpAsVec2(const __int16 *from, const __int16 *to, float frac, float *out)
{
    *out = (float)*from + (float)((float)(*to - *from) * frac);
    out[1] = (float)from[1] + (float)((float)(to[1] - from[1]) * frac);
}

