// Included by g_sp_savegame.cpp after the weapon-name helpers.
// zombies: entity images (SP 0x007EA5C0 / 0x007EC6C0), DERIVED KB field tables.
// SP gentity is 0x34C with 31 attachments; KB stays 0x2F8 with 19. SP-only fields
// follow the core image in a separate extension image, then the scripted-animation record.
#include <xanim/xanim.h>
#include <game_mp/actor_mp.h>
#include <game/actor_animapi.h>
#include <game/sentient.h>
struct SPEntitySaveField
{
    unsigned offset;
    unsigned size;
    int type;
};

#define EF(member, type) { offsetof(gentity_s, member), sizeof(((gentity_s *)0)->member), type }
#define ATTACH(n) EF(attachModelNames[n], 17), EF(attachTagNames[n], 1)
static const SPEntitySaveField s_entityFields[] = {
    // Same traversal order as SP 0x00A53708, except SP-only fields in s_entityExtFields.
    EF(client, 4), EF(actor, 5), EF(sentient, 6), EF(scr_vehicle, 8),
    EF(pTurretInfo, 9), EF(destructible, 10), EF(classname, 1), EF(model, 17),
    EF(parent, 3), EF(targetname, 1), EF(script_noteworthy, 1), EF(chain, 2), EF(target, 1),
    ATTACH(0), ATTACH(1), ATTACH(2), ATTACH(3), ATTACH(4), ATTACH(5), ATTACH(6),
    ATTACH(7), ATTACH(8), ATTACH(9), ATTACH(10), ATTACH(11), ATTACH(12), ATTACH(13),
    ATTACH(14), ATTACH(15), ATTACH(16), ATTACH(17), ATTACH(18),
    EF(r.ownerNum, 3), EF(missileTargetEnt, 3), EF(pAnimTree, 14), EF(tagInfo, 15), EF(tagChildren, 2),
    // KB-only pointer fields, absent from the SP table. No raw process pointers in the image.
    EF(flame_timed_damage[0].attacker, 2), EF(flame_timed_damage[1].attacker, 2),
    EF(flame_timed_damage[2].attacker, 2), EF(flame_timed_damage[3].attacker, 2), EF(nextFree, 2)
};
#undef ATTACH
#undef EF
#define XF(member, type) { offsetof(gentitySpExt, member), sizeof(((gentitySpExt *)0)->member), type }
static const SPEntitySaveField s_entityExtFields[] = {
    XF(script_linkname, 1), XF(activator, 3), XF(soundNotify, 1)
};
#undef XF
static const SPEntitySaveField s_tagFields[] = { {0, 4, 2}, {4, 4, 2}, {8, 2, 1} };
static_assert(sizeof(gentity_s) == 0x2F8, "DERIVED entity image");
static_assert(sizeof(gentitySpExt) == 0x20, "DERIVED entity extension image");
static_assert(sizeof(tagInfo_s) == 0x70 && offsetof(tagInfo_s, name) == 8, "DERIVED tag image");
static_assert(sizeof(scripted_anim_sp_t) == 0x64, "SP type 16 has no pointer fields");
static_assert(sizeof(PathLinkInfo) == 8 && offsetof(PathLinkInfo, next) == 6, "DERIVED disconnected link");
static_assert(ARRAY_COUNT(((gentity_s *)0)->attachModelNames) == 19, "DERIVED attachments");

static void SP_CheckEntityFields(const SPEntitySaveField *fields, unsigned count, unsigned size)
{
    // The EXE table is not sorted. Validate bounds and pairwise non-overlap without reordering it.
    for (unsigned i = 0; i < count; ++i)
    {
        iassert(fields[i].size == (fields[i].type == 1 || fields[i].type == 11 || fields[i].type == 17 ? 2 : 4));
        iassert(fields[i].type != 18 || fields[i].size == 4);
        iassert(fields[i].offset + fields[i].size <= size);
        for (unsigned j = 0; j < i; ++j)
            iassert(fields[i].offset >= fields[j].offset + fields[j].size
                || fields[j].offset >= fields[i].offset + fields[i].size);
    }
}

extern Destructible s_destructibles[64];

static void *SP_EntityFieldBase(int type, unsigned *stride, unsigned *count)
{
    switch (type)
    {
    case 2: case 3: *stride = sizeof(gentity_s); *count = MAX_GENTITIES; return g_entities;
    case 4: *stride = sizeof(gclient_s); *count = level.maxclients; return level.clients;
    case 5: *stride = sizeof(actor_s); *count = MAX_ACTORS; return level.actors;
    case 6: *stride = sizeof(sentient_s); *count = MAX_SENTIENTS; return level.sentients;
    case 8: *stride = sizeof(scr_vehicle_s); *count = 16; return level.vehicles;
    case 9: *stride = sizeof(TurretInfo); *count = 32; return level.turrets;
    case 10: *stride = sizeof(Destructible); *count = ARRAY_COUNT(s_destructibles); return s_destructibles;
    }
    Com_Error(ERR_DROP, "entity save: unknown reference type %i", type);
    return nullptr;
}

static int SP_EntityReferenceIndex(int type, const void *pointer)
{
    if (!pointer)
        return 0;
    unsigned stride, count;
    const unsigned char *base = (const unsigned char *)SP_EntityFieldBase(type, &stride, &count);
    const unsigned delta = (unsigned)pointer - (unsigned)base;
    if (!base || delta % stride || delta / stride >= count)
        Com_Error(ERR_DROP, "WriteField1: reference type %i out of range", type);
    return delta / stride + 1;
}

static void *SP_EntityReference(int type, int index)
{
    unsigned stride, count;
    unsigned char *base = (unsigned char *)SP_EntityFieldBase(type, &stride, &count);
    if (index < 0 || (unsigned)index > count || (index && !base))
        Com_Error(ERR_DROP, "ReadField: reference type %i out of range (%i)", type, index);
    return index ? base + (index - 1) * stride : nullptr;
}

static void SP_WriteEntityStruct(const SPEntitySaveField *fields, unsigned count,
    const void *source, unsigned size, MemoryFile *memFile);

static int Path_SaveNode(const pathnode_t *node)
{
    // zombies: SP 0x0068F4C0 (WriteField1 type 13): null is 0, else the node index + 1.
    return node ? Path_ConvertNodeToIndex(node) + 1 : 0;
}

static pathnode_t *Path_LoadNode(unsigned int index)
{
    // zombies: SP 0x00627390 (ReadField type 13).
    if (index > g_path.actualNodeCount)
        Com_Error(ERR_DROP, "Path_LoadNode: node out of range (%i)", index);
    return index ? Path_ConvertIndexToNode(index - 1) : nullptr;
}

static unsigned short Scr_SaveObjectIndex(unsigned int id)
{
    // zombies: SP 0x0055E6F0 (WriteField1 type 11): add the object (and children) to the save
    // set if Scr_SavePre did not, then its one-based save index.
    if (!id)
        return 0;
    Scr_AddSaveObject(SCRIPTINSTANCE_SERVER, id);
    return gScrVarPub[SCRIPTINSTANCE_SERVER].saveIdMap[id];
}

static unsigned short Scr_LoadObjectIndex(unsigned int index)
{
    // zombies: SP 0x0064A740 (ReadField type 11): the loaded object, with a reference for the field.
    if (!index)
        return 0;
    const unsigned int id = gScrVarPub[SCRIPTINSTANCE_SERVER].saveIdMapRev[index];
    AddRefToObject(SCRIPTINSTANCE_SERVER, id);
    return (unsigned short)id;
}

// DERIVED animscript index (WriteField1 type 12, SP 0x007EA034 / ReadField 0x00491DF5). SP stores
// the one-based index of an 8-byte entry in its flat animscript lists (0x01C79B64, at most 0x260), or
// -1 for the record's own AnimScriptSpecific (SP actor +0xDD0). KB lists are per species.
static const unsigned SP_ANIMSCRIPTS_PER_LIST = sizeof(AnimScriptList) / sizeof(scr_animscript_t);
static_assert(sizeof(AnimScriptList) % sizeof(scr_animscript_t) == 0, "DERIVED animscript index");

static int SP_SaveAnimScript(const scr_animscript_t *script, const unsigned char *record)
{
    if (!script)
        return 0;
    if ((const unsigned char *)script == record + offsetof(actor_s, AnimScriptSpecific))
        return -1;
    for (int species = 0; species < MAX_AI_SPECIES; ++species)
    {
        const scr_animscript_t *list = (const scr_animscript_t *)g_animScriptTable[species];
        if (list && script >= list && script < list + SP_ANIMSCRIPTS_PER_LIST)
            return species * SP_ANIMSCRIPTS_PER_LIST + (script - list) + 1;
    }
    Com_Error(ERR_DROP, "WriteField1: animscript out of range");
    return 0;
}

static scr_animscript_t *SP_LoadAnimScript(int index, unsigned char *record)
{
    if (index < -1 || index > MAX_AI_SPECIES * (int)SP_ANIMSCRIPTS_PER_LIST)
        Com_Error(ERR_DROP, "ReadField: animscript out of range (%i)", index);
    if (!index)
        return nullptr;
    if (index == -1)
        return (scr_animscript_t *)(record + offsetof(actor_s, AnimScriptSpecific));
    scr_animscript_t *list = (scr_animscript_t *)g_animScriptTable[(index - 1) / SP_ANIMSCRIPTS_PER_LIST];
    if (!list)
        Com_Error(ERR_DROP, "ReadField: animscript out of range (%i)", index);
    return list + (index - 1) % SP_ANIMSCRIPTS_PER_LIST;
}

static void SP_WriteStruct(const SPEntitySaveField *fields, unsigned count,
    const void *source, unsigned char *image, unsigned size, MemoryFile *memFile)
{
    // zombies: WriteStruct / WriteField1 / WriteField2 (SP 0x007EA1D0 / 0x007E9DB0 / 0x007EA120).
    // The caller's image is a copy of source (SP writers may clear transient fields in it first);
    // fields are fixed in the image, while strings and tag records are written from source.
    SP_CheckEntityFields(fields, count, size);
    for (unsigned i = 0; i < count; ++i)
    {
        const SPEntitySaveField &field = fields[i];
        unsigned char *at = image + field.offset;
        if (field.type == 1)
            *(unsigned short *)at = *(unsigned short *)at != 0;
        else if (field.type == 17 || field.type == 18)
            continue; // SP WriteField1 types 17/18 are no-ops (short/int model configstring index).
        else if (field.type == 3)
        {
            const int number = ((EntHandle *)at)->number;
            SP_EntityReference(3, number); // validate without dereferencing an as-yet-unloaded entity
            *(int *)at = number;
        }
        else if (field.type == 14)
        {
            XAnimTree_s *tree = *(XAnimTree_s **)at;
            *(int *)at = tree ? Scr_GetAnimsIndex(XAnimGetAnims(tree), SCRIPTINSTANCE_SERVER) : 0;
        }
        else if (field.type == 15)
            *(int *)at = *(void **)at != nullptr;
        else if (field.type == 7)
        {
            // SP 0x007E9F0A: the one-based sentient number as an int (clears infoIndex).
            const int number = ((SentientHandle *)at)->number;
            if (number > (int)ARRAY_COUNT(g_sentients))
                Com_Error(ERR_DROP, "WriteField1: sentient out of range (%i)", number);
            *(int *)at = number;
        }
        else if (field.type == 11)
            *(unsigned short *)at = Scr_SaveObjectIndex(*(unsigned short *)at);
        else if (field.type == 12)
            *(int *)at = SP_SaveAnimScript(*(scr_animscript_t **)at, (const unsigned char *)source);
        else if (field.type == 13)
            *(int *)at = Path_SaveNode(*(pathnode_t **)at);
        else
            *(int *)at = SP_EntityReferenceIndex(field.type, *(void **)at);
    }
    MemFile_WriteData(memFile, size, image);
    for (unsigned i = 0; i < count; ++i)
    {
        const SPEntitySaveField &field = fields[i];
        const unsigned char *at = (const unsigned char *)source + field.offset;
        if (field.type == 1 && *(const unsigned short *)at)
            MemFile_WriteCString(memFile, (char *)SL_ConvertToString(*(const unsigned short *)at, SCRIPTINSTANCE_SERVER));
        else if (field.type == 15 && *(tagInfo_s *const *)at)
            SP_WriteEntityStruct(s_tagFields, ARRAY_COUNT(s_tagFields), *(tagInfo_s *const *)at, sizeof(tagInfo_s), memFile);
    }
}

static void SP_WriteEntityStruct(const SPEntitySaveField *fields, unsigned count,
    const void *source, unsigned size, MemoryFile *memFile)
{
    // zombies: SP writers that copy the whole record to a stack image before WriteStruct.
    unsigned char image[sizeof(gentity_s)];
    iassert(size <= sizeof(image));
    memcpy(image, source, size);
    SP_WriteStruct(fields, count, source, image, size, memFile);
}

static void SP_ReadEntityStruct(const SPEntitySaveField *fields, unsigned count,
    void *dest, unsigned size, MemoryFile *memFile)
{
    // zombies: ReadField (SP 0x00491B50). Destination handles must be empty before image replacement.
    SP_CheckEntityFields(fields, count, size);
    MemFile_ReadData(memFile, size, (unsigned char *)dest);
    for (unsigned i = 0; i < count; ++i)
    {
        const SPEntitySaveField &field = fields[i];
        unsigned char *at = (unsigned char *)dest + field.offset;
        if (field.type == 1)
        {
            if (*(unsigned short *)at)
                *(unsigned short *)at = SL_GetString(MemFile_ReadCString(memFile), 0, SCRIPTINSTANCE_SERVER);
        }
        else if (field.type == 17 || field.type == 18)
        {
            // Retained configstrings are unchanged in the partial, same-map loader. The full
            // segment-1 configstring reader must supply SP's model-index remap (0x01C06C0C);
            // ReadField remaps a short (17) or an int (18) through it.
            continue;
        }
        else if (field.type == 3)
        {
            gentity_s *ent = (gentity_s *)SP_EntityReference(3, *(int *)at);
            *(EntHandle *)at = {};
            if (ent)
                ((EntHandle *)at)->setEnt(ent);
        }
        else if (field.type == 14)
        {
            const int index = *(int *)at;
            *(XAnimTree_s **)at = index ? Com_XAnimCreateSmallTree(Scr_GetAnims(index, SCRIPTINSTANCE_SERVER)) : nullptr;
        }
        else if (field.type == 15)
        {
            if (*(int *)at)
            {
                *(tagInfo_s **)at = (tagInfo_s *)MT_Alloc(sizeof(tagInfo_s), 17, SCRIPTINSTANCE_SERVER);
                SP_ReadEntityStruct(s_tagFields, ARRAY_COUNT(s_tagFields), *(tagInfo_s **)at, sizeof(tagInfo_s), memFile);
            }
        }
        else if (field.type == 7)
        {
            // SP 0x00491CDB: clear, then register the handle (sentients are read later).
            const int number = *(int *)at;
            if (number < 0 || number > (int)ARRAY_COUNT(g_sentients))
                Com_Error(ERR_DROP, "ReadField: sentient out of range (%i)", number);
            *(SentientHandle *)at = {};
            if (number)
                ((SentientHandle *)at)->setSentient(&g_sentients[number - 1]);
        }
        else if (field.type == 11)
            *(unsigned short *)at = Scr_LoadObjectIndex(*(unsigned short *)at);
        else if (field.type == 12)
            *(scr_animscript_t **)at = SP_LoadAnimScript(*(int *)at, (unsigned char *)dest);
        else if (field.type == 13)
            *(pathnode_t **)at = Path_LoadNode(*(unsigned int *)at);
        else
            *(void **)at = SP_EntityReference(field.type, *(int *)at);
    }
}

static void Path_WriteDisconnectedLinks(const gentity_s *ent, MemoryFile *memFile)
{
    // zombies: SP 0x005E8120, circular ring in stored order, no count or sentinel suffix.
    unsigned index = ent->disconnectedLinks;
    if (!index)
        return;
    unsigned count = 0;
    do
    {
        if (index >= ARRAY_COUNT(g_path.pathLinkInfoArray) || ++count >= ARRAY_COUNT(g_path.pathLinkInfoArray))
            Com_Error(ERR_DROP, "entity save: invalid disconnected link ring");
        PathLinkInfo *info = &g_path.pathLinkInfoArray[index];
        MemFile_WriteData(memFile, sizeof(*info), (unsigned char *)info);
        index = info->next;
    } while (index != ent->disconnectedLinks);
}

static void Path_ReadDisconnectedLinks(const gentity_s *ent, MemoryFile *memFile)
{
    // zombies: SP 0x005DE870: find the destination link backwards, increment disconnect count,
    // remove this slot from the free ring, install saved ring metadata, then follow saved next.
    unsigned index = ent->disconnectedLinks;
    if (!index)
        return;
    unsigned count = 0;
    do
    {
        if (index >= ARRAY_COUNT(g_path.pathLinkInfoArray) || ++count >= ARRAY_COUNT(g_path.pathLinkInfoArray))
            Com_Error(ERR_DROP, "entity load: invalid disconnected link ring");
        PathLinkInfo saved;
        MemFile_ReadData(memFile, sizeof(saved), (unsigned char *)&saved);
        if (saved.from >= g_path.actualNodeCount || saved.prev >= ARRAY_COUNT(g_path.pathLinkInfoArray)
            || saved.next >= ARRAY_COUNT(g_path.pathLinkInfoArray))
            Com_Error(ERR_DROP, "entity load: disconnected link out of range");
        pathnode_t *node = &gameWorldCurrent->path.nodes[saved.from];
        int link = node->constant.totalLinkCount - 1;
        while (link >= 0 && node->constant.Links[link].nodeNum != saved.to)
            --link;
        if (link < 0)
            Com_Error(ERR_DROP, "entity load: disconnected link missing");
        ++node->constant.Links[link].disconnectCount;
        PathLinkInfo *slot = &g_path.pathLinkInfoArray[index];
        g_path.pathLinkInfoArray[slot->prev].next = slot->next;
        g_path.pathLinkInfoArray[slot->next].prev = slot->prev;
        *slot = saved;
        index = slot->next;
    } while (index != ent->disconnectedLinks);
}

static void G_WriteEntity(const gentity_s *ent, const gentitySpExt *ext,
    const scripted_anim_sp_t *scripted, int sequence, MemoryFile *memFile)
{
    // zombies: SP 0x007EA5C0. The DERIVED side images represent fields of SP's entity image.
    SP_WriteEntityStruct(s_entityFields, ARRAY_COUNT(s_entityFields), ent, sizeof(*ent), memFile);
    SP_WriteEntityStruct(s_entityExtFields, ARRAY_COUNT(s_entityExtFields), ext, sizeof(*ext), memFile);
    int hasScripted = scripted != nullptr; // SP field type 16; 0x00A54044 is an empty field table.
    MemFile_WriteData(memFile, 4, (unsigned char *)&hasScripted);
    if (scripted)
        MemFile_WriteData(memFile, sizeof(*scripted), (unsigned char *)scripted);
    if (ent->s.weapon)
    {
        G_SaveWeaponName(memFile, BG_GetWeaponVariantDef(ent->s.weapon)->szInternalName);
        int model = ent->s.weaponModel;
        MemFile_WriteData(memFile, 4, (unsigned char *)&model);
    }
    Path_WriteDisconnectedLinks(ent, memFile);
    MemFile_WriteData(memFile, 4, (unsigned char *)&sequence);
}

static bool G_ReadEntity(gentity_s *ent, gentitySpExt *ext, scripted_anim_sp_t *scripted,
    int *sequence, MemoryFile *memFile)
{
    // zombies: SP 0x007EC6C0. The caller supplies stable destinations for registered handles.
    SP_ReadEntityStruct(s_entityFields, ARRAY_COUNT(s_entityFields), ent, sizeof(*ent), memFile);
    SP_ReadEntityStruct(s_entityExtFields, ARRAY_COUNT(s_entityExtFields), ext, sizeof(*ext), memFile);
    int hasScripted;
    MemFile_ReadData(memFile, 4, (unsigned char *)&hasScripted);
    if (hasScripted)
        MemFile_ReadData(memFile, sizeof(*scripted), (unsigned char *)scripted);
    if (ent->s.weapon)
    {
        char name[256];
        G_LoadWeaponName(memFile, name);
        ent->s.weapon = G_GetWeaponIndexForName(name);
        int model;
        // Brief said restore the model byte; exe reads/discards this int, retaining the image byte.
        MemFile_ReadData(memFile, 4, (unsigned char *)&model);
    }
    if (ent->physObjId && ent->physObjId != -1) // SP entity +0x32C
        ent->physObjId = 0;
    Path_ReadDisconnectedLinks(ent, memFile);
    MemFile_ReadData(memFile, 4, (unsigned char *)sequence);
    return hasScripted != 0;
}

static void SP_ReleaseEntityFields(const SPEntitySaveField *fields, unsigned count, void *dest)
{
    // TEMPORARY diagnostic cleanup: decoded references target tables whose payload is not loaded.
    // Release registry entries directly, without EntHandle::ent()'s live-entity assertion.
    extern EntHandleList g_entitiesHandleList[MAX_GENTITIES_SV];
    for (unsigned i = 0; i < count; ++i)
    {
        unsigned char *at = (unsigned char *)dest + fields[i].offset;
        if (fields[i].type == 1)
            Scr_SetString((unsigned short *)at, 0, SCRIPTINSTANCE_SERVER);
        else if (fields[i].type == 3)
        {
            EntHandle *handle = (EntHandle *)at;
            if (handle->number)
                RemoveEntHandleInfo(&g_entitiesHandleList[handle->number - 1], handle->infoIndex);
            *handle = {};
        }
        else if (fields[i].type == 7)
        {
            extern EntHandleList g_sentientsHandleList[MAX_SENTIENTS_CAP];
            SentientHandle *handle = (SentientHandle *)at;
            if (handle->number)
                RemoveEntHandleInfo(&g_sentientsHandleList[handle->number - 1], handle->infoIndex);
            *handle = {};
        }
        else if (fields[i].type == 11 && *(unsigned short *)at)
        {
            RemoveRefToObject(SCRIPTINSTANCE_SERVER, *(unsigned short *)at);
            *(unsigned short *)at = 0;
        }
        else if (fields[i].type == 15 && *(tagInfo_s **)at)
        {
            SP_ReleaseEntityFields(s_tagFields, ARRAY_COUNT(s_tagFields), *(tagInfo_s **)at);
            MT_Free(*(_BYTE **)at, sizeof(tagInfo_s), SCRIPTINSTANCE_SERVER);
        }
        else if (fields[i].type == 14 && *(XAnimTree_s **)at)
            Com_XAnimFreeSmallTree(*(XAnimTree_s **)at);
    }
}
