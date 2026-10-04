#include "record.h"
#include <universal/q_shared.h>
#include <client_mp/cl_main_mp.h>
#include <client_mp/cl_input_mp.h>
#include "encode.h"
#include <qcommon/common.h>
#include "record_dsound.h"
#include <win32/win_voice.h>

int(__cdecl *current_audioCallback)(audioSample_t *);

float voice_current_voicelevel;

__int16 partial_audio_buffer[640];
int samples_in_partial_audio_buffer;

char enc_buffer[4096];
int enc_buffer_pos;

void __cdecl Record_SetRecordingCallback(int (__cdecl *new_audioCallback)(audioSample_t *))
{
    current_audioCallback = new_audioCallback;
}

int __cdecl Record_QueueAudioDataForEncoding(audioSample_t *sample)
{
    __int64 v1; // rax
    int v3; // [esp+0h] [ebp-2028h]
    int v4; // [esp+8h] [ebp-2020h]
    int v5; // [esp+Ch] [ebp-201Ch]
    int bytes; // [esp+10h] [ebp-2018h]
    _WORD dst[4098]; // [esp+18h] [ebp-2010h] BYREF
    int i; // [esp+2020h] [ebp-8h]
    int FrameSize; // [esp+2024h] [ebp-4h]

    voice_current_voicelevel = 0.0f;
    if ( sample->bytesPerSample == 2 && sample->lengthInSamples > 0 )
    {
        for ( i = 0; i < sample->lengthInSamples; ++i )
        {
            *(_WORD *)&sample->buffer[2 * i] = (int)(float)((float)*(__int16 *)&sample->buffer[2 * i] * voice_current_scaler);
            v1 = *(__int16 *)&sample->buffer[2 * i];
            voice_current_voicelevel = (float)(int)((HIDWORD(v1) ^ v1) - HIDWORD(v1)) + voice_current_voicelevel;
        }
        voice_current_voicelevel = voice_current_voicelevel / (float)sample->lengthInSamples;
    }
    if ( !Voice_SendVoiceData() )
        return 0;
    if ( IN_IsTalkKeyHeld() )
    {
        FrameSize = Encode_GetFrameSize();
        sample->sampleOffset = 0;
        v4 = 0;
        if ( FrameSize > 0 )
        {
            if ( sample->lengthInBytes > 0 )
            {
                if ( sample->sampleOffset >= 0 )
                {
                    while ( FrameSize <= samples_in_partial_audio_buffer + sample->lengthInSamples - sample->sampleOffset )
                    {
                        v5 = 0;
                        if ( samples_in_partial_audio_buffer )
                        {
                            if ( samples_in_partial_audio_buffer < FrameSize - 4 )
                                v3 = samples_in_partial_audio_buffer;
                            else
                                v3 = FrameSize - 4;
                            samples_in_partial_audio_buffer = v3;
                            if ( sample->bytesPerSample * v3 > 0x2000
                                && !Assert_MyHandler(
                                            "C:\\projects_pc\\cod\\codsrc\\src\\groupvoice\\record.cpp",
                                            119,
                                            0,
                                            "%s",
                                            "2*ENC_BUFFER_SIZE >= samples_copied * sample->bytesPerSample") )
                            {
                                __debugbreak();
                            }
                            if ( sample->bytesPerSample * v3 > 1280
                                && !Assert_MyHandler(
                                            "C:\\projects_pc\\cod\\codsrc\\src\\groupvoice\\record.cpp",
                                            120,
                                            0,
                                            "%s",
                                            "2*AUDIO_BUFFER_SIZE >= samples_copied * sample->bytesPerSample") )
                            {
                                __debugbreak();
                            }
                            memcpy((unsigned __int8 *)dst, (unsigned __int8 *)partial_audio_buffer, sample->bytesPerSample * v3);
                            v5 = v3;
                            samples_in_partial_audio_buffer -= v3;
                        }
                        memcpy(
                            (unsigned __int8 *)&dst[v5],
                            &sample->buffer[sample->bytesPerSample * sample->sampleOffset],
                            sample->bytesPerSample * (FrameSize - v5));
                        sample->sampleOffset += FrameSize - v5;
                        bytes = Encode_Sample((short*)dst, &enc_buffer[enc_buffer_pos], 4096 - enc_buffer_pos);
                        v4 += bytes;
                        Client_SendVoiceData(bytes, &enc_buffer[enc_buffer_pos]);
                    }
                    if ( sample->sampleOffset < sample->lengthInSamples )
                    {
                        memcpy(
                            (unsigned __int8 *)&partial_audio_buffer[samples_in_partial_audio_buffer],
                            &sample->buffer[sample->bytesPerSample * sample->sampleOffset],
                            sample->bytesPerSample * (sample->lengthInSamples - sample->sampleOffset));
                        samples_in_partial_audio_buffer += sample->lengthInSamples - sample->sampleOffset;
                    }
                    return v4;
                }
                else
                {
                    Com_Printf(9, "Invalid sample offset of %i\n", sample->sampleOffset);
                    return 0;
                }
            }
            else
            {
                Com_Printf(9, "Invalid sample length of %i samples\n", sample->lengthInSamples);
                return 0;
            }
        }
        else
        {
            Com_Printf(9, "Invalid encode frame size of %i\n", FrameSize);
            return 0;
        }
    }
    else
    {
        CL_VoiceTransmit(0);
        return 0;
    }
}

int __cdecl Record_AudioCallback(audioSample_t *sample)
{
    return Record_QueueAudioDataForEncoding(sample);
}

HRESULT __cdecl Record_Start(dsound_sample_t *sample)
{
    return DSOUNDRecord_Start(sample);
}

HRESULT __cdecl Record_Stop(dsound_sample_t *sample)
{
    return DSOUNDRecord_Stop(sample);
}

dsound_sample_t *__cdecl Record_NewSample()
{
    return DSOUNDRecord_NewSample();
}

int __cdecl Record_DestroySample(dsound_sample_t *sample)
{
    return DSOUNDRecord_DestroySample(sample);
}

void __cdecl Record_Shutdown()
{
    Encode_Shutdown();
    DSOUNDRecord_Shutdown();
}

void __cdecl Record_Frame()
{
    DSOUNDRecord_Frame();
}

int __cdecl Record_Init()
{
    Record_SetRecordingCallback(Record_AudioCallback);
    return DSOUNDRecord_Init(1);
}

