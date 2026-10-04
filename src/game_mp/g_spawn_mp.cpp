#include "g_spawn_mp.h"
#include <game/g_load_utils.h>
#include <clientscript/cscr_vm.h>
#include "g_utils_mp.h"
#include <clientscript/cscr_stringlist.h>
#include <server_mp/sv_init_mp.h>
#include <game/pathnode_load_obj.h>
#include "actor_mp.h"
#include <clientscript/scr_const.h>
#include <game/g_mover.h>
#include "g_trigger_mp.h"
#include "g_misc_mp.h"
#include "game/turret.h"
#include <game/g_scr_mover.h>
#include <bgame/bg_misc.h>
#include <game/g_weapon.h>
#include <game/g_client_fields.h>
#include <cgame/cg_hudelem.h>
#include <bgame/bg_weapons_def.h>
#include <game_sp/g_sp_ext.h>
#include <game/g_scr_vehicle.h>
#include <bgame/bg_misctables.h>
#include <qcommon/common.h>
#include <game/actor_zombie_exposed.h>
#include <server/sv_game.h>
#include <server/sv_world.h>

struct ent_field_t // sizeof=0x14
{                                       // XREF: .rdata:fields_1/r
    const char *name;
    int ofs;
    int size[1];
    fieldtype_t type;
    void (__cdecl *callback)(gentity_s *, int);
};

static void __cdecl Scr_SetSpEntityField(gentity_s *ent, int offset);
static void __cdecl Scr_GetSpEntityField(gentity_s *ent, int offset);

// zombies: SP_info_volume (SP 0x0051e5a0). A non-solid, unsent brush volume: the zombie zone manager's
// player_volume brushes (maps\_zombiemode_zone_manager::player_in_zone, IsTouching).
// SP entity +0x4 s.lerp.eFlags (|= 1), +0x100 r.contents, +0xda r.svFlags (16-bit store of 1).
static void __cdecl SP_info_volume_SP(gentity_s *ent, SpawnVar *)
{
    if ( !SV_SetBrushModel(ent) )
    {
        Com_PrintError(
            1,
            "Killing info_volume at (%f %f %f) because the brush model is invalid.\n",
            ent->s.lerp.pos.trBase[0],
            ent->s.lerp.pos.trBase[1],
            ent->s.lerp.pos.trBase[2]);
        G_FreeEntity(ent);
        return;
    }
    ent->s.lerp.eFlags |= 1u;
    ent->r.contents = 0;
    ent->r.svFlags = 1;
    SV_LinkEntity(ent);
}

enum
{
    ENTITY_FIELD_MP_COUNT = 15,
    ENTITY_FIELD_SP_EXT_BEGIN = 19,
    ENTITY_FIELD_SP_VEH_BEGIN = 22
};

static void __cdecl Scr_SetSpVehicleTeam(gentity_s *ent, int offset);

const ent_field_t fields_1[] =
{
  { "classname", 356, { 2 }, F_STRING, Scr_ReadOnlyField },
  { "origin", 292, { 12 }, F_VECTOR, Scr_SetOrigin },
  { "model", 348, { 2 }, F_MODEL, Scr_ReadOnlyField },
  { "spawnflags", 368, { 4 }, F_INT, Scr_ReadOnlyField },
  { "target", 358, { 2 }, F_STRING, NULL },
  { "targetname", 360, { 2 }, F_STRING, NULL },
  { "script_noteworthy", 362, { 2 }, F_STRING, NULL },
  { "count", 532, { 4 }, F_INT, NULL },
  { "health", 404, { 4 }, F_INT, Scr_SetHealth },
  { "dmg", 412, { 4 }, F_INT, NULL },
  { "angles", 304, { 12 }, F_VECTOR, Scr_SetAngles },
  { "birthtime", 748, { 4 }, F_INT, Scr_ReadOnlyField },
  { "index", 560, { 4 }, F_INT, Scr_SetExposureIndex },
  { "lerp_to_lighter", 564, { 4 }, F_FLOAT, Scr_SetExposureLerpToLighter },
  { "lerp_to_darker", 568, { 4 }, F_FLOAT, Scr_SetExposureLerpToDarker },
  // zombies: SP +0x188 -> gentity_s::maxHealth (SP 0x00a5576c).
  { "maxhealth", offsetof(gentity_s, maxHealth), { 4 }, F_INT, NULL },
  // zombies: SP +0x157 -> gentity_s::takedamage (SP 0x00a557a8).
  { "takedamage", offsetof(gentity_s, takedamage), { 1 }, F_BYTE, NULL },
  // zombies: SP +0xe8 / +0xf4 -> r.mins / r.maxs; read-only (SP 0x00a557bc / 0x00a557d0).
  { "mins", offsetof(gentity_s, r.mins), { 12 }, F_VECTOR, Scr_ReadOnlyField },
  { "maxs", offsetof(gentity_s, r.maxs), { 12 }, F_VECTOR, Scr_ReadOnlyField },
  // zombies: SP +0x15e -> gentitySpExt::script_linkname (SP 0x00a55744).
  { "script_linkname", offsetof(gentitySpExt, script_linkname), { 2 }, F_STRING, Scr_SetSpEntityField },
  // zombies: SP +0x328 -> gentitySpExt::anglelerprate (SP 0x00a55780).
  { "anglelerprate", offsetof(gentitySpExt, anglelerprate), { 4 }, F_FLOAT, Scr_SetSpEntityField },
  // zombies: SP +0x210 -> gentitySpExt::activator; read-only (SP 0x00a55794).
  { "activator", offsetof(gentitySpExt, activator), { 4 }, F_ENTITY, Scr_ReadOnlyField },
  // zombies: SP vehicle field class (table SP 0x00a55ed8, 56 rows, registered by SP 0x0062b440; get SP 0x00448690,
  // set SP 0x0058c4d0 on ent->scr_vehicle). Only the rows the zombie scripts read (_vehicle.gsc / _utility.gsc on
  // Call of the Dead's zipline vehicles). ofs = the vehicle_info_t offset; not map keys, not copied (G_IsEntityFieldStored).
  { "vehicleclass", 64, { 2 }, F_INT, Scr_ReadOnlyField },
  { "vehicletype", 0, { 64 }, F_INT, Scr_ReadOnlyField },
  { "healthdefault", 7176, { 4 }, F_INT, Scr_ReadOnlyField },
  { "healthmin", 7180, { 4 }, F_INT, Scr_ReadOnlyField },
  { "healthmax", 7184, { 4 }, F_INT, Scr_ReadOnlyField },
  { "vteam", 0, { 4 }, F_INT, Scr_SetSpVehicleTeam },
  // zombies: SP row 53 isphysicsvehicle = the def's isNitrous (SP def +0x1c90, KB +7328: MP has 4 more button keys).
  { "isphysicsvehicle", 7328, { 4 }, F_INT, Scr_ReadOnlyField },
  { NULL, 0, { 0 }, F_INT, NULL }
};

static bool G_IsEntityFieldEnabled(const ent_field_t *field)
{
    // zombies: G_InitFields has no native index row (SP 0x004d5d70, table 0x00a55668).
    // Omit registration so the VM allocates a normal script variable. The same
    // filter keeps map parsing and entity copying away from the exposure union.
    // The SP rows are registered on every SP level (Com_IsSPLevel: zombiemode or the SP front end).
    if ( Com_IsSPLevel() && field->callback == Scr_SetExposureIndex )
        return false;
    return field - fields_1 < ENTITY_FIELD_MP_COUNT || Com_IsSPLevel();
}

// zombies: rows that live in the entity or its SP side table (map keys, G_CopyEntity); not the vehicle rows.
static bool G_IsEntityFieldStored(const ent_field_t *field)
{
    return G_IsEntityFieldEnabled(field) && field - fields_1 < ENTITY_FIELD_SP_VEH_BEGIN;
}

// The original row layout has no getter. Keep its layout and field IDs intact;
// the trailing side-table rows share this custom getter and storage resolver.
static const unsigned __int8 *G_EntityFieldAddress(const gentity_s *ent, const ent_field_t *field)
{
    if ( field - fields_1 >= ENTITY_FIELD_SP_EXT_BEGIN )
        return (const unsigned __int8 *)&G_EntSpExt(ent) + field->ofs;
    return (const unsigned __int8 *)ent + field->ofs;
}

static unsigned __int8 *G_EntityFieldAddress(gentity_s *ent, const ent_field_t *field)
{
    return const_cast<unsigned __int8 *>(G_EntityFieldAddress((const gentity_s *)ent, field));
}

static void __cdecl Scr_SetSpEntityField(gentity_s *ent, int offset)
{
    const ent_field_t *field = &fields_1[offset];
    GScr_SetGenericField((unsigned __int8 *)&G_EntSpExt(ent), field->type, field->ofs, 0);
}

// zombies: vteam setter (SP 0x00579e90): vehicles only; "axis" 1, "allies" 2, "none" 0, anything else is an error.
static void __cdecl Scr_SetSpVehicleTeam(gentity_s *ent, int offset)
{
    scr_vehicle_s *veh = ent->scr_vehicle;
    if ( !veh )
        return;
    const char *team = Scr_GetString(0, SCRIPTINSTANCE_SERVER);
    if ( !I_stricmp(team, "axis") )
        veh->team = 1;
    else if ( !I_stricmp(team, "allies") )
        veh->team = 2;
    else if ( !I_stricmp(team, "none") )
        veh->team = 0;
    else
        Scr_Error("vehicle.team=: invalid team used must be 'axis', 'allies', or 'none'\n", 0);
}

// zombies: vehicle field getters (SP 0x00448690 dispatch): not a vehicle -> undefined. vehicleclass SP 0x006505d0
// (class name of the def's type short at +0x40), vteam SP 0x00487c10, the def rows SP 0x005f8330 (generic read).
static void __cdecl Scr_GetSpVehicleField(gentity_s *ent, int offset)
{
    scr_vehicle_s *veh = ent->scr_vehicle;
    if ( !veh )
        return;
    const ent_field_t *field = &fields_1[offset];
    const unsigned __int8 *info = (const unsigned __int8 *)BG_GetVehicleInfo(veh->infoIdx);
    switch ( offset - ENTITY_FIELD_SP_VEH_BEGIN )
    {
        case 0:
            Scr_AddString((char *)s_vehicleClassNames[*(const __int16 *)(info + field->ofs)], SCRIPTINSTANCE_SERVER);
            break;
        case 1:
            Scr_AddString((char *)info + field->ofs, SCRIPTINSTANCE_SERVER);
            break;
        case 5:
            if ( veh->team == 1 )
                Scr_AddConstString(scr_const.axis, SCRIPTINSTANCE_SERVER);
            else if ( veh->team == 2 )
                Scr_AddConstString(scr_const.allies, SCRIPTINSTANCE_SERVER);
            else if ( veh->team == 3 )
                Scr_AddString((char *)"neutral", SCRIPTINSTANCE_SERVER);
            else
                Scr_AddConstString(scr_const.none, SCRIPTINSTANCE_SERVER);
            break;
        default:
            Scr_AddInt(*(const int *)(info + field->ofs), SCRIPTINSTANCE_SERVER);
            break;
    }
}

static void __cdecl Scr_GetSpEntityField(gentity_s *ent, int offset)
{
    const ent_field_t *field = &fields_1[offset];
    // SP's raw activator pointer is represented by a lifetime-tracked KB handle.
    const fieldtype_t type = field->type == F_ENTITY ? F_ENTHANDLE : field->type;
    GScr_GetGenericField((unsigned __int8 *)&G_EntSpExt(ent), type, field->ofs, 0);
}

const SpawnFuncEntry s_bspOnlySpawns[15] =
{
  { "trigger_use", trigger_use_touch },
  { "trigger_use_touch", trigger_use_touch },
  { "trigger_multiple", SP_trigger_multiple },
  { "trigger_disk", SP_trigger_disk },
  { "trigger_hurt", SP_trigger_hurt },
  { "trigger_once", SP_trigger_once },
  { "trigger_damage", SP_trigger_damage },
  { "trigger_lookat", SP_trigger_lookat },
  { "trigger_ik_playerclip_terrain", SP_trigger_ik_playerclip_terrain },
  { "light", SP_light },
  { "misc_mg42", SP_turret },
  { "misc_turret", SP_turret },
  { "script_brushmodel", SP_script_brushmodel },
  { "script_struct", G_FreeEntityWrapper },
  { "script_vehicle", SP_script_vehicle }
};

const SpawnFuncEntry s_bspOrDynamicSpawns[7] =
{
  { "info_notnull", SP_info_notnull },
  { "info_notnull_big", SP_info_notnull },
  { "trigger_radius", SP_trigger_radius },
  { "trigger_radius_use", SP_trigger_radius_use },
  { "script_model", SP_script_model },
  { "script_origin", SP_script_origin },
  { "script_vehicle_collmap", SP_script_vehicle_collmap }
};




int __cdecl G_SpawnFloat(const SpawnVar *spawnVar, const char *key, const char *defaultString, float *out)
{
    int present; // [esp+0h] [ebp-8h]
    const char *s; // [esp+4h] [ebp-4h] BYREF

    present = G_SpawnString(spawnVar, key, defaultString, &s);
    *out = atof(s);
    return present;
}

int __cdecl G_SpawnInt(const SpawnVar *spawnVar, const char *key, const char *defaultString, int *out)
{
    int present; // [esp+0h] [ebp-8h]
    const char *s; // [esp+4h] [ebp-4h] BYREF

    present = G_SpawnString(spawnVar, key, defaultString, &s);
    *out = atoi(s);
    return present;
}

void __cdecl Scr_ReadOnlyField(gentity_s *ent, int)
{
    Scr_Error("Tried to set a read only entity field", 0);
}

void __cdecl G_FreeEntityWrapper(gentity_s *ent, SpawnVar *v)
{
    G_FreeEntity(ent);
}

unsigned int __cdecl G_SetEntityScriptVariableInternal(const char *key, char *value)
{
    int v3; // eax
    const char *v4; // eax
    float v5; // [esp+8h] [ebp-1Ch]
    unsigned int index; // [esp+10h] [ebp-14h]
    int type; // [esp+14h] [ebp-10h] BYREF
    float vec[3]; // [esp+18h] [ebp-Ch] BYREF

    index = Scr_FindField(key, &type, SCRIPTINSTANCE_SERVER);
    if ( !index )
        return 0;
    switch ( type )
    {
        case 2:
            Scr_AddString(value, SCRIPTINSTANCE_SERVER);
            break;
        case 4:
            memset(vec, 0, sizeof(vec));
            sscanf(value, "%f %f %f", vec, &vec[1], &vec[2]);
            Scr_AddVector(vec, SCRIPTINSTANCE_SERVER);
            break;
        case 5:
            v5 = atof(value);
            Scr_AddFloat(v5, SCRIPTINSTANCE_SERVER);
            break;
        case 6:
            v3 = atoi(value);
            Scr_AddInt(v3, SCRIPTINSTANCE_SERVER);
            break;
        default:
            v4 = va("G_SetEntityScriptVariableInternal: bad case %d", type);
            if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawn_mp.cpp", 210, 0, v4) )
                __debugbreak();
            break;
    }
    return index;
}

void __cdecl G_ParseEntityFields(const SpawnVar *spawnVar, gentity_s *ent, int radiant_update, int ignoreModel)
{
    int i; // [esp+0h] [ebp-4h]

    if ( !spawnVar->spawnVarsValid
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawn_mp.cpp",
                    316,
                    0,
                    "%s",
                    "spawnVar.spawnVarsValid") )
    {
        __debugbreak();
    }
    for ( i = 0; i < spawnVar->numSpawnVars; ++i )
        G_ParseEntityField(spawnVar->spawnVars[i][0], spawnVar->spawnVars[i][1], ent, radiant_update, ignoreModel);
    G_SetOrigin(ent, ent->r.currentOrigin);
    G_SetAngle(ent, ent->r.currentAngles);
}

void __cdecl G_ParseEntityField(const char *key, char *value, gentity_s *ent, int radiant_update, int ignoreModel)
{
    float v4; // [esp+0h] [ebp-20h]
    const ent_field_t *f; // [esp+Ch] [ebp-14h]
    float vec[3]; // [esp+10h] [ebp-10h] BYREF
    int modelIndex; // [esp+1Ch] [ebp-4h]

    for ( f = fields_1; ; ++f )
    {
        if ( !f->name )
        {
            G_SetEntityScriptVariable(key, value, ent);
            return;
        }
        if ( G_IsEntityFieldStored(f) && !I_stricmp(f->name, key) )
            break;
    }
    switch ( f->type )
    {
        case F_BYTE:
            *G_EntityFieldAddress(ent, f) = (unsigned __int8)atoi(value);
            break;
        case F_INT:
            *(int *)((char *)G_EntityFieldAddress(ent, f)) = atoi(value);
            break;
        case F_FLOAT:
            v4 = atof(value);
            *(float *)((char *)G_EntityFieldAddress(ent, f)) = v4;
            break;
        case F_STRING:
            Scr_SetStringLiveUpdateSafe((unsigned __int16 *)((char *)G_EntityFieldAddress(ent, f)), value, SCRIPTINSTANCE_SERVER);
            break;
        case F_VECTOR:
            memset(vec, 0, sizeof(vec));
            sscanf(value, "%f %f %f", vec, &vec[1], &vec[2]);
            *(float *)((char *)G_EntityFieldAddress(ent, f)) = vec[0];
            *(float *)((char *)G_EntityFieldAddress(ent, f) + 4) = vec[1];
            *(float *)((char *)G_EntityFieldAddress(ent, f) + 8) = vec[2];
            break;
        case F_MODEL:
            // zombies: SP skips the model key when ignoreModel is set (SP 0x008128c2), brush models included
            if ( ignoreModel )
                break;
            if ( *value == 42 )
            {
                modelIndex = atoi(value + 1);
                if ( modelIndex != (unsigned __int16)modelIndex
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawn_mp.cpp",
                                282,
                                0,
                                "%s",
                                "modelIndex == (modelNameIndex_t)modelIndex") )
                {
                    __debugbreak();
                }
                ent->s.index.brushmodel = modelIndex;
            }
            else
            {
                if ( Com_IsLegacyXModelName(value) )
                    value += 7;
                if ( radiant_update )
                    G_SetModelIfLoaded(ent, value);
                else
                    G_SetModel(ent, value);
            }
            break;
        default:
            return;
    }
}

void __cdecl G_SetEntityScriptVariable(const char *key, char *value, gentity_s *ent)
{
    unsigned int index; // [esp+0h] [ebp-4h]

    index = G_SetEntityScriptVariableInternal(key, value);
    if ( index )
        GScr_SetDynamicEntityField(ent, index);
}

unsigned int __cdecl G_SpawnStruct(SpawnVar *spawnVar)
{
    unsigned int index; // [esp+0h] [ebp-10h]
    int i; // [esp+4h] [ebp-Ch]
    const char *scriptLabel; // [esp+8h] [ebp-8h] BYREF
    unsigned int structId; // [esp+Ch] [ebp-4h]

    if ( !spawnVar->spawnVarsValid
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawn_mp.cpp",
                    339,
                    0,
                    "%s",
                    "spawnVar.spawnVarsValid") )
    {
        __debugbreak();
    }
    G_SpawnString(spawnVar, "script_label", "", &scriptLabel);
    if ( !I_stricmp(scriptLabel, "random") || !I_stricmp(scriptLabel, "looper") || !I_stricmp(scriptLabel, "line_emitter") )
        return 0;
    if ( !g_scr_data.createstruct
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawn_mp.cpp",
                    348,
                    0,
                    "%s",
                    "g_scr_data.createstruct") )
    {
        __debugbreak();
    }
    Scr_AddExecThread(SCRIPTINSTANCE_SERVER, g_scr_data.createstruct, 0);
    structId = Scr_GetObject(0, SCRIPTINSTANCE_SERVER);
    for ( i = 0; i < spawnVar->numSpawnVars; ++i )
    {
        index = G_SetEntityScriptVariableInternal(spawnVar->spawnVars[i][0], spawnVar->spawnVars[i][1]);
        if ( index )
            Scr_SetStructField(structId, index, SCRIPTINSTANCE_SERVER);
    }
    return structId;
}

void __cdecl G_DuplicateEntityFields(gentity_s *dest, const gentity_s *source)
{
    float *v2; // [esp+8h] [ebp-14h]
    float *v3; // [esp+Ch] [ebp-10h]
    const ent_field_t *f; // [esp+14h] [ebp-8h]

    for ( f = fields_1; f->name; ++f )
    {
        if ( !G_IsEntityFieldStored(f) )
            continue;
        switch ( f->type )
        {
            // zombies: byte copy in G_CopyEntityFields (SP 0x0068f900).
            case F_BYTE:
                *G_EntityFieldAddress(dest, f) = *G_EntityFieldAddress(source, f);
                break;
            case F_INT:
                *(int *)((char *)G_EntityFieldAddress(dest, f)) = *(int *)((const char *)G_EntityFieldAddress(source, f));
                break;
            case F_FLOAT:
                *(int *)((char *)G_EntityFieldAddress(dest, f)) = *(int *)((const char *)G_EntityFieldAddress(source, f));
                break;
            case F_STRING:
                Scr_SetString(
                    (unsigned __int16 *)((char *)G_EntityFieldAddress(dest, f)),
                    *(unsigned __int16 *)((const char *)G_EntityFieldAddress(source, f)),
                    SCRIPTINSTANCE_SERVER);
                break;
            case F_VECTOR:
                v2 = (float *)((char *)G_EntityFieldAddress(dest, f));
                v3 = (float *)((const char *)G_EntityFieldAddress(source, f));
                *v2 = *v3;
                v2[1] = v3[1];
                v2[2] = v3[2];
                break;
            case F_MODEL:
                *(_WORD *)((char *)G_EntityFieldAddress(dest, f)) = *(_WORD *)((const char *)G_EntityFieldAddress(source, f));
                break;
            default:
                continue;
        }
    }
}

void __cdecl G_DuplicateScriptFields(gentity_s *dest, const gentity_s *source)
{
    if ( dest->s.number != dest - g_entities
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawn_mp.cpp",
                    412,
                    0,
                    "%s",
                    "dest->s.number == dest - g_entities") )
    {
        __debugbreak();
    }
    if ( source->s.number != source - g_entities
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawn_mp.cpp",
                    413,
                    0,
                    "%s",
                    "source->s.number == source - g_entities") )
    {
        __debugbreak();
    }
    Scr_CopyEntityNum(source->s.number, dest->s.number, 0, SCRIPTINSTANCE_SERVER);
}

bool __cdecl IsClientOnlyEntity(const SpawnVar *spawnVar)
{
    bool result; // al
    char *out; // [esp+28h] [ebp-24h] BYREF
    const char *modelName; // [esp+2Ch] [ebp-20h] BYREF
    const char *clientSide; // [esp+30h] [ebp-1Ch] BYREF
    DestructibleDef *ddef; // [esp+34h] [ebp-18h]
    const char *destructibledefname; // [esp+38h] [ebp-14h] BYREF
    bool bClientSide; // [esp+3Fh] [ebp-Dh]
    const char *classname; // [esp+40h] [ebp-Ch] BYREF
    unsigned int flags; // [esp+44h] [ebp-8h]
    const char *spawnFlags; // [esp+48h] [ebp-4h] BYREF

    G_SpawnString(spawnVar, "classname", "", &classname);
    G_SpawnString(spawnVar, "spawnflags", "", &spawnFlags);
    flags = atoi(spawnFlags);
    bClientSide = (flags & 2) != 0;
    if ( strcmp("script_model", classname) )
        goto LABEL_15;
    G_SpawnString(spawnVar, "destructibledef", 0, &destructibledefname);
    if ( destructibledefname )
    {
        ddef = DestructibleDefPrecache(destructibledefname);
        if ( ddef && ddef->clientOnly )
            return 1;
        goto LABEL_15;
    }
    if ( (clientSide = 0, G_SpawnString(spawnVar, "clientside", 0, &clientSide), !bClientSide) && !clientSide
        || !bClientSide && atof(clientSide) != 1.0 )
    {
LABEL_15:
        result = !strcmp("script_origin", classname)
                    && ((out = 0, G_SpawnString(spawnVar, "clientside", 0, (const char **)&out), bClientSide) || out)
                    && (bClientSide || atof(out) == 1.0);
        return result;
    }
    G_SpawnString(spawnVar, "model", 0, &modelName);
    if ( modelName && *modelName )
        G_ModelIndex((char *)modelName);
    return 1;
}

void __cdecl G_SetupDestructible(gentity_s *ent, char *destructibleName)
{
    char szConfigString[1024]; // [esp+0h] [ebp-408h] BYREF
    int iDestID; // [esp+404h] [ebp-4h]

    if ( !ent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawn_mp.cpp", 533, 0, "%s", "ent") )
        __debugbreak();
    if ( !destructibleName
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawn_mp.cpp", 534, 0, "%s", "destructibleName") )
    {
        __debugbreak();
    }
    for ( iDestID = 0; iDestID < 64; ++iDestID )
    {
        SV_GetConfigstring(iDestID + 3180, szConfigString, 1024);
        if ( !szConfigString[0] )
        {
            SV_SetConfigstring(iDestID + 3180, destructibleName);
            break;
        }
        if ( !I_stricmp(szConfigString, destructibleName) )
            break;
    }
    if ( iDestID >= 64
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawn_mp.cpp",
                    552,
                    0,
                    "%s",
                    "iDestID < MAX_DESTRUCTIBLE_DEFS") )
    {
        __debugbreak();
    }
    AssignToSmallerType<unsigned char>(&ent->s.un1.scale, iDestID);
    ent->s.lerp.eFlags |= 0x20000u;
}

void __cdecl G_SetupDestructible(gentity_s *ent, SpawnVar *spawnVar)
{
    const char *destructibleName; // [esp+0h] [ebp-4h] BYREF

    destructibleName = 0;
    G_SpawnString(spawnVar, "destructibledef", 0, &destructibleName);
    if ( destructibleName )
        G_SetupDestructible(ent, (char *)destructibleName);
}

// mod (L25): with bo1_mod_maxactors > 32, map triggers (not trigger_damage: bullets and traces report its number),
// info_volume and actor spawners take server-only numbers 1024+ so actors keep the networked 0..1021.
static gentity_s *G_ModSpawnForMap(SpawnVar *spawnVar, const char *classname)
{
    extern int g_maxActors;
    int spawnflags = 0;

    if ( g_maxActors > 32 && level.spawnVar.spawnVarsValid )
    {
        if ( !strncmp(classname, "actor_", 6u) )
            G_SpawnInt(spawnVar, "spawnflags", "0", &spawnflags);
        if ( (!strncmp(classname, "trigger_", 8u) && strcmp(classname, "trigger_damage"))
            || !strcmp(classname, "info_volume") || (spawnflags & 1) != 0
            // mod (L43q): stream_info has no spawn callback, model or link operation. Keep these map metadata
            // entities in the server-only range too (Kino has 32); scripts can still find their fields.
            || (g_maxActors >= 900 && !strcmp(classname, "stream_info")) )
            return G_SpawnHigh();
    }
    return G_Spawn();
}

void __cdecl G_CallSpawn(SpawnVar *spawnVar)
{
    const gitem_s *item; // [esp+8Ch] [ebp-14h]
    void (__cdecl *spawnFunc)(gentity_s *, SpawnVar *); // [esp+90h] [ebp-10h]
    const char *classname; // [esp+94h] [ebp-Ch] BYREF
    gentity_s *ent; // [esp+98h] [ebp-8h]
    int radiantLiveUpdate; // [esp+9Ch] [ebp-4h]

    if ( !spawnVar->spawnVarsValid
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawn_mp.cpp",
                    584,
                    1,
                    "%s",
                    "spawnVar->spawnVarsValid") )
    {
        __debugbreak();
    }
    radiantLiveUpdate = !level.spawnVar.spawnVarsValid;
    G_SpawnString(spawnVar, "classname", "", &classname);
    if ( classname )
    {
        if ( strncmp(classname, "dyn_", 4u) )
        {
            if ( !strncmp(classname, "node_", 5u) )
            {
                if ( !useFastFile->current.enabled )
                    G_SpawnPathnodeStatic(spawnVar, classname);
                G_SpawnPathnodeDynamic(spawnVar);
            }
            else if ( !strncmp(classname, "actor_", 6u) )
            {
                ent = G_ModSpawnForMap(spawnVar, classname);
                // zombies: SP spawns actor_ spawners with ignoreModel = 1 (SP 0x00517705..0x00517711), so their
                // "model" key (Kino: char_ger_honorgd_fullbody, in no zone) is never loaded
                G_ParseEntityFields(spawnVar, ent, radiantLiveUpdate, 1);
                SP_actor(ent, spawnVar);
            }
            else if ( !strcmp("info_vehicle_node", classname) )
            {
                SP_info_vehicle_node(spawnVar, 0, 0);
            }
            else if ( !strcmp("info_vehicle_node_rotate", classname) )
            {
                SP_info_vehicle_node(spawnVar, 1, 0);
            }
            else
            {
                item = G_GetItemForClassname(classname);
                if ( item )
                {
                    ent = G_Spawn();
                    G_ParseEntityFields(spawnVar, ent, radiantLiveUpdate);
                    G_SpawnItem(ent, item);
                }
                else if ( !IsClientOnlyEntity(spawnVar)
                             && strcmp("occlusion_override", classname)
                             && strcmp("undertow_volume", classname)
                             && strcmp("rope", classname) )
                {
                    if ( !strcmp("heli_height_lock", classname) )
                    {
                        G_SpawnHeliHeightLock(spawnVar);
                    }
                    else if ( strcmp("glass", classname) )
                    {
                        spawnFunc = G_FindSpawnFunc(classname, s_bspOrDynamicSpawns, 7);
                        if ( !spawnFunc && level.spawnVar.spawnVarsValid )
                            spawnFunc = G_FindSpawnFunc(classname, s_bspOnlySpawns, 15);
                        if ( (char *)spawnFunc != (char *)G_FreeEntityWrapper )
                        {
                            ent = G_ModSpawnForMap(spawnVar, classname);
                            G_ParseEntityFields(spawnVar, ent, radiantLiveUpdate);
                            G_SetupDestructible(ent, spawnVar);
                            if ( spawnFunc )
                                spawnFunc(ent, spawnVar);
                        }
                    }
                }
            }
        }
    }
    else
    {
        Com_Printf(15, "G_CallSpawn: NULL classname\n");
    }
}

const gitem_s *__cdecl G_GetItemForClassname(const char *classname)
{
    unsigned int weapIndex; // [esp+0h] [ebp-8h]

    if ( g_connectpaths->current.integer )
        return 0;
    if ( strncmp(classname, "weapon_", 7u) )
        return 0;
    weapIndex = G_GetWeaponIndexForName((char *)classname + 7);
    if ( !weapIndex )
        return 0;
    BG_GetWeaponDef(weapIndex);
    return BG_FindItemForWeapon(weapIndex);
}

void (__cdecl *__cdecl G_FindSpawnFunc(
                const char *classname,
                const SpawnFuncEntry *spawnFuncArray,
                int spawnFuncCount))(gentity_s *, SpawnVar *)
{
    int spawnFuncIter; // [esp+14h] [ebp-4h]

    // zombies: map-only info_grenade_hint row (SP 0x00a557f8 -> 0x00441ff0).
    if ( zombiemode && zombiemode->current.enabled && spawnFuncArray == s_bspOnlySpawns
        && !strcmp(classname, "info_grenade_hint") )
        return SP_info_grenade_hint_SP;
    // zombies: SP's bsp-or-dynamic table has an info_volume row (SP 0x00a55890 -> 0x0051e5a0); KB's MP
    // table does not, so the zone manager's player_volume brushes were plain unlinked entities.
    if ( zombiemode && zombiemode->current.enabled && spawnFuncArray == s_bspOrDynamicSpawns
        && !strcmp(classname, "info_volume") )
        return SP_info_volume_SP;
    // zombies: the same SP table has a trigger_damage row (SP 0x00a558b0 -> SP_trigger_damage 0x004b8bf0),
    // which the freezegun's shatter trigger spawns.
    if ( zombiemode && zombiemode->current.enabled && spawnFuncArray == s_bspOrDynamicSpawns
        && !strcmp(classname, "trigger_damage") )
        return SP_trigger_damage;

    for ( spawnFuncIter = 0; spawnFuncIter < spawnFuncCount; ++spawnFuncIter )
    {
        if ( !strcmp(classname, spawnFuncArray[spawnFuncIter].classname) )
            return spawnFuncArray[spawnFuncIter].callback;
    }
    return 0;
}

int __cdecl G_CallSpawnEntity(gentity_s *ent)
{
    const gitem_s *item; // [esp+0h] [ebp-Ch]
    void (__cdecl *spawnFunc)(gentity_s *); // [esp+4h] [ebp-8h]
    char *classname; // [esp+8h] [ebp-4h]

    if ( level.spawnVar.spawnVarsValid
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawn_mp.cpp",
                    705,
                    1,
                    "%s",
                    "!level.spawnVar.spawnVarsValid") )
    {
        __debugbreak();
    }
    if ( ent->classname )
    {
        classname = SL_ConvertToString(ent->classname, SCRIPTINSTANCE_SERVER);
        item = G_GetItemForClassname(classname);
        if ( item )
        {
            G_SpawnItem(ent, item);
            return 1;
        }
        else
        {
            spawnFunc = (void (__cdecl *)(gentity_s *))G_FindSpawnFunc(classname, s_bspOrDynamicSpawns, 7);
            if ( spawnFunc )
            {
                //if ( spawnFunc == G_FreeEntityWrapper
                //    && !Assert_MyHandler(
                //                "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawn_mp.cpp",
                //                729,
                //                0,
                //                "%s",
                //                "spawnFunc != G_FreeEntityWrapper") )
                //{
                //    __debugbreak();
                //}
                ((void (__cdecl *)(gentity_s *, unsigned int))spawnFunc)(ent, 0);
                if ( !ent->r.inuse
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawn_mp.cpp",
                                731,
                                0,
                                "%s",
                                "ent->r.inuse") )
                {
                    __debugbreak();
                }
                return 1;
            }
            else
            {
                Com_Printf(15, "%s cannot be spawned dynamically\n", classname);
                return 0;
            }
        }
    }
    else
    {
        Com_Printf(15, "G_CallSpawnEntity: NULL classname\n");
        return 0;
    }
}

void __cdecl GScr_AddFieldsForEntity()
{
    const ent_field_t *f; // [esp+4h] [ebp-4h]

    for ( f = fields_1; f->name; ++f )
    {
        if ( !G_IsEntityFieldEnabled(f) )
            continue;
        if ( ((f - fields_1) & 0xC000) != 0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawn_mp.cpp",
                        747,
                        0,
                        "%s",
                        "((f - fields) & ENTFIELD_MASK) == ENTFIELD_ENTITY") )
        {
            __debugbreak();
        }
        if ( f - fields_1 != (unsigned __int16)(f - fields_1)
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawn_mp.cpp",
                        748,
                        0,
                        "%s",
                        "(f - fields) == (unsigned short)( f - fields )") )
        {
            __debugbreak();
        }
        Scr_AddClassField(0, (char *)f->name, (unsigned __int16)(f - fields_1), SCRIPTINSTANCE_SERVER);
    }
    GScr_AddFieldsForActor();
    GScr_AddFieldsForSentient();
    GScr_AddFieldsForClient();
}

void __cdecl GScr_AddFieldsForRadiant()
{
    Scr_AddFields("radiant", (char*)"txt", SCRIPTINSTANCE_SERVER);
}

void __cdecl GScr_SetGenericField(unsigned __int8 *b, fieldtype_t type, int ofs, unsigned int whichbits)
{
    gentity_s *ent; // [esp+8h] [ebp-4h]
    gentity_s *enta; // [esp+8h] [ebp-4h]
    gentity_s *entb; // [esp+8h] [ebp-4h]

    switch ( type )
    {
        case F_ACTOR:
            ent = Scr_GetEntityAllowNull(0, SCRIPTINSTANCE_SERVER);
            if ( ent )
                *(unsigned int *)&b[ofs] = (unsigned int)ent->actor;
            else
                *(unsigned int *)&b[ofs] = 0;
            break;
        case F_SENTIENT:
            enta = Scr_GetEntityAllowNull(0, SCRIPTINSTANCE_SERVER);
            if ( enta )
                *(unsigned int *)&b[ofs] = (unsigned int)enta->sentient;
            else
                *(unsigned int *)&b[ofs] = 0;
            break;
        case F_SENTIENTHANDLE:
            entb = Scr_GetEntityAllowNull(0, SCRIPTINSTANCE_SERVER);
            if ( entb )
                ((SentientHandle *)&b[ofs])->setSentient(entb->sentient);
            else
                ((SentientHandle *)&b[ofs])->setSentient(0);
            break;
        case F_PATHNODE:
            *(unsigned int *)&b[ofs] = (unsigned int)Scr_GetPathnode(0, SCRIPTINSTANCE_SERVER);
            break;
        default:
            Scr_SetGenericField(b, type, ofs, SCRIPTINSTANCE_SERVER, whichbits);
            break;
    }
}

int __cdecl Scr_SetObjectField(unsigned int classnum, unsigned int entnum, unsigned int offset)
{
    const char *v4; // eax
    const char *v5; // eax

    switch ( classnum )
    {
        case 0u:
            return Scr_SetEntityField(entnum, offset);
        case 1u:
            Scr_SetHudElemField(entnum, offset);
            return 1;
        case 2u:
            Scr_SetPathnodeField(entnum, offset);
            return 1;
        case 3u:
            v4 = va("vehicle node is read-only");
            Scr_Error(v4, 0);
            return 1;
        default:
            v5 = va("bad class num %u", classnum);
            if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawn_mp.cpp", 894, 0, v5) )
                __debugbreak();
            return 1;
    }
}

int __cdecl Scr_SetEntityField(unsigned int entnum, unsigned int offset)
{
    const ent_field_t *f; // [esp+0h] [ebp-8h]
    gentity_s *ent; // [esp+4h] [ebp-4h]

    if ( entnum >= MAX_GENTITIES_SV
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawn_mp.cpp",
                    781,
                    0,
                    "%s",
                    "(unsigned)entnum < MAX_GENTITIES") )
    {
        __debugbreak();
    }
    ent = &g_entities[entnum];
    if ( !ent->r.inuse
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawn_mp.cpp",
                    784,
                    0,
                    "%s\n\t(ent->s.number) = %i",
                    "(ent->r.inuse)",
                    ent->s.number) )
    {
        __debugbreak();
    }
    switch ( offset & 0xC000 )
    {
        case 0x8000u:
            if ( ent->actor )
            {
                Scr_SetActorField(ent->actor, offset & 0xFFFF3FFF);
                return 1;
            }
            else
            {
                return 0;
            }
        case 0x4000u:
            if ( ent->sentient )
            {
                Scr_SetSentientField(ent->sentient, offset & 0xFFFF3FFF);
                return 1;
            }
            else
            {
                return 0;
            }
        case 0xC000u:
            if ( ent->client )
            {
                Scr_SetClientField(ent->client, offset & 0xFFFF3FFF);
                return 1;
            }
            else
            {
                return 0;
            }
        default:
            if ( offset >= ARRAY_COUNT(fields_1) - 1
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawn_mp.cpp",
                            815,
                            0,
                            "%s",
                            "(unsigned)offset < ARRAY_COUNT( fields ) - 1") )
            {
                __debugbreak();
            }
            // zombies: SP's vehicle class (0x8000) is dispatched like the actor class: not a vehicle -> 0, the VM keeps a
            // script variable (_vehicle.gsc sets .vehicletype on script_models). Rows 0-1/46-48 are read-only in SP too.
            if ( offset >= ENTITY_FIELD_SP_VEH_BEGIN && !ent->scr_vehicle )
                return 0;
            f = &fields_1[offset];
            if ( f->callback )
                f->callback(ent, offset);
            else
                GScr_SetGenericField((unsigned __int8 *)ent, f->type, f->ofs, 0);
            return 1;
    }
}

void __cdecl Scr_GetEntityField(unsigned int entnum, unsigned int offset)
{
    gentity_s *ent; // [esp+4h] [ebp-4h]

    if ( entnum >= MAX_GENTITIES_SV
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawn_mp.cpp",
                    911,
                    0,
                    "%s",
                    "(unsigned)entnum < MAX_GENTITIES") )
    {
        __debugbreak();
    }
    ent = &g_entities[entnum];
    if ( !ent->r.inuse
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawn_mp.cpp",
                    914,
                    0,
                    "%s\n\t(ent->s.number) = %i",
                    "(ent->r.inuse)",
                    ent->s.number) )
    {
        __debugbreak();
    }
    switch ( offset & 0xC000 )
    {
        case 0x8000u:
            if ( ent->actor )
                Scr_GetActorField(ent->actor, offset & 0xFFFF3FFF);
            break;
        case 0x4000u:
            if ( ent->sentient )
                Scr_GetSentientField(ent->sentient, offset & 0xFFFF3FFF);
            break;
        case 0xC000u:
            if ( ent->client )
                Scr_GetClientField(ent->client, offset & 0xFFFF3FFF);
            break;
        default:
            if ( offset >= ARRAY_COUNT(fields_1) - 1
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawn_mp.cpp",
                            942,
                            0,
                            "%s",
                            "(unsigned)offset < ARRAY_COUNT( fields ) - 1") )
            {
                __debugbreak();
            }
            if ( offset >= ENTITY_FIELD_SP_VEH_BEGIN )
                Scr_GetSpVehicleField(ent, offset);
            else if ( offset >= ENTITY_FIELD_SP_EXT_BEGIN )
                Scr_GetSpEntityField(ent, offset);
            else
                GScr_GetGenericField((unsigned __int8 *)ent, fields_1[offset].type, fields_1[offset].ofs, 0);
            break;
    }
}

void __cdecl GScr_GetGenericField(unsigned __int8 *b, fieldtype_t type, int ofs, unsigned int whichbits)
{
    gentity_s *v4; // eax
    sentient_s *v5; // eax
    unsigned int v6; // eax
    unsigned int id; // [esp+4h] [ebp-4h]

    switch ( type )
    {
        case F_ENTITY:
            if ( *(unsigned int *)&b[ofs] )
                Scr_AddEntity(*(gentity_s **)&b[ofs], SCRIPTINSTANCE_SERVER);
            break;
        case F_ENTHANDLE:
            if ( ((EntHandle *)&b[ofs])->isDefined() )
            {
                v4 = ((EntHandle *)&b[ofs])->ent();
                Scr_AddEntity(v4, SCRIPTINSTANCE_SERVER);
            }
            break;
        case F_ACTOR:
            if ( *(unsigned int *)&b[ofs] )
                Scr_AddEntity(**(gentity_s ***)&b[ofs], SCRIPTINSTANCE_SERVER);
            break;
        case F_SENTIENT:
            if ( *(unsigned int *)&b[ofs] )
                Scr_AddEntity(**(gentity_s ***)&b[ofs], SCRIPTINSTANCE_SERVER);
            break;
        case F_SENTIENTHANDLE:
            if ( ((SentientHandle *)&b[ofs])->isDefined() )
            {
                v5 = ((SentientHandle *)&b[ofs])->sentient();
                Scr_AddEntity(v5->ent, SCRIPTINSTANCE_SERVER);
            }
            break;
        case F_PATHNODE:
            if ( *(unsigned int *)&b[ofs] )
                Scr_AddPathnode(*(pathnode_t **)&b[ofs]);
            break;
        case F_OBJECT:
            id = *(unsigned int *)&b[ofs];
            if ( id )
                Scr_AddObject(id, SCRIPTINSTANCE_SERVER);
            break;
        case F_MODEL:
            v6 = G_ModelName(b[ofs]);
            Scr_AddConstString(v6, SCRIPTINSTANCE_SERVER);
            break;
        default:
            Scr_GetGenericField(b, type, ofs, SCRIPTINSTANCE_SERVER, whichbits);
            break;
    }
}

void __cdecl Scr_GetObjectField(unsigned int classnum, unsigned int entnum, unsigned int offset)
{
    const char *v3; // eax

    switch ( classnum )
    {
        case 0u:
            Scr_GetEntityField(entnum, offset);
            break;
        case 1u:
            Scr_GetHudElemField(entnum, offset);
            break;
        case 2u:
            Scr_GetPathnodeField(entnum, offset);
            break;
        case 3u:
            GScr_GetVehicleNodeField(entnum, offset);
            break;
        default:
            v3 = va("bad class num %u", classnum);
            if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawn_mp.cpp", 1023, 0, v3) )
                __debugbreak();
            break;
    }
}

void __cdecl Scr_FreeEntityConstStrings(gentity_s *pEnt)
{
    const ent_field_t *f; // [esp+4h] [ebp-8h]
    int i; // [esp+8h] [ebp-4h]

    for ( f = fields_1; f->name; ++f )
    {
        if ( f->type == F_STRING )
            Scr_SetString((unsigned __int16 *)((char *)G_EntityFieldAddress(pEnt, f)), 0, SCRIPTINSTANCE_SERVER);
    }
    for ( i = 0; i < 19; ++i )
    {
        pEnt->attachModelNames[i] = 0;
        Scr_SetString(&pEnt->attachTagNames[i], 0, SCRIPTINSTANCE_SERVER);
    }
}

void __cdecl Scr_FreeEntity(gentity_s *ent)
{
    if ( !ent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawn_mp.cpp", 1065, 0, "%s", "ent") )
        __debugbreak();
    if ( ent->s.number != ent - g_entities
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawn_mp.cpp",
                    1066,
                    0,
                    "ent->s.number == ent - g_entities\n\t%i, %i",
                    ent->s.number,
                    ent - g_entities) )
    {
        __debugbreak();
    }
    if ( !ent->r.inuse
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawn_mp.cpp",
                    1067,
                    0,
                    "%s\n\t(ent->s.number) = %i",
                    "(ent->r.inuse)",
                    ent->s.number) )
    {
        __debugbreak();
    }
    Scr_FreeEntityConstStrings(ent);
    Scr_FreeEntityNum(ent->s.number, 0, SCRIPTINSTANCE_SERVER);
}

void __cdecl Scr_AddEntity(gentity_s *ent, scriptInstance_t inst)
{
    if ( !ent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawn_mp.cpp", 1076, 0, "%s", "ent") )
        __debugbreak();
    if ( ent->s.number != ent - g_entities
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawn_mp.cpp",
                    1077,
                    0,
                    "ent->s.number == ent - g_entities\n\t%i, %i",
                    ent->s.number,
                    ent - g_entities) )
    {
        __debugbreak();
    }
    if ( !ent->r.inuse
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawn_mp.cpp",
                    1078,
                    0,
                    "%s\n\t(ent->s.number) = %i",
                    "(ent->r.inuse)",
                    ent->s.number) )
    {
        __debugbreak();
    }
    Scr_AddEntityNum(ent->s.number, 0, inst, 0);
}

gentity_s *__cdecl Scr_GetEntityAllowNull(unsigned int index, scriptInstance_t inst)
{
    scr_entref_t v3; // [esp+0h] [ebp-1Ch] BYREF
    scr_entref_t v4; // [esp+Ah] [ebp-12h]
    scr_entref_t entref; // [esp+10h] [ebp-Ch]

    if ( !Scr_GetType(index, inst) )
        return 0;
    //v4 = *Scr_GetEntityRef(&v3, index, inst);
    v4 = Scr_GetEntityRef(index, inst);
    entref = v4;
    if ( v4.classnum )
        return 0;
    if ( entref.entnum >= MAX_GENTITIES_SV
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawn_mp.cpp",
                    1101,
                    0,
                    "%s",
                    "entref.entnum < MAX_GENTITIES") )
    {
        __debugbreak();
    }
    return &g_entities[entref.entnum];
}

gentity_s *__cdecl Scr_GetEntity(unsigned int index)
{
    scr_entref_t v2; // [esp+0h] [ebp-18h] BYREF
    scr_entref_t v3; // [esp+Ah] [ebp-Eh]
    scr_entref_t entref; // [esp+10h] [ebp-8h]

    //v3 = *Scr_GetEntityRef(&v2, index, SCRIPTINSTANCE_SERVER);
    v3 = Scr_GetEntityRef(index, SCRIPTINSTANCE_SERVER);
    entref = v3;
    if ( v3.classnum )
    {
        Scr_ParamError(index, "not an entity", SCRIPTINSTANCE_SERVER);
        return 0;
    }
    else
    {
        if ( entref.entnum >= MAX_GENTITIES_SV
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawn_mp.cpp",
                        1120,
                        0,
                        "%s",
                        "entref.entnum < MAX_GENTITIES") )
        {
            __debugbreak();
        }
        return &g_entities[entref.entnum];
    }
}

void __cdecl Scr_FreeHudElem(game_hudelem_s *hud)
{
    if ( !hud && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawn_mp.cpp", 1137, 0, "%s", "hud") )
        __debugbreak();
    if ( (unsigned int)(hud - g_hudelems) >= 0x400
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawn_mp.cpp",
                    1138,
                    0,
                    "hud - g_hudelems doesn't index MAX_HUDELEMS_TOTAL\n\t%i not in [0, %i)",
                    hud - g_hudelems,
                    1024) )
    {
        __debugbreak();
    }
    if ( !hud->elem.type
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawn_mp.cpp",
                    1139,
                    0,
                    "%s",
                    "hud->elem.type != HE_TYPE_FREE") )
    {
        __debugbreak();
    }
    Scr_NotifyNum(hud - g_hudelems, 1u, scr_const.death, 0);
    Scr_FreeHudElemConstStrings(hud);
    Scr_FreeEntityNum(hud - g_hudelems, 1u, SCRIPTINSTANCE_SERVER);
}

void __cdecl Scr_AddHudElem(game_hudelem_s *hud)
{
    if ( !hud && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawn_mp.cpp", 1158, 0, "%s", "hud") )
        __debugbreak();
    if ( (unsigned int)(hud - g_hudelems) >= 0x400
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawn_mp.cpp",
                    1159,
                    0,
                    "hud - g_hudelems doesn't index MAX_HUDELEMS_TOTAL\n\t%i not in [0, %i)",
                    hud - g_hudelems,
                    1024) )
    {
        __debugbreak();
    }
    if ( !hud->elem.type
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawn_mp.cpp",
                    1160,
                    0,
                    "%s",
                    "hud->elem.type != HE_TYPE_FREE") )
    {
        __debugbreak();
    }
    Scr_AddEntityNum(hud - g_hudelems, 1u, SCRIPTINSTANCE_SERVER, 0);
}

unsigned int __cdecl Scr_ExecEntThread(gentity_s *ent, int handle, unsigned int paramcount)
{
    if ( !ent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawn_mp.cpp", 1193, 0, "%s", "ent") )
        __debugbreak();
    if ( ent->s.number != ent - g_entities
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawn_mp.cpp",
                    1194,
                    0,
                    "ent->s.number == ent - g_entities\n\t%i, %i",
                    ent->s.number,
                    ent - g_entities) )
    {
        __debugbreak();
    }
    if ( !ent->r.inuse
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawn_mp.cpp",
                    1195,
                    0,
                    "%s\n\t(ent->s.number) = %i",
                    "(ent->r.inuse)",
                    ent->s.number) )
    {
        __debugbreak();
    }
    return Scr_ExecEntThreadNum(SCRIPTINSTANCE_SERVER, ent->s.number, 0, handle, paramcount, 0);
}

void __cdecl Scr_Notify(gentity_s *ent, unsigned __int16 stringValue, unsigned int paramcount)
{
    char *v3; // eax
    const char *v4; // eax

    if ( !ent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawn_mp.cpp", 1223, 0, "%s", "ent") )
        __debugbreak();
    if ( ent->s.number != ent - g_entities )
    {
        v3 = SL_ConvertToString(stringValue, SCRIPTINSTANCE_SERVER);
        v4 = va("s.number = %i, index = %i, stringValue = %s", ent->s.number, ent - g_entities, v3);
        if ( !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawn_mp.cpp",
                        1224,
                        0,
                        "%s\n\t%s",
                        "ent->s.number == ent - g_entities",
                        v4) )
            __debugbreak();
    }
    if ( !ent->r.inuse
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawn_mp.cpp",
                    1225,
                    0,
                    "%s\n\t(ent->s.number) = %i",
                    "(ent->r.inuse)",
                    ent->s.number) )
    {
        __debugbreak();
    }
    Scr_NotifyNum(ent->s.number, 0, stringValue, paramcount);
}

void __cdecl Scr_GetEnt()
{
    gentity_s *result; // [esp+0h] [ebp-24h]
    unsigned __int16 name; // [esp+8h] [ebp-1Ch]
    int offset; // [esp+Ch] [ebp-18h]
    char *key; // [esp+10h] [ebp-14h]
    gentity_s *ent; // [esp+18h] [ebp-Ch]
    int i; // [esp+1Ch] [ebp-8h]
    unsigned __int16 value; // [esp+20h] [ebp-4h]

    name = (unsigned __int16)Scr_GetConstString(0, SCRIPTINSTANCE_SERVER);
    key = Scr_GetString(1u, SCRIPTINSTANCE_SERVER);
    offset = Scr_GetOffset(0, key, SCRIPTINSTANCE_SERVER);
    if ( offset >= 0 )
    {
        if ( offset >= ARRAY_COUNT(fields_1) - 1
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawn_mp.cpp",
                        1255,
                        0,
                        "%s",
                        "offset >= 0 && offset < static_cast<int>( ARRAY_COUNT( fields ) - 1 )") )
        {
            __debugbreak();
        }
        if ( fields_1[offset].type == F_STRING )
        {
            result = 0;
            i = 0;
            ent = g_entities;
            while ( i < G_EntEnd() )
            {
                if ( ent->r.inuse )
                {
                    value = *(_WORD *)((char *)G_EntityFieldAddress(ent, &fields_1[offset]));
                    if ( value )
                    {
                        if ( value == name )
                        {
                            if ( result )
                                Scr_Error("getent used with more than one entity", 0);
                            result = ent;
                        }
                    }
                }
                i = G_EntNext(i);
                ent = &g_entities[i];
            }
            if ( result )
                Scr_AddEntity(result, SCRIPTINSTANCE_SERVER);
        }
    }
}

void __cdecl Scr_GetEntArray()
{
    unsigned __int16 name; // [esp+4h] [ebp-1Ch]
    int offset; // [esp+8h] [ebp-18h]
    char *key; // [esp+Ch] [ebp-14h]
    gentity_s *ent; // [esp+14h] [ebp-Ch]
    gentity_s *enta; // [esp+14h] [ebp-Ch]
    int i; // [esp+18h] [ebp-8h]
    int ia; // [esp+18h] [ebp-8h]
    unsigned __int16 value; // [esp+1Ch] [ebp-4h]

    if ( Scr_GetNumParam(SCRIPTINSTANCE_SERVER) )
    {
        name = (unsigned __int16)Scr_GetConstString(0, SCRIPTINSTANCE_SERVER);
        key = Scr_GetString(1u, SCRIPTINSTANCE_SERVER);
        offset = Scr_GetOffset(0, key, SCRIPTINSTANCE_SERVER);
        if ( offset >= 0 )
        {
            if ( offset >= ARRAY_COUNT(fields_1) - 1
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawn_mp.cpp",
                            1321,
                            0,
                            "%s",
                            "offset >= 0 && offset < static_cast<int>( ARRAY_COUNT( fields ) - 1 )") )
            {
                __debugbreak();
            }
            if ( fields_1[offset].type == F_STRING )
            {
                Scr_MakeArray(SCRIPTINSTANCE_SERVER);
                ia = 0;
                enta = g_entities;
                while ( ia < G_EntEnd() )
                {
                    if ( enta->r.inuse )
                    {
                        value = *(_WORD *)((char *)G_EntityFieldAddress(enta, &fields_1[offset]));
                        if ( value )
                        {
                            if ( value == name )
                            {
                                Scr_AddEntity(enta, SCRIPTINSTANCE_SERVER);
                                Scr_AddArray(SCRIPTINSTANCE_SERVER);
                            }
                        }
                    }
                    ia = G_EntNext(ia);
                    enta = &g_entities[ia];
                }
            }
        }
    }
    else
    {
        Scr_MakeArray(SCRIPTINSTANCE_SERVER);
        i = 0;
        ent = g_entities;
        while ( i < G_EntEnd() )
        {
            if ( ent->r.inuse )
            {
                Scr_AddEntity(ent, SCRIPTINSTANCE_SERVER);
                Scr_AddArray(SCRIPTINSTANCE_SERVER);
            }
            i = G_EntNext(i);
            ent = &g_entities[i];
        }
    }
}

void __cdecl GScr_SetDynamicEntityField(gentity_s *ent, unsigned int index)
{
    Scr_SetDynamicEntityField(ent->s.number, 0, index, SCRIPTINSTANCE_SERVER, 0);
}

void __cdecl SP_worldspawn(SpawnVar *spawnVar)
{
    char *v1; // eax
    float v2; // [esp+4h] [ebp-14h]
    float value; // [esp+8h] [ebp-10h]
    float v4; // [esp+10h] [ebp-8h]
    const char *s; // [esp+14h] [ebp-4h] BYREF

    G_SpawnString(spawnVar, "classname", "", &s);
    if ( I_stricmp(s, "worldspawn") )
        Com_Error(ERR_DROP, "SP_worldspawn: The first entity isn't 'worldspawn'");
    SV_SetConfigstring(2, (char*)"cod");
    G_SpawnString(spawnVar, "ambienttrack", "", &s);
    if ( *s )
    {
        v1 = va("n\\%s", s);
        SV_SetConfigstring(1547, v1);
    }
    else
    {
        SV_SetConfigstring(1547, (char *)"");
    }
    G_SpawnString(spawnVar, "message", "", &s);
    SV_SetConfigstring(4, (char *)s);
    SV_SetConfigstring(11, (char *)g_motd->current.integer);
    G_SpawnString(spawnVar, "gravity", "800", &s);
    if ( !bg_gravity
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawn_mp.cpp", 1381, 0, "%s", "bg_gravity") )
    {
        __debugbreak();
    }
    value = atof(s);
    Dvar_SetFloat((dvar_s *)bg_gravity, value);
    G_SpawnString(spawnVar, "northyaw", "", &s);
    if ( *s )
    {
        SV_SetConfigstring(1548, (char *)s);
        v2 = atof(s);
        v4 = v2 * 0.017453292;
        level.compassNorth[0] = cos(v4);
        level.compassNorth[1] = sin(v4);
    }
    else
    {
        SV_SetConfigstring(1548, (char*)"0");
        level.compassNorth[0] = 1.0f;
        level.compassNorth[1] = 0.0f;
    }
    G_SpawnString(spawnVar, "spawnflags", "0", &s);
    g_entities[1022].spawnflags = atoi(s);
    g_entities[1022].s.number = 1022;
    Scr_SetString(&g_entities[1022].classname, scr_const.worldspawn, SCRIPTINSTANCE_SERVER);
    g_entities[1022].r.inuse = 1;
    if ( g_entities[1022].r.ownerNum.isDefined() )
    {
        if ( !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawn_mp.cpp",
                        1403,
                        0,
                        "%s",
                        "!g_entities[ENTITYNUM_WORLD].r.ownerNum.isDefined()") )
            __debugbreak();
    }
}

void __cdecl G_SpawnEntitiesFromString()
{
    if ( !G_ParseSpawnVars(&level.spawnVar) )
        Com_Error(ERR_DROP, "SpawnEntities: no entities");
    SP_worldspawn(&level.spawnVar);
    while ( G_ParseSpawnVars(&level.spawnVar) )
        G_CallSpawn(&level.spawnVar);
    G_ResetEntityParsePoint();
}

void __cdecl G_LoadStructs()
{
    unsigned int hThread; // [esp+14h] [ebp-8h]
    const char *classname; // [esp+18h] [ebp-4h] BYREF

    if ( !g_scr_data.initstructs
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_spawn_mp.cpp",
                    1443,
                    0,
                    "%s",
                    "g_scr_data.initstructs") )
    {
        __debugbreak();
    }
    hThread = Scr_ExecThread(SCRIPTINSTANCE_SERVER, g_scr_data.initstructs, 0);
    Scr_FreeThread(hThread, SCRIPTINSTANCE_SERVER);
    while ( G_ParseSpawnVars(&level.spawnVar) )
    {
        G_SpawnString(&level.spawnVar, "classname", "", &classname);
        if ( !strcmp("script_struct", classname) )
            G_SpawnStruct(&level.spawnVar);
    }
    G_ResetEntityParsePoint();
}

