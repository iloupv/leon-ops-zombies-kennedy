#include "g_scr_helicopter.h"
#include <clientscript/cscr_vm.h>
#include <qcommon/cm_load.h>
#include <game_mp/g_utils_mp.h>
#include <cgame/cg_scr_main.h>
#include <qcommon/dobj_management.h>
#include <bgame/bg_weapons_def.h>

// Looks congruent to retail blops mp latest
const BuiltinMethodDef s_methods[27] =
{
  { "freehelicopter", &CMD_Heli_FreeHelicopter, 0 },
  { "setspeed", &CMD_VEH_SetSpeed, 0 },
  { "getspeed", &CMD_VEH_GetSpeed, 0 },
  { "getspeedmph", &CMD_VEH_GetSpeedMPH, 0 },
  { "resumespeed", &CMD_VEH_ResumeSpeed, 0 },
  { "setyawspeed", &CMD_VEH_SetYawSpeed, 0 },
  { "setmaxpitchroll", &CMD_VEH_SetMaxPitchRoll, 0 },
  { "setturningability", &CMD_VEH_SetTurningAbility, 0 },
  { "setairresistance", &CMD_VEH_SetAirResitance, 0 },
  { "sethoverparams", &CMD_VEH_SetHoverParams, 0 },
  { "setneargoalnotifydist", &CMD_VEH_NearGoalNotifyDist, 0 },
  { "setvehgoalpos", &CMD_VEH_SetGoalPos, 0 },
  { "setgoalyaw", &CMD_VEH_SetGoalYaw, 0 },
  { "cleargoalyaw", &CMD_VEH_ClearGoalYaw, 0 },
  { "settargetyaw", &CMD_VEH_SetTargetYaw, 0 },
  { "cleartargetyaw", &CMD_VEH_ClearTargetYaw, 0 },
  { "setlookatent", &CMD_VEH_SetLookAtEnt, 0 },
  { "clearlookatent", &CMD_VEH_ClearLookAtEnt, 0 },
  { "setvehweapon", &CMD_VEH_SetWeapon, 0 },
  { "fireweapon", &CMD_VEH_FireWeapon, 0 },
  { "stopfireweapon", &CMD_VEH_StopFireWeapon, 0 },
  { "setturrettargetvec", &CMD_VEH_SetTurretTargetVec, 0 },
  { "setturrettargetent", &CMD_VEH_SetTurretTargetEnt, 0 },
  { "clearturrettarget", &CMD_VEH_ClearTurretTarget, 0 },
  { "setdamagestage", &CMD_Heli_SetDamageStage, 0 },
  { "setheliheightlock", &CMD_Heli_SetHeliHeightLock, 0 },
  { "isinsideheliheightlock", &CMD_Heli_IsInsideHeliHeightLock, 0 }
};


heli_height_lock_patches_t heli_height_lock_patches[32];

void __cdecl CMD_Heli_FreeHelicopter(scr_entref_t entref)
{
    gentity_s *ent; // [esp+8h] [ebp-4h]

    ent = GScr_GetVehicle(entref);
    G_FreeVehicle(ent);
}

void __cdecl CMD_Heli_SetDamageStage(scr_entref_t entref)
{
    VariableUnion v1; // eax
    gentity_s *ent; // [esp+8h] [ebp-4h]

    ent = GScr_GetVehicle(entref);
    v1.intValue = Scr_GetInt(0, SCRIPTINSTANCE_SERVER);
    AssignToSmallerType<unsigned char>(&ent->s.un1.scale, v1.intValue);
}

void __cdecl CMD_Heli_SetHeliHeightLock(scr_entref_t entref)
{
    gentity_s *ent; // [esp+8h] [ebp-4h]

    ent = GScr_GetVehicle(entref);
    if ( Scr_GetInt(0, SCRIPTINSTANCE_SERVER) )
        ent->scr_vehicle->flags |= 0x200u;
    else
        ent->scr_vehicle->flags &= ~0x200u;
}

void __cdecl CMD_Heli_IsInsideHeliHeightLock(scr_entref_t entref)
{
    float v2; // [esp+0h] [ebp-4Ch]
    float v3; // [esp+4h] [ebp-48h]
    float v4; // [esp+8h] [ebp-44h]
    float meshMaxs[3]; // [esp+28h] [ebp-24h] BYREF
    float meshMins[3]; // [esp+34h] [ebp-18h] BYREF
    int i; // [esp+40h] [ebp-Ch]
    gentity_s *ent; // [esp+44h] [ebp-8h]
    float *heliOrigin; // [esp+48h] [ebp-4h]

    ent = GScr_GetVehicle(entref);
    if (!ent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_scr_helicopter.cpp", 88, 0, "%s", "ent"))
        __debugbreak();
    if (!ent->scr_vehicle
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_scr_helicopter.cpp",
            89,
            0,
            "%s",
            "ent->scr_vehicle"))
    {
        __debugbreak();
    }
    heliOrigin = ent->scr_vehicle->phys.origin;
    for (i = 0; ; ++i)
    {
        if (i >= num_heli_height_lock_patches)
        {
            Scr_AddInt(0, SCRIPTINSTANCE_SERVER);
            return;
        }
        CM_ModelBounds(heli_height_lock_patches[i].brushmodel, meshMins, meshMaxs);
        meshMins[0] = meshMins[0] + heli_height_lock_patches[i].origin[0];
        meshMins[1] = meshMins[1] + heli_height_lock_patches[i].origin[1];
        meshMins[2] = meshMins[2] + heli_height_lock_patches[i].origin[2];
        meshMaxs[0] = meshMaxs[0] + heli_height_lock_patches[i].origin[0];
        meshMaxs[1] = meshMaxs[1] + heli_height_lock_patches[i].origin[1];
        meshMaxs[2] = meshMaxs[2] + heli_height_lock_patches[i].origin[2];
        if (*heliOrigin >= meshMins[0]
            && meshMaxs[0] >= *heliOrigin
            && heliOrigin[1] >= meshMins[1]
            && meshMaxs[1] >= heliOrigin[1])
        {
            break;
        }
    }
    if ((float)((float)(meshMaxs[0] - *heliOrigin) - (float)(*heliOrigin - meshMins[0])) < 0.0)
        v4 = meshMaxs[0] - *heliOrigin;
    else
        v4 = *heliOrigin - meshMins[0];
    if ((float)((float)(heliOrigin[1] - meshMins[1]) - v4) < 0.0)
        v3 = heliOrigin[1] - meshMins[1];
    else
        v3 = v4;
    if ((float)((float)(meshMaxs[1] - heliOrigin[1]) - v3) < 0.0)
        v2 = meshMaxs[1] - heliOrigin[1];
    else
        v2 = v3;
    Scr_AddInt((int)v2, SCRIPTINSTANCE_SERVER);
}

void (__cdecl *__cdecl Helicopter_GetMethod(const char **pName))(scr_entref_t)
{
    unsigned int i; // [esp+18h] [ebp-4h]

    for ( i = 0; i < ARRAY_COUNT(s_methods); ++i )
    {
        if ( !strcmp(*pName, s_methods[i].actionString) )
        {
            *pName = s_methods[i].actionString;
            return s_methods[i].actionFunc;
        }
    }
    return 0;
}

void __cdecl G_SpawnHelicopter(gentity_s *ent, gentity_s *owner, char *vehicleInfoName, char *modelName)
{
    scr_vehicle_s *veh; // [esp+Ch] [ebp-10h]
    team_t team; // [esp+10h] [ebp-Ch]
    unsigned int ownerIndex; // [esp+18h] [ebp-4h]

    G_SetModel(ent, modelName);
    G_SpawnVehicle(ent, vehicleInfoName, 1);
    ent->clipmask = 8389632;
    ent->s.eType = ET_HELICOPTER;
    veh = ent->scr_vehicle;
    bg_vehicleInfos[veh->infoIdx].type = 6;
    veh->targetEnt = 1023;

    iassert(!veh->lookAtEnt.isDefined());
    
    veh->phys.mins[0] = -50.0f;
    veh->phys.mins[1] = -50.0f;
    veh->phys.mins[2] = -50.0f;
    veh->phys.maxs[0] = 50.0f;
    veh->phys.maxs[1] = 50.0f;
    veh->phys.maxs[2] = 50.0f;
    if ( !owner->client
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_scr_helicopter.cpp",
                    245,
                    0,
                    "%s",
                    "owner->client") )
    {
        __debugbreak();
    }
    team = owner->client->sess.cs.team;
    if ( (unsigned int)team >= TEAM_NUM_TEAMS
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_scr_helicopter.cpp",
                    248,
                    0,
                    "team doesn't index (1 << 2)\n\t%i not in [0, %i)",
                    team,
                    4) )
    {
        __debugbreak();
    }
    ownerIndex = owner->client - level.clients;
    if ( ownerIndex >= level.maxclients
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_scr_helicopter.cpp",
                    250,
                    0,
                    "ownerIndex doesn't index level.maxclients\n\t%i not in [0, %i)",
                    ownerIndex,
                    level.maxclients) )
    {
        __debugbreak();
    }
    AssignToSmallerType<unsigned char>(&ent->s.faction.iHeadIconTeam, team | (4 * ownerIndex));
    ent->handler = 25;
    ent->nextthink = level.time + 50;
    ent->r.svFlags |= 0x10u;
    Heli_InitFirstThink(ent);
}

void __cdecl Heli_InitFirstThink(gentity_s *pSelf)
{
    float *prevAngles; // [esp+0h] [ebp-28h]
    float *angles; // [esp+4h] [ebp-24h]
    float *prevOrigin; // [esp+8h] [ebp-20h]
    float pos[3]; // [esp+Ch] [ebp-1Ch] BYREF
    vehicle_physic_t *phys; // [esp+18h] [ebp-10h]
    const vehicle_info_t *info; // [esp+1Ch] [ebp-Ch]
    scr_vehicle_s *veh; // [esp+20h] [ebp-8h]
    int wheelIndex; // [esp+24h] [ebp-4h]

    veh = pSelf->scr_vehicle;
    phys = &veh->phys;
    info = BG_GetVehicleInfo(veh->infoIdx);
    for ( wheelIndex = 0; wheelIndex < 6; ++wheelIndex )
    {
        if ( veh->boneIndex.wheel[wheelIndex] >= 0 )
        {
            G_DObjGetWorldBoneIndexPos(pSelf, veh->boneIndex.wheel[wheelIndex], pos);
            phys->wheelZPos[wheelIndex] = pos[2];
        }
    }
    VEH_SetPosition(pSelf, phys->origin, phys->vel, phys->angles);
    prevOrigin = phys->prevOrigin;
    phys->prevOrigin[0] = phys->origin[0];
    prevOrigin[1] = phys->origin[1];
    prevOrigin[2] = phys->origin[2];
    prevAngles = phys->prevAngles;
    angles = phys->angles;
    phys->prevAngles[0] = phys->angles[0];
    prevAngles[1] = angles[1];
    prevAngles[2] = angles[2];
    pSelf->health = 99999;
    pSelf->handler = 25;
    pSelf->nextthink = level.time + 50;
}

void __cdecl Helicopter_Pain(
    gentity_s *pSelf,
    gentity_s *pAttacker,
    int damage,
    const float *point,
    const int mod,
    const float *dir,
    const hitLocation_t __formal,
    const int __formal2)
{
    const WeaponDef *weapDef; // [esp+Ch] [ebp-4h]

    if ( pAttacker )
    {
        if ( pAttacker->s.weapon )
        {
            weapDef = BG_GetWeaponDef(pAttacker->s.weapon);
            if ( weapDef->weapType == WEAPTYPE_PROJECTILE || weapDef->weapType == WEAPTYPE_GRENADE )
                VEH_JoltBody(pSelf, dir, 1.0, 0.0, 0.0);
        }
    }
}

void __cdecl Helicopter_Die(
    gentity_s *pSelf,
    gentity_s *pInflictor,
    gentity_s *pAttacker,
    int damage,
    int mod,
    const int weapon,
    const float *dir,
    const hitLocation_t __formal,
    int __formal2)
{
    const WeaponDef *weapDef; // [esp+Ch] [ebp-4h]

    if ( pAttacker )
    {
        if ( pAttacker->s.weapon )
        {
            weapDef = BG_GetWeaponDef(pAttacker->s.weapon);
            if ( weapDef->weapType == WEAPTYPE_PROJECTILE || weapDef->weapType == WEAPTYPE_GRENADE )
                VEH_JoltBody(pSelf, dir, 1.0, 0.0, 0.0);
        }
    }
}

void __cdecl Helicopter_Controller(const gentity_s *pSelf, int *partBits)
{
    float v2; // [esp+4h] [ebp-3Ch]
    float v3; // [esp+Ch] [ebp-34h]
    float barrelAngles[3]; // [esp+10h] [ebp-30h] BYREF
    DObj *obj; // [esp+1Ch] [ebp-24h]
    scr_vehicle_s *veh; // [esp+20h] [ebp-20h]
    float bodyAngles[3]; // [esp+24h] [ebp-1Ch] BYREF
    float turretAngles[3]; // [esp+30h] [ebp-10h] BYREF
    int i; // [esp+3Ch] [ebp-4h]

    if ( !pSelf
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_scr_helicopter.cpp", 311, 0, "%s", "pSelf") )
    {
        __debugbreak();
    }
    if ( !pSelf->scr_vehicle
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_scr_helicopter.cpp",
                    312,
                    0,
                    "%s",
                    "pSelf->scr_vehicle") )
    {
        __debugbreak();
    }
    veh = pSelf->scr_vehicle;
    obj = Com_GetServerDObj(pSelf->s.number);
    if ( !obj
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_scr_helicopter.cpp", 316, 0, "%s", "obj") )
    {
        __debugbreak();
    }
    if ( veh->nitrousVehicle )
    {
        memset(bodyAngles, 0, sizeof(bodyAngles));
    }
    else
    {
        bodyAngles[0] = (float)pSelf->s.lerp.u.vehicle.throttle * 0.0054931641;
        bodyAngles[1] = 0.0f;
        bodyAngles[2] = pSelf->s.lerp.u.turret.gunAngles[1];
    }
    if ( veh->boneIndex.body >= 0 )
        DObjSetLocalBoneIndex(obj, partBits, veh->boneIndex.body, vec3_origin, bodyAngles);
    v3 = (float)pSelf->s.lerp.u.vehicle.gunYaw * 0.0054931641;
    turretAngles[0] = 0.0f;
    turretAngles[1] = v3;
    turretAngles[2] = 0.0f;
    barrelAngles[0] = (float)pSelf->s.lerp.u.vehicle.gunPitch * 0.0054931641;
    barrelAngles[1] = 0.0f;
    barrelAngles[2] = 0.0f;
    if ( veh->boneIndex.turret >= 0 )
        DObjSetLocalBoneIndex(obj, partBits, veh->boneIndex.turret, vec3_origin, turretAngles);
    if ( veh->boneIndex.barrel >= 0 )
        DObjSetLocalBoneIndex(obj, partBits, veh->boneIndex.barrel, vec3_origin, barrelAngles);
    for ( i = 0; i < 4; ++i )
    {
        v2 = (float)pSelf->s.lerp.u.vehicle.gunnerAngles[i].yaw * 0.0054931641;
        turretAngles[0] = 0.0f;
        turretAngles[1] = v2;
        turretAngles[2] = 0.0f;
        barrelAngles[0] = (float)pSelf->s.lerp.u.vehicle.gunnerAngles[i].pitch * 0.0054931641;
        barrelAngles[1] = 0.0f;
        barrelAngles[2] = 0.0f;
        if ( veh->boneIndex.gunnerTags[i].turret >= 0 )
            DObjSetLocalBoneIndex(obj, partBits, veh->boneIndex.gunnerTags[i].turret, vec3_origin, turretAngles);
        if ( veh->boneIndex.gunnerTags[i].barrel >= 0 )
            DObjSetLocalBoneIndex(obj, partBits, veh->boneIndex.gunnerTags[i].barrel, vec3_origin, barrelAngles);
    }
}

void __cdecl Helicopter_Think(gentity_s *ent)
{
    Scr_Vehicle_Think(ent);
    ent->nextthink = level.time + 50;
}

