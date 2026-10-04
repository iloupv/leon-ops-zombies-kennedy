#include "bg_dog.h"
#include <clientscript/cscr_stringlist.h>
#include <universal/q_shared.h>
#include <universal/assertive.h>
#include <clientscript/scr_const.h>
#include <qcommon/ent.h>
#include <xanim/xanim.h>
#include "bg_local.h"
#include "bg_misc.h"
#include "bg_dog_animations_mp.h"
#include <qcommon/ent.h>

const dvar_s *dog_debug;
const dvar_s *dog_turn90_angle;
const dvar_s *dog_turn180_angle;
const dvar_s *dog_turn_min_goal_dist;

struct AnimStateName // sizeof=0x8
{                                                                             // XREF: .data:s_animStateNames/r
        unsigned __int16 *name;                         // XREF: BG_Actor_GetAnimStateIndex(ushort)+26/r
                                                                                // BG_Actor_GetAnimStateName(int)+3D/r
        int category;                                             // XREF: BG_Actor_GetAnimStateCategoryName(int)+40/r
};

// aislop helped generate this
AnimStateName s_animStateNames[47] =
{
        { &scr_const.idle,                                                             0 },

        { &scr_const.move_run,                                                     2 },
        { &scr_const.move_start,                                                 2 },
        { &scr_const.move_stop,                                                    2 },
        { &scr_const.move_walk,                                                    2 },
        { &scr_const.move_turn_left,                                         2 },
        { &scr_const.move_turn_right,                                        2 },
        { &scr_const.move_run_turn_left,                                 2 },
        { &scr_const.move_run_turn_right,                                2 },
        { &scr_const.move_turn_around_left,                            2 },
        { &scr_const.move_turn_around_right,                         2 },
        { &scr_const.move_run_turn_around_left,                    2 },
        { &scr_const.move_run_turn_around_right,                 2 },
        { &scr_const.move_attack,                                                2 },

        { &scr_const.stop_idle,                                                    1 },
        { &scr_const.stop_attackidle,                                        1 },
        { &scr_const.stop_attackidle_bark,                             1 },
        { &scr_const.stop_attackidle_growl,                            1 },

        { &scr_const.pain_main,                                                    4 },
        { &scr_const.pain_front,                                                 4 },
        { &scr_const.pain_back,                                                    4 },
        { &scr_const.pain_left,                                                    4 },
        { &scr_const.pain_right,                                                 4 },
        { &scr_const.pain_run_front,                                         4 },
        { &scr_const.pain_run_back,                                            4 },
        { &scr_const.pain_run_left,                                            4 },
        { &scr_const.pain_run_right,                                         4 },

        { &scr_const.death_front,                                                5 },
        { &scr_const.death_back,                                                 5 },
        { &scr_const.death_left,                                                 5 },
        { &scr_const.death_right,                                                5 },

        { &scr_const.flashed,                                                        6 },

        { &scr_const.combat_attackidle,                                    3 },
        { &scr_const.combat_attackidle_bark,                         3 },
        { &scr_const.combat_attackidle_growl,                        3 },
        { &scr_const.combat_attack_run,                                    3 },
        { &scr_const.combat_attack_player_close_range,     3 },
        { &scr_const.combat_attack_miss,                                 3 },
        { &scr_const.combat_attack_miss_left,                        3 },
        { &scr_const.combat_attack_miss_right,                     3 },

        { &scr_const.traverse_jump_up_40,                                7 },
        { &scr_const.traverse_jump_up_80,                                7 },
        { &scr_const.traverse_jump_down_40,                            7 },
        { &scr_const.traverse_jump_down_80,                            7 },
        { &scr_const.traverse_wallhop,                                     7 },
        { &scr_const.traverse_window,                                        7 },
        { &scr_const.traverse_through_hole_42,                     7 }
};

unsigned __int16 *s_animStateCategories[8] =
{
        &scr_const.init,                // 0x3F392C0
        &scr_const.stop,                // 0x3F3926A
        &scr_const.move,                // 0x3F392C2
        &scr_const.combat,            // 0x3F392C4
        &scr_const.pain,                // 0x3F3925A
        &scr_const.death,             // 0x3F38F6C
        &scr_const.flashed,         // 0x3F39306
        &scr_const.traverse         // 0x3F392C6
};

const ai_animation_funcs_t AIAnimationFuncTable[47] =
{
    { NULL, NULL, NULL, NULL },
    { BG_Dog_Move_Run, BG_Dog_Move_Run_Think, NULL, NULL },
    { BG_Dog_Move_Start, NULL, NULL, NULL },
    { BG_Dog_Move_Stop, NULL, NULL, NULL },
    { BG_Dog_Move_Walk, BG_Dog_Move_Walk_Think, NULL, NULL },
    { BG_Dog_Move_Turn_Left, NULL, NULL, NULL },
    { BG_Dog_Move_Turn_Right, NULL, NULL, NULL },
    { BG_Dog_Move_Run_Turn_Left, NULL, NULL, NULL },
    { BG_Dog_Move_Run_Turn_Right, NULL, NULL, NULL },
    { BG_Dog_Move_Turn_Around_Left, NULL, NULL, NULL },
    { BG_Dog_Move_Turn_Around_Right, NULL, NULL, NULL },
    { BG_Dog_Move_Run_Turn_Around_Left, NULL, NULL, NULL },
    { BG_Dog_Move_Run_Turn_Around_Right, NULL, NULL, NULL },
    { NULL, NULL, NULL, NULL },
    { BG_Dog_Stop_Idle, BG_Dog_Look_At, NULL, NULL },
    { BG_Dog_Stop_AttackIdle, BG_Dog_Look_At, NULL, NULL },
    { BG_Dog_Stop_AttackIdle_Bark, BG_Dog_Look_At, NULL, NULL },
    { BG_Dog_Stop_AttackIdle_Growl, BG_Dog_Look_At, NULL, NULL },
    { BG_Dog_Pain_Main, NULL, NULL, NULL },
    { BG_Dog_Pain_Front, NULL, NULL, NULL },
    { BG_Dog_Pain_Back, NULL, NULL, NULL },
    { BG_Dog_Pain_Left, NULL, NULL, NULL },
    { BG_Dog_Pain_Right, NULL, NULL, NULL },
    { BG_Dog_Run_Pain_Front, NULL, NULL, NULL },
    { BG_Dog_Run_Pain_Back, NULL, NULL, NULL },
    { BG_Dog_Run_Pain_Left, NULL, NULL, NULL },
    { BG_Dog_Run_Pain_Right, NULL, NULL, NULL },
    { BG_Dog_Death_Front, NULL, NULL, BG_Dog_Death_Fast_Forward_Front },
    { BG_Dog_Death_Back, NULL, NULL, BG_Dog_Death_Fast_Forward_Back },
    { BG_Dog_Death_Left, NULL, NULL, BG_Dog_Death_Fast_Forward_Left },
    { BG_Dog_Death_Right, NULL, NULL, BG_Dog_Death_Fast_Forward_Right },
    { BG_Dog_Flashed, NULL, NULL, NULL },
    { BG_Dog_Combat_AttackIdle, NULL, NULL, NULL },
    { BG_Dog_Combat_AttackIdle_Bark, NULL, NULL, NULL },
    { BG_Dog_Combat_AttackIdle_Growl, NULL, NULL, NULL },
    { BG_Dog_Combat_Attack_Run, NULL, NULL, NULL },
    { BG_Dog_Combat_Attack_Player_Close_Range, NULL, NULL, NULL },
    { BG_Dog_Combat_Attack_Miss, BG_Dog_Look_At, NULL, NULL },
    { BG_Dog_Combat_Attack_Miss_Left, NULL, NULL, NULL },
    { BG_Dog_Combat_Attack_Miss_Right, NULL, NULL, NULL },
    { BG_Dog_Traverse_Jump_Up_40, NULL, NULL, NULL },
    { BG_Dog_Traverse_Jump_Up_80, NULL, NULL, NULL },
    { BG_Dog_Traverse_Jump_Down_40, NULL, NULL, NULL },
    { BG_Dog_Traverse_Jump_Down_80, NULL, NULL, NULL },
    { BG_Dog_Traverse_Wallhop, NULL, NULL, NULL },
    { BG_Dog_Traverse_Window, NULL, NULL, NULL },
    { BG_Dog_Traverse_Through_Hole_42, NULL, NULL, NULL }
};



int __cdecl BG_Actor_GetAnimStateIndex(unsigned __int16 animStateName)
{
    char *v2; // eax
    const char *v3; // eax
    int i; // [esp+0h] [ebp-4h]

    for ( i = 0; i < 47; ++i )
    {
        if ( *s_animStateNames[i].name == animStateName )
            return i;
    }
    if ( i != 47 )
    {
        v2 = SL_ConvertToString(animStateName, SCRIPTINSTANCE_SERVER);
        v3 = va("Actor_GetAnimStateIndex: Could not find anim state %s", v2);
        if ( !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_dog.cpp",
                        104,
                        0,
                        "%s\n\t%s",
                        "i == ACTOR_ANIMATION_COUNT",
                        v3) )
            __debugbreak();
    }
    return 0;
}

char *__cdecl BG_Actor_GetAnimStateName(int animStateIndex)
{
    if ( (animStateIndex & 0xFFFFFFBF) >= 0x2F
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_dog.cpp",
                    125,
                    0,
                    "animStateIndex & ~ANIM_STATE_TOGGLEBIT doesn't index ARRAY_COUNT( s_animStateNames )\n\t%i not in [0, %i)",
                    animStateIndex & 0xFFFFFFBF,
                    47) )
    {
        __debugbreak();
    }
    return SL_ConvertToString(*s_animStateNames[animStateIndex & 0xFFFFFFBF].name, SCRIPTINSTANCE_SERVER);
}

char *__cdecl BG_Actor_GetAnimStateCategoryName(int animStateIndex)
{
    if ( (animStateIndex & 0xFFFFFFBF) >= 0x2F
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_dog.cpp",
                    132,
                    0,
                    "animStateIndex & ~ANIM_STATE_TOGGLEBIT doesn't index ARRAY_COUNT( s_animStateNames )\n\t%i not in [0, %i)",
                    animStateIndex & 0xFFFFFFBF,
                    47) )
    {
        __debugbreak();
    }
    return SL_ConvertToString(
                     *s_animStateCategories[s_animStateNames[animStateIndex & 0xFFFFFFBF].category],
                     SCRIPTINSTANCE_SERVER);
}

void __cdecl Dog_RegisterDvars()
{
    dog_debug = _Dvar_RegisterBool("dog_debug", 1, 0x180u, "MP dog debugging");
    dog_turn90_angle = _Dvar_RegisterFloat(
                                             "dog_turn90_angle",
                                             70.0,
                                             0.0,
                                             360.0,
                                             0x80u,
                                             "If the dog needs to turn at this angle or greater, he'll execute the 90-degree turn animation");
    dog_turn180_angle = _Dvar_RegisterFloat(
                                                "dog_turn180_angle",
                                                135.0,
                                                0.0,
                                                360.0,
                                                0x80u,
                                                "If the dog needs to turn at this angle or greater, he'll execute the 180-degree turn animation");
    dog_turn_min_goal_dist = _Dvar_RegisterFloat(
                                                         "dog_turn_min_goal_dist",
                                                         40.0,
                                                         0.0,
                                                         1024.0,
                                                         0x80u,
                                                         "If the dog is at this distance or less to it's goal, it won't execute any turn animations");
}

void __cdecl BG_Dog_UpdateAnimationState(int localClientNum, const entityState_s *es, actorInfo_t *actorInfo)
{
    animInfo_t *lf; // [esp+8h] [ebp-Ch]
    int newAnimationState; // [esp+10h] [ebp-4h]

    lf = &actorInfo->animInfo;
    newAnimationState = es->animState.state;
    if (ApplyNewAnimState(actorInfo->pXAnimTree, &actorInfo->animInfo.lerp, newAnimationState))
    {
        BG_Dog_SetNewAnimationState(localClientNum, &actorInfo->animInfo, &lf->lerp, newAnimationState, es);
    }
    else if (lf->lerp.bAnimating && bgs->GetDObj(es->number, localClientNum))
    {
        if (AIAnimationFuncTable[newAnimationState & 0xFFFFFFBF].pfnThink)
            AIAnimationFuncTable[newAnimationState & 0xFFFFFFBF].pfnThink(localClientNum, es);
    }
}

void __cdecl BG_Dog_SetNewAnimationState(
                int localClientNum,
                animInfo_t *animInfo,
                actorLerpFrame_t *lf,
                int newAnimationState,
                const entityState_s *es)
{
    BG_CheckThread();
    iassert(bgs);
    if ( AIAnimationFuncTable[newAnimationState & 0xFFFFFFBF].pfnStart )
    {
        AIAnimationFuncTable[newAnimationState & 0xFFFFFFBF].pfnStart(
            localClientNum,
            es,
            (ActorAnimStates)(lf->animationState & 0xFFFFFFBF));
        lf->bAnimating = 1;
        lf->animationState = newAnimationState;
    }
}

bool __cdecl ApplyNewAnimState(XAnimTree_s *pAnimTree, actorLerpFrame_t *lf, int newAnimationState)
{
    if ( newAnimationState != lf->animationState )
        return 1;
    if ( (newAnimationState & 0xFFFFFFBF) == 0 )
        return 0;
    if ( !lf->bAnimating )
        return 1;
    return pAnimTree && !XAnimTreeHasInfo(pAnimTree);
}

void __cdecl BG_Dog_SetAnimState(entityState_s *es, int animState)
{
    es->animState.state = animState | es->animState.state & 0x40 ^ 0x40;
}

