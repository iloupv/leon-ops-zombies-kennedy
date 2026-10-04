#pragma once

#include <DemonWare/bdPlatform/bdPlatformThread/bdPlatformMutex.h>

struct bdMutex // sizeof=0x4
{                                       // XREF: .data:bdMutex g_MemoryThreadLock/r
    MutexHandle m_handle;

    bdMutex();
    ~bdMutex();

    void unlock();
    void lock();
};