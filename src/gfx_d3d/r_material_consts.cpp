#include "r_material_consts.h"
#include <universal/q_shared.h>

void __cdecl R_GetPixelLiteralConsts(
                const Material *mtl,
                const MaterialPass *pass,
                GfxShaderConstantBlock *pixelLiteralConsts)
{
    const char *v3; // eax
    MaterialConstantDef *constDef; // [esp+0h] [ebp-Ch]
    unsigned int argCount; // [esp+4h] [ebp-8h]
    const MaterialShaderArgument *arg; // [esp+8h] [ebp-4h]

    pixelLiteralConsts->count = 0;
    argCount = pass->stableArgCount;
    if ( pass->stableArgCount )
    {
        for ( arg = &pass->localArgs[pass->perPrimArgCount + pass->perObjArgCount]; arg->type < 6u; ++arg )
        {
            if ( !--argCount )
                return;
        }
        constDef = mtl->localConstantTable;
        while ( arg->type == 6 )
        {
            while ( constDef->nameHash != arg->u.codeSampler )
            {
                if ( ++constDef == &mtl->localConstantTable[mtl->constantCount] )
                {
                    v3 = va("material '%s' is missing a required named constant", mtl->info.name);
                    if ( !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_consts.cpp",
                                    59,
                                    0,
                                    "%s\n\t%s",
                                    "constDef != &mtl->constantTable[mtl->constantCount]",
                                    v3) )
                        __debugbreak();
                }
            }
            R_RegisterShaderConst(arg->dest, constDef->literal, pixelLiteralConsts);
            ++arg;
            if ( !--argCount )
                return;
        }
        do
        {
            if ( arg->type != 7 )
                break;
            R_RegisterShaderConst(arg->dest, arg->u.literalConst, pixelLiteralConsts);
            ++arg;
            --argCount;
        }
        while ( argCount );
    }
}

void __cdecl R_RegisterShaderConst(unsigned int dest, const float *value, GfxShaderConstantBlock *consts)
{
    unsigned int sortedIndex; // [esp+4h] [ebp-4h]

    if ( consts->count >= 0x20
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_consts.cpp",
                    16,
                    1,
                    "consts->count doesn't index ARRAY_COUNT( consts->dest )\n\t%i not in [0, %i)",
                    consts->count,
                    32) )
    {
        __debugbreak();
    }
    for ( sortedIndex = consts->count;
                sortedIndex && *((unsigned __int16 *)&consts->count + sortedIndex + 1) > dest;
                consts->value[sortedIndex + 1] = consts->value[sortedIndex] )
    {
        --sortedIndex;
        consts->dest[sortedIndex + 1] = consts->dest[sortedIndex];
    }
    consts->dest[sortedIndex] = dest;
    consts->value[sortedIndex] = value;
    ++consts->count;
}

int __cdecl R_ComparePixelConsts(const Material **material, const MaterialPass **pass)
{
    int j; // [esp+4h] [ebp-5ACh]
    GfxShaderConstantBlock pixelLiteralConsts[2]; // [esp+8h] [ebp-5A8h] BYREF
    unsigned __int16 pixelConsts[2][256]; // [esp+190h] [ebp-420h] BYREF
    unsigned int argCount; // [esp+594h] [ebp-1Ch]
    const MaterialShaderArgument *arg; // [esp+598h] [ebp-18h]
    int i; // [esp+59Ch] [ebp-14h]
    int comparison; // [esp+5A0h] [ebp-10h]
    unsigned int constIndex; // [esp+5A4h] [ebp-Ch]
    unsigned int pixelConstsCount[2]; // [esp+5A8h] [ebp-8h]

    for ( i = 0; i < 2; ++i )
    {
        pixelConstsCount[i] = 0;
        pixelLiteralConsts[i].count = 0;
        arg = &pass[i]->localArgs[pass[i]->perPrimArgCount + pass[i]->perObjArgCount];
        argCount = pass[i]->stableArgCount;
        if ( argCount )
        {
            while ( arg->type < 5u )
            {
                ++arg;
                if ( !--argCount )
                    goto done_3;
            }
            while ( arg->type == 5 )
            {
                if ( pixelConstsCount[i] >= 0x100
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_material_consts.cpp",
                                148,
                                0,
                                "pixelConstsCount[i] doesn't index ARRAY_COUNT( pixelConsts[i] )\n\t%i not in [0, %i)",
                                pixelConstsCount[i],
                                256) )
                {
                    __debugbreak();
                }
                pixelConsts[i][pixelConstsCount[i]++] = arg->u.codeConst.index;
                ++arg;
                if ( !--argCount )
                    goto done_3;
            }
            R_GetPixelLiteralConsts(material[i], pass[i], &pixelLiteralConsts[i]);
        }
done_3:
        ;
    }
    comparison = pixelConstsCount[0] - pixelConstsCount[1];
    if ( pixelConstsCount[0] != pixelConstsCount[1] )
        return comparison;
    for ( constIndex = 0; constIndex < pixelConstsCount[0]; ++constIndex )
    {
        comparison = pixelConsts[0][constIndex] - pixelConsts[1][constIndex];
        if ( comparison )
            return comparison;
    }
    comparison = pixelLiteralConsts[0].count - pixelLiteralConsts[1].count;
    if ( pixelLiteralConsts[0].count != pixelLiteralConsts[1].count )
        return comparison;
    for ( constIndex = 0; constIndex < pixelLiteralConsts[0].count; ++constIndex )
    {
        comparison = pixelLiteralConsts[0].dest[constIndex] - pixelLiteralConsts[1].dest[constIndex];
        if ( comparison )
            return comparison;
        for ( j = 0; j < 4; ++j )
        {
            if ( pixelLiteralConsts[1].value[constIndex][j] > pixelLiteralConsts[0].value[constIndex][j] )
                return -1;
            if ( pixelLiteralConsts[0].value[constIndex][j] > pixelLiteralConsts[1].value[constIndex][j] )
                return 1;
        }
    }
    return 0;
}

