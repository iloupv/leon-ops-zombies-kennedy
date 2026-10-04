#include "rb_superflare.h"
#include "r_dvars.h"
#include "rb_logfile.h"
#include "rb_corona.h"
#include "rb_shade.h"
#include "rb_stats.h"
#include <physics/phys_render.h>

IDirect3DQuery9 *SuperFlareQuery[5][2][4][4];
bool SuperFlareQueryIssued[5][4][4];

void __cdecl RB_AllocSuperFlareQueries()
{
    int iFrame; // [esp+8h] [ebp-10h]
    int iPass; // [esp+Ch] [ebp-Ch]
    int iType; // [esp+10h] [ebp-8h]
    int iClient; // [esp+14h] [ebp-4h]

    for ( iClient = 0; iClient < 4; ++iClient )
    {
        for ( iType = 0; iType < 2; ++iType )
        {
            for ( iPass = 0; iPass < 4; ++iPass )
            {
                for ( iFrame = 0; iFrame < 5; ++iFrame )
                {
                    SuperFlareQuery[iFrame][iType][iClient][iPass] = RB_HW_AllocOcclusionQuery();
                    SuperFlareQueryIssued[iFrame][iClient][iPass] = 0;
                }
            }
        }
    }
}

void __cdecl RB_FreeSuperFlareQueries()
{
    IDirect3DQuery9 *varCopy; // [esp+0h] [ebp-14h]
    int iFrame; // [esp+4h] [ebp-10h]
    int iPass; // [esp+8h] [ebp-Ch]
    int iType; // [esp+Ch] [ebp-8h]
    int iClient; // [esp+10h] [ebp-4h]

    for ( iClient = 0; iClient < 4; ++iClient )
    {
        for ( iType = 0; iType < 2; ++iType )
        {
            for ( iPass = 0; iPass < 4; ++iPass )
            {
                for ( iFrame = 0; iFrame < 5; ++iFrame )
                {
                    do
                    {
                        if ( r_logFile )
                        {
                            if ( r_logFile->current.integer )
                                RB_LogPrint("SuperFlareQuery[iFrame][iType][iClient][iPass]->Release()\n");
                        }
                        varCopy = SuperFlareQuery[iFrame][iType][iClient][iPass];
                        SuperFlareQuery[iFrame][iType][iClient][iPass] = 0;
                        R_ReleaseAndSetNULL<IDirect3DIndexBuffer9>(
                            (IDirect3DSurface9 *)varCopy,
                            "SuperFlareQuery[iFrame][iType][iClient][iPass]",
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_superflare.cpp",
                            77);
                    }
                    while ( alwaysfails );
                }
            }
        }
    }
}

void __cdecl RB_DrawSuperFlareOccluders(const GfxViewInfo *viewInfo)
{
    Material *superFlareOccluderDebug; // [esp+0h] [ebp-74h]
    Material *superFlareOccluderNoZDebug; // [esp+4h] [ebp-70h]
    float v3; // [esp+8h] [ebp-6Ch]
    float v4; // [esp+1Ch] [ebp-58h]
    float v5; // [esp+2Ch] [ebp-48h]
    float v6; // [esp+30h] [ebp-44h]
    int iOccluder; // [esp+44h] [ebp-30h]
    unsigned int PossiblePixelCount; // [esp+48h] [ebp-2Ch]
    bool Error; // [esp+4Fh] [ebp-25h]
    unsigned int VisiblePixelCount; // [esp+50h] [ebp-24h]
    Material *mat[2]; // [esp+54h] [ebp-20h]
    bool QueryInitialized; // [esp+5Fh] [ebp-15h]
    int iFrame; // [esp+60h] [ebp-14h]
    int iPass; // [esp+64h] [ebp-10h]
    int iFilter; // [esp+68h] [ebp-Ch]
    GfxGenericFilter *FilterInfo; // [esp+70h] [ebp-4h]

    if ( r_superFlare_enable->current.enabled )
    {
        PROF_SCOPED("RB_SuperFlareVisibility");

        FilterInfo = (GfxGenericFilter *)&viewInfo->genericFilter;
        for ( iFilter = 0; iFilter < 3; ++iFilter )
        {
            for ( iPass = 0; iPass < 16; ++iPass )
            {
                if ( iPass < 3 )
                {
                    iFrame = r_glob.backEndFrameCount % 5;
                    QueryInitialized = 1;
                    if ( !SuperFlareQuery[r_glob.backEndFrameCount % 5][0][viewInfo->localClientNum][iPass]
                        || !SuperFlareQuery[iFrame][1][viewInfo->localClientNum][iPass] )
                    {
                        QueryInitialized = 0;
                    }
                    if ( !SuperFlareQuery[iFrame][0][viewInfo->localClientNum][iPass]
                        || !SuperFlareQuery[iFrame][1][viewInfo->localClientNum][iPass] )
                    {
                        QueryInitialized = 0;
                    }
                    if ( QueryInitialized
                        && FilterInfo->passEnabled[iFilter][iPass]
                        && FilterInfo->passMaterial[iFilter][iPass]
                        && FilterInfo->passQuads[iFilter][iPass] > 0 )
                    {
                        if ( SuperFlareQueryIssued[iFrame][viewInfo->localClientNum][iPass] )
                        {
                            PossiblePixelCount = RB_HW_ReadOcclusionQuery(SuperFlareQuery[iFrame][0][viewInfo->localClientNum][iPass]);
                            VisiblePixelCount = RB_HW_ReadOcclusionQuery(SuperFlareQuery[iFrame][1][viewInfo->localClientNum][iPass]);
                            if ( PossiblePixelCount )
                                Error = VisiblePixelCount > PossiblePixelCount;
                            else
                                Error = 1;
                            if ( Error )
                            {
                                v4 = 0.0f;
                            }
                            else
                            {
                                v5 = (double)VisiblePixelCount / (double)PossiblePixelCount;
                                if ( (float)(v5 - 1.0) < 0.0 )
                                    v6 = (double)VisiblePixelCount / (double)PossiblePixelCount;
                                else
                                    v6 = 1.0f;
                                if ( (float)(0.0 - v5) < 0.0 )
                                    v3 = v6;
                                else
                                    v3 = 0.0f;
                                v4 = v3;
                            }
                            FilterInfo->passFlareOcclusion[iFilter][iPass] = v4;
                        }
                        else
                        {
                            FilterInfo->passFlareOcclusion[iFilter][iPass] = 0.0f;
                        }
                        if ( r_superFlare_debug->current.enabled )
                            superFlareOccluderNoZDebug = rgp.superFlareOccluderNoZDebug;
                        else
                            superFlareOccluderNoZDebug = rgp.superFlareOccluderNoZ;
                        mat[0] = superFlareOccluderNoZDebug;
                        if ( r_superFlare_debug->current.enabled )
                            superFlareOccluderDebug = rgp.superFlareOccluderDebug;
                        else
                            superFlareOccluderDebug = rgp.superFlareOccluder;
                        mat[1] = superFlareOccluderDebug;
                        for ( iOccluder = 0; iOccluder < 2; ++iOccluder )
                        {
                            RB_HW_BeginOcclusionQuery(SuperFlareQuery[iFrame][iOccluder][viewInfo->localClientNum][iPass]);
                            RB_SetTessTechnique(mat[iOccluder], 4u);
                            R_TrackPrims(&gfxCmdBufState.prim, GFX_PRIM_STATS_CODE);
                            RB_CheckTessOverflow(4, 6);
                            RB_SetIdentity();
                            RB_DrawSuperFlareOccluder(FilterInfo, iFilter, iPass);
                            RB_EndTessSurface();
                            SuperFlareQuery[iFrame][iOccluder][viewInfo->localClientNum][iPass]->Issue(1);
                            SuperFlareQueryIssued[iFrame][viewInfo->localClientNum][iPass] = 1;
                        }
                    }
                    else
                    {
                        FilterInfo->passFlareOcclusion[iFilter][iPass] = 0.0f;
                    }
                }
                else
                {
                    FilterInfo->passFlareOcclusion[iFilter][iPass] = 0.0f;
                    FilterInfo->passFlareOcclusion[iFilter][iPass] = 0.0f;
                }
            }
        }
    }
}

void __cdecl RB_DrawSuperFlareOccluder(GfxGenericFilter *FilterInfo, int iFilter, int iPass)
{
    float v3; // [esp+30h] [ebp-4Ch]
    float transformedPosition; // [esp+40h] [ebp-3Ch]
    float transformedPosition_4; // [esp+44h] [ebp-38h]
    float transformedPosition_8; // [esp+48h] [ebp-34h]
    float transformedPosition_12; // [esp+4Ch] [ebp-30h]
    float radius; // [esp+54h] [ebp-28h]
    float sizeX; // [esp+64h] [ebp-18h]
    float position; // [esp+6Ch] [ebp-10h]
    float position_4; // [esp+70h] [ebp-Ch]
    float position_8; // [esp+74h] [ebp-8h]
    GfxVertex *vert; // [esp+78h] [ebp-4h]

    position = FilterInfo->passParam[iFilter][iPass][0];
    position_4 = FilterInfo->passParam[iFilter][iPass][1];
    position_8 = FilterInfo->passParam[iFilter][iPass][2];
    if (position == 0.0 && position_4 == 0.0 && position_8 == 0.0)
    {
        position = FilterInfo->sunPosition[0];
        position_4 = FilterInfo->sunPosition[1];
        position_8 = FilterInfo->sunPosition[2];
    }
    radius = FilterInfo->passParam[iFilter][iPass][5];
    transformedPosition = (float)((float)((float)(position * gfxCmdBufSourceState.viewParms3D->viewProjectionMatrix.m[0][0])
        + (float)(position_4
            * gfxCmdBufSourceState.viewParms3D->viewProjectionMatrix.m[1][0]))
        + (float)(position_8 * gfxCmdBufSourceState.viewParms3D->viewProjectionMatrix.m[2][0]))
        + gfxCmdBufSourceState.viewParms3D->viewProjectionMatrix.m[3][0];
    transformedPosition_4 = (float)((float)((float)(position
        * gfxCmdBufSourceState.viewParms3D->viewProjectionMatrix.m[0][1])
        + (float)(position_4
            * gfxCmdBufSourceState.viewParms3D->viewProjectionMatrix.m[1][1]))
        + (float)(position_8 * gfxCmdBufSourceState.viewParms3D->viewProjectionMatrix.m[2][1]))
        + gfxCmdBufSourceState.viewParms3D->viewProjectionMatrix.m[3][1];
    transformedPosition_8 = (float)((float)((float)(position
        * gfxCmdBufSourceState.viewParms3D->viewProjectionMatrix.m[0][2])
        + (float)(position_4
            * gfxCmdBufSourceState.viewParms3D->viewProjectionMatrix.m[1][2]))
        + (float)(position_8 * gfxCmdBufSourceState.viewParms3D->viewProjectionMatrix.m[2][2]))
        + gfxCmdBufSourceState.viewParms3D->viewProjectionMatrix.m[3][2];
    transformedPosition_12 = (float)((float)((float)(position
        * gfxCmdBufSourceState.viewParms3D->viewProjectionMatrix.m[0][3])
        + (float)(position_4
            * gfxCmdBufSourceState.viewParms3D->viewProjectionMatrix.m[1][3]))
        + (float)(position_8 * gfxCmdBufSourceState.viewParms3D->viewProjectionMatrix.m[2][3]))
        + gfxCmdBufSourceState.viewParms3D->viewProjectionMatrix.m[3][3];

    if (radius < 0.0)
    {
        //radius = COERCE_FLOAT(LODWORD(radius) & _mask__AbsFloat_) * transformedPosition_12;
        radius = (-(radius)) * transformedPosition_12;
    }

    sizeX = (float)((float)gfxCmdBufSourceState.renderTargetHeight * (float)(radius / transformedPosition_12)) / (float)gfxCmdBufSourceState.renderTargetWidth;
    vert = RB_SetSuperFlareQuads();
    v3 = (float)(radius / transformedPosition_12) * transformedPosition_12;
    vert->xyzw[0] = transformedPosition;
    vert->xyzw[1] = transformedPosition_4;
    vert->xyzw[2] = transformedPosition_8;
    vert->xyzw[3] = transformedPosition_12;
    vert[1].xyzw[0] = transformedPosition;
    vert[1].xyzw[1] = transformedPosition_4;
    vert[1].xyzw[2] = transformedPosition_8;
    vert[1].xyzw[3] = transformedPosition_12;
    vert[2].xyzw[0] = transformedPosition;
    vert[2].xyzw[1] = transformedPosition_4;
    vert[2].xyzw[2] = transformedPosition_8;
    vert[2].xyzw[3] = transformedPosition_12;
    vert[3].xyzw[0] = transformedPosition;
    vert[3].xyzw[1] = transformedPosition_4;
    vert[3].xyzw[2] = transformedPosition_8;
    vert[3].xyzw[3] = transformedPosition_12;
    vert->xyzw[0] = vert->xyzw[0] + (float)(sizeX * transformedPosition_12);
    vert->xyzw[1] = vert->xyzw[1] + v3;
    vert->xyzw[2] = vert->xyzw[2] + (float)(0.000049999999 * transformedPosition_12);
    vert[1].xyzw[0] = vert[1].xyzw[0] + (float)(sizeX * transformedPosition_12);
    vert[1].xyzw[1] = vert[1].xyzw[1] - v3;
    vert[1].xyzw[2] = vert[1].xyzw[2] + (float)(0.000049999999 * transformedPosition_12);
    vert[2].xyzw[0] = vert[2].xyzw[0] - (float)(sizeX * transformedPosition_12);
    vert[2].xyzw[1] = vert[2].xyzw[1] - v3;
    vert[2].xyzw[2] = vert[2].xyzw[2] + (float)(0.000049999999 * transformedPosition_12);
    vert[3].xyzw[0] = vert[3].xyzw[0] - (float)(sizeX * transformedPosition_12);
    vert[3].xyzw[1] = vert[3].xyzw[1] + v3;
    vert[3].xyzw[2] = vert[3].xyzw[2] + (float)(0.000049999999 * transformedPosition_12);
    Vec4DivideByW(vert->xyzw, vert->xyzw);
    Vec4DivideByW(vert[1].xyzw, vert[1].xyzw);
    Vec4DivideByW(vert[2].xyzw, vert[2].xyzw);
    Vec4DivideByW(vert[3].xyzw, vert[3].xyzw);
}

GfxVertex *__cdecl RB_SetSuperFlareQuads()
{
    unsigned __int16 vertCount; // [esp+10h] [ebp-8h]
    GfxVertex *vert; // [esp+14h] [ebp-4h]

    if ( tess.vertexCount
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_superflare.cpp",
                    98,
                    0,
                    "%s",
                    "tess.vertexCount == 0") )
    {
        __debugbreak();
    }
    if ( tess.indexCount
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_superflare.cpp",
                    99,
                    0,
                    "%s",
                    "tess.indexCount == 0") )
    {
        __debugbreak();
    }
    vertCount = tess.vertexCount;
    tess.indices[tess.indexCount] = LOWORD(tess.vertexCount) + 3;
    tess.indices[tess.indexCount + 1] = vertCount;
    tess.indices[tess.indexCount + 2] = vertCount + 2;
    tess.indices[tess.indexCount + 3] = vertCount + 2;
    tess.indices[tess.indexCount + 4] = vertCount;
    tess.indices[tess.indexCount + 5] = vertCount + 1;
    vert = &tess.verts[vertCount];
    vert->normal.packed = 1073643391;
    vert->texCoord[0] = 0.0f;
    vert->texCoord[1] = 0.0f;
    vert->color.packed = -1;
    vert[1].normal.packed = 1073643391;
    vert[1].texCoord[0] = 1.0f;
    vert[1].texCoord[1] = 0.0f;
    vert[1].color.packed = -1;
    vert[2].normal.packed = 1073643391;
    vert[2].texCoord[0] = 1.0f;
    vert[2].texCoord[1] = 1.0f;
    vert[2].color.packed = -1;
    vert[3].normal.packed = 1073643391;
    vert[3].texCoord[0] = 0.0f;
    vert[3].texCoord[1] = 1.0f;
    vert[3].color.packed = -1;
    tess.vertexCount += 4;
    tess.indexCount += 6;
    return vert;
}

