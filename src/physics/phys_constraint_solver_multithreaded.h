#pragma once

#include "phys_local.h"
#include "rigid_body.h"
#include "physics_system_internal.h"

class contact_point_info;

struct constraint_solver_task_input // sizeof=0x20
{
    rigid_body **m_list_island;
    int m_list_island_count;
    int *m_list_island_cur;
    // These two fields are un-used and probably leftover from a previous version. Can save some bytes in the transient allocator by removing them (alignment=4)
    //int m_psys_psc_visit_counter;
    //int m_psys_next_psc_visit_counter;
    int m_psys_max_vel_iters;
    int m_psys_max_vel_pos_iters;
    float m_outside_delta_t;
};

struct pulse_sum_node : phys_link_list_base<pulse_sum_node> // sizeof=0x80
{
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
    phys_mat44 m_world_inv_inertia;
    phys_vec3 t_vel;
    phys_vec3 a_vel;
    float m_inv_mass;
    rigid_body *m_rb;
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
};

struct pulse_sum_normal : phys_link_list_base<pulse_sum_normal> // sizeof=0xA0
{                                       // XREF: pulse_sum_wheel/r
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
    phys_vec3 m_ud;
    phys_vec3 m_b1_r;
    phys_vec3 m_b2_r;
    phys_vec3 m_b1_ap;
    phys_vec3 m_b2_ap;
    float m_pulse_sum_min;
    float m_pulse_sum_max;
    float m_pulse_sum;
    float m_right_side;
    float m_big_dirt;
    float m_cfm;
    float m_denom;
    float m_pulse_limit_ratio;
    unsigned int m_flags;
    pulse_sum_normal *m_pulse_parent;
    pulse_sum_node *m_b1;
    pulse_sum_node *m_b2;
    pulse_sum_cache *m_pulse_sum_cache;
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

    void set_object_vel(const phys_vec3 *object_vel);
    void set_object_col_pt(const phys_vec3 *object_col_pt);
    void calc_abs(const phys_vec3 *b1_r_displace);
    double get_vel();
    double get_last_vel();
    double get_pos();
    void set(
        rigid_body *const b1,
        const phys_vec3 *b1_r,
        rigid_body *const b2,
        const phys_vec3 *b2_r,
        const phys_vec3 *ud,
        pulse_sum_cache *const ps_cache,
        const phys_vec3 *b1_r_displace);
    const phys_vec3 *get_relative_velocity_change_dir(phys_vec3 *result);
    const phys_vec3 *get_relative_velocity(phys_vec3 *result);
    double get_unclamped_pulse_sum();
    double get_objective();
    void apply(const float *s_);
    double clamp_pulse_sum(float ps);
    void SOLVER_apply_relaxation(
        float *error_sq,
        const bool add_error);
    void SOLVER_solver_prolog(float delta_t);

    void set_pulse_sum_limits_parent_ratio(float limit_ratio, pulse_sum_normal *const parent);
    void setup_vel_uni_standard(float delta_t, float max_penalty_restitution_vel);

    void setup_vel_uni_standard_pos_adjust(
        float delta_t,
        float pos_adjust,
        float max_penalty_restitution_vel);
};

struct __declspec(align(8)) pulse_sum_wheel : phys_link_list_base<pulse_sum_wheel> // sizeof=0xC0
{
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
    pulse_sum_normal m_suspension;
    pulse_sum_normal *m_side;
    pulse_sum_normal *m_fwd;
    float m_side_fric_max;
    // padding byte
    // padding byte
    // padding byte
    // padding byte

    void SOLVER_solver_intermediate(float delta_t);
    bool pulse_chain_within_limits();
    char clamp_pulse_sum_pulse_chain(float *ps1_, float *ps2_);
    void addp_pulse_chain();
    void SOLVER_apply_relaxation(float *error_sq);
    void SOLVER_solver_prolog(float delta_t);

    void set_side_fwd_ratios(float side_ratio, float fwd_ratio, float side_fric_max);
};

struct pulse_sum_angular : phys_link_list_base<pulse_sum_angular> // sizeof=0x90
{
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
    phys_vec3 m_ud;
    phys_vec3 m_b1_r;
    phys_vec3 m_b2_r;
    phys_vec3 m_b1_ap;
    phys_vec3 m_b2_ap;
    float m_pulse_sum_min;
    float m_pulse_sum_max;
    float m_pulse_sum;
    float m_right_side;
    float m_big_dirt;
    float m_cfm;
    float m_denom;
    unsigned int m_flags;
    pulse_sum_node *m_b1;
    pulse_sum_node *m_b2;
    pulse_sum_cache *m_pulse_sum_cache;
    // padding byte
    // padding byte
    // padding byte
    // padding byte

    void set_object_vel(const phys_vec3 *object_vel);
    void set_object_col_pt(const phys_vec3 *object_col_pt);
    double get_vel();
    double get_pos();

    void set(
        rigid_body *const b1,
        const phys_vec3 *b1_r,
        rigid_body *const b2,
        const phys_vec3 *b2_r,
        const phys_vec3 *ud,
        pulse_sum_cache *const ps_cache);
    void calc_abs();
    double get_objective();
    void apply(const float *s_);
    double clamp_pulse_sum(float ps);
    void SOLVER_apply_relaxation(float *error_sq);

    void setup_vel_uni_standard(float delta_t, float max_penalty_restitution_vel);
};

struct pulse_sum_point : phys_link_list_base<pulse_sum_point> // sizeof=0x110
{
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
    phys_vec3 m_b1_r;
    phys_vec3 m_b2_r;
    phys_vec3 m_b1_apx;
    phys_vec3 m_b2_apx;
    phys_vec3 m_b1_apy;
    phys_vec3 m_b2_apy;
    phys_vec3 m_b1_apz;
    phys_vec3 m_b2_apz;
    phys_vec3 m_pulse_sum;
    phys_vec3 m_right_side;
    phys_vec3 m_big_dirt;
    phys_vec3 m_cr23;
    phys_vec3 m_cr31;
    phys_vec3 m_cr12;
    phys_vec3 m_denom;
    float m_cfm;
    pulse_sum_node *m_b1;
    pulse_sum_node *m_b2;
    pulse_sum_cache *m_pulse_sum_cache;

    void set_object_vel(const phys_vec3 *object_vel);
    const phys_vec3 *get_vel(phys_vec3 *result);
    const phys_vec3 *get_pos(phys_vec3 *result);

    void set(
        rigid_body *const b1,
        const phys_vec3 *b1_r,
        rigid_body *const b2,
        const phys_vec3 *b2_r,
        pulse_sum_cache *const ps_cache,
        float delta_t,
        bool is_spring,
        float spring_k,
        float damp_k);
    const phys_vec3 *get_objective(phys_vec3 *result);
    void apply(const phys_vec3 *s_);
    void SOLVER_solver_intermediate(float delta_t);
    void SOLVER_apply_relaxation(float *error_sq);
    void calc_abs_and_setup(
        float delta_t,
        bool is_spring,
        float spring_k,
        float damp_k);
    void SOLVER_solver_prolog(float delta_t);

};

struct pulse_sum_contact : phys_link_list_base<pulse_sum_contact> // sizeof=0x60
{
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
    phys_vec3 m_ud_n;
    phys_vec3 m_ud_f1;
    phys_vec3 m_ud_f2;
    float m_fric_coef;
    pulse_sum_node *m_b1;
    pulse_sum_node *m_b2;
    struct pulse_sum_contact_point *m_list_pscp;
    void *m_pulse_sum_cache_list;
    int m_list_pscp_count;
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte

    //void pulse_sum_contact::set(rigid_body *const b1, rigid_body *const b2, contact_point_info *cpi, float delta_t);
    void set(rigid_body *const b1, rigid_body *const b2, contact_point_info *cpi, float delta_t);
    void SOLVER_solver_intermediate(float delta_t);
};

struct __declspec(align(16)) pulse_sum_contact_point // sizeof=0xD0
{
    phys_vec3 m_b1_r;
    phys_vec3 m_b2_r;
    phys_vec3 m_b1_ap_n;
    phys_vec3 m_b2_ap_n;
    phys_vec3 m_b1_ap_f1;
    phys_vec3 m_b2_ap_f1;
    phys_vec3 m_b1_ap_f2;
    phys_vec3 m_b2_ap_f2;
    phys_vec3 m_pulse_sum;
    phys_vec3 m_right_side;
    float m_big_dirt;
    float m_denom_xx;
    float m_denom_yy;
    float m_denom_zz;
    float m_denom_xy;
    float m_denom_xz;
    float m_inv_yy;
    float m_inv_yz;
    float m_inv_zz;
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

    const phys_vec3 *get_vel(phys_vec3 *result, pulse_sum_contact *psc);
    double get_pos(pulse_sum_contact *psc);
    double get_impact_vel(pulse_sum_contact *psc);
    void calc_abs(pulse_sum_contact *psc);
    const phys_vec3 *get_objective(phys_vec3 *result, pulse_sum_contact *psc);
    void apply(pulse_sum_contact *psc, const phys_vec3 *s_);
    void clamp_f(pulse_sum_contact *psc);
    void SOLVER_solver_prolog(
        pulse_sum_contact *psc,
        pulse_sum_cache *m_pulse_sum_cache,
        float delta_t);
    void SOLVER_apply_relaxation(
        pulse_sum_contact *psc,
        float *error_sq);
    void setup_vel_uni_restitution(
        pulse_sum_contact *psc,
        float restitution_k,
        float max_restitution_v,
        float delta_t,
        float max_penalty_restitution_vel,
        float min_restitution_impact_vel_sq);
};

struct pulse_sum_constraint_solver // sizeof=0x84
{                                       // XREF: ?phys_jq_constraint_solver_batch_function@@YAHPAUjqBatch@@@Z/r
    struct solver_info // sizeof=0x14
    {                                       // XREF: pulse_sum_constraint_solver/r
        int m_max_vel_iters;
        int m_max_vel_pos_iters;
        float m_max_vel_error_sq;
        float m_max_vel_pos_error_sq;
        float m_delta_t;
    };

    struct __declspec(align(16)) temp_user_rigid_body : user_rigid_body, phys_link_list_base<pulse_sum_constraint_solver::temp_user_rigid_body> // sizeof=0x1D0
    {
        struct avl_tree_accessor
        {
            using key_type = user_rigid_body *;
            using node_type = pulse_sum_constraint_solver::temp_user_rigid_body;

            static bool less(const key_type &key, const node_type *node)
            {
                return key < node->m_avl_key;
            }

            static bool less(const node_type *node, const key_type &key)
            {
                return node->m_avl_key < key;
            }

            static bool equals(const node_type *node, const key_type &key)
            {
                return node->m_avl_key == key;
            }

            // Hacky aislop shit
            static void set_key(node_type *node, const key_type &key)
            {
                node->m_avl_key = key;
            }
        };
        user_rigid_body *m_avl_key;
        phys_inplace_avl_tree_node<pulse_sum_constraint_solver::temp_user_rigid_body> m_avl_tree_node;
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
    };

    struct user_rigid_body_restore_info : phys_link_list_base<pulse_sum_constraint_solver::user_rigid_body_restore_info> // sizeof=0xC
    {
        user_rigid_body **m_rbc_urb;
        user_rigid_body *m_original_urb;
    };

    float m_outside_delta_t;            // XREF: phys_jq_constraint_solver_batch_function(jqBatch *)+43/w
    int m_psys_max_vel_iters;           // XREF: phys_jq_constraint_solver_batch_function(jqBatch *)+4F/w
    int m_psys_max_vel_pos_iters;       // XREF: phys_jq_constraint_solver_batch_function(jqBatch *)+49/w
    pulse_sum_constraint_solver::solver_info m_si;
    phys_transient_allocator m_solver_memory_allocator;
                                        // XREF: phys_jq_constraint_solver_batch_function(jqBatch *):loc_A27FBA/r
    phys_link_list<pulse_sum_node> m_list_pulse_sum_node;
    int m_memory_high_water;            // XREF: phys_jq_constraint_solver_batch_function(jqBatch *)+55/w
    phys_link_list<pulse_sum_normal> m_list_pulse_sum_normal;
    phys_link_list<pulse_sum_point> m_list_pulse_sum_point;
    phys_link_list<pulse_sum_angular> m_list_pulse_sum_angular;
    phys_link_list<pulse_sum_wheel> m_list_pulse_sum_wheel;
    phys_link_list<pulse_sum_contact> m_list_pulse_sum_contact;

    ~pulse_sum_constraint_solver();
    pulse_sum_constraint_solver();

    void solve_constraints(rigid_body *const head);
    void execute_constraint_solver(rigid_body *const head);

    void solve_iterative(int max_iters, float max_error_sq);

    pulse_sum_normal *create_pulse_sum_wheel_fwd(pulse_sum_wheel *psw);
    pulse_sum_wheel *create_pulse_sum_wheel();
    pulse_sum_normal *create_pulse_sum_normal();
    pulse_sum_contact *create_pulse_sum_contact(
        rigid_body *b1,
        rigid_body *b2,
        contact_point_info *cpi,
        float delta_t);
    pulse_sum_angular *create_pulse_sum_angular(
        rigid_body *const b1,
        const phys_vec3 *b1_r,
        rigid_body *const b2,
        const phys_vec3 *b2_r,
        const phys_vec3 *ud,
        pulse_sum_cache *const ps_cache);
    void create_point(
        rigid_body *const b1,
        const phys_vec3 *b1_r,
        rigid_body *const b2,
        const phys_vec3 *b2_r,
        pulse_sum_cache *const ps_cache,
        float delta_t,
        bool is_spring,
        float spring_k,
        float damp_k);
    void create_hinge(
        rigid_body *const b1,
        const phys_vec3 *b1_axis,
        rigid_body *const b2,
        const phys_vec3 *b2_axis,
        const phys_vec3 *a1,
        const phys_vec3 *a2,
        pulse_sum_cache *const ps_cache,
        float delta_t);

    void add_urb(
        phys_inplace_avl_tree<user_rigid_body *, pulse_sum_constraint_solver::temp_user_rigid_body, pulse_sum_constraint_solver::temp_user_rigid_body::avl_tree_accessor> *turb_search_tree,
        phys_simple_link_list<pulse_sum_constraint_solver::temp_user_rigid_body> *list_turb,
        phys_simple_link_list<pulse_sum_constraint_solver::user_rigid_body_restore_info> *list_urbri,
        rigid_body_constraint *rbc);
};




void __cdecl orthonormalize(phys_mat44 *mat);
void __cdecl constraint_solver_process(
                phys_transient_allocator *transient_buffer,
                physics_system *psys,
                float outside_delta_t);
int __cdecl phys_jq_constraint_solver_batch_function(struct jqBatch *pBatch);