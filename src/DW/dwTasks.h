#pragma once
#ifdef BO1_DW

#include <win32/win_tasks.h>

void __cdecl dwClearTask();
int __cdecl dwGetTaskStatus(overlappedTask *const task);
#endif