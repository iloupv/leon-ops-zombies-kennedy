#include "mem_largelocal.h"
#include <qcommon/threads.h>
#include "assertive.h"

int g_largeLocalPos;
unsigned __int8 g_largeLocalBuf[524288];
int g_largeLocalRightPos = 524288;


int __cdecl LargeLocalBegin(int size)
{
    int startPos; // [esp+0h] [ebp-4h]
    unsigned int sizea; // [esp+Ch] [ebp+8h]

    if ( !Sys_IsMainThread()
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\mem_largelocal.cpp",
                    35,
                    0,
                    "%s",
                    "Sys_IsMainThread()") )
    {
        __debugbreak();
    }
    sizea = LargeLocalRoundSize(size);
    startPos = g_largeLocalPos;
    g_largeLocalPos += sizea;
    if ( g_largeLocalPos > 0x80000
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\mem_largelocal.cpp",
                    43,
                    0,
                    "%s",
                    "g_largeLocalPos <= static_cast< int >( sizeof( g_largeLocalBuf ) )") )
    {
        __debugbreak();
    }
    if ( g_largeLocalPos > g_largeLocalRightPos
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\mem_largelocal.cpp",
                    45,
                    0,
                    "%s",
                    "g_largeLocalPos <= g_largeLocalRightPos") )
    {
        __debugbreak();
    }
    return startPos;
}

unsigned int __cdecl LargeLocalRoundSize(int size)
{
    return (size + 3) & 0xFFFFFFFC;
}

void __cdecl LargeLocalEnd(int startPos)
{
    if ( !Sys_IsMainThread()
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\mem_largelocal.cpp",
                    59,
                    0,
                    "%s",
                    "Sys_IsMainThread()") )
    {
        __debugbreak();
    }
    g_largeLocalPos = startPos;
    if ( startPos > g_largeLocalRightPos
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\mem_largelocal.cpp",
                    64,
                    0,
                    "%s",
                    "g_largeLocalPos <= g_largeLocalRightPos") )
    {
        __debugbreak();
    }
}

unsigned __int8 *__cdecl LargeLocalGetBuf(int startPos, int size)
{
    int startIndex; // [esp+0h] [ebp-4h]

    if ( !Sys_IsMainThread()
        && !Sys_IsServerThread()
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\mem_largelocal.cpp",
                    121,
                    0,
                    "%s",
                    "Sys_IsMainThread() || Sys_IsServerThread()") )
    {
        __debugbreak();
    }
    if ( Sys_IsMainThread() )
        return &g_largeLocalBuf[startPos];
    startIndex = startPos - LargeLocalRoundSize(size);
    if ( startIndex < 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\mem_largelocal.cpp",
                    134,
                    0,
                    "%s",
                    "startIndex >= 0") )
    {
        __debugbreak();
    }
    return &g_largeLocalBuf[startIndex];
}

LargeLocal::LargeLocal(int sizeParam)
{
    if ( !Sys_IsMainThread()
        && !Sys_IsServerThread()
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\mem_largelocal.cpp",
                    149,
                    0,
                    "%s",
                    "Sys_IsMainThread() || Sys_IsServerThread()") )
    {
        __debugbreak();
    }
    if ( Sys_IsMainThread() )
        this->startPos = LargeLocalBegin(sizeParam);
    else
        this->startPos = LargeLocalBeginRight(sizeParam);
    this->size = sizeParam;
}

int __cdecl LargeLocalBeginRight(int size)
{
    int startPos; // [esp+0h] [ebp-4h]
    unsigned int sizea; // [esp+Ch] [ebp+8h]

    if ( !Sys_IsServerThread()
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\mem_largelocal.cpp",
                    80,
                    0,
                    "%s",
                    "Sys_IsServerThread()") )
    {
        __debugbreak();
    }
    sizea = LargeLocalRoundSize(size);
    if ( (int)(g_largeLocalRightPos - sizea) <= g_largeLocalPos
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\mem_largelocal.cpp",
                    84,
                    0,
                    "%s",
                    "g_largeLocalRightPos - size > g_largeLocalPos") )
    {
        __debugbreak();
    }
    startPos = g_largeLocalRightPos;
    g_largeLocalRightPos -= sizea;
    return startPos;
}

LargeLocal::~LargeLocal()
{
    if ( !Sys_IsMainThread()
        && !Sys_IsServerThread()
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\mem_largelocal.cpp",
                    175,
                    0,
                    "%s",
                    "Sys_IsMainThread() || Sys_IsServerThread()") )
    {
        __debugbreak();
    }
    if ( Sys_IsMainThread() )
        LargeLocalEnd(this->startPos);
    else
        LargeLocalEndRight(this->startPos);
}

void __cdecl LargeLocalEndRight(int startPos)
{
    if ( !Sys_IsServerThread()
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\mem_largelocal.cpp",
                    100,
                    0,
                    "%s",
                    "Sys_IsServerThread()") )
    {
        __debugbreak();
    }
    g_largeLocalRightPos = startPos;
    if ( startPos <= g_largeLocalPos
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\mem_largelocal.cpp",
                    104,
                    0,
                    "%s",
                    "g_largeLocalRightPos > g_largeLocalPos") )
    {
        __debugbreak();
    }
}

unsigned __int8 *LargeLocal::GetBuf()
{
    if ( !Sys_IsMainThread()
        && !Sys_IsServerThread()
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\mem_largelocal.cpp",
                    199,
                    0,
                    "%s",
                    "Sys_IsMainThread() || Sys_IsServerThread()") )
    {
        __debugbreak();
    }
    return LargeLocalGetBuf(this->startPos, this->size);
}

void __cdecl LargeLocalReset()
{
    if ( !Sys_IsMainThread()
        && !Sys_IsServerThread()
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\mem_largelocal.cpp",
                    216,
                    0,
                    "%s",
                    "Sys_IsMainThread() || Sys_IsServerThread()") )
    {
        __debugbreak();
    }
    g_largeLocalPos = 0;
    g_largeLocalRightPos = 0x80000;
}

