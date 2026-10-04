// Included by g_sp_savegame.cpp after g_sp_save_entity.inl (uses its WriteStruct/ReadField port).
// zombies: bad places (SP 0x007EA960 / 0x007EAA40), g_badplaces[256] (SP 0x01A4C088).
// KB badplace_t has SP's 0x2C layout, so the DERIVED tables keep SP's offsets.
extern badplace_t g_badplaces[256]; // game/actor_badplace.cpp (SP 0x01A4C088)
static const SPEntitySaveField s_badPlaceFields[] = {
    { offsetof(badplace_t, name), sizeof(((badplace_t *)0)->name), 1 } // SP 0x00A54008
};
static const SPEntitySaveField s_badPlaceBrushFields[] = {
    { offsetof(badplace_parms_t, brush.volume), sizeof(((badplace_parms_t *)0)->brush.volume), 2 } // SP 0x00A5402C
};
static_assert(sizeof(badplace_t) == 0x2C && sizeof(badplace_parms_t) == 0x1C, "DERIVED bad place image");
static_assert(offsetof(badplace_t, name) == 8 && offsetof(badplace_t, type) == 0xA
    && offsetof(badplace_t, parms) == 0x10, "DERIVED bad place fields");
static_assert(ARRAY_COUNT(g_badplaces) == 256, "SP bad place count");

// SP 0x00A54020 is the empty table used for every parms type except brush (2).
static const SPEntitySaveField *G_BadPlaceParmsFields(const badplace_t *place, unsigned *count)
{
    if (place->type == 2)
    {
        *count = ARRAY_COUNT(s_badPlaceBrushFields);
        return s_badPlaceBrushFields;
    }
    *count = 0;
    return nullptr;
}

static void G_WriteBadPlaces(const badplace_t *places, MemoryFile *memFile)
{
    // zombies: SP 0x007EA960. Header image 0x10 (name fixed), then the 0x1C parms image.
    for (unsigned i = 0; i < ARRAY_COUNT(g_badplaces); ++i)
    {
        const badplace_t *place = &places[i];
        SP_WriteEntityStruct(s_badPlaceFields, ARRAY_COUNT(s_badPlaceFields), place, offsetof(badplace_t, parms), memFile);
        unsigned count;
        const SPEntitySaveField *fields = G_BadPlaceParmsFields(place, &count);
        SP_WriteEntityStruct(fields, count, &place->parms, sizeof(place->parms), memFile);
    }
}

static void G_ReadBadPlace(badplace_t *place, MemoryFile *memFile)
{
    // zombies: one record of SP 0x007EAA40. The parms table is chosen from the restored type byte.
    SP_ReadEntityStruct(s_badPlaceFields, ARRAY_COUNT(s_badPlaceFields), place, offsetof(badplace_t, parms), memFile);
    unsigned count;
    const SPEntitySaveField *fields = G_BadPlaceParmsFields(place, &count);
    SP_ReadEntityStruct(fields, count, &place->parms, sizeof(place->parms), memFile);
}

static void G_ReadBadPlaces(MemoryFile *memFile)
{
    // zombies: SP 0x007EAA40. Live reader: entity geometry must already be restored, because a
    // brush record's count update reads its volume entity (SP 0x00445650).
    for (unsigned i = 0; i < ARRAY_COUNT(g_badplaces); ++i)
    {
        G_ReadBadPlace(&g_badplaces[i], memFile);
        if (g_badplaces[i].type)
            Path_UpdateBadPlaceCount(&g_badplaces[i], 1);
    }
}

static void G_ReleaseBadPlace(badplace_t *place)
{
    // TEMPORARY diagnostic cleanup: the name is the only registered reference in a record.
    Scr_SetString(&place->name, 0, SCRIPTINSTANCE_SERVER);
}
