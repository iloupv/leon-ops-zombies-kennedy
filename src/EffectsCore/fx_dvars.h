#pragma once

#include <universal/dvar.h>

void __cdecl FX_RegisterDvars();
void __cdecl FX_CreateDevGui();

extern const dvar_t *fx_enable;
extern const dvar_t *fx_draw;
extern const dvar_t *fx_cull_elem_spawn;
extern const dvar_t *fx_cull_elem_draw;
extern const dvar_t *fx_cull_effect_spawn;
extern const dvar_t *fx_marks;
extern const dvar_t *fx_marks_smodels;
extern const dvar_t *fx_marks_ents;
extern const dvar_t *fx_freeze;
extern const dvar_t *fx_debugBolt;
extern const dvar_t *fx_count;
extern const dvar_t *fx_visMinTraceDist;
extern const dvar_t *fx_profile;
extern const dvar_t *fx_mark_profile;
extern const dvar_t *fx_drawClouds;
extern const dvar_t *fx_priority_debug;
extern const dvar_t *fx_seethru_parallax;
extern const dvar_t *fx_priority_enable;
extern const dvar_t *fx_draw_weapon_tags;
extern const dvar_t *fx_marks_draw_impact_axis;
extern const dvar_t *fx_marks_debug_text;
extern const dvar_t *fx_marks_range;
extern const dvar_t *bo1_fxcolor; // zombies (L16): measurement only