#pragma once
#include <client/cl_main.h>
#include <qcommon/bitarray.h>

struct Material;
struct usercmd_s;
struct DObj;
struct DObjAnimMat;
struct refdef_s;
struct snapshot_s;


void __cdecl CL_GetScreenDimensions(int *width, int *height, float *aspect);
double __cdecl CL_GetScreenAspectRatioDisplayPixel();
int __cdecl CL_GetUserCmd(int localClientNum, int cmdNumber, usercmd_s *ucmd);
void __cdecl CL_AdjustUserCmdAngles(int localClientNum, int cmdNumber, float *adjust);
int __cdecl CL_GetCurrentCmdNumber(int localClientNum);
void __cdecl CL_GetCurrentSnapshotNumber(int localClientNum, int *snapshotNumber, int *serverTime);
int __cdecl CL_GetSnapshot(int localClientNum, int snapshotNumber, snapshot_s *snapshot);
void __cdecl CL_SetUserCmdWeapons(int localClientNum, int weapon, int offHandIndex, int lastWeaponForAlt);
void __cdecl CL_SetUserCmdAimValues(int localClientNum, const float *kickAngles);
void __cdecl CL_SetUserCmdOrigin(
                int localClientNum,
                const float *origin,
                const float *velocity,
                const float *viewangles,
                int bobCycle,
                int movementDir);
void __cdecl CL_SetUserCmdVehicle(
                int localClientNum,
                bool inVehicle,
                const float *origin,
                const float *angles,
                const float *tVelocity,
                const float *aVelocity);
void __cdecl CL_SetFOVSensitivityScale(int localClientNum, float scale);
void __cdecl CL_SetExtraButtons(int localClientNum, bitarray<51> *button_bits);
void __cdecl CL_DumpReliableCommands(int localClientNum);
int __cdecl CL_CGameNeedsServerCommand(int localClientNum, int serverCommandNumber);
void __cdecl CL_ConfigstringModified(int localClientNum);
void __cdecl CL_SetExpectedHunkUsage(const char *mapname);
void __cdecl CL_CM_LoadMap(char *mapname);
void __cdecl CL_ShutdownCGame(int localClientNum);
void __cdecl CL_LocalClient_ClearCUIFlag(int localClientNum, int flag);
void __cdecl CL_ShutdownAllClientsCGame();
bool __cdecl CL_DObjCreateSkelForBone(DObj *obj, int boneIndex, DObjAnimMat **pMatOut);
void __cdecl CL_SubtitlePrint(int localClientNum, const char *text, int duration, int lineWidth);
char *__cdecl CL_GetConfigString(unsigned int configStringIndex);
void __cdecl CL_FinishLoadingModels();
void __cdecl CL_GetDebugViewForward(float *forward);
void __cdecl CL_GetDebugViewPos(float *pos);
void __cdecl CL_RenderScene(refdef_s *fd, int frameTime);
void __cdecl CL_RenderMissileCam(const refdef_s *fd, int frameTime);
void __cdecl CL_SetCustomConstant(unsigned int type, const float *vec);
void __cdecl CL_DrawStretchPicPhysical(
                float x,
                float y,
                float w,
                float h,
                float s1,
                float t1,
                float s2,
                float t2,
                const float *color,
                Material *material);
void __cdecl CL_DrawStretchPicPhysicalW(
                float x,
                float y,
                float w0,
                float w,
                float h,
                float s1,
                float t1,
                float s2,
                float t2,
                const float *color,
                Material *material);
void __cdecl CL_DrawStretchPicPhysicalRotateXY(
                float x,
                float y,
                float w,
                float h,
                float s1,
                float t1,
                float s2,
                float t2,
                float angle,
                const float *color,
                Material *material);
void __cdecl CL_DrawStretchPicPhysicalRotateXYW(
                float x,
                float y,
                float w0,
                float w,
                float h,
                float s1,
                float t1,
                float s2,
                float t2,
                float angle,
                const float *color,
                Material *material);
void __cdecl CL_DrawStretchPicW(
                const ScreenPlacement *scrPlace,
                float x,
                float y,
                float w0,
                float w,
                float h,
                int horzAlign,
                int vertAlign,
                float s1,
                float t1,
                float s2,
                float t2,
                const float *color,
                Material *material);
void __cdecl CL_DrawStretchPic(
                const ScreenPlacement *scrPlace,
                float x,
                float y,
                float w,
                float h,
                int horzAlign,
                int vertAlign,
                float s1,
                float t1,
                float s2,
                float t2,
                const float *color,
                Material *material);
void __cdecl CL_DrawStretchPicRotatedST(
                const ScreenPlacement *scrPlace,
                float x,
                float y,
                float w,
                float h,
                int horzAlign,
                int vertAlign,
                float centerS,
                float centerT,
                float radiusST,
                float scaleFinalS,
                float scaleFinalT,
                float angle,
                const float *color,
                Material *material);
void __cdecl CL_DrawStretchPicRotatedXY(
                const ScreenPlacement *scrPlace,
                float x,
                float y,
                float w,
                float h,
                float angle,
                const float *color,
                Material *material);
void __cdecl CL_CapTurnRate(int localClientNum, float maxPitchSpeed, float maxYawSpeed);
void __cdecl CL_SetViewAngles(int localClientNum, const float *angles);
void __cdecl CL_AdjustViewAngles(int localClientNum, const float *adjust);
void __cdecl CL_ClampViewAngle(int localClientNum, unsigned int angle, float min, float max);
void __cdecl CL_SyncTimes(int localClientNum);
int __cdecl LoadWorld(char *mapname);
void __cdecl CL_UpdateLevelHunkUsage();
void    CL_StartLoading();
void __cdecl CL_InitCGame(int localClientNum);
void __cdecl CL_SetLocalClientConnectionState(int client, connstate_t state);
void __cdecl CL_LocalClient_SetCUIFlag(int localClientNum, int flag);
void __cdecl CL_RestartCGame(int localClientNum);
void __cdecl CL_FirstSnapshot(int localClientNum);
void __cdecl CL_SetCGameTime(int localClientNum);
void __cdecl CL_AdjustTimeDelta(int localClientNum);
void __cdecl CL_UpdateTimeDemo(int localClientNum);
char *__cdecl CL_TimeDemoLogBaseName(const char *mapname);
void __cdecl CL_UpdateDemoPlaybackSpeed(int localClientNum);
void __cdecl CL_SetADS(int localClientNum, bool ads);
void __cdecl CL_DrawString(int x, int y, char *pszString, int bShadow, int iCharHeight);
void __cdecl CL_LookupColor(unsigned __int8 c, float *color);
void __cdecl CL_UpdateColor();
bool __cdecl CL_IsServerRestarting(int localClientNum);
