#pragma once
#include <physics/phys_local.h>
#include <physics/phys_gjk.h>
#include <physics/phys_colgeom.h>

gjk_polygon_cylinder_t *__cdecl create_gjkcc_gjk_geom(
                float (*mins)[3],
                float (*maxs)[3],
                int stype,
                gjk_collision_visitor *allocator);
gjkcc_info *__cdecl create_gjkcc_info(unsigned int gjkcc_id, bool is_server_thread, const float *origin);

void __cdecl destroy_gjkcc_info(gjkcc_info *gcci_);
void __cdecl destroy_client_gjkcc_info(int localClientNum);
void    gjkcc_prolog(const gjkcc_input_t *gjkcc_in, float *origin);
void __cdecl gjkcc_epilog(const gjkcc_input_t *gjkcc_in, const float *origin);
gjkcc_info *__cdecl find_gjkcc_info(unsigned int gjkcc_id, bool is_server_thread);



void __cdecl calc_time(phys_gjk_info *gjk_info, bool keep_all_collisions, float *time, float *dist);
bool gjk_collide(
                phys_gjk_info *gjk_info,
                phys_gjk_input *pgi,
                gjk_trace_output_t *gto,
                const gjk_trace_input_t &gti,
                gjk_geom_info_t *gi);
void __cdecl sort_gi_list(gjk_geom_info_t **list, int list_count);
void    gjk_trace(const gjk_trace_input_t &gti, list_gjk_trace_output *list);

void __cdecl init_pgi(phys_gjk_input *pgi, const gjk_trace_input_t &gti);
void __cdecl set_pgi_cg2(phys_gjk_input *pgi, const gjk_trace_input_t &gti, gjk_geom_info_t *gi);
void __cdecl setup_query_input(
                const gjkcc_input_t *gjkcc_in,
                const phys_vec3 *aabb_min,
                const phys_vec3 *aabb_max,
                const phys_vec3 *position,
                const phys_vec3 *translation,
                int passEntityNum,
                int contentMask,
                gjk_query_input *query_input);
void    setup_trace_info(
                const gjkcc_input_t *gjkcc_in,
                float *start,
                float *mins,
                float *maxs,
                float *end,
                int passEntityNum,
                int contentMask,
                gjk_trace_input_t *gti);
void    gjk_player_trace(
                const gjkcc_input_t *gjkcc_in,
                trace_t *results,
                float *start,
                float *mins,
                float *maxs,
                float *end,
                int passEntityNum,
                int contentMask);
char __cdecl no_push_out(const gjk_trace_output_t *gto);

void __cdecl set_hit_info(geom_plane *gp, const gjk_trace_output_t *gto);
void __cdecl add_hit_info(
                const gjk_trace_output_t *gto,
                phys_static_array<geom_plane,128> *list_geom_plane,
                phys_link_list<gjk_geom_info_t> *geom_skip_list);
bool __cdecl project_succeeded(
                phys_static_array<geom_plane,128> *list_geom_plane,
                const phys_vec3 *new_position,
                float PROJECT_FAIL_THRESH);
bool gjk_push_out(
                const gjkcc_input_t *gjkcc_in,
                gjk_slide_move_input_t *input,
                gjk_slide_move_output_t *output);
bool    gjk_slide_move1(
                const gjkcc_input_t *gjkcc_in,
                gjk_slide_move_input_t *input,
                const bool no_push_out,
                gjk_slide_move_output_t *output,
                bool *needs_push_out);
int __cdecl gjk_slide_move(
                const gjkcc_input_t *gjkcc_in,
                gjk_slide_move_input_t *input,
                gjk_slide_move_output_t *output);
void    setup_player_push_slide_move_input(
                struct player_push_slide_move_input_t *input,
                struct pmove_t *pm,
                struct pml_t *pml,
                const phys_vec3 *push_movement);
void    gjk_sentient_push(
                pmove_t *pm,
                pml_t *pml,
                float *origin,
                float *mins,
                float *maxs,
                int contentMask,
                int passEntityNum,
                const gjkcc_input_t *gjkcc_in,
                const float pushRadius,
                float frameTime,
                float *velocity_,
                float *origin_);
// local variable allocation has failed, the output may be wrong!
void    render_gjkcc_collision(float (*mins)[3], float (*maxs)[3], float (*origin)[3]);
//phys_vec3 *__cdecl operator+(phys_vec3 *result, const phys_vec3 *a, const phys_vec3 *b);

//void __thiscall phys_gjk_geom::set_simplex(
//                phys_gjk_geom *this,
//                const phys_vec3 *simplex_inds,
//                int w_set,
//                const phys_vec3 *normal,
//                cached_simplex_info *cache_info);
//bool __thiscall phys_gjk_geom::ray_cast(
//                phys_gjk_geom *this,
//                const phys_vec3 *__formal,
//                const phys_vec3 *ray_dir,
//                float t_input,
//                float *t_output,
//                phys_vec3 *hitn);
//double __thiscall phys_gjk_geom::get_geom_radius(phys_gjk_geom *this);





void __cdecl render_gjkcc_collision(pmove_t *pm);


void    PM_gjk_ground_trace(
                const gjkcc_input_t *gjkcc_in,
                trace_t *results,
                float *start,
                float *mins,
                float *maxs,
                float *end,
                int passEntityNum,
                int contentMask,
                float *first_hit_time);
double __cdecl PM_PermuteRestrictiveClipPlanes(
                const float *velocity,
                int planeCount,
                const float (*planes)[3],
                int *permutation);
void __cdecl setup_player_gjk_slide_move_input(
                player_gjk_slide_move_input_t *input,
                pmove_t *pm,
                pml_t *pml,
                int gravity);

void __cdecl PM_StepSlideMove(pmove_t *pm, pml_t *pml, int gravity);
int __cdecl PM_VerifyPronePosition(pmove_t *pm, float *vFallbackOrg, float *vFallbackVel);
int __cdecl PM_SlideMove(pmove_t *pm, pml_t *pml, int gravity);







//void __thiscall phys_simple_allocator<phys_heap_gjk_cache_system_avl_tree::phys_gjk_cache_info_internal>::free(
//                phys_simple_allocator<phys_heap_gjk_cache_system_avl_tree::phys_gjk_cache_info_internal> *this,
//                phys_heap_gjk_cache_system_avl_tree::phys_gjk_cache_info_internal *slot);
//void __thiscall phys_inplace_avl_tree<phys_gjk_geom_id_pair_key,phys_heap_gjk_cache_system_avl_tree::phys_gjk_cache_info_internal,phys_heap_gjk_cache_system_avl_tree::phys_gjk_cache_info_internal::avl_tree_accessor>::remove(
//                phys_inplace_avl_tree<phys_gjk_geom_id_pair_key,phys_heap_gjk_cache_system_avl_tree::phys_gjk_cache_info_internal,phys_heap_gjk_cache_system_avl_tree::phys_gjk_cache_info_internal::avl_tree_accessor> *this,
//                const phys_gjk_geom_id_pair_key *key);
//void __thiscall phys_link_list<gjk_geom_info_t>::add(phys_link_list<gjk_geom_info_t> *this, gjk_geom_info_t *p);
//gjkcc_info *__thiscall phys_simple_allocator<gjkcc_info>::allocate(phys_simple_allocator<gjkcc_info> *this);
//void __thiscall gjk_collision_visitor::get_local_query_aabb(
//                gjk_collision_visitor *this,
//                float *local_query_aabb_min,
//                float *local_query_aabb_max);
//char __thiscall gjk_collision_visitor::query_create_prolog_1(
//                gjk_collision_visitor *this,
//                const float *local_aabb_min,
//                const float *local_aabb_max,
//                const void *geom);
//void __thiscall gjk_collision_visitor::query_create_epilog_1(gjk_collision_visitor *this, gjk_base_t *gjk_geom);
//void __thiscall phys_simple_allocator<gjkcc_info>::free(phys_simple_allocator<gjkcc_info> *this, gjkcc_info *slot);
//void __thiscall phys_link_list<gjk_trace_output_t>::add(
//                phys_link_list<gjk_trace_output_t> *this,
//                gjk_trace_output_t *p);
//geom_plane *__thiscall phys_static_array<geom_plane,128>::add(
//                phys_static_array<geom_plane,128> *this,
//                int no_error,
//                const char *error_msg);
//geom_plane *__thiscall phys_static_array<geom_plane,128>::operator[](phys_static_array<geom_plane,128> *this, int i);

//void __thiscall phys_inplace_avl_tree<bpei_database_id,broad_phase_environment_info,broad_phase_environment_info::avl_tree_accessor>::remove(
//                phys_inplace_avl_tree<bpei_database_id,broad_phase_environment_info,broad_phase_environment_info::avl_tree_accessor> *this,
//                const bpei_database_id *key);
//void __thiscall phys_simple_allocator<broad_phase_environment_info>::free(
//                phys_simple_allocator<broad_phase_environment_info> *this,
//                broad_phase_environment_info *slot);
//void __thiscall phys_inplace_avl_tree<phys_gjk_geom_id_pair_key,phys_heap_gjk_cache_system_avl_tree::phys_gjk_cache_info_internal,phys_heap_gjk_cache_system_avl_tree::phys_gjk_cache_info_internal::avl_tree_accessor>::rotate_right(
//                phys_inplace_avl_tree<phys_gjk_geom_id_pair_key,phys_heap_gjk_cache_system_avl_tree::phys_gjk_cache_info_internal,phys_heap_gjk_cache_system_avl_tree::phys_gjk_cache_info_internal::avl_tree_accessor> *this,
//                phys_heap_gjk_cache_system_avl_tree::phys_gjk_cache_info_internal **root);
//void __thiscall phys_inplace_avl_tree<phys_gjk_geom_id_pair_key,phys_heap_gjk_cache_system_avl_tree::phys_gjk_cache_info_internal,phys_heap_gjk_cache_system_avl_tree::phys_gjk_cache_info_internal::avl_tree_accessor>::rotate_left(
//                phys_inplace_avl_tree<phys_gjk_geom_id_pair_key,phys_heap_gjk_cache_system_avl_tree::phys_gjk_cache_info_internal,phys_heap_gjk_cache_system_avl_tree::phys_gjk_cache_info_internal::avl_tree_accessor> *this,
//                phys_heap_gjk_cache_system_avl_tree::phys_gjk_cache_info_internal **root);
//bool __thiscall gjk_collision_visitor::is_query(gjk_collision_visitor *this);