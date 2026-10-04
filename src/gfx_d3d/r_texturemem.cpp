#include "r_texturemem.h"
#include <win32/win_main.h>
#include <universal/assertive.h>
#include "r_init.h"
#include <qcommon/common.h>

DDraw ddraw;

DDraw *DDraw::Instance()
{
    return &ddraw;
}

DDraw::DDraw()
{
    this->pDirectDrawCreateEx = 0;
    this->pDirectDrawEnumerateEx = 0;
    this->hDDraw = 0;
    this->hDDraw = LoadLibraryA("ddraw.dll");
    if (this->hDDraw)
    {
        this->pDirectDrawCreateEx = (HRESULT(__stdcall *)(_GUID *, void **, const _GUID *, IUnknown *))GetProcAddress(this->hDDraw, "DirectDrawCreateEx");
        this->pDirectDrawEnumerateEx = (HRESULT(__stdcall *)(int(__stdcall *)(_GUID *, char *, char *, void *, HMONITOR__ *), void *, unsigned int))GetProcAddress(this->hDDraw, "DirectDrawEnumerateExA");
        this->Initialized = 1;
    }
}

unsigned int __cdecl R_AvailableTextureMemory()
{
    unsigned int texMemInMegs; // [esp+0h] [ebp-Ch]
    unsigned int vidMemInMegs; // [esp+8h] [ebp-4h]

    if ( !dx.device
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_texturemem.cpp", 228, 0, "%s", "dx.device") )
    {
        __debugbreak();
    }
    vidMemInMegs = R_VideoMemory();
    //texMemInMegs = (unsigned int)((int (__thiscall *)(IDirect3DDevice9 *, IDirect3DDevice9 *))dx.device->GetAvailableTextureMem)(
    //                                                             dx.device,
    //                                                             dx.device) >> 20;
    texMemInMegs = dx.device->GetAvailableTextureMem() >> 20;
    if ( vidMemInMegs )
    {
        Com_Printf(
            8,
            "DirectX reports %i MB of video memory and %i MB of available texture memory.\n",
            vidMemInMegs,
            texMemInMegs);
        if ( vidMemInMegs >= texMemInMegs )
        {
            return texMemInMegs;
        }
        else
        {
            Com_Printf(8, "Using video memory size to cap used texture memory at %i MB.\n", vidMemInMegs - 16);
            return vidMemInMegs - 16;
        }
    }
    else
    {
        Com_Printf(
            8,
            "DirectX reports %i MB of available texture memory, but wouldn't tell available video memory.\n",
            texMemInMegs);
        return texMemInMegs;
    }
}

unsigned int __cdecl R_VideoMemory()
{
    const DDraw *v1; // [esp+8h] [ebp-10h]
    const DDraw *v2; // [esp+Ch] [ebp-Ch]
    unsigned int total; // [esp+10h] [ebp-8h] BYREF
    unsigned int size; // [esp+14h] [ebp-4h]

    total = R_VideoMemoryForDevice(0);
    if ( total )
    {
        Com_Printf(8, "Video memory for device: %i MB.\n", total >> 20);
    }
    else
    {
        v2 = DDraw::Instance();
        if ( v2->pDirectDrawEnumerateEx )
            v2->pDirectDrawEnumerateEx(
                (int (__stdcall *)(_GUID *, char *, char *, void *, HMONITOR__ *))R_DDEnumCallback,
                &total,
                0);
        if ( total )
        {
            Com_Printf(8, "Enumerated video memory for device: %i MB.\n", total >> 20);
        }
        else
        {
            total = R_DrasticVideoMemoryForDevice(0);
            if ( total )
            {
                Com_Printf(8, "Drastic video memory for device: %i MB.\n", total >> 20);
            }
            else
            {
                v1 = DDraw::Instance();
                if ( v1->pDirectDrawEnumerateEx )
                    v1->pDirectDrawEnumerateEx(
                        (int (__stdcall *)(_GUID *, char *, char *, void *, HMONITOR__ *))R_DDEnumDrasticCallback,
                        &total,
                        0);
                if ( !total )
                {
                    Com_Printf(8, "Failed to get video memory size.\n");
                    return 0;
                }
                Com_Printf(8, "Enumerated drastic video memory for device: %i MB.\n", total >> 20);
            }
        }
    }
    total = ((total - 1) >> 20) + 1;
    for ( size = 1; size < total; size *= 2 )
        ;
    if ( size - total > 0x20 )
        size >>= 1;
    Com_Printf(8, "Rounded video memory size: %i MB.\n", size);
    return size;
}

//const DDraw *__cdecl DDraw::Instance()
//{
//    if ( (`DDraw::Instance'::`2'::`local static guard' & 1) == 0 )
//    {
//        `DDraw::Instance'::`2'::`local static guard' |= 1u;
//        DDraw::DDraw(&`DDraw::Instance'::`2'::ddraw);
//        atexit(`DDraw::Instance'::`2'::`dynamic atexit destructor for 'ddraw'');
//    }
//    return &`DDraw::Instance'::`2'::ddraw;
//}
//
//DDraw *__thiscall DDraw::DDraw(DDraw *this)
//{
//    this->pDirectDrawCreateEx = 0;
//    this->pDirectDrawEnumerateEx = 0;
//    this->hDDraw = 0;
//    this->hDDraw = LoadLibraryA("ddraw.dll");
//    if ( this->hDDraw )
//    {
//        this->pDirectDrawCreateEx = (HRESULT (__stdcall *)(_GUID *, void **, const _GUID *, IUnknown *))GetProcAddress(this->hDDraw, "DirectDrawCreateEx");
//        this->pDirectDrawEnumerateEx = (HRESULT (__stdcall *)(int (__stdcall *)(_GUID *, char *, char *, void *, HMONITOR__ *), void *, unsigned int))GetProcAddress(this->hDDraw, "DirectDrawEnumerateExA");
//        this->Initialized = 1;
//    }
//    return this;
//}

unsigned int __cdecl R_VideoMemoryForDevice(_GUID *lpGUID)
{
    HRESULT v2; // [esp+0h] [ebp-28h]
    const DDraw *v3; // [esp+4h] [ebp-24h]
    _DDSCAPS2 caps; // [esp+8h] [ebp-20h] BYREF
    HRESULT hr; // [esp+18h] [ebp-10h]
    IDirectDraw7 *dd = NULL; // [esp+1Ch] [ebp-Ch] BYREF
    DWORD total; // [esp+20h] [ebp-8h] BYREF
    DWORD free; // [esp+24h] [ebp-4h] BYREF

    v3 = DDraw::Instance();
    if ( v3->pDirectDrawCreateEx )
        v2 = v3->pDirectDrawCreateEx(lpGUID, (void **)&dd, &IID_IDirectDraw7, 0);
    else
        v2 = -1;
    hr = v2;
    if ( v2 < 0 )
        return 0;
    memset(&caps.dwCaps2, 0, 12);
    caps.dwCaps = 0x10000000;
    hr = dd->GetAvailableVidMem(&caps, &total, &free);
    //((void (__thiscall *)(IDirectDraw7 *, IDirectDraw7 *))dd->Release)(dd, dd);
    dd->Release();
    if ( hr >= 0 )
        return total;
    else
        return 0;
}

int __stdcall R_DDEnumCallback(
                _GUID *lpGUID,
                char *lpDriverDescription,
                char *lpDriverName,
                unsigned int *lpContext,
                HMONITOR__ *hm)
{
    unsigned int total; // [esp+0h] [ebp-4h]

    if ( hm )
        return 1;
    total = R_VideoMemoryForDevice(lpGUID);
    if ( *lpContext < total )
        *lpContext = total;
    return 1;
}

unsigned int __cdecl R_DrasticVideoMemoryForDevice(_GUID *lpGUID)
{
    HMODULE ModuleHandleA; // eax
    HRESULT v3; // [esp+0h] [ebp-2Ch]
    const DDraw *v4; // [esp+4h] [ebp-28h]
    _DDSCAPS2 caps; // [esp+8h] [ebp-24h] BYREF
    HRESULT hr; // [esp+18h] [ebp-14h]
    IDirectDraw7 *dd = NULL; // [esp+1Ch] [ebp-10h] BYREF
    DWORD total; // [esp+20h] [ebp-Ch] BYREF
    HWND__ *hwndDummy; // [esp+24h] [ebp-8h]
    DWORD free; // [esp+28h] [ebp-4h] BYREF

    if ( Sys_IsHeadlessClient() ) // zombies: a headless client never takes DirectDraw exclusive fullscreen mode
        return 0;
    v4 = DDraw::Instance();
    if ( v4->pDirectDrawCreateEx )
        v3 = v4->pDirectDrawCreateEx(lpGUID, (void **)&dd, &IID_IDirectDraw7, 0);
    else
        v3 = -1;
    hr = v3;
    if ( v3 < 0 )
        return 0;
    ModuleHandleA = GetModuleHandleA(0);
    hwndDummy = CreateWindowExA(0, "static", "dummy", 0, 0, 0, 1, 1, 0, 0, ModuleHandleA, 0);
    if ( hwndDummy )
    {
        //hr = ((int (__thiscall *)(IDirectDraw7 *, IDirectDraw7 *, HWND__ *, int))dd->SetCooperativeLevel)(
        //             dd,
        //             dd,
        //             hwndDummy,
        //             17);
        hr = dd->SetCooperativeLevel(hwndDummy, 17);
        if ( hr >= 0 )
        {
            memset(&caps.dwCaps2, 0, 12);
            caps.dwCaps = 0x4000;
            hr = dd->GetAvailableVidMem(&caps, &total, &free);
            DestroyWindow(hwndDummy);
            //((void (__thiscall *)(IDirectDraw7 *, IDirectDraw7 *))dd->Release)(dd, dd);
            dd->Release();
            if ( hr >= 0 )
                return total;
            else
                return 0;
        }
        else
        {
            DestroyWindow(hwndDummy);
            dd->Release();
            return 0;
        }
    }
    else
    {
        dd->Release();
        return 0;
    }
}

int __stdcall R_DDEnumDrasticCallback(
                _GUID *lpGUID,
                char *lpDriverDescription,
                char *lpDriverName,
                unsigned int *lpContext,
                HMONITOR__ *hm)
{
    unsigned int total; // [esp+0h] [ebp-4h]

    total = R_DrasticVideoMemoryForDevice(lpGUID);
    if ( *lpContext < total )
        *lpContext = total;
    return 1;
}

