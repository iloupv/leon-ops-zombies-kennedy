// Windows stand-in for the two POSIX calls the RE4 decomp's motion host driver uses (tools/motion/host/
// motion_host.cpp): an anonymous read/write mapping below 4 GB (MAP_32BIT, because the game's MotionSetCore
// relocates offsets into 32-bit pointers) and its release. VirtualAlloc is tried at 64 KB steps upward from
// 256 MB until a free range below 4 GB takes the size.
#pragma once
#include <stddef.h>
#include <windows.h>

#define PROT_READ 1
#define PROT_WRITE 2
#define MAP_PRIVATE 2
#define MAP_ANONYMOUS 0x20
#define MAP_32BIT 0x40
#define MAP_FAILED ((void *)-1)

static inline void *mmap(void *, size_t size, int, int, int, long)
{
    for ( unsigned long long addr = 0x10000000ull; addr + size < 0x100000000ull; addr += 0x10000ull )
    {
        void *p = VirtualAlloc((void *)addr, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
        if ( p )
            return p;
    }
    return MAP_FAILED;
}

static inline int munmap(void *p, size_t)
{
    return VirtualFree(p, 0, MEM_RELEASE) ? 0 : -1;
}
