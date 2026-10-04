#include "radiant_remote.h"
#include <clientscript/cscr_stringlist.h>
#include <game_mp/g_main_mp.h>
#include <game_mp/g_spawn_mp.h>
#include <game_mp/actor_mp.h>
#include <clientscript/scr_const.h>
#include <game_mp/g_utils_mp.h>
#include "dobj_management.h"
#include <cgame_mp/cg_local_mp.h>
#include <gfx_d3d/r_init.h>
#include <universal/com_math_anglevectors.h>
#include <gfx_d3d/r_dvars.h>
#include <clientscript/cscr_vm.h>
#include <game/g_load_utils.h>
#include <cgame/cg_scr_main.h>
#include <xanim/xmodel.h>
#include <cgame/cg_drawtools.h>
#include <game/g_debug.h>

int g_selected_misc_model = -1;
int g_selected_corona = -1;
int gSelectedScriptStruct;

bool runningSavedCommands;
int savedCommandCount;
RadiantCommand savedCommands[128];
RadaintToGameMapping gObjectMapping[128];
int gObjectMappingCount;
gentity_s *g_radiant_selected_ent;
int cgSelectedScriptStruct;

int gCommandCount;
RadiantCommand gCommands[24];
int cgCommandCount;
RadiantCommand cgCommands[24];

static void __cdecl NULLSUB(const RadiantCommand *command, SpawnVar *spawnVar)
{

}


const RadiantCommandProcessor gCommandProcessors[9] =
{
  { &G_ProcessPathnodeCommand, &G_ClearSelectedPathNode, NULL, "node_" },
  {
    &G_ProcessVehicleNodeCommand,
    &G_ClearSelectedVehicleNode,
    NULL,
    "info_vehicle_node"
  },
  {
    &G_ProcessVehicleNodeCommand,
    &G_ClearSelectedVehicleNode,
    NULL,
    "info_vehicle_node_rotate"
  },
  { &G_ProcessMiscModelCommand, &G_ClearSelectedMiscModel, NULL, "misc_model" },
  {
    &G_ProcessScriptStructCommand,
    &G_ClearSelectedScriptStruct,
    NULL,
    "script_struct"
  },
  { &G_ProcessCoronaCommand, &G_ClearSelectedCorona, NULL, "info_corona" },
  { &NULLSUB, &BLOPS_NULLSUB, NULL, "light" },
  {
    &G_ProcessEntityCommand,
    &G_ClearSelectedEntity,
    &IsEntityType,
    ""
  },
  { NULL, NULL, NULL, NULL }
};

const RadiantCommandProcessor cgCommandProcessors[2] =
{
  {
    &CG_ProcessScriptStructCommand,
    &CG_ClearSelectedScriptStruct,
    NULL,
    "script_struct"
  },
  { NULL, NULL, NULL, NULL }
};





char *__cdecl GetPairValue(const SpawnVar *spawnVar, const char *key)
{
    int i; // [esp+0h] [ebp-4h]

    for ( i = 0; i < spawnVar->numSpawnVars; ++i )
    {
        if ( !I_stricmp(key, spawnVar->spawnVars[i][0]) )
            return spawnVar->spawnVars[i][1];
    }
    return 0;
}

void __cdecl AddSavedCommand(const RadiantCommand *command)
{
    int i; // [esp+0h] [ebp-4h]

    if ( command->liveUpdateId && !runningSavedCommands )
    {
        if ( command->type != RADIANT_COMMAND_DELETE )
        {
            for ( i = 0; i < savedCommandCount; ++i )
            {
                if ( savedCommands[i].liveUpdateId == command->liveUpdateId )
                {
                    I_strncpyz(savedCommands[i].strCommand, command->strCommand, 512);
                    if ( savedCommands[i].type != RADIANT_COMMAND_CREATE )
                        savedCommands[i].type = command->type;
                    if ( savedCommands[i].type == RADIANT_COMMAND_UPDATE_SELECTED )
                        savedCommands[i].type = RADIANT_COMMAND_UPDATE;
                    return;
                }
            }
        }
        if ( savedCommandCount >= 128 )
        {
            Com_Printf(
                5,
                "Radiant Live Update: Ran out of saved commands, not everything will be updated after a map restart\n");
        }
        else
        {
            I_strncpyz(savedCommands[savedCommandCount].strCommand, command->strCommand, 512);
            savedCommands[savedCommandCount].liveUpdateId = command->liveUpdateId;
            savedCommands[savedCommandCount++].type = command->type;
        }
    }
}

void __cdecl RunSavedRadiantCmds()
{
    int i; // [esp+0h] [ebp-4h]

    runningSavedCommands = 1;
    for ( i = 0; i < savedCommandCount; ++i )
        G_ProcessRadiantCmd(&savedCommands[i]);
    runningSavedCommands = 0;
}

void __cdecl G_AssignGameIdMapping(int liveUpdateId, int gameId)
{
    if ( liveUpdateId > 0 )
        gObjectMapping[liveUpdateId - 1].gameId = gameId;
}

int __cdecl G_GetGameIdMapping(int liveUpdateId)
{
    if ( liveUpdateId <= 0 )
        return -1;
    else
        return gObjectMapping[liveUpdateId - 1].gameId;
}

void __cdecl CG_AssignGameIdMapping(int liveUpdateId, int gameId)
{
    if ( liveUpdateId > 0 )
        gObjectMapping[liveUpdateId - 1].cg_gameId = gameId;
}

int __cdecl CG_GetGameIdMapping(int liveUpdateId)
{
    if ( liveUpdateId <= 0 )
        return -1;
    else
        return gObjectMapping[liveUpdateId - 1].cg_gameId;
}

void __cdecl RadiantRemoteInit()
{
    savedCommandCount = 0;
    gObjectMappingCount = 0;
}

gentity_s *__cdecl FindEntity(const SpawnVar *spawnVar, int gameId)
{
    gentity_s *ent; // [esp+14h] [ebp-2Ch]
    int i; // [esp+18h] [ebp-28h]
    unsigned __int16 classname_hash; // [esp+1Ch] [ebp-24h]
    float origin[3]; // [esp+20h] [ebp-20h] BYREF
    int twodimcheck; // [esp+2Ch] [ebp-14h]
    const char *classname; // [esp+30h] [ebp-10h]
    int actor; // [esp+34h] [ebp-Ch]
    const char *origin_string; // [esp+38h] [ebp-8h]
    gentity_s *bestEnt; // [esp+3Ch] [ebp-4h]

    classname = GetPairValue(spawnVar, "classname");
    actor = I_stristr(classname, "actor_") != 0;
    twodimcheck = I_stristr(classname, "spawn") != 0;
    twodimcheck |= I_stristr(classname, "start") != 0;
    classname_hash = SL_GetLowercaseString(classname, 0, SCRIPTINSTANCE_SERVER);
    origin_string = GetPairValue(spawnVar, "origin");
    if ( origin_string )
        sscanf(origin_string, "%f %f %f", origin, &origin[1], &origin[2]);
    bestEnt = 0;
    for ( i = 0; i < 1024; ++i )
    {
        ent = &g_entities[i];
        if ( ent->r.inuse )
        {
            if ( actor )
            {
                if ( origin_string
                    && (float)((float)((float)(ent->r.currentOrigin[0] - origin[0]) * (float)(ent->r.currentOrigin[0] - origin[0]))
                                     + (float)((float)(ent->r.currentOrigin[1] - origin[1]) * (float)(ent->r.currentOrigin[1] - origin[1]))) < 0.1
                    && (ent->s.eType == 17 || ent->s.eType == 18 || ent->s.eType == 19) )
                {
                    bestEnt = &g_entities[i];
                }
            }
            else if ( ent->classname == classname_hash && origin_string )
            {
                if ( twodimcheck && VecNCompareCustomEpsilon(origin, ent->r.currentOrigin, 0.1, 2) )
                {
                    bestEnt = &g_entities[i];
                }
                else if ( VecNCompareCustomEpsilon(origin, ent->r.currentOrigin, 0.1, 3) )
                {
                    bestEnt = &g_entities[i];
                }
            }
        }
    }
    if ( gameId > 0 && !bestEnt )
    {
        bestEnt = &g_entities[gameId];
        if ( !bestEnt->r.inuse )
            return 0;
    }
    return bestEnt;
}

bool __cdecl IsEntityType(const SpawnVar *spawnVar)
{
    char *classname; // [esp+0h] [ebp-4h]

    classname = GetPairValue(spawnVar, "classname");
    return I_stricmp(classname, "worldspawn")
            && I_stricmp(classname, "misc_prefab")
            && I_stricmp(classname, "script_brushmodel")
            && I_stricmp(classname, "rope")
            && I_stricmp(classname, "spawn_manager")
            && I_strncmp(classname, "dyn_", 4);
}

void __cdecl G_ClearSelectedEntity()
{
    g_radiant_selected_ent = 0;
}

void __cdecl G_ProcessEntityCommand(const RadiantCommand *command, SpawnVar *spawnVar)
{
    char *v2; // eax
    char *v3; // eax
    char *v4; // eax
    DObj *obj; // [esp+0h] [ebp-20h]
    bool bShouldUpdateSpawns; // [esp+7h] [ebp-19h]
    RadiantCommandType commandType; // [esp+8h] [ebp-18h]
    char *classname; // [esp+Ch] [ebp-14h]
    gentity_s *ent; // [esp+18h] [ebp-8h]
    gentity_s *enta; // [esp+18h] [ebp-8h]
    gentity_s *entb; // [esp+18h] [ebp-8h]
    int gameId; // [esp+1Ch] [ebp-4h]

    ent = 0;
    commandType = command->type;
    classname = GetPairValue(spawnVar, "classname");
    gameId = G_GetGameIdMapping(command->liveUpdateId);
    if ( commandType == RADIANT_COMMAND_CREATE )
    {
        G_CallSpawn(spawnVar);
        enta = FindEntity(spawnVar, gameId);
        g_radiant_selected_ent = enta;
        if ( enta )
        {
            G_AssignGameIdMapping(command->liveUpdateId, enta->s.number);
            AddSavedCommand(command);
            if ( enta->actor )
            {
                Actor_FinishSpawning(enta->actor);
                Actor_InitAnimScript(enta->actor);
            }
            if ( G_IsSpawnPoint(classname) )
                Scr_UpdateSpawnPoints();
        }
    }
    else if ( commandType == RADIANT_COMMAND_DELETE )
    {
        entb = FindEntity(spawnVar, gameId);
        if ( entb )
        {
            G_AssignGameIdMapping(command->liveUpdateId, entb->s.number);
            AddSavedCommand(command);
            v2 = SL_ConvertToString(entb->classname, SCRIPTINSTANCE_SERVER);
            bShouldUpdateSpawns = G_IsSpawnPoint(v2);
            if ( entb->actor )
            {
                Actor_ClearPath(entb->actor);
                Scr_Notify(entb, scr_const.death, 0);
                G_FreeEntityDelay(entb);
            }
            else
            {
                G_FreeEntity(entb);
            }
            if ( bShouldUpdateSpawns )
                Scr_UpdateSpawnPoints();
        }
    }
    else
    {
        switch ( commandType )
        {
            case RADIANT_COMMAND_UPDATE:
                ent = FindEntity(spawnVar, gameId);
                break;
            case RADIANT_COMMAND_UPDATE_SELECTED:
                ent = g_radiant_selected_ent;
                break;
            case RADIANT_COMMAND_SELECT:
                ent = FindEntity(spawnVar, gameId);
                g_radiant_selected_ent = ent;
                if ( !ent )
                    Com_Printf(
                        5,
                        "Radiant Live Update: Can't find entity %s.    Maps out of sync (Radiant/Game), re-bsp!\n",
                        classname);
                break;
        }
        v3 = strstr(classname, "actor_");
        if ( ent )
        {
            G_ParseEntityFields(spawnVar, ent, v3 != 0);
            if ( ent->s.eType == 17 || ent->s.eType == 18 )
                Actor_droptofloor(ent);
            G_SetupDestructible(ent, spawnVar);
            G_DObjUpdate(ent);
            obj = Com_GetServerDObj(ent->s.number);
            if ( obj && !ent->sentient )
            {
                ent->r.contents |= DObjGetContents(obj);
                DObjCalcBounds(obj, ent->r.mins, ent->r.maxs);
            }
            G_AssignGameIdMapping(command->liveUpdateId, ent->s.number);
            v4 = SL_ConvertToString(ent->classname, SCRIPTINSTANCE_SERVER);
            if ( G_IsSpawnPoint(v4) )
                Scr_UpdateSpawnPoints();
            AddSavedCommand(command);
        }
    }
}

bool __cdecl G_IsSpawnPoint(const char *classname)
{
    return I_stricmp("mp_dm_spawn", classname)
            || I_strnicmp("mp_wager_spawn", classname, 14)
            || I_strnicmp("mp_tdm_spawn", classname, 12)
            || I_strnicmp("mp_ctf_spawn", classname, 12)
            || I_strnicmp("mp_dom_spawn", classname, 12)
            || I_strnicmp("mp_sab_spawn", classname, 12)
            || I_strnicmp("mp_sd_spawn", classname, 11)
            || I_strnicmp("mp_twar_spawn", classname, 13)
            || I_stricmp("mp_uspawn_point", classname);
}

void __cdecl G_ProcessCameraCommand(SpawnVar *spawnVar)
{
    int v1; // eax
    cg_s *cgameGlob; // [esp+0h] [ebp-Ch]
    char *angles_string; // [esp+4h] [ebp-8h]
    char *origin_string; // [esp+8h] [ebp-4h]

    origin_string = GetPairValue(spawnVar, "origin");
    angles_string = GetPairValue(spawnVar, "angles");
    v1 = RETURN_ZERO32();
    cgameGlob = CG_GetLocalClientGlobals(v1);
    if ( cgameGlob )
    {
        if ( origin_string )
            sscanf(
                origin_string,
                "%f %f %f",
                cgameGlob->radiantCameraOrigin,
                &cgameGlob->radiantCameraOrigin[1],
                &cgameGlob->radiantCameraOrigin[2]);
        if ( angles_string )
            sscanf(
                angles_string,
                "%f %f %f",
                cgameGlob->radiantCameraAngles,
                &cgameGlob->radiantCameraAngles[1],
                &cgameGlob->radiantCameraAngles[2]);
        //LODWORD(cgameGlob->radiantCameraAngles[0]) ^= _mask__NegFloat_;
        cgameGlob->radiantCameraAngles[0] = -cgameGlob->radiantCameraAngles[0];
        cgameGlob->radiantCamInUse = 1;
    }
}

unsigned int __cdecl G_FindMiscModel(const float *origin, int gameId)
{
    unsigned int i; // [esp+Ch] [ebp-4h]

    for ( i = 0; i < rgp.world->dpvs.smodelCount; ++i )
    {
        if ( VecNCompareCustomEpsilon(rgp.world->dpvs.smodelDrawInsts[i].placement.origin, origin, 0.1, 3) )
            return i;
    }
    return gameId;
}

void __cdecl G_ProcessMiscModelCommand(const RadiantCommand *command, SpawnVar *spawnVar)
{
    int v2; // eax
    GfxStaticModelDrawInst *smodelDrawInst; // [esp+18h] [ebp-60h]
    RadiantCommandType commandType; // [esp+1Ch] [ebp-5Ch]
    float origin[3]; // [esp+20h] [ebp-58h] BYREF
    cg_s *cgameGlob; // [esp+2Ch] [ebp-4Ch]
    const char *angles_string; // [esp+30h] [ebp-48h]
    const char *scale_string; // [esp+34h] [ebp-44h]
    float angles[3]; // [esp+38h] [ebp-40h] BYREF
    const char *origin_string; // [esp+44h] [ebp-34h]
    float scale; // [esp+48h] [ebp-30h]
    float axis[3][3]; // [esp+4Ch] [ebp-2Ch] BYREF
    int gameId; // [esp+70h] [ebp-8h]
    int misc_model_index; // [esp+74h] [ebp-4h]

    origin_string = GetPairValue(spawnVar, "origin");
    angles_string = GetPairValue(spawnVar, "angles");
    scale_string = GetPairValue(spawnVar, "modelscale");
    scale = 1.0f;
    memset(origin, 0, sizeof(origin));
    memset(angles, 0, sizeof(angles));
    v2 = RETURN_ZERO32();
    cgameGlob = CG_GetLocalClientGlobals(v2);
    if ( cgameGlob && rgp.world )
    {
        if ( origin_string )
            sscanf(origin_string, "%f %f %f", origin, &origin[1], &origin[2]);
        if ( angles_string )
            sscanf(angles_string, "%f %f %f", angles, &angles[1], &angles[2]);
        if ( scale_string )
            scale = atof(scale_string);
        commandType = command->type;
        gameId = G_GetGameIdMapping(command->liveUpdateId);
        if ( commandType == RADIANT_COMMAND_SELECT )
        {
            g_selected_misc_model = G_FindMiscModel(origin, gameId);
            if ( g_selected_misc_model >= 0 )
                G_AssignGameIdMapping(command->liveUpdateId, g_selected_misc_model);
            else
                Com_Printf(5, "Radiant Live Update: Can't find misc_model.    Maps out of sync (Radiant/Game), re-bsp!\n");
        }
        if ( commandType == RADIANT_COMMAND_UPDATE_SELECTED )
        {
            misc_model_index = g_selected_misc_model;
        }
        else if ( commandType == RADIANT_COMMAND_UPDATE )
        {
            misc_model_index = G_FindMiscModel(origin, gameId);
        }
        else
        {
            misc_model_index = -1;
        }
        if ( misc_model_index >= 0 )
        {
            smodelDrawInst = &rgp.world->dpvs.smodelDrawInsts[misc_model_index];
            smodelDrawInst->placement.origin[0] = origin[0];
            smodelDrawInst->placement.origin[1] = origin[1];
            smodelDrawInst->placement.origin[2] = origin[2];
            AnglesToAxis(angles, axis);
            smodelDrawInst->placement.axis[0][0] = axis[0][0];
            *(_QWORD *)&smodelDrawInst->placement.axis[0][1] = *(_QWORD *)&axis[0][1];
            smodelDrawInst->placement.axis[1][0] = axis[1][0];
            smodelDrawInst->placement.axis[1][1] = axis[1][1];
            smodelDrawInst->placement.axis[1][2] = axis[1][2];
            smodelDrawInst->placement.axis[2][0] = axis[2][0];
            smodelDrawInst->placement.axis[2][1] = axis[2][1];
            smodelDrawInst->placement.axis[2][2] = axis[2][2];
            smodelDrawInst->placement.scale = scale;
            Dvar_SetBool((dvar_s *)r_cacheSModelLighting, 1);
        }
    }
}

void __cdecl G_ClearSelectedMiscModel()
{
    g_selected_misc_model = -1;
    Dvar_SetBool((dvar_s *)r_cacheSModelLighting, 0);
}

unsigned int __cdecl G_FindCorona(const float *origin, int gameId)
{
    unsigned int i; // [esp+Ch] [ebp-4h]

    for ( i = 0; i < rgp.world->coronaCount; ++i )
    {
        if ( VecNCompareCustomEpsilon(rgp.world->coronas[i].origin, origin, 0.1, 3) )
            return i;
    }
    return gameId;
}

void __cdecl G_ProcessCoronaCommand(const RadiantCommand *command, SpawnVar *spawnVar)
{
    int v2; // eax
    GfxLightCorona *corona; // [esp+4h] [ebp-44h]
    signed int corona_index; // [esp+8h] [ebp-40h]
    RadiantCommandType commandType; // [esp+Ch] [ebp-3Ch]
    float origin[3]; // [esp+10h] [ebp-38h] BYREF
    cg_s *cgameGlob; // [esp+1Ch] [ebp-2Ch]
    const char *intensity_string; // [esp+20h] [ebp-28h]
    float radius; // [esp+24h] [ebp-24h]
    const char *color_string; // [esp+28h] [ebp-20h]
    float intensity; // [esp+2Ch] [ebp-1Ch]
    const char *radius_string; // [esp+30h] [ebp-18h]
    const char *origin_string; // [esp+34h] [ebp-14h]
    float color[3]; // [esp+38h] [ebp-10h] BYREF
    int gameId; // [esp+44h] [ebp-4h]

    origin_string = GetPairValue(spawnVar, "origin");
    color_string = GetPairValue(spawnVar, "color");
    radius_string = GetPairValue(spawnVar, "radius");
    intensity_string = GetPairValue(spawnVar, "intensity");
    radius = 20.0f;
    intensity = 1.0f;
    memset(origin, 0, sizeof(origin));
    memset(color, 0, sizeof(color));
    v2 = RETURN_ZERO32();
    cgameGlob = CG_GetLocalClientGlobals(v2);
    if ( cgameGlob && rgp.world )
    {
        if ( origin_string )
            sscanf(origin_string, "%f %f %f", origin, &origin[1], &origin[2]);
        if ( color_string )
            sscanf(color_string, "%f %f %f", color, &color[1], &color[2]);
        if ( radius_string )
            radius = atof(radius_string);
        if ( intensity_string )
            intensity = atof(intensity_string);
        commandType = command->type;
        gameId = G_GetGameIdMapping(command->liveUpdateId);
        if ( commandType )
        {
            if ( commandType == RADIANT_COMMAND_UPDATE_SELECTED )
            {
                corona_index = g_selected_corona;
            }
            else if ( commandType == RADIANT_COMMAND_UPDATE )
            {
                corona_index = G_FindCorona(origin, gameId);
            }
            else
            {
                corona_index = -1;
            }
        }
        else
        {
            g_selected_corona = G_FindCorona(origin, gameId);
            corona_index = g_selected_corona;
            if ( g_selected_corona >= 0 )
                G_AssignGameIdMapping(command->liveUpdateId, g_selected_corona);
            else
                Com_Printf(5, "Radiant Live Update: Can't find misc_model.    Maps out of sync (Radiant/Game), re-bsp!\n");
        }
        if ( corona_index >= 0 )
        {
            corona = &rgp.world->coronas[corona_index];
            corona->origin[0] = origin[0];
            corona->origin[1] = origin[1];
            corona->origin[2] = origin[2];
            corona->color[0] = color[0];
            corona->color[1] = color[1];
            corona->color[2] = color[2];
            corona->radius = radius;
            corona->intensity = intensity;
        }
    }
}

void __cdecl G_ClearSelectedCorona()
{
    g_selected_corona = -1;
}

void __cdecl G_NotifyScriptsOfSelectedScriptStruct(int structId)
{
    unsigned int numparams; // [esp+0h] [ebp-8h]
    unsigned int t; // [esp+4h] [ebp-4h]

    gSelectedScriptStruct = structId;
    numparams = 1;
    if (structId)
    {
        Scr_AddObject(structId, SCRIPTINSTANCE_SERVER);
        numparams = 2;
    }
    Scr_AddConstString(scr_const.obstacle, SCRIPTINSTANCE_SERVER);
    t = Scr_ExecThread(SCRIPTINSTANCE_SERVER, g_scr_data.levelnotify, numparams);
    Scr_FreeThread(t, SCRIPTINSTANCE_SERVER);
}

unsigned int __cdecl G_FindStruct(SpawnVar *spawnVar, int gameId)
{
    float origin[3]; // [esp+0h] [ebp-18h] BYREF
    const char *origin_string; // [esp+Ch] [ebp-Ch]
    const char *scriptLabel; // [esp+10h] [ebp-8h] BYREF
    unsigned int structId; // [esp+14h] [ebp-4h]

    if ( !spawnVar->spawnVarsValid
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\radiant_remote.cpp",
                    750,
                    0,
                    "%s",
                    "spawnVar.spawnVarsValid") )
    {
        __debugbreak();
    }
    if ( !g_scr_data.findstruct
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\radiant_remote.cpp",
                    751,
                    0,
                    "%s",
                    "g_scr_data.findstruct") )
    {
        __debugbreak();
    }
    G_SpawnString(spawnVar, "script_label", "", &scriptLabel);
    if ( !I_stricmp(scriptLabel, "random") || !I_stricmp(scriptLabel, "looper") || !I_stricmp(scriptLabel, "line_emitter") )
        return 0;
    memset(origin, 0, sizeof(origin));
    origin_string = GetPairValue(spawnVar, "origin");
    if ( origin_string )
        sscanf(origin_string, "%f %f %f", origin, &origin[1], &origin[2]);
    Scr_AddVector(origin, SCRIPTINSTANCE_SERVER);
    Scr_AddExecThread(SCRIPTINSTANCE_SERVER, g_scr_data.findstruct, 1u);
    structId = 0;
    if ( gameId > 0 )
        structId = gameId;
    if ( Scr_GetType(0, SCRIPTINSTANCE_SERVER) )
        structId = Scr_GetObject(0, SCRIPTINSTANCE_SERVER);
    Scr_ClearOutParams(SCRIPTINSTANCE_SERVER);
    return structId;
}

void __cdecl G_ProcessScriptStructCommand(const RadiantCommand *command, SpawnVar *spawnVar)
{
    RadiantCommandType commandType; // [esp+0h] [ebp-14h]
    unsigned int index; // [esp+4h] [ebp-10h]
    int i; // [esp+8h] [ebp-Ch]
    int gameId; // [esp+Ch] [ebp-8h]
    unsigned int script_struct; // [esp+10h] [ebp-4h]
    unsigned int script_structa; // [esp+10h] [ebp-4h]
    unsigned int script_structb; // [esp+10h] [ebp-4h]

    commandType = command->type;
    script_struct = 0;
    gameId = G_GetGameIdMapping(command->liveUpdateId);
    if ( commandType == RADIANT_COMMAND_CREATE )
    {
        script_structa = G_SpawnStruct(spawnVar);
        if ( script_structa )
        {
            G_NotifyScriptsOfSelectedScriptStruct(script_structa);
            G_AssignGameIdMapping(command->liveUpdateId, script_structa);
            AddSavedCommand(command);
        }
    }
    else if ( commandType == RADIANT_COMMAND_DELETE )
    {
        script_structb = G_FindStruct(spawnVar, gameId);
        if ( script_structb )
        {
            G_AssignGameIdMapping(command->liveUpdateId, script_structb);
            AddSavedCommand(command);
        }
        G_NotifyScriptsOfSelectedScriptStruct(0);
    }
    else
    {
        switch ( commandType )
        {
            case RADIANT_COMMAND_UPDATE:
                script_struct = G_FindStruct(spawnVar, gameId);
                break;
            case RADIANT_COMMAND_UPDATE_SELECTED:
                script_struct = gSelectedScriptStruct;
                break;
            case RADIANT_COMMAND_SELECT:
                script_struct = G_FindStruct(spawnVar, gameId);
                G_NotifyScriptsOfSelectedScriptStruct(script_struct);
                if ( !script_struct )
                    Com_Printf(
                        5,
                        "Radiant Live Update: Can't find server script_struct.    Maps out of sync (Radiant/Game), re-bsp!\n");
                break;
        }
        if ( script_struct )
        {
            for ( i = 0; i < spawnVar->numSpawnVars; ++i )
            {
                index = G_SetEntityScriptVariableInternal(spawnVar->spawnVars[i][0], spawnVar->spawnVars[i][1]);
                if ( index )
                    Scr_SetStructField(script_struct, index, SCRIPTINSTANCE_SERVER);
            }
            G_AssignGameIdMapping(command->liveUpdateId, script_struct);
            AddSavedCommand(command);
        }
    }
}

void __cdecl G_ClearSelectedScriptStruct()
{
    G_NotifyScriptsOfSelectedScriptStruct(0);
}

void __cdecl CG_NotifyScriptsOfSelectedScriptStruct(int structId)
{
    unsigned int numparams; // [esp+4h] [ebp-8h]
    unsigned __int16 t; // [esp+8h] [ebp-4h]

    cgSelectedScriptStruct = structId;
    numparams = 1;
    if ( structId )
    {
        Scr_AddObject(structId, SCRIPTINSTANCE_CLIENT);
        numparams = 2;
    }
    Scr_AddConstString(cscr_const.liveupdate, SCRIPTINSTANCE_CLIENT);
    t = Scr_ExecThread(SCRIPTINSTANCE_CLIENT, cg_scr_data.levelnotify, numparams);
    Scr_FreeThread(t, SCRIPTINSTANCE_CLIENT);
}

unsigned int __cdecl CG_FindStruct(SpawnVar *spawnVar, int gameId)
{
    float origin[3]; // [esp+0h] [ebp-14h] BYREF
    const char *origin_string; // [esp+Ch] [ebp-8h]
    unsigned int structId; // [esp+10h] [ebp-4h]

    if ( !spawnVar->spawnVarsValid
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\radiant_remote.cpp",
                    895,
                    0,
                    "%s",
                    "spawnVar.spawnVarsValid") )
    {
        __debugbreak();
    }
    if ( !cg_scr_data.findstruct
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\radiant_remote.cpp",
                    896,
                    0,
                    "%s",
                    "cg_scr_data.findstruct") )
    {
        __debugbreak();
    }
    memset(origin, 0, sizeof(origin));
    origin_string = GetPairValue(spawnVar, "origin");
    if ( origin_string )
        sscanf(origin_string, "%f %f %f", origin, &origin[1], &origin[2]);
    Scr_AddVector(origin, SCRIPTINSTANCE_CLIENT);
    Scr_AddExecThread(SCRIPTINSTANCE_CLIENT, cg_scr_data.findstruct, 1u);
    structId = 0;
    if ( gameId > 0 )
        structId = gameId;
    if ( Scr_GetType(0, SCRIPTINSTANCE_CLIENT) )
        structId = Scr_GetObject(0, SCRIPTINSTANCE_CLIENT);
    Scr_ClearOutParams(SCRIPTINSTANCE_CLIENT);
    return structId;
}

void __cdecl CG_ProcessScriptStructCommand(const RadiantCommand *command, SpawnVar *spawnVar)
{
    RadiantCommandType commandType; // [esp+0h] [ebp-14h]
    unsigned int index; // [esp+4h] [ebp-10h]
    int i; // [esp+8h] [ebp-Ch]
    int gameId; // [esp+Ch] [ebp-8h]
    unsigned int script_struct; // [esp+10h] [ebp-4h]
    unsigned int script_structa; // [esp+10h] [ebp-4h]
    unsigned int script_structb; // [esp+10h] [ebp-4h]

    commandType = command->type;
    script_struct = 0;
    gameId = CG_GetGameIdMapping(command->liveUpdateId);
    if ( commandType == RADIANT_COMMAND_CREATE )
    {
        script_structa = CG_SpawnStruct(spawnVar).stringValue;
        if ( script_structa )
        {
            CG_NotifyScriptsOfSelectedScriptStruct(script_structa);
            CG_AssignGameIdMapping(command->liveUpdateId, script_structa);
        }
    }
    else if ( commandType == RADIANT_COMMAND_DELETE )
    {
        script_structb = CG_FindStruct(spawnVar, gameId);
        if ( script_structb )
            CG_AssignGameIdMapping(command->liveUpdateId, script_structb);
        CG_NotifyScriptsOfSelectedScriptStruct(0);
    }
    else
    {
        switch ( commandType )
        {
            case RADIANT_COMMAND_UPDATE:
                script_struct = CG_FindStruct(spawnVar, gameId);
                break;
            case RADIANT_COMMAND_UPDATE_SELECTED:
                script_struct = cgSelectedScriptStruct;
                break;
            case RADIANT_COMMAND_SELECT:
                script_struct = CG_FindStruct(spawnVar, gameId);
                CG_NotifyScriptsOfSelectedScriptStruct(script_struct);
                if ( !script_struct )
                    Com_Printf(
                        5,
                        "Radiant Live Update: Can't find client script_struct.    Maps out of sync (Radiant/Game), re-bsp!\n");
                break;
        }
        if ( script_struct )
        {
            for ( i = 0; i < spawnVar->numSpawnVars; ++i )
            {
                index = CG_SetEntityScriptVariableInternal(spawnVar->spawnVars[i][0], spawnVar->spawnVars[i][1]);
                if ( index )
                    Scr_SetStructField(script_struct, index, SCRIPTINSTANCE_CLIENT);
            }
            CG_AssignGameIdMapping(command->liveUpdateId, script_struct);
        }
    }
}

void __cdecl CG_ClearSelectedScriptStruct()
{
    CG_NotifyScriptsOfSelectedScriptStruct(0);
}

void __cdecl G_RadiantDebugDraw()
{
    int v0; // eax
    float v1; // [esp+48h] [ebp-A0h]
    float v2; // [esp+50h] [ebp-98h]
    float v3; // [esp+54h] [ebp-94h]
    float v4; // [esp+58h] [ebp-90h]
    float v5; // [esp+60h] [ebp-88h]
    float v6; // [esp+64h] [ebp-84h]
    float v7; // [esp+68h] [ebp-80h]
    float v8; // [esp+74h] [ebp-74h]
    float *v9; // [esp+78h] [ebp-70h]
    float scale; // [esp+7Ch] [ebp-6Ch]
    float *v11; // [esp+80h] [ebp-68h]
    float dir[3]; // [esp+84h] [ebp-64h] BYREF
    cg_s *cgameGlob; // [esp+90h] [ebp-58h]
    GfxLightCorona *corona; // [esp+94h] [ebp-54h]
    float color[4]; // [esp+98h] [ebp-50h] BYREF
    GfxStaticModelDrawInst *smodelDrawInst; // [esp+A8h] [ebp-40h]
    float mins[3]; // [esp+ACh] [ebp-3Ch] BYREF
    float maxs[3]; // [esp+B8h] [ebp-30h] BYREF
    float axis[3][3]; // [esp+C4h] [ebp-24h] BYREF

    if ( g_selected_misc_model >= 0 )
    {
        smodelDrawInst = &rgp.world->dpvs.smodelDrawInsts[g_selected_misc_model];
        scale = smodelDrawInst->placement.scale;
        v11 = smodelDrawInst->model->mins;
        mins[0] = scale * *v11;
        mins[1] = scale * v11[1];
        mins[2] = scale * v11[2];
        v8 = smodelDrawInst->placement.scale;
        v9 = smodelDrawInst->model->maxs;
        maxs[0] = v8 * *v9;
        maxs[1] = v8 * v9[1];
        maxs[2] = v8 * v9[2];
        v1 = smodelDrawInst->placement.axis[0][2];
        v2 = smodelDrawInst->placement.axis[1][0];
        v3 = smodelDrawInst->placement.axis[1][1];
        v4 = smodelDrawInst->placement.axis[1][2];
        v5 = smodelDrawInst->placement.axis[2][0];
        v6 = smodelDrawInst->placement.axis[2][1];
        v7 = smodelDrawInst->placement.axis[2][2];
        *(_QWORD *)&axis[0][0] = *(_QWORD *)&smodelDrawInst->placement.axis[0][0];
        axis[0][2] = v1;
        axis[1][0] = v2;
        axis[1][1] = v3;
        axis[1][2] = v4;
        axis[2][0] = v5;
        axis[2][1] = v6;
        axis[2][2] = v7;
        CG_DebugBoxOriented(smodelDrawInst->placement.origin, mins, maxs, axis, colorWhite, 1, 0);
    }
    if ( g_selected_corona >= 0 )
    {
        corona = &rgp.world->coronas[g_selected_corona];
        color[0] = corona->color[0];
        color[1] = corona->color[1];
        color[2] = corona->color[2];
        color[3] = corona->intensity;
        v0 = RETURN_ZERO32();
        cgameGlob = CG_GetLocalClientGlobals(v0);
        if ( cgameGlob )
        {
            dir[0] = corona->origin[0] - cgameGlob->cameraData.lastViewOrg[0];
            dir[1] = corona->origin[1] - cgameGlob->cameraData.lastViewOrg[1];
            dir[2] = corona->origin[2] - cgameGlob->cameraData.lastViewOrg[2];
            G_DebugCircleEx(corona->origin, corona->radius, dir, color, 0, 1);
        }
    }
}

void __cdecl G_ClearSelection(void (__cdecl *ignoreFunc)())
{
    const RadiantCommandProcessor *p; // [esp+0h] [ebp-4h]

    for ( p = gCommandProcessors; p->processorFunc; ++p )
    {
        if ( p->clearSelectionFunc )
        {
            if ( p->clearSelectionFunc != ignoreFunc )
                p->clearSelectionFunc();
        }
    }
}

void __cdecl CG_ClearSelection(void (__cdecl *ignoreFunc)())
{
    const RadiantCommandProcessor *p; // [esp+0h] [ebp-4h]

    for ( p = cgCommandProcessors; p->processorFunc; ++p )
    {
        if ( p->clearSelectionFunc )
        {
            if ( p->clearSelectionFunc != ignoreFunc )
                p->clearSelectionFunc();
        }
    }
}

void __cdecl CG_ProcessRadiantCmd(const RadiantCommand *command)
{
    char *classname; // [esp+10h] [ebp-A24h]
    const RadiantCommandProcessor *p; // [esp+14h] [ebp-A20h]
    RadiantCommandType commandType; // [esp+18h] [ebp-A1Ch]
    SpawnVar spawnVar; // [esp+1Ch] [ebp-A18h] BYREF

    commandType = command->type;
    G_SetEntityParsePoint(command->strCommand);
    G_ParseSpawnVars(&spawnVar);
    if ( commandType == RADIANT_COMMAND_CAMERA )
    {
        G_ProcessCameraCommand(&spawnVar);
    }
    else if ( commandType == RADIANT_COMMAND_SELECT
                 || commandType == RADIANT_COMMAND_UPDATE
                 || commandType == RADIANT_COMMAND_UPDATE_SELECTED
                 || commandType == RADIANT_COMMAND_DELETE
                 || commandType == RADIANT_COMMAND_CREATE )
    {
        classname = GetPairValue(&spawnVar, "classname");
        for ( p = cgCommandProcessors; p->processorFunc; ++p )
        {
            if ( p->conditionFunc && p->conditionFunc(&spawnVar)
                || !p->conditionFunc && !I_strncmp(classname, p->classname, strlen(p->classname)) )
            {
                p->processorFunc(command, &spawnVar);
                CG_ClearSelection(p->clearSelectionFunc);
                return;
            }
        }
    }
    else
    {
        CG_ClearSelection(0);
    }
}

char __cdecl CG_ProcessRadiantCmds()
{
    int i; // [esp+0h] [ebp-4h]

    Sys_EnterCriticalSection(CRITSECT_RADIANT_CLIENT_COMMAND);
    for ( i = 0; i < cgCommandCount; ++i )
        CG_ProcessRadiantCmd(&cgCommands[i]);
    cgCommandCount = 0;
    Sys_LeaveCriticalSection(CRITSECT_RADIANT_CLIENT_COMMAND);
    return 1;
}

CommandProcessorType __cdecl GetCommandProcessorType(const SpawnVar *spawnVar)
{
    char *classname; // [esp+0h] [ebp-4h]

    classname = GetPairValue(spawnVar, "classname");
    if ( IsClientOnlyEntity(spawnVar) )
        return COMMAND_CLIENT;
    if ( !classname || I_strcmp(classname, "script_struct") )
        return COMMAND_SERVER;
    return COMMAND_BOTH;
}

void __cdecl G_ProcessRadiantCmd(const RadiantCommand *command)
{
    char *classname; // [esp+10h] [ebp-A24h]
    const RadiantCommandProcessor *p; // [esp+14h] [ebp-A20h]
    RadiantCommandType commandType; // [esp+18h] [ebp-A1Ch]
    SpawnVar spawnVar; // [esp+1Ch] [ebp-A18h] BYREF
    CommandProcessorType commandProcessorType; // [esp+A30h] [ebp-4h]

    commandType = command->type;
    G_SetEntityParsePoint(command->strCommand);
    G_ParseSpawnVars(&spawnVar);
    commandProcessorType = GetCommandProcessorType(&spawnVar);
    if ( commandProcessorType == COMMAND_CLIENT || commandProcessorType == COMMAND_BOTH )
    {
        Sys_EnterCriticalSection(CRITSECT_RADIANT_CLIENT_COMMAND);
        if ( cgCommandCount >= 24 )
        {
            Com_Printf(5, "Radiant Live Update: Too many commands\n");
        }
        else
        {
            I_strncpyz(cgCommands[cgCommandCount].strCommand, command->strCommand, 512);
            cgCommands[cgCommandCount].type = command->type;
            cgCommands[cgCommandCount++].liveUpdateId = command->liveUpdateId;
        }
        Sys_LeaveCriticalSection(CRITSECT_RADIANT_CLIENT_COMMAND);
    }
    if ( commandProcessorType == COMMAND_SERVER || commandProcessorType == COMMAND_BOTH )
    {
        if ( commandType == RADIANT_COMMAND_CAMERA )
        {
            G_ProcessCameraCommand(&spawnVar);
        }
        else if ( commandType == RADIANT_COMMAND_SELECT
                     || commandType == RADIANT_COMMAND_UPDATE
                     || commandType == RADIANT_COMMAND_UPDATE_SELECTED
                     || commandType == RADIANT_COMMAND_DELETE
                     || commandType == RADIANT_COMMAND_CREATE )
        {
            classname = GetPairValue(&spawnVar, "classname");
            for ( p = gCommandProcessors; p->processorFunc; ++p )
            {
                if ( p->conditionFunc && p->conditionFunc(&spawnVar)
                    || !p->conditionFunc && !I_strncmp(classname, p->classname, strlen(p->classname)) )
                {
                    p->processorFunc(command, &spawnVar);
                    G_ClearSelection(p->clearSelectionFunc);
                    return;
                }
            }
        }
        else
        {
            G_ClearSelection(0);
        }
    }
}

char __cdecl G_ProcessRadiantCmds()
{
    int i; // [esp+0h] [ebp-4h]

    Sys_EnterCriticalSection(CRITSECT_RADIANT_SERVER_COMMAND);
    for ( i = 0; i < gCommandCount; ++i )
        G_ProcessRadiantCmd(&gCommands[i]);
    gCommandCount = 0;
    Sys_LeaveCriticalSection(CRITSECT_RADIANT_SERVER_COMMAND);
    G_RadiantDebugDraw();
    return 1;
}

