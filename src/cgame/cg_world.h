#pragma once
#include <bgame/bg_local.h>

struct CEntityAreaParms // sizeof=0x18
{                                       // XREF: ?CG_AreaEntities@@YAHQBM0PAHHH@Z/r
    const float *mins;                  // XREF: CG_AreaEntities(float const * const,float const * const,int *,int,int)+297/w
    const float *maxs;                  // XREF: CG_AreaEntities(float const * const,float const * const,int *,int,int)+29D/w
    int contentMask;                    // XREF: CG_AreaEntities(float const * const,float const * const,int *,int,int)+2A3/w
    int *list;                          // XREF: CG_AreaEntities(float const * const,float const * const,int *,int,int)+2A9/w
    int maxCount;                       // XREF: CG_AreaEntities(float const * const,float const * const,int *,int,int)+2AF/w
    int count;                          // XREF: CG_AreaEntities(float const * const,float const * const,int *,int,int)+2B2/w
};

int __cdecl CG_GetEntityBModelContents(const centity_s *cent);
void    CG_GetEntityBModelBounds(
                const centity_s *cent,
                float *mins,
                float *maxs,
                float *absMins,
                float *absMaxs);
void __cdecl CG_GetEntityDobjBounds(
                const centity_s *cent,
                const DObj *dobj,
                float *mins,
                float *maxs,
                float *absMins,
                float *absMaxs);
unsigned int __cdecl CG_GetEntityDObjContents(const centity_s *cent, const DObj *dobj);
bool __cdecl CG_EntityNeedsLinked(int localClientNum, unsigned int entIndex);
void __cdecl CG_UnlinkEntity(int localClientNum, unsigned int entIndex);
void __cdecl CG_CalcWorldBounds(centity_s *cent, DObj *dobj);
void __cdecl CG_LinkEntity(int localClientNum, unsigned int entIndex);
void __cdecl CG_PointTraceToEntity(const pointtrace_t *clip, unsigned int entIndex, trace_t *results);
void __cdecl CG_PointTraceToEntities(const pointtrace_t *clip, trace_t *results);
void __cdecl CG_PointTraceToEntities_r(
                const pointtrace_t *clip,
                unsigned __int16 sectorIndex,
                const float *p1,
                const float *p2,
                trace_t *results);
void __cdecl CG_TraceCapsule(
                trace_t *results,
                const float *start,
                const float *mins,
                const float *maxs,
                const float *end,
                int passEntityNum,
                int contentMask,
                col_context_t *context);
void __cdecl CG_ClipMoveToEntities(const moveclip_t *clip, trace_t *results);
void __cdecl CG_ClipMoveToEntities_r(
                const moveclip_t *clip,
                unsigned __int16 sectorIndex,
                const float *p1,
                const float *p2,
                trace_t *results);
void __cdecl CG_ClipMoveToEntity(const moveclip_t *clip, int entIndex, trace_t *results);
void __cdecl CG_LocationalTrace(
                trace_t *results,
                const float *start,
                const float *end,
                int passEntityNum,
                int contentMask,
                bool checkRopes,
                col_context_t *context);
void __cdecl CG_TracePoint(
                trace_t *results,
                const float *start,
                const float *end,
                int passEntityNum,
                int contentMask,
                bool locational,
                bool staticModels,
                bool checkRopes,
                col_context_t *context);
void __cdecl CG_AreaEntities_r(unsigned __int16 sectorIndex, CEntityAreaParms *areaParms);
char __cdecl CG_EntityInArea(unsigned int entIndex, const float *mins, const float *maxs, int contentMask);
int __cdecl CG_AreaEntities(const float *mins, const float *maxs, int *entityList, int maxEntities, int contentMask);
bool __cdecl CG_SightTracePointInternal(int *hitNum, const float *start, const float *end, int mask, trace_t *trace);
bool __cdecl CG_SightTracePoint(int *hitNum, const float *start, const float *end, int mask, trace_t *trace);
int __cdecl cmpr(float *e0, float *e1);
int __cdecl CG_GetEntityBoneInfo(int entID, int boneIndex, float *bonePos, float (*boneAxis)[3], char **boneName);

extern int cgCollWorldLocalClientNum;

extern float actorLocationalMinsBig[3];
extern float actorLocationalMaxsBig[3];
extern float actorLocationalMins[3];
extern float actorLocationalMaxs[3];