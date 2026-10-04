// j1: play-session recorder (TOOL for a playtests, off by default; see g_sp_playtrace.h and notes/playtrace.md).
// It only reads game state; it never changes it. The client half is src/cgame/cg_sp_playtrace.cpp.
#include "g_sp_playtrace.h"
#include <Windows.h>
#include <cstdio>
#include <cstdarg>
#include <ctime>
#include <string>
#include <game_mp/g_main_mp.h>
#include <game/actor.h>
#include <game/actor_animapi.h>
#include <game/sentient.h>
#include <game_mp/actor_mp.h>
#include <bgame/bg_weapons.h>
#include <bgame/bg_weapons_def.h>
#include <bgame/bg_mantle.h>
#include <qcommon/common.h>
#include <qcommon/cmd.h>
#include <universal/dvar.h>
#include <win32/win_shared.h>
#include <server/sv_world.h>
#include <qcommon/cm_world.h>
#include <game_mp/g_utils_mp.h>
#include <xanim/xanim.h>

namespace
{
const dvar_s *s_on;
const dvar_s *s_ms;
const dvar_s *s_radius;
cmd_function_s s_markCmd;
bool s_registered;
bool s_openFailed; // one attempt per enable
FILE *s_file;
char s_path[MAX_PATH];
int s_marks;
int s_nextSample = -1;
int s_lastLevelTime = -1;
int s_health[64];
int s_tmResult[MAX_ACTORS_CAP]; // last Actor_GetTeamMoveStatus result
int s_pcReason[MAX_ACTORS_CAP]; // last exposed-combat path clear reason, 0 none
int s_pcTime[MAX_ACTORS_CAP];
int s_plReason[MAX_ACTORS_CAP];
int s_plTime[MAX_ACTORS_CAP];
int s_deadSince[MAX_ACTORS_CAP]; // L9: level time an actor was first sampled dead, 0 = alive / free

struct Lock
{
    CRITICAL_SECTION cs;
    Lock() { InitializeCriticalSection(&cs); }
} s_lock;

void OpenFile()
{
    const dvar_s *home = Dvar_FindVar("fs_homepath");
    std::string dir = home && home->current.string[0] ? home->current.string : ".";
    dir += "\\playtrace";
    CreateDirectoryA(dir.c_str(), nullptr);
    time_t now = time(nullptr);
    tm local{};
    localtime_s(&local, &now);
    sprintf_s(s_path, "%s\\playtrace-%04d%02d%02d-%02d%02d%02d.jsonl", dir.c_str(), local.tm_year + 1900,
        local.tm_mon + 1, local.tm_mday, local.tm_hour, local.tm_min, local.tm_sec);
    if (fopen_s(&s_file, s_path, "ab"))
        s_file = nullptr;
    Com_Printf(16, "PLAYTRACE: %s %s\n", s_file ? "writing" : "could not open", s_path);
}

void MarkCmd()
{
    G_SP_PlayTraceMark("command");
}

const char *ScriptName(const actor_s *actor)
{
    const AnimScriptList *list = actor->species < MAX_AI_SPECIES ? g_animScriptTable[actor->species] : nullptr;
    const scr_animscript_t *s = actor->pAnimScriptFunc;
    if (!list || !s)
        return "none";
    if (s == &list->move) return "move";
    if (s == &list->combat) return "combat";
    if (s == &list->stop) return "stop";
    if (s == &list->scripted) return "scripted";
    if (s == &list->pain) return "pain";
    if (s == &list->death) return "death";
    if (s == &list->react) return "react";
    if (s == &list->turn) return "turn";
    if (s == &list->jump) return "jump";
    if (s == &list->init) return "init";
    if (s == &list->flashed) return "flashed";
    if (s == &list->cover_arrival) return "cover_arrival";
    if (s == &actor->AnimScriptSpecific) return "specific"; // traversals / custom animscripts
    return "other";
}

const char *Stance(const playerState_s *ps)
{
    if (ps->viewHeightTarget <= 20) return "prone";
    if (ps->viewHeightTarget <= 45) return "crouch";
    return "stand";
}

void Sample()
{
    const float radius = s_radius->current.value;
    std::string line;
    char buf[512];
    const dvar_s *map = Dvar_FindVar("mapname");
    sprintf_s(buf, "{\"k\":\"s\",\"t\":%d,\"map\":\"%s\",\"p\":[", level.time, map ? map->current.string : "");
    line = buf;
    int actorsAlive = 0;
    for (int a = 0; level.actors && a < MAX_ACTORS; ++a)
        if (level.actors[a].inuse && level.actors[a].ent && level.actors[a].ent->health > 0)
            ++actorsAlive;
    bool firstPlayer = true;
    for (int i = 0; i < level.maxclients && i < 64; ++i)
    {
        const gentity_s *pl = &g_entities[i];
        if (!pl->r.inuse || !pl->client)
            continue;
        const playerState_s *ps = &pl->client->ps;
        const WeaponVariantDef *variant = ps->weapon ? BG_GetWeaponVariantDef(ps->weapon) : nullptr;
        sprintf_s(buf, "%s{\"n\":%d,\"o\":[%.1f,%.1f,%.1f],\"yaw\":%.0f,\"hp\":%d,\"maxhp\":%d,\"stance\":\"%s\","
            "\"sess\":%d,\"pm\":%d,\"ef\":%d,\"w\":\"%s\",\"clip\":%d,\"res\":%d,\"pts\":%d,\"z\":[",
            firstPlayer ? "" : ",", i, pl->r.currentOrigin[0], pl->r.currentOrigin[1], pl->r.currentOrigin[2],
            ps->viewangles[1], pl->health, pl->client->sess.maxHealth, Stance(ps),
            (int)pl->client->sess.sessionState, ps->pm_type, ps->eFlags,
            variant ? variant->szInternalName : "none", ps->weapon ? BG_GetAmmoInClip(ps, ps->weapon) : 0,
            ps->weapon ? BG_GetAmmoNotInClip(ps, ps->weapon) : 0, pl->client->sess.cs.score.score);
        line += buf;
        firstPlayer = false;
        bool firstZombie = true;
        for (int a = 0; level.actors && a < MAX_ACTORS; ++a)
        {
            const actor_s *actor = &level.actors[a];
            const gentity_s *ent = actor->ent;
            if (!actor->inuse || !ent || ent->health <= 0)
                continue;
            const float dx = ent->r.currentOrigin[0] - pl->r.currentOrigin[0];
            const float dy = ent->r.currentOrigin[1] - pl->r.currentOrigin[1];
            const float dz = ent->r.currentOrigin[2] - pl->r.currentOrigin[2];
            const float d2 = sqrtf(dx * dx + dy * dy);
            if (d2 > radius || fabsf(dz) > radius)
                continue;
            sentient_s *enemy = Actor_GetTargetSentient(const_cast<actor_s *>(actor));
            const float gdx = ent->r.currentOrigin[0] - actor->codeGoal.pos[0];
            const float gdy = ent->r.currentOrigin[1] - actor->codeGoal.pos[1];
            // ye: animscripts GetYawToEnemy against this player (own yaw - yaw to the player, -180..180).
            float yawToPlayer = ent->r.currentAngles[1] - atan2f(-dy, -dx) * (180.0f / 3.14159265f);
            yawToPlayer -= 360.0f * floorf((yawToPlayer + 180.0f) / 360.0f);
            // kn: ms since this actor last knew the player's position (sentientInfo lastKnownPosTime), -1 = never;
            // vis: its sight cache says visible.
            int known = -1, visible = 0;
            if (pl->sentient)
            {
                const sentient_info_t *info = &actor->sentientInfo[pl->sentient - level.sentients];
                known = info->lastKnownPosTime > 0 ? level.time - info->lastKnownPosTime : -1;
                visible = info->VisCache.bVisible ? 1 : 0;
            }
            // or: CodeOrient.eMode; lv: ms since its target sentient was last visible in its sight cache
            // (-1 = never, -2 = no target sentient); un: that target's attackTime is 0x7fffffff (FlagEnemyUnattackable).
            int lastVis = -2, unattackable = 0;
            if (enemy)
            {
                const sentient_info_t *einfo = &actor->sentientInfo[enemy - level.sentients];
                lastVis = einfo->VisCache.iLastVisTime ? level.time - einfo->VisCache.iLastVisTime : -1;
                unattackable = einfo->attackTime == 0x7FFFFFFF ? 1 : 0;
            }
            // atk: ms since this actor's last melee()/shot (Actor_Melee sets lastShotTime), -1 = never.
            sprintf_s(buf, "%s{\"e\":%d,\"d\":%.0f,\"dz\":%.0f,\"hp\":%d,\"sc\":\"%s\",\"ai\":%d,\"en\":%d,\"atk\":%d,"
                "\"gr\":%.0f,\"gd\":%.0f,\"path\":%d,\"yaw\":%.0f,\"ye\":%.0f,\"kn\":%d,\"vis\":%d,\"safe\":%d,"
                "\"cl\":%d,\"pu\":%d,\"mm\":%d,\"ss\":%d,\"tw\":%d,\"tm\":%d,\"pc\":%d,\"pca\":%d,\"hit\":%d,\"pl\":%d,\"pla\":%d,"
                "\"or\":%d,\"lv\":%d,\"un\":%d}",
                firstZombie ? "" : ",", ent->s.number, d2, dz, ent->health, ScriptName(actor),
                (int)actor->eState[actor->stateLevel], enemy && enemy->ent ? enemy->ent->s.number : -1,
                actor->lastShotTime > 0 ? level.time - actor->lastShotTime : -1, actor->codeGoal.radius,
                sqrtf(gdx * gdx + gdy * gdy), (int)actor->Path.wPathLen, ent->r.currentAngles[1],
                yawToPlayer, known, visible, actor->safeToChangeScript ? 1 : 0,
                actor->pCloseEnt.isDefined() ? const_cast<actor_s *>(actor)->pCloseEnt.ent()->s.number : -1,
                actor->pPileUpActor && actor->pPileUpEnt ? actor->pPileUpEnt->s.number : -1, (int)actor->moveMode,
                (int)actor->eSubState[actor->stateLevel], actor->iTeamMoveWaitTime - level.time, s_tmResult[a],
                s_pcReason[a], s_pcReason[a] ? level.time - s_pcTime[a] : -1, actor->Physics.iHitEntnum, s_plReason[a], s_plReason[a] ? level.time - s_plTime[a] : -1,
                (int)actor->CodeOrient.eMode, lastVis, unattackable);
            line += buf;
            firstZombie = false;
        }
        line += "]}";
    }
    sprintf_s(buf, "],\"alive\":%d,\"dead\":[", actorsAlive);
    line += buf;
    // L9: every dead actor still in the actor array (death animscript / not yet a corpse): a stuck death shows here.
    bool firstDead = true;
    for (int a = 0; level.actors && a < MAX_ACTORS; ++a)
    {
        const actor_s *actor = &level.actors[a];
        if (!actor->inuse || !actor->ent || actor->ent->health > 0)
        {
            s_deadSince[a] = 0;
            continue;
        }
        if (!s_deadSince[a])
            s_deadSince[a] = level.time;
        line += firstDead ? "{" : ",{";
        G_SP_PlayTraceDeadActor(actor, s_deadSince[a], buf, sizeof(buf));
        line += buf;
        line += "}";
        firstDead = false;
    }
    line += "]}";
    G_SP_PlayTraceLine(line.c_str());
}
} // namespace

const char *G_SP_PlayTraceScriptName(const actor_s *actor)
{
    return ScriptName(actor);
}

void G_SP_PlayTraceDeadActor(const actor_s *actor, int deadSince, char *buf, int size)
{
    gentity_s *ent = actor->ent;
    int len = sprintf_s(buf, size, "\"e\":%d,\"hp\":%d,\"sc\":\"%s\",\"ai\":%d,\"dead\":%d,\"tr\":%d,\"o\":[%.1f,%.1f,%.1f],\"anim\":\"",
        ent->s.number, ent->health, ScriptName(actor), (int)actor->eState[actor->stateLevel], level.time - deadSince,
        (int)ent->s.lerp.pos.trType, ent->r.currentOrigin[0], ent->r.currentOrigin[1], ent->r.currentOrigin[2]);
    XAnimTree_s *tree = G_GetEntAnimTree(ent);
    int shown = 0;
    for (unsigned int i = 0; tree && tree->anims && i < tree->anims->size && shown < 4 && len > 0 && len < size - 80; ++i)
    {
        const char *name = XAnimGetAnimName(tree->anims, i);
        const float weight = (float)XAnimGetWeight(tree, i);
        if (!name[0] || weight <= 0.0f || XAnimGetNumChildren(tree->anims, i) > 0) // leaves only (the playing anims)
            continue;
        len += sprintf_s(buf + len, size - len, "%s%s:%.2f:%.2f", shown ? " " : "", name, (float)XAnimGetTime(tree, i), weight);
        ++shown;
    }
    if (len > 0 && len < size - 2)
        sprintf_s(buf + len, size - len, "\"");
}

void G_SP_PlayTraceRegister()
{
    if (s_registered)
        return;
    s_registered = true;
    s_on = _Dvar_RegisterBool("bo1_playtrace", false, 0,
        "Play-session recorder (tool): write <fs_homepath>\\playtrace\\playtrace-<date>.jsonl; F8 drops a MARK");
    s_ms = _Dvar_RegisterInt("bo1_playtrace_ms", 1000, 50, 60000, 0, "Play-session recorder sample interval (level ms)");
    s_radius = _Dvar_RegisterFloat("bo1_playtrace_radius", 400.0f, 32.0f, 4096.0f, 0,
        "Play-session recorder: zombies within this 2D distance of a player are listed");
    Cmd_AddCommandInternal("playtrace_mark", MarkCmd, &s_markCmd);
}

bool G_SP_PlayTraceOn()
{
    return s_on && s_on->current.enabled;
}

int G_SP_PlayTraceIntervalMs()
{
    return s_ms ? s_ms->current.integer : 1000;
}

void G_SP_PlayTraceLine(const char *line)
{
    EnterCriticalSection(&s_lock.cs);
    if (!s_file && !s_openFailed)
    {
        OpenFile();
        s_openFailed = !s_file;
    }
    if (s_file)
    {
        fputs(line, s_file);
        fputc('\n', s_file);
        fflush(s_file);
    }
    LeaveCriticalSection(&s_lock.cs);
}

void G_SP_PlayTraceServerFrame()
{
    if (!s_on || !s_on->current.enabled)
    {
        s_openFailed = false;
        if (s_file)
        {
            EnterCriticalSection(&s_lock.cs);
            if (s_file)
                fclose(s_file);
            s_file = nullptr;
            LeaveCriticalSection(&s_lock.cs);
        }
        return;
    }
    char buf[160];
    if (level.time < s_lastLevelTime) // new map / map restart
        s_nextSample = -1;
    s_lastLevelTime = level.time;
    for (int i = 0; i < level.maxclients && i < 64; ++i)
    {
        const gentity_s *pl = &g_entities[i];
        const int hp = pl->r.inuse && pl->client ? pl->health : 0;
        if (s_nextSample >= 0 && hp < s_health[i] && pl->r.inuse && pl->client)
        {
            sprintf_s(buf, "{\"k\":\"hurt\",\"t\":%d,\"n\":%d,\"hp\":%d,\"was\":%d}", level.time, i, hp, s_health[i]);
            G_SP_PlayTraceLine(buf);
        }
        s_health[i] = hp;
    }
    if (s_nextSample >= 0 && level.time < s_nextSample)
        return;
    s_nextSample = level.time + s_ms->current.integer;
    Sample();
}

void G_SP_PlayTraceMelee(const actor_s *self, const gentity_s *hitEnt)
{
    if (!s_on || !s_on->current.enabled || !self || !self->ent)
        return;
    sentient_s *enemy = Actor_GetTargetSentient(const_cast<actor_s *>(self));
    float d = -1.0f;
    if (enemy && enemy->ent)
    {
        const float dx = enemy->ent->r.currentOrigin[0] - self->ent->r.currentOrigin[0];
        const float dy = enemy->ent->r.currentOrigin[1] - self->ent->r.currentOrigin[1];
        d = sqrtf(dx * dx + dy * dy);
    }
    const int a = level.actors ? (int)(self - level.actors) : -1;
    const bool inRange = a >= 0 && a < MAX_ACTORS;
    char buf[200];
    sprintf_s(buf, "{\"k\":\"melee\",\"t\":%d,\"e\":%d,\"en\":%d,\"d\":%.0f,\"hit\":%d,\"pl\":%d,\"pla\":%d}", level.time,
        self->ent->s.number, enemy && enemy->ent ? enemy->ent->s.number : -1, d, hitEnt ? hitEnt->s.number : -1,
        inRange ? s_plReason[a] : 0, inRange && s_plReason[a] ? level.time - s_plTime[a] : -1);
    G_SP_PlayTraceLine(buf);
}

void G_SP_PlayTraceMark(const char *source)
{
    if (!s_on || !s_on->current.enabled)
        return;
    const int n = ++s_marks;
    char buf[160];
    sprintf_s(buf, "{\"k\":\"mark\",\"t\":%d,\"n\":%d,\"src\":\"%s\",\"rt\":%d}", level.time, n, source,
        Sys_Milliseconds());
    G_SP_PlayTraceLine(buf);
    Com_Printf(16, "PLAYTRACE MARK %d at level time %d\n", n, level.time);
}

void G_SP_PlayTraceTeamMove(const actor_s *self, int result)
{
    if (!s_on || !s_on->current.enabled || !self || !level.actors)
        return;
    const int a = (int)(self - level.actors);
    if (a >= 0 && a < MAX_ACTORS)
        s_tmResult[a] = result;
}

void G_SP_PlayTracePathClear(const actor_s *self, int reason)
{
    if (!s_on || !s_on->current.enabled || !self || !level.actors)
        return;
    const int a = (int)(self - level.actors);
    if (a >= 0 && a < MAX_ACTORS)
    {
        s_pcReason[a] = reason;
        s_pcTime[a] = level.time;
        if (reason < 100 && self->Path.wPathLen > 0)
        {
            s_plReason[a] = reason;
            s_plTime[a] = level.time;
        }
    }
}

void G_SP_PlayTraceInvalidPath(const actor_s *self, const sentient_s *enemy, int branch, bool hadPath)
{
    if (!s_on || !s_on->current.enabled || !self || !self->ent || !enemy || !enemy->ent)
        return;
    const float *o = self->ent->r.currentOrigin;
    const float *en = enemy->ent->r.currentOrigin;
    const path_t *p = &self->Path;
    const float lx = p->lookaheadDir[0] * p->fLookaheadDist + o[0] - en[0];
    const float ly = p->lookaheadDir[1] * p->fLookaheadDist + o[1] - en[1];
    const float *end = p->wPathLen > 0 ? p->pts[0].vOrigPoint : p->vFinalGoal;
    // blk: 2D distance from the segment self -> enemy of the nearest other live actor strictly between them.
    float blk = 99.0f;
    int blkEnt = -1;
    const float ax = en[0] - o[0], ay = en[1] - o[1], l2 = ax * ax + ay * ay;
    for (int i = 0; l2 > 0.0f && i < MAX_ACTORS; ++i)
    {
        const actor_s *y = &level.actors[i];
        if (y == self || !y->inuse || !y->ent || y->ent->health <= 0)
            continue;
        const float *yo = y->ent->r.currentOrigin;
        const float t = ((yo[0] - o[0]) * ax + (yo[1] - o[1]) * ay) / l2;
        if (t <= 0.0f || t >= 1.0f)
            continue;
        const float px = o[0] + ax * t - yo[0], py = o[1] + ay * t - yo[1];
        const float dd = sqrtf(px * px + py * py);
        if (dd < blk)
        {
            blk = dd;
            blkEnt = y->ent->s.number;
        }
    }
    const vis_cache_t *vc = &self->sentientInfo[enemy - level.sentients].VisCache;
    // st/sh: a fresh sight trace eye -> enemy eye with Actor_SightTrace_SP's mask (0x2809803), passing self and the
    // enemy: 1 = clear; sh = the hit number (-1 = an entity, >0 = world brush/hint).
    float eye[3], enEye[3];
    Vec3Copy(self->eyeInfo.pos, eye);
    Sentient_GetEyePosition(enemy, enEye);
    col_context_t tc;
    tc.mask = 0x2809803;
    tc.passEntityNum0 = self->ent->s.number;
    tc.passEntityNum1 = enemy->ent->s.number;
    int sightHit = 0;
    const bool sightClear = SV_SightTracePoint(&sightHit, eye, enEye, &tc);
    // bc/bl/bf/bz/bd/bx: the nearest blocker's r.contents, r.linked, svFlags, mins z/maxs z/origin z - self z,
    // a direct SV_SightTracePointToEntity on it (0 = miss) and a direct CM_TransformedBoxSightTrace on its clip box.
    int bc = -1, bl = -1, bf = -1, bd = 0, bx = 0;
    float bz[3] = { 0.0f, 0.0f, 0.0f };
    if (blkEnt >= 0)
    {
        const gentity_s *be = &g_entities[blkEnt];
        bc = be->r.contents;
        bl = be->r.linked;
        bf = be->r.svFlags;
        bz[0] = be->r.mins[2];
        bz[1] = be->r.maxs[2];
        bz[2] = be->r.currentOrigin[2] - o[2];
        sightpointtrace_t clip;
        Vec3Copy(eye, clip.start);
        Vec3Copy(enEye, clip.end);
        clip.passEntityNum[0] = tc.passEntityNum0;
        clip.passEntityNum[1] = tc.passEntityNum1;
        clip.contentmask = tc.mask;
        clip.locational = tc.locational;
        clip.priorityMap = tc.priorityMap;
        bd = SV_SightTracePointToEntity(&clip, blkEnt);
        bx = CM_TransformedBoxSightTrace(0, eye, enEye, vec3_origin, vec3_origin, SV_ClipHandleForEntity(be),
            tc.mask, be->r.currentOrigin, vec3_origin);
    }
    char buf[1024];
    sprintf_s(buf, "{\"k\":\"hip\",\"t\":%d,\"e\":%d,\"b\":%d,\"had\":%d,\"d\":%.2f,\"la\":%.3f,"
        "\"lad\":[%.4f,%.4f],\"le\":[%.4f,%.4f],\"dot\":%.5f,\"fg\":[%.3f,%.3f],\"p0\":[%.3f,%.3f],\"cg\":[%.3f,%.3f],"
        "\"cp\":[%.3f,%.3f],\"len\":%d,\"ln\":%d,\"neg\":%d,\"fl\":%d,\"amt\":%.1f,\"cl\":%.1f,\"ue\":%d,\"es\":[%.1f,%.1f]}",
        level.time, self->ent->s.number, branch, hadPath ? 1 : 0, sqrtf((o[0] - en[0]) * (o[0] - en[0]) + (o[1] - en[1]) * (o[1] - en[1])),
        p->fLookaheadDist, p->lookaheadDir[0], p->lookaheadDir[1], lx, ly, p->lookaheadDir[0] * lx + p->lookaheadDir[1] * ly,
        p->vFinalGoal[0] - en[0], p->vFinalGoal[1] - en[1], end[0] - en[0], end[1] - en[1],
        self->codeGoal.pos[0] - en[0], self->codeGoal.pos[1] - en[1], p->vCurrPoint[0] - en[0], p->vCurrPoint[1] - en[1],
        (int)p->wPathLen, (int)p->lookaheadNextNode, (int)p->wNegotiationStartNode, p->flags, p->fLookaheadAmount,
        p->fCurrLength, self->useEnemyGoal ? 1 : 0, o[0] - en[0], o[1] - en[1]);
    const size_t n = strlen(buf) - 1; // reopen the object for the sight fields
    sprintf_s(buf + n, sizeof(buf) - n, ",\"blk\":%.1f,\"be\":%d,\"vb\":%d,\"vu\":%d,\"vl\":%d,\"st\":%d,\"sh\":%d,\"ez\":[%.1f,%.1f],"
        "\"bc\":%d,\"bl\":%d,\"bf\":%d,\"bz\":[%.1f,%.1f,%.1f],\"bd\":%d,\"bx\":%d,\"eye\":[%.2f,%.2f,%.2f],\"eny\":[%.2f,%.2f,%.2f],\"bo\":[%.2f,%.2f,%.2f]}", blk, blkEnt,
        vc->bVisible ? 1 : 0, vc->iLastUpdateTime ? level.time - vc->iLastUpdateTime : -1,
        vc->iLastVisTime ? level.time - vc->iLastVisTime : -1, sightClear ? 1 : 0, sightHit, eye[2] - o[2], enEye[2] - en[2],
        bc, bl, bf, bz[0], bz[1], bz[2], bd, bx, eye[0], eye[1], eye[2], enEye[0], enEye[1], enEye[2],
        blkEnt >= 0 ? g_entities[blkEnt].r.currentOrigin[0] : 0.0f, blkEnt >= 0 ? g_entities[blkEnt].r.currentOrigin[1] : 0.0f,
        blkEnt >= 0 ? g_entities[blkEnt].r.currentOrigin[2] : 0.0f);
    G_SP_PlayTraceLine(buf);
}
