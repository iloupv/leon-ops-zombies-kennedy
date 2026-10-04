#pragma once

#include <ddraw.h>

// genius in training
struct __declspec(align(4)) DDraw // sizeof=0x14
{                    
    HRESULT (__stdcall *pDirectDrawCreateEx)(_GUID *, void **, const _GUID *, IUnknown *);
    HRESULT (__stdcall *pDirectDrawEnumerateEx)(int (__stdcall *)(_GUID *, char *, char *, void *, HMONITOR__ *), void *, unsigned int);
    HINSTANCE__ *hDDraw;                // XREF: `DDraw::Instance(void)'::`2'::`dynamic atexit destructor for 'ddraw''(void)+17/r
    _GUID *lpGUID;
    bool Initialized;
    // padding byte
    // padding byte
    // padding byte

    DDraw();
    //~DDraw();
    static DDraw *Instance();
};

unsigned int __cdecl R_AvailableTextureMemory();
unsigned int __cdecl R_VideoMemory();
//const DDraw *__cdecl DDraw::Instance();
//DDraw *__thiscall DDraw::DDraw(DDraw *this);
unsigned int __cdecl R_VideoMemoryForDevice(_GUID *lpGUID);
int __stdcall R_DDEnumCallback(
                _GUID *lpGUID,
                char *lpDriverDescription,
                char *lpDriverName,
                unsigned int *lpContext,
                HMONITOR__ *hm);
unsigned int __cdecl R_DrasticVideoMemoryForDevice(_GUID *lpGUID);
int __stdcall R_DDEnumDrasticCallback(
                _GUID *lpGUID,
                char *lpDriverDescription,
                char *lpDriverName,
                unsigned int *lpContext,
                HMONITOR__ *hm);
