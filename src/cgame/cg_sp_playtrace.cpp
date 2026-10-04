// j1: client half of the play-session recorder (TOOL, off by default; see game_sp/g_sp_playtrace.h and
// notes/playtrace.md). Main thread only. Reads the local client's snapshot / ping / prediction / HUD state and
// writes one "c" line per bo1_playtrace_ms of real time; F8 (while unbound) drops a MARK. Changes nothing.
#include "cg_sp_playtrace.h"
#include <game_sp/g_sp_playtrace.h>
#include <client_mp/cl_main_mp.h>
#include <client/client.h>
#include <client/cl_main.h>
#include <cgame_mp/cg_local_mp.h>
#include <cgame_mp/cg_newDraw_mp.h>
#include <universal/dvar.h>
#include <win32/win_shared.h>
#include <ui/keycodes.h>
#include <qcommon/common.h>
#include <gfx_d3d/r_dvars.h>
#include <Windows.h>
#include <algorithm>
#include <cstdio>
#include <game_sp/g_sp_measure.h>
#include <qcommon/dobj_management.h>
#include <cgame_mp/cg_ents_mp.h>
#include <clientscript/scr_const.h>
#include <bgame/bg_local.h>

namespace
{
// L8: frame times of the window (Com_Frame to Com_Frame, what the player sees; QueryPerformanceCounter)
constexpr int MAX_FRAME_TIMES = 8192; // 60 s at 125 fps
float s_frameMs[MAX_FRAME_TIMES];
int s_frameCount;
long long s_lastFrameTick; // 0 = no previous frame (window not running)
double s_ticksPerMs;

struct ClientWindow
{
    int start;          // Sys_Milliseconds at the window start (0 = not started)
    int frames;
    int lastMessageNum;
    int lastServerTime;
    int messages;       // server message numbers advanced (snapshots sent to us)
    int snapshots;      // new snapshots seen by the client frame
    int maxSnapGap;     // largest serverTime step between consecutive seen snapshots
    int pingMin, pingMax;
    int extrapolated;   // frames the client ran past its newest snapshot
    int misses;         // CG_PredictPlayerState "Prediction miss" count (> 0.1 units)
    float missMax;
} s_win;

void ResetWindow(int now)
{
    const int lastMessageNum = s_win.lastMessageNum, lastServerTime = s_win.lastServerTime;
    s_win = {};
    s_win.start = now;
    s_win.lastMessageNum = lastMessageNum;
    s_win.lastServerTime = lastServerTime;
    s_win.pingMin = 0x7FFFFFFF;
    s_frameCount = 0;
}

void AddFrameTime()
{
    LARGE_INTEGER t;
    QueryPerformanceCounter(&t);
    if (s_ticksPerMs == 0.0)
    {
        LARGE_INTEGER f;
        QueryPerformanceFrequency(&f);
        s_ticksPerMs = (double)f.QuadPart / 1000.0;
    }
    if (s_lastFrameTick && s_frameCount < MAX_FRAME_TIMES)
        s_frameMs[s_frameCount++] = (float)((double)(t.QuadPart - s_lastFrameTick) / s_ticksPerMs);
    s_lastFrameTick = t.QuadPart;
}

// "fps" frames per real second of the window; "ft" frame time p50 / p95 / p99 / max ms; "ft20" / "ft33" frames >= 20 / 33.3 ms;
// "cap" com_maxfps and "vsync" r_vsync as the frames ran
void FrameTimeFields(char *out, size_t size, int elapsedMs)
{
    float p50 = 0.0f, p95 = 0.0f, p99 = 0.0f, mx = 0.0f;
    int over20 = 0, over33 = 0;
    if (s_frameCount)
    {
        std::sort(s_frameMs, s_frameMs + s_frameCount);
        p50 = s_frameMs[s_frameCount / 2];
        p95 = s_frameMs[(s_frameCount * 95) / 100 < s_frameCount - 1 ? (s_frameCount * 95) / 100 : s_frameCount - 1];
        p99 = s_frameMs[(s_frameCount * 99) / 100 < s_frameCount - 1 ? (s_frameCount * 99) / 100 : s_frameCount - 1];
        mx = s_frameMs[s_frameCount - 1];
        for (int i = 0; i < s_frameCount; ++i)
        {
            over20 += s_frameMs[i] >= 20.0f;
            over33 += s_frameMs[i] >= 33.3f;
        }
    }
    sprintf_s(out, size, "\"fps\":%.1f,\"ft\":[%.1f,%.1f,%.1f,%.1f],\"ft20\":%d,\"ft33\":%d,\"cap\":%d,\"vsync\":%d,",
        elapsedMs > 0 ? s_win.frames * 1000.0f / elapsedMs : 0.0f, p50, p95, p99, mx, over20, over33,
        com_maxfps ? com_maxfps->current.integer : -1, r_vsync ? (int)r_vsync->current.enabled : -1);
}
} // namespace

// L9: what the client draws for dying actors and actor corpses, every 250 ms of real time: entity, type (a dead
// actor or a corpse clone), ragdoll handle, pose origin and the world position of j_head (a lying body has its head
// near the floor; a frozen standing one ~60 units up). "k":"body" playtrace lines and bo1_body log lines
// (bo1_measure). Evidence of the death / ragdoll / corpse hand-over; reads only.
static void CG_SP_BodyTrace(int localClientNum, int now)
{
    static int s_next;
    const bool measure = G_SP_MeasureEnabled();
    if ((!measure && !G_SP_PlayTraceOn()) || now < s_next || !cgArray)
        return;
    s_next = now + 250;
    const cg_s *cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    for (int i = 0; i < (1 << g_entNumBits); i = EntNum_NextNet(i)) // mod (L49): was i < 1024; networked 1537+
    {
        centity_s *cent = CG_GetEntity(localClientNum, i);
        if (!cent->nextValid)
            continue;
        const int type = cent->nextState.eType;
        const bool deadActor = type == ET_ACTOR && (cent->nextState.lerp.eFlags & 0x40000) != 0; // KB EF_DEAD
        if (!deadActor && type != ET_ACTOR_CORPSE)
            continue;
        float head[3] = { 0.0f, 0.0f, 0.0f };
        DObj *obj = Com_GetClientDObj(i, localClientNum);
        const int hasHead = obj ? CG_DObjGetWorldTagPos(&cent->pose, obj, scr_const.j_head, head) : 0;
        char line[256];
        sprintf_s(line, "{\"k\":\"body\",\"cgt\":%d,\"e\":%d,\"type\":\"%s\",\"rd\":%d,\"tr\":%d,\"o\":[%.1f,%.1f,%.1f],"
            "\"hashead\":%d,\"head\":[%.1f,%.1f,%.1f]}",
            cgameGlob ? cgameGlob->time : -1, i, deadActor ? "dead" : "corpse", cent->pose.ragdollHandle ? 1 : 0,
            (int)cent->nextState.lerp.pos.trType, cent->pose.origin[0], cent->pose.origin[1], cent->pose.origin[2],
            hasHead ? 1 : 0, head[0], head[1], head[2]);
        if (G_SP_PlayTraceOn())
            G_SP_PlayTraceLine(line);
        if (measure)
            Com_Printf(16, "bo1_body: %s\n", line);
    }
}

void CG_SP_PlayTraceClientFrame(int localClientNum)
{
    {
        clientUIActive_t *bodyUi = CL_GetLocalClientUIGlobals(localClientNum);
        if (bodyUi && bodyUi->connectionState == CA_ACTIVE && clients)
            CG_SP_BodyTrace(localClientNum, (int)Sys_Milliseconds());
    }
    if (!G_SP_PlayTraceOn())
        return;
    clientUIActive_t *ui = CL_GetLocalClientUIGlobals(localClientNum);
    if (!ui || ui->connectionState != CA_ACTIVE || !clients)
    {
        s_win.start = 0;
        s_lastFrameTick = 0;
        return;
    }
    const int now = (int)Sys_Milliseconds();
    if (!s_win.start)
    {
        s_win.lastMessageNum = clients[localClientNum].snap.messageNum;
        s_win.lastServerTime = clients[localClientNum].snap.serverTime;
        ResetWindow(now);
    }
    const clSnapshot_t &snap = clients[localClientNum].snap;
    ++s_win.frames;
    AddFrameTime();
    if (snap.messageNum != s_win.lastMessageNum)
    {
        s_win.messages += snap.messageNum - s_win.lastMessageNum;
        ++s_win.snapshots;
        if (snap.serverTime - s_win.lastServerTime > s_win.maxSnapGap)
            s_win.maxSnapGap = snap.serverTime - s_win.lastServerTime;
        s_win.lastMessageNum = snap.messageNum;
        s_win.lastServerTime = snap.serverTime;
    }
    if (snap.ping < s_win.pingMin) s_win.pingMin = snap.ping;
    if (snap.ping > s_win.pingMax) s_win.pingMax = snap.ping;
    if (clients[localClientNum].extrapolatedSnapshot)
        ++s_win.extrapolated;
    if (now - s_win.start < G_SP_PlayTraceIntervalMs())
        return;
    const dvar_s *hudEnable = Dvar_FindVar("hud_enable");
    const dvar_s *draw2D = Dvar_FindVar("cg_draw2D");
    const cg_s *cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    char line[768];
    char frameFields[160];
    FrameTimeFields(frameFields, sizeof(frameFields), now - s_win.start);
    sprintf_s(line, "{\"k\":\"c\",\"rt\":%d,\"st\":%d,\"cgt\":%d,\"frames\":%d,%s\"msgs\":%d,\"snaps\":%d,\"gap\":%d,"
        "\"ping\":[%d,%d],\"extrap\":%d,\"miss\":%d,\"missmax\":%.1f,\"hud\":{\"draw\":%d,\"should\":%d,\"enable\":%d,"
        "\"draw2d\":%d,\"catchers\":%d}}",
        now, snap.serverTime, cgameGlob ? cgameGlob->time : -1, s_win.frames, frameFields, s_win.messages, s_win.snapshots,
        s_win.maxSnapGap, s_win.pingMin == 0x7FFFFFFF ? -1 : s_win.pingMin, s_win.pingMax, s_win.extrapolated,
        s_win.misses, s_win.missMax, cgameGlob ? (int)cgameGlob->drawHud : -1,
        (int)CG_ShouldDrawHud(localClientNum), hudEnable ? (int)hudEnable->current.enabled : -1,
        draw2D ? (int)draw2D->current.enabled : -1, ui->keyCatchers);
    G_SP_PlayTraceLine(line);
    ResetWindow(now);
}

void CG_SP_PlayTracePredictionMiss(float len)
{
    if (!G_SP_PlayTraceOn())
        return;
    ++s_win.misses;
    if (len > s_win.missMax)
        s_win.missMax = len;
}

void CG_SP_PlayTraceKey(int key, int down, int repeats, const char *binding)
{
    // F8 is unbound in the shipped config_mp.cfg; a user binding on F8 wins over the marker.
    if (key != K_F8 || !down || repeats != 1 || binding || !G_SP_PlayTraceOn())
        return;
    G_SP_PlayTraceMark("F8");
}
