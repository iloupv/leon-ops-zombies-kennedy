#pragma once

// SP's per-local-client blur record (0x00bf88f0), kept outside cg_s.
struct CgSPBlurState
{
    int priority;
    int timeMode;
    int startTime;
    int endTime;
    float startRadius;
    float endRadius;
    float radius;
};

void CG_SP_ResetBlur(CgSPBlurState *blur);
void CG_SP_SetBlur(CgSPBlurState *blur, int time, int duration, float radius,
    int timeMode, int priority, float currentRadius);
void CG_SP_StartFadingBlur(CgSPBlurState *blur, int time, int duration, float radius);
float CG_SP_UpdateBlur(CgSPBlurState *blur, int time);

void CG_SP_BlurServerCommand(int localClientNum);
void CG_SP_FadingBlurServerCommand(int localClientNum);
// Reset beside cg_s clears and on map restart; CG_CalcViewValues includes the returned
// radius squared in refdef.blurRadius's sum (SP 0x00793fb0). The getter is zombiemode-only.
void CG_SP_ResetClientBlur(int localClientNum);
float CG_SP_GetBlurRadius(int localClientNum);
// zombies: the client script setblur transition on a local client (SP 0x0055e720).
void CG_SP_StartClientBlur(int localClientNum, int duration, float radius, int timeMode, int priority);
