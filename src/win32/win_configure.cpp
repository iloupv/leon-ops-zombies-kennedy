#include "win_configure.h"

#include <Windows.h>
#include "win_localize.h"
#include "win_main.h"
#include <d3d9.h>

int __cdecl Sys_SystemMemoryMB()
{
    HWND ActiveWindow; // eax
    HWND v2; // eax
    char *v3; // [esp-Ch] [ebp-C8h]
    char *v4; // [esp-Ch] [ebp-C8h]
    char *v5; // [esp-8h] [ebp-C4h]
    char *v6; // [esp-8h] [ebp-C4h]
    float v7; // [esp+30h] [ebp-8Ch]
    float v8; // [esp+40h] [ebp-7Ch]
    int sysMB; // [esp+50h] [ebp-6Ch]
    int sysMBa; // [esp+50h] [ebp-6Ch]
    HINSTANCE__ *hm; // [esp+54h] [ebp-68h]
    _MEMORYSTATUS status; // [esp+58h] [ebp-64h] BYREF
    int (__stdcall *MemStatEx)(_MEMORYSTATUSEX *); // [esp+78h] [ebp-44h]
    _MEMORYSTATUSEX statusEx; // [esp+7Ch] [ebp-40h] BYREF

    hm = GetModuleHandleA("kernel32.dll");
    if ( hm && (MemStatEx = (int (__stdcall *)(_MEMORYSTATUSEX *))GetProcAddress(hm, "GlobalMemoryStatusEx")) != 0 )
    {
        statusEx.dwLength = 64;
        MemStatEx(&statusEx);
        if (statusEx.ullAvailVirtual < 0x8000000 && !Sys_IsHeadless()) // zombies: headless never prompts
        {
            v5 = Win_LocalizeRef("WIN_LOW_MEMORY_TITLE");
            v3 = Win_LocalizeRef("WIN_LOW_MEMORY_BODY");
            ActiveWindow = GetActiveWindow();
            if ( MessageBoxA(ActiveWindow, v3, v5, 0x34u) != 6 )
            {
                Sys_NormalExit();
                exit(0);
            }
        }
        v8 = (double)statusEx.ullTotalPhys * 0.00000095367432;
        sysMB = (int)(v8 + 0.4999999990686774);
        if ( (double)statusEx.ullTotalPhys > (double)sysMB * 1048576.0 || sysMB > 1024 )
            return 1024;
        return sysMB;
    }
    else
    {
        status.dwLength = 32;
        GlobalMemoryStatus(&status);
        if (status.dwAvailVirtual < 0x8000000 && !Sys_IsHeadless()) // zombies: headless never prompts
        {
            v6 = Win_LocalizeRef("WIN_LOW_MEMORY_TITLE");
            v4 = Win_LocalizeRef("WIN_LOW_MEMORY_BODY");
            v2 = GetActiveWindow();
            if ( MessageBoxA(v2, v4, v6, 0x34u) != 6 )
            {
                Sys_NormalExit();
                exit(0);
            }
        }
        v7 = (double)status.dwTotalPhys * 0.00000095367432;
        sysMBa = (int)(v7 + 0.4999999990686774);
        if ( (double)status.dwTotalPhys > (double)sysMBa * 1048576.0 || sysMBa > 1024 )
            return 1024;
        return sysMBa;
    }
}

void __cdecl Sys_DetectVideoCard(int descLimit, char *description)
{
    char v2; // dl
    char *v3; // [esp+8h] [ebp-468h]
    char *v4; // [esp+Ch] [ebp-464h]
    IDirect3D9 *d3d9; // [esp+14h] [ebp-45Ch]
    _D3DADAPTER_IDENTIFIER9 id; // [esp+18h] [ebp-458h] BYREF

    if ( descLimit != 512
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\win32\\win_configure.cpp",
                    85,
                    0,
                    "%s\n\t(descLimit) = %i",
                    "(descLimit == sizeof( id.Description ))",
                    descLimit) )
    {
        __debugbreak();
    }
    strcpy(description, "Unknown video card");
    d3d9 = Direct3DCreate9(0x20u);
    if ( d3d9 )
    {
        if ( d3d9->GetAdapterIdentifier(0, 0, &id) >= 0 )
        {
            v4 = id.Description;
            v3 = description;
            do
            {
                v2 = *v4;
                *v3++ = *v4++;
            }
            while ( v2 );
        }
        //((void (__thiscall *)(IDirect3D9 *, IDirect3D9 *))d3d9->Release)(d3d9, d3d9);
        d3d9->Release();
    }
}

void __cdecl Sys_DetectCpuVendorAndName(char *vendor, char *name)
{
#if 0
    bool v12; // cf
    bool v13; // cf
    unsigned int v29[5]; // [esp+0h] [ebp-90h] BYREF
    int v30; // [esp+14h] [ebp-7Ch]
    unsigned __int8 v31; // [esp+1Ah] [ebp-76h]
    unsigned __int8 v32; // [esp+1Bh] [ebp-75h]
    char *v33; // [esp+1Ch] [ebp-74h]
    const char *v34; // [esp+20h] [ebp-70h]
    int v35; // [esp+24h] [ebp-6Ch]
    int v36; // [esp+28h] [ebp-68h]
    unsigned __int8 v37; // [esp+2Eh] [ebp-62h]
    unsigned __int8 v38; // [esp+2Fh] [ebp-61h]
    char *v39; // [esp+30h] [ebp-60h]
    const char *v40; // [esp+34h] [ebp-5Ch]
    Sys_DetectCpuVendorAndName::__l2::<unnamed_type_cpuid_desc> cpuid_desc; // [esp+38h] [ebp-58h] BYREF
    Sys_DetectCpuVendorAndName::__l2::<unnamed_type_cpuid_vendor> cpuid_vendor; // [esp+6Ch] [ebp-24h] BYREF
    unsigned int maxCpuidArg; // [esp+7Ch] [ebp-14h]
    unsigned int *v44; // [esp+80h] [ebp-10h]
    int v45; // [esp+8Ch] [ebp-4h]

    v44 = v29;
    v45 = 0;
    __asm { pushaw }
    _EAX = 0;
    __asm { cpuid }
    cpuid_vendor.reg.ebx = _EBX;
    *(_QWORD *)&cpuid_vendor.name[4] = __PAIR64__(_ECX, _EDX);
    _EAX = 0x80000000;
    __asm { cpuid }
    maxCpuidArg = _EAX;
    __asm { popaw }
    Sys_CopyCpuidString(vendor, (const char *)&cpuid_vendor, 0xCu);
    if ( maxCpuidArg >= 0x80000004 )
    {
        __asm { pushaw }
        _EAX = -2147483646;
        __asm { cpuid }
        *(_QWORD *)&cpuid_desc.s.reg0.eax = __PAIR64__(_EBX, _EAX);
        *(_QWORD *)&cpuid_desc.string[8] = __PAIR64__(_EDX, _ECX);
        _EAX = -2147483645;
        __asm { cpuid }
        *(_QWORD *)&cpuid_desc.string[16] = __PAIR64__(_EBX, _EAX);
        *(_QWORD *)&cpuid_desc.string[24] = __PAIR64__(_EDX, _ECX);
        _EAX = -2147483644;
        __asm { cpuid }
        *(_QWORD *)&cpuid_desc.string[32] = __PAIR64__(_EBX, _EAX);
        *(_QWORD *)&cpuid_desc.string[40] = __PAIR64__(_EDX, _ECX);
        __asm { popaw }
        Sys_CopyCpuidString(name, (const char *)&cpuid_desc, 0x30u);
    }
    else
    {
        v40 = "GenuineIntel";
        v39 = vendor;
        while ( 1 )
        {
            v38 = *v39;
            v12 = v38 < (unsigned int)*v40;
            if ( v38 != *v40 )
                break;
            if ( !v38 )
                goto LABEL_7;
            v37 = v39[1];
            v12 = v37 < (unsigned int)v40[1];
            if ( v37 != v40[1] )
                break;
            v39 += 2;
            v40 += 2;
            if ( !v37 )
            {
LABEL_7:
                v36 = 0;
                goto LABEL_9;
            }
        }
        v36 = -v12 - (v12 - 1);
LABEL_9:
        v35 = v36;
        if ( v36 )
        {
            v34 = "AuthenticAMD";
            v33 = vendor;
            while ( 1 )
            {
                v32 = *v33;
                v13 = v32 < (unsigned int)*v34;
                if ( v32 != *v34 )
                    break;
                if ( !v32 )
                    goto LABEL_16;
                v31 = v33[1];
                v13 = v31 < (unsigned int)v34[1];
                if ( v31 != v34[1] )
                    break;
                v33 += 2;
                v34 += 2;
                if ( !v31 )
                {
LABEL_16:
                    v30 = 0;
                    goto LABEL_18;
                }
            }
            v30 = -v13 - (v13 - 1);
LABEL_18:
            v29[4] = v30;
            if ( v30 )
            {
                *(unsigned int *)name = *(unsigned int *)aUnkn_0;
                *((unsigned int *)name + 1) = 544110447;
                *((unsigned int *)name + 2) = (char *)&loc_55503F + 4;
            }
            else
            {
                *(unsigned int *)name = *(unsigned int *)aUnkn_3;
                *((unsigned int *)name + 1) = 544110447;
                *((unsigned int *)name + 2) = 541347137;
                *((unsigned int *)name + 3) = (char *)&loc_55503F + 4;
            }
        }
        else
        {
            strcpy(name, "Unknown Intel CPU");
        }
    }
#endif
    // BO1TODO?
    strcpy(name, "BO1Tech KT-6500 (Black Edition)");
    strcpy(vendor, "SwagHardware");
}

void __cdecl Sys_CopyCpuidString(char *dest, const char *source, unsigned int maxLen)
{
    unsigned int from; // [esp+0h] [ebp-8h]
    unsigned int to; // [esp+4h] [ebp-4h]

    for ( from = 0; from < maxLen && source[from] == 32; ++from )
        ;
    to = 0;
    while ( from < maxLen && source[from] )
        dest[to++] = source[from++];
    dest[to] = 0;
}

bool __cdecl Sys_SupportsSSE()
{
    return true;
    //_EAX = 1;
    //__asm { cpuid }
    //return ((unsigned int)&cls.wagerServers[5331].basictraining & _EDX) != 0;
}

void __cdecl Sys_SetAutoConfigureGHz(SysInfo *sysInfo)
{
    double multiCpuFactor; // [esp+8h] [ebp-8h]

    Sys_GetPhysicalCpuCount(sysInfo);
    if ( sysInfo->physicalCpuCount < 1
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\win32\\win_configure.cpp",
                    493,
                    0,
                    "sysInfo->physicalCpuCount >= 1\n\t%i, %i",
                    sysInfo->physicalCpuCount,
                    1) )
    {
        __debugbreak();
    }
    if ( sysInfo->physicalCpuCount == 1 )
    {
        multiCpuFactor = 1.0;
    }
    else if ( sysInfo->physicalCpuCount == 2 )
    {
        multiCpuFactor = 1.75;
    }
    else
    {
        multiCpuFactor = 2.0;
    }
    sysInfo->configureGHz = Sys_BenchmarkGHz() * multiCpuFactor;
}

void __cdecl Sys_GetPhysicalCpuCount(SysInfo *sysInfo)
{
#if 0
    bool v1; // cf
    int v8; // eax
    unsigned int v14[5]; // [esp+0h] [ebp-E8h] BYREF
    int v15; // [esp+14h] [ebp-D4h]
    unsigned __int8 v16; // [esp+1Ah] [ebp-CEh]
    unsigned __int8 v17; // [esp+1Bh] [ebp-CDh]
    char *cpuVendor; // [esp+1Ch] [ebp-CCh]
    const char *v19; // [esp+20h] [ebp-C8h]
    int v20; // [esp+28h] [ebp-C0h]
    int v21; // [esp+2Ch] [ebp-BCh]
    unsigned int regEdx; // [esp+30h] [ebp-B8h]
    unsigned int regEax; // [esp+34h] [ebp-B4h]
    unsigned int regEbx; // [esp+38h] [ebp-B0h]
    void *process; // [esp+3Ch] [ebp-ACh]
    unsigned int apicId; // [esp+40h] [ebp-A8h]
    unsigned int logicalPerPhysical; // [esp+44h] [ebp-A4h]
    unsigned int systemAffinityMask; // [esp+48h] [ebp-A0h] BYREF
    unsigned int logicalIdMask; // [esp+4Ch] [ebp-9Ch]
    unsigned int processAffinityMask; // [esp+50h] [ebp-98h] BYREF
    unsigned int testAffinityMask; // [esp+54h] [ebp-94h]
    unsigned int existingApicId[32]; // [esp+58h] [ebp-90h] BYREF
    unsigned int *v33; // [esp+D8h] [ebp-10h]
    int v34; // [esp+E4h] [ebp-4h]

    v33 = v14;
    sysInfo->physicalCpuCount = sysInfo->logicalCpuCount;
    v19 = "GenuineIntel";
    cpuVendor = sysInfo->cpuVendor;
    while ( 1 )
    {
        v17 = *cpuVendor;
        v1 = v17 < (unsigned int)*v19;
        if ( v17 != *v19 )
            break;
        if ( !v17 )
            goto LABEL_6;
        v16 = cpuVendor[1];
        v1 = v16 < (unsigned int)v19[1];
        if ( v16 != v19[1] )
            break;
        cpuVendor += 2;
        v19 += 2;
        if ( !v16 )
        {
LABEL_6:
            v15 = 0;
            goto LABEL_8;
        }
    }
    v15 = -v1 - (v1 - 1);
LABEL_8:
    v14[4] = v15;
    if ( !v15 )
    {
        _EAX = 1;
        __asm { cpuid }
        regEax = _EAX;
        regEbx = _EBX;
        regEdx = _EDX;
        v34 = -1;
        if ( (_EAX & 0xF00) == 0xF00
            || ((unsigned int)&cg_bgsAnim.animScriptData.scriptItems[416].commands[0].tagName & regEax) != 0 )
        {
            logicalPerPhysical = BYTE2(regEbx);
            if ( BYTE2(regEbx) )
            {
                if ( logicalPerPhysical != 255 )
                {
                    if ( !_BitScanReverse((unsigned int *)&v8, logicalPerPhysical) )
                        v8 = `CountLeadingZeros'::`2'::notFound;
                    v21 = v8 ^ 0x1F;
                    v20 = 1 << (32 - (v8 ^ 0x1F));
                    logicalIdMask = v20 - 1;
                    sysInfo->physicalCpuCount = 0;
                    process = GetCurrentProcess();
                    GetProcessAffinityMask(process, &processAffinityMask, &systemAffinityMask);
                    if ( !processAffinityMask
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\win32\\win_configure.cpp",
                                    315,
                                    0,
                                    "%s",
                                    "processAffinityMask") )
                    {
                        __debugbreak();
                    }
                    if ( !systemAffinityMask
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\win32\\win_configure.cpp",
                                    316,
                                    0,
                                    "%s",
                                    "systemAffinityMask") )
                    {
                        __debugbreak();
                    }
                    if ( (processAffinityMask & ~systemAffinityMask) != 0
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\win32\\win_configure.cpp",
                                    317,
                                    0,
                                    "%s",
                                    "!(processAffinityMask & ~systemAffinityMask)") )
                    {
                        __debugbreak();
                    }
                    for ( testAffinityMask = 1; testAffinityMask && testAffinityMask <= processAffinityMask; testAffinityMask *= 2 )
                    {
                        if ( SetProcessAffinityMask(process, testAffinityMask) )
                        {
                            Sleep(0);
                            _EAX = 1;
                            __asm { cpuid }
                            regEbx = _EBX;
                            apicId = ~logicalIdMask & HIBYTE(_EBX);
                        }
                        else
                        {
                            apicId = -1;
                        }
                        sysInfo->physicalCpuCount = Sys_AddApicIdIfUnique(apicId, existingApicId, sysInfo->physicalCpuCount);
                    }
                    SetProcessAffinityMask(process, processAffinityMask);
                    Sleep(0);
                    if ( (sysInfo->physicalCpuCount < 1 || sysInfo->physicalCpuCount > sysInfo->logicalCpuCount)
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\win32\\win_configure.cpp",
                                    339,
                                    0,
                                    "sysInfo->physicalCpuCount not in [1, sysInfo->logicalCpuCount]\n\t%i not in [%i, %i]",
                                    sysInfo->physicalCpuCount,
                                    1,
                                    sysInfo->logicalCpuCount) )
                    {
                        __debugbreak();
                    }
                }
            }
        }
    }
#endif
    // BO1TODO?

    sysInfo->physicalCpuCount = 8; // HACK!
}

unsigned int __cdecl Sys_AddApicIdIfUnique(
                unsigned int apicId,
                unsigned int *existingApicId,
                unsigned int existingCount)
{
    unsigned int existingIter; // [esp+0h] [ebp-4h]

    for ( existingIter = 0; existingIter < existingCount; ++existingIter )
    {
        if ( existingIter >= 0x20
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\win32\\win_configure.cpp",
                        256,
                        0,
                        "existingIter doesn't index 32\n\t%i not in [0, %i)",
                        existingIter,
                        32) )
        {
            __debugbreak();
        }
        if ( existingApicId[existingIter] == apicId )
            return existingCount;
    }
    existingApicId[existingCount] = apicId;
    return existingCount + 1;
}

long double __cdecl Sys_BenchmarkGHz()
{
    // BO1TODO
#if 0
    unsigned int i; // ecx
    unsigned __int64 v1; // kr00_8
    int holdrand; // [esp+10h] [ebp-68h]
    Sys_BenchmarkGHz::__l2::<unnamed_type_start> start; // [esp+30h] [ebp-48h]
    int priority; // [esp+44h] [ebp-34h]
    unsigned __int64 minTime; // [esp+48h] [ebp-30h]
    unsigned int attempt; // [esp+54h] [ebp-24h]
    float x; // [esp+68h] [ebp-10h]
    float xa; // [esp+68h] [ebp-10h]
    float y; // [esp+6Ch] [ebp-Ch]
    float ya; // [esp+6Ch] [ebp-Ch]
    HANDLE thread; // [esp+70h] [ebp-8h]

    thread = GetCurrentThread();
    priority = GetThreadPriority(thread);
    SetThreadPriority(thread, 15);
    minTime = -1;
    for ( attempt = 0; attempt < 0x3E8; ++attempt )
    {
        Sleep(0);
        start.u64 = __rdtsc();
        holdrand = 0;
        x = 0.25;
        y = 0.75;
        for ( i = 0; i < 0x3E8; ++i )
        {
            xa = (1.0 - x) * x * 2.5999999 + x;
            ya = (1.0 - y) * y * 2.5999999 + y;
            x = (1.0 - xa) * xa * 2.5999999 + xa;
            y = (1.0 - ya) * ya * 2.5999999 + ya;
            if ( (i & 1) != 0 )
                holdrand = 214013 * (214013 * (214013 * holdrand + 2531011) + 2531011) + 2531011;
        }
        v1 = __rdtsc() - start.u64;
        if ( minTime > v1 )
            minTime = v1;
    }
    SetThreadPriority(thread, priority);
    return 0.1010328 / ((double)minTime * msecPerRawTimerTick);
#else
    return 2.4; // swag
#endif
}

