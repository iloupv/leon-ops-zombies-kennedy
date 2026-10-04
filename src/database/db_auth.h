#pragma once

#include <zlib/zlib.h>

int __cdecl DB_AuthLoad_InflateInit(z_stream_s *stream, bool isSecure);
void __cdecl DB_AuthLoad_InflateEnd(z_stream_s *stream);
int __cdecl DB_AuthLoad_Inflate(z_stream_s *stream, int flush);
void *__cdecl tom_game_malloc(unsigned int size);
unsigned __int8 *__cdecl tom_game_realloc(unsigned __int8 *ptr, unsigned int size);
unsigned __int8 *__cdecl tom_game_calloc(unsigned int num, unsigned int size);
void __cdecl tom_game_free(void *ptr);