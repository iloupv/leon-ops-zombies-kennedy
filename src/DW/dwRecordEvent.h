#pragma once
#ifdef BO1_DW
#include <win32/win_tasks.h>

void __cdecl dwRecordEvent(overlappedTask *const task, char *buff, unsigned int buffSize, unsigned int category);
void __cdecl dwRecordEventAscii(overlappedTask *const task, char *buff, unsigned int buffSize, unsigned int category);
void dwRecordEventFormat(overlappedTask *const task, unsigned int category, const char *fmt, ...);
taskCompleteResults __cdecl dwRecordEventComplete(overlappedTask *const task);
#endif