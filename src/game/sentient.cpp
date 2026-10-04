#include "sentient.h"
#include <game_mp/g_main_mp.h>
#include <game_mp/g_utils_mp.h>
#include <game_mp/actor_mp.h>
#include "actor_senses.h"
#include <client_mp/g_client_mp.h>
#include "actor_events.h"
#include <clientscript/cscr_vm.h>
#include <clientscript/cscr_stringlist.h>
#include <clientscript/scr_const.h>
#include <game_mp/g_spawn_mp.h>
#include "actor_exposed.h"
#include <universal/memfile.h>

struct SentientGlobals // sizeof=0x28
{                                       // XREF: .data:glob/r
    int lastTime;                       // XREF: G_InitSentients(void)+8/w
    int lastSample;                     // XREF: G_InitSentients(void)+D/w
    float playerTrail[2][3];            // XREF: G_InitSentients(void)+12/w
                                        // G_InitSentients(void)+17/w ...
    int sampleTime[2];                  // XREF: G_InitSentients(void)+30/w
                                        // G_InitSentients(void)+35/w
} glob;

// DERIVED KB layout corresponding to the pointer-free SP globals at 0x023A5950.
static_assert(sizeof(SentientGlobals) == 0x28, "DERIVED sentient globals save layout");
static_assert(offsetof(SentientGlobals, playerTrail) == 8
    && offsetof(SentientGlobals, sampleTime) == 0x20, "DERIVED sentient globals fields");

void Sentient_WriteGlobals(MemoryFile *memFile)
{
    // zombies: write the complete sentient globals image (SP 0x004AAEA0).
    MemFile_WriteData(memFile, sizeof(glob), (unsigned char *)&glob);
}

void Sentient_ReadGlobals(MemoryFile *memFile)
{
    // zombies: restore the sentient globals image before the script VM (SP 0x0047E6E0).
    MemFile_ReadData(memFile, sizeof(glob), (unsigned char *)&glob);
}

// mod (L55): bo1_mod_sentientscan (read at map start, g_main_mp.cpp). Sentient_FirstSentient / NextSentient walk every
// sentient slot reading inuse / eTeam from 1000+ scattered structs (a zombie looking for its enemies passes every other
// zombie). s_modSentientTeam keeps each slot's match bits (0 = free slot, -1 = TEAM_FREE: matches any flags, else
// 1 << eTeam as the x86 shift) beside the structs: set where inuse / eTeam are written (alloc, free, dissociate, set team,
// actor / client team setup) and resynced for every slot at the start of each server frame. 1 scans the copy (same slot,
// same order); 2 scans both and counts disagreements (BO1_SENTIENTSCAN lines).
int g_modSentientScan;
static int s_modSentientTeam[MAX_SENTIENTS_CAP];
static unsigned int s_modSentientScanChecks, s_modSentientScanMisses;
static inline int Sentient_ModTeamBits(const sentient_s *s)
{
    if ( !s->inuse )
        return 0;
    return s->eTeam == TEAM_FREE ? -1 : (int)(1u << ((unsigned int)s->eTeam & 31));
}
void Sentient_ModTeamSync(const sentient_s *s)
{
    if ( s && level.sentients && s >= level.sentients && s < level.sentients + MAX_SENTIENTS )
        s_modSentientTeam[s - level.sentients] = Sentient_ModTeamBits(s);
}
void Sentient_ModTeamResync()
{
    if ( !level.sentients )
        return;
    for ( int i = 0; i < MAX_SENTIENTS; ++i )
        s_modSentientTeam[i] = Sentient_ModTeamBits(&level.sentients[i]);
}
static sentient_s *Sentient_ModScan(int from, int iTeamFlags, sentient_s *retail)
{
    int i;
    for ( i = from; i < MAX_SENTIENTS; ++i )
    {
        const int b = s_modSentientTeam[i];
        if ( (b & iTeamFlags) != 0 || b == -1 )
            break;
    }
    sentient_s *found = i < MAX_SENTIENTS ? &level.sentients[i] : 0;
    if ( g_modSentientScan == 2 )
    {
        if ( found != retail && s_modSentientScanMisses++ < 20 )
            Com_Printf(15, "BO1_SENTIENTSCAN miss %u: from %d flags %x copy %d retail %d\n", s_modSentientScanMisses, from,
                iTeamFlags, found ? (int)(found - level.sentients) : -1, retail ? (int)(retail - level.sentients) : -1);
        if ( (++s_modSentientScanChecks & 0xFFFFF) == 0 )
            Com_Printf(15, "BO1_SENTIENTSCAN checks %u misses %u\n", s_modSentientScanChecks, s_modSentientScanMisses);
        return retail;
    }
    return found;
}

sentient_s *__cdecl Sentient_Alloc()
{
    iassert(level.sentients != NULL);

    for ( int i = 0; i < MAX_SENTIENTS; ++i )
    {
        sentient_s *pSentient = &level.sentients[i];
        if ( !pSentient->inuse )
        {
            memset(pSentient, 0, sizeof(sentient_s));
            pSentient->inuse = 1;
            { void Sentient_ModTeamSync(const sentient_s *); Sentient_ModTeamSync(pSentient); } // mod (L55): bo1_mod_sentientscan
            return pSentient;
        }
    }

    Com_DPrintf(15, "Sentient allocation failed\n");
    return 0;
}

void __cdecl Sentient_Free(sentient_s *sentient)
{
    iassert(sentient);
    iassert(level.sentients);
    iassert(sentient >= level.sentients && sentient < level.sentients + MAX_SENTIENTS);
    iassert(&level.sentients[sentient - level.sentients] == sentient);
    iassert(sentient->ent);
    iassert(sentient->ent->actor == NULL);
    
    for ( int i = 0; i < MAX_ACTORS; ++i )
    {
        actor_s *pActor = &level.actors[i];
        if ( pActor->inuse )
        {
            if ( pActor->pPileUpEnt == sentient->ent)
                Actor_ClearPileUp(pActor);
        }
    }

    G_FreeEntityRefs(sentient->ent);
    Sentient_Dissociate(sentient);
    sentient->ent->sentient = 0;
    SentientHandleDissociate(sentient);
    // zombies: Scr_FreeSentientFields, table 0x00a53f78 (SP 0x004a84e0).
    // SP 0x004751b0 is SentientHandleDissociate above, not an outgoing-field free.
    sentient->targetEnt.setEnt(NULL);
    sentient->scriptTargetEnt.setEnt(NULL);
    sentient->lastAttacker.setEnt(NULL);
    sentient->syncedMeleeEnt.setEnt(NULL);
    // KB-only fields also own references and must be released before slot reuse.
    sentient->scriptOwner.setEnt(NULL);
    Scr_SetString(&sentient->scriptTargetTag, 0, SCRIPTINSTANCE_SERVER);
    if (!zombiemode || !zombiemode->current.enabled)
        memset(sentient, 0xF0, sizeof(sentient_s));
    sentient->inuse = 0;
    { void Sentient_ModTeamSync(const sentient_s *); Sentient_ModTeamSync(sentient); } // mod (L55): bo1_mod_sentientscan
}

void __cdecl Sentient_Dissociate(sentient_s *pSentient)
{
    sentient_s *other; // [esp+4h] [ebp-4h]

    Sentient_SetEnemy(pSentient, 0, 0);
    if ( pSentient->pClaimedNode )
        Path_RelinquishNodeNow(pSentient);
    pSentient->eTeam = TEAM_SPECTATOR;
    { void Sentient_ModTeamSync(const sentient_s *); Sentient_ModTeamSync(pSentient); } // mod (L55): bo1_mod_sentientscan
    for ( other = Sentient_FirstSentient(-1); other; other = Sentient_NextSentient(other, -1) )
    {
        if ( other != pSentient )
            Sentient_DissociateSentient(other, pSentient);
    }
}

void __cdecl Sentient_DissociateSentient(sentient_s *self, sentient_s *other)
{
    iassert(self);
    iassert(self != other);
    iassert(other);
    iassert(other->eTeam == SENTIENT_TEAM_DEAD);

    actor_s *actor = self->ent->actor;
    if ( actor )
        Actor_DissociateSentient(actor, other);

    gclient_s *client = self->ent->client;
    if ( client && client->ps.throwBackGrenadeOwner == other->ent->s.number )
        client->ps.throwBackGrenadeOwner = ENTITYNUM_WORLD;

    if ( self->targetEnt.isDefined() && self->targetEnt.ent()->sentient == other && actor)
        actor->lastEnemySightPosValid = 0;

    for ( int i = 0; i < 4; ++i )
    {
        if ( self->meleeAttackerSpot[i] )
        {
            if ( self->meleeAttackerSpot[i] == other->ent->s.number )
                self->meleeAttackerSpot[i] = 0;
        }
    }
}

void __fastcall Sentient_GetOrigin(const sentient_s *self, float *vOriginOut)
{
    iassert(self);
    iassert(self->ent);
    iassert(self->ent->actor || self->ent->client);
    iassert(vOriginOut);

    Vec3Copy(self->ent->r.currentOrigin, vOriginOut);
}

void __fastcall Sentient_GetVelocity(const sentient_s *self, float *vVelOut)
{
    iassert(self);
    iassert(self->ent);
    iassert(self->ent->actor || self->ent->client);
    iassert(vVelOut);

    if (self->ent->actor)
    {
        Vec3Copy(self->ent->actor->Physics.vVelocity, vVelOut);
    }
    else
    {
        Vec3Copy(self->ent->client->ps.velocity, vVelOut);
    }
}

void __fastcall Sentient_GetEyePosition(const sentient_s *self, float *vEyePosOut)
{
    PROF_SCOPED("Sentient_GetEyePosition");

    iassert(self);
    iassert(self->ent);
    iassert(self->ent->actor || self->ent->client);
    iassert(vEyePosOut);

    if (self->ent->actor)
        Actor_GetEyePosition(self->ent->actor, vEyePosOut);
    else
        G_GetPlayerViewOrigin(&self->ent->client->ps, vEyePosOut);
}

void __fastcall Sentient_GetThirdPersonEyePosition(const sentient_s *self, float *vEyePosOut)
{
    float *currentOrigin; // [esp+8h] [ebp-4h]

    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\sentient.cpp", 325, 0, "%s", "self") )
        __debugbreak();
    if ( !self->ent
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\sentient.cpp", 326, 0, "%s", "self->ent") )
    {
        __debugbreak();
    }
    if ( !self->ent->actor
        && !self->ent->client
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\sentient.cpp",
                    327,
                    0,
                    "%s",
                    "self->ent->actor || self->ent->client") )
    {
        __debugbreak();
    }
    if ( !vEyePosOut
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\sentient.cpp", 328, 0, "%s", "vEyePosOut") )
    {
        __debugbreak();
    }
    if ( self->ent->actor )
    {
        Actor_GetEyePosition(self->ent->actor, vEyePosOut);
    }
    else
    {
        currentOrigin = self->ent->r.currentOrigin;
        *vEyePosOut = *currentOrigin;
        vEyePosOut[1] = currentOrigin[1];
        vEyePosOut[2] = currentOrigin[2];
        vEyePosOut[2] = (float)(self->ent->client->ps.viewHeightCurrent * 0.69999999) + vEyePosOut[2];
    }
}

void __fastcall Sentient_GetDebugEyePosition(const sentient_s *self, float *vEyePosOut)
{
    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\sentient.cpp", 358, 0, "%s", "self") )
        __debugbreak();
    if ( !self->ent
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\sentient.cpp", 359, 0, "%s", "self->ent") )
    {
        __debugbreak();
    }
    if ( !self->ent->actor
        && !self->ent->client
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\sentient.cpp",
                    360,
                    0,
                    "%s",
                    "self->ent->actor || self->ent->client") )
    {
        __debugbreak();
    }
    if ( !vEyePosOut
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\sentient.cpp", 361, 0, "%s", "vEyePosOut") )
    {
        __debugbreak();
    }
    if ( self->ent->actor )
        Actor_GetDebugEyePosition(self->ent->actor, vEyePosOut);
    else
        G_GetPlayerViewOrigin(&self->ent->client->ps, vEyePosOut);
}

pathnode_t *__fastcall Sentient_NearestNode(sentient_s *self)
{
    float dist2; // [esp+48h] [ebp-32Ch]
    int ni; // [esp+4Ch] [ebp-328h]
    float best_dist2; // [esp+50h] [ebp-324h]
    pathnode_t *node; // [esp+5Ch] [ebp-318h]
    int iNodeCount; // [esp+60h] [ebp-314h] BYREF
    pathsort_t nodes[64]; // [esp+64h] [ebp-310h] BYREF
    float vOrigin[3]; // [esp+368h] [ebp-Ch] BYREF

    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\sentient.cpp", 439, 0, "%s", "self") )
        __debugbreak();
    if ( self->bNearestNodeValid )
    {
        if ( self->ent->client )
        {
            Sentient_GetOrigin(self, vOrigin);
            if ( Vec3DistanceSq(vOrigin, self->vNearestNodeCheckPos) > 10000.0 )
            {
                self->bNearestNodeValid = 0;
                self->pNearestNode = 0;
            }
        }
    }
    if ( !self->bNearestNodeValid )
    {
        Sentient_GetOrigin(self, vOrigin);
        node = Path_NearestNodeNotCrossPlanes(
                         vOrigin,
                         nodes,
                         -2,
                         192.0,
                         0,
                         0,
                         0,
                         &iNodeCount,
                         64,
                         NEAREST_NODE_DO_HEIGHT_CHECK);
        if ( !node && !iNodeCount && self->ent->client )
            node = Path_NearestNodeNotCrossPlanes(
                             vOrigin,
                             nodes,
                             -2,
                             512.0,
                             0,
                             0,
                             0,
                             &iNodeCount,
                             64,
                             NEAREST_NODE_DO_HEIGHT_CHECK);
        if ( node )
        {
            self->pNearestNode = node;
            self->bNearestNodeBad = 0;
        }
        else
        {
            if ( iNodeCount )
            {
                if ( !nodes[0].node
                    && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\sentient.cpp", 471, 0, "%s", "nodes[0].node") )
                {
                    __debugbreak();
                }
                self->pNearestNode = nodes[0].node;
                best_dist2 = Vec3DistanceSq(self->ent->r.currentOrigin, nodes[0].node->constant.vOrigin);
                for ( ni = 1; ni < iNodeCount; ++ni )
                {
                    dist2 = Vec3DistanceSq(self->ent->r.currentOrigin, nodes[ni].node->constant.vOrigin);
                    if ( best_dist2 > dist2 )
                    {
                        best_dist2 = dist2;
                        self->pNearestNode = nodes[ni].node;
                    }
                }
            }
            self->bNearestNodeBad = 1;
        }
        self->bNearestNodeValid = 1;
        self->vNearestNodeCheckPos[0] = vOrigin[0];
        self->vNearestNodeCheckPos[1] = vOrigin[1];
        self->vNearestNodeCheckPos[2] = vOrigin[2];
    }
    if ( !self->bNearestNodeValid
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\sentient.cpp",
                    491,
                    0,
                    "%s",
                    "self->bNearestNodeValid") )
    {
        __debugbreak();
    }
    return self->pNearestNode;
}

pathnode_t *__fastcall Sentient_NearestNodeSuppressed(
                sentient_s *self,
                float (*vNormal)[2],
                float *fDist,
                int iPlaneCount)
{
    int iNodeCount; // [esp+24h] [ebp-314h] BYREF
    pathsort_t nodes[64]; // [esp+28h] [ebp-310h] BYREF
    float vOrigin[3]; // [esp+32Ch] [ebp-Ch] BYREF

    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\sentient.cpp", 509, 0, "%s", "self") )
        __debugbreak();
    Sentient_GetOrigin(self, vOrigin);
    return Path_NearestNodeNotCrossPlanes(
                     vOrigin,
                     nodes,
                     -2,
                     192.0,
                     vNormal,
                     fDist,
                     iPlaneCount,
                     &iNodeCount,
                     64,
                     NEAREST_NODE_DO_HEIGHT_CHECK);
}

void __fastcall Sentient_InvalidateNearestNode(sentient_s *self)
{
    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\sentient.cpp", 525, 0, "%s", "self") )
        __debugbreak();
    self->bNearestNodeValid = 0;
}

void __fastcall Sentient_SetEnemy(sentient_s *self, gentity_s *enemy, int bNotify)
{
    gentity_s *myEnemy; // [esp+8h] [ebp-8h]
    actor_s *actor; // [esp+Ch] [ebp-4h]

    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\sentient.cpp", 544, 0, "%s", "self") )
        __debugbreak();
    if ( self->ent == enemy
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\sentient.cpp", 545, 0, "%s", "self->ent != enemy") )
    {
        __debugbreak();
    }
    if ( !self->targetEnt.isDefined() )
    {
        if ( !enemy )
            return;
        goto LABEL_21;
    }
    myEnemy = self->targetEnt.ent();
    if ( bNotify && myEnemy->sentient && self->iEnemyNotifyTime && level.time >= self->iEnemyNotifyTime )
    {
        Actor_BroadcastTeamEvent(self, AI_EV_NEW_ENEMY);
        self->iEnemyNotifyTime = 0;
    }
    if ( self->targetEnt.ent() != enemy )
    {
        if ( myEnemy->sentient )
        {
            if ( bNotify && self->iEnemyNotifyTime )
            {
                Actor_BroadcastTeamEvent(self, AI_EV_NEW_ENEMY);
                self->iEnemyNotifyTime = 0;
            }
            --myEnemy->sentient->attackerCount;
        }
LABEL_21:
        actor = self->ent->actor;
        if ( enemy && enemy->sentient )
        {
            ++enemy->sentient->attackerCount;
            self->iEnemyNotifyTime = level.time + 1000;
        }
        else
        {
            self->iEnemyNotifyTime = 0;
        }
        //EntHandle::setEnt(&self->targetEnt, enemy);
        self->targetEnt.setEnt(enemy);
        if ( bNotify && Scr_IsSystemActive(1u, SCRIPTINSTANCE_SERVER) )
            Scr_Notify(self->ent, scr_const.enemy, 0);
        if ( actor )
        {
            if ( actor->useEnemyGoal )
            {
                actor->useEnemyGoal = 0;
                Actor_UpdateGoalPos(actor);
            }
            actor->iPotentialCoverNodeCount = 0;
            actor->iPotentialReacquireNodeCount = 0;
            if ( actor->provideCoveringFire && enemy && enemy->sentient )
            {
                actor->lastEnemySightPosValid = 1;
                Sentient_GetEyePosition(enemy->sentient, actor->lastEnemySightPos);
            }
            else
            {
                actor->lastEnemySightPosValid = 0;
                Actor_UpdateLastEnemySightPos(actor);
            }
            //if ( EntHandle::isDefined(&self->targetEnt) )
            if ( self->targetEnt.isDefined() )
                self->ent->s.lerp.u.turret.ownerNum = self->targetEnt.entnum();
            else
                self->ent->s.lerp.u.turret.ownerNum = 1023;
        }
    }
}

sentient_s *__fastcall Sentient_FirstSentient(int iTeamFlags)
{
    int i; // [esp+4h] [ebp-4h]

    if ( iTeamFlags > 15
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\sentient.cpp",
                    675,
                    0,
                    "%s",
                    "iTeamFlags <= (1 << TEAM_NUM_TEAMS) - 1") )
    {
        __debugbreak();
    }
    if ( g_modSentientScan == 1 ) // mod (L55): bo1_mod_sentientscan
        return Sentient_ModScan(0, iTeamFlags, 0);
    sentient_s *retail = 0;
    for ( i = 0; i < MAX_SENTIENTS; ++i )
    {
        if ( level.sentients[i].inuse
            && ((iTeamFlags & (1 << level.sentients[i].eTeam)) != 0 || level.sentients[i].eTeam == TEAM_FREE) )
        {
            retail = &level.sentients[i];
            break;
        }
    }
    return g_modSentientScan == 2 ? Sentient_ModScan(0, iTeamFlags, retail) : retail;
}

sentient_s *__fastcall Sentient_NextSentient(sentient_s *pPrevSentient, int iTeamFlags)
{
    int i; // [esp+8h] [ebp-4h]

    if ( iTeamFlags > 15
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\sentient.cpp",
                    708,
                    0,
                    "%s",
                    "iTeamFlags <= (1 << TEAM_NUM_TEAMS) - 1") )
    {
        __debugbreak();
    }
    if ( !pPrevSentient
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\sentient.cpp", 710, 0, "%s", "pPrevSentient") )
    {
        __debugbreak();
    }
    if ( (pPrevSentient < level.sentients || pPrevSentient >= &level.sentients[MAX_SENTIENTS])
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\sentient.cpp",
                    711,
                    0,
                    "%s",
                    "pPrevSentient >= level.sentients && pPrevSentient < level.sentients + MAX_SENTIENTS") )
    {
        __debugbreak();
    }
    if ( pPrevSentient != &level.sentients[pPrevSentient - level.sentients]
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\sentient.cpp",
                    712,
                    0,
                    "%s",
                    "pPrevSentient == level.sentients + (pPrevSentient - level.sentients)") )
    {
        __debugbreak();
    }
    if ( g_modSentientScan == 1 ) // mod (L55): bo1_mod_sentientscan
        return Sentient_ModScan(pPrevSentient - level.sentients + 1, iTeamFlags, 0);
    sentient_s *retail = 0;
    for ( i = pPrevSentient - level.sentients + 1; i < MAX_SENTIENTS; ++i )
    {
        if ( level.sentients[i].inuse
            && ((iTeamFlags & (1 << level.sentients[i].eTeam)) != 0 || level.sentients[i].eTeam == TEAM_FREE) )
        {
            retail = &level.sentients[i];
            break;
        }
    }
    return g_modSentientScan == 2 ? Sentient_ModScan(pPrevSentient - level.sentients + 1, iTeamFlags, retail) : retail;
}

const char *pszTeamName[4] = { "free", "axis", "allies", "spectator" };

const char *__fastcall Sentient_NameForTeam(unsigned int eTeam)
{
    if ( eTeam >= 4
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\sentient.cpp",
                    744,
                    0,
                    "%s",
                    "eTeam >= 0 && eTeam < TEAM_NUM_TEAMS") )
    {
        __debugbreak();
    }
    return pszTeamName[eTeam];
}

void __fastcall Sentient_SetTeam(sentient_s *self, team_t eTeam)
{
    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\sentient.cpp", 778, 0, "%s", "self != 0") )
        __debugbreak();
    if ( (unsigned int)eTeam >= TEAM_NUM_TEAMS
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\sentient.cpp",
                    780,
                    0,
                    "%s",
                    "eTeam >= TEAM_FREE && eTeam < TEAM_NUM_TEAMS") )
    {
        __debugbreak();
    }
    if ( self->eTeam != eTeam )
        self->eTeam = eTeam;
    { void Sentient_ModTeamSync(const sentient_s *); Sentient_ModTeamSync(self); } // mod (L55): bo1_mod_sentientscan
}

void __fastcall Sentient_ClaimNode(sentient_s *self, pathnode_t *node)
{
    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\sentient.cpp", 804, 0, "%s", "self") )
        __debugbreak();
    if ( !self->ent
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\sentient.cpp", 805, 0, "%s", "self->ent") )
    {
        __debugbreak();
    }
    if ( self->ent->actor
        && Actor_KeepClaimedNode(self->ent->actor)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\sentient.cpp",
                    806,
                    0,
                    "%s",
                    "!self->ent->actor || !Actor_KeepClaimedNode(self->ent->actor)") )
    {
        __debugbreak();
    }
    if ( self->pClaimedNode != node )
    {
        self->pPrevClaimedNode = self->pClaimedNode;
        if ( self->pClaimedNode )
            Path_RelinquishNodeNow(self);
        if ( node )
        {
            if ( (node->constant.spawnflags & 0x40) != 0 && level.time <= node->dynamic.iFreeTime )
                Path_ForceClaimNode(node, self);
            else
                Path_ClaimNode(node, self);
        }
    }
}

void __fastcall Sentient_NodeClaimRevoked(sentient_s *self, pathnode_t *node)
{
    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\sentient.cpp", 837, 0, "%s", "self") )
        __debugbreak();
    if ( !node && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\sentient.cpp", 838, 0, "%s", "node") )
        __debugbreak();
    if ( node != self->pClaimedNode
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\sentient.cpp",
                    839,
                    0,
                    "%s",
                    "node == self->pClaimedNode") )
    {
        __debugbreak();
    }
    if ( !self->ent->actor
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\sentient.cpp", 841, 0, "%s", "self->ent->actor") )
    {
        __debugbreak();
    }
    Actor_NodeClaimRevoked(self->ent->actor, 5000);
    if ( self->pClaimedNode == node
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\sentient.cpp",
                    845,
                    0,
                    "%s",
                    "self->pClaimedNode != node") )
    {
        __debugbreak();
    }
}

void __fastcall Sentient_StealClaimNode(sentient_s *self, sentient_s *other)
{
    pathnode_t *node; // [esp+8h] [ebp-4h]

    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\sentient.cpp", 858, 0, "%s", "self") )
        __debugbreak();
    if ( !other && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\sentient.cpp", 859, 0, "%s", "other") )
        __debugbreak();
    if ( !other->ent->actor
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\sentient.cpp", 860, 0, "%s", "other->ent->actor") )
    {
        __debugbreak();
    }
    node = other->pClaimedNode;
    if ( node )
        Path_ForceClaimNode(node, self);
}

void __fastcall Sentient_BanNearNodes(sentient_s *self)
{
    pathnode_t *node; // [esp+34h] [ebp-330h]
    float delta[2]; // [esp+38h] [ebp-32Ch] BYREF
    int iNodeCount; // [esp+40h] [ebp-324h]
    float origin[2]; // [esp+44h] [ebp-320h]
    pathsort_t nodes[64]; // [esp+4Ch] [ebp-318h] BYREF
    pathnode_t *pTestNode; // [esp+350h] [ebp-14h]
    float forward[2]; // [esp+354h] [ebp-10h]
    team_t eTeam; // [esp+35Ch] [ebp-8h]
    int i; // [esp+360h] [ebp-4h]

    if ( level.time - self->banNodeTime >= 0 )
    {
        self->banNodeTime = level.time + 4950;
        node = self->pClaimedNode;
        if ( node )
        {
            if ( Path_IsValidClaimNode(node) )
            {
                iNodeCount = Path_NodesInCylinder(node->constant.vOrigin, 80.0, 80.0, nodes, 64, 540668);
                eTeam = self->eTeam;
                forward[0] = node->constant.forward[0];
                forward[1] = node->constant.forward[1];
                origin[0] = node->constant.vOrigin[0];
                origin[1] = node->constant.vOrigin[1];
                for ( i = 0; i < iNodeCount; ++i )
                {
                    pTestNode = nodes[i].node;
                    if ( (pTestNode->constant.spawnflags & 0x8000) != 0 && pTestNode->dynamic.iFreeTime != 0x7FFFFFFF )
                    {
                        delta[0] = origin[0] - pTestNode->constant.vOrigin[0];
                        delta[1] = origin[1] - pTestNode->constant.vOrigin[1];
                        Vec2Normalize(delta);
                        if ( (float)((float)(forward[0] * delta[0]) + (float)(forward[1] * delta[1])) <= -0.5
                            && (float)((float)(pTestNode->constant.forward[0] * delta[0])
                                             + (float)(pTestNode->constant.forward[1] * delta[1])) >= 0.5 )
                        {
                            Path_MarkNodeInvalid(pTestNode, eTeam);
                        }
                    }
                }
            }
        }
    }
}

void __cdecl G_InitSentients()
{
    // zombies: SP 0x005954A0 clears 0x023A5950, sentient inuse flags, then threat bias groups.
    // Earlier save notes said this block was cleared with turrets; the exe does sentient init.
    int i; // [esp+4h] [ebp-4h]

    glob.lastTime = 0;
    glob.lastSample = 0;
    *(_QWORD *)&glob.playerTrail[0][0] = 0;
    glob.playerTrail[0][2] = 0.0;
    *(_QWORD *)&glob.playerTrail[1][0] = 0;
    glob.playerTrail[1][2] = 0.0;
    glob.sampleTime[0] = 0;
    glob.sampleTime[1] = 0;
    for ( i = 0; i < MAX_SENTIENTS; ++i )
        level.sentients[i].inuse = 0;
    Sentient_ModTeamResync(); // mod (L55): bo1_mod_sentientscan
    Actor_InitThreatBiasGroups();
}

bool __fastcall Sentient_NearestNodeDirty(sentient_s *self, bool originChanged)
{
    if ( originChanged )
        self->originChanged = 1;
    if ( (level.time + 50 * self->ent->s.number) % 1000 )
        return 0;
    if ( self->originChanged )
    {
        self->originChanged = 0;
        return 1;
    }
    else
    {
        return !self->pNearestNode || !self->pNearestNode->dynamic.wLinkCount;
    }
}

