#include "mem_fixed.h"
#include "assertive.h"

_fixed_heapnode *__cdecl Hunk_FixedInit(
                _fixed_heapnode *buffer,
                unsigned int size,
                _fixed_heapnode *scheme,
                unsigned int flags,
                int *scheme_specific_data,
                _fixed_heapnode *name,
                _fixed_heapnode *type)
{
    int v8; // [esp+0h] [ebp-1Ch]
    int i; // [esp+8h] [ebp-14h]
    _fixed_heapnode *node; // [esp+Ch] [ebp-10h]
    unsigned int count; // [esp+18h] [ebp-4h]

    if ( (flags & 2) == 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\mem_fixed.cpp",
                    28,
                    0,
                    "%s",
                    "(flags & HF_FROMBUFFER)!=0") )
    {
        __debugbreak();
    }
    if ( size <= 0x20
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\mem_fixed.cpp",
                    29,
                    0,
                    "%s",
                    "size>sizeof(FIXED_HUNKUSER)+sizeof(FIXED_HEAPNODE)") )
    {
        __debugbreak();
    }
    if ( *scheme_specific_data != (*scheme_specific_data & (*scheme_specific_data - 1))
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\mem_fixed.cpp",
                    30,
                    0,
                    "%s",
                    "scheme_data->block_size==(scheme_data->block_size&(scheme_data->block_size-1))") )
    {
        __debugbreak();
    }
    buffer[2].next = name;
    buffer->next = scheme;
    buffer[1].next = (_fixed_heapnode *)flags;
    buffer[3].next = type;
    buffer[4].next = (_fixed_heapnode *)size;
    if ( *scheme_specific_data < 4 )
        v8 = 4;
    else
        v8 = *scheme_specific_data;
    buffer[5].next = (_fixed_heapnode *)v8;
    count = (size - 28) / (unsigned int)&buffer[5].next[1];
    node = buffer + 7;
    buffer[6].next = buffer + 7;
    for ( i = 0; i < (int)(count - 1); ++i )
        node->next = (_fixed_heapnode *)((char *)buffer[5].next + (unsigned int)node);
    node->next = 0;
    return buffer;
}

void __cdecl Hunk_FixedReset(HunkUser *_user)
{
    int i; // [esp+0h] [ebp-10h]
    _fixed_heapnode *node; // [esp+4h] [ebp-Ch]
    unsigned __int32 count; // [esp+Ch] [ebp-4h]

    count = (_user[1].scheme - 28) / (_user[1].flags + 4);
    node = (_fixed_heapnode *)&_user[1].type;
    _user[1].name = (const char *)&_user[1].type;
    for ( i = 0; i < (int)(count - 1); ++i )
        node->next = (_fixed_heapnode *)((char *)node + _user[1].flags + 4);
    node->next = 0;
}

void __cdecl Hunk_FixedDestroy(HunkUser *_user)
{
    _user->scheme = HU_SCHEME_DEFAULT;
    _user->flags = 0;
    _user->name = 0;
    _user->type = 0;
    _user[1].scheme = HU_SCHEME_DEFAULT;
    _user[1].flags = 0;
    _user[1].name = 0;
}

const char **__cdecl Hunk_FixedAlloc(HunkUser *_user)
{
    const char **result; // [esp+0h] [ebp-8h]

    result = 0;
    if ( _user[1].name )
    {
        result = (const char **)_user[1].name;
        _user[1].name = *result;
    }
    return result;
}

void __cdecl Hunk_FixedFree(HunkUser *_user, char *ptr)
{
    if ( ptr )
    {
        if ( ptr <= (char *)_user || ptr >= (char *)_user + _user[1].scheme )
        {
            if ( !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\universal\\mem_fixed.cpp",
                            99,
                            0,
                            "%s\n\t%s",
                            "0",
                            "invalid pointer passed into first fit free\n") )
                __debugbreak();
        }
        else
        {
            *(unsigned int *)ptr = (unsigned int)_user[1].name;
            _user[1].name = ptr;
        }
    }
}

