#pragma once
#include "snd.h"

void __cdecl SND_InitCommands();
snd_command *__cdecl SND_GetNewCommand();
int __cdecl SND_NextCommandIndex(__int16 i);
unsigned int __cdecl SND_CommandPush(snd_command *cmd);
const snd_command *__cdecl SND_CommandPeek();
void __cdecl SND_CommandPop();
unsigned int __cdecl SND_CommandPump();
void __cdecl SND_CommandProcess(const snd_command *cmd);
snd_notify *__cdecl SND_GetNewNotify();
int __cdecl SND_NextNotifyIndex(__int16 i);
void __cdecl SND_NotifyPush(snd_notify *cmd);
const snd_notify *__cdecl SND_NotifyPeek();
void __cdecl SND_NotifyPop();
void __cdecl SND_NotifyPump();
void __cdecl SND_NotifyProcess(const snd_notify *cmd);
bool __cdecl SND_GetProcessCommandFlag();
