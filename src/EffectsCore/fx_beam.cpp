#include "fx_beam.h"
#include <xanim/xanim.h>
#include <gfx_d3d/r_material_load_obj.h>
#include <gfx_d3d/r_drawsurf.h>
#include <gfx_d3d/r_scene.h>

const unsigned __int16 templateIndices[12] =
{ 0u, 2u, 1u, 2u, 4u, 1u, 1u, 4u, 3u, 3u, 4u, 5u };

float4 g_unit = { { 0.0, 0.0, 0.0, 1.0 } };

const float4 wiggle[8] =
{
  { { 0.0, 1.0, 0.0, 0.0 } },
  { { 0.70999998, 0.70999998, 0.0, 0.0 } },
  { { 1.0, 0.0, 0.0, 0.0 } },
  { { 0.70999998, -0.70999998, 0.0, 0.0 } },
  { { 0.0, -1.0, 0.0, 0.0 } },
  { { -0.70999998, -0.70999998, 0.0, 0.0 } },
  { { -1.0, 0.0, 0.0, 0.0 } },
  { { -0.70999998, 0.70999998, 0.0, 0.0 } }
};

FxBeamInfo g_beamInfo;

// local variable allocation has failed, the output may be wrong!
void __cdecl FX_Beam_GenerateVerts(FxGenerateVertsCmd *cmd)
{
    int v1; // eax
    unsigned __int64 v2; // kr08_8
    int v3; // eax
    __int16 v4; // [esp+8h] [ebp-660h]
    __int16 v5; // [esp+Ch] [ebp-65Ch]
    __int16 v6; // [esp+10h] [ebp-658h]
    __int16 v7; // [esp+14h] [ebp-654h]
    __int16 v8; // [esp+18h] [ebp-650h]
    __int16 v9; // [esp+1Ch] [ebp-64Ch]
    __int16 v10; // [esp+20h] [ebp-648h]
    __int16 v11; // [esp+24h] [ebp-644h]
    float v12; // [esp+28h] [ebp-640h]
    __int16 v13; // [esp+2Ch] [ebp-63Ch]
    __int16 v14; // [esp+30h] [ebp-638h]
    __int16 v15; // [esp+34h] [ebp-634h]
    __int16 v16; // [esp+38h] [ebp-630h]
    __int16 v17; // [esp+3Ch] [ebp-62Ch]
    __int16 v18; // [esp+40h] [ebp-628h]
    __int16 v19; // [esp+44h] [ebp-624h]
    __int16 v20; // [esp+48h] [ebp-620h]
    __int16 v21; // [esp+4Ch] [ebp-61Ch]
    __int16 v22; // [esp+50h] [ebp-618h]
    __int16 v23; // [esp+54h] [ebp-614h]
    __int16 v24; // [esp+58h] [ebp-610h]
    __int16 v25; // [esp+5Ch] [ebp-60Ch]
    __int16 v26; // [esp+60h] [ebp-608h]
    __int16 v27; // [esp+64h] [ebp-604h]
    __int16 v28; // [esp+68h] [ebp-600h]
    float v29; // [esp+6Ch] [ebp-5FCh]
    int v30; // [esp+74h] [ebp-5F4h]
    int v31; // [esp+88h] [ebp-5E0h]
    int v32; // [esp+A8h] [ebp-5C0h]
    int v33; // [esp+BCh] [ebp-5ACh]
    int v34; // [esp+DCh] [ebp-58Ch]
    int v35; // [esp+F0h] [ebp-578h]
    int v36; // [esp+110h] [ebp-558h]
    int v37; // [esp+124h] [ebp-544h]
    GfxPackedVertex *v38; // [esp+140h] [ebp-528h]
    GfxPackedVertex *v39; // [esp+144h] [ebp-524h]
    GfxPackedVertex *v40; // [esp+14Ch] [ebp-51Ch]
    int v41; // [esp+15Ch] [ebp-50Ch]
    int v42; // [esp+170h] [ebp-4F8h]
    int v43; // [esp+190h] [ebp-4D8h]
    int v44; // [esp+1A4h] [ebp-4C4h]
    int v45; // [esp+1C4h] [ebp-4A4h]
    int v46; // [esp+1D8h] [ebp-490h]
    int v47; // [esp+1F8h] [ebp-470h]
    int v48; // [esp+20Ch] [ebp-45Ch]
    int v49; // [esp+22Ch] [ebp-43Ch]
    int v50; // [esp+240h] [ebp-428h]
    int v51; // [esp+260h] [ebp-408h]
    int v52; // [esp+274h] [ebp-3F4h]
    unsigned __int8 v53; // [esp+2A8h] [ebp-3C0h]
    int v54; // [esp+2ACh] [ebp-3BCh]
    int v55; // [esp+2B4h] [ebp-3B4h]
    int v56; // [esp+2C8h] [ebp-3A0h]
    int v57; // [esp+2E8h] [ebp-380h]
    int v58; // [esp+2FCh] [ebp-36Ch]
    float4 *v59; // [esp+318h] [ebp-350h]
    float4 *v60; // [esp+31Ch] [ebp-34Ch]
    //__int64 v61; // [esp+324h] [ebp-344h]
    //unsigned int v62; // [esp+32Ch] [ebp-33Ch]
    //int v63; // [esp+330h] [ebp-338h]
    //int v64; // [esp+334h] [ebp-334h]
    //int v65; // [esp+338h] [ebp-330h]
    //int v66; // [esp+33Ch] [ebp-32Ch]
    //int v67; // [esp+340h] [ebp-328h]
    //float4 v68; // [esp+344h] [ebp-324h]
    //float4 *v69; // [esp+358h] [ebp-310h]
    float v70; // [esp+35Ch] [ebp-30Ch]
    float v71; // [esp+378h] [ebp-2F0h]
    float *v72; // [esp+37Ch] [ebp-2ECh]
    float *v73; // [esp+380h] [ebp-2E8h]
    float *end; // [esp+384h] [ebp-2E4h]
    PackedTexCoords v76; // [esp+38Ch] [ebp-2DCh]
    PackedTexCoords v77; // [esp+390h] [ebp-2D8h]
    PackedTexCoords v78; // [esp+394h] [ebp-2D4h]
    PackedTexCoords v79; // [esp+398h] [ebp-2D0h]
    PackedTexCoords v80; // [esp+39Ch] [ebp-2CCh]
    PackedTexCoords v81; // [esp+3A0h] [ebp-2C8h]
    PackedTexCoords v82; // [esp+3A4h] [ebp-2C4h]
    PackedTexCoords v83; // [esp+3A8h] [ebp-2C0h]
    PackedTexCoords v84; // [esp+3ACh] [ebp-2BCh]
    PackedTexCoords v85; // [esp+3B0h] [ebp-2B8h]
    PackedTexCoords v86; // [esp+3B4h] [ebp-2B4h]
    PackedTexCoords v87; // [esp+3B8h] [ebp-2B0h]
    GfxPackedVertex *perp_verts; // [esp+3BCh] [ebp-2ACh]
    MaterialInfo matInfo; // [esp+3C0h] [ebp-2A8h] BYREF
    int v90; // [esp+3ECh] [ebp-27Ch]
    int v91; // [esp+3F0h] [ebp-278h]
    float v92; // [esp+3F4h] [ebp-274h]
    GfxPackedVertex *basePerp_verts; // [esp+3F8h] [ebp-270h]
    int v94; // [esp+3FCh] [ebp-26Ch]
    const float4 *wiggleVec; // [esp+400h] [ebp-268h]
    GfxPackedVertex *vert2; // [esp+404h] [ebp-264h]
    float4 scaledWiggle; // [esp+408h] [ebp-260h]
    int dim; // [esp+418h] [ebp-250h]
    float4 basePos; // [esp+41Ch] [ebp-24Ch]
    float lerpedRadius; // [esp+42Ch] [ebp-23Ch]
    float4 wiggleYs; // [esp+430h] [ebp-238h]
    GfxPackedVertex *vert1; // [esp+440h] [ebp-228h]
    float alpha; // [esp+444h] [ebp-224h]
    float4 wiggleXs; // [esp+448h] [ebp-220h]
    MaterialInfo mtlInfo; // [esp+458h] [ebp-210h] BYREF
    int atlasIndex; // [esp+480h] [ebp-1E8h]
    int row; // [esp+484h] [ebp-1E4h]
    float step; // [esp+488h] [ebp-1E0h]
    int col; // [esp+48Ch] [ebp-1DCh]
    float endRadius; // [esp+490h] [ebp-1D8h]
    int perpSegCount; // [esp+494h] [ebp-1D4h]
    int perp_sMin; // [esp+498h] [ebp-1D0h]
    r_double_index_t workingIndex; // [esp+49Ch] [ebp-1CCh] OVERLAPPED
    r_double_index_t *baseIndices; // [esp+4A4h] [ebp-1C4h] BYREF
    r_double_index_t *perp_indices; // [esp+4A8h] [ebp-1C0h] BYREF
    float4 flatDelta; // [esp+4ACh] [ebp-1BCh] BYREF
    int segIter; // [esp+4BCh] [ebp-1ACh]
    int perp_indexCount; // [esp+4C0h] [ebp-1A8h]
    int perp_vertexCount; // [esp+4C4h] [ebp-1A4h]
    int t1; // [esp+4C8h] [ebp-1A0h]
    int s1; // [esp+4CCh] [ebp-19Ch]
    int s2; // [esp+4D0h] [ebp-198h]
    float perp_tLen; // [esp+4D4h] [ebp-194h]
    float4 *baseArgs; // [esp+4D8h] [ebp-190h]
    const FxBeam *beam; // [esp+4DCh] [ebp-18Ch]
    int tMin; // [esp+4E0h] [ebp-188h]
    float4 beamWorldEnd; // [esp+4E4h] [ebp-184h] BYREF
    float4 vertPos; // [esp+4F4h] [ebp-174h]
    GfxColor endColor; // [esp+504h] [ebp-164h]
    float beginRadius; // [esp+508h] [ebp-160h]
    float wiggleDist; // [esp+50Ch] [ebp-15Ch]
    int vertexCount; // [esp+510h] [ebp-158h]
    int sMin; // [esp+514h] [ebp-154h]
    unsigned __int16 offset; // [esp+518h] [ebp-150h]
    int segCount; // [esp+51Ch] [ebp-14Ch]
    int indexCount; // [esp+520h] [ebp-148h]
    GfxPackedVertex *verts; // [esp+524h] [ebp-144h]
    float4 beamDot; // [esp+528h] [ebp-140h]
    int t2; // [esp+538h] [ebp-130h]
    r_double_index_t *indices; // [esp+53Ch] [ebp-12Ch]
    unsigned __int16 perp_baseVertex; // [esp+540h] [ebp-128h] BYREF
    float4 beamWorldBegin; // [esp+544h] [ebp-124h] BYREF
    GfxColor beginColor; // [esp+554h] [ebp-114h]
    float perp_sLen; // [esp+558h] [ebp-110h]
    GfxColor lerpedColor; // [esp+560h] [ebp-108h]
    float sLen; // [esp+564h] [ebp-104h]
    int perp_tMin; // [esp+568h] [ebp-100h]
    float4 tpos0; // [esp+56Ch] [ebp-FCh]
    float4 normDelta; // [esp+57Ch] [ebp-ECh] BYREF
    unsigned int argOffset; // [esp+58Ch] [ebp-DCh] BYREF
    float tLen; // [esp+590h] [ebp-D8h]
    unsigned __int16 baseVertex; // [esp+594h] [ebp-D4h] BYREF
    float4 perpFlatDelta; // [esp+598h] [ebp-D0h] BYREF
    float4 *args; // [esp+5A8h] [ebp-C0h]
    GfxPackedVertex *baseVerts; // [esp+5ACh] [ebp-BCh]
    float4 tpos1; // [esp+5B0h] [ebp-B8h]
    FxBeamInfo *beamInfo; // [esp+5C0h] [ebp-A8h]
    float4 viewAxis; // [esp+5C8h] [ebp-A0h] BYREF
    float4x4 clipMtx; // [esp+5D8h] [ebp-90h] BYREF
    int beamIter; // [esp+618h] [ebp-50h]
    float *segVerts; // [esp+61Ch] [ebp-4Ch]
    float4x4 invClipMtx; // [esp+620h] [ebp-48h] BYREF
    int indexPairIter;

    segVerts = 0;

    iassert(cmd);
    iassert(cmd->beamInfo);

    beamInfo = cmd->beamInfo;

    CreateClipMatrix(&clipMtx, cmd->vieworg, cmd->viewaxis);
    MatrixInverse44(clipMtx.mat, invClipMtx.mat);

    viewAxis.v[0] = cmd->viewaxis[0][0];
    viewAxis.v[1] = cmd->viewaxis[0][1];
    viewAxis.v[2] = cmd->viewaxis[0][2];
    viewAxis.u[3] = 0;

    for ( beamIter = 0; beamIter != beamInfo->beamCount; ++beamIter )
    {
        perp_indexCount = 0;
        perp_vertexCount = 0;
        sMin = 0;
        sLen = 1.0f;
        tMin = 0;
        tLen = 1.0f;
        perp_sMin = 0;
        perp_sLen = 1.0f;
        perp_tMin = 0;
        perp_tLen = 1.0f;
        perp_indices = 0;
        perp_baseVertex = 0;
        beam = &beamInfo->beams[beamIter];
        beginRadius = beam->beginRadius;
        endRadius = beam->endRadius;
        segCount = beam->segmentCount;
        perpSegCount = beam->perpSegmentCount;
        flatDelta = g_zero;

        //*(_QWORD *)beamWorldBegin.v = *(_QWORD *)beam->begin;
        beamWorldBegin.v[0] = beam->begin[0];
        beamWorldBegin.v[1] = beam->begin[1];
        beamWorldBegin.v[2] = beam->begin[2];
        beamWorldBegin.u[3] = 0;

        //end = beam->end;
        //*(_QWORD *)beamWorldEnd.v = *(_QWORD *)beam->end;
        beamWorldEnd.v[0] = beam->end[0];
        beamWorldEnd.v[1] = beam->end[1];
        beamWorldEnd.v[2] = beam->end[2];
        beamWorldEnd.u[3] = 0;

        indexCount = 3 * (2 * segCount + 2);
        vertexCount = 2 * segCount + 4;
        if ( beam->segmentVerts )
            segVerts = beam->segmentVerts;
        if ( (beam->flags & 8) != 0 )
        {
            if ( !segVerts
                && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_beam.cpp", 382, 0, "%s", "segVerts") )
            {
                __debugbreak();
            }
            beamWorldBegin.v[0] = segVerts[0];
            beamWorldBegin.v[1] = segVerts[1];
            beamWorldBegin.v[2] = segVerts[2];
            beamWorldBegin.u[3] = 0;

            v73 = &segVerts[3 * (segCount / 3)];
            beamWorldEnd.v[0] = v73[0];
            beamWorldEnd.v[1] = v73[1];
            beamWorldEnd.v[2] = v73[2];
            beamWorldEnd.u[3] = 0;

            if (!FX_GenerateBeam_GetFlatDelta(
                &clipMtx,
                &invClipMtx,
                &beamWorldBegin,
                &beamWorldEnd,
                &flatDelta))
            {
                continue;
            }

            beamWorldBegin.v[0] = beam->begin[0];
            beamWorldBegin.v[1] = beam->begin[1];
            beamWorldBegin.v[2] = beam->begin[2];
            beamWorldBegin.u[3] = 0;

            beamWorldEnd.v[0] = beam->end[0];
            beamWorldEnd.v[1] = beam->end[1];
            beamWorldEnd.v[2] = beam->end[2];
            beamWorldEnd.u[3] = 0;
        }
        else if ( !FX_GenerateBeam_GetFlatDelta(
                                 &clipMtx,
                                 &invClipMtx,
                                 &beamWorldBegin,
                                 &beamWorldEnd,
                                 &flatDelta) )
        {
            continue;
        }
        if ( !R_ReserveCodeMeshIndices(indexCount, &baseIndices)
            || !R_ReserveCodeMeshVerts(vertexCount, &baseVertex)
            || !R_ReserveCodeMeshArgs(2, &argOffset) )
        {
            return;
        }
        indices = baseIndices;
        indexPairIter = 0;
        workingIndex.value[0] = templateIndices[0] + baseVertex;
        //*(r_double_index_t *)((char *)&workingIndex + 2) = (r_double_index_t)(unsigned __int16)(baseVertex + 2);
        workingIndex.value[1] = baseVertex + 2;
        *baseIndices = workingIndex;
        ++indexPairIter;
        workingIndex.value[0] = baseVertex + 1;
        for (segIter = 0; segIter != segCount; ++segIter)
        {
            offset = 2 * segIter;
            workingIndex.value[1] = 2 * segIter + baseVertex + 2;
            indices[indexPairIter++] = workingIndex;
            workingIndex.value[0] = offset + baseVertex + 4;
            workingIndex.value[1] = offset + baseVertex + 1;
            indices[indexPairIter++] = workingIndex;
            workingIndex.value[0] = offset + baseVertex + 1;
            workingIndex.value[1] = offset + baseVertex + 4;
            indices[indexPairIter++] = workingIndex;
            workingIndex.value[0] = offset + baseVertex + 3;
        }
        offset = 2 * segCount - 2;
        workingIndex.value[1] = offset + baseVertex + 3;
        indices[indexPairIter++] = workingIndex;
        workingIndex.value[0] = offset + baseVertex + 4;
        workingIndex.value[1] = offset + baseVertex + 5;
        indices[indexPairIter] = workingIndex;

        iassert(workingIndex.value[1] == baseVertex + vertexCount - 1);
        indexPairIter++;
        iassert(indexPairIter == indexCount / 2);

        Material_GetInfo(beam->material, &mtlInfo);

        if ( mtlInfo.textureAtlasColumnCount > 1u || mtlInfo.textureAtlasRowCount > 1u )
        {
            v71 = (float)(mtlInfo.textureAtlasRowCount * mtlInfo.textureAtlasColumnCount) * beam->animFrac;
            v29 = floor(v71);
            v1 = (int)v29;
            atlasIndex = v1;
            row = v1 / mtlInfo.textureAtlasColumnCount;
            col = v1 % mtlInfo.textureAtlasColumnCount;
            step = 1.0 / (float)mtlInfo.textureAtlasColumnCount;
            *(float *)&sMin = (float)col * step;
            sLen = step;
            step = 1.0 / (float)mtlInfo.textureAtlasRowCount;
            *(float *)&tMin = (float)row * step;
            tLen = step;
        }

        baseArgs = (float4 *)R_GetCodeMeshArgs(argOffset);
        baseVerts = R_GetCodeMeshVerts(baseVertex);
        Vec3Cross(flatDelta.v, viewAxis.v, perpFlatDelta.v);
        perpFlatDelta.u[3] = 0;
        Vec3NormalizeTo(perpFlatDelta.v, perpFlatDelta.v);
        perpFlatDelta.u[3] = 0;
        normDelta.v[0] = beamWorldEnd.v[0] - beamWorldBegin.v[0];
        normDelta.v[1] = beamWorldEnd.v[1] - beamWorldBegin.v[1];
        normDelta.v[2] = beamWorldEnd.v[2] - beamWorldBegin.v[2];
        normDelta.v[3] = beamWorldEnd.v[3] - beamWorldBegin.v[3];
        Vec3NormalizeTo(normDelta.v, normDelta.v);
        normDelta.u[3] = 0;

        float negdot = -Vec3Dot(beamWorldBegin.v, normDelta.v);
        beamDot.v[0] = negdot;
        beamDot.v[1] = negdot;
        beamDot.v[2] = negdot;
        beamDot.v[3] = negdot;

        args = baseArgs;
        //v61 = *(_QWORD *)normDelta.v;
        //v62 = normDelta.u[2];
        //v63 = 0;
        //v64 = -(v70);
        //v65 = -(v70);
        //v66 = -(v70);
        //v67 = -(v70);
        //v68 = g_swizzleXYZA;
        //v69 = baseArgs;

        baseArgs->unitVec[0].array[0] = normDelta.unitVec[0].array[g_swizzleXYZA.unitVec[0].array[3]];
        baseArgs->unitVec[0].array[1] = normDelta.unitVec[0].array[g_swizzleXYZA.unitVec[0].array[2]];
        baseArgs->unitVec[0].array[2] = normDelta.unitVec[0].array[g_swizzleXYZA.unitVec[0].array[1]];
        baseArgs->unitVec[0].array[3] = normDelta.unitVec[0].array[g_swizzleXYZA.unitVec[0].array[0]];
        baseArgs->unitVec[1].array[0] = normDelta.unitVec[0].array[g_swizzleXYZA.unitVec[1].array[3]];
        baseArgs->unitVec[1].array[1] = normDelta.unitVec[0].array[g_swizzleXYZA.unitVec[1].array[2]];
        baseArgs->unitVec[1].array[2] = normDelta.unitVec[0].array[g_swizzleXYZA.unitVec[1].array[1]];
        baseArgs->unitVec[1].array[3] = normDelta.unitVec[0].array[g_swizzleXYZA.unitVec[1].array[0]];
        baseArgs->unitVec[2].array[0] = normDelta.unitVec[0].array[g_swizzleXYZA.unitVec[2].array[3]];
        baseArgs->unitVec[2].array[1] = normDelta.unitVec[0].array[g_swizzleXYZA.unitVec[2].array[2]];
        baseArgs->unitVec[2].array[2] = normDelta.unitVec[0].array[g_swizzleXYZA.unitVec[2].array[1]];
        baseArgs->unitVec[2].array[3] = normDelta.unitVec[0].array[g_swizzleXYZA.unitVec[2].array[0]];
        // At this point, the swizzle goes over 16 (float vec4x4) and goes oob into "beamDot".
        baseArgs->unitVec[3].array[0] = beamDot.unitVec[0].array[g_swizzleXYZA.unitVec[3].array[3] - 16];
        baseArgs->unitVec[3].array[1] = beamDot.unitVec[0].array[g_swizzleXYZA.unitVec[3].array[2] - 16];
        baseArgs->unitVec[3].array[2] = beamDot.unitVec[0].array[g_swizzleXYZA.unitVec[3].array[1] - 16];
        baseArgs->unitVec[3].array[3] = beamDot.unitVec[0].array[g_swizzleXYZA.unitVec[3].array[0] - 16];

        args[1] = g_unit;
        v59 = args + 1;
        v60 = args + 1;
        args[1].v[0] = args[1].v[0] + beamWorldBegin.v[0];
        v59->v[1] = v60->v[1] + beamWorldBegin.v[1];
        v59->v[2] = v60->v[2] + beamWorldBegin.v[2];
        v59->v[3] = v60->v[3] + beamWorldBegin.v[3];
        beginColor.packed = (unsigned int)beam->beginColor.packed;
        endColor.packed = (unsigned int)beam->endColor.packed;
        verts = baseVerts;
        vertPos.v[0] = (float)(0.0 * flatDelta.v[0]) + beamWorldBegin.v[0];
        vertPos.v[1] = (float)(0.0 * flatDelta.v[1]) + beamWorldBegin.v[1];
        vertPos.v[2] = (float)(0.0 * flatDelta.v[2]) + beamWorldBegin.v[2];
        vertPos.v[3] = (float)(0.0 * flatDelta.v[3]) + beamWorldBegin.v[3];
        verts->xyz[0] = vertPos.v[0];
        verts->xyz[1] = vertPos.v[1];
        verts->xyz[2] = vertPos.v[2];
        verts->color = beginColor;
        verts->binormalSign = 1.0f;
        verts->normal.packed = 1073643391;
        verts->tangent.packed = 1065320446;
        verts->texCoord.packed = 0;

        if ( (beam->flags & 1) != 0 )
        {
            s1 = sMin;
            *(float *)&t1 = (float)(tLen * 0.5) + *(float *)&tMin;
            if ( (int)((2 * sMin) ^ 0x80000000) >> 14 < 0x3FFF )
                v58 = (int)((2 * sMin) ^ 0x80000000) >> 14;
            else
                v58 = 0x3FFF;
            if ( v58 > -16384 )
                v28 = v58;
            else
                v28 = -16384;
            if ( (int)((2 * t1) ^ 0x80000000) >> 14 < 0x3FFF )
                v57 = (int)((2 * t1) ^ 0x80000000) >> 14;
            else
                v57 = 0x3FFF;
            if ( v57 > -16384 )
                v27 = v57;
            else
                v27 = -16384;
            v87.packed = (v27 & 0x3FFF | (t1 >> 16) & 0xC000) + ((v28 & 0x3FFF | (sMin >> 16) & 0xC000) << 16);
            verts->texCoord = v87;
        }
        else if ( (beam->flags & 2) != 0 )
        {
            *(float *)&s1 = (float)(sLen * 0.5) + *(float *)&sMin;
            *(float *)&t1 = *(float *)&tMin + tLen;
            if ( (int)((2 * s1) ^ 0x80000000) >> 14 < 0x3FFF )
                v56 = (int)((2 * COERCE_INT((float)(sLen * 0.5) + *(float *)&sMin)) ^ 0x80000000) >> 14;
            else
                v56 = 0x3FFF;
            if ( v56 > -16384 )
                v26 = v56;
            else
                v26 = -16384;
            if ( (int)((2 * t1) ^ 0x80000000) >> 14 < 0x3FFF )
                v55 = (int)((2 * t1) ^ 0x80000000) >> 14;
            else
                v55 = 0x3FFF;
            if ( v55 > -16384 )
                v25 = v55;
            else
                v25 = -16384;
            v86.packed = (v25 & 0x3FFF | (t1 >> 16) & 0xC000)
                                 + ((v26 & 0x3FFF | (int((float)(sLen * 0.5) + *(float *)&sMin) >> 16) & 0xC000) << 16);
            verts->texCoord = v86;
        }
        else
        {
            verts->texCoord.packed = 0;
        }

        ++verts;
        wiggleDist = beam->wiggleDist;
        lerpedColor.packed = -1;
        for ( segIter = 0; segIter <= segCount; ++segIter )
        {
            alpha = (float)segIter / (float)segCount;
            wiggleVec = &wiggle[segIter % 8u];

            scaledWiggle.v[0] = wiggleDist * wiggleVec->v[0];
            scaledWiggle.v[1] = wiggleDist * wiggleVec->v[1];
            scaledWiggle.v[2] = wiggleDist * wiggleVec->v[2];
            scaledWiggle.v[3] = wiggleDist * wiggleVec->v[3];

            wiggleXs.v[0] = scaledWiggle.v[0];
            wiggleXs.v[1] = scaledWiggle.v[0];
            wiggleXs.v[2] = scaledWiggle.v[0];
            wiggleXs.v[3] = scaledWiggle.v[0];

            wiggleYs.v[0] = scaledWiggle.v[1];
            wiggleYs.v[1] = scaledWiggle.v[1];
            wiggleYs.v[2] = scaledWiggle.v[1];
            wiggleYs.v[3] = scaledWiggle.v[1];

            for ( dim = 0; dim != 4; ++dim )
            {
                v54 = (int)(float)((float)beginColor.array[dim]
                                                 + (float)((float)(endColor.array[dim] - beginColor.array[dim]) * alpha));
                if ( v54 >= 0 )
                {
                    if ( v54 <= 255 )
                        v53 = (int)(float)((float)beginColor.array[dim]
                                                         + (float)((float)(endColor.array[dim] - beginColor.array[dim]) * alpha));
                    else
                        v53 = -1;
                }
                else
                {
                    v53 = 0;
                }
                lerpedColor.array[dim] = v53;
            }
            lerpedRadius = (float)((float)(endRadius - beginRadius) * alpha) + beginRadius;
            if ( segVerts )
            {
                basePos.v[0] = segVerts[0];
                basePos.v[1] = segVerts[1];
                basePos.v[2] = segVerts[2];
                basePos.u[3] = 0;
                segVerts += 3;
            }
            else
            {
                basePos.v[0] = (float)(alpha * (float)(beamWorldEnd.v[0] - beamWorldBegin.v[0])) + beamWorldBegin.v[0];
                basePos.v[1] = (float)(alpha * (float)(beamWorldEnd.v[1] - beamWorldBegin.v[1])) + beamWorldBegin.v[1];
                basePos.v[2] = (float)(alpha * (float)(beamWorldEnd.v[2] - beamWorldBegin.v[2])) + beamWorldBegin.v[2];
                basePos.v[3] = (float)(alpha * (float)(beamWorldEnd.v[3] - beamWorldBegin.v[3])) + beamWorldBegin.v[3];
            }
            basePos.v[0] = (float)(wiggleXs.v[0] * perpFlatDelta.v[0]) + basePos.v[0];
            basePos.v[1] = (float)(wiggleXs.v[1] * perpFlatDelta.v[1]) + basePos.v[1];
            basePos.v[2] = (float)(wiggleXs.v[2] * perpFlatDelta.v[2]) + basePos.v[2];
            basePos.v[3] = (float)(wiggleXs.v[3] * perpFlatDelta.v[3]) + basePos.v[3];
            basePos.v[0] = (float)(wiggleYs.v[0] * flatDelta.v[0]) + basePos.v[0];
            basePos.v[1] = (float)(wiggleYs.v[1] * flatDelta.v[1]) + basePos.v[1];
            basePos.v[2] = (float)(wiggleYs.v[2] * flatDelta.v[2]) + basePos.v[2];
            basePos.v[3] = (float)(wiggleYs.v[3] * flatDelta.v[3]) + basePos.v[3];
            tpos0.v[0] = (float)((-(lerpedRadius)) * perpFlatDelta.v[0]) + basePos.v[0];
            tpos0.v[1] = (float)((-(lerpedRadius)) * perpFlatDelta.v[1]) + basePos.v[1];
            tpos0.v[2] = (float)((-(lerpedRadius)) * perpFlatDelta.v[2]) + basePos.v[2];
            tpos0.v[3] = (float)((-(lerpedRadius)) * perpFlatDelta.v[3]) + basePos.v[3];
            tpos1.v[0] = (float)(lerpedRadius * perpFlatDelta.v[0]) + basePos.v[0];
            tpos1.v[1] = (float)(lerpedRadius * perpFlatDelta.v[1]) + basePos.v[1];
            tpos1.v[2] = (float)(lerpedRadius * perpFlatDelta.v[2]) + basePos.v[2];
            tpos1.v[3] = (float)(lerpedRadius * perpFlatDelta.v[3]) + basePos.v[3];
            vert1 = verts;
            verts->xyz[0] = tpos0.v[0];
            verts->xyz[1] = tpos0.v[1];
            verts->xyz[2] = tpos0.v[2];
            verts->color = lerpedColor;
            verts->binormalSign = 1.0f;
            verts->normal.packed = 1073643391;
            verts->tangent.packed = 1065320446;
            verts->texCoord.packed = 15360;
            ++verts;
            verts->xyz[0] = tpos1.v[0];
            verts->xyz[1] = tpos1.v[1];
            verts->xyz[2] = tpos1.v[2];
            vert2 = verts;
            verts->color = lerpedColor;
            verts->binormalSign = 1.0f;
            verts->normal.packed = 1073643391;
            verts->tangent.packed = 1065320446;
            verts->texCoord.packed = 1006632960;
            ++verts;
            if ( (beam->flags & 1) != 0 )
            {
                *(float *)&s1 = (float)((float)((float)segIter * sLen) / (float)segCount) + *(float *)&sMin;
                s2 = s1;
                t1 = tMin;
                *(float *)&t2 = *(float *)&tMin + tLen;
                if ( (int)((2 * s1) ^ 0x80000000) >> 14 < 0x3FFF )
                    v52 = (int)((2 * COERCE_INT((float)((float)((float)segIter * sLen) / (float)segCount) + *(float *)&sMin))
                                        ^ 0x80000000) >> 14;
                else
                    v52 = 0x3FFF;
                if ( v52 > -16384 )
                    v24 = v52;
                else
                    v24 = -16384;
                if ( (int)((2 * t1) ^ 0x80000000) >> 14 < 0x3FFF )
                    v51 = (int)((2 * t1) ^ 0x80000000) >> 14;
                else
                    v51 = 0x3FFF;
                if ( v51 > -16384 )
                    v23 = v51;
                else
                    v23 = -16384;
                v85.packed = (v23 & 0x3FFF | (t1 >> 16) & 0xC000)
                                     + ((v24 & 0x3FFF
                                         | (int((float)((float)((float)segIter * sLen) / (float)segCount) + *(float *)&sMin) >> 16)
                                         & 0xC000) << 16);
                vert1->texCoord = v85;
                if ( (int)((2 * s2) ^ 0x80000000) >> 14 < 0x3FFF )
                    v50 = (int)((2 * s2) ^ 0x80000000) >> 14;
                else
                    v50 = 0x3FFF;
                if ( v50 > -16384 )
                    v22 = v50;
                else
                    v22 = -16384;
                if ( (int)((2 * t2) ^ 0x80000000) >> 14 < 0x3FFF )
                    v49 = (int)((2 * t2) ^ 0x80000000) >> 14;
                else
                    v49 = 0x3FFF;
                if ( v49 > -16384 )
                    v21 = v49;
                else
                    v21 = -16384;
                v84.packed = (v21 & 0x3FFF | (t2 >> 16) & 0xC000) + ((v22 & 0x3FFF | (s2 >> 16) & 0xC000) << 16);
                vert2->texCoord = v84;
            }
            else if ( (beam->flags & 2) != 0 )
            {
                s1 = sMin;
                *(float *)&s2 = *(float *)&sMin + sLen;
                *(float *)&t1 = (float)((float)(1.0 - (float)((float)segIter / (float)segCount)) * tLen) + *(float *)&tMin;
                t2 = t1;
                if ( (int)((2 * sMin) ^ 0x80000000) >> 14 < 0x3FFF )
                    v48 = (int)((2 * sMin) ^ 0x80000000) >> 14;
                else
                    v48 = 0x3FFF;
                if ( v48 > -16384 )
                    v20 = v48;
                else
                    v20 = -16384;
                if ( (int)((2 * t1) ^ 0x80000000) >> 14 < 0x3FFF )
                    v47 = (int)((2 * t1) ^ 0x80000000) >> 14;
                else
                    v47 = 0x3FFF;
                if ( v47 > -16384 )
                    v19 = v47;
                else
                    v19 = -16384;
                v83.packed = (v19 & 0x3FFF | (t1 >> 16) & 0xC000) + ((v20 & 0x3FFF | (sMin >> 16) & 0xC000) << 16);
                vert1->texCoord = v83;
                if ( (int)((2 * s2) ^ 0x80000000) >> 14 < 0x3FFF )
                    v46 = (int)((2 * s2) ^ 0x80000000) >> 14;
                else
                    v46 = 0x3FFF;
                if ( v46 > -16384 )
                    v18 = v46;
                else
                    v18 = -16384;
                if ( (int)((2 * t2) ^ 0x80000000) >> 14 < 0x3FFF )
                    v45 = (int)((2 * t2) ^ 0x80000000) >> 14;
                else
                    v45 = 0x3FFF;
                if ( v45 > -16384 )
                    v17 = v45;
                else
                    v17 = -16384;
                v82.packed = (v17 & 0x3FFF | (t2 >> 16) & 0xC000) + ((v18 & 0x3FFF | (s2 >> 16) & 0xC000) << 16);
                vert2->texCoord = v82;
            }
            else
            {
                vert1->texCoord.packed = 15360;
                vert2->texCoord.packed = 1006632960;
            }
        }
        tpos0.v[0] = (float)(0.0 * flatDelta.v[0]) + beamWorldEnd.v[0];
        tpos0.v[1] = (float)(0.0 * flatDelta.v[1]) + beamWorldEnd.v[1];
        tpos0.v[2] = (float)(0.0 * flatDelta.v[2]) + beamWorldEnd.v[2];
        tpos0.v[3] = (float)(0.0 * flatDelta.v[3]) + beamWorldEnd.v[3];
        verts->xyz[0] = tpos0.v[0];
        verts->xyz[1] = tpos0.v[1];
        verts->xyz[2] = tpos0.v[2];
        verts->color = endColor;
        verts->binormalSign = 1.0f;
        verts->normal.packed = 1073643391;
        verts->tangent.packed = 1065320446;
        verts->texCoord.packed = 1006648320;
        if ( (beam->flags & 1) != 0 )
        {
            *(float *)&s1 = *(float *)&sMin + sLen;
            *(float *)&t1 = (float)(tLen * 0.5) + *(float *)&tMin;
            if ( (int)((2 * COERCE_INT(*(float *)&sMin + sLen)) ^ 0x80000000) >> 14 < 0x3FFF )
                v44 = (int)((2 * COERCE_INT(*(float *)&sMin + sLen)) ^ 0x80000000) >> 14;
            else
                v44 = 0x3FFF;
            if ( v44 > -16384 )
                v16 = v44;
            else
                v16 = -16384;
            if ( (int)((2 * t1) ^ 0x80000000) >> 14 < 0x3FFF )
                v43 = (int)((2 * t1) ^ 0x80000000) >> 14;
            else
                v43 = 0x3FFF;
            if ( v43 > -16384 )
                v15 = v43;
            else
                v15 = -16384;
            v81.packed = (v15 & 0x3FFF | (t1 >> 16) & 0xC000)
                                 + ((v16 & 0x3FFF | (int(*(float *)&sMin + sLen) >> 16) & 0xC000) << 16);
            verts->texCoord = v81;
        }
        else if ( (beam->flags & 2) != 0 )
        {
            *(float *)&s1 = (float)(sLen * 0.5) + *(float *)&sMin;
            t1 = tMin;
            if ( (int)((2 * s1) ^ 0x80000000) >> 14 < 0x3FFF )
                v42 = (int)((2 * COERCE_INT((float)(sLen * 0.5) + *(float *)&sMin)) ^ 0x80000000) >> 14;
            else
                v42 = 0x3FFF;
            if ( v42 > -16384 )
                v14 = v42;
            else
                v14 = -16384;
            if ( (int)((2 * t1) ^ 0x80000000) >> 14 < 0x3FFF )
                v41 = (int)((2 * t1) ^ 0x80000000) >> 14;
            else
                v41 = 0x3FFF;
            if ( v41 > -16384 )
                v13 = v41;
            else
                v13 = -16384;
            v80.packed = (v13 & 0x3FFF | (t1 >> 16) & 0xC000)
                                 + ((v14 & 0x3FFF | (int((float)(sLen * 0.5) + *(float *)&sMin) >> 16) & 0xC000) << 16);
            verts->texCoord = v80;
        }
        else
        {
            verts->texCoord.packed = 1006648320;
        }
        R_AddCodeMeshDrawSurf(beam->material, baseIndices, indexCount, argOffset, 2u, "Beam", 0xDu);
        if ( (beam->flags & 4) != 0 )
        {
            iassert(perpSegCount == 1);
            perp_indexCount = 6 * perpSegCount;
            perp_vertexCount = 4 * perpSegCount;
            if ( !R_ReserveCodeMeshIndices(6 * perpSegCount, &perp_indices)
                || !R_ReserveCodeMeshVerts(perp_vertexCount, &perp_baseVertex)
                || !R_ReserveCodeMeshArgs(2, &argOffset) )
            {
                return;
            }
            indexPairIter = 0;
            for (segIter = 0; segIter != perpSegCount; ++segIter)
            {
                offset = 4 * segIter;
                workingIndex.value[0] = 4 * segIter + perp_baseVertex;
                workingIndex.value[1] = workingIndex.value[0] + 2;
                perp_indices[indexPairIter++] = workingIndex;
                workingIndex.value[0] = perp_baseVertex + offset + 1;
                workingIndex.value[1] = offset + perp_baseVertex;
                perp_indices[indexPairIter++] = workingIndex;
                workingIndex.value[0] = perp_baseVertex + offset + 1;
                workingIndex.value[1] = perp_baseVertex + offset + 3;
                perp_indices[indexPairIter++] = workingIndex;
            }
            Material_GetInfo(beam->material2, &matInfo);
            if ( matInfo.textureAtlasColumnCount > 1u || matInfo.textureAtlasRowCount > 1u )
            {
                v12 = floor((float)((float)(matInfo.textureAtlasRowCount * matInfo.textureAtlasColumnCount) * beam->animFrac));
                v3 = (int)v12;
                v90 = v3;
                v91 = v3 / matInfo.textureAtlasColumnCount;
                v94 = v3 % matInfo.textureAtlasColumnCount;
                v92 = 1.0 / (float)matInfo.textureAtlasColumnCount;
                *(float *)&perp_sMin = (float)v94 * v92;
                perp_sLen = v92;
                v92 = 1.0 / (float)matInfo.textureAtlasRowCount;
                *(float *)&perp_tMin = (float)v91 * v92;
                perp_tLen = v92;
            }
            basePerp_verts = R_GetCodeMeshVerts(perp_baseVertex);
            perp_verts = basePerp_verts;
            tpos0.v[1] = (float)((-(beginRadius)) * perpFlatDelta.v[1]) + beamWorldBegin.v[1];
            tpos0.v[2] = (float)((-(beginRadius)) * perpFlatDelta.v[2]) + beamWorldBegin.v[2];
            tpos1.v[0] = (float)(beginRadius * perpFlatDelta.v[0]) + beamWorldBegin.v[0];
            tpos1.v[1] = (float)(beginRadius * perpFlatDelta.v[1]) + beamWorldBegin.v[1];
            tpos1.v[2] = (float)(beginRadius * perpFlatDelta.v[2]) + beamWorldBegin.v[2];
            basePerp_verts->xyz[0] = (float)((-(beginRadius)) * perpFlatDelta.v[0]) + beamWorldBegin.v[0];
            *(_QWORD *)&perp_verts->xyz[1] = *(_QWORD *)&tpos0.unitVec[1].packed;
            v40 = perp_verts + 1;
            LODWORD(perp_verts[1].xyz[0]) = tpos1.u[0];
            *(_QWORD *)&v40->xyz[1] = *(_QWORD *)&tpos1.unitVec[1].packed;
            tpos0.v[0] = (float)((-(beginRadius)) * flatDelta.v[0]) + beamWorldBegin.v[0];
            tpos0.v[1] = (float)((-(beginRadius)) * flatDelta.v[1]) + beamWorldBegin.v[1];
            tpos0.v[2] = (float)((-(beginRadius)) * flatDelta.v[2]) + beamWorldBegin.v[2];
            tpos0.v[3] = (float)((-(beginRadius)) * flatDelta.v[3]) + beamWorldBegin.v[3];
            tpos1.v[0] = (float)(beginRadius * flatDelta.v[0]) + beamWorldBegin.v[0];
            tpos1.v[1] = (float)(beginRadius * flatDelta.v[1]) + beamWorldBegin.v[1];
            tpos1.v[2] = (float)(beginRadius * flatDelta.v[2]) + beamWorldBegin.v[2];
            tpos1.v[3] = (float)(beginRadius * flatDelta.v[3]) + beamWorldBegin.v[3];
            v39 = perp_verts + 2;
            perp_verts[2].xyz[0] = tpos0.v[0];
            *(_QWORD *)&v39->xyz[1] = *(_QWORD *)&tpos0.unitVec[1].packed;
            v38 = perp_verts + 3;
            LODWORD(perp_verts[3].xyz[0]) = tpos1.u[0];
            *(_QWORD *)&v38->xyz[1] = *(_QWORD *)&tpos1.unitVec[1].packed;
            perp_verts->color = beginColor;
            perp_verts[1].color = beginColor;
            perp_verts[2].color = beginColor;
            perp_verts[3].color = beginColor;
            perp_verts->binormalSign = 1.0f;
            perp_verts->normal.packed = 1073643391;
            perp_verts->tangent.packed = 1065320446;
            perp_verts[1].binormalSign = 1.0f;
            perp_verts[1].normal.packed = 1073643391;
            perp_verts[1].tangent.packed = 1065320446;
            perp_verts[2].binormalSign = 1.0f;
            perp_verts[2].normal.packed = 1073643391;
            perp_verts[2].tangent.packed = 1065320446;
            perp_verts[3].binormalSign = 1.0f;
            perp_verts[3].normal.packed = 1073643391;
            perp_verts[3].tangent.packed = 1065320446;
            s1 = perp_sMin;
            *(float *)&s2 = *(float *)&perp_sMin + perp_sLen;
            t1 = perp_tMin;
            *(float *)&t2 = *(float *)&perp_tMin + perp_tLen;
            if ( (int)((2 * perp_sMin) ^ 0x80000000) >> 14 < 0x3FFF )
                v37 = (int)((2 * perp_sMin) ^ 0x80000000) >> 14;
            else
                v37 = 0x3FFF;
            if ( v37 > -16384 )
                v11 = v37;
            else
                v11 = -16384;
            if ( (int)((2 * t1) ^ 0x80000000) >> 14 < 0x3FFF )
                v36 = (int)((2 * t1) ^ 0x80000000) >> 14;
            else
                v36 = 0x3FFF;
            if ( v36 > -16384 )
                v10 = v36;
            else
                v10 = -16384;
            v79.packed = (v10 & 0x3FFF | (t1 >> 16) & 0xC000) + ((v11 & 0x3FFF | (perp_sMin >> 16) & 0xC000) << 16);
            perp_verts->texCoord = v79;
            if ( (int)((2 * s2) ^ 0x80000000) >> 14 < 0x3FFF )
                v35 = (int)((2 * s2) ^ 0x80000000) >> 14;
            else
                v35 = 0x3FFF;
            if ( v35 > -16384 )
                v9 = v35;
            else
                v9 = -16384;
            if ( (int)((2 * t2) ^ 0x80000000) >> 14 < 0x3FFF )
                v34 = (int)((2 * t2) ^ 0x80000000) >> 14;
            else
                v34 = 0x3FFF;
            if ( v34 > -16384 )
                v8 = v34;
            else
                v8 = -16384;
            v78.packed = (v8 & 0x3FFF | (t2 >> 16) & 0xC000) + ((v9 & 0x3FFF | (s2 >> 16) & 0xC000) << 16);
            perp_verts[1].texCoord = v78;
            if ( (int)((2 * s1) ^ 0x80000000) >> 14 < 0x3FFF )
                v33 = (int)((2 * s1) ^ 0x80000000) >> 14;
            else
                v33 = 0x3FFF;
            if ( v33 > -16384 )
                v7 = v33;
            else
                v7 = -16384;
            if ( (int)((2 * t2) ^ 0x80000000) >> 14 < 0x3FFF )
                v32 = (int)((2 * t2) ^ 0x80000000) >> 14;
            else
                v32 = 0x3FFF;
            if ( v32 > -16384 )
                v6 = v32;
            else
                v6 = -16384;
            v77.packed = (v6 & 0x3FFF | (t2 >> 16) & 0xC000) + ((v7 & 0x3FFF | (s1 >> 16) & 0xC000) << 16);
            perp_verts[2].texCoord = v77;
            if ( (int)((2 * s2) ^ 0x80000000) >> 14 < 0x3FFF )
                v31 = (int)((2 * s2) ^ 0x80000000) >> 14;
            else
                v31 = 0x3FFF;
            if ( v31 > -16384 )
                v5 = v31;
            else
                v5 = -16384;
            if ( (int)((2 * t1) ^ 0x80000000) >> 14 < 0x3FFF )
                v30 = (int)((2 * t1) ^ 0x80000000) >> 14;
            else
                v30 = 0x3FFF;
            if ( v30 > -16384 )
                v4 = v30;
            else
                v4 = -16384;
            v76.packed = (v4 & 0x3FFF | (t1 >> 16) & 0xC000) + ((v5 & 0x3FFF | (s2 >> 16) & 0xC000) << 16);
            perp_verts[3].texCoord = v76;
            R_AddCodeMeshDrawSurf(beam->material2, perp_indices, perp_indexCount, argOffset, 2u, "Beam", 0xDu);
        }
    }
}

void __cdecl CreateClipMatrix(float4x4 *clipMtx, const float *vieworg, const float (*viewaxis)[3])
{
    float4x4 viewMtx; // [esp+Ch] [ebp-88h] BYREF
    float4x4 projMtx; // [esp+4Ch] [ebp-48h] BYREF

    Float4x4ForViewer(&viewMtx, vieworg, viewaxis);

    Float4x4InfinitePerspectiveMatrix(&projMtx,
        CG_GetLocalClientGlobals(R_GetLocalClientNum())->refdef.tanHalfFovX,
        CG_GetLocalClientGlobals(R_GetLocalClientNum())->refdef.tanHalfFovY,
        1.0);

    clipMtx->x.v[0] = viewMtx.x.v[0] * projMtx.x.v[0]
        + viewMtx.x.v[1] * projMtx.y.v[0]
        + viewMtx.x.v[2] * projMtx.z.v[0]
        + viewMtx.x.v[3] * projMtx.w.v[0];
    clipMtx->x.v[1] = viewMtx.x.v[0] * projMtx.x.v[1]
        + viewMtx.x.v[1] * projMtx.y.v[1]
        + viewMtx.x.v[2] * projMtx.z.v[1]
        + viewMtx.x.v[3] * projMtx.w.v[1];
    clipMtx->x.v[2] = viewMtx.x.v[0] * projMtx.x.v[2]
        + viewMtx.x.v[1] * projMtx.y.v[2]
        + viewMtx.x.v[2] * projMtx.z.v[2]
        + viewMtx.x.v[3] * projMtx.w.v[2];
    clipMtx->x.v[3] = viewMtx.x.v[0] * projMtx.x.v[3]
        + viewMtx.x.v[1] * projMtx.y.v[3]
        + viewMtx.x.v[2] * projMtx.z.v[3]
        + viewMtx.x.v[3] * projMtx.w.v[3];
    clipMtx->y.v[0] = viewMtx.y.v[0] * projMtx.x.v[0]
        + viewMtx.y.v[1] * projMtx.y.v[0]
        + viewMtx.y.v[2] * projMtx.z.v[0]
        + viewMtx.y.v[3] * projMtx.w.v[0];
    clipMtx->y.v[1] = viewMtx.y.v[0] * projMtx.x.v[1]
        + viewMtx.y.v[1] * projMtx.y.v[1]
        + viewMtx.y.v[2] * projMtx.z.v[1]
        + viewMtx.y.v[3] * projMtx.w.v[1];
    clipMtx->y.v[2] = viewMtx.y.v[0] * projMtx.x.v[2]
        + viewMtx.y.v[1] * projMtx.y.v[2]
        + viewMtx.y.v[2] * projMtx.z.v[2]
        + viewMtx.y.v[3] * projMtx.w.v[2];
    clipMtx->y.v[3] = viewMtx.y.v[0] * projMtx.x.v[3]
        + viewMtx.y.v[1] * projMtx.y.v[3]
        + viewMtx.y.v[2] * projMtx.z.v[3]
        + viewMtx.y.v[3] * projMtx.w.v[3];
    clipMtx->z.v[0] = viewMtx.z.v[0] * projMtx.x.v[0]
        + viewMtx.z.v[1] * projMtx.y.v[0]
        + viewMtx.z.v[2] * projMtx.z.v[0]
        + viewMtx.z.v[3] * projMtx.w.v[0];
    clipMtx->z.v[1] = viewMtx.z.v[0] * projMtx.x.v[1]
        + viewMtx.z.v[1] * projMtx.y.v[1]
        + viewMtx.z.v[2] * projMtx.z.v[1]
        + viewMtx.z.v[3] * projMtx.w.v[1];
    clipMtx->z.v[2] = viewMtx.z.v[0] * projMtx.x.v[2]
        + viewMtx.z.v[1] * projMtx.y.v[2]
        + viewMtx.z.v[2] * projMtx.z.v[2]
        + viewMtx.z.v[3] * projMtx.w.v[2];
    clipMtx->z.v[3] = viewMtx.z.v[0] * projMtx.x.v[3]
        + viewMtx.z.v[1] * projMtx.y.v[3]
        + viewMtx.z.v[2] * projMtx.z.v[3]
        + viewMtx.z.v[3] * projMtx.w.v[3];
    clipMtx->w.v[0] = viewMtx.w.v[0] * projMtx.x.v[0]
        + viewMtx.w.v[1] * projMtx.y.v[0]
        + viewMtx.w.v[2] * projMtx.z.v[0]
        + viewMtx.w.v[3] * projMtx.w.v[0];
    clipMtx->w.v[1] = viewMtx.w.v[0] * projMtx.x.v[1]
        + viewMtx.w.v[1] * projMtx.y.v[1]
        + viewMtx.w.v[2] * projMtx.z.v[1]
        + viewMtx.w.v[3] * projMtx.w.v[1];
    clipMtx->w.v[2] = viewMtx.w.v[0] * projMtx.x.v[2]
        + viewMtx.w.v[1] * projMtx.y.v[2]
        + viewMtx.w.v[2] * projMtx.z.v[2]
        + viewMtx.w.v[3] * projMtx.w.v[2];
    clipMtx->w.v[3] = viewMtx.w.v[0] * projMtx.x.v[3]
        + viewMtx.w.v[1] * projMtx.y.v[3]
        + viewMtx.w.v[2] * projMtx.z.v[3]
        + viewMtx.w.v[3] * projMtx.w.v[3];
}

void __cdecl Float4x4ForViewer(float4x4 *mtx, const float *origin3, const float (*axis3)[3])
{
    float4 origin; // [esp+144h] [ebp-A8h]
    float4x4 tAxis; // [esp+154h] [ebp-98h] BYREF
    float4 transRow; // [esp+194h] [ebp-58h]
    float4x4 axis; // [esp+1A4h] [ebp-48h] BYREF

    origin.v[0] = origin3[0];
    origin.v[1] = origin3[1];
    origin.v[2] = origin3[2];
    origin.v[3] = 0.0;

    tAxis.x.v[0] = (axis3)[0][0];
    tAxis.x.v[1] = (axis3)[0][1];
    tAxis.x.v[2] = (axis3)[0][2];
    tAxis.x.v[3] = 0.0;

    tAxis.y.v[0] = (axis3)[1][0];
    tAxis.y.v[1] = (axis3)[1][1];
    tAxis.y.v[2] = (axis3)[1][2];
    tAxis.y.v[3] = 0.0;

    tAxis.z.v[0] = (axis3)[2][0];
    tAxis.z.v[1] = (axis3)[2][1];
    tAxis.z.v[2] = (axis3)[2][2];
    tAxis.z.v[3] = 0.0;

    tAxis.w.v[0] = 0.0;
    tAxis.w.v[1] = 0.0;
    tAxis.w.v[2] = 0.0;
    tAxis.w.v[3] = 1.0;

    tAxis.y.v[0] = -tAxis.y.v[0];
    tAxis.y.v[1] = -tAxis.y.v[1];
    tAxis.y.v[2] = -tAxis.y.v[2];
    tAxis.y.v[3] = -(float)0.0;

    axis.x = { tAxis.x.v[0], tAxis.y.v[0], tAxis.z.v[0], 0.0f };
    axis.y = { tAxis.x.v[1], tAxis.y.v[1], tAxis.z.v[1], 0.0f };
    axis.z = { tAxis.x.v[2], tAxis.y.v[2], tAxis.z.v[2], 0.0f };
    axis.w = { 0.0f,         tAxis.y.v[3], 0.0f,         1.0f };

    mtx->x.unitVec[0].array[0] = axis.x.unitVec[0].array[g_swizzleYZXW.unitVec[0].array[3]];
    mtx->x.unitVec[0].array[1] = axis.x.unitVec[0].array[g_swizzleYZXW.unitVec[0].array[2]];
    mtx->x.unitVec[0].array[2] = axis.x.unitVec[0].array[g_swizzleYZXW.unitVec[0].array[1]];
    mtx->x.unitVec[0].array[3] = axis.x.unitVec[0].array[g_swizzleYZXW.unitVec[0].array[0]];
    mtx->x.unitVec[1].array[0] = axis.x.unitVec[0].array[g_swizzleYZXW.unitVec[1].array[3]];
    mtx->x.unitVec[1].array[1] = axis.x.unitVec[0].array[g_swizzleYZXW.unitVec[1].array[2]];
    mtx->x.unitVec[1].array[2] = axis.x.unitVec[0].array[g_swizzleYZXW.unitVec[1].array[1]];
    mtx->x.unitVec[1].array[3] = axis.x.unitVec[0].array[g_swizzleYZXW.unitVec[1].array[0]];
    mtx->x.unitVec[2].array[0] = axis.x.unitVec[0].array[g_swizzleYZXW.unitVec[2].array[3]];
    mtx->x.unitVec[2].array[1] = axis.x.unitVec[0].array[g_swizzleYZXW.unitVec[2].array[2]];
    mtx->x.unitVec[2].array[2] = axis.x.unitVec[0].array[g_swizzleYZXW.unitVec[2].array[1]];
    mtx->x.unitVec[2].array[3] = axis.x.unitVec[0].array[g_swizzleYZXW.unitVec[2].array[0]];
    mtx->x.unitVec[3].array[0] = axis.x.unitVec[0].array[g_swizzleYZXW.unitVec[3].array[3]];
    mtx->x.unitVec[3].array[1] = axis.x.unitVec[0].array[g_swizzleYZXW.unitVec[3].array[2]];
    mtx->x.unitVec[3].array[2] = axis.x.unitVec[0].array[g_swizzleYZXW.unitVec[3].array[1]];
    mtx->x.unitVec[3].array[3] = axis.x.unitVec[0].array[g_swizzleYZXW.unitVec[3].array[0]];

    mtx->y.unitVec[0].array[0] = axis.y.unitVec[0].array[g_swizzleYZXW.unitVec[0].array[3]];
    mtx->y.unitVec[0].array[1] = axis.y.unitVec[0].array[g_swizzleYZXW.unitVec[0].array[2]];
    mtx->y.unitVec[0].array[2] = axis.y.unitVec[0].array[g_swizzleYZXW.unitVec[0].array[1]];
    mtx->y.unitVec[0].array[3] = axis.y.unitVec[0].array[g_swizzleYZXW.unitVec[0].array[0]];
    mtx->y.unitVec[1].array[0] = axis.y.unitVec[0].array[g_swizzleYZXW.unitVec[1].array[3]];
    mtx->y.unitVec[1].array[1] = axis.y.unitVec[0].array[g_swizzleYZXW.unitVec[1].array[2]];
    mtx->y.unitVec[1].array[2] = axis.y.unitVec[0].array[g_swizzleYZXW.unitVec[1].array[1]];
    mtx->y.unitVec[1].array[3] = axis.y.unitVec[0].array[g_swizzleYZXW.unitVec[1].array[0]];
    mtx->y.unitVec[2].array[0] = axis.y.unitVec[0].array[g_swizzleYZXW.unitVec[2].array[3]];
    mtx->y.unitVec[2].array[1] = axis.y.unitVec[0].array[g_swizzleYZXW.unitVec[2].array[2]];
    mtx->y.unitVec[2].array[2] = axis.y.unitVec[0].array[g_swizzleYZXW.unitVec[2].array[1]];
    mtx->y.unitVec[2].array[3] = axis.y.unitVec[0].array[g_swizzleYZXW.unitVec[2].array[0]];
    mtx->y.unitVec[3].array[0] = axis.y.unitVec[0].array[g_swizzleYZXW.unitVec[3].array[3]];
    mtx->y.unitVec[3].array[1] = axis.y.unitVec[0].array[g_swizzleYZXW.unitVec[3].array[2]];
    mtx->y.unitVec[3].array[2] = axis.y.unitVec[0].array[g_swizzleYZXW.unitVec[3].array[1]];
    mtx->y.unitVec[3].array[3] = axis.y.unitVec[0].array[g_swizzleYZXW.unitVec[3].array[0]];

    mtx->z.unitVec[0].array[0] = axis.z.unitVec[0].array[g_swizzleYZXW.unitVec[0].array[3]];
    mtx->z.unitVec[0].array[1] = axis.z.unitVec[0].array[g_swizzleYZXW.unitVec[0].array[2]];
    mtx->z.unitVec[0].array[2] = axis.z.unitVec[0].array[g_swizzleYZXW.unitVec[0].array[1]];
    mtx->z.unitVec[0].array[3] = axis.z.unitVec[0].array[g_swizzleYZXW.unitVec[0].array[0]];
    mtx->z.unitVec[1].array[0] = axis.z.unitVec[0].array[g_swizzleYZXW.unitVec[1].array[3]];
    mtx->z.unitVec[1].array[1] = axis.z.unitVec[0].array[g_swizzleYZXW.unitVec[1].array[2]];
    mtx->z.unitVec[1].array[2] = axis.z.unitVec[0].array[g_swizzleYZXW.unitVec[1].array[1]];
    mtx->z.unitVec[1].array[3] = axis.z.unitVec[0].array[g_swizzleYZXW.unitVec[1].array[0]];
    mtx->z.unitVec[2].array[0] = axis.z.unitVec[0].array[g_swizzleYZXW.unitVec[2].array[3]];
    mtx->z.unitVec[2].array[1] = axis.z.unitVec[0].array[g_swizzleYZXW.unitVec[2].array[2]];
    mtx->z.unitVec[2].array[2] = axis.z.unitVec[0].array[g_swizzleYZXW.unitVec[2].array[1]];
    mtx->z.unitVec[2].array[3] = axis.z.unitVec[0].array[g_swizzleYZXW.unitVec[2].array[0]];
    mtx->z.unitVec[3].array[0] = axis.z.unitVec[0].array[g_swizzleYZXW.unitVec[3].array[3]];
    mtx->z.unitVec[3].array[1] = axis.z.unitVec[0].array[g_swizzleYZXW.unitVec[3].array[2]];
    mtx->z.unitVec[3].array[2] = axis.z.unitVec[0].array[g_swizzleYZXW.unitVec[3].array[1]];
    mtx->z.unitVec[3].array[3] = axis.z.unitVec[0].array[g_swizzleYZXW.unitVec[3].array[0]];

    mtx->w = g_unit;
    transRow.v[0] = -(origin.v[0] * mtx->x.v[0] + origin.v[1] * mtx->y.v[0] + origin.v[2] * mtx->z.v[0] + mtx->w.v[0]);
    transRow.v[1] = -(origin.v[0] * mtx->x.v[1] + origin.v[1] * mtx->y.v[1] + origin.v[2] * mtx->z.v[1] + mtx->w.v[1]);
    transRow.v[2] = -(origin.v[0] * mtx->x.v[2] + origin.v[1] * mtx->y.v[2] + origin.v[2] * mtx->z.v[2] + mtx->w.v[2]);
    transRow.v[3] = -(origin.v[0] * mtx->x.v[3] + origin.v[1] * mtx->y.v[3] + origin.v[2] * mtx->z.v[3] + mtx->w.v[3]);
    transRow.v[0] += 0.0;
    transRow.v[1] += 0.0;
    transRow.v[2] += 0.0;
    transRow.v[3] += 2.0;
    mtx->w = transRow;
}

void __cdecl Float4x4InfinitePerspectiveMatrix(float4x4 *mtx, float tanHalfFovX, float tanHalfFovY, float zNear)
{
    mtx->x.v[0] = MAX_11BIT_FLT / tanHalfFovX;
    mtx->x.v[1] = 0.0f;
    mtx->x.v[2] = 0.0f;
    mtx->x.v[3] = 0.0f;

    mtx->y.v[0] = 0.0f;
    mtx->y.v[1] = MAX_11BIT_FLT / tanHalfFovY;
    mtx->y.v[2] = 0.0f;
    mtx->y.v[3] = 0.0f;

    mtx->z.v[0] = 0.0f;
    mtx->z.v[1] = 0.0f;
    mtx->z.v[2] = MAX_11BIT_FLT;
    mtx->z.v[3] = 1.0f;

    mtx->w.v[0] = 0.0f;
    mtx->w.v[1] = 0.0f;
    mtx->w.v[2] = -zNear * MAX_11BIT_FLT;
    mtx->w.v[3] = 0.0f;
}

// local variable allocation has failed, the output may be wrong!
char    FX_GenerateBeam_GetFlatDelta(
                const float4x4 *clipMtx,
                const float4x4 *invClipMtx,
                const float4 *beamWorldBegin,
                const float4 *beamWorldEnd,
                float4 *outFlatDelta)
{
    float v7; // [esp-10h] [ebp-E0h]
    float4 v8; // [esp-Ch] [ebp-DCh] BYREF
    float4 worldSpaceFlatDelta; // [esp+8h] [ebp-C8h]
    float4 v10; // [esp+18h] [ebp-B8h]
    float4 clipSpaceFlatDelta; // [esp+28h] [ebp-A8h]
    float4 clipSpaceBeamEndDivided; // [esp+38h] [ebp-98h]
    float v13; // [esp+48h] [ebp-88h]
    float4 clipSpaceBeamBeginDivided; // [esp+4Ch] [ebp-84h]
    float v15; // [esp+5Ch] [ebp-74h]
    float4 clipSpaceBeamEnd; // [esp+60h] [ebp-70h] BYREF
    float4 v17; // [esp+70h] [ebp-60h]
    float4 clipSpaceBeamBegin; // [esp+80h] [ebp-50h] BYREF
    float4 v19; // [esp+90h] [ebp-40h]
    float4 worldSpaceBeamEnd; // [esp+A0h] [ebp-30h]
    float4 worldSpaceBeamBegin; // [esp+B0h] [ebp-20h]
    //_UNKNOWN *v22; // [esp+C4h] [ebp-Ch]
    //const float4x4 *clipMtxa; // [esp+C8h] [ebp-8h]
    //const float4 *beamWorldBegina; // [esp+D0h] [ebp+0h]
    //
    //v22 = a1;
    //clipMtxa = (const float4x4 *)beamWorldBegina;
    worldSpaceBeamBegin.v[0] = beamWorldBegin->v[0] + 0.0;
    worldSpaceBeamBegin.v[1] = beamWorldBegin->v[1] + 0.0;
    worldSpaceBeamBegin.v[2] = beamWorldBegin->v[2] + 0.0;
    worldSpaceBeamBegin.v[3] = beamWorldBegin->v[3] + 1.0;
    worldSpaceBeamEnd.v[0] = beamWorldEnd->v[0] + 0.0;
    worldSpaceBeamEnd.v[1] = beamWorldEnd->v[1] + 0.0;
    worldSpaceBeamEnd.v[2] = beamWorldEnd->v[2] + 0.0;
    worldSpaceBeamEnd.v[3] = beamWorldEnd->v[3] + 1.0;
    v19.v[0] = (float)((float)((float)(worldSpaceBeamBegin.v[0] * clipMtx->x.v[0])
        + (float)(worldSpaceBeamBegin.v[1] * clipMtx->y.v[0]))
        + (float)(worldSpaceBeamBegin.v[2] * clipMtx->z.v[0]))
        + (float)(worldSpaceBeamBegin.v[3] * clipMtx->w.v[0]);
    v19.v[1] = (float)((float)((float)(worldSpaceBeamBegin.v[0] * clipMtx->x.v[1])
        + (float)(worldSpaceBeamBegin.v[1] * clipMtx->y.v[1]))
        + (float)(worldSpaceBeamBegin.v[2] * clipMtx->z.v[1]))
        + (float)(worldSpaceBeamBegin.v[3] * clipMtx->w.v[1]);
    v19.v[2] = (float)((float)((float)(worldSpaceBeamBegin.v[0] * clipMtx->x.v[2])
        + (float)(worldSpaceBeamBegin.v[1] * clipMtx->y.v[2]))
        + (float)(worldSpaceBeamBegin.v[2] * clipMtx->z.v[2]))
        + (float)(worldSpaceBeamBegin.v[3] * clipMtx->w.v[2]);
    v19.v[3] = (float)((float)((float)(worldSpaceBeamBegin.v[0] * clipMtx->x.v[3])
        + (float)(worldSpaceBeamBegin.v[1] * clipMtx->y.v[3]))
        + (float)(worldSpaceBeamBegin.v[2] * clipMtx->z.v[3]))
        + (float)(worldSpaceBeamBegin.v[3] * clipMtx->w.v[3]);
    clipSpaceBeamBegin = v19;
    v17.v[0] = (float)((float)((float)(worldSpaceBeamEnd.v[0] * clipMtx->x.v[0])
        + (float)(worldSpaceBeamEnd.v[1] * clipMtx->y.v[0]))
        + (float)(worldSpaceBeamEnd.v[2] * clipMtx->z.v[0]))
        + (float)(worldSpaceBeamEnd.v[3] * clipMtx->w.v[0]);
    v17.v[1] = (float)((float)((float)(worldSpaceBeamEnd.v[0] * clipMtx->x.v[1])
        + (float)(worldSpaceBeamEnd.v[1] * clipMtx->y.v[1]))
        + (float)(worldSpaceBeamEnd.v[2] * clipMtx->z.v[1]))
        + (float)(worldSpaceBeamEnd.v[3] * clipMtx->w.v[1]);
    v17.v[2] = (float)((float)((float)(worldSpaceBeamEnd.v[0] * clipMtx->x.v[2])
        + (float)(worldSpaceBeamEnd.v[1] * clipMtx->y.v[2]))
        + (float)(worldSpaceBeamEnd.v[2] * clipMtx->z.v[2]))
        + (float)(worldSpaceBeamEnd.v[3] * clipMtx->w.v[2]);
    v17.v[3] = (float)((float)((float)(worldSpaceBeamEnd.v[0] * clipMtx->x.v[3])
        + (float)(worldSpaceBeamEnd.v[1] * clipMtx->y.v[3]))
        + (float)(worldSpaceBeamEnd.v[2] * clipMtx->z.v[3]))
        + (float)(worldSpaceBeamEnd.v[3] * clipMtx->w.v[3]);
    clipSpaceBeamEnd = v17;
    if (!Vec4HomogenousClipBothZ(&clipSpaceBeamBegin, &clipSpaceBeamEnd))
        return 0;
    if (clipSpaceBeamBegin.v[3] == 0.0
        && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\universal\\com_vector4_novec.h", 645, 0, "%s", "in.v[3]"))
    {
        __debugbreak();
    }
    v15 = 1.0 / clipSpaceBeamBegin.v[3];
    clipSpaceBeamBeginDivided.v[0] = clipSpaceBeamBegin.v[0] * (float)(1.0 / clipSpaceBeamBegin.v[3]);
    clipSpaceBeamBeginDivided.v[1] = clipSpaceBeamBegin.v[1] * (float)(1.0 / clipSpaceBeamBegin.v[3]);
    clipSpaceBeamBeginDivided.v[2] = clipSpaceBeamBegin.v[2] * (float)(1.0 / clipSpaceBeamBegin.v[3]);
    clipSpaceBeamBeginDivided.v[3] = 1.0f;
    if (clipSpaceBeamEnd.v[3] == 0.0
        && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\universal\\com_vector4_novec.h", 645, 0, "%s", "in.v[3]"))
    {
        __debugbreak();
    }
    v13 = 1.0 / clipSpaceBeamEnd.v[3];
    clipSpaceBeamEndDivided.v[0] = clipSpaceBeamEnd.v[0] * (float)(1.0 / clipSpaceBeamEnd.v[3]);
    clipSpaceBeamEndDivided.v[1] = clipSpaceBeamEnd.v[1] * (float)(1.0 / clipSpaceBeamEnd.v[3]);
    clipSpaceBeamEndDivided.v[2] = clipSpaceBeamEnd.v[2] * (float)(1.0 / clipSpaceBeamEnd.v[3]);
    clipSpaceBeamEndDivided.v[3] = 1.0f;
    clipSpaceFlatDelta.v[0] = clipSpaceBeamEndDivided.v[0] - clipSpaceBeamBeginDivided.v[0];
    clipSpaceFlatDelta.v[1] = clipSpaceBeamEndDivided.v[1] - clipSpaceBeamBeginDivided.v[1];
    clipSpaceFlatDelta.v[2] = clipSpaceBeamEndDivided.v[2] - clipSpaceBeamBeginDivided.v[2];
    clipSpaceFlatDelta.v[3] = 1.0 - clipSpaceBeamBeginDivided.v[3];
    *(_QWORD *)clipSpaceFlatDelta.v &= *(_QWORD *)g_keepXYW.v;
    *(_QWORD *)&clipSpaceFlatDelta.unitVec[2].packed &= *(_QWORD *)&g_keepXYW.unitVec[2].packed;
    v10.v[0] = (float)((float)((float)(clipSpaceFlatDelta.v[0] * invClipMtx->x.v[0])
        + (float)(clipSpaceFlatDelta.v[1] * invClipMtx->y.v[0]))
        + (float)(clipSpaceFlatDelta.v[2] * invClipMtx->z.v[0]))
        + (float)(clipSpaceFlatDelta.v[3] * invClipMtx->w.v[0]);
    v10.v[1] = (float)((float)((float)(clipSpaceFlatDelta.v[0] * invClipMtx->x.v[1])
        + (float)(clipSpaceFlatDelta.v[1] * invClipMtx->y.v[1]))
        + (float)(clipSpaceFlatDelta.v[2] * invClipMtx->z.v[1]))
        + (float)(clipSpaceFlatDelta.v[3] * invClipMtx->w.v[1]);
    v10.v[2] = (float)((float)((float)(clipSpaceFlatDelta.v[0] * invClipMtx->x.v[2])
        + (float)(clipSpaceFlatDelta.v[1] * invClipMtx->y.v[2]))
        + (float)(clipSpaceFlatDelta.v[2] * invClipMtx->z.v[2]))
        + (float)(clipSpaceFlatDelta.v[3] * invClipMtx->w.v[2]);
    v10.v[3] = (float)((float)((float)(clipSpaceFlatDelta.v[0] * invClipMtx->x.v[3])
        + (float)(clipSpaceFlatDelta.v[1] * invClipMtx->y.v[3]))
        + (float)(clipSpaceFlatDelta.v[2] * invClipMtx->z.v[3]))
        + (float)(clipSpaceFlatDelta.v[3] * invClipMtx->w.v[3]);
    worldSpaceFlatDelta = v10;
    *(_QWORD *)outFlatDelta->v = *(_QWORD *)g_keepXYZ.v & *(_QWORD *)v10.v;
    *(_QWORD *)&outFlatDelta->unitVec[2].packed = *(_QWORD *)&g_keepXYZ.unitVec[2].packed
        & *(_QWORD *)&worldSpaceFlatDelta.unitVec[2].packed;
    v8 = *outFlatDelta;
    if (Vec4LengthSq(v8.v) < 0.000002)
        return 0;
    v7 = 1.0
        / sqrtf(
            (float)((float)((float)(outFlatDelta->v[0] * outFlatDelta->v[0])
                + (float)(outFlatDelta->v[1] * outFlatDelta->v[1]))
                + (float)(outFlatDelta->v[2] * outFlatDelta->v[2]))
            + (float)(outFlatDelta->v[3] * outFlatDelta->v[3]));
    outFlatDelta->v[0] = outFlatDelta->v[0] * v7;
    outFlatDelta->v[1] = outFlatDelta->v[1] * v7;
    outFlatDelta->v[2] = outFlatDelta->v[2] * v7;
    outFlatDelta->v[3] = outFlatDelta->v[3] * v7;
    return 1;
}

bool __cdecl Vec4HomogenousClipBothZ(float4 *pt0, float4 *pt1)
{
    float4 coeffZGtW; // [esp+140h] [ebp-24h] BYREF
    float4 coeffZLt0; // [esp+150h] [ebp-14h] BYREF

    coeffZLt0.u[0] = 0;
    coeffZLt0.v[1] = 0.0f;
    coeffZLt0.v[2] = 1.0f;
    coeffZLt0.u[3] = 0;

    coeffZGtW.u[0] = 0;
    coeffZGtW.v[1] = 0.0;
    coeffZGtW.v[2] = -1.0f;
    coeffZGtW.v[3] = 1.0f;

    return Vec4HomogenousClipZW(pt0, pt1, &coeffZLt0) && Vec4HomogenousClipZW(pt0, pt1, &coeffZGtW);
}

bool __cdecl Vec4HomogenousClipZW(float4 *pt0, float4 *pt1, const float4 *coeffZW)
{
    float v4; // [esp+4h] [ebp-D0h]
    float v5; // [esp+8h] [ebp-CCh]
    float v6; // [esp+Ch] [ebp-C8h]
    float v7; // [esp+10h] [ebp-C4h]
    int v8; // [esp+14h] [ebp-C0h]
    int v9; // [esp+18h] [ebp-BCh]
    int v10; // [esp+1Ch] [ebp-B8h]
    int v11; // [esp+20h] [ebp-B4h]
    int v12; // [esp+24h] [ebp-B0h]
    int v13; // [esp+28h] [ebp-ACh]
    int v14; // [esp+2Ch] [ebp-A8h]
    int v15; // [esp+30h] [ebp-A4h]
    __int64 dist1Cmp; // [esp+3Ch] [ebp-98h]
    __int64 dist1Cmp_8; // [esp+44h] [ebp-90h]
    __int64 clipped; // [esp+50h] [ebp-84h]
    __int64 clipped_8; // [esp+58h] [ebp-7Ch]
    __int64 dist1Sel1; // [esp+70h] [ebp-64h]
    __int64 dist1Sel1_8; // [esp+78h] [ebp-5Ch]
    float dist1_12; // [esp+8Ch] [ebp-48h]
    __int64 dist0Cmp; // [esp+90h] [ebp-44h]
    __int64 dist0Cmp_8; // [esp+98h] [ebp-3Ch]
    float dist0_12; // [esp+ACh] [ebp-28h]
    float alpha; // [esp+B0h] [ebp-24h]
    float alpha_4; // [esp+B4h] [ebp-20h]
    float alpha_8; // [esp+B8h] [ebp-1Ch]
    float alpha_12; // [esp+BCh] [ebp-18h]
    __int64 dist1Sel0_8; // [esp+C8h] [ebp-Ch]

    dist0_12 = (float)((float)((float)(coeffZW->v[0] * pt0->v[0]) + (float)(coeffZW->v[1] * pt0->v[1])) + (float)(coeffZW->v[2] * pt0->v[2])) + (float)(coeffZW->v[3] * pt0->v[3]);
    dist1_12 = (float)((float)((float)(coeffZW->v[0] * pt1->v[0]) + (float)(coeffZW->v[1] * pt1->v[1])) + (float)(coeffZW->v[2] * pt1->v[2])) + (float)(coeffZW->v[3] * pt1->v[3]);
    if ( dist0_12 >= 0.0 )
        v15 = 0;
    else
        v15 = -1;
    LODWORD(dist0Cmp) = v15;
    if ( dist0_12 >= 0.0 )
        v14 = 0;
    else
        v14 = -1;
    HIDWORD(dist0Cmp) = v14;
    if ( dist0_12 >= 0.0 )
        v13 = 0;
    else
        v13 = -1;
    LODWORD(dist0Cmp_8) = v13;
    if ( dist0_12 >= 0.0 )
        v12 = 0;
    else
        v12 = -1;
    HIDWORD(dist0Cmp_8) = v12;
    alpha = dist0_12 - dist1_12;
    alpha_4 = dist0_12 - dist1_12;
    alpha_8 = dist0_12 - dist1_12;
    alpha_12 = dist0_12 - dist1_12;
    if ( dist1_12 >= 0.0 )
        v11 = 0;
    else
        v11 = -1;
    LODWORD(dist1Cmp) = v11;
    if ( dist1_12 >= 0.0 )
        v10 = 0;
    else
        v10 = -1;
    HIDWORD(dist1Cmp) = v10;
    if ( dist1_12 >= 0.0 )
        v9 = 0;
    else
        v9 = -1;
    LODWORD(dist1Cmp_8) = v9;
    if ( dist1_12 >= 0.0 )
        v8 = 0;
    else
        v8 = -1;
    HIDWORD(dist1Cmp_8) = v8;
    if ( alpha == 0.0 )
        v7 = FLT_MAX;
    else
        v7 = I_fres(alpha);
    if ( alpha_4 == 0.0 )
        v6 = FLT_MAX;
    else
        v6 = I_fres(alpha_4);
    if ( alpha_8 == 0.0 )
        v5 = FLT_MAX;
    else
        v5 = I_fres(alpha_8);
    if ( alpha_12 == 0.0 )
        v4 = FLT_MAX;
    else
        v4 = I_fres(alpha_12);
    *(float *)&clipped = (float)((float)(dist0_12 * v7) * (float)(pt1->v[0] - pt0->v[0])) + pt0->v[0];
    *((float *)&clipped + 1) = (float)((float)(dist0_12 * v6) * (float)(pt1->v[1] - pt0->v[1])) + pt0->v[1];
    *(float *)&clipped_8 = (float)((float)(dist0_12 * v5) * (float)(pt1->v[2] - pt0->v[2])) + pt0->v[2];
    *((float *)&clipped_8 + 1) = (float)((float)(dist0_12 * v4) * (float)(pt1->v[3] - pt0->v[3])) + pt0->v[3];
    dist1Sel0_8 = *(_QWORD *)&pt0->unitVec[2].packed & dist1Cmp_8 | clipped_8 & ~dist1Cmp_8;
    dist1Sel1 = clipped & dist1Cmp | *(_QWORD *)pt1->v & ~dist1Cmp;
    dist1Sel1_8 = clipped_8 & dist1Cmp_8 | *(_QWORD *)&pt1->unitVec[2].packed & ~dist1Cmp_8;
    *(_QWORD *)pt0->v = (*(_QWORD *)pt0->v & dist1Cmp | clipped & ~dist1Cmp) & dist0Cmp | *(_QWORD *)pt0->v & ~dist0Cmp;
    *(_QWORD *)&pt0->unitVec[2].packed = dist1Sel0_8 & dist0Cmp_8 | *(_QWORD *)&pt0->unitVec[2].packed & ~dist0Cmp_8;
    *(_QWORD *)pt1->v = *(_QWORD *)pt1->v & dist0Cmp | dist1Sel1 & ~dist0Cmp;
    *(_QWORD *)&pt1->unitVec[2].packed = *(_QWORD *)&pt1->unitVec[2].packed & dist0Cmp_8 | dist1Sel1_8 & ~dist0Cmp_8;
    return (v11 & v15) == 0;
}

double __cdecl I_fres(float val)
{
    iassert(val);
    return 1.0 / val;
}

void __cdecl FX_Beam_Begin()
{
    g_beamInfo.beamCount = 0;
}

void __cdecl FX_Beam_Add(FxBeam *beam)
{
    Sys_EnterCriticalSection(CRITSECT_FXBEAM);
    if (g_beamInfo.beamCount != 96)
    {
        g_beamInfo.beams[g_beamInfo.beamCount++] = *beam;
    }
    Sys_LeaveCriticalSection(CRITSECT_FXBEAM);
}

FxBeamInfo *__cdecl FX_Beam_GetInfo()
{
    return &g_beamInfo;
}

