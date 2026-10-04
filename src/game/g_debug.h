#pragma once

struct gentity_s;
struct cLeafBrushNode_s;

struct DebugDrawBrushInfo // sizeof=0x38
{                                       // XREF: .data:g_debugDrawBrushInfo/r
    int depthTest;                      // XREF: G_DebugDrawBrushModel(gentity_s *,float const * const,int,int)+4D/w
                                        // DrawBrushPoly+5C/r ...
    int duration;                       // XREF: G_DebugDrawBrushModel(gentity_s *,float const * const,int,int)+56/w
                                        // DrawBrushPoly+56/r ...
    float transform[4][3];              // XREF: G_DebugDrawBrushModel(gentity_s *,float const * const,int,int)+5C/o
                                        // DrawBrushPoly+A/o ...
};

void __cdecl G_DebugStar(const float *point, const float *color, int duration);
void __cdecl G_DebugCircleEx(
                const float *center,
                float radius,
                const float *dir,
                const float *color,
                int depthTest,
                int duration);
void __cdecl G_DebugLine(const float *start, const float *end, const float *color, int depthTest);
void __cdecl G_DebugCircle(
                const float *center,
                float radius,
                const float *color,
                int depthTest,
                int onGround,
                int duration);
void __cdecl G_DebugArc(
                const float *center,
                float radius,
                float angle0,
                float angle1,
                const float *color,
                int depthTest,
                int duration);
void __cdecl G_DebugDrawBrushModel(gentity_s *entity, const float *color, int depthTest, int duration);
void __cdecl G_DebugDrawBrush_r(cLeafBrushNode_s *node, const float *color);
void __cdecl DrawBrushPoly(int numPoints, float (*points)[3], const float *color);
