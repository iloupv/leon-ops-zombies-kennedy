#include "timing.h"

#include <Windows.h>

long double msecPerRawTimerTick;

void __cdecl InitTiming()
{
    msecPerRawTimerTick = SecondsPerTick() * 1000.0;
}

double __cdecl SecondsPerTick()
{
    _LARGE_INTEGER qpcFrequency; // [esp+0h] [ebp-10h] BYREF

    Sleep(0);
    QueryPerformanceFrequency(&qpcFrequency);
    return 1.0 / (double)qpcFrequency.QuadPart;
}

