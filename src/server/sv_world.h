#pragma once

struct svEntity_s;
struct gentity_s;
struct trace_t;
struct col_context_t;
struct pointtrace_t;
struct DObj;
struct moveclip_t;
struct sightpointtrace_t;
struct sightclip_t;
struct IgnoreEntParams;


unsigned int __cdecl SV_ClipHandleForEntity(const gentity_s *ent);
void __cdecl SV_UnlinkEntity(gentity_s *gEnt);
void    SV_LinkEntity(gentity_s *gEnt);
void __cdecl SnapAngles(float *vAngles);
void __cdecl SV_TraceCapsuleToEntity(const moveclip_t *clip, svEntity_s *check, trace_t *trace);
void __cdecl SV_TracePointToEntity(const pointtrace_t *clip, svEntity_s *check, trace_t *trace);
DObj *__cdecl SV_LocationalTraceDObj(const pointtrace_t *clip, const gentity_s *touch);
int __cdecl SV_SightTraceCapsuleToEntity(const sightclip_t *clip, int entnum);
int __cdecl SV_SightTracePointToEntity(const sightpointtrace_t *clip, int entnum);
DObj *__cdecl SV_LocationalSightTraceDObj(const sightpointtrace_t *clip, const gentity_s *touch);
void __cdecl SV_SetupIgnoreEntParams(IgnoreEntParams *ignoreEntParams, int baseEntity);
void __cdecl SV_TracePoint(trace_t *results, const float *start, const float *end, col_context_t *context);
void __cdecl SV_TraceCapsule(
                trace_t *results,
                const float *start,
                const float *mins,
                const float *maxs,
                const float *end,
                col_context_t *context);
bool __cdecl SV_SightTraceCapsule(
                int *hitNum,
                const float *start,
                const float *mins,
                const float *maxs,
                const float *end,
                col_context_t *context);
bool __cdecl SV_SightTracePoint(int *hitNum, const float *start, const float *end, col_context_t *context);
int __cdecl SV_PointContents(float *p, int passEntityNum, int contentmask);
void __cdecl G_TraceCapsule(
                trace_t *results,
                const float *start,
                const float *mins,
                const float *maxs,
                const float *end,
                int passEntityNum,
                int contentmask,
                col_context_t *context);
void __cdecl G_LocationalTrace(
                trace_t *results,
                const float *start,
                const float *end,
                int passEntityNum,
                int contentmask,
                unsigned __int8 *priorityMap,
                int (__cdecl *collide_entity_func)(int, col_context_t *));
void __cdecl G_LocationalTraceAllowChildren(
                trace_t *results,
                const float *start,
                const float *end,
                int passEntityNum,
                int contentmask,
                unsigned __int8 *priorityMap);
