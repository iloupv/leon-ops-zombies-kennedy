#pragma once

struct XAnim_s;
struct entityState_s;
struct actorInfo_t;
struct DObj;

enum ActorAnimStates : __int32
{                                                                             // XREF: BG_Dog_Move_Start/r
                                                                                // BG_Dog_Move_Stop/r ...
    ACTOR_ANIMATION_IDLE = 0x0,
    ACTOR_ANIMATION_MOVE_RUN = 0x1,
    ACTOR_ANIMATION_MOVE_START = 0x2,
    ACTOR_ANIMATION_MOVE_STOP = 0x3,
    ACTOR_ANIMATION_MOVE_WALK = 0x4,
    ACTOR_ANIMATION_MOVE_TURN_LEFT = 0x5,
    ACTOR_ANIMATION_MOVE_TURN_RIGHT = 0x6,
    ACTOR_ANIMATION_MOVE_RUN_TURN_LEFT = 0x7,
    ACTOR_ANIMATION_MOVE_RUN_TURN_RIGHT = 0x8,
    ACTOR_ANIMATION_MOVE_TURN_AROUND_LEFT = 0x9,
    ACTOR_ANIMATION_MOVE_TURN_AROUND_RIGHT = 0xA,
    ACTOR_ANIMATION_MOVE_RUN_TURN_AROUND_LEFT = 0xB,
    ACTOR_ANIMATION_MOVE_RUN_TURN_AROUND_RIGHT = 0xC,
    ACTOR_ANIMATION_ATTACK = 0xD,
    ACTOR_ANIMATION_STOP_IDLE = 0xE,
    ACTOR_ANIMATION_STOP_ATTACKIDLE = 0xF,
    ACTOR_ANIMATION_STOP_ATTACKIDLE_BARK = 0x10,
    ACTOR_ANIMATION_STOP_ATTACKIDLE_GROWL = 0x11,
    ACTOR_ANIMATION_PAIN_MAIN = 0x12,
    ACTOR_ANIMATION_PAIN_FRONT = 0x13,
    ACTOR_ANIMATION_PAIN_BACK = 0x14,
    ACTOR_ANIMATION_PAIN_LEFT = 0x15,
    ACTOR_ANIMATION_PAIN_RIGHT = 0x16,
    ACTOR_ANIMATION_RUN_PAIN_FRONT = 0x17,
    ACTOR_ANIMATION_RUN_PAIN_BACK = 0x18,
    ACTOR_ANIMATION_RUN_PAIN_LEFT = 0x19,
    ACTOR_ANIMATION_RUN_PAIN_RIGHT = 0x1A,
    ACTOR_ANIMATION_DEATH_FRONT = 0x1B,
    ACTOR_ANIMATION_DEATH_BACK = 0x1C,
    ACTOR_ANIMATION_DEATH_LEFT = 0x1D,
    ACTOR_ANIMATION_DEATH_RIGHT = 0x1E,
    ACTOR_ANIMATION_FLASHED = 0x1F,
    ACTOR_ANIMATION_ATTACK_ATTACKIDLE = 0x20,
    ACTOR_ANIMATION_ATTACK_ATTACKIDLE_BARK = 0x21,
    ACTOR_ANIMATION_ATTACK_ATTACKIDLE_GROWL = 0x22,
    ACTOR_ANIMATION_ATTACK_RUN = 0x23,
    ACTOR_ANIMATION_ATTACK_PLAYER_CLOSE_RANGE = 0x24,
    ACTOR_ANIMATION_ATTACK_MISS = 0x25,
    ACTOR_ANIMATION_ATTACK_MISS_LEFT = 0x26,
    ACTOR_ANIMATION_ATTACK_MISS_RIGHT = 0x27,
    ACTOR_ANIMATION_TRAVERSE_JUMP_UP_40 = 0x28,
    ACTOR_ANIMATION_TRAVERSE_JUMP_UP_80 = 0x29,
    ACTOR_ANIMATION_TRAVERSE_JUMP_DOWN_40 = 0x2A,
    ACTOR_ANIMATION_TRAVERSE_JUMP_DOWN_80 = 0x2B,
    ACTOR_ANIMATION_TRAVERSE_WALLHOP = 0x2C,
    ACTOR_ANIMATION_TRAVERSE_WINDOW = 0x2D,
    ACTOR_ANIMATION_TRAVERSE_THROUGH_HOLE_42 = 0x2E,
    ACTOR_ANIMATION_COUNT = 0x2F,
};

XAnim_s *__cdecl Dog_GetAnims();
void __cdecl Dog_CreateAnims(void *(__cdecl *xanimAlloc)(unsigned int));
void __cdecl Dog_DebugPrint(const char *msg);
void __cdecl BG_Dog_CreateAnimChildGroup(
                unsigned int index,
                unsigned int childindex,
                const char **animNames,
                int count,
                unsigned int flags);
void __cdecl Dog_ShutdownAnims();
void __cdecl BG_Actor_FastForwardAnimState(int localClientNum, const entityState_s *es, actorInfo_t *actorInfo);

void __cdecl BG_Dog_Look_At(int localClientNum, const entityState_s *es);
void __cdecl BG_Dog_Move_Start(int localClientNum, const entityState_s *es, ActorAnimStates prevState);
void __cdecl BG_Dog_Move_Stop(int localClientNum, const entityState_s *es, ActorAnimStates prevState);
void __cdecl BG_Dog_Move_Run(int localClientNum, const entityState_s *es, ActorAnimStates prevState);
void __cdecl BG_Dog_GetRunAnimWeights(float leanAmount, float *center, float *left, float *right);
void __cdecl BG_Dog_Init_Normal_Look_At(int localClientNum, const entityState_s *es);
void __cdecl BG_Dog_Clear_Look_At(int localClientNum, const entityState_s *es);
void __cdecl BG_Dog_Move_Run_Think(int localClientNum, const entityState_s *es);
void __cdecl BG_Dog_Move_Walk(int localClientNum, const entityState_s *es, ActorAnimStates prevState);
void __cdecl BG_Dog_Move_Walk_Think(int localClientNum, const entityState_s *es);
void __cdecl BG_Dog_Move_Turn_Left(int localClientNum, const entityState_s *es, ActorAnimStates prevState);
void __cdecl BG_Dog_Clear_All_Turns(DObj *obj);
void __cdecl BG_Dog_Move_Turn_Right(int localClientNum, const entityState_s *es, ActorAnimStates prevState);
void __cdecl BG_Dog_Move_Run_Turn_Left(int localClientNum, const entityState_s *es, ActorAnimStates prevState);
void __cdecl BG_Dog_Move_Run_Turn_Right(int localClientNum, const entityState_s *es, ActorAnimStates prevState);
void __cdecl BG_Dog_Move_Turn_Around_Left(int localClientNum, const entityState_s *es, ActorAnimStates prevState);
void __cdecl BG_Dog_Move_Turn_Around_Right(int localClientNum, const entityState_s *es, ActorAnimStates prevState);
void __cdecl BG_Dog_Move_Run_Turn_Around_Left(int localClientNum, const entityState_s *es, ActorAnimStates prevState);
void __cdecl BG_Dog_Move_Run_Turn_Around_Right(int localClientNum, const entityState_s *es, ActorAnimStates prevState);
void __cdecl BG_Dog_Stop_Idle(int localClientNum, const entityState_s *es, ActorAnimStates prevState);
void __cdecl BG_Dog_Stop_Init(int localClientNum, const entityState_s *es);
void __cdecl BG_Dog_Init_Attack_Look_At(int localClientNum, const entityState_s *es);
void __cdecl BG_Dog_Stop_AttackIdle(int localClientNum, const entityState_s *es, ActorAnimStates prevState);
void __cdecl BG_Dog_Stop_AttackIdle_Bark(int localClientNum, const entityState_s *es, ActorAnimStates prevState);
void __cdecl BG_Dog_Stop_AttackIdle_Growl(int localClientNum, const entityState_s *es, ActorAnimStates prevState);
void __cdecl BG_Dog_Pain_Main(int localClientNum, const entityState_s *es, ActorAnimStates prevState);
void __cdecl BG_Dog_Pain_Front(int localClientNum, const entityState_s *es, ActorAnimStates prevState);
void __cdecl BG_Dog_Pain_Back(int localClientNum, const entityState_s *es, ActorAnimStates prevState);
void __cdecl BG_Dog_Pain_Left(int localClientNum, const entityState_s *es, ActorAnimStates prevState);
void __cdecl BG_Dog_Pain_Right(int localClientNum, const entityState_s *es, ActorAnimStates prevState);
void __cdecl BG_Dog_Run_Pain_Front(int localClientNum, const entityState_s *es, ActorAnimStates prevState);
void __cdecl BG_Dog_Run_Pain_Back(int localClientNum, const entityState_s *es, ActorAnimStates prevState);
void __cdecl BG_Dog_Run_Pain_Left(int localClientNum, const entityState_s *es, ActorAnimStates prevState);
void __cdecl BG_Dog_Run_Pain_Right(int localClientNum, const entityState_s *es, ActorAnimStates prevState);
void __cdecl BG_Dog_Death_Front(int localClientNum, const entityState_s *es, ActorAnimStates prevState);
void __cdecl BG_Dog_Death_Back(int localClientNum, const entityState_s *es, ActorAnimStates prevState);
void __cdecl BG_Dog_Death_Left(int localClientNum, const entityState_s *es, ActorAnimStates prevState);
void __cdecl BG_Dog_Death_Right(int localClientNum, const entityState_s *es, ActorAnimStates prevState);
void __cdecl BG_Dog_Death_Fast_Forward_Front(int localClientNum, const entityState_s *es, actorInfo_t *actorInfo);
void __cdecl BG_Dog_FastForwardAnim(unsigned int anim, actorInfo_t *actorInfo);
void __cdecl BG_Dog_Death_Fast_Forward_Back(int localClientNum, const entityState_s *es, actorInfo_t *actorInfo);
void __cdecl BG_Dog_Death_Fast_Forward_Left(int localClientNum, const entityState_s *es, actorInfo_t *actorInfo);
void __cdecl BG_Dog_Death_Fast_Forward_Right(int localClientNum, const entityState_s *es, actorInfo_t *actorInfo);
void __cdecl BG_Dog_Flashed(int localClientNum, const entityState_s *es, ActorAnimStates prevState);

void __cdecl BG_Dog_Combat_AttackIdle(int localClientNum, const entityState_s *es, ActorAnimStates prevState);
void __cdecl BG_Dog_Combat_AttackIdle_Bark(int localClientNum, const entityState_s *es, ActorAnimStates prevState);
void __cdecl BG_Dog_Combat_AttackIdle_Growl(int localClientNum, const entityState_s *es, ActorAnimStates prevState);
void __cdecl BG_Dog_Combat_Attack_Run(int localClientNum, const entityState_s *es, ActorAnimStates prevState);
void __cdecl BG_Dog_Combat_Attack_Player_Close_Range(int localClientNum, const entityState_s *es, ActorAnimStates prevState);
void __cdecl BG_Dog_Combat_Attack_Miss(int localClientNum, const entityState_s *es, ActorAnimStates prevState);
void __cdecl BG_Dog_Combat_Attack_Miss_Left(int localClientNum, const entityState_s *es, ActorAnimStates prevState);
void __cdecl BG_Dog_Combat_Attack_Miss_Right(int localClientNum, const entityState_s *es, ActorAnimStates prevState);
void __cdecl BG_Dog_Clear_Traverse_Anims(int localClientNum, const entityState_s *es, ActorAnimStates prevState);
void __cdecl BG_Dog_Traverse_Jump_Up_40(int localClientNum, const entityState_s *es, ActorAnimStates prevState);
void __cdecl BG_Dog_Traverse_Jump_Up_80(int localClientNum, const entityState_s *es, ActorAnimStates prevState);
void __cdecl BG_Dog_Traverse_Jump_Down_40(int localClientNum, const entityState_s *es, ActorAnimStates prevState);
void __cdecl BG_Dog_Traverse_Jump_Down_80(int localClientNum, const entityState_s *es, ActorAnimStates prevState);
void __cdecl BG_Dog_Traverse_Wallhop(int localClientNum, const entityState_s *es, ActorAnimStates prevState);
void __cdecl BG_Dog_Traverse_Window(int localClientNum, const entityState_s *es, ActorAnimStates prevState);
void __cdecl BG_Dog_Traverse_Through_Hole_42(int localClientNum, const entityState_s *es, ActorAnimStates prevState);
