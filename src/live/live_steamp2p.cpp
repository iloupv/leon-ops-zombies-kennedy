#include "live_steamp2p.h"
#include <qcommon/common.h>

LiveSteamP2P::LiveSteamP2P()
    : m_OnSessionRequest(this, &LiveSteamP2P::OnSessionRequest)
{
}

void __thiscall LiveSteamP2P::SendPacketToSteamID(
                CSteamID remoteID,
                const void *data,
                unsigned int datasize)
{
    //void (__thiscall ***v4)(unsigned int, unsigned int, unsigned int, const void *, unsigned int, int, unsigned int); // [esp+0h] [ebp-8h]
    //
    //v4 = (void (__thiscall ***)(unsigned int, unsigned int, unsigned int, const void *, unsigned int, int, unsigned int))_SteamNetworking();
    //(**v4)(v4, *(unsigned int *)&remoteID.m_steamid.m_comp, *((unsigned int *)&remoteID.m_steamid.m_comp + 1), data, datasize, 2, 0);

    if (!SteamNetworking()) // no-steam: no Steam client, no P2P
        return;
    SteamNetworking()->SendP2PPacket(remoteID, data, datasize, k_EP2PSendReliable, 0);
}

int __thiscall LiveSteamP2P::IsDataAvailable(unsigned int *messageSize)
{
    //int v3; // [esp+0h] [ebp-8h]
    //
    //v3 = _SteamNetworking();
    //return (*(int (__thiscall **)(int, unsigned int *, unsigned int))(*(unsigned int *)v3 + 4))(v3, messageSize, 0);

    if (!SteamNetworking()) // no-steam: no Steam client, no P2P
        return 0;
    return SteamNetworking()->IsP2PPacketAvailable(messageSize, 0);
}

int __thiscall LiveSteamP2P::ReadPacket(
                void *buf,
                unsigned int bufsize,
                unsigned int *msgsize,
                CSteamID *remoteID)
{
    //int v6; // [esp+0h] [ebp-8h]
    //
    //v6 = _SteamNetworking();
    //return (*(int (__thiscall **)(int, void *, unsigned int, unsigned int *, CSteamID *, unsigned int))(*(unsigned int *)v6 + 8))(
    //                 v6,
    //                 buf,
    //                 bufsize,
    //                 msgsize,
    //                 remoteID,
    //                 0);

    if (!SteamNetworking()) // no-steam: no Steam client, no P2P
        return 0;
    return SteamNetworking()->IsP2PPacketAvailable(msgsize, 0);
}

void __thiscall LiveSteamP2P::OnSessionRequest(P2PSessionRequest_t *sessionRequest)
{
    int v2; // [esp+0h] [ebp-8h]

    //Com_DPrintf(0, "Received session request from %llu\n", sessionRequest->m_steamIDRemote.m_steamid.m_unAll64Bits);
    Com_DPrintf(0, "Received session request from %llu\n", sessionRequest->m_steamIDRemote.ConvertToUint64());
    //v2 = _SteamNetworking();
    //(*(void (__thiscall **)(int, unsigned int, unsigned int))(*(unsigned int *)v2 + 12))(
    //    v2,
    //    *(unsigned int *)&sessionRequest->m_steamIDRemote.m_steamid.m_comp,
    //    *((unsigned int *)&sessionRequest->m_steamIDRemote.m_steamid.m_comp + 1));

    if (!SteamNetworking()) // no-steam: no Steam client, no P2P
        return;
    SteamNetworking()->AcceptP2PSessionWithUser(sessionRequest->m_steamIDRemote);
}