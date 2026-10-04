#pragma once

struct SessionData_s;

bool __cdecl Voice_Init();
void __cdecl Voice_StopClientSamples();
void __cdecl Voice_Shutdown();
double __cdecl Voice_GetVoiceLevel();
void __cdecl Voice_Playback();
int __cdecl Voice_GetLocalVoiceData();
void __cdecl Voice_IncomingVoiceData(unsigned __int8 talker, unsigned __int8 *data, int packetDataSize);
bool __cdecl Voice_IsClientTalking(unsigned int clientNum);
char __cdecl Voice_StartRecording();
char __cdecl Voice_StopRecording();
unsigned int __cdecl mixerGetRecordLevel(char *SrcName);
int __cdecl mixerSetRecordLevel(char *SrcName, unsigned __int16 newLevel);
int __cdecl mixerGetRecordSource(char *srcName);
int __cdecl mixerSetRecordSource(char *SrcName);
int __cdecl mixerSetMicrophoneMute(unsigned __int8 bMute);
int __cdecl Live_GetClientNumForXuid(const SessionData_s *session, unsigned __int64 xuid);

extern float voice_current_scaler;