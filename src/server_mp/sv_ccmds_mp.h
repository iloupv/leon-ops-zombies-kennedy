#pragma once

#include <server/server.h>

char *__cdecl SV_GetMapBaseName(char *mapname);
void __cdecl SV_ReconnectClients(int savepersist);
void __cdecl SV_MapRestart(int fast_restart);
void __cdecl SV_RankUpClient(client_t *client);
void __cdecl SV_Heartbeat_f();
void __cdecl SV_AddOperatorCommands();
void __cdecl SV_Map_f();
char __cdecl SV_CheckMapExists(const char *map);
void __cdecl ShowLoadErrorsSummary(const char *mapName, unsigned int count);
void __cdecl SV_MapRestart_f();
void __cdecl SV_FastRestart_f();
void __cdecl SV_MapRotate_f();
void __cdecl SV_TempBan_f();
int __cdecl SV_KickUser_f(char *playerName, int maxPlayerNameLen);
client_t *__cdecl SV_GetPlayerByName();
int __cdecl SV_KickClient(client_t *cl, char *playerName, int maxPlayerNameLen, const char *reason);
void __cdecl SV_Ban_f();
void __cdecl SV_RankUp_f();
void __cdecl SV_BanNum_f();
client_t *__cdecl SV_GetPlayerByNum();
void __cdecl SV_Unban_f();
void __cdecl SV_Drop_f();
void __cdecl SV_DropNum_f();
int __cdecl SV_KickClient_f(char *playerName, int maxPlayerNameLen);
void __cdecl SV_TempBanNum_f();
void __cdecl SV_Status_f();
void __cdecl SV_TeamStatus_f();
void __cdecl SV_Serverinfo_f();
void __cdecl SV_Systeminfo_f();
void __cdecl SV_DumpUser_f();
void __cdecl SV_KillServer_f();
void __cdecl SV_ScriptUsage_f();
void __cdecl SV_StringUsage_f();
void __cdecl SV_SetPerk_f();
void __cdecl SV_AddDedicatedCommands();
void __cdecl SV_ConSay_f();
void __cdecl SV_AssembleConSayMessage(int firstArg, char *text, int sizeofText);
void __cdecl SV_ConTell_f();
void __cdecl SV_RemoveDedicatedCommands();
