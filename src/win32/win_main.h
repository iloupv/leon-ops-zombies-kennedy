#pragma once

#include <win32/win_local.h>
#include <Windows.h>
#include <qcommon/common.h>

bool __cdecl PC_StartWithNoSounds();
void __cdecl Sys_GetInfo(SysInfo *info);
bool __cdecl Sys_HasConfigureChecksumChanged(int checksum);
bool __cdecl Sys_ShouldUpdateForConfigChange();
void Sys_RegisterInfoDvars();
bool __cdecl Sys_HasInfoChanged();
bool __cdecl Sys_ShouldUpdateForInfoChange();
void __cdecl Sys_ArchiveInfo(int checksum);
void __cdecl    Sys_DirectXFatalError();
void __cdecl    Sys_OutOfMemErrorInternal(const char *filename, int line);
void __cdecl Sys_QuitAndStartProcess(const char *exeName);
void __cdecl Sys_OpenURL(const char *url, int doexit);
void    Sys_Error(char *error, ...);
void __cdecl    Sys_Quit();
void Sys_SpawnQuitProcess();
void __cdecl Sys_Print(char *msg);
char *__cdecl Sys_GetClipboardData();
void __cdecl Sys_QueEvent(unsigned int time, sysEventType_t type, int value, int value2, int ptrLength, void *ptr);
void Sys_ShutdownEvents();
void __cdecl Sys_LoadingKeepAlive();
sysEvent_t *__cdecl Win_GetEvent(sysEvent_t *result);
sysEvent_t *__cdecl Sys_GetEvent(sysEvent_t *result);
void __cdecl Sys_Mjpeg();
void __cdecl Sys_MjpegClose();
void __cdecl Sys_Init();
void __cdecl Sys_In_Restart_f();
void __cdecl Sys_Net_Restart_f();
int __cdecl Sys_CheckCrashOrRerun();
void    Sys_NoFreeFilesError();
int __cdecl Sys_IsGameProcess(unsigned int id);
void __cdecl Sys_NormalExit();
int __cdecl PrivateUnhandledExceptionFilter(_EXCEPTION_POINTERS *ExceptionInfo);
int __stdcall WinMain(HINSTANCE__ *hInstance, HINSTANCE__ *hPrevInstance, char *lpCmdLine, int nCmdShow);
void Sys_FindInfo();
int Sys_GetSemaphoreFileName();
void Win_RegisterClass();
void PrintWorkingDir();
char __cdecl CheckRemoteSession();
bool __cdecl StartingDedicatedServer(char *cmdline);
bool __cdecl Sys_IsHeadless(); // zombies: '+set bo1_headless 1' - no windows at all (see win_main.cpp)
bool __cdecl Sys_IsHeadlessClient(); // zombies: '+set bo1_headless_client 1' - headless listen server + local client (see win_main.cpp)
void __cdecl Sys_HeadlessNoteHud(int pmType, bool hudDrawn); // zombies: headless restart timeline (cgame, per 2D frame)
void __cdecl Sys_HeadlessTimeline(const char *what); // zombies: headless restart timeline event, real ms after init
void __cdecl Sys_HeadlessLevelNotify(const char *name); // L20: bo1_quitnotify - count a level notify (headless)
void __cdecl Sys_HeadlessLogAssert(const char *filename, int line, const char *message); // zombies: headless assert report
unsigned int __cdecl Sys_HitchWatchBegin(); // k1: '+set bo1_hitchwatch <ms>' - thread dumps while one client frame runs long
void __cdecl Sys_HitchWatchEnd();
void __cdecl Sys_SetQuitRequested(); // L5: a quit / window close was asked for (Com_Quit_f, WM_CLOSE)
bool __cdecl Sys_IsQuitRequested();
void __cdecl Sys_QuitWithLostDevice(int lostMs); // L5: render thread, device lost and a quit pending - ends the process
unsigned int __cdecl Sys_FramePerfHitchMs(); // p1: '+set bo1_frameperf <ms>' (headless) - whole-frame timing and zone profile

extern const dvar_t *sys_configureGHz;
extern const dvar_t *sys_sysMB;
extern const dvar_t *sys_gpu;
extern const dvar_t *sys_configSum;
extern const dvar_t *sys_SSE;

extern bool g_allowMature;
