#pragma once

#ifdef BO1_DW
TaskRecord *__cdecl dwMessaging_SendInstantMessage(
                unsigned __int64 recipientID,
                char *payload,
                unsigned int payloadsize);
bdRemoteTask *__cdecl dwMessaging_SendDeferredInstantMessage(
                unsigned __int64 recipientID,
                unsigned __int8 *payload,
                unsigned int payloadsize);
void __cdecl dwInitMessaging(int controllerIndex);
void __cdecl dwUpdateMessaging(int controllerIndex);
void __cdecl dwProcessNotifications(TaskRecord *task);
void __thiscall dwLobbyEventHandler::onNewNotification(dwLobbyEventHandler *this);
void __thiscall dwLobbyEventHandler::onTeamMemberStatusUpdate(
                dwLobbyEventHandler *this,
                unsigned __int64 __formal,
                char *a3,
                void *a4,
                unsigned int a5,
                unsigned __int64 a6);
void __thiscall dwLobbyEventHandler::onTeamProposal(dwLobbyEventHandler *this);
void __thiscall dwLobbyEventHandler::onGlobalInstantMessage(
                dwLobbyEventHandler *this,
                unsigned __int64 senderID,
                char *senderName,
                unsigned __int8 *message,
                unsigned int messageSize);
void __thiscall dwLobbyEventHandler::onNotifyLeave(
                dwLobbyEventHandler *this,
                unsigned __int64 userID,
                bdSessionID sessionID);
void __thiscall dwLobbyEventHandler::onSessionInvite(dwLobbyEventHandler *this, bdSessionInvite *invite);
void __thiscall dwLobbyEventHandler::onMultipleLogon(dwLobbyEventHandler *this, unsigned __int64 userID);
#endif