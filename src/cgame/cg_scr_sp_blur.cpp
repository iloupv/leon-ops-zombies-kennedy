#include "cg_scr_sp_blur.h"

// zombies: reset the client's blur transition (SP 0x0043d550).
void CG_SP_ResetBlur(CgSPBlurState *blur)
{
    blur->startTime = 0;
    blur->endTime = 0;
    blur->startRadius = 0.0f;
    blur->endRadius = 0.0f;
    blur->radius = 0.0f;
    blur->priority = 0;
}

// zombies: setblur's client transition (SP 0x0055e720).
void CG_SP_SetBlur(CgSPBlurState *blur, int time, int duration, float radius,
    int timeMode, int priority, float currentRadius)
{
    if (blur->priority <= priority)
    {
        blur->startTime = time;
        blur->endTime = time + duration;
        blur->startRadius = currentRadius;
        blur->timeMode = timeMode;
        blur->endRadius = radius;
        blur->priority = priority;
    }
}

// zombies: startfadingblur starts at radius and fades to zero (SP 0x004378e0).
void CG_SP_StartFadingBlur(CgSPBlurState *blur, int time, int duration, float radius)
{
    if (blur->priority < 2)
    {
        blur->startTime = time;
        blur->startRadius = radius;
        blur->endTime = time + 50 + duration;
        blur->endRadius = 0.0f;
        blur->timeMode = 0;
        blur->priority = 1;
    }
}

// zombies: interpolate and retire the blur timer (SP 0x007724b0).
float CG_SP_UpdateBlur(CgSPBlurState *blur, int time)
{
    if (blur->startTime)
    {
        if (time < blur->endTime)
        {
            blur->radius = (float)(time - blur->startTime) / (float)(blur->endTime - blur->startTime)
                * (blur->endRadius - blur->startRadius) + blur->startRadius;
            return blur->radius;
        }
        blur->startTime = 0;
        blur->endTime = 0;
        blur->priority = 0;
        blur->radius = blur->endRadius;
    }
    return blur->radius;
}
