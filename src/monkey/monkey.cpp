#include "monkey.h"
#include "monkey_comm.h"

#include <cstring>
#include <universal/q_shared.h>
#include <Windows.h>
#include <universal/dvar.h>
#include <universal/assertive.h>
#include <qcommon/common.h>
#include <qcommon/cmd.h>
#include <win32/win_common.h>
#include <universal/timing.h>

char g_MonkeyPrintBuffer[0x20000];
unsigned int g_MonkeyPrintBufferSize;
bool g_MonkeyGrabPrints;
bool g_MonkeyIsRunning;
unsigned __int64 g_MonkeyOokTimer;
unsigned __int64 g_MonkeyOokTimeoutTicks;

const dvar_s *in_monkey;

void __cdecl Monkey_Error(const char *text)
{
    if ( Monkey_IsRunning() )
    {
        if ( text )
            Monkey_Send("error", text);
        else
            Monkey_Send("error", g_MonkeyPrintBuffer);
        Monkey_DisconnectAndExit();
    }
}

void __cdecl Monkey_GrabComPrints(bool enable)
{
    if ( Monkey_IsRunning() )
    {
        if ( enable )
        {
            if ( !g_MonkeyGrabPrints )
            {
                g_MonkeyPrintBuffer[0] = 0;
                g_MonkeyPrintBufferSize = 0;
            }
            g_MonkeyGrabPrints = 1;
        }
        else
        {
            g_MonkeyPrintBuffer[0] = 0;
            g_MonkeyPrintBufferSize = 0;
            g_MonkeyGrabPrints = 0;
        }
    }
}

void __cdecl Monkey_ComPrintHook(const char *msg)
{
    unsigned int v1; // kr00_4
    char v2; // dl
    char *v3; // [esp+10h] [ebp-58h]
    const char *v4; // [esp+14h] [ebp-54h]
    int v5; // [esp+18h] [ebp-50h]
    int msg_len; // [esp+64h] [ebp-4h]

    if ( g_MonkeyGrabPrints )
    {
        msg_len = strlen(msg);
        if ( g_MonkeyPrintBufferSize + msg_len >= 0x1FFFF )
        {
            v1 = strlen("\n(Message truncated)\n");
            if ( 0x20000 - (v1 + 1) >= g_MonkeyPrintBufferSize )
                v5 = g_MonkeyPrintBufferSize;
            else
                v5 = 0x20000 - (v1 + 1);
            v4 = "\n(Message truncated)\n";
            v3 = &g_MonkeyPrintBuffer[v5];
            do
            {
                v2 = *v4;
                *v3++ = *v4++;
            }
            while ( v2 );
            g_MonkeyPrintBufferSize = 0x20000;
        }
        else
        {
            strcat(g_MonkeyPrintBuffer, msg);
            g_MonkeyPrintBufferSize += msg_len;
        }
    }
}

void __cdecl Monkey_Start()
{
    LARGE_INTEGER PerformanceCount; // [esp+0h] [ebp-Ch] BYREF
    const char *zookeeper; // [esp+8h] [ebp-4h]

    in_monkey = _Dvar_RegisterBool("in_monkey", 0, 0, "Randomly generate input");
    zookeeper = Dvar_GetString("zookeeper");
    if ( zookeeper )
    {
        if ( *zookeeper )
        {
            Monkey_InitCommunication(zookeeper);
            g_MonkeyIsRunning = 1;
            QueryPerformanceCounter(&PerformanceCount);
            g_MonkeyOokTimer = PerformanceCount.QuadPart;
            g_MonkeyOokTimeoutTicks = (unsigned __int64)(1000.0 / msecPerRawTimerTick);
            Assert_SetMonkeyCallbackHandler(Monkey_AssertCallback);
        }
    }
}

void __cdecl Monkey_AssertCallback(const char *assertMsg)
{
    Monkey_Send("assert", assertMsg);
    Monkey_ShutdownCommunication();
}

bool __cdecl Monkey_IsRunning()
{
    return g_MonkeyIsRunning;
}

void __cdecl Monkey_Frame()
{
    char message[1024]; // [esp+0h] [ebp-488h] BYREF
    char type[132]; // [esp+400h] [ebp-88h] BYREF

    if ( Monkey_IsRunning() )
    {
        if ( Monkey_Recv(type, 128, message, 1024) )
        {
            if ( !I_stricmp(type, "command") )
            {
                Com_Printf(16, "Monkey: Recieved command '%s'\n", message);
                Cbuf_AddText(0, message);
                Cbuf_AddText(0, "\n");
            }
            if ( I_stricmp(type, "command_capture") )
            {
                Com_Printf(16, "Monkey: Unknown command type '%s', message='%s'\n", type, message);
            }
            else
            {
                Com_Printf(16, "Monkey: Recieved command_capture '%s'\n", message);
                Cbuf_AddText(0, message);
                Cbuf_AddText(0, "\n");
                Sys_EnterCriticalSection(CRITSECT_CONSOLE);
                Monkey_GrabComPrints(0);
                Monkey_GrabComPrints(1);
                Cbuf_Execute(0, 0);
                Monkey_Send("output", g_MonkeyPrintBuffer);
                Monkey_GrabComPrints(0);
                Sys_LeaveCriticalSection(CRITSECT_CONSOLE);
            }
        }
        Monkey_KeepAlive();
    }
}

void __cdecl Monkey_KeepAlive()
{
    LARGE_INTEGER PerformanceCount; // [esp+8h] [ebp-18h] BYREF
    const char *string; // [esp+10h] [ebp-10h]
    const char *ook; // [esp+14h] [ebp-Ch]
    unsigned __int64 now; // [esp+18h] [ebp-8h]

    if ( Monkey_IsRunning() )
    {
        QueryPerformanceCounter(&PerformanceCount);
        now = PerformanceCount.QuadPart;
        if ( PerformanceCount.QuadPart <= g_MonkeyOokTimer )
        {
            if ( g_MonkeyOokTimer > g_MonkeyOokTimeoutTicks + now )
            {
                g_MonkeyOokTimer = g_MonkeyOokTimeoutTicks + now;
                string = "Ook, ook, eek (strange happenings)!\n";
                Monkey_Send("ook", "Ook, ook, eek (strange happenings)!\n");
            }
        }
        else
        {
            g_MonkeyOokTimer = g_MonkeyOokTimeoutTicks + now;
            ook = "Ook, ook, eek!\n";
            Monkey_Send("ook", "Ook, ook, eek!\n");
        }
    }
}

void __cdecl Monkey_Event(const char *event)
{
    if ( Monkey_IsRunning() )
        Monkey_Send("event", event);
}

bool __cdecl Monkey_UseRandomInput()
{
    return in_monkey->current.enabled;
}

