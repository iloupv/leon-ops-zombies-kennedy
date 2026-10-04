#include "cl_voice.h"
#include "cl_main.h"
#include <client_mp/cl_main_mp.h>
#include <cgame_mp/cg_main_mp.h>
#include "client.h"
#include <client_mp/cl_main_pc_mp.h>
#include <win32/win_voice.h>

unsigned __int8 tempVoicePacketBuf[1260];

void __cdecl CL_WriteVoicePacket(int localClientNum)
{
    connstate_t connstate; // [esp+4h] [ebp-40h]
    int voicePacket; // [esp+8h] [ebp-3Ch]
    voiceCommunication_t *vc; // [esp+Ch] [ebp-38h]
    msg_t msg; // [esp+10h] [ebp-34h] BYREF
    const clientConnection_t *clc; // [esp+40h] [ebp-4h]

    clc = CL_GetLocalClientConnection(localClientNum);
    CL_GetLocalClientGlobals(localClientNum);
    vc = CL_GetLocalClientVoiceCommunication(localClientNum);
    connstate = CL_GetLocalClientConnectionState(localClientNum);
    if ( !clc->demoplaying && (connstate == CA_ACTIVE || connstate == CA_LOADING || connstate == CA_PRIMED) )
    {
        MSG_Init(&msg, tempVoicePacketBuf, 1260);
        MSG_WriteString(&msg, "v");
        MSG_WriteShort(&msg, clc->qport);
        MSG_WriteByte(&msg, vc->voicePacketCount);
        for ( voicePacket = 0; voicePacket < vc->voicePacketCount; ++voicePacket )
        {
            if ( vc->voicePackets[voicePacket].dataSize <= 0
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_voice.cpp",
                            1453,
                            0,
                            "%s",
                            "vc->voicePackets[voicePacket].dataSize > 0") )
            {
                __debugbreak();
            }
            if ( vc->voicePackets[voicePacket].dataSize >= 0x10000
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_voice.cpp",
                            1454,
                            0,
                            "%s",
                            "vc->voicePackets[voicePacket].dataSize < (2<<15)") )
            {
                __debugbreak();
            }
            MSG_WriteByte(&msg, vc->voicePackets[voicePacket].dataSize);
            MSG_WriteData(&msg, vc->voicePackets[voicePacket].data, vc->voicePackets[voicePacket].dataSize);
        }
        NET_OutOfBandVoiceData(clc->netchan.sock, clc->serverAddress, msg.data, msg.cursize);
        if ( cl_showSend->current.enabled )
            Com_Printf(30, "voice: %i\n", msg.cursize);
    }
}

void __cdecl CL_VoicePacket(int localClientNum, msg_t *msg)
{
    int packet; // [esp+0h] [ebp-114h]
    VoicePacket_t voicePacket; // [esp+4h] [ebp-110h] BYREF
    int numPackets; // [esp+110h] [ebp-4h]

    numPackets = MSG_ReadByte(msg);
    if ( (unsigned int)numPackets <= 0x28 )
    {
        for ( packet = 0; packet < numPackets; ++packet )
        {
            voicePacket.talker = MSG_ReadByte(msg);
            voicePacket.dataSize = MSG_ReadByte(msg);
            if ( voicePacket.dataSize <= 0 || voicePacket.dataSize > 256 )
            {
                Com_Printf(30, "Invalid server voice packet of %i bytes\n", voicePacket.dataSize);
                return;
            }
            MSG_ReadData(msg, voicePacket.data, voicePacket.dataSize);
            if ( voicePacket.talker >= 0x20u )
            {
                Com_Printf(30, "Invalid voice packet - talker was %i\n", voicePacket.talker);
                return;
            }
            if ( !CL_IsPlayerMuted(localClientNum, voicePacket.talker) && cl_voice->current.enabled )
                Voice_IncomingVoiceData(voicePacket.talker, voicePacket.data, voicePacket.dataSize);
        }
    }
}

bool __cdecl CL_IsPlayerTalking(int localClientNum, unsigned int talkingClientIndex)
{
    return Voice_IsClientTalking(talkingClientIndex);
}

