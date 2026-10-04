// Included by g_sp_savegame.cpp after g_sp_save_client.inl (uses the WriteStruct/ReadField port).
// zombies: actors (SP 0x007EDC3F loop, extras 0x007EA6B0, reader 0x007EC760 / 0x007EA710) and
// sentients (SP 0x007EDCCE loop, reader loop 0x007ECFD0). SP actor is 0x3240 with a 0x2254-byte
// image; KB keeps its 0x27B0 actor_s (MP layout, 16-byte aligned Physics), so the core image is KB's with a DERIVED table, the SP
// fields KB keeps in actor_sp_ext_t follow as one DERIVED extension image, then the extras.
#include <new>
#include <cfloat>
#include <game/sentient.h>

#define AF(member, type) { offsetof(actor_s, member), sizeof(((actor_s *)0)->member), type }
#define REACQUIRE(n) AF(pPotentialReacquireNode[n], 13)
#define KNOWN(n) AF(sentientInfo[n].pLastKnownNode, 13)
#define KNOWN8(n) KNOWN(n), KNOWN(n + 1), KNOWN(n + 2), KNOWN(n + 3), KNOWN(n + 4), KNOWN(n + 5), KNOWN(n + 6), KNOWN(n + 7)
static const SPEntitySaveField s_actorFields[] = {
    // SP 0x00A53B58 order. SP +0xDC8 AnimScriptHandle (11), +0xDD4 AnimScriptSpecific.name,
    // +0xDCC pAnimScriptFunc (12); SP +0x1AE8 (type 12) has no KB field.
    AF(ent, 2), AF(sentient, 6), AF(AnimScriptHandle, 11), AF(AnimScriptSpecific.name, 1), AF(pAnimScriptFunc, 12),
    REACQUIRE(0), REACQUIRE(1), REACQUIRE(2), REACQUIRE(3), REACQUIRE(4),
    REACQUIRE(5), REACQUIRE(6), REACQUIRE(7), REACQUIRE(8), REACQUIRE(9),
    // SP converts sentientInfo[0..32].pLastKnownNode (+0x1B1C step 0x28) and writes [33..35]
    // raw; the reader clears all of sentientInfo anyway. KB converts all 48.
    KNOWN8(0), KNOWN8(8), KNOWN8(16), KNOWN8(24), KNOWN8(32), KNOWN8(40),
    // SP +0x20CC.. (type 6): the sentient of each suppression record; actor_sp_ext_t keeps none.
    // SP +0xDB0/+0xDB2/+0xDB4 damagelocation/weapon/mod, +0xC96 "weapon" (KB weaponName).
    AF(damageHitLoc, 1), AF(damageWeapon, 1), AF(damageMod, 1), AF(weaponName, 1),
    AF(pFavoriteEnemy, 7), AF(pGrenade, 3), AF(GrenadeTossMethod, 1),
    AF(scriptState, 1), AF(lastScriptState, 1), AF(stateChangeReason, 1), AF(pCloseEnt, 3),
    AF(pPileUpActor, 5), AF(pPileUpEnt, 2),
    // SP +0xD06 anim_pose (string) has no KB field.
    AF(codeGoal.node, 13), AF(codeGoal.volume, 2), AF(scriptGoalEnt, 3),
    AF(scriptGoal.node, 13), AF(scriptGoal.volume, 2), AF(fixedNodeSafeVolume, 3),
    // SP +0x1A78 chainnode (13) has no KB field.
    AF(faceLikelyEnemyPathNode, 13)
    // SP +0x18..+0x30: seven int model indices (18, no-op on write) with no KB field.
};
#undef KNOWN8
#undef KNOWN
#undef REACQUIRE
#undef AF
#define XF(member, type) { offsetof(actor_sp_ext_t, member), sizeof(((actor_sp_ext_t *)0)->member), type }
static const SPEntitySaveField s_actorExtFields[] = {
    // SP +0xC94 name, +0xC98 primaryweapon, +0xC9A secondaryweapon, +0xC9C sidearm, +0x21A8 turret.
    XF(name, 1), XF(primaryWeapon, 1), XF(secondaryWeapon, 1), XF(sidearm, 1), XF(turret, 2)
};
#undef XF
// SP's image stops before pPotentialCoverNode (SP +0x2258, written by the extras) and the
// sightTraceHitNum tail (SP +0x31F8); KB's image stops at the same member.
static const unsigned SP_ACTOR_IMAGE_SIZE = offsetof(actor_s, pPotentialCoverNode);
static const unsigned SP_ACTOR_EXT_IMAGE_SIZE = offsetof(actor_sp_ext_t, sightTraceHitNum);
static_assert(sizeof(actor_s) == 0x27B0, "DERIVED actor image (KB: 16-byte aligned Physics, actor_fields.cpp)");
static_assert(ARRAY_COUNT(((actor_s *)0)->sentientInfo.inl) == 48 && ARRAY_COUNT(((actor_s *)0)->pPotentialCoverNode) == 1000,
    "DERIVED actor arrays");
static_assert(sizeof(scr_animscript_t) == 8, "SP 8-byte animscript entries");

#define SF(member, type) { offsetof(sentient_s, member), sizeof(((sentient_s *)0)->member), type }
static const SPEntitySaveField s_sentientFields[] = {
    // SP 0x00A53F78 order. SP sentient is 0x8C: KB's 0x90 without scriptOwner (KB +0x8), so
    // SP +0x34/+0x38/+0x5C/+0x60/+0x64/+0x6C/+0x2C/+0x30 are KB +4.
    SF(ent, 2), SF(targetEnt, 3), SF(scriptTargetEnt, 3), SF(pClaimedNode, 13), SF(pPrevClaimedNode, 13),
    SF(pActualChainPos, 13), SF(pNearestNode, 13), SF(lastAttacker, 3), SF(syncedMeleeEnt, 3),
    // KB-only: scriptOwner. SP writes scriptTargetTag (+0x3C) raw; KB saves the string.
    SF(scriptOwner, 3), SF(scriptTargetTag, 1)
};
#undef SF
static_assert(sizeof(sentient_s) == 0x90 && offsetof(sentient_s, inuse) == 0x84, "DERIVED sentient image");
static_assert(ARRAY_COUNT(g_sentients) == MAX_SENTIENTS_CAP, "DERIVED sentient count");

static void G_WriteActor(const actor_s *actor, const actor_sp_ext_t *ext, MemoryFile *memFile)
{
    // zombies: SP 0x007EDC50 loop body: the in-use int (SP +0x21B4), then for an actor in use
    // the image (copied to SP 0x01C74CC0) and the extras (SP 0x007EA6B0).
    MemFile_WriteData(memFile, 4, (unsigned char *)&actor->inuse);
    if (!actor->inuse)
        return;
    static actor_s image;
    memcpy(&image, actor, SP_ACTOR_IMAGE_SIZE);
    // SP writes these raw and the reader resets them (SP 0x007EC7B6..0x007EC902). KB writes no
    // process pointers: the debug string, the transient GJK input and the collision visitor.
    image.pszDebugInfo = nullptr;
    image.Physics.m_gjkcc_input = nullptr;
    memset((void *)&image.Physics.proximity_data, 0, sizeof(image.Physics.proximity_data));
    SP_WriteStruct(s_actorFields, ARRAY_COUNT(s_actorFields), actor, (unsigned char *)&image, SP_ACTOR_IMAGE_SIZE, memFile);
    SP_WriteEntityStruct(s_actorExtFields, ARRAY_COUNT(s_actorExtFields), ext, SP_ACTOR_EXT_IMAGE_SIZE, memFile);
    // SP 0x007EA6B0: every potential cover node below the count (SP +0x1AA8) as Path_SaveNode.
    for (int i = 0; i < actor->iPotentialCoverNodeCount; ++i)
    {
        int node = Path_SaveNode(actor->pPotentialCoverNode[i]);
        MemFile_WriteData(memFile, 4, (unsigned char *)&node);
    }
    // KB-only: ikPriority lives after pPotentialCoverNode in KB's actor_s.
    MemFile_WriteData(memFile, 4, (unsigned char *)&actor->ikPriority);
}

static bool G_ReadActorRecord(actor_s *actor, actor_sp_ext_t *ext, MemoryFile *memFile)
{
    // zombies: SP 0x007EC760 up to 0x007EC79C (the reads); G_ResetLoadedActor is the rest.
    MemFile_ReadData(memFile, 4, (unsigned char *)&actor->inuse);
    if (!actor->inuse)
        return false;
    SP_ReadEntityStruct(s_actorFields, ARRAY_COUNT(s_actorFields), actor, SP_ACTOR_IMAGE_SIZE, memFile); // SP 0x007EA260
    SP_ReadEntityStruct(s_actorExtFields, ARRAY_COUNT(s_actorExtFields), ext, SP_ACTOR_EXT_IMAGE_SIZE, memFile);
    // SP 0x007EA710 (no bound check in SP; KB refuses a count past the array).
    if (actor->iPotentialCoverNodeCount < 0 || actor->iPotentialCoverNodeCount > (int)ARRAY_COUNT(actor->pPotentialCoverNode))
        Com_Error(ERR_DROP, "G_LoadGame: potential cover node count out of range (%i)", actor->iPotentialCoverNodeCount);
    for (int i = 0; i < actor->iPotentialCoverNodeCount; ++i)
    {
        int node;
        MemFile_ReadData(memFile, 4, (unsigned char *)&node);
        actor->pPotentialCoverNode[i] = Path_LoadNode(node);
    }
    MemFile_ReadData(memFile, 4, (unsigned char *)&actor->ikPriority);
    return true;
}

static void G_ResetLoadedActor(actor_s *actor)
{
    // zombies: SP 0x007EC7A0..0x007EC922, after the reads of an actor in use.
    actor->pszDebugInfo = ""; // SP 0x007EC7B6: +0x2254 = "" (0x009DD354)
    // SP 0x007EC7AF..0x007EC902: the collision visitor (SP +0xF04) takes a default-constructed
    // visitor's vtable, bounds, prim count and overflow flag.
    new (&actor->Physics.proximity_data) colgeom_visitor_inlined_t<200>();
    // SP 0x007EC90A: every sentient info record is cleared (0x00566D40, 36 x 0x28 at +0x1AF8).
    memset(actor->sentientInfo.inl, 0, sizeof(actor->sentientInfo.inl));
}

static void G_ReadActor(actor_s *actor, actor_sp_ext_t *ext, MemoryFile *memFile)
{
    // zombies: SP 0x007EC760.
    if (G_ReadActorRecord(actor, ext, memFile))
        G_ResetLoadedActor(actor);
}

static void G_WriteSentient(const sentient_s *sentient, MemoryFile *memFile)
{
    // zombies: SP 0x007EDCE0 loop body: the in-use byte (SP +0x80), then the stack image.
    MemFile_WriteData(memFile, 1, (unsigned char *)&sentient->inuse);
    if (sentient->inuse)
        SP_WriteEntityStruct(s_sentientFields, ARRAY_COUNT(s_sentientFields), sentient, sizeof(*sentient), memFile);
}

static void G_ReadSentient(sentient_s *sentient, MemoryFile *memFile)
{
    // zombies: SP 0x007ECFD0 loop body: the byte, then the whole record and ReadField per field.
    MemFile_ReadData(memFile, 1, (unsigned char *)&sentient->inuse);
    if (sentient->inuse)
        SP_ReadEntityStruct(s_sentientFields, ARRAY_COUNT(s_sentientFields), sentient, sizeof(*sentient), memFile);
}

static void G_SaveActors(MemoryFile *memFile)
{
    // zombies: G_SaveState actor loop (SP 0x007EDC3F..0x007EDCCC), all 32 slots.
    for (int i = 0; i < MAX_ACTORS; ++i)
        G_WriteActor(&level.actors[i], Actor_SP_Ext(&level.actors[i]), memFile);
}

static void G_SaveSentients(MemoryFile *memFile)
{
    // zombies: G_SaveState sentient loop (SP 0x007EDCCE..0x007EDD56); SP has 36, KB 48.
    for (int i = 0; i < MAX_SENTIENTS; ++i)
        G_WriteSentient(&level.sentients[i], memFile);
}

static void G_LoadActorsForCheck(MemoryFile *memFile, MemoryFile *writer)
{
    // zombies: G_LoadGame actor loop (SP 0x007ECFA6 -> 0x007EC760) into diagnostic storage.
    // TEMPORARY: the live loop reads into level.actors and the side table.
    static actor_s actor;
    static actor_sp_ext_t ext;
    int used = 0, coverNodes = 0;
    for (int i = 0; i < MAX_ACTORS; ++i)
    {
        // Rewrite before the reader's resets: SP clears sentientInfo, which the saved bytes keep.
        const bool inuse = G_ReadActorRecord(&actor, &ext, memFile);
        G_WriteActor(&actor, &ext, writer);
        if (!inuse)
            continue;
        G_ResetLoadedActor(&actor);
        ++used;
        coverNodes += actor.iPotentialCoverNodeCount;
        SP_ReleaseEntityFields(s_actorFields, ARRAY_COUNT(s_actorFields), &actor);
        SP_ReleaseEntityFields(s_actorExtFields, ARRAY_COUNT(s_actorExtFields), &ext);
    }
    Com_Printf(15, "loadgame: decoded %i actors (%i in use, %i potential cover nodes)\n", MAX_ACTORS, used, coverNodes);
}

static void G_LoadSentientsForCheck(MemoryFile *memFile, MemoryFile *writer)
{
    // zombies: G_LoadGame sentient loop (SP 0x007ECFD0..0x007ED024) into diagnostic storage.
    static sentient_s sentient;
    int used = 0;
    for (int i = 0; i < MAX_SENTIENTS; ++i)
    {
        G_ReadSentient(&sentient, memFile);
        G_WriteSentient(&sentient, writer);
        if (!sentient.inuse)
            continue;
        ++used;
        SP_ReleaseEntityFields(s_sentientFields, ARRAY_COUNT(s_sentientFields), &sentient);
    }
    Com_Printf(15, "loadgame: decoded %i sentients (%i in use)\n", MAX_SENTIENTS, used);
}
