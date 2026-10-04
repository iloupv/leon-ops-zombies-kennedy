#pragma once

namespace avi
{
    struct idx1_s //avi::idx1_s // sizeof=0x10004
    {                                       // XREF: avi::RIFF_s/r
        int count;
        int offsets[8192];
        int sizes[8192];

        void Out(unsigned __int8 **out);
    };

    struct RIFF_s // sizeof=0x1013C
    {      
        struct LIST_hdrl_s // sizeof=0x12C
        {
            struct avih_s //avi::RIFF_s::LIST_hdrl_s::avih_s // sizeof=0x40
            {                                       // XREF: avi::RIFF_s::LIST_hdrl_s/r
                char header[4];
                unsigned int size;
                unsigned int MicroSecPerFrame;
                unsigned int MaxBytesPerSec;
                unsigned int PaddingGranularity;
                unsigned int Flags;
                unsigned int TotalFrames;
                unsigned int InitialFrames;
                unsigned int Streams;
                unsigned int SuggestedBufferSize;
                unsigned int Width;
                unsigned int Height;
                unsigned int Reserved[4];

                avih_s();
                void Out(unsigned __int8 **out);
            };

            struct LIST_aud_strl_s //avi::RIFF_s::LIST_hdrl_s::LIST_aud_strl_s // sizeof=0x64
            {
                struct strh_s //avi::RIFF_s::LIST_hdrl_s::LIST_aud_strl_s::strh_s // sizeof=0x40
                {                                       // XREF: avi::RIFF_s::LIST_hdrl_s::LIST_aud_strl_s/r
                    char header[4];
                    unsigned int size;
                    char type[4];
                    char format[4];
                    unsigned int Flags;
                    unsigned __int16 Priority;
                    unsigned __int16 Language;
                    unsigned int InitialFrames;
                    unsigned int Scale;
                    unsigned int Rate;
                    unsigned int Start;
                    unsigned int Length;
                    unsigned int SuggestedBufferSize;
                    unsigned int Quality;
                    unsigned int SampleSize;
                    unsigned __int16 rLeft;
                    unsigned __int16 rTop;
                    unsigned __int16 rRight;
                    unsigned __int16 rBottom;

                    strh_s();
                    void Out(unsigned __int8 **out);
                };    
                struct strf_s //avi::RIFF_s::LIST_hdrl_s::LIST_aud_strl_s::strf_s // sizeof=0x18
                {                                       // XREF: avi::RIFF_s::LIST_hdrl_s::LIST_aud_strl_s/r
                    char header[4];
                    unsigned int size;
                    unsigned __int16 format;
                    unsigned __int16 channels;
                    unsigned int samples_per_sec;
                    unsigned int avg_bytes_per_sec;
                    unsigned __int16 block_align;
                    unsigned __int16 bits_per_sample;

                    strf_s();
                    void Out(unsigned __int8 **out);
                };
                char header[4];
                unsigned int size;
                char type[4];
                avi::RIFF_s::LIST_hdrl_s::LIST_aud_strl_s::strh_s strh;
                avi::RIFF_s::LIST_hdrl_s::LIST_aud_strl_s::strf_s strf;

                LIST_aud_strl_s();
                void Out(unsigned __int8 **out);
            };

            struct LIST_vid_strl_s //avi::RIFF_s::LIST_hdrl_s::LIST_vid_strl_s // sizeof=0x7C
            {                                       // XREF: avi::RIFF_s::LIST_hdrl_s/r
                struct strh_s //__cppobj avi::RIFF_s::LIST_hdrl_s::LIST_vid_strl_s::strh_s // sizeof=0x40
                {                                       // XREF: avi::RIFF_s::LIST_hdrl_s::LIST_vid_strl_s/r
                    char header[4];
                    unsigned int size;
                    char type[4];
                    char format[4];
                    unsigned int Flags;
                    unsigned __int16 Priority;
                    unsigned __int16 Language;
                    unsigned int InitialFrames;
                    unsigned int Scale;
                    unsigned int Rate;
                    unsigned int Start;
                    unsigned int Length;
                    unsigned int SuggestedBufferSize;
                    unsigned int Quality;
                    unsigned int SampleSize;
                    unsigned __int16 rLeft;
                    unsigned __int16 rTop;
                    unsigned __int16 rRight;
                    unsigned __int16 rBottom;
                    
                    strh_s();
                    void Out(unsigned __int8 **out);
                };
                struct strf_s //__cppobj avi::RIFF_s::LIST_hdrl_s::LIST_vid_strl_s::strf_s // sizeof=0x30
                {                                       // XREF: avi::RIFF_s::LIST_hdrl_s::LIST_vid_strl_s/r
                    char header[4];
                    unsigned int size;
                    unsigned int sizeBi;
                    unsigned int width;
                    unsigned int height;
                    unsigned __int16 planes;
                    unsigned __int16 bit_count;
                    char compression[4];
                    unsigned int size_image;
                    unsigned int horizontal_ppm;
                    unsigned int vertical_ppm;
                    unsigned int colors_used;
                    unsigned int colors_important;

                    strf_s();
                    void Out(unsigned __int8 **out);
                };
                char header[4];
                unsigned int size;
                char type[4];
                avi::RIFF_s::LIST_hdrl_s::LIST_vid_strl_s::strh_s strh;
                avi::RIFF_s::LIST_hdrl_s::LIST_vid_strl_s::strf_s strf;

                LIST_vid_strl_s();
                void Out(unsigned __int8 **out);
            }; // LIST_vid_strl_s
            char header[4];
            unsigned int size;
            char type[4];
            avi::RIFF_s::LIST_hdrl_s::avih_s avih;
            avi::RIFF_s::LIST_hdrl_s::LIST_vid_strl_s strl_vid;
            avi::RIFF_s::LIST_hdrl_s::LIST_aud_strl_s strl_aud;

            LIST_hdrl_s();
            void Out(unsigned __int8 **out);

        }; // LIST_hdrl_s

        char header[4];
        unsigned int size;
        char type[4];
        avi::RIFF_s::LIST_hdrl_s list;
        avi::idx1_s idx1;

        RIFF_s();
        void Out(unsigned __int8 **out);
        void Setup(unsigned int width, unsigned int height, unsigned int fps);
        void AddFrame(unsigned int offset, unsigned int size);
    }; // RIFF_s

    unsigned int __cdecl Out32(unsigned __int8 **out, unsigned int d);
} // namespace avi

struct __declspec(align(4)) avi_s // sizeof=0xA7870
{
    avi::RIFF_s riff;
    void *hFile;
    unsigned __int8 buffer[262144];
    unsigned __int8 encodeBuff[262144];
    unsigned __int8 audioBufferHeaders[8];
    __int16 audioBuffer[48000];
    int audioBufferCount;
    unsigned __int8 *outputPtr;
    unsigned __int8 *flushToPtr;
    unsigned __int8 *writePtr;
    unsigned int fileSize;
    void *hOutput;
    void *hOutputLock;
    void *hOutputThread;
    void *hOutputAddLock;
    bool threadQuit;
    // padding byte
    // padding byte
    // padding byte

    void write_thread();
    void create(
        const char *szFile,
        unsigned int width,
        unsigned int height,
        unsigned int fps);


    void add_frame(unsigned __int8 *y, unsigned __int8 *u, unsigned __int8 *v);
    unsigned int    output_data(unsigned __int8 *data, unsigned int size);
    void close();
    void flush_samples();
    void add_samples(__int16 *in, int count);
};








void __cdecl mjpeg_create(const char *szFile, int width, int height, unsigned int fps);
unsigned int __stdcall mjpeg_write_thread(void *__formal);
void __cdecl mjpeg_close_int();
void __cdecl mjpeg_add_samples(__int16 *in, int count);
void __cdecl mjpeg_add_frame(unsigned __int8 *y, unsigned __int8 *u, unsigned __int8 *v);
