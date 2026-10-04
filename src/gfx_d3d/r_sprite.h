#pragma once
#include "r_material.h"

struct __declspec(align(8)) renderQuad_t // sizeof=0x20
{
    float pos[4];
    float rotation;
    float radius;
    float lifeFrac;
    // padding byte
    // padding byte
    // padding byte
    // padding byte
};

void __cdecl R_BuildQuadStampCodeMeshVerts(
                Material *material,
                const float (*viewAxis)[3],
                const float *origin,
                const float *left,
                const float *up,
                const unsigned __int8 *rgbaColor,
                int s0,
                int t0,
                int s1,
                int t1);
void __cdecl R_GenerateQuadStampCodeMeshVerts(
                Material *material,
                const float *viewAngles,
                const float *origin,
                float worldRadius,
                const unsigned __int8 *rgbaColor,
                float frameFrac);
void __cdecl R_GenerateQuadStampCodeMeshVertsArray(
                Material *material,
                renderQuad_t *quads,
                int numQuads,
                float (*viewAxis)[3]);
void __cdecl really_fast_sincos(float in, float *s, float *c);
double __cdecl fast_round(double x);
unsigned int __cdecl PackTwoShorts(unsigned int first, unsigned int second);
