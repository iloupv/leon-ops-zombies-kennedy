#pragma once
#include <client_mp/cl_main_mp.h>

enum taskCompleteResults : __int32;

struct XSESSION_INFO // sizeof=0x31
{                                       // XREF: SessionData_s/r
                                        // LaunchData/r ...
    bdSecurityID sessionID;
    XNADDR hostAddress;
    bdSecurityKey keyExchangeKey;
};

struct RegisteredUser // sizeof=0x28
{                                       // XREF: SessionData_s/r
    bool active;
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    unsigned __int64 xuid;
    netadr_t addr;
    bool privateSlot;
    // padding byte
    // padding byte
    // padding byte
    int performanceValue;
};

struct __declspec(align(4)) RemoteTalkerInfo // sizeof=0x20
{                                       // XREF: SessionData_s/r
    bool registered;
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    unsigned __int64 xuid;
    int natType;
    int voiceConnectivityBits;
    int lastConnectivityTestTime[1];
    bool muted;
    // padding byte
    // padding byte
    // padding byte
};

struct LocalTalkerInfo // sizeof=0x1
{                                       // XREF: SessionData_s/r
    bool enabled;
};

struct __declspec(align(8)) SessionData_s // sizeof=0x978
{                                       // XREF: .data:SessionData_s g_serverSession/r
                                        // .data:SessionData_s g_partySession/r ...
    char *sessionName;                  // XREF: Session_Init(void)+38/w
    bool sessionHandle;                 // XREF: SV_SpawnServer(int,char const *,int,int)+690/r
                                        // Live_IsInLiveGame(void)+4/r
    XSESSION_INFO sessionInfo;          // XREF: _dynamic_initializer_for__g_serverSession__+4/o
                                        // _dynamic_initializer_for__g_partySession__+4/o ...
    bool keysGenerated;
    bool sessionStartCalled;
    bool registerUsersWithVoice;        // XREF: Session_Init(void)+42/w
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    unsigned __int64 sessionNonce;
    int privateSlots;                   // XREF: Live_Frame_MP(int,int)+1FB/r
    int publicSlots;                    // XREF: Live_Frame_MP(int,int)+202/r
    int flags;
    struct PS3VoiceEngine *voiceEngine;
    bool qosListenEnabled;
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    RegisteredUser registeredUsers[32];
    RemoteTalkerInfo remoteTalkers[32];
    LocalTalkerInfo localTalkers[4];
    int voiceConnectivityBits;
    int sessionCreateController;
    int ackedMeetBits;
    int lastMeetSendTime;
    // padding byte
    // padding byte
    // padding byte
    // padding byte
};

struct __declspec(align(8)) SessionJoinData // sizeof=0x20
{                                       // XREF: .data:sessionJoinData/r
    bool active;
    // padding byte
    // padding byte
    // padding byte
    int privateSlot;
    int slot;
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    unsigned __int64 player;
    SessionData_s *session;
    // padding byte
    // padding byte
    // padding byte
    // padding byte
};

struct SessionCreateData // sizeof=0x8
{                                       // XREF: .data:SessionCreateData sessionCreateData/r
                                        // .data:sessionCreateData/r
    bool active;                        // XREF: Session_StartHost(SessionData_s *,int,int,int)+146/w
    // Session_GetFreeCreateSessionSlot+1F/r
// padding byte
// padding byte
// padding byte
    SessionData_s *session;             // XREF: Session_StartHost(SessionData_s *,int,int,int)+154/w
};

struct SessionGraveYard // sizeof=0x980
{
    bool active;
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    SessionData_s sessionData;
};

void __cdecl Session_ClearDWOverlappedTasks();
void __cdecl Live_FindSessionsStart(bool reset, int servertype);
void __cdecl Live_FindSessionsPump();
void __cdecl Session_QoSListenStart(SessionData_s *session);
void __cdecl Session_QoSListenStop(SessionData_s *session);
int __cdecl Session_FindRegisteredUser(const SessionData_s *session, unsigned __int64 xuid);
void __cdecl Session_EndGameSession(SessionData_s *session);
void __cdecl Session_DeleteSession(SessionData_s *session);
void __cdecl Session_DeleteHandle(bool *sessionHandle);
void __cdecl Session_UnregisterAllUsersFromVoice(SessionData_s *session);
int __cdecl Session_GetFreeSessionGraveYardSlot();
void __cdecl Session_StartHost(SessionData_s *session, int sessionFlags, int numPrivateSlots, int numPublicSlots);
int __cdecl Session_GetFreeCreateSessionSlot();
void __cdecl Session_Modify(
                int localControllerIndex,
                SessionData_s *session,
                int flags,
                int publicSlots,
                int privateSlots);
taskCompleteResults __cdecl Session_ModifyComplete(int slot);
void __cdecl Session_EveryoneLeaveSessionAsync(int localControllerIndex, SessionData_s *session);
char __cdecl Session_JoinInProgress();
bool __cdecl Session_SessionTasksInProgress(SessionData_s *session);
int __cdecl Session_StartSessionComplete(int slot);
int __cdecl Session_JoinSessionComplete(int slot);
void __cdecl Session_EndOngoingSessionTasks(SessionData_s *session);
taskCompleteResults __cdecl Session_StartHostComplete(int slot);
int __cdecl Session_EveryoneLeaveSessionComplete(int slot);
void __cdecl Live_FinishOngoingSessionJoinTasksForXUID(unsigned __int64 player);
void __cdecl Live_CheckOngoingSessionTasks();
void Session_ManageGraveYard();
void __cdecl Session_Init();

extern SessionData_s g_serverSession;
