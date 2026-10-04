#pragma once
#include "r_material.h"
#include <win32/win_common.h>

struct StreamAllocGlob // sizeof=0x109
{                                       // XREF: .data:s_allocGlob/r
    FastCriticalSection lock;
    GfxImage *tempImages[64];           // XREF: R_StreamAlloc_InitTempImages(void)+33/w
                                        // R_StreamAlloc_InitTempImages(void)+3D/r ...
    bool tempImagesInit;                // XREF: R_StreamAlloc_InitTempImages(void)+6/r
};


void __cdecl R_StreamAlloc_InitTempImages();
GfxImage *__cdecl R_StreamAlloc_SetupTempImage(
                _D3DFORMAT format,
                bool linear,
                unsigned __int16 width,
                unsigned __int16 height);
void __cdecl R_StreamAlloc_ReleaseTempImage(GfxImage *image);
void __cdecl R_StreamAlloc_Lock();
void __cdecl R_StreamAlloc_Unlock();
char __cdecl R_StreamAlloc_CanAllocate(int size, float importance, GfxImage **unloadImage);
char __cdecl R_StreamAlloc_FreeImageByImportance(int size, float importance, GfxImage **unloadImage);
unsigned __int8 *__cdecl R_StreamAlloc_FreeImage(GfxImage *image, int imagePart, bool dirty, int *freedSize);
void __cdecl R_StreamAlloc_Flush();

extern int list_count;
