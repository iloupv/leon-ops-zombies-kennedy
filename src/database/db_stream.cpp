#include "db_stream.h"
#include <universal/assertive.h>

XBlock *g_streamBlocks;
unsigned __int8 *g_streamPos;
unsigned int g_streamPosIndex;
unsigned int g_streamDelayIndex;
unsigned __int8 *g_streamPosArray[7];
unsigned int g_streamPosStackIndex;

StreamPosInfo g_streamPosStack[64];

void __cdecl DB_InitStreams(XBlock *blocks)
{
    int i; // [esp+0h] [ebp-4h]

    g_streamBlocks = blocks;
    g_streamPos = blocks->data;
    g_streamPosIndex = 0;
    g_streamDelayIndex = 0;
    g_streamPosArray[0] = 0;
    g_streamPosStackIndex = 0;
    for ( i = 0; i < 7; ++i )
    {
        if ( i )
            g_streamPosArray[i] = blocks[i].data;
    }
}

void __cdecl DB_PushStreamPos(unsigned int index)
{
    iassert(index < ARRAY_COUNT(g_streamPosArray));
    iassert(g_streamPosIndex < ARRAY_COUNT(g_streamPosArray));
    iassert(g_streamPosStackIndex < ARRAY_COUNT(g_streamPosStack));

    //g_streamPosStack[g_streamPosStackIndex++].index = g_streamPosIndex;
    //DB_SetStreamIndex(index);
    //*(&g_streamPosIndex + 2 * g_streamPosStackIndex) = (unsigned int)g_streamPos;

    g_streamPosStack[g_streamPosStackIndex].index = g_streamPosIndex;
    DB_SetStreamIndex(index);
    g_streamPosStack[g_streamPosStackIndex++].pos = g_streamPos;
}

void __cdecl DB_SetStreamIndex(unsigned int index)
{
    if ( index != g_streamPosIndex )
    {
        g_streamPosArray[g_streamPosIndex] = g_streamPos;
        g_streamPosIndex = index;
        g_streamPos = g_streamPosArray[index];
        g_streamPosArray[index] = 0;
    }
}

void __cdecl DB_PopStreamPos()
{
    if ( !g_streamPosStackIndex
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\database\\db_stream.cpp",
                    83,
                    0,
                    "%s\n\t(g_streamPosStackIndex) = %i",
                    "(g_streamPosStackIndex > 0)",
                    0) )
    {
        __debugbreak();
    }
    --g_streamPosStackIndex;
    if ( !g_streamPosIndex )
        g_streamPos = g_streamPosStack[g_streamPosStackIndex].pos;
    DB_SetStreamIndex(g_streamPosStack[g_streamPosStackIndex].index);
}

unsigned __int8 *__cdecl DB_GetStreamPos()
{
    return g_streamPos;
}

unsigned __int8 *__cdecl DB_AllocStreamPos(int alignment)
{
    if ( !g_streamPos
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\database\\db_stream.cpp", 110, 0, "%s", "g_streamPos") )
    {
        __debugbreak();
    }
    g_streamPos = (unsigned __int8 *)(~alignment & (unsigned int)&g_streamPos[alignment]);
    return g_streamPos;
}

void __cdecl DB_IncStreamPos(int size)
{
    iassert(g_streamPos);
    iassert(g_streamPos + size <= g_streamBlocks[g_streamPosIndex].data + g_streamBlocks[g_streamPosIndex].size);
    //if ( &g_streamPos[size] > &g_streamBlocks[g_streamPosIndex].data[g_streamBlocks[g_streamPosIndex].size]
    //    && !Assert_MyHandler(
    //                "C:\\projects_pc\\cod\\codsrc\\src\\database\\db_stream.cpp",
    //                120,
    //                0,
    //                "%s",
    //                "g_streamPos + size <= g_streamBlocks[g_streamPosIndex].data + g_streamBlocks[g_streamPosIndex].size") )
    //{
    //    __debugbreak();
    //}
    g_streamPos += size;
}

const void **__cdecl DB_InsertPointer()
{
    const void **pData; // [esp+0h] [ebp-4h]

    DB_PushStreamPos(4u);
    pData = (const void **)DB_AllocStreamPos(3);
    DB_IncStreamPos(4);
    DB_PopStreamPos();
    return pData;
}

