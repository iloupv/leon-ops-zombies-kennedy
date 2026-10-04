#pragma once

enum streamName_t : __int32;

struct countersNewsState_t // sizeof=0x34
{                                       // XREF: .data:s_countersNewsState/r
    int total;                          // XREF: LiveNews_ProcessCounter(int)+19/r
                                        // LiveNews_InitCountersTicker(void)+37/w ...
    int numProcessed;                   // XREF: LiveNews_ProcessCounter(int)+49/r
                                        // LiveNews_ProcessCounter(int)+8E/r ...
    int counters[10];                   // XREF: LiveNews_ProcessCounter(int)+5D/r
                                        // LiveNews_ProcessCounter(int)+9A/w
    int lastFlushTime;                  // XREF: LiveNews_UpdateCountersTicker(int)+8/r
                                        // LiveNews_UpdateCountersTicker(int)+24/w ...
};

struct TickerMessageQueue // sizeof=0x504
{                                       // XREF: .data:s_tickerMessageQueue/r
    char messageToBeAddedToTicker[10][128];
    int newsItemCount;                  // XREF: LiveNews_PopulateTickerQueueMessages(void):loc_96D4B8/r
                                        // LiveNews_PopulateTickerQueueMessages(void)+21/r ...
};

struct PCachePublicProfile;
struct CmdArgs;

char __cdecl LiveNews_NeedToGetFriendNews();
void __cdecl LiveNews_PopulateTickerQueueMessages();
int __cdecl LiveNews_FindDuplicateNewsItemIndex(char *outputString);
void __cdecl LiveNews_PopulateFriendNews(int controllerIndex, unsigned __int64 xuid, char *name);
void __cdecl LiveNews_GetFriendsXuids(int controllerIndex);
char __cdecl LiveNews_GetFriendsPublicProfile(int controllerIndex, unsigned __int64 xuid);
void __cdecl LiveNews_UpdateFriendsNewsTicker(int localControllerIndex);
void __cdecl LiveNews_GetOwnNews(int controllerIndex);
int __cdecl LiveNews_GetNextIndexForPublishingNews(PCachePublicProfile *profile, CmdArgs *cmd_args);
void __cdecl LiveNews_PublishNews(int controllerIndex, CmdArgs *cmd_args);
void __cdecl LiveNews_PublishNews_f();
void __cdecl LiveNews_GetOwnNews_f();
void __cdecl LiveNews_Init();
void __cdecl formatStringWithCommas(char *outputString, int stringLength, unsigned __int64 value);
bool __cdecl LiveNews_EvaluateExpression(const char *expr, char *outputString, int stringLength);
char __cdecl LiveNews_EvaluateCounter(int row, char *outputString, int stringLength);
char __cdecl LiveNews_ProcessCounter();
void __cdecl LiveNews_UpdateCountersTicker();
void __cdecl LiveNews_InitCountersTicker();
