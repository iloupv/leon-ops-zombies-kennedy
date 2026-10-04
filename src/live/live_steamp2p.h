#pragma once

#include <steam/steam_api.h>

struct LiveSteamP2P // sizeof=0x14
{                                       // XREF: .data:s_SteamP2P/r
    CCallback<LiveSteamP2P,P2PSessionRequest_t,0> m_OnSessionRequest;
                                        // XREF: _dynamic_atexit_destructor_for__s_SteamP2P__+3/w
                                        // _dynamic_atexit_destructor_for__s_SteamP2P__+D/r

    LiveSteamP2P();
    void SendPacketToSteamID(CSteamID remoteID, const void *data, unsigned int datasize);
    int IsDataAvailable(unsigned int *messageSize);
    int ReadPacket(void *buf, unsigned int bufsize, unsigned int *msgsize, CSteamID *remoteID);
    void OnSessionRequest(P2PSessionRequest_t *sessionRequest);
};



