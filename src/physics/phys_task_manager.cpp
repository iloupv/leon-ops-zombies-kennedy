#include "phys_task_manager.h"
#include <win32/win_workercmds.h>

int g_phys_task_manager_inited;
int g_phys_task_manager_current_worker_count;
jqBatch g_phys_task_manager_batch;

void __cdecl phys_task_manager_init()
{
    iassert(g_phys_task_manager_inited == 0);
    iassert(g_phys_task_manager_current_worker_count == 0);

    g_phys_task_manager_inited = 1;
    g_phys_task_manager_batch.Module = 0;
    g_phys_task_manager_batch.Input = 0;
    g_phys_task_manager_batch.Output = 0;
}

void __cdecl phys_task_manager_shutdown()
{
    iassert(g_phys_task_manager_inited == 1);
    iassert(g_phys_task_manager_current_worker_count == 0);

    g_phys_task_manager_inited = 0;
}

bool __cdecl phys_task_manager_needs_flush()
{
    iassert(g_phys_task_manager_inited == 1);

    return g_phys_task_manager_current_worker_count > 0;
}

void __cdecl phys_task_manager_flush()
{
    iassert(g_phys_task_manager_inited == 1);

    if ( g_phys_task_manager_current_worker_count > 0 )
    {
        g_phys_task_manager_current_worker_count = 0;
        IW_task_manager_flush();
    }
}

void __cdecl phys_task_manager_process(jqModule *module, void *input, int input_count)
{
    int v3; // eax
    int v4; // esi

    iassert(g_phys_task_manager_inited == 1);
    iassert(g_phys_task_manager_current_worker_count == 0);

    v3 = input_count;
    if ( input_count )
    {
        if ( input_count > 6 )
            v3 = 6;
        g_phys_task_manager_current_worker_count = v3;
        g_phys_task_manager_batch.Module = module;
        g_phys_task_manager_batch.Input = input;
        if ( v3 > 0 )
        {
            v4 = v3;
            do
            {
                IW_task_manager_add_batch(&g_phys_task_manager_batch);
                --v4;
            }
            while ( v4 );
        }
    }
}

