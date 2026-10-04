#pragma once
#include "live_storage.h"

enum servertype_t : __int32
{                                       // XREF: ?LiveGroups_GetCountByType@@YAIW4servertype_t@@@Z/r
    SERVER_GROUP_RANKED   = 0x1EA,
    SERVER_GROUP_UNRANKED = 0x1EB,
    SERVER_GROUP_WAGER    = 0x1EC,
};

struct GroupSet // sizeof=0x14
{                                       // XREF: .data:s_groupSets/r
    const char *name;                   // XREF: LiveGroups_GetGroupID(char const *,int *,GroupSet * *)+A4/r
    bool exclusive;
    // padding byte
    // padding byte
    // padding byte
    const char **groupNames;
    int capacity;
    unsigned int start;
};

struct GroupMembership // sizeof=0x88
{                                       // XREF: .data:s_groupMembership/r
    unsigned int bits[32];
    int dirty;                          // XREF: LiveGroups_SetGroupsComplete(TaskRecord *)+3F/r
                                        // LiveGroups_SetGroupsComplete(TaskRecord *)+54/w ...
    int updateTime;
};

void __cdecl LiveGroups_Init();
void __cdecl LiveGroups_SetGroupsComplete(TaskRecord *task);
void __cdecl LiveGroups_SetGroups(int localControllerIndex);
void __cdecl LiveGroups_GetGroupCountsComplete(TaskRecord *task);
void __cdecl LiveGroups_GetCounts(int localControllerIndex);
char __cdecl LiveGroups_GetGroupID(char *path, int *offset, GroupSet **gs);
void __cdecl LiveGroups_JoinGroup(int localControllerIndex, char *path);
void __cdecl LiveGroups_LeaveGroup(int localControllerIndex, char *path);
unsigned int __cdecl LiveGroups_GetCount(char *path);
unsigned int __cdecl LiveGroups_GetCountByType(servertype_t type);
void __cdecl LiveGroups_RegisterPlayer(int localControllerIndex);
void __cdecl LiveGroups_Update(int localControllerIndex);
void __cdecl LiveGroups_JoinGroup_f();
void __cdecl LiveGroups_LeaveGroup_f();
void __cdecl LiveGroups_Dump_f();
