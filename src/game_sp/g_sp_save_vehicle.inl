// Included by g_sp_savegame.cpp after g_sp_save_actor.inl (uses the WriteStruct/ReadField port).
// zombies: script vehicles (SP 0x007EDD58 loop -> 0x007ED410, reader loop 0x007ED030 -> 0x007EC930).
// SP scr_vehicle_s is 0xE10 with 64 slots; KB keeps its 0xE60 MP struct (0xE2C in the MP exe; KB aligns the visitor to 16) with 16 (s_vehicles), so the
// image is KB's with a DERIVED table.
#include <game/g_vehicle_path.h>
#include <vehicle/nitrous_vehicle.h>

extern vehicle_custom_path_t gCustomPaths[10];

#define VF(member, type) { offsetof(scr_vehicle_s, member), sizeof(((scr_vehicle_s *)0)->member), type }
#define NODE(n) VF(pathPos.switchNode[n].name, 1), VF(pathPos.switchNode[n].target, 1), \
    VF(pathPos.switchNode[n].script_linkname, 1), VF(pathPos.switchNode[n].script_noteworthy, 1)
static const SPEntitySaveField s_vehicleFields[] = {
    // SP 0x00A54150 order: switch node strings (SP +0x3C/+0x3E/+0x42/+0x44 and +0x80..), lookAtText0/1
    // (SP +0x3F0/+0x3F2), lookAtEnt (SP +0x430).
    NODE(0), NODE(1), VF(lookAtText0, 1), VF(lookAtText1, 1), VF(lookAtEnt, 3),
    // KB-only: SP skips node +0x4 (target2); KB's node owns a reference there (VP_ClearNode).
    VF(pathPos.switchNode[0].target2, 1), VF(pathPos.switchNode[1].target2, 1)
};
#undef NODE
#undef VF
static_assert(sizeof(scr_vehicle_s) == 0xE60, "DERIVED vehicle image (KB: 16-byte aligned collision visitor)");
static_assert(sizeof(vehicle_node_t) == 0x44 && offsetof(vehicle_pathpos_t, switchNode) == 0x3C, "SP vehicle node layout");
static const int SP_VEHICLE_COUNT = 16; // KB s_vehicles (SP 64)

static void G_WriteVehicle(const scr_vehicle_s *vehicle, MemoryFile *memFile)
{
    // zombies: SP 0x007ED410: in use = entNum (SP +0x1E8) is not ENTITYNUM_NONE; then the image
    // (copy-constructed, 0x005BE020) and the vehicle info name of infoIdx (SP +0x1EC, 0x007E9CD0).
    int inuse = vehicle->entNum != ENTITYNUM_NONE;
    MemFile_WriteData(memFile, 4, (unsigned char *)&inuse);
    if (!inuse)
        return;
    static scr_vehicle_s image;
    memcpy(&image, vehicle, sizeof(image));
    // SP writes these raw and the reader resets the last two (SP 0x007EC989, 0x007ECA97..0x007ECB36).
    // KB writes no process pointers: the custom path as a one-based gCustomPaths index (its
    // contents are not in this payload), the nitrous state and the collision visitor.
    const vehicle_custom_path_t *path = vehicle->pathPos.customPath;
    if (path && (path < gCustomPaths || path >= gCustomPaths + ARRAY_COUNT(gCustomPaths)))
        Com_Error(ERR_DROP, "G_SaveGame: vehicle custom path out of range");
    *(int *)&image.pathPos.customPath = path ? path - gCustomPaths + 1 : 0;
    image.nitrousVehicle = nullptr;
    memset((void *)&image.vehicle_cache.proximity_data, 0, sizeof(image.vehicle_cache.proximity_data));
    SP_WriteStruct(s_vehicleFields, ARRAY_COUNT(s_vehicleFields), vehicle, (unsigned char *)&image, sizeof(image), memFile);
    // Same byte-length name format as the weapon names (SP 0x007E9CD0 / 0x007E9D50).
    G_SaveWeaponName(memFile, BG_GetVehicleInfo(vehicle->infoIdx)->name);
}

static bool G_ReadVehicleRecord(scr_vehicle_s *vehicle, MemoryFile *memFile)
{
    // zombies: SP 0x007EC930 up to 0x007EC97D (the reads); G_ResetLoadedVehicle is the rest.
    int inuse;
    MemFile_ReadData(memFile, 4, (unsigned char *)&inuse);
    if (!inuse)
        return false;
    SP_ReadEntityStruct(s_vehicleFields, ARRAY_COUNT(s_vehicleFields), vehicle, sizeof(*vehicle), memFile); // SP 0x007EA260
    const int path = *(int *)&vehicle->pathPos.customPath;
    if (path < 0 || path > (int)ARRAY_COUNT(gCustomPaths))
        Com_Error(ERR_DROP, "G_LoadGame: vehicle custom path out of range (%i)", path);
    vehicle->pathPos.customPath = path ? &gCustomPaths[path - 1] : nullptr;
    char name[256];
    G_LoadWeaponName(memFile, name);
    vehicle->infoIdx = (__int16)VEH_GetVehicleInfoFromName(name); // SP 0x007E9D50 -> 0x0040FCD0
    return true;
}

static void G_ResetLoadedVehicle(scr_vehicle_s *vehicle)
{
    // zombies: SP 0x007EC984..0x007ECB36.
    vehicle->nitrousVehicle = nullptr; // SP +0x680
    vehicle_cache_t *cache = &vehicle->vehicle_cache;
    new (&cache->proximity_data) colgeom_visitor_inlined_t<200>(); // SP +0x750, as for actors
    for (int i = 0; i < 3; ++i)
    {
        cache->lastOrigin[i] = FLT_MAX; // SP 0x00A470F4
        cache->lastAngles[i] = FLT_MAX;
    }
    for (int i = 0; i < 6; ++i)
        cache->hit_indices[i] = -1;
}

static void G_ReadVehicle(scr_vehicle_s *vehicle, MemoryFile *memFile)
{
    // zombies: SP 0x007EC930.
    if (G_ReadVehicleRecord(vehicle, memFile))
        G_ResetLoadedVehicle(vehicle);
}

static void G_SaveVehicles(MemoryFile *memFile)
{
    // zombies: G_SaveState vehicle loop (SP 0x007EDD58..0x007EDD7F).
    for (int i = 0; i < SP_VEHICLE_COUNT; ++i)
        G_WriteVehicle(&level.vehicles[i], memFile);
}

static void G_LoadVehiclesForCheck(MemoryFile *memFile, MemoryFile *writer)
{
    // zombies: G_LoadGame vehicle loop (SP 0x007ED030..0x007ED04B) into diagnostic storage.
    static scr_vehicle_s vehicle;
    int used = 0;
    for (int i = 0; i < SP_VEHICLE_COUNT; ++i)
    {
        vehicle.entNum = ENTITYNUM_NONE;
        const bool inuse = G_ReadVehicleRecord(&vehicle, memFile);
        G_WriteVehicle(&vehicle, writer);
        if (!inuse)
            continue;
        ++used;
        G_ResetLoadedVehicle(&vehicle);
        SP_ReleaseEntityFields(s_vehicleFields, ARRAY_COUNT(s_vehicleFields), &vehicle);
    }
    Com_Printf(15, "loadgame: decoded %i vehicles (%i in use)\n", SP_VEHICLE_COUNT, used);
}
