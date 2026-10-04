#pragma once

struct XBlock;

void __cdecl DB_AllocXBlocks(
                unsigned int *blockSize,
                const char *filename,
                XBlock *blocks,
                unsigned int allocType,
                int flags);
unsigned __int8 *__cdecl DB_MemAlloc(
                unsigned int size,
                unsigned int type,
                unsigned int allocType,
                const char *filename,
                int flags);
void __cdecl DB_FreeXBlocks(XBlock *blocks, int flags);
