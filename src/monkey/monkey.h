#pragma once

void __cdecl Monkey_Error(const char *text);
void __cdecl Monkey_GrabComPrints(bool enable);
void __cdecl Monkey_ComPrintHook(const char *msg);
void __cdecl Monkey_Start();
void __cdecl Monkey_AssertCallback(const char *assertMsg);
bool __cdecl Monkey_IsRunning();
void __cdecl Monkey_Frame();
void __cdecl Monkey_KeepAlive();
void __cdecl Monkey_Event(const char *event);
bool __cdecl Monkey_UseRandomInput();
