#pragma once
#include "mem_userhunk.h"

struct _fixed_heapnode // sizeof=0x4
{                                       // XREF: FIXED_HEAPNODE/r
    _fixed_heapnode *next;
};

_fixed_heapnode *__cdecl Hunk_FixedInit(
                _fixed_heapnode *buffer,
                unsigned int size,
                _fixed_heapnode *scheme,
                unsigned int flags,
                int *scheme_specific_data,
                _fixed_heapnode *name,
                _fixed_heapnode *type);


void __cdecl Hunk_FixedReset(HunkUser *_user);
void __cdecl Hunk_FixedDestroy(HunkUser *_user);
const char **__cdecl Hunk_FixedAlloc(HunkUser *_user);
void __cdecl Hunk_FixedFree(HunkUser *_user, char *ptr);
