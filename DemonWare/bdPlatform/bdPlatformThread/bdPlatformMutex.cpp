#include "bdPlatformMutex.h"

#include <Windows.h>

MutexHandle bdPlatformMutex::createMutex()
{
    return CreateMutexA(NULL, FALSE, NULL);
}

void bdPlatformMutex::lock(MutexHandle &hand)
{
    WaitForSingleObject(hand, 0xFFFFFFFF);
}

void bdPlatformMutex::unlock(MutexHandle &hand)
{
    ReleaseMutex(hand);
}

void bdPlatformMutex::destroy(MutexHandle &hand)
{
    ReleaseMutex(hand);
    CloseHandle(hand);
}