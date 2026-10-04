#pragma once

#include <cstdio>
#include <universal/dvar.h>

void __cdecl SteamAPIDebugTextHook(int severity, const char *pchDebugText);
void __cdecl    LiveSteam_Quit(bool showErrorMessage);
void __cdecl LiveSteam_CheckAccess();
bool __cdecl LiveSteam_IsInitialized();
char __cdecl LiveSteam_IsClientSignedInLocally();
bool __cdecl LiveSteam_IsClientSignedInOnline();
unsigned __int64 __cdecl LiveSteam_GetClientIDAsXUID();
const char *__cdecl LiveSteam_GetCountryCode();
unsigned __int64 __cdecl LiveSteam_GetUid();
unsigned int __stdcall ShellExecuteThredProc(const char *lpParam);
bool __cdecl LiveSteam_LaunchOtherApp(const char *cmd);
void __cdecl LiveSteam_ShowStore();
void __cdecl LiveSteam_PopOverlayForSteamID(unsigned __int64 steamID);
void __cdecl LiveSteam_Init();
void __cdecl LiveSteam_TestFriends_f();
FILE *LiveSteam_CreateSteamAppIdFile();
void __cdecl LiveSteam_Shutdown();
void __cdecl LiveSteam_CheckForP2PMessages();
void __cdecl LiveSteam_SendP2PMessage(unsigned __int64 uid, unsigned __int8 *payload, unsigned int payloadsize);
void __cdecl LiveSteam_Frame();
void __cdecl LiveSteam_RunCallbacks();
const char *__cdecl LiveSteam_GetClientPersonaName(bool shortName);
char __cdecl LiveSteam_DWUserNameFromSteamID(char *dwUserName);
char __cdecl LiveSteam_AuthRequestTicket(const void *authBlob, unsigned int authBlobSize);
char __cdecl LiveSteam_AuthGetRequestedTicket(void *ticketBuf, unsigned int ticketBufSize, unsigned int *ticketSize);
int __cdecl LiveSteam_Client_ConnectToSteamServer(unsigned __int64 serverID, void *authBlob, unsigned int bufferSize);
void LiveSteam_Client_SteamDisconnect();

extern const dvar_t *livesteam_sv_vac;