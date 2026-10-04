#include "rb_showcollision.h"
#include "r_dvars.h"
#include <qcommon/cm_showcollision.h>
#include "rb_backend.h"
#include "rb_shade.h"
#include "rb_stats.h"
#include <physics/phys_render.h>
#include "rb_debug.h"

int showCollisionContentMasks[10] = { 0, 210624, 65536, 8192, 128, 512, 131072, 1024, 64, 4096 };

const float frustumSidePlanes[5][4] =
{
  { -1.0, 0.0, 0.0, 1.0 },
  { 1.0, 0.0, 0.0, 1.0 },
  { 0.0, -1.0, 0.0, 1.0 },
  { 0.0, 1.0, 0.0, 1.0 },
  { 0.0, 0.0, 1.0, 1.0 }
};

GfxPointVertex debugLineVerts[2725];

void __cdecl RB_ShowCollision(const GfxViewParms *viewParms)
{
    char v1; // [esp+0h] [ebp-98h]
    char v2; // [esp+4h] [ebp-94h]
    unsigned __int8 v3; // [esp+8h] [ebp-90h]
    int contentMask; // [esp+10h] [ebp-88h]
    cplane_s frustumPlanes[6]; // [esp+18h] [ebp-80h] BYREF

    if ( !viewParms
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_showcollision.cpp", 135, 0, "%s", "viewParms") )
    {
        __debugbreak();
    }
    if ( r_showCollision->current.integer )
    {
        contentMask = showCollisionContentMasks[r_showCollision->current.integer];
        BuildFrustumPlanes(viewParms, frustumPlanes);
        frustumPlanes[5].normal[0] = -1.0 * frustumPlanes[4].normal[0];
        frustumPlanes[5].normal[1] = -1.0 * frustumPlanes[4].normal[1];
        frustumPlanes[5].normal[2] = -1.0 * frustumPlanes[4].normal[2];
        frustumPlanes[5].dist = (-(frustumPlanes[4].dist)) - r_showCollisionDist->current.value;
        if ( (float)(-1.0 * frustumPlanes[4].normal[0]) == 1.0 )
        {
            v3 = 0;
        }
        else
        {
            if ( frustumPlanes[5].normal[1] == 1.0 )
            {
                v2 = 1;
            }
            else
            {
                if ( frustumPlanes[5].normal[2] == 1.0 )
                    v1 = 2;
                else
                    v1 = 3;
                v2 = v1;
            }
            v3 = v2;
        }
        frustumPlanes[5].type = v3;
        SetPlaneSignbits(&frustumPlanes[5]);
        if ( r_showCollisionGroups->current.integer <= 1u )
            CM_ShowBrushCollision(contentMask, frustumPlanes, 6, RB_DrawCollisionPoly);
        if ( tess.indexCount )
            RB_EndTessSurface();
    }
}

void __cdecl BuildFrustumPlanes(const GfxViewParms *viewParms, cplane_s *frustumPlanes)
{
    char v2; // [esp+4h] [ebp-84h]
    char v3; // [esp+8h] [ebp-80h]
    unsigned __int8 v4; // [esp+Ch] [ebp-7Ch]
    cplane_s *v5; // [esp+10h] [ebp-78h]
    DpvsPlane *v6; // [esp+14h] [ebp-74h]
    DpvsPlane dpvsFrustumPlanes[5]; // [esp+18h] [ebp-70h] BYREF
    unsigned int planeIndex; // [esp+84h] [ebp-4h]

    if ( !viewParms
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_showcollision.cpp", 63, 0, "%s", "viewParms") )
    {
        __debugbreak();
    }
    if ( !frustumPlanes
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_showcollision.cpp",
                    64,
                    0,
                    "%s",
                    "frustumPlanes") )
    {
        __debugbreak();
    }
    R_FrustumClipPlanes(&viewParms->viewProjectionMatrix, frustumSidePlanes, 5, dpvsFrustumPlanes);
    for ( planeIndex = 0; planeIndex < 5; ++planeIndex )
    {
        v5 = &frustumPlanes[planeIndex];
        v6 = &dpvsFrustumPlanes[planeIndex];
        v5->normal[0] = v6->coeffs[0];
        v5->normal[1] = v6->coeffs[1];
        v5->normal[2] = v6->coeffs[2];
        v5->dist = v6->coeffs[3];
        frustumPlanes[planeIndex].dist = frustumPlanes[planeIndex].dist * -1.0;
        if ( frustumPlanes[planeIndex].normal[0] == 1.0 )
        {
            v4 = 0;
        }
        else
        {
            if ( frustumPlanes[planeIndex].normal[1] == 1.0 )
            {
                v3 = 1;
            }
            else
            {
                if ( frustumPlanes[planeIndex].normal[2] == 1.0 )
                    v2 = 2;
                else
                    v2 = 3;
                v3 = v2;
            }
            v4 = v3;
        }
        frustumPlanes[planeIndex].type = v4;
        SetPlaneSignbits(&frustumPlanes[planeIndex]);
    }
}

void __cdecl RB_DrawCollisionPoly(int numPoints, float (*points)[3], const float *colorFloat)
{
    int vertCount; // [esp+4h] [ebp-10h]
    int vertIndex; // [esp+8h] [ebp-Ch]
    int vertIndexa; // [esp+8h] [ebp-Ch]
    int vertIndexb; // [esp+8h] [ebp-Ch]
    int vertIndexPrev; // [esp+Ch] [ebp-8h]
    GfxColor color; // [esp+10h] [ebp-4h] BYREF

    if ( numPoints < 3
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_showcollision.cpp",
                    87,
                    0,
                    "%s",
                    "numPoints >= 3") )
    {
        __debugbreak();
    }
    if ( !points
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_showcollision.cpp", 88, 0, "%s", "points") )
    {
        __debugbreak();
    }
    if ( r_showCollisionPolyType->current.integer == 2 || !r_showCollisionPolyType->current.integer )
    {
        R_ConvertColorToBytes(colorFloat, (unsigned __int8 *)&color);
        RB_BeginSurface(rgp.whiteMaterial, 4u);
        R_TrackPrims(&gfxCmdBufState.prim, GFX_PRIM_STATS_DEBUG);
        RB_CheckTessOverflow(numPoints, 3 * (numPoints - 2));
        for ( vertIndex = 0; vertIndex < numPoints; ++vertIndex )
            RB_SetPolyVert(&(*points)[3 * vertIndex], color, vertIndex + tess.vertexCount);
        for ( vertIndexa = 2; vertIndexa < numPoints; ++vertIndexa )
        {
            tess.indices[tess.indexCount] = tess.vertexCount;
            tess.indices[tess.indexCount + 1] = vertIndexa + LOWORD(tess.vertexCount);
            tess.indices[tess.indexCount + 2] = LOWORD(tess.vertexCount) + vertIndexa - 1;
            tess.indexCount += 3;
        }
        tess.vertexCount += numPoints;
        RB_EndTessSurface();
    }
    if ( r_showCollisionPolyType->current.integer <= 1u )
    {
        vertCount = 0;
        vertIndexPrev = numPoints - 1;
        for ( vertIndexb = 0; vertIndexb < numPoints; ++vertIndexb )
        {
            vertCount = RB_AddDebugLine(
                                        &(*points)[3 * vertIndexPrev],
                                        &(*points)[3 * vertIndexb],
                                        colorFloat,
                                        r_showCollisionDepthTest->current.enabled,
                                        vertCount,
                                        2725,
                                        debugLineVerts);
            vertIndexPrev = vertIndexb;
        }
        RB_EndDebugLines(vertCount / 2, debugLineVerts);
    }
}

