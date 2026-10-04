#pragma once

#include <universal/dvar.h>

struct pmove_t;
struct pml_t;
struct playerState_s;

void __cdecl Dtp_RegisterDvars();
int __cdecl Dtp_CalcSurfaceType(pmove_t *pm, pml_t *pml);
bool __cdecl Dtp_IsSliding(const playerState_s *ps);
void __cdecl Dtp_AdjustDamage(const playerState_s *ps, float fallHeight, int *damage);
void __cdecl Dtp_Interrupt(pmove_t *pm);
void __cdecl Dtp_CheckForEnd(pmove_t *pm);
void __cdecl Dtp_End(pmove_t *pm);
double __cdecl Dtp_ReduceFriction(pmove_t *pm, const playerState_s *ps);
void __cdecl Dtp_AirMove(playerState_s *ps, const pmove_t *pm);
bool __cdecl Dtp_CanMove(pmove_t *pm);
int __cdecl Dtp_Update(pmove_t *pm, pml_t *pml);
void __cdecl Dtp_Start(pmove_t *pm, pml_t *pml);
bool __cdecl Dtp_IsDtp(const pmove_t *pm, const playerState_s *ps);

extern const dvar_s *dtp;
extern const dvar_s *dtp_debug;
extern const dvar_s *dtp_max_slide_duration;
extern const dvar_s *dtp_max_slide_addition;
extern const dvar_s *dtp_startup_delay;
extern const dvar_s *dtp_post_move_pause;
extern const dvar_s *dtp_fall_damage_min_height;
extern const dvar_s *dtp_fall_damage_max_height;
extern const dvar_s *dtp_exhaustion_window;
extern const dvar_s *dtp_min_speed;
extern const dvar_s *door_breach_weapondrop;
extern const dvar_s *dtp_new_trajectory;
extern const dvar_s *dtp_new_trajectory_multiplier;
extern const dvar_s *dtp_max_apex_duration;