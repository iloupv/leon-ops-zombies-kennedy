#pragma once

struct gentity_s;
struct client_t;
struct msg_t;
struct snapshot_s;
struct actorInfo_t;

// zombies: model fields of SP actorState_s (0x007BADB0), kept outside MP layouts.
struct SPActorModelState
{
    int actorNum;
    int entityNum;
    int modelIndex;
    int attachModelIndex[6];
    int attachTagIndex[6];
    unsigned int attachIgnoreCollision;
};

struct SPActorModelInfo
{
    char model[64];
    char attachModelNames[6][64];
    char attachTagNames[6][64];
    unsigned int attachIgnoreCollision;
};

void G_SP_ResetActorModels();
void G_SP_UpdateActorModels(gentity_s *ent);
void SV_SP_WriteActorModels(client_t *client, msg_t *msg);
void CL_SP_ResetActorModels(int localClientNum);
void CL_SP_ParseActorModels(int localClientNum, msg_t *msg, int messageNum);
bool CL_SP_GetActorModels(int localClientNum, int messageNum, snapshot_s *snapshot);
void CG_SP_SetActorModels(int localClientNum, const snapshot_s *snapshot);
const SPActorModelInfo *CG_SP_GetActorModels(int localClientNum, const actorInfo_t *ai);
void CG_SP_CopyActorModels(int localClientNum, actorInfo_t *corpseInfo, const actorInfo_t *ai);
void CG_SP_TransitionActors(int localClientNum, const snapshot_s *snapshot);
void CG_SP_SwapCorpseTree(int localClientNum, const struct centity_s *cent, int actorNum);
