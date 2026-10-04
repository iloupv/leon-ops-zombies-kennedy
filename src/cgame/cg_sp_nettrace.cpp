// a1 chunk 6: snapshot latency evidence (TEST SWITCH bo1_nettrace, headless -Client only, measurement only).
// Prints the listen-server snapshot pipeline so tools/a1_nettrace.py can say where the client's latest snapshot
// age comes from:
//   "knet cl ..."  every client frame after CG_PredictPlayerState: cls.realtime, cl.serverTime/serverTimeDelta,
//                  cl.snap (latest parsed) serverTime/ps.commandTime, cg.snap/nextSnap serverTime/ps.commandTime,
//                  the predicted commandTime and the newest usercmd's serverTime/number.
//   "knet cmd ..." each usercmd of the local (non-bot) client run by SV_ClientThink: its serverTime, svs.time.
//   "knet snap ..." each snapshot built for the local client: svs.time, the server ps.commandTime in it.
//   "knet sv ..."  server thread frame wake (stage 0, before SV_PreFrame) and after SV_PostFrame (stage 1).
// Every line ends in t=<Sys_Milliseconds> so the server- and client-thread lines can be put on one clock.
#include "cg_sp_nettrace.h"
#include "cg_hudelem.h"
#include <cgame_mp/cg_local_mp.h>
#include <client/client.h>
#include <client_mp/cl_main_mp.h>
#include <server_mp/sv_main_mp.h>
#include <game_mp/g_main_mp.h>
#include <qcommon/common.h>
#include <universal/dvar.h>
#include <win32/win_main.h>
#include <win32/win_shared.h>

static const dvar_s *s_netTrace;

bool CG_SP_NetTraceEnabled()
{
    return s_netTrace && s_netTrace->current.enabled;
}

void CG_SP_NetTraceClientFrame(int localClientNum)
{
    if (!Sys_IsHeadlessClient())
        return;
    if (!s_netTrace)
        s_netTrace = _Dvar_RegisterBool("bo1_nettrace", false, 0,
            "TEST SWITCH (headless client): log the snapshot / usercmd pipeline every client frame");
    if (!s_netTrace->current.enabled)
        return;
    const clientActive_t *cl = CL_GetLocalClientGlobals(localClientNum);
    const cg_s *cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    const usercmd_s *cmd = &cl->cmds[cl->cmdNumber & 0x7F];
    Com_Printf(16, "knet cl rt=%d st=%d dt=%d cls=%d clsct=%d cgt=%d s=%d sct=%d n=%d nct=%d pct=%d cmd=%d cmdn=%d ext=%d t=%u\n",
        cls.realtime, cl->serverTime, cl->serverTimeDelta, cl->snap.serverTime, cl->snap.ps.commandTime, cgameGlob->time,
        cgameGlob->snap ? cgameGlob->snap->serverTime : -1, cgameGlob->snap ? cgameGlob->snap->ps.commandTime : -1,
        cgameGlob->nextSnap ? cgameGlob->nextSnap->serverTime : -1, cgameGlob->nextSnap ? cgameGlob->nextSnap->ps.commandTime : -1,
        cgameGlob->predictedPlayerState.commandTime, cmd->serverTime, cl->cmdNumber, cl->extrapolatedSnapshot,
        Sys_Milliseconds());
}

void CG_SP_NetTraceServerCmd(const client_t *cl, const usercmd_s *cmd)
{
    if (!CG_SP_NetTraceEnabled() || cl->bIsTestClient)
        return;
    Com_Printf(16, "knet cmd c=%d ct=%d sv=%d t=%u\n", (int)(cl - svs.clients), cmd->serverTime, svs.time, Sys_Milliseconds());
}

void CG_SP_NetTraceServerSnap(const client_t *cl)
{
    if (!CG_SP_NetTraceEnabled() || cl->bIsTestClient)
        return;
    const int clientNum = (int)(cl - svs.clients);
    const gclient_s *client = g_entities[clientNum].client;
    Com_Printf(16, "knet snap c=%d sv=%d psct=%d t=%u\n", clientNum, svs.time, client ? client->ps.commandTime : -1,
        Sys_Milliseconds());
}

void CG_SP_NetTraceServerFrame(int stage)
{
    if (!CG_SP_NetTraceEnabled())
        return;
    Com_Printf(16, "knet sv stage=%d sv=%d t=%u\n", stage, svs.time, Sys_Milliseconds());
}

// L13: TEST SWITCHES (measurement only, no behaviour change; any client, not only headless).
//   bo1_hudtrace:   "khud ..." each client frame for every hud elem of cg.snap whose type / fontScale / colour /
//                     glow colour / value / text / label changed since the last print for that slot: the values
//                     the client received (wire precision), i.e. what it draws.
//   bo1_shocktrace: "kshock ..." each client frame UpdateShellShockCamera runs: time, base, fraction, fade time,
//                     viewDelta (cg_shellshock.cpp).
static const dvar_s *s_hudTrace;
static const dvar_s *s_shockTrace;

bool CG_SP_ShockTraceEnabled()
{
    return s_shockTrace && s_shockTrace->current.enabled;
}

void CG_SP_HudTraceClientFrame(int localClientNum)
{
    if (!s_hudTrace)
    {
        s_hudTrace = _Dvar_RegisterBool("bo1_hudtrace", false, 0,
            "TEST SWITCH: log hud elem fontScale / alpha / value as the client receives them");
        s_shockTrace = _Dvar_RegisterBool("bo1_shocktrace", false, 0,
            "TEST SWITCH: log the shellshock camera kick every client frame");
    }
    if (!s_hudTrace->current.enabled)
        return;
    const cg_s *cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if (!cgameGlob->snap)
        return;
    static hudelem_s last[2][31];
    for (int arr = 0; arr < 2; ++arr)
    {
        const hudelem_s *elems = arr ? cgameGlob->snap->ps.hud.current : cgameGlob->snap->ps.hud.archival;
        for (int i = 0; i < 31; ++i)
        {
            const hudelem_s *e = &elems[i];
            hudelem_s *l = &last[arr][i];
            if (e->type == l->type && e->fontScale == l->fontScale && e->color.rgba == l->color.rgba
                && e->glowColor.rgba == l->glowColor.rgba && e->value == l->value && e->text == l->text
                && e->label == l->label)
                continue;
            *l = *e;
            if (!e->type)
                continue;
            char text[260];
            text[0] = 0;
            SafeTranslateHudElemString(localClientNum, e->text ? e->text : e->label, text);
            Com_Printf(16, "khud t=%d %s[%d] type=%d fs=%.4f a=%d ga=%d rgb=%d,%d,%d fa=%d fadeTime=%d value=%g text=%d label=%d str=\"%s\"\n",
                cgameGlob->time, arr ? "cur" : "arc", i, e->type, e->fontScale, e->color.a, e->glowColor.a,
                e->color.r, e->color.g, e->color.b, e->fromColor.a, e->fadeTime, e->value, e->text, e->label, text);
        }
    }
}
