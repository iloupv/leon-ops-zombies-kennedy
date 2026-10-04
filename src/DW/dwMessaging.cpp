#ifdef BO1_DW
#include "dwMessaging.h"

TaskRecord *__cdecl dwMessaging_SendInstantMessage(
                unsigned __int64 recipientID,
                char *payload,
                unsigned int payloadsize)
{
    const bdReference<bdCommonAddr> *v3; // eax
    bdReference<bdCommonAddr> v5; // [esp+1Ch] [ebp-Ch] BYREF
    TaskRecord *task; // [esp+20h] [ebp-8h]
    bdMessaging *messaging; // [esp+24h] [ebp-4h]

    task = 0;
    messaging = dwGetMessaging(0);
    if ( messaging && payload && payloadsize && recipientID )
    {
        task = TaskManager2_CreateTask(task_sendMessage, 0, 0, 0);
        v3 = (const bdReference<bdCommonAddr> *)bdMessaging::sendGlobalInstantMessage(
                                                                                            messaging,
                                                                                            (int)&v5,
                                                                                            recipientID,
                                                                                            (unsigned __int8 *)payload,
                                                                                            payloadsize);
        bdReference<bdCommonAddr>::operator=((bdReference<bdCommonAddr> *)&task->remoteTask, v3);
        bdReference<bdRemoteTask>::~bdReference<bdRemoteTask>(&v5);
        TaskManager2_StartTask(task);
    }
    else
    {
        Com_PrintWarning(0, "Failed to send instant message. You're doing it wrong.\n");
    }
    return task;
}

bdRemoteTask *__cdecl dwMessaging_SendDeferredInstantMessage(
                unsigned __int64 recipientID,
                unsigned __int8 *payload,
                unsigned int payloadsize)
{
    bdReference<bdCommonAddr> v4; // [esp+10h] [ebp-Ch] BYREF
    bdRemoteTask *retval; // [esp+14h] [ebp-8h]
    bdMessaging *messaging; // [esp+18h] [ebp-4h]

    retval = 0;
    messaging = dwGetMessaging(0);
    if ( messaging && payload && payloadsize && recipientID )
    {
        dwEnterDeferredCritsec();
        retval = *(bdRemoteTask **)bdMessaging::sendGlobalInstantMessage(
                                                                 messaging,
                                                                 (int)&v4,
                                                                 recipientID,
                                                                 payload,
                                                                 payloadsize);
        bdReference<bdRemoteTask>::~bdReference<bdRemoteTask>(&v4);
        dwLeaveDeferredCritsec();
        if ( retval && bdRemoteTask::getStatus(retval) == BD_PENDING )
            TaskManager2_DeferTaskToMainThread(retval, task_sendMessage, 0);
    }
    else
    {
        Com_PrintWarning(0, "Failed to send message. You're doing it wrong.\n");
    }
    return retval;
}

void __cdecl dwInitMessaging(int controllerIndex)
{
    bdLobbyService *lobby; // [esp+0h] [ebp-4h]

    if ( !Sys_IsMainThread()
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\DW\\dwMessaging.cpp", 474, 0, "%s", "Sys_IsMainThread()") )
    {
        __debugbreak();
    }
    lobby = dwGetLobby(controllerIndex);
    if ( !lobby
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\DW\\dwMessaging.cpp", 480, 0, "%s", "lobby != NULL") )
    {
        __debugbreak();
    }
    bdLobbyService::registerEventHandler(lobby, &s_eventHandler);
    Com_PrintWarning(16, "Initialized Messaging for controller %d.\n", controllerIndex);
}

void __cdecl dwUpdateMessaging(int controllerIndex)
{
    const bdReference<bdCommonAddr> *Notifications; // eax
    bdReference<bdCommonAddr> v2; // [esp+1Ch] [ebp-Ch] BYREF
    TaskRecord *task; // [esp+20h] [ebp-8h]
    bdMessaging *messaging; // [esp+24h] [ebp-4h]

    if ( !Sys_IsMainThread()
        && !Sys_IsServerThread()
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\DW\\dwMessaging.cpp",
                    494,
                    0,
                    "%s",
                    "Sys_IsMainThread() || Sys_IsServerThread()") )
    {
        __debugbreak();
    }
    messaging = dwGetMessaging(controllerIndex);
    if ( s_notificationsPending && messaging && !TaskManager2_TaskIsInProgress(task_processNotifications) )
    {
        task = TaskManager2_CreateTask(task_processNotifications, controllerIndex, 0, 0);
        if ( !task && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\DW\\dwMessaging.cpp", 506, 0, "%s", "task") )
            __debugbreak();
        s_notificationsPending = 0;
        Notifications = (const bdReference<bdCommonAddr> *)bdMessaging::getNotifications(
                                                                                                                 messaging,
                                                                                                                 (int)&v2,
                                                                                                                 &s_notifications,
                                                                                                                 0,
                                                                                                                 0xAu,
                                                                                                                 1);
        bdReference<bdCommonAddr>::operator=((bdReference<bdCommonAddr> *)&task->remoteTask, Notifications);
        bdReference<bdRemoteTask>::~bdReference<bdRemoteTask>(&v2);
        TaskManager2_StartTask(task);
    }
}

void __cdecl dwProcessNotifications(TaskRecord *task)
{
    unsigned int HeaderSize; // esi
    bdReference<bdRemoteTask> taskRef; // [esp+2Ch] [ebp-8h] BYREF
    unsigned int n; // [esp+30h] [ebp-4h]

    if ( !Sys_IsMainThread()
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\DW\\dwMessaging.cpp", 518, 0, "%s", "Sys_IsMainThread()") )
    {
        __debugbreak();
    }
    taskRef.m_ptr = task->remoteTask.m_ptr;
    if ( taskRef.m_ptr )
        InterlockedIncrement(&taskRef.m_ptr->m_refCount);
    for ( n = 0; n < bdTaskByteBuffer::getHeaderSize((bdTaskByteBuffer *)taskRef.m_ptr); ++n )
        Com_DPrintf(0, "new notification: %d\n", dword_3369F3C[60 * n]);
    HeaderSize = bdTaskByteBuffer::getHeaderSize((bdTaskByteBuffer *)taskRef.m_ptr);
    if ( HeaderSize != bdRemoteTask::getTotalNumResults(taskRef.m_ptr) )
        s_notificationsPending = 1;
    bdReference<bdRemoteTask>::~bdReference<bdRemoteTask>((bdReference<bdCommonAddr> *)&taskRef);
}

void __thiscall dwLobbyEventHandler::onNewNotification(dwLobbyEventHandler *this)
{
    s_notificationsPending = 1;
}

void __thiscall dwLobbyEventHandler::onTeamMemberStatusUpdate(
                dwLobbyEventHandler *this,
                unsigned __int64 __formal,
                char *a3,
                void *a4,
                unsigned int a5,
                unsigned __int64 a6)
{
    ;
}

void __thiscall dwLobbyEventHandler::onTeamProposal(dwLobbyEventHandler *this)
{
    int LocalClientNum; // eax
    char *v2; // [esp-Ch] [ebp-10h]
    char *v3; // [esp-8h] [ebp-Ch]

    v3 = UI_SafeTranslateString("MENU_CLAN_INVITATION_RECEIVED");
    v2 = UI_SafeTranslateString("MENU_INVITE");
    LocalClientNum = Com_ControllerIndex_GetLocalClientNum(0);
    UI_OpenToastPopup(LocalClientNum, "menu_mp_killstreak_select", v2, v3, 2700);
}

void __thiscall dwLobbyEventHandler::onGlobalInstantMessage(
                dwLobbyEventHandler *this,
                unsigned __int64 senderID,
                char *senderName,
                unsigned __int8 *message,
                unsigned int messageSize)
{
    if ( !Live_CACValidate_DispatchMessage(senderID, message, messageSize)
        && !Live_HandleDWChallengeResponse(senderID, message, messageSize)
        && !Live_HandleInviteMessage(senderID, (char *)message) )
    {
        Com_DPrintf(0, "Unhandled instant message from %s\n", senderName);
    }
}

void __thiscall dwLobbyEventHandler::onNotifyLeave(
                dwLobbyEventHandler *this,
                unsigned __int64 userID,
                bdSessionID sessionID)
{
    bdSecurityID *SessionSecID; // eax
    const bdSecurityID *v4; // eax

    SessionSecID = (bdSecurityID *)dwGetSessionSecID();
    if ( bdSecurityID::operator==(sessionID.m_sessionID.ab, SessionSecID) )
    {
        SV_DWUserDisconnected(userID);
    }
    else
    {
        v4 = dwGetSessionSecID();
        Com_DPrintf(
            15,
            "Er, we got a notifyleave for sessionID %llu, but we're apparently adverting %llu. Ask Ewan.",
            *(_QWORD *)&sessionID.m_sessionID,
            *v4);
    }
    bdTaskResult::~bdTaskResult(&sessionID);
}

void __thiscall dwLobbyEventHandler::onSessionInvite(dwLobbyEventHandler *this, bdSessionInvite *invite)
{
    bdSessionID v2; // [esp-10h] [ebp-20h] BYREF
    const char *v3; // [esp-4h] [ebp-14h]
    dwLobbyEventHandler *thisa; // [esp+0h] [ebp-10h]
    const char *password; // [esp+8h] [ebp-8h]
    int protocolversion; // [esp+Ch] [ebp-4h]

    thisa = this;
    Com_Printf(23, "Received invite from %s\n", invite->m_senderName);
    if ( invite->m_attachmentSize >= 4 )
    {
        protocolversion = 0;
        protocolversion = *(unsigned int *)invite->m_attachment;
        if ( protocolversion == 1044 )
        {
            password = (const char *)&invite->m_attachment[4];
            v3 = (const char *)&invite->m_attachment[4];
            bdSessionID::bdSessionID(&v2, &invite->m_sessionID);
            Live_OnInvite(invite->m_senderID, v2, v3);
        }
        else
        {
            Com_DPrintf(
                14,
                "dropping invite from %s on the floor, their proto is %i, ours is %i\n",
                invite->m_senderName,
                protocolversion,
                1044);
        }
    }
}

void __thiscall dwLobbyEventHandler::onMultipleLogon(dwLobbyEventHandler *this, unsigned __int64 userID)
{
    Com_Error(ERR_FATAL, "EXE_DW_MULTIPLE_LOGON");
}


#endif