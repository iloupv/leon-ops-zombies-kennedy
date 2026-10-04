#include "db_auth.h"
#include <universal/assertive.h>
#include <qcommon/common.h>

int __cdecl DB_AuthLoad_InflateInit(z_stream_s *stream, bool isSecure)
{
    if ( isSecure
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\database\\db_auth.cpp", 944, 0, "%s", "!isSecure") )
    {
        __debugbreak();
    }
    return inflateInit_(stream, "1.2.3", sizeof(z_stream_s));
}

void __cdecl DB_AuthLoad_InflateEnd(z_stream_s *stream)
{
    inflateEnd(stream);
}

int __cdecl DB_AuthLoad_Inflate(z_stream_s *stream, int flush)
{
    return inflate(stream, flush);
}

void *__cdecl tom_game_malloc(unsigned int size)
{
    return Com_LiveAllocate(size);
}

unsigned __int8 *__cdecl tom_game_realloc(unsigned __int8 *ptr, unsigned int size)
{
    return Com_LiveRealloc(ptr, size);
}

unsigned __int8 *__cdecl tom_game_calloc(unsigned int num, unsigned int size)
{
    return Com_LiveCalloc(num, size);
}

void __cdecl tom_game_free(void *ptr)
{
    Com_LiveDeallocate(ptr);
}

