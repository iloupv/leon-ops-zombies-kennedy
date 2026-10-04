#pragma once
#include <bgame/bg_public.h>

#define MAX_TARGETS 32

struct target_t // sizeof=0x1C
{                                       // XREF: $A5C519FFED38118F396585C413DE405F/r
    gentity_s *ent;                     // XREF: G_InitTargets(void)+2C/w
                                        // Scr_Target_SetShader(void)+6B/r ...
    float offset[3];
    int materialIndex;
    int offscreenMaterialIndex;
    int flags;
};

void __cdecl G_InitTargets();
void __cdecl Scr_Target_SetShader();
void __cdecl Scr_Target_SetOffscreenShader();
void __cdecl Scr_Target_GetArray();
unsigned int __cdecl Target_GetTargetIndex(gentity_s *ent);
void __cdecl Scr_Target_IsTarget();
void __cdecl Scr_Target_SetTurretAquire();
void __cdecl Scr_Target_Set();
bool Targ_Remove(gentity_s *ent);
void __cdecl Targ_RemoveAll();
void __cdecl Scr_Target_Remove();
void __cdecl Scr_Target_IsInCircle();
char __cdecl ScrGetTargetScreenPos(float *screenPos);
char __cdecl G_WorldDirToScreenPos(const gentity_s *player, float fov_x, const float *worldDir, float *outScreenPos);
void __cdecl Scr_Target_IsInRect();
void __cdecl Scr_Target_StartLockOn();
void __cdecl Scr_Target_ClearLockOn();
char __cdecl G_TargetGetOffset(const gentity_s *targ, float *result);
unsigned int __cdecl GetTargetIdx(const gentity_s *ent);
bool __cdecl G_TargetAttackProfileTop(const gentity_s *ent);
void __cdecl Scr_Target_SetAttackMode();
void __cdecl Scr_Target_SetJavelinOnly();
target_t *__cdecl Target_GetTargetArray();
