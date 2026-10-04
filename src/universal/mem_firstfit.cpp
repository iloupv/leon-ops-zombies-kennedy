#include "mem_firstfit.h"
#include "assertive.h"
#include <win32/win_common.h>

HunkUser *__cdecl Hunk_FirstFitInit(
                unsigned int *buffer,
                unsigned int size,
                HU_ALLOCATION_SCHEME scheme,
                unsigned int flags,
                void *scheme_specific_data,
                const char *name,
                int type)
{
    if ( (flags & 2) == 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\mem_firstfit.cpp",
                    49,
                    0,
                    "%s",
                    "(flags & HF_FROMBUFFER)!=0") )
    {
        __debugbreak();
    }
    if ( size <= 0x28
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\mem_firstfit.cpp",
                    50,
                    0,
                    "%s",
                    "size>sizeof(FIRSTFIT_HUNKUSER)+sizeof(FIRSTFIT_HEAPNODE)") )
    {
        __debugbreak();
    }
    buffer[2] = (unsigned int)name;
    *buffer = scheme;
    buffer[1] = flags;
    buffer[3] = type;
    buffer[6] = -1;
    buffer[4] = size;
    buffer[5] = (unsigned int)(buffer + 8);
    *(unsigned int *)buffer[5] = 0;
    *(unsigned int *)(buffer[5] + 4) = buffer[4] - 32;
    buffer[7] = 32;
    return (HunkUser *)buffer;
}

void __cdecl Hunk_FirstFitReset(HunkUser *_user)
{
    _user[1].flags = (unsigned int)&_user[2];
    *(unsigned int *)_user[1].flags = 0;
    *(unsigned int *)(_user[1].flags + 4) = _user[1].scheme - 32;
    _user[1].type = 32;
}

void __cdecl Hunk_FirstFitDestroy(HunkUser *_user)
{
    _user->scheme = HU_SCHEME_DEFAULT;
    _user->flags = 0;
    _user->name = 0;
    _user->type = 0;
    _user[1].scheme = HU_SCHEME_DEFAULT;
    _user[1].flags = 0;
    _user[1].name = 0;
    _user[1].type = 0;
}

int __cdecl Hunk_FirstFitAlloc(HunkUser *_user, int size, int alignment)
{
    unsigned int *free_link; // [esp+10h] [ebp-20h]
    int adj_size; // [esp+18h] [ebp-18h]
    _firstfit_heapnode *last; // [esp+2Ch] [ebp-4h]

    if ( alignment <= 0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\mem_firstfit.cpp", 90, 0, "%s", "alignment>0") )
    {
        __debugbreak();
    }
    Sys_EnterCriticalSection(CRITSECT_MEMFIRSTFIT);
    free_link = (unsigned int *)_user[1].flags;
    last = 0;
    while ( 1 )
    {
        if ( !free_link )
            goto LABEL_16;
        adj_size = (~(alignment - 1) & ((unsigned int)free_link + alignment + 11)) - (unsigned int)free_link + size;
        if ( (int)free_link[1] >= adj_size )
            break;
        last = (_firstfit_heapnode *)free_link;
        free_link = (unsigned int *)*free_link;
    }
    if ( !*free_link && (int)(free_link[1] - adj_size) <= 1024 )
    {
LABEL_16:
        Sys_LeaveCriticalSection(CRITSECT_MEMFIRSTFIT);
        return 0;
    }
    if ( (int)(free_link[1] - adj_size) > 1024 )
    {
        *(unsigned int *)((~(alignment - 1) & ((unsigned int)free_link + alignment + 11)) + size) = *free_link;
        *(unsigned int *)((char *)free_link + adj_size + 4) = free_link[1] - adj_size;
        *free_link = (~(alignment - 1) & ((unsigned int)free_link + alignment + 11)) + size;
        free_link[1] = adj_size;
    }
    if ( last )
        last->next = (_firstfit_heapnode *)*free_link;
    else
        _user[1].flags = *free_link;
    *free_link = -559038737;
    *(unsigned int *)((~(alignment - 1) & ((unsigned int)free_link + alignment + 11)) - 12 + 8) = (unsigned int)free_link;
    _user[1].type += free_link[1];
    Sys_LeaveCriticalSection(CRITSECT_MEMFIRSTFIT);
    return ~(alignment - 1) & ((unsigned int)free_link + alignment + 11);
}

void __cdecl Hunk_FirstFitFree(HunkUser *_user, unsigned int *ptr)
{
    _firstfit_heapnode *free_link; // [esp+0h] [ebp-14h]
    _firstfit_heapnode *scan; // [esp+8h] [ebp-Ch]
    _firstfit_heapnode *last; // [esp+10h] [ebp-4h]

    if ( ptr )
    {
        Sys_EnterCriticalSection(CRITSECT_MEMFIRSTFIT);
        last = 0;
        scan = (_firstfit_heapnode *)_user[1].flags;
        free_link = (_firstfit_heapnode *)*(ptr - 1);
        if ( free_link->next == (_firstfit_heapnode *)-559038737 )
        {
            _user[1].type -= free_link->size;
            if ( !_user[1].flags
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\universal\\mem_firstfit.cpp",
                            178,
                            0,
                            "%s",
                            "hunk->free_blocks!=NULL") )
            {
                __debugbreak();
            }
            while ( scan )
            {
                if ( scan > free_link )
                {
                    if ( last )
                    {
                        free_link->next = scan;
                        last->next = free_link;
                    }
                    else
                    {
                        free_link->next = (_firstfit_heapnode *)_user[1].flags;
                        _user[1].flags = (unsigned int)free_link;
                    }
                    if ( last && (_firstfit_heapnode *)((char *)last + last->size) == free_link )
                    {
                        last->size += free_link->size;
                        last->next = free_link->next;
                        free_link = last;
                    }
                    if ( free_link->next && (_firstfit_heapnode *)((char *)free_link + free_link->size) == free_link->next )
                    {
                        free_link->size += free_link->next->size;
                        free_link->next = free_link->next->next;
                    }
                    goto LABEL_24;
                }
                last = scan;
                scan = scan->next;
            }
            if ( !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\universal\\mem_firstfit.cpp",
                            220,
                            0,
                            "%s\n\t%s",
                            "0",
                            "invalid pointer passed into first fit free\n") )
                __debugbreak();
        }
        else if ( !Assert_MyHandler(
                                 "C:\\projects_pc\\cod\\codsrc\\src\\universal\\mem_firstfit.cpp",
                                 173,
                                 0,
                                 "%s\n\t%s",
                                 "0",
                                 "buffer overrun or underrun or illegal ptr detected") )
        {
            __debugbreak();
        }
LABEL_24:
        Sys_LeaveCriticalSection(CRITSECT_MEMFIRSTFIT);
    }
}

