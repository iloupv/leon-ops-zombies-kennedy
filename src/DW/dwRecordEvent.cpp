#ifdef BO1_DW

#include "dwRecordEvent.h"

void __cdecl dwRecordEvent(overlappedTask *const task, char *buff, unsigned int buffSize, unsigned int category)
{
#ifdef BO1_DEMON
    bdReference<bdCommonAddr> v4; // [esp+20h] [ebp-18h] BYREF
    bdRemoteTask *remoteTask; // [esp+24h] [ebp-14h]
    unsigned __int64 userID; // [esp+28h] [ebp-10h] BYREF
    bdTitleUtilities *titleUtil; // [esp+34h] [ebp-4h]

    titleUtil = dwGetTitleUtilities(task->controllerIndex);
    if ( titleUtil && task && dwGetOnlineUserID(task->controllerIndex, &userID) )
    {
        remoteTask = *(bdRemoteTask **)bdTitleUtilities::recordEventBin(
                                                                         titleUtil,
                                                                         (int)&v4,
                                                                         (unsigned __int8 *)buff,
                                                                         buffSize,
                                                                         category);
        bdReference<bdRemoteTask>::~bdReference<bdRemoteTask>(&v4);
        if ( remoteTask )
        {
            bdReference<bdCommonAddr>::operator=(&task->overlappedIO, remoteTask);
        }
        else
        {
            TaskManager_ClearTask(task);
            Com_Printf(16, "Error occurred when uploading title data.\n");
        }
    }
#endif
}

void __cdecl dwRecordEventAscii(overlappedTask *const task, char *buff, unsigned int buffSize, unsigned int category)
{
#ifdef BO1_DEMON
    bdReference<bdCommonAddr> v4; // [esp+20h] [ebp-18h] BYREF
    bdRemoteTask *remoteTask; // [esp+24h] [ebp-14h]
    unsigned __int64 userID; // [esp+28h] [ebp-10h] BYREF
    bdTitleUtilities *titleUtil; // [esp+34h] [ebp-4h]

    titleUtil = dwGetTitleUtilities(task->controllerIndex);
    if ( titleUtil && task && dwGetOnlineUserID(task->controllerIndex, &userID) )
    {
        remoteTask = *(bdRemoteTask **)bdTitleUtilities::recordEvent(
                                                                         titleUtil,
                                                                         (int)&v4,
                                                                         (unsigned __int8 *)buff,
                                                                         buffSize,
                                                                         category);
        bdReference<bdRemoteTask>::~bdReference<bdRemoteTask>(&v4);
        if ( remoteTask )
        {
            bdReference<bdCommonAddr>::operator=(&task->overlappedIO, remoteTask);
        }
        else
        {
            TaskManager_ClearTask(task);
            Com_Printf(16, "Error occurred when uploading title data.\n");
        }
    }
#endif
}

void dwRecordEventFormat(overlappedTask *const task, unsigned int category, const char *fmt, ...)
{
#ifdef BO1_DEMON
    char event[68]; // [esp+10h] [ebp-50h] BYREF
    unsigned int MAX_EVENT_LEN; // [esp+58h] [ebp-8h]
    char *args; // [esp+5Ch] [ebp-4h]
    va_list va; // [esp+74h] [ebp+14h] BYREF

    va_start(va, fmt);
    MAX_EVENT_LEN = 63;
    va_copy(args, va);
    _vsnprintf(event, 0x3Fu, fmt, va);
    args = 0;
    event[62] = 0;
    if ( !task )
    {
        if ( (_S1_7 & 1) == 0 )
        {
            _S1_7 |= 1u;
            overlappedIO.overlappedIO.m_ptr = 0;
            atexit(dwRecordEventFormat_::_5_::_dynamic_atexit_destructor_for__overlappedIO__);
        }
        task = &overlappedIO;
        dwClearTask();
    }
    dwRecordEventAscii(task, event, &event[strlen(event) + 1] - &event[1], category);
#endif
}

taskCompleteResults __cdecl dwRecordEventComplete(overlappedTask *const task)
{
#ifdef BO1_DEMON
    enum bdRemoteTask::bdStatus taskStatus; // [esp+Ch] [ebp-10h]
    taskCompleteResults status; // [esp+14h] [ebp-8h]
    enum bdLobbyErrorCode errorCode; // [esp+18h] [ebp-4h]

    status = TASK_ERROR;
    errorCode = BD_NO_ERROR;
    if ( dwGetTitleUtilities(task->controllerIndex) && task && task->overlappedIO.m_ptr )
    {
        dwLobbyPump(task->controllerIndex);
        taskStatus = bdRemoteTask::getStatus(task->overlappedIO.m_ptr);
        if ( taskStatus == BD )
        {
            errorCode = bdRemoteTask::getErrorCode(task->overlappedIO.m_ptr);
            if ( errorCode )
                Com_Printf(16, "Title data uploading failed to complete.\n");
        }
        status = dwTaskStatusConvert(taskStatus, errorCode);
        if ( taskStatus != BD_PENDING )
            dwCloseRemoteTask(&task->overlappedIO);
    }
    else
    {
        Com_Printf(16, "Failed to retrieve bdTitleUtilities for record event complete.\n");
    }
    return status;
#else
    return TASK_COMPLETE;
#endif
}


#endif