#include "bdMemory.h"
#include <cstddef>
#include <DemonWare/bdCore/bdThread/bdMutex.h>

static bdMutex g_MemoryThreadLock;

void(__cdecl *bdMemory::m_deallocateFunc)(void*) = NULL;
void *(__cdecl *bdMemory::m_allocateFunc)(unsigned int) = NULL;
void *(__cdecl *bdMemory::m_reallocateFunc)(void*, unsigned int) = NULL;
void *(__cdecl *bdMemory::m_alignedAllocateFunc)(unsigned int, unsigned int) = NULL;
void(__cdecl *bdMemory::m_alignedDeallocateFunc)(void *) = NULL;
void *(__cdecl *bdMemory::m_alignedReallocateFunc)(void *, unsigned int, unsigned int) = NULL;


void *__cdecl bdMemory::allocate(unsigned int a1)
{
    void *Func; // [esp+0h] [ebp-4h]

    Func = 0;
    if (bdMemory::m_allocateFunc)
    {
        //bdMutex::lock(&g_MemoryThreadLock);
        g_MemoryThreadLock.lock();
        Func = bdMemory::m_allocateFunc(a1);
        //bdMutex::unlock(&g_MemoryThreadLock);
        g_MemoryThreadLock.unlock();
        //if (!Func)
        //    DebugBreak();
    }
    return Func;
}

void __cdecl bdMemory::deallocate(void *a1)
{
    if (bdMemory::m_deallocateFunc)
    {
        //bdMutex::lock(&g_MemoryThreadLock);
        g_MemoryThreadLock.lock();
        bdMemory::m_deallocateFunc(a1);
        g_MemoryThreadLock.unlock();
        //bdMutex::unlock(&g_MemoryThreadLock);
    }
}

void *__cdecl bdMemory::reallocate(void *a1, unsigned int a2)
{
    void *v3; // [esp+0h] [ebp-4h]

    v3 = 0;
    if (bdMemory::m_reallocateFunc)
    {
        //bdMutex::lock(&g_MemoryThreadLock);
        g_MemoryThreadLock.lock();
        v3 = bdMemory::m_reallocateFunc(a1, a2);
        g_MemoryThreadLock.unlock();
        //bdMutex::unlock(&g_MemoryThreadLock);
        //if (!v3)
        //    DebugBreak();
    }
    return v3;
}

void __cdecl bdMemory::setAlignedAllocateFunc(void *(__cdecl *const a1)(unsigned int, unsigned int))
{
    bdMemory::m_alignedAllocateFunc = a1;
}

void __cdecl bdMemory::setAlignedDeallocateFunc(void(__cdecl *const a1)(void *))
{
    bdMemory::m_alignedDeallocateFunc = a1;
}

void __cdecl bdMemory::setAlignedReallocateFunc(void *(__cdecl *const a1)(void *, unsigned int, unsigned int))
{
    bdMemory::m_alignedReallocateFunc = a1;
}

void __cdecl bdMemory::setAllocateFunc(void *(__cdecl *const a1)(unsigned int))
{
    bdMemory::m_allocateFunc = a1;
}

void __cdecl setDeallocateFunc(void(__cdecl *const a1)(void *))
{
    bdMemory::m_deallocateFunc = a1;
}

void __cdecl bdMemory::setReallocateFunc(void *(__cdecl *const a1)(void *, unsigned int))
{
    bdMemory::m_reallocateFunc = a1;
}