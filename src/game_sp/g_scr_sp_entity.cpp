#include <game_mp/actor_mp.h> // mod: MAX_ACTORS
#include "g_sp_ext.h"
// SP builtin table, lane E: entity / misc builtins.
// Searched after BO1Zombies's own tables and before the generated not-ported table (see
// src/game_sp/scr_sp_tables.h). Add a row per real port, keep the end marker last, then rerun
// `python tools/gen_sp_builtins.py` so the name's not-ported stub is dropped.
#include "scr_sp_tables.h"
#include "g_scr_sp_entity.h"
#include <game_mp/g_scr_main_mp.h>
#include <game_mp/g_main_mp.h>
#include <game_mp/g_spawn_mp.h>
#include <game_mp/g_utils_mp.h>
#include <clientscript/cscr_vm.h>
#include <clientscript/cscr_stringlist.h>
#include <clientscript/scr_const.h>
#include "g_sp_measure.h"
#include <bgame/bg_misc.h>
#include <server/sv_game.h>
#include <physics/destructible.h>
#include <xanim/xmodel.h>
#include <qcommon/cm_load.h>
#include <qcommon/cm_staticmodel.h>
#include <DynEntity/DynEntity_client.h>
#include <xanim/dobj.h>
#include <xanim/dobj_utils.h>
#include <qcommon/dobj_management.h>
#include <game/g_weapon.h>
#include <bgame/bg_weapons_def.h>
#include <game/actor.h>
#include <clientscript/cscr_animtree.h>
#include <universal/dvar.h>
#include <game/g_vehicle_path.h>
#include <server/sv_world.h>
#include <universal/surfaceflags.h>
#include "g_sp_level_exit.h"
#include <gfx_d3d/r_cinematic.h>
#include <database/db_registry.h>
#include <database/db_assetnames.h>
#include <win32/win_common.h>
#include <server_mp/server_mp.h>
#include <server_mp/sv_init_mp.h>
#include <sound/snd_bank.h>
#include <game/g_missile.h>
#include <game/bullet.h>
#include <qcommon/cm_world.h>
#include <xanim/xanim.h>
#include <universal/com_math_anglevectors.h>
#include <gfx_d3d/r_dvars.h>
#include <bgame/bg_pmove.h>
#include <game/g_bsp.h>
#include <physics/phys_local.h>
#include <physics/phys_gjk.h>
#include <physics/phys_collision.h>
#include <qcommon/cm_load_obj.h>
#include "g_sp_player_state.h"
#include <cmath>

// Existing vehicle-path storage (g_vehicle_path.cpp).
extern vehicle_node_t s_nodes[2000];
extern __int16 s_numNodes;
// g_scr_main_mp.cpp, not in its header.
void GScr_GetArrayKeys();
// g_vehicle_path.cpp, not in its header.
extern vn_field_t vn_fields[12];

static void G_SP_ResetEntityBuiltinsX2();

int g_spActorLimit = 32;
unsigned int g_spCodeSpawnCount;

struct SpEntityBuiltinState
{
    int useCount;
    bool codeSpawned;
};
static SpEntityBuiltinState s_spEntityBuiltinState[MAX_GENTITIES_SV];

void G_SP_ResetEntityBuiltins()
{
    g_spActorLimit = 32;
    g_spCodeSpawnCount = 0;
    memset(s_spEntityBuiltinState, 0, sizeof(s_spEntityBuiltinState));
    G_SP_ResetEntityBuiltinsX2();
}

void G_SP_ClearEntityBuiltinState(unsigned int entnum)
{
    iassert(entnum < MAX_GENTITIES_SV);
    s_spEntityBuiltinState[entnum] = {};
}

bool G_SP_IsCodeSpawned(const gentity_s *ent)
{
    iassert(ent->s.number >= 0 && ent->s.number < MAX_GENTITIES_SV);
    SpEntityBuiltinState *state = &s_spEntityBuiltinState[ent->s.number];
    if (!ent->r.inuse || state->useCount != ent->useCount)
        *state = {};
    return state->codeSpawned;
}

// zombies: SP 0x004acd50, the out-of-range origin diagnostic of the code-spawn builtins; the
// reported bounds use bits-2.
static void G_SP_OriginOutOfRange(const char *type, const float *origin, const float *center)
{
    Scr_Error(va("Trying to spawn %s at position (%0.2f, %0.2f, %0.2f) will place it outside of map ranges that can be sent across the network.\n Which given a map center (%0.2f, %0.2f, %0.2f) should be between (%0.2f, %0.2f, %0.2f) to (%0.2f, %0.2f, %0.2f)\n",
        type, origin[0], origin[1], origin[2], center[0], center[1], center[2],
        center[0] - 65536.0f, center[1] - 65536.0f, center[2] - 32768.0f,
        center[0] + 65536.0f, center[1] + 65536.0f, center[2] + 32768.0f), 0);
}

// zombies: codespawn (SP 0x007f15c0).
static void G_f_codespawn()
{
    unsigned __int16 classname = Scr_GetConstString(0, SCRIPTINSTANCE_SERVER);
    float origin[3];
    Scr_GetVector(1, origin, SCRIPTINSTANCE_SERVER);
    const float *center = *SV_GetMapCenter();
    if (!BG_ValidateOrigin(origin, 18, 17, center))
        G_SP_OriginOutOfRange("entity", origin, center);
    int spawnflags = Scr_GetNumParam(SCRIPTINSTANCE_SERVER) < 3 ? 0 : Scr_GetInt(2, SCRIPTINSTANCE_SERVER);
    if (classname != scr_const.script_origin)
        ++g_spCodeSpawnCount;
    gentity_s *ent = G_Spawn();
    Scr_SetString(&ent->classname, classname, SCRIPTINSTANCE_SERVER);
    Vec3Copy(origin, ent->r.currentOrigin);
    ent->spawnflags = spawnflags;
    // SP ent+0x33e, stored separately to preserve KB's fixed entity stride.
    s_spEntityBuiltinState[ent->s.number] = { ent->useCount, true };
    if (Scr_GetNumParam(SCRIPTINSTANCE_SERVER) > 5)
    {
        unsigned int def = Scr_GetConstString(5, SCRIPTINSTANCE_SERVER);
        G_SetupDestructible(ent, SL_ConvertToString(def, SCRIPTINSTANCE_SERVER));
        ent->model = 1;
    }
    if (G_CallSpawnEntity(ent))
    {
        if (ent->destructible)
            ent->model = G_ModelIndex((char *)ent->destructible->ddef->model->name);
        Scr_AddEntity(ent, SCRIPTINSTANCE_SERVER);
        return;
    }
    Scr_Error(va("unable to spawn \"%s\" entity", SL_ConvertToString(classname, SCRIPTINSTANCE_SERVER)), 0);
}

// zombies: getdestructibledefs (SP 0x005CA550). The exported builtin metadata reports codeSize 1;
// the exe body: an array of ddef->name for every in-use entity (gentity+0xDD) with a destructible.
static void G_f_getdestructibledefs()
{
    Scr_MakeArray(SCRIPTINSTANCE_SERVER);
    for (int i = 0; i < level.num_entities; ++i)
    {
        gentity_s *ent = &level.gentities[i];
        if (ent->r.inuse && ent->destructible)
        {
            Scr_AddString(ent->destructible->ddef->name, SCRIPTINSTANCE_SERVER);
            Scr_AddArray(SCRIPTINSTANCE_SERVER);
        }
    }
}

// zombies: getmiscmodels (SP 0x004EAEE0), used by the developer-only _createdynents. The exported
// builtin metadata reports codeSize 1; the exe body: the xmodel name of every clip-map static model.
static void G_f_getmiscmodels()
{
    Scr_MakeArray(SCRIPTINSTANCE_SERVER);
    for (unsigned int i = 0; i < cm.numStaticModels; ++i)
    {
        if (cm.staticModelList[i].xmodel)
        {
            Scr_AddString(cm.staticModelList[i].xmodel->name, SCRIPTINSTANCE_SERVER);
            Scr_AddArray(SCRIPTINSTANCE_SERVER);
        }
    }
}

// zombies: getdynmodels (SP 0x0042A210). As getmiscmodels, over the first dyn-entity list
// (cm.dynEntCount[0] / cm.dynEntDefList[0], xModel at +0x20).
static void G_f_getdynmodels()
{
    Scr_MakeArray(SCRIPTINSTANCE_SERVER);
    for (unsigned __int16 i = 0; i < cm.dynEntCount[0]; ++i)
    {
        if (cm.dynEntDefList[0][i].xModel)
        {
            Scr_AddString(cm.dynEntDefList[0][i].xModel->name, SCRIPTINSTANCE_SERVER);
            Scr_AddArray(SCRIPTINSTANCE_SERVER);
        }
    }
}

// zombies: developer-only getdebugdvar (SP 0x007F03F0): the dvar's displayable value, or "" when
// the dvar does not exist. Same body as the client CScr_GetDebugDvar.
static void G_f_getdebugdvar()
{
    const dvar_s *dvar = Dvar_FindVar(Scr_GetString(0, SCRIPTINSTANCE_SERVER));
    if (dvar)
    {
        Scr_AddString((char *)Dvar_DisplayableValue(dvar), SCRIPTINSTANCE_SERVER);
        return;
    }
    Scr_AddString((char *)"", SCRIPTINSTANCE_SERVER);
}

// zombies: developer-only getdebugdvarint (SP 0x007F0430): atoi(Dvar_GetVariantString(name)).
static void G_f_getdebugdvarint()
{
    Scr_AddInt(atoi(Dvar_GetVariantString(Scr_GetString(0, SCRIPTINSTANCE_SERVER))), SCRIPTINSTANCE_SERVER);
}

// zombies: developer-only getdebugdvarfloat (SP 0x007F0460): atof(Dvar_GetVariantString(name)).
static void G_f_getdebugdvarfloat()
{
    Scr_AddFloat((float)atof(Dvar_GetVariantString(Scr_GetString(0, SCRIPTINSTANCE_SERVER))), SCRIPTINSTANCE_SERVER);
}

// zombies: getspawnerarray (SP 0x007f0be0).
static void G_f_getspawnerarray()
{
    if (Scr_GetNumParam(SCRIPTINSTANCE_SERVER))
        Scr_Error("cannot call getspawnerarray with parameters", 0);
    Scr_MakeArray(SCRIPTINSTANCE_SERVER);
    for ( int i = 0; i < G_EntEnd(); i = G_EntNext(i) )
    {
        gentity_s *ent = &level.gentities[i];
        if (ent->r.inuse && ent->s.eType == ET_ACTOR_SPAWNER)
        {
            Scr_AddEntity(ent, SCRIPTINSTANCE_SERVER);
            Scr_AddArray(SCRIPTINSTANCE_SERVER);
        }
    }
}

// zombies: setspawnerteam (SP 0x00803af0).
static void G_m_setspawnerteam(scr_entref_t entref)
{
    gentity_s *ent = GetEntity(entref);
    if (ent->s.eType != ET_ACTOR_SPAWNER)
        Scr_Error("setspawnerteam can only be applied to AI spawners", 0);
    const char *team = Scr_GetString(0, SCRIPTINSTANCE_SERVER);
    if (!I_stricmp(team, "axis"))
    {
        ent->spawner.team = TEAM_AXIS;
        return;
    }
    if (!I_stricmp(team, "allies"))
    {
        ent->spawner.team = TEAM_ALLIES;
        return;
    }
    if (!I_stricmp(team, "neutral"))
    {
        // SP team 3 is neutral; KB has no named neutral enumerator.
        ent->spawner.team = 3;
        return;
    }
    Scr_ParamError(0, va("unknown team '%s', should be axis, allies, or neutral", team), SCRIPTINSTANCE_SERVER);
}

// zombies: setteamforentity (SP 0x008042d0).
static void G_m_setteamforentity(scr_entref_t entref)
{
    gentity_s *ent = GetEntity(entref);
    unsigned int team = Scr_GetConstString(0, SCRIPTINSTANCE_SERVER);
    if (team == scr_const.allies)
    {
        ent->team = TEAM_ALLIES;
        return;
    }
    if (team == scr_const.axis)
    {
        ent->team = TEAM_AXIS;
        return;
    }
    if (team == scr_const.none)
    {
        ent->team = TEAM_FREE;
        return;
    }
    Scr_Error(va("setteamforentity: invalid team used must be %s, %s or %s",
        SL_ConvertToString(scr_const.allies, SCRIPTINSTANCE_SERVER),
        SL_ConvertToString(scr_const.axis, SCRIPTINSTANCE_SERVER),
        SL_ConvertToString(scr_const.none, SCRIPTINSTANCE_SERVER)), 0);
}

// zombies: haseyes (SP 0x008062e0). See the shared header for the MP bit remap.
static void G_m_haseyes(scr_entref_t entref)
{
    gentity_s *ent = GetEntity(entref);
    if (Scr_GetNumParam(SCRIPTINSTANCE_SERVER) != 1)
    {
        Scr_Error("HasEyes() called with wrong params.\n", 0);
        return;
    }
    if (Scr_GetInt(0, SCRIPTINSTANCE_SERVER))
        ent->s.lerp.eFlags2 |= SP_EFLAGS2_HAS_EYES;
    else
        ent->s.lerp.eFlags2 &= ~SP_EFLAGS2_HAS_EYES;
}

// zombies: setplayercollision (SP 0x00806d40): SetPlayerCollision( <enable> ). SP clears (enable) or sets bit
// 0x1000000 of the entity's eFlags2 (SP gentity +8) and, for a player, of its ps.eFlags2 (SP client +0xe4). Used by
// zombie_temple's napalm zombie (maps/_zombiemode_ai_napalm.gsc:639). The SP reader of the bit (player collision)
// is not traced here; KB MP reads the same entity bit in aim_target.cpp.
static void G_m_setplayercollision(scr_entref_t entref)
{
    if (Scr_GetNumParam(SCRIPTINSTANCE_SERVER) != 1)
        Scr_Error("USAGE: <entity> SetPlayerCollision( <enable> )", 0);
    gentity_s *ent = GetEntity(entref);
    if (!ent)
        return;
    const int enable = Scr_GetInt(0, SCRIPTINSTANCE_SERVER);
    if (enable)
        ent->s.lerp.eFlags2 &= ~0x1000000u;
    else
        ent->s.lerp.eFlags2 |= 0x1000000u;
    if (ent->client)
    {
        if (enable)
            ent->client->ps.eFlags2 &= ~0x1000000u;
        else
            ent->client->ps.eFlags2 |= 0x1000000u;
    }
}

// zombies: useweaponhidetags (SP 0x007fe370).
static void G_m_useweaponhidetags(scr_entref_t entref)
{
    if (!Scr_GetNumParam(SCRIPTINSTANCE_SERVER))
        Scr_Error("useweaponhidetags <weaponName>.\n", 0);
    char *name = Scr_GetString(0, SCRIPTINSTANCE_SERVER);
    unsigned int weapon = G_GetWeaponIndexForName(name);
    if (!weapon)
        Scr_Error(va("useweaponhidetags called with unknown weapon name %s\n", name), 0);
    const WeaponVariantDef *def = BG_GetWeaponVariantDef(weapon);
    gentity_s *ent = GetEntity(entref);
    memset(ent->s.partBits, 0, sizeof(ent->s.partBits));
    DObj *obj = Com_GetServerDObj(ent->s.number);
    for (unsigned int i = 0; i < 32 && def->hideTags[i]; ++i)
    {
        unsigned __int8 bone = 254;
        if (DObjGetBoneIndex(obj, def->hideTags[i], &bone, -1))
            ent->s.partBits[bone >> 5] |= 0x80000000u >> (bone & 31);
    }
    DObjSetHidePartBits(obj, ent->s.partBits);
}

// zombies: weaponisgasweapon (SP 0x007fc500).
static void G_f_weaponisgasweapon()
{
    unsigned int weapon = G_GetWeaponIndexForName(Scr_GetString(0, SCRIPTINSTANCE_SERVER));
    if (!weapon)
    {
        Scr_AddBool(0, SCRIPTINSTANCE_SERVER);
        return;
    }
    Scr_AddBool(BG_GetWeaponDef(weapon)->weapType == WEAPTYPE_GAS, SCRIPTINSTANCE_SERVER);
}

// zombies: stopsounds (SP 0x007f4a50).
static void G_m_stopsounds(scr_entref_t entref)
{
    gentity_s *ent = GetEntity(entref);
    ent->r.svFlags &= ~1u;
    G_AddEvent(ent, EV_STOPSOUNDS, 0);
}

// zombies: trackscriptstate (SP 0x007cd130).
static void G_m_trackscriptstate(scr_entref_t entref)
{
    actor_s *actor = 0;
    if (!entref.classnum)
        actor = g_entities[entref.entnum].actor;
    if (!actor)
        Scr_ObjectError("not an actor", SCRIPTINSTANCE_SERVER);
    if (Scr_GetNumParam(SCRIPTINSTANCE_SERVER) != 2)
        Scr_Error("trackScriptState newStateName, reasonForTransition", 0);
    Scr_SetString(&actor->lastScriptState, actor->scriptState, SCRIPTINSTANCE_SERVER);
    Scr_SetString(&actor->scriptState, Scr_GetConstString(0, SCRIPTINSTANCE_SERVER), SCRIPTINSTANCE_SERVER);
    Scr_SetString(&actor->stateChangeReason, Scr_GetConstString(1, SCRIPTINSTANCE_SERVER), SCRIPTINSTANCE_SERVER);
    if (actor->scriptState == actor->lastScriptState)
    {
        // SP 0x004fa0e0 -> 0x008a7eb0 raises a script error, despite the TS
        // approximation describing this path as a warning.
        Scr_Error(va("trackScriptState should only be called on script state transitions.  Called for state %s from state %s.",
            SL_ConvertToString(actor->scriptState, SCRIPTINSTANCE_SERVER),
            SL_ConvertToString(actor->lastScriptState, SCRIPTINSTANCE_SERVER)), 0);
    }
}

// zombies: dontinterpolate (SP 0x007f3200).
static void G_m_dontinterpolate(scr_entref_t entref)
{
    gentity_s *ent = GetEntity(entref);
    if (ent->client)
    {
        ent->client->ps.eFlags ^= 2;
        return;
    }
    ent->s.lerp.eFlags ^= 2;
}

// zombies: getdestructiblename (SP 0x007F1DB0). The exported builtin metadata reports codeSize 1;
// the exe body is GetEntity then Scr_AddString(ent->destructible->ddef->name) with no null checks
// (SP reads gentity+0x150 then Destructible+0x10; KB's field names are used for its layout).
static void G_m_getdestructiblename(scr_entref_t entref)
{
    gentity_s *ent = GetEntity(entref);
    Scr_AddString(ent->destructible->ddef->name, SCRIPTINSTANCE_SERVER);
}

// zombies: V11 gib uses KB EV_GIB (0xCC), SP emits 0xCA (SP 0x00806EB0).
static void G_m_gib(scr_entref_t entref)
{
    gentity_s *ent = GetEntity(entref);
    unsigned int mask = 0;
    unsigned int type = Scr_GetConstString(0, SCRIPTINSTANCE_SERVER);
    // SP's const-string init at 0x005ec074/0x005ec089 writes "freeze" to
    // 0x023a594c. The TS approximation incorrectly labels this value "normal".
    if (type == scr_const.freeze)
        mask = 0x100;
    else if (type == scr_const.up)
        mask = 0x200;
    if (Scr_GetPointerType(1, SCRIPTINSTANCE_SERVER) != VAR_ARRAY)
        Scr_ParamError(1, va("Parameter (%s) must be an array", Scr_GetTypeName(1, SCRIPTINSTANCE_SERVER)), SCRIPTINSTANCE_SERVER);
    unsigned int array = Scr_GetObject(1, SCRIPTINSTANCE_SERVER);
    unsigned int sibling = FindFirstSibling(SCRIPTINSTANCE_SERVER, array);
    for (int i = 0; sibling; ++i, sibling = FindNextSibling(SCRIPTINSTANCE_SERVER, sibling))
    {
        if (GetValueType(SCRIPTINSTANCE_SERVER, sibling) == VAR_INTEGER)
        {
            unsigned int tag = GetVariableValueAddress(SCRIPTINSTANCE_SERVER, sibling)->u.intValue;
            if (tag > 7)
            {
                Scr_Error("Gib tag array passed to 'Gib' contains value out of range 0 -> 7", 0);
                return;
            }
            mask |= 1u << tag;
        }
        else
            Scr_Error(va("Array passed to gib contained member [%i] - valid types are int.", i), 0);
    }
    if (G_SP_MeasureEnabled()) // w1 evidence: script gib requests (freeze gun shatter/crumple)
        Com_Printf(16, "bo1_gib: time %d ent %d mask 0x%x\n", level.time, ent->s.number, mask);
    G_AddEvent(ent, EV_GIB, mask);
}

// zombies: setailimit (SP 0x00804ad0).
static void G_f_setailimit()
{
    int limit = Scr_GetInt(0, SCRIPTINSTANCE_SERVER);
    if (limit < 0 || limit > MAX_ACTORS) // mod: retail 32; bo1_mod_maxactors raises it
        Scr_ParamError(0, va("SetAILimit must take a value between 0 and %d inclusive.", MAX_ACTORS), SCRIPTINSTANCE_SERVER);
    g_spActorLimit = limit;
}

// zombies: watersimenable (SP 0x007fa500).
static void G_f_watersimenable()
{
    if (Scr_GetNumParam(SCRIPTINSTANCE_SERVER) != 1)
        Scr_Error("watersimenable() called with wrong params.\n", 0);
    Dvar_SetBoolByName("r_watersim_enabled", Scr_GetInt(0, SCRIPTINSTANCE_SERVER) != 0);
}

// zombies: activateclientexploder (SP 0x007fda90).
static void G_f_activateclientexploder()
{
    int id = Scr_GetInt(0, SCRIPTINSTANCE_SERVER);
    SV_GameSendServerCommand(-1, SV_CMD_RELIABLE, va("%c %i", '@', id));
}

// zombies: deactivateclientexploder (SP 0x007fdac0).
static void G_f_deactivateclientexploder()
{
    int id = Scr_GetInt(0, SCRIPTINSTANCE_SERVER);
    // x86 neg wraps INT_MIN rather than invoking signed-overflow UB.
    SV_GameSendServerCommand(-1, SV_CMD_RELIABLE, va("%c %i", '@', (int)(0u - (unsigned int)id)));
}

// zombies: getallvehiclenodes (SP 0x006270e0).
static void G_f_getallvehiclenodes()
{
    Scr_MakeArray(SCRIPTINSTANCE_SERVER);
    for (__int16 i = 0; i < s_numNodes; ++i)
    {
        Scr_AddEntityNum(s_nodes[i].index, 3, SCRIPTINSTANCE_SERVER, 0);
        Scr_AddArray(SCRIPTINSTANCE_SERVER);
    }
}

// zombies: groundtrace (SP 0x00807c00).
static void G_f_groundtrace()
{
    int ignoreEnt = ENTITYNUM_NONE;
    int mask = 0x0280ECB3;
    trace_t trace = {};
    float start[3], end[3];
    Scr_GetVector(0, start, SCRIPTINSTANCE_SERVER);
    Scr_GetVector(1, end, SCRIPTINSTANCE_SERVER);
    if (!Scr_GetInt(2, SCRIPTINSTANCE_SERVER))
        mask = 0x00802CB3;
    if (Scr_GetType(3, SCRIPTINSTANCE_SERVER) == VAR_POINTER
        && Scr_GetPointerType(3, SCRIPTINSTANCE_SERVER) == VAR_ENTITY)
        ignoreEnt = Scr_GetEntity(3)->s.number;
    // SP deliberately tests exact argument counts here (also true of bullettrace).
    if (Scr_GetNumParam(SCRIPTINSTANCE_SERVER) == 5 && Scr_GetInt(4, SCRIPTINSTANCE_SERVER))
        mask &= ~0x20;
    if (Scr_GetNumParam(SCRIPTINSTANCE_SERVER) == 6 && Scr_GetInt(5, SCRIPTINSTANCE_SERVER))
        mask &= ~0x10;
    G_LocationalTrace(&trace, start, end, ignoreEnt, mask, 0, 0);
    Scr_MakeArray(SCRIPTINSTANCE_SERVER);
    Scr_AddFloat(trace.fraction, SCRIPTINSTANCE_SERVER);
    Scr_AddArrayStringIndexed(scr_const.fraction, SCRIPTINSTANCE_SERVER);
    float position[3];
    Vec3Lerp(start, end, trace.fraction, position);
    Scr_AddVector(position, SCRIPTINSTANCE_SERVER);
    Scr_AddArrayStringIndexed(scr_const.position, SCRIPTINSTANCE_SERVER);
    unsigned __int16 hit = Trace_GetEntityHitId(&trace);
    if (hit == ENTITYNUM_NONE || hit == ENTITYNUM_WORLD)
        Scr_AddUndefined(SCRIPTINSTANCE_SERVER);
    else
        Scr_AddEntity(&g_entities[hit], SCRIPTINSTANCE_SERVER);
    Scr_AddArrayStringIndexed(scr_const.entity, SCRIPTINSTANCE_SERVER);
    if (trace.fraction < 1.0f)
    {
        Scr_AddVector(trace.normal.vec.v, SCRIPTINSTANCE_SERVER);
        Scr_AddArrayStringIndexed(scr_const.normal, SCRIPTINSTANCE_SERVER);
        Scr_AddString(Com_SurfaceTypeToName((trace.sflags >> 20) & 63), SCRIPTINSTANCE_SERVER);
        Scr_AddArrayStringIndexed(scr_const.surfacetype, SCRIPTINSTANCE_SERVER);
        return;
    }
    float normal[3];
    Vec3Sub(end, start, normal);
    Vec3Normalize(normal);
    Scr_AddVector(normal, SCRIPTINSTANCE_SERVER);
    Scr_AddArrayStringIndexed(scr_const.normal, SCRIPTINSTANCE_SERVER);
    Scr_AddConstString(scr_const.none, SCRIPTINSTANCE_SERVER);
    Scr_AddArrayStringIndexed(scr_const.surfacetype, SCRIPTINSTANCE_SERVER);
}

// zombies: issentient (SP 0x007f00a0).
static void G_f_issentient()
{
    if (Scr_GetType(0, SCRIPTINSTANCE_SERVER) == VAR_POINTER
        && Scr_GetPointerType(0, SCRIPTINSTANCE_SERVER) == VAR_ENTITY
        && Scr_GetEntity(0)->sentient)
    {
        Scr_AddBool(1, SCRIPTINSTANCE_SERVER);
        return;
    }
    Scr_AddBool(0, SCRIPTINSTANCE_SERVER);
}

// zombies: setphysparams (SP 0x00806bf0).
static void G_m_setphysparams(scr_entref_t entref)
{
    if (!zombiemode->current.enabled)
    {
        Scr_Error("Invalid call to SetPhysParams()\n", 0);
        return;
    }
    if (Scr_GetNumParam(SCRIPTINSTANCE_SERVER) != 3)
        Scr_Error("setphysparams takes three parameters\n", 0);
    gentity_s *ent = GetEntity(entref);
    if (!ent->actor)
        Scr_Error("setphysparams must be called on an AI only.", 0);
    float radius = (float)Scr_GetFloat(0, SCRIPTINSTANCE_SERVER);
    float minZ = (float)Scr_GetFloat(1, SCRIPTINSTANCE_SERVER);
    float maxZ = (float)Scr_GetFloat(2, SCRIPTINSTANCE_SERVER);
    ent->r.mins[2] = minZ;
    ent->r.mins[0] = -radius;
    ent->r.mins[1] = -radius;
    ent->r.maxs[0] = radius;
    ent->r.maxs[1] = radius;
    ent->r.maxs[2] = maxZ;
    Vec3Copy(ent->r.mins, ent->actor->Physics.vMins);
    Vec3Copy(ent->r.maxs, ent->actor->Physics.vMaxs);
}

// zombies: weapondualwieldweaponname (SP 0x007fc800).
static void G_f_weapondualwieldweaponname()
{
    char *name = Scr_GetString(0, SCRIPTINSTANCE_SERVER);
    unsigned int weapon = G_GetWeaponIndexForName(name);
    Scr_VerifyWeaponIndex(weapon, name);
    const WeaponDef *def = BG_GetWeaponDef(weapon);
    if (def->bDualWield && def->dualWieldWeaponIndex)
    {
        Scr_AddString(BG_WeaponName(def->dualWieldWeaponIndex), SCRIPTINSTANCE_SERVER);
        return;
    }
    Scr_AddConstString(scr_const.none, SCRIPTINSTANCE_SERVER);
}

// ---- lane x2: entity / level / fx / vehicles ---------------------------------------------------

// zombies: SP 0x01C88DE4, the frame's peak compressed snapshot size (SV_SendMessageToClient,
// SP 0x0049E716..0x0049E724: reset for the first client slot, then a running maximum). oktospawn reads it.
static int s_spPeakSnapshotSize;

void G_SP_NoteSnapshotSize(bool firstClient, int size)
{
    if (firstClient)
        s_spPeakSnapshotSize = 0;
    if (s_spPeakSnapshotSize <= size)
        s_spPeakSnapshotSize = size;
}

// zombies: SP gclient+0x584, setscripthintstring's hint index + 1 (0 = none). KB's gclient_s keeps
// its MP layout, so the value lives here, one slot per g_clients entry, cleared per level.
static int s_spScriptHintString[32];

bool G_SP_GetScriptHintString(const gclient_s *client, int *hintString)
{
    unsigned int clientNum = (unsigned int)(client - level.clients);
    if (clientNum >= ARRAY_COUNT(s_spScriptHintString) || !s_spScriptHintString[clientNum])
        return false;
    *hintString = s_spScriptHintString[clientNum] - 1;
    return true;
}

// zombies: missionsuccess (SP 0x007fbcd0).
static void G_f_missionsuccess()
{
    if (Scr_GetNumParam(SCRIPTINSTANCE_SERVER) > 2)
        Scr_Error("missionSuccess only takes two parameters: missionSuccess(nextMap, persistent)\n", 0);
    G_SP_ChangeLevel();
    g_spLevelExit.missionSuccess = 1;
}

// zombies: forcelevelend (SP 0x00804ab0). SP's reader is its server's map-end check (SP 0x00479C59),
// which KB (MP server) has no counterpart for; the flag is kept for it.
static void G_f_forcelevelend()
{
    g_spLevelExit.forceLevelEnd = 1;
    Dvar_SetString((dvar_s *)nextmap, "");
}

// zombies: codespawnfx (SP 0x007fd780). SP's spawnFx body: unlike MP's Scr_SpawnFX it does not set
// SVF_BROADCAST and does not snap the origin to integers.
static void G_f_codespawnfx()
{
    if (Scr_GetNumParam(SCRIPTINSTANCE_SERVER) < 2 || Scr_GetNumParam(SCRIPTINSTANCE_SERVER) > 4)
        Scr_Error("Incorrect number of parameters", 0);
    int givenAxisCount = 0;
    float axis[3][3];
    int fxId = Scr_GetInt(0, SCRIPTINSTANCE_SERVER);
    int numParam = Scr_GetNumParam(SCRIPTINSTANCE_SERVER);
    if (numParam == 3 || numParam == 4)
    {
        if (numParam == 4)
        {
            Scr_GetVector(3, axis[2], SCRIPTINSTANCE_SERVER);
            if (Vec3Normalize(axis[2]) == 0.0f)
                Scr_FxParamError(3, "spawnFx called with (0 0 0) up direction", fxId);
            ++givenAxisCount;
        }
        Scr_GetVector(2, axis[0], SCRIPTINSTANCE_SERVER);
        if (Vec3Normalize(axis[0]) == 0.0f)
            Scr_FxParamError(2, "spawnFx called with (0 0 0) forward direction", fxId);
        ++givenAxisCount;
    }
    float pos[3];
    Scr_GetVector(1, pos, SCRIPTINSTANCE_SERVER);
    gentity_s *ent = G_Spawn();
    ent->s.eType = ET_FX;
    ent->s.un1.scale = (unsigned __int8)fxId;
    G_SetOrigin(ent, pos);
    Scr_SetFxAngles(givenAxisCount, axis, ent->s.lerp.apos.trBase);
    SV_LinkEntity(ent);
    Scr_AddEntity(ent, SCRIPTINSTANCE_SERVER);
}

// zombies: codeplayloopedfx (SP 0x007fd4e0). SP's playLoopedFx body: no SVF_BROADCAST, unlike MP.
static void G_f_codeplayloopedfx()
{
    if (Scr_GetNumParam(SCRIPTINSTANCE_SERVER) < 3 || Scr_GetNumParam(SCRIPTINSTANCE_SERVER) > 6)
        Scr_Error("Incorrect number of parameters", 0);
    int givenAxisCount = 0;
    float cullDist = 0.0f;
    float axis[3][3];
    int fxId = Scr_GetInt(0, SCRIPTINSTANCE_SERVER);
    int numParam = Scr_GetNumParam(SCRIPTINSTANCE_SERVER);
    if (numParam == 4 || numParam == 5 || numParam == 6)
    {
        if (numParam != 4)
        {
            if (numParam == 6)
            {
                givenAxisCount = 1;
                Scr_GetVector(5, axis[2], SCRIPTINSTANCE_SERVER);
                if (Vec3Normalize(axis[2]) == 0.0f)
                    Scr_FxParamError(5, "playLoopedFx called with (0 0 0) up direction", fxId);
            }
            Scr_GetVector(4, axis[0], SCRIPTINSTANCE_SERVER);
            if (Vec3Normalize(axis[0]) == 0.0f)
                Scr_FxParamError(4, "playLoopedFx called with (0 0 0) forward direction", fxId);
            ++givenAxisCount;
        }
        cullDist = (float)Scr_GetFloat(3, SCRIPTINSTANCE_SERVER);
    }
    float pos[3];
    Scr_GetVector(2, pos, SCRIPTINSTANCE_SERVER);
    // SP: seconds * 1000.0f (float), + 9.313225746154785e-10, fistp (round to nearest).
    float repeatMsec = (float)Scr_GetFloat(1, SCRIPTINSTANCE_SERVER) * 1000.0f;
    int repeat = (int)lrint((double)repeatMsec + 9.313225746154785e-10);
    if (repeat <= 0)
        Scr_FxParamError(1, "playLoopedFx called with repeat < 0.001 seconds", fxId);
    gentity_s *ent = G_Spawn();
    ent->s.eType = ET_LOOP_FX;
    ent->s.un1.scale = (unsigned __int8)fxId;
    G_SetOrigin(ent, pos);
    Scr_SetFxAngles(givenAxisCount, axis, ent->s.lerp.apos.trBase);
    ent->s.lerp.u.loopFx.cullDist = cullDist;
    ent->s.lerp.u.loopFx.period = repeat;
    SV_LinkEntity(ent);
    Scr_AddEntity(ent, SCRIPTINSTANCE_SERVER);
}

// zombies: cleanupspawneddynents (SP 0x00805340): '>' to every client (they drop their script dynents).
static void G_f_cleanupspawneddynents()
{
    SV_SendServerCommand(0, SV_CMD_RELIABLE, "%c", '>');
}

// zombies: codespawnvehicle (SP 0x007f1730). codespawnvehicle(model, targetname, vehicletype, origin,
// angles [, destructibledef]). Unlike MP's GScr_SpawnVehicle: the origin range check, the spawn
// statistic, the model falling back to the vehicle info's worldModel when argument 0 is not a
// string, no model check and no G_MakeVehicleUsable.
static void G_f_codespawnvehicle()
{
    unsigned int targetname = Scr_GetConstString(1, SCRIPTINSTANCE_SERVER);
    unsigned int vehicletype = Scr_GetConstString(2, SCRIPTINSTANCE_SERVER);
    float origin[3], angles[3];
    Scr_GetVector(3, origin, SCRIPTINSTANCE_SERVER);
    Scr_GetVector(4, angles, SCRIPTINSTANCE_SERVER);
    const float *center = *SV_GetMapCenter();
    if (!BG_ValidateOrigin(origin, 18, 17, center))
        G_SP_OriginOutOfRange("vehicle", origin, center);
    // SP compares the vehicle type (not a classname) with script_origin here.
    if (vehicletype != scr_const.script_origin)
        ++g_spCodeSpawnCount;
    gentity_s *ent = G_Spawn();
    Scr_SetString(&ent->classname, scr_const.script_vehicle, SCRIPTINSTANCE_SERVER);
    if (Scr_GetType(0, SCRIPTINSTANCE_SERVER) == VAR_STRING)
    {
        G_SetModel(ent, Scr_GetString(0, SCRIPTINSTANCE_SERVER));
    }
    else
    {
        const vehicle_info_t *info = BG_GetVehicleInfo(VEH_GetVehicleInfoFromName(SL_ConvertToString(vehicletype, SCRIPTINSTANCE_SERVER)));
        G_SetModel(ent, (char *)info->model);
    }
    Scr_SetString(&ent->targetname, targetname, SCRIPTINSTANCE_SERVER);
    Vec3Copy(origin, ent->r.currentOrigin);
    Vec3Copy(angles, ent->r.currentAngles);
    if (Scr_GetNumParam(SCRIPTINSTANCE_SERVER) > 5)
        G_SetupDestructible(ent, SL_ConvertToString(Scr_GetConstString(5, SCRIPTINSTANCE_SERVER), SCRIPTINSTANCE_SERVER));
    G_SpawnVehicle(ent, SL_ConvertToString(vehicletype, SCRIPTINSTANCE_SERVER), 0);
    Scr_AddEntity(ent, SCRIPTINSTANCE_SERVER);
}

// zombies: codespawnturret (SP 0x007f18c0). SP's SpawnTurretInternal (0x004FEB30) is MP's plus the
// spawn statistic; MP's GScr_SpawnTurret additionally sets takedamage / svFlags, SP does not.
static void G_f_codespawnturret()
{
    unsigned int classname = Scr_GetConstString(0, SCRIPTINSTANCE_SERVER);
    float origin[3];
    Scr_GetVector(1, origin, SCRIPTINSTANCE_SERVER);
    const char *weaponinfoname = Scr_GetString(2, SCRIPTINSTANCE_SERVER);
    const float *center = *SV_GetMapCenter();
    if (!BG_ValidateOrigin(origin, 18, 17, center))
        G_SP_OriginOutOfRange("entity", origin, center);
    if (classname != scr_const.script_origin)
        ++g_spCodeSpawnCount;
    gentity_s *ent = SpawnTurretInternal(classname, origin, weaponinfoname);
    Scr_AddEntity(ent, SCRIPTINSTANCE_SERVER);
}

// zombies: getvehiclenodearray (SP 0x0060d8c0): every vehicle node whose string field matches.
static void G_f_getvehiclenodearray()
{
    unsigned int name = Scr_GetConstString(0, SCRIPTINSTANCE_SERVER);
    const char *key = Scr_GetString(1, SCRIPTINSTANCE_SERVER);
    int offset = Scr_GetOffset(3, (char *)key, SCRIPTINSTANCE_SERVER);
    if (offset < 0)
        return;
    const vn_field_t *f = &vn_fields[offset];
    if (f->type != F_STRING)
        Scr_ParamError(1, "key is not internally a string", SCRIPTINSTANCE_SERVER);
    Scr_MakeArray(SCRIPTINSTANCE_SERVER);
    for (__int16 i = 0; i < s_numNodes; ++i)
    {
        unsigned __int16 value = *(unsigned __int16 *)((char *)&s_nodes[i] + f->ofs);
        if (value && value == name)
        {
            Scr_AddEntityNum(s_nodes[i].index, 3, SCRIPTINSTANCE_SERVER, 0);
            Scr_AddArray(SCRIPTINSTANCE_SERVER);
        }
    }
}

// SP reads the vehicle info's model name at +0xF0C and the script vehicle's team at +0x1F4.
static_assert(offsetof(vehicle_info_t, model) == 0xF0C, "vehicle_info_t::model moved");
// KB's scr_vehicle_s has the MP layout (team is not at +0x1F4), so the team is matched by name.

// zombies: SP 0x00809480, the script_vehicle check used by the compass methods.
static gentity_s *G_SP_GetScriptVehicleEnt(scr_entref_t entref)
{
    if (entref.classnum)
    {
        Scr_ObjectError("not an entity", SCRIPTINSTANCE_SERVER);
        return 0;
    }
    gentity_s *ent = &g_entities[entref.entnum];
    if (ent->classname != scr_const.script_vehicle)
        Scr_Error(va("entity %i is not a script_vehicle\n", entref.entnum), 0);
    if (!ent->scr_vehicle)
        Scr_Error(va("entity %i doesn't have a script_vehicle\n", entref.entnum), 0);
    return ent;
}

// zombies: addvehicletocompass (SP 0x004b5190). The icon type goes in un1 (shared with
// destructibleid, hence the destructible checks) and drawOnCompass in the vehicle lerp state.
static void G_m_addvehicletocompass(scr_entref_t entref)
{
    gentity_s *ent = G_SP_GetScriptVehicleEnt(entref);
    if (!Scr_GetNumParam(SCRIPTINSTANCE_SERVER))
    {
        Com_PrintWarning(25, "Script AddVehicleToCompass(); Was not passed a compassIconType, defaulting to \"tank\".");
        // SP leaves drawOnCompass alone on this path.
        if (!ent->destructible)
            ent->s.un1.scale = 1;
        return;
    }
    if (!ent->destructible)
    {
        const char *type = Scr_GetString(0, SCRIPTINSTANCE_SERVER);
        if (!I_stricmp(type, "tank"))
        {
            int team = ent->scr_vehicle->team;
            if (team == 2)
                ent->s.un1.scale = 2;
            else
                ent->s.un1.scale = team == 1 ? 3 : 1;
        }
        else if (!I_stricmp(type, "helicopter"))
        {
            ent->s.un1.scale = 4;
        }
        else if (!I_stricmp(type, "plane"))
        {
            ent->s.un1.scale = 5;
        }
        else if (!I_stricmp(type, "automobile"))
        {
            ent->s.un1.scale = 6;
        }
        else if (!*type)
        {
            Com_PrintWarning(25, "Script AddVehicleToCompass(); Was not passed a compassIconType, defaulting to \"tank\".");
            ent->s.un1.scale = 1;
        }
        else
        {
            Scr_Error(va("Unrecognized vehicle type given, \"%s\".", type), 0);
        }
    }
    ent->s.lerp.u.vehicle.drawOnCompass = 1;
}

// zombies: removevehiclefromcompass (SP 0x005970d0).
static void G_m_removevehiclefromcompass(scr_entref_t entref)
{
    gentity_s *ent = G_SP_GetScriptVehicleEnt(entref);
    ent->s.lerp.u.vehicle.drawOnCompass = 0;
    if (!ent->destructible)
        ent->s.un1.scale = 0;
}

// zombies: setvehicleattachments (SP 0x00806350): eFlags2 0x2000000 (as KB's G_VehicleRestore
// uses it) and '=' <entnum> <value> to every client.
static void G_m_setvehicleattachments(scr_entref_t entref)
{
    gentity_s *ent = GetEntity(entref);
    int value = Scr_GetInt(0, SCRIPTINSTANCE_SERVER);
    if (value)
        ent->s.lerp.eFlags2 |= 0x2000000;
    else
        ent->s.lerp.eFlags2 &= ~0x2000000;
    SV_SendServerCommand(0, SV_CMD_RELIABLE, "%c %d %d", '=', ent->s.number, value);
}

// zombies: setenginevolume / getenginevolume (SP vehicle methods 31/32, table SP 0x00a551d8: the shared null sub
// SP 0x00651a30, like startenginesound/stopenginesound in the MP table). _vehicle.gsc volume_up calls it on every vehicle.
static void G_m_enginevolume_null(scr_entref_t entref)
{
}

// zombies: issprinting() (SP 0x00806620): PM_IsSprinting (SP 0x005f8b60) of the player's ps.
static void G_m_issprinting(scr_entref_t entref)
{
    gentity_s *ent = 0;
    if (entref.classnum)
        Scr_ObjectError("not an entity", SCRIPTINSTANCE_SERVER);
    else
        ent = &g_entities[entref.entnum];
    if (!ent->client)
    {
        Scr_Error("IsSprinting can only be called on a player.", SCRIPTINSTANCE_SERVER);
        return;
    }
    Scr_AddInt(PM_IsSprinting(&ent->client->ps), SCRIPTINSTANCE_SERVER);
}

// zombies: setzombieshrink( on ) (SP 0x00807ba0): writes the entityState un1 byte (SP gentity +0xd7 = es.un1; KB's
// MP entityState has un1 at +0xd9, +0xd7 is faction). Sent to clients; no SP code reads it on an actor, the shrink
// ray's visual shrink is the script's model swap (_zombiemode_weap_shrink_ray.gsc).
static void G_m_setzombieshrink(scr_entref_t entref)
{
    gentity_s *ent = 0;
    if (entref.classnum)
        Scr_ObjectError("not an entity", SCRIPTINSTANCE_SERVER);
    else
        ent = &g_entities[entref.entnum];
    ent->s.un1.scale = (unsigned __int8)Scr_GetInt(0, SCRIPTINSTANCE_SERVER);
}

// zombies: allowmipstoload (SP 0x00651a30, the shared null sub).
static void G_f_allowmipstoload()
{
}

// zombies: distance2dsquared (SP 0x007f92c0): (dy * dy) + (dx * dx), in that order.
static void G_f_distance2dsquared()
{
    float a[3], b[3];
    Scr_GetVector(0, a, SCRIPTINSTANCE_SERVER);
    Scr_GetVector(1, b, SCRIPTINSTANCE_SERVER);
    float dy = a[1] - b[1];
    float dx = a[0] - b[0];
    Scr_AddFloat(dy * dy + dx * dx, SCRIPTINSTANCE_SERVER);
}

// zombies: bulletspread (SP 0x005e9d20): bulletspread(start, end, spread) -> the spread end point
// (Bullet_Endpos seeded with level.time over bulletrange, as a weapon-0 shot along start->end).
static void G_f_bulletspread()
{
    float start[3], end[3];
    Scr_GetVector(0, start, SCRIPTINSTANCE_SERVER);
    Scr_GetVector(1, end, SCRIPTINSTANCE_SERVER);
    float spread = (float)Scr_GetFloat(2, SCRIPTINSTANCE_SERVER);
    weaponParms wp;
    Weapon_SetWeaponParamsWeapon(&wp, 0);
    Vec3Copy(start, wp.muzzleTrace);
    Vec3Sub(end, start, wp.forward);
    float len = Vec3Length(wp.forward);
    float scale = 1.0f / (-len >= 0.0f ? 1.0f : len);
    Vec3Scale(wp.forward, scale, wp.forward);
    Vec3Copy(wp.forward, wp.gunForward);
    // SP 0x004C64E0: PerpendicularVector(forward, right), up = forward x right.
    PerpendicularVector(wp.forward, wp.right);
    Vec3Cross(wp.forward, wp.right, wp.up);
    Bullet_Endpos(level.time, spread, end, 0, &wp, sv_bullet_range->current.value);
    Scr_AddVector(end, SCRIPTINSTANCE_SERVER);
}

// zombies: entsearch (SP 0x005f63c0): entsearch([contentmask [, origin, radius [, dir, fov]]]).
static void G_f_entsearch()
{
    int numParam = Scr_GetNumParam(SCRIPTINSTANCE_SERVER);
    if (!numParam)
    {
        Scr_MakeArray(SCRIPTINSTANCE_SERVER);
        for ( int i = 0; i < G_EntEnd(); i = G_EntNext(i) )
        {
            if (g_entities[i].r.inuse)
            {
                Scr_AddEntity(&g_entities[i], SCRIPTINSTANCE_SERVER);
                Scr_AddArray(SCRIPTINSTANCE_SERVER);
            }
        }
        return;
    }
    int contentmask = Scr_GetInt(0, SCRIPTINSTANCE_SERVER);
    if ((unsigned int)numParam <= 1)
    {
        Scr_MakeArray(SCRIPTINSTANCE_SERVER);
        for ( int i = 0; i < G_EntEnd(); i = G_EntNext(i) )
        {
            if (g_entities[i].r.inuse && (g_entities[i].r.contents & contentmask))
            {
                Scr_AddEntity(&g_entities[i], SCRIPTINSTANCE_SERVER);
                Scr_AddArray(SCRIPTINSTANCE_SERVER);
            }
        }
        return;
    }
    if ((unsigned int)numParam < 3)
    {
        Scr_ParamError(1, "Cannot specify an origin without a radius", SCRIPTINSTANCE_SERVER);
        return;
    }
    float origin[3];
    Scr_GetVector(1, origin, SCRIPTINSTANCE_SERVER);
    float radius = (float)Scr_GetFloat(2, SCRIPTINSTANCE_SERVER);
    if (0.0f >= radius)
    {
        Scr_ParamError(2, "Range should be greater than 0", SCRIPTINSTANCE_SERVER);
        return;
    }
    float dir[3] = { 1.0f, 0.0f, 0.0f };
    float cosFov;
    if ((unsigned int)numParam <= 3)
    {
        cosFov = -1.0f;
    }
    else
    {
        if ((unsigned int)numParam < 5)
        {
            Scr_ParamError(3, "Cannot specify an direction without a FOV", SCRIPTINSTANCE_SERVER);
            return;
        }
        Scr_GetVector(3, dir, SCRIPTINSTANCE_SERVER);
        float fov = (float)Scr_GetFloat(4, SCRIPTINSTANCE_SERVER);
        if (0.0f >= fov)
        {
            Scr_ParamError(4, "FOV should be greater than 0", SCRIPTINSTANCE_SERVER);
            return;
        }
        if (fov > 360.0f)
            fov = 360.0f;
        float halfFov = fov * 0.5f;
        if (179.9f > halfFov)
            cosFov = (float)cos((double)(halfFov * 0.017453292f));
        else
            cosFov = -1.0f;
    }
    if (Vec3Normalize(dir) == 0.0f)
    {
        Scr_ParamError(3, "Direction length must be greater than 0", SCRIPTINSTANCE_SERVER);
        return;
    }
    Scr_MakeArray(SCRIPTINSTANCE_SERVER);
    float mins[3], maxs[3];
    mins[0] = origin[0] - radius;
    mins[1] = origin[1] - radius;
    mins[2] = origin[2] - radius;
    maxs[0] = origin[0] + radius;
    maxs[1] = origin[1] + radius;
    maxs[2] = origin[2] + radius;
    int entityList[1024];
    int count = CM_AreaEntities(mins, maxs, entityList, 1024, contentmask);
    for (int i = 0; i < count; ++i)
    {
        gentity_s *ent = &g_entities[entityList[i]];
        float delta[3];
        Vec3Sub(ent->r.currentOrigin, origin, delta);
        float dist = Vec3Normalize(delta);
        if (radius > dist && Vec3Dot(delta, dir) >= cosFov)
        {
            Scr_AddEntity(ent, SCRIPTINSTANCE_SERVER);
            Scr_AddArray(SCRIPTINSTANCE_SERVER);
        }
    }
}

// zombies: getstartorigin (SP 0x005c8770): the anim's absolute delta at time 0, placed by
// (origin, angles). getstartorigin(origin, angles, anim).
static void G_f_getstartorigin()
{
    float origin[3], angles[3];
    Scr_GetVector(0, origin, SCRIPTINSTANCE_SERVER);
    Scr_GetVector(1, angles, SCRIPTINSTANCE_SERVER);
    scr_anim_s anim = Scr_GetAnim(2, 0, SCRIPTINSTANCE_SERVER);
    float rot[2], trans[3];
    XAnimGetAbsDelta(Scr_GetAnims(anim.tree, SCRIPTINSTANCE_SERVER), anim.index, rot, trans, 0.0f);
    float axis[4][3];
    AnglesToAxis(angles, axis);
    Vec3Copy(origin, axis[3]);
    float out[3];
    MatrixTransformVector43(trans, axis, out);
    Scr_AddVector(out, SCRIPTINSTANCE_SERVER);
}

// zombies: getstartangles (SP 0x004dcfc0): angles of YawToAxis(delta yaw at time 0) * angles.
static void G_f_getstartangles()
{
    float origin[3], angles[3];
    Scr_GetVector(0, origin, SCRIPTINSTANCE_SERVER);
    Scr_GetVector(1, angles, SCRIPTINSTANCE_SERVER);
    scr_anim_s anim = Scr_GetAnim(2, 0, SCRIPTINSTANCE_SERVER);
    float rot[2], trans[3];
    XAnimGetAbsDelta(Scr_GetAnims(anim.tree, SCRIPTINSTANCE_SERVER), anim.index, rot, trans, 0.0f);
    float axis[4][3];
    AnglesToAxis(angles, axis);
    Vec3Copy(origin, axis[3]);
    float yawAxis[3][3];
    YawToAxis((float)RotationToYaw(rot), yawAxis);
    float startAxis[3][3];
    MatrixMultiply(yawAxis, axis, startAxis);
    float out[3];
    AxisToAngles(startAxis, out);
    Scr_AddVector(out, SCRIPTINSTANCE_SERVER);
}

// zombies: oktospawn (SP 0x007f1580): fewer than 4 code spawns this level and room for 50 bytes per
// spawn under 800 bytes of the frame's peak snapshot.
static void G_f_oktospawn()
{
    int count = (int)g_spCodeSpawnCount;
    if ((unsigned int)(count * 50 + s_spPeakSnapshotSize) < 800 && count < 4)
    {
        Scr_AddInt(1, SCRIPTINSTANCE_SERVER);
        return;
    }
    Scr_AddInt(0, SCRIPTINSTANCE_SERVER);
}

// zombies: getsnapshotindexarray (SP 0x007ff1b0): each client slot's next outgoing message number
// (undefined for a slot that is not active), then level.time + 1000.
static void G_f_getsnapshotindexarray()
{
    Scr_MakeArray(SCRIPTINSTANCE_SERVER);
    client_t *client = svs.clients;
    for (int i = 0; i < com_maxclients->current.integer; ++i, ++client)
    {
        if (client->header.state == CS_ACTIVE)
            Scr_AddInt(client->header.netchan.outgoingSequence + 1, SCRIPTINSTANCE_SERVER);
        else
            Scr_AddUndefined(SCRIPTINSTANCE_SERVER);
        Scr_AddArray(SCRIPTINSTANCE_SERVER);
    }
    Scr_AddInt(level.time + 1000, SCRIPTINSTANCE_SERVER);
    Scr_AddArray(SCRIPTINSTANCE_SERVER);
}

// zombies: snapshotacknowledged (SP 0x007ff230). SP treats index 4 and up as the time entry (its
// coop has 4 client slots) and clears the result when an active client's messageAcknowledge is past
// the recorded number; both exactly as the exe does.
static void G_f_snapshotacknowledged()
{
    if (Scr_GetPointerType(0, SCRIPTINSTANCE_SERVER) != VAR_ARRAY)
        Scr_ParamError(0, va("Parameter (%s) must be an array", Scr_GetTypeName(0, SCRIPTINSTANCE_SERVER)), SCRIPTINSTANCE_SERVER);
    unsigned int array = Scr_GetObject(0, SCRIPTINSTANCE_SERVER);
    unsigned int size = GetArraySize(SCRIPTINSTANCE_SERVER, array);
    client_t *client = svs.clients;
    bool acknowledged = true;
    for (unsigned int i = 0; i < size; ++i, ++client)
    {
        unsigned int id = FindArrayVariable(SCRIPTINSTANCE_SERVER, array, i);
        if (!id || GetValueType(SCRIPTINSTANCE_SERVER, id) != VAR_INTEGER)
            continue;
        int value = GetVariableValueAddress(SCRIPTINSTANCE_SERVER, id)->u.intValue;
        if (i >= 4)
        {
            if (level.time >= value)
                acknowledged = true;
        }
        else if (client->header.state == CS_ACTIVE && value < client->messageAcknowledge)
        {
            acknowledged = false;
        }
    }
    if (acknowledged)
        Scr_AddInt(1, SCRIPTINSTANCE_SERVER);
    else
        Scr_AddUndefined(SCRIPTINSTANCE_SERVER);
}

// zombies: start3dcinematic (SP 0x007fbd50): '<' <name> <flags> to every client. Flags start at 0x42;
// looping keeps 0x2, inmemory adds 0x8, a 4th non-zero argument makes it 0xC2; the named SP
// cinematics add 0x1. Nothing is sent while r_reflectionProbeGenerate is set.
static void G_f_start3dcinematic()
{
    if (r_reflectionProbeGenerate->current.enabled)
        return;
    unsigned int flags = 0x42;
    switch (Scr_GetNumParam(SCRIPTINSTANCE_SERVER))
    {
    case 4:
        if (Scr_GetInt(3, SCRIPTINSTANCE_SERVER))
            flags = 0xC2;
        // fall through
    case 3:
        if (Scr_GetInt(2, SCRIPTINSTANCE_SERVER))
            flags |= 8;
        // fall through
    case 2:
        if (!Scr_GetInt(1, SCRIPTINSTANCE_SERVER))
            flags &= ~2u;
        // fall through
    case 1:
        break;
    default:
        Scr_Error("start3DCinematic takes one, two, or three parameters: start3DCinematic(<cinematic name>, <looping>, <inmemory>)\n", 0);
        break;
    }
    static const char *s_fullscreenNames[] =
    {
        "mid_cuba_1", "mid_cuba_3", "mid_vorkuta_2", "mid_vorkuta_3", "mid_flashpoint_1", "mid_flashpoint_2",
        "mid_hue_city_2", "mid_river_1", "wmd_load", "mid_rebirth_2", "int_hudson_explains",
        "int_reznov_disappearing_flashback"
    };
    const char *name = Scr_GetString(0, SCRIPTINSTANCE_SERVER);
    for (unsigned int i = 0; i < ARRAY_COUNT(s_fullscreenNames); ++i)
    {
        if (!I_stricmp(name, s_fullscreenNames[i]))
        {
            flags |= 1;
            break;
        }
    }
    SV_GameSendServerCommand(-1, SV_CMD_RELIABLE, va("%c %s %i", '<', name, flags));
}

// zombies: stop3dcinematic (SP 0x007fbf40).
static void G_f_stop3dcinematic()
{
    if (Scr_GetNumParam(SCRIPTINSTANCE_SERVER))
        Scr_Error("stop3DCinematic takes no parameters: stop3DCinematic()\n", 0);
    SV_GameSendServerCommand(-1, SV_CMD_RELIABLE, va("%c", '.'));
}

// zombies: pause3dcinematic (SP 0x007fbef0).
static void G_f_pause3dcinematic()
{
    SV_GameSendServerCommand(-1, SV_CMD_RELIABLE, va("%c %i", ';', Scr_GetInt(0, SCRIPTINSTANCE_SERVER)));
}

// zombies: getcinematictimeremaining (SP 0x007fbf20): SP's server reads the in-process renderer
// (SP 0x006D9F30); undefined while r_reflectionProbeGenerate is set.
static void G_f_getcinematictimeremaining()
{
    if (r_reflectionProbeGenerate->current.enabled)
        return;
    Scr_AddFloat(R_Cinematic_GetTimeRemaining(), SCRIPTINSTANCE_SERVER);
}

// zombies: setmissiondvar (SP 0x005c31c0): only the dvars in SP's list (.data 0x00B776EC).
static void G_f_setmissiondvar()
{
    static const char *s_missionDvars[] = { "mis_01", "mis_difficulty" };
    const char *name = Scr_GetString(0, SCRIPTINSTANCE_SERVER);
    for (unsigned int i = 0; i < ARRAY_COUNT(s_missionDvars); ++i)
    {
        if (strcmp(name, s_missionDvars[i]))
            continue;
        const dvar_s *dvar = Dvar_FindVar(name);
        if (!dvar)
            Scr_Error(va("Mission dvar has valid name, but was not found: %s\n", name), 0);
        if (dvar->type == DVAR_TYPE_INT)
        {
            Dvar_SetIntByName(name, Scr_GetInt(1, SCRIPTINSTANCE_SERVER));
            Com_Printf(16, " * SetMissionDvar(): \"%s\" set to \"%s\".\n", name, Dvar_GetVariantString(name));
            return;
        }
        if (dvar->type == DVAR_TYPE_STRING)
        {
            Dvar_SetStringByName(name, Scr_GetString(1, SCRIPTINSTANCE_SERVER));
            Com_Printf(16, " * SetMissionDvar(): \"%s\" set to \"%s\".\n", name, Dvar_GetVariantString(name));
            return;
        }
        Scr_Error(va("Mission dvar has valid name, but invalid type: %s is of type %i\n", name, dvar->type), 0);
    }
    Scr_Error(va("Invalid mission dvar name: %s\n", name), 0);
}

// zombies: modelhasphyspreset (SP 0x004db8d0).
static void G_f_modelhasphyspreset()
{
    XModel *model = SV_XModelGet(Scr_GetString(0, SCRIPTINSTANCE_SERVER));
    Scr_AddInt(model->physPreset != 0, SCRIPTINSTANCE_SERVER);
}

// zombies: isturretactive (SP 0x007fc910).
static void G_f_isturretactive()
{
    if (Scr_GetNumParam(SCRIPTINSTANCE_SERVER) != 1)
    {
        Scr_Error("illegal call to isturretactive()\n", 0);
        return;
    }
    gentity_s *turret = Scr_GetEntity(0);
    if (!turret)
    {
        Scr_Error("NULL turret passed to isturretactive", 0);
        return;
    }
    Scr_AddInt(turret->active, SCRIPTINSTANCE_SERVER);
}

// zombies: stopsound (SP 0x00807b30): a stop-alias event at the entity (KB EV_STOP_SOUND_ALIAS = SP 3).
static void G_m_stopsound(scr_entref_t entref)
{
    gentity_s *ent = GetEntity(entref);
    int alias = SND_FindAliasId(Scr_GetString(0, SCRIPTINSTANCE_SERVER));
    if (!alias)
        return;
    gentity_s *tempEnt = G_TempEntity(ent->r.currentOrigin, EV_STOP_SOUND_ALIAS);
    tempEnt->s.loopSoundId = alias;
    tempEnt->s.otherEntityNum = ent->s.number;
}

// zombies: setexploderid (SP 0x00449e20). SP writes s+0x6C; KB's script-mover state has the same
// field, exploderIndex, at s+0x68.
static void G_m_setexploderid(scr_entref_t entref)
{
    gentity_s *ent = GetEntity(entref);
    if (ent->s.eType != ET_SCRIPTMOVER)
    {
        Scr_Error(SCRIPTINSTANCE_SERVER, "SetExploderId called on an ent that's not a script mover.", 0);
        return;
    }
    ent->s.lerp.u.scriptMover.exploderIndex = (__int16)Scr_GetInt(0, SCRIPTINSTANCE_SERVER);
}

// zombies: transmittargetname (SP 0x005e6d50). SP clears SVF_NOCLIENT and sets bit 0x100 of its 16-bit
// svFlags; KB's svFlags is 8 bits, so bit 0x100 is kept here per entity (no KB reader yet).
static bool s_spTransmitTargetname[MAX_GENTITIES_SV];

bool G_SP_TransmitsTargetname(const gentity_s *ent)
{
    return s_spTransmitTargetname[ent->s.number];
}

static void G_m_transmittargetname(scr_entref_t entref)
{
    gentity_s *ent = GetEntity(entref);
    ent->r.svFlags &= ~1u;
    s_spTransmitTargetname[ent->s.number] = true;
}

// zombies: getlinkedent (SP 0x007f2640).
static void G_m_getlinkedent(scr_entref_t entref)
{
    gentity_s *ent = GetEntity(entref);
    if (ent->tagInfo)
        Scr_AddEntity(ent->tagInfo->parent, SCRIPTINSTANCE_SERVER);
}

// zombies: bloodimpact (SP 0x006050d0): none=0, normal=1, hero=2 (SP const slots 0x023A5626 /
// 0x023A5628 / 0x023A560C). KB EV_BLOOD_IMPACTS = SP 0x4F; the client reads pos.trTime / eventParm.
static void G_m_bloodimpact(scr_entref_t entref)
{
    gentity_s *ent = GetEntity(entref);
    if (!Scr_GetNumParam(SCRIPTINSTANCE_SERVER))
    {
        Scr_Error("BloodImpact() called without an argument\n", 0);
        return;
    }
    unsigned int type = Scr_GetConstString(0, SCRIPTINSTANCE_SERVER);
    int value;
    if (type == scr_const.none)
        value = 0;
    else if (type == scr_const.normal)
        value = 1;
    else if (type == scr_const.hero)
        value = 2;
    else
    {
        Scr_Error("BloodImpact() only takes \"normal\", \"hero\" or \"none\" as parameters.\n", 0);
        return;
    }
    gentity_s *tempEnt = G_TempEntity(vec3_origin, EV_BLOOD_IMPACTS);
    tempEnt->s.lerp.pos.trTime = ent->s.number;
    tempEnt->s.eventParm = (unsigned __int16)value;
    Scr_AddInt(1, SCRIPTINSTANCE_SERVER);
}

// zombies: playweapondeatheffects (SP 0x00808210): KB EV_PLAY_WEAPON_DEATH_EFFECTS = SP 0xC1.
static void G_m_playweapondeatheffects(scr_entref_t entref)
{
    if (!Scr_GetNumParam(SCRIPTINSTANCE_SERVER))
        Scr_Error("PlayWeaponDeathEffects <weaponName>.\n", 0);
    char *name = Scr_GetString(0, SCRIPTINSTANCE_SERVER);
    int weapon = G_GetWeaponIndexForName(name);
    if (!weapon)
        Scr_Error(va("PlayWeaponDeathEffects called with unknown weapon name %s\n", name), 0);
    gentity_s *ent = GetEntity(entref);
    float origin[3] = { 0.0f, 0.0f, 0.0f };
    gentity_s *tempEnt = G_TempEntity(origin, EV_PLAY_WEAPON_DEATH_EFFECTS);
    tempEnt->s.otherEntityNum = ent->s.number;
    tempEnt->s.weapon = (unsigned __int16)weapon;
    if (Scr_GetNumParam(SCRIPTINSTANCE_SERVER) == 2)
        tempEnt->s.eventParm = (unsigned __int16)Scr_GetInt(1, SCRIPTINSTANCE_SERVER);
}

// zombies: magicgrenademanual (SP 0x007f3e20): actor-only; the fuse defaults to 5000 ms.
static void G_m_magicgrenademanual(scr_entref_t entref)
{
    gentity_s *ent = GetEntity(entref);
    actor_s *actor = ent->actor;
    if (!actor)
    {
        Scr_Error("MagicGrenadeManual only supports actors.\n", 0);
        return;
    }
    if (Scr_GetNumParam(SCRIPTINSTANCE_SERVER) != 2 && Scr_GetNumParam(SCRIPTINSTANCE_SERVER) != 3)
        Scr_Error("<actor> MagicGrenadeManual <origin> <velocity> [time To Blow (seconds)].\n", 0);
    float origin[3], velocity[3];
    Scr_GetVector(0, origin, SCRIPTINSTANCE_SERVER);
    Scr_GetVector(1, velocity, SCRIPTINSTANCE_SERVER);
    int fuseTime = 5000;
    if (Scr_GetNumParam(SCRIPTINSTANCE_SERVER) == 3)
        fuseTime = (int)((float)Scr_GetFloat(2, SCRIPTINSTANCE_SERVER) * 1000.0f);
    if (!actor->iGrenadeWeaponIndex)
        Scr_Error(va("Actor [%s] doesn't have a grenade weapon set.", SL_ConvertToString(ent->classname, SCRIPTINSTANCE_SERVER)), 0);
    gentity_s *grenade = G_FireGrenade(ent, origin, velocity, actor->iGrenadeWeaponIndex, 0, 1, fuseTime);
    Scr_AddEntity(grenade, SCRIPTINSTANCE_SERVER);
}

// zombies: magicgrenadetype (SP 0x00500400): any entity; the weapon (a grenade) is argument 0, the fuse
// defaults to 5000 ms. The exe's error strings say MagicGrenadeManual (kept); its Scr_Errors fall through.
static void G_m_magicgrenadetype(scr_entref_t entref)
{
    gentity_s *ent = GetEntity(entref);
    int weapon = G_GetWeaponIndexForName((char *)Scr_GetString(0, SCRIPTINSTANCE_SERVER));
    if (BG_GetWeaponDef(weapon)->weapType != WEAPTYPE_GRENADE)
        Scr_Error("MagicGrenadeManual invalid weapon type specified.\n", 0);
    if (Scr_GetNumParam(SCRIPTINSTANCE_SERVER) != 3 && Scr_GetNumParam(SCRIPTINSTANCE_SERVER) != 4)
        Scr_Error("<ent> MagicGrenadeManual <weaponname> <origin> <velocity> [time To Blow (seconds)].\n", 0);
    float origin[3], velocity[3];
    Scr_GetVector(1, origin, SCRIPTINSTANCE_SERVER);
    Scr_GetVector(2, velocity, SCRIPTINSTANCE_SERVER);
    int fuseTime = 5000;
    if (Scr_GetNumParam(SCRIPTINSTANCE_SERVER) == 4)
        fuseTime = (int)((float)Scr_GetFloat(3, SCRIPTINSTANCE_SERVER) * 1000.0f);
    gentity_s *grenade = G_FireGrenade(ent, origin, velocity, weapon, 0, 1, fuseTime);
    Scr_AddEntity(grenade, SCRIPTINSTANCE_SERVER);
}

// zombies: resetmissiledetonationtime (SP 0x007fca60): a missile's nextthink (SP ent+0x180) restarted
// at level.time + the given seconds, or the weapon's fuse (a player's) / AI fuse.
static void G_m_resetmissiledetonationtime(scr_entref_t entref)
{
    gentity_s *ent = GetEntity(entref);
    if (ent->s.eType != ET_MISSILE || !ent->s.weapon)
        return;
    if (Scr_GetNumParam(SCRIPTINSTANCE_SERVER))
    {
        ent->nextthink = level.time + (int)((float)Scr_GetFloat(0, SCRIPTINSTANCE_SERVER) * 1000.0f);
        return;
    }
    const WeaponDef *weapDef = BG_GetWeaponDef(ent->s.weapon);
    if (!weapDef || !weapDef->timedDetonation)
        return;
    if (ent->r.ownerNum.isDefined() && ent->r.ownerNum.ent()->client)
        ent->nextthink = level.time + weapDef->fuseTime;
    else
        ent->nextthink = level.time + weapDef->aiFuseTime;
}

// zombies: getcentroid (SP 0x007f35d0).
static void G_m_getcentroid(scr_entref_t entref)
{
    gentity_s *ent = GetEntity(entref);
    float centroid[3];
    G_EntityCentroid(ent, centroid);
    Scr_AddVector(centroid, SCRIPTINSTANCE_SERVER);
}

// zombies: setscripthintstring (SP 0x007da330): a player method; "" clears it. Player_UpdateCursorHints
// shows it in place of the usable-entity hints (SP 0x00603889).
static void G_m_setscripthintstring(scr_entref_t entref)
{
    gentity_s *ent = 0;
    if (entref.classnum)
    {
        Scr_ObjectError("not an entity", SCRIPTINSTANCE_SERVER);
    }
    else
    {
        ent = &g_entities[entref.entnum];
        if (!ent->client)
            Scr_ObjectError(va("entity %i is not a player", entref.entnum), SCRIPTINSTANCE_SERVER);
    }
    unsigned int clientNum = (unsigned int)(ent->client - level.clients);
    iassert(clientNum < ARRAY_COUNT(s_spScriptHintString));
    if (Scr_GetType(0, SCRIPTINSTANCE_SERVER) == VAR_STRING && !I_stricmp(Scr_GetString(0, SCRIPTINSTANCE_SERVER), ""))
    {
        s_spScriptHintString[clientNum] = 0;
        return;
    }
    char string[1024];
    int index;
    Scr_ConstructMessageString(0, Scr_GetNumParam(SCRIPTINSTANCE_SERVER) - 1, "Hint String", string, sizeof(string));
    if (!G_GetHintStringIndex(&index, string))
        Scr_Error(va("Too many different hintstring values. Max allowed is %i different strings", 96), 0);
    s_spScriptHintString[clientNum] = index + 1;
}

// zombies: the asset-type lookup inside isassetloaded (SP 0x00694550): linear I_stricmp over g_assetNames,
// then DB_FindXAssetEntry (SP 0x007a2a20, type in edi) under the DB hash lock (SP critsect 0x46).
static bool G_SP_IsAssetLoaded(const char *typeName, const char *assetName)
{
    int type;
    XAssetEntryPoolEntry *entry;

    for (type = 0; type < 43; ++type)
    {
        if (!I_stricmp(typeName, g_assetNames[type]))
            break;
    }
    if (type == 43)
    {
        // SP prints on channel 4.
        Com_Printf(4, "type %s is not a valid asset type: see g_assetNames in db_assetnames.h\n", typeName);
        return false;
    }
    Sys_EnterCriticalSection(CRITSECT_DBHASH);
    entry = DB_FindXAssetEntry((XAssetType)type, assetName);
    Sys_LeaveCriticalSection(CRITSECT_DBHASH);
    return entry != NULL;
}

// zombies: isassetloaded( <type>, <name> ) (SP 0x007faad0).
static void G_f_isassetloaded()
{
    const char *typeName = Scr_GetString(0, SCRIPTINSTANCE_SERVER);
    const char *assetName = Scr_GetString(1, SCRIPTINSTANCE_SERVER);
    Scr_AddInt(G_SP_IsAssetLoaded(typeName, assetName), SCRIPTINSTANCE_SERVER);
}

// zombies: <ent> setclientflagasval( <value> ) (SP 0x008064e0): the low 16 bits of eFlags2 (the client flag
// bits of setclientflag) become the value. SP's range check is 0xffff, its message still says "0 - 15".
static void G_m_setclientflagasval(scr_entref_t entref)
{
    gentity_s *ent = GetEntity(entref);
    int value = Scr_GetInt(0, SCRIPTINSTANCE_SERVER);
    if ((unsigned int)value > 0xFFFF)
    {
        Scr_ParamError(0, va("SetClientFlagAsVal: Index %i out of range (0 - %i)\n", value, 15), SCRIPTINSTANCE_SERVER);
        return;
    }
    if (ent->client)
        ent->client->ps.eFlags2 = (ent->client->ps.eFlags2 & 0xFFFF0000) | value;
    else
        ent->s.lerp.eFlags2 = (ent->s.lerp.eFlags2 & 0xFFFF0000) | value;
}

// zombies: the player entity of a player method, as SP's openmainmenu / closemainmenu fetch it.
static gentity_s *G_SP_GetMainMenuPlayer(scr_entref_t entref)
{
    if (entref.classnum)
    {
        Scr_ObjectError("not an entity", SCRIPTINSTANCE_SERVER);
        return nullptr;
    }
    gentity_s *ent = &g_entities[entref.entnum];
    if (!ent->client)
        Scr_ObjectError(va("entity %i is not a player", entref.entnum), SCRIPTINSTANCE_SERVER);
    return ent;
}

// zombies: <player> openmainmenu( <menu> ) (SP 0x007d7160): MP openmenu with the command letter 'b'; the
// client end (CG_SP_OpenMainMenuCommand) opens the SP main menu when the name is "main". Returns 1 / 0.
static void G_m_openmainmenu(scr_entref_t entref)
{
    gentity_s *ent = G_SP_GetMainMenuPlayer(entref);
    if (ent->client->sess.connected == CON_CONNECTED)
    {
        int menuIndex = GScr_GetScriptMenuIndex(Scr_GetString(0, SCRIPTINSTANCE_SERVER));
        SV_GameSendServerCommand(entref.entnum, SV_CMD_RELIABLE, va("%c %i", 'b', menuIndex));
        Scr_AddInt(1, SCRIPTINSTANCE_SERVER);
    }
    else
    {
        Scr_AddInt(0, SCRIPTINSTANCE_SERVER);
    }
}

// zombies: <player> closemainmenu( [menu] ) (SP 0x007d7510): 'Z' with the menu's index, -1 (close all) by
// default. SP reads the menu only when Scr_GetNumParam() > 1 (0x007d757f cmp eax,1 / jbe), so the one-argument
// form also sends -1; ported as the exe does it.
static void G_m_closemainmenu(scr_entref_t entref)
{
    gentity_s *ent = G_SP_GetMainMenuPlayer(entref);
    if (ent->client->sess.connected == CON_CONNECTED)
    {
        int menuIndex = -1;
        if (Scr_GetNumParam(SCRIPTINSTANCE_SERVER) > 1)
            menuIndex = GScr_GetScriptMenuIndex(Scr_GetString(0, SCRIPTINSTANCE_SERVER));
        SV_GameSendServerCommand(entref.entnum, SV_CMD_RELIABLE, va("%c %i", 'Z', menuIndex));
        Scr_AddInt(1, SCRIPTINSTANCE_SERVER);
    }
    else
    {
        Scr_AddInt(0, SCRIPTINSTANCE_SERVER);
    }
}

// Lane x2 per-level state (from G_SP_ResetEntityBuiltins, zombiemode G_InitGame).
static void G_SP_ResetEntityBuiltinsX2()
{
    G_SP_ResetLevelExit();
    s_spPeakSnapshotSize = 0;
    memset(s_spScriptHintString, 0, sizeof(s_spScriptHintString));
    memset(s_spTransmitTargetname, 0, sizeof(s_spTransmitTargetname));
}

// zombies: ropesetflag( rope, flagname, onoff ) (SP 0x007fdd90). Sends every client '#' 0x53 <rope> <flag> <on>
// (the client's rope sub-command 0x53 = Rope_SetFlag). An unknown flag name returns silently. Kino calls it over
// its "techrope01" rope ents, which G_CallSpawn never spawns (SP 0x005177df), so the loop has no element there.
static void G_f_ropesetflag()
{
    static const struct { const char *name; int flag; } flags[] =
    {
        { "keep_ent_anchors", 0x10 }, { "collide", 0x1 }, { "detach_opposite_anchor", 0x20 },
        { "force_update", 0x40 }, { "no_wind", 0x80 }, { "no_lod", 0x100 },
    };
    if (Scr_GetNumParam(SCRIPTINSTANCE_SERVER) != 3)
    {
        Scr_Error("Incorrect number of parameters", SCRIPTINSTANCE_SERVER);
        return;
    }
    int rope = Scr_GetInt(0, SCRIPTINSTANCE_SERVER);
    const char *name = Scr_GetString(1, SCRIPTINSTANCE_SERVER);
    int flag = 0;
    for (const auto &f : flags)
    {
        if (!I_stricmp(name, f.name))
        {
            flag = f.flag;
            break;
        }
    }
    if (!flag)
        return;
    int on = Scr_GetInt(2, SCRIPTINSTANCE_SERVER) != 0;
    SV_GameSendServerCommand(-1, SV_CMD_RELIABLE, va("%c %d %d %d %d", '#', 0x53, rope, flag, on));
}

// zombies: deleterope( rope ) (SP 0x007fe310). Sends every client '#' 0x51 <rope> (the client's rope sub-command 0x51).
// Temple's pack_a_punch.gsc:208 calls it on a rope whose createrope is commented out in the retail script.
static void G_f_deleterope()
{
    if (Scr_GetNumParam(SCRIPTINSTANCE_SERVER) != 1)
        Scr_Error("Incorrect number of parameters.", SCRIPTINSTANCE_SERVER);
    int rope = Scr_GetInt(0, SCRIPTINSTANCE_SERVER);
    if (rope < 0)
        Scr_ParamError(1, "Bad rope index", SCRIPTINSTANCE_SERVER); // SP passes param index 1
    SV_GameSendServerCommand(-1, SV_CMD_RELIABLE, va("%c %d %d", '#', 0x51, rope));
}

// zombies: script node links. SP keeps one record per changed node in an avl map keyed by the node address: the fastfile
// link array (restored on restart) and a PMM-allocated copy the node points at. Record/field names are ours, the layout
// (0x14 bytes) and allocators are SP's (record pool count SP 0x01d04898, map SP 0x01d0489c, restore flag SP 0x01d04878).
struct pathnode_link_override_t
{
    pathnode_t *node;
    pathlink_s *origLinks;
    int origCount;
    pathlink_s *links;
    int count;
};
extern phys_simple_allocator<generic_avl_map_node_t> g_generic_avl_map_node_allocator;
static phys_simple_allocator<pathnode_link_override_t> g_pathnodeLinkAllocator;
static phys_inplace_avl_tree<unsigned int, generic_avl_map_node_t, generic_avl_map_node_t> g_pathnodeLinkMap;
static int g_pathnodeLinkRestore;

// zombies: find or create the node's link record; the node then points at the record's copy (SP 0x005468e0).
static pathnode_link_override_t *Path_GetLinkOverride(pathnode_t *node)
{
    for (generic_avl_map_node_t *t = g_pathnodeLinkMap.m_tree_root; t; )
    {
        if ((unsigned int)node == t->m_avl_key)
            return (pathnode_link_override_t *)t->m_data;
        t = (unsigned int)node >= t->m_avl_key ? t->m_avl_tree_node.m_right : t->m_avl_tree_node.m_left;
    }
    pathnode_link_override_t *rec = g_pathnodeLinkAllocator.allocate();
    rec->node = node;
    rec->origLinks = node->constant.Links;
    rec->origCount = node->constant.totalLinkCount;
    rec->links = (pathlink_s *)PMM_ALLOC(sizeof(pathlink_s) * node->constant.totalLinkCount, 4);
    rec->count = node->constant.totalLinkCount;
    for (int i = 0; i < rec->count; ++i)
        rec->links[i] = rec->origLinks[i];
    node->constant.Links = rec->links;
    generic_avl_map_add(&g_pathnodeLinkMap, rec, (unsigned int)node);
    return rec;
}

// zombies: one-way link from -> to, inserted at index 0; nothing if it exists (SP 0x00679f50).
static void Path_LinkNodes(pathnode_t *from, pathnode_t *to)
{
    pathnode_link_override_t *rec = Path_GetLinkOverride(from);
    unsigned __int16 nodeNum = (unsigned __int16)(to - gameWorldCurrent->path.nodes);
    for (int i = 0; i < rec->count; ++i)
    {
        if (rec->links[i].nodeNum == nodeNum)
            return;
    }
    int newCount = rec->count + 1;
    pathlink_s *links = (pathlink_s *)PMM_ALLOC(sizeof(pathlink_s) * newCount, 4);
    float dz = from->constant.vOrigin[2] - to->constant.vOrigin[2];
    float dx = from->constant.vOrigin[0] - to->constant.vOrigin[0];
    float dy = from->constant.vOrigin[1] - to->constant.vOrigin[1];
    links[0].fDist = sqrtf(dz * dz + dx * dx + dy * dy);
    links[0].nodeNum = nodeNum;
    links[0].disconnectCount = 0;
    links[0].negotiationLink = 0;
    memset(links[0].ubBadPlaceCount, 0, sizeof(links[0].ubBadPlaceCount));
    for (int i = 0; i < rec->count; ++i)
        links[i + 1] = rec->links[i];
    ++from->dynamic.wLinkCount;
    from->constant.Links = links;
    from->constant.totalLinkCount = newCount;
    PMM_FREE((unsigned __int8 *)rec->links, sizeof(pathlink_s) * rec->count, 4);
    rec->links = links;
    rec->count = newCount;
}

// zombies: drop the first from -> to link; nothing if there is none (SP 0x0050e8a0).
static void Path_UnlinkNodes(pathnode_t *from, pathnode_t *to)
{
    pathnode_link_override_t *rec = Path_GetLinkOverride(from);
    unsigned __int16 nodeNum = (unsigned __int16)(to - gameWorldCurrent->path.nodes);
    pathlink_s *found = 0;
    for (int i = 0; i < rec->count && !found; ++i)
    {
        if (rec->links[i].nodeNum == nodeNum)
            found = &rec->links[i];
    }
    if (!found)
        return;
    int newCount = rec->count - 1;
    pathlink_s *links = (pathlink_s *)PMM_ALLOC(sizeof(pathlink_s) * newCount, 4);
    pathlink_s *out = links;
    for (int i = 0; i < rec->count; ++i)
    {
        if (&rec->links[i] != found)
            *out++ = rec->links[i];
    }
    from->constant.Links = links;
    from->constant.totalLinkCount = newCount;
    if (found - rec->links < from->dynamic.wLinkCount)
        --from->dynamic.wLinkCount;
    PMM_FREE((unsigned __int8 *)rec->links, sizeof(pathlink_s) * rec->count, 4);
    rec->links = links;
    rec->count = newCount;
}

// zombies: linknodes( node0, node1 ) (SP 0x00563f90); the param count is read and not checked.
static void G_f_linknodes()
{
    Scr_GetNumParam(SCRIPTINSTANCE_SERVER);
    pathnode_t *from = Scr_GetPathnode(0, SCRIPTINSTANCE_SERVER);
    Path_LinkNodes(from, Scr_GetPathnode(1, SCRIPTINSTANCE_SERVER));
}

// zombies: unlinknodes( node0, node1 ) (SP 0x00522eb0).
static void G_f_unlinknodes()
{
    Scr_GetNumParam(SCRIPTINSTANCE_SERVER);
    pathnode_t *from = Scr_GetPathnode(0, SCRIPTINSTANCE_SERVER);
    Path_UnlinkNodes(from, Scr_GetPathnode(1, SCRIPTINSTANCE_SERVER));
}

// zombies: record destructor: on restart the node gets its fastfile links back (SP 0x004b9ee0).
static void Path_FreeLinkOverride(pathnode_link_override_t *rec)
{
    if (g_pathnodeLinkRestore)
    {
        rec->node->constant.Links = rec->origLinks;
        rec->node->constant.totalLinkCount = rec->origCount;
        rec->node->dynamic.wLinkCount = rec->origCount;
    }
    PMM_FREE((unsigned __int8 *)rec->links, sizeof(pathlink_s) * rec->count, 4);
    g_pathnodeLinkAllocator.free(rec);
}

// zombies: post-order clear of the record map (SP 0x004ecf10 / 0x0063e4a0).
static void Path_ClearLinkOverrides_r(generic_avl_map_node_t *t)
{
    if (!t)
        return;
    Path_ClearLinkOverrides_r(t->m_avl_tree_node.m_left);
    Path_ClearLinkOverrides_r(t->m_avl_tree_node.m_right);
    Path_FreeLinkOverride((pathnode_link_override_t *)t->m_data);
    g_generic_avl_map_node_allocator.free(t);
}

// zombies: G_InitGame's node link reset (SP 0x004587d0, called with G_InitGame's restart argument).
void Path_SP_ResetLinkOverrides(int restart)
{
    g_pathnodeLinkRestore = restart;
    Path_ClearLinkOverrides_r(g_pathnodeLinkMap.m_tree_root);
    g_pathnodeLinkMap.m_tree_root = 0;
}

static void G_f_zerogravityvolumeon();
static void G_f_zerogravityvolumeoff();

const BuiltinFunctionDef g_sp_entity_functions[] =
{
    { "zerogravityvolumeon", G_f_zerogravityvolumeon, 0 },
    { "zerogravityvolumeoff", G_f_zerogravityvolumeoff, 0 },
    { "deleterope", G_f_deleterope, 0 },
    { "linknodes", G_f_linknodes, 0 },
    { "unlinknodes", G_f_unlinknodes, 0 },
    { "ropesetflag", G_f_ropesetflag, 0 },
    { "codespawn", G_f_codespawn, 0 },
    { "isassetloaded", G_f_isassetloaded, 0 },
    { "getspawnerarray", G_f_getspawnerarray, 0 },
    { "getdestructibledefs", G_f_getdestructibledefs, 0 },
    { "getmiscmodels", G_f_getmiscmodels, 0 },
    { "getdynmodels", G_f_getdynmodels, 0 },
    { "getdebugdvar", G_f_getdebugdvar, 1 },
    { "getdebugdvarint", G_f_getdebugdvarint, 1 },
    { "getdebugdvarfloat", G_f_getdebugdvarfloat, 1 },
    { "weaponisgasweapon", G_f_weaponisgasweapon, 0 },
    { "setailimit", G_f_setailimit, 0 },
    { "watersimenable", G_f_watersimenable, 0 },
    { "activateclientexploder", G_f_activateclientexploder, 0 },
    { "deactivateclientexploder", G_f_deactivateclientexploder, 0 },
    { "getallvehiclenodes", G_f_getallvehiclenodes, 0 },
    { "groundtrace", G_f_groundtrace, 0 },
    { "issentient", G_f_issentient, 0 },
    { "weapondualwieldweaponname", G_f_weapondualwieldweaponname, 0 },
    { "missionsuccess", G_f_missionsuccess, 0 },
    { "changelevel", G_SP_ChangeLevel, 0 },
    { "forcelevelend", G_f_forcelevelend, 0 },
    { "codespawnfx", G_f_codespawnfx, 0 },
    { "codeplayloopedfx", G_f_codeplayloopedfx, 0 },
    { "cleanupspawneddynents", G_f_cleanupspawneddynents, 0 },
    { "codespawnvehicle", G_f_codespawnvehicle, 0 },
    { "codespawnturret", G_f_codespawnturret, 0 },
    { "getvehiclenodearray", G_f_getvehiclenodearray, 0 },
    { "allowmipstoload", G_f_allowmipstoload, 0 },
    // zombies: SP's clearlocalizedstrings row points at 0x00803f20, the getarraykeys body.
    { "clearlocalizedstrings", GScr_GetArrayKeys, 0 },
    { "distance2dsquared", G_f_distance2dsquared, 0 },
    { "bulletspread", G_f_bulletspread, 0 },
    { "entsearch", G_f_entsearch, 0 },
    { "getstartorigin", G_f_getstartorigin, 0 },
    { "getstartangles", G_f_getstartangles, 0 },
    { "oktospawn", G_f_oktospawn, 0 },
    { "getsnapshotindexarray", G_f_getsnapshotindexarray, 0 },
    { "snapshotacknowledged", G_f_snapshotacknowledged, 0 },
    { "start3dcinematic", G_f_start3dcinematic, 0 },
    { "stop3dcinematic", G_f_stop3dcinematic, 0 },
    { "pause3dcinematic", G_f_pause3dcinematic, 0 },
    { "getcinematictimeremaining", G_f_getcinematictimeremaining, 0 },
    { "setmissiondvar", G_f_setmissiondvar, 0 },
    { "modelhasphyspreset", G_f_modelhasphyspreset, 0 },
    { "isturretactive", G_f_isturretactive, 0 },
    { nullptr, nullptr, 0 } // end marker, keep last
};
const unsigned int g_sp_entity_function_count = ARRAY_COUNT(g_sp_entity_functions) - 1;

// zombies: iswaitingonsound tests the retained notify string (SP 0x007F4AA0).
static void G_m_iswaitingonsound(scr_entref_t entref)
{
    Scr_AddBool(G_EntSpExt(GetEntity(entref)).soundNotify != 0, SCRIPTINSTANCE_SERVER);
}

// zombies: setplayergravity( gravity ) (SP 0x00806dd0): Scr_GetInt(0); on a player sets the override flag and value
// (SP client array 0x01a53770 +0x910/+0x914); on anything else a silent no-op.
static void G_m_setplayergravity(scr_entref_t entref)
{
    const int gravity = Scr_GetInt(0, SCRIPTINSTANCE_SERVER);
    if (entref.classnum)
        return;
    gentity_s *ent = &g_entities[entref.entnum];
    if (!ent->client)
        return;
    G_SP_PlayerGravityState(ent->client).gravityOverride = true;
    G_SP_PlayerGravityState(ent->client).gravityOverrideValue = gravity;
}

// zombies: clearplayergravity() (SP 0x00806e60): clears the override flag.
static void G_m_clearplayergravity(scr_entref_t entref)
{
    if (entref.classnum)
        return;
    gentity_s *ent = &g_entities[entref.entnum];
    if (ent->client)
        G_SP_PlayerGravityState(ent->client).gravityOverride = false;
}

// zombies: setentgravitytrajectory( moon ) (SP 0x008070e0): only an entity already falling with TR_GRAVITY (6) or
// TR_MOON_GRAVITY (15) switches: 0 -> 6, 1 -> 15. trBase / trTime are not rebased.
static void G_m_setentgravitytrajectory(scr_entref_t entref)
{
    const int moon = Scr_GetInt(0, SCRIPTINSTANCE_SERVER);
    if (entref.classnum)
        return;
    gentity_s *ent = &g_entities[entref.entnum];
    if (ent->s.lerp.pos.trType != TR_GRAVITY && ent->s.lerp.pos.trType != TR_MOON_GRAVITY)
        return;
    if (moon == 0)
        ent->s.lerp.pos.trType = TR_GRAVITY;
    else if (moon == 1)
        ent->s.lerp.pos.trType = TR_MOON_GRAVITY;
}

// zombies: setlowready( on ) (SP 0x007da1e0). SP sets ps.pm_flags 0x200000 (KB's MP uses that bit for something
// else, so the flag is kept in SPPlayerBuiltinState) and ends a dive like Dtp_End (SP 0x0075ba20; KB's dive bit is
// 0x400000). NOT PORTED: the SP PM readers (lowready weapon state SP 0x00767cd0 from PM_Weapon, fire gate SP 0x00769cd0).
static void G_m_setlowready(scr_entref_t entref)
{
    const int on = Scr_GetInt(0, SCRIPTINSTANCE_SERVER);
    gentity_s *ent = 0;
    if (entref.classnum)
        Scr_ObjectError("not an entity", SCRIPTINSTANCE_SERVER);
    else
        ent = &g_entities[entref.entnum];
    if (!ent->client)
    {
        Scr_ObjectError(va("entity %i is not a player", entref.entnum), SCRIPTINSTANCE_SERVER);
        return;
    }
    playerState_s *ps = &ent->client->ps;
    G_SP_PlayerBuiltinState(ps->clientNum).lowReady = on != 0;
    if (on && (ps->pm_flags & 0x400000))
    {
        ps->pm_flags &= ~0x400000u;
        ps->jumpTime = 0;
        ps->velocity[0] = 0.0f;
        ps->velocity[1] = 0.0f;
        ps->velocity[2] = 0.0f;
        ps->lastDtpEnd = ps->commandTime;
        ps->sprintState.sprintButtonUpRequired = 0;
    }
}

// zombies: zero gravity volumes (zombie_moon). SP builds the table at level load (SP 0x00561830 -> 0x00488070)
// from the map's info_volume entities whose script_string contains "gravity": name = targetname (32 bytes,
// 0x02429480), bounds of the brush model *N (0x02428e80 / 0x02428e8c), airlock flag if it contains "airlock"
// (0x02429fa0), enabled (0x02429d00), count 0x02428500, max 64. KB parses the same entity string.
// NOT PORTED: the only reader, CG_PlayImpactFx (SP 0x00798a42 -> point test 0x00633b30) swapping the impact effect
// surface inside an enabled volume.
struct GravityVolume
{
    char name[32];
    float mins[3];
    float maxs[3];
    bool airlock;
    bool enabled;
};
static GravityVolume s_gravityVolumes[64];
static int s_gravityVolumeCount;

static const char *ReadQuoted(const char *p, char *out, int size)
{
    int n = 0;
    while (*p && *p != '"')
    {
        if (n < size - 1)
            out[n++] = *p;
        ++p;
    }
    out[n] = 0;
    return *p ? p + 1 : p;
}

void G_SP_InitGravityVolumes()
{
    s_gravityVolumeCount = 0;
    memset(s_gravityVolumes, 0, sizeof(s_gravityVolumes));
    const char *p = CM_EntityString();
    char key[64], value[256], classname[64], scriptString[256], targetname[64], model[64];
    while (p && *p)
    {
        if (*p != '{')
        {
            ++p;
            continue;
        }
        ++p;
        classname[0] = scriptString[0] = targetname[0] = model[0] = 0;
        while (*p && *p != '}')
        {
            if (*p != '"')
            {
                ++p;
                continue;
            }
            p = ReadQuoted(p + 1, key, sizeof(key));
            while (*p && *p != '"' && *p != '}')
                ++p;
            if (*p != '"')
                break;
            p = ReadQuoted(p + 1, value, sizeof(value));
            if (!I_stricmp(key, "classname"))
                I_strncpyz(classname, value, sizeof(classname));
            else if (!I_stricmp(key, "script_string"))
                I_strncpyz(scriptString, value, sizeof(scriptString));
            else if (!I_stricmp(key, "targetname"))
                I_strncpyz(targetname, value, sizeof(targetname));
            else if (!I_stricmp(key, "model"))
                I_strncpyz(model, value, sizeof(model));
        }
        if (*p == '}')
            ++p;
        if (strcmp(classname, "info_volume") || !strstr(scriptString, "gravity") || model[0] != '*'
            || s_gravityVolumeCount >= (int)ARRAY_COUNT(s_gravityVolumes))
            continue;
        GravityVolume &v = s_gravityVolumes[s_gravityVolumeCount++];
        I_strncpyz(v.name, targetname, sizeof(v.name));
        CM_ModelBounds(atoi(model + 1), v.mins, v.maxs);
        v.airlock = strstr(scriptString, "airlock") != nullptr;
    }
}

// zombies: zerogravityvolumeon / zerogravityvolumeoff( name ) (SP 0x007f04d0 / 0x007f0520): strcmp against every
// volume name, set enabled on each match.
static void G_SP_SetGravityVolume(bool enabled)
{
    const char *name = Scr_GetString(0, SCRIPTINSTANCE_SERVER);
    for (int i = 0; i < s_gravityVolumeCount; ++i)
        if (!strcmp(s_gravityVolumes[i].name, name))
            s_gravityVolumes[i].enabled = enabled;
}

static void G_f_zerogravityvolumeon()
{
    G_SP_SetGravityVolume(true);
}

static void G_f_zerogravityvolumeoff()
{
    G_SP_SetGravityVolume(false);
}

const BuiltinMethodDef g_sp_entity_methods[] =
{
    { "iswaitingonsound", G_m_iswaitingonsound, 0 },
    { "setplayercollision", G_m_setplayercollision, 0 },
    { "setclientflagasval", G_m_setclientflagasval, 0 },
    { "openmainmenu", G_m_openmainmenu, 0 },
    { "closemainmenu", G_m_closemainmenu, 0 },
    { "setspawnerteam", G_m_setspawnerteam, 0 },
    { "setteamforentity", G_m_setteamforentity, 0 },
    { "haseyes", G_m_haseyes, 0 },
    { "issprinting", G_m_issprinting, 0 },
    { "setzombieshrink", G_m_setzombieshrink, 0 },
    { "setenginevolume", G_m_enginevolume_null, 0 },
    { "getenginevolume", G_m_enginevolume_null, 0 },
    { "useweaponhidetags", G_m_useweaponhidetags, 0 },
    { "stopsounds", G_m_stopsounds, 0 },
    { "trackscriptstate", G_m_trackscriptstate, 0 },
    { "dontinterpolate", G_m_dontinterpolate, 0 },
    { "getdestructiblename", G_m_getdestructiblename, 0 },
    { "gib", G_m_gib, 0 },
    { "setphysparams", G_m_setphysparams, 0 },
    { "addvehicletocompass", G_m_addvehicletocompass, 0 },
    { "removevehiclefromcompass", G_m_removevehiclefromcompass, 0 },
    { "setvehicleattachments", G_m_setvehicleattachments, 0 },
    { "stopsound", G_m_stopsound, 0 },
    { "setexploderid", G_m_setexploderid, 0 },
    { "transmittargetname", G_m_transmittargetname, 0 },
    { "getlinkedent", G_m_getlinkedent, 0 },
    { "bloodimpact", G_m_bloodimpact, 0 },
    { "playweapondeatheffects", G_m_playweapondeatheffects, 0 },
    { "magicgrenademanual", G_m_magicgrenademanual, 0 },
    { "magicgrenadetype", G_m_magicgrenadetype, 0 },
    { "resetmissiledetonationtime", G_m_resetmissiledetonationtime, 0 },
    { "getcentroid", G_m_getcentroid, 0 },
    { "setscripthintstring", G_m_setscripthintstring, 0 },
    { "setplayergravity", G_m_setplayergravity, 0 },
    { "clearplayergravity", G_m_clearplayergravity, 0 },
    { "setentgravitytrajectory", G_m_setentgravitytrajectory, 0 },
    { "setlowready", G_m_setlowready, 0 },
    { nullptr, nullptr, 0 } // end marker, keep last
};
const unsigned int g_sp_entity_method_count = ARRAY_COUNT(g_sp_entity_methods) - 1;
