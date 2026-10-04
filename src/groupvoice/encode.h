#pragma once

void __cdecl Encode_SetOptions(int frequency, int quality);
bool __cdecl Encode_Init(int bandwidthEnum);
char __cdecl Encode_Shutdown();
int __cdecl Encode_Sample(__int16 *buffer_in, char *buffer_out, int maxLength);
int __cdecl Encode_GetFrameSize();

extern int g_current_bandwidth_setting; // BO1TODO: move somewhere
extern int g_encoder_samplerate;