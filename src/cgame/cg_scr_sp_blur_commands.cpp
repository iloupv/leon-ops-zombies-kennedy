#include "cg_scr_sp_blur.h"
#include <cgame_mp/cg_local_mp.h>
#include <qcommon/cmd.h>
#include <win32/win_shared.h>

static CgSPBlurState cg_spBlur[MAX_LOCAL_CLIENTS];

void CG_SP_ResetClientBlur(int localClientNum)
{
    iassert(localClientNum >= 0 && localClientNum < MAX_LOCAL_CLIENTS);
    CG_SP_ResetBlur(&cg_spBlur[localClientNum]);
}

float CG_SP_GetBlurRadius(int localClientNum)
{
    iassert(localClientNum >= 0 && localClientNum < MAX_LOCAL_CLIENTS);
    if (!Dvar_GetBool("zombiemode"))
        return 0.0f;
    CgSPBlurState *blur = &cg_spBlur[localClientNum];
    int time = CG_GetLocalClientGlobals(localClientNum)->time;
    if (blur->timeMode == 1)
        time = Sys_Milliseconds();
    return CG_SP_UpdateBlur(blur, time);
}

// zombies: receive setblur's "A <ms> <radius> <time mode> <priority>" (SP 0x0078d6d0).
void CG_SP_BlurServerCommand(int localClientNum)
{
    int duration = atoi(Cmd_Argv(1));
    float radius = (float)atof(Cmd_Argv(2));
    int timeMode = atoi(Cmd_Argv(3));
    int priority = atoi(Cmd_Argv(4));
    if (!duration && radius <= 0.0f && !timeMode && !priority)
    {
        CG_SP_ResetClientBlur(localClientNum);
        return;
    }
    cg_s *cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    int time = timeMode == 1 ? Sys_Milliseconds() : cgameGlob->time;
    CG_SP_SetBlur(&cg_spBlur[localClientNum], time, duration, radius, timeMode, priority,
        cgameGlob->refdef.blurRadius);
}

// zombies: receive startfadingblur's "| <ms> <radius>" (SP 0x0078d7e0).
void CG_SP_FadingBlurServerCommand(int localClientNum)
{
    int duration = atoi(Cmd_Argv(1));
    float radius = (float)atof(Cmd_Argv(2));
    CG_SP_StartFadingBlur(&cg_spBlur[localClientNum], CG_GetLocalClientGlobals(localClientNum)->time,
        duration, radius);
}

// zombies: SP 0x0055e720 - the transition setblur (client method, SP 0x00896d40) starts: from the
// current blur radius to <radius> over <duration> ms, on the system clock when timeMode is 1.
void CG_SP_StartClientBlur(int localClientNum, int duration, float radius, int timeMode, int priority)
{
    iassert(localClientNum >= 0 && localClientNum < MAX_LOCAL_CLIENTS);
    cg_s *cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    int time = timeMode == 1 ? Sys_Milliseconds() : cgameGlob->time;
    CG_SP_SetBlur(&cg_spBlur[localClientNum], time, duration, radius, timeMode, priority,
        cgameGlob->refdef.blurRadius);
}
