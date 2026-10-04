#pragma once

struct centity_s;
struct snapshot_s;
struct clientInfo_t;
struct actorInfo_t;
struct MatchState;
struct clientState_s;

void __cdecl CG_ShutdownEntity(int localClientNum, centity_s *cent, bool shutdown_script_for_local_client);
void __cdecl CG_ShutdownEntities(int localClientNum);
void __cdecl CG_SetInitialSnapshot(int localClientNum, snapshot_s *snap);
void __cdecl CG_TransitionSnapshot(int localClientNum);
void __cdecl CG_SetNextSnap(int localClientNum, snapshot_s *snap);
void __cdecl CG_ResetEntity(int localClientNum, centity_s *cent, int newEntity);
void __cdecl CG_CopyCorpseInfo(clientInfo_t *corpseInfo, const clientInfo_t *ci);
void __cdecl CG_CopyActorCorpseInfo(actorInfo_t *corpseInfo, const actorInfo_t *ai);
void __cdecl CG_TransitionKillcam(int localClientNum);
void __cdecl CG_UnpackMatchState(int localClientNum, MatchState *matchState);
void __cdecl CG_CopyScoreFromClientState(int localClientNum, clientInfo_t *ci, clientState_s *cs);
void __cdecl CG_CopyUIVisibilityBitsFomClientState(int localClientNum, clientInfo_t *ci, clientState_s *cs);
void __cdecl CG_UpdateScoreData(int localClientNum);
void __cdecl CG_UpdateClientFlags(int localClientNum, centity_s *cent, int oldEFlags2, bool newEnt);
char __cdecl CG_LocalPlayerRespawned(int localClientNum, snapshot_s *snap);
void __cdecl CG_ProcessSnapshots(int localClientNum);
snapshot_s *__cdecl CG_ReadNextSnapshot(int localClientNum);
