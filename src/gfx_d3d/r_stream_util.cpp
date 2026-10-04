#include "r_stream_util.h"
#include <universal/com_math.h>

double __cdecl R_CalculateTriangleArea2D(const float (*texCoord)[2])
{
    float v2; // [esp+0h] [ebp-38h]
    float v6; // [esp+10h] [ebp-28h]
    float v7; // [esp+20h] [ebp-18h]
    float v8; // [esp+24h] [ebp-14h]

    if ( (*texCoord)[0] == (*texCoord)[2] && (*texCoord)[1] == (*texCoord)[3] )
        return 0.0;
    if ( (*texCoord)[0] == (*texCoord)[4] && (*texCoord)[1] == (*texCoord)[5] )
        return 0.0;
    if ( (*texCoord)[2] == (*texCoord)[4] && (*texCoord)[3] == (*texCoord)[5] )
        return 0.0;
    v7 = (float)(*texCoord)[4] - (float)(*texCoord)[2];
    v8 = (float)(*texCoord)[5] - (float)(*texCoord)[3];
    v6 = PointToLineDistSq2D((const float *)texCoord, &(*texCoord)[2], &(*texCoord)[4])
         * (float)((float)(v7 * v7) + (float)(v8 * v8));
    v2 = sqrt(v6);
    return v2 * 0.5;
}

double __cdecl R_CalculateTriangleArea3D(const float (*texCoord)[3])
{
    float v2; // [esp+0h] [ebp-34h]
    float v3; // [esp+4h] [ebp-30h]
    float v7; // [esp+14h] [ebp-20h]

    if ( (*texCoord)[0] == (*texCoord)[3] && (*texCoord)[1] == (*texCoord)[4] && (*texCoord)[2] == (*texCoord)[5] )
        return 0.0;
    if ( (*texCoord)[0] == (*texCoord)[6] && (*texCoord)[1] == (*texCoord)[7] && (*texCoord)[2] == (*texCoord)[8] )
        return 0.0;
    if ( (*texCoord)[3] == (*texCoord)[6] && (*texCoord)[4] == (*texCoord)[7] && (*texCoord)[5] == (*texCoord)[8] )
        return 0.0;
    v3 = PointToLineDistSq((const float *)texCoord, &(*texCoord)[3], &(*texCoord)[6]);
    v7 = Vec3DistanceSq(&(*texCoord)[3], &(*texCoord)[6]) * v3;
    v2 = sqrt(v7);
    return v2 * 0.5;
}

char __cdecl R_CalculateTriangleTopMipAabb(
                const float (*pos)[3],
                const float (*texCoord)[2],
                const float *textureSize,
                char filterState,
                float *outMins,
                float *outMaxs,
                float *outRadius)
{
    const float *v8; // ecx
    float *v9; // [esp+8h] [ebp-F8h]
    float v10; // [esp+Ch] [ebp-F4h]
    float *v11; // [esp+10h] [ebp-F0h]
    float *v12; // [esp+14h] [ebp-ECh]
    int triCorner; // [esp+98h] [ebp-68h]
    int triCornera; // [esp+98h] [ebp-68h]
    float normal[3]; // [esp+A4h] [ebp-5Ch] BYREF
    unsigned int filterState_FilterMasked; // [esp+B0h] [ebp-50h]
    float viewSphereRadii[3]; // [esp+B4h] [ebp-4Ch] BYREF
    float MAX_RADIUS; // [esp+C0h] [ebp-40h]
    float viewSphereCenters[3][3]; // [esp+C4h] [ebp-3Ch] BYREF
    float texGradient[2][3]; // [esp+E8h] [ebp-18h] BYREF

    MAX_RADIUS = sqrtf(5.0e11) * 0.99000001;
    if ( !R_CalculateTriangleTextureGradient(pos, texCoord, texGradient, normal) )
        return 0;
    if ( (float)((float)((float)(texGradient[0][0] * texGradient[0][0]) + (float)(texGradient[0][1] * texGradient[0][1]))
                         + (float)(texGradient[0][2] * texGradient[0][2])) <= 0.0
        && (float)((float)((float)(texGradient[1][0] * texGradient[1][0]) + (float)(texGradient[1][1] * texGradient[1][1]))
                         + (float)(texGradient[1][2] * texGradient[1][2])) <= 0.0 )
    {
        return 0;
    }
    viewSphereRadii[0] = R_CalculateTexelDensityFromGradient(texGradient, textureSize) * 800.0;
    filterState_FilterMasked = filterState & 7;
    if ( filterState_FilterMasked == 3 || filterState_FilterMasked == 4 )
    {
        if ( filterState_FilterMasked == 4 )
            viewSphereRadii[0] = viewSphereRadii[0] * 4.0;
        else
            viewSphereRadii[0] = viewSphereRadii[0] * 2.0;
        for ( triCorner = 0; triCorner != 3; ++triCorner )
        {
            v12 = viewSphereCenters[triCorner];
            *v12 = (*pos)[3 * triCorner];
            v8 = &(*pos)[3 * triCorner];
            v12[1] = v8[1];
            v12[2] = v8[2];
        }
    }
    else
    {
        if ( (filterState & 0x18) == 8 )
            viewSphereRadii[0] = viewSphereRadii[0] * 0.5;
        for ( triCornera = 0; triCornera != 3; ++triCornera )
        {
            v9 = viewSphereCenters[triCornera];
            v10 = viewSphereRadii[0];
            v11 = (float *)&(*pos)[3 * triCornera];
            *v9 = (float)(viewSphereRadii[0] * normal[0]) + *v11;
            v9[1] = (float)(v10 * normal[1]) + v11[1];
            v9[2] = (float)(v10 * normal[2]) + v11[2];
        }
    }
    if ( viewSphereRadii[0] > MAX_RADIUS )
        viewSphereRadii[0] = MAX_RADIUS;
    *outRadius = viewSphereRadii[0];
    viewSphereRadii[1] = viewSphereRadii[0];
    viewSphereRadii[2] = viewSphereRadii[0];
    if ( outMins )
    {
        if ( !outMaxs
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_stream_util.cpp", 199, 0, "%s", "outMaxs") )
        {
            __debugbreak();
        }
        R_CalculateMinimalAabbForSpheres(viewSphereCenters, viewSphereRadii, 3, outMins, outMaxs);
    }
    return 1;
}

void __cdecl R_CalculateMinimalAabbForSpheres(
                const float (*sphereCenters)[3],
                const float *sphereRadii,
                int sphereCount,
                float *outMins,
                float *outMaxs)
{
    int dimIter; // [esp+4h] [ebp-8h]
    int sphereIter; // [esp+8h] [ebp-4h]

    ClearBounds(outMins, outMaxs);
    for ( sphereIter = 0; sphereIter != sphereCount; ++sphereIter )
    {
        for ( dimIter = 0; dimIter != 3; ++dimIter )
        {
            if ( outMins[dimIter] > (float)((float)(*sphereCenters)[3 * sphereIter + dimIter] - sphereRadii[sphereIter]) )
                outMins[dimIter] = (float)(*sphereCenters)[3 * sphereIter + dimIter] - sphereRadii[sphereIter];
            if ( (float)((float)(*sphereCenters)[3 * sphereIter + dimIter] + sphereRadii[sphereIter]) > outMaxs[dimIter] )
                outMaxs[dimIter] = (float)(*sphereCenters)[3 * sphereIter + dimIter] + sphereRadii[sphereIter];
        }
    }
}

char __cdecl R_CalculateTriangleTextureGradient(
                const float (*pos)[3],
                const float (*texCoord)[2],
                float (*outTexGradient)[3],
                float *outNormal)
{
    float inputMatrixInv[3][3]; // [esp+28h] [ebp-68h] BYREF
    float outputValues[2][3]; // [esp+4Ch] [ebp-44h]
    int texCoordDim; // [esp+64h] [ebp-2Ch]
    float inputMatrix[3][3]; // [esp+68h] [ebp-28h] BYREF
    int xyzDim; // [esp+8Ch] [ebp-4h]

    inputMatrix[0][0] = (float)(*pos)[3] - (*pos)[0];
    inputMatrix[0][1] = (float)(*pos)[4] - (float)(*pos)[1];
    inputMatrix[0][2] = (float)(*pos)[5] - (float)(*pos)[2];
    inputMatrix[1][0] = (float)(*pos)[6] - (*pos)[0];
    inputMatrix[1][1] = (float)(*pos)[7] - (float)(*pos)[1];
    inputMatrix[1][2] = (float)(*pos)[8] - (float)(*pos)[2];
    Vec3Cross(inputMatrix[1], inputMatrix[0], outNormal);
    if ( (float)((float)((float)(*outNormal * *outNormal) + (float)(outNormal[1] * outNormal[1]))
                         + (float)(outNormal[2] * outNormal[2])) == 0.0 )
        return 0;
    Vec3Normalize(outNormal);
    inputMatrix[2][0] = *outNormal;
    inputMatrix[2][1] = outNormal[1];
    inputMatrix[2][2] = outNormal[2];
    outputValues[0][0] = (float)(*texCoord)[2] - (*texCoord)[0];
    outputValues[0][1] = (float)(*texCoord)[3] - (float)(*texCoord)[1];
    outputValues[1][0] = (float)(*texCoord)[4] - (*texCoord)[0];
    outputValues[1][1] = (float)(*texCoord)[5] - (float)(*texCoord)[1];
    MatrixInverse(inputMatrix, inputMatrixInv);
    for ( xyzDim = 0; xyzDim != 3; ++xyzDim )
    {
        for ( texCoordDim = 0; texCoordDim != 2; ++texCoordDim )
            (*outTexGradient)[3 * texCoordDim + xyzDim] = (float)(inputMatrixInv[xyzDim][0] * outputValues[0][texCoordDim])
                                                                                                    + (float)(inputMatrixInv[xyzDim][1] * outputValues[1][texCoordDim]);
    }
    return 1;
}

double __cdecl R_CalculateTexelDensityFromGradient(const float (*texGradient)[3], const float *textureSize)
{
    float v3; // [esp+0h] [ebp-1Ch]
    float v4; // [esp+4h] [ebp-18h]
    float v5; // [esp+8h] [ebp-14h]
    float v6; // [esp+Ch] [ebp-10h]
    float texGradientLengthSq; // [esp+14h] [ebp-8h]
    float texGradientLengthSq_4; // [esp+18h] [ebp-4h]

    texGradientLengthSq = (float)((float)((float)((float)((*texGradient)[0] * (*texGradient)[0])
                                                                                            + (float)((float)(*texGradient)[1] * (float)(*texGradient)[1]))
                                                                            + (float)((float)(*texGradient)[2] * (float)(*texGradient)[2]))
                                                            * *textureSize)
                                            * *textureSize;
    texGradientLengthSq_4 = (float)((float)((float)((float)((float)(*texGradient)[3] * (float)(*texGradient)[3])
                                                                                                + (float)((float)(*texGradient)[4] * (float)(*texGradient)[4]))
                                                                                + (float)((float)(*texGradient)[5] * (float)(*texGradient)[5]))
                                                                * textureSize[1])
                                                * textureSize[1];
    if ( texGradientLengthSq_4 >= 0.000001 )
    {
        if ( texGradientLengthSq >= 0.000001 )
        {
            if ( texGradientLengthSq_4 <= texGradientLengthSq )
            {
                v3 = sqrt(texGradientLengthSq_4);
                return 1.0 / v3;
            }
            else
            {
                v4 = sqrt(texGradientLengthSq);
                return 1.0 / v4;
            }
        }
        else
        {
            v5 = sqrt(texGradientLengthSq_4);
            return 1.0 / v5;
        }
    }
    else
    {
        v6 = sqrt(texGradientLengthSq);
        return 1.0 / v6;
    }
}

char __cdecl R_StreamGetMaterialTextureSize(
                const Material *material,
                int texIndex,
                float *texSize,
                unsigned int *filterState)
{
    const GfxImage *image; // [esp+0h] [ebp-8h]
    const MaterialTextureDef *texDef; // [esp+4h] [ebp-4h]

    if ( (texIndex < 0 || texIndex >= material->textureCount)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_stream_util.cpp",
                    220,
                    0,
                    "%s\n\t(texIndex) = %i",
                    "(texIndex >= 0 && texIndex < material->textureCount)",
                    texIndex) )
    {
        __debugbreak();
    }
    texDef = &material->textureTable[texIndex];
    if ( texDef->semantic == 11 || !texDef->u.image->streaming )
        return 0;
    if ( !texDef->u.image
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_stream_util.cpp",
                    226,
                    0,
                    "%s",
                    "texDef->u.image") )
    {
        __debugbreak();
    }
    image = texDef->u.image;
    *texSize = (float)image->width * 1.0;
    texSize[1] = (float)image->height * 1.0;
    *filterState = texDef->samplerState & 0x1F;
    return 1;
}

