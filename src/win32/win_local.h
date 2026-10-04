#pragma once

enum sysEventType_t : __int32
{                                       // XREF: sysEvent_t/r
    SE_NONE    = 0x0,
    SE_KEY     = 0x1,
    SE_CHAR    = 0x2,
    SE_CONSOLE = 0x3,
};

struct sysEvent_t // sizeof=0x18
{                                       // XREF: .data:sysEvent_t * eventQue/r
                                        // ?Sys_LoadingKeepAlive@@YAXXZ/r ...
    int evTime;                         // XREF: Com_EventLoop(void)+47/w
    sysEventType_t evType;              // XREF: Com_EventLoop(void)+4D/w
    int evValue;                        // XREF: Com_EventLoop(void)+53/w
    int evValue2;                       // XREF: Com_EventLoop(void)+59/w
    int evPtrLength;                    // XREF: Com_EventLoop(void)+5F/w
    void *evPtr;                        // XREF: Com_EventLoop(void)+65/w
};