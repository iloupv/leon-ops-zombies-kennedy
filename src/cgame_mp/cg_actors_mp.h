#pragma once

struct centity_s;
struct actorInfo_t;
struct cg_s;
struct cgs_t;

void __cdecl CG_ActorProcessSnapshot(int localClientNum, centity_s *cent);
void __cdecl CG_SP_ActorUseServerAnims(int localClientNum, centity_s *cent);
void __cdecl CG_UpdateActorDObj(int localClientNum, centity_s *cent, actorInfo_t *ai);
cgs_t *__cdecl CG_GetLocalClientStaticGlobals(int localClientNum);
void __cdecl CG_ResetActorEntity(int localClientNum, cg_s *cgameGlob, centity_s *cent);
void __cdecl CG_Actor_PreControllers(int localClientNum, centity_s *cent);
void __cdecl CG_Actor(int localClientNum, centity_s *cent);
void __cdecl CG_SP_PlayAIFootstep(int localClientNum, centity_s *cent, int isLeft, bool isDog);
void __cdecl CG_SP_DoAIFootsteps(int localClientNum, centity_s *cent);
bool __cdecl CG_EntityNeedsScriptThread(int localClientNum, centity_s *cent);
void __cdecl CG_ActorCorpse(int localClientNum, centity_s *cent);
