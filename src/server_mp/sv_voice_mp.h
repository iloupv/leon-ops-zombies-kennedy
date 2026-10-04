#pragma once

struct client_t;
struct msg_t;

struct __declspec(align(1)) VoicePacket_t // sizeof=0x105
{                                       // XREF: client_t/r
                                        // ?SV_UserVoice@@YAXPAUclient_t@@PAUmsg_t@@@Z/r ...
    unsigned __int8 talker;             // XREF: CL_VoicePacket(int,msg_t *)+69/w
                                        // CL_VoicePacket(int,msg_t *)+BC/r ...
    unsigned __int8 data[256];          // XREF: CL_VoicePacket(int,msg_t *)+A9/o
                                        // CL_VoicePacket(int,msg_t *)+118/o ...
    int dataSize;                       // XREF: CL_VoicePacket(int,msg_t *)+7B/w
                                        // CL_VoicePacket(int,msg_t *)+7E/r ...
};

bool __cdecl SV_VoiceEnabled();
void __cdecl SV_SendClientVoiceData(client_t *client);
void __cdecl SV_WriteVoiceDataToClient(client_t *client, msg_t *msg);
bool __cdecl SV_ClientWantsVoiceData(unsigned int clientNum);
bool __cdecl SV_ClientHasClientMuted(unsigned int listener, unsigned int talker);
void __cdecl SV_QueueVoicePacket(int talkerNum, int clientNum, VoicePacket_t *voicePacket);
void __cdecl SV_UserVoice(client_t *cl, msg_t *msg);
void __cdecl SV_PreGameUserVoice(client_t *cl, msg_t *msg);
