#include "avi.h"

#include <cstring>
#include <Windows.h>
#include "yuv.h"
#include <gfx_d3d/r_dvars.h>
#include <vpx/vpx.h>
#include "mjpeg.h"
#include <qcommon/threads.h>
#include <universal/q_shared.h>
#include <qcommon/common.h>

avi_s *AVI;

avi::RIFF_s::RIFF_s()
{
    //avi::RIFF_s::LIST_hdrl_s::LIST_hdrl_s(&this->list);
    this->header[0] = 'R';
    this->header[1] = 'I';
    this->header[2] = 'F';
    this->header[3] = 'F';
    this->type[0] = 'A';
    this->type[1] = 'V';
    this->type[2] = 'I';
    this->type[3] = ' ';
}

avi::RIFF_s::LIST_hdrl_s::LIST_hdrl_s()
{
    //avi::RIFF_s::LIST_hdrl_s::avih_s::avih_s(&this->avih);
    //avi::RIFF_s::LIST_hdrl_s::LIST_vid_strl_s::LIST_vid_strl_s(&this->strl_vid);
    //avi::RIFF_s::LIST_hdrl_s::LIST_aud_strl_s::LIST_aud_strl_s(&this->strl_aud);
    this->header[0] = 'L';
    this->header[1] = 'I';
    this->header[2] = 'S';
    this->header[3] = 'T';
    this->type[0] = 'h';
    this->type[1] = 'd';
    this->type[2] = 'r';
    this->type[3] = 'l';
}

avi::RIFF_s::LIST_hdrl_s::avih_s::avih_s()
{
    this->header[0] = 'a';
    this->header[1] = 'v';
    this->header[2] = 'i';
    this->header[3] = 'h';
    this->MicroSecPerFrame = 0;
    this->MaxBytesPerSec = 0;
    this->PaddingGranularity = 0;
    this->Flags = 0x110;
    this->TotalFrames = 0;
    this->InitialFrames = 0;
    this->Streams = 2;
    this->SuggestedBufferSize = 0;
    this->Width = 0;
    this->Height = 0;
    this->Reserved[3] = 0;
    this->Reserved[2] = 0;
    this->Reserved[1] = 0;
    this->Reserved[0] = 0;
}

avi::RIFF_s::LIST_hdrl_s::LIST_vid_strl_s::LIST_vid_strl_s()
{
    //avi::RIFF_s::LIST_hdrl_s::LIST_vid_strl_s::strh_s::strh_s(&this->strh);
    //avi::RIFF_s::LIST_hdrl_s::LIST_vid_strl_s::strf_s::strf_s(&this->strf);
    this->header[0] = 'L';
    this->header[1] = 'I';
    this->header[2] = 'S';
    this->header[3] = 'T';
    this->type[0] = 's';
    this->type[1] = 't';
    this->type[2] = 'r';
    this->type[3] = 'l';
}

avi::RIFF_s::LIST_hdrl_s::LIST_vid_strl_s::strh_s::strh_s()
{
    this->header[0] = 's';
    this->header[1] = 't';
    this->header[2] = 'r';
    this->header[3] = 'h';
    strcpy(this->type, "vidsmjpg");
    //BYTE1(this->Flags) = 0;
    //HIWORD(this->Flags) = 0;
    this->Flags = 0;
    this->Priority = 0;
    this->Language = 0;
    this->InitialFrames = 0;
    this->Scale = 1;
    this->Rate = 0;
    this->Start = 0;
    this->Length = 0;
    this->SuggestedBufferSize = 0;
    this->Quality = 10000;
    this->SampleSize = 0;
    this->rLeft = 0;
    this->rTop = 0;
    this->rBottom = 0;
    this->rRight = 0;
}

avi::RIFF_s::LIST_hdrl_s::LIST_vid_strl_s::strf_s::strf_s()
{
    this->header[0] = 's';
    this->header[1] = 't';
    this->header[2] = 'r';
    this->header[3] = 'f';
    this->width = 0;
    this->height = 0;
    this->planes = 1;
    this->bit_count = 24;
    this->compression[0] = 'M';
    this->compression[1] = 'J';
    this->compression[2] = 'P';
    this->compression[3] = 'G';
    this->size_image = 0;
    this->horizontal_ppm = 0;
    this->vertical_ppm = 0;
    this->colors_used = 0;
    this->colors_important = 0;
}

avi::RIFF_s::LIST_hdrl_s::LIST_aud_strl_s::LIST_aud_strl_s()
{
    //avi::RIFF_s::LIST_hdrl_s::LIST_aud_strl_s::strh_s::strh_s(&this->strh);
    //avi::RIFF_s::LIST_hdrl_s::LIST_aud_strl_s::strf_s::strf_s(&this->strf);
    this->header[0] = 'L';
    this->header[1] = 'I';
    this->header[2] = 'S';
    this->header[3] = 'T';
    this->type[0] = 's';
    this->type[1] = 't';
    this->type[2] = 'r';
    this->type[3] = 'l';
}

avi::RIFF_s::LIST_hdrl_s::LIST_aud_strl_s::strh_s::strh_s()
{
    this->header[0] = 's';
    this->header[1] = 't';
    this->header[2] = 'r';
    this->header[3] = 'h';
    strcpy(this->type, "auds");
    this->format[1] = 0;
    this->format[2] = 0;
    this->format[3] = 0;
    this->Flags = 0;
    this->Priority = 0;
    this->Language = 0;
    this->InitialFrames = 0;
    this->Scale = 2;
    this->Rate = 48000;
    this->Start = 0;
    this->Length = 0;
    this->SuggestedBufferSize = 24000;
    this->Quality = -1;
    this->SampleSize = 2;
    this->rLeft = 0;
    this->rTop = 0;
    this->rBottom = 0;
    this->rRight = 0;
}

avi::RIFF_s::LIST_hdrl_s::LIST_aud_strl_s::strf_s::strf_s()
{
    this->header[0] = 's';
    this->header[1] = 't';
    this->header[2] = 'r';
    this->header[3] = 'f';
    this->format = 1;
    this->channels = 1;
    this->samples_per_sec = 24000;
    this->avg_bytes_per_sec = 48000;
    this->block_align = 2;
    this->bits_per_sample = 16;
}

unsigned int __stdcall mjpeg_write_thread(void *__formal)
{
    //avi_s::write_thread(AVI);
    AVI->write_thread();
    return 0;
}

void avi_s::write_thread()
{
    HANDLE CurrentThread; // eax
    unsigned __int8 *locFlushToPtr; // [esp+4h] [ebp-8h]
    DWORD bytes; // [esp+8h] [ebp-4h] BYREF

    this->threadQuit = 0;
    CurrentThread = GetCurrentThread();
    SetThreadPriority(CurrentThread, 2);
    while ( this->flushToPtr != this->writePtr || !this->threadQuit )
    {
        WaitForSingleObject(this->hOutput, 0xFFFFFFFF);
        locFlushToPtr = this->flushToPtr;
        if ( locFlushToPtr != this->writePtr )
        {
            if ( locFlushToPtr < this->writePtr )
            {
                if ( this->encodeBuff != this->writePtr )
                    WriteFile(this->hFile, this->writePtr, this->encodeBuff - this->writePtr, &bytes, 0);
                this->writePtr = this->buffer;
            }
            WriteFile(this->hFile, this->writePtr, locFlushToPtr - this->writePtr, &bytes, 0);
            this->writePtr = locFlushToPtr;
        }
    }
}

void __cdecl mjpeg_create(const char *szFile, int width, int height, unsigned int fps)
{
    avi_s *v4; // [esp+0h] [ebp-28h]
    avi::RIFF_s *v5; // [esp+24h] [ebp-4h]

    if ( !AVI )
    {
        //v5 = (avi::RIFF_s *)operator new(0xA7870u);
        //if ( v5 )
        //{
        //    avi::RIFF_s::RIFF_s(v5);
        //    v4 = (avi_s *)v5;
        //}
        //else
        //{
        //    v4 = 0;
        //}
        //AVI = v4;
        AVI = new avi_s;
    }
    yuv_init(width, height, 1);
    if ( r_clipCodec->current.integer == 1 )
        vpx_init(szFile, width, height);
    else
        AVI->create(szFile, width, height, fps);
    mjpeg_run_encoder = 1;
}

void avi_s::create(
                const char *szFile,
                unsigned int width,
                unsigned int height,
                unsigned int fps)
{
    DWORD bytes; // [esp+1Ch] [ebp-Ch] BYREF
    unsigned __int8 *now; // [esp+20h] [ebp-8h] BYREF
    DWORD threadId; // [esp+24h] [ebp-4h] BYREF

    if ( !this->hFile )
    {
        this->hFile = CreateFileA(szFile, 0x40000000u, 0, 0, 2u, 0, 0);
        this->audioBufferCount = 0;
        this->writePtr = this->buffer;
        this->flushToPtr = this->writePtr;
        this->outputPtr = this->flushToPtr;
        //avi::RIFF_s::Setup(&this->riff, width, height, fps);
        this->riff.Setup(width, height, fps);
        now = this->buffer;
        //avi::RIFF_s::Out(&this->riff, &now);
        this->riff.Out(&now);
        WriteFile(this->hFile, this->buffer, now - this->buffer, &bytes, 0);
        WriteFile(this->hFile, "LISTXXXXmovi", 0xCu, &bytes, 0);
        this->fileSize = GetFileSize(this->hFile, 0);
        this->hOutput = CreateSemaphoreA(0, 0, 0x7FFFFFFF, 0);
        this->hOutputAddLock = CreateMutexA(0, 0, 0);
        this->hOutputThread = CreateThread(0, 0x1000u, (LPTHREAD_START_ROUTINE)mjpeg_write_thread, 0, 0, &threadId);
        SetThreadName(threadId, "MJPEG Write Thread");
    }
}

void avi::RIFF_s::Out(unsigned __int8 **out)
{
    unsigned int j; // [esp+14h] [ebp-Ch]
    unsigned int i; // [esp+18h] [ebp-8h]
    unsigned __int8 *sizePtr; // [esp+1Ch] [ebp-4h] BYREF

    for ( i = 0; i < 4; ++i )
        *(*out)++ = this->header[i];
    sizePtr = *out;
    avi::Out32(out, this->size);
    for ( j = 0; j < 4; ++j )
        *(*out)++ = this->type[j];
    //avi::RIFF_s::LIST_hdrl_s::Out(&this->list, out);
    this->list.Out(out);
    avi::Out32(&sizePtr, *out - sizePtr - 4);
}

unsigned int __cdecl avi::Out32(unsigned __int8 **out, unsigned int d)
{
    *(*out)++ = d;
    *(*out)++ = BYTE1(d);
    *(*out)++ = BYTE2(d);
    *(*out)++ = HIBYTE(d);
    return 4;
}

void avi::RIFF_s::LIST_hdrl_s::Out(unsigned __int8 **out)
{
    unsigned int j; // [esp+24h] [ebp-Ch]
    unsigned int i; // [esp+28h] [ebp-8h]
    unsigned __int8 *sizePtr; // [esp+2Ch] [ebp-4h] BYREF

    for ( i = 0; i < 4; ++i )
        *(*out)++ = this->header[i];
    sizePtr = *out;
    avi::Out32(out, this->size);
    for ( j = 0; j < 4; ++j )
        *(*out)++ = this->type[j];
    this->avih.Out(out);
    this->strl_vid.Out(out);
    this->strl_aud.Out(out);
    avi::Out32(&sizePtr, *out - sizePtr - 4);
}

void avi::RIFF_s::LIST_hdrl_s::avih_s::Out(unsigned __int8 **out)
{
    unsigned int j; // [esp+4h] [ebp-Ch]
    unsigned int i; // [esp+8h] [ebp-8h]
    unsigned __int8 *sizePtr; // [esp+Ch] [ebp-4h] BYREF

    for ( i = 0; i < 4; ++i )
        *(*out)++ = this->header[i];
    sizePtr = *out;
    avi::Out32(out, this->size);
    avi::Out32(out, this->MicroSecPerFrame);
    avi::Out32(out, this->MaxBytesPerSec);
    avi::Out32(out, this->PaddingGranularity);
    avi::Out32(out, this->Flags);
    avi::Out32(out, this->TotalFrames);
    avi::Out32(out, this->InitialFrames);
    avi::Out32(out, this->Streams);
    avi::Out32(out, this->SuggestedBufferSize);
    avi::Out32(out, this->Width);
    avi::Out32(out, this->Height);
    for ( j = 0; j < 0x10; ++j )
        *(*out)++ = *((_BYTE *)this->Reserved + j);
    avi::Out32(&sizePtr, *out - sizePtr - 4);
}

void avi::RIFF_s::LIST_hdrl_s::LIST_vid_strl_s::Out(unsigned __int8 **out)
{
    unsigned int j; // [esp+28h] [ebp-Ch]
    unsigned int i; // [esp+2Ch] [ebp-8h]
    unsigned __int8 *sizePtr; // [esp+30h] [ebp-4h] BYREF

    for ( i = 0; i < 4; ++i )
        *(*out)++ = this->header[i];
    sizePtr = *out;
    avi::Out32(out, this->size);
    for ( j = 0; j < 4; ++j )
        *(*out)++ = this->type[j];
    //avi::RIFF_s::LIST_hdrl_s::LIST_vid_strl_s::strh_s::Out((avi::RIFF_s::LIST_hdrl_s::LIST_aud_strl_s::strh_s *)&this->strh, out);
    this->strh.Out(out);
    //avi::RIFF_s::LIST_hdrl_s::LIST_vid_strl_s::strf_s::Out(&this->strf, out);
    this->strf.Out(out);
    avi::Out32(&sizePtr, *out - sizePtr - 4);
}

void avi::RIFF_s::LIST_hdrl_s::LIST_vid_strl_s::strf_s::Out(unsigned __int8 **out)
{
    unsigned int j; // [esp+4h] [ebp-10h]
    unsigned __int16 bit_count; // [esp+8h] [ebp-Ch]
    unsigned __int16 planes; // [esp+Ah] [ebp-Ah]
    unsigned int i; // [esp+Ch] [ebp-8h]
    unsigned __int8 *sizePtr; // [esp+10h] [ebp-4h] BYREF

    for ( i = 0; i < 4; ++i )
        *(*out)++ = this->header[i];
    sizePtr = *out;
    avi::Out32(out, this->size);
    avi::Out32(out, this->sizeBi);
    avi::Out32(out, this->width);
    avi::Out32(out, this->height);
    planes = this->planes;
    *(*out)++ = planes;
    *(*out)++ = HIBYTE(planes);
    bit_count = this->bit_count;
    *(*out)++ = bit_count;
    *(*out)++ = HIBYTE(bit_count);
    for ( j = 0; j < 4; ++j )
        *(*out)++ = this->compression[j];
    avi::Out32(out, this->size_image);
    avi::Out32(out, this->horizontal_ppm);
    avi::Out32(out, this->vertical_ppm);
    avi::Out32(out, this->colors_used);
    avi::Out32(out, this->colors_important);
    this->size = *out - sizePtr - 4;
    avi::Out32(&sizePtr, this->size);
    avi::Out32(&sizePtr, this->size);
}

void avi::RIFF_s::LIST_hdrl_s::LIST_aud_strl_s::Out(unsigned __int8 **out)
{
    unsigned int j; // [esp+2Ch] [ebp-Ch]
    unsigned int i; // [esp+30h] [ebp-8h]
    unsigned __int8 *sizePtr; // [esp+34h] [ebp-4h] BYREF

    for ( i = 0; i < 4; ++i )
        *(*out)++ = this->header[i];
    sizePtr = *out;
    avi::Out32(out, this->size);
    for ( j = 0; j < 4; ++j )
        *(*out)++ = this->type[j];
    //avi::RIFF_s::LIST_hdrl_s::LIST_vid_strl_s::strh_s::Out(&this->strh, out);
    this->strh.Out(out);
    //avi::RIFF_s::LIST_hdrl_s::LIST_aud_strl_s::strf_s::Out(&this->strf, out);
    this->strf.Out(out);
    avi::Out32(&sizePtr, *out - sizePtr - 4);
}

void avi::RIFF_s::LIST_hdrl_s::LIST_vid_strl_s::strh_s::Out(unsigned __int8 **out)
{
    unsigned __int16 rBottom; // [esp+4h] [ebp-1Ch]
    unsigned __int16 rRight; // [esp+6h] [ebp-1Ah]
    unsigned __int16 rTop; // [esp+8h] [ebp-18h]
    unsigned __int16 rLeft; // [esp+Ah] [ebp-16h]
    unsigned __int16 Language; // [esp+Ch] [ebp-14h]
    unsigned __int16 Priority; // [esp+Eh] [ebp-12h]
    unsigned int k; // [esp+10h] [ebp-10h]
    unsigned int j; // [esp+14h] [ebp-Ch]
    unsigned int i; // [esp+18h] [ebp-8h]
    unsigned __int8 *sizePtr; // [esp+1Ch] [ebp-4h] BYREF

    for ( i = 0; i < 4; ++i )
        *(*out)++ = this->header[i];
    sizePtr = *out;
    avi::Out32(out, this->size);
    for ( j = 0; j < 4; ++j )
        *(*out)++ = this->type[j];
    for ( k = 0; k < 4; ++k )
        *(*out)++ = this->format[k];
    avi::Out32(out, this->Flags);
    Priority = this->Priority;
    *(*out)++ = Priority;
    *(*out)++ = HIBYTE(Priority);
    Language = this->Language;
    *(*out)++ = Language;
    *(*out)++ = HIBYTE(Language);
    avi::Out32(out, this->InitialFrames);
    avi::Out32(out, this->Scale);
    avi::Out32(out, this->Rate);
    avi::Out32(out, this->Start);
    avi::Out32(out, this->Length);
    avi::Out32(out, this->SuggestedBufferSize);
    avi::Out32(out, this->Quality);
    avi::Out32(out, this->SampleSize);
    rLeft = this->rLeft;
    *(*out)++ = rLeft;
    *(*out)++ = HIBYTE(rLeft);
    rTop = this->rTop;
    *(*out)++ = rTop;
    *(*out)++ = HIBYTE(rTop);
    rRight = this->rRight;
    *(*out)++ = rRight;
    *(*out)++ = HIBYTE(rRight);
    rBottom = this->rBottom;
    *(*out)++ = rBottom;
    *(*out)++ = HIBYTE(rBottom);
    avi::Out32(&sizePtr, *out - sizePtr - 4);
}

void avi::RIFF_s::LIST_hdrl_s::LIST_aud_strl_s::strf_s::Out(
                unsigned __int8 **out)
{
    unsigned __int16 bits_per_sample; // [esp+4h] [ebp-10h]
    unsigned __int16 block_align; // [esp+6h] [ebp-Eh]
    unsigned __int16 channels; // [esp+8h] [ebp-Ch]
    unsigned __int16 format; // [esp+Ah] [ebp-Ah]
    unsigned int i; // [esp+Ch] [ebp-8h]
    unsigned __int8 *sizePtr; // [esp+10h] [ebp-4h] BYREF

    for ( i = 0; i < 4; ++i )
        *(*out)++ = this->header[i];
    sizePtr = *out;
    avi::Out32(out, this->size);
    format = this->format;
    *(*out)++ = format;
    *(*out)++ = HIBYTE(format);
    channels = this->channels;
    *(*out)++ = channels;
    *(*out)++ = HIBYTE(channels);
    avi::Out32(out, this->samples_per_sec);
    avi::Out32(out, this->avg_bytes_per_sec);
    block_align = this->block_align;
    *(*out)++ = block_align;
    *(*out)++ = HIBYTE(block_align);
    bits_per_sample = this->bits_per_sample;
    *(*out)++ = bits_per_sample;
    *(*out)++ = HIBYTE(bits_per_sample);
    this->size = *out - sizePtr - 4;
    avi::Out32(&sizePtr, this->size);
}

void avi::RIFF_s::Setup(unsigned int width, unsigned int height, unsigned int fps)
{
    this->list.avih.Width = width;
    this->list.avih.Height = height;
    this->list.avih.SuggestedBufferSize = 0;
    this->list.strl_vid.strf.width = width;
    this->list.strl_vid.strf.height = height;
    this->list.strl_vid.strh.rTop = 0;
    this->list.strl_vid.strh.rLeft = 0;
    this->list.strl_vid.strh.rBottom = height;
    this->list.strl_vid.strh.rRight = width;
    this->list.strl_vid.strf.size_image = 3 * height * width;
    this->list.avih.MicroSecPerFrame = 0xF4240 / fps;
    this->list.strl_vid.strh.Rate = fps;
    this->list.avih.TotalFrames = 0;
    this->list.strl_vid.strh.Length = 0;
    this->list.strl_aud.strh.Length = 0;
    this->idx1.count = 0;
}

void __cdecl mjpeg_add_frame(unsigned __int8 *y, unsigned __int8 *u, unsigned __int8 *v)
{
    if ( r_clipCodec->current.integer == 1 )
        vpx_encode_frame(y, u, v, 0);
    else
        AVI->add_frame(y, u, v);
}

void avi_s::add_frame(unsigned __int8 *y, unsigned __int8 *u, unsigned __int8 *v)
{
#if 0 // BO1TODO: jpeg bits
    unsigned int v4; // eax
    unsigned int encodeSize; // [esp+10h] [ebp-1E4h]
    unsigned __int8 *encodeEnd; // [esp+14h] [ebp-1E0h]
    unsigned __int8 *sizePtr; // [esp+18h] [ebp-1DCh] BYREF
    jpeg_encode encode; // [esp+1Ch] [ebp-1D8h] BYREF

    if ( this->riff.list.avih.TotalFrames <= this->riff.list.strl_vid.strh.Rate
                                                                                 * (this->audioBufferCount + this->riff.list.strl_aud.strh.Length)
                                                                                 / 0x5DC0 )
    {
        this->encodeBuff[0] = a0_2[0];
        this->encodeBuff[1] = 48;
        this->encodeBuff[2] = 100;
        this->encodeBuff[3] = 99;
        sizePtr = &this->encodeBuff[4];
        for ( encodeEnd = jpeg_encode::encode(
                                                &encode,
                                                &this->encodeBuff[8],
                                                this->riff.list.avih.Width,
                                                this->riff.list.avih.Height,
                                                this->riff.list.avih.Width,
                                                1024,
                                                y,
                                                u,
                                                v,
                                                this->riff.list.avih.TotalFrames == 0,
                                                0); (((_BYTE)encodeEnd - ((_BYTE)this + 64)) & 3) != 0; ++encodeEnd )
            *encodeEnd = 0;
        encodeSize = encodeEnd - this->encodeBuff;
        avi::Out32(&sizePtr, encodeSize - 8);
        if ( encodeSize > 0x40000 )
            Com_PrintWarning(16, "WARNING: Jpeg encoding buffer overrun\n");
        v4 = avi_s::output_data(this, this->encodeBuff, encodeSize);
        avi::RIFF_s::AddFrame(&this->riff, v4, encodeSize - 8);
    }
    else
    {
        Com_PrintWarning(16, "WARNING: Movie encoding. Sound out of sync with video, droping frames\n");
    }
#endif
}

void avi::RIFF_s::AddFrame(unsigned int offset, unsigned int size)
{
    ++this->list.avih.TotalFrames;
    ++this->list.strl_vid.strh.Length;
    if ( size > this->list.strl_vid.strh.SuggestedBufferSize )
        this->list.strl_vid.strh.SuggestedBufferSize = size;
    this->idx1.offsets[this->idx1.count] = offset;
    this->idx1.sizes[this->idx1.count++] = size | 0x80000000;
}

unsigned int avi_s::output_data(unsigned __int8 *data, unsigned int size)
{
    int bufferLeft; // [esp+4h] [ebp-8h]
    unsigned int ret; // [esp+8h] [ebp-4h]

    if ( !this->hOutputAddLock )
        return this->fileSize;
    WaitForSingleObject(this->hOutputAddLock, 0xFFFFFFFF);
    if ( &this->outputPtr[size] >= this->encodeBuff )
    {
        bufferLeft = this->encodeBuff - this->outputPtr;
        if ( bufferLeft )
        {
            while ( this->outputPtr >= this->writePtr != &this->outputPtr[bufferLeft] >= this->writePtr )
            {
                Com_PrintWarning(16, "WARNING: Movie encoding buffer overrun\n");
                Sleep(1u);
            }
            memcpy(this->outputPtr, data, bufferLeft);
        }
        while ( &this->buffer[size - bufferLeft] >= this->writePtr )
        {
            Com_PrintWarning(16, "WARNING: Movie encoding buffer overrun\n");
            Sleep(1u);
        }
        memcpy(this->buffer, &data[bufferLeft], size - bufferLeft);
        this->outputPtr = &this->buffer[size - bufferLeft];
    }
    else
    {
        while ( this->outputPtr >= this->writePtr != &this->outputPtr[size] >= this->writePtr )
        {
            Com_PrintWarning(16, "WARNING: Movie encoding buffer overrun\n");
            Sleep(1u);
        }
        memcpy(this->outputPtr, data, size);
        this->outputPtr += size;
    }
    ret = this->fileSize;
    this->fileSize = size + ret;
    this->flushToPtr = this->outputPtr;
    ReleaseSemaphore(this->hOutput, 1, 0);
    ReleaseMutex(this->hOutputAddLock);
    return ret;
}

void __cdecl mjpeg_close_int()
{
    if ( r_clipCodec->current.integer == 1 )
        vpx_shutdown();
    else
        AVI->close();
    yuv_shutdown();
    operator delete(AVI);
    AVI = 0;
}

void avi_s::close()
{
    DWORD bytes; // [esp+2Ch] [ebp-18h] BYREF
    int dataSize; // [esp+30h] [ebp-14h]
    unsigned __int8 *now; // [esp+34h] [ebp-10h] BYREF
    unsigned __int8 *riffSize; // [esp+38h] [ebp-Ch] BYREF
    int sizeOfRiff; // [esp+3Ch] [ebp-8h]
    unsigned __int8 *listSize; // [esp+40h] [ebp-4h] BYREF

    avi_s::flush_samples();
    dataSize = this->fileSize;
    now = this->encodeBuff;
    //avi::idx1_s::Out(&this->riff.idx1, &now);
    this->riff.idx1.Out(&now);
    avi_s::output_data(this->encodeBuff, now - this->encodeBuff);
    this->threadQuit = 1;
    ReleaseSemaphore(this->hOutput, 1, 0);
    WaitForSingleObject(this->hOutputThread, 0xFFFFFFFF);
    CloseHandle(this->hOutputThread);
    CloseHandle(this->hOutput);
    CloseHandle(this->hOutputAddLock);
    this->hOutputAddLock = 0;
    SetFilePointer(this->hFile, 0, 0, 0);
    now = this->encodeBuff;
    //avi::RIFF_s::Out(&this->riff, &now);
    this->riff.Out(&now);
    riffSize = &this->encodeBuff[4];
    avi::Out32(&riffSize, this->fileSize - 8);
    sizeOfRiff = now - this->encodeBuff;
    WriteFile(this->hFile, this->encodeBuff, sizeOfRiff, &bytes, 0);
    strcpy((char *)this->encodeBuff, "LISTXXXXmovi");
    listSize = &this->encodeBuff[4];
    avi::Out32(&listSize, dataSize - sizeOfRiff - 8);
    WriteFile(this->hFile, this->encodeBuff, 0xCu, &bytes, 0);
    CloseHandle(this->hFile);
}

void avi::idx1_s::Out(unsigned __int8 **out)
{
    int i; // [esp+4h] [ebp-8h]
    unsigned __int8 *sizePtr; // [esp+8h] [ebp-4h] BYREF

    **out = 'i';
    (*out)[1] = 'd';
    (*out)[2] = 'x';
    (*out)[3] = '1';
    *out += 4;
    sizePtr = *out;
    avi::Out32(out, 0);
    for (i = 0; i < this->count; ++i)
    {
        if (this->sizes[i] >= 0)
        {
            **out = '0';
            (*out)[1] = '1';
            (*out)[2] = 'w';
            (*out)[3] = 'b';
        }
        else
        {
            **out = '0';
            (*out)[1] = '0';
            (*out)[2] = 'd';
            (*out)[3] = 'c';
        }
        *out += 4;
        avi::Out32(out, 0x10u);
        avi::Out32(out, this->offsets[i]);
        avi::Out32(out, this->sizes[i] & 0x7FFFFFFF);
    }
    avi::Out32(&sizePtr, *out - sizePtr - 4);
}

void avi_s::flush_samples()
{
    int v2; // [esp+4h] [ebp-18h]
    unsigned __int8 *now; // [esp+18h] [ebp-4h] BYREF

    now = this->audioBufferHeaders;
    this->audioBufferHeaders[0] = '0';
    now[1] = '1';
    now[2] = 'w';
    now[3] = 'b';
    now += 4;
    avi::Out32(&now, 2 * this->audioBufferCount);
    this->riff.list.strl_aud.strh.Length += this->audioBufferCount;
    v2 = 2 * this->audioBufferCount;
    this->riff.idx1.offsets[this->riff.idx1.count] = avi_s::output_data(this->audioBufferHeaders, v2 + 8);
    this->riff.idx1.sizes[this->riff.idx1.count++] = v2;
    this->audioBufferCount = 0;
}

void __cdecl mjpeg_add_samples(__int16 *in, int count)
{
    if ( mjpeg_run_encoder )
        AVI->add_samples(in, count);
}

void avi_s::add_samples(__int16 *in, int count)
{
    __int16 v3; // [esp+1Eh] [ebp-Ah]
    int i; // [esp+20h] [ebp-8h]
    unsigned __int8 *now; // [esp+24h] [ebp-4h]
    unsigned __int8 *nowa; // [esp+24h] [ebp-4h]

    if ( this->riff.list.avih.TotalFrames >= this->riff.list.strl_vid.strh.Rate
                                                                                 * (count / 2 + this->audioBufferCount + this->riff.list.strl_aud.strh.Length)
                                                                                 / 0x5DC0 )
    {
        now = (unsigned __int8 *)&this->audioBuffer[this->audioBufferCount];
        for ( i = 0; i < count; i += 2 )
        {
            v3 = in[i];
            *now = v3;
            nowa = now + 1;
            *nowa = HIBYTE(v3);
            now = nowa + 1;
            ++this->audioBufferCount;
        }
        if ( this->audioBufferCount >= 24000 )
            avi_s::flush_samples();
    }
    else
    {
        Com_PrintWarning(16, "WARNING: Movie encoding. Sound out of sync with video, droping samples\n");
    }
}

void avi::RIFF_s::LIST_hdrl_s::LIST_aud_strl_s::strh_s::Out(unsigned __int8 **out)
{
    unsigned __int16 rBottom;
    unsigned __int16 rRight;
    unsigned __int16 rTop;
    unsigned __int16 rLeft;
    unsigned __int16 Language;
    unsigned __int16 Priority;
    unsigned int k;
    unsigned int j;
    unsigned int i;
    unsigned __int8 *sizePtr;

    for (i = 0; i < 4; ++i)
        *(*out)++ = this->header[i];

    sizePtr = *out;
    avi::Out32(out, this->size);

    for (j = 0; j < 4; ++j)
        *(*out)++ = this->type[j];

    for (k = 0; k < 4; ++k)
        *(*out)++ = this->format[k];

    avi::Out32(out, this->Flags);

    Priority = this->Priority;
    *(*out)++ = Priority;
    *(*out)++ = HIBYTE(Priority);

    Language = this->Language;
    *(*out)++ = Language;
    *(*out)++ = HIBYTE(Language);

    avi::Out32(out, this->InitialFrames);
    avi::Out32(out, this->Scale);
    avi::Out32(out, this->Rate);
    avi::Out32(out, this->Start);
    avi::Out32(out, this->Length);
    avi::Out32(out, this->SuggestedBufferSize);
    avi::Out32(out, this->Quality);
    avi::Out32(out, this->SampleSize);

    rLeft = this->rLeft;
    *(*out)++ = rLeft;
    *(*out)++ = HIBYTE(rLeft);

    rTop = this->rTop;
    *(*out)++ = rTop;
    *(*out)++ = HIBYTE(rTop);

    rRight = this->rRight;
    *(*out)++ = rRight;
    *(*out)++ = HIBYTE(rRight);

    rBottom = this->rBottom;
    *(*out)++ = rBottom;
    *(*out)++ = HIBYTE(rBottom);

    avi::Out32(&sizePtr, *out - sizePtr - 4);
}