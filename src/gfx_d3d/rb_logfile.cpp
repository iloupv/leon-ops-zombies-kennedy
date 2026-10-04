#include "rb_logfile.h"
#include <universal/dvar.h>
#include "r_dvars.h"
#include <cstdio>
#include <ctime>
#include <universal/assertive.h>
#include <universal/q_shared.h>

struct
{
    FILE *fp;
} r_logFileGlob;

void __cdecl RB_UpdateLogging()
{
    if ( RB_IsLogging() )
        Dvar_SetInt((dvar_s *)r_logFile, r_logFile->current.integer - 1);
    if ( r_logFile->current.integer )
        RB_OpenLogFile();
    else
        RB_CloseLogFile();
}

void RB_CloseLogFile()
{
    if ( r_logFileGlob.fp )
    {
        fclose(r_logFileGlob.fp);
        r_logFileGlob.fp = 0;
    }
}

void RB_OpenLogFile()
{
    const char *v0; // eax
    __int64 aclock; // [esp+0h] [ebp-10h] BYREF
    tm *newtime; // [esp+Ch] [ebp-4h]

    if ( !r_logFileGlob.fp )
    {
        r_logFileGlob.fp = fopen("dx.log", "wt");
        if ( r_logFileGlob.fp )
        {
            _time64(&aclock);
            newtime = _localtime64(&aclock);
            v0 = asctime(newtime);
            fprintf(r_logFileGlob.fp, "%s\n", v0);
            fflush(r_logFileGlob.fp);
        }
    }
}

bool __cdecl RB_IsLogging()
{
    return r_logFile->current.integer && r_logFileGlob.fp != 0;
}

void __cdecl RB_LogPrint(const char *text)
{
    if ( !r_logFile->current.integer
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_logfile.cpp",
                    102,
                    0,
                    "%s",
                    "r_logFile->current.integer") )
    {
        __debugbreak();
    }
    if ( RB_IsLogging() )
    {
        fprintf(r_logFileGlob.fp, "%s", text);
        fflush(r_logFileGlob.fp);
    }
}

const char *__cdecl RB_LogTechniqueType(unsigned __int8 techType)
{
    const char *techniqueNames[131]; // [esp+0h] [ebp-210h]

    techniqueNames[0] = "TECHNIQUE_DEPTH_PREPASS";
    techniqueNames[1] = "TECHNIQUE_BUILD_FLOAT_Z";
    techniqueNames[2] = "TECHNIQUE_BUILD_SHADOWMAP_DEPTH";
    techniqueNames[3] = "TECHNIQUE_BUILD_SHADOWMAP_COLOR";
    techniqueNames[4] = "TECHNIQUE_UNLIT";
    techniqueNames[5] = "TECHNIQUE_EMISSIVE";
    techniqueNames[6] = "TECHNIQUE_EMISSIVE_SHADOW";
    techniqueNames[7] = "TECHNIQUE_EMISSIVE_NV_INTZ";
    techniqueNames[8] = "TECHNIQUE_EMISSIVE_SHADOW_NV_INTZ";
    techniqueNames[9] = "TECHNIQUE_EMISSIVE_REFLECTED";
    techniqueNames[10] = "TECHNIQUE_LIT";
    techniqueNames[11] = "TECHNIQUE_LIT_SUN";
    techniqueNames[12] = "TECHNIQUE_LIT_SUN_SHADOW";
    techniqueNames[13] = "TECHNIQUE_LIT_SPOT";
    techniqueNames[14] = "TECHNIQUE_LIT_SPOT_SHADOW";
    techniqueNames[15] = "TECHNIQUE_LIT_OMNI";
    techniqueNames[16] = "TECHNIQUE_LIT_OMNI_SHADOW";
    techniqueNames[17] = "TECHNIQUE_LIT_DLIGHT";
    techniqueNames[18] = "TECHNIQUE_LIT_SUN_DLIGHT";
    techniqueNames[19] = "TECHNIQUE_LIT_SUN_SHADOW_DLIGHT";
    techniqueNames[20] = "TECHNIQUE_LIT_SPOT_DLIGHT";
    techniqueNames[21] = "TECHNIQUE_LIT_SPOT_SHADOW_DLIGHT";
    techniqueNames[22] = "TECHNIQUE_LIT_OMNI_DLIGHT";
    techniqueNames[23] = "TECHNIQUE_LIT_OMNI_SHADOW_DLIGHT";
    techniqueNames[24] = "TECHNIQUE_LIT_GLIGHT";
    techniqueNames[25] = "TECHNIQUE_LIT_SUN_GLIGHT";
    techniqueNames[26] = "TECHNIQUE_LIT_SUN_SHADOW_GLIGHT";
    techniqueNames[27] = "TECHNIQUE_LIT_SPOT_GLIGHT";
    techniqueNames[28] = "TECHNIQUE_LIT_SPOT_SHADOW_GLIGHT";
    techniqueNames[29] = "TECHNIQUE_LIT_OMNI_GLIGHT";
    techniqueNames[30] = "TECHNIQUE_LIT_OMNI_SHADOW_GLIGHT";
    techniqueNames[31] = "TECHNIQUE_LIT_DLIGHT_GLIGHT";
    techniqueNames[32] = "TECHNIQUE_LIT_SUN_DLIGHT_GLIGHT";
    techniqueNames[33] = "TECHNIQUE_LIT_SUN_SHADOW_DLIGHT_GLIGHT";
    techniqueNames[34] = "TECHNIQUE_LIT_SPOT_DLIGHT_GLIGHT";
    techniqueNames[35] = "TECHNIQUE_LIT_SPOT_SHADOW_DLIGHT_GLIGHT";
    techniqueNames[36] = "TECHNIQUE_LIT_OMNI_DLIGHT_GLIGHT";
    techniqueNames[37] = "TECHNIQUE_LIT_OMNI_SHADOW_DLIGHT_GLIGHT";
    techniqueNames[38] = "TECHNIQUE_LIT_ALPHA";
    techniqueNames[39] = "TECHNIQUE_LIT_SUN_ALPHA";
    techniqueNames[40] = "TECHNIQUE_LIT_SUN_SHADOW_ALPHA";
    techniqueNames[41] = "TECHNIQUE_LIT_SPOT_ALPHA";
    techniqueNames[42] = "TECHNIQUE_LIT_SPOT_SHADOW_ALPHA";
    techniqueNames[43] = "TECHNIQUE_LIT_OMNI_ALPHA";
    techniqueNames[44] = "TECHNIQUE_LIT_OMNI_SHADOW_ALPHA";
    techniqueNames[45] = "TECHNIQUE_LIT_REMAP";
    techniqueNames[46] = "TECHNIQUE_LIT_SUN_REMAP";
    techniqueNames[47] = "TECHNIQUE_LIT_SUN_SHADOW_REMAP";
    techniqueNames[48] = "TECHNIQUE_LIT_SPOT_REMAP";
    techniqueNames[49] = "TECHNIQUE_LIT_SPOT_SHADOW_REMAP";
    techniqueNames[50] = "TECHNIQUE_LIT_OMNI_REMAP";
    techniqueNames[51] = "TECHNIQUE_LIT_OMNI_SHADOW_REMAP";
    techniqueNames[52] = "TECHNIQUE_LIT_FADE";
    techniqueNames[53] = "TECHNIQUE_LIT_SUN_FADE";
    techniqueNames[54] = "TECHNIQUE_LIT_SUN_SHADOW_FADE";
    techniqueNames[55] = "TECHNIQUE_LIT_SPOT_FADE";
    techniqueNames[56] = "TECHNIQUE_LIT_SPOT_SHADOW_FADE";
    techniqueNames[57] = "TECHNIQUE_LIT_OMNI_FADE";
    techniqueNames[58] = "TECHNIQUE_LIT_OMNI_SHADOW_FADE";
    techniqueNames[59] = "TECHNIQUE_LIT_CHARRED";
    techniqueNames[60] = "TECHNIQUE_LIT_FADE_CHARRED";
    techniqueNames[61] = "TECHNIQUE_LIT_SUN_CHARRED";
    techniqueNames[62] = "TECHNIQUE_LIT_SUN_FADE_CHARRED";
    techniqueNames[63] = "TECHNIQUE_LIT_SUN_SHADOW_CHARRED";
    techniqueNames[64] = "TECHNIQUE_LIT_SUN_SHADOW_FADE_CHARRED";
    techniqueNames[65] = "TECHNIQUE_LIT_SPOT_CHARRED";
    techniqueNames[66] = "TECHNIQUE_LIT_SPOT_FADE_CHARRED";
    techniqueNames[67] = "TECHNIQUE_LIT_SPOT_SHADOW_CHARRED";
    techniqueNames[68] = "TECHNIQUE_LIT_SPOT_SHADOW_FADE_CHARRED";
    techniqueNames[69] = "TECHNIQUE_LIT_OMNI_CHARRED";
    techniqueNames[70] = "TECHNIQUE_LIT_OMNI_FADE_CHARRED";
    techniqueNames[71] = "TECHNIQUE_LIT_OMNI_SHADOW_CHARRED";
    techniqueNames[72] = "TECHNIQUE_LIT_OMNI_SHADOW_FADE_CHARRED";
    techniqueNames[73] = "TECHNIQUE_LIT_INSTANCED";
    techniqueNames[74] = "TECHNIQUE_LIT_INSTANCED_SUN";
    techniqueNames[75] = "TECHNIQUE_LIT_INSTANCED_SUN_SHADOW";
    techniqueNames[76] = "TECHNIQUE_LIT_INSTANCED_SPOT";
    techniqueNames[77] = "TECHNIQUE_LIT_INSTANCED_SPOT_SHADOW";
    techniqueNames[78] = "TECHNIQUE_LIT_INSTANCED_OMNI";
    techniqueNames[79] = "TECHNIQUE_LIT_INSTANCED_OMNI_SHADOW";
    techniqueNames[80] = "TECHNIQUE_LIT_NV_INTZ";
    techniqueNames[81] = "TECHNIQUE_LIT_SUN_NV_INTZ";
    techniqueNames[82] = "TECHNIQUE_LIT_SUN_SHADOW_NV_INTZ";
    techniqueNames[83] = "TECHNIQUE_LIT_SPOT_NV_INTZ";
    techniqueNames[84] = "TECHNIQUE_LIT_SPOT_SHADOW_NV_INTZ";
    techniqueNames[85] = "TECHNIQUE_LIT_OMNI_NV_INTZ";
    techniqueNames[86] = "TECHNIQUE_LIT_OMNI_SHADOW_NV_INTZ";
    techniqueNames[87] = "TECHNIQUE_LIT_DLIGHT_NV_INTZ";
    techniqueNames[88] = "TECHNIQUE_LIT_SUN_DLIGHT_NV_INTZ";
    techniqueNames[89] = "TECHNIQUE_LIT_SUN_SHADOW_DLIGHT_NV_INTZ";
    techniqueNames[90] = "TECHNIQUE_LIT_SPOT_DLIGHT_NV_INTZ";
    techniqueNames[91] = "TECHNIQUE_LIT_SPOT_SHADOW_DLIGHT_NV_INTZ";
    techniqueNames[92] = "TECHNIQUE_LIT_OMNI_DLIGHT_NV_INTZ";
    techniqueNames[93] = "TECHNIQUE_LIT_OMNI_SHADOW_DLIGHT_NV_INTZ";
    techniqueNames[94] = "TECHNIQUE_LIT_GLIGHT_NV_INTZ";
    techniqueNames[95] = "TECHNIQUE_LIT_SUN_GLIGHT_NV_INTZ";
    techniqueNames[96] = "TECHNIQUE_LIT_SUN_SHADOW_GLIGHT_NV_INTZ";
    techniqueNames[97] = "TECHNIQUE_LIT_SPOT_GLIGHT_NV_INTZ";
    techniqueNames[98] = "TECHNIQUE_LIT_SPOT_SHADOW_GLIGHT_NV_INTZ";
    techniqueNames[99] = "TECHNIQUE_LIT_OMNI_GLIGHT_NV_INTZ";
    techniqueNames[100] = "TECHNIQUE_LIT_OMNI_SHADOW_GLIGHT_NV_INTZ";
    techniqueNames[101] = "TECHNIQUE_LIT_DLIGHT_GLIGHT_NV_INTZ";
    techniqueNames[102] = "TECHNIQUE_LIT_SUN_DLIGHT_GLIGHT_NV_INTZ";
    techniqueNames[103] = "TECHNIQUE_LIT_SUN_SHADOW_DLIGHT_GLIGHT_NV_INTZ";
    techniqueNames[104] = "TECHNIQUE_LIT_SPOT_DLIGHT_GLIGHT_NV_INTZ";
    techniqueNames[105] = "TECHNIQUE_LIT_SPOT_SHADOW_DLIGHT_GLIGHT_NV_INTZ";
    techniqueNames[106] = "TECHNIQUE_LIT_OMNI_DLIGHT_GLIGHT_NV_INTZ";
    techniqueNames[107] = "TECHNIQUE_LIT_OMNI_SHADOW_DLIGHT_GLIGHT_NV_INTZ";
    techniqueNames[108] = "TECHNIQUE_LIT_INSTANCED_NV_INTZ";
    techniqueNames[109] = "TECHNIQUE_LIT_INSTANCED_SUN_NV_INTZ";
    techniqueNames[110] = "TECHNIQUE_LIT_INSTANCED_SUN_SHADOW_NV_INTZ";
    techniqueNames[111] = "TECHNIQUE_LIT_INSTANCED_SPOT_NV_INTZ";
    techniqueNames[112] = "TECHNIQUE_LIT_INSTANCED_SPOT_SHADOW_NV_INTZ";
    techniqueNames[113] = "TECHNIQUE_LIT_INSTANCED_OMNI_NV_INTZ";
    techniqueNames[114] = "TECHNIQUE_LIT_INSTANCED_OMNI_SHADOW_NV_INTZ";
    techniqueNames[115] = "TECHNIQUE_LIGHT_SPOT";
    techniqueNames[116] = "TECHNIQUE_LIGHT_OMNI";
    techniqueNames[117] = "TECHNIQUE_LIGHT_SPOT_SHADOW";
    techniqueNames[118] = "TECHNIQUE_LIGHT_SPOT_CHARRED";
    techniqueNames[119] = "TECHNIQUE_LIGHT_OMNI_CHARRED";
    techniqueNames[120] = "TECHNIQUE_LIGHT_SPOT_SHADOW_CHARRED";
    techniqueNames[121] = "TECHNIQUE_FAKELIGHT_NORMAL";
    techniqueNames[122] = "TECHNIQUE_FAKELIGHT_VIEW";
    techniqueNames[123] = "TECHNIQUE_SUNLIGHT_PREVIEW";
    techniqueNames[124] = "TECHNIQUE_CASE_TEXTURE";
    techniqueNames[125] = "TECHNIQUE_WIREFRAME_SOLID";
    techniqueNames[126] = "TECHNIQUE_WIREFRAME_SHADED";
    techniqueNames[127] = "TECHNIQUE_DEBUG_BUMPMAP";
    techniqueNames[128] = "TECHNIQUE_DEBUG_BUMPMAP_INSTANCED";
    techniqueNames[129] = "TECHNIQUE_IMPACT_MASK";
    techniqueNames[130] = "TECHNIQUE_COUNT";
    if ( techType == 255 )
        return "TECHNIQUE_NONE";
    if ( techType >= 0x83u
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_logfile.cpp",
                    295,
                    0,
                    "%s\n\t(techType) = %i",
                    "(techType < TECHNIQUE_TOTAL_COUNT)",
                    techType) )
    {
        __debugbreak();
    }
    return techniqueNames[techType];
}

void __cdecl RB_LogPrintState_0(int stateBits0, int changedBits0)
{
    const char *v2; // eax
    const char *v3; // eax
    const char *v4; // eax
    const char *v5; // eax
    const char *v6; // eax
    const char *v7; // eax
    const char *v8; // eax
    StateBitsTable alphaTestTable[4]; // [esp+10h] [ebp-38h] BYREF
    StateBitsTable cullTable[3]; // [esp+30h] [ebp-18h] BYREF

    alphaTestTable[0].stateBits = 2048;
    alphaTestTable[0].name = "Disabled";
    alphaTestTable[1].stateBits = 4096;
    alphaTestTable[1].name = "GT0";
    alphaTestTable[2].stateBits = 0x2000;
    alphaTestTable[2].name = "GE255";
    alphaTestTable[3].stateBits = 12288;
    alphaTestTable[3].name = "GE128";
    cullTable[0].stateBits = 0x4000;
    cullTable[0].name = "None";
    cullTable[1].stateBits = 0x8000;
    cullTable[1].name = "Back";
    cullTable[2].stateBits = 49152;
    cullTable[2].name = "Front";
    if ( !r_logFile->current.integer
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_logfile.cpp",
                    420,
                    0,
                    "%s",
                    "r_logFile->current.integer") )
    {
        __debugbreak();
    }
    v2 = va("---------- (%c)Blend                 : ", (changedBits0 & 0x7FF) != 0 ? 42 : 32);
    RB_LogPrint(v2);
    if ( (stateBits0 & 0x700) != 0 )
    {
        RB_LogBlendOp("%s( ", (stateBits0 & 0x700) >> 8);
        RB_LogBlend("%s, ", stateBits0 & 0xF);
        RB_LogBlend("%s )\n", (stateBits0 & 0xF0) >> 4);
    }
    else
    {
        RB_LogPrint("Disabled\n");
    }
    v3 = va("---------- (%c)SeparateAlpha : ", (changedBits0 & 0x7FF0000) != 0 ? 42 : 32);
    RB_LogPrint(v3);
    if ( (stateBits0 & 0x7000000) != 0 )
    {
        RB_LogBlendOp("%s( ", (stateBits0 & 0x7000000) >> 24);
        RB_LogBlend("%s, ", (stateBits0 & 0xF0000) >> 16);
        RB_LogBlend("%s )\n", (stateBits0 & 0xF00000) >> 20);
    }
    else
    {
        RB_LogPrint("Disabled\n");
    }
    RB_LogFromTable("AlphaTest", stateBits0, changedBits0, 14336, 0, alphaTestTable, 4);
    v4 = va("---------- (%c)Color Write     : ", (changedBits0 & 0x18000000) != 0 ? 42 : 32);
    RB_LogPrint(v4);
    if ( (stateBits0 & 0x8000000) != 0 )
        v5 = va("%s, ", "true");
    else
        v5 = va("%s, ", "false");
    RB_LogPrint(v5);
    if ( (stateBits0 & 0x8000000) != 0 )
        v6 = va("%s, ", "true");
    else
        v6 = va("%s, ", "false");
    RB_LogPrint(v6);
    if ( (stateBits0 & 0x8000000) != 0 )
        v7 = va("%s, ", "true");
    else
        v7 = va("%s, ", "false");
    RB_LogPrint(v7);
    if ( (stateBits0 & 0x10000000) != 0 )
        v8 = va("%s\n", "true");
    else
        v8 = va("%s\n", "false");
    RB_LogPrint(v8);
    RB_LogFromTable("Cull Face", stateBits0, changedBits0, 49152, 0, cullTable, 3);
}

void __cdecl RB_LogBlend(const char *format, unsigned int blend)
{
    const char *v2; // eax
    const char *blendNames[14]; // [esp+0h] [ebp-38h]

    blendNames[0] = "Disabled";
    blendNames[1] = "Zero";
    blendNames[2] = "One";
    blendNames[3] = "SrcColor";
    blendNames[4] = "InvSrcColor";
    blendNames[5] = "SrcAlpha";
    blendNames[6] = "InvSrcAlpha";
    blendNames[7] = "DestAlpha";
    blendNames[8] = "InvDestAlpha";
    blendNames[9] = "DestColor";
    blendNames[10] = "InvDestColor";
    blendNames[11] = "SrcAlphaSat";
    blendNames[12] = "BlendFactor";
    blendNames[13] = "InvBlendFactor";
    if ( !r_logFile->current.integer
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_logfile.cpp",
                    319,
                    0,
                    "%s",
                    "r_logFile->current.integer") )
    {
        __debugbreak();
    }
    if ( blend >= 0xE
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_logfile.cpp",
                    320,
                    0,
                    "blend doesn't index ARRAY_COUNT( blendNames )\n\t%i not in [0, %i)",
                    blend,
                    14) )
    {
        __debugbreak();
    }
    v2 = va(format, blendNames[blend]);
    RB_LogPrint(v2);
}

void __cdecl RB_LogBlendOp(const char *format, unsigned int blendOp)
{
    const char *v2; // eax
    const char *blendOpNames[6]; // [esp+0h] [ebp-18h]

    blendOpNames[0] = "Disabled";
    blendOpNames[1] = "Add";
    blendOpNames[2] = "Subtract";
    blendOpNames[3] = "RevSubtract";
    blendOpNames[4] = "Min";
    blendOpNames[5] = "Max";
    if ( !r_logFile->current.integer
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_logfile.cpp",
                    337,
                    0,
                    "%s",
                    "r_logFile->current.integer") )
    {
        __debugbreak();
    }
    if ( blendOp >= 6
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_logfile.cpp",
                    338,
                    0,
                    "blendOp doesn't index ARRAY_COUNT( blendOpNames )\n\t%i not in [0, %i)",
                    blendOp,
                    6) )
    {
        __debugbreak();
    }
    v2 = va(format, blendOpNames[blendOp]);
    RB_LogPrint(v2);
}

void __cdecl RB_LogFromTable(
                const char *keyName,
                int stateBits,
                int changedBits,
                int bitMask,
                char bitShift,
                const StateBitsTable *table,
                int tableCount)
{
    const char *v7; // eax
    const char *v8; // eax
    int tableIndex; // [esp+0h] [ebp-8h]
    int stateBitsa; // [esp+14h] [ebp+Ch]

    stateBitsa = (bitMask & stateBits) >> bitShift;
    for ( tableIndex = 0; tableIndex < tableCount; ++tableIndex )
    {
        if ( table[tableIndex].stateBits == stateBitsa )
        {
            v7 = va("---------- (%c)%-14s: %s\n", (bitMask & changedBits) != 0 ? 42 : 32, keyName, table[tableIndex].name);
            RB_LogPrint(v7);
            return;
        }
    }
    v8 = va("---------- (%c)%-14s: unknown - %08x\n", (bitMask & changedBits) != 0 ? 42 : 32, keyName, stateBitsa);
    RB_LogPrint(v8);
}

void __cdecl RB_LogPrintState_1(int stateBits1, int changedBits1)
{
    StencilLogBits stencilLogFront; // [esp+0h] [ebp-78h] BYREF
    StencilLogBits stencilLogBack; // [esp+18h] [ebp-60h] BYREF
    StateBitsTable depthTestTable[5]; // [esp+30h] [ebp-48h] BYREF
    StateBitsTable polygonOffsetTable[4]; // [esp+58h] [ebp-20h] BYREF

    depthTestTable[0].stateBits = 2;
    depthTestTable[0].name = "Disabled";
    depthTestTable[1].stateBits = 4;
    depthTestTable[1].name = "Less";
    depthTestTable[2].stateBits = 12;
    depthTestTable[2].name = "LessEqual";
    depthTestTable[3].stateBits = 8;
    depthTestTable[3].name = "Equal";
    depthTestTable[4].stateBits = 0;
    depthTestTable[4].name = "Always";
    polygonOffsetTable[0].stateBits = 0;
    polygonOffsetTable[0].name = "0";
    polygonOffsetTable[1].stateBits = 16;
    polygonOffsetTable[1].name = "1";
    polygonOffsetTable[2].stateBits = 32;
    polygonOffsetTable[2].name = "2";
    polygonOffsetTable[3].stateBits = 48;
    polygonOffsetTable[3].name = "shadowmap";
    stencilLogFront.description = "Front";
    stencilLogFront.enableMask = 64;
    stencilLogFront.passShift = 8;
    stencilLogFront.failShift = 11;
    stencilLogFront.zfailShift = 14;
    stencilLogFront.funcShift = 17;
    stencilLogBack.description = "Back";
    stencilLogBack.enableMask = 128;
    stencilLogBack.passShift = 20;
    stencilLogBack.failShift = 23;
    stencilLogBack.zfailShift = 26;
    stencilLogBack.funcShift = 29;
    if ( !r_logFile->current.integer
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_logfile.cpp",
                    493,
                    0,
                    "%s",
                    "r_logFile->current.integer") )
    {
        __debugbreak();
    }
    RB_LogBool("Depth Write", stateBits1, changedBits1, 1, "Enabled", "Disabled");
    RB_LogFromTable("Depth Test", stateBits1, changedBits1, 14, 0, depthTestTable, 5);
    RB_LogFromTable("Polygon Offset", stateBits1, changedBits1, 48, 0, polygonOffsetTable, 4);
    RB_LogStencilState(stateBits1, changedBits1, &stencilLogFront);
    RB_LogStencilState(stateBits1, changedBits1, &stencilLogBack);
}

void __cdecl RB_LogBool(
                const char *keyName,
                int stateBits,
                int changedBits,
                int bitMask,
                const char *trueName,
                const char *falseName)
{
    const char *v6; // eax
    int changedBitsa; // [esp+14h] [ebp+10h]

    changedBitsa = bitMask & changedBits;
    if ( (bitMask & stateBits) != 0 )
        v6 = va("---------- (%c)%-14s: %s\n", changedBitsa != 0 ? 42 : 32, keyName, trueName);
    else
        v6 = va("---------- (%c)%-14s: %s\n", changedBitsa != 0 ? 42 : 32, keyName, falseName);
    RB_LogPrint(v6);
}

void __cdecl RB_LogStencilState(int stateBits1, int changedBits1, const StencilLogBits *desc)
{
    const char *v3; // eax
    const char *v4; // eax
    const char *v5; // eax
    const char *v6; // eax
    const char *v7; // eax
    int enableMask; // [esp-Ch] [ebp-8Ch]
    int funcShift; // [esp-Ch] [ebp-8Ch]
    int passShift; // [esp-Ch] [ebp-8Ch]
    int failShift; // [esp-Ch] [ebp-8Ch]
    int zfailShift; // [esp-Ch] [ebp-8Ch]
    StateBitsTable stencilFuncNames[8]; // [esp+0h] [ebp-80h] BYREF
    StateBitsTable stencilOpNames[8]; // [esp+40h] [ebp-40h] BYREF

    stencilFuncNames[0].stateBits = 0;
    stencilFuncNames[0].name = "Never";
    stencilFuncNames[1].stateBits = 1;
    stencilFuncNames[1].name = "Less";
    stencilFuncNames[2].stateBits = 2;
    stencilFuncNames[2].name = "Equal";
    stencilFuncNames[3].stateBits = 3;
    stencilFuncNames[3].name = "LessEqual";
    stencilFuncNames[4].stateBits = 4;
    stencilFuncNames[4].name = "Greater";
    stencilFuncNames[5].stateBits = 5;
    stencilFuncNames[5].name = "NotEqual";
    stencilFuncNames[6].stateBits = 6;
    stencilFuncNames[6].name = "GreaterEqual";
    stencilFuncNames[7].stateBits = 7;
    stencilFuncNames[7].name = "Always";
    stencilOpNames[0].stateBits = 0;
    stencilOpNames[0].name = "Keep";
    stencilOpNames[1].stateBits = 1;
    stencilOpNames[1].name = "Zero";
    stencilOpNames[2].stateBits = 2;
    stencilOpNames[2].name = "Replace";
    stencilOpNames[3].stateBits = 3;
    stencilOpNames[3].name = "IncrSat";
    stencilOpNames[4].stateBits = 4;
    stencilOpNames[4].name = "DecrSat";
    stencilOpNames[5].stateBits = 5;
    stencilOpNames[5].name = "Invert";
    stencilOpNames[6].stateBits = 6;
    stencilOpNames[6].name = "Incr";
    stencilOpNames[7].stateBits = 7;
    stencilOpNames[7].name = "Decr";
    enableMask = desc->enableMask;
    v3 = va("Stencil %s", desc->description);
    RB_LogBool(v3, stateBits1, changedBits1, enableMask, "Enabled", "Disabled");
    if ( (desc->enableMask & stateBits1) != 0 )
    {
        funcShift = desc->funcShift;
        v4 = va("%s Func", desc->description);
        RB_LogFromTable(v4, stateBits1, changedBits1, 7 << funcShift, funcShift, stencilFuncNames, 8);
        passShift = desc->passShift;
        v5 = va("%s Pass", desc->description);
        RB_LogFromTable(v5, stateBits1, changedBits1, 7 << passShift, passShift, stencilOpNames, 8);
        failShift = desc->failShift;
        v6 = va("%s Fail", desc->description);
        RB_LogFromTable(v6, stateBits1, changedBits1, 7 << failShift, failShift, stencilOpNames, 8);
        zfailShift = desc->zfailShift;
        v7 = va("%s ZFail", desc->description);
        RB_LogFromTable(v7, stateBits1, changedBits1, 7 << zfailShift, zfailShift, stencilOpNames, 8);
    }
}

