#pragma once
#include "actor.h"

enum scriptAnimAIFunctionTypes_t : __int32
{                                       // XREF: ?Actor_SetAnimScript@@YIXPAUactor_s@@PAUscr_animscript_t@@EW4ai_animmode_t@@W4scriptAnimAIFunctionTypes_t@@@Z/r
    AI_ANIM_FUNCTION_STOP   = 0x0,
    AI_ANIM_FUNCTION_MOVE   = 0x1,
    AI_ANIM_FUNCTION_COMBAT = 0x2,
    AI_ANIM_FUNCTION_PAIN   = 0x3,
    AI_ANIM_FUNCTION_REACT  = 0x4,
    AI_ANIM_FUNCTION_DEATH  = 0x5,
};

struct AnimScriptList // sizeof=0x40C0
{                                       // XREF: scr_data_t/r
    scr_animscript_t combat;
    scr_animscript_t concealment_crouch;
    scr_animscript_t concealment_prone;
    scr_animscript_t concealment_stand;
    scr_animscript_t cover_arrival;
    scr_animscript_t cover_crouch;
    scr_animscript_t cover_left;
    scr_animscript_t cover_prone;
    scr_animscript_t cover_right;
    scr_animscript_t cover_stand;
    scr_animscript_t cover_wide_left;
    scr_animscript_t cover_wide_right;
    scr_animscript_t death;             // XREF: .text:006418CE/o
    scr_animscript_t grenade_return_throw;
    scr_animscript_t init;              // XREF: .text:006418E4/o
    scr_animscript_t pain;              // XREF: .text:006418FA/o
    scr_animscript_t react;
    scr_animscript_t move;              // XREF: .text:00641910/o
    scr_animscript_t scripted;
    scr_animscript_t stop;              // XREF: .text:00641926/o
    scr_animscript_t grenade_cower;
    scr_animscript_t flashed;           // XREF: .text:0064193C/o
    scr_animscript_t jump;              // XREF: .text:00641952/o
                                        // Actor_Dog_Exposed_Think(actor_s *)+39F/o
    scr_animscript_t turn;              // XREF: .text:00641968/o
                                        // Actor_Dog_Exposed_Think(actor_s *)+3E0/o ...
    scr_animscript_t weapons[2048];
};

void __fastcall Actor_InitAnim(actor_s *self);
bool __fastcall Actor_IsAnimScriptAlive(actor_s *self);
void __fastcall Actor_KillAnimScript(actor_s *self);
void __fastcall Actor_SetAnimScript(
                actor_s *self,
                scr_animscript_t *pAnimScriptFunc,
                ai_movemode_t moveMode,
                ai_animmode_t animMode,
                scriptAnimAIFunctionTypes_t animScript);
void __fastcall Actor_AnimStop(actor_s *self, scr_animscript_t *pAnimScriptFunc);
void __fastcall Actor_AnimMoveAway(actor_s *self, scr_animscript_t *pAnimScriptFunc);
scr_animscript_t *__fastcall Actor_GetStopAnim(actor_s *self);
void __fastcall Actor_AnimTryWalk(actor_s *self);
void __fastcall Actor_AnimTryRun(actor_s *self);
void __fastcall Actor_AnimPain(actor_s *self);
void __fastcall Actor_AnimDeath(actor_s *self);
void __fastcall Actor_AnimSpecific(actor_s *self, scr_animscript_t *func, ai_animmode_t eAnimMode, bool bUseGoalWeight);
void __stdcall Actor_AnimSetCompleteGoalWeight(
                XAnimTree_s *tree,
                unsigned int animIndex,
                float goalWeight,
                float goalTime,
                float rate,
                unsigned int notifyName,
                unsigned int notifyType,
                int bRestart);
void __stdcall Actor_AnimClearGoalWeight(unsigned int animIndex, float blendTime);

extern AnimScriptList *g_animScriptTable[MAX_AI_SPECIES];