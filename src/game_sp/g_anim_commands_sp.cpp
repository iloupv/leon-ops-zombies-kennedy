#include "g_anim_commands_sp.h"
#include <game_mp/g_main_mp.h>
#include <game/enthandle.h>
#include <bgame/bg_local.h>
#include <bgame/bg_animation.h>
#include <qcommon/common.h>
#include <xanim/xanim.h>
#include <qcommon/msg_anim_commands.h>
#include <qcommon/msg.h>
#include <server_mp/sv_main_mp.h>
#include <algorithm>
#include <climits>
#include <universal/memfile.h>
#include <game_sp/g_sp_measure.h>

namespace
{
    // SP has 1024 records (0x01be6390..0x01bf1390, stride 44). Reserve index 0:
    // KB's existing SetAnim passes literal 0, which must never refer to a lane command.
    struct AnimCommandRefs
    {
        unsigned short current;
        unsigned short dependent;
        unsigned short previous;
        int blendEndTime;
    };
    struct EntityCommands
    {
        EntHandle owner;
        int sequence;
    };
    AnimCommand commands[1025];
    int references[1025];
    bool deferred[32][1025];
    AnimCommandRefs nodeRefs[XANIM_INFO_MAX]; // mod: entries >= g_xAnimInfoCount unused (retail 4096)
    struct NodeRefsRange { AnimCommandRefs *b, *e; AnimCommandRefs *begin() const { return b; } AnimCommandRefs *end() const { return e; } };
    NodeRefsRange NodeRefsInUse() { return { nodeRefs, nodeRefs + g_xAnimInfoCount }; }
    EntityCommands entities[MAX_GENTITIES_SV];
    unsigned int nextCommand = 1;
    // zombies: collection watermark, SP global 0x028890E0 (SP 0x006048E6).
    // Independent of level_bgs.latestSnapshotTime, which G_RunFrame advances.
    int latestAnimCommandSnapshotTime;

    float ClampCommandValue(float value, float maximum)
    {
        float result = maximum;
        if (value - maximum < 0.0f)
            result = value;
        if (0.0f <= -value)
            result = 0.0f;
        return result;
    }

    void Release(unsigned short &index)
    {
        if (index)
            --references[index];
        index = 0;
    }

    int AcknowledgedTime(const client_t *client)
    {
        if (client->header.deltaMessage <= 0
            || client->header.netchan.outgoingSequence - client->header.deltaMessage >= 32)
            return -1;
        return client->frames[client->header.deltaMessage & 31].serverTime;
    }

    // zombies: G_AnimCmdRefCount (SP 0x00473510). The brief said the server half
    // was ported; the exe also retains the previous command until blend/ack permits replacement.
    void __cdecl G_AnimCmdRefCount(short cmdIndex, short infoIndex, int blendEndTime)
    {
        iassert(infoIndex > 0 && infoIndex < ARRAY_COUNT(nodeRefs));
        AnimCommandRefs &refs = nodeRefs[infoIndex];
        unsigned short index = cmdIndex > 0 ? cmdIndex : 0;
        if (cmdIndex == -1)
        {
            Release(refs.current);
            Release(refs.previous);
            // KB frees both references through this callback; SP has a separate dependent callback.
            Release(refs.dependent);
            refs.blendEndTime = -1;
            return;
        }
        if (!index || refs.current == index || refs.previous == index)
            return;
        int acknowledged = INT_MAX;
        for (int i = 0; i < com_maxclients->current.integer; ++i)
        {
            const client_t *client = &svs.clients[i];
            // KB dedicated test clients have no receiving client or acknowledgements.
            if (client->header.state >= CS_CONNECTED && !client->bIsTestClient)
                acknowledged = (std::min)(acknowledged, AcknowledgedTime(client));
        }
        if (refs.previous)
        {
            Release(refs.current);
            refs.current = refs.previous;
            refs.previous = 0;
            refs.blendEndTime = -1;
        }
        if (blendEndTime >= svs.time && (!refs.current
            || commands[refs.current].startTime == blendEndTime
            || commands[refs.current].startTime <= acknowledged))
        {
            Release(refs.current);
            refs.current = index;
            refs.blendEndTime = -1;
        }
        else
        {
            refs.previous = index;
            refs.blendEndTime = blendEndTime;
        }
        ++references[index];
    }

    // zombies: G_AnimCmdAddDependent (SP 0x00491850).
    void __cdecl G_AnimCmdAddDependent(short cmdIndex, short infoIndex)
    {
        iassert(infoIndex > 0 && infoIndex < ARRAY_COUNT(nodeRefs));
        unsigned short index = cmdIndex > 0 ? cmdIndex : 0;
        unsigned short &dependent = nodeRefs[infoIndex].dependent;
        if (dependent == index)
            return;
        Release(dependent);
        dependent = index;
        if (index)
            ++references[index];
    }

    // zombies: drop every XAnim node reference to a command being freed (SP 0x007d4df0, command in ESI;
    // node records 0x01BDA388..0x01BE638C, stride 12: current +0, dependent +2, previous +4, blend end +8).
    // A current reference is replaced by the node's previous command when it has one. Only current and
    // previous references count down the command's reference count; the walk stops when that is used up.
    void ReleaseCommandReferences(unsigned short index)
    {
        int count = references[index];
        for (AnimCommandRefs &refs : NodeRefsInUse())
        {
            if (count <= 0)
                return;
            if (refs.current == index)
            {
                if (refs.previous)
                {
                    refs.current = refs.previous;
                    refs.previous = 0;
                    refs.blendEndTime = -1;
                }
                else
                {
                    refs.current = 0;
                }
                --count;
            }
            else if (refs.previous == index)
            {
                refs.previous = 0;
                --count;
            }
            else if (refs.dependent == index)
            {
                refs.dependent = 0;
            }
        }
    }

    void ClearEntityCommands(int entnum)
    {
        for (AnimCommandRefs &refs : NodeRefsInUse())
        {
            if (refs.current && commands[refs.current].entnum == entnum)
                Release(refs.current);
            if (refs.dependent && commands[refs.dependent].entnum == entnum)
                Release(refs.dependent);
            if (refs.previous && commands[refs.previous].entnum == entnum)
                Release(refs.previous);
        }
        for (AnimCommand &command : commands)
        {
            if (command.type && command.entnum == entnum)
                command = {};
        }
        entities[entnum].sequence = 0;
    }
}

// zombies: entity-save sequence (SP 0x007EA67B, table 0x01C0317C).
int G_SP_GetAnimCommandSequence(int entnum)
{
    iassert(entnum >= 0 && entnum < MAX_GENTITIES_SV);
    return entities[entnum].sequence;
}

// zombies: G_StoreAnimCommand (SP 0x004e7d80), unoptimized allocation
// path (SP 0x007d4c80). Keep SP's stored-value clamps, timestamp and per-entity order.
// The caller must still pass the ORIGINAL floats to XAnim, as the exe does.
int G_StoreAnimCommand(gentity_s *ent, XAnimTree_s *tree, int type, unsigned int anim,
    unsigned int root, float weight, float time, float rate, unsigned char flags)
{
    if (!zombiemode || !zombiemode->current.enabled)
        return -1;

    iassert(tree);
    iassert(bgs == &level_bgs);
    level_bgs.AnimCmdRefCount = G_AnimCmdRefCount;
    level_bgs.AnimCmdAddDependent = G_AnimCmdAddDependent;

    // EntHandleDissociate clears these handles when KB frees an entity, including
    // level shutdown. Retire its history before a recycled entity number can use it.
    // No pointers into freed trees or script strings are retained in the history.
    // (L15: entities with their own tree are freed eagerly as SP, G_SP_FreeEntityAnimCommands from
    // G_FreeEntity / G_SetAnimTree. KB-only: actors, whose tree KB's Actor_Free releases but SP keeps in
    // its actor bank, still retire here.)
    for (int i = 0; i < MAX_GENTITIES_SV; ++i)
    {
        if (entities[i].sequence && !entities[i].owner.isDefined())
            ClearEntityCommands(i);
    }
    EntityCommands &owner = entities[ent->s.number];
    if (!owner.owner.isDefined())
        owner.owner.setEnt(ent);

    for (unsigned int count = 0; count < ARRAY_COUNT(commands) - 1; ++count)
    {
        unsigned int index = nextCommand++;
        if (nextCommand == ARRAY_COUNT(commands))
            nextCommand = 1;
        AnimCommand &command = commands[index];
        // zombies: SP 0x007D4D98 only protects clear/knob commands past collection;
        // unreferenced set-weight/set-time commands can be reused immediately.
        if (references[index] || ((command.type == 1 || command.type == 2
            || command.type == 4 || command.type == 5) && command.startTime > latestAnimCommandSnapshotTime))
            continue;

        command.index = index;
        command.type = type;
        // zombies: defer commands recorded after collection (SP 0x004E7D80).
        command.startTime = level.time == latestAnimCommandSnapshotTime ? level.time + 50 : level.time;
        if (!level.time)
            command.startTime = 50;
        command.sequence = owner.sequence++;
        command.entnum = ent->s.number;
        command.anim = anim;
        command.root = root;
        command.weight = ClampCommandValue(weight, 1.0f);
        command.time = ClampCommandValue(time, 1.5f);
        command.rate = ClampCommandValue(rate, 3.0f);
        command.flags = flags;
        return index;
    }
    Com_Printf(19, "G_StoreAnimCommand:  T-Pose potential. No available animation slots\n");
    return -1;
}

// zombies: free an entity's stored animation commands (SP 0x004fdf30), from G_SetAnimTree (SP 0x00502895)
// and G_FreeEntity (SP 0x00438b89). Each command of the entity in the used list gets start time 0 (so no
// snapshot sends it again), loses its node references (0x007d4df0), reference count 0, and goes back to the
// free list (0x00558180); KB's free slot is type 0. SP's g_oldAnimCmdNetwork (dvar 0x01BDA36C, default 0,
// registered 0x007E283F) instead only zeroes the start times of all 1024 records; KB does not register it.
// The per-entity sequence (SP 0x01C0317C) is not touched here.
void G_SP_FreeEntityAnimCommands(const gentity_s *ent)
{
    if (!zombiemode || !zombiemode->current.enabled)
        return;
    const int entnum = ent->s.number;
    int freed = 0, pending = 0, referenced = 0; // L15 TEST counts (bo1_measure)
    for (unsigned short i = 1; i < ARRAY_COUNT(commands); ++i)
    {
        AnimCommand &command = commands[i];
        if (!command.type || command.entnum != entnum)
            continue;
        ++freed;
        pending += command.startTime > latestAnimCommandSnapshotTime;
        referenced += references[i] != 0;
        command.startTime = 0;
        ReleaseCommandReferences(i);
        references[i] = 0;
        command = {};
    }
    // L15 (TEST): commands freed per call; "pending" ones were not yet collected for a snapshot.
    if (freed && G_SP_MeasureEnabled())
        Com_Printf(15, "bo1_animcmd_free: time %d ent %d eType %d inuse %d freed %d pending %d referenced %d\n",
            level.time, entnum, (int)ent->s.eType, (int)ent->r.inuse, freed, pending, referenced);
}

void G_SP_ResetAnimCommands()
{
    memset(commands, 0, sizeof(commands));
    memset(references, 0, sizeof(references));
    memset(nodeRefs, 0, sizeof(nodeRefs));
    memset(entities, 0, sizeof(entities));
    memset(deferred, 0, sizeof(deferred));
    nextCommand = 1;
    latestAnimCommandSnapshotTime = 0;
}

void G_SP_GetAnimCommandUsage(int *referenced, int *protectedCommands, int *snapshotTime)
{
    *referenced = *protectedCommands = 0;
    *snapshotTime = latestAnimCommandSnapshotTime;
    for (int i = 1; i < ARRAY_COUNT(commands); ++i)
    {
        if (references[i])
            ++*referenced;
        else if ((commands[i].type == 1 || commands[i].type == 2
            || commands[i].type == 4 || commands[i].type == 5)
            && commands[i].startTime > latestAnimCommandSnapshotTime)
            ++*protectedCommands;
    }
}

// zombies: G_SaveState's animation command sections. SP keeps the commands in a 1024-entry
// pool with free/used lists of 6-byte nodes {self, prev, next} (SP 0x01C01908, heads 0x01A55FDC..E8)
// and a uint16 reference count per command (SP 0x01BD9B58); the per-node references live in the
// XAnim info. KB's history (above) has no lists, so each SP section carries KB's counterpart
// (DERIVED): the records section the allocator cursor, the list section every command slot in use
// in index order, the heads section the per-info references. Not saved: the per-client deferral
// flags (network acknowledgement state, cleared by the reader) and the per-entity owner handles
// (the entity records carry the sequence, SP 0x007EA67B).
void G_SP_WriteAnimCommandRecords(MemoryFile *memFile)
{
    // SP 0x007EDD8A..0x007EDDD8 writes the 1024 list nodes.
    int cursor = nextCommand;
    MemFile_WriteData(memFile, 4, (unsigned char *)&cursor);
}

void G_SP_ReadAnimCommandRecords(MemoryFile *memFile)
{
    // SP 0x007ED056..0x007ED075 reads the 1024 list nodes.
    int cursor;
    MemFile_ReadData(memFile, 4, (unsigned char *)&cursor);
    if (cursor < 1 || cursor >= ARRAY_COUNT(commands))
        Com_Error(ERR_DROP, "\x15G_LoadGame: animcmd num out of range (%i, MAX = %i)", cursor, ARRAY_COUNT(commands));
    nextCommand = cursor;
    memset(deferred, 0, sizeof(deferred));
}

void G_SP_WriteAnimCommandList(MemoryFile *memFile)
{
    // SP 0x007EDDDA..0x007EDF1E: for each used command, its index (int), the 0x2C-byte command
    // and its uint16 reference count; -1 ends the list.
    for (int i = 1; i < ARRAY_COUNT(commands); ++i)
    {
        if (!commands[i].type)
            continue;
        iassert(references[i] >= 0 && references[i] <= 0xFFFF);
        unsigned short count = (unsigned short)references[i];
        MemFile_WriteData(memFile, 4, (unsigned char *)&i);
        MemFile_WriteData(memFile, sizeof(AnimCommand), (unsigned char *)&commands[i]);
        MemFile_WriteData(memFile, 2, (unsigned char *)&count);
    }
    int end = -1;
    MemFile_WriteData(memFile, 4, (unsigned char *)&end);
}

void G_SP_ReadAnimCommandList(MemoryFile *memFile)
{
    // SP 0x007ED077..0x007ED0F1. The pool is empty here (SP 0x005579D0 ran before the VM load).
    memset(commands, 0, sizeof(commands));
    memset(references, 0, sizeof(references));
    int index;
    MemFile_ReadData(memFile, 4, (unsigned char *)&index);
    while (index >= 0)
    {
        if (index < 1 || index >= ARRAY_COUNT(commands))
            Com_Error(ERR_DROP, "\x15G_LoadGame: animcmd num out of range (%i, MAX = %i)", index, ARRAY_COUNT(commands));
        unsigned short count;
        MemFile_ReadData(memFile, sizeof(AnimCommand), (unsigned char *)&commands[index]);
        MemFile_ReadData(memFile, 2, (unsigned char *)&count);
        references[index] = count;
        MemFile_ReadData(memFile, 4, (unsigned char *)&index);
    }
}

void G_SP_WriteAnimCommandHeads(MemoryFile *memFile)
{
    // SP 0x007EA760 writes the free and used list heads/tails as int16 node indices.
    for (int i = 1; i < g_xAnimInfoCount; ++i)
    {
        const AnimCommandRefs &refs = nodeRefs[i];
        if (!refs.current && !refs.dependent && !refs.previous)
            continue;
        short info = (short)i;
        MemFile_WriteData(memFile, 2, (unsigned char *)&info);
        MemFile_WriteData(memFile, sizeof(refs), (unsigned char *)&refs);
    }
    short end = -1;
    MemFile_WriteData(memFile, 2, (unsigned char *)&end);
}

void G_SP_ReadAnimCommandHeads(MemoryFile *memFile)
{
    // SP 0x007EA890.
    memset(nodeRefs, 0, sizeof(nodeRefs));
    short info;
    MemFile_ReadData(memFile, 2, (unsigned char *)&info);
    while (info >= 0)
    {
        if (info < 1 || info >= g_xAnimInfoCount)
            Com_Error(ERR_DROP, "\x15G_LoadGame: animcmd info out of range (%i, MAX = %i)", info, g_xAnimInfoCount);
        MemFile_ReadData(memFile, sizeof(AnimCommandRefs), (unsigned char *)&nodeRefs[info]);
        MemFile_ReadData(memFile, 2, (unsigned char *)&info);
    }
}

void G_SP_BackupAnimCommands(bool save)
{
    static AnimCommand savedCommands[ARRAY_COUNT(commands)];
    static int savedReferences[ARRAY_COUNT(references)];
    static bool savedDeferred[32][1025];
    static AnimCommandRefs savedRefs[ARRAY_COUNT(nodeRefs)];
    static unsigned int savedNext;
    if (save)
    {
        memcpy(savedCommands, commands, sizeof(commands));
        memcpy(savedReferences, references, sizeof(references));
        memcpy(savedDeferred, deferred, sizeof(deferred));
        memcpy(savedRefs, nodeRefs, sizeof(nodeRefs));
        savedNext = nextCommand;
        return;
    }
    memcpy(commands, savedCommands, sizeof(commands));
    memcpy(references, savedReferences, sizeof(references));
    memcpy(deferred, savedDeferred, sizeof(deferred));
    memcpy(nodeRefs, savedRefs, sizeof(nodeRefs));
    nextCommand = savedNext;
}

void SV_SP_CollectAnimCommands()
{
    if (!zombiemode || !zombiemode->current.enabled)
        return;
    // zombies: snapshot collection advances the reuse watermark (SP 0x006048e6).
    // KB omits bot snapshots entirely. Collect once per send pass even with no
    // receiving client, or unreferenced clear/knob commands remain protected forever.
    latestAnimCommandSnapshotTime = level.time;
}

// zombies: collect/sort commands (SP 0x00604894, comparator 0x00650100), then write
// the list (SP 0x0087F000). KB uses the serialized path for loopback too; no server pointers
// cross the message boundary. Collection and writing are adjacent in KB's snapshot send.
void SV_SP_WriteAnimCommands(client_t *client, msg_t *msg)
{
    if (!zombiemode || !zombiemode->current.enabled)
        return;
    const int clientNum = client - svs.clients;
    const clientSnapshot_t *frame = &client->frames[client->header.netchan.outgoingSequence & 31];
    bool sent[MAX_GENTITIES_SV] = {};
    for (int i = 0; i < frame->num_entities; ++i)
        sent[svsHeader.snapshotEntities[(frame->first_entity + i) % svsHeader.numSnapshotEntities].number] = true;
    const int acknowledged = AcknowledgedTime(client);
    // zombies: SP Com_ServerLoop runs G_RunFrame, then SV_SendClientMessages
    // (0x0087E2A0), which calls SV_BuildClientSnapshot at 0x004659C7. Its
    // command collection advances this separate watermark (0x006048E0).
    // KB collects here immediately before serialization, after G_RunFrame.
    // SV_SendClientMessages also collects when KB has only snapshot-less bots.
    SV_SP_CollectAnimCommands();
    AnimCommand list[1024];
    int count = 0;
    for (int i = 1; i < ARRAY_COUNT(commands); ++i)
    {
        const AnimCommand &command = commands[i];
        if (!command.startTime)
            continue;
        if (command.startTime <= acknowledged && !deferred[clientNum][i])
            continue;
        // KB sends every visible entity each snapshot, without SP's distance anim throttling.
        if (sent[command.entnum])
        {
            deferred[clientNum][i] = false;
            list[count++] = command;
        }
        else
            deferred[clientNum][i] = true;
    }
    std::sort(list, list + count, [](const AnimCommand &a, const AnimCommand &b)
    {
        if (a.startTime != b.startTime)
            return a.startTime < b.startTime;
        if (a.entnum != b.entnum)
            return a.entnum < b.entnum;
        return a.sequence < b.sequence;
    });
    AnimCommandMsgState state;
    for (int i = 0; i < count; ++i)
        MSG_WriteAnimCommand(msg, svsHeader.time, &state, &list[i]);
    MSG_WriteBits(msg, 11, 4);
}
