#include "cm_mesh.h"
#include "cm_trace.h"
#include "cm_load.h"
#include <gfx_d3d/r_material.h>
#include <physics/phys_gjk.h>

void __cdecl CM_TracePointThroughTriangle(const traceWork_t *tw, const unsigned __int16 *indices, trace_t *trace)
{
    float *v3; // [esp+18h] [ebp-58h]
    float *v4; // [esp+1Ch] [ebp-54h]
    const float *v0; // [esp+20h] [ebp-50h]
    float t; // [esp+24h] [ebp-4Ch]
    float negativeU; // [esp+28h] [ebp-48h]
    float triNormalScaledByAreaX2[3]; // [esp+2Ch] [ebp-44h] BYREF
    float v0_start[3]; // [esp+38h] [ebp-38h] BYREF
    float v0_v2[3]; // [esp+44h] [ebp-2Ch] BYREF
    float tracePlaneScaledNormal[3]; // [esp+50h] [ebp-20h] BYREF
    float projTriAreaScaledByTraceLenX2; // [esp+5Ch] [ebp-14h]
    float v; // [esp+60h] [ebp-10h]
    float v0_v1[3]; // [esp+64h] [ebp-Ch] BYREF

    v0 = cm.verts[*indices];
    v4 = cm.verts[indices[1]];
    v0_v1[0] = *v0 - *v4;
    v0_v1[1] = v0[1] - v4[1];
    v0_v1[2] = v0[2] - v4[2];
    v3 = cm.verts[indices[2]];
    v0_v2[0] = *v0 - *v3;
    v0_v2[1] = v0[1] - v3[1];
    v0_v2[2] = v0[2] - v3[2];
    Vec3Cross(v0_v2, v0_v1, triNormalScaledByAreaX2);
    projTriAreaScaledByTraceLenX2 = (float)((float)(tw->delta.vec.v[0] * triNormalScaledByAreaX2[0])
                                                                                + (float)(tw->delta.vec.v[1] * triNormalScaledByAreaX2[1]))
                                                                + (float)(tw->delta.vec.v[2] * triNormalScaledByAreaX2[2]);
    if ( projTriAreaScaledByTraceLenX2 < 0.0 )
    {
        v0_start[0] = *v0 - tw->extents.start.vec.v[0];
        v0_start[1] = v0[1] - tw->extents.start.vec.v[1];
        v0_start[2] = v0[2] - tw->extents.start.vec.v[2];
        t = (float)((float)(v0_start[0] * triNormalScaledByAreaX2[0]) + (float)(v0_start[1] * triNormalScaledByAreaX2[1]))
            + (float)(v0_start[2] * triNormalScaledByAreaX2[2]);
        if ( t <= 0.0 && (float)(trace->fraction * projTriAreaScaledByTraceLenX2) < t )
        {
            Vec3Cross(tw->delta.vec.v, v0_start, tracePlaneScaledNormal);
            v = (float)((float)(tracePlaneScaledNormal[0] * v0_v1[0]) + (float)(tracePlaneScaledNormal[1] * v0_v1[1]))
                + (float)(tracePlaneScaledNormal[2] * v0_v1[2]);
            if ( v <= 0.0 && projTriAreaScaledByTraceLenX2 <= v )
            {
                negativeU = (float)((float)(tracePlaneScaledNormal[0] * v0_v2[0]) + (float)(tracePlaneScaledNormal[1] * v0_v2[1]))
                                    + (float)(tracePlaneScaledNormal[2] * v0_v2[2]);
                if ( negativeU >= 0.0 && projTriAreaScaledByTraceLenX2 <= (float)(v - negativeU) )
                {
                    trace->walkable = 0;
                    Vec3NormalizeTo(triNormalScaledByAreaX2, trace->normal.vec.v);
                    trace->fraction = t / projTriAreaScaledByTraceLenX2;
                    if ( (trace->fraction < 0.0 || trace->fraction > 1.0)
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_mesh.cpp",
                                    168,
                                    1,
                                    "%s\n\t(trace->fraction) = %g",
                                    "(trace->fraction >= 0 && trace->fraction <= 1.0f)",
                                    trace->fraction) )
                    {
                        __debugbreak();
                    }
                }
            }
        }
    }
}

void __cdecl CM_TraceThroughPartition(const traceWork_t *tw, int partitionIndex, trace_t *trace)
{
    float v0_v2[3]; // [esp+1Ch] [ebp-40h] BYREF
    float v0_v1[3]; // [esp+28h] [ebp-34h] BYREF
    bool hit; // [esp+37h] [ebp-25h]
    int checkStamp; // [esp+38h] [ebp-24h]
    int borderIndex; // [esp+3Ch] [ebp-20h]
    const float *v0; // [esp+40h] [ebp-1Ch]
    const float *v2; // [esp+48h] [ebp-14h]
    unsigned __int16 *indices; // [esp+4Ch] [ebp-10h]
    const float *v1; // [esp+50h] [ebp-Ch]
    int triIndex; // [esp+54h] [ebp-8h]
    CollisionPartition *partition; // [esp+58h] [ebp-4h]

    checkStamp = tw->threadInfo.checkcount.global;
    if ( tw->threadInfo.checkcount.partitions && tw->threadInfo.checkcount.partitions[partitionIndex] != checkStamp )
    {
        tw->threadInfo.checkcount.partitions[partitionIndex] = checkStamp;
        partition = &cm.partitions[partitionIndex];
        indices = &cm.triIndices[3 * partition->firstTri];
        if ( tw->isPoint )
        {
            if ( tw->doublesided )
            {
                hit = 0;
                for ( triIndex = partition->firstTri; triIndex < partition->firstTri + partition->triCount; ++triIndex )
                {
                    v0 = cm.verts[indices[3 * triIndex]];
                    v1 = cm.verts[indices[3 * triIndex + 1]];
                    v2 = cm.verts[indices[3 * triIndex + 2]];
                    if ( trace_point_through_triangle(
                                 tw->extents.start.vec.v,
                                 tw->delta.vec.v,
                                 v0,
                                 v1,
                                 v2,
                                 trace->fraction,
                                 &trace->fraction) )
                    {
                        hit = 1;
                        v0_v1[0] = *v0 - *v1;
                        v0_v1[1] = v0[1] - v1[1];
                        v0_v1[2] = v0[2] - v1[2];
                        v0_v2[0] = *v0 - *v2;
                        v0_v2[1] = v0[1] - v2[1];
                        v0_v2[2] = v0[2] - v2[2];
                        Vec3Cross(v0_v2, v0_v1, trace->normal.vec.v);
                    }
                }
                if ( hit )
                {
                    Vec3Normalize(trace->normal.vec.v);
                    if ( (float)((float)((float)(trace->normal.vec.v[0] * tw->delta.vec.v[0])
                                                         + (float)(trace->normal.vec.v[1] * tw->delta.vec.v[1]))
                                         + (float)(trace->normal.vec.v[2] * tw->delta.vec.v[2])) > 0.0 )
                    {
                        //trace->normal.vec.u[0] ^= _mask__NegFloat_;
                        //trace->normal.vec.u[1] ^= _mask__NegFloat_;
                        //trace->normal.vec.u[2] ^= _mask__NegFloat_;
                        trace->normal.vec.v[0] = -trace->normal.vec.v[0];
                        trace->normal.vec.v[1] = -trace->normal.vec.v[1];
                        trace->normal.vec.v[2] = -trace->normal.vec.v[2];
                    }
                }
            }
            else
            {
                for ( triIndex = partition->firstTri; triIndex < partition->firstTri + partition->triCount; ++triIndex )
                    CM_TracePointThroughTriangle(tw, (const unsigned short*)&cm.triIndices[3 * triIndex], trace);
            }
        }
        else
        {
            for ( triIndex = partition->firstTri; triIndex < partition->firstTri + partition->triCount; ++triIndex )
                CM_TraceCapsuleThroughTriangle(tw, triIndex, &cm.triIndices[3 * triIndex], trace);
            if ( (tw->delta.vec.v[0] != 0.0 || tw->delta.vec.v[1] != 0.0) && tw->offsetZ != 0.0 )
            {
                for ( borderIndex = 0; borderIndex < partition->borderCount; ++borderIndex )
                    CM_TraceCapsuleThroughBorder(tw, &partition->borders[borderIndex], trace);
            }
        }
    }
}

void __cdecl CM_TraceCapsuleThroughTriangle(
                const traceWork_t *tw,
                int triIndex,
                const unsigned __int16 *indices,
                trace_t *trace)
{
    double v4; // st7
    const char *v5; // eax
    float v6; // [esp+24h] [ebp-F4h]
    const float *v7; // [esp+28h] [ebp-F0h]
    const float *v8; // [esp+2Ch] [ebp-ECh]
    const float *v9; // [esp+30h] [ebp-E8h]
    float offsetZ; // [esp+34h] [ebp-E4h]
    float v11; // [esp+5Ch] [ebp-BCh]
    const float *v0; // [esp+70h] [ebp-A8h]
    const float *vertToCheck; // [esp+74h] [ebp-A4h]
    float start_v0[3]; // [esp+78h] [ebp-A0h] BYREF
    float triNormalScaledByAreaX2[3]; // [esp+84h] [ebp-94h] BYREF
    float hitFrac; // [esp+90h] [ebp-88h]
    bool missedEdge; // [esp+97h] [ebp-81h]
    SphereEdgeTraceResult edgeTraceResult; // [esp+98h] [ebp-80h]
    float v1_v2[3]; // [esp+9Ch] [ebp-7Ch] BYREF
    float areaX2; // [esp+A8h] [ebp-70h]
    float v0_v2[3]; // [esp+ACh] [ebp-6Ch] BYREF
    float normal[3]; // [esp+B8h] [ebp-60h] BYREF
    float startDist; // [esp+C4h] [ebp-54h]
    float sphereStart[3]; // [esp+C8h] [ebp-50h] BYREF
    const float *v2; // [esp+D4h] [ebp-44h]
    float tracePlaneScaledNormal[3]; // [esp+D8h] [ebp-40h] BYREF
    float projTriAreaScaledByTraceLenX2; // [esp+E4h] [ebp-34h]
    float negativeV; // [esp+E8h] [ebp-30h]
    const float *v1; // [esp+ECh] [ebp-2Ch]
    float u; // [esp+F0h] [ebp-28h]
    float shiftedStart[3]; // [esp+F4h] [ebp-24h]
    bool startSolid; // [esp+103h] [ebp-15h]
    float v0_v1[3]; // [esp+104h] [ebp-14h] BYREF
    float hitDist; // [esp+110h] [ebp-8h]
    bool isWalkable; // [esp+117h] [ebp-1h]

    v0 = cm.verts[*indices];
    v1 = cm.verts[indices[1]];
    v2 = cm.verts[indices[2]];
    v0_v1[0] = *v0 - *v1;
    v0_v1[1] = v0[1] - v1[1];
    v0_v1[2] = v0[2] - v1[2];
    v0_v2[0] = *v0 - *v2;
    v0_v2[1] = v0[1] - v2[1];
    v0_v2[2] = v0[2] - v2[2];
    Vec3Cross(v0_v2, v0_v1, triNormalScaledByAreaX2);
    projTriAreaScaledByTraceLenX2 = (float)((float)(tw->delta.vec.v[0] * triNormalScaledByAreaX2[0])
                                                                                + (float)(tw->delta.vec.v[1] * triNormalScaledByAreaX2[1]))
                                                                + (float)(tw->delta.vec.v[2] * triNormalScaledByAreaX2[2]);
    if ( projTriAreaScaledByTraceLenX2 < 0.0 )
    {
        areaX2 = Vec3NormalizeTo(triNormalScaledByAreaX2, normal);
        *(_QWORD *)sphereStart = *(_QWORD *)tw->extents.start.vec.v;
        sphereStart[2] = tw->extents.start.vec.v[2];
        if ( normal[2] < 0.0 )
            offsetZ = -tw->offsetZ;
        else
            offsetZ = tw->offsetZ;
        sphereStart[2] = sphereStart[2] - offsetZ;
        v11 = -tw->radius;
        shiftedStart[0] = (float)(v11 * normal[0]) + sphereStart[0];
        shiftedStart[1] = (float)(v11 * normal[1]) + sphereStart[1];
        shiftedStart[2] = (float)(v11 * normal[2]) + sphereStart[2];
        start_v0[0] = shiftedStart[0] - *v0;
        start_v0[1] = shiftedStart[1] - v0[1];
        start_v0[2] = shiftedStart[2] - v0[2];
        hitDist = (float)((float)(start_v0[0] * normal[0]) + (float)(start_v0[1] * normal[1]))
                        + (float)(start_v0[2] * normal[2]);
        if ( hitDist >= 0.0 )
        {
            //hitFrac = (float)(COERCE_FLOAT(COERCE_UNSIGNED_INT(hitDist - 0.125) ^ _mask__NegFloat_) * areaX2) / projTriAreaScaledByTraceLenX2;
            hitFrac = (float)((-(hitDist - 0.125)) * areaX2) / projTriAreaScaledByTraceLenX2;
            if ( hitFrac >= trace->fraction )
                return;
            startSolid = 0;
        }
        else
        {
            start_v0[0] = sphereStart[0] - *v0;
            start_v0[1] = sphereStart[1] - v0[1];
            start_v0[2] = sphereStart[2] - v0[2];
            startDist = (float)((float)(start_v0[0] * normal[0]) + (float)(start_v0[1] * normal[1]))
                                + (float)(start_v0[2] * normal[2]);
            if ( (float)(startDist * startDist) >= (float)(tw->radius * tw->radius) )
                return;
            //start_v0[0] = (float)(COERCE_FLOAT(LODWORD(startDist) ^ _mask__NegFloat_) * normal[0]) + start_v0[0];
            start_v0[0] = (float)(-((startDist)) * normal[0]) + start_v0[0];
            //start_v0[1] = (float)(COERCE_FLOAT(LODWORD(startDist) ^ _mask__NegFloat_) * normal[1]) + start_v0[1];
            start_v0[1] = (float)((-(startDist)) * normal[1]) + start_v0[1];
            //start_v0[2] = (float)(COERCE_FLOAT(LODWORD(startDist) ^ _mask__NegFloat_) * normal[2]) + start_v0[2];
            start_v0[2] = (float)((-(startDist)) * normal[2]) + start_v0[2];
            hitFrac = 0.0f;
            startSolid = 1;
        }
        Vec3Cross(tw->delta.vec.v, start_v0, tracePlaneScaledNormal);
        missedEdge = 0;
        isWalkable = 1;
        vertToCheck = 0;
        negativeV = (float)((float)(tracePlaneScaledNormal[0] * v0_v1[0]) + (float)(tracePlaneScaledNormal[1] * v0_v1[1]))
                            + (float)(tracePlaneScaledNormal[2] * v0_v1[2]);
        if ( negativeV < 0.0 )
        {
            missedEdge = 1;
            edgeTraceResult = CM_TraceSphereThroughEdge(tw, sphereStart, v0, v0_v1, trace);
            if ( edgeTraceResult == SPHERE_HITS_EDGE )
            {
                trace->walkable = ((1 << ((3 * triIndex + 2) & 7)) & cm.triEdgeIsWalkable[(3 * triIndex + 2) >> 3]) != 0;
                return;
            }
            if ( edgeTraceResult != SPHERE_MISSES_EDGE )
            {
                if ( edgeTraceResult == SPHERE_MAY_HIT_V0 )
                    v9 = v0;
                else
                    v9 = v1;
                vertToCheck = v9;
                isWalkable &= ((1 << ((3 * triIndex + 2) & 7)) & cm.triEdgeIsWalkable[(3 * triIndex + 2) >> 3]) != 0;
            }
        }
        u = (float)((float)(tracePlaneScaledNormal[0] * v0_v2[0]) + (float)(tracePlaneScaledNormal[1] * v0_v2[1]))
            + (float)(tracePlaneScaledNormal[2] * v0_v2[2]);
        if ( u > 0.0 )
        {
            missedEdge = 1;
            edgeTraceResult = CM_TraceSphereThroughEdge(tw, sphereStart, v0, v0_v2, trace);
            if ( edgeTraceResult == SPHERE_HITS_EDGE )
            {
                trace->walkable = ((1 << ((3 * triIndex + 1) & 7)) & cm.triEdgeIsWalkable[(3 * triIndex + 1) >> 3]) != 0;
                return;
            }
            if ( edgeTraceResult != SPHERE_MISSES_EDGE )
            {
                if ( edgeTraceResult == SPHERE_MAY_HIT_V0 )
                    v8 = v0;
                else
                    v8 = v2;
                vertToCheck = v8;
                isWalkable &= ((1 << ((3 * triIndex + 1) & 7)) & cm.triEdgeIsWalkable[(3 * triIndex + 1) >> 3]) != 0;
            }
        }
        if ( projTriAreaScaledByTraceLenX2 > (float)(u - negativeV) )
        {
            missedEdge = 1;
            v1_v2[0] = *v1 - *v2;
            v1_v2[1] = v1[1] - v2[1];
            v1_v2[2] = v1[2] - v2[2];
            edgeTraceResult = CM_TraceSphereThroughEdge(tw, sphereStart, v1, v1_v2, trace);
            if ( edgeTraceResult == SPHERE_HITS_EDGE )
            {
                trace->walkable = ((1 << ((3 * triIndex) & 7)) & cm.triEdgeIsWalkable[(3 * triIndex) >> 3]) != 0;
                return;
            }
            if ( edgeTraceResult != SPHERE_MISSES_EDGE )
            {
                if ( edgeTraceResult == SPHERE_MAY_HIT_V0 )
                    v7 = v1;
                else
                    v7 = v2;
                vertToCheck = v7;
                isWalkable &= ((1 << ((3 * triIndex) & 7)) & cm.triEdgeIsWalkable[(3 * triIndex) >> 3]) != 0;
            }
        }
        if ( missedEdge )
        {
            if ( vertToCheck )
                CM_TraceSphereThroughVertex(tw, isWalkable, sphereStart, vertToCheck, trace);
        }
        else
        {
            *(_QWORD *)trace->normal.vec.v = *(_QWORD *)normal;
            trace->normal.vec.v[2] = normal[2];
            if ( !Vec3IsNormalized(trace->normal.vec.v) )
            {
                v4 = Vec3Length(trace->normal.vec.v);
                v5 = va("(%g %g %g) len %g", trace->normal.vec.v[0], trace->normal.vec.v[1], trace->normal.vec.v[2], v4);
                if ( !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_mesh.cpp",
                                592,
                                0,
                                "%s\n\t%s",
                                "Vec3IsNormalized( trace->normal )",
                                v5) )
                    __debugbreak();
            }
            trace->walkable = 0;
            if ( (float)(0.0 - hitFrac) < 0.0 )
                v6 = hitFrac;
            else
                v6 = 0.0f;
            trace->fraction = v6;
            trace->startsolid = startSolid;
            if ( (trace->fraction < 0.0 || trace->fraction > 1.0)
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_mesh.cpp",
                            596,
                            1,
                            "%s\n\t(trace->fraction) = %g",
                            "(trace->fraction >= 0 && trace->fraction <= 1.0f)",
                            trace->fraction) )
            {
                __debugbreak();
            }
        }
    }
}

SphereEdgeTraceResult __cdecl CM_TraceSphereThroughEdge(
                const traceWork_t *tw,
                const float *sphereStart,
                const float *v0,
                const float *v0_v1,
                trace_t *trace)
{
    const char *v6; // eax
    float v7; // [esp+80h] [ebp-84h]
    float perpendicularLenSq; // [esp+98h] [ebp-6Ch]
    float t; // [esp+9Ch] [ebp-68h]
    float scaledProjectionDist; // [esp+A0h] [ebp-64h]
    float scaledProjectionDista; // [esp+A0h] [ebp-64h]
    float hitDelta; // [esp+A4h] [ebp-60h]
    float hitDelta_4; // [esp+A8h] [ebp-5Ch]
    float hitDelta_8; // [esp+ACh] [ebp-58h]
    float discriminant; // [esp+B0h] [ebp-54h]
    float edgeLenSq; // [esp+B4h] [ebp-50h]
    float edgeCrossDelta[3]; // [esp+B8h] [ebp-4Ch] BYREF
    float radius; // [esp+C4h] [ebp-40h]
    float radiusSq; // [esp+C8h] [ebp-3Ch]
    float scaledNormal[3]; // [esp+CCh] [ebp-38h] BYREF
    float tScaled; // [esp+D8h] [ebp-2Ch]
    float fracEnter; // [esp+DCh] [ebp-28h]
    float startDelta[3]; // [esp+E0h] [ebp-24h] BYREF
    float fracLeave; // [esp+ECh] [ebp-18h]
    float f; // [esp+F0h] [ebp-14h]
    float perpendicular[3]; // [esp+F4h] [ebp-10h] BYREF
    float scaledDist; // [esp+100h] [ebp-4h]

    startDelta[0] = *sphereStart - *v0;
    startDelta[1] = sphereStart[1] - v0[1];
    startDelta[2] = sphereStart[2] - v0[2];
    Vec3Cross(v0_v1, tw->delta.vec.v, perpendicular);
    scaledDist = (float)((float)(startDelta[0] * perpendicular[0]) + (float)(startDelta[1] * perpendicular[1]))
                         + (float)(startDelta[2] * perpendicular[2]);
    perpendicularLenSq = (float)((float)(perpendicular[0] * perpendicular[0])
                                                         + (float)(perpendicular[1] * perpendicular[1]))
                                         + (float)(perpendicular[2] * perpendicular[2]);
    radius = tw->radius + 0.125;
    radiusSq = radius * radius;
    discriminant = (float)((float)(radius * radius) * perpendicularLenSq) - (float)(scaledDist * scaledDist);
    if ( discriminant <= 0.0 )
        return SPHERE_MISSES_EDGE;
    edgeLenSq = (float)((float)(*v0_v1 * *v0_v1) + (float)(v0_v1[1] * v0_v1[1])) + (float)(v0_v1[2] * v0_v1[2]);
    f = sqrtf(edgeLenSq * discriminant) / perpendicularLenSq;
    Vec3Cross(startDelta, v0_v1, edgeCrossDelta);
    tScaled = (float)((float)(edgeCrossDelta[0] * perpendicular[0]) + (float)(edgeCrossDelta[1] * perpendicular[1]))
                    + (float)(edgeCrossDelta[2] * perpendicular[2]);
    t = tScaled / perpendicularLenSq;
    fracLeave = (float)(tScaled / perpendicularLenSq) + f;
    if ( fracLeave < 0.0 )
        return SPHERE_MISSES_EDGE;
    fracEnter = t - f;
    if ( (float)(t - f) >= trace->fraction )
        return SPHERE_MISSES_EDGE;
    if ( fracEnter >= 0.0 )
    {
        hitDelta = (float)(fracEnter * tw->delta.vec.v[0]) + startDelta[0];
        hitDelta_4 = (float)(fracEnter * tw->delta.vec.v[1]) + startDelta[1];
        hitDelta_8 = (float)(fracEnter * tw->delta.vec.v[2]) + startDelta[2];
        scaledProjectionDista = -((float)((float)(hitDelta * *v0_v1) + (float)(hitDelta_4 * v0_v1[1])) + (float)(hitDelta_8 * v0_v1[2]));
        if ( scaledProjectionDista > 0.0 )
        {
            if ( scaledProjectionDista < edgeLenSq )
            {
                scaledNormal[0] = (float)((float)(scaledProjectionDista / edgeLenSq) * *v0_v1) + hitDelta;
                scaledNormal[1] = (float)((float)(scaledProjectionDista / edgeLenSq) * v0_v1[1]) + hitDelta_4;
                scaledNormal[2] = (float)((float)(scaledProjectionDista / edgeLenSq) * v0_v1[2]) + hitDelta_8;
                v7 = 1.0 / radius;
                trace->normal.vec.v[0] = (float)(1.0 / radius) * scaledNormal[0];
                trace->normal.vec.v[1] = v7 * scaledNormal[1];
                trace->normal.vec.v[2] = v7 * scaledNormal[2];
                if ( !Vec3IsNormalizedEpsilon(trace->normal.vec.v, 0.003) )
                {
                    v6 = va(
                                 "(%g %g %g) from (%g %g %g) / %g;\n\t\tdelta (%g %g %g), scale %g / %g, edge (%g %g %g)",
                                 trace->normal.vec.v[0],
                                 trace->normal.vec.v[1],
                                 trace->normal.vec.v[2],
                                 scaledNormal[0],
                                 scaledNormal[1],
                                 scaledNormal[2],
                                 radius,
                                 hitDelta,
                                 hitDelta_4,
                                 hitDelta_8,
                                 scaledProjectionDista,
                                 edgeLenSq,
                                 *v0_v1,
                                 v0_v1[1],
                                 v0_v1[2]);
                    if ( !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_mesh.cpp",
                                    354,
                                    0,
                                    "%s\n\t%s",
                                    "Vec3IsNormalizedEpsilon( trace->normal, TRACE_NORMAL_EPSILON )",
                                    v6) )
                        __debugbreak();
                }
                trace->fraction = fracEnter;
                return SPHERE_HITS_EDGE;
            }
            else
            {
                return SPHERE_MAY_HIT_V1;
            }
        }
        else
        {
            return SPHERE_MAY_HIT_V0;
        }
    }
    else
    {
        scaledProjectionDist = -((float)((float)(startDelta[0] * *v0_v1) + (float)(startDelta[1] * v0_v1[1])) + (float)(startDelta[2] * v0_v1[2]));
        if ( scaledProjectionDist > 0.0 )
        {
            if ( scaledProjectionDist < edgeLenSq )
            {
                scaledNormal[0] = (float)((float)(scaledProjectionDist / edgeLenSq) * *v0_v1) + startDelta[0];
                scaledNormal[1] = (float)((float)(scaledProjectionDist / edgeLenSq) * v0_v1[1]) + startDelta[1];
                scaledNormal[2] = (float)((float)(scaledProjectionDist / edgeLenSq) * v0_v1[2]) + startDelta[2];
                if ( (float)((float)((float)(scaledNormal[0] * tw->delta.vec.v[0])
                                                     + (float)(scaledNormal[1] * tw->delta.vec.v[1]))
                                     + (float)(scaledNormal[2] * tw->delta.vec.v[2])) < 0.0 )
                {
                    Vec3NormalizeTo(scaledNormal, trace->normal.vec.v);
                    trace->fraction = 0.0f;
                    trace->startsolid = (float)(edgeLenSq
                                                                        * (float)((float)((float)(tw->radius * tw->radius) * perpendicularLenSq)
                                                                                        - (float)(scaledDist * scaledDist))) > (float)(tScaled * tScaled);
                    return SPHERE_HITS_EDGE;
                }
                else
                {
                    return SPHERE_MISSES_EDGE;
                }
            }
            else
            {
                return SPHERE_MAY_HIT_V1;
            }
        }
        else
        {
            return SPHERE_MAY_HIT_V0;
        }
    }
}

bool __cdecl Vec3IsNormalizedEpsilon(const float *v, float epsilon)
{
    return (float)(2.0 * epsilon) > (-((float)((float)((float)(*v * *v) + (float)(v[1] * v[1])) + (float)(v[2] * v[2])) - 1.0));
}

void __cdecl CM_TraceSphereThroughVertex(
                const traceWork_t *tw,
                bool isWalkable,
                const float *sphereStart,
                const float *v,
                trace_t *trace)
{
    double v5; // st7
    const char *v6; // eax
    double v7; // [esp+20h] [ebp-54h]
    double epsilon; // [esp+28h] [ebp-4Ch]
    float v9; // [esp+40h] [ebp-34h]
    float delta; // [esp+44h] [ebp-30h]
    float delta_4; // [esp+48h] [ebp-2Ch]
    float delta_8; // [esp+4Ch] [ebp-28h]
    float deltaLenSq; // [esp+50h] [ebp-24h]
    float c; // [esp+54h] [ebp-20h]
    float fraca; // [esp+58h] [ebp-1Ch]
    float frac; // [esp+58h] [ebp-1Ch]
    float discriminant; // [esp+5Ch] [ebp-18h]
    float b; // [esp+60h] [ebp-14h]
    float approxRecipLen; // [esp+68h] [ebp-Ch]
    float a; // [esp+6Ch] [ebp-8h]

    delta = *sphereStart - *v;
    delta_4 = sphereStart[1] - v[1];
    delta_8 = sphereStart[2] - v[2];
    b = (float)((float)(tw->delta.vec.v[0] * delta) + (float)(tw->delta.vec.v[1] * delta_4))
        + (float)(tw->delta.vec.v[2] * delta_8);
    if ( b < 0.0 )
    {
        deltaLenSq = (float)((float)(delta * delta) + (float)(delta_4 * delta_4)) + (float)(delta_8 * delta_8);
        if ( deltaLenSq <= 0.0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_mesh.cpp",
                        407,
                        0,
                        "%s\n\t(deltaLenSq) = %g",
                        "(deltaLenSq > 0.0f)",
                        deltaLenSq) )
        {
            __debugbreak();
        }
        c = deltaLenSq - (float)((float)(tw->radius + 0.125) * (float)(tw->radius + 0.125));
        if ( c > 0.0 )
        {
            a = tw->deltaLenSq;
            if ( a <= 0.0
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_mesh.cpp",
                            422,
                            0,
                            "%s\n\t(a) = %g",
                            "(a > 0.0f)",
                            a) )
            {
                __debugbreak();
            }
            discriminant = (float)(b * b) - (float)(a * c);
            if ( (float)((float)(b * b) * 0.001) <= discriminant )
            {
                //frac = (float)(COERCE_FLOAT(COERCE_UNSIGNED_INT(sqrtf(discriminant)) ^ _mask__NegFloat_) - b) / a;
                frac = (float)((-(sqrtf(discriminant))) - b) / a;
                if ( frac < trace->fraction )
                {
                    trace->normal.vec.v[0] = (float)(frac * tw->delta.vec.v[0]) + delta;
                    trace->normal.vec.v[1] = (float)(frac * tw->delta.vec.v[1]) + delta_4;
                    trace->normal.vec.v[2] = (float)(frac * tw->delta.vec.v[2]) + delta_8;
                    v9 = 1.0 / (float)(tw->radius + 0.125);
                    trace->normal.vec.v[0] = v9 * trace->normal.vec.v[0];
                    trace->normal.vec.v[1] = v9 * trace->normal.vec.v[1];
                    trace->normal.vec.v[2] = v9 * trace->normal.vec.v[2];
                    approxRecipLen = (float)(3.0
                                                                 - (float)((float)((float)(trace->normal.vec.v[0] * trace->normal.vec.v[0])
                                                                                                 + (float)(trace->normal.vec.v[1] * trace->normal.vec.v[1]))
                                                                                 + (float)(trace->normal.vec.v[2] * trace->normal.vec.v[2])))
                                                 * 0.5;
                    trace->normal.vec.v[0] = approxRecipLen * trace->normal.vec.v[0];
                    trace->normal.vec.v[1] = approxRecipLen * trace->normal.vec.v[1];
                    trace->normal.vec.v[2] = approxRecipLen * trace->normal.vec.v[2];
                    if ( !Vec3IsNormalizedEpsilon(trace->normal.vec.v, 0.003) )
                    {
                        epsilon = (float)((-((float)((float)((float)(trace->normal.vec.v[0] * trace->normal.vec.v[0]) + (float)(trace->normal.vec.v[1] * trace->normal.vec.v[1]))+ (float)(trace->normal.vec.v[2] * trace->normal.vec.v[2])) - 1.0)) / 2.0);
                        v7 = (float)(tw->radius + 0.125);
                        v5 = Vec3Length(trace->normal.vec.v);
                        v6 = va(
                                     "Vec3IsNormalized (%g %g %g) %g, %g %g",
                                     trace->normal.vec.v[0],
                                     trace->normal.vec.v[1],
                                     trace->normal.vec.v[2],
                                     v5,
                                     v7,
                                     epsilon);
                        if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_mesh.cpp", 448, 0, v6) )
                            __debugbreak();
                    }
                    trace->walkable = isWalkable;
                    trace->fraction = frac;
                    if ( (trace->fraction < 0.0 || trace->fraction > 1.0)
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_mesh.cpp",
                                    453,
                                    1,
                                    "%s\n\t(trace->fraction) = %g",
                                    "(trace->fraction >= 0 && trace->fraction <= 1.0f)",
                                    trace->fraction) )
                    {
                        __debugbreak();
                    }
                }
            }
        }
        else
        {
            fraca = 1.0 / sqrtf(deltaLenSq);
            trace->normal.vec.v[0] = fraca * delta;
            trace->normal.vec.v[1] = fraca * delta_4;
            trace->normal.vec.v[2] = fraca * delta_8;
            trace->walkable = isWalkable;
            trace->fraction = 0.0f;
            if ( (float)(tw->radius * tw->radius) > deltaLenSq )
                trace->startsolid = 1;
        }
    }
}

void __cdecl CM_TraceCapsuleThroughBorder(const traceWork_t *tw, const CollisionBorder *border, trace_t *trace)
{
    const char *v3; // eax
    double v4; // st7
    const char *v5; // eax
    const char *v6; // eax
    double v7; // st7
    const char *v8; // eax
    const char *v9; // eax
    double v10; // st7
    const char *v11; // eax
    float v12; // [esp+2Ch] [ebp-BCh]
    float v13; // [esp+4Ch] [ebp-9Ch]
    float v14; // [esp+7Ch] [ebp-6Ch]
    float edgeZ; // [esp+A0h] [ebp-48h]
    float edgePoint_8; // [esp+ACh] [ebp-3Ch]
    float edgePoint_8a; // [esp+ACh] [ebp-3Ch]
    float traceDeltaDot; // [esp+B0h] [ebp-38h]
    float t; // [esp+B4h] [ebp-34h]
    float c; // [esp+B8h] [ebp-30h]
    float ca; // [esp+B8h] [ebp-30h]
    float deltaDotOffset; // [esp+BCh] [ebp-2Ch]
    float deltaDotOffseta; // [esp+BCh] [ebp-2Ch]
    float discriminant; // [esp+C0h] [ebp-28h]
    float discriminanta; // [esp+C0h] [ebp-28h]
    float radius; // [esp+C4h] [ebp-24h]
    float offset; // [esp+CCh] [ebp-1Ch]
    float offseta; // [esp+CCh] [ebp-1Ch]
    float offset_4; // [esp+D0h] [ebp-18h]
    float offset_4a; // [esp+D0h] [ebp-18h]
    float endpos_8; // [esp+DCh] [ebp-Ch]
    float s; // [esp+E0h] [ebp-8h]
    float offsetLenSq; // [esp+E4h] [ebp-4h]
    float offsetLenSqa; // [esp+E4h] [ebp-4h]

    traceDeltaDot = (float)(tw->delta.vec.v[0] * border->distEq[0]) + (float)(tw->delta.vec.v[1] * border->distEq[1]);
    if ( traceDeltaDot >= 0.0 )
        return;
    radius = tw->radius + 0.125;
    t = (float)(radius
                        - (float)((float)((float)(tw->extents.start.vec.v[0] * border->distEq[0])
                                                        + (float)(tw->extents.start.vec.v[1] * border->distEq[1]))
                                        - border->distEq[2]))
        / traceDeltaDot;
    if ( t >= trace->fraction || (-(radius)) > (float)(t * tw->deltaLen) )
        return;
    endpos_8 = (float)(t * tw->delta.vec.v[2]) + tw->extents.start.vec.v[2];
    s = (float)((float)(border->distEq[1] * (float)((float)(t * tw->delta.vec.v[0]) + tw->extents.start.vec.v[0]))
                        - (float)(border->distEq[0] * (float)((float)(t * tw->delta.vec.v[1]) + tw->extents.start.vec.v[1])))
        - border->start;
    if ( s < 0.0 )
    {
        offset = tw->extents.start.vec.v[0]
                     - (float)((float)(border->distEq[1] * border->start) + (float)(border->distEq[0] * border->distEq[2]));
        offset_4 = tw->extents.start.vec.v[1]
                         - (float)((float)(border->distEq[1] * border->distEq[2]) - (float)(border->distEq[0] * border->start));
        deltaDotOffset = (float)(tw->delta.vec.v[0] * offset) + (float)(tw->delta.vec.v[1] * offset_4);
        if ( deltaDotOffset >= 0.0 )
            return;
        offsetLenSq = (float)(offset * offset) + (float)(offset_4 * offset_4);
        c = offsetLenSq - (float)(radius * radius);
        if ( c < 0.0 )
        {
            edgePoint_8 = border->zBase;
            if ( tw->offsetZ != (float)(tw->size.vec.v[2] - tw->radius) )
            {
                v3 = va("tw->offsetZ: %f, tw->size[2]: %f, tw->radius: %f", tw->offsetZ, tw->size.vec.v[2], tw->radius);
                if ( !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_mesh.cpp",
                                1231,
                                0,
                                "%s\n\t%s",
                                "tw->offsetZ == tw->size[2] - tw->radius",
                                v3) )
                    __debugbreak();
            }
            if ( tw->offsetZ < 0.0
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_mesh.cpp",
                            1232,
                            0,
                            "%s",
                            "tw->offsetZ >= 0") )
            {
                __debugbreak();
            }
            if ( fabs(edgePoint_8 - tw->extents.start.vec.v[2]) <= tw->offsetZ )
            {
                v14 = border->distEq[1];
                trace->normal.vec.u[0] = LODWORD(border->distEq[0]);
                trace->normal.vec.v[1] = v14;
                trace->normal.vec.u[2] = 0;
                if ( !Vec3IsNormalized(trace->normal.vec.v) )
                {
                    v4 = Vec3Length(trace->normal.vec.v);
                    v5 = va("(%g %g %g) len %g", trace->normal.vec.v[0], trace->normal.vec.v[1], trace->normal.vec.v[2], v4);
                    if ( !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_mesh.cpp",
                                    1237,
                                    0,
                                    "%s\n\t%s",
                                    "Vec3IsNormalized( trace->normal )",
                                    v5) )
                        __debugbreak();
                }
                trace->walkable = 0;
                trace->fraction = 0.0f;
                if ( (float)(tw->radius * tw->radius) > offsetLenSq )
                    trace->startsolid = 1;
            }
            return;
        }
        discriminant = (float)(deltaDotOffset * deltaDotOffset) - (float)(tw->deltaLenSq * c);
        if ( discriminant < 0.0 )
            return;
        if ( tw->deltaLenSq <= 0.0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_mesh.cpp",
                        1249,
                        0,
                        "%s\n\t(tw->deltaLenSq) = %g",
                        "(tw->deltaLenSq > 0.0f)",
                        tw->deltaLenSq) )
        {
            __debugbreak();
        }
        t = (float)((-(deltaDotOffset)) - sqrtf(discriminant)) / tw->deltaLenSq;
        if ( t >= trace->fraction || t <= 0.0 )
            return;
        endpos_8 = (float)(t * tw->delta.vec.v[2]) + tw->extents.start.vec.v[2];
        s = 0.0f;
LABEL_55:
        edgeZ = (float)((float)(s * border->zSlope) + border->zBase) - endpos_8;
        if ( tw->offsetZ != (float)(tw->size.vec.v[2] - tw->radius) )
        {
            v9 = va("tw->offsetZ: %f, tw->size[2]: %f, tw->radius: %f", tw->offsetZ, tw->size.vec.v[2], tw->radius);
            if ( !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_mesh.cpp",
                            1300,
                            0,
                            "%s\n\t%s",
                            "tw->offsetZ == tw->size[2] - tw->radius",
                            v9) )
                __debugbreak();
        }
        if ( tw->offsetZ < 0.0
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_mesh.cpp", 1301, 0, "%s", "tw->offsetZ >= 0") )
        {
            __debugbreak();
        }
        if ( edgeZ <= tw->offsetZ )
        {
            if ( (-(tw->offsetZ)) <= edgeZ )
            {
                trace->fraction = t;
                if ( (trace->fraction < 0.0 || trace->fraction > 1.0)
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_mesh.cpp",
                                1315,
                                1,
                                "%s\n\t(trace->fraction) = %g",
                                "(trace->fraction >= 0 && trace->fraction <= 1.0f)",
                                trace->fraction) )
                {
                    __debugbreak();
                }
                trace->walkable = 0;
                v12 = border->distEq[1];
                trace->normal.vec.u[0] = LODWORD(border->distEq[0]);
                trace->normal.vec.v[1] = v12;
                trace->normal.vec.u[2] = 0;
                if ( !Vec3IsNormalized(trace->normal.vec.v) )
                {
                    v10 = Vec3Length(trace->normal.vec.v);
                    v11 = va("(%g %g %g) len %g", trace->normal.vec.v[0], trace->normal.vec.v[1], trace->normal.vec.v[2], v10);
                    if ( !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_mesh.cpp",
                                    1318,
                                    0,
                                    "%s\n\t%s",
                                    "Vec3IsNormalized( trace->normal )",
                                    v11) )
                        __debugbreak();
                }
            }
            else if ( edgeZ > (float)((-(tw->offsetZ)) - tw->radius) )
            {
                CM_TraceSphereThroughBorder(tw, border, (-(tw->offsetZ)), trace);
            }
        }
        else if ( (float)(tw->offsetZ + tw->radius) > edgeZ )
        {
            CM_TraceSphereThroughBorder(tw, border, tw->offsetZ, trace);
        }
        return;
    }
    if ( s <= border->length )
    {
        if ( t < 0.0 )
            t = 0.0f;
        goto LABEL_55;
    }
    offseta = tw->extents.start.vec.v[0]
                    - (float)((float)((float)(border->start + border->length) * border->distEq[1])
                                    + (float)(border->distEq[0] * border->distEq[2]));
    offset_4a = tw->extents.start.vec.v[1]
                        - (float)((float)(border->distEq[1] * border->distEq[2])
                                        - (float)((float)(border->start + border->length) * border->distEq[0]));
    deltaDotOffseta = (float)(tw->delta.vec.v[0] * offseta) + (float)(tw->delta.vec.v[1] * offset_4a);
    if ( deltaDotOffseta >= 0.0 )
        return;
    offsetLenSqa = (float)(offseta * offseta) + (float)(offset_4a * offset_4a);
    ca = offsetLenSqa - (float)(radius * radius);
    if ( ca < 0.0 )
    {
        edgePoint_8a = (float)(border->zSlope * border->length) + border->zBase;
        if ( tw->offsetZ != (float)(tw->size.vec.v[2] - tw->radius) )
        {
            v6 = va("tw->offsetZ: %f, tw->size[2]: %f, tw->radius: %f", tw->offsetZ, tw->size.vec.v[2], tw->radius);
            if ( !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_mesh.cpp",
                            1270,
                            0,
                            "%s\n\t%s",
                            "tw->offsetZ == tw->size[2] - tw->radius",
                            v6) )
                __debugbreak();
        }
        if ( tw->offsetZ < 0.0
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_mesh.cpp", 1271, 0, "%s", "tw->offsetZ >= 0") )
        {
            __debugbreak();
        }
        if ( fabs(tw->extents.start.vec.v[2] - edgePoint_8a) <= tw->offsetZ )
        {
            v13 = border->distEq[1];
            trace->normal.vec.u[0] = LODWORD(border->distEq[0]);
            trace->normal.vec.v[1] = v13;
            trace->normal.vec.u[2] = 0;
            if ( !Vec3IsNormalized(trace->normal.vec.v) )
            {
                v7 = Vec3Length(trace->normal.vec.v);
                v8 = va("(%g %g %g) len %g", trace->normal.vec.v[0], trace->normal.vec.v[1], trace->normal.vec.v[2], v7);
                if ( !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_mesh.cpp",
                                1276,
                                0,
                                "%s\n\t%s",
                                "Vec3IsNormalized( trace->normal )",
                                v8) )
                    __debugbreak();
            }
            trace->walkable = 0;
            trace->fraction = 0.0f;
            if ( (float)(tw->radius * tw->radius) > offsetLenSqa )
                trace->startsolid = 1;
        }
        return;
    }
    discriminanta = (float)(deltaDotOffseta * deltaDotOffseta) - (float)(tw->deltaLenSq * ca);
    if ( discriminanta >= 0.0 )
    {
        if ( tw->deltaLenSq <= 0.0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_mesh.cpp",
                        1287,
                        0,
                        "%s\n\t(tw->deltaLenSq) = %g",
                        "(tw->deltaLenSq > 0.0f)",
                        tw->deltaLenSq) )
        {
            __debugbreak();
        }
        t = (float)((-(deltaDotOffseta)) - sqrtf(discriminanta)) / tw->deltaLenSq;
        if ( t < trace->fraction && t > 0.0 )
        {
            endpos_8 = (float)(t * tw->delta.vec.v[2]) + tw->extents.start.vec.v[2];
            s = border->length;
            goto LABEL_55;
        }
    }
}

void __cdecl CM_TraceSphereThroughBorder(
                const traceWork_t *tw,
                const CollisionBorder *border,
                float offsetZ,
                trace_t *trace)
{
    float v0[3]; // [esp+40h] [ebp-34h] BYREF
    SphereEdgeTraceResult result; // [esp+4Ch] [ebp-28h]
    float sphereStart[3]; // [esp+50h] [ebp-24h] BYREF
    float v1[3]; // [esp+5Ch] [ebp-18h] BYREF
    float v0_v1[3]; // [esp+68h] [ebp-Ch] BYREF

    v0[0] = (float)(border->distEq[1] * border->start) + (float)(border->distEq[0] * border->distEq[2]);
    v0[1] = (float)(border->distEq[1] * border->distEq[2]) - (float)(border->distEq[0] * border->start);
    v0[2] = border->zBase;
    v1[0] = (float)((float)(border->start + border->length) * border->distEq[1])
                + (float)(border->distEq[0] * border->distEq[2]);
    v1[1] = (float)(border->distEq[1] * border->distEq[2])
                - (float)((float)(border->start + border->length) * border->distEq[0]);
    v1[2] = (float)(border->zSlope * border->length) + border->zBase;
    v0_v1[0] = v0[0] - v1[0];
    v0_v1[1] = v0[1] - v1[1];
    v0_v1[2] = v0[2] - v1[2];
    *(_QWORD *)sphereStart = *(_QWORD *)tw->extents.start.vec.v;
    sphereStart[2] = tw->extents.start.vec.v[2];
    sphereStart[2] = sphereStart[2] + offsetZ;
    result = CM_TraceSphereThroughEdge(tw, sphereStart, v0, v0_v1, trace);
    if ( result == SPHERE_MAY_HIT_V0 )
    {
        CM_TraceSphereThroughVertex(tw, 0, sphereStart, v0, trace);
    }
    else if ( result == SPHERE_MAY_HIT_V1 )
    {
        CM_TraceSphereThroughVertex(tw, 0, sphereStart, v1, trace);
    }
}

void __cdecl CM_TraceThroughAabbTree(const traceWork_t *tw, const CollisionAabbTree *aabbTree, trace_t *trace)
{
    const dmaterial_t *materialInfo; // [esp+0h] [ebp-Ch]
    float oldFraction; // [esp+8h] [ebp-4h]

    materialInfo = &cm.materials[aabbTree->materialIndex];
    if ( (materialInfo->contentFlags & tw->contents) != 0 )
    {
        oldFraction = trace->fraction;
        CM_TraceThroughAabbTree_r(tw, aabbTree, trace);
        if ( oldFraction > trace->fraction )
        {
            trace->sflags = materialInfo->surfaceFlags;
            trace->cflags = materialInfo->contentFlags;
        }
    }
}

void __cdecl CM_TraceThroughAabbTree_r(const traceWork_t *tw, const CollisionAabbTree *aabbTree, trace_t *trace)
{
    bool v3; // [esp+3h] [ebp-3A5h]
    float v4; // [esp+4h] [ebp-3A4h] BYREF
    float v5; // [esp+8h] [ebp-3A0h]
    float v6; // [esp+Ch] [ebp-39Ch]
    int v7; // [esp+10h] [ebp-398h]
    float v8; // [esp+14h] [ebp-394h] BYREF
    float v9; // [esp+18h] [ebp-390h]
    float v10; // [esp+1Ch] [ebp-38Ch]
    int v11; // [esp+20h] [ebp-388h]
    __int64 v12; // [esp+24h] [ebp-384h]
    int v13; // [esp+2Ch] [ebp-37Ch]
    float v14; // [esp+30h] [ebp-378h]
    float v15; // [esp+34h] [ebp-374h]
    float v16; // [esp+38h] [ebp-370h]
    float v17; // [esp+3Ch] [ebp-36Ch]
    float v18; // [esp+40h] [ebp-368h]
    int v19; // [esp+44h] [ebp-364h]
    float v20; // [esp+48h] [ebp-360h]
    float v21; // [esp+4Ch] [ebp-35Ch]
    float v22; // [esp+50h] [ebp-358h]
    __int64 v23; // [esp+54h] [ebp-354h]
    int v24; // [esp+5Ch] [ebp-34Ch]
    float v25; // [esp+60h] [ebp-348h]
    float v26; // [esp+64h] [ebp-344h]
    float v27; // [esp+68h] [ebp-340h]
    float v28; // [esp+6Ch] [ebp-33Ch]
    int v29; // [esp+70h] [ebp-338h]
    float v30; // [esp+74h] [ebp-334h]
    float v31; // [esp+78h] [ebp-330h]
    int v32; // [esp+7Ch] [ebp-32Ch]
    float v33; // [esp+80h] [ebp-328h]
    float v34; // [esp+84h] [ebp-324h]
    int v35; // [esp+88h] [ebp-320h]
    float v36; // [esp+8Ch] [ebp-31Ch]
    hybrid_vector *p_size; // [esp+90h] [ebp-318h]
    hybrid_vector *v38; // [esp+94h] [ebp-314h]
    float v39; // [esp+98h] [ebp-310h]
    float v40; // [esp+9Ch] [ebp-30Ch]
    float v41; // [esp+A0h] [ebp-308h]
    float v42; // [esp+A4h] [ebp-304h]
    float v43; // [esp+A8h] [ebp-300h]
    float v44; // [esp+ACh] [ebp-2FCh]
    float *v45; // [esp+B4h] [ebp-2F4h]
    float *v46; // [esp+B8h] [ebp-2F0h]
    float *v47; // [esp+BCh] [ebp-2ECh]
    bool v48; // [esp+C3h] [ebp-2E5h]
    float v49; // [esp+C4h] [ebp-2E4h] BYREF
    float v50; // [esp+C8h] [ebp-2E0h]
    float v51; // [esp+CCh] [ebp-2DCh]
    int v52; // [esp+D0h] [ebp-2D8h]
    float a[4]; // [esp+D4h] [ebp-2D4h] BYREF
    __int64 v54; // [esp+E4h] [ebp-2C4h]
    int v55; // [esp+ECh] [ebp-2BCh]
    float v56; // [esp+F0h] [ebp-2B8h]
    float v57; // [esp+F4h] [ebp-2B4h]
    float v58; // [esp+F8h] [ebp-2B0h]
    float v59; // [esp+FCh] [ebp-2ACh]
    float v60; // [esp+100h] [ebp-2A8h]
    int v61; // [esp+104h] [ebp-2A4h]
    float v62; // [esp+108h] [ebp-2A0h]
    float v63; // [esp+10Ch] [ebp-29Ch]
    float v64; // [esp+110h] [ebp-298h]
    __int64 v65; // [esp+114h] [ebp-294h]
    int v66; // [esp+11Ch] [ebp-28Ch]
    float v67; // [esp+120h] [ebp-288h]
    float v68; // [esp+124h] [ebp-284h]
    float v69; // [esp+128h] [ebp-280h]
    float v70; // [esp+12Ch] [ebp-27Ch]
    int v71; // [esp+130h] [ebp-278h]
    float v72; // [esp+134h] [ebp-274h]
    float v73; // [esp+138h] [ebp-270h]
    int v74; // [esp+13Ch] [ebp-26Ch]
    float v75; // [esp+140h] [ebp-268h]
    float v76; // [esp+144h] [ebp-264h]
    int v77; // [esp+148h] [ebp-260h]
    float v78; // [esp+14Ch] [ebp-25Ch]
    hybrid_vector *p_midpoint; // [esp+150h] [ebp-258h]
    float v80; // [esp+154h] [ebp-254h]
    float v81; // [esp+158h] [ebp-250h]
    float v82; // [esp+15Ch] [ebp-24Ch]
    float sum[3]; // [esp+160h] [ebp-248h] BYREF
    //float v84; // [esp+164h] [ebp-244h]
    //float v85; // [esp+168h] [ebp-240h]
    float *v86; // [esp+170h] [ebp-238h]
    float *halfSize; // [esp+174h] [ebp-234h]
    float *v88; // [esp+178h] [ebp-230h]
    const CollisionAabbTree *localTree; // [esp+18Ch] [ebp-21Ch]
    const CollisionAabbTree *now; // [esp+194h] [ebp-214h]
    const CollisionAabbTree *child; // [esp+198h] [ebp-210h]
    const CollisionAabbTree *stack[128]; // [esp+1A0h] [ebp-208h]
    unsigned int v93; // [esp+3A4h] [ebp-4h]
    const CollisionAabbTree *aabbTreea; // [esp+3B4h] [ebp+Ch]

    v93 = 0;
    halfSize = (float*)aabbTree->halfSize;
    v88 = a;
    a[0] = aabbTree->halfSize[0];
    a[1] = aabbTree->halfSize[1];
    a[2] = aabbTree->halfSize[2];
    a[3] = 0.0f;
    v86 = &v49;
    v49 = aabbTree->origin[0];
    v50 = aabbTree->origin[1];
    v51 = aabbTree->origin[2];
    v52 = 0;
    p_midpoint = (hybrid_vector*) & tw->midpoint;
    v80 = tw->midpoint.vec.v[0] - v49;
    v81 = tw->midpoint.vec.v[1] - v50;
    v82 = tw->midpoint.vec.v[2] - v51;
    Vec3Add(a, tw->size.vec.v, sum);
    v78 = v80;
    v77 = fabs(v80);
    v76 = tw->halfDeltaAbs.vec.v[0];
    if ( fabs(v80) <= (float)(sum[0] + v76))
    {
        v75 = v81;
        v74 = fabs(v81);
        v73 = tw->halfDeltaAbs.vec.v[1];
        if ( fabs(v81) <= (float)(sum[1] + v73) )
        {
            v72 = v82;
            v71 = fabs(v82);
            v70 = tw->halfDeltaAbs.vec.v[2];
            if ( fabs(v82) <= (float)(sum[2] + v70) )
            {
                if ( tw->axialCullOnly )
                {
                    v48 = 0;
                }
                else
                {
                    v69 = tw->halfDelta.vec.v[1];
                    v68 = tw->halfDelta.vec.v[2];
                    v67 = (float)(v69 * v82) - (float)(v68 * v81);
                    v66 = fabs(v67);
                    v65 = *(_QWORD *)&tw->halfDeltaAbs.vec.unitVec[1].packed;
                    if ( fabs(v67) <= (float)((float)(sum[1] * *((float *)&v65 + 1))
                                                                                                                                            + (float)(sum[2] * *(float *)&v65)) )
                    {
                        v64 = tw->halfDelta.vec.v[2];
                        v63 = tw->halfDelta.vec.v[0];
                        v62 = (float)(v64 * v80) - (float)(v63 * v82);
                        v61 = fabs(v62);
                        v60 = tw->halfDeltaAbs.vec.v[0];
                        v59 = tw->halfDeltaAbs.vec.v[2];
                        if ( fabs(v62) <= (float)((float)(sum[2] * v60) + (float)(sum[0] * v59)) )
                        {
                            v58 = tw->halfDelta.vec.v[0];
                            v57 = tw->halfDelta.vec.v[1];
                            v56 = (float)(v58 * v81) - (float)(v57 * v80);
                            v55 = fabs(v56);
                            v54 = *(_QWORD *)tw->halfDeltaAbs.vec.v;
                            v48 = fabs(v56) > (float)((float)(sum[0] * *((float *)&v54 + 1))
                                                                                                                                                    + (float)(sum[1] * *(float *)&v54));
                        }
                        else
                        {
                            v48 = 1;
                        }
                    }
                    else
                    {
                        v48 = 1;
                    }
                }
            }
            else
            {
                v48 = 1;
            }
        }
        else
        {
            v48 = 1;
        }
    }
    else
    {
        v48 = 1;
    }
    if ( !v48 )
    {
        stack[v93++] = aabbTree;
        do
        {
            aabbTreea = stack[--v93];
            if ( aabbTreea->childCount )
            {
                child = &cm.aabbTrees[aabbTreea->u.firstChildIndex];
                now = &child[aabbTreea->childCount];
                do
                {
                    localTree = --now;
                    v46 = (float*)now->halfSize;
                    v47 = &v8;
                    v8 = now->halfSize[0];
                    v9 = now->halfSize[1];
                    v10 = now->halfSize[2];
                    v11 = 0;
                    v45 = &v4;
                    v4 = now->origin[0];
                    v5 = now->origin[1];
                    v6 = now->origin[2];
                    v7 = 0;
                    v38 = (hybrid_vector*)&tw->midpoint;
                    v39 = tw->midpoint.vec.v[0] - v4;
                    v40 = tw->midpoint.vec.v[1] - v5;
                    v41 = tw->midpoint.vec.v[2] - v6;
                    p_size = (hybrid_vector *)&tw->size;
                    v42 = v8 + tw->size.vec.v[0];
                    v43 = v9 + tw->size.vec.v[1];
                    v44 = v10 + tw->size.vec.v[2];
                    v36 = v39;
                    v35 = fabs(v39);
                    v34 = tw->halfDeltaAbs.vec.v[0];
                    if ( fabs(v39) <= (float)(v42 + v34) )
                    {
                        v33 = v40;
                        v32 = fabs(v40);
                        v31 = tw->halfDeltaAbs.vec.v[1];
                        if ( fabs(v40) <= (float)(v43 + v31) )
                        {
                            v30 = v41;
                            v29 = fabs(v41);
                            v28 = tw->halfDeltaAbs.vec.v[2];
                            if ( fabs(v41) <= (float)(v44 + v28) )
                            {
                                if ( tw->axialCullOnly )
                                {
                                    v3 = 0;
                                }
                                else
                                {
                                    v27 = tw->halfDelta.vec.v[1];
                                    v26 = tw->halfDelta.vec.v[2];
                                    v25 = (float)(v27 * v41) - (float)(v26 * v40);
                                    v24 = fabs(v25);
                                    v23 = *(_QWORD *)&tw->halfDeltaAbs.vec.unitVec[1].packed;
                                    if ( fabs(v25) <= (float)((float)(v43 * *((float *)&v23 + 1))
                                                                                                                                                            + (float)(v44 * *(float *)&v23)) )
                                    {
                                        v22 = tw->halfDelta.vec.v[2];
                                        v21 = tw->halfDelta.vec.v[0];
                                        v20 = (float)(v22 * v39) - (float)(v21 * v41);
                                        v19 = fabs(v20);
                                        v18 = tw->halfDeltaAbs.vec.v[0];
                                        v17 = tw->halfDeltaAbs.vec.v[2];
                                        if ( fabs(v20) <= (float)((float)(v44 * v18) + (float)(v42 * v17)) )
                                        {
                                            v16 = tw->halfDelta.vec.v[0];
                                            v15 = tw->halfDelta.vec.v[1];
                                            v14 = (float)(v16 * v40) - (float)(v15 * v39);
                                            v13 = fabs(v14);
                                            v12 = *(_QWORD *)tw->halfDeltaAbs.vec.v;
                                            v3 = fabs(v14) > (float)((float)(v42 * *((float *)&v12 + 1))
                                                                                                                                                                 + (float)(v43 * *(float *)&v12));
                                        }
                                        else
                                        {
                                            v3 = 1;
                                        }
                                    }
                                    else
                                    {
                                        v3 = 1;
                                    }
                                }
                            }
                            else
                            {
                                v3 = 1;
                            }
                        }
                        else
                        {
                            v3 = 1;
                        }
                    }
                    else
                    {
                        v3 = 1;
                    }
                    if ( !v3 )
                    {
                        if ( v93 >= 0x80
                            && !Assert_MyHandler(
                                        "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_mesh.cpp",
                                        1492,
                                        0,
                                        "%s\n\t(sp) = %i",
                                        "(sp < (sizeof( stack ) / (sizeof( stack[0] ) * (sizeof( stack ) != 4 || sizeof( stack[0] ) <= 4))))",
                                        v93) )
                        {
                            __debugbreak();
                        }
                        stack[v93++] = localTree;
                    }
                }
                while ( now != child );
            }
            else
            {
                CM_TraceThroughAabbTree_work(tw, aabbTreea, trace);
            }
        }
        while ( v93 );
    }
}

void __cdecl CM_TraceThroughAabbTree_work(const traceWork_t *tw, const CollisionAabbTree *aabbTree, trace_t *trace)
{
    CollisionBorder *borders; // [esp+CCh] [ebp-58h]
    unsigned __int16 *v4; // [esp+D4h] [ebp-50h]
    float v0_v2[3]; // [esp+D8h] [ebp-4Ch] BYREF
    float v0_v1[3]; // [esp+E4h] [ebp-40h] BYREF
    bool hit; // [esp+F3h] [ebp-31h]
    const dmaterial_t *materialInfo; // [esp+F4h] [ebp-30h]
    unsigned __int16 *indices; // [esp+FCh] [ebp-28h]
    int checkStamp; // [esp+100h] [ebp-24h]
    int borderIndex; // [esp+104h] [ebp-20h]
    const float *v0; // [esp+108h] [ebp-1Ch]
    const float *v2; // [esp+10Ch] [ebp-18h]
    int partitionIndex; // [esp+110h] [ebp-14h]
    const float *v1; // [esp+114h] [ebp-10h]
    int triIndex; // [esp+118h] [ebp-Ch]
    float orig_frac; // [esp+11Ch] [ebp-8h]
    const CollisionPartition *partition; // [esp+120h] [ebp-4h]

    orig_frac = trace->fraction;
    partitionIndex = aabbTree->u.firstChildIndex;
    checkStamp = tw->threadInfo.checkcount.global;
    if ( tw->threadInfo.checkcount.partitions && tw->threadInfo.checkcount.partitions[partitionIndex] != checkStamp )
    {
        tw->threadInfo.checkcount.partitions[partitionIndex] = checkStamp;
        partition = &cm.partitions[partitionIndex];
        if ( tw->isPoint )
        {
            indices = &cm.triIndices[3 * partition->firstTri];
            materialInfo = &cm.materials[aabbTree->materialIndex];
            if ( !tw->doublesided || (materialInfo->contentFlags & 0x20) != 0 )
            {
                for ( triIndex = 0; triIndex < partition->triCount; ++triIndex )
                    CM_TracePointThroughTriangle(tw, &indices[3 * triIndex], trace);
            }
            else
            {
                hit = 0;
                for ( triIndex = 0; triIndex < partition->triCount; ++triIndex )
                {
                    v0 = cm.verts[indices[3 * triIndex]];
                    v1 = cm.verts[indices[3 * triIndex + 1]];
                    v2 = cm.verts[indices[3 * triIndex + 2]];
                    if ( trace_point_through_triangle(
                                 tw->extents.start.vec.v,
                                 tw->delta.vec.v,
                                 v0,
                                 v1,
                                 v2,
                                 trace->fraction,
                                 &trace->fraction) )
                    {
                        hit = 1;
                        v0_v1[0] = *v0 - *v1;
                        v0_v1[1] = v0[1] - v1[1];
                        v0_v1[2] = v0[2] - v1[2];
                        v0_v2[0] = *v0 - *v2;
                        v0_v2[1] = v0[1] - v2[1];
                        v0_v2[2] = v0[2] - v2[2];
                        Vec3Cross(v0_v2, v0_v1, trace->normal.vec.v);
                    }
                }
                if ( hit )
                {
                    Vec3Normalize(trace->normal.vec.v);
                    if ( (float)((float)((float)(trace->normal.vec.v[0] * tw->delta.vec.v[0])
                                                         + (float)(trace->normal.vec.v[1] * tw->delta.vec.v[1]))
                                         + (float)(trace->normal.vec.v[2] * tw->delta.vec.v[2])) > 0.0 )
                    {
                        trace->normal.vec.v[0] = -trace->normal.vec.v[0];
                        trace->normal.vec.v[1] = -trace->normal.vec.v[1];
                        trace->normal.vec.v[2] = -trace->normal.vec.v[2];
                    }
                }
            }
        }
        else
        {
            v4 = &cm.triIndices[3 * partition->firstTri];
            for ( triIndex = 0; triIndex < partition->triCount; ++triIndex )
                CM_TraceCapsuleThroughTriangle(tw, triIndex, &v4[3 * triIndex], trace);
            if ( (tw->delta.vec.v[0] != 0.0 || tw->delta.vec.v[1] != 0.0) && tw->offsetZ != 0.0 && partition->borderCount )
            {
                borders = partition->borders;
                for ( borderIndex = 0; borderIndex < partition->borderCount; ++borderIndex )
                    CM_TraceCapsuleThroughBorder(tw, &borders[borderIndex], trace);
            }
        }
        if ( orig_frac > trace->fraction )
            trace->hitPartition = partitionIndex;
    }
}

void __cdecl CM_SightTraceThroughAabbTree(const traceWork_t *tw, const CollisionAabbTree *aabbTree, trace_t *trace)
{
    CM_TraceThroughAabbTree(tw, aabbTree, trace);
}

void __cdecl CM_MeshTestInLeaf(const traceWork_t *tw, cLeaf_s *leaf, trace_t *trace)
{
    dmaterial_t *materialInfo; // [esp+4h] [ebp-Ch]
    int k; // [esp+8h] [ebp-8h]
    CollisionAabbTree *aabbTree; // [esp+Ch] [ebp-4h]

    if ( tw->isPoint
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_mesh.cpp", 1660, 0, "%s", "!tw->isPoint") )
    {
        __debugbreak();
    }
    if ( trace->allsolid
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_mesh.cpp", 1661, 0, "%s", "!trace->allsolid") )
    {
        __debugbreak();
    }
    for ( k = 0; k < leaf->collAabbCount; ++k )
    {
        aabbTree = &cm.aabbTrees[k + leaf->firstCollAabbIndex];
        materialInfo = &cm.materials[aabbTree->materialIndex];
        if ( (materialInfo->contentFlags & tw->contents) != 0 )
        {
            CM_PositionTestInAabbTree_r(tw, aabbTree, trace);
            if ( trace->allsolid )
            {
                trace->sflags = materialInfo->surfaceFlags;
                trace->cflags = materialInfo->contentFlags;
                return;
            }
        }
    }
}

void __cdecl CM_PositionTestInAabbTree_r(const traceWork_t *tw, CollisionAabbTree *aabbTree, trace_t *trace)
{
    bool v3; // [esp+3h] [ebp-E5h]
    float v4; // [esp+4h] [ebp-E4h] BYREF
    float v5; // [esp+8h] [ebp-E0h]
    float v6; // [esp+Ch] [ebp-DCh]
    int v7; // [esp+10h] [ebp-D8h]
    float v8; // [esp+14h] [ebp-D4h] BYREF
    float v9; // [esp+18h] [ebp-D0h]
    float v10; // [esp+1Ch] [ebp-CCh]
    int v11; // [esp+20h] [ebp-C8h]
    __int64 v12; // [esp+24h] [ebp-C4h]
    int v13; // [esp+2Ch] [ebp-BCh]
    float v14; // [esp+30h] [ebp-B8h]
    float v15; // [esp+34h] [ebp-B4h]
    float v16; // [esp+38h] [ebp-B0h]
    float v17; // [esp+3Ch] [ebp-ACh]
    float v18; // [esp+40h] [ebp-A8h]
    int v19; // [esp+44h] [ebp-A4h]
    float v20; // [esp+48h] [ebp-A0h]
    float v21; // [esp+4Ch] [ebp-9Ch]
    float v22; // [esp+50h] [ebp-98h]
    __int64 v23; // [esp+54h] [ebp-94h]
    int v24; // [esp+5Ch] [ebp-8Ch]
    float v25; // [esp+60h] [ebp-88h]
    float v26; // [esp+64h] [ebp-84h]
    float v27; // [esp+68h] [ebp-80h]
    float v28; // [esp+6Ch] [ebp-7Ch]
    int v29; // [esp+70h] [ebp-78h]
    float v30; // [esp+74h] [ebp-74h]
    float v31; // [esp+78h] [ebp-70h]
    int v32; // [esp+7Ch] [ebp-6Ch]
    float v33; // [esp+80h] [ebp-68h]
    float v34; // [esp+84h] [ebp-64h]
    int v35; // [esp+88h] [ebp-60h]
    float v36; // [esp+8Ch] [ebp-5Ch]
    hybrid_vector *p_size; // [esp+90h] [ebp-58h]
    hybrid_vector *p_midpoint; // [esp+94h] [ebp-54h]
    float v39; // [esp+98h] [ebp-50h]
    float v40; // [esp+9Ch] [ebp-4Ch]
    float v41; // [esp+A0h] [ebp-48h]
    float v42; // [esp+A4h] [ebp-44h]
    float v43; // [esp+A8h] [ebp-40h]
    float v44; // [esp+ACh] [ebp-3Ch]
    float *v45; // [esp+B4h] [ebp-34h]
    float *halfSize; // [esp+B8h] [ebp-30h]
    float *v47; // [esp+BCh] [ebp-2Ch]
    int checkStamp; // [esp+C8h] [ebp-20h]
    int childIndex; // [esp+D0h] [ebp-18h]
    int triCount; // [esp+D4h] [ebp-14h]
    unsigned __int16 *indices; // [esp+D8h] [ebp-10h]
    int partitionIndex; // [esp+DCh] [ebp-Ch]
    CollisionAabbTree *child; // [esp+E0h] [ebp-8h]
    CollisionPartition *partition; // [esp+E4h] [ebp-4h]

    halfSize = aabbTree->halfSize;
    v47 = &v8;
    v8 = aabbTree->halfSize[0];
    v9 = aabbTree->halfSize[1];
    v10 = aabbTree->halfSize[2];
    v11 = 0;
    v45 = &v4;
    v4 = aabbTree->origin[0];
    v5 = aabbTree->origin[1];
    v6 = aabbTree->origin[2];
    v7 = 0;
    p_midpoint = (hybrid_vector *)&tw->midpoint;
    v39 = tw->midpoint.vec.v[0] - v4;
    v40 = tw->midpoint.vec.v[1] - v5;
    v41 = tw->midpoint.vec.v[2] - v6;
    p_size = (hybrid_vector *)&tw->size;
    v42 = v8 + tw->size.vec.v[0];
    v43 = v9 + tw->size.vec.v[1];
    v44 = v10 + tw->size.vec.v[2];
    v36 = v39;
    v35 = fabs(v39);
    v34 = tw->halfDeltaAbs.vec.v[0];
    if ( fabs(v39) <= (float)(v42 + v34) )
    {
        v33 = v40;
        v32 = fabs(v40);
        v31 = tw->halfDeltaAbs.vec.v[1];
        if ( fabs(v40) <= (float)(v43 + v31) )
        {
            v30 = v41;
            v29 = fabs(v41);
            v28 = tw->halfDeltaAbs.vec.v[2];
            if ( fabs(v41) <= (float)(v44 + v28) )
            {
                if ( tw->axialCullOnly )
                {
                    v3 = 0;
                }
                else
                {
                    v27 = tw->halfDelta.vec.v[1];
                    v26 = tw->halfDelta.vec.v[2];
                    v25 = (float)(v27 * v41) - (float)(v26 * v40);
                    v24 = fabs(v25);
                    v23 = *(_QWORD *)&tw->halfDeltaAbs.vec.unitVec[1].packed;
                    if ( fabs(v25) <= (float)((float)(v43 * *((float *)&v23 + 1))
                                                                                                                                            + (float)(v44 * *(float *)&v23)) )
                    {
                        v22 = tw->halfDelta.vec.v[2];
                        v21 = tw->halfDelta.vec.v[0];
                        v20 = (float)(v22 * v39) - (float)(v21 * v41);
                        v19 = fabs(v20);
                        v18 = tw->halfDeltaAbs.vec.v[0];
                        v17 = tw->halfDeltaAbs.vec.v[2];
                        if ( fabs(v20) <= (float)((float)(v44 * v18) + (float)(v42 * v17)) )
                        {
                            v16 = tw->halfDelta.vec.v[0];
                            v15 = tw->halfDelta.vec.v[1];
                            v14 = (float)(v16 * v40) - (float)(v15 * v39);
                            v13 = fabs(v14);
                            v12 = *(_QWORD *)tw->halfDeltaAbs.vec.v;
                            v3 = fabs(v14) > (float)((float)(v42 * *((float *)&v12 + 1))
                                                                                                                                                 + (float)(v43 * *(float *)&v12));
                        }
                        else
                        {
                            v3 = 1;
                        }
                    }
                    else
                    {
                        v3 = 1;
                    }
                }
            }
            else
            {
                v3 = 1;
            }
        }
        else
        {
            v3 = 1;
        }
    }
    else
    {
        v3 = 1;
    }
    if ( !v3 )
    {
        if ( aabbTree->childCount )
        {
            child = &cm.aabbTrees[aabbTree->u.firstChildIndex];
            childIndex = 0;
            while ( childIndex < aabbTree->childCount )
            {
                CM_PositionTestInAabbTree_r(tw, child, trace);
                if ( trace->startsolid )
                    break;
                ++childIndex;
                ++child;
            }
        }
        else
        {
            partitionIndex = aabbTree->u.firstChildIndex;
            checkStamp = tw->threadInfo.checkcount.global;
            if ( tw->threadInfo.checkcount.partitions && tw->threadInfo.checkcount.partitions[partitionIndex] != checkStamp )
            {
                tw->threadInfo.checkcount.partitions[partitionIndex] = checkStamp;
                partition = &cm.partitions[partitionIndex];
                indices = &cm.triIndices[3 * partition->firstTri];
                for ( triCount = partition->triCount; triCount; --triCount )
                {
                    CM_PositionTestCapsuleInTriangle(tw, indices, trace);
                    if ( trace->startsolid )
                        break;
                    indices += 3;
                }
            }
        }
    }
}

void __cdecl CM_PositionTestCapsuleInTriangle(const traceWork_t *tw, const unsigned __int16 *indices, trace_t *trace)
{
    float v3; // [esp+8h] [ebp-3Ch]
    float v4; // [esp+10h] [ebp-34h]
    float v5; // [esp+18h] [ebp-2Ch]
    float v6; // [esp+20h] [ebp-24h]
    float start[3]; // [esp+28h] [ebp-1Ch] BYREF
    float end[3]; // [esp+34h] [ebp-10h] BYREF
    float distSq; // [esp+40h] [ebp-4h]

    if ( tw->offsetZ == 0.0 )
    {
        distSq = CM_DistanceSquaredFromPointToTriangle(tw->extents.start.vec.v, indices);
        if ( (float)(tw->radius * tw->radius) > distSq )
        {
            trace->fraction = 0.0f;
            trace->startsolid = 1;
            trace->allsolid = 1;
        }
    }
    else
    {
        v6 = tw->extents.start.vec.v[1];
        v5 = tw->extents.start.vec.v[2] + tw->offsetZ;
        start[0] = tw->extents.start.vec.v[0];
        start[1] = v6;
        start[2] = v5;
        v4 = tw->extents.start.vec.v[1];
        v3 = tw->extents.start.vec.v[2] - tw->offsetZ;
        end[0] = tw->extents.start.vec.v[0];
        end[1] = v4;
        end[2] = v3;
        if ( CM_DoesCapsuleIntersectTriangle(start, end, tw->radius * tw->radius, indices) )
        {
            trace->fraction = 0.0f;
            trace->startsolid = 1;
            trace->allsolid = 1;
        }
    }
}

double __cdecl CM_DistanceSquaredFromPointToTriangle(const float *pt, const unsigned __int16 *indices)
{
    const float *v0; // [esp+1Ch] [ebp-3Ch]
    float e1[3]; // [esp+24h] [ebp-34h] BYREF
    float a11; // [esp+30h] [ebp-28h]
    float a00; // [esp+34h] [ebp-24h]
    const float *v2; // [esp+38h] [ebp-20h]
    const float *v1; // [esp+3Ch] [ebp-1Ch]
    float e0[3]; // [esp+40h] [ebp-18h] BYREF
    float ptOnTri[3]; // [esp+4Ch] [ebp-Ch] BYREF

    v0 = cm.verts[*indices];
    v1 = cm.verts[indices[1]];
    v2 = cm.verts[indices[2]];
    e0[0] = *v1 - *v0;
    e0[1] = v1[1] - v0[1];
    e0[2] = v1[2] - v0[2];
    e1[0] = *v2 - *v0;
    e1[1] = v2[1] - v0[1];
    e1[2] = v2[2] - v0[2];
    a00 = (float)((float)(e0[0] * e0[0]) + (float)(e0[1] * e0[1])) + (float)(e0[2] * e0[2]);
    a11 = (float)((float)(e1[0] * e1[0]) + (float)(e1[1] * e1[1])) + (float)(e1[2] * e1[2]);
    CM_ClosestPointOnTri(
        pt,
        v0,
        e0,
        e1,
        a00,
        (float)((float)(e0[0] * e1[0]) + (float)(e0[1] * e1[1])) + (float)(e0[2] * e1[2]),
        a11,
        ptOnTri);
    return Vec3DistanceSq(pt, ptOnTri);
}

void __cdecl CM_ClosestPointOnTri(
                const float *pt,
                const float *v0,
                const float *e0,
                const float *e1,
                float a00,
                float a01,
                float a11,
                float *ptOnTri)
{
    float b1; // [esp+0h] [ebp-30h]
    float det; // [esp+Ch] [ebp-24h]
    float u; // [esp+10h] [ebp-20h]
    float ua; // [esp+10h] [ebp-20h]
    float v; // [esp+14h] [ebp-1Ch]
    float va; // [esp+14h] [ebp-1Ch]
    float denom; // [esp+18h] [ebp-18h]
    float v0_pt; // [esp+1Ch] [ebp-14h]
    float v0_pt_4; // [esp+20h] [ebp-10h]
    float v0_pt_8; // [esp+24h] [ebp-Ch]
    float b0; // [esp+28h] [ebp-8h]
    float numer; // [esp+2Ch] [ebp-4h]

    v0_pt = *pt - *v0;
    v0_pt_4 = pt[1] - v0[1];
    v0_pt_8 = pt[2] - v0[2];
    b0 = (float)((float)(*e0 * v0_pt) + (float)(e0[1] * v0_pt_4)) + (float)(e0[2] * v0_pt_8);
    b1 = (float)((float)(*e1 * v0_pt) + (float)(e1[1] * v0_pt_4)) + (float)(e1[2] * v0_pt_8);
    det = (float)(a00 * a11) - (float)(a01 * a01);
    u = (float)(a01 * b1) - (float)(a11 * b0);
    v = (float)(a01 * b0) - (float)(a00 * b1);
    if ( det >= (float)(u + v) )
    {
        if ( u < 0.0 )
        {
            if ( v < 0.0 )
            {
                ua = 0.0f;
                va = 0.0f;
                if ( b0 >= 0.0 )
                {
                    if ( b1 >= 0.0 )
                        goto LABEL_30;
                    goto region_3;
                }
                goto region_5;
            }
region_3:
            ua = 0.0f;
            if ( b1 < 0.0 )
            {
                if ( (-(b1)) < a11 )
                    va = (-(b1)) / a11;
                else
                    va = 1.0f;
            }
            else
            {
                va = 0.0f;
            }
            goto LABEL_30;
        }
        if ( v >= 0.0 )
        {
            ua = u * (float)(1.0 / det);
            va = v * (float)(1.0 / det);
            goto LABEL_30;
        }
region_5:
        va = 0.0f;
        if ( b0 < 0.0 )
        {
            if ( (-(b0)) < a00 )
                ua = (-(b0)) / a00;
            else
                ua = 1.0f;
        }
        else
        {
            ua = 0.0f;
        }
        goto LABEL_30;
    }
    if ( u >= 0.0 )
    {
        if ( v < 0.0 )
        {
            ua = 1.0f;
            va = 0.0f;
            if ( (float)(a00 + b0) > 0.0 )
                goto region_5;
            if ( (float)(a00 + b0) <= (float)(a01 + b1) )
                goto LABEL_30;
        }
    }
    else
    {
        ua = 0.0f;
        va = 1.0f;
        if ( (float)(a11 + b1) > 0.0 )
            goto region_3;
        if ( (float)(a11 + b1) <= (float)(a01 + b0) )
            goto LABEL_30;
    }
    numer = (float)((float)(a11 - a01) + b1) - b0;
    if ( numer >= 0.0 )
    {
        denom = (float)(a00 - (float)(2.0 * a01)) + a11;
        if ( numer < denom )
        {
            ua = numer / denom;
            va = 1.0 - (float)(numer / denom);
        }
        else
        {
            ua = 1.0f;
            va = 0.0f;
        }
    }
    else
    {
        ua = 0.0f;
        va = 1.0f;
    }
LABEL_30:
    *ptOnTri = (float)((float)(ua * *e0) + *v0) + (float)(va * *e1);
    ptOnTri[1] = (float)((float)(ua * e0[1]) + v0[1]) + (float)(va * e1[1]);
    ptOnTri[2] = (float)((float)(ua * e0[2]) + v0[2]) + (float)(va * e1[2]);
}

bool __cdecl CM_DoesCapsuleIntersectTriangle(
                const float *start,
                const float *end,
                float radiusSq,
                const unsigned __int16 *indices)
{
    float fraction; // xmm0_4
    const float *v0; // [esp+B4h] [ebp-B0h]
    float v2_v0[3]; // [esp+B8h] [ebp-ACh] BYREF
    long double a01; // [esp+C4h] [ebp-A0h]
    float scaledPlaneNormal[3]; // [esp+CCh] [ebp-98h] BYREF
    bool isEndCloseEnough; // [esp+DBh] [ebp-89h]
    float distSqToEdge; // [esp+DCh] [ebp-88h]
    float delta[3]; // [esp+E0h] [ebp-84h] BYREF
    long double a11; // [esp+ECh] [ebp-78h]
    double scaledPlaneDist; // [esp+F4h] [ebp-70h]
    bool isStartCloseEnough; // [esp+103h] [ebp-61h]
    long double a00; // [esp+104h] [ebp-60h]
    double det; // [esp+10Ch] [ebp-58h]
    const float *v2; // [esp+118h] [ebp-4Ch]
    float v1_v0[3]; // [esp+11Ch] [ebp-48h] BYREF
    const float *v1; // [esp+128h] [ebp-3Ch]
    float v2_v1[3]; // [esp+12Ch] [ebp-38h] BYREF
    float pt[3]; // [esp+138h] [ebp-2Ch] BYREF
    double cutoffDistSq; // [esp+144h] [ebp-20h]
    double scaleSq; // [esp+14Ch] [ebp-18h]
    long double scaledDist[2]; // [esp+154h] [ebp-10h]

    v0 = cm.verts[*indices];
    v1 = cm.verts[indices[1]];
    v2 = cm.verts[indices[2]];
    v1_v0[0] = *v1 - *v0;
    v1_v0[1] = v1[1] - v0[1];
    v1_v0[2] = v1[2] - v0[2];
    v2_v0[0] = *v2 - *v0;
    v2_v0[1] = v2[1] - v0[1];
    v2_v0[2] = v2[2] - v0[2];
    Vec3Cross(v1_v0, v2_v0, scaledPlaneNormal);
    scaledPlaneDist = (float)((float)((float)(*v0 * scaledPlaneNormal[0]) + (float)(v0[1] * scaledPlaneNormal[1]))
                                                    + (float)(v0[2] * scaledPlaneNormal[2]));
    scaledDist[0] = (float)((float)((float)(*start * scaledPlaneNormal[0]) + (float)(start[1] * scaledPlaneNormal[1]))
                                                + (float)(start[2] * scaledPlaneNormal[2]))
                                - scaledPlaneDist;
    scaledDist[1] = (float)((float)((float)(*end * scaledPlaneNormal[0]) + (float)(end[1] * scaledPlaneNormal[1]))
                                                + (float)(end[2] * scaledPlaneNormal[2]))
                                - scaledPlaneDist;
    if ( scaledDist[0] * scaledDist[1] >= 0.0 )
    {
        scaleSq = (float)((float)((float)(scaledPlaneNormal[0] * scaledPlaneNormal[0])
                                                        + (float)(scaledPlaneNormal[1] * scaledPlaneNormal[1]))
                                        + (float)(scaledPlaneNormal[2] * scaledPlaneNormal[2]));
        cutoffDistSq = radiusSq * scaleSq;
        isStartCloseEnough = cutoffDistSq > scaledDist[0] * scaledDist[0];
        isEndCloseEnough = cutoffDistSq > scaledDist[1] * scaledDist[1];
        if ( !isEndCloseEnough && !isStartCloseEnough )
            return 0;
        a00 = (float)((float)((float)(v1_v0[0] * v1_v0[0]) + (float)(v1_v0[1] * v1_v0[1])) + (float)(v1_v0[2] * v1_v0[2]));
        a01 = (float)((float)((float)(v1_v0[0] * v2_v0[0]) + (float)(v1_v0[1] * v2_v0[1])) + (float)(v1_v0[2] * v2_v0[2]));
        a11 = (float)((float)((float)(v2_v0[0] * v2_v0[0]) + (float)(v2_v0[1] * v2_v0[1])) + (float)(v2_v0[2] * v2_v0[2]));
        det = a00 * a11 - a01 * a01;
    }
    else
    {
        a00 = (float)((float)((float)(v1_v0[0] * v1_v0[0]) + (float)(v1_v0[1] * v1_v0[1])) + (float)(v1_v0[2] * v1_v0[2]));
        a01 = (float)((float)((float)(v1_v0[0] * v2_v0[0]) + (float)(v1_v0[1] * v2_v0[1])) + (float)(v1_v0[2] * v2_v0[2]));
        a11 = (float)((float)((float)(v2_v0[0] * v2_v0[0]) + (float)(v2_v0[1] * v2_v0[1])) + (float)(v2_v0[2] * v2_v0[2]));
        det = a00 * a11 - a01 * a01;
        fraction = scaledDist[0] / (scaledDist[0] - scaledDist[1]);
        Vec3Lerp(start, end, fraction, pt);
        if ( CM_IsPointOverTriangle(pt, v0, v1_v0, v2_v0, a00, a01, a11, det) )
            return 1;
        scaleSq = (float)((float)((float)(scaledPlaneNormal[0] * scaledPlaneNormal[0])
                                                        + (float)(scaledPlaneNormal[1] * scaledPlaneNormal[1]))
                                        + (float)(scaledPlaneNormal[2] * scaledPlaneNormal[2]));
        cutoffDistSq = radiusSq * scaleSq;
        isStartCloseEnough = cutoffDistSq > scaledDist[0] * scaledDist[0];
        isEndCloseEnough = cutoffDistSq > scaledDist[1] * scaledDist[1];
    }
    if ( isStartCloseEnough && CM_IsPointOverTriangle(start, v0, v1_v0, v2_v0, a00, a01, a11, det) )
        return 1;
    if ( isEndCloseEnough && CM_IsPointOverTriangle(end, v0, v1_v0, v2_v0, a00, a01, a11, det) )
        return 1;
    delta[0] = *end - *start;
    delta[1] = end[1] - start[1];
    delta[2] = end[2] - start[2];
    distSqToEdge = CM_DistanceSquaredBetweenSegments(start, delta, v0, v1_v0);
    if ( radiusSq > distSqToEdge )
        return 1;
    distSqToEdge = CM_DistanceSquaredBetweenSegments(start, delta, v0, v2_v0);
    if ( radiusSq > distSqToEdge )
        return 1;
    v2_v1[0] = *v2 - *v1;
    v2_v1[1] = v2[1] - v1[1];
    v2_v1[2] = v2[2] - v1[2];
    distSqToEdge = CM_DistanceSquaredBetweenSegments(start, delta, v1, v2_v1);
    return radiusSq > distSqToEdge;
}

bool __cdecl CM_IsPointOverTriangle(
                const float *pt,
                const float *v0,
                const float *e0,
                const float *e1,
                long double a00,
                long double a01,
                long double a11,
                double det)
{
    double b1; // [esp+4h] [ebp-30h]
    double u; // [esp+Ch] [ebp-28h]
    double v; // [esp+14h] [ebp-20h]
    float v0_pt; // [esp+20h] [ebp-14h]
    float v0_pt_4; // [esp+24h] [ebp-10h]
    float v0_pt_8; // [esp+28h] [ebp-Ch]
    double b0; // [esp+2Ch] [ebp-8h]

    v0_pt = *v0 - *pt;
    v0_pt_4 = v0[1] - pt[1];
    v0_pt_8 = v0[2] - pt[2];
    b0 = (float)((float)((float)(*e0 * v0_pt) + (float)(e0[1] * v0_pt_4)) + (float)(e0[2] * v0_pt_8));
    b1 = (float)((float)((float)(*e1 * v0_pt) + (float)(e1[1] * v0_pt_4)) + (float)(e1[2] * v0_pt_8));
    u = a01 * b1 - a11 * b0;
    if ( u < 0.0 )
        return 0;
    v = a01 * b0 - a00 * b1;
    return v >= 0.0 && det >= u + v;
}

double __cdecl CM_DistanceSquaredBetweenSegments(
                const float *start0,
                const float *delta0,
                const float *start1,
                const float *delta1)
{
    double b1; // [esp+24h] [ebp-78h]
    double a01; // [esp+3Ch] [ebp-60h]
    double c; // [esp+44h] [ebp-58h]
    double a11; // [esp+5Ch] [ebp-40h]
    double numer0; // [esp+64h] [ebp-38h]
    double a00; // [esp+6Ch] [ebp-30h]
    double det; // [esp+74h] [ebp-28h]
    float s0_s1; // [esp+80h] [ebp-1Ch]
    float s0_s1_4; // [esp+84h] [ebp-18h]
    float s0_s1_8; // [esp+88h] [ebp-14h]
    double numer1; // [esp+8Ch] [ebp-10h]
    double numer1a; // [esp+8Ch] [ebp-10h]
    double b0; // [esp+94h] [ebp-8h]

    a00 = (float)((float)((float)(*delta0 * *delta0) + (float)(delta0[1] * delta0[1])) + (float)(delta0[2] * delta0[2]));
    a01 = (float)((float)((float)(*delta0 * *delta1) + (float)(delta0[1] * delta1[1])) + (float)(delta0[2] * delta1[2]));
    a11 = (float)((float)((float)(*delta1 * *delta1) + (float)(delta1[1] * delta1[1])) + (float)(delta1[2] * delta1[2]));
    s0_s1 = *start0 - *start1;
    s0_s1_4 = start0[1] - start1[1];
    s0_s1_8 = start0[2] - start1[2];
    b0 = (float)((float)((float)(*delta0 * s0_s1) + (float)(delta0[1] * s0_s1_4)) + (float)(delta0[2] * s0_s1_8));
    b1 = (float)((float)((float)(*delta1 * s0_s1) + (float)(delta1[1] * s0_s1_4)) + (float)(delta1[2] * s0_s1_8));
    c = (float)((float)((float)(s0_s1 * s0_s1) + (float)(s0_s1_4 * s0_s1_4)) + (float)(s0_s1_8 * s0_s1_8));
    det = a00 * a11 - a01 * a01;
    numer0 = b1 * a01 - b0 * a11;
    if ( numer0 > 0.0 )
    {
        if ( numer0 < det )
        {
            numer1a = b1 * a00 - b0 * a01;
            if ( numer1a > 0.0 )
            {
                if ( numer1a < det )
                    return (float)((b0
                                                - a01 * (numer1a * (1.0 / det))
                                                + b0
                                                - a01 * (numer1a * (1.0 / det))
                                                + a00 * (numer0 * (1.0 / det)))
                                             * (numer0
                                                * (1.0
                                                 / det))
                                             + (a11 * (numer1a * (1.0 / det)) - (b1 + b1)) * (numer1a * (1.0 / det))
                                             + c);
                goto t1_is_clamped_to_1;
            }
        }
        else
        {
            numer1 = b1 + a01;
            if ( b1 + a01 > 0.0 )
            {
                if ( numer1 < a11 )
                    return (float)((a11 * (numer1 / a11) - (a01 + b1 + a01 + b1)) * (numer1 / a11) + a00 + b0 + b0 + c);
                goto t1_is_clamped_to_1;
            }
        }
t1_is_clamped_to_0:
        if ( b0 >= 0.0 )
            return (float)((float)((float)(s0_s1 * s0_s1) + (float)(s0_s1_4 * s0_s1_4)) + (float)(s0_s1_8 * s0_s1_8));
        if ( -b0 < a00 )
            return (float)((b0 + b0 + a00 * (-b0 / a00)) * (-b0 / a00) + c);
        else
            return (float)(b0 + b0 + a00 + c);
    }
    if ( b1 <= 0.0 )
        goto t1_is_clamped_to_0;
    if ( b1 < a11 )
        return (float)((a11 * (b1 / a11) - (b1 + b1)) * (b1 / a11) + c);
t1_is_clamped_to_1:
    if ( b0 < a01 )
    {
        if ( a01 - b0 < a00 )
            return (float)((a00 * ((a01 - b0) / a00) + b0 - a01 + b0 - a01) * ((a01 - b0) / a00) + a11 - (b1 + b1) + c);
        else
            return (float)(a00 + a11 + b0 - b1 - a01 + b0 - b1 - a01 + c);
    }
    else
    {
        return (float)(a11 - (b1 + b1) + c);
    }
}

void __cdecl CM_MeshTest(const traceWork_t *tw, const CollisionAabbTree *tree, trace_t *trace)
{
    int checkStamp; // [esp+0h] [ebp-1Ch]
    dmaterial_t *materialInfo; // [esp+4h] [ebp-18h]
    int triCount; // [esp+Ch] [ebp-10h]
    unsigned __int16 *indices; // [esp+10h] [ebp-Ch]
    int partitionIndex; // [esp+14h] [ebp-8h]
    const CollisionPartition *partition; // [esp+18h] [ebp-4h]

    if ( tw->isPoint
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_mesh.cpp", 1687, 0, "%s", "!tw->isPoint") )
    {
        __debugbreak();
    }
    if ( trace->allsolid
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_mesh.cpp", 1688, 0, "%s", "!trace->allsolid") )
    {
        __debugbreak();
    }
    materialInfo = &cm.materials[tree->materialIndex];
    if ( (materialInfo->contentFlags & tw->contents) != 0 )
    {
        partitionIndex = tree->u.firstChildIndex;
        partition = &cm.partitions[partitionIndex];
        checkStamp = tw->threadInfo.checkcount.global;
        if ( tw->threadInfo.checkcount.partitions )
        {
            if ( tw->threadInfo.checkcount.partitions[partitionIndex] != checkStamp )
            {
                tw->threadInfo.checkcount.partitions[partitionIndex] = checkStamp;
                indices = &cm.triIndices[3 * partition->firstTri];
                for ( triCount = partition->triCount; triCount; --triCount )
                {
                    CM_PositionTestCapsuleInTriangle(tw, indices, trace);
                    if ( trace->startsolid )
                        break;
                    indices += 3;
                }
                if ( trace->allsolid )
                {
                    trace->sflags = materialInfo->surfaceFlags;
                    trace->cflags = materialInfo->contentFlags;
                }
            }
        }
    }
}

