#pragma once
#include <qcommon/msg_mp.h>

struct ClientVoicePacket_t // sizeof=0x104
{                                       // XREF: voiceCommunication_t/r
    unsigned __int8 data[256];
    int dataSize;
};

struct voiceCommunication_t // sizeof=0xA30
{                                       // XREF: .data:voiceCommunication_t * cl_voiceCommunication/r
    ClientVoicePacket_t voicePackets[10];
    int voicePacketCount;
    int voicePacketLastTransmit;
};

void __cdecl CL_WriteVoicePacket(int localClientNum);
void __cdecl CL_VoicePacket(int localClientNum, msg_t *msg);
bool __cdecl CL_IsPlayerTalking(int localClientNum, unsigned int talkingClientIndex);
