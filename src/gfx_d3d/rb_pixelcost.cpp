#include "rb_pixelcost.h"
#include "r_init.h"
#include "rb_corona.h"
#include "r_foliage.h"

struct $24C327571DE1B1A743908CE1FB1197BD // sizeof=0x10030
{                                       // XREF: .data:pixelCostGlob/r
    GfxPixelCostMode savedMode;
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    long double msecOverhead;
    unsigned __int64 timeBegin;
    long double msecElapsed;            // XREF: R_PixelCost_EndSurface(GfxCmdBufContext)+A3/r
                                        // RB_PixelCost_AccumulateMsec+6/r
    int frameIndex;                     // XREF: R_PixelCost_EndSurface(GfxCmdBufContext)+11F/r
                                        // RB_PixelCost_AccumulateMsec+64/r
    int expectedCount;                  // XREF: R_PixelCost_BeginSurface(GfxCmdBufContext):loc_B08CDD/r
                                        // R_PixelCost_BeginSurface(GfxCmdBufContext)+76/w ...
    int recordCount;                    // XREF: R_PixelCost_BeginSurface(GfxCmdBufContext):loc_B08DB4/r
                                        // RB_PixelCost_DoesPrimMatch+6/r ...
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    GfxPixelCostRecord records[2048];   // XREF: RB_PixelCost_GetCostForRecordIndex+35/r
                                        // RB_PixelCost_ResetPrim+C/o ...
};

$24C327571DE1B1A743908CE1FB1197BD pixelCostGlob;

GfxPixelCostMode pixelCostMode;

const Material *__cdecl R_PixelCost_GetAccumulationMaterial(const Material *material)
{
    const Material *result; // eax
    const char *v2; // eax
    int v3; // [esp+4h] [ebp-8h]

    if ( pixelCostMode == GFX_PIXEL_COST_MODE_ADD_COST_IGNORE_DEPTH )
        return rgp.pixelCostAddDepthDisableMaterial;
    if ( pixelCostMode == GFX_PIXEL_COST_MODE_ADD_PASSES_IGNORE_DEPTH )
        return rgp.pixelCostAddDepthDisableMaterial;
    if ( material->localTechniqueSet->techniques[4] )
        v3 = material->stateBitsEntry[4];
    else
        v3 = 0;
    switch ( material->stateBitsTable[v3].loadBits[1] & 0xF )
    {
        case 0u:
        case 1u:
            result = rgp.pixelCostAddDepthAlwaysMaterial;
            break;
        case 2u:
        case 3u:
            result = rgp.pixelCostAddDepthDisableMaterial;
            break;
        case 4u:
        case 5u:
            result = rgp.pixelCostAddDepthLessMaterial;
            break;
        case 6u:
        case 7u:
            result = rgp.pixelCostAddDepthDisableMaterial;
            break;
        case 8u:
        case 9u:
            result = rgp.pixelCostAddDepthEqualMaterial;
            break;
        case 0xAu:
        case 0xBu:
            result = rgp.pixelCostAddDepthDisableMaterial;
            break;
        case 0xCu:
            result = rgp.pixelCostAddNoDepthWriteMaterial;
            break;
        case 0xDu:
            result = rgp.pixelCostAddDepthWriteMaterial;
            break;
        case 0xEu:
        case 0xFu:
            result = rgp.pixelCostAddDepthDisableMaterial;
            break;
        default:
            v2 = va("unhandled case %i", material->stateBitsTable[v3].loadBits[1] & 0xF);
            if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_pixelcost.cpp", 185, 1, v2) )
                __debugbreak();
            result = rgp.pixelCostAddNoDepthWriteMaterial;
            break;
    }
    return result;
}

void __cdecl R_PixelCost_BeginSurface(GfxCmdBufContext context)
{
    int cost; // [esp+4h] [ebp-Ch]
    unsigned __int64 packedKey; // [esp+8h] [ebp-8h]
    unsigned __int64 packedKeya; // [esp+8h] [ebp-8h]

    if ( pixelCostMode == GFX_PIXEL_COST_MODE_MEASURE_COST )
    {
        packedKey = R_PixelCost_PackedKeyForMaterial(context.state->material, context.state->techType);
        if ( !RB_PixelCost_DoesPrimMatch(packedKey) )
            RB_PixelCost_ResetPrim(packedKey);
        ++pixelCostGlob.expectedCount;
        //BLOPS_NULLSUB();
        RB_HW_BeginOcclusionQuery(gfxAssets.pixelCountQuery[r_glob.backEndFrameCount % 4]);
    }
    else if ( pixelCostMode == GFX_PIXEL_COST_MODE_MEASURE_MSEC )
    {
        packedKeya = R_PixelCost_PackedKeyForMaterial(context.state->material, context.state->techType);
        if ( !RB_PixelCost_DoesPrimMatch(packedKeya) )
            RB_PixelCost_ResetPrim(packedKeya);
        ++pixelCostGlob.expectedCount;
        //BLOPS_NULLSUB();
    }
    else
    {
        if ( pixelCostMode <= GFX_PIXEL_COST_MODE_MEASURE_MSEC
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_pixelcost.cpp",
                        353,
                        0,
                        "%s",
                        "RB_PixelCost_IsAccumulating()") )
        {
            __debugbreak();
        }
        cost = RB_PixelCost_GetCostForRecordIndex(pixelCostGlob.recordCount);
        R_PixelCost_SetConstant(context.source, cost);
    }
}

void __cdecl R_PixelCost_SetConstant(GfxCmdBufSourceState *source, int cost)
{
    float weights[4]; // [esp+4h] [ebp-10h] BYREF

    if ( &gfxAssets )
    {
        weights[0] = (float)((float)(cost >> 6) + 0.0099999998) * 0.0039215689;
        weights[1] = (float)((float)((cost >> 4) & 3) + 0.0099999998) * 0.0039215689;
        weights[2] = (float)((float)((cost >> 2) & 3) + 0.0099999998) * 0.0039215689;
        weights[3] = (float)((float)(cost & 3) + 0.0099999998) * 0.0039215689;
    }
    else
    {
        weights[0] = 0.094156861f;
        weights[1] = 0.031411767f;
        weights[2] = 0.0f;
        weights[3] = 0.0039607841f;
    }
    R_SetCodeConstantFromVec4(source, CONST_SRC_CODE_PIXEL_COST_FRACS, weights);
}

int __cdecl RB_PixelCost_GetCostForRecordIndex(int recordIndex)
{
    __int64 v2; // rax
    int v3; // [esp+0h] [ebp-64h]
    int v4; // [esp+4h] [ebp-60h]
    int v5; // [esp+8h] [ebp-5Ch]
    int avgCost; // [esp+10h] [ebp-54h]
    int cost; // [esp+14h] [ebp-50h]
    int frameIndex; // [esp+18h] [ebp-4Ch]
    int frameIndexa; // [esp+18h] [ebp-4Ch]
    int frameIndexb; // [esp+18h] [ebp-4Ch]
    int totalCost; // [esp+1Ch] [ebp-48h]
    int validCount; // [esp+20h] [ebp-44h]
    double costDelta; // [esp+24h] [ebp-40h]
    double standardDeviationSum; // [esp+2Ch] [ebp-38h]
    int costHistory[12]; // [esp+34h] [ebp-30h]

    totalCost = 0;
    validCount = 0;
    for ( frameIndex = 0; frameIndex < 12; ++frameIndex )
    {
        cost = pixelCostGlob.records[recordIndex].costHistory[frameIndex];
        if ( pixelCostGlob.records[recordIndex].costHistory[frameIndex] )
        {
            totalCost += cost;
            costHistory[validCount++] = cost;
        }
    }
    if ( validCount == 1 )
        return totalCost;
    if ( validCount == 2 )
    {
        if ( costHistory[0] < costHistory[1] )
            v4 = costHistory[1];
        else
            v4 = costHistory[0];
        return totalCost - v4;
    }
    else
    {
        if ( validCount <= 0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_pixelcost.cpp",
                        220,
                        0,
                        "%s\n\t(validCount) = %i",
                        "(validCount > 0)",
                        validCount) )
        {
            __debugbreak();
        }
        standardDeviationSum = 0.0;
        avgCost = totalCost / validCount;
        for ( frameIndexa = 0; frameIndexa < validCount; ++frameIndexa )
        {
            costDelta = (double)(costHistory[frameIndexa] - avgCost);
            standardDeviationSum = costDelta * costDelta + standardDeviationSum;
        }
        v5 = (int)(sqrt(standardDeviationSum / (double)validCount) * 1.5);
        if ( v5 > 10 )
            v3 = v5;
        else
            v3 = 10;
        for ( frameIndexb = validCount - 1; frameIndexb >= 0; --frameIndexb )
        {
            v2 = costHistory[frameIndexb] - avgCost;
            if ( (double)(int)((HIDWORD(v2) ^ v2) - HIDWORD(v2)) > (double)v3 )
            {
                totalCost -= costHistory[frameIndexb];
                --validCount;
            }
        }
        if ( validCount < 2
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_pixelcost.cpp",
                        240,
                        0,
                        "%s\n\t(validCount) = %i",
                        "(validCount >= 2)",
                        validCount) )
        {
            __debugbreak();
        }
        return totalCost / validCount;
    }
}

unsigned __int64 __cdecl R_PixelCost_PackedKeyForMaterial(const Material *material, unsigned __int8 techType)
{
    unsigned int key_4; // [esp+4h] [ebp-4h]

    if ( !material
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_pixelcost.cpp", 254, 0, "%s", "material") )
    {
        __debugbreak();
    }
    LOBYTE(key_4) = techType;
    return __PAIR64__(key_4, (unsigned int)material);
}

bool __cdecl RB_PixelCost_DoesPrimMatch(unsigned __int64 packedKey)
{
    return __PAIR64__(
                     HIDWORD(pixelCostGlob.records[pixelCostGlob.recordCount].key.packed),
                     pixelCostGlob.records[pixelCostGlob.recordCount].key.mtl.material) == packedKey;
}

void __cdecl RB_PixelCost_ResetPrim(unsigned __int64 packedKey)
{
    GfxPixelCostRecord *record; // [esp+0h] [ebp-4h]

    record = &pixelCostGlob.records[pixelCostGlob.expectedCount];
    record->key.packed = packedKey;
    *(unsigned int *)record->costHistory = 0;
    *(unsigned int *)&record->costHistory[2] = 0;
    *(unsigned int *)&record->costHistory[4] = 0;
    *(unsigned int *)&record->costHistory[6] = 0;
    *(unsigned int *)&record->costHistory[8] = 0;
    *(unsigned int *)&record->costHistory[10] = 0;
}

void __cdecl R_PixelCost_EndSurface(GfxCmdBufContext context)
{
    unsigned __int16 v1; // [esp+10h] [ebp-38h]
    int v2; // [esp+14h] [ebp-34h]
    int v3; // [esp+28h] [ebp-20h]
    int cost; // [esp+40h] [ebp-8h]
    unsigned int pixelCount; // [esp+44h] [ebp-4h]

    if ( pixelCostMode == GFX_PIXEL_COST_MODE_MEASURE_COST )
    {
        gfxAssets.pixelCountQuery[r_glob.backEndFrameCount % 4]->Issue(D3DISSUE_END);
        //BLOPS_NULLSUB(); RB_PixelCost_EndTiming()
        pixelCount = RB_HW_ReadOcclusionQuery(gfxAssets.pixelCountQuery[r_glob.backEndFrameCount % 4]);
        if ( pixelCount )
        {
            //v3 = (int)ceil(
            //    (double)(LODWORD(context.source[1].matrices.matrix[0].m[2][1])
            //        * LODWORD(context.source[1].matrices.matrix[0].m[2][0]))
            //    * pixelCostGlob.msecElapsed
            //    / (double)pixelCount
            //    * 30.72);
            v3 = (int)ceil(
                (double)(context.source->renderTargetHeight * context.source->renderTargetWidth)
                * pixelCostGlob.msecElapsed
                / (double)pixelCount
                * 30.72);
            if ( v3 > 1 )
                v2 = v3;
            else
                v2 = 1;
            cost = v2;
        }
        else
        {
            cost = 1;
        }
        if ( cost > 0xFFFF )
            v1 = -1;
        else
            v1 = cost;
        pixelCostGlob.records[pixelCostGlob.recordCount++].costHistory[pixelCostGlob.frameIndex] = v1;
    }
    else if ( pixelCostMode == GFX_PIXEL_COST_MODE_MEASURE_MSEC )
    {
        //BLOPS_NULLSUB();
        RB_PixelCost_AccumulateMsec();
    }
    else
    {
        if ( pixelCostMode <= GFX_PIXEL_COST_MODE_MEASURE_MSEC
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_pixelcost.cpp",
                        393,
                        0,
                        "%s",
                        "RB_PixelCost_IsAccumulating()") )
        {
            __debugbreak();
        }
        ++pixelCostGlob.recordCount;
    }
}

int RB_PixelCost_AccumulateMsec()
{
    int result; // eax
    unsigned __int16 v1; // [esp+8h] [ebp-Ch]
    int v2; // [esp+Ch] [ebp-8h]
    int timeQuantized; // [esp+10h] [ebp-4h]

    timeQuantized = (int)ceil(pixelCostGlob.msecElapsed * 3932.1);
    if ( timeQuantized < 0xFFFF )
        v2 = timeQuantized;
    else
        v2 = 0xFFFF;
    if ( v2 > 1 )
        v1 = v2;
    else
        v1 = 1;
    result = pixelCostGlob.frameIndex;
    pixelCostGlob.records[pixelCostGlob.recordCount++].costHistory[pixelCostGlob.frameIndex] = v1;
    return result;
}

GfxRenderTargetId __cdecl RB_PixelCost_OverrideRenderTarget(GfxRenderTargetId targetId)
{
    if (targetId < R_RENDERTARGET_SCENE)
        return R_RENDERTARGET_FRAME_BUFFER;

    if ( targetId >= R_RENDERTARGET_POST_EFFECT_SRC)
        return targetId;

    return R_RENDERTARGET_SCENE;
}

