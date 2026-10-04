#pragma once

struct __declspec(align(4)) WaveletDecode // sizeof=0x20
{                                       // XREF: Image_LoadWavelet/r
    unsigned __int16 value;             // XREF: Image_LoadWavelet+A7/w
    unsigned __int16 bit;               // XREF: Image_LoadWavelet+AD/w
    const unsigned __int8 *data;        // XREF: Image_LoadWavelet+189/w
    int width;                          // XREF: Image_LoadWavelet+B8/w
                                        // Image_LoadWavelet+198/r
    int height;                         // XREF: Image_LoadWavelet+C2/w
                                        // Image_LoadWavelet+1D0/r
    int channels;                       // XREF: Image_LoadWavelet+DA/w
    int bpp;                            // XREF: Image_LoadWavelet+E0/w
                                        // Image_LoadWavelet+E7/r ...
    int mipLevel;                       // XREF: Image_LoadWavelet+D4/w
                                        // Image_LoadWavelet:loc_B29A6C/r ...
    bool dataInitialized;               // XREF: Image_LoadWavelet+E3/w
    // padding byte
    // padding byte
    // padding byte
};

struct WaveletHuffmanDecode // sizeof=0x4
{                                       // XREF: .rdata:waveletDecodeBlue/r
    __int16 value;
    __int16 bits;
};

void __cdecl Wavelet_DecompressLevel(unsigned __int8 *src, unsigned __int8 *dst, WaveletDecode *decode);
void __cdecl Wavelet_ConsumeBits(unsigned __int16 bitCount, WaveletDecode *decode);
int __cdecl Wavelet_DecodeValue(
                const WaveletHuffmanDecode *decodeTable,
                unsigned __int16 bitCount,
                int bias,
                WaveletDecode *decode);
void __cdecl Wavelet_AddDeltaToMipmap(
                unsigned __int8 *inout,
                int size,
                WaveletDecode *decode,
                const int *dstChanOffset);
