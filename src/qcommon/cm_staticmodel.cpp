#include "cm_staticmodel.h"
#include <universal/com_memory.h>
#include <xanim/xmodel.h>

unsigned __int8 *__cdecl CM_Hunk_AllocXModel(unsigned int size)
{
    return Hunk_Alloc(size, "CM_Hunk_AllocXModel", 22);
}

XModel *__cdecl CM_XModelPrecache(char *name)
{
    return XModelPrecache(
                     name,
                     (void *(__cdecl *)(int))CM_Hunk_AllocXModel,
                     (void *(__cdecl *)(int))CM_Hunk_AllocXModelColl);
}

unsigned __int8 *__cdecl CM_Hunk_AllocXModelColl(unsigned int size)
{
    return Hunk_Alloc(size, "CM_Hunk_AllocXModelColl", 28);
}

void __cdecl CM_TraceStaticModel(
                cStaticModel_s *sm,
                trace_t *results,
                const float *start,
                const float *end,
                int contentmask)
{
    float localStart[3]; // [esp+14h] [ebp-30h] BYREF
    float delta[3]; // [esp+20h] [ebp-24h] BYREF
    float normal[3]; // [esp+2Ch] [ebp-18h] BYREF
    float localEnd[3]; // [esp+38h] [ebp-Ch] BYREF

    delta[0] = *start - sm->origin[0];
    delta[1] = start[1] - sm->origin[1];
    delta[2] = start[2] - sm->origin[2];
    MatrixTransformVector(delta, sm->invScaledAxis, localStart);
    delta[0] = *end - sm->origin[0];
    delta[1] = end[1] - sm->origin[1];
    delta[2] = end[2] - sm->origin[2];
    MatrixTransformVector(delta, sm->invScaledAxis, localEnd);
    if ( XModelTraceLine(sm->xmodel, results, localStart, localEnd, contentmask) >= 0 )
    {
        results->staticModel = sm;
        if ( !results && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_public.h", 175, 0, "%s", "trace") )
            __debugbreak();
        results->hitType = TRACE_HITTYPE_ENTITY;
        results->hitId = 1022;
        MatrixTransposeTransformVector(results->normal.vec.v, sm->invScaledAxis, normal);
        Vec3Normalize(normal);
        *(_QWORD *)results->normal.vec.v = *(_QWORD *)normal;
        results->normal.vec.v[2] = normal[2];
    }
}

bool __cdecl CM_TraceStaticModelComplete(cStaticModel_s *sm, const float *start, const float *end, int contentmask)
{
    float localStart[3]; // [esp+8h] [ebp-60h] BYREF
    float delta[3]; // [esp+14h] [ebp-54h] BYREF
    float localEnd[3]; // [esp+20h] [ebp-48h] BYREF
    trace_t results; // [esp+2Ch] [ebp-3Ch] BYREF

    delta[0] = *start - sm->origin[0];
    delta[1] = start[1] - sm->origin[1];
    delta[2] = start[2] - sm->origin[2];
    MatrixTransformVector(delta, sm->invScaledAxis, localStart);
    delta[0] = *end - sm->origin[0];
    delta[1] = end[1] - sm->origin[1];
    delta[2] = end[2] - sm->origin[2];
    MatrixTransformVector(delta, sm->invScaledAxis, localEnd);
    results.fraction = 1.0f;
    return XModelTraceLine(sm->xmodel, &results, localStart, localEnd, contentmask) < 0;
}

