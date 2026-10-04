#include "cg_anim_commands.h"
#include "cg_local_mp.h"
#include "cg_ents_mp.h"
#include <qcommon/msg_anim_commands.h>
#include <qcommon/msg.h>
#include <qcommon/dobj_management.h>
#include <xanim/xanim.h>
#include <xanim/xanim_clientnotify.h>
#include <ragdoll/ragdoll.h>
#include <algorithm>
#include <game_sp/g_sp_measure.h>
#include <game_mp/g_main_mp.h>
#include <game/actor_scripted.h>
#include <game_sp/actor_sp_ext.h>
#include <universal/com_math.h>

namespace
{
    // Keep MP structure layouts intact. These are the SP additions to clSnapshot_t,
    // clientActive_t and snapshot_s (SP 0x008851F0, 0x005D7A30), indexed alongside KB's snapshots.
    struct ParsedCommands
    {
        int messageNum;
        unsigned int first;
        int count;
    };
    struct SnapshotCommands
    {
        const snapshot_s *snapshot;
        int count;
        AnimCommand commands[1024];
        bool done[1024];
    };
    struct ClientCommands
    {
        unsigned int next;
        AnimCommand ring[4096];
        ParsedCommands parsed[32];
        SnapshotCommands snapshots[2];
        int lastTime;
        int readCount;
        int replayCount;
        int lastLogTime;
    };
    ClientCommands clients[MAX_LOCAL_CLIENTS];
    const dvar_s *cg_defensive_anim_delay;

    SnapshotCommands *FindCommands(int localClientNum, const snapshot_s *snapshot)
    {
        if (!snapshot)
            return nullptr;
        for (SnapshotCommands &commands : clients[localClientNum].snapshots)
            if (commands.snapshot == snapshot)
                return &commands;
        return nullptr;
    }

    // zombies: advance a newly commanded anim (SP 0x00584000). Only a node at time zero
    // is advanced. The exe clamps at .999 even for looping animations and delivers notetracks.
    void CG_AdvanceAnimCommand(DObj *obj, unsigned int anim, int elapsed)
    {
        XAnimTree_s *tree = DObjGetTree(obj);
        if (!tree->children)
            return;
        unsigned int index = XAnimGetInfoIndex(tree, anim);
        if (!index)
            return;
        XAnimInfo *info = GetAnimInfo(index);
        if (info->state.currentAnimTime != 0.0f)
            return;
        float time = (float)(elapsed * 0.001f * XAnimGetAverageRateFrequency(tree, index) * info->state.rate);
        time = (std::max)(0.0f, (std::min)(time, 0.999f));
        info->state.currentAnimTime = time;
        if (time > 0.0f && info->state.goalWeight != 0.0f)
            XAnimProcessClientNotify(info, elapsed * 0.001f);
        info->state.oldTime = time;
    }

    // zombies: command dispatch (SP 0x00525AB0). Types 7/8 have wire encodings but
    // this exe's dispatch handles ONLY 1..6; keep that distinction.
    void CG_ApplyAnimCommand(int localClientNum, DObj *obj, XAnimTree_s *tree, const AnimCommand &command)
    {
        if (ai_useServerAnims->current.enabled || ai_useServerAnimsEntity->current.integer == command.entnum)
            return;
        cg_s *cg = CG_GetLocalClientGlobals(localClientNum);
        int elapsed = (std::max)(0, cg->time - command.startTime);
        centity_s *cent = CG_GetEntity(localClientNum, command.entnum);
        // KB stores a RagdollBody pointer in the handle; SP 0x004A98D0 resolves an index.
        if (cent->pose.isRagdoll && cent->pose.ragdollHandle > 0
            && ((RagdollBody *)cent->pose.ragdollHandle)->state >= BS_RUNNING)
            return;
        XAnimClientNotifyList notifies;
        DObjSetClientNotifies(&notifies);
        unsigned int notifyType = command.weight > 0.001f ? 2 : 0;
        int restart = (command.flags >> 1) & 1;
        // zombies: jump table SP 0x00525E20. Type 1: flag 1 -> XAnimClearTree 0x0041F500 (the node and its
        // subtree, KB XAnimClearTreeGoalWeights), else 0x0050B0C0 (the node only, KB XAnimClearGoalWeight).
        // Type 2 (entry 0x00525BB4) -> 0x00665890, the root's children (KB XAnimClearTreeGoalWeightsStrict),
        // as the server's animscripted clear (actor_scripted.cpp). KB had types 1-else and 2 swapped.
        switch (command.type)
        {
        case 1:
            if (command.flags & 1)
                XAnimClearTreeGoalWeights(tree, command.anim, command.time, -1);
            else
                XAnimClearGoalWeight(tree, command.anim, command.time, (unsigned short)-1);
            break;
        case 2:
            XAnimClearTreeGoalWeightsStrict(tree, command.anim, command.time, -1);
            break;
        case 3:
            if (command.flags & 1)
                XAnimSetCompleteGoalWeight(obj, command.anim, command.weight, command.time, command.rate, 0, notifyType, restart, -1);
            else
                XAnimSetGoalWeight(obj, command.anim, command.weight, command.time, command.rate, 0, notifyType, restart, -1);
            CG_AdvanceAnimCommand(obj, command.anim, elapsed);
            break;
        case 4:
            if (command.flags & 1)
                XAnimSetCompleteGoalWeightKnob(obj, command.anim, command.weight, command.time, command.rate, 0, notifyType, restart, -1);
            else
                XAnimSetGoalWeightKnob(obj, command.anim, command.weight, command.time, command.rate, 0, notifyType, restart, -1);
            CG_AdvanceAnimCommand(obj, command.anim, elapsed);
            break;
        case 5:
            if (command.flags & 1)
                XAnimSetCompleteGoalWeightKnobAll(obj, command.anim, command.root, command.weight, command.time, command.rate, 0, notifyType, restart, -1);
            else
                XAnimSetGoalWeightKnobAll(obj, command.anim, command.root, command.weight, command.time, command.rate, 0, notifyType, restart, -1);
            CG_AdvanceAnimCommand(obj, command.anim, elapsed);
            break;
        case 6:
            XAnimSetTime(tree, command.anim, command.time, (unsigned short)-1);
            break;
        }
        // SP suppresses these during client save restoration (cg+0x18, set at
        // 0x0054C5B4). KB's live MP snapshot path has no SP client-save restoration.
        CG_ProcessFakeEntClientNoteTracks(localClientNum, command.entnum);
        DObjClearClientNotifies();
        ++clients[localClientNum].replayCount;
    }

    // zombies: ordered replay (SP 0x00897660); entity reset replay is SP 0x00648830.
    int Replay(int localClientNum, const snapshot_s *snapshot, int entnum)
    {
        SnapshotCommands *batch = FindCommands(localClientNum, snapshot);
        if (!batch)
            return 0;
        ClientCommands &client = clients[localClientNum];
        cg_s *cg = CG_GetLocalClientGlobals(localClientNum);
        int applied = 0;
        for (int i = 0; i < batch->count; ++i)
        {
            const AnimCommand &command = batch->commands[i];
            if (entnum >= 0 ? command.entnum != entnum : batch->done[i])
                continue;
            if (cg_defensive_anim_delay->current.enabled && command.startTime > cg->time)
                continue;
            if (entnum < 0 && command.startTime < client.lastTime)
            {
                batch->done[i] = true;
                continue;
            }
            DObj *obj = Com_GetClientDObj(command.entnum, localClientNum);
            if (!obj)
                continue;
            XAnimTree_s *tree = DObjGetTree(obj);
            if (!tree || command.anim >= tree->anims->size)
                continue;
            CG_ApplyAnimCommand(localClientNum, obj, tree, command);
            batch->done[i] = true;
            if (entnum < 0)
                client.lastTime = command.startTime;
            ++applied;
        }
        return applied;
    }
}

void CL_SP_ResetAnimCommands(int localClientNum)
{
    memset(&clients[localClientNum], 0, sizeof(clients[localClientNum]));
    // zombies: SP 0x004A4E5D, default true, flags 0.
    cg_defensive_anim_delay = _Dvar_RegisterBool("cg_defensive_anim_delay", true, 0, "");
    // L15 (TEST): client-side scripted anim trace, CG_SP_MeasureAnimScripted.
    _Dvar_RegisterBool("bo1_measure_animscripted_cl", false, 0, "L15 TEST: per-frame client scripted anim weights and placement");
}

// zombies: read snapshot command list into the 4096-command parse ring (SP 0x008851F0).
void CL_SP_ParseAnimCommands(int localClientNum, msg_t *msg, int messageNum, int serverTime)
{
    if (!zombiemode || !zombiemode->current.enabled)
        return;
    ClientCommands &client = clients[localClientNum];
    ParsedCommands &parsed = client.parsed[messageNum & 31];
    parsed = {messageNum, client.next, 0};
    AnimCommandMsgState state;
    AnimCommand command;
    while (MSG_ReadAnimCommand(msg, serverTime, &state, &command))
    {
        if (parsed.count == 1024)
        {
            msg->overflowed = 1;
            return;
        }
        client.ring[client.next++ & 4095] = command;
        ++parsed.count;
        ++client.readCount;
    }
}

// zombies: copy the parsed commands and clear applied flags (SP 0x005D7A30).
bool CL_SP_GetAnimCommands(int localClientNum, int messageNum, snapshot_s *snapshot)
{
    if (!zombiemode || !zombiemode->current.enabled)
        return true;
    ClientCommands &client = clients[localClientNum];
    const ParsedCommands &parsed = client.parsed[messageNum & 31];
    if (parsed.messageNum != messageNum || client.next - parsed.first > 4096)
        return false;
    SnapshotCommands *batch = FindCommands(localClientNum, snapshot);
    if (!batch)
        for (SnapshotCommands &slot : client.snapshots)
            if (!slot.snapshot)
            {
                batch = &slot;
                break;
            }
    iassert(batch);
    batch->snapshot = snapshot;
    batch->count = parsed.count;
    for (int i = 0; i < parsed.count; ++i)
    {
        batch->commands[i] = client.ring[(parsed.first + i) & 4095];
        batch->done[i] = false;
    }
    return true;
}

// zombies: replay current then next snapshot; advance watermark only after each list
// (SP 0x004DEB90). This prevents equal-time commands being replayed in later snapshots.
void CG_SP_ReplayAnimCommands(int localClientNum)
{
    if (!zombiemode || !zombiemode->current.enabled)
        return;
    cg_s *cg = CG_GetLocalClientGlobals(localClientNum);
    ClientCommands &client = clients[localClientNum];
    if (Replay(localClientNum, cg->snap, -1))
        ++client.lastTime;
    if (Replay(localClientNum, cg->nextSnap, -1))
        ++client.lastTime;
    // L12 (TEST, bo1_measure_animents): the client DObj of every snapshot entity whose server twin has an anim
    // tree (listen server only) and is not a player / actor: does the client animate it, and which anims.
    static int s_nextAnimEntLog;
    if ( G_SP_MeasureAnimEntsOn() && cg->nextSnap && (cg->time >= s_nextAnimEntLog || cg->time < s_nextAnimEntLog - 1000) )
    {
        s_nextAnimEntLog = cg->time + 250;
        for (int num = 0; num < cg->nextSnap->numEntities; ++num)
        {
            const entityState_s *es = &cg->nextSnap->entities[num];
            if (es->eType == ET_PLAYER || es->eType == ET_ACTOR || es->number >= MAX_GENTITIES
                || !g_entities[es->number].r.inuse || !g_entities[es->number].pAnimTree)
                continue;
            DObj *obj = Com_GetClientDObj(es->number, localClientNum);
            const XAnimTree_s *tree = obj ? DObjGetTree(obj) : nullptr;
            char anims[512] = "-";
            if (tree)
                G_SP_FormatWeightedAnims(tree, anims, sizeof(anims));
            centity_s *cent = CG_GetEntity(localClientNum, es->number);
            Com_Printf(14, "bo1_animent_cl: time %d ent %d eType %d dobj %d tree %d centtree %d origin %.0f %.0f %.0f anims %s\n",
                cg->time, es->number, (int)es->eType, obj ? 1 : 0, tree ? 1 : 0, cent->tree ? 1 : 0,
                cent->pose.origin[0], cent->pose.origin[1], cent->pose.origin[2], anims);
        }
    }
    const dvar_s *measure = Dvar_FindVar("bo1_measure");
    if (measure && measure->current.enabled && cg->time - client.lastLogTime >= 10000)
    {
        client.lastLogTime = cg->time;
        Com_Printf(14, "bo1_anim_snapshot: time=%d read=%d replay=%d watermark=%d ai_useServerAnims=%d entity=%d\n",
            cg->time, client.readCount, client.replayCount, client.lastTime,
            ai_useServerAnims->current.enabled, ai_useServerAnimsEntity->current.integer);
    }
}

namespace
{
    // L15 (TEST): the client leaves with an effective weight >= 0.05, "name:weight,...".
    void MeasureEffectiveLeaves(const XAnimTree_s *tree, char *out, int size)
    {
        int used = 0;
        out[0] = 0;
        const XAnim_s *anims = XAnimGetAnims(tree);
        const unsigned int count = XAnimGetAnimTreeSize(anims);
        for (unsigned int anim = 1; anim < count && used < size - 1; ++anim)
        {
            if (XAnimGetNumChildren(anims, anim) || !XAnimGetInfoIndex(tree, anim))
                continue;
            const float weight = G_SP_MeasureEffectiveWeight(tree, anim);
            if (weight < 0.05f)
                continue;
            const int n = _snprintf_s(out + used, size - used, _TRUNCATE, "%s%s:%.2f", used ? "," : "",
                XAnimGetAnimName(anims, anim), weight);
            if (n < 0)
                break;
            used += n;
        }
        if (!used)
            strcpy_s(out, size, "-");
    }
}

// L15 (TEST, bo1_measure_animscripted_cl; listen server): every client frame, for each entity the server thread
// last saw in a scripted anim (its copy of the record, G_SP_MeasureScriptedPlacement; no server pointers are read
// here): the scripted anim's effective weight in the server tree (at its last think) and in the client tree, the
// client's weighted leaves, and the drawn origin / yaw against an SP-style placement (SP 0x00574990 places the entity
// from its client anim: the scripted anim's delta through the scripted axis, SP 0x007d4fb0) plus the server's last
// offset from its own anim placement. Called after the entity's per-type processing (CG_ProcessEntity).
void CG_SP_MeasureAnimScripted(int localClientNum, const centity_s *cent)
{
    static const dvar_s *enabled;
    if (!enabled)
        enabled = Dvar_FindVar("bo1_measure_animscripted_cl");
    if (!enabled || !enabled->current.enabled || !zombiemode || !zombiemode->current.enabled)
        return;
    const int number = cent->nextState.number;
    DObj *clientObj = Com_GetClientDObj(number, localClientNum);
    const XAnimTree_s *clientTree = clientObj ? DObjGetTree(clientObj) : nullptr;
    if (!clientTree)
        return;
    const cg_s *cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    float ideal[3], idealYaw = 0.0f, serverWeight;
    unsigned int anim;
    int startTime;
    bool placed;
    if (!G_SP_MeasureScriptedPlacement(number, cgameGlob->time, clientObj, ideal, &idealYaw, &anim, &startTime,
            &serverWeight, &placed))
        return;
    char leaves[256];
    MeasureEffectiveLeaves(clientTree, leaves, sizeof(leaves));
    const float dpos = placed ? Vec3Distance(ideal, cent->pose.origin) : -1.0f;
    const float dyaw = placed ? (float)AngleDelta(cent->pose.angles[1], idealYaw) : 0.0f;
    Com_Printf(14, "bo1_asc: t %d ent %d eType %d anim %s since %d svw %.3f clw %.3f cltime %.3f dpos %.2f dyaw %.2f org %.1f %.1f %.1f leaves %s\n",
        cgameGlob->time, number, (int)cent->nextState.eType, XAnimGetAnimName(XAnimGetAnims(clientTree), anim),
        cgameGlob->time - startTime, serverWeight, G_SP_MeasureEffectiveWeight(clientTree, anim),
        (float)XAnimGetTime(clientTree, anim), dpos, dyaw,
        cent->pose.origin[0], cent->pose.origin[1], cent->pose.origin[2], leaves);
}

// zombies: rebuild an actor tree from both buffered snapshots (SP 0x004B8D80,
// called by CG_ResetActorEntity 0x0050E3D0 after updating its DObj).
void CG_SP_ReplayEntityAnimCommands(int localClientNum, int entnum)
{
    if (!zombiemode || !zombiemode->current.enabled)
        return;
    cg_s *cg = CG_GetLocalClientGlobals(localClientNum);
    Replay(localClientNum, cg->snap, entnum);
    Replay(localClientNum, cg->nextSnap, entnum);
}
