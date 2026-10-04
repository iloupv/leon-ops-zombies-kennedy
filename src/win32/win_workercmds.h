#pragma once

struct jqBatch;

int __cdecl nuge_physicsCallback(jqBatch *batch);
void __cdecl IW_task_manager_add_batch(jqBatch *ptr);
void __cdecl IW_task_manager_flush();
void __cdecl R_InitWorkerCmds();
void __cdecl R_InitWorkerThreads();
