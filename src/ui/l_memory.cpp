#include "l_memory.h"
#include <universal/com_memory.h>

unsigned int *__cdecl GetMemory(unsigned int size)
{
    unsigned int *ptr; // [esp+4h] [ebp-4h]

    ptr = Z_Malloc(size + 4, "GetMemory", 11);
    if ( !ptr )
        return 0;
    *ptr = 305419896;
    return ptr + 1;
}

unsigned __int8 *__cdecl GetClearedMemory(unsigned int size)
{
    unsigned __int8 *ptr; // [esp+0h] [ebp-4h]

    ptr = (unsigned __int8 *)GetMemory(size);
    memset(ptr, 0, size);
    return ptr;
}

void __cdecl FreeMemory(char *ptr)
{
    if ( *((unsigned int *)ptr - 1) == 305419896 )
        Z_Free(ptr - 4, 11);
}

