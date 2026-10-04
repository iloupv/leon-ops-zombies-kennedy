#include "actor_negotiation.h"
#include <game_sp/actor_sp_ext.h>
#include "actor_navigation.h"
#include "actor_orientation.h"
#include <clientscript/cscr_stringlist.h>
#include "actor_animapi.h"
#include <game_mp/actor_mp.h>
#include <game_mp/g_main_mp.h>
#include <bgame/bg_dog.h>
#include "actor_state.h"

bool __fastcall Actor_Negotiation_Start(actor_s *pSelf, ai_state_t ePrevState)
{
    if ( !Path_HasNegotiationNode(&pSelf->Path) )
        return 0;
    Actor_SetOrientMode(pSelf, AI_ORIENT_DONT_CHANGE);
    pSelf->eTraverseMode = AI_TRAVERSE_NOGRAVITY;
    pSelf->prevMoveDir[0] = 0.0f;
    pSelf->prevMoveDir[1] = 0.0f;
    return 1;
}

actor_think_result_t __fastcall Actor_Negotiation_Think(actor_s *pSelf)
{
    const char *v1; // eax
    char *v2; // eax
    const char *v3; // eax
    ai_animmode_t eAnimMode; // [esp+34h] [ebp-4h]

    PROF_SCOPED("negotiationthink");

    eAnimMode = AI_ANIM_USE_BOTH_DELTAS_NOGRAVITY;
    if ( pSelf->eTraverseMode == AI_TRAVERSE_GRAVITY )
    {
        eAnimMode = AI_ANIM_USE_BOTH_DELTAS;
    }
    else if ( pSelf->eTraverseMode == AI_TRAVERSE_NOCLIP )
    {
        eAnimMode = AI_ANIM_USE_BOTH_DELTAS_NOCLIP;
    }
    else if ( pSelf->eTraverseMode != AI_TRAVERSE_NOGRAVITY )
    {
        v1 = va("pSelf->eTraverseMode is unhandled %i", pSelf->eTraverseMode);
        if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_negotiation.cpp", 55, 0, v1) )
            __debugbreak();
    }
    if ( !pSelf->AnimScriptSpecific.func )
    {
        v2 = SL_ConvertToString(pSelf->AnimScriptSpecific.name, SCRIPTINSTANCE_SERVER);
        v3 = va("Invalid negotiation script %s", v2);
        if ( !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_negotiation.cpp",
                        58,
                        0,
                        "%s\n\t%s",
                        "pSelf->AnimScriptSpecific.func",
                        v3) )
            __debugbreak();
    }
    Actor_AnimSpecific(pSelf, &pSelf->AnimScriptSpecific, eAnimMode, 1);
    pSelf->pushable = 0;
    if ( Actor_IsAnimScriptAlive(pSelf) )
    {
        pSelf->pszDebugInfo = "negotiation";
        Actor_PreThink(pSelf);
        Actor_UpdateOriginAndAngles(pSelf);
        // zombies: Actor_Negotiation_Think has no dog animation updater (SP 0x005189a0).
        if ( !zombiemode->current.enabled )
            BG_Dog_UpdateAnimationState(-1, &pSelf->ent->s, BG_SP_GetActorInfo(&level_bgs, pSelf - level.actors));
        return ACTOR_THINK_DONE;
    }
    else
    {
        if ( Path_HasNegotiationNode(&pSelf->Path) )
        {
            if ( pSelf->Path.wNegotiationStartNode >= pSelf->Path.wPathLen
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_negotiation.cpp",
                            68,
                            0,
                            "%s",
                            "pSelf->Path.wNegotiationStartNode < pSelf->Path.wPathLen") )
            {
                __debugbreak();
            }
            // BO1Zombies decompilation bug: the pointer-arithmetic form (&Physics.proximity_data.prims[199].tree
            // + 7 * n) sits 8 bytes before pts[n-1].iNodeNum in KB's actor_s (it read pts[n-1].fOrigLength).
            // SP 0x005189a0 reads [actor + 0x15B8 + 28 * n] = Path(0x15BC).pts[n - 1].iNodeNum, as the assert says.
            if ( pSelf->Path.pts[pSelf->Path.wNegotiationStartNode - 1].iNodeNum < 0
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_negotiation.cpp",
                            69,
                            0,
                            "%s",
                            "pSelf->Path.pts[pSelf->Path.wNegotiationStartNode - 1].iNodeNum >= 0") )
            {
                __debugbreak();
            }
            pSelf->sentient->pNearestNode
                = Path_ConvertIndexToNode(pSelf->Path.pts[pSelf->Path.wNegotiationStartNode - 1].iNodeNum);
            pSelf->sentient->bNearestNodeBad = 0;
            pSelf->sentient->bNearestNodeValid = 1;
        }
        pSelf->Physics.vWishDelta[0] = 0.0f;
        pSelf->Physics.vWishDelta[1] = 0.0f;
        pSelf->Physics.vWishDelta[2] = 0.0f;
        pSelf->Physics.vVelocity[0] = 0.0f;
        pSelf->Physics.vVelocity[1] = 0.0f;
        pSelf->Physics.vVelocity[2] = 0.0f;
        YawVectors2D(pSelf->fDesiredBodyYaw, pSelf->prevMoveDir, 0);
        Actor_RecalcPath(pSelf);
        Actor_PopState(pSelf);
        return ACTOR_THINK_REPEAT;
    }
}

