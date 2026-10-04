#include "cg_sp_client_ext.h"
#include <cgame_mp/cg_local_mp.h>
#include <qcommon/common.h>
#include <qcommon/cmd.h>
#include <win32/win_shared.h>
#include <qcommon/cm_trace.h>
#include <universal/q_shared.h>

static CgSPClientExt cg_spClientExt[MAX_LOCAL_CLIENTS];
static const dvar_s *cg_spVisuals;
static const dvar_s *cg_spVisualShots;
static const dvar_s *cg_spVisualShotDelay;
static const dvar_s *cg_spVisualFrame;
static const dvar_s *cg_spVisualDumpAt;
static unsigned int s_visualDumpsDone[MAX_LOCAL_CLIENTS];

// Opt-in evidence: bo1_visuals_shots "powerup_on,ape_spawn" takes one screenshot ("bo1_fx<i>_<cg time>", i = the
// matching entry) after the first traced effect / vision whose name contains entry i.
// The default delay is 600 ms; shorten it to capture a transient such as the teleport flare. An entry may carry its own
// delay as "name@ms" (e.g. "electrified@300,electrified_clear@2500"). Up to 8 captures may be pending at once.
static unsigned int s_visualShotsTaken;
static int s_visualShotDue[8];
static char s_visualShotName[8][64];

void CG_SP_VisualShot(int localClientNum, const char *name)
{
    if (!CG_SP_VisualsEnabled() || !cg_spVisualShots || !name)
        return;
    int slot = 0;
    while (slot < 8 && s_visualShotDue[slot])
        ++slot;
    if (slot == 8)
        return;
    const char *p = cg_spVisualShots->current.string;
    for (int i = 0; i < 32 && *p; ++i)
    {
        char entry[64];
        int len = 0;
        while (*p == ',' || *p == ' ')
            ++p;
        while (*p && *p != ',' && *p != ' ')
        {
            if (len < 63)
                entry[len++] = *p;
            ++p;
        }
        entry[len] = 0;
        int delay = cg_spVisualShotDelay->current.integer;
        char *at = strchr(entry, '@');
        if (at)
        {
            *at = 0;
            delay = atoi(at + 1);
        }
        if (!entry[0] || (s_visualShotsTaken & (1u << i)) || !strstr(name, entry))
            continue;
        s_visualShotsTaken |= 1u << i;
        s_visualShotDue[slot] = (int)Sys_Milliseconds() + (delay > 0 ? delay : 1);
        Com_sprintf(s_visualShotName[slot], sizeof(s_visualShotName[slot]), "bo1_fx%d_%d", i, CG_GetLocalClientGlobals(localClientNum)->time);
        Com_Printf(14, "BO1_VISUAL shot time=%d client=%d match=%s name=%s file=%s delay=%d\n",
            CG_GetLocalClientGlobals(localClientNum)->time, localClientNum, entry, name, s_visualShotName[slot], delay);
        return;
    }
}

static void CG_SP_VisualShotFrame()
{
    // one capture per frame, the earliest due first
    int best = -1;
    for (int slot = 0; slot < 8; ++slot)
    {
        if (s_visualShotDue[slot] && (int)Sys_Milliseconds() >= s_visualShotDue[slot]
            && (best < 0 || s_visualShotDue[slot] < s_visualShotDue[best]))
            best = slot;
    }
    if (best < 0)
        return;
    s_visualShotDue[best] = 0;
    Cbuf_AddText(0, va("screenshotJpeg %s\n", s_visualShotName[best]));
}

// Opt-in evidence (p1 chunk 26): bo1_visuals_frame N logs the renderer's effective view values (film, bloom, dof,
// blur, exposure, fog, sun: BO1_RENDERVIS in r_scene.cpp) on every frame they change, and at least every N ms.
int CG_SP_VisualsFrameMs()
{
    if (!zombiemode || !zombiemode->current.enabled || !cg_spVisualFrame)
        return 0;
    return cg_spVisualFrame->current.integer;
}

// bo1_visuals_dumpat "t1,t2,...": at the first frame with cg time >= t, CG_SP_DumpVisionSets prints every channel's
// name and lerp record and the naked channel's 68 fields (BO1_VISDUMP), for a field diff against a retail memdump.
void CG_SP_VisionDumpFrame(int localClientNum)
{
    if (!zombiemode || !zombiemode->current.enabled || !cg_spVisualDumpAt || !cg_spVisualDumpAt->current.string[0])
        return;
    const int time = CG_GetLocalClientGlobals(localClientNum)->time;
    const char *p = cg_spVisualDumpAt->current.string;
    for (unsigned int i = 0; *p && i < 32; ++i)
    {
        const int t = atoi(p);
        if (!(s_visualDumpsDone[localClientNum] & (1u << i)) && time >= t)
        {
            s_visualDumpsDone[localClientNum] |= 1u << i;
            CG_SP_DumpVisionSets(localClientNum, t);
        }
        while (*p && *p != ',')
            ++p;
        if (*p == ',')
            ++p;
    }
}

bool CG_SP_VisualsEnabled()
{
    return zombiemode && zombiemode->current.enabled && cg_spVisuals && cg_spVisuals->current.enabled;
}

CgSPClientExt *CG_SP_GetClientExt(int localClientNum)
{
    iassert(localClientNum >= 0 && localClientNum < MAX_LOCAL_CLIENTS);
    return &cg_spClientExt[localClientNum];
}

// zombies: the SP-only tail of CG_InitVisionSets (SP 0x0047c180): the four extra channels' default
// names, and the water set loaded straight into its current settings. SP's cg_s memset
// (CG_Init) is what clears the rest; this side table is cleared here instead.
void CG_SP_InitVisionSets(int localClientNum)
{
    CgSPClientExt *ext = CG_SP_GetClientExt(localClientNum);
    memset(ext, 0, sizeof(*ext));
    cg_spVisuals = _Dvar_RegisterBool("bo1_visuals", false, 0,
        "Log zombie client vision requests, renderer values and script effects");
    cg_spVisualShots = _Dvar_RegisterString("bo1_visuals_shots", "", 0,
        "With bo1_visuals: name fragments; one screenshot after the first traced effect/vision matching each");
    cg_spVisualShotDelay = _Dvar_RegisterInt("bo1_visuals_shotdelay", 600, 0, 5000, 0,
        "With bo1_visuals_shots: delay in real milliseconds before capturing a matching effect/vision");
    cg_spVisualFrame = _Dvar_RegisterInt("bo1_visuals_frame", 0, 0, 600000, 0,
        "Log the renderer's effective view values on every frame they change and at least every N ms (0 = off)");
    cg_spVisualDumpAt = _Dvar_RegisterString("bo1_visuals_dumpat", "", 0,
        "Comma-separated cg times: dump every vision channel's name/lerp and the naked channel's fields once each");
    s_visualDumpsDone[localClientNum] = 0;
    if (!zombiemode || !zombiemode->current.enabled)
        return;
    I_strncpyz(ext->visionName[SP_VISIONSET_LASTSTAND - SP_VISIONSET_FIRST], "laststand", 64);
    I_strncpyz(ext->visionName[SP_VISIONSET_DEATH - SP_VISIONSET_FIRST], "death", 64);
    I_strncpyz(ext->visionName[SP_VISIONSET_LOWHEALTH - SP_VISIONSET_FIRST], "low_health", 64);
    I_strncpyz(ext->visionName[SP_VISIONSET_UNDERWATER - SP_VISIONSET_FIRST], "creek_1_water", 64);
    if (GetVisionSet(localClientNum, ext->visionName[SP_VISIONSET_UNDERWATER - SP_VISIONSET_FIRST],
        &ext->visionSetCurrent[SP_VISIONSET_UNDERWATER - SP_VISIONSET_FIRST]))
    {
        ext->visionSetLerpData[SP_VISIONSET_UNDERWATER - SP_VISIONSET_FIRST].style = VISIONSETLERP_NONE;
    }
}

// zombies: CG_VisionSetStartLerp (SP 0x006699f0). Channels 0..5 are KB's own and go through KB's
// CG_VisionSetStartLerp_To (same branches: a timed lerp only when the channel already holds a
// set, otherwise load it as the current one). Channels 6..9 live in the side table. SP loads every
// channel through R_LoadVisionFile (0x00794af0), which is the 8-slot preload cache = KB's GetVisionSet
// (checked against the exe: 0x006699f0 calls it at 0x00669a38 / 0x00669aa3; CG_InitVisionSets' creek_1_water
// load at 0x0047c2c2 too), so channels 6..9 take cache slots like 0..5. (The earlier note here said SP
// bypassed the cache; the exe does not.)
bool CG_SP_VisionSetStartLerp(int localClientNum, int mode, visionSetLerpStyle_t style, const char *name, int duration)
{
    if (mode < SP_VISIONSET_FIRST)
        return CG_VisionSetStartLerp_To(localClientNum, (visionSetMode_t)mode, style, name, duration);

    if (CG_SP_VisualsEnabled())
    {
        Com_Printf(14, "BO1_VISUAL vision time=%d client=%d mode=%d name=%s duration=%d\n",
            CG_GetLocalClientGlobals(localClientNum)->time, localClientNum, mode, name, duration);
        CG_SP_VisualShot(localClientNum, name);
    }
    iassert(mode < SP_VISIONSET_COUNT);
    CgSPClientExt *ext = CG_SP_GetClientExt(localClientNum);
    int slot = mode - SP_VISIONSET_FIRST;
    if (duration > 0 && ext->visionSetLerpData[slot].style != VISIONSETLERP_UNDEFINED)
    {
        if (!GetVisionSet(localClientNum, name, &ext->visionSetTo[slot]))
            return false;
        memcpy(&ext->visionSetFrom[slot], &ext->visionSetCurrent[slot], sizeof(ext->visionSetFrom[slot]));
        ext->visionSetLerpData[slot].style = style;
        ext->visionSetLerpData[slot].timeDuration = duration;
        ext->visionSetLerpData[slot].timeStart = CG_GetLocalClientGlobals(localClientNum)->time;
        return true;
    }
    if (!GetVisionSet(localClientNum, name, &ext->visionSetCurrent[slot]))
        return false;
    ext->visionSetLerpData[slot].style = VISIONSETLERP_NONE;
    return true;
}

// zombies: per-frame update of the SP-only channels, beside KB's CG_VisionSetsUpdate. The lerp is
// KB's UpdateVarsLerp (the MP twin of SP's per-channel walk). The underwater flag is the one SP's
// CG_CalcViewValues (0x00793fb0, 0x00794425) sets from the view origin: 0x005fcb10 when the eye is
// at or below the water height, 0x00471fb0 otherwise. It is sampled here, once per frame, from the
// refdef the view code last produced.
void CG_SP_VisionSetsUpdate(int localClientNum)
{
    if (!zombiemode || !zombiemode->current.enabled)
        return;
    CG_SP_VisualShotFrame();
    cg_s *cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    CgSPClientExt *ext = CG_SP_GetClientExt(localClientNum);
    float waterHeight = (float)CM_GetWaterHeight(cgameGlob->refdef.vieworg, 200.0f, -200.0f);
    ext->underwater = !(cgameGlob->refdef.vieworg[2] > waterHeight);
    for (int slot = 0; slot < SP_VISIONSET_COUNT - SP_VISIONSET_FIRST; ++slot)
    {
        UpdateVarsLerp(
            cgameGlob->time,
            &ext->visionSetFrom[slot],
            &ext->visionSetTo[slot],
            &ext->visionSetLerpData[slot],
            &ext->visionSetCurrent[slot]);
    }
}

// zombies: CG_SelectVisionSet (SP 0x0045ea30), the part KB's channel choice lacks. SP tests, in
// order: flare (channel 2) first, then cg+0xccd31 -> 8, pm_type 6 or 7 -> 6, time < cg+0xccd34 -> 7,
// underwater -> 9, and only then infrared / extracam / tvguided / night / naked, which KB's
// CG_VisionSetApplyToRefdef already picks. Channel 6 is selected whether or not it holds a set (an
// empty one draws with the film off, as in SP); its loader is server command 'F'
// (CG_ConfigString_VisionSetLastStand 0x005b5ce0, cg_sp_servercmds.cpp).
static int CG_SP_SelectVisionSet(int localClientNum, visionSetMode_t channel)
{
    if (channel == VISIONSETMODE_FLARE)
        return -1;
    cg_s *cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    CgSPClientExt *ext = CG_SP_GetClientExt(localClientNum);
    if (ext->vsVision)
        return SP_VISIONSET_LOWHEALTH;
    if (cgameGlob->predictedPlayerState.pm_type == 6 || cgameGlob->predictedPlayerState.pm_type == 7)
        return SP_VISIONSET_LASTSTAND;
    if (cgameGlob->time < ext->damageVisionEndTime)
        return SP_VISIONSET_DEATH;
    if (ext->underwater)
        return SP_VISIONSET_UNDERWATER;
    return -1;
}

// zombies: when SP would draw one of channels 6..9, CG_VisionSetApplyToRefdef draws it through
// KB's chosen channel: that channel's current settings and lerp style are swapped for the SP
// channel's for the call and put back by CG_SP_VisionSetApplyEnd. A style of UNDEFINED takes KB's
// "no vision set" branch, as SP's does (film and revive fx off).
bool CG_SP_VisionSetApplyBegin(int localClientNum, visionSetMode_t channel, CgSPVisionSwap *swap)
{
    swap->active = false;
    if (!zombiemode || !zombiemode->current.enabled)
        return false;
    int mode = CG_SP_SelectVisionSet(localClientNum, channel);
    if (mode < 0)
        return false;
    cg_s *cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    CgSPClientExt *ext = CG_SP_GetClientExt(localClientNum);
    int slot = mode - SP_VISIONSET_FIRST;
    swap->active = true;
    swap->channel = channel;
    swap->style = cgameGlob->visionSetLerpData[channel].style;
    memcpy(&swap->saved, &cgameGlob->visionSetCurrent[channel], sizeof(swap->saved));
    memcpy(&cgameGlob->visionSetCurrent[channel], &ext->visionSetCurrent[slot], sizeof(swap->saved));
    cgameGlob->visionSetLerpData[channel].style = ext->visionSetLerpData[slot].style;
    return true;
}

void CG_SP_VisionSetApplyEnd(int localClientNum, CgSPVisionSwap *swap)
{
    if (!swap->active)
        return;
    cg_s *cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    memcpy(&cgameGlob->visionSetCurrent[swap->channel], &swap->saved, sizeof(swap->saved));
    cgameGlob->visionSetLerpData[swap->channel].style = swap->style;
    swap->active = false;
}
