#pragma once
#include "rigid_body.h"

Phys_UnitQuaternion *__cdecl Phys_GetQuaternion(Phys_UnitQuaternion *result, const phys_mat44 *M);

struct nuge // sizeof=0x0
{
    static void __cdecl get_ballistic_info(
        rigid_body *const *list_rigid_body,
        int rbodies_count,
        phys_vec3 *center_of_mass,
        phys_vec3 *total_momentum,
        float *total_mass);
    static void    apply_ballistic_target(
        rigid_body *const *list_rigid_body,
        int rbodies_count,
        const phys_vec3 *target,
        float *const dist_sq);
    static void    calc_velocities(
        const phys_mat44 *mat0,
        const phys_mat44 *mat1,
        float delta_t,
        phys_vec3 *t_vel,
        phys_vec3 *a_vel);
    static void    calc_velocities(
        const phys_mat44 *mat0,
        const phys_mat44 *mat1,
        const phys_vec3 *center_offset_loc,
        float delta_t,
        phys_vec3 *t_vel,
        phys_vec3 *a_vel);
    static void __cdecl calc_sphere_inertia(float radius, phys_vec3 *unit_inertia, float *volume);
    static void __cdecl calc_box_inertia(const phys_vec3 *dim, phys_vec3 *unit_inertia, float *volume);
    static void __cdecl calc_bound_sphere(const phys_vec3 *vert_list, int vert_count, float *radius, phys_vec3 *com);
    static void __cdecl calc_bound_box(const phys_vec3 *vert_list, int vert_count, phys_vec3 *dim, phys_vec3 *com);
    static void __cdecl tensor_transform_principle(const phys_vec3 *diag, const phys_mat44 *mat, phys_mat44 *tensor);
};