#pragma once
#include "rigid_body.h"

struct __declspec(align(16)) physics_system // sizeof=0x32C0
{
    int m_flags;
    float m_outside_sub_delta_t;
    void (__cdecl *m_collision_callback)();
    float m_max_delta_t;
    int m_max_vel_iters;
    int m_max_vel_pos_iters;
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    environment_rigid_body m_environment_rigid_body;
    phys_inplace_avl_tree<rigid_body_pair_key,rigid_body_constraint_contact,rigid_body_constraint_contact::avl_tree_accessor> m_search_tree_rbc_contact;
    phys_free_list<user_rigid_body> m_list_user_rigid_body;
    phys_free_list<rigid_body> m_list_rigid_body;
    phys_free_list<rigid_body_constraint_contact> m_list_rbc_contact;
    phys_free_list<rigid_body_constraint_point> m_list_rbc_point;
    phys_free_list<rigid_body_constraint_hinge> m_list_rbc_hinge;
    phys_free_list<rigid_body_constraint_distance> m_list_rbc_dist;
    phys_free_list<rigid_body_constraint_ragdoll> m_list_rbc_ragdoll;
    phys_free_list<rigid_body_constraint_wheel> m_list_rbc_wheel;
    phys_free_list<rigid_body_constraint_custom_orientation> m_list_rbc_custom_orientation;
    phys_free_list<rigid_body_constraint_custom_path> m_list_rbc_custom_path;
    phys_free_list<rigid_body_constraint_angular_actuator> m_list_rbc_angular_actuator;
    phys_free_list<rigid_body_constraint_upright> m_list_rbc_upright;
    phys_transient_allocator m_contact_point_buffer_1;
    phys_transient_allocator m_contact_point_buffer_2;
    rigid_body **m_list_island;
    int m_list_island_count;
    int m_solver_memory_high_water;
    int m_contact_point_buffer_high_water;
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    physics_system()
    {
        this->m_outside_sub_delta_t = 0.0;
        this->m_max_delta_t = 0.051282052;
        this->m_flags = 0;
        this->m_collision_callback = 0;
        this->m_max_vel_iters = 8;
        this->m_max_vel_pos_iters = 4;
        //environment_rigid_body::set(&this->m_environment_rigid_body);
        this->m_environment_rigid_body.set();
        this->m_solver_memory_high_water = 0;
        this->m_contact_point_buffer_high_water = 0;
    }
    ~physics_system() = default;

    void validate_member(environment_rigid_body *rb);
    void solver_priority_sort(phys_transient_allocator *transient_buffer);
    void generate_partitions_and_stuff(phys_transient_allocator *transient_buffer);
    void time_step(float outside_delta_t, bool last_step);
    void frame_advance(float delta_t);

    static physics_system *create_physics_system();
    static void initialize();
    static void destroy_physics_system(physics_system *psys);
    static void shutdown();
};

struct rbint // sizeof=0x0
{
    static void __cdecl take_last_step(user_rigid_body *rb);
    static const phys_vec3 *inv_L(
        phys_vec3 *result,
        const rigid_body *rb,
        const phys_vec3 *t,
        float delta_t);
    static const phys_vec3 *mul_L(
        phys_vec3 *result,
        const rigid_body *rb,
        const phys_vec3 *t);
    static const phys_vec3 *__cdecl sub_pos(phys_vec3 *result, rigid_body *const b, const phys_vec3 *p);

    static void update_stability(rigid_body *const rb, float delta_t);
    static const phys_vec3 *__cdecl gtv(phys_vec3 *result, rigid_body *const b, const phys_vec3 *r);
    static void substep(user_rigid_body *rb, float delta_t);
    static void solver_prolog(rigid_body *rb, float delta_t);
    static void euler_integrate_velocity(rigid_body *const rb, float delta_t);
    static void euler_integrate_pos(rigid_body *const rb, float delta_t);
    static void add_vel(rigid_body *rb, const phys_vec3 *t, const phys_vec3 *a);
    static void prolog_frame_advance(user_rigid_body *rb, float delta_t);
    static void collision_prolog(user_rigid_body *rb, float delta_t);
    static void collision_prolog(rigid_body *const rb, float delta_t);
    static void take_next_step(user_rigid_body *rb, float delta_t);
    static void setup_constraint(rigid_body *rb, pulse_sum_node *psn);
};


void __cdecl IPN_partition_process(const rigid_body_constraint *rbc, int *island_count);
void __cdecl IPN_merge(rigid_body *dest, rigid_body *source);
void __cdecl IPN_verify_rigid_bodies(rigid_body *rb_partition_head);

void __cdecl rigid_body_island_qsort(rigid_body **list, int list_count);
void __cdecl merge_sort(rigid_body_constraint_contact **list, int list_count);
void __cdecl SetIdentity(phys_mat44 *m);

extern physics_system *g_physics_system;
