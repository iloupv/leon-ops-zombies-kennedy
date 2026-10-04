#include <qcommon/actor_model_state.h>
#include <qcommon/msg.h>
#include <game_mp/g_main_mp.h>
#include <game_mp/g_utils_mp.h>
#include <game/actor.h>
#include <game_mp/actor_mp.h>
#include <clientscript/cscr_stringlist.h>

static SPActorModelState actorModels[MAX_ACTORS_CAP];

void G_SP_ResetActorModels()
{
    memset(actorModels, 0, sizeof(actorModels));
}

// zombies: Actor_UpdateActorInfo model/attachment writer (SP 0x007BADB0).
// SP uses a separate actorState list, not entityState attachment fields:
// +0x0c model, +0x10 models[6], +0x28 tags[6], +0x40 collision mask.
void G_SP_UpdateActorModels(gentity_s *ent)
{
    if (!zombiemode || !zombiemode->current.enabled || !ent->actor)
        return;
    const unsigned int actorNum = ent->actor - level.actors;
    iassert(actorNum < MAX_ACTORS);
    SPActorModelState &state = actorModels[actorNum];
    state.actorNum = actorNum;
    state.entityNum = ent->s.number;
    state.attachIgnoreCollision = ent->attachIgnoreCollision;
    state.modelIndex = ent->model;
    for (int i = 0; i < 6; ++i)
    {
        if (!ent->attachModelNames[i])
        {
            state.attachModelIndex[i] = 0;
            state.attachTagIndex[i] = 0;
            continue;
        }
        state.attachModelIndex[i] = ent->attachModelNames[i];
        state.attachTagIndex[i] = G_TagIndex(SL_ConvertToString(ent->attachTagNames[i], SCRIPTINSTANCE_SERVER));
    }
}

// zombies: SP actor-state snapshot list consumed by CG_SetNextSnap (0x0053D8CB).
// KB transport adapter: full model records appended to zombie snapshots, alongside
// the anim-command extension. No MP netfields/layout changes; each packet stands alone.
void SV_SP_WriteActorModels(client_t *, msg_t *msg)
{
    if (!zombiemode || !zombiemode->current.enabled)
        return;
    int count = 0;
    for (int i = 0; i < MAX_ACTORS; ++i)
        if (level.actors[i].inuse)
            ++count;
    MSG_WriteBits(msg, count, G_ActorCountBits()); // mod: 6 at 32 actors
    for (int i = 0; i < MAX_ACTORS; ++i)
    {
        if (!level.actors[i].inuse)
            continue;
        const SPActorModelState &state = actorModels[i];
        MSG_WriteBits(msg, state.actorNum, G_ActorNumBits()); // mod: 5 at 32 actors
        MSG_WriteBits(msg, state.entityNum, g_entNumBits); // mod (L25): networked entity numbers 1537+
        MSG_WriteBits(msg, state.modelIndex, 9);
        for (int j = 0; j < 6; ++j)
        {
            MSG_WriteBits(msg, state.attachModelIndex[j], 9);
            MSG_WriteBits(msg, state.attachTagIndex[j], 5);
        }
        MSG_WriteBits(msg, state.attachIgnoreCollision, 6);
    }
}
