#include "g_save_mp.h"
#include <universal/q_shared.h>

void __cdecl MemFile_WriteByte(MemoryFile *memFile, unsigned __int8 value)
{
    if ( (unsigned int)(memFile->cacheBufferUsed + 1) >= 0x7FF8 )
        MemFile_WriteDataFlushInternal(memFile);
    memFile->cacheBuffer[memFile->cacheBufferUsed++] = value;
}

void __cdecl MemFile_WriteShort(MemoryFile *memFile, __int16 value)
{
    if ( (unsigned int)(memFile->cacheBufferUsed + 2) >= 0x7FF8 )
        MemFile_WriteDataFlushInternal(memFile);
    *(_WORD *)&memFile->cacheBuffer[memFile->cacheBufferUsed] = value;
    memFile->cacheBufferUsed += 2;
}

