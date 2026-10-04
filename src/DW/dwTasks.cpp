#ifdef BO1_DW

#include "dwTasks.h"
#include "dwUtils.h"
#include <universal/assertive.h>

void __cdecl dwClearTask()
{
    ;
}

int __cdecl dwGetTaskStatus(overlappedTask *const task)
{
#ifdef BO1_DEMON
    enum bdRemoteTask::bdStatus taskStatus; // [esp+8h] [ebp-8h]
    taskCompleteResults status; // [esp+Ch] [ebp-4h]

    status = TASK_ERROR;
    if ( !task && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\Dw\\dwTasks.cpp", 21, 0, "%s", "task") )
        __debugbreak();
    if ( task && task->overlappedIO.m_ptr )
    {
        //taskStatus = bdRemoteTask::getStatus(task->overlappedIO.m_ptr);
        taskStatus = task->overlappedIO.m_ptr->getStatus();
        return dwTaskStatusConvert(taskStatus, BD_NO_ERROR);
    }
    return status;
#else
    return TASK_ERROR;
#endif
}

#endif