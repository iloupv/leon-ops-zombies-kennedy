#pragma once


struct dsound_sample_t;

unsigned int __cdecl Sound_UpdateSample(dsound_sample_t *sample, char *data, unsigned int data_len);
dsound_sample_t *__cdecl Sound_NewSample();
int __cdecl Sound_DestroySample(dsound_sample_t *sample);
char __cdecl Sound_StopSample(dsound_sample_t *sample);
void __cdecl Sound_SampleFrame(dsound_sample_t *sample);
int __cdecl Sound_Init(void *handle);
void __cdecl Sound_Shutdown();
