#include <qcommon/actor_model_state.h>
#include <clientscript/cscr_stringlist.h>
#include <cgame_mp/cg_anim_commands.h>
#include <game_sp/actor_sp_ext.h>
#include <game_sp/g_sp_levelstart.h>
#include <game_mp/actor_mp.h>
#include "cg_actors_mp.h"
#include "cg_scr_main_mp.h"
#include <cgame/cg_sp_client_ext.h>
#include <game_sp/g_scr_sp_entity.h>
#include "cg_local_mp.h"
#include <client/splitscreen.h>
#include <clientscript/cscr_vm.h>
#include <clientscript/scr_const.h>
#include <EffectsCore/fx_marks.h>
#include <qcommon/dobj_management.h>
#include "cg_ents_mp.h"
#include "cg_animtree_mp.h"
#include <xanim/dobj_utils.h>
#include "cg_main_mp.h"
#include <cgame/cg_scr_main.h>
#include <bgame/bg_dog.h>
#include <ragdoll/ragdoll.h>
#include <cgame/cg_world.h>
#include "cg_players_mp.h"
#include <gfx_d3d/r_scene.h>
#include <gfx_d3d/r_dobj_skin.h>
#include <game/actor.h>
#include <game_mp/g_main_mp.h>
#include <game_mp/g_scr_main_mp.h>
#include <xanim/xanim.h>
#include <qcommon/cm_trace.h>
#include <universal/surfaceflags.h>
#include <bgame/bg_misc.h>

void __cdecl CG_ActorProcessSnapshot(int localClientNum, centity_s *cent)
{
    bool v2; // [esp+3h] [ebp-5h]
    cg_s *cgameGlob; // [esp+4h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    CG_UpdateActorDObj(localClientNum, cent, BG_SP_GetActorInfo(&cgameGlob->bgs, cent->nextState.lerp.u.actor.actorNum));
    if ( CL_LocalClient_IsFirstActive(localClientNum) )
    {
        if ( cent )
            v2 = ((*((unsigned int *)cent + 201) >> 8) & 1) != 0;
        else
            v2 = 0;
    }
    else
    {
        v2 = 0;
    }
    if ( v2 && cent->currentState.u.turret.ownerNum != cent->nextState.lerp.u.turret.ownerNum )
        CScr_NotifyNum(localClientNum, cent->nextState.number, 0, cscr_const.enemy, 0);
    // zombies: SP CG_SetNextSnap (0x0053D2F0) calls 0x005D8BA0 right after CG_UpdateActorDObj (0x005F9920).
    if ( zombiemode && zombiemode->current.enabled )
        CG_SP_ActorUseServerAnims(localClientNum, cent);
}

// zombies: SP 0x00454430 (unnamed) - the client anim tree slot of an actor or actor corpse entity.
static XAnimTree_s **CG_SP_GetActorClientTreeSlot(int localClientNum, centity_s *cent)
{
    if ( cent->nextState.eType == ET_ACTOR )
        return &BG_SP_GetActorInfo(&CG_GetLocalClientGlobals(localClientNum)->bgs, cent->nextState.lerp.u.actor.actorNum)->pXAnimTree;
    // SP numbers its actor corpses by lerp.u.actor.corpseNum (the corpse is the actor's own entity, 0x00642B80).
    iassert((unsigned int)cent->nextState.lerp.u.actor.corpseNum < (unsigned int)Com_GetMaxActorCorpses());
    return &CG_GetLocalClientStaticGlobals(localClientNum)->actorCorpseInfo[cent->nextState.lerp.u.actor.corpseNum].pXAnimTree;
}

// zombies: SP 0x00646710 (unnamed) - the anims of an entity's es.animtreeIndex: the server's anim tree list while
// sv_running (0x00519160), else the client's (0x00583C90). 0 means the default actor anims (cg+0xBB618 -> +0x8D39C;
// KB uses the listen server's actor anims, the MEASURED choice of CG_LoadAnimTreeInstances, cg_main_mp.cpp).
static XAnim_s *CG_SP_GetActorClientAnims(const centity_s *cent)
{
    if ( !cent->nextState.animtreeIndex )
        return G_SP_GetActorAnims();
    if ( com_sv_running->current.enabled )
        return Scr_GetAnims(cent->nextState.animtreeIndex, SCRIPTINSTANCE_SERVER);
    return Scr_GetClientAnims(cent->nextState.animtreeIndex, SCRIPTINSTANCE_SERVER);
}

// zombies: SP 0x00454430 (unnamed, called by CG_UpdateActorDObj 0x005F9920 and 0x005D8BA0) with 0x0060A060 - the
// actor / actor corpse client tree, rebuilt when its anims are not those of es.animtreeIndex (a dog's own tree).
static XAnimTree_s *CG_SP_GetActorClientTree(int localClientNum, centity_s *cent)
{
    XAnimTree_s **slot = CG_SP_GetActorClientTreeSlot(localClientNum, cent);
    XAnim_s *anims = CG_SP_GetActorClientAnims(cent);

    // KB: XAnimGetAnims asserts on a null tree (SP reads *tree); an empty slot is rebuilt too.
    if ( *slot && XAnimGetAnims(*slot) == anims )
        return *slot;
    // SP 0x0060A060: create the new tree, clear the old one (XAnimFreeTree(slot, 0, 0) 0x004F1720), store the new.
    XAnimTree_s *tree = anims ? XAnimCreateTree(anims, Hunk_AllocXAnimClient) : 0;
    if ( *slot )
        XAnimFreeTree(*slot, 0, SCRIPTINSTANCE_SERVER);
    *slot = tree;
    return tree;
}

// zombies: SP 0x00542FC0 (unnamed) - the listen server's anim tree for an entity, actor and actor corpse arms
// (the only ones 0x005D8BA0 reaches). Dog species have none. An actor's is its slot in the tree bank
// (0x01C7AE80[actorNum]); a corpse's is the tree of corpse slot lerp.u.actor.corpseNum when that slot holds it.
static XAnimTree_s *CG_SP_GetServerAnimTree(int entnum, int eType)
{
    gentity_s *ent;

    if ( entnum >= level.num_entities )
        return 0;
    ent = &g_entities[entnum];
    if ( !ent->r.inuse || ent->s.eType != eType )
        return 0;
    if ( eType == ET_ACTOR )
    {
        if ( Actor_IsDogSpecies(ent->s.lerp.u.actor.species)
            || (unsigned int)ent->s.lerp.u.actor.actorNum >= (unsigned int)MAX_ACTORS ) // mod: was ARRAY_COUNT(actorXAnimTrees) = 32
            return 0;
        return G_ActorXAnimTreeSlot(ent->s.lerp.u.actor.actorNum);
    }
    if ( eType == ET_ACTOR_CORPSE )
    {
        const int corpseNum = ent->s.lerp.u.actor.corpseNum;
        if ( Actor_IsDogSpecies(ent->s.lerp.u.actor.species) || (unsigned int)corpseNum >= MAX_ACTOR_CORPSES_SP
            || g_scr_data.actorCorpseInfo[corpseNum].entnum != entnum )
            return 0;
        return g_scr_data.actorCorpseInfo[corpseNum].tree;
    }
    return 0;
}

// zombies: SP 0x005D8BA0 (unnamed). The brief said SP copies the server's actor anim state to the client after
// every snapshot; the exe does that only on a listen server when ai_useServerAnims is set, or for the one entity
// ai_useServerAnimsEntity names (defaults 0 / -1, SP 0x004A4E90). Retail SP instead replays the anim commands the
// server stores (G_StoreAnimCommand 0x004E7D80) and sends in the snapshot (client side 0x00897660 / 0x00648830 ->
// 0x00525AB0). The serialized snapshot replay now supplies those commands; the copy retains SP's debug gate.
void __cdecl CG_SP_ActorUseServerAnims(int localClientNum, centity_s *cent)
{
    XAnimTree_s **clientTreeSlot;
    XAnimTree_s *clientTree;
    XAnimTree_s *serverTree;
    XAnim_s *clientAnims;
    XAnim_s *serverAnims;
    DObj *obj;

    clientTreeSlot = CG_SP_GetActorClientTreeSlot(localClientNum, cent);
    clientTree = CG_SP_GetActorClientTree(localClientNum, cent); // SP 0x00454430
    if ( !clientTree )
        return;
    if ( !CG_GetLocalClientStaticGlobals(localClientNum)->localServer )
        return;
    if ( !ai_useServerAnims->current.enabled
        && ai_useServerAnimsEntity->current.integer != cent->nextState.number )
    {
        return;
    }
    serverTree = CG_SP_GetServerAnimTree(cent->nextState.number, cent->nextState.eType);
    obj = Com_GetClientDObj(cent->nextState.number, localClientNum);
    if ( !serverTree )
        return;
    clientAnims = XAnimGetAnims(clientTree);
    serverAnims = XAnimGetAnims(serverTree);
    if ( clientAnims != serverAnims )
    {
        XAnimFreeTree(clientTree, 0, SCRIPTINSTANCE_SERVER);
        clientTree = serverAnims ? XAnimCreateTree(serverAnims, Hunk_AllocXAnimClient) : 0;
        *clientTreeSlot = clientTree;
    }
    // KB: SP does not check the DObj; CG_UpdateActorDObj only builds one for a valid entity.
    if ( !clientTree || !obj )
        return;
    DObjSetTree(obj, clientTree);
    DObjSyncServerTree(serverTree, obj, 0.0f);
}

void __cdecl CG_UpdateActorDObj(int localClientNum, centity_s *cent, actorInfo_t *ai)
{
    XAnimTree_s *Tree; // eax
    DObj *v4; // eax
    float *v5; // [esp+0h] [ebp-230h]
    XModel *xmodel; // [esp+4h] [ebp-22Ch]
    cg_s *cgameGlob; // [esp+Ch] [ebp-224h]
    DObj *pDObj; // [esp+10h] [ebp-220h]
    int model; // [esp+14h] [ebp-21Ch]
    entityState_s *p_nextState; // [esp+18h] [ebp-218h]
    const cgs_t *cgs; // [esp+1Ch] [ebp-214h]
    FxMarkDObjUpdateContext markUpdateContext; // [esp+20h] [ebp-210h] BYREF
    int objExists; // [esp+128h] [ebp-108h]
    XAnimTree_s *pAnimTree; // [esp+12Ch] [ebp-104h]
    DObjModel_s dobjModels[32]; // [esp+130h] [ebp-100h] BYREF
    unsigned short numModels = 1;

    if ( ((*((unsigned int *)cent + 201) >> 1) & 1) != 0 )
    {
        p_nextState = &cent->nextState;
        cgameGlob = CG_GetLocalClientGlobals(localClientNum);
        if ( cent->nextState.lerp.u.actor.actorNum >= MAX_ACTORS
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_actors_mp.cpp",
                        123,
                        0,
                        "es->lerp.u.actor.actorNum doesn't index MAX_ACTORS\n\t%i not in [0, %i)",
                        cent->nextState.lerp.u.actor.actorNum,
                        MAX_ACTORS) )
        {
            __debugbreak();
        }
        cgs = CG_GetLocalClientStaticGlobals(localClientNum);
        pDObj = Com_GetClientDObj(p_nextState->number, localClientNum);
        FX_MarkEntUpdateBegin(&markUpdateContext, pDObj, 0, 0);
        objExists = pDObj != 0;
        // zombies: SP CG_UpdateActorDObj (0x005F9920) takes the tree from 0x00454430, which rebuilds it for
        // es.animtreeIndex (dogs); ai is that same slot.
        pAnimTree = zombiemode && zombiemode->current.enabled ? CG_SP_GetActorClientTree(localClientNum, cent) : ai->pXAnimTree;
        model = CG_WhatModelShouldLocalPlayerSee(
                            localClientNum,
                            cgameGlob,
                            cent,
                            cent->nextState.lerp.u.actor.team,
                            cent->nextState.index.brushmodel,
                            cent->nextState.enemyModel);
        if ( pAnimTree && !model )
        {
            XAnimClearTree(pAnimTree);
            if ( objExists )
                CG_SafeDObjFree(localClientNum, p_nextState->number);
            return;
        }
        if ( objExists )
        {
            xmodel = DObjGetModel(pDObj, 0);
            if ( !ai->dobjDirty )
            {
                Tree = DObjGetTree(pDObj);
                if ( pAnimTree == Tree && cgs->gameModels[model] == xmodel )
                    return;
            }
            CG_SafeDObjFree(localClientNum, p_nextState->number);
        }
        dobjModels[0].model = cgs->gameModels[model];
        if ( !dobjModels[0].model
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_actors_mp.cpp",
                        167,
                        0,
                        "%s",
                        "dobjModels[iNumModels].model") )
        {
            __debugbreak();
        }
        dobjModels[0].boneName = 0;
        dobjModels[0].ignoreCollision = 0;
        // zombies: CG_UpdateActorDObj attachment loop (SP 0x005F9A3E..0x005F9A85).
        if ( zombiemode && zombiemode->current.enabled )
        {
            const SPActorModelInfo *info = CG_SP_GetActorModels(localClientNum, ai);
            for ( int i = 0; i < 6 && info->attachModelNames[i][0]; ++i )
            {
                dobjModels[numModels].model = cgameGlob->bgs.GetXModel(const_cast<char *>(info->attachModelNames[i]));
                dobjModels[numModels].boneName = SL_FindString(info->attachTagNames[i], SCRIPTINSTANCE_SERVER);
                dobjModels[numModels].ignoreCollision = (info->attachIgnoreCollision & (1 << i)) != 0;
                ++numModels;
            }
            const dvar_s *measure = Dvar_FindVar("bo1_measure");
            if ( measure && measure->current.enabled )
                Com_Printf(14, "bo1_actor_models: time=%d ent=%d actor=%d attached=%d ignore=%u base=%s first=%s tag=%s\n",
                    cgameGlob->time, p_nextState->number, ai->actorNum, numModels - 1,
                    info->attachIgnoreCollision, info->model, info->attachModelNames[0], info->attachTagNames[0]);
        }
        if ( pAnimTree )
        {
            v4 = Com_ClientDObjCreate(dobjModels, numModels, pAnimTree, p_nextState->number, localClientNum);
            ai->dobjDirty = 0;
            v5 = cg_entityOriginArray[localClientNum][p_nextState->number];
            *v5 = 131072.0f;
            v5[1] = 131072.0f;
            v5[2] = 131072.0f;
            FX_MarkEntUpdateEnd(&markUpdateContext, localClientNum, p_nextState->number, v4, 0, 0);
        }
    }
}

//cgs_t *__cdecl CG_GetLocalClientStaticGlobals(int localClientNum)
//{
//    if ( localClientNum
//        && !Assert_MyHandler(
//                    "c:\\projects_pc\\cod\\codsrc\\src\\cgame\\../cgame_mp/cg_local_mp.h",
//                    1843,
//                    0,
//                    "%s\n\t(localClientNum) = %i",
//                    "(localClientNum == 0)",
//                    localClientNum) )
//    {
//        __debugbreak();
//    }
//    return cgsArray;
//}

void __cdecl CG_ResetActorEntity(int localClientNum, cg_s *cgameGlob, centity_s *cent)
{
    actorInfo_t *ai; // [esp+8h] [ebp-10h]
    XAnimTree_s *pAnimTree; // [esp+14h] [ebp-4h]

    if ( cent->nextState.lerp.u.actor.actorNum >= MAX_ACTORS
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_actors_mp.cpp",
                    201,
                    0,
                    "es->lerp.u.actor.actorNum doesn't index MAX_ACTORS\n\t%i not in [0, %i)",
                    cent->nextState.lerp.u.actor.actorNum,
                    MAX_ACTORS) )
    {
        __debugbreak();
    }
    ai = BG_SP_GetActorInfo(&cgameGlob->bgs, cent->nextState.lerp.u.actor.actorNum);
    CG_GetLocalClientStaticGlobals(localClientNum);
    pAnimTree = ai->pXAnimTree;
    ai->dobjDirty = 1;
    if ( !pAnimTree
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_actors_mp.cpp", 209, 0, "%s", "pAnimTree") )
    {
        __debugbreak();
    }
    XAnimClearTreeGoalWeights(pAnimTree, 0, 0.0, -1);
    CG_UpdateActorDObj(localClientNum, cent, ai);
    CG_SP_ReplayEntityAnimCommands(localClientNum, cent->nextState.number);
}

void __cdecl CG_Actor_PreControllers(int localClientNum, centity_s *cent)
{
    cent->pose.player.nextWaterHeightCheck = 0;
    cent->pose.turret.barrelPitch = 0.0f;
    cent->pose.fx.triggerTime = 1;
    cent->pose.player.nextWaterHeightCheck = 0;
}

// zombies: actor client script / eye state transitions (SP 0x005812E4..0x005813C3).
// The same SP actor renderer handles living actors and actor corpses. Preserve the entityspawned
// callback before zombie_eye_callback. KB carries haseyes in eFlags2 bit 31 (SP eFlags bit 17),
// and caches it in padding bit 25 because MP already uses SP's cached bit 24 for bUpdateToggle.
static void CG_ActorScript(int localClientNum, centity_s *cent)
{
    if (CG_EntityNeedsScriptThread(localClientNum, cent))
    {
        if (zombiemode && zombiemode->current.enabled)
            cent->bZombieEyes = 0;
        cent->scriptThreaded = 1;
        Scr_AddInt(localClientNum, SCRIPTINSTANCE_CLIENT);
        unsigned short thread = CScr_ExecEntThread(cent, cg_scr_data.entityspawned, 1);
        Scr_FreeThread(thread, SCRIPTINSTANCE_CLIENT);
    }
    if (!zombiemode || !zombiemode->current.enabled)
        return;
    if (Actor_IsDogSpecies(cent->nextState.lerp.u.actor.species))
        return;
    const bool hasEyes = (cent->nextState.lerp.eFlags2 & SP_EFLAGS2_HAS_EYES) != 0;
    if (cent->bZombieEyes == hasEyes || !cg_scr_sp_data.zombieEyeCallback)
        return;
    cent->bZombieEyes = hasEyes;
    if (CG_SP_VisualsEnabled())
        Com_Printf(14, "BO1_VISUAL zombie_eyes time=%d client=%d ent=%d on=%d\n",
            CG_GetLocalClientGlobals(localClientNum)->time, localClientNum, cent->nextState.number, hasEyes);
    Scr_AddInt(hasEyes, SCRIPTINSTANCE_CLIENT);
    Scr_AddInt(localClientNum, SCRIPTINSTANCE_CLIENT);
    unsigned short thread = CScr_ExecEntThread(cent, cg_scr_sp_data.zombieEyeCallback, 2);
    Scr_FreeThread(thread, SCRIPTINSTANCE_CLIENT);
}

// zombies (a1): SP CG_PlayAIFootstep (SP 0x00685a70). An actor's footstep_left/right notetrack (the footstep arm of
// CG_ProcessClientNote, SP 0x00775780) runs clientscripts/_footsteps::playAIFootstep(client_num, ent, pos,
// ground_type, on_fire[, is_dog]) with the foot bone's ground point and surface. Five's _zombiemode.csc sets the
// prepend to "fly_step_zombie_", so the alias is fly_step_zombie_<surface> (its secondary chain adds fly_gear_zombie /
// fly_cloth_zombie). The MP sibling is CScr_PlayDogstepSound.
void __cdecl CG_SP_PlayAIFootstep(int localClientNum, centity_s *cent, int isLeft, bool isDog)
{
    unsigned __int16 t;
    int surfType;
    int argCount;
    DObj *obj;
    cg_s *cgameGlob;
    float dx, dy, dz;
    float start[3];
    float end[3];
    float footMatrix[4][3];

    if ( !cg_scr_sp_data.playAIFootstep )
        return;
    obj = Com_GetClientDObj(cent->nextState.number, localClientNum);
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    dz = cent->pose.origin[2] - cgameGlob->refdef.vieworg[2];
    dy = cent->pose.origin[1] - cgameGlob->refdef.vieworg[1];
    dx = cent->pose.origin[0] - cgameGlob->refdef.vieworg[0];
    // SP: the foot bone only inside 1024 u of the eye (DAT_00b84c14 = 1048576.0), else the entity origin
    if ( !obj
        || dz * dz + dy * dy + dx * dx >= 1048576.0f
        || !CG_DObjGetWorldTagMatrix(
                &cent->pose,
                obj,
                isLeft ? scr_const.j_ball_le : scr_const.j_ball_ri,
                footMatrix,
                footMatrix[3]) )
    {
        footMatrix[3][0] = cent->pose.origin[0];
        footMatrix[3][1] = cent->pose.origin[1];
        footMatrix[3][2] = cent->pose.origin[2];
    }
    // SP: +-15 (DAT_00b84984 / DAT_00b84c10), contentmask 0x2820011 (push at 0x00685b9d), surface mask 0x3f00000
    start[0] = footMatrix[3][0];
    start[1] = footMatrix[3][1];
    start[2] = footMatrix[3][2] + 15.0f;
    end[0] = footMatrix[3][0];
    end[1] = footMatrix[3][1];
    end[2] = footMatrix[3][2] - 15.0f;
    surfType = CM_TracePointDown(start, end, 0x2820011, 0x3F00000, footMatrix[3], 0, 0);
    argCount = 5;
    if ( isDog )
    {
        Scr_AddInt(1, SCRIPTINSTANCE_CLIENT);
        argCount = 6;
    }
    // on_fire: the same test as the MP sibling CScr_PlayDogstepSound (burning eFlag or cent isBurning)
    if ( (cent->currentState.eFlags2 & 0x200000) != 0 || cent->isBurning )
        Scr_AddInt(1, SCRIPTINSTANCE_CLIENT);
    else
        Scr_AddInt(0, SCRIPTINSTANCE_CLIENT);
    Scr_AddString((char *)Com_SurfaceTypeToName((surfType >> 20) & 0x3F), SCRIPTINSTANCE_CLIENT);
    Scr_AddVector(footMatrix[3], SCRIPTINSTANCE_CLIENT);
    CScr_AddEntity(cent, localClientNum);
    Scr_AddInt(localClientNum, SCRIPTINSTANCE_CLIENT);
    t = Scr_ExecThread(SCRIPTINSTANCE_CLIENT, cg_scr_sp_data.playAIFootstep, argCount);
    Scr_FreeThread(t, SCRIPTINSTANCE_CLIENT);
}

// zombies (a1): SP 0x004ee890, called once per actor per frame from CG_Actor (SP 0x00777dd0). The foot bits the
// footstep arm set are played left first, only while the actor is inside footstep_sounds_cutoff of the eye; outside
// it they stay set. The foot decal (SP 0x005a9b60, the cg_footprints dvar) is not ported.
void __cdecl CG_SP_DoAIFootsteps(int localClientNum, centity_s *cent)
{
    cg_s *cgameGlob;
    float dx, dy, dz;
    bool isDog;

    if ( !cent->nextValid || (!cent->leftFootstep && !cent->rightFootstep) )
        return;
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    dx = cent->pose.origin[0] - cgameGlob->refdef.vieworg[0];
    dz = cent->pose.origin[2] - cgameGlob->refdef.vieworg[2];
    dy = cent->pose.origin[1] - cgameGlob->refdef.vieworg[1];
    if ( dx * dx + dz * dz + dy * dy > footstep_sounds_cutoff->current.value * footstep_sounds_cutoff->current.value )
        return;
    isDog = Actor_IsDogSpecies(cent->nextState.lerp.u.actor.species);
    if ( cent->leftFootstep )
    {
        CG_SP_PlayAIFootstep(localClientNum, cent, 1, isDog);
        cent->leftFootstep = 0;
    }
    if ( cent->rightFootstep )
    {
        CG_SP_PlayAIFootstep(localClientNum, cent, 0, isDog);
        cent->rightFootstep = 0;
    }
}

void __cdecl CG_Actor(int localClientNum, centity_s *cent)
{
    unsigned int renderFxFlags; // [esp+2Ch] [ebp-5Ch]
    const DObj *obj; // [esp+34h] [ebp-54h]
    entityState_s *s1; // [esp+38h] [ebp-50h]
    float mins[3]; // [esp+40h] [ebp-48h] BYREF
    const cgs_t *cgs; // [esp+4Ch] [ebp-3Ch]
    float bounds[2][3]; // [esp+50h] [ebp-38h] BYREF
    float maxs[3]; // [esp+68h] [ebp-20h] BYREF
    float lightingOrigin[3]; // [esp+74h] [ebp-14h] BYREF
    actorInfo_t *actorInfo; // [esp+84h] [ebp-4h]

    s1 = &cent->nextState;
    if ( (cent->nextState.lerp.eFlags & 0x20) == 0 )
    {
        cgs = CG_GetLocalClientStaticGlobals(localClientNum);
        obj = Com_GetClientDObj(s1->number, localClientNum);
        if ( obj )
        {
            CG_ActorScript(localClientNum, cent);
            actorInfo = BG_SP_GetActorInfo(&CG_GetLocalClientGlobals(localClientNum)->bgs, cent->nextState.lerp.u.actor.actorNum);
            // zombies: the client tree follows the server (CG_SP_ActorUseServerAnims), no dog animation state.
            if ( !zombiemode || !zombiemode->current.enabled )
                BG_Dog_UpdateAnimationState(localClientNum, &cent->nextState, actorInfo);
            CG_Actor_PreControllers(localClientNum, cent);
            lightingOrigin[0] = cent->pose.origin[0];
            lightingOrigin[1] = cent->pose.origin[1];
            lightingOrigin[2] = cent->pose.origin[2] + 32.0;
            if (cent->pose.isRagdoll && cent->pose.ragdollHandle > 0)
            {
                Ragdoll_GetRootOrigin(cent->pose.ragdollHandle, cent->pose.origin);
            }
            // zombies (a1): SP CG_Actor drains the SP footstep bits (SP 0x004ee890); MP's dogstep drain otherwise
            if ( zombiemode && zombiemode->current.enabled )
                CG_SP_DoAIFootsteps(localClientNum, cent);
            else
                CG_DoFootsteps(localClientNum, cent);
            CG_GetEntityDobjBounds(cent, obj, mins, maxs, bounds[0], bounds[1]);
            if ( !R_CullBoxCurDpvs(bounds[0], localClientNum) )
            {
                CG_HighlightPlayer(localClientNum, cent, &cent->pose.constantSet, 0);
                renderFxFlags = 4194308;
                if ( CG_IsInfrared(localClientNum) )
                    renderFxFlags = 4194436;
                R_AddDObjToScene(
                    obj,
                    &cent->pose,
                    s1->number,
                    renderFxFlags,
                    lightingOrigin,
                    0.0,
                    0.0,
                    0.0,
                    0.0,
                    -1,
                    -1,
                    &cent->pose.constantSet,
                    0,
                    0.0,
                    1.0);
            }
            if ( cent->nextState.eType == 17 )
            {
                if ( CL_LocalClient_IsFirstActive(localClientNum) )
                    CG_DoTouchTriggers(cent, localClientNum);
            }
        }
    }
}

bool __cdecl CG_EntityNeedsScriptThread(int localClientNum, centity_s *cent)
{
    if ( !cg_loadScripts || !cg_loadScripts->current.enabled )
        return 0;
    if ( !CL_LocalClient_IsFirstActive(localClientNum) )
        return 0;
    if ( cent )
        return ((*((unsigned int *)cent + 201) >> 8) & 1) == 0;
    return 0;
}

void __cdecl CG_ActorCorpse(int localClientNum, centity_s *cent)
{
    actorInfo_t *ai; // [esp+2Ch] [ebp-20h]
    const DObj *obj; // [esp+30h] [ebp-1Ch]
    entityState_s *p_nextState; // [esp+38h] [ebp-14h]
    unsigned int corpseIndex; // [esp+3Ch] [ebp-10h]
    float lightingOrigin[3]; // [esp+40h] [ebp-Ch] BYREF

    p_nextState = &cent->nextState;
    if ( (cent->nextState.lerp.eFlags & 0x40000) == 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_actors_mp.cpp",
                    338,
                    0,
                    "%s",
                    "es->lerp.eFlags & EF_DEAD") )
    {
        __debugbreak();
    }
    // zombies: SP CG_Actor (SP 0x00581260) draws a corpse only while its skin-cache latch (cent+0x318) is clear, and
    // latches it (no draw, until CG_ShutdownEntity) once the skinned-vertex cache passed 0x6AACC0 or ran out
    // (g_skinErrorFlags & 3, set by R_AllocSkinnedCachedVerts); MP has no such check.
    const bool spCorpseLatch = zombiemode && zombiemode->current.enabled;
    if ( (cent->nextState.lerp.eFlags & 0x20) == 0 && !(spCorpseLatch && cent->bSkinCacheHidden) )
    {
        if ( spCorpseLatch && (g_skinErrorFlags.allbits & 3) != 0 )
        {
            cent->bSkinCacheHidden = 1;
            return;
        }
        // zombies: SP numbers the corpse by lerp.u.actor.corpseNum and draws it with the DObj its actor entity already
        // had (G_CorpseFromActor 0x00642B80 keeps the entity; SP CG_Actor 0x00581260 updates no DObj for it, and the
        // actor's client tree moved to this corpse slot at the snapshot transition, 0x00639E20). MP builds its clone's
        // DObj from the corpse info (entities 36..).
        corpseIndex = spCorpseLatch ? p_nextState->lerp.u.actor.corpseNum : p_nextState->number - 36;
        if ( corpseIndex >= Com_GetMaxActorCorpses() // zombies: SP level 32, MP 8
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_actors_mp.cpp",
                        345,
                        0,
                        "%s",
                        "(unsigned)corpseIndex < MAX_ACTOR_CORPSES") )
        {
            __debugbreak();
        }
        ai = &CG_GetLocalClientStaticGlobals(localClientNum)->actorCorpseInfo[corpseIndex];
        if ( !spCorpseLatch )
            CG_UpdateActorDObj(localClientNum, cent, ai);
        obj = Com_GetClientDObj(p_nextState->number, localClientNum);
        if ( obj )
        {
            if ( zombiemode && zombiemode->current.enabled )
                CG_ActorScript(localClientNum, cent);
            if ( !zombiemode || !zombiemode->current.enabled ) // zombies: see CG_Actor
                BG_Dog_UpdateAnimationState(localClientNum, &cent->nextState, ai);
            lightingOrigin[0] = cent->pose.origin[0];
            lightingOrigin[1] = cent->pose.origin[1];
            lightingOrigin[2] = cent->pose.origin[2];
            if ( (cent->nextState.lerp.eFlags & 8) != 0 )
            {
                lightingOrigin[2] = lightingOrigin[2] + 12.0;
            }
            else if ( (cent->nextState.lerp.eFlags & 4) != 0 )
            {
                lightingOrigin[2] = lightingOrigin[2] + 20.0;
            }
            else
            {
                lightingOrigin[2] = lightingOrigin[2] + 32.0;
            }
            CG_HighlightPlayer(localClientNum, cent, &cent->pose.constantSet, 0);
            R_AddDObjToScene(
                obj,
                &cent->pose,
                p_nextState->number,
                0x400000u,
                lightingOrigin,
                0.0,
                0.0,
                0.0,
                0.0,
                -1,
                -1,
                &cent->pose.constantSet,
                0,
                0.0,
                1.0);
        }
    }
}

