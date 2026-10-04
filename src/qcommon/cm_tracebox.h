#pragma once

struct TraceExtents;

void __cdecl CM_CalcTraceExtents(TraceExtents *extents);
int __cdecl intersect_extents_aabb(const TraceExtents *extents, const float *mins, const float *maxs, float fraction);
int __cdecl intersect_extents_aabb(
                const TraceExtents *extents,
                const float *mins,
                const float *maxs,
                float *enterFrac,
                float *leaveFrac);
bool __cdecl intersect_extents_sphere(const TraceExtents *extents, const float *origin, float radius, float fraction);
