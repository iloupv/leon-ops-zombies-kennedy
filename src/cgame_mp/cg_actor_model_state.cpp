#include <qcommon/actor_model_state.h>
#include <qcommon/msg.h>
#include <game_sp/actor_sp_ext.h>
#include <game_mp/actor_mp.h>
#include <client_mp/cl_cgame_mp.h>
#include "cg_local_mp.h"
#include <client_mp/client_mp.h>
#include "cg_animtree_mp.h"

namespace
{
    struct ModelSnapshot
    {
        int messageNum;
        int count;
        SPActorModelState actors[MAX_ACTORS_CAP];
    };
    struct ClientModels
    {
        ModelSnapshot parsed[32];
        const snapshot_s *snapshots[2];
        ModelSnapshot models[2];
        SPActorModelInfo actors[MAX_ACTORS_CAP];
        SPActorModelInfo corpses[MAX_ACTOR_CORPSES_SP];
    };
    ClientModels actorModelClients[MAX_LOCAL_CLIENTS];

    SPActorModelInfo *FindInfo(int localClientNum, const actorInfo_t *ai)
    {
        cg_s *cg = CG_GetLocalClientGlobals(localClientNum);
        for (int i = 0; i < MAX_ACTORS; ++i)
            if (BG_SP_GetActorInfo(&cg->bgs, i) == ai)
                return &actorModelClients[localClientNum].actors[i];
        cgs_t *cgs = CG_GetLocalClientStaticGlobals(localClientNum);
        for (int i = 0; i < MAX_ACTOR_CORPSES_SP; ++i)
            if (&cgs->actorCorpseInfo[i] == ai)
                return &actorModelClients[localClientNum].corpses[i];
        iassert(false);
        return nullptr;
    }
}

void CL_SP_ResetActorModels(int localClientNum)
{
    memset(&actorModelClients[localClientNum], 0, sizeof(actorModelClients[localClientNum]));
}

void CL_SP_ParseActorModels(int localClientNum, msg_t *msg, int messageNum)
{
    if (!zombiemode || !zombiemode->current.enabled)
        return;
    ModelSnapshot &parsed = actorModelClients[localClientNum].parsed[messageNum & 31];
    parsed.messageNum = messageNum;
    parsed.count = MSG_ReadBits(msg, G_ActorCountBits()); // mod: 6 at 32 actors
    if (parsed.count < 0 || parsed.count > MAX_ACTORS)
    {
        msg->overflowed = 1;
        return;
    }
    unsigned int seen[MAX_ACTORS_CAP / 32] = {}; // mod: was one 32-bit mask
    for (int i = 0; i < parsed.count; ++i)
    {
        SPActorModelState &state = parsed.actors[i];
        state.actorNum = MSG_ReadBits(msg, G_ActorNumBits()); // mod: 5 at 32 actors
        if (state.actorNum < 0 || state.actorNum >= MAX_ACTORS || (seen[state.actorNum >> 5] & (1u << (state.actorNum & 31))))
        {
            msg->overflowed = 1;
            return;
        }
        seen[state.actorNum >> 5] |= 1u << (state.actorNum & 31);
        state.entityNum = MSG_ReadBits(msg, g_entNumBits); // mod (L25): networked entity numbers 1537+
        state.modelIndex = MSG_ReadBits(msg, 9);
        for (int j = 0; j < 6; ++j)
        {
            state.attachModelIndex[j] = MSG_ReadBits(msg, 9);
            state.attachTagIndex[j] = MSG_ReadBits(msg, 5);
        }
        state.attachIgnoreCollision = MSG_ReadBits(msg, 6);
    }
}

// zombies: SP CL_GetSnapshot copies actorState records with the snapshot (0x005D7A30).
bool CL_SP_GetActorModels(int localClientNum, int messageNum, snapshot_s *snapshot)
{
    if (!zombiemode || !zombiemode->current.enabled)
        return true;
    ClientModels &client = actorModelClients[localClientNum];
    const ModelSnapshot &parsed = client.parsed[messageNum & 31];
    if (parsed.messageNum != messageNum)
        return false;
    for (int i = 0; i < 2; ++i)
    {
        if (!client.snapshots[i] || client.snapshots[i] == snapshot)
        {
            client.snapshots[i] = snapshot;
            client.models[i] = parsed;
            return true;
        }
    }
    iassert(false);
    return false;
}

// zombies: CG_SetNextSnap actor-info reader (SP 0x0053D8CB..0x0053DAE6).
// Resolve configstrings after server commands and before any actor DObj update.
void CG_SP_SetActorModels(int localClientNum, const snapshot_s *snapshot)
{
    if (!zombiemode || !zombiemode->current.enabled)
        return;
    ClientModels &client = actorModelClients[localClientNum];
    for (int s = 0; s < 2; ++s)
    {
        if (client.snapshots[s] != snapshot)
            continue;
        const ModelSnapshot &models = client.models[s];
        for (int i = 0; i < models.count; ++i)
        {
            const SPActorModelState &state = models.actors[i];
            actorInfo_t *ai = BG_SP_GetActorInfo(&CG_GetLocalClientGlobals(localClientNum)->bgs, state.actorNum);
            SPActorModelInfo &info = client.actors[state.actorNum];
            ai->infoValid = 1;
            ai->nextValid = 1;
            ai->actorNum = state.actorNum;
            ai->entityNum = state.entityNum;
            info.attachIgnoreCollision = state.attachIgnoreCollision;
            const char *modelName = CL_GetConfigString(state.modelIndex + 1568);
            if (strcmp(info.model, modelName))
            {
                I_strncpyz(info.model, modelName, 64);
                ai->dobjDirty = 1;
            }
            for (int j = 0; j < 6; ++j)
            {
                modelName = CL_GetConfigString(state.attachModelIndex[j] + 1568);
                if (strcmp(info.attachModelNames[j], modelName))
                {
                    I_strncpyz(info.attachModelNames[j], modelName, 64);
                    ai->dobjDirty = 1;
                }
                const char *tagName = CL_GetConfigString(state.attachTagIndex[j] + 3115);
                if (strcmp(info.attachTagNames[j], tagName))
                {
                    I_strncpyz(info.attachTagNames[j], tagName, 64);
                    ai->dobjDirty = 1;
                }
            }
        }
        return;
    }
}

// zombies: SP CG_TransitionSnapshot's actor loop (SP 0x00639E20, after its client loop), over the actors of the
// snapshot being left. An actor the next snapshot does not list (nextValid still 0) loses its client info, all but the
// anim tree the slot owns. If its entity became the actor's corpse (G_CorpseFromActor 0x00642B80), the actor's client
// tree - the one the entity's DObj and ragdoll animate - moves to the corpse's client slot, swapped with that slot's
// tree (0x0042B190); otherwise the entity's DObj is freed. A listed actor only has nextValid cleared (and the same
// swap if its old entity is a corpse now).
void CG_SP_TransitionActors(int localClientNum, const snapshot_s *snapshot)
{
    if (!zombiemode || !zombiemode->current.enabled)
        return;
    ClientModels &client = actorModelClients[localClientNum];
    for (int s = 0; s < 2; ++s)
    {
        if (client.snapshots[s] != snapshot)
            continue;
        const ModelSnapshot &models = client.models[s];
        for (int i = 0; i < models.count; ++i)
        {
            const SPActorModelState &state = models.actors[i];
            actorInfo_t *ai = BG_SP_GetActorInfo(&CG_GetLocalClientGlobals(localClientNum)->bgs, state.actorNum);
            centity_s *cent = CG_GetEntity(localClientNum, state.entityNum);
            if (ai->nextValid)
            {
                ai->nextValid = 0;
                if (cent->nextState.eType == ET_ACTOR_CORPSE)
                    CG_SP_SwapCorpseTree(localClientNum, cent, state.actorNum);
                continue;
            }
            XAnimTree_s *tree = ai->pXAnimTree;
            memset(ai, 0, sizeof(*ai));
            ai->pXAnimTree = tree;
            memset(&client.actors[state.actorNum], 0, sizeof(client.actors[state.actorNum])); // SP's info holds the models
            if (cent->nextState.eType == ET_ACTOR_CORPSE)
                CG_SP_SwapCorpseTree(localClientNum, cent, state.actorNum);
            else
                CG_SafeDObjFree(localClientNum, state.entityNum);
        }
        return;
    }
}

// zombies: SP 0x0042B190 - the actor's client tree and the client tree of corpse slot lerp.u.actor.corpseNum change
// places.
void CG_SP_SwapCorpseTree(int localClientNum, const centity_s *cent, int actorNum)
{
    actorInfo_t *ai = BG_SP_GetActorInfo(&CG_GetLocalClientGlobals(localClientNum)->bgs, actorNum);
    actorInfo_t *corpseInfo = &CG_GetLocalClientStaticGlobals(localClientNum)->actorCorpseInfo[cent->nextState.lerp.u.actor.corpseNum];
    XAnimTree_s *tree = ai->pXAnimTree;
    ai->pXAnimTree = corpseInfo->pXAnimTree;
    corpseInfo->pXAnimTree = tree;
}

const SPActorModelInfo *CG_SP_GetActorModels(int localClientNum, const actorInfo_t *ai)
{
    return FindInfo(localClientNum, ai);
}

void CG_SP_CopyActorModels(int localClientNum, actorInfo_t *corpseInfo, const actorInfo_t *ai)
{
    if (zombiemode && zombiemode->current.enabled)
        *FindInfo(localClientNum, corpseInfo) = *FindInfo(localClientNum, ai);
}
