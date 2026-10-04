#include "live_steam_server.h"
#include <win32/win_shared.h>
#include <qcommon/common.h>
#include <server_mp/sv_main_mp.h>
#include <server_mp/sv_main_pc_mp.h>
#include <qcommon/cmd.h>
#include <client_mp/sv_client_mp.h>
#include <steam/steam_gameserver.h>
#include "live_steam.h"

LiveSteamServer *g_liveSteamServer;

LiveSteamServer::LiveSteamServer()
    : currentMapDlcAppID(0)
    , serverIP(0)
    , lastRunTime(0)
    , serverActive(false)
    , m_CallbackSteamServersConnected(this, &LiveSteamServer::OnSteamServersConnected)
    , m_CallbackSteamServersDisconnected(this, &LiveSteamServer::OnSteamServersDisconnected)
    , m_CallbackPolicyResponse(this, &LiveSteamServer::OnPolicyResponse)
    , m_CallbackGSClientApprove(this, &LiveSteamServer::OnGSClientApprove)
    , m_CallbackGSClientDeny(this, &LiveSteamServer::OnGSClientDeny)
    , m_CallbackGSClientKick(this, &LiveSteamServer::OnGSClientKick)
{
}

LiveSteamServer::~LiveSteamServer()
{

}

void LiveSteamServer::RunFrame()
{
    int v4; // [esp+0h] [ebp-30h]
    unsigned __int64 serverSteamID; // [esp+8h] [ebp-28h]
    _BYTE v7[8]; // [esp+14h] [ebp-1Ch] BYREF
    client_t *client; // [esp+1Ch] [ebp-14h]
    unsigned int RUN_INTERVAL; // [esp+20h] [ebp-10h]
    unsigned int currentTime; // [esp+24h] [ebp-Ch]
    int i; // [esp+28h] [ebp-8h]
    int MAX_STEAM_AUTH_FAIL_COUNT; // [esp+2Ch] [ebp-4h]

    MAX_STEAM_AUTH_FAIL_COUNT = 4;
    RUN_INTERVAL = 10000;
    if ( this->serverActive && SteamGameServer() ) // no-steam: serverActive is only set by a Steam callback; never touch a NULL game server
    {
        currentTime = Sys_Milliseconds();
        if ( currentTime - this->lastRunTime >= 0x2710 )
        {
            this->lastRunTime = currentTime;
            for ( i = 0; i < com_maxclients->current.integer; ++i )
            {
                client = &svs.clients[i];
                if ( client->header.state >= CS_ACTIVE && !client->bIsTestClient && !client->bIsDemoClient && !client->steamAuthorized )
                {
                    if ( ++client->steamAuthFailCount == 4 )
                    {
                        Com_PrintError(15, va("Kicking '%s' <%llx> in runframe, unable to Steam authorize\n", client->name, client->steamID));
                        SV_SysLog_LogMessage(0, va("Kicking '%s' <%llx> in runframe, unable to Steam authorize\n", client->name, client->steamID));
                        Cbuf_AddText(0, va("clientkick %d PLATFORM_STEAM_CONNECT_FAIL\n", i));
                        client->steamAuthFailCount = 0;
                    }
                    else
                    {
                        Com_DPrintf(
                            0,
                            "Sending steam auth request to client <%llx>, fail count is now %i\n",
                            client->steamID,
                            client->steamAuthFailCount);
                        //v4 = SteamGameServer();
                        //serverSteamID = *(_QWORD *)(*(int (__thiscall **)(int, _BYTE *))(*(unsigned int *)v4 + 16))(v4, v7);
                        SV_SteamAuthClientRequest(client->header.netchan.remoteAddress, SteamGameServer()->GetSteamID().ConvertToUint64());
                    }
                }
            }
        }
    }
}

void LiveSteamServer::OnSteamServersConnected(SteamServersConnected_t *pLogonSuccess)
{
#ifdef BO1_LIVE_STUBS
    bdAddr *SecurityKey; // eax
    bdAddr *Address; // eax
    bdAddr *v4; // eax
    bdReference<bdCommonAddr> localAddr; // [esp+1Ch] [ebp-8h] BYREF
    unsigned int n; // [esp+20h] [ebp-4h] BYREF

    Com_Printf(23, "STEAM: SteamServer connected to Steam successfully\n");
    dwGetLocalCommonAddr(&localAddr);
    SecurityKey = (bdAddr *)bdGameInfo::getSecurityKey((bdGameInfo *)localAddr.m_ptr);
    Address = bdAddr::getAddress(SecurityKey);
    bdInetAddr::serialize(&Address->m_address, (unsigned __int8 *)&this->serverIP, 4u, 0, &n);
    v4 = (bdAddr *)bdGameInfo::getSecurityKey((bdGameInfo *)localAddr.m_ptr);
    bdAddr::getPort(v4);
    bdReference<bdRemoteTask>::~bdReference<bdRemoteTask>(&localAddr);
#endif
}

void LiveSteamServer::OnSteamServersDisconnected(SteamServersDisconnected_t *pLoggedOff)
{
    int i; // [esp+4h] [ebp-8h]
    client_t *client; // [esp+8h] [ebp-4h]

    this->serverActive = 0;
    Com_PrintError(15, "STEAM: SteamServer got logged out of Steam\n");
    SV_SysLog_LogMessage(0, "STEAM: SteamServer got logged out of Steam\n");
    for ( i = 0; i < com_maxclients->current.integer; ++i )
    {
        client = &svs.clients[i];
        if ( client->header.state >= CS_ACTIVE && !client->bIsTestClient && !client->bIsDemoClient && !client->steamAuthorized )
            client->steamAuthFailCount = 0;
    }
}

void __thiscall LiveSteamServer::OnPolicyResponse(GSPolicyResponse_t *pPolicyResponse)
{
    int v2; // eax

    this->serverActive = 1;
    this->lastRunTime = 0;
    //v2 = _SteamGameServer();
    //if ( (*(unsigned __int8 (__thiscall **)(int, int))(*(unsigned int *)v2 + 12))(v2, v2) )
    if (SteamGameServer()->BSecure())
        Com_Printf(23, "STEAM: SteamServer is VAC Secure!\n");
    else
        Com_Printf(23, "STEAM: SteamServer is not VAC Secure!\n");
}

void __thiscall LiveSteamServer::OnGSClientApprove(GSClientApprove_t *pGSClientApprove)
{
    const char *v2; // eax
    char *v3; // eax
    const char *v4; // eax
    const char *v5; // eax
    int v6; // [esp+4h] [ebp-30h]
    int v7; // [esp+8h] [ebp-2Ch]
    int v8; // [esp+Ch] [ebp-28h]
    client_t *client; // [esp+2Ch] [ebp-8h]
    int i; // [esp+30h] [ebp-4h]

    if ( SteamFriends() )
    {
        //v8 = _SteamFriends();
        //v2 = (const char *)(*(int (__thiscall **)(int, unsigned int, unsigned int))(*(unsigned int *)v8 + 28))(
        //                                         v8,
        //                                         *(unsigned int *)&pGSClientApprove->m_SteamID.m_steamid.m_comp,
        //                                         *((unsigned int *)&pGSClientApprove->m_SteamID.m_steamid.m_comp + 1));
        Com_PrintWarning(15, "STEAM: Approved client with SteamPersonaName: %s\n", SteamFriends()->GetFriendPersonaName(pGSClientApprove->m_SteamID));
    }
    else
    {
        Com_PrintWarning(
            15,
            "STEAM: Approved client with SteamID: %I64d\n",
            //pGSClientApprove->m_SteamID.m_steamid.m_unAll64Bits);
            pGSClientApprove->m_SteamID.ConvertToUint64());
    }
    for ( i = 0; i < com_maxclients->current.integer; ++i )
    {
        client = &svs.clients[i];
        if ( client->header.state >= CS_CONNECTED )
        {
            if ( this->currentMapDlcAppID )
            {
                //v7 = _SteamGameServer();
                //if ( (*(int (__thiscall **)(int, unsigned int, unsigned int, unsigned int))(*(unsigned int *)v7 + 72))(
                //             v7,
                //             *(unsigned int *)&pGSClientApprove->m_SteamID.m_steamid.m_comp,
                //             *((unsigned int *)&pGSClientApprove->m_SteamID.m_steamid.m_comp + 1),
                //             this->currentMapDlcAppID) )
                if (SteamGameServer()->UserHasLicenseForApp(pGSClientApprove->m_SteamID, this->currentMapDlcAppID))
                {
                    v3 = va("kicking client %s, no license\n", client->name);
                    Com_PrintError(15, v3);
                    v4 = va("kicking client %s, no license\n", client->name);
                    SV_SysLog_LogMessage(0, v4);
                    KickClientFromSteamGameServer(pGSClientApprove->m_SteamID, k_EDenyNoLicense);
                    return;
                }
                if ( SteamFriends() )
                {
                    //v6 = _SteamFriends();
                    //v5 = (const char *)(*(int (__thiscall **)(int, unsigned int, unsigned int))(*(unsigned int *)v6 + 28))(
                    //                                         v6,
                    //                                         *(unsigned int *)&pGSClientApprove->m_SteamID.m_steamid.m_comp,
                    //                                         *((unsigned int *)&pGSClientApprove->m_SteamID.m_steamid.m_comp + 1));
                    Com_PrintWarning(15, "STEAM: Approved client for DLC, SteamPersonaName: %s\n", SteamFriends()->GetFriendPersonaName(pGSClientApprove->m_SteamID));
                }
                else
                {
                    Com_PrintWarning(
                        15,
                        "STEAM: Approved client DLC, SteamID: %I64d\n",
                        //pGSClientApprove->m_SteamID.m_steamid.m_unAll64Bits);
                        pGSClientApprove->m_SteamID.ConvertToUint64());
                }
            }
            //if ( LODWORD(client->steamID) == pGSClientApprove->m_SteamID.m_steamid.m_comp
            //    && HIDWORD(client->steamID) == *((unsigned int *)&pGSClientApprove->m_SteamID.m_steamid.m_comp + 1) )
            if (client->steamID == pGSClientApprove->m_SteamID.ConvertToUint64())
            {
                client->steamAuthorized = 1;
                return;
            }
        }
    }
}

void __cdecl KickClientFromSteamGameServer(CSteamID clientID, EDenyReason reason)
{
#if 0
    char *v2; // eax
    const char *v3; // eax
    const char *v4; // eax
    const char *v5; // eax
    const char *v6; // eax
    int v7; // [esp+4h] [ebp-10h]
    client_t *client; // [esp+Ch] [ebp-8h]
    int i; // [esp+10h] [ebp-4h]

    if ( reason != k_EDenySteamValidationStalled )
    {
        for ( i = 0; i < com_maxclients->current.integer; ++i )
        {
            client = &svs.clients[i];
            if ( client->header.state >= CS_CONNECTED && client->steamID == clientID.ConvertToUint64() )
            {
                //v7 = _SteamGameServer();
                //(*(void (__thiscall **)(int, unsigned int, unsigned int))(*(unsigned int *)v7 + 28))(
                //    v7,
                //    *(unsigned int *)&clientID.m_steamid.m_comp,
                //    *((unsigned int *)&clientID.m_steamid.m_comp + 1));
                SteamGameServer()->SendUserDisconnect(clientID);
                v2 = va(
                             "STEAM: kicking '%s' <%llx> for reason: %d in KickClientFromSteamGameServer\n",
                             client->name,
                             client->steamID,
                             reason);
                Com_PrintError(15, v2);
                v3 = va(
                             "STEAM: kicking '%s' <%llx> for reason: %d in KickClientFromSteamGameServer\n",
                             client->name,
                             client->steamID,
                             reason);
                SV_SysLog_LogMessage(0, v3);
                switch ( reason )
                {
                    case k_EDenyNotLoggedOn:
                    case k_EDenyNoLicense:
                    case k_EDenyLoggedInElseWhere:
                        v4 = va("clientkick %d PLATFORM_STEAM_AUTH_DENIED\n", i);
                        Cbuf_AddText(0, v4);
                        break;
                    case k_EDenyCheater:
                        v5 = va("tempBanClient %d PLATFORM_STEAM_KICK_CHEAT\n", i);
                        Cbuf_AddText(0, v5);
                        break;
                    default:
                        v6 = va("clientkick %d PLATFORM_STEAM_CONNECT_FAIL\n", i);
                        Cbuf_AddText(0, v6);
                        break;
                }
                return;
            }
        }
    }
#endif
}

void __thiscall LiveSteamServer::OnGSClientDeny(GSClientDeny_t *pGSClientDeny)
{
    char *v2; // eax
    const char *v3; // eax

    v2 = va(
                 "STEAM: Denying <%llx> for reason: %d\n",
                 pGSClientDeny->m_SteamID.ConvertToUint64(),
                 pGSClientDeny->m_eDenyReason);
    Com_PrintError(15, v2);
    v3 = va(
                 "STEAM: Denying <%llx> for reason: %d\n",
                 pGSClientDeny->m_SteamID.ConvertToUint64(),
                 pGSClientDeny->m_eDenyReason);
    SV_SysLog_LogMessage(0, v3);
    KickClientFromSteamGameServer(pGSClientDeny->m_SteamID, pGSClientDeny->m_eDenyReason);
}

void __thiscall LiveSteamServer::OnGSClientKick(GSClientKick_t *pGSClientKick)
{
    Com_PrintError(
        0,
        "Steam kicking <%llx> for reason: %d\n",
        pGSClientKick->m_SteamID.ConvertToUint64(),
        pGSClientKick->m_eDenyReason);
    KickClientFromSteamGameServer(pGSClientKick->m_SteamID, pGSClientKick->m_eDenyReason);
}

void __cdecl LiveSteam_Server_Init()
{
#if 0
    LiveSteamServer *v0; // [esp+0h] [ebp-18h]
    void (__thiscall ***v1)(unsigned int, unsigned int); // [esp+4h] [ebp-14h]
    LiveSteamServer *v2; // [esp+8h] [ebp-10h]

    if ( LiveSteam_IsInitialized() && !g_liveSteamServer )
    {
        if ( !(unsigned __int8)SteamGameServer_Init(0, 8766, 27015, 0, 27016, eServerModeAuthenticationAndSecure, "", "1.0.0.0") )
            Com_PrintError(0, "ERROR: STEAM: SteamGameServer_Init call failed\n");
        if ( SteamMasterServerUpdater() )
        {
            //v1 = (void (__thiscall ***)(unsigned int, unsigned int))_SteamMasterServerUpdater();
            //(**v1)(v1, 0);
            SteamMasterServerUpdater()->SetActive(false);
        }
        //v2 = (LiveSteamServer *)operator new(0x88u);
        //if ( v2 )
        //    v0 = LiveSteamServer::LiveSteamServer(v2);
        //else
        //    v0 = 0;
        //g_liveSteamServer = v0;

        g_liveSteamServer = new LiveSteamServer();
    }
#endif
}

void __cdecl LiveSteam_Server_Shutdown()
{
#if 0
    ISteamGameServer *v0; // [esp+4h] [ebp-10h]
    ISteamMasterServerUpdater *v1; // [esp+8h] [ebp-Ch]
    LiveSteamServer *v2; // [esp+10h] [ebp-4h]

    if (g_liveSteamServer)
    {
        v1 = SteamMasterServerUpdater();
        v1->SetActive(0);
        v0 = SteamGameServer();
        v0->LogOff();
        SteamGameServer_Shutdown();
        v2 = g_liveSteamServer;
        if (g_liveSteamServer)
        {
            delete g_liveSteamServer;
            //LiveSteamServer::~LiveSteamServer(g_liveSteamServer);
            //operator delete(v2);
        }
        g_liveSteamServer = 0;
    }
#endif
}

void __cdecl LiveSteam_Server_Frame()
{
    if (g_liveSteamServer)
    {
        //LiveSteamServer::RunFrame(g_liveSteamServer);
        g_liveSteamServer->RunFrame();
    }
}

void __cdecl LiveSteam_Server_RunCallbacks()
{
    if (LiveSteam_IsInitialized()) // no-steam: no Steam client, no callbacks (G_RunFrame calls this every frame)
        SteamGameServer_RunCallbacks();
}

void __cdecl LiveSteam_Server_ClientSteamAuthentication(
                unsigned __int64 clientSteamID,
                netadr_t netAddr,
                void *authBlob,
                unsigned int authBlobSize)
{
#if 0
    int v4; // [esp+0h] [ebp-18h]
    unsigned int clientIP; // [esp+8h] [ebp-10h]
    CSteamID steamID; // [esp+Ch] [ebp-Ch] BYREF

    //CSteamID::CSteamID(&steamID);
    if ( g_liveSteamServer )
    {
        if ( netAddr.type == NA_LOOPBACK )
        {
            clientIP = g_liveSteamServer->serverIP;
        }
        else
        {
            if ( netAddr.type != NA_IP )
                return;
            clientIP = _byteswap_ulong(*(unsigned int *)netAddr.ip);
        }
        //v4 = _SteamGameServer();
        //if ( (*(unsigned __int8 (__thiscall **)(int, unsigned int, void *, unsigned int, CSteamID *))(*(unsigned int *)v4 + 20))(
        //             v4,
        //             clientIP,
        //             authBlob,
        //             authBlobSize,
        //             &steamID) )
        if (SteamGameServer()->SendUserConnectAndAuthenticate(clientIP, authBlob, authBlobSize, &steamID))
        {
            iassert(clientSteamID == steamID.ConvertToUint64());
            //if ( clientSteamID != steamID.m_steamid.m_comp
            //    && !Assert_MyHandler(
            //                "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_steam_server.cpp",
            //                431,
            //                0,
            //                "%s",
            //                "clientSteamID == steamID.ConvertToUint64()") )
            //{
            //    __debugbreak();
            //}
        }
        else
        {
            Com_PrintError(
                15,
                "ERROR: STEAM: Dev print - Unable to connect user %llu from IP: %u.%u.%u.%u:%u to steam server\n",
                clientSteamID,
                netAddr.ip[0],
                netAddr.ip[1],
                netAddr.ip[2],
                netAddr.ip[3],
                netAddr.port);
        }
    }
#endif
}

void __cdecl LiveSteam_Server_ClientSteamDisconnect(unsigned __int64 clientSteamID)
{
#if 0
    int v1; // eax
    CSteamID clientID; // [esp+4h] [ebp-8h] BYREF

    if ( g_liveSteamServer )
    {
        //clientID.m_steamid.m_comp = (CSteamID::SteamID_t::SteamIDComponent_t)clientSteamID;
        clientID.SetFromUint64(clientSteamID);
        //if ( CSteamID::IsValid(&clientID) )
        if (clientID.IsValid())
        {
            //v1 = _SteamGameServer();
            //(*(void (__thiscall **)(int, unsigned int, unsigned int))(*(unsigned int *)v1 + 28))(
            //    v1,
            //    *(unsigned int *)&clientID.m_steamid.m_comp,
            //    *((unsigned int *)&clientID.m_steamid.m_comp + 1));

            SteamGameServer()->SendUserDisconnect(clientID);
        }
    }
#endif
}

