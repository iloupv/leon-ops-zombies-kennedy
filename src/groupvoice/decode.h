#pragma once

char __cdecl Decode_Init(int bandwidthEnum);
void __cdecl Decode_SetOptions();
void __cdecl Decode_Shutdown();
int __cdecl Decode_Sample(char *buffer, int maxLength, __int16 *out, int frame_size);

extern int g_decode_frame_size;
