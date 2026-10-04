#pragma once
#include "actor.h"

bool __fastcall Actor_ScriptedAnim_Start_SP(actor_s *self, ai_state_t previous);
void __fastcall Actor_ScriptedAnim_Finish_SP(actor_s *self, ai_state_t next);
actor_think_result_t __fastcall Actor_ScriptedAnim_Think_SP(actor_s *self);
bool __fastcall Actor_CustomAnim_Start_SP(actor_s *self, ai_state_t previous);
actor_think_result_t __fastcall Actor_CustomAnim_Think_SP(actor_s *self);

void __cdecl GScr_AnimScripted_SP(scr_entref_t entref);
void __cdecl GScr_StartScriptedAnim_SP(scr_entref_t entref);
void __cdecl GScr_StopAnimScripted_SP(scr_entref_t entref);
// SP 0x008084f0: stopanimscripted's body; G_SetAnimTree runs it first.
void G_SP_StopScriptedAnim(gentity_s *ent);
void __cdecl ActorCmd_SetProneAnimNodes_SP(scr_entref_t entref);

// SP 0x00501350, also consumed by unlinked point-relative actor animation mode.
void G_ScriptedAnim_Think_SP(gentity_s *ent);

// SP G_RunMover scripted-animation prefix; true while native scripted placement owns movement.
bool G_RunScriptedMover_SP(gentity_s *ent);

// L15 (TEST, bo1_measure_animscripted_cl): SP-style placement of entnum's scripted anim from obj (SP 0x007d4fb0)
// plus the server's last offset, from the server thread's copy of the record; measurement only.
bool G_SP_MeasureScriptedPlacement(int entnum, int time, DObj *obj, float *origin, float *yaw,
    unsigned int *anim, int *startTime, float *serverWeight, bool *placed);
