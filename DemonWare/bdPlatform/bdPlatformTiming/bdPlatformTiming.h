#pragma once

#include <Windows.h>

namespace bdPlatformTiming
{
    inline LARGE_INTEGER __cdecl getHiResTimeStamp()
    {
        LARGE_INTEGER PerformanceCount; // [esp+0h] [ebp-8h] BYREF

        QueryPerformanceCounter(&PerformanceCount);
        return PerformanceCount;
    }

    inline double __cdecl getElapsedTime(unsigned __int64 a1, unsigned __int64 a2)
    {
        LARGE_INTEGER Frequency; // [esp+14h] [ebp-10h] BYREF

        QueryPerformanceFrequency(&Frequency);
        return (float)((double)(__int64)(a2 - a1) / (double)Frequency.QuadPart);
   }

    inline void sleep(DWORD dwMilliseconds)
    {
        Sleep(dwMilliseconds);
    }

    inline unsigned __int64 getLoResElapsedTime(unsigned int a1, unsigned int a2)
    {
        __int64 v3; // [esp+8h] [ebp-8h]

        v3 = a2;
        if (a2 < a1)
            v3 = a2 + 0xFFFFFFFFLL;
        return (v3 - (unsigned __int64)a1) / 0x3E8;
    }
}