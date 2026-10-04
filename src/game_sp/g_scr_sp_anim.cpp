// SP builtin table, lane D: the anim family (clearanim, set*animknob*, animscripted, ...) and notetracks.
// Searched after BO1Zombies's own tables and before the generated not-ported table (see
// src/game_sp/scr_sp_tables.h). Add a row per real port, keep the end marker last, then rerun
// `python tools/gen_sp_builtins.py` so the name's not-ported stub is dropped.
#include "scr_sp_tables.h"
#include "g_anim_commands_sp.h"
#include <game/actor_scripted.h>
#include <game_mp/g_scr_main_mp.h>
#include <game_mp/g_main_mp.h>
#include <clientscript/cscr_vm.h>
#include <clientscript/cscr_animtree.h>
#include <qcommon/dobj_management.h>
#include <xanim/xanim.h>
#include <universal/dvar.h>
#include <cmath>

// zombies: clearanim (SP 0x00800660).
static void __cdecl GScr_ClearAnim(scr_entref_t entref)
{
    gentity_s *ent = GetEntity(entref);
    XAnimTree_s *tree = GScr_GetEntAnimTree(ent);
    unsigned int anim = Scr_GetAnim(0, tree, SCRIPTINSTANCE_SERVER).index;
    float blendTime = (float)Scr_GetFloat(1, SCRIPTINSTANCE_SERVER);
    int cmdIndex = G_StoreAnimCommand(ent, tree, 1, anim, 0, 0.0f, blendTime, 1.0f, 1);
    XAnimClearTreeGoalWeights(tree, anim, blendTime, cmdIndex);
}

// zombies: SetAnimKnob / SetAnimKnobAll cores (SP 0x00800880 / 0x00800d40),
// flagged cores (SP 0x00801870 / 0x00801d70 / 0x00802220).
// KB exposes GScr_GetOptionalFloat and GScr_HandleAnimError, but its SetAnim core
// cannot perform the knob sibling/ancestor walk. Share that marshalling here.
static void GScr_SetAnimKnobInternal(scr_entref_t entref, unsigned char flags, bool all,
    bool flagged = false, bool knob = true)
{
    gentity_s *ent = GetEntity(entref);
    XAnimTree_s *tree = GScr_GetEntAnimTree(ent);
    float rate = 1.0f;
    float goalTime = 0.2f;
    float goalWeight = 1.0f;
    unsigned int count = Scr_GetNumParam(SCRIPTINSTANCE_SERVER);
    unsigned int animParam = flagged ? 1 : 0;
    if (count > animParam)
    {
        scr_anim_s anim = Scr_GetAnim(animParam, tree, SCRIPTINSTANCE_SERVER);
        XAnimGetParamValue(tree, anim.index, "rate", &rate);
        XAnimGetParamValue(tree, anim.index, "goaltime", &goalTime);
        XAnimGetParamValue(tree, anim.index, "goalweight", &goalWeight);
    }
    unsigned int firstFloat = animParam + (all ? 2 : 1);
    if (count < firstFloat || count > firstFloat + 3)
    {
        const char *message = all ? "incorrect number of parameters" : "too many parameters";
        if (all && flagged)
            message = flags == 3 ? "illegal call to SetFlaggedAnimKnobAllRestart()\n"
                : "illegal call to SetFlaggedAnimKnobAll()\n";
        Scr_Error(message, false);
    }
    // SP reads optional arguments in reverse order, retaining animtree defaults for undefined.
    if (count == firstFloat + 3)
    {
        rate = (float)GScr_GetOptionalFloat(firstFloat + 2, rate);
        if (rate < 0.0f)
            Scr_ParamError(firstFloat + 2, "must set nonnegative rate", SCRIPTINSTANCE_SERVER);
    }
    if (count >= firstFloat + 2)
    {
        goalTime = (float)GScr_GetOptionalFloat(firstFloat + 1, goalTime);
        if (goalTime < 0.0f)
            Scr_ParamError(firstFloat + 1, "must set nonnegative goal time", SCRIPTINSTANCE_SERVER);
    }
    if (count >= firstFloat + 1)
    {
        goalWeight = (float)GScr_GetOptionalFloat(firstFloat, goalWeight);
        if (flagged && goalWeight <= 0.0f)
            Scr_ParamError(firstFloat, "must set positive weight", SCRIPTINSTANCE_SERVER);
        if (!flagged && goalWeight < 0.0f)
            Scr_ParamError(firstFloat, "must set nonnegative weight", SCRIPTINSTANCE_SERVER);
    }
    scr_anim_s root = {};
    if (all)
        root = Scr_GetAnim(animParam + 1, tree, SCRIPTINSTANCE_SERVER);
    scr_anim_s anim = Scr_GetAnim(animParam, tree, SCRIPTINSTANCE_SERVER);
    unsigned int notifyName = 0;
    if (flagged)
    {
        // XAnim owns this name and delivers each note through XAnimProcessServerNotify
        // -> NotifyServerNotetrack -> Scr_NotifyNum(entnum, notifyName, note).
        // SP's command record contains no notify name; it is not a script-notify queue.
        notifyName = Scr_GetConstString(0, SCRIPTINSTANCE_SERVER);
        if (!XAnimHasTime(XAnimGetAnims(tree), anim.index))
            Scr_ParamError(1, "blended nonsynchronized animation has no concept of time", SCRIPTINSTANCE_SERVER);
    }
    if (all)
    {
        if (root.tree != anim.tree)
            Scr_Error("root anim is not in the same anim tree", false);
        if (root.index == anim.index)
            Scr_Error("root anim is not an ancestor of the anim", false);
    }
    // The flagged single-knob core checks the model before storing the command;
    // the other SP cores store first. Keep the error path's ordering too.
    DObj *obj = nullptr;
    if (flagged && knob && !all)
    {
        obj = Com_GetServerDObj(ent->s.number);
        if (!obj)
            Scr_ObjectError("No model exists.", SCRIPTINSTANCE_SERVER);
    }
    int cmdIndex = G_StoreAnimCommand(ent, tree, all ? 5 : (knob ? 4 : 3), anim.index, root.index,
        goalWeight, goalTime, rate, flags);
    if (!obj)
        obj = Com_GetServerDObj(ent->s.number);
    if (!obj)
        Scr_ObjectError("No model exists.", SCRIPTINSTANCE_SERVER);
    unsigned int notifyType = goalWeight <= 0.001f ? 0 : 2;
    int error;
    if (all)
    {
        if (flags & 1)
            error = XAnimSetCompleteGoalWeightKnobAll(obj, anim.index, root.index, goalWeight,
                goalTime, rate, notifyName, notifyType, (flags & 2) != 0, cmdIndex);
        else
            error = XAnimSetGoalWeightKnobAll(obj, anim.index, root.index, goalWeight,
                goalTime, rate, notifyName, notifyType, (flags & 2) != 0, cmdIndex);
    }
    else if (!knob)
    {
        if (flags & 1)
            error = XAnimSetCompleteGoalWeight(obj, anim.index, goalWeight, goalTime, rate,
                notifyName, notifyType, (flags & 2) != 0, cmdIndex);
        else
            error = XAnimSetGoalWeight(obj, anim.index, goalWeight, goalTime, rate,
                notifyName, notifyType, (flags & 2) != 0, cmdIndex);
    }
    else if (flags & 1)
        error = XAnimSetCompleteGoalWeightKnob(obj, anim.index, goalWeight, goalTime, rate,
            notifyName, notifyType, (flags & 2) != 0, cmdIndex);
    else
        error = XAnimSetGoalWeightKnob(obj, anim.index, goalWeight, goalTime, rate,
            notifyName, notifyType, (flags & 2) != 0, cmdIndex);
    if (error)
        GScr_HandleAnimError(error);
    else
        G_FlagAnimForUpdate(ent);
}

// zombies: setanimknob (SP 0x00800cc0).
static void __cdecl GScr_SetAnimKnob(scr_entref_t entref) { GScr_SetAnimKnobInternal(entref, 1, false); }
// zombies: setanimknoblimited (SP 0x00800ce0).
static void __cdecl GScr_SetAnimKnobLimited(scr_entref_t entref) { GScr_SetAnimKnobInternal(entref, 0, false); }
// zombies: setanimknobrestart (SP 0x00800d00).
static void __cdecl GScr_SetAnimKnobRestart(scr_entref_t entref) { GScr_SetAnimKnobInternal(entref, 3, false); }
// zombies: setanimknoball (SP 0x00801170).
static void __cdecl GScr_SetAnimKnobAll(scr_entref_t entref) { GScr_SetAnimKnobInternal(entref, 1, true); }
// zombies: setanimknoballrestart (SP 0x008011b0).
static void __cdecl GScr_SetAnimKnobAllRestart(scr_entref_t entref) { GScr_SetAnimKnobInternal(entref, 3, true); }
// zombies: setflaggedanimknob (SP 0x00801cf0).
static void __cdecl GScr_SetFlaggedAnimKnob(scr_entref_t entref) { GScr_SetAnimKnobInternal(entref, 1, false, true); }
// zombies: setflaggedanimknobrestart (SP 0x00801d30).
static void __cdecl GScr_SetFlaggedAnimKnobRestart(scr_entref_t entref) { GScr_SetAnimKnobInternal(entref, 3, false, true); }
// zombies: setflaggedanimknoball (SP 0x008021c0).
static void __cdecl GScr_SetFlaggedAnimKnobAll(scr_entref_t entref) { GScr_SetAnimKnobInternal(entref, 1, true, true); }
// zombies: setflaggedanimknoballrestart (SP 0x008021f0).
static void __cdecl GScr_SetFlaggedAnimKnobAllRestart(scr_entref_t entref) { GScr_SetAnimKnobInternal(entref, 3, true, true); }
// zombies: setanimlimited (SP 0x00801660), same server setter/validation as KB.
static void __cdecl GScr_SetAnimLimited(scr_entref_t entref) { GScr_SetAnimInternal(entref, 0); }
// zombies: setanimrestart (SP 0x00801680).
static void __cdecl GScr_SetAnimRestart(scr_entref_t entref) { GScr_SetAnimInternal(entref, 3); }
// zombies: setflaggedanim (SP 0x008026a0).
static void __cdecl GScr_SetFlaggedAnim(scr_entref_t entref) { GScr_SetAnimKnobInternal(entref, 1, false, true, false); }
// zombies: setflaggedanimlimited (SP 0x008026c0).
static void __cdecl GScr_SetFlaggedAnimLimited(scr_entref_t entref) { GScr_SetAnimKnobInternal(entref, 0, false, true, false); }
// zombies: setflaggedanimrestart (SP 0x008026e0).
static void __cdecl GScr_SetFlaggedAnimRestart(scr_entref_t entref) { GScr_SetAnimKnobInternal(entref, 3, false, true, false); }

// zombies: getanimtime (SP 0x008016c0).
static void __cdecl GScr_GetAnimTime(scr_entref_t entref)
{
    XAnimTree_s *tree = GScr_GetEntAnimTree(GetEntity(entref));
    unsigned int anim = Scr_GetAnim(0, tree, SCRIPTINSTANCE_SERVER).index;
    if (!XAnimHasTime(XAnimGetAnims(tree), anim))
        Scr_ParamError(0, "blended nonsynchronized animation has no concept of time", SCRIPTINSTANCE_SERVER);
    Scr_AddFloat((float)XAnimGetTime(tree, anim), SCRIPTINSTANCE_SERVER);
}

// zombies: setanimtime (SP 0x008027d0).
static void __cdecl GScr_SetAnimTime(scr_entref_t entref)
{
    gentity_s *ent = GetEntity(entref);
    float time = 0.0f;
    XAnimTree_s *tree = GScr_GetEntAnimTree(ent);
    unsigned int count = Scr_GetNumParam(SCRIPTINSTANCE_SERVER);
    if (count != 1)
    {
        if (count != 2)
            Scr_Error("too many parameters", false);
        time = (float)Scr_GetFloat(1, SCRIPTINSTANCE_SERVER);
        if (time < 0.0f)
            Scr_ParamError(1, "must be > 0", SCRIPTINSTANCE_SERVER);
        else if (time > 1.0f)
            Scr_ParamError(1, "must be < 1", SCRIPTINSTANCE_SERVER);
    }
    unsigned int anim = Scr_GetAnim(0, tree, SCRIPTINSTANCE_SERVER).index;
    XAnim_s *anims = XAnimGetAnims(tree);
    if (!XAnimHasTime(anims, anim))
        Scr_ParamError(0, "not a timed animation", SCRIPTINSTANCE_SERVER);
    if (time == 1.0f && XAnimIsLooped(anims, anim))
        Scr_ParamError(1, "cannot set time 1 on looping animation", SCRIPTINSTANCE_SERVER);
    int cmdIndex = G_StoreAnimCommand(ent, tree, 6, anim, 0, 1.0f, time, 1.0f, 1);
    XAnimSetTime(tree, anim, time, cmdIndex);
    G_FlagAnimForUpdate(ent);
}

// zombies: XAnimAddNotetrackTimesToScriptArray delta overload (SP 0x005a3490).
// The brief said [t, t+d] and the TS returns names in (t, t+d]; the exe instead
// compares absolute distance in SECONDS and returns [anim name, note, time, delta].
// Preserve its two products before subtraction, including float boundary rounding.
static void XAnimAddNotetrackTimesToScriptArray(const XAnim_s *anims, unsigned int animIndex,
    float requestedTime, float delta)
{
    XAnimParts *parts = anims->entries[animIndex].parts;
    iassert(parts);
    const XAnimNotifyInfo *notify = parts->notify;
    if (!notify)
        return;
    float length = (float)parts->numframes / parts->framerate;
    for (unsigned int i = 0; i < parts->notifyCount; ++i, ++notify)
    {
        if (fabsf(length * notify->time - length * requestedTime) <= delta)
        {
            Scr_MakeArray(SCRIPTINSTANCE_SERVER);
            Scr_AddString(parts->name, SCRIPTINSTANCE_SERVER);
            Scr_AddArray(SCRIPTINSTANCE_SERVER);
            Scr_AddConstString(notify->name, SCRIPTINSTANCE_SERVER);
            Scr_AddArray(SCRIPTINSTANCE_SERVER);
            Scr_AddFloat(notify->time, SCRIPTINSTANCE_SERVER);
            Scr_AddArray(SCRIPTINSTANCE_SERVER);
            float rotation[2];
            float translation[3];
            XAnimGetRelDelta(anims, animIndex, rotation, translation, 0.0f, notify->time);
            Scr_AddVector(translation, SCRIPTINSTANCE_SERVER);
            Scr_AddArray(SCRIPTINSTANCE_SERVER);
            Scr_AddArray(SCRIPTINSTANCE_SERVER);
        }
    }
}

// zombies: getnotetracksindelta (SP 0x007f1180).
static void __cdecl GScr_GetNotetracksInDelta()
{
    float delta = 0.15f;
    scr_anim_s anim = Scr_GetAnim(0, nullptr, SCRIPTINSTANCE_SERVER);
    float time = (float)Scr_GetFloat(1, SCRIPTINSTANCE_SERVER);
    if (Scr_GetNumParam(SCRIPTINSTANCE_SERVER) == 3)
        delta = (float)Scr_GetFloat(2, SCRIPTINSTANCE_SERVER);
    Scr_MakeArray(SCRIPTINSTANCE_SERVER);
    XAnimAddNotetrackTimesToScriptArray(Scr_GetAnims(anim.tree, SCRIPTINSTANCE_SERVER), anim.index, time, delta);
}

// zombies: isanimleaf (SP 0x007f12c0).
static void __cdecl GScr_IsAnimLeaf()
{
    scr_anim_s anim = Scr_GetAnim(0, nullptr, SCRIPTINSTANCE_SERVER);
    Scr_AddBool(XAnimIsPrimitive(Scr_GetAnims(anim.tree, SCRIPTINSTANCE_SERVER), anim.index), SCRIPTINSTANCE_SERVER);
}

// zombies: isanimlooping (SP 0x007f1300).
static void __cdecl GScr_IsAnimLooping()
{
    scr_anim_s anim = Scr_GetAnim(0, nullptr, SCRIPTINSTANCE_SERVER);
    Scr_AddBool(XAnimIsLooped(Scr_GetAnims(anim.tree, SCRIPTINSTANCE_SERVER), anim.index), SCRIPTINSTANCE_SERVER);
}

// zombies: getanimassettype (SP 0x00801800 -> 0x00643ff0).
static void __cdecl GScr_GetAnimAssetType(scr_entref_t entref)
{
    XAnimTree_s *tree = GScr_GetEntAnimTree(GetEntity(entref));
    unsigned int anim = Scr_GetAnim(0, tree, SCRIPTINSTANCE_SERVER).index;
    Scr_AddInt(tree->anims->entries[anim].parts->assetType, SCRIPTINSTANCE_SERVER);
}

// zombies: getanimdumpmodel, getanimdumptree, getclosestanimdumpframefortime
// (SP 0x007f1140). These share an actual retail implementation: return "".
static void __cdecl GScr_GetAnimDumpString()
{
    Scr_AddString("", SCRIPTINSTANCE_SERVER);
}

// zombies: getanimdumptotaltime (SP 0x00803dc0). Retail returns integer zero.
static void __cdecl GScr_GetAnimDumpTotalTime()
{
    Scr_AddInt(0, SCRIPTINSTANCE_SERVER);
}

// zombies: setanimdumpuseserveranims (SP 0x007f1150).
static void __cdecl GScr_SetAnimDumpUseServerAnims()
{
    Dvar_SetBoolByName("scriptmover_useServerAnims", Scr_GetInt(0, SCRIPTINSTANCE_SERVER) > 0);
}

// zombies: setanimforcenew (SP 0x00651a30). The retail method is a genuine no-op.
static void __cdecl GScr_SetAnimForceNew(scr_entref_t)
{
}

const BuiltinFunctionDef g_sp_anim_functions[] =
{
    { "getnotetracksindelta", GScr_GetNotetracksInDelta, 0 },
    { "isanimleaf", GScr_IsAnimLeaf, 1 },
    { "isanimlooping", GScr_IsAnimLooping, 1 },
    { "getanimdumpmodel", GScr_GetAnimDumpString, 1 },
    { "getanimdumptree", GScr_GetAnimDumpString, 1 },
    { "getclosestanimdumpframefortime", GScr_GetAnimDumpString, 1 },
    { "getanimdumptotaltime", GScr_GetAnimDumpTotalTime, 1 },
    { "setanimdumpuseserveranims", GScr_SetAnimDumpUseServerAnims, 1 },
    { nullptr, nullptr, 0 } // end marker, keep last
};
const unsigned int g_sp_anim_function_count = ARRAY_COUNT(g_sp_anim_functions) - 1;

const BuiltinMethodDef g_sp_anim_methods[] =
{
    { "animscripted", GScr_AnimScripted_SP, 0 },
    { "startscriptedanim", GScr_StartScriptedAnim_SP, 0 },
    { "stopanimscripted", GScr_StopAnimScripted_SP, 0 },
    { "setproneanimnodes", ActorCmd_SetProneAnimNodes_SP, 0 },
    { "clearanim", GScr_ClearAnim, 0 },
    { "setanimknob", GScr_SetAnimKnob, 0 },
    { "setanimknoblimited", GScr_SetAnimKnobLimited, 0 },
    { "setanimknobrestart", GScr_SetAnimKnobRestart, 0 },
    { "setanimknoball", GScr_SetAnimKnobAll, 0 },
    { "setanimknoballrestart", GScr_SetAnimKnobAllRestart, 0 },
    { "setflaggedanimknob", GScr_SetFlaggedAnimKnob, 0 },
    { "setflaggedanimknobrestart", GScr_SetFlaggedAnimKnobRestart, 0 },
    { "setflaggedanimknoball", GScr_SetFlaggedAnimKnobAll, 0 },
    { "setflaggedanimknoballrestart", GScr_SetFlaggedAnimKnobAllRestart, 0 },
    { "setanimlimited", GScr_SetAnimLimited, 0 },
    { "setanimrestart", GScr_SetAnimRestart, 0 },
    { "setflaggedanim", GScr_SetFlaggedAnim, 0 },
    { "setflaggedanimlimited", GScr_SetFlaggedAnimLimited, 0 },
    { "setflaggedanimrestart", GScr_SetFlaggedAnimRestart, 0 },
    { "getanimtime", GScr_GetAnimTime, 0 },
    { "setanimtime", GScr_SetAnimTime, 0 },
    { "getanimassettype", GScr_GetAnimAssetType, 1 },
    { "setanimforcenew", GScr_SetAnimForceNew, 1 },
    { nullptr, nullptr, 0 } // end marker, keep last
};
const unsigned int g_sp_anim_method_count = ARRAY_COUNT(g_sp_anim_methods) - 1;
