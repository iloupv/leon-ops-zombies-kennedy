#include "ik_process.h"
#include "ik_import.h"
#include "ik_math.h"
#include "ik_layers.h"

void __cdecl IK_GeneratePreIKMatrices(IKState *ikState, bool isLocalBones)
{
    IKImport_GetBoneMatrixArray(ikState, isLocalBones);
    if ( !isLocalBones
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ik\\ik_process.cpp", 64, 0, "%s", "isLocalBones") )
    {
        __debugbreak();
    }
}

// local variable allocation has failed, the output may be wrong!
void    IK_GenerateIKXformMatrices(IKState *ikState, bool isLocalBones)
{
    float xformMatPreScaled[4][4]; // [esp+18h] [ebp-1ECh] BYREF
    float *v4; // [esp+5Ch] [ebp-1A8h]
    float *v5; // [esp+60h] [ebp-1A4h]
    float *v6; // [esp+64h] [ebp-1A0h]
    __int64 v7; // [esp+68h] [ebp-19Ch]
    __int64 v8; // [esp+70h] [ebp-194h]
    __int64 v9; // [esp+78h] [ebp-18Ch]
    __int64 v10; // [esp+80h] [ebp-184h]
    __int64 v11; // [esp+88h] [ebp-17Ch]
    __int64 v12; // [esp+90h] [ebp-174h]
    __int64 v13; // [esp+98h] [ebp-16Ch]
    __int64 v14; // [esp+A0h] [ebp-164h]
    float *v15; // [esp+A8h] [ebp-15Ch]
    float *v16; // [esp+ACh] [ebp-158h]
    float *v17; // [esp+B0h] [ebp-154h]
    float *v18; // [esp+B4h] [ebp-150h]
    float preModelInverseMat[19]; // [esp+B8h] [ebp-14Ch] BYREF
    float v20[5]; // [esp+104h] [ebp-100h] BYREF
    float v21[4]; // [esp+118h] [ebp-ECh] BYREF
    float v22; // [esp+128h] [ebp-DCh]
    float v23; // [esp+12Ch] [ebp-D8h]
    float v24; // [esp+130h] [ebp-D4h]
    int v25; // [esp+134h] [ebp-D0h]
    float v26; // [esp+138h] [ebp-CCh]
    float v27; // [esp+13Ch] [ebp-C8h]
    float v28; // [esp+140h] [ebp-C4h]
    int v29; // [esp+144h] [ebp-C0h]
    float postModelMat[4][4]; // [esp+158h] [ebp-ACh] BYREF
    float preModelMat[4][4]; // [esp+198h] [ebp-6Ch] BYREF
    int v32; // [esp+1DCh] [ebp-28h]
    int i; // [esp+1E0h] [ebp-24h]
    float (*pPostModelMat)[4][4]; // [esp+1E4h] [ebp-20h]
    float (*pPreModelMat)[4][4]; // [esp+1E8h] [ebp-1Ch]
    float (*xformMat)[4][4]; // [esp+1ECh] [ebp-18h]
    float maxLayerLerp; // [esp+1F0h] [ebp-14h]
    //_UNKNOWN *v38; // [esp+1F8h] [ebp-Ch]
    //IKState *ikStatea; // [esp+1FCh] [ebp-8h]
    //int vars0; // [esp+204h] [ebp+0h]
    //
    //v38 = a1;
    //ikStatea = (IKState *)vars0;
    maxLayerLerp = IKImport_GetMaxLayerLerp(ikState);
    xformMat = ikState->matArrayXforms;
    pPreModelMat = ikState->matArrayPreIK;
    pPostModelMat = ikState->matArrayPostIK;
    i = 0;
    while (i < 23)
    {
        HIBYTE(v32) = ikState->ikBoneToObjBone[i] != 161;
        if (HIBYTE(v32))
        {
            if (isLocalBones)
            {
                if (pPreModelMat == (float (*)[4][4])preModelInverseMat
                    && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\ik\\ik_math.h", 165, 0, "%s", "in != out"))
                {
                    __debugbreak();
                }
                preModelInverseMat[0] = (*pPreModelMat)[0][0];
                preModelInverseMat[1] = (*pPreModelMat)[1][0];
                preModelInverseMat[2] = (*pPreModelMat)[2][0];
                preModelInverseMat[3] = 0.0f;
                preModelInverseMat[4] = (*pPreModelMat)[0][1];
                preModelInverseMat[5] = (*pPreModelMat)[1][1];
                preModelInverseMat[6] = (*pPreModelMat)[2][1];
                preModelInverseMat[7] = 0.0f;
                preModelInverseMat[8] = (*pPreModelMat)[0][2];
                preModelInverseMat[9] = (*pPreModelMat)[1][2];
                preModelInverseMat[10] = (*pPreModelMat)[2][2];
                preModelInverseMat[11] = 0.0f;
                (*xformMat)[0][0] = (float)((float)((*pPostModelMat)[0][0] * preModelInverseMat[0])
                    + (float)((*pPostModelMat)[0][1] * preModelInverseMat[4]))
                    + (float)((*pPostModelMat)[0][2] * preModelInverseMat[8]);
                (*xformMat)[0][1] = (float)((float)((*pPostModelMat)[0][0] * preModelInverseMat[1])
                    + (float)((*pPostModelMat)[0][1] * preModelInverseMat[5]))
                    + (float)((*pPostModelMat)[0][2] * preModelInverseMat[9]);
                (*xformMat)[0][2] = (float)((float)((*pPostModelMat)[0][0] * preModelInverseMat[2])
                    + (float)((*pPostModelMat)[0][1] * preModelInverseMat[6]))
                    + (float)((*pPostModelMat)[0][2] * preModelInverseMat[10]);
                (*xformMat)[0][3] = 0.0f;
                (*xformMat)[1][0] = (float)((float)((*pPostModelMat)[1][0] * preModelInverseMat[0])
                    + (float)((*pPostModelMat)[1][1] * preModelInverseMat[4]))
                    + (float)((*pPostModelMat)[1][2] * preModelInverseMat[8]);
                (*xformMat)[1][1] = (float)((float)((*pPostModelMat)[1][0] * preModelInverseMat[1])
                    + (float)((*pPostModelMat)[1][1] * preModelInverseMat[5]))
                    + (float)((*pPostModelMat)[1][2] * preModelInverseMat[9]);
                (*xformMat)[1][2] = (float)((float)((*pPostModelMat)[1][0] * preModelInverseMat[2])
                    + (float)((*pPostModelMat)[1][1] * preModelInverseMat[6]))
                    + (float)((*pPostModelMat)[1][2] * preModelInverseMat[10]);
                (*xformMat)[1][3] = 0.0f;
                (*xformMat)[2][0] = (float)((float)((*pPostModelMat)[2][0] * preModelInverseMat[0])
                    + (float)((*pPostModelMat)[2][1] * preModelInverseMat[4]))
                    + (float)((*pPostModelMat)[2][2] * preModelInverseMat[8]);
                (*xformMat)[2][1] = (float)((float)((*pPostModelMat)[2][0] * preModelInverseMat[1])
                    + (float)((*pPostModelMat)[2][1] * preModelInverseMat[5]))
                    + (float)((*pPostModelMat)[2][2] * preModelInverseMat[9]);
                (*xformMat)[2][2] = (float)((float)((*pPostModelMat)[2][0] * preModelInverseMat[2])
                    + (float)((*pPostModelMat)[2][1] * preModelInverseMat[6]))
                    + (float)((*pPostModelMat)[2][2] * preModelInverseMat[10]);
                (*xformMat)[2][3] = 0.0f;
                v18 = (*xformMat)[3];
                v17 = (*pPreModelMat)[3];
                v16 = (*pPostModelMat)[3];
                (*xformMat)[3][0] = (*pPostModelMat)[3][0] - (*pPreModelMat)[3][0];
                v18[1] = v16[1] - v17[1];
                v18[2] = v16[2] - v17[2];
                (*xformMat)[3][3] = 1.0f;
            }
            else
            {
                ikCalcBoneModelMatrix(ikState, i, (*ikState->matArrayPreIK)[0], preModelMat);
                ikCalcBoneModelMatrix(ikState, i, (*ikState->matArrayPostIK)[0], postModelMat);
                if (preModelMat == (float (*)[4])v21
                    && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\ik\\ik_math.h", 165, 0, "%s", "in != out"))
                {
                    __debugbreak();
                }
                v21[0] = preModelMat[0][0];
                v21[1] = preModelMat[1][0];
                v21[2] = preModelMat[2][0];
                v21[3] = 0.0f;
                v22 = preModelMat[0][1];
                v23 = preModelMat[1][1];
                v24 = preModelMat[2][1];
                v25 = 0.0f;
                v26 = preModelMat[0][2];
                v27 = preModelMat[1][2];
                v28 = preModelMat[2][2];
                v29 = 0.0f;
                (*xformMat)[0][0] = (float)((float)(postModelMat[0][0] * preModelMat[0][0])
                    + (float)(postModelMat[0][1] * preModelMat[0][1]))
                    + (float)(postModelMat[0][2] * preModelMat[0][2]);
                (*xformMat)[0][1] = (float)((float)(postModelMat[0][0] * v21[1]) + (float)(postModelMat[0][1] * v23))
                    + (float)(postModelMat[0][2] * v27);
                (*xformMat)[0][2] = (float)((float)(postModelMat[0][0] * v21[2]) + (float)(postModelMat[0][1] * v24))
                    + (float)(postModelMat[0][2] * v28);
                (*xformMat)[0][3] = 0.0f;
                (*xformMat)[1][0] = (float)((float)(postModelMat[1][0] * v21[0]) + (float)(postModelMat[1][1] * v22))
                    + (float)(postModelMat[1][2] * v26);
                (*xformMat)[1][1] = (float)((float)(postModelMat[1][0] * v21[1]) + (float)(postModelMat[1][1] * v23))
                    + (float)(postModelMat[1][2] * v27);
                (*xformMat)[1][2] = (float)((float)(postModelMat[1][0] * v21[2]) + (float)(postModelMat[1][1] * v24))
                    + (float)(postModelMat[1][2] * v28);
                (*xformMat)[1][3] = 0.0f;
                (*xformMat)[2][0] = (float)((float)(postModelMat[2][0] * v21[0]) + (float)(postModelMat[2][1] * v22))
                    + (float)(postModelMat[2][2] * v26);
                (*xformMat)[2][1] = (float)((float)(postModelMat[2][0] * v21[1]) + (float)(postModelMat[2][1] * v23))
                    + (float)(postModelMat[2][2] * v27);
                (*xformMat)[2][2] = (float)((float)(postModelMat[2][0] * v21[2]) + (float)(postModelMat[2][1] * v24))
                    + (float)(postModelMat[2][2] * v28);
                (*xformMat)[2][3] = 0.0f;
                //LODWORD(v20[4]) = preModelMat[3];
                //LODWORD(v20[3]) = postModelMat[3];
                v20[0] = postModelMat[3][0] - preModelMat[3][0];
                v20[1] = postModelMat[3][1] - preModelMat[3][1];
                v20[2] = postModelMat[3][2] - preModelMat[3][2];
                ikMatrixTransformVector34(v20, (const float (*)[4])v21, (*xformMat)[3]);
                (*xformMat)[3][3] = 1.0f;
            }
            if (maxLayerLerp < 1.0)
            {
                v15 = (float *)xformMat;
                v7 = *(_QWORD *)xformMat;
                v8 = *(_QWORD *)&(*xformMat)[0][2];
                v6 = (*xformMat)[1];
                v9 = *(_QWORD *)&(*xformMat)[1][0];
                v10 = *(_QWORD *)&(*xformMat)[1][2];
                v5 = (*xformMat)[2];
                v11 = *(_QWORD *)&(*xformMat)[2][0];
                v12 = *(_QWORD *)&(*xformMat)[2][2];
                v4 = (*xformMat)[3];
                v13 = *(_QWORD *)&(*xformMat)[3][0];
                v14 = *(_QWORD *)&(*xformMat)[3][2];
                *(_QWORD *)&xformMatPreScaled[0][0] = v7;
                *(_QWORD *)&xformMatPreScaled[0][2] = v8;
                *(_QWORD *)&xformMatPreScaled[1][0] = v9;
                *(_QWORD *)&xformMatPreScaled[1][2] = v10;
                *(_QWORD *)&xformMatPreScaled[2][0] = v11;
                *(_QWORD *)&xformMatPreScaled[2][2] = v12;
                *(_QWORD *)&xformMatPreScaled[3][0] = v13;
                *(_QWORD *)&xformMatPreScaled[3][2] = v14;
                ikMatrixLerp44((float (*)[4])ikIdentityMatrix44, xformMatPreScaled, maxLayerLerp, (float (*)[4])xformMat);
            }
            ikNormalizedMatrixAssert_func((float (*)[4])xformMat);
        }
        else
        {
            ikMatrixIdentity44((float (*)[4])xformMat);
        }
        ++i;
        ++xformMat;
        ++pPreModelMat;
        ++pPostModelMat;
    }
}

char __cdecl IK_HasAnimatedBones(IKState *ikState)
{
    int i; // [esp+4h] [ebp-4h]

    for ( i = 1; i < 23; ++i )
    {
        if ( ikState->ikBoneToObjBone[i] != 161 && IK_IsCalcBone(ikState, i) )
            return 1;
    }
    return 0;
}

void    IK_Process(IKState *ikState, bool isLocalBones)
{
    float matArrayPostIK[23][4][4];
    float matArrayPreIK[23][4][4];
    float matArrayXforms[23][4][4];
    float matArrayCache[23][4][4];
    float matArrayCachePre[23][4][4];

    iassert(!ikState->matArrayPostIK);

    ikState->matArrayPostIK = matArrayPostIK;
    ikState->matArrayPreIK = matArrayPreIK;
    ikState->matArrayXforms = matArrayXforms;
    ikState->matArrayCache = matArrayCache;
    ikState->matArrayCachePre = matArrayCachePre;

    ikState->cacheActive = 1;
    ikState->bHasActiveLayers = 1;
    if ( ikState->cacheActive )
    {
        memset(ikState->matArrayCache, 0, sizeof(matArrayCache));
        ikState->cacheActive = 0;
    }
    memset(ikState->matArrayCachePre, 0, sizeof(matArrayCachePre));
    if ( IK_HasAnimatedBones(ikState) )
    {
        ikState->modifiedIKBones = 0;
        IK_GeneratePreIKMatrices(ikState, isLocalBones);
        memcpy(ikState->matArrayPostIK, ikState->matArrayPreIK, sizeof(matArrayPostIK));
        if ( !ikState->bJointVarsValid )
            IK_GetJointVars(ikState);
        IKImport_Profiler(ikState);
        IK_ProcessLayers(ikState);
        if ( ikState->bHasActiveLayers )
            IK_GenerateIKXformMatrices(ikState, isLocalBones);
        IKImport_ApplyIKToSkeleton(ikState, isLocalBones);
        if ( IKImport_GetVar_IK_Debug() == 2 )
            IKImport_DrawDebugSkeleton(ikState);
    }

    ikState->matArrayPostIK = 0;
    ikState->matArrayPreIK = 0;
    ikState->matArrayXforms = 0;
    ikState->matArrayCache = 0;
    ikState->matArrayCachePre = 0;
}

