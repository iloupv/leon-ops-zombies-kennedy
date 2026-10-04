#include <game_sp/g_sp_measure.h>
#include "actor_spawner.h"
#include <game_sp/g_sp_headless_dice.h>
#include <qcommon/cm_world.h>
#include <game_mp/g_main_mp.h>
#include <clientscript/cscr_stringlist.h>
#include <game_mp/g_active_mp.h>
#include <clientscript/cscr_vm.h>
#include <game_mp/g_utils_mp.h>
#include <game_mp/g_spawn_mp.h>
#include <game_mp/actor_mp.h>
#include "actor_events.h"
#include "actor_senses.h"
#include "g_load_utils.h"
#include <clientscript/scr_const.h>
#include <game_sp/g_scr_sp_entity.h>

const float g_vSpawnCheckPoints[11][3] =
{
  { 0.5, 0.5, 0.80000001 },
  { 0.5, 0.5, 0.5 },
  { 0.5, 0.5, 0.2 },
  { 0.0, 0.0, 1.0 },
  { 0.0, 1.0, 1.0 },
  { 1.0, 1.0, 1.0 },
  { 1.0, 0.0, 1.0 },
  { 1.0, 0.0, 0.0 },
  { 1.0, 1.0, 0.0 },
  { 0.0, 1.0, 0.0 },
  { 0.0, 0.0, 0.0 }
};


int __cdecl SpotWouldTelefrag(gentity_s *spot)
{
    int entityList[1025]; // [esp+8h] [ebp-1028h] BYREF
    float mins[3]; // [esp+100Ch] [ebp-24h] BYREF
    gentity_s *v4; // [esp+1018h] [ebp-18h]
    float maxs[3]; // [esp+101Ch] [ebp-14h] BYREF
    int v6; // [esp+1028h] [ebp-8h]
    int i; // [esp+102Ch] [ebp-4h]

    mins[0] = spot->r.currentOrigin[0] + -15.0;
    mins[1] = spot->r.currentOrigin[1] + -15.0;
    mins[2] = spot->r.currentOrigin[2] + 0.0;
    maxs[0] = spot->r.currentOrigin[0] + 15.0;
    maxs[1] = spot->r.currentOrigin[1] + 15.0;
    maxs[2] = spot->r.currentOrigin[2] + 70.0;
    v6 = CM_AreaEntities(mins, maxs, entityList, 1024, 0x2008000);
    for ( i = 0; i < v6; ++i )
    {
        v4 = &g_entities[entityList[i]];
        if ( v4->client && v4->client->ps.pm_type < 9 )
            return 1;
        if ( v4->actor && v4->health > 0 )
            return 1;
    }
    return 0;
}

int __cdecl PointCouldSeeSpawn(const float *vEyePos, const float *vSpawnPos, int iIgnoreEnt1, int iIgnoreEnt2)
{
    col_context_t context; // [esp+0h] [ebp-4Ch] BYREF
    float fDistSqrd; // [esp+28h] [ebp-24h]
    unsigned int i; // [esp+2Ch] [ebp-20h]
    float vCheckPos[3]; // [esp+30h] [ebp-1Ch] BYREF
    int hitNum; // [esp+3Ch] [ebp-10h] BYREF
    float vDelta[3]; // [esp+40h] [ebp-Ch]

    //col_context_t::col_context_t(&context);
    vDelta[0] = *vEyePos - *vSpawnPos;
    vDelta[1] = vEyePos[1] - vSpawnPos[1];
    vDelta[2] = vEyePos[2] - vSpawnPos[2];
    fDistSqrd = (float)((float)(vDelta[0] * vDelta[0]) + (float)(vDelta[1] * vDelta[1])) + (float)(vDelta[2] * vDelta[2]);
    if ( (float)(fDistSqrd * 1.0) > level.fFogOpaqueDistSqrd )
        return 0;
    hitNum = 0;
    context.mask = 6145;
    context.passEntityNum0 = iIgnoreEnt1;
    context.passEntityNum1 = iIgnoreEnt2;
    for ( i = 0; i < 0xB; ++i )
    {
        vCheckPos[0] = (float)(*vSpawnPos + -15.0) + (float)((float)(15.0 - -15.0) * (float)g_vSpawnCheckPoints[i][0]);
        vCheckPos[1] = (float)(vSpawnPos[1] + -15.0) + (float)((float)(15.0 - -15.0) * (float)g_vSpawnCheckPoints[i][1]);
        vCheckPos[2] = (float)(vSpawnPos[2] + 0.0) + (float)((float)(48.0 - 0.0) * (float)g_vSpawnCheckPoints[i][2]);
        SV_SightTracePoint(&hitNum, vEyePos, vCheckPos, &context);
        if ( !hitNum )
            return 1;
    }
    return 0;
}

gentity_s *__cdecl SpawnActor(gentity_s *ent, unsigned int targetname, enumForceSpawn forceSpawn, int getEnemyInfo)
{
    const char *v5; // [esp+18h] [ebp-38h]
    const char *v6; // [esp+1Ch] [ebp-34h]
    const char *v7; // [esp+20h] [ebp-30h]
    int clientNum; // [esp+24h] [ebp-2Ch]
    actor_s *pSelf; // [esp+28h] [ebp-28h]
    actor_s *pActor; // [esp+2Ch] [ebp-24h]
    sentient_s *pEnemy; // [esp+30h] [ebp-20h]
    gentity_s *spawn; // [esp+34h] [ebp-1Ch]
    float vEyePos[3]; // [esp+38h] [ebp-18h] BYREF
    unsigned int i; // [esp+44h] [ebp-Ch]
    team_t eTeam; // [esp+48h] [ebp-8h]
    gentity_s *player; // [esp+4Ch] [ebp-4h]

    if ( ai_disableSpawn->current.enabled )
    {
        Com_DPrintf(18, "Attempted spawn prevented by ai_disableSpawn.\n");
        return 0;
    }
    // zombies: count and in-use actor limit precede CHECK_SPAWN (SP 0x00526e50). Every SP level (the
    // front end runs zombiemode 0).
    if ( Com_IsSPLevel() )
    {
        if ( !ent->count )
        {
            const char *name = ent->targetname
                ? SL_ConvertToString(ent->targetname, SCRIPTINSTANCE_SERVER) : "<unnamed>";
            Com_DPrintf(18, "^3Warning: SpawnActor( %s ) failed due to 0 count.\n", name);
            return 0;
        }
        if ( g_spActorLimit )
        {
            int actorCount = 0;
            for ( int actorIndex = 0; actorIndex < MAX_ACTORS; ++actorIndex )
                actorCount += level.actors[actorIndex].inuse != 0;
            if ( actorCount >= g_spActorLimit )
                return 0;
        }
    }
    if ( forceSpawn == CHECK_SPAWN )
    {
        if ( SpotWouldTelefrag(ent) )
        {
            if ( ent->targetname )
                v7 = SL_ConvertToString(ent->targetname, SCRIPTINSTANCE_SERVER);
            else
                v7 = "<unnamed>";
            Com_DPrintf(
                18,
                "^3couldn't spawn from %s because spawnpoint (%7.3f %7.3f %7.3f) would telefrag\n",
                v7,
                ent->r.currentOrigin[0],
                ent->r.currentOrigin[1],
                ent->r.currentOrigin[2]);
            return 0;
        }
        for ( clientNum = 0; clientNum < com_maxclients->current.integer; ++clientNum )
        {
            player = G_GetPlayer(clientNum);
            if ( !player )
                Scr_Error("Attempt to spawn actor before player setup.", 0);
            if ( player->sentient )
            {
                if ( player->sentient->eTeam == TEAM_ALLIES )
                {
                    Sentient_GetEyePosition(player->sentient, vEyePos);
                    if ( PointCouldSeeSpawn(vEyePos, ent->r.currentOrigin, player->s.number, ent->s.number) )
                    {
                        if ( ent->targetname )
                            v6 = SL_ConvertToString(ent->targetname, SCRIPTINSTANCE_SERVER);
                        else
                            v6 = "<unnamed>";
                        Com_DPrintf(
                            18,
                            "^3couldn't spawn from %s because player can see spawnpoint (%7.3f %7.3f %7.3f)\n",
                            v6,
                            ent->r.currentOrigin[0],
                            ent->r.currentOrigin[1],
                            ent->r.currentOrigin[2]);
                        return 0;
                    }
                }
            }
        }
    }
    {
        // mod (L25): above the retail 32 actors the 1022 game entities, not the actor slots, end the horde (Five
        // keeps ~790 in use without AI). Refuse the spawn, as when a player can see the spawner, while fewer than 32
        // entities stay free for temp entities, power-ups and script models; G_Spawn would end the map instead.
        extern int g_maxActors;
        const int modFreeEnts = g_maxActors > 32 ? G_ModFreeEntities(32) : 32; // mod (L43): shared count
        if ( g_maxActors > 32 && modFreeEnts < 32 )
        {
            static int lastModRefuseTime = -100000;
            if ( level.time - lastModRefuseTime >= 5000 || level.time < lastModRefuseTime )
            {
                Com_Printf(15, "mod: actor spawn refused at the entity limit (num_entities %d)\n", level.num_entities);
                lastModRefuseTime = level.time;
                { void __cdecl G_BO1EntCensus(bool); G_BO1EntCensus(true); } // mod (L55): who holds the entities
            }
            return 0;
        }
    }
    spawn = G_Spawn();
    G_DuplicateEntityFields(spawn, ent);
    G_DuplicateScriptFields(spawn, ent);
    Scr_SetString(&spawn->targetname, targetname, SCRIPTINSTANCE_SERVER);
    spawn->spawnflags &= ~1u;
    if ( SP_actor(spawn, 0) )
    {
        G_SP_MeasureActor("actor_spawn", spawn, ent);
        G_SP_HeadlessDiceActorSpawned(spawn); // k1 TEST SWITCH bo1_testclient_dice: spawn order
        if ( !spawn->actor
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_spawner.cpp", 211, 0, "%s", "spawn->actor") )
        {
            __debugbreak();
        }
        if ( (ent->spawnflags & 8) != 0 )
        {
            for ( pEnemy = Sentient_FirstSentient(-1); pEnemy; pEnemy = Sentient_NextSentient(pEnemy, -1) )
                Actor_GetPerfectInfo(spawn->actor, pEnemy);
        }
        // zombies: run aitype main once, after perfect info and before sharing enemy info (SP 0x00526e50).
        if ( Com_IsSPLevel() )
            Actor_FinishSpawning(spawn->actor);
        G_SP_MeasureActor("actor_ready", spawn, ent);
        pSelf = spawn->actor;
        if ( getEnemyInfo )
        {
            eTeam = spawn->sentient->eTeam;
            void SentientInfo_CopyAllBegin(const actor_s *), SentientInfo_CopyAllEnd();
            if ( g_modSentientCopy )
                SentientInfo_CopyAllBegin(pSelf); // mod (L55): exact per-index bound (actor_mp.cpp)
            for ( pActor = Actor_FirstActor(1 << eTeam); pActor; pActor = Actor_NextActor(pActor, 1 << eTeam) )
            {
                if ( pSelf != pActor )
                {
                    // mod (L43q): same knowledge merge, with the extended row addresses resolved once per pair.
                    if ( g_modSentientCopy )
                        SentientInfo_CopyAll(pSelf, pActor);
                    else
                    {
                        for ( i = 0; i < MAX_SENTIENTS; ++i )
                            SentientInfo_Copy(pSelf, pActor, i);
                    }
                }
            }
            if ( g_modSentientCopy )
                SentientInfo_CopyAllEnd();
        }
        Actor_UpdateSight(pSelf);
        Actor_UpdateThreat(pSelf);
        Actor_InitAnimScript(spawn->actor);
        Scr_AddEntity(spawn, SCRIPTINSTANCE_SERVER);
        Scr_Notify(ent, scr_const.spawned, 1u);
        if ( ent->count > 0 )
            --ent->count;
        return spawn;
    }
    else
    {
        if ( ent->targetname )
        {
            v5 = SL_ConvertToString(ent->targetname, SCRIPTINSTANCE_SERVER);
            Com_DPrintf(18, "^3couldn't spawn from %s because there are no free actors\n", v5);
        }
        else
        {
            Com_DPrintf(18, "^3couldn't spawn from %s because there are no free actors\n", "<unnamed>");
        }
        return 0;
    }
}

void __cdecl G_DropActorSpawnersToFloor()
{
    unsigned int v0[MAX_GENTITIES_SV + 1]; // mod (L25): server-only entity numbers 1024+ // [esp+20h] [ebp-1018h]
    float v1; // [esp+1024h] [ebp-14h]
    float v2; // [esp+1028h] [ebp-10h]
    float v3; // [esp+102Ch] [ebp-Ch]
    gentity_s *ent; // [esp+1030h] [ebp-8h]
    int i; // [esp+1034h] [ebp-4h]

    for ( i = 0; i < G_EntEnd(); i = G_EntNext(i) )
    {
        ent = &level.gentities[i];
        if ( ent->r.inuse )
        {
            v0[i] = ent->r.contents;
            if ( Path_IsDynamicBlockingEntity(ent) )
                ent->r.contents = 0;
        }
    }
    for ( i = 0; i < G_EntEnd(); i = G_EntNext(i) )
    {
        ent = &level.gentities[i];
        if ( ent->r.inuse )
        {
            if ( ent->s.eType == ET_ACTOR_SPAWNER )
            {
                v1 = ent->r.currentOrigin[0];
                v2 = ent->r.currentOrigin[1];
                v3 = ent->r.currentOrigin[2];
                if ( Actor_droptofloor(ent) )
                {
                    Com_Printf(18, "^3Spawner at (%g %g %g) is in solid\n", v1, v2, v3);
                    ent->r.svFlags &= ~1u;
                }
            }
        }
    }
    for ( i = 0; i < G_EntEnd(); i = G_EntNext(i) )
    {
        ent = &level.gentities[i];
        if ( ent->r.inuse )
            ent->r.contents = v0[i];
    }
}

int __cdecl SP_actor_spawner(gentity_s *pEnt, SpawnVar *spawnVar)
{
    if ( !spawnVar->spawnVarsValid
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_spawner.cpp",
                    324,
                    0,
                    "%s",
                    "spawnVar.spawnVarsValid") )
    {
        __debugbreak();
    }
    pEnt->clipmask = 0;
    pEnt->r.contents = 0;
    pEnt->r.svFlags = 1;
    pEnt->s.eType = ET_ACTOR_SPAWNER;
    pEnt->item[0].clipAmmoCount = -1;
    pEnt->item[0].ammoCount = 0;
    // zombies: omitted count defaults to one, explicit count zero stays zero (SP 0x0043c770).
    if ( Com_IsSPLevel() && !pEnt->count )
    {
        const char *count;
        pEnt->count = 1;
        if ( G_SpawnString(spawnVar, "count", "", &count) )
            pEnt->count = 0;
        // SP also handles sm_count here; that script field is outside this port.
    }
    return 1;
}

