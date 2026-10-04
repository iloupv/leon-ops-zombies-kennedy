#pragma once

struct ddlEnumDef_t // sizeof=0xC
{
    const char *name;
    int memberCount;
    const char **members;
};

union ddlResult_t // sizeof=0x40
{                                       // XREF: SV_SetClientDIntStat(int,ddlState_t *,uint)+171/r
                                        // SV_SetClientDInt64Stat(int,ddlState_t *,unsigned __int64)+174/r ...
    unsigned int intValue;
    unsigned __int64 int64Value;
    float floatValue;
    char stringValue[64];
};

struct ddlMemberDef_t // sizeof=0x30
{
    const char *name;
    int size;
    int offset;
    int type;
    int externalIndex;
    unsigned int min;
    unsigned int max;
    unsigned int serverDelta;
    unsigned int clientDelta;
    int arraySize;
    int enumIndex;
    int permission;
};

struct ddlStructDef_t // sizeof=0x10
{
    const char *name;
    int size;
    int memberCount;
    ddlMemberDef_t *members;
};

struct ddlDef_t // sizeof=0x1C
{                                       // XREF: XAssetPoolEntry<ddlDef_t>/r
                                        // ddlDefNext/r
    int version;
    int size;
    ddlStructDef_t *structList;
    int structCount;
    ddlEnumDef_t *enumList;
    int enumCount;
    ddlDef_t *next;
};

struct ddlState_t // sizeof=0x10
{                                       // XREF: .data:ddlState_t g_UploadBandwitdh/r
                                        // .data:ddlState_t g_Rank/r ...
    int absoluteOffset;                 // XREF: MatchRecordSpawn(gclient_s *)+B0/r
                                        // MatchRecordSpawn(gclient_s *)+23A/r ...
    int arrayIndex;                     // XREF: MatchRecordSpawn(gclient_s *)+B5/r
                                        // MatchRecordSpawn(gclient_s *)+23F/r ...
    ddlMemberDef_t *member;             // XREF: MatchRecordSpawn(gclient_s *)+BB/r
                                        // MatchRecordSpawn(gclient_s *)+245/r ...
    ddlDef_t *ddl;                      // XREF: MatchRecordSpawn(gclient_s *)+C1/r
                                        // MatchRecordSpawn(gclient_s *)+24B/r ...
};

struct ddlRoot_t // sizeof=0x8
{                                       // XREF: XAssetPoolEntry<ddlRoot_t>/r
    const char *name;
    ddlDef_t *ddlDef;
};

void __cdecl DDL_Init();
int __cdecl DDL_GetTotalBufferBitSize(ddlDef_t *ddl);
void DDL_Printf(const char *fmt, ...);
void DDL_NoCheckPrintf(const char *fmt, ...);
void DDL_PrintWarning(const char *fmt, ...);
void DDL_NoCheckPrintWarning(const char *fmt, ...);
void DDL_PrintError(const char *fmt, ...);
void DDL_NoCheckPrintError(const char *fmt, ...);
int __cdecl DDL_IsStateReadyForIO(const ddlState_t *state);
int __cdecl DDL_AssociateBuffer(char *buffer, int bufferSize, ddlDef_t *ddl);
int __cdecl DDL_FixBufferVersion(
                char *buffer,
                ddlDef_t *ddlTo,
                const char *ddlName,
                char *backupBuffer,
                int bufferSize);
ddlDef_t *__cdecl DDL_LoadAssetWithVersion(const char *fileName, int version);
ddlDef_t *__cdecl DDL_LoadAsset(const char *fileName);
void __cdecl DDL_CopyFromBaseState(ddlState_t *newState, const ddlState_t *baseState);
void __cdecl DDL_Reset(ddlState_t *searchState, ddlDef_t *ddl);
int DDL_MoveTo(const ddlState_t *searchState, ddlState_t *resultState, int argNum, ...);
int __cdecl DDL_MoveToPath(const ddlState_t *searchState, ddlState_t *resultState, int depth, const char **ddlPath);
bool __cdecl DDL_MoveToStart(ddlState_t *searchState, ddlDef_t *ddl);
int __cdecl DDL_MoveToName(const ddlState_t *searchState, ddlState_t *resultState, const char *name);
int __cdecl DDL_MoveToIndex(
                const ddlState_t *searchState,
                ddlState_t *resultState,
                int arrayIndex,
                int checkValidIndex);
int __cdecl DDL_MoveNext(const ddlState_t *searchState, ddlState_t *resultState);
int __cdecl DDL_IterateNext(const ddlState_t *searchState, ddlState_t *resultState);
int __cdecl DDL_IteratePrev(const ddlState_t *searchState, ddlState_t *resultState);
int __cdecl DDL_IterateFirst(const ddlState_t *searchState, ddlState_t *resultState);
int __cdecl DDL_SetRawBytes(char *buffer, int offset, int size, const char *valueToInsert);
void __cdecl DDL_SetValueChanged(char *buffer);
bool __cdecl DDL_WriteIntValueInternal(
                ddlMemberDef_t *member,
                int absoluteOffset,
                unsigned int valueToInsert,
                char *buffer);
int __cdecl DDL_WriteIntValueIfChanged(char *buffer, int offset, int size, unsigned int valueToInsert);
bool __cdecl DDL_WriteInt64ValueInternal(
                ddlMemberDef_t *member,
                int absoluteOffset,
                unsigned __int64 valueToInsert,
                char *buffer);
int __cdecl DDL_WriteStringValueInternal(ddlMemberDef_t *member, int absoluteOffset, const char *value, char *buffer);
int __cdecl DDL_WriteValueInternal(ddlMemberDef_t *member, int absoluteOffset, const char *valueToInsert, char *buffer);
bool __cdecl DDL_SetInt(const ddlState_t *searchState, unsigned int value, char *buffer);
bool __cdecl DDL_SetInt64(const ddlState_t *searchState, unsigned __int64 value, char *buffer);
int __cdecl DDL_SetString(const ddlState_t *searchState, const char *value, char *buffer);
int __cdecl DDL_SetValue(const ddlState_t *searchState, const char *value, char *buffer);
void __cdecl DDL_SetUserFlag(char *buffer, int flagIndex, bool state);
unsigned int __cdecl DDL_ReadIntValueInternal(ddlMemberDef_t *member, int absoluteOffset, char *buffer);
unsigned __int64 __cdecl DDL_ReadInt64ValueInternal(ddlMemberDef_t *member, int absoluteOffset, char *buffer);
char *__cdecl DDL_ReadStringValueInternal(ddlMemberDef_t *member, int absoluteOffset, char *buffer);
int __cdecl DDL_ReadValueInternal(ddlMemberDef_t *member, int absoluteOffset, ddlResult_t *result, char *buffer);
unsigned __int64 __cdecl DDL_GetInt64(const ddlState_t *searchState, char *buffer);
unsigned int __cdecl DDL_GetInt(const ddlState_t *searchState, char *buffer);
char *__cdecl DDL_GetString(const ddlState_t *searchState, char *buffer);
int __cdecl DDL_GetValue(const ddlState_t *searchState, ddlResult_t *result, char *buffer);
int __cdecl DDL_GetBits(char *buffer, int bitOffset, unsigned int bitSize, unsigned int *value);
int __cdecl DDL_GetHeaderVersion(char *buffer);
bool __cdecl DDL_GetUserFlag(char *buffer, int flagIndex);
