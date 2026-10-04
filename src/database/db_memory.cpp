#include "db_memory.h"
#include "db_file_load.h"
#include "db_registry.h"
#include <universal/physicalmemory.h>
#include <universal/com_memory.h>
#include <qcommon/common.h>

const char *g_block_mem_name[7] =
{
  "temp",
  "runtime",
  "large_runtime",
  "physical_runtime",
  "virtual",
  "large",
  "physical"
};

int g_block_mem_type[7] =
{ 0, 1, 1, 2, 1, 1, 2 };


void __cdecl DB_AllocXBlocks(
                unsigned int *blockSize,
                const char *filename,
                XBlock *blocks,
                unsigned int allocType,
                int flags)
{
    double v5; // xmm0_8
    unsigned int blockIndex; // [esp+28h] [ebp-10h]
    XBlock buf; // [esp+2Ch] [ebp-Ch]
    float total_size; // [esp+34h] [ebp-4h]

    total_size = 0.0f;
    for ( blockIndex = 0; blockIndex < 7; ++blockIndex )
    {
        buf.size = blockSize[blockIndex];
        if ( buf.size )
        {
            total_size = (double)buf.size + total_size;
            buf.data = DB_MemAlloc(buf.size, g_block_mem_type[blockIndex], allocType, filename, flags);
            if ( !buf.data )
            {
                v5 = (float)((float)PMem_GetOverAllocatedSize() * 0.00000095367432);
                Com_Error(
                    ERR_DROP,
                    "Could not allocate %.2f MB of type '%s' for zone '%s' needed an additional %.2f MB",
                    (double)buf.size * 0.00000095367432,
                    g_block_mem_name[blockIndex],
                    filename,
                    v5);
            }
            blocks[blockIndex] = buf;
        }
    }
    Com_Printf(16, "used %0.2f MB memory in DB alloc\n", (float)(total_size / 1048576.0));
}

unsigned __int8 *__cdecl DB_MemAlloc(
                unsigned int size,
                unsigned int type,
                unsigned int allocType,
                const char *filename,
                int flags)
{
    if ( type > 1 )
    {
        if ( type != 2
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\database\\db_memory.cpp",
                        81,
                        0,
                        "%s",
                        "type == DM_MEMORY_PHYSICAL") )
        {
            __debugbreak();
        }
        return _PMem_AllocNamed(size, 0x1000u, -2147482620, allocType, filename, TRACK_FASTFILE);
    }
    else if ( (flags & 0x40000000) != 0 )
    {
        return (unsigned __int8 *)Z_Malloc(size, "ZONE LOAD", 12);
    }
    else
    {
        return _PMem_AllocNamed(size + 15, 0x1000u, 4, allocType, filename, TRACK_FASTFILE);
    }
}

void __cdecl DB_FreeXBlocks(XBlock *blocks, int flags)
{
    unsigned int blockIndex; // [esp+0h] [ebp-4h]

    for ( blockIndex = 0; blockIndex < 7; ++blockIndex )
    {
        if ( (flags & 0x40000000) != 0 )
        {
            if ( blocks[blockIndex].data )
                Z_Free((char *)blocks[blockIndex].data, 12);
        }
        blocks[blockIndex].data = 0;
        blocks[blockIndex].size = 0;
    }
}

