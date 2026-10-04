#include "fx_postlight.h"
#include <gfx_d3d/r_drawsurf.h>
#include <xanim/xanim.h>

FxPostLightInfo g_postLightInfo;

void __cdecl FX_PostLight_GenerateVerts(FxPostLightInfo *postLightInfoAddr, FxSystem *system)
{
    int v2; // eax
    float *end; // [esp+8h] [ebp-108h]
    float v4; // [esp+Ch] [ebp-104h]
    GfxPackedVertex *v5; // [esp+10h] [ebp-100h]
    float v6; // [esp+14h] [ebp-FCh]
    float v7; // [esp+18h] [ebp-F8h]
    float v8; // [esp+2Ch] [ebp-E4h]
    float *v9; // [esp+3Ch] [ebp-D4h]
    float v10; // [esp+44h] [ebp-CCh]
    float v11; // [esp+48h] [ebp-C8h]
    float v12; // [esp+50h] [ebp-C0h]
    float v13; // [esp+54h] [ebp-BCh]
    float v14; // [esp+58h] [ebp-B8h]
    float c; // [esp+64h] [ebp-ACh]
    float s; // [esp+68h] [ebp-A8h]
    float unitOffset; // [esp+6Ch] [ebp-A4h]
    float unitOffset_4; // [esp+70h] [ebp-A0h]
    float unitOffset_8; // [esp+74h] [ebp-9Ch]
    int around; // [esp+78h] [ebp-98h]
    int arounda; // [esp+78h] [ebp-98h]
    int aroundb; // [esp+78h] [ebp-98h]
    int aroundc; // [esp+78h] [ebp-98h]
    float ortho0[3]; // [esp+7Ch] [ebp-94h] BYREF
    float radius; // [esp+88h] [ebp-88h]
    float normalizedDelta[3]; // [esp+8Ch] [ebp-84h] BYREF
    float ortho1[3]; // [esp+98h] [ebp-78h] BYREF
    r_double_index_t workingIndex; // [esp+A4h] [ebp-6Ch]
    r_double_index_t *baseIndices; // [esp+A8h] [ebp-68h] BYREF
    float (*baseArgs)[4]; // [esp+ACh] [ebp-64h]
    float posDeltaLenSq; // [esp+B0h] [ebp-60h]
    float posDelta[3]; // [esp+B4h] [ebp-5Ch] BYREF
    GfxPackedVertex *verts; // [esp+C0h] [ebp-50h]
    r_double_index_t *indices; // [esp+C4h] [ebp-4Ch]
    unsigned int argOffset; // [esp+C8h] [ebp-48h] BYREF
    FxPostLight *postLight; // [esp+CCh] [ebp-44h]
    unsigned __int16 baseVertex; // [esp+D0h] [ebp-40h] BYREF
    float (*args)[4]; // [esp+D4h] [ebp-3Ch]
    GfxPackedVertex *baseVerts; // [esp+D8h] [ebp-38h]
    int VERT_COUNT; // [esp+DCh] [ebp-34h]
    float lightOrigin[3]; // [esp+E0h] [ebp-30h]
    FxPostLightInfo *postLightInfo; // [esp+ECh] [ebp-24h]
    int postLightIter; // [esp+F0h] [ebp-20h]
    float eyeOffset[3]; // [esp+F4h] [ebp-1Ch]
    int IND_COUNT; // [esp+104h] [ebp-Ch]
    float POLYGON_RADIUS_GROW; // [esp+108h] [ebp-8h]
    int POINTS_AROUND; // [esp+10Ch] [ebp-4h]

    POINTS_AROUND = 8;
    POLYGON_RADIUS_GROW = 1.4142135f;
    VERT_COUNT = 16;
    IND_COUNT = 84;
    postLightInfo = postLightInfoAddr;
    eyeOffset[0] = system->camera.origin[0];
    eyeOffset[1] = system->camera.origin[1];
    eyeOffset[2] = system->camera.origin[2];
    for ( postLightIter = 0; postLightIter != postLightInfo->postLightCount; ++postLightIter )
    {
        postLight = &postLightInfo->postLights[postLightIter];
        if ( postLight->radius < 0.001
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_postlight.cpp",
                        100,
                        0,
                        "%s\n\t(postLight->radius) = %g",
                        "(postLight->radius >= 0.001f)",
                        postLight->radius) )
        {
            __debugbreak();
        }
        posDelta[0] = postLight->end[0] - postLight->begin[0];
        posDelta[1] = postLight->end[1] - postLight->begin[1];
        posDelta[2] = postLight->end[2] - postLight->begin[2];
        posDeltaLenSq = (float)((float)(posDelta[0] * posDelta[0]) + (float)(posDelta[1] * posDelta[1]))
                                    + (float)(posDelta[2] * posDelta[2]);
        if ( posDeltaLenSq >= 0.000099999997 )
        {
            if ( !R_ReserveCodeMeshIndices(84, &baseIndices)
                || !R_ReserveCodeMeshVerts(16, &baseVertex)
                || !R_ReserveCodeMeshArgs(2, &argOffset) )
            {
                return;
            }
            baseVerts = R_GetCodeMeshVerts(baseVertex);
            baseArgs = R_GetCodeMeshArgs(argOffset);
            lightOrigin[0] = postLight->begin[0] - eyeOffset[0];
            lightOrigin[1] = postLight->begin[1] - eyeOffset[1];
            lightOrigin[2] = postLight->begin[2] - eyeOffset[2];
            args = baseArgs;
            v12 = lightOrigin[1];
            v13 = lightOrigin[2];
            v14 = 1.0 / postLight->radius;
            (*baseArgs)[0] = lightOrigin[0];
            (*args)[1] = v12;
            (*args)[2] = v13;
            (*args)[3] = v14;
            v9 = &(*args)[4];
            v10 = posDelta[1];
            v11 = posDelta[2];
            (*args)[4] = posDelta[0];
            v9[1] = v10;
            v9[2] = v11;
            v9[3] = 1.0 / posDeltaLenSq;
            radius = postLight->radius;
            Vec3NormalizeTo(posDelta, normalizedDelta);
            PerpendicularVector(normalizedDelta, ortho0);
            Vec3Cross(normalizedDelta, ortho0, ortho1);
            verts = baseVerts;
            for ( around = 0; around != 8; ++around )
            {
                v8 = (float)((float)((float)around * 2.0) * 3.1415927) / 8.0;
                c = cos(v8);
                s = sin(v8);
                unitOffset = (float)(s * ortho0[0]) + (float)(c * ortho1[0]);
                unitOffset_4 = (float)(s * ortho0[1]) + (float)(c * ortho1[1]);
                unitOffset_8 = (float)(s * ortho0[2]) + (float)(c * ortho1[2]);
                v6 = -radius;
                v7 = radius * POLYGON_RADIUS_GROW;
                verts->xyz[0] = (float)((float)((-(radius)) * normalizedDelta[0]) + postLight->begin[0]) + (float)((float)(radius * POLYGON_RADIUS_GROW) * unitOffset);
                verts->xyz[1] = (float)((float)(v6 * normalizedDelta[1]) + postLight->begin[1]) + (float)(v7 * unitOffset_4);
                verts->xyz[2] = (float)((float)(v6 * normalizedDelta[2]) + postLight->begin[2]) + (float)(v7 * unitOffset_8);
                end = postLight->end;
                v4 = radius * POLYGON_RADIUS_GROW;
                v5 = verts + 8;
                verts[8].xyz[0] = (float)((float)(radius * normalizedDelta[0]) + postLight->end[0])
                                                + (float)((float)(radius * POLYGON_RADIUS_GROW) * unitOffset);
                v5->xyz[1] = (float)((float)(radius * normalizedDelta[1]) + end[1]) + (float)(v4 * unitOffset_4);
                v5->xyz[2] = (float)((float)(radius * normalizedDelta[2]) + end[2]) + (float)(v4 * unitOffset_8);
                verts->texCoord.packed = 0;
                verts->color.packed = postLight->color.packed;
                verts[8].texCoord.packed = 0;
                verts[8].color.packed = postLight->color.packed;
                ++verts;
            }
            indices = baseIndices;
            for ( arounda = 0; arounda != 8; ++arounda )
            {
                v2 = (arounda + 1) % 8;
                workingIndex.value[0] = arounda + baseVertex;
                workingIndex.value[1] = v2 + baseVertex;
                *indices++ = workingIndex;
                workingIndex.value[0] = baseVertex + arounda + 8;
                workingIndex.value[1] = workingIndex.value[0];
                *indices++ = workingIndex;
                workingIndex.value[0] = v2 + baseVertex;
                workingIndex.value[1] = v2 + baseVertex + 8;
                *indices++ = workingIndex;
            }
            for ( aroundb = 0; aroundb != 6; aroundb += 2 )
            {
                workingIndex.value[0] = baseVertex + aroundb + 2;
                workingIndex.value[1] = baseVertex + aroundb + 1;
                *indices++ = workingIndex;
                workingIndex.value[0] = baseVertex;
                workingIndex.value[1] = baseVertex + aroundb + 3;
                *indices++ = workingIndex;
                workingIndex.value[0] = baseVertex + aroundb + 2;
                workingIndex.value[1] = baseVertex;
                *indices++ = workingIndex;
            }
            for ( aroundc = 0; aroundc != 6; aroundc += 2 )
            {
                workingIndex.value[0] = baseVertex + 8;
                workingIndex.value[1] = baseVertex + aroundc + 9;
                *indices++ = workingIndex;
                workingIndex.value[0] = baseVertex + aroundc + 10;
                workingIndex.value[1] = baseVertex + 8;
                *indices++ = workingIndex;
                workingIndex.value[0] = baseVertex + aroundc + 10;
                workingIndex.value[1] = baseVertex + aroundc + 11;
                *indices++ = workingIndex;
            }
            R_AddCodeMeshDrawSurf(postLight->material, baseIndices, 0x54u, argOffset, 2u, "PostLight", 0xDu);
        }
    }
}

void __cdecl FX_PostLight_Begin()
{
    g_postLightInfo.postLightCount = 0;
}

void __cdecl FX_PostLight_Add(FxPostLight *postLight)
{
    if ( g_postLightInfo.postLightCount != 96 )
        memcpy(
            &g_postLightInfo.postLights[g_postLightInfo.postLightCount++],
            postLight,
            sizeof(g_postLightInfo.postLights[g_postLightInfo.postLightCount++]));
}

FxPostLightInfo *__cdecl FX_PostLight_GetInfo()
{
    return &g_postLightInfo;
}

