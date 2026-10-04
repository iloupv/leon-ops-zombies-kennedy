#pragma once
#include "live_storage.h"
#include <DW/MatchMakingInfo_win32.h>
#include <universal/UserInfo.h>

enum EUserTier : __int32
{                                       // XREF: XenonUserData/r
                                        // GetIsSuperUser/r
    USER_TIER_NONE      = 0x0,
    USER_TIER_SUPER     = 0x1,
    USER_TIER_DEVELOPER = 0x2,
    USER_TIER_FRIEND    = 0x3,
    USER_TIER_ENEMY     = 0x29A,
};

struct __declspec(align(8)) PlayerRank // sizeof=0x10
{                                       // XREF: Live_UpdatePerformanceValuesComplete/r
    unsigned __int64 xuid;              // XREF: Live_UpdatePerformanceValuesComplete+F3/r
                                        // Live_UpdatePerformanceValuesComplete+FB/r
    int rank;                           // XREF: Live_UpdatePerformanceValuesComplete+139/r
    // padding byte
    // padding byte
    // padding byte
    // padding byte
};

struct __declspec(align(8)) recentServer_t // sizeof=0x10
{                                       // XREF: .data:s_recentServers/r
    unsigned __int64 serverID;          // XREF: Live_FindRecentServers(void)+84/r
                                        // Live_SaveRecentServers(void)+31/r ...
    int joinTime;                       // XREF: Live_FindRecentServers(void)+6A/r
                                        // Live_SaveRecentServers(void)+74/r ...
    // padding byte
    // padding byte
    // padding byte
    // padding byte
};

struct favourite_t // sizeof=0x10
{                                       // XREF: .data:s_favourites/r
    unsigned __int8 addressblob[6];
    // padding byte
    // padding byte
    unsigned __int64 uid;               // XREF: Live_DumpFavourites(void)+2A/r
                                        // Live_FindFavouriteServers(void)+76/r ...
};

struct XenonUserData // sizeof=0x50
{
    int signinState;
    char gamertag[32];
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    unsigned __int64 xuid;
    char xuidString[17];
    bool isGuestUser;
    // padding byte
    // padding byte
    EUserTier tier;
    __int64 totalGamesPlayed;
};

struct dwQoSMultiProbeListener;

void __cdecl Live_ClearDWOverlappedTasks();
char __cdecl Live_ContentRatingAllowed();
bool __cdecl Live_IsUserSignedInToLive();
bool __cdecl Live_IsUserSignedInToDemonware(int controllerIndex);
char __cdecl Live_RequireUserToPlayOnline();
char *__cdecl Live_ControllerIndex_GetClientName(int controllerIndex);
int __cdecl CL_ControllerIndex_GetSignInState(int controllerIndex);
void __cdecl Live_InitiateDemonWareConnect_f();
void __cdecl Live_SendInvite_f();
void __cdecl Live_AcceptInvite_f();
void __cdecl Live_AcceptLastInvite_f();
void __cdecl Live_RevokeInvite_f();
void __cdecl Live_JoinSessionInProgress_f();
void __cdecl Live_ToggleMute_f();
void __cdecl Live_InitPlatform();
bool __cdecl Live_HandleDWChallengeResponse(
                unsigned __int64 senderID,
                unsigned __int8 *message,
                unsigned int messageSize);
int __cdecl Live_GetControllerFromXUID(unsigned __int64 player);
void __cdecl Live_GiveAchievement(int localControllerIndex, const char *achievementName);
bool __cdecl Live_IsInLiveGame();
void __cdecl Session_CleanUpStatsWrites();
taskCompleteResults __cdecl Live_SetPlayerTeamRankComplete(int slot);
taskCompleteResults __cdecl Live_SetPlayerTeamRanksComplete(int slot);
char __cdecl XUserGetXUID(int controllerIndex, unsigned __int64 *xuid);
char __cdecl Live_UserGetName(int controllerIndex, char *buf, int bufsize);
bool __cdecl Live_UserSignedInLocally(int controllerIndex, char **disconnectMessage);
void __cdecl Live_UserSignedIn(int controllerIndex);
bool __cdecl Live_UserSignedInToLive(int controllerIndex, char **disconnectMessage);
bool __cdecl Live_IsSignedIn(int controllerIndex);
bool __cdecl Live_IsSignedInToLive();
unsigned long long __cdecl Live_GetXuid(int controllerIndex);
int __cdecl Live_GetTier(int controllerIndex);
char __cdecl Live_ShowMarketplaceUI();
int __cdecl Live_GetUploadSpeed();
void __cdecl Live_GetOurUploadBandwidth(int localControllerIndex);
char __cdecl Live_BandwidthTestInProgress();
void __cdecl Live_CheckOngoingTasks();
int __cdecl Live_GetBandwidthTestComplete(int slot);
taskCompleteResults __cdecl Live_QoSProbeComplete(int slot);
bool __cdecl Live_QoSProbeEarlyComplete(dwQoSMultiProbeListener *listener);
void __cdecl PC_InitSigninState();
void Live_LogInProfileLocally_SP(int controllerIndex);
bool __cdecl Live_UserSignedOut(int controllerIndex);
void __cdecl Live_DelayedComError(const char *comErrorString);
void __cdecl Live_Frame();
int __cdecl Live_FetchPartyPerformanceValuesComplete();
taskCompleteResults __cdecl Live_UpdatePerformanceValuesComplete(int slot);
void Live_UpdateAveragePerformance();
int __cdecl Live_GetAveragePerformance();
char __cdecl Live_IsUserBlocked(int controllerIndex, unsigned __int64 xuid);
bool __cdecl Live_CanConsoleViewContentFromUser(unsigned __int64 xuid);
bool __cdecl Live_CanViewContentFromUser(int controllerIndex, unsigned __int64 xuid);
void __cdecl Live_SendInvite(const char *friendName);
void __cdecl Live_DumpFavourites();
void __cdecl Live_FindFavouriteServersSuccess(TaskRecord *task);
bool __cdecl Live_FindFavouritesInProgress();
void __cdecl Live_FindFavouriteServers();
void __cdecl Live_FindFriendServersSuccess(TaskRecord *task);
void __cdecl Live_FindFriendServers();
void __cdecl Live_FindRecentServersSuccess(TaskRecord *task);
void __cdecl Live_FindRecentServers();
void __cdecl Live_SaveRecentServers();
void __cdecl Live_ReadRecentServers();
void __cdecl Live_PopulateRecentServers(unsigned __int8 *buf, int bufsize);
void __cdecl Live_AddRecentServer(unsigned __int64 serveruid);
int __cdecl compareRecentServers(unsigned int *server1, unsigned int *server2);
void __cdecl Live_GetFriendsOnServer(unsigned __int64 serverId, unsigned __int64 *friendIDs, int *numfriends);
char __cdecl Live_AddFavourite_Ingame(unsigned __int64 serverid, unsigned __int64 serveruid);
void __cdecl Live_AddFavourite(unsigned __int64 serverid, unsigned __int64 serveruid);
void __cdecl Live_DeleteFavourite(unsigned __int64 serverid);
void __cdecl Live_ParseFavsBlobs(unsigned __int8 *addrblob, unsigned __int8 *uidblob);
void __cdecl Live_SetFavsBlobs(unsigned __int8 *addrblob, unsigned __int8 *uidblob);
void __cdecl Live_GetPrivateProfileComplete();
void __cdecl Live_GetPrivateProfileFailure();
void __cdecl Live_SetPrivateProfileComplete();
void __cdecl Live_SetPrivateProfileFailure();
TaskRecord *__cdecl Live_GetPrivateProfile();
TaskRecord *__cdecl Live_SetPrivateProfile();
void __cdecl CL_GetFavourites_f();
void __cdecl CL_SetFavourites_f();
void __cdecl CL_AddFavourite_f();
void __cdecl CL_DeleteFavourite_f();
void __cdecl CL_DumpFavourites_f();
void __cdecl CL_NukeFavourites_f();
void __cdecl Live_InitFavourites();
void __cdecl Live_AddRecentPlayers(unsigned __int64 *uids, const char **names, int numIDs);
unsigned __int64 __cdecl Live_GetServerForFriend(unsigned __int64 friendId);
void __cdecl Live_JoinSessionInProgressComplete(TaskRecord *task);
TaskRecord *__cdecl Live_JoinSessionInProgress(unsigned __int64 uid, bool recent);
void __cdecl Live_AddFriendServer(unsigned __int64 serverID, unsigned __int64 friendID);
void __cdecl Live_OnInvite(unsigned __int64 uid, bdSessionID sessionID, const char *password);
void __cdecl Live_OnRevokeInvite(unsigned __int64 uid);
bool __cdecl Live_RevokeInvite(unsigned __int64 friendID);
char __cdecl Live_FindInviteFromFriend(unsigned __int64 friendID, bdSessionID *sessionID, char **password);
char __cdecl Live_HandleInviteMessage(unsigned __int64 senderID, char *message);
void __cdecl Live_AcceptInviteAsyncFailure();
void __cdecl Live_JoinWagerFromInvite();
void __cdecl Live_AcceptInviteAsyncComplete(TaskRecord *task);
TaskRecord *__cdecl Live_AcceptInviteAsync(bdSessionID sessionID);
void __cdecl Live_AcceptInvite(unsigned __int64 frienduid);
int __cdecl Live_GetInvitesCount();
int __cdecl Live_GetInviteFriend(int index);
void __cdecl Live_ListInvites_f();
void __cdecl Live_AddPlayerAsFriend_f();
bool __cdecl Live_ShouldBroadcastNewServer();
void __cdecl Live_RespondToSessionRequest(unsigned __int64 from, unsigned __int8 flags);
void __cdecl Live_RequestSessionsFromFriends();
void __cdecl Live_RequestSessionsFromRecentPlayers();
void __cdecl Live_DispatchP2PMessage(unsigned __int8 *message, unsigned int messagesize, unsigned __int64 from);
void __cdecl Live_BroadcastSessionToFriends(unsigned __int64 sessionUID, unsigned __int8 flags);
void __cdecl Live_BroadcastSessionToRecentPlayers(unsigned __int64 sessionUID, unsigned __int8 flags);
void __cdecl Live_BroadcastSessionIfNeeded();

#ifdef BO1_DW
extern MatchMakingInfo *g_matchmakingInfo;
#endif

extern unsigned __int64 s_selectedPlayerXUID;
extern unsigned __int64 s_selectedMetPlayerXUID;


extern const dvar_t *live_service;
extern const dvar_t *dw_loggedin;
extern const dvar_t *dw_active;
extern const dvar_t *pc_newversionavailable;
extern const dvar_t *dw_dupe_key;
extern const dvar_t *xblive_loggedin;
extern const dvar_t *xenon_voiceDebug;
extern const dvar_t *xenon_voiceDegrade;
extern const dvar_t *getdlcmapsfrommaindrive;
extern const dvar_t *session_nonblocking;
extern const dvar_t *systemUiActive;
extern const dvar_t *xblive_showmarketplace;
extern const dvar_t *xblive_clanmatch;
extern const dvar_t *xblive_theater;
extern const dvar_t *xblive_hostingprivateparty;
extern const dvar_t *xblive_privatepartyclient;
extern const dvar_t *xblive_wagermatch;
extern const dvar_t *xblive_basictraining;
extern const dvar_t *xblive_basictraining_popup;
extern const dvar_t *bot_tips;
extern const dvar_t *party_simulateLongQoS;
extern const dvar_t *xblive_clanListChanged;
extern const dvar_t *teamsplitter_verbose;
extern const dvar_t *xblive_matchEndingSoon;
extern const dvar_t *ui_isClanMember;
extern const dvar_t *dw_numaccounts;
extern const dvar_t *xenon_maxVoicePacketsPerSec;
extern const dvar_t *xenon_maxVoicePacketsPerSecForServer;
extern const dvar_t *bandwidth_retry_interval;
extern const dvar_t *xblive_mappacks;
extern const dvar_t *bot_friends;
extern const dvar_t *bot_enemies;
extern const dvar_t *invite_waitPeriod;
extern const dvar_t *steamid;
extern const dvar_t *dw_popup;
extern const dvar_t *inviteText;
extern const dvar_t *scr_bot_difficulty;
extern const dvar_t *clancard_clanid;
extern const dvar_t *clanName;
extern const dvar_t *bot_difficulty;
extern const dvar_t *dw_usernames[5];
