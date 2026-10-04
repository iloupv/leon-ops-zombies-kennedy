#include "rb_debug.h"
#include "rb_shade.h"
#include "r_state_utils.h"
#include "rb_stats.h"
#include <physics/phys_render.h>
#include "r_debug.h"

bool s_addingDebugLines;
bool s_lastDepthTest;
void(__cdecl *s_DebugBrushesAndPatchesCallback)();

GfxPointVertex g_debugLineVerts[2725];
GfxPointVertex g_debugPolyVerts[2725];
GfxPointVertex g_debugExternLineVerts[2725];
GfxPointVertex *s_gfxverts;
int s_lineVertCount;

int __cdecl RB_AddDebugLine(
                const float *start,
                const float *end,
                const float *color,
                bool depthTest,
                int vertCount,
                int vertLimit,
                GfxPointVertex *verts)
{
    GfxPointVertex *v8; // [esp+4h] [ebp-8h]
    GfxPointVertex *v9; // [esp+8h] [ebp-4h]

    if ( vertCount + 2 > vertLimit || s_addingDebugLines && s_lastDepthTest != depthTest )
    {
        RB_DrawLines3D(vertCount / 2, 1, verts, s_lastDepthTest);
        vertCount = 0;
    }
    s_lastDepthTest = depthTest;
    s_addingDebugLines = 1;
    R_ConvertColorToBytes(color, verts[vertCount].color);
    *(unsigned int *)verts[vertCount + 1].color = *(unsigned int *)verts[vertCount].color;
    v9 = &verts[vertCount];
    v9->xyz[0] = *start;
    v9->xyz[1] = start[1];
    v9->xyz[2] = start[2];
    v8 = &verts[vertCount + 1];
    v8->xyz[0] = *end;
    v8->xyz[1] = end[1];
    v8->xyz[2] = end[2];
    return vertCount + 2;
}

int __cdecl RB_EndDebugLines(int vertCount, const GfxPointVertex *verts)
{
    if ( vertCount )
    {
        RB_DrawLines3D(vertCount, 1, verts, s_lastDepthTest);
        s_addingDebugLines = 0;
    }
    return 0;
}

void __cdecl RB_SetPolyVert(float *xyz, GfxColor color, int tessVertIndex)
{
    float *xyzw; // [esp+4h] [ebp-4h]

    xyzw = tess.verts[tessVertIndex].xyzw;
    *xyzw = *xyz;
    xyzw[1] = xyz[1];
    xyzw[2] = xyz[2];
    *(float *)&tess.indices[16 * tessVertIndex - 87194] = 1.0f;
    *(unsigned int *)&tess.indices[16 * tessVertIndex - 87186] = 1073643391;
    *(GfxColor *)&tess.indices[16 * tessVertIndex - 87192] = color;
    *(unsigned int *)&tess.indices[16 * tessVertIndex - 87190] = 0;
    *(unsigned int *)&tess.indices[16 * tessVertIndex - 87188] = 0;
}

void __cdecl RB_SetDebugBrushesAndPatchesCallback(void (__cdecl *callback)())
{
    s_DebugBrushesAndPatchesCallback = callback;
}

void __cdecl RB_DrawDebug(const GfxViewParms *viewParms)
{
    if ( !viewParms
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_debug.cpp", 501, 0, "%s", "viewParms") )
    {
        __debugbreak();
    }
    RB_AddPlumeStrings(viewParms);
    RB_DrawDebugBrushesAndPatches();
    RB_DrawDebugPolys();
    RB_DrawDebugSpheres(backEndData->debugGlobals.spheres, backEndData->debugGlobals.sphereCount);
    RB_DrawDebugSpheres(backEndData->debugGlobals.externSpheres, backEndData->debugGlobals.externSphereCount);
    backEndData->debugGlobals.sphereCount = 0;
    RB_DrawDebugLines(backEndData->debugGlobals.lines, backEndData->debugGlobals.lineCount, g_debugLineVerts);
    RB_DrawDebugLines(
        backEndData->debugGlobals.externLines,
        backEndData->debugGlobals.externLineCount,
        g_debugExternLineVerts);
    backEndData->debugGlobals.lineCount = 0;
    RB_DrawDebugStrings(backEndData->debugGlobals.strings, backEndData->debugGlobals.stringCount);
    RB_DrawDebugStrings(backEndData->debugGlobals.externStrings, backEndData->debugGlobals.externStringCount);
    if ( tess.indexCount )
        RB_EndTessSurface();
}

const GfxBackEndData *RB_DrawDebugPolys()
{
  const GfxBackEndData *result; // eax

  result = backEndData;
  if ( backEndData->debugGlobals.polySet.polyCount )
  {
    if ( tess.indexCount )
      RB_EndTessSurface();
    R_Set3D(&gfxCmdBufSourceState);
    RB_DrawPolyInteriors(&backEndData->debugGlobals.polySet);
    RB_DrawPolyOutlines(&backEndData->debugGlobals.polySet);
    result = backEndData;
    backEndData->debugGlobals.polySet.polyCount = 0;
  }
  return result;
}

void __cdecl RB_DrawPolyInteriors(PolySet *polySet)
{
    const Material *material; // [esp+4h] [ebp-1Ch]
    GfxDebugPoly *poly; // [esp+8h] [ebp-18h]
    float *verts; // [esp+Ch] [ebp-14h]
    int polyIndex; // [esp+10h] [ebp-10h]
    int vertIndex; // [esp+18h] [ebp-8h]
    int vertIndexa; // [esp+18h] [ebp-8h]
    GfxColor color; // [esp+1Ch] [ebp-4h] BYREF

    if ( polySet->polyCount <= 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_debug.cpp",
                    103,
                    0,
                    "%s",
                    "polySet->polyCount > 0") )
    {
        __debugbreak();
    }
    for ( polyIndex = 0; polyIndex < polySet->polyCount; ++polyIndex )
    {
        poly = &polySet->polys[polyIndex];
        verts = polySet->verts[poly->firstVert];
        material = RB_DebugPolyGetMaterialByDepthTest(poly->faceDepthTest, poly->faceColor[3] < 1.0);
        RB_BeginSurface(material, 4u);
        R_TrackPrims(&gfxCmdBufState.prim, GFX_PRIM_STATS_DEBUG);
        R_ConvertColorToBytes(poly->faceColor, (unsigned __int8 *)&color);
        RB_CheckTessOverflow(poly->vertCount, 3 * (poly->vertCount - 2));
        for ( vertIndex = 0; vertIndex < poly->vertCount; ++vertIndex )
            RB_SetPolyVert(&verts[3 * vertIndex], color, vertIndex + tess.vertexCount);
        for ( vertIndexa = 2; vertIndexa < poly->vertCount; ++vertIndexa )
        {
            tess.indices[tess.indexCount] = tess.vertexCount;
            tess.indices[tess.indexCount + 1] = vertIndexa + LOWORD(tess.vertexCount);
            tess.indices[tess.indexCount + 2] = LOWORD(tess.vertexCount) + vertIndexa - 1;
            tess.indexCount += 3;
        }
        tess.vertexCount += poly->vertCount;
        RB_EndTessSurface();
    }
}

const Material *__cdecl RB_DebugPolyGetMaterialByDepthTest(bool depthTest, bool blend)
{
    if ( depthTest && blend )
        return rgp.blendMaterial;
    if ( depthTest && !blend )
        return rgp.lineMaterial;
    if ( depthTest || !blend )
        return rgp.lineMaterialNoDepth;
    return rgp.blendMaterialNoDepth;
}

void __cdecl RB_DrawPolyOutlines(PolySet *polySet)
{
    GfxDebugPoly *poly; // [esp+0h] [ebp-1Ch]
    float *polyVerts; // [esp+4h] [ebp-18h]
    int polyIndex; // [esp+Ch] [ebp-10h]
    int vertCount; // [esp+10h] [ebp-Ch]
    int vertIndex; // [esp+14h] [ebp-8h]
    int vertIndexPrev; // [esp+18h] [ebp-4h]

    if ( polySet->polyCount <= 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_debug.cpp",
                    144,
                    0,
                    "%s",
                    "polySet->polyCount > 0") )
    {
        __debugbreak();
    }
    vertCount = 0;
    for ( polyIndex = 0; polyIndex < polySet->polyCount; ++polyIndex )
    {
        poly = &polySet->polys[polyIndex];
        polyVerts = polySet->verts[poly->firstVert];
        vertIndexPrev = poly->vertCount - 1;
        for ( vertIndex = 0; vertIndex < poly->vertCount; ++vertIndex )
        {
            vertCount = RB_AddDebugLine(
                                        &polyVerts[3 * vertIndexPrev],
                                        &polyVerts[3 * vertIndex],
                                        poly->edgeColor,
                                        poly->edgeDepthTest,
                                        vertCount,
                                        2725,
                                        g_debugPolyVerts);
            vertIndexPrev = vertIndex;
        }
    }
    RB_EndDebugLines(vertCount / 2, g_debugPolyVerts);
}

void __cdecl RB_DrawDebugSpheres(trDebugSphere_t *spheres, int sphereCount)
{
    int j; // [esp+10h] [ebp-51Ch]
    float dist; // [esp+14h] [ebp-518h]
    int sortedList[256]; // [esp+1Ch] [ebp-510h]
    int i; // [esp+41Ch] [ebp-110h]
    float furthest; // [esp+420h] [ebp-10Ch]
    unsigned __int8 usedList[260]; // [esp+424h] [ebp-108h] BYREF

    if ( sphereCount > 256
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_debug.cpp",
                    365,
                    0,
                    "%s",
                    "sphereCount <= MAX_SORTED_SPHERES") )
    {
        __debugbreak();
    }
    memset(usedList, 0, 0x100u);
    for ( j = 0; j < sphereCount; ++j )
    {
        furthest = -1.0f;
        for ( i = 0; i < sphereCount; ++i )
        {
            if ( !usedList[i] )
            {
                dist = Vec3Distance(gfxCmdBufSourceState.viewParms.origin, spheres[i].center);
                if ( dist > furthest )
                {
                    furthest = dist;
                    sortedList[j] = i;
                }
            }
        }
        usedList[sortedList[j]] = 1;
    }
    for ( i = 0; i < sphereCount; ++i )
        RB_DrawDebugSphere(&spheres[sortedList[i]]);
}

unsigned __int8 tech = 4;

void __cdecl RB_DrawDebugSphere(trDebugSphere_t *sphere)
{
    float *v1; // eax
    float *v2; // [esp+0h] [ebp-6580h]
    float *v3; // [esp+4h] [ebp-657Ch]
    int v4; // [esp+Ch] [ebp-6574h]
    float *xyzw; // [esp+28h] [ebp-6558h]
    float *v6; // [esp+2Ch] [ebp-6554h]
    unsigned int *v7; // [esp+30h] [ebp-6550h]
    float *v8; // [esp+3Ch] [ebp-6544h]
    float *v9; // [esp+40h] [ebp-6540h]
    _BYTE *v10; // [esp+44h] [ebp-653Ch]
    float *v11; // [esp+48h] [ebp-6538h]
    float *v12; // [esp+54h] [ebp-652Ch]
    float *v13; // [esp+58h] [ebp-6528h]
    _BYTE *v14; // [esp+5Ch] [ebp-6524h]
    float *v15; // [esp+60h] [ebp-6520h]
    float *v16; // [esp+6Ch] [ebp-6514h]
    float *v17; // [esp+70h] [ebp-6510h]
    float *v18; // [esp+74h] [ebp-650Ch]
    float v19; // [esp+7Ch] [ebp-6504h]
    int v20; // [esp+80h] [ebp-6500h]
    float v21; // [esp+90h] [ebp-64F0h]
    float v22[33]; // [esp+A0h] [ebp-64E0h] BYREF
    int sideCount; // [esp+124h] [ebp-645Ch]
    _BYTE v24[13500]; // [esp+128h] [ebp-6458h] BYREF
    float v25; // [esp+35E4h] [ebp-2F9Ch]
    Material *material; // [esp+35E8h] [ebp-2F98h]
    int i; // [esp+35ECh] [ebp-2F94h]
    float v28; // [esp+35F0h] [ebp-2F90h]
    unsigned __int8 colorBytes[7]; // [esp+35F4h] [ebp-2F8Ch] BYREF
    bool v30; // [esp+35FBh] [ebp-2F85h]
    float radius; // [esp+35FCh] [ebp-2F84h]
    float colorFloat; // [esp+3600h] [ebp-2F80h] BYREF
    float v33; // [esp+3604h] [ebp-2F7Ch]
    float v34; // [esp+3608h] [ebp-2F78h]
    float v35; // [esp+360Ch] [ebp-2F74h]
    _QWORD v36[482]; // [esp+3610h] [ebp-2F70h] BYREF
    int indexCount; // [esp+4524h] [ebp-205Ch]
    int j; // [esp+4528h] [ebp-2058h]
    int v39; // [esp+452Ch] [ebp-2054h]
    unsigned __int8 src[2]; // [esp+4530h] [ebp-2050h] BYREF
    _WORD v41[3071]; // [esp+4532h] [ebp-204Eh]
    float v42; // [esp+5D30h] [ebp-850h]
    int v43; // [esp+5D34h] [ebp-84Ch]
    int v44; // [esp+5D38h] [ebp-848h]
    int v45; // [esp+5D3Ch] [ebp-844h]
    int v46; // [esp+5D40h] [ebp-840h]
    int v47; // [esp+5D44h] [ebp-83Ch]
    float v48[33]; // [esp+5D48h] [ebp-838h]
    int vertexCount; // [esp+5DCCh] [ebp-7B4h]
    unsigned int v50[482]; // [esp+5DD0h] [ebp-7B0h]
    float v51; // [esp+655Ch] [ebp-24h]
    float v52; // [esp+6560h] [ebp-20h]
    float v53; // [esp+6564h] [ebp-1Ch]
    int m; // [esp+6568h] [ebp-18h]
    int k; // [esp+656Ch] [ebp-14h]
    float v56; // [esp+6570h] [ebp-10h]
    float v57; // [esp+6574h] [ebp-Ch]
    float v58; // [esp+6578h] [ebp-8h]
    float v59; // [esp+657Ch] [ebp-4h]

    v39 = 482;
    v51 = 0.57700002f;
    v52 = 0.57700002f;
    v53 = 0.57700002f;
    v42 = 0.0f;
    v25 = Vec3DistanceSq(gfxCmdBufSourceState.viewParms.origin, sphere->center);
    v30 = (float)(sphere->radius * sphere->radius) > v25;
    v57 = sphere->center[0];
    v58 = sphere->center[1];
    v59 = sphere->center[2];
    radius = sphere->radius;
    sideCount = sphere->sideCount;
    if ( radius > 0.001 && sideCount >= 2 )
    {
        if ( sideCount > 32 )
            sideCount = 32;
        sideCount &= ~1u;
        v56 = 1.0 / (float)sideCount;
        v28 = 6.2831855 * v56;
        for ( i = 0; i < sideCount; ++i )
        {
            v21 = (float)i * v28;
            v1 = &v22[i];
            v48[i] = cos(v21);
            *v1 = sin(v21);
        }
        colorFloat = sphere->color[0];
        v33 = sphere->color[1];
        v34 = sphere->color[2];
        v35 = sphere->color[3];
        vertexCount = 0;
        for ( j = 1; j < sideCount / 2; ++j )
        {
            for ( k = 0; k < sideCount; ++k )
            {
                if ( vertexCount != k + sideCount * (j - 1)
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_debug.cpp",
                                243,
                                1,
                                "%s",
                                "vertCount == (row - 1) * sideCount + col") )
                {
                    __debugbreak();
                }
                v18 = (float *)&v24[12 * vertexCount + 7712];
                v19 = v22[j] * v22[k];
                v20 = LODWORD(v48[j]);
                *v18 = v22[j] * v48[k];
                v18[1] = v19;
                *((unsigned int *)v18 + 2) = v20;
                v16 = (float *)&v24[16 * vertexCount];
                v17 = (float *)&v24[12 * vertexCount + 7712];
                *v16 = (float)(radius * *v17) + v57;
                v16[1] = (float)(radius * v17[1]) + v58;
                v16[2] = (float)(radius * v17[2]) + v59;
                v42 = (float)((float)((float)((float)(*(float *)&v24[12 * vertexCount + 7712] * v51)
                                                                        + (float)(*(float *)&v24[12 * vertexCount + 7716] * v52))
                                                        + (float)(*(float *)&v24[12 * vertexCount + 7720] * v53))
                                        + 1.0)
                        * 0.5;
                colorFloat = v42 * sphere->color[0];
                v33 = v42 * sphere->color[1];
                v34 = v42 * sphere->color[2];
                R_ConvertColorToBytes(&colorFloat, colorBytes);
                *(float *)&v24[16 * vertexCount + 12] = 1.0f;
                v50[vertexCount] = *(unsigned int *)colorBytes;
                v15 = (float *)&v36[vertexCount];
                *v15 = (float)j * v56;
                v15[1] = (float)k * v56;
                ++vertexCount;
            }
        }
        if ( vertexCount != sideCount * (sideCount / 2 - 1)
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_debug.cpp",
                        256,
                        1,
                        "%s",
                        "vertCount == sideCount * (sideCount / 2 - 1)") )
        {
            __debugbreak();
        }
        v14 = &v24[12 * vertexCount + 7712];
        *(unsigned int *)v14 = 0;
        *((unsigned int *)v14 + 1) = 0;
        *((float *)v14 + 2) = 1.0f;
        v12 = (float *)&v24[16 * vertexCount];
        v13 = (float *)&v24[12 * vertexCount + 7712];
        *v12 = (float)(radius * *v13) + v57;
        v12[1] = (float)(radius * v13[1]) + v58;
        v12[2] = (float)(radius * v13[2]) + v59;
        *(float *)&v24[16 * vertexCount + 12] = 1.0f;
        v42 = (float)((float)((float)((float)(*(float *)&v24[12 * vertexCount + 7712] * v51)
                                                                + (float)(*(float *)&v24[12 * vertexCount + 7716] * v52))
                                                + (float)(*(float *)&v24[12 * vertexCount + 7720] * v53))
                                + 1.0)
                * 0.5;
        colorFloat = v42 * sphere->color[0];
        v33 = v42 * sphere->color[1];
        v34 = v42 * sphere->color[2];
        R_ConvertColorToBytes(&colorFloat, colorBytes);
        v50[vertexCount] = *(unsigned int *)colorBytes;
        v11 = (float *)&v36[vertexCount];
        *v11 = 0.0f;
        v11[1] = 1.0f;
        ++vertexCount;
        v10 = &v24[12 * vertexCount + 7712];
        *(unsigned int *)v10 = 0;
        *((unsigned int *)v10 + 1) = 0;
        *((float *)v10 + 2) = -1.0f;
        v8 = (float *)&v24[16 * vertexCount];
        v9 = (float *)&v24[12 * vertexCount + 7712];
        *v8 = (float)(radius * *v9) + v57;
        v8[1] = (float)(radius * v9[1]) + v58;
        v8[2] = (float)(radius * v9[2]) + v59;
        *(float *)&v24[16 * vertexCount + 12] = 1.0f;
        v42 = (float)((float)((float)((float)(*(float *)&v24[12 * vertexCount + 7712] * v51)
                                                                + (float)(*(float *)&v24[12 * vertexCount + 7716] * v52))
                                                + (float)(*(float *)&v24[12 * vertexCount + 7720] * v53))
                                + 1.0)
                * 0.5;
        colorFloat = v42 * sphere->color[0];
        v33 = v42 * sphere->color[1];
        v34 = v42 * sphere->color[2];
        R_ConvertColorToBytes(&colorFloat, colorBytes);
        v50[vertexCount] = *(unsigned int *)colorBytes;
        v7 = (unsigned int*)&v36[vertexCount];
        *v7 = 0;
        v7[1] = 0;
        ++vertexCount;
        indexCount = 0;
        for ( j = 1; j < sideCount / 2 - 1; ++j )
        {
            for ( k = 0; k < sideCount; ++k )
            {
                v44 = k + sideCount * (j - 1);
                v45 = v44 + 1;
                v46 = sideCount + v44;
                v47 = v44 + sideCount + 1;
                if ( k == sideCount - 1 )
                {
                    v45 -= sideCount;
                    v47 -= sideCount;
                }
                *(_WORD *)&src[2 * indexCount] = v44;
                v41[indexCount] = v45;
                v41[indexCount + 1] = v46;
                v41[indexCount + 2] = v46;
                v41[indexCount + 3] = v45;
                v41[indexCount + 4] = v47;
                indexCount += 6;
            }
        }
        for ( k = 0; k < sideCount; ++k )
        {
            *(_WORD *)&src[2 * indexCount] = vertexCount - 2;
            v41[indexCount] = k + 1;
            v41[indexCount + 1] = k;
            v41[indexCount + 2] = vertexCount - 1;
            v41[indexCount + 3] = k + sideCount * (sideCount / 2 - 2);
            v41[indexCount + 4] = k + sideCount * (sideCount / 2 - 2) + 1;
            if ( k == sideCount - 1 )
            {
                v41[indexCount] -= sideCount;
                v41[indexCount + 4] -= sideCount;
            }
            indexCount += 6;
        }
        material = rgp.whiteDebugExteriorMaterial;
        if ( (float)(sphere->radius * sphere->radius) > v25 )
            material = rgp.whiteDepthInteriorMaterial;
        RB_BeginSurface(material, tech);
        R_TrackPrims(&gfxCmdBufState.prim, GFX_PRIM_STATS_DEBUG);
        RB_CheckTessOverflow(vertexCount, indexCount);
        for ( m = 0; m < vertexCount; ++m )
        {
            v43 = m + tess.vertexCount;
            xyzw = tess.verts[m + tess.vertexCount].xyzw;
            v6 = (float *)&v24[16 * m];
            *xyzw = *v6;
            xyzw[1] = v6[1];
            xyzw[2] = v6[2];
            xyzw[3] = v6[3];
            LOBYTE(v4) = (int)(float)((float)(*(float *)&v24[12 * m + 7712] * 127.0) + 127.5);
            BYTE1(v4) = (int)(float)((float)(*(float *)&v24[12 * m + 7716] * 127.0) + 127.5);
            BYTE2(v4) = (int)(float)((float)(*(float *)&v24[12 * m + 7720] * 127.0) + 127.5);
            HIBYTE(v4) = 63;
            *(unsigned int *)&tess.indices[16 * v43 - 87186] = v4;
            *(unsigned int *)&tess.indices[16 * v43 - 87192] = v50[m];
            v2 = (float *)&tess.indices[16 * v43 - 87190];
            v3 = (float *)&v36[m];
            *v2 = *v3;
            v2[1] = v3[1];
        }
        tess.vertexCount += vertexCount;
        memcpy((unsigned __int8 *)&tess.indices[tess.indexCount], src, 2 * indexCount);
        tess.indexCount += indexCount;
        RB_EndTessSurface();
    }
}

void __cdecl RB_DrawDebugLines(trDebugLine_t *lines, int lineCount, GfxPointVertex *verts)
{
  char lineDepthTest; // [esp+3h] [ebp-11h]
  int vertCount; // [esp+4h] [ebp-10h]
  int lineIndex; // [esp+Ch] [ebp-8h]
  char depthTest; // [esp+13h] [ebp-1h]

  if ( lineCount )
  {
    if ( tess.indexCount )
      RB_EndTessSurface();
    R_Set3D(&gfxCmdBufSourceState);
    depthTest = lines->depthTest != 0;
    vertCount = 0;
    for ( lineIndex = 0; lineIndex < lineCount; ++lineIndex )
    {
      lineDepthTest = lines[lineIndex].depthTest != 0;
      if ( depthTest != lineDepthTest )
      {
        vertCount = RB_EndDebugLines(vertCount / 2, verts);
        depthTest = lineDepthTest;
      }
      vertCount = RB_AddDebugLine(
                    lines[lineIndex].start,
                    lines[lineIndex].end,
                    lines[lineIndex].color,
                    depthTest,
                    vertCount,
                    2725,
                    verts);
    }
    RB_EndDebugLines(vertCount / 2, verts);
  }
}

void __cdecl RB_DrawDebugStrings(trDebugString_t *strings, int stringCount)
{
  float v2; // [esp+0h] [ebp-30h]
  float v3; // [esp+8h] [ebp-28h]
  float xStep[3]; // [esp+10h] [ebp-20h] BYREF
  float yStep[3]; // [esp+1Ch] [ebp-14h] BYREF
  int stringIndex; // [esp+28h] [ebp-8h]
  GfxColor color; // [esp+2Ch] [ebp-4h] BYREF

  if ( stringCount )
  {
    if ( tess.indexCount )
      RB_EndTessSurface();
    R_Set3D(&gfxCmdBufSourceState);
    for ( stringIndex = 0; stringIndex < stringCount; ++stringIndex )
    {
      R_ConvertColorToBytes(strings[stringIndex].color, (unsigned __int8 *)&color);
      //LODWORD(v3) = LODWORD(strings[stringIndex].scale) ^ _mask__NegFloat_;
      v3 = -(strings[stringIndex].scale);
      xStep[0] = v3 * gfxCmdBufSourceState.viewParms.axis[1][0];
      xStep[1] = v3 * gfxCmdBufSourceState.viewParms.axis[1][1];
      xStep[2] = v3 * gfxCmdBufSourceState.viewParms.axis[1][2];
      //LODWORD(v2) = LODWORD(strings[stringIndex].scale) ^ _mask__NegFloat_;
      v2= -(strings[stringIndex].scale);
      yStep[0] = v2 * gfxCmdBufSourceState.viewParms.axis[2][0];
      yStep[1] = v2 * gfxCmdBufSourceState.viewParms.axis[2][1];
      yStep[2] = v2 * gfxCmdBufSourceState.viewParms.axis[2][2];
      RB_DrawTextInSpace(
        strings[stringIndex].text,
        backEnd.debugFont,
        strings[stringIndex].xyz,
        xStep,
        yStep,
        color.packed);
    }
    if ( tess.indexCount )
      RB_EndTessSurface();
  }
}

void __cdecl RB_AddPlumeStrings(const GfxViewParms *viewParms)
{
    float v2; // xmm0_4
    char *v3; // eax
    GfxDebugPlume *v4; // [esp+Ch] [ebp-1Ch]
    int dt; // [esp+10h] [ebp-18h]
    float org[3]; // [esp+14h] [ebp-14h] BYREF
    int plumeIndex; // [esp+20h] [ebp-8h]
    float wiggle; // [esp+24h] [ebp-4h]

    for (plumeIndex = 0; plumeIndex < backEndData->debugGlobals.plumeCount; ++plumeIndex)
    {
        dt = gfxCmdBufSourceState.sceneDef.time - backEndData->debugGlobals.plumes[plumeIndex].startTime;
        if (dt >= 0 && dt <= backEndData->debugGlobals.plumes[plumeIndex].duration)
        {
            backEndData->debugGlobals.plumes[plumeIndex].color[3] = 1.0f;
            if (2 * dt > backEndData->debugGlobals.plumes[plumeIndex].duration)
                backEndData->debugGlobals.plumes[plumeIndex].color[3] = 2.0
                - (float)((float)((float)dt * 2.0)
                    / (float)backEndData->debugGlobals.plumes[plumeIndex].duration);
            //v2 = __libm_sse2_sin((float)((float)((float)dt * 0.012566371) + (float)plumeIndex));
            v2 = sin((float)((float)((float)dt * 0.012566371) + (float)plumeIndex));
            wiggle = v2 * 4.0;
            v4 = &backEndData->debugGlobals.plumes[plumeIndex];
            org[0] = (float)((float)(v2 * 4.0) * viewParms->axis[1][0]) + v4->origin[0];
            org[1] = (float)((float)(v2 * 4.0) * viewParms->axis[1][1]) + v4->origin[1];
            org[2] = (float)((float)(v2 * 4.0) * viewParms->axis[1][2]) + v4->origin[2];
            org[2] = (float)((float)dt * 0.064000003) + org[2];
            v3 = va("%i", backEndData->debugGlobals.plumes[plumeIndex].score);
            R_AddDebugString(&backEndData->debugGlobals, org, backEndData->debugGlobals.plumes[plumeIndex].color, 0.5, v3);
        }
    }
}

void RB_DrawDebugBrushesAndPatches()
{
  if ( tess.indexCount )
    RB_EndTessSurface();
  R_Set3D(&gfxCmdBufSourceState);
  if ( !s_DebugBrushesAndPatchesCallback
    && !Assert_MyHandler(
          "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_debug.cpp",
          495,
          0,
          "%s",
          "s_DebugBrushesAndPatchesCallback") )
  {
    __debugbreak();
  }
  s_DebugBrushesAndPatchesCallback();
}

void __cdecl RB_ApplySunLight(const float (*verts)[3], const float *color, float *out_color)
{
    float normal[3]; // [esp+2Ch] [ebp-44h] BYREF
    float triPos[3]; // [esp+38h] [ebp-38h]
    float intensity; // [esp+44h] [ebp-2Ch]
    float minInt; // [esp+48h] [ebp-28h]
    float v01[3]; // [esp+4Ch] [ebp-24h] BYREF
    float v02[3]; // [esp+58h] [ebp-18h] BYREF
    float lightDirUnit[3]; // [esp+64h] [ebp-Ch] BYREF

    *out_color = *color;
    out_color[1] = color[1];
    out_color[2] = color[2];
    out_color[3] = color[3];
    v01[0] = (float)(*verts)[3] - (*verts)[0];
    v01[1] = (float)(*verts)[4] - (float)(*verts)[1];
    v01[2] = (float)(*verts)[5] - (float)(*verts)[2];
    v02[0] = (float)(*verts)[6] - (*verts)[0];
    v02[1] = (float)(*verts)[7] - (float)(*verts)[1];
    v02[2] = (float)(*verts)[8] - (float)(*verts)[2];
    Vec3Cross(v01, v02, normal);
    Vec3Normalize(normal);
    triPos[0] = (*verts)[0] + (float)(*verts)[3];
    triPos[1] = (float)(*verts)[1] + (float)(*verts)[4];
    triPos[2] = (float)(*verts)[2] + (float)(*verts)[5];
    triPos[0] = triPos[0] + (float)(*verts)[6];
    triPos[1] = triPos[1] + (float)(*verts)[7];
    triPos[2] = triPos[2] + (float)(*verts)[8];
    triPos[0] = 0.33333334 * triPos[0];
    triPos[1] = 0.33333334 * triPos[1];
    triPos[2] = 0.33333334 * triPos[2];
    lightDirUnit[0] = frontEndDataOut->sunLight.dir[0];
    lightDirUnit[1] = frontEndDataOut->sunLight.dir[1];
    lightDirUnit[2] = frontEndDataOut->sunLight.dir[2];
    Vec3Normalize(lightDirUnit);
    minInt = 0.2f;
    intensity = (float)((float)(normal[0] * lightDirUnit[0]) + (float)(normal[1] * lightDirUnit[1]))
                        + (float)(normal[2] * lightDirUnit[2]);
    if ( intensity < 0.2 )
        intensity = minInt;
    intensity = (float)((float)(intensity - 1.0) * 0.75) + 1.0;
    *out_color = intensity * *out_color;
    out_color[1] = intensity * out_color[1];
    out_color[2] = intensity * out_color[2];
}

unsigned int _S1_15;
const Material *material;
void __cdecl RB_BeginCollisionPolygons(bool faceDepthTest, bool faceBlend)
{
    if ( (_S1_15 & 1) == 0 )
    {
        _S1_15 |= 1u;
        material = RB_DebugPolyGetMaterialByDepthTest(faceDepthTest, faceBlend);
    }
    RB_BeginSurface(material, 4u);
    R_TrackPrims(&gfxCmdBufState.prim, GFX_PRIM_STATS_DEBUG);
}

void __cdecl RB_DrawCollisionPolygon(
                int vertCount,
                const float (*verts)[3],
                const float *faceColor,
                bool faceDepthTest,
                int debug_partition)
{
    float litColor[3]; // [esp+4h] [ebp-14h] BYREF
    int vertIndex; // [esp+10h] [ebp-8h]
    GfxColor color; // [esp+14h] [ebp-4h] BYREF

    RB_CheckTessOverflow(vertCount, 3 * (vertCount - 2));
    if ( vertCount != 3 || debug_partition )
    {
        litColor[0] = *faceColor;
        litColor[1] = faceColor[1];
        litColor[2] = faceColor[2];
    }
    else
    {
        RB_ApplySunLight(verts, faceColor, litColor);
    }
    R_ConvertColorToBytes(litColor, (unsigned __int8 *)&color);
    for ( vertIndex = 0; vertIndex < vertCount; ++vertIndex )
        RB_SetPolyVert((float *)&(*verts)[3 * vertIndex], color, vertIndex + tess.vertexCount);
    for ( vertIndex = 2; vertIndex < vertCount; ++vertIndex )
    {
        tess.indices[tess.indexCount] = tess.vertexCount;
        tess.indices[tess.indexCount + 1] = vertIndex + LOWORD(tess.vertexCount);
        tess.indices[tess.indexCount + 2] = LOWORD(tess.vertexCount) + vertIndex - 1;
        tess.indexCount += 3;
    }
    tess.vertexCount += vertCount;
}

materialCommands_t *__cdecl RB_GetCmd()
{
    return &tess;
}

void __cdecl RB_EndCollisionPolygons()
{
    RB_EndTessSurface();
}

