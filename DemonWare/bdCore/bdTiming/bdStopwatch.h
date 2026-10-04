#pragma once

#include <DemonWare/bdPlatform/bdPlatformTiming/bdPlatformTiming.h>

struct bdStopwatch // sizeof=0x8
{                                       // XREF: bdRemoteTask/r
    unsigned __int64 m_start;

    bdStopwatch();

    inline void start()
    {
        m_start = bdPlatformTiming::getHiResTimeStamp().QuadPart;
    }

    inline void reset()
    {
        m_start = 0;
    }

    double getElapsedTimeInSeconds() const 
    {
        LARGE_INTEGER HiResTimeStamp; // [esp+8h] [ebp-8h]

        if (!this->m_start)
            return 0.0;
        HiResTimeStamp = bdPlatformTiming::getHiResTimeStamp();
        return bdPlatformTiming::getElapsedTime(this->m_start, HiResTimeStamp.QuadPart);
    }
};