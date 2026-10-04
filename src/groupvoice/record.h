#pragma once
#include "directsound.h"

struct audioSample_t // sizeof=0x20
{                                       // XREF: ?DSOUNDRecord_UpdateSample@@YAXPAUdsound_sample_t@@@Z/r
    unsigned __int8 *buffer;            // XREF: DSOUNDRecord_UpdateSample(dsound_sample_t *)+1A7/w
                                        // DSOUNDRecord_UpdateSample(dsound_sample_t *)+1D7/w
    int lengthInBytes;                  // XREF: DSOUNDRecord_UpdateSample(dsound_sample_t *)+1A1/w
                                        // DSOUNDRecord_UpdateSample(dsound_sample_t *)+1D1/w
    int lengthInSamples;                // XREF: DSOUNDRecord_UpdateSample(dsound_sample_t *)+19B/w
                                        // DSOUNDRecord_UpdateSample(dsound_sample_t *)+1CB/w
    int bytesPerSample;                 // XREF: DSOUNDRecord_UpdateSample(dsound_sample_t *)+163/w
    int frequency;                      // XREF: DSOUNDRecord_UpdateSample(dsound_sample_t *)+16C/w
    bool stereo;                        // XREF: DSOUNDRecord_UpdateSample(dsound_sample_t *)+13E/w
                                        // DSOUNDRecord_UpdateSample(dsound_sample_t *)+141/r
    // padding byte
    // padding byte
    // padding byte
    int channels;                       // XREF: DSOUNDRecord_UpdateSample(dsound_sample_t *)+17B/w
    int sampleOffset;                   // XREF: DSOUNDRecord_UpdateSample(dsound_sample_t *)+16F/w
};

void __cdecl Record_SetRecordingCallback(int (__cdecl *new_audioCallback)(audioSample_t *));
int __cdecl Record_QueueAudioDataForEncoding(audioSample_t *sample);
int __cdecl Record_AudioCallback(audioSample_t *sample);
HRESULT __cdecl Record_Start(dsound_sample_t *sample);
HRESULT __cdecl Record_Stop(dsound_sample_t *sample);
dsound_sample_t *__cdecl Record_NewSample();
int __cdecl Record_DestroySample(dsound_sample_t *sample);
void __cdecl Record_Shutdown();
void __cdecl Record_Frame();
int __cdecl Record_Init();

extern int(__cdecl *current_audioCallback)(audioSample_t *);

extern float voice_current_voicelevel;