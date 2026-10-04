#include "rb_stats.h"

const int drawPrimHistogramLimit[15] =
{ 10, 25, 50, 100, 200, 300, 400, 600, 800, 1000, 1200, 1400, 1600, 1800, 2000 };

const char *primStatsLabel[10] =
{
  "world",
  "smodel cached",
  "smodel rigid",
  "xmodel rigid",
  "xmodel skinned",
  "bmodel",
  "fx",
  "hud & 2d",
  "debug",
  "code"
};

const unsigned int s_stencilFuncTable_85[8] =
{ 1u, 2u, 3u, 4u, 5u, 6u, 7u, 8u };

GfxFrameStats g_frameStatsCur;
GfxViewStats *g_viewStats;
int histogramHistory[64][2][16];
int histogramHistoryIndex;

void __cdecl R_TrackPrims(GfxCmdBufPrimState *state, GfxPrimStatsTarget target)
{
    if ( state->backupPrimStats
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_stats.cpp",
                    107,
                    0,
                    "%s",
                    "state->backupPrimStats == NULL") )
    {
        __debugbreak();
    }
    if ( !state->viewStats
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_stats.cpp", 108, 0, "%s", "state->viewStats") )
    {
        __debugbreak();
    }
    if ( state->primStats
        && state->primStats != &state->viewStats->primStats[target]
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_stats.cpp",
                    109,
                    0,
                    "%s\n\t(target) = %i",
                    "(state->primStats == 0 || state->primStats == &state->viewStats->primStats[target])",
                    target) )
    {
        __debugbreak();
    }
    state->primStats = &state->viewStats->primStats[target];
}

void __cdecl RB_TrackImmediatePrims(GfxCmdBufPrimState *state, GfxPrimStatsTarget target)
{
    if ( state->backupPrimStats
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_stats.cpp",
                    121,
                    0,
                    "%s",
                    "state->backupPrimStats == NULL") )
    {
        __debugbreak();
    }
    if ( !state->viewStats
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_stats.cpp", 122, 0, "%s", "state->viewStats") )
    {
        __debugbreak();
    }
    state->backupPrimStats = state->primStats;
    state->primStats = &state->viewStats->primStats[target];
}

void __cdecl RB_EndTrackImmediatePrims(GfxCmdBufPrimState *state)
{
    state->primStats = state->backupPrimStats;
    state->backupPrimStats = 0;
}

void __cdecl RB_TrackDrawPrimCall(GfxCmdBufPrimState *state, int triCount)
{
    int histogramIndex; // [esp+0h] [ebp-4h]

    if ( state->primStats )
    {
        state->primStats->triCount += triCount;
        ++state->primStats->primCount;
        for ( histogramIndex = 0; histogramIndex < 15 && triCount > drawPrimHistogramLimit[histogramIndex]; ++histogramIndex )
            ;
        ++state->viewStats->drawPrimHistogram[histogramIndex];
    }
}

int __cdecl RB_Stats_TotalVertexCount()
{
    int totalVertexCount; // [esp+0h] [ebp-14h]
    int primIndex; // [esp+Ch] [ebp-8h]
    int viewIndex; // [esp+10h] [ebp-4h]

    totalVertexCount = 0;
    for ( viewIndex = 0; viewIndex < 2; ++viewIndex )
    {
        for ( primIndex = 0; primIndex < 10; ++primIndex )
            totalVertexCount += g_frameStatsCur.viewStats[viewIndex].primStats[primIndex].dynamicVertexCount
                                                + g_frameStatsCur.viewStats[viewIndex].primStats[primIndex].staticVertexCount;
    }
    return totalVertexCount;
}

int __cdecl RB_Stats_TotalPrimCount()
{
    int totalPrimCount; // [esp+4h] [ebp-10h]
    int primIndex; // [esp+Ch] [ebp-8h]
    int viewIndex; // [esp+10h] [ebp-4h]

    totalPrimCount = 0;
    for ( viewIndex = 0; viewIndex < 2; ++viewIndex )
    {
        for ( primIndex = 0; primIndex < 10; ++primIndex )
            totalPrimCount += g_frameStatsCur.viewStats[viewIndex].primStats[primIndex].primCount;
    }
    return totalPrimCount;
}

int __cdecl RB_Stats_ViewIndexCount(const GfxViewStats *viewStats)
{
    int totalIndexCount; // [esp+0h] [ebp-Ch]
    int primIndex; // [esp+8h] [ebp-4h]

    totalIndexCount = 0;
    for ( primIndex = 0; primIndex < 10; ++primIndex )
        totalIndexCount += viewStats->primStats[primIndex].dynamicIndexCount
                                         + viewStats->primStats[primIndex].staticIndexCount;
    return totalIndexCount;
}

void __cdecl RB_Stats_f()
{
    const char *v0; // eax
    const char *v1; // eax
    const char *v2; // eax

    if ( Cmd_Argc() > 2 )
        goto LABEL_2;
    R_SyncRenderThread();
    if ( Cmd_Argc() == 2 )
    {
        v1 = Cmd_Argv(1);
        if ( !I_stricmp(v1, "max") )
        {
            RB_Stats_Display(&backEnd.frameStatsMax);
            return;
        }
        v2 = Cmd_Argv(1);
        if ( I_stricmp(v2, "cur") )
        {
LABEL_2:
            v0 = Cmd_Argv(0);
            Com_Printf(8, "USAGE: %s [cur|max]\n", v0);
            return;
        }
    }
    RB_Stats_Display(&g_frameStatsCur);
}

void __cdecl RB_Stats_Display(const GfxFrameStats *frameStats)
{
    int v1; // eax
    int v2; // eax

    RB_Stats_Summarize("Visible", frameStats->viewStats);
    RB_Stats_Summarize("Shadow", &frameStats->viewStats[1]);
    Com_Printf(8, "SUMMARY:\n");
    Com_Printf(
        8,
        "%i raw tris (%i geo, %i fx)\n",
        frameStats->fxIndexCount / 3 + frameStats->geoIndexCount / 3,
        frameStats->geoIndexCount / 3,
        frameStats->fxIndexCount / 3);
    v1 = RB_Stats_TotalIndexCount();
    Com_Printf(8, "%i total tris\n", v1 / 3);
    v2 = RB_Stats_TotalPrimCount();
    Com_Printf(8, "%i total prims\n", v2);
}

int __cdecl RB_Stats_TotalIndexCount()
{
    int totalIndexCount; // [esp+0h] [ebp-14h]
    int primIndex; // [esp+Ch] [ebp-8h]
    int viewIndex; // [esp+10h] [ebp-4h]

    totalIndexCount = 0;
    for ( viewIndex = 0; viewIndex < 2; ++viewIndex )
    {
        for ( primIndex = 0; primIndex < 10; ++primIndex )
            totalIndexCount += g_frameStatsCur.viewStats[viewIndex].primStats[primIndex].dynamicIndexCount
                                             + g_frameStatsCur.viewStats[viewIndex].primStats[primIndex].staticIndexCount;
    }
    return totalIndexCount;
}

void __cdecl RB_Stats_Summarize(const char *label, const GfxViewStats *viewStats)
{
    int barBlocks; // [esp+8h] [ebp-30h]
    GfxPrimStats total; // [esp+Ch] [ebp-2Ch] BYREF
    int barIndex; // [esp+24h] [ebp-14h]
    int maxPrimsInHistogram; // [esp+28h] [ebp-10h]
    const GfxPrimStats *primStats; // [esp+2Ch] [ebp-Ch]
    int primIndex; // [esp+30h] [ebp-8h]
    int histogramIndex; // [esp+34h] [ebp-4h]

    Com_Printf(8, "\n=== %s Geometry (%i drawsurfs) ===\n", label, viewStats->drawSurfCount);
    Com_Printf(8, "+--------------+-----+-------+-------+-------+-------+-------+\n");
    Com_Printf(8, "|geometry type |prims|     tris|s indxs|s verts|d indxs|d verts|\n");
    Com_Printf(8, "+--------------+-----+-------+-------+-------+-------+-------+\n");
    memset(&total, 0, sizeof(total));
    for ( primIndex = 0; primIndex < 10; ++primIndex )
    {
        primStats = &viewStats->primStats[primIndex];
        RB_Stats_SummarizePrimStats(primStatsLabel[primIndex], primStats);
        RB_Stats_AccumulatePrimStats(primStats, &total);
    }
    Com_Printf(8, "+--------------+-----+-------+-------+-------+-------+-------+\n");
    RB_Stats_SummarizePrimStats("total", &total);
    Com_Printf(8, "+--------------+-----+-------+-------+-------+-------+-------+\n");
    maxPrimsInHistogram = 0;
    for ( histogramIndex = 0; histogramIndex < 16; ++histogramIndex )
    {
        if ( maxPrimsInHistogram < viewStats->drawPrimHistogram[histogramIndex] )
            maxPrimsInHistogram = viewStats->drawPrimHistogram[histogramIndex];
    }
    if ( maxPrimsInHistogram )
    {
        Com_Printf(8, "\nTriangles Per Primitive Histogram:\n");
        for ( histogramIndex = 0; histogramIndex < 16; ++histogramIndex )
        {
            if ( histogramIndex >= 15 )
                Com_Printf(8, " > %4i: ", s_stencilFuncTable_85[histogramIndex + 7]);
            else
                Com_Printf(8, "<= %4i: ", drawPrimHistogramLimit[histogramIndex]);
            Com_Printf(
                8,
                "%3i (%4.1f%%) ",
                viewStats->drawPrimHistogram[histogramIndex],
                (float)((float)((float)viewStats->drawPrimHistogram[histogramIndex] * 100.0) / (float)total.primCount));
            barBlocks = (40 * viewStats->drawPrimHistogram[histogramIndex] + maxPrimsInHistogram / 2) / maxPrimsInHistogram;
            for ( barIndex = 0; barIndex < barBlocks; ++barIndex )
                Com_Printf(8, "#");
            Com_Printf(8, "\n");
        }
    }
}

void __cdecl RB_Stats_AccumulatePrimStats(const GfxPrimStats *primStats, GfxPrimStats *total)
{
    total->primCount += primStats->primCount;
    total->triCount += primStats->triCount;
    total->staticIndexCount += primStats->staticIndexCount;
    total->staticVertexCount += primStats->staticVertexCount;
    total->dynamicIndexCount += primStats->dynamicIndexCount;
    total->dynamicVertexCount += primStats->dynamicVertexCount;
}

void __cdecl RB_Stats_SummarizePrimStats(const char *label, const GfxPrimStats *primStats)
{
    char *v2; // [esp+0h] [ebp-3Ch]
    char *v3; // [esp+4h] [ebp-38h]
    char *v4; // [esp+8h] [ebp-34h]
    char *v5; // [esp+Ch] [ebp-30h]
    char *v6; // [esp+10h] [ebp-2Ch]
    char *v7; // [esp+14h] [ebp-28h]
    char text[32]; // [esp+18h] [ebp-24h] BYREF

    Com_Printf(8, "|%-14s", label);
    if ( primStats->primCount )
    {
        v7 = _itoa(primStats->primCount, text, 0xAu);
        Com_Printf(8, "|%5s", v7);
    }
    else
    {
        Com_Printf(8, "|%5s", "");
    }
    if ( primStats->triCount )
    {
        v6 = _itoa(primStats->triCount, text, 0xAu);
        Com_Printf(8, "|%7s", v6);
    }
    else
    {
        Com_Printf(8, "|%7s", "");
    }
    if ( primStats->staticIndexCount )
    {
        v5 = _itoa(primStats->staticIndexCount, text, 0xAu);
        Com_Printf(8, "|%7s", v5);
    }
    else
    {
        Com_Printf(8, "|%7s", "");
    }
    if ( primStats->staticVertexCount )
    {
        v4 = _itoa(primStats->staticVertexCount, text, 0xAu);
        Com_Printf(8, "|%7s", v4);
    }
    else
    {
        Com_Printf(8, "|%7s", "");
    }
    if ( primStats->dynamicIndexCount )
    {
        v3 = _itoa(primStats->dynamicIndexCount, text, 0xAu);
        Com_Printf(8, "|%7s", v3);
    }
    else
    {
        Com_Printf(8, "|%7s", "");
    }
    if ( primStats->dynamicVertexCount )
    {
        v2 = _itoa(primStats->dynamicVertexCount, text, 0xAu);
        Com_Printf(8, "|%7s|\n", v2);
    }
    else
    {
        Com_Printf(8, "|%7s|\n", "");
    }
}

void __cdecl RB_Stats_UpdateMaxs(const GfxFrameStats *frameStatsCur, GfxFrameStats *frameStatsMax)
{
    int index; // [esp+8h] [ebp-Ch]

    for ( index = 0; index < 159; ++index )
    {
        if ( *(&frameStatsMax->viewStats[0].primStats[0].primCount + index) < *(&frameStatsCur->viewStats[0].primStats[0].primCount
                                                                                                                                                    + index) )
            *(&frameStatsMax->viewStats[0].primStats[0].primCount + index) = *(&frameStatsCur->viewStats[0].primStats[0].primCount
                                                                                                                                             + index);
    }
}

void __cdecl RB_DrawPrimHistogramOverlay()
{
    const char *v0; // eax
    Font_s *s1; // [esp+18h] [ebp-B8h]
    float t1; // [esp+1Ch] [ebp-B4h]
    float v3; // [esp+20h] [ebp-B0h]
    GfxColor v4; // [esp+24h] [ebp-ACh]
    const char *v5; // [esp+28h] [ebp-A8h]
    int rowMax; // [esp+54h] [ebp-7Ch]
    float x1; // [esp+58h] [ebp-78h]
    int subTotalPrims[2][3]; // [esp+5Ch] [ebp-74h] BYREF
    unsigned int subTotalIndex; // [esp+74h] [ebp-5Ch]
    const char *caption; // [esp+78h] [ebp-58h]
    GfxColor colorNow[3]; // [esp+7Ch] [ebp-54h]
    int historyIndex; // [esp+88h] [ebp-48h]
    int totalPrims[2]; // [esp+8Ch] [ebp-44h]
    float wMax; // [esp+94h] [ebp-3Ch]
    float countWidth; // [esp+98h] [ebp-38h]
    int maxPrimsInHistogram; // [esp+9Ch] [ebp-34h]
    int curCount; // [esp+A0h] [ebp-30h]
    int viewStatsIndex; // [esp+A4h] [ebp-2Ch]
    GfxColor colorPeak[3]; // [esp+A8h] [ebp-28h]
    float fontHeight; // [esp+B8h] [ebp-18h]
    GfxColor black; // [esp+BCh] [ebp-14h]
    int histogramIndex; // [esp+C0h] [ebp-10h]
    float x; // [esp+C4h] [ebp-Ch]
    float y; // [esp+C8h] [ebp-8h]
    float w; // [esp+CCh] [ebp-4h]

    maxPrimsInHistogram = 0;
    for ( histogramIndex = 0; histogramIndex < 16; ++histogramIndex )
    {
        for ( viewStatsIndex = 0; viewStatsIndex < 2; ++viewStatsIndex )
        {
            if ( maxPrimsInHistogram < backEnd.frameStatsMax.viewStats[viewStatsIndex].drawPrimHistogram[histogramIndex] )
                maxPrimsInHistogram = backEnd.frameStatsMax.viewStats[viewStatsIndex].drawPrimHistogram[histogramIndex];
        }
    }
    if ( maxPrimsInHistogram )
    {
        fontHeight = (float)R_TextHeight(backEnd.debugFont);
        countWidth = (float)R_TextWidth("8888 ", 5, backEnd.debugFont);
        x1 = (float)((float)R_TextWidth("8888:", 5, backEnd.debugFont) + 48.0) + 4.0;
        y = 48.0f;
        wMax = (float)((float)((float)(360.0 - 48.0) / 2.0) - countWidth) - 4.0;
        black.packed = -16777216;
        colorNow[0].packed = -49088;
        colorPeak[0].packed = -6291456;
        colorNow[1].packed = -192;
        colorPeak[1].packed = -6250496;
        colorNow[2].packed = -12517568;
        colorPeak[2].packed = -16736256;
        totalPrims[0] = 0;
        totalPrims[1] = 0;
        memset(subTotalPrims, 0, sizeof(subTotalPrims));
        for ( histogramIndex = 0; histogramIndex < 16; ++histogramIndex )
        {
            if ( histogramIndex >= 15 )
                v5 = "more:";
            else
                v5 = va("%4i:", drawPrimHistogramLimit[histogramIndex]);
            caption = v5;
            RB_DrawText(v5, backEnd.debugFont, 48.0, (float)(y + fontHeight) - 1.0, (GfxColor)-1);
            x = x1;
            if ( histogramIndex == 15 || drawPrimHistogramLimit[histogramIndex] > 300 )
                subTotalIndex = 2;
            else
                subTotalIndex = drawPrimHistogramLimit[histogramIndex] > 50;
            for ( viewStatsIndex = 0; viewStatsIndex < 2; ++viewStatsIndex )
            {
                curCount = g_frameStatsCur.viewStats[viewStatsIndex].drawPrimHistogram[histogramIndex];
                subTotalPrims[viewStatsIndex][subTotalIndex] += curCount;
                totalPrims[viewStatsIndex] += curCount;
                if ( curCount )
                {
                    v4.packed = colorNow[subTotalIndex].packed;
                    v3 = (float)(y + fontHeight) - 1.0;
                    t1 = x;
                    s1 = backEnd.debugFont;
                    v0 = va("%4i", curCount);
                    RB_DrawText(v0, s1, t1, v3, v4);
                }
                x = x + countWidth;
                rowMax = curCount;
                for ( historyIndex = 0; historyIndex < 64; ++historyIndex )
                {
                    if ( rowMax < histogramHistory[historyIndex][viewStatsIndex][histogramIndex] )
                        rowMax = histogramHistory[historyIndex][viewStatsIndex][histogramIndex];
                }
                if ( rowMax )
                {
                    w = (float)((float)rowMax * wMax) / (float)maxPrimsInHistogram;
                    RB_DrawStretchPic(
                        rgp.whiteMaterial,
                        x - 2.0,
                        y + 2.0,
                        w + 4.0,
                        fontHeight - 4.0,
                        0.0,
                        0.0,
                        1.0,
                        1.0,
                        black.packed,
                        GFX_PRIM_STATS_DEBUG);
                    RB_DrawStretchPic(
                        rgp.whiteMaterial,
                        x,
                        y + 4.0,
                        w,
                        fontHeight - 8.0,
                        0.0,
                        0.0,
                        1.0,
                        1.0,
                        colorPeak[subTotalIndex].packed,
                        GFX_PRIM_STATS_DEBUG);
                    if ( curCount )
                    {
                        w = (float)((float)curCount * wMax) / (float)maxPrimsInHistogram;
                        RB_DrawStretchPic(
                            rgp.whiteMaterial,
                            x,
                            y + 4.0,
                            w,
                            fontHeight - 8.0,
                            0.0,
                            0.0,
                            1.0,
                            1.0,
                            colorNow[subTotalIndex].packed,
                            GFX_PRIM_STATS_DEBUG);
                    }
                }
                histogramHistory[histogramHistoryIndex & 0x3F][viewStatsIndex][histogramIndex] = curCount;
                x = (float)(wMax + 4.0) + x;
            }
            y = y + fontHeight;
        }
        x = x1;
        for ( viewStatsIndex = 0; viewStatsIndex < 2; ++viewStatsIndex )
        {
            x = x + countWidth;
            if ( totalPrims[viewStatsIndex] )
            {
                RB_DrawStretchPic(
                    rgp.whiteMaterial,
                    x - 2.0,
                    y + 2.0,
                    wMax + 4.0,
                    fontHeight - 4.0,
                    0.0,
                    0.0,
                    1.0,
                    1.0,
                    black.packed,
                    GFX_PRIM_STATS_DEBUG);
                for ( subTotalIndex = 0; subTotalIndex < 3; ++subTotalIndex )
                {
                    w = (float)((float)subTotalPrims[viewStatsIndex][subTotalIndex] * wMax) / (float)totalPrims[viewStatsIndex];
                    RB_DrawStretchPic(
                        rgp.whiteMaterial,
                        x,
                        y + 4.0,
                        w,
                        fontHeight - 8.0,
                        0.0,
                        0.0,
                        1.0,
                        1.0,
                        colorNow[subTotalIndex].packed,
                        GFX_PRIM_STATS_DEBUG);
                    x = x + w;
                }
                x = x + 4.0;
            }
            else
            {
                x = (float)(wMax + 4.0) + x;
            }
        }
        ++histogramHistoryIndex;
    }
}

