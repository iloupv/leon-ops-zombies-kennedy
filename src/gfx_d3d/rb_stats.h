#pragma once
#include "rb_state.h"
#include "rb_backend.h"

void __cdecl R_TrackPrims(GfxCmdBufPrimState *state, GfxPrimStatsTarget target);
void __cdecl RB_TrackImmediatePrims(GfxCmdBufPrimState *state, GfxPrimStatsTarget target);
void __cdecl RB_EndTrackImmediatePrims(GfxCmdBufPrimState *state);
void __cdecl RB_TrackDrawPrimCall(GfxCmdBufPrimState *state, int triCount);
int __cdecl RB_Stats_TotalVertexCount();
int __cdecl RB_Stats_TotalPrimCount();
int __cdecl RB_Stats_ViewIndexCount(const GfxViewStats *viewStats);
void __cdecl RB_Stats_f();
void __cdecl RB_Stats_Display(const GfxFrameStats *frameStats);
int __cdecl RB_Stats_TotalIndexCount();
void __cdecl RB_Stats_Summarize(const char *label, const GfxViewStats *viewStats);
void __cdecl RB_Stats_AccumulatePrimStats(const GfxPrimStats *primStats, GfxPrimStats *total);
void __cdecl RB_Stats_SummarizePrimStats(const char *label, const GfxPrimStats *primStats);
void __cdecl RB_Stats_UpdateMaxs(const GfxFrameStats *frameStatsCur, GfxFrameStats *frameStatsMax);
void __cdecl RB_DrawPrimHistogramOverlay();

extern GfxFrameStats g_frameStatsCur;
extern GfxViewStats *g_viewStats;