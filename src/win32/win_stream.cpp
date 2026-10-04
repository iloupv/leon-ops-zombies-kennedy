#include "win_stream.h"
#include <universal/assertive.h>
#include <universal/profile.h>
#include <qcommon/threads.h>
#include <gfx_d3d/r_stream.h>
#include <sound/snd_stream.h>
#include <setjmp.h>

bool g_streamReadThreadSpawned;

char __cdecl Stream_Init()
{
    if ( !g_streamReadThreadSpawned && !Sys_SpawnStreamThread(Stream_Thread) )
        return 0;
    g_streamReadThreadSpawned = 1;
    return 1;
}

void __cdecl    Stream_Thread(unsigned int threadContext)
{
    void *Value; // eax

    if ( threadContext != 14
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\win32\\win_stream.cpp",
                    19,
                    0,
                    "threadContext == THREAD_CONTEXT_STREAM\n\t%i, %i",
                    threadContext,
                    14) )
    {
        __debugbreak();
    }
    do
        Value = Sys_GetValue(2);
    //while ( _setjmp3(Value, 0) );
    while ( _setjmp((int*)Value) );

    while ( 1 )
    {
        Sys_StreamSleep();
        {
            PROF_SCOPED("R_StreamUpdate_ReadTextures"); // ADD
            R_StreamUpdate_ReadTextures();
        }
        {
            PROF_SCOPED("Snd_StreamUpdate"); // ADD
            Snd_StreamUpdate();
        }
    }
}

