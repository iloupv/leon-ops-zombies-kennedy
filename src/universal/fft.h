#pragma once

struct complex_s // sizeof=0x8
{                                                                             // XREF: WaterGlob/r
    float real;                                                 // XREF: TransposeArray+4F/w
    // TransposeArray+8A/r ...
    float imag;                                                 // XREF: TransposeArray+52/w
    // TransposeArray+90/r ...
};

void __cdecl FFT_Init(int *fftBitswap, complex_s *fftTrigTable);
void __cdecl FFT(complex_s *data_inout, unsigned int log2_count, int *bitSwap, complex_s *trigTable);
