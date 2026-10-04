#pragma once
#include <client_mp/cl_main_mp.h>

void __cdecl CL_DebugInitSVThreadVariables();
void __cdecl CL_AddDebugString(const float *xyz, const float *color, float scale, char *text, int duration);
bool __cdecl CreateDebugStringsIfNeeded();
void __cdecl AddDebugStringInternal(
                const float *xyz,
                const float *color,
                float scale,
                char *text,
                int duration,
                clientDebugStringInfo_t *info);
void __cdecl CL_AddDebugSphere(
                const float *center,
                float radius,
                const float *color,
                int sideCount,
                int depthTest,
                int duration);
bool __cdecl CreateDebugSpheresIfNeeded();
void __cdecl AddDebugSphereInternal(
                const float *center,
                float radius,
                const float *color,
                int sideCount,
                int depthTest,
                int duration,
                clientDebugSphereInfo_t *info);
void __cdecl CL_AddDebugLine(const float *start, const float *end, const float *color, int depthTest, int duration);
bool __cdecl CreateDebugLinesIfNeeded();
void __cdecl AddDebugLineInternal(
                const float *start,
                const float *end,
                const float *color,
                int depthTest,
                int duration,
                clientDebugLineInfo_t *info);
void __cdecl CL_AddDebugAxis(const float *origin, const float (*axis)[3], float length, int doDepthTest, int duration);
void __cdecl CL_AddDebugStarWithText(
                const float *point,
                const float *starColor,
                const float *textColor,
                char *string,
                float fontsize,
                int duration);
void __cdecl CL_AddDebugStar(const float *point, const float *color, int duration);
void __cdecl CL_AddDebugBox(
                const float *origin,
                const float *mins,
                const float *maxs,
                float yaw,
                const float *color,
                int depthTest,
                int duration);
void __cdecl CL_AddDebugBox(
                const float (*axis)[3],
                const float *origin,
                const float *mins,
                const float *maxs,
                const float *color,
                int depthTest,
                int duration);
void __cdecl CL_FlushDebugClientData();
void __cdecl FlushDebugStrings(clientDebugStringInfo_t *info, int fromServer);
void __cdecl FlushDebugLines(clientDebugLineInfo_t *info, int fromServer);
void __cdecl FlushDebugSpheres(clientDebugSphereInfo_t *info, int fromServer);
void __cdecl CL_UpdateDebugClientData();
void __cdecl CL_FlushDebugServerData();
void __cdecl CL_UpdateDebugServerData();
void __cdecl CL_ShutdownDebugData();
