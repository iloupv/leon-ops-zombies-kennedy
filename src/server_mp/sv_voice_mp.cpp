#include "sv_voice_mp.h"
#include "sv_init_mp.h"
#include <universal/com_memory.h>
#include <qcommon/msg_mp.h>
#include <server/server.h>
#include <qcommon/common.h>
#include "sv_main_mp.h"
#include <client_mp/g_client_mp.h>


bool __cdecl SV_VoiceEnabled()
{
    return sv_voice->current.enabled;
}

void __cdecl SV_SendClientVoiceData(client_t *client)
{
    unsigned __int8 *msg_buf; // [esp+0h] [ebp-3Ch]
    LargeLocal msg_buf_large_local(0x10000); // [esp+4h] [ebp-38h] BYREF
    msg_t msg; // [esp+Ch] [ebp-30h] BYREF

    //LargeLocal::LargeLocal(&msg_buf_large_local, 0x10000);
    msg_buf = msg_buf_large_local.GetBuf();//LargeLocal::GetBuf(&msg_buf_large_local);
    if ( client->voicePacketCount < 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_voice_mp.cpp",
                    66,
                    0,
                    "%s",
                    "client->voicePacketCount >= 0") )
    {
        __debugbreak();
    }
    if ( client->header.state == CS_ACTIVE && client->voicePacketCount )
    {
        MSG_Init(&msg, msg_buf, 0x10000);
        if ( msg.cursize
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_voice_mp.cpp",
                        75,
                        0,
                        "%s",
                        "msg.cursize == 0") )
        {
            __debugbreak();
        }
        if ( msg.bit
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_voice_mp.cpp", 76, 0, "%s", "msg.bit == 0") )
        {
            __debugbreak();
        }
        MSG_WriteString(&msg, "v");
        SV_WriteVoiceDataToClient(client, &msg);
        if ( msg.overflowed )
        {
            Com_PrintWarning(15, "WARNING: voice msg overflowed for %s\n", client->name);
        }
        else
        {
            NET_OutOfBandVoiceData(NS_SERVER, client->header.netchan.remoteAddress, msg.data, msg.cursize);
            client->voicePacketCount = 0;
        }
    }
    //LargeLocal::~LargeLocal(&msg_buf_large_local);
}

void __cdecl SV_WriteVoiceDataToClient(client_t *client, msg_t *msg)
{
    int packet; // [esp+0h] [ebp-4h]

    if ( client->voicePacketCount <= 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_voice_mp.cpp",
                    30,
                    0,
                    "%s",
                    "client->voicePacketCount > 0") )
    {
        __debugbreak();
    }
    if ( client->voicePacketCount > 40
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_voice_mp.cpp",
                    31,
                    0,
                    "%s",
                    "client->voicePacketCount <= MAX_SERVER_QUEUED_VOICE_PACKETS") )
    {
        __debugbreak();
    }
    MSG_WriteByte(msg, client->voicePacketCount);
    for ( packet = 0; packet < client->voicePacketCount; ++packet )
    {
        MSG_WriteByte(msg, client->voicePackets[packet].talker);
        if ( client->voicePackets[packet].dataSize >= 0x10000
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_voice_mp.cpp",
                        40,
                        0,
                        "%s",
                        "client->voicePackets[packet].dataSize < (2<<15)") )
        {
            __debugbreak();
        }
        MSG_WriteByte(msg, client->voicePackets[packet].dataSize);
        MSG_WriteData(msg, client->voicePackets[packet].data, client->voicePackets[packet].dataSize);
    }
    if ( msg->overflowed
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_voice_mp.cpp",
                    50,
                    0,
                    "%s",
                    "!msg->overflowed") )
    {
        __debugbreak();
    }
}

bool __cdecl SV_ClientWantsVoiceData(unsigned int clientNum)
{
    if ( clientNum >= com_maxclients->current.integer
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_voice_mp.cpp",
                    101,
                    0,
                    "clientNum doesn't index com_maxclients->current.integer\n\t%i not in [0, %i)",
                    clientNum,
                    com_maxclients->current.integer) )
    {
        __debugbreak();
    }
    return svs.clients[clientNum].sendVoice;
}

bool __cdecl SV_ClientHasClientMuted(unsigned int listener, unsigned int talker)
{
    if ( listener >= com_maxclients->current.integer
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_voice_mp.cpp",
                    110,
                    0,
                    "listener doesn't index com_maxclients->current.integer\n\t%i not in [0, %i)",
                    listener,
                    com_maxclients->current.integer) )
    {
        __debugbreak();
    }
    if ( talker >= com_maxclients->current.integer
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_voice_mp.cpp",
                    111,
                    0,
                    "talker doesn't index com_maxclients->current.integer\n\t%i not in [0, %i)",
                    talker,
                    com_maxclients->current.integer) )
    {
        __debugbreak();
    }
    return svs.clients[listener].muteList[talker];
}

void __cdecl SV_QueueVoicePacket(int talkerNum, int clientNum, VoicePacket_t *voicePacket)
{
    client_t *client; // [esp+0h] [ebp-8h]

    if ( talkerNum < 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_voice_mp.cpp",
                    126,
                    0,
                    "%s",
                    "talkerNum >= 0") )
    {
        __debugbreak();
    }
    if ( clientNum < 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_voice_mp.cpp",
                    127,
                    0,
                    "%s",
                    "clientNum >= 0") )
    {
        __debugbreak();
    }
    if ( talkerNum >= com_maxclients->current.integer
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_voice_mp.cpp",
                    128,
                    0,
                    "%s",
                    "talkerNum < com_maxclients->current.integer") )
    {
        __debugbreak();
    }
    if ( clientNum >= com_maxclients->current.integer
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_voice_mp.cpp",
                    129,
                    0,
                    "%s",
                    "clientNum < com_maxclients->current.integer") )
    {
        __debugbreak();
    }
    client = &svs.clients[clientNum];
    if ( client->voicePacketCount < 40 )
    {
        client->voicePackets[client->voicePacketCount].dataSize = voicePacket->dataSize;
        memcpy(client->voicePackets[client->voicePacketCount].data, voicePacket->data, voicePacket->dataSize);
        if ( talkerNum != (unsigned __int8)talkerNum
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_voice_mp.cpp",
                        149,
                        0,
                        "%s",
                        "talkerNum == static_cast<byte>(talkerNum)") )
        {
            __debugbreak();
        }
        client->voicePackets[client->voicePacketCount++].talker = talkerNum;
    }
}

void __cdecl SV_UserVoice(client_t *cl, msg_t *msg)
{
    int packet; // [esp+0h] [ebp-11Ch]
    VoicePacket_t voicePacket; // [esp+4h] [ebp-118h] BYREF
    int packetCount; // [esp+114h] [ebp-8h]
    int totalBytes; // [esp+118h] [ebp-4h]

    totalBytes = 0;
    if ( SV_VoiceEnabled() )
    {
        packetCount = MSG_ReadByte(msg);
        if ( !cl->gentity
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_voice_mp.cpp", 177, 0, "%s", "cl->gentity") )
        {
            __debugbreak();
        }
        for ( packet = 0; packet < packetCount; ++packet )
        {
            voicePacket.dataSize = MSG_ReadByte(msg);
            if ( voicePacket.dataSize <= 0 || voicePacket.dataSize > 256 )
            {
                Com_Printf(15, "Received invalid voice packet of size %i from %s\n", voicePacket.dataSize, cl->name);
                return;
            }
            if ( !msg->data
                && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_voice_mp.cpp", 189, 0, "%s", "msg->data") )
            {
                __debugbreak();
            }
            if ( &voicePacket == (VoicePacket_t *)-1
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_voice_mp.cpp",
                            190,
                            0,
                            "%s",
                            "voicePacket.data") )
            {
                __debugbreak();
            }
            MSG_ReadData(msg, voicePacket.data, voicePacket.dataSize);
            G_BroadcastVoice(cl->gentity, &voicePacket);
        }
    }
}

void __cdecl SV_PreGameUserVoice(client_t *cl, msg_t *msg)
{
    int packet; // [esp+0h] [ebp-124h]
    VoicePacket_t voicePacket; // [esp+4h] [ebp-120h] BYREF
    int otherPlayer; // [esp+114h] [ebp-10h]
    int talker; // [esp+118h] [ebp-Ch]
    int packetCount; // [esp+11Ch] [ebp-8h]
    int totalBytes; // [esp+120h] [ebp-4h]

    totalBytes = 0;
    if ( SV_VoiceEnabled() )
    {
        talker = cl - svs.clients;
        if ( (unsigned int)talker >= com_maxclients->current.integer
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_voice_mp.cpp",
                        228,
                        0,
                        "talker doesn't index com_maxclients->current.integer\n\t%i not in [0, %i)",
                        talker,
                        com_maxclients->current.integer) )
        {
            __debugbreak();
        }
        packetCount = MSG_ReadByte(msg);
        for ( packet = 0; packet < packetCount; ++packet )
        {
            voicePacket.dataSize = MSG_ReadShort(msg);
            if ( voicePacket.dataSize <= 0 || voicePacket.dataSize > 256 )
            {
                Com_Printf(15, "Received invalid voice packet of size %i from %s\n", voicePacket.dataSize, cl->name);
                return;
            }
            if ( !msg->data
                && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_voice_mp.cpp", 241, 0, "%s", "msg->data") )
            {
                __debugbreak();
            }
            if ( &voicePacket == (VoicePacket_t *)-1
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_voice_mp.cpp",
                            242,
                            0,
                            "%s",
                            "voicePacket.data") )
            {
                __debugbreak();
            }
            MSG_ReadData(msg, voicePacket.data, voicePacket.dataSize);
            for ( otherPlayer = 0; otherPlayer < com_maxclients->current.integer; ++otherPlayer )
            {
                if ( otherPlayer != talker
                    && svs.clients[otherPlayer].header.state >= CS_CONNECTED
                    && !SV_ClientHasClientMuted(otherPlayer, talker)
                    && SV_ClientWantsVoiceData(otherPlayer) )
                {
                    SV_QueueVoicePacket(talker, otherPlayer, &voicePacket);
                }
            }
        }
    }
}

