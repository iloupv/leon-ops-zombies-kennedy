#pragma once

#include <physics/phys_local.h>
#include <physics/phys_main.h>

struct flamePhysics_t // sizeof=0x34
{                                                                             // XREF: flameGeneric_s/r
        float origin[3];
        float newPos[3];
        float velocity[3];
        float invStartSpeed;
        float gravity;
        float rotation;
        float rotVel;
};

struct col_context_t;
struct trace_t;
struct flameGeneric_s;

//void __thiscall colgeom_visitor_inlined_t<500>::reset(colgeom_visitor_inlined_t<200> *this);
void __cdecl Flame_ClipVelocity(const float *in, const float *normal, float *out);
void __cdecl trace_sphere(trace_t *trace, const float *start, const float *end, float radius, col_context_t *context);
void __cdecl Flame_Phys_Collision(
                bool is_server,
                trace_t *trace,
                flameGeneric_s *gen,
                col_context_t *context,
                unsigned __int16 (*dynEnts)[2][100],
                const int *dynEntsCount,
                int *ents,
                int entsCount,
                int *close_characters,
                int *close_characters_count,
                int *close_triggers,
                int *close_triggers_count);
void __cdecl Flame_Server_Trace(
                trace_t *trace,
                flameGeneric_s *gen,
                const float *startPos,
                const float *mins,
                const float *maxs,
                float *endPos,
                col_context_t *context,
                unsigned __int16 (*dynEnts)[2][100],
                const int *dynEntsCount,
                int *ents,
                int entsCount,
                int *close_characters,
                int *close_characters_count,
                int *close_triggers,
                int *close_triggers_count);
double __cdecl point_aabb_dist2(float *a, const float *mn, float *mx);
void __cdecl Flame_Client_Trace(
                trace_t *trace,
                flameGeneric_s *gen,
                const float *startPos,
                const float *mins,
                const float *maxs,
                float *endPos,
                col_context_t *context,
                unsigned __int16 (*dynEnts)[2][100],
                const int *dynEntsCount,
                int *ents,
                int entsCount,
                int *close_characters,
                int *close_characters_count);
void __cdecl Flame_Phys_Collision_Response(bool is_server, trace_t *trace, flameGeneric_s *gen);
void __cdecl Flame_Impact_Process(bool is_server, flameGeneric_s *gen, trace_t *trace, int *models, int models_count);
void __cdecl Flame_Phys_Update_Items(bool is_server);
void __cdecl Flame_Phys_Update_Items_PerStream(bool is_server, int nitems, flameGeneric_s **items);
void __cdecl Flame_Phys_Update_Item_Stream(flameGeneric_s *gen, int time);
void __cdecl Flame_Phys_Update_Item_Chunk(
                flameGeneric_s *gen,
                int time,
                phys_static_array<flameGeneric_s *,1000> *flames);
void __cdecl Flame_Phys_Update_Item_Fire(flameGeneric_s *gen, int time);
void __cdecl Flame_Phys_Update_Item_Smoke(flameGeneric_s *gen, int time);
void __cdecl Flame_Phys_Update_Item_Drip(
                flameGeneric_s *gen,
                int time,
                phys_static_array<flameGeneric_s *,1000> *flames);

//void __thiscall colgeom_visitor_inlined_t<500>::visit(
//                colgeom_visitor_inlined_t<500> *this,
//                const CollisionAabbTree *tree);
//void __thiscall colgeom_visitor_inlined_t<500>::visit(colgeom_visitor_inlined_t<500> *this, const cbrush_t *brush);
//void __thiscall colgeom_visitor_inlined_t<500>::update(
//                colgeom_visitor_inlined_t<500> *this,
//                const float *_mn,
//                const float *_mx,
//                int mask,
//                const float *expand_vec);
//

extern cdl_proftimer sv_flame_proftimer;
extern cdl_proftimer cl_flame_proftimer;
