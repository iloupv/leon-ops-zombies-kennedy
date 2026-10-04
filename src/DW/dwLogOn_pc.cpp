#ifdef BO1_DW
#include "dwLogOn_pc.h"
#include <universal/dvar.h>
#include <universal/assertive.h>
#include <qcommon/common.h>
#include <live/live_win.h>
#include <live/live_sessions_win.h>
#include <demo/demo_recording.h>
#include "MatchRecorder.h"
#include <qcommon/threads.h>

sv_dedicatedauthState g_svdedicatedauthstate;

bool __cdecl SV_IsConnectedToDW()
{
    return g_svdedicatedauthstate == SV_DWAUTHORIZED;
}

void __cdecl dwInit()
{
    int i; // [esp+0h] [ebp-4h]

    for ( i = 0; i < 1; ++i )
        dwInitForController(i);
}

void __cdecl dwInitForController(int localControllerIndex)
{
#ifdef BO1_DEMON
    DWControllerData *dwControllerData; // [esp+0h] [ebp-4h]

    dwControllerData = &g_dwControllerData[localControllerIndex];
    dwControllerData->logOnStatus = DW_LOGON_ERROR;
    dwControllerData->forceLogOn = 0;
    dwControllerData->dwAuthTimer = 0;
    dwControllerData->authGetHostByName = 0;
    dwControllerData->lsgGetHostByName = 0;
    dwControllerData->ticketSize = 0;
    dwControllerData->ticket = 0;
    dwControllerData->bCachedCookie = 0;
    dwControllerData->logOnAttempts = 0;
    dwControllerData->couldNotConnectToLSP = 0;
#endif
}

void __cdecl DWDisconnectLobby(bdLobbyService *lobby)
{
#ifdef BO1_DEMON
    if ( lobby )
    {
        Live_ClearDWOverlappedTasks();
        Session_ClearDWOverlappedTasks();
        Demo_StopStreaming();
        MatchRecorder_ClearDWOverlappedTasks();
        bdLobbyService::disconnect(lobby);
    }
#endif
}

void __cdecl dw_disconnect(int controllerIndex)
{
#ifdef BO1_DEMON
    int i; // [esp+0h] [ebp-8h]
    bdLobbyService *lobby; // [esp+4h] [ebp-4h]

    lobby = dwGetLobby(controllerIndex);
    g_numAccounts = 0;
    DWDisconnectLobby(lobby);
    Dvar_SetBoolByName("dw_active", 0);
    Dvar_SetBoolByName("dw_loggedin", 0);
    for ( i = 5; i--; Dvar_SetString((dvar_s *)dw_usernames[i], "") )
        ;
    Dvar_SetInt((dvar_s *)dw_numaccounts, -1);
#endif
}

int __cdecl dwGetLogOnStatus(int localControllerIndex)
{
#ifdef BO1_DEMON
    DWOnlineStatus status; // [esp+4h] [ebp-4h]

    status = DW_LIVE_DISCONNECTED;
    if ( byte_3326800[84 * localControllerIndex] )
        return 2;
    switch ( dword_33267D4[21 * localControllerIndex] )
    {
        case 0:
            status = DW_LIVE_ERROR;
            break;
        case 1:
            status = DW_LIVE_DISCONNECTED;
            break;
        case 2:
        case 3:
        case 4:
        case 5:
        case 7:
        case 8:
        case 9:
        case 10:
        case 11:
        case 12:
        case 13:
        case 14:
        case 15:
        case 16:
        case 17:
            status = DW_LIVE_CONNECTING;
            break;
        case 6:
            status = DW_WAITING_FOR_USERNAME;
            break;
        case 19:
            status = DW_LIVE_CONNECTED;
            break;
        default:
            return status;
    }
    return status;
#else
    return DW_LIVE_DISCONNECTED;
#endif
}

void __cdecl dwLogOnStart(int controllerIndex)
{
#ifdef BO1_DEMON
    if ( dword_33267DC[21 * controllerIndex] <= 3 )
    {
        ++dword_33267DC[21 * controllerIndex];
        dword_33267D4[21 * controllerIndex] = 2;
        byte_3326800[84 * controllerIndex] = 0;
    }
    else
    {
        dword_33267D4[21 * controllerIndex] = 0;
        dw_disconnect(controllerIndex);
        Dvar_SetBoolByName("dw_active", 1);
    }
#endif
}

void __cdecl dwLogonSeAcquiredSteamTicket()
{
#ifdef BO1_DEMON
    dword_33267D4[0] = 5;
    s_acquiredSteamTicket = 1;
#endif
}

bool __cdecl SV_DedicatedReadCDKey(char *cdkey, int cdkeysize)
{
#ifdef BO1_DEMON
    int licensenum; // [esp+0h] [ebp-Ch]
    bool retval; // [esp+7h] [ebp-5h]
    _iobuf *fp; // [esp+8h] [ebp-4h]

    licensenum = Dvar_GetInt("sv_licensenum");
    retval = 0;
    if ( cdkeysize < 25
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\Dw\\dwLogOn_pc.cpp",
                    517,
                    0,
                    "%s",
                    "cdkeysize >= SV_DEDICATED_DWKEYSIZE") )
    {
        __debugbreak();
    }
    if ( cdkeysize >= 25 )
    {
        fp = fopen("dwkey.dat", "r");
        if ( fp )
        {
            if ( licensenum )
                fseek(fp, 26 * licensenum, 0);
            fread(cdkey, 1u, 0x19u, fp);
            cdkey[cdkeysize - 1] = 0;
            Com_DPrintf(15, "Read cdkey %s\n", cdkey);
            retval = 1;
            fclose(fp);
        }
    }
    return retval;
#else
    return false;
#endif
}

void __cdecl DW_DedicatedLogonStart(int controllerIndex)
{
#ifdef BO1_DEMON
    bdLobbyService *lobby; // [esp+0h] [ebp-4h]

    if ( g_svdedicatedauthstate == SV_DWNOTAUTHORIZED )
    {
        lobby = dwGetLobby(controllerIndex);
        DWDisconnectLobby(lobby);
    }
    dwAuthServiceCleanup();
    dword_33267D4[21 * controllerIndex] = 2;
    g_svdedicatedauthstate = SV_DWAUTHORIZING;
#endif
}

void __cdecl DW_DedicatedLogonComplete(int controllerIndex)
{
#ifdef BO1_DEMON
    unsigned intv1; // eax
    int v2; // ecx
    bdAddr v3; // [esp-10h] [ebp-74h] BYREF
    int v4; // [esp-8h] [ebp-6Ch]
    unsigned int v5; // [esp-4h] [ebp-68h]
    bdAuthService::bdStatus v6; // [esp+0h] [ebp-64h]
    __int32 v7; // [esp+8h] [ebp-5Ch]
    bdAuthService *v8; // [esp+1Ch] [ebp-48h]
    const bdAuthTicket *ticket; // [esp+20h] [ebp-44h]
    enum bdLobbyConnection::Status v10; // [esp+24h] [ebp-40h]
    bdLobbyService *v11; // [esp+28h] [ebp-3Ch]
    const bdAuthInfo *authInfo; // [esp+2Ch] [ebp-38h]
    bdAddr lsgAddr; // [esp+30h] [ebp-34h] BYREF
    bdLobbyService *lobby; // [esp+38h] [ebp-2Ch]
    int handle; // [esp+3Ch] [ebp-28h]
    bdInAddr inaddr; // [esp+40h] [ebp-24h] BYREF
    const char *lsgAddress; // [esp+44h] [ebp-20h]
    bdLobbyErrorCode authError; // [esp+4Ch] [ebp-18h]
    bdAuthService::bdStatus authStatus; // [esp+50h] [ebp-14h]
    bdAuthService *v21; // [esp+54h] [ebp-10h]
    bdAuthService *authService; // [esp+58h] [ebp-Ch]
    DWLogonStatus status; // [esp+60h] [ebp-4h]

    status = dword_33267D4[21 * controllerIndex];
    dword_33267D4[21 * controllerIndex] = 1;
    g_svdedicatedauthstate = SV_DWAUTHORIZEFAIL;
    v7 = status - 2;
    switch ( status )
    {
        case DW_DNS_NOT_RESOLVED:
            if ( dwDNSInit("cod7-pc-auth.live.demonware.net", controllerIndex) ) // lol this still resolves
            {
                dword_33267D4[21 * controllerIndex] = 3;
                g_svdedicatedauthstate = SV_DWAUTHORIZING;
                Com_Printf(16, "DNS\n");
                dw_timeout = Sys_Milliseconds();
            }
            break;
        case DW_DNS_RESOLVING:
            switch ( dwDNSPump(
                                 controllerIndex,
                                 &stru_33267F0 + 21 * controllerIndex,
                                 (bdInetAddr *)&unk_33267F4 + 21 * controllerIndex) )
            {
                case 0:
                case 3:
                case 4:
                case 5:
                case 6:
                    dword_33267D4[21 * controllerIndex] = 1;
                    break;
                case 1:
                    if ( (int)(Sys_Milliseconds() - dw_timeout) >= 5000 )
                    {
                        Com_Printf(0, "giving up on DNS resolution!\n");
                        dword_33267D4[21 * controllerIndex] = 1;
                    }
                    else
                    {
                        dword_33267D4[21 * controllerIndex] = 3;
                        g_svdedicatedauthstate = SV_DWAUTHORIZING;
                        Com_Printf(0, ".");
                    }
                    break;
                case 2:
                    g_svdedicatedauthstate = SV_DWAUTHORIZING;
                    dword_33267D4[21 * controllerIndex] = 13;
                    break;
                default:
                    return;
            }
            break;
        case DW_LOGIN:
            v5 = 18301;
            v4 = 3074;
            *(unsigned int *)&v3.m_port = &stru_33267F0 + 21 * controllerIndex;
            bdInetAddr::bdInetAddr((bdInetAddr *)&v3.m_port, *(const struct bdInetAddr **)&v3.m_port);
            authService = dwGetAuthService(*(bdInetAddr *)&v3.m_port, v4, v5);
            if ( authService )
            {
                if ( SV_DedicatedReadCDKey(g_svcdkey, 25) )
                {
                    if ( bdAuthService::authorizeDedicatedHost(authService, g_svcdkey) )
                    {
                        dword_33267D4[21 * controllerIndex] = 14;
                        g_svdedicatedauthstate = SV_DWAUTHORIZING;
                        Com_Printf(16, "DW_AUTHORIZING\n");
                        dword_3326804[21 * controllerIndex] = Sys_Milliseconds();
                    }
                }
                else
                {
                    Com_Printf(15, "Failed to read dw key from file\n");
                    dword_33267D4[21 * controllerIndex] = 1;
                }
            }
            break;
        case DW_AUTHORIZING:
            v5 = 18301;
            v4 = 3074;
            *(unsigned int *)&v3.m_port = status - 2;
            bdInetAddr::bdInetAddr((bdInetAddr *)&v3.m_port, &stru_33267F0 + 21 * controllerIndex);
            v21 = dwGetAuthService(*(bdInetAddr *)&v3.m_port, v4, v5);
            if ( v21 )
            {
                dwLobbyPump(controllerIndex);
                authStatus = bdAuthService::getStatus(v21);
                v6 = authStatus;
                if ( authStatus )
                {
                    if ( v6 == BD_CONNECTING )
                    {
                        g_svdedicatedauthstate = SV_DWAUTHORIZING;
                        dword_33267D4[21 * controllerIndex] = 14;
                    }
                    else if ( v6 == BD_PENDING_REPLY )
                    {
                        dword_33267D4[21 * controllerIndex] = 14;
                        if ( (int)(Sys_Milliseconds() - dword_3326804[21 * controllerIndex]) >= 10000 )
                        {
                            Com_Printf(16, "DW_AUTHORIZING TIMED OUT\n");
                            dword_33267D4[21 * controllerIndex] = 1;
                        }
                        else
                        {
                            g_svdedicatedauthstate = SV_DWAUTHORIZING;
                            dword_33267D4[21 * controllerIndex] = 14;
                        }
                    }
                }
                else
                {
                    authError = bdAuthService::getErrorCode(v21);
                    if ( authError == BD_AUTH_NO_ERROR )
                    {
                        v1 = Sys_Milliseconds();
                        Com_Printf(16, "DW_AUTHORIZED (%d msecs)\n", v1 - dword_3326804[21 * controllerIndex]);
                        lsgAddress = dwSelectLSGAddress();
                        bdInAddr::bdInAddr(&inaddr);
                        handle = 0;
                        v5 = 0;
                        if ( bdPlatformSocket::getHostByName((char *)lsgAddress, &inaddr, 1u) )
                        {
                            bdInetAddr::set((bdInetAddr *)&unk_33267F4 + 21 * controllerIndex, inaddr.inUn.m_iaddr);
                            lobby = dwGetLobby(controllerIndex);
                            bdAddr::bdAddr(&lsgAddr, (const struct bdInetAddr *)&unk_33267F4 + 21 * controllerIndex, 0xC02u);
                            DWDisconnectLobby(lobby);
                            authInfo = bdAuthService::getAuthInfo(v21);
                            v5 = 1;
                            v4 = (int)authInfo;
                            bdAddr::bdAddr(&v3, &lsgAddr);
                            if ( bdLobbyService::connect(lobby, v3, (const struct bdAuthInfo *)v4, v5) )
                            {
                                dword_33267D4[21 * controllerIndex] = 15;
                                g_svdedicatedauthstate = SV_DWAUTHORIZING;
                                Com_Printf(16, "DW_LOBBY_CONNECTING\n");
                            }
                            bdInetAddr::~bdInetAddr(&lsgAddr.m_address);
                        }
                    }
                }
            }
            break;
        case DW_LOBBY_CONNECTING:
            v11 = dwGetLobby(controllerIndex);
            if ( v11 )
            {
                dwLobbyPump(controllerIndex);
                v10 = bdLobbyService::getStatus(v11);
                if ( v10 == BD_NOT_CONNECTED )
                {
                    v5 = 18301;
                    v4 = 3074;
                    *(unsigned int *)&v3.m_port = v2;
                    bdInetAddr::bdInetAddr((bdInetAddr *)&v3.m_port, &stru_33267F0 + 21 * controllerIndex);
                    v8 = dwGetAuthService(*(bdInetAddr *)&v3.m_port, v4, v5);
                    g_bdUserID = bdAuthUtility::getLicenseID(g_svcdkey);
                    if ( !g_bdUserID
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\Dw\\dwLogOn_pc.cpp",
                                    753,
                                    0,
                                    "%s",
                                    "0 != g_bdUserID") )
                    {
                        __debugbreak();
                    }
                    ticket = bdAuthService::getAuthTicket(v8);
                    Com_DPrintf(15, "Got license type %u\n", ticket->m_type);
                    SV_SetLicenseType(ticket->m_type);
                    g_svdedicatedauthstate = SV_DWAUTHORIZED;
                    dword_33267D4[21 * controllerIndex] = 19;
                    Com_Printf(16, "DW_LOGON_COMPLETE\n");
                    Dvar_SetBoolByName("dw_loggedin", 1);
                }
                else if ( v10 == BD_TOO_MANY_TASKS )
                {
                    g_svdedicatedauthstate = SV_DWAUTHORIZING;
                    dword_33267D4[21 * controllerIndex] = 15;
                }
            }
            break;
        default:
            return;
    }
#endif
}

const char *__cdecl dwSelectLSGAddress()
{
    return "cod7-pc-live.lsg.mmp3.demonware.net";
}

void __cdecl CL_GetSecretSauce(char *secretsauce)
{
    I_strncpyz(secretsauce, "cod42700", 9);
    secretsauce[8] = 0;
}

void __cdecl dwLogOnComplete()
{
    ;
}

void __cdecl dwDNSCleanup(int localControllerIndex)
{
#ifdef BO1_DEMON
    if ( dword_33267E8[21 * localControllerIndex] )
    {
        bdGetHostByName::quit((bdGetHostByName *)dword_33267E8[21 * localControllerIndex]);
        if ( dword_33267E8[21 * localControllerIndex] )
            (**(void (__thiscall ***)(int, int))dword_33267E8[21 * localControllerIndex])(
                dword_33267E8[21 * localControllerIndex],
                1);
        dword_33267E8[21 * localControllerIndex] = 0;
    }
    if ( dword_33267EC[21 * localControllerIndex] )
    {
        bdGetHostByName::quit((bdGetHostByName *)dword_33267EC[21 * localControllerIndex]);
        if ( dword_33267EC[21 * localControllerIndex] )
            (**(void (__thiscall ***)(int, int))dword_33267EC[21 * localControllerIndex])(
                dword_33267EC[21 * localControllerIndex],
                1);
        dword_33267EC[21 * localControllerIndex] = 0;
    }
#endif
}

void __cdecl dwLobbyPump(int localControllerIndex)
{
#ifdef BO1_DEMON
    enum bdLobbyConnection::Status status; // [esp+0h] [ebp-10h]
    bdPooledStorage *pooledStorage; // [esp+4h] [ebp-Ch]
    bdContentStreaming *contentStreaming; // [esp+8h] [ebp-8h]
    bdLobbyService *lobby; // [esp+Ch] [ebp-4h]

    if ( !Sys_IsMainThread()
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\Dw\\dwLogOn_pc.cpp", 1506, 0, "%s", "Sys_IsMainThread()") )
    {
        __debugbreak();
    }
    if ( localControllerIndex
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\Dw\\dwLogOn_pc.cpp",
                    1508,
                    0,
                    "%s",
                    "0 == localControllerIndex") )
    {
        __debugbreak();
    }
    lobby = dwGetLobby(localControllerIndex);
    if ( lobby )
    {
        dwEnterDeferredCritsec();
        bdLobbyService::pump(lobby);
        dwLeaveDeferredCritsec();
        dwUpdateMessaging(localControllerIndex);
        contentStreaming = bdLobbyService::getContentStreaming(lobby);
        if ( contentStreaming )
            bdContentStreamingBase::pump(contentStreaming);
        pooledStorage = bdLobbyService::getPooledStorage(lobby);
        if ( pooledStorage )
        {
            bdContentStreamingBase::pump(pooledStorage);
            bdContentStreamingBase::enableProgressMeter(pooledStorage, 1);
        }
        status = bdLobbyService::getStatus(lobby);
        if ( status == BD_NOT_CONNECTED )
        {
            dword_33267D4[21 * localControllerIndex] = 19;
            if ( Dvar_GetBool("dw_connectafterlogin") )
            {
                Dvar_SetBoolByName("dw_connectafterlogin", 0);
                CL_ConnectHackDW();
            }
        }
        if ( status != BD_NOT_CONNECTED )
            dword_33267D4[21 * localControllerIndex] = 1;
    }
#endif
}

void __cdecl r_PumpDemonware()
{
#ifdef BO1_DEMON
    if ( (_S1_5 & 1) == 0 )
    {
        _S1_5 |= 1u;
        lastPumpMsec = Sys_Milliseconds();
    }
    if ( (int)(Sys_Milliseconds() - lastPumpMsec) > 50 )
    {
        if ( dwGetLogOnStatus(0) == 4 )
        {
            dwNetPump();
            dwLobbyPump(0);
        }
        lastPumpMsec = Sys_Milliseconds();
    }
#endif
}

bool __cdecl dwDNSInit(char *authAddr, int controllerIndex)
{
#ifdef BO1_DEMON
    struct bdGetHostByNameConfig *HostByNameConfig; // eax
    bdGetHostByName *HostByName; // [esp+4h] [ebp-10h]
    bdGetHostByNameConfig v5; // [esp+8h] [ebp-Ch] BYREF
    bdGetHostByName *v6; // [esp+Ch] [ebp-8h]
    bool ok; // [esp+13h] [ebp-1h]

    ok = 0;
    if ( !dword_33267E8[21 * controllerIndex] )
    {
        v6 = (bdGetHostByName *)bdMemory::allocate(0x38u);
        if ( v6 )
            HostByName = bdGetHostByName::bdGetHostByName(v6);
        else
            HostByName = 0;
        dword_33267E8[21 * controllerIndex] = (int)HostByName;
        ok = dword_33267E8[21 * controllerIndex] != 0;
    }
    if ( ok )
    {
        HostByNameConfig = bdGetHostByNameConfig::bdGetHostByNameConfig(&v5);
        ok = bdGetHostByName::start(
                     (bdGetHostByName *)dword_33267E8[21 * controllerIndex],
                     authAddr,
                     LODWORD(HostByNameConfig->m_timeout));
    }
    if ( !ok )
        dwDNSCleanup(controllerIndex);
    return ok;
#else
    return false;
#endif
}

int __cdecl dwDNSPump(int localControllerIndex, bdInetAddr *authAddr, bdInetAddr *lsgAddr)
{
#ifdef BO1_DEMON
    unsigned int TotalNumResults; // eax
    unsigned int v4; // eax
    bdInAddr v6; // [esp+0h] [ebp-1Ch] BYREF
    bool v7; // [esp+7h] [ebp-15h]
    bdInAddr inAddr; // [esp+8h] [ebp-14h] BYREF
    bool addrResolved; // [esp+Fh] [ebp-Dh]
    bdGetHostByName::bdStatus getAuthHostByNameStatus; // [esp+10h] [ebp-Ch]
    bdGetHostByName::bdStatus getLSGHostByNameStatus; // [esp+14h] [ebp-8h]
    bdGetHostByName::bdStatus retVal; // [esp+18h] [ebp-4h]

    retVal = BD_LOOKUP_FAILED;
    getAuthHostByNameStatus = BD_LOOKUP_FAILED;
    getLSGHostByNameStatus = BD_LOOKUP_FAILED;
    if ( dword_33267E8[21 * localControllerIndex] )
    {
        dwLobbyPump(localControllerIndex);
        getAuthHostByNameStatus = bdPendingBufferTransfer::getAvail((bdPendingBufferTransfer *)dword_33267E8[21 * localControllerIndex]);
        if ( getAuthHostByNameStatus == BD_LOOKUP_SUCCEEDED )
        {
            TotalNumResults = bdRemoteTask::getTotalNumResults((bdRemoteTask *)dword_33267E8[21 * localControllerIndex]);
            addrResolved = TotalNumResults != 0;
            if ( TotalNumResults )
            {
                bdGetHostByName::getAddressAt((bdGetHostByName *)dword_33267E8[21 * localControllerIndex], &inAddr, 0);
                bdInetAddr::set(authAddr, inAddr.inUn.m_iaddr);
            }
            else
            {
                getAuthHostByNameStatus = BD_LOOKUP_FAILED;
            }
        }
    }
    if ( dword_33267EC[21 * localControllerIndex] )
    {
        getLSGHostByNameStatus = bdPendingBufferTransfer::getAvail((bdPendingBufferTransfer *)dword_33267EC[21 * localControllerIndex]);
        if ( getLSGHostByNameStatus == BD_LOOKUP_SUCCEEDED )
        {
            v4 = bdRemoteTask::getTotalNumResults((bdRemoteTask *)dword_33267EC[21 * localControllerIndex]);
            v7 = v4 != 0;
            if ( v4 )
            {
                bdGetHostByName::getAddressAt((bdGetHostByName *)dword_33267EC[21 * localControllerIndex], &v6, 0);
                bdInetAddr::set(lsgAddr, v6.inUn.m_iaddr);
            }
            else
            {
                getLSGHostByNameStatus = BD_LOOKUP_FAILED;
            }
        }
    }
    if ( getAuthHostByNameStatus == BD_LOOKUP_FAILED )
        goto LABEL_16;
    if ( getAuthHostByNameStatus != BD_LOOKUP_PENDING )
    {
        if ( getAuthHostByNameStatus == BD_LOOKUP_SUCCEEDED )
        {
            dwDNSCleanup(localControllerIndex);
            return 2;
        }
LABEL_16:
        dwDNSCleanup(localControllerIndex);
        return 3;
    }
    return 1;
#else
    return 1;
#endif
}

void __cdecl DWDedicatedLogon()
{
#ifdef BO1_DEMON
    SV_ResetDWState();
    Dvar_SetBoolByName("r_gfxopt_water_simulation", 0);
    dwNetStart(1);
    while ( g_dwNetStatus == DW_NET_STARTING_ONLINE )
        dwNetPump();
    DW_DedicatedLogonStart(0);
    while ( g_svdedicatedauthstate == SV_DWAUTHORIZING )
    {
        Sys_Milliseconds();
        DW_DedicatedLogonComplete(0);
    }
    if ( g_svdedicatedauthstate != SV_DWAUTHORIZED )
        Com_Error(ERR_DROP, "Dedicated server authentication failure.\n");
    Com_Printf(0, "should be logged in ok\n");
    LiveStorage_SyncTime(0);
    LiveStorage_FetchRequiredFiles(0);
#endif
}

void __cdecl DWDedicatedLobbyPump()
{
#ifdef BO1_DEMON
    dwEnterDeferredCritsec();
    dwNetPump();
    if ( g_svdedicatedauthstate == SV_DWAUTHORIZED )
        dwLobbyPump(0);
    LiveStorage_FetchRequiredFiles(0);
    LiveStorage_HandlePublisherStorageTasks();
    dwLeaveDeferredCritsec();
    if ( com_sv_running && com_sv_running->current.enabled )
    {
        SV_MasterHeartbeat(0);
        LiveCounter_Update(0);
    }
    SV_AP_Frame();
#endif
}

bool Com_CanWriteLeaderboards(void *thisptr)
{
#ifdef BO1_DEMON
    bdInetAddr v2; // [esp-Ch] [ebp-28h] BYREF
    int v3; // [esp-8h] [ebp-24h]
    unsigned int v4; // [esp-4h] [ebp-20h]
    bool v6; // [esp+4h] [ebp-18h]
    const bdAuthTicket *authTicket; // [esp+Ch] [ebp-10h]
    int controllerIndex; // [esp+10h] [ebp-Ch]
    bdAuthService *authService; // [esp+14h] [ebp-8h]
    bool retval; // [esp+1Bh] [ebp-1h]

    controllerIndex = 0;
    v4 = 18301;
    v3 = 3074;
    v2 = (bdInetAddr)this;
    bdInetAddr::bdInetAddr(&v2, &stru_33267F0);
    authService = dwGetAuthService(v2, v3, v4);
    retval = 0;
    if ( authService )
    {
        if ( g_svdedicatedauthstate == SV_DWAUTHORIZED )
        {
            authTicket = bdAuthService::getAuthTicket(authService);
            if ( authTicket )
            {
                v6 = isOfficialRentedServer(authTicket->m_type) || isOfficialServer(authTicket->m_type);
                retval = v6;
            }
        }
    }
    return retval && Pregame_GetState() != PREGAME_INPROGRESS;
#else
    return false;
#endif
}

bool __cdecl isOfficialServer(int licensetype)
{
    return licensetype == 4;
}

bool __cdecl isOfficialRentedServer(int licensetype)
{
    return licensetype == 2;
}

#endif