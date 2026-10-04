#pragma once
#include "r_rendercmds.h"

void __cdecl R_AddDebugPolygon(
                DebugGlobals *debugGlobalsEntry,
                int pointCount,
                float (*points)[3],
                const float *faceColor,
                bool faceDepthTest,
                const float *edgeColor,
                bool edgeDepthTest);
void __cdecl R_AddDebugPolygonInternal(
                PolySet *polySet,
                int pointCount,
                float (*points)[3],
                const float *faceColor,
                bool faceDepthTest,
                const float *edgeColor,
                bool edgeDepthTest);
void __cdecl R_AddPersistentDebugPolygon(
                DebugGlobals *debugGlobalsEntry,
                int pointCount,
                float (*points)[3],
                const float *faceColor,
                bool faceDepthTest,
                const float *edgeColor,
                bool edgeDepthTest);
void __cdecl R_ClearPersistentDebugPolygons();
void __cdecl R_AddDebugAxis(
                DebugGlobals *debugGlobalsEntry,
                const float *origin,
                const float (*axis)[3],
                float length,
                int doDepthTest);
void __cdecl R_AddDebugLine(
                DebugGlobals *debugGlobalsEntry,
                const float *start,
                const float *end,
                const float *color,
                int doDepthTest);
void __cdecl R_AddDebugBox(DebugGlobals *debugGlobalsEntry, const float *mins, const float *maxs, const float *color);
void __cdecl R_AddDebugString(
                DebugGlobals *debugGlobalsEntry,
                const float *origin,
                const float *color,
                float scale,
                char *string);
void __cdecl R_AddScaledDebugString(
                DebugGlobals *debugGlobalsEntry,
                const GfxViewParms *viewParms,
                const float *origin,
                const float *color,
                char *string);
void __cdecl R_InitDebugEntry(DebugGlobals *debugGlobalsEntry);
void __cdecl R_InitDebug();
void __cdecl R_ShutdownDebugEntry(DebugGlobals *debugGlobalsEntry);
void __cdecl R_TransferDebugGlobals(DebugGlobals *debugGlobalsEntry);
void __cdecl R_UpdatePersistentDebugPolys(DebugGlobals *debugGlobalsEntry);
void __cdecl R_ShutdownDebug();
void __cdecl R_CopyDebugStrings(
                trDebugString_t *clStrings,
                int clStringCnt,
                trDebugString_t *svStrings,
                int svStringCnt,
                int maxStringCount);
void __cdecl R_CopyDebugLines(
                trDebugLine_t *clLines,
                int clLineCnt,
                trDebugLine_t *svLines,
                int svLineCnt,
                int maxLineCount);
void __cdecl R_CopyDebugSpheres(
                trDebugSphere_t *clSpheres,
                int clSphereCnt,
                trDebugSphere_t *svSpheres,
                int svSphereCnt,
                int maxSphereCount);
