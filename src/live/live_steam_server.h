#pragma once

#include <steam/steam_api.h>
#include <steam/isteamgameserver.h>

#include <qcommon/net_chan_mp.h>

struct LiveSteamServer // sizeof=0x88
{
    unsigned int currentMapDlcAppID;
    unsigned int serverIP;
    unsigned int lastRunTime;
    bool serverActive;
    // padding byte
    // padding byte
    // padding byte
    CCallback<LiveSteamServer,SteamServersConnected_t,1> m_CallbackSteamServersConnected;
    CCallback<LiveSteamServer,SteamServersDisconnected_t,1> m_CallbackSteamServersDisconnected;
    CCallback<LiveSteamServer,GSPolicyResponse_t,1> m_CallbackPolicyResponse;
    CCallback<LiveSteamServer,GSClientApprove_t,1> m_CallbackGSClientApprove;
    CCallback<LiveSteamServer,GSClientDeny_t,1> m_CallbackGSClientDeny;
    CCallback<LiveSteamServer,GSClientKick_t,1> m_CallbackGSClientKick;

    LiveSteamServer();
    ~LiveSteamServer();

    void RunFrame();
    void OnSteamServersConnected(SteamServersConnected_t *pLogonSuccess);
    void OnSteamServersDisconnected(SteamServersDisconnected_t *pLoggedOff);
    void OnPolicyResponse(GSPolicyResponse_t *pPolicyResponse);
    void OnGSClientApprove(GSClientApprove_t *pGSClientApprove);
    void OnGSClientDeny(GSClientDeny_t *pGSClientDeny);
    void OnGSClientKick(GSClientKick_t *pGSClientKick);

};





void __cdecl KickClientFromSteamGameServer(CSteamID clientID, EDenyReason reason);

void __cdecl LiveSteam_Server_Init();
void __cdecl LiveSteam_Server_Shutdown();
void __cdecl LiveSteam_Server_Frame();
void __cdecl LiveSteam_Server_RunCallbacks();
void __cdecl LiveSteam_Server_ClientSteamAuthentication(
                unsigned __int64 clientSteamID,
                netadr_t netAddr,
                void *authBlob,
                unsigned int authBlobSize);
void __cdecl LiveSteam_Server_ClientSteamDisconnect(unsigned __int64 clientSteamID);
