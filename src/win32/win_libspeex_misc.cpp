#include "win_libspeex_misc.h"
#include <universal/com_memory.h>

unsigned int *__cdecl speex_alloc(int size)
{
    return Z_Malloc(size, "speex", 17);
}

void __cdecl speex_free(char *ptr)
{
    Z_Free(ptr, 17);
}

void *__cdecl speex_realloc()
{
    if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\win32\\win_libspeex_misc.cpp", 24, 0, "Speex: realloc!") )
        __debugbreak();
    return 0;
}

