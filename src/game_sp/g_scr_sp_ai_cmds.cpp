// zombies: lane x3 - SP AI builtins ported from the SP exe (BlackOps.exe, image base 0x400000).
// The builtin bodies were read from the exe bytes (no Ghidra export exists for them); each cites its
// SP address. SP actor/entity offsets are mapped to KB members by meaning; the mapping used is noted
// at each site (SP gentity is 0x34c bytes, SP actor 0x3240 - neither matches KB's MP layout).
#include "g_scr_sp_ai_cmds.h"
#include "g_scr_sp_ai.h"
#include "actor_sp_ext.h"
#include "g_sp_ext.h"
#include <game/actor_script_cmd.h>
#include <game/actor_threat.h>
#include <game/actor_zombie_exposed.h>
#include <game/sentient.h>
#include <game/pathnode.h>
#include <game_mp/g_main_mp.h>
#include <game_mp/g_scr_main_mp.h>
#include <game_mp/g_spawn_mp.h>
#include <server/sv_world.h>
#include <bgame/bg_local.h>
#include <clientscript/cscr_vm.h>
#include <clientscript/cscr_stringlist.h>
#include <game/actor_badplace.h>
#include <game/actor_navigation.h>
#include <game/actor_senses.h>
#include <game_mp/actor_mp.h>
#include <cgame/cg_drawtools.h>
#include <universal/com_math.h>
#include <game/actor_state.h>
#include <game/actor_animapi.h>
#include <game/actor_team_move.h>
#include <game/g_missile.h>
#include <game/g_weapon.h>
#include <game_mp/g_utils_mp.h>
#include <bgame/bg_weapons_def.h>
#include <bgame/bg_misc.h>
#include <qcommon/dobj_management.h>
#include <xanim/dobj.h>
#include <xanim/xmodel_utils.h>
#include <clientscript/scr_const.h>
#include <game/turret.h>
#include <universal/com_math_anglevectors.h>
#include <cmath>

extern threat_bias_t g_threatBias;

// zombies: setdeathcontents (SP 0x007f54e0).
// The script value is SP contents numbering (maps/_constants.gsc). KB keeps MP numbering, whose only
// difference in the actor range is the actor bit: SP CONTENTS_ACTOR 0x4000 and CONTENTS_FAKE_ACTOR
// 0x8000 both map to KB's actor bit 0x8000 (the mapping P6a/P6c use for SP masks: SP 0x0200c000 ->
// KB 0x02008000). Five passes level.CONTENTS_CORPSE (0x04000000), identical in both numberings.
void G_m_setdeathcontents(scr_entref_t entref)
{
    if (Scr_GetNumParam(SCRIPTINSTANCE_SERVER) != 1)
        Scr_Error("setdeathcontents takes one parameter\n", false);
    gentity_s *ent = GetEntity(entref);
    if (!ent->actor) // SP entity+0x140
        Scr_Error("setdeathcontents must be called on an AI only.", false);
    int contents = Scr_GetInt(0, SCRIPTINSTANCE_SERVER);
    if (contents & 0xC000)
        contents = (contents & ~0xC000) | 0x8000;
    ent->actor->deathContents = contents; // SP actor+0xc58
}

// zombies: getaivelocity (SP 0x007f53e0).
void G_m_getaivelocity(scr_entref_t entref)
{
    float velocity[3] = { 0.0f, 0.0f, 0.0f };
    gentity_s *ent = GetEntity(entref);
    if (!ent->sentient) // SP entity+0x144
        Scr_Error(va("entity type '%s' is not a sentient", SL_ConvertToString(ent->classname, SCRIPTINSTANCE_SERVER)), false);
    Sentient_GetVelocity(ent->sentient, velocity); // SP 0x00532ee0
    Scr_AddVector(velocity, SCRIPTINSTANCE_SERVER);
}

// zombies: lookatentity (SP 0x008061b0).
// SP stores the entity number at actor+0x88 (ENTITYNUM_NONE with no argument; anything but an entity
// argument leaves it unchanged). The value's only SP consumer is the IK look-at layer
// (IKLAYER_LOOK_AT_ENTITY); KB's ik_import.cpp has a BLOPS_NULLSUB there, so the stored number
// has no effect on KB's IK yet.
void G_m_lookatentity(scr_entref_t entref)
{
    gentity_s *ent = GetEntity(entref);
    if (!ent->actor)
        Scr_Error(va("lookatentity must be called on an AI, not on a '%s'",
            SL_ConvertToString(ent->classname, SCRIPTINSTANCE_SERVER)), false);
    actor_sp_ext_t *ext = Actor_SP_Ext(ent->actor);
    if (!Scr_GetNumParam(SCRIPTINSTANCE_SERVER))
    {
        ext->lookAtEntNum = ENTITYNUM_NONE;
        return;
    }
    if (Scr_GetType(0, SCRIPTINSTANCE_SERVER) == VAR_POINTER
        && Scr_GetPointerType(0, SCRIPTINSTANCE_SERVER) == VAR_ENTITY)
    {
        ext->lookAtEntNum = Scr_GetEntity(0)->s.number;
    }
}

// zombies: getthreatbiasgroup (SP 0x00819630). Group 0 is the unnamed default: "" is returned.
void G_m_getthreatbiasgroup(scr_entref_t entref)
{
    sentient_s *self = GScr_SP_GetSentient(entref);
    int group = self->iThreatBiasGroupIndex; // SP sentient+0xc
    if (group > 0)
        Scr_AddString((char *)SL_ConvertToString(g_threatBias.groupName[group], SCRIPTINSTANCE_SERVER), SCRIPTINSTANCE_SERVER);
    else
        Scr_AddString((char *)"", SCRIPTINSTANCE_SERVER); // SP .rdata 0x009dd354
}

// zombies: Actor_IsSuppressed (SP 0x0049ce70) - suppressionStartTime is SP actor+0x2134.
static bool Actor_IsSuppressed_SP(const actor_s *self)
{
    return self->suppressionStartTime > 0;
}

// zombies: issuppressed (SP 0x007cb9d0).
void G_m_issuppressed(scr_entref_t entref)
{
    actor_s *self = Actor_Get(entref);
    Scr_AddInt(Actor_IsSuppressed_SP(self), SCRIPTINSTANCE_SERVER);
}

// zombies: Actor_StopShoot (SP 0x00501070): clear EF_FIRING (0x40, same bit in KB's bg_pmove).
static void Actor_StopShoot_SP(actor_s *self)
{
    self->ent->s.lerp.eFlags &= ~0x40;
}

// zombies: stopshoot (SP 0x007c9cb0).
void G_m_stopshoot(scr_entref_t entref)
{
    Actor_StopShoot_SP(Actor_Get(entref));
}

// zombies: getturret (SP 0x007ccc80). SP actor+0x21a8 is the side turret pointer.
void G_m_getturret(scr_entref_t entref)
{
    actor_s *self = Actor_Get(entref);
    if (Actor_IsUsingTurret_SP(self)) // SP 0x00473160
        Scr_AddEntity(Actor_SP_Ext(self)->turret, SCRIPTINSTANCE_SERVER);
}

// zombies: makefakeai (SP 0x00610130).
// SP entity+0xbe s.eType; +0x170 flags (0x2000 = script_model's scripted-animation flag, side storage
// G_EntSpExt::scriptedAnimSupported); +0xda r.svFlags; +0xe8/+0xf4 r.mins/r.maxs; +0x100 r.contents;
// +0xd1 s.surfType (7 = flesh, as G_m_startragdoll 0x005fde7c writes).
void G_m_makefakeai(scr_entref_t entref)
{
    gentity_s *ent = GetEntity(entref);
    if (ent->s.eType != ET_SCRIPTMOVER || !G_EntSpExt(ent).scriptedAnimSupported)
        Scr_Error("makeFakeAI must be applied to a script_model", false);
    ent->r.svFlags = (ent->r.svFlags & ~4) | 2;
    ent->r.mins[0] = -15.0f; // SP .rdata 0x00a50c9c..0x00a50cb0
    ent->r.mins[1] = -15.0f;
    ent->r.mins[2] = 0.0f;
    ent->r.maxs[0] = 15.0f;
    ent->r.maxs[1] = 15.0f;
    ent->r.maxs[2] = 72.0f;
    // SP CONTENTS_FAKE_ACTOR 0x8000 -> KB actor bit 0x8000 (see setdeathcontents).
    ent->r.contents |= 0x8000;
    ent->s.surfType = 7;
    SV_LinkEntity(ent); // SP 0x00695450
}

// zombies: isnodeoccupied (SP 0x0067e850). SP node+0x48 = dynamic.iFreeTime.
void G_f_isnodeoccupied()
{
    if (Scr_GetNumParam(SCRIPTINSTANCE_SERVER) != 1)
    {
        Scr_Error("illegal call to isnodeoccupied()", false);
        return;
    }
    pathnode_t *node = Scr_GetPathnode(0, SCRIPTINSTANCE_SERVER);
    Scr_AddInt(level.time <= node->dynamic.iFreeTime, SCRIPTINSTANCE_SERVER);
}

// zombies: Path_MakeArcBadPlace (SP 0x00466870). The SP body copies parms, then (ai_enableBadPlaces,
// SP 0x01bda37c) calls Path_MakeBadPlaceEx with type 1 (SP 0x007bea20 = KB's, register-passed) and
// tail-calls Actor_BadPlacesChanged (SP 0x00584e40).
static void Path_MakeArcBadPlace_SP(unsigned int name, int duration, int teamflags, const badplace_arc_t *arc)
{
    badplace_parms_t parms;
    memcpy(&parms.arc, arc, sizeof(parms.arc));
    if (!ai_enableBadPlaces->current.enabled)
        return;
    Path_MakeBadPlaceEx(name, duration, teamflags, 1, &parms);
    Actor_BadPlacesChanged();
}

// zombies: the name/duration arguments shared by badplace_arc and badplace_cylinder (SP 0x00800185..).
// An empty name is the anonymous bad place (0). Duration: seconds * 1000 stored as a float, then
// fistp of (double)ms + 0.5 under the default round-to-nearest mode.
static void GScr_SP_GetBadPlaceNameDuration(unsigned int *name, int *duration)
{
    *name = *Scr_GetString(0, SCRIPTINSTANCE_SERVER) ? Scr_GetConstString(0, SCRIPTINSTANCE_SERVER) : 0;
    float ms = (float)(Scr_GetFloat(1, SCRIPTINSTANCE_SERVER) * 1000.0f); // SP .rdata 0x009f2500
    *duration = (int)lrint((double)ms + 0.5); // SP .rdata 0x00a1ac60
}

// zombies: badplace_arc (SP 0x00800160).
// badplace_arc(name, duration, origin, radius, height, direction, leftAngle, rightAngle [, team...])
void G_f_badplace_arc()
{
    if (Scr_GetNumParam(SCRIPTINSTANCE_SERVER) < 8)
    {
        Scr_Error("Incorrect usage for badplace_arc()\n", false);
        return;
    }
    unsigned int name;
    int duration;
    GScr_SP_GetBadPlaceNameDuration(&name, &duration);
    badplace_arc_t arc;
    Scr_GetVector(2, arc.origin, SCRIPTINSTANCE_SERVER);
    arc.radius = (float)Scr_GetFloat(3, SCRIPTINSTANCE_SERVER);
    arc.halfheight = (float)(Scr_GetFloat(4, SCRIPTINSTANCE_SERVER) * 0.5f); // SP .rdata 0x009b449c
    float dir[3];
    Scr_GetVector(5, dir, SCRIPTINSTANCE_SERVER);
    float yaw = vectoyaw(dir); // SP 0x005895a0
    float left = (float)Scr_GetFloat(6, SCRIPTINSTANCE_SERVER);
    arc.angle0 = yaw - left;
    if (arc.angle0 > yaw)
        Scr_Error("left angle < 0 in badplace_arc\n", false);
    float right = (float)Scr_GetFloat(7, SCRIPTINSTANCE_SERVER);
    arc.angle1 = right + yaw;
    if (yaw > arc.angle1)
        Scr_Error("right angle < 0 in badplace_arc\n", false);
    if (arc.angle1 - arc.angle0 >= 360.0f) // SP .rdata 0x009ebe34
    {
        arc.angle0 = 0.0f;
        arc.angle1 = 360.0f;
    }
    else
    {
        arc.angle0 = (float)AngleNormalize360(arc.angle0); // SP 0x0047fae0
        arc.angle1 = (float)AngleNormalize360(arc.angle1);
    }
    int teamflags = GScr_SP_TeamFlagsFrom(8, "badplace_arc"); // SP 0x007f08d0
    if (!teamflags)
        teamflags = 14;
    Path_MakeArcBadPlace_SP(name, duration, teamflags, &arc);
}

// zombies: badplace_cylinder (SP 0x00800060): a full 0..360 arc.
// badplace_cylinder(name, duration, origin, radius, height [, team...])
void G_f_badplace_cylinder()
{
    if (Scr_GetNumParam(SCRIPTINSTANCE_SERVER) < 5)
    {
        Scr_Error("Incorrect badplace_cylinder() call.\n", false);
        return;
    }
    unsigned int name;
    int duration;
    GScr_SP_GetBadPlaceNameDuration(&name, &duration);
    badplace_arc_t arc;
    Scr_GetVector(2, arc.origin, SCRIPTINSTANCE_SERVER);
    arc.radius = (float)Scr_GetFloat(3, SCRIPTINSTANCE_SERVER);
    arc.halfheight = (float)(Scr_GetFloat(4, SCRIPTINSTANCE_SERVER) * 0.5f);
    arc.angle0 = 0.0f;
    arc.angle1 = 360.0f;
    int teamflags = GScr_SP_TeamFlagsFrom(5, "badplace_cylinder");
    if (!teamflags)
        teamflags = 14;
    Path_MakeArcBadPlace_SP(name, duration, teamflags, &arc);
}

// zombies: findpath (SP 0x0040a420). SP keeps one static path_t (pointer 0x01d04484 -> 0x01d04488),
// re-initialised by Path_Begin (SP 0x0060f360) on every call, and returns Path_FindPath's result
// (SP 0x005ff210, team 0, negotiation links allowed).
void G_f_findpath()
{
    static path_t s_scriptPath;
    float start[3], end[3];
    Scr_GetVector(0, start, SCRIPTINSTANCE_SERVER);
    Scr_GetVector(1, end, SCRIPTINSTANCE_SERVER);
    Path_Begin(&s_scriptPath);
    Scr_AddInt(Path_FindPath(&s_scriptPath, TEAM_FREE, start, end, 1), SCRIPTINSTANCE_SERVER);
}

// zombies: getanynodearray (SP 0x00484140): every node type (-1) within radius, up to 1024, no
// height check (SP calls 0x0068e910 = Path_NearestNodeNotCrossPlanes with no planes).
void G_f_getanynodearray()
{
    static pathsort_t nodes[1024]; // SP: 0x3000 bytes of stack (alloca probe)
    float origin[3];
    int count = 0;
    Scr_GetVector(0, origin, SCRIPTINSTANCE_SERVER);
    float radius = (float)Scr_GetFloat(1, SCRIPTINSTANCE_SERVER);
    Path_NearestNodeNotCrossPlanes(origin, nodes, -1, radius, NULL, NULL, 0, &count, 1024,
        NEAREST_NODE_DONT_DO_HEIGHT_CHECK);
    Scr_MakeArray(SCRIPTINSTANCE_SERVER);
    for (int i = 0; i < count; ++i)
    {
        Scr_AddPathnode(nodes[i].node); // SP inlines 0x0062c140
        Scr_AddArray(SCRIPTINSTANCE_SERVER);
    }
}

// zombies: findbestcovernode (SP 0x007c9e20).
void G_m_findbestcovernode(scr_entref_t entref)
{
    actor_s *self = Actor_Get(entref);
    pathnode_t *node = FUN_005b5540(self);
    if (node)
        Scr_AddPathnode(node); // SP 0x0062c140
}

// zombies: canshoot (SP 0x007caba0). canshoot(targetPos [, muzzleOffset]) - the optional offset is
// added to the muzzle (tag_flash) position, not to the target.
void G_m_canshoot(scr_entref_t entref)
{
    actor_s *self = Actor_Get(entref);
    float target[3], muzzle[3];
    Scr_GetVector(0, target, SCRIPTINSTANCE_SERVER);
    if (!Actor_GetMuzzleInfo(self, muzzle, NULL)) // SP 0x004dfe10
        Scr_Error(va("Couldn't find %s in entity %d", "tag_flash", self->ent->s.number), false);
    if (Scr_GetNumParam(SCRIPTINSTANCE_SERVER) > 1)
    {
        float offset[3];
        Scr_GetVector(1, offset, SCRIPTINSTANCE_SERVER);
        muzzle[0] = offset[0] + muzzle[0];
        muzzle[1] = muzzle[1] + offset[1];
        muzzle[2] = offset[2] + muzzle[2];
    }
    bool canShoot = FUN_0049b0e0(self, target, muzzle);
    if (ai_ShowCanshootChecks->current.enabled)
    {
        static const float colorGreen[4] = { 0.0f, 1.0f, 0.0f, 1.0f }; // SP .rdata 0x00a5e7c4
        static const float colorRed[4] = { 1.0f, 0.0f, 0.0f, 1.0f };   // SP .rdata 0x00a5e794
        CG_DebugLine(target, muzzle, canShoot ? colorGreen : colorRed, 0, 30); // SP 0x0064a0b0
    }
    Scr_AddInt(canShoot, SCRIPTINSTANCE_SERVER);
}

// zombies: startactorreact (SP 0x007ce8f0): push AIS_REACT (8), and kill the animscript when pushed.
void G_m_startactorreact(scr_entref_t entref)
{
    actor_s *self = Actor_Get(entref);
    if (Actor_PushState(self, AIS_REACT)) // SP 0x004ae560
        Actor_KillAnimScript(self);       // SP 0x0045bd80
}

// zombies: FUN_005269b0 (SP 0x005269b0) - usecovernode's claim. Only in exposed/turret state, and
// only for a node the exposed cover test (0x005ae040) accepts; a linked actor (tagInfo) claims
// without pathing, otherwise a path to the node must be found (0x0046c830).
static bool Actor_UseCoverNode_SP(actor_s *self, pathnode_t *node)
{
    ai_state_t state = self->eState[self->stateLevel]; // SP actor+0xb94[+0xbc4]
    if ((state != AIS_EXPOSED && state != AIS_TURRET) || !FUN_005ae040(self, node))
        return false;
    if (!self->ent->tagInfo && !FUN_0046c830(self, node)) // SP entity+0x298
    {
        Actor_TeamMoveBlocked(self); // SP 0x00545b60
        return false;
    }
    Sentient_ClaimNode(self->sentient, node); // SP 0x00635000
    self->iPotentialCoverNodeCount = 0;       // SP actor+0x1aa8
    if (self->eState[self->stateLevel] != AIS_EXPOSED)
        Actor_SetState(self, AIS_EXPOSED);    // SP 0x004dceb0
    Actor_SetSubState(self, STATE_EXPOSED_COMBAT); // SP 0x00405550, substate 100
    return true;
}

// zombies: usecovernode (SP 0x007c9ec0).
void G_m_usecovernode(scr_entref_t entref)
{
    actor_s *self = Actor_Get(entref);
    pathnode_t *node = Scr_GetPathnode(0, SCRIPTINSTANCE_SERVER); // SP 0x0042d010
    // SP actor+0x1a65 fixedNode; sentient+0x5c pClaimedNode.
    if (self->fixedNode && self->sentient->pClaimedNode && self->sentient->pClaimedNode != node)
        Scr_Error("cannot change node when using fixedNode mode", false);
    if (Actor_KeepClaimedNode(self)) // SP 0x0068b8d0
        Scr_Error("cannot change node when keepclaimednode is set", false);
    Scr_AddInt(Actor_UseCoverNode_SP(self, node), SCRIPTINSTANCE_SERVER);
}

// zombies: FUN_005e78b0 (SP 0x005e78b0) - checkgrenadethrow's toss test against the target's
// origin led by its velocity. SP stores the lead target (actor+0x2180, +0x216c) and the method
// (+0x2164) before the toss test.
static bool Actor_CheckGrenadeThrow_SP(actor_s *self, const float *origin, const float *offset,
    unsigned int method, float *tossPosition, float *velocity, float randomRange, bool throwback)
{
    sentient_s *enemy = Actor_GetTargetSentient(self); // SP 0x004fd9b0
    if (!enemy)
        return false;
    if (bg_gravity->current.value <= 0.0f) // SP 0x00bcaefc
        return false;
    if (self->pGrenade.isDefined() && !throwback) // SP actor+0x215c
        return false;
    float enemyOrigin[3], enemyVelocity[3], target[3];
    Sentient_GetOrigin(enemy, enemyOrigin);     // SP 0x005f2350
    Sentient_GetVelocity(enemy, enemyVelocity); // SP 0x00532ee0
    target[0] = enemyVelocity[0] + enemyOrigin[0];
    target[1] = enemyVelocity[1] + enemyOrigin[1];
    target[2] = enemyVelocity[2] + enemyOrigin[2];
    Vec3Copy(target, self->vGrenadeTargetPos);
    self->bGrenadeTargetValid = 1;
    Scr_SetString(&self->GrenadeTossMethod, method, SCRIPTINSTANCE_SERVER); // SP 0x00406bd0
    return FUN_0053ac00(self, origin, offset, target, method, tossPosition, velocity, randomRange, throwback);
}

// zombies: checkgrenadethrow (SP 0x007cba70). checkgrenadethrow(offset, method, randomRange): needs a
// target and grenade ammo (actor+0x2170); returns the toss velocity when a toss is found.
void G_m_checkgrenadethrow(scr_entref_t entref)
{
    actor_s *self = Actor_Get(entref);
    float offset[3];
    Scr_GetVector(0, offset, SCRIPTINSTANCE_SERVER);
    unsigned int method = Scr_GetConstString(1, SCRIPTINSTANCE_SERVER);
    float randomRange = (float)Scr_GetFloat(2, SCRIPTINSTANCE_SERVER);
    if (!Actor_GetTargetSentient(self) || self->iGrenadeAmmo <= 0)
        return;
    float origin[3];
    Vec3Copy(self->ent->r.currentOrigin, origin); // SP entity+0x11c
    self->bGrenadeTossValid = Actor_CheckGrenadeThrow_SP(self, origin, offset, method,
        self->vGrenadeTossPos, self->vGrenadeTossVel, randomRange, false); // SP actor+0x2168/2174/218c
    if (self->bGrenadeTossValid)
    {
        Scr_SetString(&self->GrenadeTossMethod, method, SCRIPTINSTANCE_SERVER);
        Scr_AddVector(self->vGrenadeTossVel, SCRIPTINSTANCE_SERVER);
    }
}

// zombies: checkgrenadethrowpos (SP 0x007cbb70). checkgrenadethrowpos(offset, method, targetPos): no
// target sentient needed and no lead; the target is stored at actor+0x2180 but +0x216c (target
// valid) is left as it was.
void G_m_checkgrenadethrowpos(scr_entref_t entref)
{
    actor_s *self = Actor_Get(entref);
    float offset[3], target[3];
    Scr_GetVector(0, offset, SCRIPTINSTANCE_SERVER);
    unsigned int method = Scr_GetConstString(1, SCRIPTINSTANCE_SERVER);
    Scr_GetVector(2, target, SCRIPTINSTANCE_SERVER);
    if (self->iGrenadeAmmo <= 0)
        return;
    float origin[3];
    Vec3Copy(self->ent->r.currentOrigin, origin);
    Vec3Copy(target, self->vGrenadeTargetPos);
    self->bGrenadeTossValid = FUN_0053ac00(self, origin, offset, target, method,
        self->vGrenadeTossPos, self->vGrenadeTossVel, 0.0f, false);
    if (self->bGrenadeTossValid)
    {
        Scr_SetString(&self->GrenadeTossMethod, method, SCRIPTINSTANCE_SERVER);
        Scr_AddVector(self->vGrenadeTossVel, SCRIPTINSTANCE_SERVER);
    }
}

// zombies: G_InitGrenadeMovement (SP 0x005f0900) for an actor-owned grenade. SP's first branch (a
// player parent under an alternate-gravity dvar pair, trType 0xf) cannot run: both callers here pass
// an actor parent. SP draws G_irand before G_flrand (KB's MP body draws them the other way round)
// and writes entity+0x234 (item[1].ammoCount, the mapping P6c uses for the same store).
static void G_InitActorGrenadeMovement_SP(gentity_s *grenade, const float *start, const float *dir,
    int rotate, int rotateType)
{
    grenade->item[1].ammoCount = 0;
    grenade->s.lerp.pos.trType = (trType_t)G_GetGrenadeTrType(grenade); // SP 0x007e68a0
    grenade->s.lerp.pos.trTime = level.time;
    Vec3Copy(start, grenade->r.currentOrigin);
    Vec3Copy(start, grenade->s.lerp.pos.trBase);
    Vec3Copy(dir, grenade->s.lerp.pos.trDelta);
    vectoangles(dir, grenade->r.currentAngles); // SP 0x005590e0
    for (int i = 0; i < 3; ++i)
        grenade->s.lerp.pos.trDelta[i] = (float)(int)grenade->s.lerp.pos.trDelta[i];
    if (!rotate)
    {
        G_SetAngle(grenade, grenade->r.currentAngles); // SP 0x0057fc80
        return;
    }
    grenade->s.lerp.apos.trType = TR_LINEAR; // SP 3
    grenade->s.lerp.apos.trTime = level.time;
    Vec3Copy(grenade->r.currentAngles, grenade->s.lerp.apos.trBase);
    grenade->s.lerp.apos.trBase[0] = (float)AngleNormalize360(grenade->s.lerp.apos.trBase[0] - 120.0f);
    if (rotateType == WEAPROTATE_BLADE_ROTATE) // SP rotateType 1
    {
        grenade->s.lerp.apos.trDelta[0] = 1500.0f;
        grenade->s.lerp.apos.trDelta[1] = 0.0f;
        grenade->s.lerp.apos.trDelta[2] = 0.0f;
    }
    else
    {
        int sign = G_irand(0, 2);
        grenade->s.lerp.apos.trDelta[0] = (float)(G_flrand(340.0f, 800.0f) * (float)(sign * 2 - 1));
        grenade->s.lerp.apos.trDelta[1] = 0.0f;
        sign = G_irand(0, 2);
        grenade->s.lerp.apos.trDelta[2] = (float)(G_flrand(180.0f, 540.0f) * (float)(sign * 2 - 1));
        for (int i = 0; i < 3; ++i)
            grenade->s.lerp.pos.trDelta[i] = (float)(int)grenade->s.lerp.pos.trDelta[i];
    }
    Vec3Copy(grenade->s.lerp.apos.trBase, grenade->r.currentAngles);
}

// zombies: G_FireGrenade (SP 0x006318d0) for an actor parent. Same order as KB's MP body; the SP
// differences are the actor branch of G_InitGrenadeEntity (SP 0x00613620, P6c's port) and the
// movement initializer above. SP sets no attackerEntityNum. The timer (SP 0x007e6b50) for a
// non-client parent is KB's InitGrenadeTimer: nextthink = level.time + time, clamped to 30 s/60 s.
static gentity_s *G_FireActorGrenade_SP(gentity_s *parent, float *start, float *dir, int grenadeWPID,
    unsigned char grenModel, int rotate, int time)
{
    iassert(parent->actor && !parent->client);
    const WeaponDef *weapDef = BG_GetWeaponDef(grenadeWPID);
    gentity_s *grenade = G_Spawn(); // SP 0x004bf260
    Scr_SetString(&grenade->classname, scr_const.grenade, SCRIPTINSTANCE_SERVER);
    AssignToSmallerType<unsigned short>(&grenade->s.weapon, grenadeWPID);
    grenade->s.weaponModel = grenModel;
    grenade->s.lerp.u.turret.ownerNum = 0; // SP entity+0x60
    grenade->missile.grenade.effectIndex = 55; // SP entity+0x250
    G_InitActorGrenadeEntity_SP(parent, grenade);
    G_InitActorGrenadeMovement_SP(grenade, start, dir, rotate && weapDef->rotate, weapDef->rotateType);
    InitGrenadeTimer(parent, grenade, weapDef, time);
    if (weapDef->projectileModel) // SP WeaponDef+0x5e8
        G_SetModel(grenade, (char *)XModelGetName(weapDef->projectileModel));
    G_DObjUpdate(grenade);
    DObj *obj = Com_GetServerDObj(grenade->s.number);
    if (obj)
    {
        grenade->r.contents |= DObjGetContents(obj);
        DObjCalcBounds(obj, grenade->r.mins, grenade->r.maxs);
    }
    Scr_AddString((char *)BG_WeaponName(grenadeWPID), SCRIPTINSTANCE_SERVER);
    Scr_AddEntity(grenade, SCRIPTINSTANCE_SERVER);
    if (weapDef->iProjectileActivateDist) // SP WeaponDef+0x5d8
        Scr_Notify(parent, scr_const.grenade_launcher_fire, 2u);
    else
        Scr_Notify(parent, scr_const.grenade_fire, 2u);
    return grenade;
}

// zombies: throwgrenade (SP 0x007cbc90).
void G_m_throwgrenade(scr_entref_t entref)
{
    actor_s *self = Actor_Get(entref);
    gentity_s *ent = self->ent;
    float tossPosition[3], velocity[3];
    if (ent->health > 0 && (self->pGrenade.isDefined() || self->bGrenadeTossValid))
    {
        bool tossed = false;
        if (self->bGrenadeTargetValid)
        {
            float handPos[3];
            if (!G_DObjGetWorldTagPos(ent, scr_const.tag_inhand, handPos)) // SP 0x004ff790
            {
                Scr_Error(va("Missing tag [%s] on entity [%d] (%s)\n",
                    SL_ConvertToString(scr_const.tag_inhand, SCRIPTINSTANCE_SERVER), ent->s.number,
                    DObjGetName(Com_GetServerDObj(ent->s.number))), false);
                return;
            }
            tossed = FUN_0053ac00(self, handPos, vec3_origin, self->vGrenadeTargetPos,
                self->GrenadeTossMethod, tossPosition, velocity, 0.0f, true);
        }
        if (!tossed)
        {
            Vec3Copy(self->vGrenadeTossPos, tossPosition);
            Vec3Copy(self->vGrenadeTossVel, velocity);
        }
        if (self->pGrenade.isDefined() && self->eState[self->stateLevel] == AIS_GRENADE_RESPONSE)
        {
            // Throwback of a held grenade.
            FUN_004187b0(self); // SP 0x004187b0: detach from tag_inhand
            gentity_s *grenade = self->pGrenade.ent();
            // SP reads the previous owner (entity+0x134 r.ownerNum) before G_InitGrenadeEntity
            // replaces it; SP does not test for an empty handle, KB does.
            gentity_s *owner = grenade->r.ownerNum.isDefined() ? grenade->r.ownerNum.ent() : NULL;
            G_InitActorGrenadeEntity_SP(ent, grenade);
            G_InitActorGrenadeMovement_SP(grenade, tossPosition, velocity, 1, 0);
            if (owner && owner->client)
            {
                // Retune the fuse against the flight time back to the original (player) thrower:
                // horizontal distance over horizontal speed.
                float dy = tossPosition[1] - owner->r.currentOrigin[1];
                float dx = tossPosition[0] - owner->r.currentOrigin[0];
                float dist = sqrtf(dy * dy + dx * dx);
                float speed = sqrtf(velocity[1] * velocity[1] + velocity[0] * velocity[0]);
                float t = (float)(grenade->nextthink - level.time) * 0.001f - dist / speed;
                if (t > 0.0f && t < 1.0f)
                    grenade->nextthink += 1000;
                else if (t < 0.0f && t > -0.7f)
                    grenade->nextthink -= 1000;
            }
        }
        else
        {
            if (!self->iGrenadeWeaponIndex) // SP actor+0x2160
                Scr_Error(va("Actor [%s] doesn't have a grenade weapon set.",
                    SL_ConvertToString(ent->classname, SCRIPTINSTANCE_SERVER)), false);
            const WeaponDef *weapDef = BG_GetWeaponDef(self->iGrenadeWeaponIndex);
            G_FireActorGrenade_SP(ent, tossPosition, velocity, self->iGrenadeWeaponIndex, 0, 1,
                weapDef->aiFuseTime); // SP WeaponDef+0x470
            if (self->iGrenadeAmmo > 0)
                --self->iGrenadeAmmo;
        }
    }
    self->bGrenadeTossValid = 0;
    Scr_SetString(&self->GrenadeTossMethod, 0, SCRIPTINSTANCE_SERVER);
}

// zombies: FUN_007bdb00 (SP 0x007bdb00): accuracy graph value at min(distance / 4000, 1).
// type 0 = AI vs AI graph, 1 = AI vs player graph (SP WeaponDef+0x71c knots, +0x72c knot counts).
float Actor_GetWeaponAccuracyFromGraph_SP(const WeaponVariantDef *weapVariantDef, int weaponIndex,
    int type, float distance)
{
    const WeaponDef *weapDef = weapVariantDef->weapDef;
    float fraction = distance * 0.00025f;
    if (fraction > 1.0f)
        fraction = 1.0f;
    if (!weapDef->accuracyGraphKnots[type] || !weapDef->accuracyGraphKnotCount[type])
    {
        // SP 0x00553df0 looks the variant up in bg_weapVariantDefs for its index; the caller already
        // has it.
        if (type == 0)
            Com_Error(ERR_DROP, "\x15No 'AI vs AI' accuracy graph for weapon '%s'", BG_WeaponName(weaponIndex));
        else if (type == 1)
            Com_Error(ERR_DROP, "\x15No 'AI vs Player' accuracy graph for weapon '%s'", BG_WeaponName(weaponIndex));
    }
    return (float)GraphGetValueFromFraction(weapDef->accuracyGraphKnotCount[type],
        weapDef->accuracyGraphKnots[type], fraction); // SP 0x005737f0
}

// zombies: getweaponaccuracy (SP 0x007fcb90) - a function: getweaponaccuracy(ai, weaponName).
// SP 0x004375e0: origin distance (Sentient_GetOrigin 0x005f2350, z/y/x summation), scaled by
// ai_accuracyDistScale against a player target.
void G_f_getweaponaccuracy()
{
    if (Scr_GetNumParam(SCRIPTINSTANCE_SERVER) != 2)
        Scr_Error("Illegal call to getweaponaccuracy(self,self.weapon), AI entity and weapon name are mandatory fields.", false);
    gentity_s *ent = Scr_GetEntity(0);
    if (!ent->actor)
        Scr_Error("getweaponaccuracy must be called on an AI only.", false);
    actor_s *self = ent->actor;
    sentient_s *target = Actor_GetTargetSentient(self);
    const char *weaponName = Scr_GetString(1, SCRIPTINSTANCE_SERVER);
    int weaponIndex = G_GetWeaponIndexForName((char *)weaponName);
    Scr_VerifyWeaponIndex(weaponIndex, weaponName);
    const WeaponVariantDef *weapVariantDef = BG_GetWeaponVariantDef(weaponIndex);
    // SP dereferences the target without a test (a NULL target faults the SP exe); KB raises a
    // script error instead.
    if (!target)
    {
        Scr_Error("getweaponaccuracy: the AI has no target (SP faults here)", false);
        return;
    }
    int type = target->ent->client ? 1 : 0;
    float selfOrigin[3], targetOrigin[3];
    Sentient_GetOrigin(self->sentient, selfOrigin);
    Sentient_GetOrigin(target, targetOrigin);
    float dz = selfOrigin[2] - targetOrigin[2];
    float dy = selfOrigin[1] - targetOrigin[1];
    float dx = selfOrigin[0] - targetOrigin[0];
    float distance = sqrtf(dz * dz + dy * dy + dx * dx);
    if (type == 1)
        distance = ai_accuracyDistScale->current.value * distance;
    Scr_AddFloat(Actor_GetWeaponAccuracyFromGraph_SP(weapVariantDef, weaponIndex, type, distance), SCRIPTINSTANCE_SERVER);
}

// zombies: FUN_0058fe30 (SP 0x0058fe30): point sight test with the actor's fov and sight distance.
static bool Actor_CanSeePointForAccuracy_SP(actor_s *self, const float *point, const sentient_s *target)
{
    return Actor_CanSeePointEx_SP(self, point, self->fovDot, self->fMaxSightDistSqrd, target->ent->s.number);
}

// zombies: FUN_004c1170 (SP 0x004c1170): player sight accuracy from four sight tests (eye 10, 75%/50%/
// 25% up the body 30 each) scaled by 0.01, floored at 0.1.
// PARTIAL: SP also raises the result to 0.75 when the player is within ai_playerLOSHalfWidth (compared
// with a squared 2D distance) of the player-LOS position gclient+0x1cd8, which SP's per-frame player-LOS
// tracker (0x00418d00 -> 0x008189c0) maintains. KB has neither the tracker nor the fields, so that
// raise is not applied. SP also builds a collision visitor (0x004614b0) around the eye-to-body
// segment first; it only prefilters the same traces and is not ported.
static float Actor_GetPlayerSightAccuracy_SP(actor_s *self, sentient_s *target)
{
    float eye[3], origin[3], delta[3], point[3];
    Sentient_GetEyePosition(target, eye); // SP 0x004fbe10
    Vec3Copy(target->ent->client->ps.origin, origin); // SP gclient+0x24
    Vec3Sub(eye, origin, delta);
    float accuracy = 0.0f;
    if (Actor_CanSeePointForAccuracy_SP(self, eye, target))
        accuracy = 10.0f;
    static const float fractions[3] = { 0.75f, 0.5f, 0.25f };
    for (int i = 0; i < 3; ++i)
    {
        point[0] = delta[0] * fractions[i] + origin[0];
        point[1] = delta[1] * fractions[i] + origin[1];
        point[2] = delta[2] * fractions[i] + origin[2];
        if (Actor_CanSeePointForAccuracy_SP(self, point, target))
            accuracy = accuracy + 30.0f;
    }
    accuracy = accuracy * 0.01f;
    if (accuracy < 0.1f)
        accuracy = 0.1f;
    return accuracy;
}

// zombies: updateplayersightaccuracy (SP 0x007c9d70): 1 unless the target is a player.
void G_m_updateplayersightaccuracy(scr_entref_t entref)
{
    actor_s *self = Actor_Get(entref);
    sentient_s *target = Actor_GetTargetSentient(self);
    if (target && target->ent->client)
        self->playerSightAccuracy = Actor_GetPlayerSightAccuracy_SP(self, target); // SP actor+0xc74
    else
        self->playerSightAccuracy = 1.0f;
}

// zombies: FUN_007d1430 (SP 0x007d1430, register arg edi = actor) - the actor lets go of its turret:
// deactivate, clear the owner, notify turretownerchange, and move the actor's link from the turret
// to the turret's own parent (or unlink). SP TurretInfo+8 flags 0x200 is cleared either way.
// G_DeactivateTurret is KB's (SP 0x0064e0a0, same notify-and-clear body).
static void Actor_StopUseTurret_SP(actor_s *self)
{
    gentity_s *turret = Actor_SP_Ext(self)->turret;
    G_DeactivateTurret(turret);
    turret->r.ownerNum.setEnt(NULL); // SP entity+0x134
    if (Scr_IsSystemActive(1u, SCRIPTINSTANCE_SERVER)) // SP 0x00431a60
        Scr_Notify(turret, scr_const.turretownerchange, 0); // SP slot 0x023a574e
    // SP 0x004ca980: is the actor linked to the turret (entity+0x298 tagInfo->parent)?
    if (self->ent->tagInfo && self->ent->tagInfo->parent == turret)
    {
        if (!turret->tagInfo)
            G_EntUnlink(self->ent); // SP 0x0058e900
        else
            G_EntLinkTo(self->ent, turret->tagInfo->parent, turret->tagInfo->name); // SP 0x0044a8e0
    }
    turret->pTurretInfo->flags &= ~0x200;
}

// zombies: FUN_005d6090 (SP 0x005d6090): let go of the turret when this actor owns it, then forget it.
static void Actor_ClearTurret_SP(actor_s *self)
{
    gentity_s *turret = Actor_SP_Ext(self)->turret;
    if (turret && turret->r.ownerNum.isDefined() && turret->r.ownerNum.ent() == self->ent)
        Actor_StopUseTurret_SP(self);
    Actor_SP_Ext(self)->turret = NULL;
}

// zombies: SP G_FreeEntity (0x00438bca): a turret an actor has used (SP entity flags 0x4000000, side
// flag actorTurret) releases every actor still pointing at it before it is freed.
void Actor_SP_ReleaseFreedTurret(gentity_s *turret)
{
    for (actor_s *actor = Actor_FirstActor(-1); actor; actor = Actor_NextActor(actor, -1)) // SP 0x00564a50/0x00428d20
    {
        if (Actor_SP_Ext(actor)->turret == turret)
            Actor_ClearTurret_SP(actor);
    }
}

// zombies: FUN_005a7740 (SP 0x005a7740): turret sight trace, mask 0x801803 without 0x10 (KB's
// turret_SightTrace with no turret uses the same mask), then SP's smoke test: an unblocked line with
// FX visibility under 0.2 counts as blocked. Nonzero = blocked.
static int turret_SightTrace_SP(const float *start, const float *end, int passEnt1, int passEnt2)
{
    int hitNum = 0;
    col_context_t context(0x801803);
    context.mask &= ~0x10u;
    context.passEntityNum0 = passEnt1;
    context.passEntityNum1 = passEnt2;
    SV_SightTracePoint(&hitNum, start, end, &context); // SP 0x005f4330
    if (hitNum)
        return hitNum;
    if (FX_GetServerVisibility_SP(start, end) < 0.2f) // SP 0x005d8b90 -> 0x0044f050
        return 1;
    return hitNum;
}

// zombies: FUN_0081b820 (SP 0x0081b820): is the new turret farther (2D, from the actor) than the one
// the actor is using?
static bool Actor_TurretFartherThanCurrent_SP(const gentity_s *turret, actor_s *self)
{
    const float *origin = self->ent->r.currentOrigin;
    const float *current = Actor_SP_Ext(self)->turret->r.currentOrigin;
    float curY = current[1] - origin[1], curX = current[0] - origin[0];
    float newY = turret->r.currentOrigin[1] - origin[1], newX = turret->r.currentOrigin[0] - origin[0];
    return newY * newY + newX * newX > curY * curY + curX * curX;
}

// zombies: FUN_0081b8a0 (SP 0x0081b8a0, register arg edi = turret). SP TurretInfo +0x4c
// forwardAngleDot, +0x60 suppressTime, +0x64 maxRangeSquared (KB's TurretInfo has the same layout up
// to there); SP actor+0x1af8 sentientInfo (0x28 each: +0 VisCache.bVisible, +0x10 lastKnownPosTime,
// +0x18 vLastKnownPos).
static bool Actor_TurretTargetCheck_SP(actor_s *self, gentity_s *turret)
{
    TurretInfo *info = turret->pTurretInfo;
    if (Actor_IsUsingTurret_SP(self) && Actor_SP_Ext(self)->turret != turret
        && !Actor_TurretFartherThanCurrent_SP(turret, self))
    {
        return false;
    }
    sentient_s *target = Actor_GetTargetSentient(self);
    if (!target)
        return true;
    sentient_info_t *sinfo = &self->sentientInfo[target - level.sentients];
    if (level.time - sinfo->lastKnownPosTime >= info->suppressTime)
        return true;
    const float *targetOrigin = target->ent->r.currentOrigin;
    float delta[3];
    delta[0] = targetOrigin[0] - turret->r.currentOrigin[0];
    delta[1] = targetOrigin[1] - turret->r.currentOrigin[1];
    delta[2] = targetOrigin[2] - turret->r.currentOrigin[2];
    if (delta[2] * delta[2] + delta[1] * delta[1] + delta[0] * delta[0] >= info->maxRangeSquared)
        return true;
    float knownY = sinfo->vLastKnownPos[1] - targetOrigin[1];
    float knownX = sinfo->vLastKnownPos[0] - targetOrigin[0];
    if (knownY * knownY + knownX * knownX >= 4096.0f)
        return true;
    if (sinfo->VisCache.bVisible)
    {
        float eye[3], source[3], localAngles[2];
        Sentient_GetEyePosition(target, eye); // SP 0x004fbe10 (its result is not used further)
        float targetPos[3];
        if (turret_CanTargetSentient_SP(turret, target, targetPos, source, localAngles)) // SP 0x0042f450
            return true;
    }
    else
    {
        float forward[3];
        AngleVectors(turret->r.currentAngles, forward, NULL, NULL); // SP 0x005bd300
        Vec3Normalize(delta);
        if (delta[1] * forward[1] + delta[2] * forward[2] + delta[0] * forward[0] >= info->forwardAngleDot)
            return true;
    }
    // Gunner position: tag_weapon (SP slot 0x023a568c) moved 30 units away from tag_aim (0x023a5660)
    // in 2D, at tag_aim's height; can the target's eye see it?
    float weaponPos[3], aimPos[3], start[3], dir[2];
    if (!G_DObjGetWorldTagPos(turret, scr_const.tag_weapon, weaponPos))
        return false;
    if (!G_DObjGetWorldTagPos(turret, scr_const.tag_aim, aimPos))
        return false;
    start[2] = aimPos[2];
    dir[0] = weaponPos[0] - aimPos[0];
    dir[1] = weaponPos[1] - aimPos[1];
    Vec2Normalize(dir); // SP 0x005f8a00
    start[0] = dir[0] * 30.0f + weaponPos[0];
    start[1] = dir[1] * 30.0f + weaponPos[1];
    float eye[3];
    Sentient_GetEyePosition(target, eye);
    return turret_SightTrace_SP(start, eye, self->ent->s.number, target->ent->s.number) != 0;
}

// zombies: FUN_00626a60 (SP 0x00626a60): a target 256 units or more from the actor (x/z/y summation)
// does not block the turret.
static bool Actor_TurretNearTargetCheck_SP(gentity_s *turret, actor_s *self)
{
    sentient_s *target = Actor_GetTargetSentient(self);
    if (!target)
        return true;
    const float *a = self->ent->r.currentOrigin;
    const float *b = target->ent->r.currentOrigin;
    float dx = a[0] - b[0], dz = a[2] - b[2], dy = a[1] - b[1];
    if (dx * dx + dz * dz + dy * dy >= 65536.0f)
        return true;
    return Actor_TurretTargetCheck_SP(self, turret);
}

// zombies: FUN_00542f40 (SP 0x00542f40): canuseturret's test. SP TurretInfo+8 flags 0x2 selects the
// full target check; entity+0x158 active = someone is on it.
static bool Actor_CanUseTurret_SP(actor_s *self, gentity_s *turret)
{
    if (Actor_IsUsingTurret_SP(self) && Actor_SP_Ext(self)->turret == turret)
        return true;
    if (turret->active)
        return false;
    if (turret->pTurretInfo->flags & 2)
        return Actor_TurretTargetCheck_SP(self, turret);
    return Actor_TurretNearTargetCheck_SP(turret, self);
}

// zombies: FUN_0066b6a0 (SP 0x0066b6a0): useturret stores the turret (or NULL when it cannot be used,
// without letting go of a current one) and flags the turret entity (SP flags 0x4000000).
static bool Actor_UseTurret_SP(actor_s *self, gentity_s *turret)
{
    if (!Actor_CanUseTurret_SP(self, turret))
    {
        Actor_SP_Ext(self)->turret = NULL;
        return false;
    }
    Actor_SP_Ext(self)->turret = turret;
    G_EntSpExt(turret).actorTurret = true;
    return true;
}

// zombies: useturret (SP 0x007cc3d0).
void G_m_useturret(scr_entref_t entref)
{
    actor_s *self = Actor_Get(entref);
    gentity_s *turret = Scr_GetEntity(0);
    if (!turret->pTurretInfo) // SP entity+0x14c
        Scr_ParamError(0, "can only use a turret", SCRIPTINSTANCE_SERVER);
    Scr_AddInt(Actor_UseTurret_SP(self, turret), SCRIPTINSTANCE_SERVER);
}

// zombies: canuseturret (SP 0x007cc500).
void G_m_canuseturret(scr_entref_t entref)
{
    actor_s *self = Actor_Get(entref);
    gentity_s *turret = Scr_GetEntity(0);
    if (!turret->pTurretInfo)
        Scr_ParamError(0, "can only use a turret", SCRIPTINSTANCE_SERVER);
    Scr_AddInt(Actor_CanUseTurret_SP(self, turret), SCRIPTINSTANCE_SERVER);
}

// zombies: stopuseturret (SP 0x007cc4c0, tail jump to 0x005d6090).
void G_m_stopuseturret(scr_entref_t entref)
{
    Actor_ClearTurret_SP(Actor_Get(entref));
}
