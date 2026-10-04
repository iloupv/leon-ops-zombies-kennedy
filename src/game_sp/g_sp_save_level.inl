// zombies: G_SaveState segment 3 after the script vehicles (SP 0x007EDD81..0x007EE0F6) and the
// G_LoadGame readers (SP 0x007ED04D..0x007ED278). Included by g_sp_savegame.cpp.
#include <universal/CurveManager.h>
#include <game/turret.h>
#include <game/actor_threat.h>
#include <game/actor_event_listeners.h>
#include <game/g_missile.h>
#include <game/g_player_corpse.h>
#include <game_mp/g_scr_main_mp.h>

extern threat_bias_t g_threatBias;
extern TurretInfo turretInfoStore[32];
extern int g_listenerCount;
extern AIEventListener g_AIEVlisteners[32];

static_assert(sizeof(cCurve) == 0xE58, "SP curve record");
static_assert(sizeof(TurretInfo) == 0xE0 && offsetof(TurretInfo, detachSentient) == 0x68, "SP turret record");
static_assert(sizeof(threat_bias_t) == 0x424, "SP threat bias record");
static_assert(sizeof(AIEventListener) == 8, "SP event listener record");
static_assert(sizeof(trigger_info_t) == 0xC, "SP trigger record");
static_assert(sizeof(AttractorRepulsor_t) == 0x1C, "SP attractor record");
static_assert(ARRAY_COUNT(((vehicle_info_t *)0)->sndIndices) == 19, "SP vehicle info sound indices");

static void G_SaveCurves(MemoryFile *memFile)
{
    // zombies: SP 0x0065AB20 -> 0x0064A050 per curve: the 4 cCurveManager curves, raw.
    for (int i = 0; i < ARRAY_COUNT(cCurveManager::mCurves); ++i)
        MemFile_WriteData(memFile, sizeof(cCurve), (unsigned char *)&cCurveManager::mCurves[i]);
}

static void G_LoadCurves(MemoryFile *memFile, cCurve *curves)
{
    // zombies: SP 0x00546670 -> 0x004174E0 per curve.
    for (int i = 0; i < ARRAY_COUNT(cCurveManager::mCurves); ++i)
        MemFile_ReadData(memFile, sizeof(cCurve), (unsigned char *)&curves[i]);
}

static void G_SaveVehicleInfoSounds(MemoryFile *memFile)
{
    // zombies: SP 0x004C0740: for every loaded vehicle info (SP 0x01C88FE4 count, 0x1DC8 stride),
    // each non-zero sound index of the 19 (SP +0x950) is written; zero ones are skipped.
    for (int i = 0; i < bg_numVehicleInfos; ++i)
    {
        for (int j = 0; j < 19; ++j)
        {
            if (bg_vehicleInfos[i].sndIndices[j])
                MemFile_WriteData(memFile, 4, (unsigned char *)&bg_vehicleInfos[i].sndIndices[j]);
        }
    }
}

static void G_LoadVehicleInfoSounds(MemoryFile *memFile, unsigned int (*indices)[19])
{
    // zombies: SP 0x004FF9F0: reads an index where the live one is non-zero (same vehicle infos).
    for (int i = 0; i < bg_numVehicleInfos; ++i)
    {
        for (int j = 0; j < 19; ++j)
        {
            indices[i][j] = bg_vehicleInfos[i].sndIndices[j];
            if (indices[i][j])
                MemFile_ReadData(memFile, 4, (unsigned char *)&indices[i][j]);
        }
    }
}

#define TF(member, type) { offsetof(TurretInfo, member), sizeof(((TurretInfo *)0)->member), type }
static const SPEntitySaveField s_turretFields[] = {
    // SP 0x00A541E0: +0x10 / +0x14 entity handles, +0x68 sentient handle (same layout in KB).
    TF(manualTarget, 3), TF(target, 3), TF(detachSentient, 7)
};
#undef TF

static void G_WriteTurret(const TurretInfo *turret, MemoryFile *memFile)
{
    // zombies: SP 0x005D9700: the in-use byte as an int, then WriteStruct of the stack image.
    int inuse = turret->inuse;
    MemFile_WriteData(memFile, 4, (unsigned char *)&inuse);
    if (inuse)
        SP_WriteEntityStruct(s_turretFields, ARRAY_COUNT(s_turretFields), turret, sizeof(*turret), memFile);
}

static bool G_ReadTurret(TurretInfo *turret, MemoryFile *memFile)
{
    // zombies: SP 0x007ED105..0x007ED152: the int, then the record and ReadField per field.
    int inuse;
    MemFile_ReadData(memFile, 4, (unsigned char *)&inuse);
    if (!inuse)
        return false;
    SP_ReadEntityStruct(s_turretFields, ARRAY_COUNT(s_turretFields), turret, sizeof(*turret), memFile);
    return true;
}

static void G_WriteActorCorpse(const corpseInfo_t *corpse, MemoryFile *memFile)
{
    // zombies: SP 0x007EDF70..0x007EDFF8: the entity number (SP +4), then for a used slot the
    // 0x18-byte ground-orientation state (SP +8). KB's corpse has no orientation state; it
    // writes its corpse time and falling flag there (DERIVED).
    MemFile_WriteData(memFile, 4, (unsigned char *)&corpse->entnum);
    if (corpse->entnum == -1)
        return;
    int falling = corpse->falling;
    MemFile_WriteData(memFile, 4, (unsigned char *)&corpse->time);
    MemFile_WriteData(memFile, 4, (unsigned char *)&falling);
}

static void G_ReadActorCorpse(corpseInfo_t *corpse, MemoryFile *memFile)
{
    // zombies: SP 0x007ED167..0x007ED187.
    MemFile_ReadData(memFile, 4, (unsigned char *)&corpse->entnum);
    if (corpse->entnum == -1)
        return;
    int falling;
    MemFile_ReadData(memFile, 4, (unsigned char *)&corpse->time);
    MemFile_ReadData(memFile, 4, (unsigned char *)&falling);
    corpse->falling = falling != 0;
}

static const SPEntitySaveField s_threatBiasFields[] = {
    // SP 0x00A54050: the 16 group names (script strings).
    { 0x00, 2, 1 }, { 0x02, 2, 1 }, { 0x04, 2, 1 }, { 0x06, 2, 1 }, { 0x08, 2, 1 }, { 0x0A, 2, 1 },
    { 0x0C, 2, 1 }, { 0x0E, 2, 1 }, { 0x10, 2, 1 }, { 0x12, 2, 1 }, { 0x14, 2, 1 }, { 0x16, 2, 1 },
    { 0x18, 2, 1 }, { 0x1A, 2, 1 }, { 0x1C, 2, 1 }, { 0x1E, 2, 1 }
};
static_assert(offsetof(threat_bias_t, groupName) == 0 && sizeof(((threat_bias_t *)0)->groupName) == 0x20, "SP threat bias names");

static void G_WriteThreatBias(const threat_bias_t *bias, MemoryFile *memFile)
{
    // zombies: SP 0x007EE01C..0x007EE047: copy of g_threatBias (0x01A50AE0), WriteStruct.
    static threat_bias_t image;
    memcpy(&image, bias, sizeof(image));
    SP_WriteStruct(s_threatBiasFields, ARRAY_COUNT(s_threatBiasFields), bias, (unsigned char *)&image, sizeof(image), memFile);
}

static void G_ReadThreatBias(threat_bias_t *bias, MemoryFile *memFile)
{
    // zombies: SP 0x007ED1B1..0x007ED1E6.
    SP_ReadEntityStruct(s_threatBiasFields, ARRAY_COUNT(s_threatBiasFields), bias, sizeof(*bias), memFile);
}

static void G_WriteEventListeners(MemoryFile *memFile)
{
    // zombies: SP 0x0062EB40: the count (0x0058A320), then count x 8 bytes of 0x01A4ECB8.
    int count = g_listenerCount;
    MemFile_WriteData(memFile, 4, (unsigned char *)&count);
    if (count)
        MemFile_WriteData(memFile, 8 * count, (unsigned char *)g_AIEVlisteners);
}

static int G_ReadEventListeners(MemoryFile *memFile, AIEventListener *listeners)
{
    // zombies: SP 0x007ED1E8..0x007ED21D: count, 0x005E2D60(count) stores it, then the records.
    int count;
    MemFile_ReadData(memFile, 4, (unsigned char *)&count);
    if (count < 0 || count > ARRAY_COUNT(g_AIEVlisteners))
        Com_Error(ERR_DROP, "\x15G_LoadGame: listener count out of range (%i, MAX = %i)", count, ARRAY_COUNT(g_AIEVlisteners));
    if (count)
        MemFile_ReadData(memFile, 8 * count, (unsigned char *)listeners);
    return count;
}

static void G_WritePendingTriggers(MemoryFile *memFile)
{
    // zombies: SP 0x007EE052..0x007EE0C5: level.pendingTriggerListSize (0x01C06BC4), then the
    // pending list (0x01C053C4), 12 bytes each.
    MemFile_WriteData(memFile, 4, (unsigned char *)&level.pendingTriggerListSize);
    MemFile_WriteData(memFile, sizeof(trigger_info_t) * level.pendingTriggerListSize, (unsigned char *)level.pendingTriggerList);
}

static int G_ReadPendingTriggers(MemoryFile *memFile, trigger_info_t *list)
{
    // zombies: SP 0x007ED220..0x007ED240 (no range check in SP; KB refuses a count past the list).
    int count;
    MemFile_ReadData(memFile, 4, (unsigned char *)&count);
    if (count < 0 || count > ARRAY_COUNT(level.pendingTriggerList))
        Com_Error(ERR_DROP, "\x15G_LoadGame: trigger count out of range (%i, MAX = %i)", count, ARRAY_COUNT(level.pendingTriggerList));
    MemFile_ReadData(memFile, sizeof(trigger_info_t) * count, (unsigned char *)list);
    return count;
}

static void G_WriteWeaponCues(MemoryFile *memFile)
{
    // zombies: SP 0x007EB010: each of the 32 dropped-weapon cue handles (0x01C052AC) as its
    // handle number (entity number + 1, 0 = none).
    for (int i = 0; i < ARRAY_COUNT(level.droppedWeaponCue); ++i)
    {
        int number = level.droppedWeaponCue[i].number;
        MemFile_WriteData(memFile, 4, (unsigned char *)&number);
    }
}

static void G_ReadWeaponCues(MemoryFile *memFile, int *numbers)
{
    // zombies: SP 0x007EB060: range check, then setEnt(&g_entities[number - 1]) for non-zero
    // (0x005F3C20). The live install sets the handles; the check keeps the numbers.
    for (int i = 0; i < ARRAY_COUNT(level.droppedWeaponCue); ++i)
    {
        MemFile_ReadData(memFile, 4, (unsigned char *)&numbers[i]);
        if (numbers[i] > MAX_GENTITIES || numbers[i] < 0)
            Com_Error(ERR_DROP, "\x15G_LoadWeaponCue: entity out of range (%i)", numbers[i]);
    }
}

static void G_WriteAttractors(MemoryFile *memFile)
{
    // zombies: SP 0x0044CE30: the 32 attractors/repulsors (0x01C74838, 0x1C each): a byte
    // (0 free, 1 used), then the whole record for a used one.
    for (int i = 0; i < ARRAY_COUNT(attrGlob.attractors); ++i)
    {
        unsigned char used = attrGlob.attractors[i].inUse ? 1 : 0;
        MemFile_WriteData(memFile, 1, &used);
        if (used)
            MemFile_WriteData(memFile, sizeof(AttractorRepulsor_t), (unsigned char *)&attrGlob.attractors[i]);
    }
}

static void G_ReadAttractors(MemoryFile *memFile, AttractorRepulsor_t *attractors)
{
    // zombies: SP 0x00669AD0: clears the table (and 0x005614A0), then reads the used records.
    memset(attractors, 0, sizeof(attrGlob.attractors));
    for (int i = 0; i < ARRAY_COUNT(attrGlob.attractors); ++i)
    {
        unsigned char used;
        MemFile_ReadData(memFile, 1, &used);
        if (used)
            MemFile_ReadData(memFile, sizeof(AttractorRepulsor_t), (unsigned char *)&attractors[i]);
    }
}

static void G_SaveLevelSystems(MemoryFile *memFile)
{
    // zombies: G_SaveState SP 0x007EDD81..0x007EE0F6, in exe order.
    G_SaveCurves(memFile);                          // SP 0x007EDD82 -> 0x0065AB20
    G_SP_WriteAnimCommandRecords(memFile);          // SP 0x007EDD8A (1024 x 6-byte list nodes)
    G_SP_WriteAnimCommandList(memFile);             // SP 0x007EDDDA
    G_SP_WriteAnimCommandHeads(memFile);            // SP 0x007EDF21 -> 0x007EA760
    G_SaveVehicleInfoSounds(memFile);               // SP 0x007EDF27 -> 0x004C0740
    for (int i = 0; i < ARRAY_COUNT(turretInfoStore); ++i)
        G_WriteTurret(&level.turrets[i], memFile);  // SP 0x007EDF40 -> 0x005D9700 (SP 96, KB 32)
    for (int i = 0; i < ARRAY_COUNT(g_scr_data.actorCorpseInfo); ++i)
        G_WriteActorCorpse(&g_scr_data.actorCorpseInfo[i], memFile); // SP 0x007EDF64 (SP 32, KB 8)
    // SP 0x007EE00B / 0x007EE011 -> 0x00651A30 twice: the shared empty function.
    SP_SaveNotPorted("destructibles (SP 0x007EE017 -> 0x005D3220)");
    G_WriteThreatBias(&g_threatBias, memFile);      // SP 0x007EE047
    G_WriteEventListeners(memFile);                 // SP 0x007EE04D -> 0x0062EB40
    G_WritePendingTriggers(memFile);                // SP 0x007EE052
    G_WriteWeaponCues(memFile);                     // SP 0x007EE0CA -> 0x007EB010
    G_WriteAttractors(memFile);                     // SP 0x007EE0D0 -> 0x0044CE30
    int zero = 0;                                   // SP 0x007EE0D6 -> 0x00630DA0 writes 0
    MemFile_WriteData(memFile, 4, (unsigned char *)&zero);
    SP_SaveNotPorted("SP 0x007EB3A0 / 0x007EB1B0 / 0x007EB2B0 (server entity caches), 0x005B0D70, 0x006920B0");
}

static void G_LoadLevelSystemsForCheck(MemoryFile *memFile, MemoryFile *writer)
{
    // zombies: G_LoadGame SP 0x007ED04D..0x007ED278 into diagnostic storage, rewriting each
    // section with its writer. TEMPORARY: the live install reads into the game tables.
    static cCurve curves[4];
    G_LoadCurves(memFile, curves);
    for (int i = 0; i < 4; ++i)
        MemFile_WriteData(writer, sizeof(cCurve), (unsigned char *)&curves[i]);
    int activeCurves = 0;
    for (int i = 0; i < 4; ++i)
        activeCurves += curves[i].mActive ? 1 : 0;

    // The animation command readers write the live pool; keep the live history around them.
    G_SP_BackupAnimCommands(true);
    G_SP_ReadAnimCommandRecords(memFile);
    G_SP_WriteAnimCommandRecords(writer);
    G_SP_ReadAnimCommandList(memFile);
    G_SP_WriteAnimCommandList(writer);
    G_SP_ReadAnimCommandHeads(memFile);
    G_SP_WriteAnimCommandHeads(writer);
    int referenced, protectedCommands, snapshotTime;
    G_SP_GetAnimCommandUsage(&referenced, &protectedCommands, &snapshotTime);
    G_SP_BackupAnimCommands(false);

    static unsigned int sounds[32][19];
    G_LoadVehicleInfoSounds(memFile, sounds);
    for (int i = 0; i < bg_numVehicleInfos; ++i)
        for (int j = 0; j < 19; ++j)
            if (sounds[i][j])
                MemFile_WriteData(writer, 4, (unsigned char *)&sounds[i][j]);

    static TurretInfo turret;
    int turrets = 0;
    for (int i = 0; i < ARRAY_COUNT(turretInfoStore); ++i)
    {
        memset(&turret, 0, sizeof(turret));
        const bool inuse = G_ReadTurret(&turret, memFile);
        G_WriteTurret(&turret, writer);
        if (!inuse)
            continue;
        ++turrets;
        SP_ReleaseEntityFields(s_turretFields, ARRAY_COUNT(s_turretFields), &turret);
    }

    static corpseInfo_t corpse;
    int corpses = 0;
    for (int i = 0; i < ARRAY_COUNT(g_scr_data.actorCorpseInfo); ++i)
    {
        G_ReadActorCorpse(&corpse, memFile);
        G_WriteActorCorpse(&corpse, writer);
        corpses += corpse.entnum != -1;
    }
    // SP 0x007ED196: level.actorCorpseCount = 32 (its table size); the live install sets KB's 8.

    static threat_bias_t bias;
    G_ReadThreatBias(&bias, memFile);
    G_WriteThreatBias(&bias, writer);
    const int threatGroups = bias.threatGroupCount;
    SP_ReleaseEntityFields(s_threatBiasFields, ARRAY_COUNT(s_threatBiasFields), &bias);

    static AIEventListener listeners[32];
    const int listenerCount = G_ReadEventListeners(memFile, listeners);
    MemFile_WriteData(writer, 4, (unsigned char *)&listenerCount);
    if (listenerCount)
        MemFile_WriteData(writer, 8 * listenerCount, (unsigned char *)listeners);

    static trigger_info_t triggers[256];
    const int triggerCount = G_ReadPendingTriggers(memFile, triggers);
    MemFile_WriteData(writer, 4, (unsigned char *)&triggerCount);
    MemFile_WriteData(writer, sizeof(trigger_info_t) * triggerCount, (unsigned char *)triggers);

    int cues[32];
    G_ReadWeaponCues(memFile, cues);
    MemFile_WriteData(writer, sizeof(cues), (unsigned char *)cues);
    // SP 0x007ED24A -> 0x00685670 follows the weapon cues (reader only; not identified yet).

    static AttractorRepulsor_t attractors[32];
    G_ReadAttractors(memFile, attractors);
    int attractorCount = 0;
    for (int i = 0; i < 32; ++i)
    {
        unsigned char used = attractors[i].inUse ? 1 : 0;
        attractorCount += used;
        MemFile_WriteData(writer, 1, &used);
        if (used)
            MemFile_WriteData(writer, sizeof(AttractorRepulsor_t), (unsigned char *)&attractors[i]);
    }

    int zero;                                       // SP 0x007ED256 -> 0x00498510
    MemFile_ReadData(memFile, 4, (unsigned char *)&zero);
    MemFile_WriteData(writer, 4, (unsigned char *)&zero);

    Com_Printf(15, "loadgame: decoded level systems: %i active curves, animcmd %i referenced / %i protected, "
        "%i turrets, %i corpses, %i threat groups, %i listeners, %i pending triggers, %i attractors\n",
        activeCurves, referenced, protectedCommands, turrets, corpses, threatGroups, listenerCount,
        triggerCount, attractorCount);
}
