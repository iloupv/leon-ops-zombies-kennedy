#pragma once
#include "rigid_body.h"

struct phys_sys // sizeof=0x0
{
    static environment_rigid_body *__cdecl get_environment_rigid_body();

    static void __cdecl set_max_delta_t(float max_delta_t);
    static double __cdecl get_max_delta_t();
    static void __cdecl set_v_tol(int max_v_iters);
    static void __cdecl get_v_tol(int *max_v_iters);
    static void __cdecl set_vp_tol(int max_vp_iters);
    static void __cdecl get_vp_tol(int *max_vp_iters);

    static void __cdecl set_collision_callback(void(__cdecl *collision_callback)());
    static void __cdecl phys_frame_advance(float delta_t);
    static void __cdecl phys_init();
    static void __cdecl phys_shutdown();

    static int __cdecl get_rigid_body_alloc_count();
    static int __cdecl get_user_rigid_body_alloc_count();
    static int __cdecl get_rbc_point_alloc_count();
    static int __cdecl get_rbc_hinge_alloc_count();
    static int __cdecl get_rbc_dist_alloc_count();
    static int __cdecl get_rbc_ragdoll_alloc_count();
    static int __cdecl get_rbc_wheel_alloc_count();
    static int __cdecl get_rbc_angular_actuator_alloc_count();
    static int __cdecl get_rbc_upright_alloc_count();
    static int __cdecl get_rbc_custom_orientation_alloc_count();
    static int __cdecl get_rbc_custom_path_alloc_count();
    static int __cdecl get_rbc_contact_alloc_count();

    user_rigid_body *get_user_rigid_body(const phys_mat44 *const dictactor);
    rigid_body_constraint_contact *__cdecl get_rbc_contact(
        environment_rigid_body *b1,
        environment_rigid_body *b2);
    static void __cdecl fixup_wheel_constraints(rigid_body *const rb);
    void __cdecl update_constraint_infos(); // Unused!
    static rigid_body *__cdecl create_rigid_body(int no_error);
    static rigid_body_constraint_point *__cdecl create_rbc_point(
        environment_rigid_body *b1,
        environment_rigid_body *b2,
        int no_error);
    static rigid_body_constraint_custom_orientation *__cdecl create_rbc_custom_orientation(
        environment_rigid_body *b1,
        environment_rigid_body *b2,
        int no_error);
    static rigid_body_constraint_contact *__cdecl create_rbc_contact(
        environment_rigid_body *b1,
        environment_rigid_body *b2,
        int no_error);

    static void __cdecl destroy(rigid_body *const rb);
    static void __cdecl destroy(user_rigid_body *const rb);
    static void __cdecl destroy(rigid_body_constraint_contact *const rbc);
    static void __cdecl destroy(rigid_body_constraint_point *const rbc);
    static void __cdecl destroy(rigid_body_constraint_hinge *const rbc);
    static void __cdecl destroy(rigid_body_constraint_distance *const rbc);
    static void __cdecl destroy(rigid_body_constraint_ragdoll *const rbc);
    static void __cdecl destroy(rigid_body_constraint_wheel *const rbc);
    static void __cdecl destroy(rigid_body_constraint_angular_actuator *const rbc);
    static void __cdecl destroy(rigid_body_constraint_upright *const rbc);
    static void __cdecl destroy(rigid_body_constraint_custom_orientation *const rbc);
    static void __cdecl destroy(rigid_body_constraint_custom_path *const rbc);

    static void __cdecl destroy_all_rbc_point();
    static void __cdecl destroy_all_rbc_hinge();
    static void __cdecl destroy_all_rbc_dist();
    static void __cdecl destroy_all_rbc_ragdoll();
    static void __cdecl destroy_all_rbc_wheel();
    static void __cdecl destroy_all_rbc_angular_actuator();
    static void __cdecl destroy_all_rbc_upright();
    static void __cdecl destroy_all_rbc_custom_orientation();
    static void __cdecl destroy_all_rbc_custom_path();
    static void __cdecl destroy_all_unused_user_rigid_body();

    void __cdecl destroy_all_rigid_body();
    void __cdecl destroy_all_user_rigid_body();
    void __cdecl destroy_all_rbc_contact();
    static void __cdecl destroy_all_constraint(rigid_body *const rb);
    void __cdecl destroy_all_constraint_with_user_rigid_body(rigid_body *const rb);

    static user_rigid_body *__cdecl create_user_rigid_body(int no_error);

    static rigid_body_constraint_hinge *__cdecl create_rbc_hinge(
        environment_rigid_body *b1,
        environment_rigid_body *b2,
        int no_error);
    static rigid_body_constraint_distance *__cdecl create_rbc_dist(
        environment_rigid_body *b1,
        environment_rigid_body *b2,
        int no_error);
    static rigid_body_constraint_ragdoll *__cdecl create_rbc_ragdoll(
        environment_rigid_body *b1,
        environment_rigid_body *b2,
        int no_error);
    static rigid_body_constraint_wheel *__cdecl create_rbc_wheel(
        environment_rigid_body *b1,
        environment_rigid_body *b2,
        int no_error);
    static rigid_body_constraint_angular_actuator *__cdecl create_rbc_angular_actuator(
        environment_rigid_body *b1,
        environment_rigid_body *b2,
        int no_error);
    static rigid_body_constraint_upright *__cdecl create_rbc_upright(
        environment_rigid_body *b1,
        environment_rigid_body *b2,
        int no_error);
    static rigid_body_constraint_custom_path *__cdecl create_rbc_custom_path(
        environment_rigid_body *b1,
        environment_rigid_body *b2,
        int no_error);
};

rigid_body_constraint_contact *__cdecl create(
                phys_free_list<rigid_body_constraint_contact> *__formal,
                rigid_body_constraint_contact *a2,
                environment_rigid_body *b1,
                environment_rigid_body *b2,
                int no_error,
                const char *error_msg);
