#pragma once
#include <qcommon/msg_mp.h>

struct clSnapshot_t;
struct clientActive_t;
struct clientState_s;

void __cdecl SHOWNET(msg_t *msg, char *s);
void __cdecl CL_SavePredictedOriginForServerTime(
                clientActive_t *cl,
                int serverTime,
                float *predictedOrigin,
                float *predictedVelocity,
                float *viewangles,
                int bobCycle,
                int movementDir);
char __cdecl CL_GetPredictedOriginForServerTime(
                clientActive_t *cl,
                int serverTime,
                float *predictedOrigin,
                float *predictedVelocity,
                float *viewangles,
                int *bobCycle,
                int *movementDir);
void __cdecl CL_DeltaClient(
                clientActive_t *cl,
                msg_t *msg,
                int time,
                clSnapshot_t *frame,
                unsigned int newnum,
                clientState_s *old,
                int unchanged);
void __cdecl CL_ProcessMapCenterFromMatchState(int localClientNum, clSnapshot_t *clSnap);
void __cdecl CL_UpdateMapCenterFromMatchState(float *newcenter);
void __cdecl CL_ServerIdChanged(int localClientNum);
void __cdecl CL_SystemInfoChanged(int localClientNum);
void __cdecl CL_ParseMapCenter();
void __cdecl CL_ParseGamestate(int localClientNum, msg_t *msg);
void __cdecl CL_ParseWWWDownload(int localClientNum, msg_t *msg);
void __cdecl CL_ParseServerMessage(int localClientNum, msg_t *msg);
void __cdecl CL_ParseSnapshot(int localClientNum, msg_t *msg);
int __cdecl CL_ParsePacketEntities(
                clientActive_t *cl,
                msg_t *msg,
                int time,
                clSnapshot_t *oldframe,
                clSnapshot_t *newframe);
void __cdecl CL_DeltaEntity(
                clientActive_t *cl,
                msg_t *msg,
                int time,
                clSnapshot_t *frame,
                unsigned int newnum,
                entityState_s *old);
void __cdecl CL_CopyOldEntity(clientActive_t *cl, clSnapshot_t *frame, entityState_s *old);
void __cdecl CL_ParsePacketClients(
                clientActive_t *cl,
                msg_t *msg,
                int time,
                clSnapshot_t *oldframe,
                clSnapshot_t *newframe);
void __cdecl CL_ParseDownload(int localClientNum, msg_t *msg);
void __cdecl CL_ParseCommandString(int localClientNum, msg_t *msg);

extern int cl_connectedToPureServer;