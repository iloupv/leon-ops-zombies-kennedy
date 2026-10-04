#pragma once

#ifdef BO1_DW
#include <DemonWare/bdLobbyService.h>

enum sv_dedicatedauthState : __int32
{
    SV_DWNOTAUTHORIZED = 0x0,
    SV_DWAUTHORIZING   = 0x1,
    SV_DWAUTHORIZED    = 0x2,
    SV_DWAUTHORIZEFAIL = 0x3,
};

enum DWOnlineStatus : __int32
{                                       // XREF: ?Live_Frame_MP@@YA_NHH@Z/r
                                        // ?Live_Frame_MP@@YA_NHH@Z/r ...
    DW_WAITING_FOR_USERNAME = 0x0,
    DW_LIVE_ERROR = 0x1,
    DW_LIVE_DISCONNECTED = 0x2,
    DW_LIVE_CONNECTING = 0x3,
    DW_LIVE_CONNECTED = 0x4,
    DW_LIVE_PLATFORM_AUTH_ERROR = 0x5,
};


bool __cdecl SV_IsConnectedToDW();
void __cdecl dwInit();
void __cdecl dwInitForController(int localControllerIndex);
void __cdecl DWDisconnectLobby(bdLobbyService *lobby);
void __cdecl dw_disconnect(int controllerIndex);
int __cdecl dwGetLogOnStatus(int localControllerIndex);
void __cdecl dwLogOnStart(int controllerIndex);
void __cdecl dwLogonSeAcquiredSteamTicket();
bool __cdecl SV_DedicatedReadCDKey(char *cdkey, int cdkeysize);
void __cdecl DW_DedicatedLogonStart(int controllerIndex);
void __cdecl DW_DedicatedLogonComplete(int controllerIndex);
const char *__cdecl dwSelectLSGAddress();
void __cdecl CL_GetSecretSauce(char *secretsauce);
void __cdecl dwLogOnComplete();
void __cdecl dwDNSCleanup(int localControllerIndex);
void __cdecl dwLobbyPump(int localControllerIndex);
void __cdecl r_PumpDemonware();
bool __cdecl dwDNSInit(char *authAddr, int controllerIndex);
int __cdecl dwDNSPump(int localControllerIndex, bdInetAddr *authAddr, bdInetAddr *lsgAddr);
void __cdecl DWDedicatedLogon();
void __cdecl DWDedicatedLobbyPump();
bool Com_CanWriteLeaderboards(void *thisptr);
bool __cdecl isOfficialServer(int licensetype);
bool __cdecl isOfficialRentedServer(int licensetype);

#endif