#include "r_draw_method.h"
#include "r_dvars.h"

GfxDrawMethod gfxDrawMethod;

void __cdecl R_InitDrawMethod()
{
    if ( r_fullbright->current.enabled )
    {
        gfxDrawMethod.drawScene = GFX_DRAW_SCENE_FULLBRIGHT;
        gfxDrawMethod.baseTechType = 4;
        gfxDrawMethod.emissiveTechType = 4;
        R_ForceLitTechType(4u);
    }
    else if ( r_debugShader->current.integer )
    {
        gfxDrawMethod.drawScene = GFX_DRAW_SCENE_DEBUGSHADER;
        gfxDrawMethod.baseTechType = 127;
        gfxDrawMethod.emissiveTechType = 127;
        R_ForceLitTechType(0x7Fu);
    }
    else
    {
        gfxDrawMethod.drawScene = GFX_DRAW_SCENE_STANDARD;
        gfxDrawMethod.baseTechType = 10;
        gfxDrawMethod.emissiveTechType = 5;
        R_SetDefaultLitTechTypes();
    }
}

void R_SetDefaultLitTechTypes()
{
    memset(gfxDrawMethod.litTechType[0][0][0], 0xFFu, 0x1C0u);
    gfxDrawMethod.litTechType[0][0][0][0] = 10;
    gfxDrawMethod.litTechType[0][1][0][0] = 11;
    gfxDrawMethod.litTechType[0][2][0][0] = 13;
    gfxDrawMethod.litTechType[0][3][0][0] = 15;
    gfxDrawMethod.litTechType[0][4][0][0] = 12;
    gfxDrawMethod.litTechType[0][5][0][0] = 14;
    gfxDrawMethod.litTechType[0][6][0][0] = 16;
    gfxDrawMethod.litTechType[0][0][1][0] = 52;
    gfxDrawMethod.litTechType[0][1][1][0] = 53;
    gfxDrawMethod.litTechType[0][2][1][0] = 55;
    gfxDrawMethod.litTechType[0][3][1][0] = 57;
    gfxDrawMethod.litTechType[0][4][1][0] = 54;
    gfxDrawMethod.litTechType[0][5][1][0] = 56;
    gfxDrawMethod.litTechType[0][6][1][0] = 58;
    gfxDrawMethod.litTechType[1][0][0][0] = 10;
    gfxDrawMethod.litTechType[1][1][0][0] = 11;
    gfxDrawMethod.litTechType[1][2][0][0] = 13;
    gfxDrawMethod.litTechType[1][3][0][0] = 15;
    gfxDrawMethod.litTechType[1][4][0][0] = 12;
    gfxDrawMethod.litTechType[1][5][0][0] = 14;
    gfxDrawMethod.litTechType[1][6][0][0] = 16;
    gfxDrawMethod.litTechType[1][0][1][0] = 52;
    gfxDrawMethod.litTechType[1][1][1][0] = 53;
    gfxDrawMethod.litTechType[1][2][1][0] = 55;
    gfxDrawMethod.litTechType[1][3][1][0] = 57;
    gfxDrawMethod.litTechType[1][4][1][0] = 54;
    gfxDrawMethod.litTechType[1][5][1][0] = 56;
    gfxDrawMethod.litTechType[1][6][1][0] = 58;
    *(_WORD *)&gfxDrawMethod.litTechType[2][0][0][0] = 15114;
    *(_WORD *)&gfxDrawMethod.litTechType[2][1][0][0] = 15627;
    *(_WORD *)&gfxDrawMethod.litTechType[2][2][0][0] = 16653;
    *(_WORD *)&gfxDrawMethod.litTechType[2][3][0][0] = 17679;
    *(_WORD *)&gfxDrawMethod.litTechType[2][4][0][0] = 16140;
    *(_WORD *)&gfxDrawMethod.litTechType[2][5][0][0] = 17166;
    *(_WORD *)&gfxDrawMethod.litTechType[2][6][0][0] = 18192;
    gfxDrawMethod.litTechType[3][0][0][0] = 73;
    gfxDrawMethod.litTechType[3][1][0][0] = 74;
    gfxDrawMethod.litTechType[3][2][0][0] = 76;
    gfxDrawMethod.litTechType[3][3][0][0] = 78;
    gfxDrawMethod.litTechType[3][4][0][0] = 75;
    gfxDrawMethod.litTechType[3][5][0][0] = 77;
    gfxDrawMethod.litTechType[3][6][0][0] = 79;
    gfxDrawMethod.litTechType[4][0][0][0] = 73;
    gfxDrawMethod.litTechType[4][1][0][0] = 74;
    gfxDrawMethod.litTechType[4][2][0][0] = 76;
    gfxDrawMethod.litTechType[4][3][0][0] = 78;
    gfxDrawMethod.litTechType[4][4][0][0] = 75;
    gfxDrawMethod.litTechType[4][5][0][0] = 77;
    gfxDrawMethod.litTechType[4][6][0][0] = 79;
    gfxDrawMethod.litTechType[5][0][0][0] = 10;
    gfxDrawMethod.litTechType[5][1][0][0] = 11;
    gfxDrawMethod.litTechType[5][2][0][0] = 13;
    gfxDrawMethod.litTechType[5][3][0][0] = 15;
    gfxDrawMethod.litTechType[5][4][0][0] = 12;
    gfxDrawMethod.litTechType[5][5][0][0] = 14;
    gfxDrawMethod.litTechType[5][6][0][0] = 16;
    gfxDrawMethod.litTechType[6][0][0][0] = 10;
    gfxDrawMethod.litTechType[6][1][0][0] = 11;
    gfxDrawMethod.litTechType[6][2][0][0] = 13;
    gfxDrawMethod.litTechType[6][3][0][0] = 15;
    gfxDrawMethod.litTechType[6][4][0][0] = 12;
    gfxDrawMethod.litTechType[6][5][0][0] = 14;
    gfxDrawMethod.litTechType[6][6][0][0] = 16;
    gfxDrawMethod.litTechType[6][0][1][0] = 52;
    gfxDrawMethod.litTechType[6][1][1][0] = 53;
    gfxDrawMethod.litTechType[6][2][1][0] = 55;
    gfxDrawMethod.litTechType[6][3][1][0] = 57;
    gfxDrawMethod.litTechType[6][4][1][0] = 54;
    gfxDrawMethod.litTechType[6][5][1][0] = 56;
    gfxDrawMethod.litTechType[6][6][1][0] = 58;
    *(_WORD *)&gfxDrawMethod.litTechType[7][0][0][0] = 15114;
    *(_WORD *)&gfxDrawMethod.litTechType[7][1][0][0] = 15627;
    *(_WORD *)&gfxDrawMethod.litTechType[7][2][0][0] = 16653;
    *(_WORD *)&gfxDrawMethod.litTechType[7][3][0][0] = 17679;
    *(_WORD *)&gfxDrawMethod.litTechType[7][4][0][0] = 16140;
    *(_WORD *)&gfxDrawMethod.litTechType[7][5][0][0] = 17166;
    *(_WORD *)&gfxDrawMethod.litTechType[7][6][0][0] = 18192;
    gfxDrawMethod.litTechType[8][0][0][0] = 10;
    gfxDrawMethod.litTechType[8][1][0][0] = 11;
    gfxDrawMethod.litTechType[8][2][0][0] = 13;
    gfxDrawMethod.litTechType[8][3][0][0] = 15;
    gfxDrawMethod.litTechType[8][4][0][0] = 12;
    gfxDrawMethod.litTechType[8][5][0][0] = 14;
    gfxDrawMethod.litTechType[8][6][0][0] = 16;
    *(_WORD *)&gfxDrawMethod.litTechType[9][0][0][0] = 15114;
    *(_WORD *)&gfxDrawMethod.litTechType[9][1][0][0] = 15627;
    *(_WORD *)&gfxDrawMethod.litTechType[9][2][0][0] = 16653;
    *(_WORD *)&gfxDrawMethod.litTechType[9][3][0][0] = 17679;
    *(_WORD *)&gfxDrawMethod.litTechType[9][4][0][0] = 16140;
    *(_WORD *)&gfxDrawMethod.litTechType[9][5][0][0] = 17166;
    *(_WORD *)&gfxDrawMethod.litTechType[9][6][0][0] = 18192;
    gfxDrawMethod.litTechType[12][0][0][0] = 10;
    gfxDrawMethod.litTechType[12][1][0][0] = 11;
    gfxDrawMethod.litTechType[12][2][0][0] = 13;
    gfxDrawMethod.litTechType[12][3][0][0] = 15;
    gfxDrawMethod.litTechType[12][4][0][0] = 12;
    gfxDrawMethod.litTechType[12][5][0][0] = 14;
    gfxDrawMethod.litTechType[12][6][0][0] = 16;
    gfxDrawMethod.litTechType[10][0][0][0] = 10;
    gfxDrawMethod.litTechType[10][1][0][0] = 11;
    gfxDrawMethod.litTechType[10][2][0][0] = 13;
    gfxDrawMethod.litTechType[10][3][0][0] = 15;
    gfxDrawMethod.litTechType[10][4][0][0] = 12;
    gfxDrawMethod.litTechType[10][5][0][0] = 14;
    gfxDrawMethod.litTechType[10][6][0][0] = 16;
    gfxDrawMethod.litTechType[14][0][0][0] = 10;
    gfxDrawMethod.litTechType[14][1][0][0] = 11;
    gfxDrawMethod.litTechType[14][2][0][0] = 13;
    gfxDrawMethod.litTechType[14][3][0][0] = 15;
    gfxDrawMethod.litTechType[14][4][0][0] = 12;
    gfxDrawMethod.litTechType[14][5][0][0] = 14;
    gfxDrawMethod.litTechType[14][6][0][0] = 16;
    gfxDrawMethod.litTechType[15][0][0][0] = 10;
    gfxDrawMethod.litTechType[15][1][0][0] = 11;
    gfxDrawMethod.litTechType[15][2][0][0] = 13;
    gfxDrawMethod.litTechType[15][3][0][0] = 15;
    gfxDrawMethod.litTechType[15][4][0][0] = 12;
    gfxDrawMethod.litTechType[15][5][0][0] = 14;
    gfxDrawMethod.litTechType[15][6][0][0] = 16;
}

void __cdecl R_ForceLitTechType(unsigned __int8 litTechType)
{
    memset(gfxDrawMethod.litTechType[0][0][0], litTechType, 0x1C0u);
}

void __cdecl R_UpdateDrawMethod(GfxBackEndData *data)
{
    bool v1; // [esp+7h] [ebp-21h]
    bool v2; // [esp+8h] [ebp-20h]
    GfxFadeType fadeType; // [esp+Ch] [ebp-1Ch]
    GfxTechCharredType charType; // [esp+14h] [ebp-14h]
    unsigned int primaryLightIndex; // [esp+1Ch] [ebp-Ch]
    unsigned int surfType; // [esp+20h] [ebp-8h]
    unsigned int lightTypea; // [esp+24h] [ebp-4h]
    unsigned int lightType; // [esp+24h] [ebp-4h]

    if ( data->shadowableLightCount > 0xFF
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_draw_method.cpp",
                    232,
                    0,
                    "%s\n\t(data->shadowableLightCount) = %i",
                    "(data->shadowableLightCount <= 255)",
                    data->shadowableLightCount) )
    {
        __debugbreak();
    }
    for ( primaryLightIndex = 0; primaryLightIndex < data->shadowableLightCount; ++primaryLightIndex )
    {
        lightTypea = data->shadowableLights[primaryLightIndex].type;
        lightType = lightTypea + (Com_BitCheckAssert(data->shadowableLightHasShadowMap, primaryLightIndex, 32) ? 3 : 0);
        for ( surfType = 0; surfType < 0x10; ++surfType )
        {
            v2 = surfType < 2 || surfType == 6;
            fadeType = (GfxFadeType)v2;
            v1 = surfType == 9 || surfType == 7 || surfType == 2;
            charType = (GfxTechCharredType)v1;
            data->primaryLightTechType[surfType][0][0][primaryLightIndex] = gfxDrawMethod.litTechType[surfType][lightType][0][0];
            data->primaryLightTechType[surfType][0][1][primaryLightIndex] = gfxDrawMethod.litTechType[surfType][lightType][0][charType];
            data->primaryLightTechType[surfType][1][0][primaryLightIndex] = gfxDrawMethod.litTechType[surfType][lightType][fadeType][0];
            data->primaryLightTechType[surfType][1][1][primaryLightIndex] = gfxDrawMethod.litTechType[surfType][lightType][fadeType][charType];
        }
    }
}

