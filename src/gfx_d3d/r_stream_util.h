#pragma once
#include "r_material.h"

double __cdecl R_CalculateTriangleArea2D(const float (*texCoord)[2]);
double __cdecl R_CalculateTriangleArea3D(const float (*texCoord)[3]);
char __cdecl R_CalculateTriangleTopMipAabb(
                const float (*pos)[3],
                const float (*texCoord)[2],
                const float *textureSize,
                char filterState,
                float *outMins,
                float *outMaxs,
                float *outRadius);
void __cdecl R_CalculateMinimalAabbForSpheres(
                const float (*sphereCenters)[3],
                const float *sphereRadii,
                int sphereCount,
                float *outMins,
                float *outMaxs);
char __cdecl R_CalculateTriangleTextureGradient(
                const float (*pos)[3],
                const float (*texCoord)[2],
                float (*outTexGradient)[3],
                float *outNormal);
double __cdecl R_CalculateTexelDensityFromGradient(const float (*texGradient)[3], const float *textureSize);
char __cdecl R_StreamGetMaterialTextureSize(
                const Material *material,
                int texIndex,
                float *texSize,
                unsigned int *filterState);
