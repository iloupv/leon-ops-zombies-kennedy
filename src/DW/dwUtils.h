#pragma once
#ifdef BO1_DW
#include <win32/win_tasks.h>

struct bdLobbyService;
struct bdMatchMaking;
struct bdGroup;
struct bdCounter;
struct bdContentStreaming;
struct bdPooledStorage;
struct bdTags;
struct bdVoteRank;
struct bdStorage;
struct bdStats;
struct bdSecurityKeyMap;
struct bdSocketRouter;
struct bdAddressMap;
struct bdQoSProbe;
struct bdAuthService;
struct bdTitleUtilities;
struct bdMessaging;

bdLobbyService *__cdecl dwGetLobby(int controllerIndex);
bdMatchMaking *__cdecl dwGetMatchmaking(int controllerIndex);
bdGroup *__cdecl dwGetGroup(int controllerIndex);
bdCounter *__cdecl dwGetCounter(int controllerIndex);
bdContentStreaming *__cdecl dwGetContentStreaming(int controllerIndex);
bdPooledStorage *__cdecl dwGetPooledStorage(int controllerIndex);
bdTags *__cdecl dwGetTagService(int controllerIndex);
bdVoteRank *__cdecl dwGetVoteRankService(int controllerIndex);
bdStorage *__cdecl dwGetStorage(int controllerIndex);
bdStats *__cdecl dwGetStats(int controllerIndex);
bdSecurityKeyMap *__cdecl dwGetSecurityKeyMap();
bdSocketRouter *__cdecl dwGetSocketRouter();
bdAddressMap *__cdecl dwGetAddressMap();
bdQoSProbe *__cdecl dwGetQoSProbe();
bdReference<bdCommonAddr> *__cdecl dwGetLocalCommonAddr(bdReference<bdCommonAddr> *result);
bdAuthService *__cdecl dwGetAuthService(bdInetAddr authAddr, unsigned __int16 authPort, unsigned int titleID);
void __cdecl dwAuthServiceCleanup();
bdTitleUtilities *__cdecl dwGetTitleUtilities(int controllerIndex);
bdMessaging *__cdecl dwGetMessaging(int controllerIndex);
void __cdecl dwCloseRemoteTask(overlappedTask *task);
void __cdecl dwCloseRemoteTask(bdReference<bdRemoteTask> *remoteTask);
int __cdecl dwTaskStatusConvert(bdRemoteTask::bdStatus taskStatus, bdLobbyErrorCode lobbyError);
const char *__cdecl dwLobbyErrorCodeToString(bdLobbyErrorCode code);
void __cdecl dwLobbyErrorCodeToString(bdLobbyErrorCode code, char *const buffer, unsigned int bufferSize);
unsigned int __cdecl bdStrlcpy(char *const dst, char *src, unsigned int size);
void __cdecl dwConnectionErrorHandler(int error_context);
void __cdecl dwEnterDeferredCritsec();
void __cdecl dwLeaveDeferredCritsec();
#endif