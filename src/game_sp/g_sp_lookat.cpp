// zombies: SP's per-frame player look-at update (SP 0x00418d00), the writer of the client's look-at
// entity (SP client +0x1c80, gclientSpExt::lookatent) that islookingat (SP 0x007d7950) and the
// lookatent client field read, and of the crosshair flags SP's server puts in ps.weapFlags
// (8 friendly in range, 0x10 enemy, 0x200000). SP calls it from ClientEndFrame (SP 0x0047f8bf) once
// the client has a command time and level.time is past 500, right before Player_UpdateCursorHints.
//
// SP -> KB mapping used below (SP offsets are from the exe; KB names by role):
//  ent +0x100 r.contents, +0x11c r.currentOrigin, +0x13c client, +0x140 actor, +0x144 sentient,
//  +0x148 scr_vehicle, +0x15c classname, +0x184 health, +0xbe s.eType (SP 6 ET_SCRIPTMOVER = KB 6,
//  SP 13 ET_VEHICLE = KB ET_VEHICLE 14, from SP's name table at .data 0x00b775d8), +4 s.lerp.eFlags,
//  +0x280 lookAtText0 (setlookattext, SP 0x0048cd50);
//  ps +0x10 weapFlags, +0xe0 eFlags, +0xe4 eFlags2, +0x144 weapon, +0x444 viewlocked_entNum,
//  +0x448 vehiclePos; actor +0x21a0 bActivateCrosshair, +0x19d0 bDontAvoidPlayer,
//  +0xe54 Physics.iTraceMask; sentient +4 eTeam, +0x12 bIgnoreForFriendlyFire;
//  WeaponDef +0x4a0 enemyCrosshairRange, +0x54d bRifleBullet; SP 0x006938c0 = Sentient_EnemyTeam.
#include "g_sp_lookat.h"
#include "g_sp_ext.h"
#include "actor_sp_ext.h"
#include <game_mp/g_main_mp.h>
#include <game_mp/g_trigger_mp.h>
#include <game_mp/g_combat_mp.h>
#include <client_mp/g_client_mp.h>
#include <game/actor.h>
#include <game/sentient.h>
#include <game/actor_events.h>
#include <game/bullet.h>
#include <game/g_scr_vehicle.h>
#include <bgame/bg_misc.h>
#include <bgame/bg_weapons.h>
#include <cgame/cg_weapons.h>
#include <server/sv_world.h>
#include <qcommon/cm_trace.h>
#include <clientscript/scr_const.h>
#include <EffectsCore/fx_system.h>
#include <universal/q_shared.h>

// zombies: FX_GetServerVisibility (SP 0x0044f050 via thunk 0x005d8b90): smoke blockers between two
// points; the same helper as actor_zombie_exposed.cpp's (a dedicated server has no client FX system,
// which is the empty-blocker case, 1).
static float FX_GetServerVisibility_LookAt(const float *start, const float *end)
{
    if (!FX_GetSystemRemote(fx_serverVisClient))
        return 1.0f;
    return (float)FX_GetClientVisibility(fx_serverVisClient, start, end);
}

// zombies: SP 0x00818900 (register arguments: trace edi, start esi, mask ecx, priority map eax):
// the look trace. No entity, or the world, is NULL; so is a hit that smoke hides (visibility from
// the start to the hit point below 0.0001; SP scales the fraction by the 15000 trace length).
static gentity_s *Player_LookAtTrace(trace_t *trace, const float *start, const float *end, int passEntityNum,
    int contentmask, unsigned char *priorityMap, const float *forward)
{
    G_LocationalTrace(trace, start, end, passEntityNum, contentmask, priorityMap, NULL);
    unsigned short hitId = Trace_GetEntityHitId(trace);
    if (hitId >= ENTITYNUM_WORLD)
        return NULL;
    float dist = trace->fraction * 15000.0f;
    float hitPos[3];
    hitPos[0] = forward[0] * dist + start[0];
    hitPos[1] = forward[1] * dist + start[1];
    hitPos[2] = forward[2] * dist + start[2];
    if (0.0001f > FX_GetServerVisibility_LookAt(start, hitPos))
        return NULL;
    return &g_entities[hitId];
}

static float LookAt_DistSq(const float *delta)
{
    return delta[2] * delta[2] + delta[1] * delta[1] + delta[0] * delta[0];
}

// SP reads the look-at text from the entity (+0x280); KB keeps it only on script vehicles.
static unsigned short LookAt_Text0(const gentity_s *ent)
{
    return ent->scr_vehicle ? ent->scr_vehicle->lookAtText0 : 0;
}

void Player_UpdateLookAtEntity_SP(gentity_s *ent)
{
    playerState_s *ps = &ent->client->ps;
    gclientSpExt &ext = G_ClientSpExt(ent->client);

    ps->weapFlags &= ~0x200018u;
    if (player_forceRedCrosshair->current.enabled)
        ps->weapFlags |= 0x10u;
    ext.lookatent.setEnt(NULL);

    float start[3], forward[3], end[3];
    G_GetPlayerViewOrigin(ps, start); // SP Player_GetEyePos 0x00646ad0
    G_GetPlayerViewDirection(ent, forward, NULL, NULL); // SP 0x005bebb0
    const WeaponDef *weapDef = BG_GetWeaponDef(G_GetPlayerWeapon(ps, 0));

    // SP 0x00418d95..0x00418e89: in a vehicle gunner seat (eFlags 0x4000, vehiclePos 1..4) SP traces
    // the seat's turret (SP 0x00590720) and either looks at the vehicle itself or takes the eye and
    // direction from the seat's gun tag. Not ported: it needs the SP scr_vehicle seat records
    // (+0x284, +0x58c); Five has no vehicles.

    unsigned char *priorityMap = (ps->weapon && weapDef->bRifleBullet) ? riflePriorityMap : bulletPriorityMap;
    float gunOffset = bg_gunXOffset->current.value;
    start[0] = gunOffset * forward[0] + start[0];
    start[1] = gunOffset * forward[1] + start[1];
    start[2] = gunOffset * forward[2] + start[2];
    end[0] = forward[0] * 15000.0f + start[0];
    end[1] = forward[1] * 15000.0f + start[1];
    end[2] = forward[2] * 15000.0f + start[2];

    trace_t trace;
    gentity_s *traceEnt = Player_LookAtTrace(&trace, start, end, ent->s.number, 0x2280E803, priorityMap, forward);

    // SP 0x00418f69..0x00418fba: SP 0x008189c0 (the client's aim record, SP client +0x1cd8..+0x1ce8)
    // and SP 0x00818c00 (marks the path nodes along the aim within ai_playerLOSRange for the AI).
    // Not ported: AI cover awareness only, no effect on the look-at entity or the crosshair.

    if ((ps->weapFlags & 2) && ent->sentient)
    {
        float range = ai_playerLOSRange->current.value;
        float lineEnd[3];
        lineEnd[0] = range * forward[0] + start[0];
        lineEnd[1] = forward[1] * range + start[1];
        lineEnd[2] = forward[2] * range + start[2];
        Actor_BroadcastLineEvent(ent, NULL, AI_EV_BLOCK_FRIENDLIES, 1 << ent->sentient->eTeam, start, lineEnd, 0.0f);
    }

    if (!traceEnt)
        return;

    if (traceEnt->classname == scr_const.trigger_lookat)
    {
        ext.lookatent.setEnt(traceEnt);
        G_Trigger(traceEnt, ent); // SP Scr_NotifyTrigger 0x0040cac0
        traceEnt = Player_LookAtTrace(&trace, start, end, ent->s.number, 0x280E803, priorityMap, forward);
        if (!traceEnt)
            return;
    }

    float delta[3];
    // SP's body test: players 0x2000000, actors 0x4000 (SP G_SpawnActorEntity 0x004f5a70 stores
    // 0x4000 at 0x004f5bcc). KB spawns actors with MP's 0x8000 (actor_mp.cpp), so without the actor
    // term below the look-at never saw a zombie: no 0x10, no enemy (red) reticle. Map KB's actor
    // contents to SP's here rather than renumber actor contents for every trace.
    if ((traceEnt->r.contents & 0x2004000) || (traceEnt->actor && (traceEnt->r.contents & 0x8000)))
    {
        if (trace.sflags & 0x10)
            return;
        if (traceEnt->s.lerp.eFlags & 0x20)
            return;
        delta[0] = traceEnt->r.currentOrigin[0] - start[0];
        delta[1] = traceEnt->r.currentOrigin[1] - start[1];
        delta[2] = traceEnt->r.currentOrigin[2] - start[2];
        if (zombietron->current.enabled)
            return;
        if (traceEnt->actor && !traceEnt->actor->bActivateCrosshair)
            return;

        bool friendly = false;
        if (ent->client && traceEnt->client)
        {
            // SP client +0x1aec, the team (set by ClientConnect 0x005d3760, compared by G_Damage).
            int team = ent->client->sess.cs.team;
            if (traceEnt->client->sess.cs.team == team && team)
                friendly = true;
        }
        else
        {
            // SP reads both sentients without a test; a KB body without one is left alone.
            if (!ent->sentient || !traceEnt->sentient)
                return;
            unsigned int enemyBit = 1u << Sentient_EnemyTeam(ent->sentient->eTeam);
            if ((1u << traceEnt->sentient->eTeam) & ~enemyBit)
            {
                ps->eFlags2 &= ~0x20000000u;
                friendly = true;
                if (traceEnt->sentient->bIgnoreForFriendlyFire)
                    ps->eFlags2 |= 0x20000000u;
            }
        }

        // SP 0x004191d9: in zombie mode a player target is an enemy for the human gun, and for the
        // upgraded ballistic knives while that player is in last stand (pm_type 6 / 7), the revive
        // knives.
        bool treatAsEnemy = false;
        if (zombiemode->current.enabled && traceEnt->client)
        {
            const char *weaponName = BG_WeaponName(ps->weapon);
            if (!I_strncmp(weaponName, "humangun_", 9))
            {
                treatAsEnemy = true;
            }
            else if (traceEnt->client->ps.pm_type == 7 || traceEnt->client->ps.pm_type == 6)
            {
                if (!I_strcmp(weaponName, "knife_ballistic_upgraded_zm")
                    || !I_strcmp(weaponName, "knife_ballistic_bowie_upgraded_zm")
                    || !I_strcmp(weaponName, "knife_ballistic_sickle_upgraded_zm"))
                {
                    treatAsEnemy = true;
                }
            }
        }
        if (treatAsEnemy)
        {
            float range = weapDef->enemyCrosshairRange;
            if (range * range > LookAt_DistSq(delta))
            {
                if (!ext.lookatent.isDefined())
                    ext.lookatent.setEnt(traceEnt);
                ps->weapFlags |= 0x200000u;
            }
            return;
        }

        float distSq = LookAt_DistSq(delta);
        if (friendly)
        {
            float nameDist = g_friendlyNameDist->current.value;
            if (nameDist * nameDist > distSq && !ext.lookatent.isDefined())
                ext.lookatent.setEnt(traceEnt);
            float ffDist = g_friendlyfireDist->current.value;
            if (ffDist * ffDist > distSq)
            {
                ps->weapFlags |= 8u;
                actor_s *actor = traceEnt->actor;
                if (actor && (actor->bDontAvoidPlayer || !(actor->Physics.iTraceMask & 0x2000000)))
                    ps->weapFlags |= 0x200000u;
            }
            return;
        }
        float range = weapDef->enemyCrosshairRange;
        if (range * range > distSq)
        {
            if (!ext.lookatent.isDefined())
                ext.lookatent.setEnt(traceEnt);
            ps->weapFlags |= 0x10u;
        }
        return;
    }

    if (traceEnt->s.eType == ET_VEHICLE && !ext.lookatent.isDefined())
    {
        if (traceEnt->health < 0)
            return;
        delta[0] = traceEnt->r.currentOrigin[0] - start[0];
        delta[1] = traceEnt->r.currentOrigin[1] - start[1];
        delta[2] = traceEnt->r.currentOrigin[2] - start[2];
        float distSq = LookAt_DistSq(delta);
        float nameDist = g_friendlyNameDist->current.value;
        if (nameDist * nameDist > distSq)
            ext.lookatent.setEnt(traceEnt);
        if (!(ps->eFlags & 0x4000))
            return;
        float range = weapDef->enemyCrosshairRange;
        if (range * range <= distSq)
            return;
        // SP reads the vehicle's team at scr_vehicle +0x1f4 (KB scr_vehicle_s::team, by name); an
        // enemy vehicle is the enemy crosshair, any other the friendly one. SP has no null test.
        if (!traceEnt->scr_vehicle || !ent->sentient)
            return;
        unsigned int enemyBit = 1u << Sentient_EnemyTeam(ent->sentient->eTeam);
        if ((1u << traceEnt->scr_vehicle->team) & enemyBit)
            ps->weapFlags |= 0x10u;
        else
            ps->weapFlags |= 8u;
        return;
    }

    if (!LookAt_Text0(traceEnt) || ext.lookatent.isDefined())
        return;
    delta[0] = traceEnt->r.currentOrigin[0] - start[0];
    delta[1] = traceEnt->r.currentOrigin[1] - start[1];
    delta[2] = traceEnt->r.currentOrigin[2] - start[2];
    float distSq = LookAt_DistSq(delta);
    float nameDist = g_friendlyNameDist->current.value;
    if (nameDist * nameDist > distSq)
        ext.lookatent.setEnt(traceEnt);
    if (traceEnt->s.eType == ET_SCRIPTMOVER)
    {
        float ffDist = g_friendlyfireDist->current.value;
        if (ffDist * ffDist > distSq)
            ps->weapFlags |= 8u;
    }
}
