#include "cm_tracebox.h"
#include "cm_trace.h"

void __cdecl CM_CalcTraceExtents(TraceExtents *extents)
{
    float v1; // [esp+4h] [ebp-Ch]
    float diff; // [esp+8h] [ebp-8h]
    int i; // [esp+Ch] [ebp-4h]

    for ( i = 0; i < 3; ++i )
    {
        diff = extents->start.vec.v[i] - extents->end.vec.v[i];
        if ( diff == 0.0 )
            v1 = 0.0f;
        else
            v1 = 1.0 / diff;
        extents->invDelta.vec.v[i] = v1;
    }
}

int __cdecl intersect_extents_aabb(const TraceExtents *extents, const float *mins, const float *maxs, float fraction)
{
    float v5; // [esp+0h] [ebp-44h]
    float v6; // [esp+4h] [ebp-40h]
    int t; // [esp+24h] [ebp-20h]
    float enterFrac; // [esp+28h] [ebp-1Ch]
    float dist1; // [esp+2Ch] [ebp-18h]
    float frac; // [esp+30h] [ebp-14h]
    float fraca; // [esp+30h] [ebp-14h]
    float sign; // [esp+34h] [ebp-10h]
    float dist2; // [esp+3Ch] [ebp-8h]

    enterFrac = 0.0f;
    sign = -1.0f;
    while ( 1 )
    {
        const float *bounds = mins;
        nanassertvec3(bounds);

        for ( t = 0; t < 3; ++t )
        {
            dist1 = (float)(extents->start.vec.v[t] - mins[t]) * sign;
            dist2 = (float)(extents->end.vec.v[t] - mins[t]) * sign;
            if ( dist1 <= 0.0 )
            {
                if ( dist2 > 0.0 )
                {
                    fraca = (float)(dist1 * extents->invDelta.vec.v[t]) * sign;
                    if ( enterFrac >= fraca )
                        return 0;
                    if ( (float)(fraca - fraction) < 0.0 )
                        v5 = (float)(dist1 * extents->invDelta.vec.v[t]) * sign;
                    else
                        v5 = fraction;
                    fraction = v5;
                }
            }
            else
            {
                if ( dist2 > 0.0 )
                    return 0;
                frac = (float)(dist1 * extents->invDelta.vec.v[t]) * sign;
                if ( frac >= fraction )
                    return 0;
                if ( (float)(enterFrac - frac) < 0.0 )
                    v6 = (float)(dist1 * extents->invDelta.vec.v[t]) * sign;
                else
                    v6 = enterFrac;
                enterFrac = v6;
            }
        }
        if ( sign == 1.0 )
            break;
        sign = 1.0f;
        mins = maxs;
    }
    return 1;
}

int __cdecl intersect_extents_aabb(
                const TraceExtents *extents,
                const float *mins,
                const float *maxs,
                float *enterFrac,
                float *leaveFrac)
{
    float v6; // [esp+0h] [ebp-44h]
    float v7; // [esp+4h] [ebp-40h]
    int t; // [esp+2Ch] [ebp-18h]
    float dist1; // [esp+30h] [ebp-14h]
    float frac; // [esp+34h] [ebp-10h]
    float fraca; // [esp+34h] [ebp-10h]
    float sign; // [esp+38h] [ebp-Ch]
    float dist2; // [esp+40h] [ebp-4h]

    sign = -1.0f;
    while ( 1 )
    {

        const float *bounds = mins;
        nanassertvec3(bounds);

        for ( t = 0; t < 3; ++t )
        {
            dist1 = (float)(extents->start.vec.v[t] - mins[t]) * sign;
            dist2 = (float)(extents->end.vec.v[t] - mins[t]) * sign;
            if ( dist1 <= 0.0 )
            {
                if ( dist2 > 0.0 )
                {
                    fraca = (float)(dist1 * extents->invDelta.vec.v[t]) * sign;
                    if ( *enterFrac >= fraca )
                        return 0;
                    if ( (float)(fraca - *leaveFrac) < 0.0 )
                        v6 = (float)(dist1 * extents->invDelta.vec.v[t]) * sign;
                    else
                        v6 = *leaveFrac;
                    *leaveFrac = v6;
                }
            }
            else
            {
                if ( dist2 > 0.0 )
                    return 0;
                frac = (float)(dist1 * extents->invDelta.vec.v[t]) * sign;
                if ( frac >= *leaveFrac )
                    return 0;
                if ( (float)(*enterFrac - frac) < 0.0 )
                    v7 = (float)(dist1 * extents->invDelta.vec.v[t]) * sign;
                else
                    v7 = *enterFrac;
                *enterFrac = v7;
            }
        }
        if ( sign == 1.0 )
            break;
        sign = 1.0f;
        mins = maxs;
    }
    return 1;
}

bool __cdecl intersect_extents_sphere(const TraceExtents *extents, const float *origin, float radius, float fraction)
{
    float v5; // [esp+4h] [ebp-44h]
    float v6; // [esp+8h] [ebp-40h]
    float v7; // [esp+10h] [ebp-38h]
    float delta; // [esp+18h] [ebp-30h]
    float delta_4; // [esp+1Ch] [ebp-2Ch]
    float delta_8; // [esp+20h] [ebp-28h]
    float sphereFraction; // [esp+24h] [ebp-24h]
    float startOffset; // [esp+28h] [ebp-20h]
    float startOffset_4; // [esp+2Ch] [ebp-1Ch]
    float startOffset_8; // [esp+30h] [ebp-18h]

    startOffset = *origin - extents->start.vec.v[0];
    startOffset_4 = origin[1] - extents->start.vec.v[1];
    startOffset_8 = origin[2] - extents->start.vec.v[2];
    delta = extents->end.vec.v[0] - extents->start.vec.v[0];
    delta_4 = extents->end.vec.v[1] - extents->start.vec.v[1];
    delta_8 = extents->end.vec.v[2] - extents->start.vec.v[2];
    if ( (-((float)((float)(delta * delta) + (float)(delta_4 * delta_4)) + (float)(delta_8 * delta_8))) < 0.0 )
        v6 = (float)((float)(delta * delta) + (float)(delta_4 * delta_4)) + (float)(delta_8 * delta_8);
    else
        v6 = 1.0f;
    if ( v6 == 0.0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_tracebox.cpp", 151, 0, "%s", "lengthSq") )
    {
        __debugbreak();
    }
    sphereFraction = (float)((float)((float)(startOffset * delta) + (float)(startOffset_4 * delta_4))
                                                 + (float)(startOffset_8 * delta_8))
                                 / v6;
    if ( (float)(sphereFraction - fraction) < 0.0 )
        v7 = (float)((float)((float)(startOffset * delta) + (float)(startOffset_4 * delta_4))
                             + (float)(startOffset_8 * delta_8))
             / v6;
    else
        v7 = fraction;
    if ( (float)(0.0 - sphereFraction) < 0.0 )
        v5 = v7;
    else
        v5 = 0.0f;

    // jesus someone clean this up
    return (float)(radius * radius) > (float)((float)((float)((float)((float)((-(v5))
                                                                                                                                                    * delta)
                                                                                                                                    + startOffset)
                                                                                                                    * (float)((float)((-(v5))
                                                                                                                                                    * delta)
                                                                                                                                    + startOffset))
                                                                                                    + (float)((float)((float)((-(v5))
                                                                                                                                                    * delta_4)
                                                                                                                                    + startOffset_4)
                                                                                                                    * (float)((float)((-(v5))
                                                                                                                                                    * delta_4)
                                                                                                                                    + startOffset_4)))
                                                                                    + (float)((float)((float)((-(v5))
                                                                                                                                    * delta_8)
                                                                                                                    + startOffset_8)
                                                                                                    * (float)((float)((-(v5))
                                                                                                                                    * delta_8)
                                                                                                                    + startOffset_8)));
}

