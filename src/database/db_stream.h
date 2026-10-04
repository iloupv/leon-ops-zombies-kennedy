#pragma once
#include "db_registry.h"

struct StreamPosInfo // sizeof=0x8
{                                       // XREF: .data:g_streamPosStack/r
    unsigned __int8 *pos;               // XREF: DB_PopStreamPos(void)+51/r
    unsigned int index;                 // XREF: DB_PushStreamPos(uint)+8F/w
};

void __cdecl DB_InitStreams(XBlock *blocks);
void __cdecl DB_PushStreamPos(unsigned int index);
void __cdecl DB_SetStreamIndex(unsigned int index);
void __cdecl DB_PopStreamPos();
unsigned __int8 *__cdecl DB_GetStreamPos();
unsigned __int8 *__cdecl DB_AllocStreamPos(int alignment);
void __cdecl DB_IncStreamPos(int size);
const void **__cdecl DB_InsertPointer();


extern XBlock *g_streamBlocks;
extern unsigned __int8 *g_streamPos;
extern unsigned int g_streamPosIndex;
extern unsigned int g_streamDelayIndex;
extern unsigned __int8 *g_streamPosArray[7];
extern unsigned int g_streamPosStackIndex;