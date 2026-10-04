#include "r_model_skin.h"
#include "r_dvars.h"
#include <win32/win_common.h>
#include "r_dobj_skin.h"
#include <qcommon/threads.h>
#include <EffectsCore/fx_marks.h>
#include <win32/win_main.h>

bool TensionUsage[4];
float TensionBuffer[4][4000];

void __cdecl R_SkinXSurfaceSkinned(
                const XSurface *xsurf,
                const DObjSkelMat *boneMatrix,
                GfxPackedVertex *skinVerticesOut)
{
    if ( (xsurf->flags & 0x80) != 0 )
        R_SkinXSurfaceWeight(xsurf, xsurf->verts0, &xsurf->vertInfo, boneMatrix, skinVerticesOut);
    else
        R_SkinXSurfaceRigid(xsurf, xsurf->vertCount, boneMatrix, skinVerticesOut);
}

void __cdecl R_SkinXSurfaceWeight(
                const XSurface *xsurf,
                const GfxPackedVertex *inVerts,
                const XSurfaceVertexInfo *vertexInfo,
                const DObjSkelMat *boneMatrix,
                GfxPackedVertex *outVerts)
{
    float v5; // [esp+0h] [ebp-28h]
    int i; // [esp+4h] [ebp-24h]
    unsigned int numVerts; // [esp+8h] [ebp-20h]
    int tensionBufferStage; // [esp+Ch] [ebp-1Ch]
    unsigned int iVert; // [esp+10h] [ebp-18h]
    const unsigned __int16 *vertsBlend; // [esp+18h] [ebp-10h]
    const GfxPackedVertex *pverts; // [esp+1Ch] [ebp-Ch]
    int vertIndex; // [esp+20h] [ebp-8h]
    float *vertsTension; // [esp+24h] [ebp-4h]

    vertIndex = 0;
    vertsBlend = vertexInfo->vertsBlend;
    numVerts = vertexInfo->vertCount[3] + vertexInfo->vertCount[2] + vertexInfo->vertCount[1] + vertexInfo->vertCount[0];
    tensionBufferStage = -1;
    vertsTension = 0;
    if ( numVerts <= 0x3E8 && vertexInfo->tensionData && r_tension_enable->current.enabled )
    {
        Sys_EnterCriticalSection(CRITSECT_TENSION);
        for ( i = 0; i < 4; ++i )
        {
            if ( !TensionUsage[i] )
            {
                TensionUsage[i] = 1;
                tensionBufferStage = i;
                vertsTension = TensionBuffer[i];
                break;
            }
        }
        Sys_LeaveCriticalSection(CRITSECT_TENSION);
    }
    pverts = inVerts;
    if ( numVerts )
    {
        if ( vertexInfo->vertCount[0] )
        {
            R_SkinXSurfaceWeight0(inVerts, vertsBlend, vertexInfo->vertCount[0], boneMatrix, outVerts);
            pverts = &inVerts[vertexInfo->vertCount[0]];
            vertIndex = vertexInfo->vertCount[0];
            vertsBlend += vertIndex;
            if ( vertsTension )
                vertsTension += 4 * vertexInfo->vertCount[0];
        }
        if ( vertexInfo->vertCount[1] )
        {
            R_SkinXSurfaceWeight1(pverts, vertsBlend, vertexInfo->vertCount[1], boneMatrix, &outVerts[vertIndex]);
            pverts += vertexInfo->vertCount[1];
            vertIndex += vertexInfo->vertCount[1];
            vertsBlend += 3 * vertexInfo->vertCount[1];
            if ( vertsTension )
                vertsTension += 4 * vertexInfo->vertCount[1];
        }
        if ( vertexInfo->vertCount[2] )
        {
            R_SkinXSurfaceWeight2(pverts, vertsBlend, vertexInfo->vertCount[2], boneMatrix, &outVerts[vertIndex]);
            pverts += vertexInfo->vertCount[2];
            vertIndex += vertexInfo->vertCount[2];
            vertsBlend += 5 * vertexInfo->vertCount[2];
            if ( vertsTension )
                vertsTension += 4 * vertexInfo->vertCount[2];
        }
        if ( vertexInfo->vertCount[3] )
        {
            R_SkinXSurfaceWeight3(pverts, vertsBlend, vertexInfo->vertCount[3], boneMatrix, &outVerts[vertIndex]);
            if ( vertsTension )
                vertsTension += 4 * vertexInfo->vertCount[3];
        }
        if ( numVerts > 0x3E8 || !vertexInfo->tensionData || vertsTension )
        {
            if ( tensionBufferStage != -1 )
            {
                //BLOPS_NULLSUB();
                Sys_EnterCriticalSection(CRITSECT_TENSION);
                TensionUsage[tensionBufferStage] = 0;
                Sys_LeaveCriticalSection(CRITSECT_TENSION);
            }
        }
        else
        {
            for ( iVert = 0; iVert < numVerts; ++iVert )
            {
                if ( outVerts->binormalSign >= 0.0 )
                    v5 = 1.0f;
                else
                    v5 = -1.0f;
                outVerts->binormalSign = v5;
                ++inVerts;
                ++outVerts;
            }
        }
    }
}

void __cdecl R_SkinXSurfaceWeight0(
                const GfxPackedVertex *vertsIn,
                const unsigned __int16 *vertexBlend,
                int vertCount,
                const DObjSkelMat *boneMatrix,
                GfxPackedVertex *vertsOut)
{
    int vertIndex; // [esp+1Ch] [ebp-4h]

    if ( !vertsOut
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_model_skin.cpp", 1067, 0, "%s", "vertsOut") )
    {
        __debugbreak();
    }
    vertIndex = 0;
    while ( vertIndex < vertCount )
    {
        MatrixTransformVertexAndBasis(
            vertsIn->xyz,
            vertsIn->binormalSign,
            vertsIn->normal,
            vertsIn->tangent,
            (const DObjSkelMat *)((char *)boneMatrix + *vertexBlend),
            vertsOut);
        vertsOut->color.packed = vertsIn->color.packed;
        vertsOut->texCoord.packed = vertsIn->texCoord.packed;
        ++vertIndex;
        ++vertsIn;
        ++vertexBlend;
        ++vertsOut;
    }
}

void __cdecl MatrixTransformVertexAndBasis(
                const float *offset,
                float binormalSign,
                PackedUnitVec normal,
                PackedUnitVec tangent,
                const DObjSkelMat *mat,
                GfxPackedVertex *vert)
{
    PackedUnitVec v6; // [esp+0h] [ebp-78h]
    float v7[3]; // [esp+1Ch] [ebp-5Ch] BYREF
    PackedUnitVec v8; // [esp+28h] [ebp-50h]
    PackedUnitVec v9; // [esp+30h] [ebp-48h]
    PackedUnitVec v10; // [esp+34h] [ebp-44h]
    PackedUnitVec v11; // [esp+38h] [ebp-40h]
    float v12; // [esp+3Ch] [ebp-3Ch]
    float v13; // [esp+40h] [ebp-38h]
    int v14; // [esp+44h] [ebp-34h]
    float out[3]; // [esp+4Ch] [ebp-2Ch] BYREF
    PackedUnitVec in; // [esp+58h] [ebp-20h]
    float rotated[3]; // [esp+60h] [ebp-18h]
    float unpacked[3]; // [esp+6Ch] [ebp-Ch]

    vert->xyz[0] = (float)((float)((float)(*offset * mat->axis[0][0]) + (float)(offset[1] * mat->axis[1][0]))
                                             + (float)(offset[2] * mat->axis[2][0]))
                             + mat->origin[0];
    vert->xyz[1] = (float)((float)((float)(*offset * mat->axis[0][1]) + (float)(offset[1] * mat->axis[1][1]))
                                             + (float)(offset[2] * mat->axis[2][1]))
                             + mat->origin[1];
    vert->xyz[2] = (float)((float)((float)(*offset * mat->axis[0][2]) + (float)(offset[1] * mat->axis[1][2]))
                                             + (float)(offset[2] * mat->axis[2][2]))
                             + mat->origin[2];
    vert->binormalSign = binormalSign;
    out[0] = *(float *)&normal.packed;
    out[1] = *(float *)&normal.packed;
    out[2] = *(float *)&normal.packed;
    in.packed = normal.packed;
    Vec3UnpackUnitVec(normal, out);
    in.packed = 0;
    unpacked[0] = out[0];
    unpacked[1] = out[1];
    unpacked[2] = out[2];
    rotated[0] = (float)((float)(out[0] * mat->axis[0][0]) + (float)(out[1] * mat->axis[1][0]))
                         + (float)(out[2] * mat->axis[2][0]);
    rotated[1] = (float)((float)(out[0] * mat->axis[0][1]) + (float)(out[1] * mat->axis[1][1]))
                         + (float)(out[2] * mat->axis[2][1]);
    rotated[2] = (float)((float)(out[0] * mat->axis[0][2]) + (float)(out[1] * mat->axis[1][2]))
                         + (float)(out[2] * mat->axis[2][2]);
    v12 = rotated[1];
    v13 = rotated[2];
    v14 = 0;
    v9.array[0] = (int)(float)((float)(rotated[0] * 127.0) + 127.5);
    v9.array[1] = (int)(float)((float)(rotated[1] * 127.0) + 127.5);
    v9.array[2] = (int)(float)((float)(rotated[2] * 127.0) + 127.5);
    v9.array[3] = 63;
    v10.packed = v9.packed;
    v11.packed = v9.packed;
    vert->normal = v9;
    v7[0] = *(float *)&tangent.packed;
    v7[1] = *(float *)&tangent.packed;
    v7[2] = *(float *)&tangent.packed;
    v8.packed = tangent.packed;
    Vec3UnpackUnitVec(tangent, v7);
    v8.packed = 0;
    unpacked[0] = v7[0];
    unpacked[1] = v7[1];
    unpacked[2] = v7[2];
    rotated[0] = (float)((float)(v7[0] * mat->axis[0][0]) + (float)(v7[1] * mat->axis[1][0]))
                         + (float)(v7[2] * mat->axis[2][0]);
    rotated[1] = (float)((float)(v7[0] * mat->axis[0][1]) + (float)(v7[1] * mat->axis[1][1]))
                         + (float)(v7[2] * mat->axis[2][1]);
    rotated[2] = (float)((float)(v7[0] * mat->axis[0][2]) + (float)(v7[1] * mat->axis[1][2]))
                         + (float)(v7[2] * mat->axis[2][2]);
    v6.array[0] = (int)(float)((float)(rotated[0] * 127.0) + 127.5);
    v6.array[1] = (int)(float)((float)(rotated[1] * 127.0) + 127.5);
    v6.array[2] = (int)(float)((float)(rotated[2] * 127.0) + 127.5);
    v6.array[3] = 63;
    vert->tangent = v6;
}

void __cdecl R_SkinXSurfaceWeight1(
                const GfxPackedVertex *vertsIn,
                const unsigned __int16 *vertexBlend,
                int vertCount,
                const DObjSkelMat *boneMatrix,
                GfxPackedVertex *vertsOut)
{
    const float4 *matrix; // [esp+14h] [ebp-1Ch]
    float offset[3]; // [esp+1Ch] [ebp-14h] BYREF
    float boneWeight; // [esp+28h] [ebp-8h]
    int vertIndex; // [esp+2Ch] [ebp-4h]

    if ( !vertsOut
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_model_skin.cpp", 1100, 0, "%s", "vertsOut") )
    {
        __debugbreak();
    }
    vertIndex = 0;
    while ( vertIndex < vertCount )
    {
        MatrixTransformVertexAndBasis(
            vertsIn->xyz,
            vertsIn->binormalSign,
            vertsIn->normal,
            vertsIn->tangent,
            (const DObjSkelMat *)((char *)boneMatrix + *vertexBlend),
            vertsOut);
        matrix = (const float4 *)((char *)boneMatrix + vertexBlend[1]);
        boneWeight = (float)vertexBlend[2] * 0.000015258789;
        R_TransformSkelMat(vertsIn->xyz, matrix, offset);
        offset[0] = boneWeight * offset[0];
        offset[1] = boneWeight * offset[1];
        offset[2] = boneWeight * offset[2];
        boneWeight = 1.0 - boneWeight;
        vertsOut->xyz[0] = (float)(boneWeight * vertsOut->xyz[0]) + offset[0];
        vertsOut->xyz[1] = (float)(boneWeight * vertsOut->xyz[1]) + offset[1];
        vertsOut->xyz[2] = (float)(boneWeight * vertsOut->xyz[2]) + offset[2];
        vertsOut->color.packed = vertsIn->color.packed;
        vertsOut->texCoord.packed = vertsIn->texCoord.packed;
        ++vertIndex;
        ++vertsIn;
        vertexBlend += 3;
        ++vertsOut;
    }
}

void __cdecl R_SkinXSurfaceWeight2(
                const GfxPackedVertex *vertsIn,
                const unsigned __int16 *vertexBlend,
                int vertCount,
                const DObjSkelMat *boneMatrix,
                GfxPackedVertex *vertsOut)
{
    const float4 *matrix; // [esp+14h] [ebp-2Ch]
    const float4 *matrixa; // [esp+14h] [ebp-2Ch]
    float offset[3]; // [esp+1Ch] [ebp-24h] BYREF
    float boneWeight; // [esp+28h] [ebp-18h]
    float totalOffset[3]; // [esp+2Ch] [ebp-14h] BYREF
    int vertIndex; // [esp+38h] [ebp-8h]
    float totalBoneWeight; // [esp+3Ch] [ebp-4h]

    if ( !vertsOut
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_model_skin.cpp", 1152, 0, "%s", "vertsOut") )
    {
        __debugbreak();
    }
    vertIndex = 0;
    while ( vertIndex < vertCount )
    {
        MatrixTransformVertexAndBasis(
            vertsIn->xyz,
            vertsIn->binormalSign,
            vertsIn->normal,
            vertsIn->tangent,
            (const DObjSkelMat *)((char *)boneMatrix + *vertexBlend),
            vertsOut);
        matrix = (const float4 *)((char *)boneMatrix + vertexBlend[1]);
        totalBoneWeight = (float)vertexBlend[2] * 0.000015258789;
        R_TransformSkelMat(vertsIn->xyz, matrix, totalOffset);
        totalOffset[0] = totalBoneWeight * totalOffset[0];
        totalOffset[1] = totalBoneWeight * totalOffset[1];
        totalOffset[2] = totalBoneWeight * totalOffset[2];
        matrixa = (const float4 *)((char *)boneMatrix + vertexBlend[3]);
        boneWeight = (float)vertexBlend[4] * 0.000015258789;
        totalBoneWeight = totalBoneWeight + boneWeight;
        R_TransformSkelMat(vertsIn->xyz, matrixa, offset);
        totalOffset[0] = (float)(boneWeight * offset[0]) + totalOffset[0];
        totalOffset[1] = (float)(boneWeight * offset[1]) + totalOffset[1];
        totalOffset[2] = (float)(boneWeight * offset[2]) + totalOffset[2];
        boneWeight = 1.0 - totalBoneWeight;
        vertsOut->xyz[0] = (float)((float)(1.0 - totalBoneWeight) * vertsOut->xyz[0]) + totalOffset[0];
        vertsOut->xyz[1] = (float)(boneWeight * vertsOut->xyz[1]) + totalOffset[1];
        vertsOut->xyz[2] = (float)(boneWeight * vertsOut->xyz[2]) + totalOffset[2];
        vertsOut->color.packed = vertsIn->color.packed;
        vertsOut->texCoord.packed = vertsIn->texCoord.packed;
        ++vertIndex;
        ++vertsIn;
        vertexBlend += 5;
        ++vertsOut;
    }
}

void __cdecl R_SkinXSurfaceWeight3(
                const GfxPackedVertex *vertsIn,
                const unsigned __int16 *vertexBlend,
                int vertCount,
                const DObjSkelMat *boneMatrix,
                GfxPackedVertex *vertsOut)
{
    const float4 *matrix; // [esp+14h] [ebp-2Ch]
    const float4 *matrixa; // [esp+14h] [ebp-2Ch]
    const float4 *matrixb; // [esp+14h] [ebp-2Ch]
    float offset[3]; // [esp+1Ch] [ebp-24h] BYREF
    float boneWeight; // [esp+28h] [ebp-18h]
    float totalOffset[3]; // [esp+2Ch] [ebp-14h] BYREF
    int vertIndex; // [esp+38h] [ebp-8h]
    float totalBoneWeight; // [esp+3Ch] [ebp-4h]

    if ( !vertsOut
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_model_skin.cpp", 1220, 0, "%s", "vertsOut") )
    {
        __debugbreak();
    }
    vertIndex = 0;
    while ( vertIndex < vertCount )
    {
        MatrixTransformVertexAndBasis(
            vertsIn->xyz,
            vertsIn->binormalSign,
            vertsIn->normal,
            vertsIn->tangent,
            (const DObjSkelMat *)((char *)boneMatrix + *vertexBlend),
            vertsOut);
        matrix = (const float4 *)((char *)boneMatrix + vertexBlend[1]);
        totalBoneWeight = (float)vertexBlend[2] * 0.000015258789;
        R_TransformSkelMat(vertsIn->xyz, matrix, totalOffset);
        totalOffset[0] = totalBoneWeight * totalOffset[0];
        totalOffset[1] = totalBoneWeight * totalOffset[1];
        totalOffset[2] = totalBoneWeight * totalOffset[2];
        matrixa = (const float4 *)((char *)boneMatrix + vertexBlend[3]);
        boneWeight = (float)vertexBlend[4] * 0.000015258789;
        totalBoneWeight = totalBoneWeight + boneWeight;
        R_TransformSkelMat(vertsIn->xyz, matrixa, offset);
        totalOffset[0] = (float)(boneWeight * offset[0]) + totalOffset[0];
        totalOffset[1] = (float)(boneWeight * offset[1]) + totalOffset[1];
        totalOffset[2] = (float)(boneWeight * offset[2]) + totalOffset[2];
        matrixb = (const float4 *)((char *)boneMatrix + vertexBlend[5]);
        boneWeight = (float)vertexBlend[6] * 0.000015258789;
        totalBoneWeight = totalBoneWeight + boneWeight;
        R_TransformSkelMat(vertsIn->xyz, matrixb, offset);
        totalOffset[0] = (float)(boneWeight * offset[0]) + totalOffset[0];
        totalOffset[1] = (float)(boneWeight * offset[1]) + totalOffset[1];
        totalOffset[2] = (float)(boneWeight * offset[2]) + totalOffset[2];
        boneWeight = 1.0 - totalBoneWeight;
        vertsOut->xyz[0] = (float)((float)(1.0 - totalBoneWeight) * vertsOut->xyz[0]) + totalOffset[0];
        vertsOut->xyz[1] = (float)(boneWeight * vertsOut->xyz[1]) + totalOffset[1];
        vertsOut->xyz[2] = (float)(boneWeight * vertsOut->xyz[2]) + totalOffset[2];
        vertsOut->color.packed = vertsIn->color.packed;
        vertsOut->texCoord.packed = vertsIn->texCoord.packed;
        ++vertIndex;
        ++vertsIn;
        vertexBlend += 7;
        ++vertsOut;
    }
}

void __cdecl R_SkinXSurfaceRigid(
                const XSurface *surf,
                int totalVertCount,
                const DObjSkelMat *boneMatrix,
                GfxPackedVertex *vertices)
{
    unsigned int i; // [esp+14h] [ebp-1Ch]
    int vertCount; // [esp+18h] [ebp-18h]
    int vertIndex; // [esp+1Ch] [ebp-14h]
    GfxPackedVertex *vertex; // [esp+20h] [ebp-10h]
    const XRigidVertList *vertList; // [esp+24h] [ebp-Ch]
    GfxPackedVertex *v; // [esp+28h] [ebp-8h]
    const DObjSkelMat *bone; // [esp+2Ch] [ebp-4h]

    if ( !vertices
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_model_skin.cpp", 3906, 0, "%s", "vertices") )
    {
        __debugbreak();
    }
    if ( ((unsigned __int8)vertices & 0xF) != 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_model_skin.cpp",
                    3907,
                    0,
                    "%s",
                    "!(reinterpret_cast< unsigned >( vertices ) & 15)") )
    {
        __debugbreak();
    }
    if ( ((unsigned __int8)boneMatrix & 0xF) != 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_model_skin.cpp",
                    3908,
                    0,
                    "%s",
                    "!(reinterpret_cast< unsigned >( boneMatrix ) & 15)") )
    {
        __debugbreak();
    }
    v = surf->verts0;
    vertex = vertices;
    for ( i = 0; i < surf->vertListCount; ++i )
    {
        vertList = &surf->vertList[i];
        vertCount = vertList->vertCount;
        bone = (const DObjSkelMat *)((char *)boneMatrix + vertList->boneOffset);
        for ( vertIndex = 0; vertIndex < vertCount; ++vertIndex )
        {
            MatrixTransformVertexAndBasis(v->xyz, v->binormalSign, v->normal, v->tangent, bone, vertex);
            vertex->color.packed = v->color.packed;
            vertex->texCoord.packed = v->texCoord.packed;
            ++v;
            ++vertex;
        }
    }
    if ( vertex - vertices != totalVertCount
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_model_skin.cpp",
                    3937,
                    0,
                    "%s",
                    "vertex - vertices == totalVertCount") )
    {
        __debugbreak();
    }
}

void R_SkinXModelCmd(SkinXModelCmd *data)
{
    GfxPackedVertexNormal *normalOut; // [esp-28D0h] [ebp-28DCh]
    GfxPackedVertexNormal *normalIn; // [esp-28CCh] [ebp-28D8h]
    GfxPackedVertex *skinVerticesOut; // [esp-28C8h] [ebp-28D4h]
    const XSurface *xsurf; // [esp-28C4h] [ebp-28D0h]
    DObjSkelMat alignas(16) boneSkelMats[DOBJ_MAX_PARTS]; // [esp-28C0h] [ebp-28CCh] BYREF
    int j; // [esp-30h] [ebp-3Ch]
    GfxModelSkinnedSurface *skinnedSurf; // [esp-24h] [ebp-30h]
    unsigned int i; // [esp-20h] [ebp-2Ch]
    int boneIndex; // [esp-1Ch] [ebp-28h]
    GfxModelSkinnedSurface *surfPos; // [esp-18h] [ebp-24h]
    const DObjAnimMat *mat; // [esp-14h] [ebp-20h]
    GfxModelSkinnedSurface *modelSurfs; // [esp-10h] [ebp-1Ch]
    SkinXModelCmd *skinCmd; // [esp-Ch] [ebp-18h]
    bool fastSkin; // [esp-7h] [ebp-13h]
    bool sseStateUsed; // [esp-6h] [ebp-12h]
    bool useSSE_; // [esp-5h] [ebp-11h]
    BOOL useSSE; // [esp-4h] [ebp-10h]

    if (!Sys_QueryD3DDeviceOKEvent())
    {
        return;
    }

    PROF_SCOPED("R_SkinXModelCmd");

    useSSE = sys_SSE->current.enabled && r_sse_skinning->current.enabled;
    useSSE_ = useSSE;
    sseStateUsed = 0;
    fastSkin = gfxBuf.fastSkin;
    skinCmd = data;
    modelSurfs = (GfxModelSkinnedSurface *)data->modelSurfs;
    mat = data->mat;

    iassert(skinCmd);
    iassert(skinCmd->surfCount);
        
    surfPos = modelSurfs;
    boneIndex = -1;
    for (i = 0; i < skinCmd->surfCount; ++i)
    {
        skinnedSurf = surfPos;
        if (surfPos->skinnedCachedOffset == -3)
        {
            surfPos = (GfxModelSkinnedSurface *)((char *)surfPos + 4);
            continue;
        }

        if (boneIndex != skinnedSurf->info.boneIndex)
        {
            boneIndex = skinnedSurf->info.boneIndex;
            const int totalBones = boneIndex + skinnedSurf->info.boneCount;
            const DObjAnimMat *baseMats = &skinnedSurf->info.baseMat[-boneIndex];
            for (j = boneIndex; j < totalBones; ++j)
            {
                if ((skinCmd->surfacePartBits[j >> 5] & (0x80000000 >> (j & 0x1F))) == 0)
                    continue;

                if (sseStateUsed)
                {
                    sseStateUsed = 0;
                    _m_empty();
                }

                DObjSkelMat mat0, mat1;

                ConvertQuatToInverseSkelMat(&baseMats[j], &mat0);
                ConvertQuatToSkelMat(&mat[j], &mat1);

                mat1.origin[0] = mat1.origin[0] - skinCmd->viewoffset[0];
                mat1.origin[1] = mat1.origin[1] - skinCmd->viewoffset[1];
                mat1.origin[2] = mat1.origin[2] - skinCmd->viewoffset[2];
                R_MultiplySkelMat(&mat0, &mat1, &boneSkelMats[j]);

                boneSkelMats[j].axis[0][3] = 0.0f;
                boneSkelMats[j].axis[1][3] = 0.0f;
                boneSkelMats[j].axis[2][3] = 0.0f;
                boneSkelMats[j].origin[3] = 1.0f;
            }
        }

        if (skinnedSurf->skinnedCachedOffset == -2)
        {
            surfPos = (GfxModelSkinnedSurface *)((char *)surfPos + 56);
            continue;
        }

        surfPos = skinnedSurf + 1;
        xsurf = skinnedSurf->xsurf;
        iassert(xsurf);

        if (skinnedSurf->skinnedCachedOffset < 0)
        {
            iassert(((reinterpret_cast<uint>(skinnedSurf->skinnedVert) & 15) == 0));
            skinVerticesOut = skinnedSurf->skinnedVert;
        }
        else
        {
            iassert(gfxBuf.skinnedCacheLockAddr);
            iassert((reinterpret_cast<uint>(gfxBuf.skinnedCacheLockAddr) & 15) == 0);
            iassert((skinnedSurf->skinnedCachedOffset & 15) == 0);

            skinVerticesOut = (GfxPackedVertex *)&gfxBuf.skinnedCacheLockAddr[skinnedSurf->skinnedCachedOffset];
        }

        if (useSSE_)
        {
            if (!sseStateUsed)
            {
                sseStateUsed = 1;
                _m_empty();
            }
            normalIn = 0;
            normalOut = 0;
            if (fastSkin)
            {
                if (skinnedSurf->skinnedCachedOffset >= 0)
                    normalOut = &gfxBuf.skinnedCacheNormalsAddr[skinnedSurf->skinnedCachedOffset >> 5];
                if (skinnedSurf->oldSkinnedCachedOffset >= 0)
                    normalIn = &gfxBuf.oldSkinnedCacheNormalsAddr[skinnedSurf->oldSkinnedCachedOffset >> 5];
            }
            R_SkinXSurfaceSkinnedSse(xsurf, &boneSkelMats[boneIndex], normalIn, normalOut, skinVerticesOut);
        }
        else
        {
            R_SkinXSurfaceSkinned(xsurf, &boneSkelMats[boneIndex], skinVerticesOut);
        }
    }

    if (sseStateUsed)
        _m_empty();
}

void __cdecl R_MultiplySkelMat(const DObjSkelMat *mat0, const DObjSkelMat *mat1, DObjSkelMat *out)
{
    out->axis[0][0] = (float)((float)(mat0->axis[0][0] * mat1->axis[0][0]) + (float)(mat0->axis[0][1] * mat1->axis[1][0]))
                                    + (float)(mat0->axis[0][2] * mat1->axis[2][0]);
    out->axis[0][1] = (float)((float)(mat0->axis[0][0] * mat1->axis[0][1]) + (float)(mat0->axis[0][1] * mat1->axis[1][1]))
                                    + (float)(mat0->axis[0][2] * mat1->axis[2][1]);
    out->axis[0][2] = (float)((float)(mat0->axis[0][0] * mat1->axis[0][2]) + (float)(mat0->axis[0][1] * mat1->axis[1][2]))
                                    + (float)(mat0->axis[0][2] * mat1->axis[2][2]);
    out->axis[1][0] = (float)((float)(mat0->axis[1][0] * mat1->axis[0][0]) + (float)(mat0->axis[1][1] * mat1->axis[1][0]))
                                    + (float)(mat0->axis[1][2] * mat1->axis[2][0]);
    out->axis[1][1] = (float)((float)(mat0->axis[1][0] * mat1->axis[0][1]) + (float)(mat0->axis[1][1] * mat1->axis[1][1]))
                                    + (float)(mat0->axis[1][2] * mat1->axis[2][1]);
    out->axis[1][2] = (float)((float)(mat0->axis[1][0] * mat1->axis[0][2]) + (float)(mat0->axis[1][1] * mat1->axis[1][2]))
                                    + (float)(mat0->axis[1][2] * mat1->axis[2][2]);
    out->axis[2][0] = (float)((float)(mat0->axis[2][0] * mat1->axis[0][0]) + (float)(mat0->axis[2][1] * mat1->axis[1][0]))
                                    + (float)(mat0->axis[2][2] * mat1->axis[2][0]);
    out->axis[2][1] = (float)((float)(mat0->axis[2][0] * mat1->axis[0][1]) + (float)(mat0->axis[2][1] * mat1->axis[1][1]))
                                    + (float)(mat0->axis[2][2] * mat1->axis[2][1]);
    out->axis[2][2] = (float)((float)(mat0->axis[2][0] * mat1->axis[0][2]) + (float)(mat0->axis[2][1] * mat1->axis[1][2]))
                                    + (float)(mat0->axis[2][2] * mat1->axis[2][2]);
    out->origin[0] = (float)((float)((float)(mat0->origin[0] * mat1->axis[0][0])
                                                                 + (float)(mat0->origin[1] * mat1->axis[1][0]))
                                                 + (float)(mat0->origin[2] * mat1->axis[2][0]))
                                 + mat1->origin[0];
    out->origin[1] = (float)((float)((float)(mat0->origin[0] * mat1->axis[0][1])
                                                                 + (float)(mat0->origin[1] * mat1->axis[1][1]))
                                                 + (float)(mat0->origin[2] * mat1->axis[2][1]))
                                 + mat1->origin[1];
    out->origin[2] = (float)((float)((float)(mat0->origin[0] * mat1->axis[0][2])
                                                                 + (float)(mat0->origin[1] * mat1->axis[1][2]))
                                                 + (float)(mat0->origin[2] * mat1->axis[2][2]))
                                 + mat1->origin[2];
}


