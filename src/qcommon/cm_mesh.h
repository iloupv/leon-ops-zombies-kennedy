#pragma once

struct traceWork_t;
struct CollisionBorder;
struct CollisionAabbTree;
struct trace_t;
struct cLeaf_s;

enum SphereEdgeTraceResult : __int32
{                                       // XREF: CM_TraceCapsuleThroughTriangle/r
                                        // CM_TraceSphereThroughBorder/r
    SPHERE_HITS_EDGE   = 0x0,
    SPHERE_MISSES_EDGE = 0x1,
    SPHERE_MAY_HIT_V0  = 0x2,
    SPHERE_MAY_HIT_V1  = 0x3,
};

void __cdecl CM_TracePointThroughTriangle(const traceWork_t *tw, const unsigned __int16 *indices, trace_t *trace);
void __cdecl CM_TraceThroughPartition(const traceWork_t *tw, int partitionIndex, trace_t *trace);
void __cdecl CM_TraceCapsuleThroughTriangle(
                const traceWork_t *tw,
                int triIndex,
                const unsigned __int16 *indices,
                trace_t *trace);
SphereEdgeTraceResult __cdecl CM_TraceSphereThroughEdge(
                const traceWork_t *tw,
                const float *sphereStart,
                const float *v0,
                const float *v0_v1,
                trace_t *trace);
bool __cdecl Vec3IsNormalizedEpsilon(const float *v, float epsilon);
void __cdecl CM_TraceSphereThroughVertex(
                const traceWork_t *tw,
                bool isWalkable,
                const float *sphereStart,
                const float *v,
                trace_t *trace);
void __cdecl CM_TraceCapsuleThroughBorder(const traceWork_t *tw, const CollisionBorder *border, trace_t *trace);
void __cdecl CM_TraceSphereThroughBorder(
                const traceWork_t *tw,
                const CollisionBorder *border,
                float offsetZ,
                trace_t *trace);
void __cdecl CM_TraceThroughAabbTree(const traceWork_t *tw, const CollisionAabbTree *aabbTree, trace_t *trace);
void __cdecl CM_TraceThroughAabbTree_r(const traceWork_t *tw, const CollisionAabbTree *aabbTree, trace_t *trace);
void __cdecl CM_TraceThroughAabbTree_work(const traceWork_t *tw, const CollisionAabbTree *aabbTree, trace_t *trace);
void __cdecl CM_SightTraceThroughAabbTree(const traceWork_t *tw, const CollisionAabbTree *aabbTree, trace_t *trace);
void __cdecl CM_MeshTestInLeaf(const traceWork_t *tw, cLeaf_s *leaf, trace_t *trace);
void __cdecl CM_PositionTestInAabbTree_r(const traceWork_t *tw, CollisionAabbTree *aabbTree, trace_t *trace);
void __cdecl CM_PositionTestCapsuleInTriangle(const traceWork_t *tw, const unsigned __int16 *indices, trace_t *trace);
double __cdecl CM_DistanceSquaredFromPointToTriangle(const float *pt, const unsigned __int16 *indices);
void __cdecl CM_ClosestPointOnTri(
                const float *pt,
                const float *v0,
                const float *e0,
                const float *e1,
                float a00,
                float a01,
                float a11,
                float *ptOnTri);
bool __cdecl CM_DoesCapsuleIntersectTriangle(
                const float *start,
                const float *end,
                float radiusSq,
                const unsigned __int16 *indices);
bool __cdecl CM_IsPointOverTriangle(
                const float *pt,
                const float *v0,
                const float *e0,
                const float *e1,
                long double a00,
                long double a01,
                long double a11,
                double det);
double __cdecl CM_DistanceSquaredBetweenSegments(
                const float *start0,
                const float *delta0,
                const float *start1,
                const float *delta1);
void __cdecl CM_MeshTest(const traceWork_t *tw, const CollisionAabbTree *tree, trace_t *trace);
