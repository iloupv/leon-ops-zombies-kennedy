#pragma once
#include "common.h"

/* 596 */
enum RadiantCommandType : __int32
{
    RADIANT_COMMAND_SELECT = 0x0,
    RADIANT_COMMAND_DESELECT = 0x1,
    RADIANT_COMMAND_UPDATE_SELECTED = 0x2,
    RADIANT_COMMAND_UPDATE = 0x3,
    RADIANT_COMMAND_CREATE = 0x4,
    RADIANT_COMMAND_DELETE = 0x5,
    RADIANT_COMMAND_CAMERA = 0x6,
};

enum CommandProcessorType : __int32
{                                       // XREF: ?G_ProcessRadiantCmd@@YAXABURadiantCommand@@@Z/r
    COMMAND_SERVER = 0x0,
    COMMAND_CLIENT = 0x1,
    COMMAND_BOTH   = 0x2,
};

struct RadiantCommand // sizeof=0x208
{                                       // XREF: .data:cgCommands/r
    RadiantCommandType type;            // XREF: AddSavedCommand(RadiantCommand const &)+8D/r
    int liveUpdateId;                   // XREF: AddSavedCommand(RadiantCommand const &)+56/r
    char strCommand[512];               // XREF: AddSavedCommand(RadiantCommand const &)+76/o
};

struct RadaintToGameMapping // sizeof=0x10
{                                       // XREF: .data:RadaintToGameMapping * gObjectMapping/r
    int fromRadiantId;
    int liveUpdateId;
    int gameId;                         // XREF: G_AssignGameIdMapping(int,int)+15/w
    int cg_gameId;                      // XREF: CG_AssignGameIdMapping(int,int)+15/w
};

struct RadiantCommandProcessor // sizeof=0x10
{                                       // XREF: .rdata:gCommandProcessors/r
                                        // .rdata:cgCommandProcessors/r
    void (__cdecl *processorFunc)(const RadiantCommand *, SpawnVar *);
    void (__cdecl *clearSelectionFunc)();
    bool (__cdecl *conditionFunc)(const SpawnVar *);
    const char *classname;
};

char *__cdecl GetPairValue(const SpawnVar *spawnVar, const char *key);
void __cdecl AddSavedCommand(const RadiantCommand *command);
void __cdecl RunSavedRadiantCmds();
void __cdecl G_AssignGameIdMapping(int liveUpdateId, int gameId);
int __cdecl G_GetGameIdMapping(int liveUpdateId);
void __cdecl CG_AssignGameIdMapping(int liveUpdateId, int gameId);
int __cdecl CG_GetGameIdMapping(int liveUpdateId);
void __cdecl RadiantRemoteInit();
struct gentity_s *__cdecl FindEntity(const SpawnVar *spawnVar, int gameId);
bool __cdecl IsEntityType(const SpawnVar *spawnVar);
void __cdecl G_ClearSelectedEntity();
void __cdecl G_ProcessEntityCommand(const RadiantCommand *command, SpawnVar *spawnVar);
bool __cdecl G_IsSpawnPoint(const char *classname);
void __cdecl G_ProcessCameraCommand(SpawnVar *spawnVar);
unsigned int __cdecl G_FindMiscModel(const float *origin, int gameId);
void __cdecl G_ProcessMiscModelCommand(const RadiantCommand *command, SpawnVar *spawnVar);
void __cdecl G_ClearSelectedMiscModel();
unsigned int __cdecl G_FindCorona(const float *origin, int gameId);
void __cdecl G_ProcessCoronaCommand(const RadiantCommand *command, SpawnVar *spawnVar);
void __cdecl G_ClearSelectedCorona();
void __cdecl G_NotifyScriptsOfSelectedScriptStruct(int structId);
unsigned int __cdecl G_FindStruct(SpawnVar *spawnVar, int gameId);
void __cdecl G_ProcessScriptStructCommand(const RadiantCommand *command, SpawnVar *spawnVar);
void __cdecl G_ClearSelectedScriptStruct();
void __cdecl CG_NotifyScriptsOfSelectedScriptStruct(int structId);
unsigned int __cdecl CG_FindStruct(SpawnVar *spawnVar, int gameId);
void __cdecl CG_ProcessScriptStructCommand(const RadiantCommand *command, SpawnVar *spawnVar);
void __cdecl CG_ClearSelectedScriptStruct();
void __cdecl G_RadiantDebugDraw();
void __cdecl G_ClearSelection(void (__cdecl *ignoreFunc)());
void __cdecl CG_ClearSelection(void (__cdecl *ignoreFunc)());
void __cdecl CG_ProcessRadiantCmd(const RadiantCommand *command);
char __cdecl CG_ProcessRadiantCmds();
CommandProcessorType __cdecl GetCommandProcessorType(const SpawnVar *spawnVar);
void __cdecl G_ProcessRadiantCmd(const RadiantCommand *command);
char __cdecl G_ProcessRadiantCmds();


extern gentity_s *g_radiant_selected_ent;
