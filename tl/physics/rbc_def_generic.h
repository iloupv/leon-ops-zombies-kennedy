#pragma once

#include <physics/rigid_body.h>

const phys_vec3 *__cdecl construct_orth_ud(phys_vec3 *result, const phys_vec3 *ud);

void __cdecl PHYS_ASSERT_UNIT(const phys_vec3 *v);
void __cdecl PHYS_ASSERT_ORTHOGONAL(const phys_vec3 *v1, const phys_vec3 *v2);
void __cdecl PHYS_ASSERT_ORTHONORMAL(const phys_mat44 *m);

//const phys_vec3 *__cdecl rbint::sub_pos(const phys_vec3 *result, rigid_body *const b, const phys_vec3 *p);

#if 0 // so if I just use #if 0, VS will still F12 jump to these declarations. Great.
/*
void __thiscall rigid_body_constraint_point::set(const phys_vec3 *b1_r_loc, const phys_vec3 *b2_r_loc);
void __thiscall rigid_body_constraint_point::epilog_vel_constraint(float __formal);
void rigid_body_constraint_point::setup_constraint(pulse_sum_constraint_solver *psys, float delta_t);

void __thiscall rigid_body_constraint_distance::set(
        const phys_vec3 *b1_r_loc,
        const phys_vec3 *b2_r_loc,
        float min_distance,
        float max_distance);
void __thiscall rigid_body_constraint_distance::outer_prolog_update(float delta_t);
void __thiscall rigid_body_constraint_distance::inner_update(float delta_t);
void __thiscall rigid_body_constraint_distance::outer_epilog_update(const float __formal);
void rigid_body_constraint_distance::setup_constraint(pulse_sum_constraint_solver *psys, float delta_t);

void __thiscall rigid_body_constraint_angular_actuator::set(float power, const phys_mat44 *target_mat);
void rigid_body_constraint_angular_actuator::inner_update(float delta_t);
void rigid_body_constraint_angular_actuator::setup_constraint(pulse_sum_constraint_solver *psys, float delta_t);
void rigid_body_constraint_angular_actuator::outer_prolog_update(float delta_t);
void __thiscall rigid_body_constraint_angular_actuator::outer_epilog_update(const float __formal);

void rigid_body_constraint_hinge::set(
    const phys_vec3 *b1_r_loc,
    const phys_vec3 *b2_r_loc,
    const phys_vec3 *b1_axis_loc,
    const phys_vec3 *b2_axis_loc,
    const phys_vec3 *b1_ref_loc,
    const phys_vec3 *b2_ref_loc,
    float theta_min,
    float theta_max,
    float damp_k);
void rigid_body_constraint_hinge::setup_constraint(pulse_sum_constraint_solver *psys, float delta_t);

double __thiscall rigid_body_constraint_upright::calc_current_lean_angle();
const phys_vec3 *__thiscall rigid_body_constraint_upright::calc_b1_lean_axis_loc(const phys_vec3 *result,float lean_angle);
void __thiscall rigid_body_constraint_upright::epilog_vel_constraint(float delta_t);
void rigid_body_constraint_upright::update_lean_axis(const phys_vec3 *b1_lean_center, const phys_vec3 *b1_lean_axis_loc);
// local variable allocation has failed, the output may be wrong!
void rigid_body_constraint_upright::setup_constraint(pulse_sum_constraint_solver *psys, float delta_t);

void __thiscall pulse_sum_normal::setup_vel_uni_standard_pos_adjust(
        float delta_t,
        float pos_adjust,
        float max_penalty_restitution_vel);

void __thiscall pulse_sum_angular::setup_vel_uni_standard(
        float delta_t,
        float max_penalty_restitution_vel);


pulse_sum_normal *__thiscall pulse_sum_constraint_solver::create_pulse_sum_normal();
void __thiscall pulse_sum_constraint_solver::create_point(
        rigid_body *const b1,
        const phys_vec3 *b1_r,
        rigid_body *const b2,
        const phys_vec3 *b2_r,
        pulse_sum_cache *const ps_cache,
        float delta_t,
        bool is_spring,
        float spring_k,
        float damp_k);
pulse_sum_angular *__thiscall pulse_sum_constraint_solver::create_pulse_sum_angular(
        rigid_body *const b1,
        const phys_vec3 *b1_r,
        rigid_body *const b2,
        const phys_vec3 *b2_r,
        const phys_vec3 *ud,
        pulse_sum_cache *const ps_cache);
void __thiscall pulse_sum_constraint_solver::create_hinge(
        rigid_body *const b1,
        const phys_vec3 *b1_axis,
        rigid_body *const b2,
        const phys_vec3 *b2_axis,
        const phys_vec3 *a1,
        const phys_vec3 *a2,
        pulse_sum_cache *const ps_cache,
        float delta_t);
*/
#endif