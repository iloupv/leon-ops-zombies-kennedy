#pragma once

typedef void *MutexHandle;

struct bdPlatformMutex
{
    static MutexHandle createMutex();

    static void lock(MutexHandle &hand);
    static void unlock(MutexHandle &hand);
    static void destroy(MutexHandle &hand);
};

