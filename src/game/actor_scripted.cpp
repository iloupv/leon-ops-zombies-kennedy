#include "actor_scripted.h"
#include "actor_animapi.h"
#include "actor_script_cmd.h"
#include "actor_state.h"
#include "actor_senses.h"
#include "actor_events.h"
#include "actor_corpse.h"
#include "actor_zombie_exposed.h"
#include <game_mp/actor_mp.h>
#include <game_mp/g_main_mp.h>
#include <game_mp/g_scr_main_mp.h>
#include <game_mp/g_utils_mp.h>
#include <game_mp/g_spawn_mp.h>
#include <game_sp/actor_sp_ext.h>
#include <game_sp/g_anim_commands_sp.h>
#include <game_sp/g_sp_ext.h>
#include <game_sp/g_sp_levelstart.h>
#include <game_sp/g_sp_measure.h>
#include <game_sp/g_sp_headless_dice.h>
#include <clientscript/cscr_vm.h>
#include <clientscript/cscr_stringlist.h>
#include <clientscript/scr_const.h>
#include <qcommon/common.h>
#include <qcommon/dobj_management.h>
#include <universal/com_math.h>
#include <universal/com_math_anglevectors.h>
#include <universal/dvar.h>
#include <xanim/xanim.h>
#include <server/sv_world.h>
#include <cmath>

// zombies: measured via AIFuncTable (SP 0x00b750d8). The brief said the table
// starts at 0x00a51a08; the exe's species pointers are a519e8/a51e68/a51b68/a51ce8.
// Species 2 rows 9/10: a51c88/a51ca8; species 3: a51e08/a51e28.
// The eighth pointer is pfnReact (dispatcher 0x004b40a0), NULL in ALL rows of
// both zombie species. No ai_funcs_t extension or parallel dispatch is needed.

static void ScriptedAnim_RequireZombies()
{
    if (!G_SP_IsSPLevel()) // SP has no zombie gate here; MP keeps its path
        Scr_Error("SP scripted animation requires an SP level", false);
}

// zombies: Actor_ScriptedAnim_Start (SP 0x00452e90). Entity+8 is eFlags2.
bool __fastcall Actor_ScriptedAnim_Start_SP(actor_s *self, ai_state_t previous)
{
    if (!G_SP_IsSPLevel()) // SP has no zombie gate here; MP keeps its path
        return Actor_SP_NotPorted_Start(self, previous);
    self->ent->s.lerp.eFlags2 |= 2;
    return true;
}

// zombies: Actor_ScriptedAnim_Finish (SP 0x00620a70).
void __fastcall Actor_ScriptedAnim_Finish_SP(actor_s *self, ai_state_t next)
{
    if (!G_SP_IsSPLevel()) // SP has no zombie gate here; MP keeps its path
        return Actor_SP_NotPorted_Finish(self, next);
    self->ent->s.lerp.eFlags2 &= ~2;
    if (G_SP_GetScriptedAnim(self->ent))
        // SP 0x004cb9c0 is MT_Free, not another animation operation. Our fixed
        // side bank releases presence instead of freeing SP's 100-byte allocation.
        G_SP_ClearScriptedAnim(self->ent);
}

// zombies: Actor_CustomAnim_Start (SP 0x00631220).
bool __fastcall Actor_CustomAnim_Start_SP(actor_s *self, ai_state_t previous)
{
    if (!G_SP_IsSPLevel()) // SP has no zombie gate here; MP keeps its path
        return Actor_SP_NotPorted_Start(self, previous);
    // SP scr_const slot 0x023a5866 is "begin_custom_anim" (registration 0x005eb901).
    unsigned int notify = SL_GetString((char *)"begin_custom_anim", 0, SCRIPTINSTANCE_SERVER);
    Scr_Notify(self->ent, notify, 0);
    SL_RemoveRefToString(SCRIPTINSTANCE_SERVER, notify);
    return true;
}

// zombies: Actor_PreThink (SP 0x0062bbf0). P6c's equivalent is file-local in
// another worker's file. Keep this small caller dependency here until shared.
static void Actor_ScriptedPreThink_SP(actor_s *self)
{
    if (self->preThinkTime == level.time)
        return;
    self->preThinkTime = level.time;
    if (self->flashBanged)
        return;
    if (self->pFavoriteEnemy.isDefined())
        Actor_GetPerfectInfo(self, self->pFavoriteEnemy.sentient());
    if (self->eSubState[self->stateLevel] == STATE_EXPOSED_COMBAT && !Actor_GetTargetEntity(self))
    {
        // SP DAT_01a48ca0 statistic; no KB consumer. P6c has a separate local counter.
        static unsigned int noTargetCount;
        ++noTargetCount;
        self->threatUpdateTime = level.time;
    }
    Actor_UpdateSight(self);
    Actor_UpdateThreat(self);
    Actor_UpdateLastEnemySightPos(self);
}

// zombies: Actor_CustomAnim_Think (SP 0x0040d530).
actor_think_result_t __fastcall Actor_CustomAnim_Think_SP(actor_s *self)
{
    if (!G_SP_IsSPLevel()) // SP has no zombie gate here; MP keeps its path
        return Actor_SP_NotPorted_Think(self);
    self->pszDebugInfo = "animcustom";
    if (!self->noDodgeMove)
    {
        Actor_ClearKeepClaimedNode(self); // SP 0x004d01c0
        Sentient_ClaimNode(self->sentient, NULL); // SP 0x00635000
    }
    Actor_ClearPath(self);
    Actor_AnimSpecific(self, &self->AnimScriptSpecific, AI_ANIM_USE_BOTH_DELTAS, true); // SP 0x004cbd10
    if (!Actor_IsAnimScriptAlive(self))
    {
        Actor_PopState(self);
        return ACTOR_THINK_REPEAT;
    }
    Actor_ScriptedPreThink_SP(self);
    Actor_UpdateOriginAndAngles_SP(self); // zombies: shared P6c movement (SP 0x00535680)
    return ACTOR_THINK_DONE;
}

// zombies: vector correction (SP 0x00647f50). Keep the one-iteration inverse
// square root and the sums captured before the aliased output/remainder writes.
static void FUN_00647f50(float *origin, float *error, float step)
{
    float squared = error[0] * error[0] + error[1] * error[1] + error[2] * error[2];
    if (squared == 0.0f)
        return;
    int bits;
    memcpy(&bits, &squared, sizeof(bits));
    bits = 0x5f3759df - (bits >> 1);
    float inverse;
    memcpy(&inverse, &bits, sizeof(inverse));
    float scale = (1.5f - squared * 0.5f * inverse * inverse) * inverse * step;
    if (scale < 1.0f)
    {
        float sum[3];
        Vec3Add(origin, error, sum);
        for (int i = 0; i < 3; ++i)
            origin[i] = error[i] * scale + origin[i];
        Vec3Sub(sum, origin, error);
    }
    else
    {
        Vec3Add(origin, error, origin);
        Vec3Clear(error);
    }
}

// zombies: angle correction (SP 0x00406860). The within-step arm does not normalize.
static void FUN_00406860(float *angles, float *error, float step)
{
    for (int i = 0; i < 3; ++i)
    {
        float delta = error[i];
        if (delta == 0.0f)
            continue;
        if (delta <= step)
        {
            if (-step <= delta)
            {
                angles[i] += delta;
                error[i] = 0.0f;
                continue;
            }
            error[i] = delta + step;
            angles[i] = (float)AngleNormalize360(angles[i] - step);
        }
        else
        {
            error[i] = delta - step;
            angles[i] = (float)AngleNormalize360(angles[i] + step);
        }
    }
}

// zombies: scripted root-to-world transform (SP 0x005c4e00).
static void FUN_005c4e00(const scripted_anim_sp_t *scripted, const float *translation,
    const float *rotation, float *origin, float *angles)
{
    MatrixTransformVector43(translation, scripted->axis, origin);
    float localAxis[3][3], worldAxis[3][3];
    // SP 0x005fa290 / 0x0065cd60 are KB RotationToYaw / YawToAxis.
    YawToAxis((float)RotationToYaw(rotation), localAxis);
    MatrixMultiply(localAxis, scripted->axis, worldAxis);
    AxisToAngles(worldAxis, angles);
}

// zombies: absolute scripted transform (SP 0x007d4fb0).
static void FUN_007d4fb0(DObj *obj, scripted_anim_sp_t *scripted, float *origin, float *angles)
{
    float rotation[2], translation[3];
    XAnimCalcAbsDelta(obj, scripted->anim, rotation, translation); // SP 0x0059a5c0
    FUN_005c4e00(scripted, translation, rotation, origin, angles);
}

// L15 (TEST, bo1_measure_animscripted_cl, measurement only): the server thread's copy of each entity's scripted
// record at its last G_ScriptedAnim_Think, for the client trace (the client runs on its own thread and must not
// follow server tree pointers): the record, the anim tree's anims, the scripted anim's effective server weight and
// the server's offset of its final placement from the pure anim placement (error blend-out, deathplant).
namespace
{
    struct ScriptedAnimMeasure
    {
        int time;
        const XAnim_s *anims;
        scripted_anim_sp_t scripted;
        float offset[3];
        float yawOffset;
        float serverWeight;
        bool linked;
    };
    ScriptedAnimMeasure s_scriptedAnimMeasure[MAX_GENTITIES_SV];

    bool ScriptedAnimMeasureOn()
    {
        static const dvar_s *enabled;
        if (!enabled)
            enabled = Dvar_FindVar("bo1_measure_animscripted_cl");
        return enabled && enabled->current.enabled;
    }

    void ScriptedAnimMeasureRecord(gentity_s *ent, const XAnimTree_s *tree, const scripted_anim_sp_t *scripted,
        const float *animOrigin, const float *animAngles)
    {
        ScriptedAnimMeasure &record = s_scriptedAnimMeasure[ent->s.number];
        record.time = 0;
        record.linked = ent->tagInfo != nullptr;
        record.anims = XAnimGetAnims(tree);
        record.scripted = *scripted;
        Vec3Sub(ent->r.currentOrigin, animOrigin, record.offset);
        record.yawOffset = (float)AngleDelta(ent->r.currentAngles[1], animAngles[1]);
        record.serverWeight = G_SP_MeasureEffectiveWeight(tree, scripted->anim);
        record.time = level.time;
    }
}

// L15 (TEST): the SP-style placement (FUN_007d4fb0 on obj, the client's DObj) plus the server's last offset, from
// the record of entnum when its last think is at most 150 ms after time, so the server was still in the scripted
// anim at the client time (*placed false for a linked entity). False without such a record.
bool G_SP_MeasureScriptedPlacement(int entnum, int time, DObj *obj, float *origin, float *yaw,
    unsigned int *anim, int *startTime, float *serverWeight, bool *placed)
{
    if (entnum < 0 || entnum >= MAX_GENTITIES_SV || !obj)
        return false;
    ScriptedAnimMeasure record = s_scriptedAnimMeasure[entnum];
    const XAnimTree_s *tree = DObjGetTree(obj);
    if (!record.time || time > record.time || record.time - time > 150 || !tree || XAnimGetAnims(tree) != record.anims
        || !record.scripted.anim || record.scripted.anim >= record.anims->size)
        return false;
    *placed = !record.linked;
    if (*placed)
    {
        float angles[3];
        FUN_007d4fb0(obj, &record.scripted, origin, angles);
        Vec3Add(origin, record.offset, origin);
        *yaw = angles[1] + record.yawOffset;
    }
    *anim = record.scripted.anim;
    *startTime = record.scripted.startTime;
    *serverWeight = record.serverWeight;
    return true;
}

// zombies: parent movement since scripted setup (SP 0x004a1370).
static void FUN_004a1370(gentity_s *ent, float axis[4][3])
{
    float parentAxis[4][3];
    G_CalcTagParentAxis(ent, parentAxis);
    // SP 0x0050cb40 is MatrixMultiply43, including the translation row.
    MatrixMultiply43(ent->tagInfo->parentInvAxis, parentAxis, axis);
}

// zombies: placement (SP 0x007d53f0).
static void FUN_007d53f0(gentity_s *ent, const float *origin, const float *angles)
{
    if (ent->tagInfo)
    {
        float linkAxis[4][3];
        FUN_004a1370(ent, linkAxis);
        float localAxis[4][3], worldAxis[4][3];
        Vec3Copy(origin, localAxis[3]);
        AnglesToAxis(angles, localAxis);
        MatrixMultiply43(localAxis, linkAxis, worldAxis);
        Vec3Copy(worldAxis[3], ent->r.currentOrigin);
        AxisToAngles(worldAxis, ent->r.currentAngles);
        G_CalcTagAxis(ent, 0);
        return;
    }
    Vec3Copy(origin, ent->r.currentOrigin);
    Vec3Copy(angles, ent->r.currentAngles);
}

// zombies: the formerly opaque tail is four floats (SP scripted record+0x54..0x60).
// Copy through memcpy to preserve the existing record's size, layout and aliasing.
struct ScriptedDeathPlantData
{
    float heightOffset;
    float pitch;
    float roll;
    float fraction;
};
static_assert(sizeof(ScriptedDeathPlantData) == 16, "SP deathplant tail");
static_assert(offsetof(scripted_anim_sp_t, deathplantData) == 0x54, "SP deathplant offset");

// zombies: deathplant update (SP 0x007d5230). EDI supplies origin, stack arg 3 angles.
static void FUN_007d5230(gentity_s *ent, XAnimTree_s *tree, float *origin, float *angles)
{
    scripted_anim_sp_t *scripted = G_SP_GetScriptedAnim(ent);
    ScriptedDeathPlantData plant;
    memcpy(&plant, scripted->deathplantData, sizeof(plant));
    float end[3] = { origin[0], origin[1], (plant.heightOffset - 18.0f) + origin[2] };
    float start[3] = { origin[0], origin[1], plant.heightOffset + origin[2] + 18.0f };
    // SP .rdata 0x00a50c9c / 0x00a50ca8. These are not KB's 48-high dog bounds.
    const float mins[3] = { -15.0f, -15.0f, 0.0f };
    const float maxs[3] = { 15.0f, 15.0f, 72.0f };
    trace_t trace;
    col_context_t context; // SP 0x004b3e90
    G_TraceCapsule(&trace, start, mins, maxs, end, ent->s.number, 0x820011, &context); // SP 0x004ee360
    // The byte at SP trace+0x2a is allsolid, not startsolid.
    if (!trace.allsolid && trace.fraction < 1.0f)
    {
        float z = (end[2] - start[2]) * trace.fraction + start[2];
        plant.heightOffset = z - origin[2];
        origin[0] = (end[0] - start[0]) * trace.fraction + start[0];
        origin[1] = (end[1] - start[1]) * trace.fraction + start[1];
        origin[2] = z;
    }
    float fraction;
    if (plant.fraction >= 0.0f)
    {
        plant.fraction += 0.05f;
        if (plant.fraction > 1.0f)
            plant.fraction = 1.0f;
        fraction = plant.fraction;
    }
    else
        fraction = (float)XAnimGetTime(tree, scripted->anim); // SP 0x00547690
    memcpy(scripted->deathplantData, &plant, sizeof(plant));
    angles[0] += plant.pitch * fraction;
    angles[2] += plant.roll * fraction;
}

// zombies: deathplant initialization (SP 0x007d54e0). EAX supplies requested origin.
static void FUN_007d54e0(gentity_s *ent, XAnim_s *anims, unsigned int anim,
    const float *origin, const float *angles)
{
    scripted_anim_sp_t *scripted = G_SP_GetScriptedAnim(ent);
    float end[3] = { origin[0], origin[1], origin[2] - 18.0f };
    float start[3] = { origin[0], origin[1], origin[2] + 36.0f };
    const float mins[3] = { -15.0f, -15.0f, 0.0f };
    const float maxs[3] = { 15.0f, 15.0f, 72.0f };
    trace_t trace;
    col_context_t context;
    G_TraceCapsule(&trace, start, mins, maxs, end, ent->s.number, 0x820011, &context);
    if (trace.allsolid || trace.fraction >= 1.0f)
        Vec3Copy(origin, scripted->axis[3]);
    else
        for (int i = 0; i < 3; ++i)
            scripted->axis[3][i] = (end[i] - start[i]) * trace.fraction + start[i];
    ScriptedDeathPlantData plant = {};
    memcpy(scripted->deathplantData, &plant, sizeof(plant));
    float baseAngles[3] = { 0.0f, angles[1], angles[2] };
    AnglesToAxis(baseAngles, scripted->axis);
    float rotation[2], translation[3], finalOrigin[3], finalAngles[3];
    // SP 0x004a3680 is XAnimGetAbsDelta: sample the leaf's end, not its current time.
    XAnimGetAbsDelta(anims, anim, rotation, translation, 1.0f);
    FUN_005c4e00(scripted, translation, rotation, finalOrigin, finalAngles);
    float distance = sqrtf(translation[1] * translation[1] + translation[2] * translation[2]
        + translation[0] * translation[0]) + 128.0f;
    Vec3Copy(finalOrigin, start);
    Vec3Copy(finalOrigin, end);
    start[2] += distance;
    end[2] -= distance;
    G_TraceCapsule(&trace, start, mins, maxs, end, ent->s.number, 0x820011, &context);
    if (trace.allsolid || trace.fraction >= 1.0f)
    {
        plant.pitch = 0.0f;
        plant.roll = 0.0f;
    }
    else
    {
        float hit[3];
        for (int i = 0; i < 3; ++i)
            hit[i] = (end[i] - start[i]) * trace.fraction + start[i];
        // SP 0x00486fc0, same body-plant helper KB already uses for corpses.
        Actor_GetBodyPlantAngles(ent->s.number, 0x820011, hit, finalAngles[1], &plant.pitch, &plant.roll, NULL);
    }
    // SP 0x0058df80 is numframes/framerate (XAnimGetLength). Sub-second clips
    // use animation time; longer clips blend by 0.05 each server frame.
    plant.fraction = XAnimGetLength(anims, anim) < 1.0 ? -1.0f : 0.0f;
    memcpy(scripted->deathplantData, &plant, sizeof(plant));
}

// zombies: preserve look-at weights, then clear the requested root's children
// (SP 0x007d4ff0). ESI is actor, EDI tree; both are lost in the exported prototype.
static void FUN_007d4ff0(gentity_s *ent, XAnimTree_s *tree, DObj *obj, unsigned int root)
{
    actor_s *self = ent->actor;
    if (self && self->lookAtInfo.fLookAtTurnAngle != 0.0f && self->lookAtInfo.bLookAtSetup)
    {
        ActorLookAtInfo *look = &self->lookAtInfo;
        float straight = (float)XAnimGetWeight(tree, look->animLookAtStraight);
        float side = (float)XAnimGetWeight(tree, look->animLookAtLeft);
        if (side > 0.0f)
        {
            int command = G_StoreAnimCommand(ent, tree, 4, look->animLookAtLeft, 0, side, 0.2f, 1.0f, 0);
            XAnimSetGoalWeightKnob(obj, look->animLookAtLeft, side, 0.2f, 1.0f, 0, 0, false, command);
        }
        else
        {
            side = (float)XAnimGetWeight(tree, look->animLookAtRight);
            if (side > 0.0f)
            {
                int command = G_StoreAnimCommand(ent, tree, 4, look->animLookAtRight, 0, side, 0.2f, 1.0f, 0);
                XAnimSetGoalWeightKnob(obj, look->animLookAtRight, side, 0.2f, 1.0f, 0, 0, false, command);
            }
        }
        int command = G_StoreAnimCommand(ent, tree, 3, look->animLookAtStraight, 0, straight, 0.2f, 1.0f, 0);
        XAnimSetCompleteGoalWeight(obj, look->animLookAtStraight, straight, 0.2f, 1.0f, 0, 0, false, command);
        if (!root)
            return;
        ent = self->ent;
    }
    int command = G_StoreAnimCommand(ent, tree, 2, root, 0, 0.0f, 0.2f, 1.0f, 1);
    XAnimClearTreeGoalWeightsStrict(tree, root, 0.2f, command); // SP 0x00665890
}

// zombies: G_ScriptedAnim_Think outer lifetime/placement (SP 0x00501350).
void G_ScriptedAnim_Think_SP(gentity_s *ent)
{
    scripted_anim_sp_t *scripted = G_SP_GetScriptedAnim(ent);
    if (!scripted)
        return;
    XAnimTree_s *tree = G_GetEntAnimTree(ent);
    if (!tree || !scripted->anim)
    {
        G_SP_ClearScriptedAnim(ent);
        return;
    }
    DObj *obj = Com_GetServerDObj(ent->s.number);
    float origin[3] = {}, angles[3] = {};
    FUN_007d4fb0(obj, scripted, origin, angles);
    float animOrigin[3], animAngles[3]; // L15 TEST (bo1_measure_animscripted_cl)
    Vec3Copy(origin, animOrigin);
    Vec3Copy(angles, animAngles);
    if (scripted->deathplant == 1)
        FUN_007d5230(ent, tree, origin, angles);
    FUN_00647f50(origin, scripted->originError, 0.25f);
    FUN_00406860(angles, scripted->anglesError, G_EntSpExt(ent).anglelerprate * 0.05f);
    FUN_007d53f0(ent, origin, angles);
    if (ScriptedAnimMeasureOn())
        ScriptedAnimMeasureRecord(ent, tree, scripted, animOrigin, animAngles);
    if (!scripted->started)
    {
        scripted->started = 1;
        return;
    }
    // SP 0x0066cd90 matches KB XAnimHasFinished: absent node, wrap, time 1 or
    // signed cycle advance. It does not test weight or use an elapsed deadline.
    if (!XAnimHasFinished(tree, scripted->anim))
        return;
    int command = G_StoreAnimCommand(ent, tree, 3, scripted->anim, 0, 1.0f, 0.2f, 1.0f, 1);
    XAnimSetCompleteGoalWeight(obj, scripted->anim, 1.0f, 0.2f, 1.0f, 0, 0, false, command);
    scripted->anim = 0;
    G_SP_ClearScriptedAnim(ent);
    // zombies: script-model completion resets animation id and placement (SP 0x00501350).
    // The SP start timestamp lives in scripted->startTime; clearing the record resets it.
    if (ent->s.eType == ET_SCRIPTMOVER)
    {
        ent->s.lerp.u.scriptMover.animScriptedAnim = 0;
        G_SetOrigin(ent, ent->r.currentOrigin);
        G_SetAngle(ent, ent->r.currentAngles);
    }
}

// zombies: Actor_ScriptedAnim_Think (SP 0x004f9f30).
actor_think_result_t __fastcall Actor_ScriptedAnim_Think_SP(actor_s *self)
{
    if (!G_SP_IsSPLevel()) // SP has no zombie gate here; MP keeps its path
        return Actor_SP_NotPorted_Think(self);
    self->pszDebugInfo = "animscripted";
    if (!self->noDodgeMove)
    {
        Actor_ClearKeepClaimedNode(self);
        Sentient_ClaimNode(self->sentient, NULL);
    }
    Actor_ClearPath(self);
    // zombies: Actor_AnimScripted (SP 0x005d10c0); EDX is species-list+0x98
    // in SP, the semantic scripted slot in KB's differently laid-out list.
    iassert(g_animScriptTable[self->species]);
    Actor_SetAnimScript(self, &g_animScriptTable[self->species]->scripted,
        AI_MOVE_STOP, AI_ANIM_USE_BOTH_DELTAS, AI_ANIM_FUNCTION_STOP);
    self->bUseGoalWeight = true;
    self->pushable = false; // SP actor+0x21b1
    if (!Actor_IsAnimScriptAlive(self))
    {
        if (self->eSimulatedState[self->simulatedStateLevel] == AIS_SCRIPTEDANIM)
            Actor_PopState(self);
        return ACTOR_THINK_REPEAT;
    }
    gentity_s *ent = self->ent;
    G_ScriptedAnim_Think_SP(ent);
    if (!G_SP_GetScriptedAnim(ent))
    {
        Actor_PopState(self);
        return ACTOR_THINK_REPEAT;
    }
    Actor_ScriptedPreThink_SP(self);
    Actor_SetDesiredAngles(&self->CodeOrient, ent->r.currentAngles[0], ent->r.currentAngles[1]);
    Vec3Clear(self->Physics.vVelocity);
    Vec3Clear(self->Physics.vWishDelta);
    return ACTOR_THINK_DONE;
}

// zombies: G_ScriptedAnim_Begin (SP 0x0044ece0).
static void G_ScriptedAnim_Begin_SP(gentity_s *ent, const float *origin, const float *angles,
    unsigned int anim, unsigned int root, unsigned int notifyName, bool deathplant,
    float rate, bool skipRestart, float blendTime)
{
    // zombies: SP capability 0x2000 is set by actors (0x004F5B1F) and script_model (0x0044278E).
    // Keep its script-model bit separate from KB's FL_NO_AUTO_ANIM_UPDATE.
    if (!ent->actor && !G_EntSpExt(ent).scriptedAnimSupported)
        Scr_ObjectError("entity does not currently support animscripted", SCRIPTINSTANCE_SERVER);
    XAnimTree_s *tree = GScr_GetEntAnimTree(ent);
    scripted_anim_sp_t *scripted = G_SP_GetScriptedAnim(ent);
    if (!scripted)
        scripted = G_SP_AllocScriptedAnim(ent);
    scripted->started = 0;
    scripted->anim = anim;
    scripted->root = root;
    scripted->deathplant = deathplant;
    scripted->startTime = level.time;
    G_SP_MeasureAnimScripted(ent, XAnimGetAnims(tree), anim, notifyName, origin, angles, rate, blendTime);
    G_SP_HeadlessDiceAnimScripted(ent, notifyName); // k1 TEST SWITCH bo1_testclient_dice: tear cycle count
    if (ent->s.eType == ET_SCRIPTMOVER)
        ent->s.lerp.u.scriptMover.animScriptedAnim = (short)anim;
    // KB has no SP animation-command snapshot reader. Keep the start timestamp in
    // the owned record; never overwrite KB's attachment tags at SP's raw offset 0x54.
    if (deathplant)
        FUN_007d54e0(ent, XAnimGetAnims(tree), anim, origin, angles);
    else
    {
        Vec3Copy(origin, scripted->axis[3]);
        AnglesToAxis(angles, scripted->axis);
    }
    DObj *obj = Com_GetServerDObj(ent->s.number);
    FUN_007d4ff0(ent, tree, obj, root);
    bool restart = !skipRestart && !XAnimIsLooped(XAnimGetAnims(tree), anim);
    int command = G_StoreAnimCommand(ent, tree, 3, anim, 0, 1.0f, blendTime, rate, restart ? 3 : 1);
    // The notify is the script's argument 0, not "animscripted" or the animation
    // asset name. XAnim delivers ent notifyName("end"/notetrack) through Scr_NotifyNum.
    XAnimSetCompleteGoalWeight(obj, anim, 1.0f, blendTime, rate, notifyName, 2, restart, command);
    G_FlagAnimForUpdate(ent);
    float rotation[2], translation[3], initialOrigin[3], initialAngles[3];
    float localAxis[3][3], initialAxis[3][3];
    XAnimCalcAbsDelta(obj, anim, rotation, translation); // SP 0x0059a5c0
    MatrixTransformVector43(translation, scripted->axis, initialOrigin);
    YawToAxis((float)RotationToYaw(rotation), localAxis);
    MatrixMultiply(localAxis, scripted->axis, initialAxis);
    AxisToAngles(initialAxis, initialAngles);
    Vec3Sub(ent->r.currentOrigin, initialOrigin, scripted->originError);
    AnglesSubtract(ent->r.currentAngles, initialAngles, scripted->anglesError);
}

// zombies: G_ScriptedAnim_Start script-argument front end (SP 0x00624f30).
// animscripted requests the actor state and executes scripted::init with eight
// arguments; startscriptedanim bypasses that thread and calls Begin directly.
static void G_ScriptedAnim_Start_SP(scr_entref_t entref, bool runInit, bool skipRestart)
{
    gentity_s *ent = GetEntity(entref);
    DObj *obj = Com_GetServerDObj(ent->s.number);
    if (!obj)
        Scr_ObjectError("No model exists.", SCRIPTINSTANCE_SERVER);
    XAnimTree_s *tree = GScr_GetEntAnimTree(ent);
    float rate = 1.0f;
    scr_anim_s root = {};
    unsigned int mode = 0;
    float blendTime = 0.0f;
    bool deathplant = false;
    if (Scr_GetNumParam(SCRIPTINSTANCE_SERVER) > 4)
    {
        mode = Scr_GetConstString(4, SCRIPTINSTANCE_SERVER);
        const char *modeName = SL_ConvertToString(mode, SCRIPTINSTANCE_SERVER);
        deathplant = !strcmp(modeName, "deathplant");
        if (strcmp(modeName, "normal") && !deathplant)
            Scr_Error(va("Illegal mode %s for animScripted. Valid modes are normal and deathplant", modeName), false);
        if (Scr_GetNumParam(SCRIPTINSTANCE_SERVER) > 5 && Scr_GetType(5, SCRIPTINSTANCE_SERVER))
            root = Scr_GetAnim(5, tree, SCRIPTINSTANCE_SERVER);
        if (Scr_GetNumParam(SCRIPTINSTANCE_SERVER) > 6 && Scr_GetType(6, SCRIPTINSTANCE_SERVER))
            rate = (float)Scr_GetFloat(6, SCRIPTINSTANCE_SERVER);
        if (Scr_GetNumParam(SCRIPTINSTANCE_SERVER) > 7 && Scr_GetType(7, SCRIPTINSTANCE_SERVER))
            blendTime = (float)Scr_GetFloat(7, SCRIPTINSTANCE_SERVER);
    }
    scr_anim_s anim = Scr_GetAnim(3, tree, SCRIPTINSTANCE_SERVER);
    // mod (L61): an anim of another animtree than the entity's is skipped. SP's Scr_GetAnim has no tree check (the exe
    // has no such error string) and indexes past the entity's tree; retail never mixes them, but the nightmare mod on
    // Kino does: Shangri-La's sonic death scream (_zombiemode_ai_sonic.gsc zombie_sonic_scream_death) plays generic_human
    // %ai_zombie_taunts_9 on every AI in range, a hellhound (tree with 62 anims) included -> KB assert "animIndex <
    // tree->anims->size" (301, 62) in xanim.cpp, L61 runs k2/k3. Before any actor state change, so nothing is left half set.
    {
        XAnim_s *animTreeAnims = Scr_GetAnims(anim.tree, SCRIPTINSTANCE_SERVER);
        if (animTreeAnims && animTreeAnims != XAnimGetAnims(tree))
        {
            Com_DPrintf(16, "mod: animscripted on entity %i skipped: anim %i of another animtree\n", ent->s.number, anim.index);
            return;
        }
    }
    float angles[3], origin[3];
    Scr_GetVector(2, angles, SCRIPTINSTANCE_SERVER);
    Scr_GetVector(1, origin, SCRIPTINSTANCE_SERVER);
    unsigned int notifyName = Scr_GetConstString(0, SCRIPTINSTANCE_SERVER);
    actor_s *self = ent->actor;
    if (self)
    {
        if (!self->Physics.bIsAlive) // SP actor+0xe24; NOT entity health or actor inuse
            Scr_Error(va("tried to play a scripted animation on a dead AI; entity %i origin %g %g %g targetname %s classname %s",
                ent->s.number, ent->r.currentOrigin[0], ent->r.currentOrigin[1], ent->r.currentOrigin[2],
                ent->targetname ? SL_ConvertToString(ent->targetname, SCRIPTINSTANCE_SERVER) : "<undefined>",
                SL_ConvertToString(ent->classname, SCRIPTINSTANCE_SERVER)), false);
        if (runInit)
        {
            Actor_PushState(self, AIS_SCRIPTEDANIM); // SP ignores the return value here
            Actor_KillAnimScript(self);
            scripted_anim_sp_t *scripted = G_SP_GetScriptedAnim(ent);
            if (scripted && scripted->anim)
            {
                int command = G_StoreAnimCommand(ent, tree, 3, scripted->anim, 0, 1.0f, blendTime, rate, 1);
                XAnimSetCompleteGoalWeight(obj, scripted->anim, 1.0f, blendTime, rate, 0, 0, false, command);
                G_FlagAnimForUpdate(ent);
            }
            if (!g_scr_data_sp.scriptedInit)
                Scr_Error("animscripted: animscripts/zombie_scripted::init is not loaded", false);
            // zombies: SP 0x00625275..0x006252fe pushes eight script arguments
            // in reverse: float goalTime, float rate, anim/undefined root,
            // string/undefined mode, anim, vector angles, vector origin, string notifyName.
            Scr_AddFloat(blendTime, SCRIPTINSTANCE_SERVER);
            Scr_AddFloat(rate, SCRIPTINSTANCE_SERVER);
            if (!root.tree)
                Scr_AddUndefined(SCRIPTINSTANCE_SERVER);
            else
                Scr_AddAnim(root, SCRIPTINSTANCE_SERVER);
            if (!mode)
                Scr_AddUndefined(SCRIPTINSTANCE_SERVER);
            else
                Scr_AddConstString(mode, SCRIPTINSTANCE_SERVER);
            Scr_AddAnim(anim, SCRIPTINSTANCE_SERVER);
            Scr_AddVector(angles, SCRIPTINSTANCE_SERVER);
            Scr_AddVector(origin, SCRIPTINSTANCE_SERVER);
            Scr_AddConstString(notifyName, SCRIPTINSTANCE_SERVER);
            // SP 0x0062530c reads the init handle at 0x01c79b2c, not species.init.
            unsigned int thread = Scr_ExecEntThread(ent, g_scr_data_sp.scriptedInit, 8);
            Scr_FreeThread(thread, SCRIPTINSTANCE_SERVER); // SP 0x00625317
        }
        else
            G_ScriptedAnim_Begin_SP(ent, origin, angles, anim.index, root.index, notifyName,
                deathplant, rate, skipRestart, blendTime);
    }
    else
        G_ScriptedAnim_Begin_SP(ent, origin, angles, anim.index, root.index, notifyName,
            deathplant, rate, skipRestart, blendTime);
    if ((!self || runInit) && ent->tagInfo)
    {
        // SP G_GetLinkParentOrientation 0x004d6e70 -> MatrixInverseOrthogonal43
        // 0x005eee70 writes tagInfo+0x40 (parentInvAxis).
        float parentAxis[4][3];
        G_CalcTagParentAxis(ent, parentAxis);
        MatrixInverseOrthogonal43(parentAxis, ent->tagInfo->parentInvAxis);
    }
}

// zombies: animscripted (SP 0x00808690).
void __cdecl GScr_AnimScripted_SP(scr_entref_t entref)
{
    ScriptedAnim_RequireZombies();
    if (Scr_GetNumParam(SCRIPTINSTANCE_SERVER) > 8)
        Scr_Error("too many parameters", false);
    if (Scr_GetNumParam(SCRIPTINSTANCE_SERVER) < 4 || Scr_GetNumParam(SCRIPTINSTANCE_SERVER) > 8)
        Scr_Error("Incorrect number of parameters for animscripted command", false);
    G_ScriptedAnim_Start_SP(entref, true, false);
}

// zombies: startscriptedanim (SP 0x007c98f0).
void __cdecl GScr_StartScriptedAnim_SP(scr_entref_t entref)
{
    ScriptedAnim_RequireZombies();
    Actor_Get(entref);
    if (Scr_GetNumParam(SCRIPTINSTANCE_SERVER) > 8)
        Scr_Error("too many parameters", false);
    G_ScriptedAnim_Start_SP(entref, false, false);
}

// zombies: stopanimscripted (SP 0x004ce100): GetEntity, then G_SP_StopScriptedAnim.
void __cdecl GScr_StopAnimScripted_SP(scr_entref_t entref)
{
    ScriptedAnim_RequireZombies();
    G_SP_StopScriptedAnim(GetEntity(entref));
}

// zombies: stop an entity's scripted anim (SP 0x008084f0, entity in EDI), also the first call of
// G_SetAnimTree (SP 0x00502837). The optional blend time is the running script call's argument 0.
void G_SP_StopScriptedAnim(gentity_s *ent)
{
    actor_s *self = ent->actor;
    if (self && self->eSimulatedState[self->simulatedStateLevel] == AIS_SCRIPTEDANIM)
        Actor_PopState(self);
    if (ent->s.eType == ET_SCRIPTMOVER)
    {
        // zombies: stop clears the script-mover animation id (SP 0x008084F0).
        ent->s.lerp.u.scriptMover.animScriptedAnim = 0;
    }
    scripted_anim_sp_t *scripted = G_SP_GetScriptedAnim(ent);
    if (!scripted)
        return;
    if (scripted->anim)
    {
        float blendTime = 0.2f;
        if (Scr_GetNumParam(SCRIPTINSTANCE_SERVER) && Scr_GetType(0, SCRIPTINSTANCE_SERVER))
            blendTime = (float)Scr_GetFloat(0, SCRIPTINSTANCE_SERVER);
        XAnimTree_s *tree = GScr_GetEntAnimTree(ent);
        // SP 0x00808605 pushes flags 0 here, despite using the complete setter.
        int command = G_StoreAnimCommand(ent, tree, 3, scripted->anim, 0, 0.0f, blendTime, 1.0f, 0);
        DObj *obj = Com_GetServerDObj(ent->s.number);
        if (obj)
            XAnimSetCompleteGoalWeight(obj, scripted->anim, 0.0f, blendTime, 1.0f, 0, 0, false, command);
    }
    G_SP_ClearScriptedAnim(ent);
}

// zombies: setproneanimnodes (SP 0x007ca720). The three SP indices have no KB
// actor members; reciprocal pitch fields already exist. This is only the writer.
void __cdecl ActorCmd_SetProneAnimNodes_SP(scr_entref_t entref)
{
    ScriptedAnim_RequireZombies();
    float down = (float)Scr_GetFloat(0, SCRIPTINSTANCE_SERVER);
    float up = (float)Scr_GetFloat(1, SCRIPTINSTANCE_SERVER);
    actor_s *self = Actor_Get(entref);
    XAnimTree_s *tree = G_GetActorAnimTree(self);
    if (down >= 0.0f)
        Scr_Error("Down angle (parameter 1) must be set to be less than 0.", false);
    if (up <= 0.0f)
        Scr_Error("Up angle (parameter 2) must be set to be greater than 0.", false);
    self->fInvProneAnimLowPitch = 1.0f / down;
    self->fInvProneAnimHighPitch = 1.0f / up;
    unsigned short *nodes = Actor_SP_ProneAnimNodes(self);
    nodes[0] = Scr_GetAnim(2, tree, SCRIPTINSTANCE_SERVER).index;
    nodes[1] = Scr_GetAnim(3, tree, SCRIPTINSTANCE_SERVER).index;
    nodes[2] = Scr_GetAnim(4, tree, SCRIPTINSTANCE_SERVER).index;
}

// zombies: G_RunMover scripted-animation prefix (SP 0x0066AA60).
bool G_RunScriptedMover_SP(gentity_s *ent)
{
    if (!G_SP_IsSPLevel() || !G_SP_GetScriptedAnim(ent))
        return false;
    const bool ragdoll = ent->s.lerp.pos.trType >= TR_RAGDOLL && ent->s.lerp.pos.trType <= TR_RAGDOLL_INTERPOLATE;
    G_ScriptedAnim_Think_SP(ent);
    G_SetOrigin(ent, ent->r.currentOrigin);
    G_SetAngle(ent, ent->r.currentAngles);
    SV_LinkEntity(ent);
    scripted_anim_sp_t *scripted = G_SP_GetScriptedAnim(ent);
    if (scripted)
    {
        ent->s.lerp.pos.trType = TR_INTERPOLATE;
        ent->s.lerp.apos.trType = TR_INTERPOLATE;
        G_RunThink(ent);
        Vec3Copy(scripted->axis[3], ent->s.lerp.pos.trDelta);
        AxisToAngles(scripted->axis, ent->s.lerp.apos.trDelta);
        if (ragdoll)
            ent->s.lerp.pos.trType = ent->s.lerp.apos.trType = TR_RAGDOLL_INTERPOLATE;
        return true;
    }
    if (ragdoll)
        ent->s.lerp.pos.trType = ent->s.lerp.apos.trType = TR_RAGDOLL;
    return false;
}
