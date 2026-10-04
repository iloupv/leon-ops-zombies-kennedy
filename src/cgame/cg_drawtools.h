#pragma once
#include <client/screen_placement.h>
#include <gfx_d3d/r_material.h>
#include <gfx_d3d/r_font.h>

struct DObjAnimMat;
struct centity_s;

double __cdecl CG_GetPicWidth(Material *mat);
const Material *__cdecl Material_FromHandle(Material *handle);
void __cdecl CG_DrawRotatedPicPhysicalW(
                const ScreenPlacement *scrPlace,
                float x,
                float y,
                float w,
                float width,
                float height,
                float angle,
                const float *color,
                Material *material);
void __cdecl CG_DrawRotatedPicPhysical(
                const ScreenPlacement *scrPlace,
                float x,
                float y,
                float width,
                float height,
                float angle,
                const float *color,
                Material *material);
void __cdecl CG_DrawRotatedPicW(
                const ScreenPlacement *scrPlace,
                float x,
                float y,
                float w,
                float width,
                float height,
                int horzAlign,
                int vertAlign,
                float angle,
                const float *color,
                Material *material);
void __cdecl CG_DrawRotatedPic(
                const ScreenPlacement *scrPlace,
                float x,
                float y,
                float width,
                float height,
                int horzAlign,
                int vertAlign,
                float angle,
                const float *color,
                Material *material);
void __cdecl CG_DrawRotatedQuadPic(
                const ScreenPlacement *scrPlace,
                float x,
                float y,
                const float (*verts)[2],
                float angle,
                const float *color,
                Material *material);
void __cdecl CG_DrawVLine(
                const ScreenPlacement *scrPlace,
                float x,
                float top,
                float lineWidth,
                float height,
                int horzAlign,
                int vertAlign,
                const float *color,
                Material *material);
double __cdecl Vec2NormalizeTo(const float *v, float *out);
void __cdecl CG_DrawStringExt(
                const ScreenPlacement *scrPlace,
                float x,
                float y,
                char *string,
                const float *setColor,
                int forceColor,
                int shadow,
                float charHeight);
int __cdecl CG_DrawDevString(
                const ScreenPlacement *scrPlace,
                float x,
                float y,
                float xScale,
                float yScale,
                char *s,
                const float *color,
                char align,
                Font_s *font);
int __cdecl CG_DrawBigDevString(const ScreenPlacement *scrPlace, float x, float y, char *s, float alpha, char align);
int __cdecl CG_DrawBigDevStringColor(
                const ScreenPlacement *scrPlace,
                float x,
                float y,
                char *s,
                const float *color,
                char align);
int __cdecl CG_DrawSmallDevStringColor(
                const ScreenPlacement *scrPlace,
                float x,
                float y,
                char *s,
                const float *color,
                char align);
double __cdecl CG_FadeAlpha(int timeNow, int startMsec, int totalMsec, int fadeMsec);
float *__cdecl CG_FadeColor(int timeNow, int startMsec, int totalMsec, int fadeMsec);
void __cdecl CG_MiniMapChanged(int localClientNum);
void __cdecl CG_NorthDirectionChanged(int localClientNum);
void __cdecl CG_DebugBox(
                const float *origin,
                const float *mins,
                const float *maxs,
                float yaw,
                const float *color,
                int depthTest,
                int duration);
void __cdecl CG_DebugBoxOriented(
                const float *origin,
                const float *mins,
                const float *maxs,
                const float (*rotation)[3],
                const float *color,
                int depthTest,
                int duration);
void __cdecl CG_ScoreboardTeamColor(int localClientNum, int team, float *color);
void __cdecl CG_RelativeTeamColor(int clientNum, float *color, int localClientNum);
void __cdecl CG_XModelDebugBoxes(
                int localClientNum,
                const centity_s *cent,
                const float *color,
                int *partBits,
                int duration);
void __cdecl ConvertQuatToMat(const DObjAnimMat *mat, float (*axis)[3]);
void __cdecl CG_DebugLine(const float *start, const float *end, const float *color, int depthTest, int duration);
void __cdecl CG_DebugSphere(
                const float *center,
                float radius,
                const float *floatColor,
                int sideCount,
                int depthTest,
                int duration);
