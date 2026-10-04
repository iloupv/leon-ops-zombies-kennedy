#pragma once

#include <bgame/bg_local.h>

struct client_t;
struct msg_t;
struct SnapshotInfo_s;
struct entityState_s;
struct clientState_s;
struct cachedSnapshot_t;

struct snapshotEntityNumbers_t // sizeof=0x1004
{                                       // XREF: ?SV_BuildClientSnapshot@@YAXPAUclient_t@@@Z/r
    int numSnapshotEntities;            // XREF: SV_BuildClientSnapshot(client_t *):loc_71BF18/w
    int snapshotEntities[MAX_CL_ENTNUM]; // mod (L25): networked entity numbers 1537+ (retail 1024)         // XREF: SV_BuildClientSnapshot(client_t *)+324/r
};

struct clientSnapshot_t // sizeof=0x26CC
{                                       // XREF: client_t/r
    playerState_s ps;
    int num_entities;
    int num_clients;
    int matchState;
    int first_entity;
    int first_client;
    int messageSent;
    int messageAcked;
    int messageSize;
    int serverTime;
    int physicsTime;
};

void __cdecl SV_WriteSnapshotToClient(client_t *client, msg_t *msg);
void __cdecl SV_EmitPacketEntities(
                SnapshotInfo_s *snapInfo,
                int from_num_entities,
                int from_first_entity,
                int to_num_entities,
                int to_first_entity,
                msg_t *msg);
void __cdecl SV_EmitPacketEntities_GetOldEnt(
                int oldindex,
                int from_num_entities,
                int from_first_entity,
                const entityState_s **oldent,
                int *oldnum);
void __cdecl SV_EmitPacketEntities_DebugStart(int *quickBits, msg_t *msg);
void SV_EmitPacketEntities_Debug(SnapshotInfo_s *snapInfo, int *quickBits, msg_t *msg, const char *str, ...);
void __cdecl SV_EmitPacketClients(
                SnapshotInfo_s *snapInfo,
                int from_num_clients,
                int from_first_client,
                int to_num_clients,
                int to_first_client,
                msg_t *msg);
void __cdecl SV_EmitPacketClients_GetOldClient(
                int oldindex,
                int from_num_clients,
                int from_first_client,
                const clientState_s **oldclient,
                int *oldnum);
void __cdecl SV_UpdateServerCommandsToClient(client_t *client, msg_t *msg);
void __cdecl SV_UpdateServerCommandsToClient_PreventOverflow(client_t *client, msg_t *msg, int iMsgSize);
char __cdecl SV_GetClientPositionsAtTime(int gametime, float (*pos)[3], float (*angles)[3], bool *success);
cachedSnapshot_t *__cdecl SV_GetCachedSnapshot(int *pArchiveTime);
cachedSnapshot_t *__cdecl SV_GetCachedSnapshotInternal(int archivedFrame, int depth, bool expectedToSucceed);
bool __cdecl SV_FrameIsStillInArchivedSnapshotBuffer(int frameStart);
void __cdecl SV_GetClientPositionsFromCachedSnap(
                const cachedSnapshot_t *cachedFrame,
                float (*pos)[3],
                float (*angles)[3],
                bool *success);
int __cdecl SV_GetEarliestArchivedClientInfoTime();
bool __cdecl SV_HasCachedSnapshotInternal(int archivedFrame, int callDepth);
int __cdecl SV_GetArchivedClientInfo(
                unsigned int clientNum,
                int *pArchiveTime,
                playerState_s *ps,
                clientState_s *cs,
                float *origin,
                int *health,
                int *otherFlags);
int __cdecl SV_GetCurrentClientInfo(unsigned int clientNum, playerState_s *ps, clientState_s *cs);
void __cdecl SV_DecToHex(int decValue, char *hexValue, int hexLength);
int __cdecl SV_AppendToBuffer(char *msgBuffer, int *msgBufferIndex, char *data, int dataSize);
int __cdecl SV_AddModifiedStatsWithinOffset(
                unsigned int clientNum,
                int startOffset,
                int *endOffset,
                char *msgBuff,
                int *msgBuffIndex);
void __cdecl SV_AddModifiedStats(unsigned int clientNum);
void __cdecl SV_BuildClientSnapshot(client_t *client);
void __cdecl SV_AddEntitiesVisibleFromPoint(float *org, int clientNum, snapshotEntityNumbers_t *eNums);
void __cdecl SV_AddArchivedEntToSnapshot(int e, snapshotEntityNumbers_t *eNums);
void __cdecl SV_AddCachedEntitiesVisibleFromPoint(
                int from_num_entities,
                int from_first_entity,
                float *org,
                int clientNum,
                snapshotEntityNumbers_t *eNums,
                playerState_s *ps);
void __cdecl SV_SendMessageToClient(msg_t *msg, client_t *client, bool reliable);
int __cdecl SV_WindowedRateMsec(client_t *client);
void __cdecl SV_BeginClientSnapshot(client_t *client, msg_t *msg);
void __cdecl SV_EndClientSnapshot(client_t *client, msg_t *msg);
void __cdecl SV_PrintServerCommandsForClient(client_t *client);
void __cdecl SV_SetServerStaticHeader();
void __cdecl SV_GetServerStaticHeader();
void __cdecl SV_SendClientMessages();
