#pragma once

struct GfxPixelCostColorCode // sizeof=0x8
{                                       // XREF: RB_PixelCost_BuildColorCodeMap(uchar (* const)[4],int)+65/r
    unsigned __int8 count;              // XREF: RB_PixelCost_BuildColorCodeMap(uchar (* const)[4],int)+10E/r
    unsigned __int8 r;                  // XREF: RB_PixelCost_BuildColorCodeMap(uchar (* const)[4],int)+9E/r
    unsigned __int8 g;                  // XREF: RB_PixelCost_BuildColorCodeMap(uchar (* const)[4],int)+8E/r
    unsigned __int8 b;                  // XREF: RB_PixelCost_BuildColorCodeMap(uchar (* const)[4],int)+7E/r
    const char *colorName;              // XREF: R_PixelCost_PrintColorCodeKey(void)+4C/r
};

void __cdecl RB_PixelCost_BuildColorCodeMap(unsigned __int8 (*pixels)[4], int pixelCount);
void __cdecl R_PixelCost_PrintColorCodeKey();
