#pragma once

#include "DynEntity_gamestate.h"
#include <universal/q_shared.h>
#include <qcommon/cm_trace.h>
#include <universal/dvar.h>

struct __declspec(align(4)) DynEntSortStruct // sizeof=0x8
{                                       // XREF: DynEntSv_GetClosestEntities/r
    float distSq;                       // XREF: DynEntSv_GetClosestEntities+FE/w
    unsigned __int16 id;                // XREF: DynEntSv_GetClosestEntities+7B/w
                                        // DynEntSv_GetClosestEntities+1CC/r
    // padding byte
    // padding byte
};

void __cdecl DynEntSv_RegisterDvars();
void __cdecl DynEntSv_InitEntities();
void __cdecl DynEntSv_LinkModel(unsigned __int16 dynEntId);
void __cdecl DynEntSv_LinkBrush(unsigned __int16 dynEntId);
void __cdecl DynEntSv_PointTrace(const pointtrace_t *clip, trace_t *results);
void __cdecl DynEntSv_PointTrace_r(
                DynEntityDrawType drawType,
                const pointtrace_t *clip,
                unsigned int sectorIndex,
                float *p1,
                float *p2,
                trace_t *results);
void __cdecl DynEntSv_PointTrace(
                unsigned __int16 id,
                DynEntityDrawType drawType,
                const pointtrace_t *clip,
                trace_t *results);
void __cdecl DynEntSv_ClipMoveTrace(const moveclip_t *clip, trace_t *results);
void __cdecl DynEntSv_ClipMoveTrace_r(
                const moveclip_t *clip,
                unsigned int sectorIndex,
                float *p1,
                float *p2,
                trace_t *results);
unsigned __int16 __cdecl DynEntSv_AreaEntities(
                DynEntityDrawType drawType,
                const float *mins,
                const float *maxs,
                int contentMask,
                unsigned __int16 dynEntMaxCount,
                unsigned __int16 *dynEntList);
void __cdecl DynEntSv_AreaEntities_r(
                DynEntityDrawType drawType,
                unsigned int sectorIndex,
                DynEntityAreaParms *areaParms);
void __cdecl DynEntSv_Damage(
                unsigned __int16 dynEntId,
                DynEntityDrawType drawType,
                const float *hitPos,
                const float *hitDir,
                int damage,
                bool explosive);
void __cdecl DynEntSv_RadiusDamage(
                float *origin,
                float radius,
                float coneAngleCos,
                float *coneDirection,
                float innerDamage,
                float outerDamage);
unsigned int __cdecl DynEntSv_GetClosestEntities(
                DynEntityDrawType drawType,
                float *radiusMins,
                float *radiusMaxs,
                float *origin,
                unsigned __int16 *hitEnts);


extern const dvar_t *dynEnt_damageRadiusScale;