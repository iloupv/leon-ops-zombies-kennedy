#pragma once

class bdMemory // sizeof=0x0
{
public:
    static void(__cdecl *m_deallocateFunc)(void*);
    static void *(__cdecl *m_allocateFunc)(unsigned int);
    static void *(__cdecl *m_reallocateFunc)(void*, unsigned int);
    static void *(__cdecl *m_alignedAllocateFunc)(unsigned int, unsigned int);
    static void(__cdecl *m_alignedDeallocateFunc)(void *);
    static void *(__cdecl *m_alignedReallocateFunc)(void *, unsigned int, unsigned int);

    static void *__cdecl allocate(unsigned int a1);
    static void __cdecl deallocate(void *a1);
    static void* __cdecl reallocate(void *a1, unsigned int a2);

    static void __cdecl setAlignedAllocateFunc(void *(__cdecl *const a1)(unsigned int, unsigned int));
    static void __cdecl setAlignedDeallocateFunc(void(__cdecl *const a1)(void *));
    static void __cdecl setAlignedReallocateFunc(void *(__cdecl *const a1)(void *, unsigned int, unsigned int));

    static void __cdecl setAllocateFunc(void *(__cdecl *const a1)(unsigned int));
    static void __cdecl setDeallocateFunc(void(__cdecl *const a1)(void *));
    static void __cdecl setReallocateFunc(void *(__cdecl *const a1)(void *, unsigned int));
};