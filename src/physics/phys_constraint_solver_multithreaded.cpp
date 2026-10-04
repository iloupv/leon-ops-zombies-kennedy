#include "phys_constraint_solver_multithreaded.h"
#include "phys_util.h"
#include <tl/physics/rbc_def_generic.h>
#include "phys_task_manager.h"
#include "phys_assert.h"

// no workercmd
jqModule phys_jq_constraint_solverModule=
{
  "phys_jq_constraint_solver",
  JQ_WORKER_GENERIC,
  &phys_jq_constraint_solver_batch_function,
  //{ { { 0, 0 } } }
};


void __cdecl orthonormalize(phys_mat44 *mat)
{
    double v2; // st7
    double v3; // st6
    double v4; // rt2
    double v5; // st5
    float v6; // [esp-20h] [ebp-2Ch]
    float v7; // [esp-20h] [ebp-2Ch]
    float v8; // [esp-1Ch] [ebp-28h]
    float v9; // [esp-1Ch] [ebp-28h]
    float v10; // [esp-18h] [ebp-24h]
    float v11; // [esp-18h] [ebp-24h]
    float v12; // [esp-4h] [ebp-10h]
    float v13; // [esp-4h] [ebp-10h]
    float v14; // [esp-4h] [ebp-10h]
    float v15; // [esp-4h] [ebp-10h]
    float v16; // [esp-4h] [ebp-10h]
    float v17; // [esp-4h] [ebp-10h]
    float v18; // [esp-4h] [ebp-10h]
    float v19; // [esp-4h] [ebp-10h]
    float v20; // [esp-4h] [ebp-10h]
    float v21; // [esp-4h] [ebp-10h]

    v12 = mat->x.y * mat->x.y + mat->x.x * mat->x.x + mat->x.z * mat->x.z;
    v13 = sqrt(v12);
    v14 = 1.0 / v13;
    v2 = v14;
    v15 = mat->x.x * v14;
    v3 = v15;
    mat->x.x = v15;
    v16 = v2 * mat->x.y;
    mat->x.y = v16;
    v4 = v16;
    v17 = v2 * mat->x.z;
    v5 = v17;
    mat->x.z = v17;
    v18 = mat->y.y * v4 + v3 * mat->y.x + mat->y.z * v17;
    v6 = v3 * v18;
    v8 = v4 * v18;
    v10 = v5 * v18;
    mat->y.x = mat->y.x - v6;
    mat->y.y = mat->y.y - v8;
    mat->y.z = mat->y.z - v10;
    v19 = mat->y.y * mat->y.y + mat->y.x * mat->y.x + mat->y.z * mat->y.z;
    v20 = sqrt(v19);
    v21 = 1.0 / v20;
    mat->y.x = mat->y.x * v21;
    mat->y.y = mat->y.y * v21;
    mat->y.z = v21 * mat->y.z;
    v7 = mat->y.z * mat->x.y - mat->y.y * mat->x.z;
    v9 = mat->x.z * mat->y.x - mat->y.z * mat->x.x;
    v11 = mat->y.y * mat->x.x - mat->y.x * mat->x.y;
    mat->z.x = v7;
    mat->z.y = v9;
    mat->z.z = v11;
}

void __thiscall pulse_sum_normal::set_object_vel(const phys_vec3 *object_vel)
{
    if ( this->m_b2
        && _tlAssert(
                 "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\constraint_solver\\pulse_sum_normal.h",
                 69,
                 "m_b2 == NULL",
                 "") )
    {
        __debugbreak();
    }
    this->m_b2_ap.x = object_vel->x;
    this->m_b2_ap.y = object_vel->y;
    this->m_b2_ap.z = object_vel->z;
}

void __thiscall pulse_sum_normal::set_object_col_pt(const phys_vec3 *object_col_pt)
{
    if ( this->m_b2
        && _tlAssert(
                 "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\constraint_solver\\pulse_sum_normal.h",
                 72,
                 "m_b2 == NULL",
                 "") )
    {
        __debugbreak();
    }
    this->m_b2_r.x = object_col_pt->x;
    this->m_b2_r.y = object_col_pt->y;
    this->m_b2_r.z = object_col_pt->z;
}

void __thiscall pulse_sum_point::set_object_vel(const phys_vec3 *object_vel)
{
    if ( this->m_b2
        && _tlAssert(
                 "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\constraint_solver\\pulse_sum_point.h",
                 46,
                 "m_b2 == NULL",
                 "") )
    {
        __debugbreak();
    }
    this->m_b2_apx.x = object_vel->x;
    this->m_b2_apx.y = object_vel->y;
    this->m_b2_apx.z = object_vel->z;
}

void __thiscall pulse_sum_angular::set_object_vel(const phys_vec3 *object_vel)
{
    if ( this->m_b2
        && _tlAssert(
                 "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\constraint_solver\\pulse_sum_angular.h",
                 45,
                 "m_b2 == NULL",
                 "") )
    {
        __debugbreak();
    }
    this->m_b2_ap.x = object_vel->x;
    this->m_b2_ap.y = object_vel->y;
    this->m_b2_ap.z = object_vel->z;
}

void __thiscall pulse_sum_angular::set_object_col_pt(const phys_vec3 *object_col_pt)
{
    if ( this->m_b2
        && _tlAssert(
                 "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\constraint_solver\\pulse_sum_angular.h",
                 47,
                 "m_b2 == NULL",
                 "") )
    {
        __debugbreak();
    }
    this->m_b2_r.x = object_col_pt->x;
    this->m_b2_r.y = object_col_pt->y;
    this->m_b2_r.z = object_col_pt->z;
}

rigid_body &rigid_body::operator=(const rigid_body *__that)
{
    rigid_body *result; // eax

    result = this;
    this->m_last_position.x = __that->m_last_position.x;
    this->m_last_position.y = __that->m_last_position.y;
    this->m_last_position.z = __that->m_last_position.z;
    this->m_moved_vec.x = __that->m_moved_vec.x;
    this->m_moved_vec.y = __that->m_moved_vec.y;
    this->m_moved_vec.z = __that->m_moved_vec.z;
    this->m_smallest_lambda = __that->m_smallest_lambda;
    this->m_mat.x.x = __that->m_mat.x.x;
    this->m_mat.x.y = __that->m_mat.x.y;
    this->m_mat.x.z = __that->m_mat.x.z;
    this->m_mat.y.x = __that->m_mat.y.x;
    this->m_mat.y.y = __that->m_mat.y.y;
    this->m_mat.y.z = __that->m_mat.y.z;
    this->m_mat.z.x = __that->m_mat.z.x;
    this->m_mat.z.y = __that->m_mat.z.y;
    this->m_mat.z.z = __that->m_mat.z.z;
    this->m_mat.w.x = __that->m_mat.w.x;
    this->m_mat.w.y = __that->m_mat.w.y;
    this->m_mat.w.z = __that->m_mat.w.z;
    this->m_inv_inertia.x = __that->m_inv_inertia.x;
    this->m_inv_inertia.y = __that->m_inv_inertia.y;
    this->m_inv_inertia.z = __that->m_inv_inertia.z;
    this->m_gravity_acc_vec.x = __that->m_gravity_acc_vec.x;
    this->m_gravity_acc_vec.y = __that->m_gravity_acc_vec.y;
    this->m_gravity_acc_vec.z = __that->m_gravity_acc_vec.z;
    this->m_t_vel.x = __that->m_t_vel.x;
    this->m_t_vel.y = __that->m_t_vel.y;
    this->m_t_vel.z = __that->m_t_vel.z;
    this->m_a_vel.x = __that->m_a_vel.x;
    this->m_a_vel.y = __that->m_a_vel.y;
    this->m_a_vel.z = __that->m_a_vel.z;
    this->m_last_t_vel.x = __that->m_last_t_vel.x;
    this->m_last_t_vel.y = __that->m_last_t_vel.y;
    this->m_last_t_vel.z = __that->m_last_t_vel.z;
    this->m_last_a_vel.x = __that->m_last_a_vel.x;
    this->m_last_a_vel.y = __that->m_last_a_vel.y;
    this->m_last_a_vel.z = __that->m_last_a_vel.z;
    this->m_force_sum.x = __that->m_force_sum.x;
    this->m_force_sum.y = __that->m_force_sum.y;
    this->m_force_sum.z = __that->m_force_sum.z;
    this->m_torque_sum.x = __that->m_torque_sum.x;
    this->m_torque_sum.y = __that->m_torque_sum.y;
    this->m_torque_sum.z = __that->m_torque_sum.z;
    this->m_inv_mass = __that->m_inv_mass;
    this->m_max_avel = __that->m_max_avel;
    this->m_max_delta_t = __that->m_max_delta_t;
    this->m_flags = __that->m_flags;
    this->m_tick = __that->m_tick;
    this->m_node = __that->m_node;
    this->m_constraint_count = __that->m_constraint_count;
    this->m_contact_count = __that->m_contact_count;
    this->m_stable_min_contact_count = __that->m_stable_min_contact_count;
    this->m_stable_energy_time = __that->m_stable_energy_time;
    this->m_largest_vel_sq = __that->m_largest_vel_sq;
    this->m_t_drag_coef = __that->m_t_drag_coef;
    this->m_a_drag_coef = __that->m_a_drag_coef;
    this->m_userdata = __that->m_userdata;
    memcpy(&this->m_partition_node, &__that->m_partition_node, sizeof(this->m_partition_node));
    return *result;
}

void __cdecl nuge::tensor_transform_principle(const phys_vec3 *diag, const phys_mat44 *mat, phys_mat44 *tensor)
{
    double v3; // st7
    double v4; // st5
    double v5; // st6
    double v6; // st3
    double v7; // st2
    double v8; // st4
    float v9; // [esp-Ch] [ebp-8Ch]
    float v10; // [esp-8h] [ebp-88h]
    float v11; // [esp-4h] [ebp-84h]
    float y__4; // [esp+4h] [ebp-7Ch]
    float y__8; // [esp+8h] [ebp-78h]
    float y__12; // [esp+Ch] [ebp-74h]
    float x__4; // [esp+14h] [ebp-6Ch]
    float x__4a; // [esp+14h] [ebp-6Ch]
    float x__4b; // [esp+14h] [ebp-6Ch]
    float x__4c; // [esp+14h] [ebp-6Ch]
    float x__8; // [esp+18h] [ebp-68h]
    float x__8a; // [esp+18h] [ebp-68h]
    float x__8b; // [esp+18h] [ebp-68h]
    float x__8c; // [esp+18h] [ebp-68h]
    float x__12; // [esp+1Ch] [ebp-64h]
    float x__12a; // [esp+1Ch] [ebp-64h]
    float x__12b; // [esp+1Ch] [ebp-64h]
    float x__12c; // [esp+1Ch] [ebp-64h]
    float z__4; // [esp+24h] [ebp-5Ch]
    float z__4a; // [esp+24h] [ebp-5Ch]
    float z__4b; // [esp+24h] [ebp-5Ch]
    float z__8; // [esp+28h] [ebp-58h]
    float z__8a; // [esp+28h] [ebp-58h]
    float z__8b; // [esp+28h] [ebp-58h]
    float z__12; // [esp+2Ch] [ebp-54h]
    float z__12a; // [esp+2Ch] [ebp-54h]
    float z__12b; // [esp+2Ch] [ebp-54h]
    float v36; // [esp+34h] [ebp-4Ch]
    float v37; // [esp+34h] [ebp-4Ch]
    float v38; // [esp+34h] [ebp-4Ch]
    float v39; // [esp+34h] [ebp-4Ch]
    float v40; // [esp+38h] [ebp-48h]
    float v41; // [esp+38h] [ebp-48h]
    float v42; // [esp+38h] [ebp-48h]
    float v43; // [esp+38h] [ebp-48h]
    float v44; // [esp+3Ch] [ebp-44h]
    float v45; // [esp+3Ch] [ebp-44h]
    float v46; // [esp+3Ch] [ebp-44h]
    float v47; // [esp+3Ch] [ebp-44h]
    float left_yrow_4; // [esp+44h] [ebp-3Ch]
    float left_yrow_4a; // [esp+44h] [ebp-3Ch]
    float left_yrow_4b; // [esp+44h] [ebp-3Ch]
    float left_yrow_4c; // [esp+44h] [ebp-3Ch]
    float left_yrow_8; // [esp+48h] [ebp-38h]
    float left_yrow_8a; // [esp+48h] [ebp-38h]
    float left_yrow_8b; // [esp+48h] [ebp-38h]
    float left_yrow_8c; // [esp+48h] [ebp-38h]
    float left_yrow_12; // [esp+4Ch] [ebp-34h]
    float left_yrow_12a; // [esp+4Ch] [ebp-34h]
    float left_yrow_12b; // [esp+4Ch] [ebp-34h]
    float left_yrow_12c; // [esp+4Ch] [ebp-34h]
    float left_zrow_4; // [esp+54h] [ebp-2Ch]
    float left_zrow_8; // [esp+58h] [ebp-28h]
    float left_zrow_12; // [esp+5Ch] [ebp-24h]
    float y; // [esp+70h] [ebp-10h]
    float z; // [esp+70h] [ebp-10h]
    float x; // [esp+70h] [ebp-10h]
    float v66; // [esp+70h] [ebp-10h]
    float v67; // [esp+70h] [ebp-10h]
    float v68; // [esp+70h] [ebp-10h]
    float v69; // [esp+70h] [ebp-10h]
    float v70; // [esp+70h] [ebp-10h]
    float v71; // [esp+70h] [ebp-10h]
    float v72; // [esp+70h] [ebp-10h]

    left_zrow_4 = mat->x.x * diag->x;
    left_zrow_8 = diag->x * mat->x.y;
    left_zrow_12 = diag->x * mat->x.z;
    y = diag->y;
    v36 = y * mat->y.x;
    v40 = mat->y.y * y;
    v44 = y * mat->y.z;
    z = diag->z;
    left_yrow_4 = mat->z.x * z;
    left_yrow_8 = mat->z.y * z;
    left_yrow_12 = z * mat->z.z;
    x = mat->z.x;
    v3 = left_yrow_4;
    x__4 = left_yrow_4 * x;
    v4 = left_yrow_8;
    x__8 = left_yrow_8 * x;
    v5 = left_yrow_12;
    x__12 = x * left_yrow_12;
    v66 = mat->y.x;
    v6 = v36;
    left_yrow_4a = v66 * v36;
    v7 = v40;
    left_yrow_8a = v40 * v66;
    v8 = v44;
    left_yrow_12a = v66 * v44;
    v37 = mat->x.x * left_zrow_4;
    v41 = left_zrow_8 * mat->x.x;
    v45 = left_zrow_12 * mat->x.x;
    z__4 = v37 + left_yrow_4a;
    z__8 = v41 + left_yrow_8a;
    z__12 = v45 + left_yrow_12a;
    y__4 = z__4 + x__4;
    y__8 = z__8 + x__8;
    y__12 = z__12 + x__12;
    v67 = mat->z.y;
    left_yrow_4b = v67 * v3;
    left_yrow_8b = v4 * v67;
    left_yrow_12b = v5 * v67;
    v68 = mat->y.y;
    z__4a = v68 * v6;
    z__8a = v7 * v68;
    z__12a = v8 * v68;
    v69 = mat->x.y;
    x__4a = v69 * left_zrow_4;
    x__8a = left_zrow_8 * v69;
    x__12a = left_zrow_12 * v69;
    v38 = x__4a + z__4a;
    v42 = x__8a + z__8a;
    v46 = x__12a + z__12a;
    v9 = v38 + left_yrow_4b;
    v10 = v42 + left_yrow_8b;
    v11 = v46 + left_yrow_12b;
    v70 = mat->z.z;
    left_yrow_4c = v3 * v70;
    left_yrow_8c = v4 * v70;
    left_yrow_12c = v70 * v5;
    v71 = mat->y.z;
    z__4b = v6 * v71;
    z__8b = v7 * v71;
    z__12b = v8 * v71;
    v72 = mat->x.z;
    x__4b = v72 * left_zrow_4;
    x__8b = left_zrow_8 * v72;
    x__12b = v72 * left_zrow_12;
    v39 = x__4b + z__4b;
    v43 = x__8b + z__8b;
    v47 = x__12b + z__12b;
    x__4c = v39 + left_yrow_4c;
    x__8c = v43 + left_yrow_8c;
    x__12c = v47 + left_yrow_12c;
    tensor->x.x = y__4;
    tensor->x.y = y__8;
    tensor->x.z = y__12;
    tensor->y.x = v9;
    tensor->y.y = v10;
    tensor->y.z = v11;
    tensor->z.x = x__4c;
    tensor->z.y = x__8c;
    tensor->z.z = x__12c;
}

const phys_vec3 * rbint::inv_L(
                phys_vec3 *result,
                const rigid_body *rb,
                const phys_vec3 *t,
                float delta_t)
{
    const phys_vec3 *v5; // eax
    double v6; // st7
    const phys_vec3 *v7; // eax
    phys_vec3 v8; // [esp-10h] [ebp-1Ch] BYREF
    //int v9; // [esp+0h] [ebp-Ch]
    //void *v10; // [esp+4h] [ebp-8h]
    //void *retaddr; // [esp+Ch] [ebp+0h]
    //
    //v9 = a1;
    //v10 = retaddr;
    v5 = phys_multiply(&v8, &rb->m_node->m_world_inv_inertia, t);
    result->x = v5->x * delta_t;
    result->y = v5->y * delta_t;
    v6 = delta_t * v5->z;
    v7 = result;
    result->z = v6;
    return v7;
}

void __cdecl rbint::update_stability(rigid_body *const rb, float delta_t)
{
    int m_stable_min_contact_count; // esi
    unsigned int m_flags; // edx
    float max_stable_energy_sq; // [esp+4h] [ebp-4h]
    float rba; // [esp+10h] [ebp+8h]

    max_stable_energy_sq = 141.60999;
    m_stable_min_contact_count = rb->m_stable_min_contact_count;
    if ( !m_stable_min_contact_count )
        max_stable_energy_sq = 29.593601;
    if ( max_stable_energy_sq >= (double)rb->m_largest_vel_sq )
    {
        m_flags = rb->m_flags;
        if ( (m_flags & 4) == 0 )
        {
            rba = rb->m_stable_energy_time + delta_t;
            rb->m_stable_energy_time = rba;
            if ( rba >= 1.0 )
            {
                rb->m_flags = m_flags | 4;
            }
            else if ( rba >= 0.5 && rb->m_contact_count >= m_stable_min_contact_count )
            {
                rb->m_flags = m_flags | 4;
            }
        }
    }
    else
    {
        rb->m_flags &= ~4u;
        rb->m_stable_energy_time = 0.0;
    }
}

const phys_vec3 *__cdecl rbint::gtv(phys_vec3 *result, rigid_body *const b, const phys_vec3 *r)
{
    const phys_vec3 *v3; // eax
    float v4; // [esp-10h] [ebp-1Ch]
    float v5; // [esp-Ch] [ebp-18h]
    float v6; // [esp-8h] [ebp-14h]

    v4 = b->m_a_vel.y * r->z - b->m_a_vel.z * r->y;
    v5 = b->m_a_vel.z * r->x - b->m_a_vel.x * r->z;
    v3 = result;
    v6 = b->m_a_vel.x * r->y - b->m_a_vel.y * r->x;
    result->x = b->m_t_vel.x + v4;
    result->y = b->m_t_vel.y + v5;
    result->z = b->m_t_vel.z + v6;
    return v3;
}

void pulse_sum_normal::calc_abs(const phys_vec3 *b1_r_displace)
{
    double x; // st7
    phys_mat44 *p_m_world_inv_inertia; // ecx
    const phys_vec3 *v6; // eax
    pulse_sum_node *m_b1; // eax
    double v8; // st7
    pulse_sum_node *m_b2; // eax
    const phys_vec3 *v10; // eax
    pulse_sum_node *v11; // eax
    phys_vec3 v12; // [esp-1Ch] [ebp-3Ch] BYREF
    phys_vec3 v13; // [esp-Ch] [ebp-2Ch] BYREF
    float v14; // [esp+10h] [ebp-10h]
    //int v15; // [esp+14h] [ebp-Ch]
    //void *v16; // [esp+18h] [ebp-8h]
    //void *retaddr; // [esp+20h] [ebp+0h]
    //
    //v15 = a2;
    //v16 = retaddr;
    x = this->m_b1_r.x;
    p_m_world_inv_inertia = &this->m_b1->m_world_inv_inertia;
    v13.x = x + b1_r_displace->x;
    v13.y = this->m_b1_r.y + b1_r_displace->y;
    v13.z = this->m_b1_r.z + b1_r_displace->z;
    v12.x = this->m_ud.z * v13.y - v13.z * this->m_ud.y;
    v12.y = v13.z * this->m_ud.x - this->m_ud.z * v13.x;
    v12.z = v13.x * this->m_ud.y - v13.y * this->m_ud.x;
    v6 = phys_multiply(&v13, p_m_world_inv_inertia, &v12);
    this->m_b1_ap.x = v6->x;
    this->m_b1_ap.y = v6->y;
    this->m_b1_ap.z = v6->z;
    m_b1 = this->m_b1;
    v13.x = this->m_b1_ap.y * this->m_b1_r.z - this->m_b1_ap.z * this->m_b1_r.y;
    v13.y = this->m_b1_ap.z * this->m_b1_r.x - this->m_b1_ap.x * this->m_b1_r.z;
    v13.z = this->m_b1_r.y * this->m_b1_ap.x - this->m_b1_ap.y * this->m_b1_r.x;
    v14 = this->m_ud.y * v13.y + this->m_ud.x * v13.x + this->m_ud.z * v13.z;
    v8 = v14 + m_b1->m_inv_mass;
    m_b2 = this->m_b2;
    this->m_denom = v8;
    if (m_b2)
    {
        v13.x = this->m_b2_r.y * this->m_ud.z - this->m_b2_r.z * this->m_ud.y;
        v13.y = this->m_b2_r.z * this->m_ud.x - this->m_b2_r.x * this->m_ud.z;
        v13.z = this->m_ud.y * this->m_b2_r.x - this->m_b2_r.y * this->m_ud.x;
        v10 = phys_multiply(&v12, &m_b2->m_world_inv_inertia, &v13);
        this->m_b2_ap.x = v10->x;
        this->m_b2_ap.y = v10->y;
        this->m_b2_ap.z = v10->z;
        v11 = this->m_b2;
        v14 = this->m_b2_ap.y * v13.y + this->m_b2_ap.x * v13.x + this->m_b2_ap.z * v13.z;
        this->m_denom = v14 + v11->m_inv_mass + this->m_denom;
    }
    if (this->m_denom <= 0.0000099999997)
    {
        if (_tlAssert(
            "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\constraint_solver\\pulse_sum_normal_inline.h",
            11,
            "m_denom > 0.00001f",
            ""))
        {
            __debugbreak();
        }
    }
}

double    pulse_sum_normal::get_vel()
{
    rigid_body *m_rb; // eax
    phys_vec3 *p_m_b2_ap; // edx
    rigid_body *v4; // eax
    float v6[4]; // [esp-40h] [ebp-4Ch] BYREF
    float v7; // [esp-30h] [ebp-3Ch]
    float v8; // [esp-2Ch] [ebp-38h]
    float v9; // [esp-28h] [ebp-34h]
    float v10; // [esp-20h] [ebp-2Ch]
    float v11; // [esp-1Ch] [ebp-28h]
    float v12; // [esp-18h] [ebp-24h]
    //int v14; // [esp+0h] [ebp-Ch]
    //void *v15; // [esp+4h] [ebp-8h]
    //void *retaddr; // [esp+Ch] [ebp+0h]
    //
    //v14 = a2;
    //v15 = retaddr;
    if ( this->m_b2 )
    {
        m_rb = this->m_b2->m_rb;
        p_m_b2_ap = (phys_vec3 *)v6;
        v10 = this->m_b2_r.z * m_rb->m_a_vel.y - this->m_b2_r.y * m_rb->m_a_vel.z;
        v11 = this->m_b2_r.x * m_rb->m_a_vel.z - this->m_b2_r.z * m_rb->m_a_vel.x;
        v12 = this->m_b2_r.y * m_rb->m_a_vel.x - m_rb->m_a_vel.y * this->m_b2_r.x;
        v6[0] = m_rb->m_t_vel.x + v10;
        v6[1] = m_rb->m_t_vel.y + v11;
        v6[2] = m_rb->m_t_vel.z + v12;
    }
    else
    {
        p_m_b2_ap = &this->m_b2_ap;
    }
    v4 = this->m_b1->m_rb;
    v10 = this->m_b1_r.z * v4->m_a_vel.y - this->m_b1_r.y * v4->m_a_vel.z;
    v11 = this->m_b1_r.x * v4->m_a_vel.z - this->m_b1_r.z * v4->m_a_vel.x;
    v12 = v4->m_a_vel.x * this->m_b1_r.y - v4->m_a_vel.y * this->m_b1_r.x;
    v7 = v4->m_t_vel.x + v10;
    v8 = v4->m_t_vel.y + v11;
    v9 = v4->m_t_vel.z + v12;
    v10 = v7 - p_m_b2_ap->x;
    v11 = v8 - p_m_b2_ap->y;
    v12 = v9 - p_m_b2_ap->z;
    return (float)(this->m_ud.y * v11 + this->m_ud.x * v10 + this->m_ud.z * v12);
}

double    pulse_sum_normal::get_last_vel()
{
    rigid_body *m_rb; // eax
    phys_vec3 *p_m_b2_ap; // edx
    rigid_body *v4; // eax
    float v6[4]; // [esp-40h] [ebp-4Ch] BYREF
    float v7; // [esp-30h] [ebp-3Ch]
    float v8; // [esp-2Ch] [ebp-38h]
    float v9; // [esp-28h] [ebp-34h]
    float v10; // [esp-20h] [ebp-2Ch]
    float v11; // [esp-1Ch] [ebp-28h]
    float v12; // [esp-18h] [ebp-24h]
    //int v14; // [esp+0h] [ebp-Ch]
    //void *v15; // [esp+4h] [ebp-8h]
    //void *retaddr; // [esp+Ch] [ebp+0h]
    //
    //v14 = a2;
    //v15 = retaddr;
    if ( this->m_b2 )
    {
        m_rb = this->m_b2->m_rb;
        p_m_b2_ap = (phys_vec3 *)v6;
        v10 = this->m_b2_r.z * m_rb->m_last_a_vel.y - this->m_b2_r.y * m_rb->m_last_a_vel.z;
        v11 = this->m_b2_r.x * m_rb->m_last_a_vel.z - this->m_b2_r.z * m_rb->m_last_a_vel.x;
        v12 = this->m_b2_r.y * m_rb->m_last_a_vel.x - m_rb->m_last_a_vel.y * this->m_b2_r.x;
        v6[0] = m_rb->m_last_t_vel.x + v10;
        v6[1] = m_rb->m_last_t_vel.y + v11;
        v6[2] = m_rb->m_last_t_vel.z + v12;
    }
    else
    {
        p_m_b2_ap = &this->m_b2_ap;
    }
    v4 = this->m_b1->m_rb;
    v10 = this->m_b1_r.z * v4->m_last_a_vel.y - this->m_b1_r.y * v4->m_last_a_vel.z;
    v11 = this->m_b1_r.x * v4->m_last_a_vel.z - this->m_b1_r.z * v4->m_last_a_vel.x;
    v12 = v4->m_last_a_vel.x * this->m_b1_r.y - v4->m_last_a_vel.y * this->m_b1_r.x;
    v7 = v4->m_last_t_vel.x + v10;
    v8 = v4->m_last_t_vel.y + v11;
    v9 = v4->m_last_t_vel.z + v12;
    v10 = v7 - p_m_b2_ap->x;
    v11 = v8 - p_m_b2_ap->y;
    v12 = v9 - p_m_b2_ap->z;
    return (float)(this->m_ud.y * v11 + this->m_ud.x * v10 + this->m_ud.z * v12);
}

double pulse_sum_normal::get_pos()
{
    pulse_sum_node *m_b2; // eax
    float *p_x; // eax
    double v4; // st7
    phys_vec3 *p_m_b2_r; // edx
    rigid_body *m_rb; // eax
    double x; // st7
    float v9[4]; // [esp-40h] [ebp-4Ch] BYREF
    float v10; // [esp-30h] [ebp-3Ch]
    float v11; // [esp-2Ch] [ebp-38h]
    float v12; // [esp-28h] [ebp-34h]
    float v13; // [esp-20h] [ebp-2Ch]
    float v14; // [esp-1Ch] [ebp-28h]
    float v15; // [esp-18h] [ebp-24h]
    //int v17; // [esp+0h] [ebp-Ch]
    //void *v18; // [esp+4h] [ebp-8h]
    //void *retaddr; // [esp+Ch] [ebp+0h]
    //
    //v17 = a2;
    //v18 = retaddr;
    m_b2 = this->m_b2;
    if ( m_b2 )
    {
        p_x = &m_b2->m_rb->m_last_position.x;
        v4 = this->m_b2_r.x + p_x[24];
        p_x += 24;
        p_m_b2_r = (phys_vec3 *)v9;
        v9[0] = v4;
        v9[1] = p_x[1] + this->m_b2_r.y;
        v9[2] = p_x[2] + this->m_b2_r.z;
    }
    else
    {
        p_m_b2_r = &this->m_b2_r;
    }
    m_rb = this->m_b1->m_rb;
    x = m_rb->m_mat.w.x;
    m_rb = (rigid_body *)((char *)m_rb + 96);
    v13 = x + this->m_b1_r.x;
    v14 = m_rb->m_last_position.y + this->m_b1_r.y;
    v15 = m_rb->m_last_position.z + this->m_b1_r.z;
    v10 = v13 - p_m_b2_r->x;
    v11 = v14 - p_m_b2_r->y;
    v12 = v15 - p_m_b2_r->z;
    return (float)(this->m_ud.y * v11 + v10 * this->m_ud.x + this->m_ud.z * v12);
}

void pulse_sum_normal::set(
                rigid_body *const b1,
                const phys_vec3 *b1_r,
                rigid_body *const b2,
                const phys_vec3 *b2_r,
                const phys_vec3 *ud,
                pulse_sum_cache *const ps_cache,
                const phys_vec3 *b1_r_displace)
{
    rigid_body *v10; // ecx
    int v11; // eax
    unsigned int v12; // eax
    pulse_sum_node *m_node; // eax
    pulse_sum_node *v14; // eax
    const phys_vec3 *v15; // eax
    double z; // st7
    const phys_vec3 *v17; // eax
    bool v18; // zf
    phys_vec3 v19; // [esp-10h] [ebp-1Ch] BYREF
    //unsigned int v20[3]; // [esp+0h] [ebp-Ch] BYREF
    //_UNKNOWN *retaddr; // [esp+Ch] [ebp+0h]
    //
    //v20[0] = a2;
    //v20[1] = retaddr;
    if ( !ps_cache
        && _tlAssert(
                 "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\constraint_solver\\pulse_sum_normal_inline.h",
                 117,
                 "ps_cache",
                 "") )
    {
        __debugbreak();
    }
    if ( !b1
        && _tlAssert(
                 "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\constraint_solver\\pulse_sum_normal_inline.h",
                 118,
                 "b1",
                 "") )
    {
        __debugbreak();
    }
    if ( !b2
        && _tlAssert(
                 "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\constraint_solver\\pulse_sum_normal_inline.h",
                 119,
                 "b2",
                 "") )
    {
        __debugbreak();
    }
    if ( b1 == b2
        && _tlAssert(
                 "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\constraint_solver\\pulse_sum_normal_inline.h",
                 120,
                 "b1 != b2",
                 "") )
    {
        __debugbreak();
    }
    PHYS_ASSERT_UNIT(ud);
    v10 = b1;
    if ( b1->m_node )
        goto LABEL_19;
    if ( !b2->m_node )
    {
        if ( _tlAssert(
                     "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\constraint_solver\\pulse_sum_normal_inline.h",
                     122,
                     "rbint::get_pulse_sum_node(b1) || rbint::get_pulse_sum_node(b2)",
                     "") )
        {
            __debugbreak();
        }
        v10 = b1;
    }
    if ( v10->m_node )
LABEL_19:
        v11 = (v10->m_flags & 0x30) == 0;
    else
        v11 = v10->m_flags & 0x30;
    if ( !v11 || (!b2->m_node ? (v12 = b2->m_flags & 0x30) : (v12 = (b2->m_flags & 0x30) == 0), !v12) )
    {
        if ( _tlAssert(
                     "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\constraint_solver\\pulse_sum_normal_inline.h",
                     123,
                     "rbint::verify_pulse_sum_node(b1) && rbint::verify_pulse_sum_node(b2)",
                     "") )
        {
            __debugbreak();
        }
        v10 = b1;
    }
    m_node = v10->m_node;
    if ( m_node )
    {
        this->m_b1 = m_node;
        this->m_b1_r.x = b1_r->x;
        this->m_b1_r.y = b1_r->y;
        this->m_b1_r.z = b1_r->z;
        v14 = b2->m_node;
        if ( v14 )
        {
            this->m_b2 = v14;
            this->m_b2_r.x = b2_r->x;
            this->m_b2_r.y = b2_r->y;
            this->m_b2_r.z = b2_r->z;
        }
        else
        {
            this->m_b2 = 0;
            v15 = rbint::gtv(&v19, b2, b2_r);
            pulse_sum_normal::set_object_vel(v15);
            v19.x = b2_r->x + b2->m_mat.w.x;
            v19.y = b2->m_mat.w.y + b2_r->y;
            v19.z = b2->m_mat.w.z + b2_r->z;
            pulse_sum_normal::set_object_col_pt(&v19);
        }
        this->m_ud.x = ud->x;
        this->m_ud.y = ud->y;
        z = ud->z;
    }
    else
    {
        this->m_b1 = b2->m_node;
        this->m_b1_r.x = b2_r->x;
        this->m_b1_r.y = b2_r->y;
        this->m_b1_r.z = b2_r->z;
        this->m_b2 = 0;
        v17 = rbint::gtv(&v19, v10, b1_r);
        pulse_sum_normal::set_object_vel(v17);
        v18 = this->m_b2 == 0;
        v19.x = b1_r->x + b1->m_mat.w.x;
        v19.y = b1->m_mat.w.y + b1_r->y;
        v19.z = b1->m_mat.w.z + b1_r->z;
        if ( !v18
            && _tlAssert(
                     "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\constraint_solver\\pulse_sum_normal.h",
                     72,
                     "m_b2 == NULL",
                     "") )
        {
            __debugbreak();
        }
        this->m_b2_r.x = v19.x;
        this->m_b2_r.y = v19.y;
        this->m_b2_r.z = v19.z;
        v19.x = -ud->x;
        v19.y = -ud->y;
        v19.z = -ud->z;
        this->m_ud.x = v19.x;
        this->m_ud.y = v19.y;
        z = v19.z;
    }
    this->m_ud.z = z;
    this->m_pulse_sum_cache = ps_cache;
    this->m_flags = 0;
    pulse_sum_normal::calc_abs(b1_r_displace);
}

const phys_vec3 *__thiscall pulse_sum_normal::get_relative_velocity_change_dir(phys_vec3 *result)
{
    pulse_sum_node *m_b2; // edx
    const phys_vec3 *v3; // eax
    double v4; // st7
    double v5; // st6
    double v6; // st5
    float v7; // [esp-40h] [ebp-4Ch]
    float v8; // [esp-3Ch] [ebp-48h]
    float v9; // [esp-38h] [ebp-44h]
    float v10; // [esp-30h] [ebp-3Ch]
    float v11; // [esp-30h] [ebp-3Ch]
    float v12; // [esp-2Ch] [ebp-38h]
    float v13; // [esp-2Ch] [ebp-38h]
    float v14; // [esp-28h] [ebp-34h]
    float v15; // [esp-28h] [ebp-34h]
    float v16; // [esp-20h] [ebp-2Ch]
    float v17; // [esp-20h] [ebp-2Ch]
    float v18; // [esp-1Ch] [ebp-28h]
    float v19; // [esp-1Ch] [ebp-28h]
    float v20; // [esp-18h] [ebp-24h]
    float v21; // [esp-18h] [ebp-24h]
    float m_inv_mass; // [esp-4h] [ebp-10h]
    float v23; // [esp-4h] [ebp-10h]
    float v24; // [esp-4h] [ebp-10h]
    float v25; // [esp-4h] [ebp-10h]
    float v26; // [esp-4h] [ebp-10h]

    m_b2 = this->m_b2;
    v10 = this->m_b1_ap.y * this->m_b1_r.z - this->m_b1_ap.z * this->m_b1_r.y;
    v12 = this->m_b1_ap.z * this->m_b1_r.x - this->m_b1_r.z * this->m_b1_ap.x;
    v14 = this->m_b1_r.y * this->m_b1_ap.x - this->m_b1_ap.y * this->m_b1_r.x;
    v3 = result;
    m_inv_mass = this->m_b1->m_inv_mass;
    v16 = this->m_ud.x * m_inv_mass;
    v18 = this->m_ud.y * m_inv_mass;
    v20 = m_inv_mass * this->m_ud.z;
    v23 = v16 + v10;
    v4 = v23;
    result->x = v23;
    v24 = v18 + v12;
    v5 = v24;
    result->y = v24;
    v25 = v20 + v14;
    v6 = v25;
    result->z = v25;
    if ( m_b2 )
    {
        v17 = this->m_b2_ap.y * this->m_b2_r.z - this->m_b2_ap.z * this->m_b2_r.y;
        v19 = this->m_b2_ap.z * this->m_b2_r.x - this->m_b2_ap.x * this->m_b2_r.z;
        v21 = this->m_b2_r.y * this->m_b2_ap.x - this->m_b2_r.x * this->m_b2_ap.y;
        v26 = m_b2->m_inv_mass;
        v11 = this->m_ud.x * v26;
        v13 = this->m_ud.y * v26;
        v15 = v26 * this->m_ud.z;
        v7 = v11 + v17;
        v8 = v13 + v19;
        v9 = v15 + v21;
        result->x = v4 + v7;
        result->y = v5 + v8;
        result->z = v6 + v9;
    }
    return v3;
}

const phys_vec3 *pulse_sum_normal::get_relative_velocity(phys_vec3 *result)
{
    rigid_body *m_rb; // eax
    phys_vec3 *p_m_b2_ap; // edx
    rigid_body *v5; // eax
    double z; // st7
    const phys_vec3 *v7; // eax
    float v8[4]; // [esp-30h] [ebp-3Ch] BYREF
    float v9; // [esp-20h] [ebp-2Ch]
    float v10; // [esp-1Ch] [ebp-28h]
    float v11; // [esp-18h] [ebp-24h]
    float v12; // [esp-10h] [ebp-1Ch]
    float v13; // [esp-Ch] [ebp-18h]
    float v14; // [esp-8h] [ebp-14h]
    //int v15; // [esp+0h] [ebp-Ch]
    //void *v16; // [esp+4h] [ebp-8h]
    //void *retaddr; // [esp+Ch] [ebp+0h]
    //
    //v15 = a2;
    //v16 = retaddr;
    if ( this->m_b2 )
    {
        m_rb = this->m_b2->m_rb;
        p_m_b2_ap = (phys_vec3 *)v8;
        v12 = this->m_b2_r.z * m_rb->m_a_vel.y - this->m_b2_r.y * m_rb->m_a_vel.z;
        v13 = this->m_b2_r.x * m_rb->m_a_vel.z - this->m_b2_r.z * m_rb->m_a_vel.x;
        v14 = this->m_b2_r.y * m_rb->m_a_vel.x - m_rb->m_a_vel.y * this->m_b2_r.x;
        v8[0] = m_rb->m_t_vel.x + v12;
        v8[1] = m_rb->m_t_vel.y + v13;
        v8[2] = m_rb->m_t_vel.z + v14;
    }
    else
    {
        p_m_b2_ap = &this->m_b2_ap;
    }
    v5 = this->m_b1->m_rb;
    v12 = this->m_b1_r.z * v5->m_a_vel.y - this->m_b1_r.y * v5->m_a_vel.z;
    v13 = this->m_b1_r.x * v5->m_a_vel.z - this->m_b1_r.z * v5->m_a_vel.x;
    v14 = v5->m_a_vel.x * this->m_b1_r.y - v5->m_a_vel.y * this->m_b1_r.x;
    v9 = v5->m_t_vel.x + v12;
    v10 = v5->m_t_vel.y + v13;
    z = v5->m_t_vel.z;
    v7 = result;
    v11 = z + v14;
    result->x = v9 - p_m_b2_ap->x;
    result->y = v10 - p_m_b2_ap->y;
    result->z = v11 - p_m_b2_ap->z;
    return v7;
}

double __thiscall pulse_sum_normal::get_unclamped_pulse_sum()
{
    //double ps; // [esp+8h] [ebp-8h]
    //
    //ps = this->m_right_side;
    //*((float *)&ps + 1) = (ps - pulse_sum_normal::get_objective(this) - this->m_cfm * this->m_pulse_sum) / this->m_denom;
    //*((float *)&ps + 1) = this->m_pulse_sum + *((float *)&ps + 1);
    //return pulse_sum_normal::clamp_pulse_sum(this, *((float *)&ps + 1));

    // aislopped
    // Compute incremental impulse
    float delta =
        (m_right_side
            - get_objective()
            - m_cfm * m_pulse_sum)
        / m_denom;

    // Accumulate
    float newPulseSum = m_pulse_sum + delta;

    // Clamp to limits (min/max)
    return clamp_pulse_sum(newPulseSum);
}

const phys_vec3 *pulse_sum_point::get_vel(phys_vec3 *result)
{
    rigid_body *m_rb; // eax
    phys_vec3 *p_m_b2_apx; // edx
    rigid_body *v5; // eax
    double z; // st7
    const phys_vec3 *v7; // eax
    float v8[4]; // [esp-30h] [ebp-3Ch] BYREF
    float v9; // [esp-20h] [ebp-2Ch]
    float v10; // [esp-1Ch] [ebp-28h]
    float v11; // [esp-18h] [ebp-24h]
    float v12; // [esp-10h] [ebp-1Ch]
    float v13; // [esp-Ch] [ebp-18h]
    float v14; // [esp-8h] [ebp-14h]
    //int v15; // [esp+0h] [ebp-Ch]
    //void *v16; // [esp+4h] [ebp-8h]
    //void *retaddr; // [esp+Ch] [ebp+0h]
    //
    //v15 = a2;
    //v16 = retaddr;
    if ( this->m_b2 )
    {
        m_rb = this->m_b2->m_rb;
        p_m_b2_apx = (phys_vec3 *)v8;
        v12 = this->m_b2_r.z * m_rb->m_a_vel.y - this->m_b2_r.y * m_rb->m_a_vel.z;
        v13 = this->m_b2_r.x * m_rb->m_a_vel.z - this->m_b2_r.z * m_rb->m_a_vel.x;
        v14 = this->m_b2_r.y * m_rb->m_a_vel.x - m_rb->m_a_vel.y * this->m_b2_r.x;
        v8[0] = m_rb->m_t_vel.x + v12;
        v8[1] = m_rb->m_t_vel.y + v13;
        v8[2] = m_rb->m_t_vel.z + v14;
    }
    else
    {
        p_m_b2_apx = &this->m_b2_apx;
    }
    v5 = this->m_b1->m_rb;
    v12 = this->m_b1_r.z * v5->m_a_vel.y - this->m_b1_r.y * v5->m_a_vel.z;
    v13 = this->m_b1_r.x * v5->m_a_vel.z - this->m_b1_r.z * v5->m_a_vel.x;
    v14 = v5->m_a_vel.x * this->m_b1_r.y - v5->m_a_vel.y * this->m_b1_r.x;
    v9 = v5->m_t_vel.x + v12;
    v10 = v5->m_t_vel.y + v13;
    z = v5->m_t_vel.z;
    v7 = result;
    v11 = z + v14;
    result->x = v9 - p_m_b2_apx->x;
    result->y = v10 - p_m_b2_apx->y;
    result->z = v11 - p_m_b2_apx->z;
    return v7;
}

const phys_vec3 *pulse_sum_point::get_pos(phys_vec3 *result)
{
    pulse_sum_node *m_b2; // eax
    float *p_x; // eax
    double v5; // st7
    phys_vec3 *p_m_b2_r; // edx
    rigid_body *m_rb; // eax
    double x; // st7
    double z; // st7
    const phys_vec3 *v10; // eax
    float v11[4]; // [esp-20h] [ebp-2Ch] BYREF
    float v12; // [esp-10h] [ebp-1Ch]
    float v13; // [esp-Ch] [ebp-18h]
    float v14; // [esp-8h] [ebp-14h]
    //int v15; // [esp+0h] [ebp-Ch]
    //void *v16; // [esp+4h] [ebp-8h]
    //void *retaddr; // [esp+Ch] [ebp+0h]
    //
    //v15 = a2;
    //v16 = retaddr;
    m_b2 = this->m_b2;
    if ( m_b2 )
    {
        p_x = &m_b2->m_rb->m_last_position.x;
        v5 = this->m_b2_r.x + p_x[24];
        p_x += 24;
        p_m_b2_r = (phys_vec3 *)v11;
        v11[0] = v5;
        v11[1] = p_x[1] + this->m_b2_r.y;
        v11[2] = p_x[2] + this->m_b2_r.z;
    }
    else
    {
        p_m_b2_r = &this->m_b2_r;
    }
    m_rb = this->m_b1->m_rb;
    x = m_rb->m_mat.w.x;
    m_rb = (rigid_body *)((char *)m_rb + 96);
    v12 = x + this->m_b1_r.x;
    v13 = m_rb->m_last_position.y + this->m_b1_r.y;
    z = m_rb->m_last_position.z;
    v10 = result;
    v14 = z + this->m_b1_r.z;
    result->x = v12 - p_m_b2_r->x;
    result->y = v13 - p_m_b2_r->y;
    result->z = v14 - p_m_b2_r->z;
    return v10;
}

double __thiscall pulse_sum_angular::get_vel()
{
    pulse_sum_node *m_b2; // eax
    phys_vec3 *p_m_a_vel; // edx
    rigid_body *m_rb; // eax
    double x; // st7
    float v6; // [esp-20h] [ebp-2Ch]
    float v7; // [esp-1Ch] [ebp-28h]
    float v8; // [esp-18h] [ebp-24h]

    m_b2 = this->m_b2;
    if ( m_b2 )
        p_m_a_vel = &m_b2->m_rb->m_a_vel;
    else
        p_m_a_vel = &this->m_b2_ap;
    m_rb = this->m_b1->m_rb;
    x = m_rb->m_a_vel.x;
    m_rb = (rigid_body *)((char *)m_rb + 160);
    v6 = x - p_m_a_vel->x;
    v7 = m_rb->m_last_position.y - p_m_a_vel->y;
    v8 = m_rb->m_last_position.z - p_m_a_vel->z;
    return (float)(this->m_ud.y * v7 + this->m_ud.x * v6 + this->m_ud.z * v8);
}

double __thiscall pulse_sum_angular::get_pos()
{
    float v2; // [esp-20h] [ebp-2Ch]
    float v3; // [esp-1Ch] [ebp-28h]
    float v4; // [esp-18h] [ebp-24h]
    float v5; // [esp-4h] [ebp-10h]

    v2 = this->m_b1_r.y * this->m_b2_r.z - this->m_b1_r.z * this->m_b2_r.y;
    v3 = this->m_b1_r.z * this->m_b2_r.x - this->m_b1_r.x * this->m_b2_r.z;
    v4 = this->m_b2_r.y * this->m_b1_r.x - this->m_b1_r.y * this->m_b2_r.x;
    v5 = this->m_ud.y * v3 + this->m_ud.x * v2 + this->m_ud.z * v4;
    return -v5;
}

void pulse_sum_angular::set(
                rigid_body *const b1,
                const phys_vec3 *b1_r,
                rigid_body *const b2,
                const phys_vec3 *b2_r,
                const phys_vec3 *ud,
                pulse_sum_cache *const ps_cache)
{
    rigid_body *v9; // ecx
    int v10; // eax
    unsigned int v11; // eax
    pulse_sum_node *m_node; // eax
    pulse_sum_node *v13; // eax
    double z; // st7
    float v15; // [esp-10h] [ebp-1Ch]
    float v16; // [esp-Ch] [ebp-18h]
    float v17; // [esp-8h] [ebp-14h]
    //unsigned int v18[3]; // [esp+0h] [ebp-Ch] BYREF
    //_UNKNOWN *retaddr; // [esp+Ch] [ebp+0h]
    //
    //v18[0] = a2;
    //v18[1] = retaddr;
    if ( !ps_cache
        && _tlAssert(
                 "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\constraint_solver\\pulse_sum_angular_inline.h",
                 86,
                 "ps_cache",
                 "") )
    {
        __debugbreak();
    }
    if ( !b1
        && _tlAssert(
                 "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\constraint_solver\\pulse_sum_angular_inline.h",
                 87,
                 "b1",
                 "") )
    {
        __debugbreak();
    }
    if ( !b2
        && _tlAssert(
                 "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\constraint_solver\\pulse_sum_angular_inline.h",
                 88,
                 "b2",
                 "") )
    {
        __debugbreak();
    }
    if ( b1 == b2
        && _tlAssert(
                 "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\constraint_solver\\pulse_sum_angular_inline.h",
                 89,
                 "b1 != b2",
                 "") )
    {
        __debugbreak();
    }
    PHYS_ASSERT_UNIT(ud);
    v9 = b1;
    if ( b1->m_node )
        goto LABEL_19;
    if ( !b2->m_node )
    {
        if ( _tlAssert(
                     "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\constraint_solver\\pulse_sum_angular_inline.h",
                     91,
                     "rbint::get_pulse_sum_node(b1) || rbint::get_pulse_sum_node(b2)",
                     "") )
        {
            __debugbreak();
        }
        v9 = b1;
    }
    if ( v9->m_node )
LABEL_19:
        v10 = (v9->m_flags & 0x30) == 0;
    else
        v10 = v9->m_flags & 0x30;
    if ( !v10 || (!b2->m_node ? (v11 = b2->m_flags & 0x30) : (v11 = (b2->m_flags & 0x30) == 0), !v11) )
    {
        if ( _tlAssert(
                     "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\constraint_solver\\pulse_sum_angular_inline.h",
                     92,
                     "rbint::verify_pulse_sum_node(b1) && rbint::verify_pulse_sum_node(b2)",
                     "") )
        {
            __debugbreak();
        }
        v9 = b1;
    }
    m_node = v9->m_node;
    if ( m_node )
    {
        this->m_b1 = m_node;
        this->m_b1_r.x = b1_r->x;
        this->m_b1_r.y = b1_r->y;
        this->m_b1_r.z = b1_r->z;
        v13 = b2->m_node;
        if ( v13 )
        {
            this->m_b2 = v13;
            this->m_b2_r.x = b2_r->x;
            this->m_b2_r.y = b2_r->y;
            this->m_b2_r.z = b2_r->z;
        }
        else
        {
            this->m_b2 = 0;
            pulse_sum_angular::set_object_vel(&b2->m_a_vel);
            pulse_sum_angular::set_object_col_pt(b2_r);
        }
        this->m_ud.x = ud->x;
        this->m_ud.y = ud->y;
        z = ud->z;
    }
    else
    {
        this->m_b1 = b2->m_node;
        this->m_b1_r.x = b2_r->x;
        this->m_b1_r.y = b2_r->y;
        this->m_b1_r.z = b2_r->z;
        this->m_b2 = 0;
        pulse_sum_angular::set_object_vel(&v9->m_a_vel);
        pulse_sum_angular::set_object_col_pt(b1_r);
        v15 = -ud->x;
        v16 = -ud->y;
        v17 = -ud->z;
        this->m_ud.x = v15;
        this->m_ud.y = v16;
        z = v17;
    }
    this->m_ud.z = z;
    this->m_pulse_sum_cache = ps_cache;
    this->m_flags = 0;
    pulse_sum_angular::calc_abs();
}

const phys_vec3 *pulse_sum_contact_point::get_vel(phys_vec3 *result, pulse_sum_contact *psc)
{
    rigid_body *m_rb; // eax
    phys_vec3 *p_m_b2_ap_n; // esi
    rigid_body *v6; // eax
    double z; // st7
    const phys_vec3 *v8; // eax
    double v9; // st6
    double v10; // st5
    double v11; // st7
    float v12[4]; // [esp-2Ch] [ebp-3Ch] BYREF
    float v13; // [esp-1Ch] [ebp-2Ch]
    float v14; // [esp-18h] [ebp-28h]
    float v15; // [esp-14h] [ebp-24h]
    float v16; // [esp-Ch] [ebp-1Ch]
    float v17; // [esp-8h] [ebp-18h]
    float v18; // [esp-4h] [ebp-14h]
    //phys_vec3 retv; // [esp+0h] [ebp-10h]
    //float retaddr; // [esp+10h] [ebp+0h]
    //
    //retv.y = a2;
    //retv.z = retaddr;
    if ( psc->m_b2 )
    {
        m_rb = psc->m_b2->m_rb;
        p_m_b2_ap_n = (phys_vec3 *)v12;
        v16 = this->m_b2_r.z * m_rb->m_a_vel.y - this->m_b2_r.y * m_rb->m_a_vel.z;
        v17 = this->m_b2_r.x * m_rb->m_a_vel.z - this->m_b2_r.z * m_rb->m_a_vel.x;
        v18 = m_rb->m_a_vel.x * this->m_b2_r.y - m_rb->m_a_vel.y * this->m_b2_r.x;
        v12[0] = m_rb->m_t_vel.x + v16;
        v12[1] = m_rb->m_t_vel.y + v17;
        v12[2] = m_rb->m_t_vel.z + v18;
    }
    else
    {
        p_m_b2_ap_n = &this->m_b2_ap_n;
    }
    v6 = psc->m_b1->m_rb;
    v16 = v6->m_a_vel.y * this->m_b1_r.z - v6->m_a_vel.z * this->m_b1_r.y;
    v17 = v6->m_a_vel.z * this->m_b1_r.x - v6->m_a_vel.x * this->m_b1_r.z;
    v18 = this->m_b1_r.y * v6->m_a_vel.x - v6->m_a_vel.y * this->m_b1_r.x;
    v13 = v6->m_t_vel.x + v16;
    v14 = v6->m_t_vel.y + v17;
    z = v6->m_t_vel.z;
    v8 = result;
    v15 = z + v18;
    v16 = v13 - p_m_b2_ap_n->x;
    v17 = v14 - p_m_b2_ap_n->y;
    v18 = v15 - p_m_b2_ap_n->z;
    v9 = v17;
    v10 = v16;
    v11 = v18;
    result->x = psc->m_ud_n.z * v18 + psc->m_ud_n.y * v17 + v16 * psc->m_ud_n.x;
    result->y = psc->m_ud_f1.y * v9 + psc->m_ud_f1.x * v10 + psc->m_ud_f1.z * v11;
    result->z = v11 * psc->m_ud_f2.z + v9 * psc->m_ud_f2.y + v10 * psc->m_ud_f2.x;
    return v8;
}

double pulse_sum_contact_point::get_pos(pulse_sum_contact *psc)
{
    pulse_sum_node *m_b2; // eax
    float *p_x; // eax
    double v5; // st7
    phys_vec3 *p_m_b2_r; // edx
    rigid_body *m_rb; // eax
    double v8; // st7
    float v10[4]; // [esp-40h] [ebp-4Ch] BYREF
    float v11; // [esp-30h] [ebp-3Ch]
    float v12; // [esp-2Ch] [ebp-38h]
    float v13; // [esp-28h] [ebp-34h]
    float v14; // [esp-20h] [ebp-2Ch]
    float v15; // [esp-1Ch] [ebp-28h]
    float v16; // [esp-18h] [ebp-24h]
    //int v18; // [esp+0h] [ebp-Ch]
    //void *v19; // [esp+4h] [ebp-8h]
    //void *retaddr; // [esp+Ch] [ebp+0h]
    //
    //v18 = a2;
    //v19 = retaddr;
    m_b2 = psc->m_b2;
    if ( m_b2 )
    {
        p_x = &m_b2->m_rb->m_last_position.x;
        v5 = p_x[24];
        p_x += 24;
        p_m_b2_r = (phys_vec3 *)v10;
        v10[0] = v5 + this->m_b2_r.x;
        v10[1] = p_x[1] + this->m_b2_r.y;
        v10[2] = p_x[2] + this->m_b2_r.z;
    }
    else
    {
        p_m_b2_r = &this->m_b2_r;
    }
    m_rb = psc->m_b1->m_rb;
    v8 = this->m_b1_r.x + m_rb->m_mat.w.x;
    m_rb = (rigid_body *)((char *)m_rb + 96);
    v14 = v8;
    v15 = m_rb->m_last_position.y + this->m_b1_r.y;
    v16 = m_rb->m_last_position.z + this->m_b1_r.z;
    v11 = v14 - p_m_b2_r->x;
    v12 = v15 - p_m_b2_r->y;
    v13 = v16 - p_m_b2_r->z;
    return (float)(psc->m_ud_n.y * v12 + psc->m_ud_n.x * v11 + psc->m_ud_n.z * v13);
}

void pulse_sum_point::set(
                rigid_body *const b1,
                const phys_vec3 *b1_r,
                rigid_body *const b2,
                const phys_vec3 *b2_r,
                pulse_sum_cache *const ps_cache,
                float delta_t,
                bool is_spring,
                float spring_k,
                float damp_k)
{
    unsigned int v12; // eax
    unsigned int v13; // eax
    pulse_sum_node *m_node; // eax
    double z; // st7
    const phys_vec3 *v16; // eax
    bool v17; // zf
    phys_vec3 v18; // [esp+20h] [ebp-1Ch] BYREF
    //unsigned int v19[3]; // [esp+30h] [ebp-Ch] BYREF
    //_UNKNOWN *retaddr; // [esp+3Ch] [ebp+0h]
    //
    //v19[0] = a2;
    //v19[1] = retaddr;
    if ( !ps_cache
        && _tlAssert(
                 "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\constraint_solver\\pulse_sum_point_inline.h",
                 167,
                 "ps_cache",
                 "") )
    {
        __debugbreak();
    }
    if ( !b1
        && _tlAssert(
                 "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\constraint_solver\\pulse_sum_point_inline.h",
                 168,
                 "b1",
                 "") )
    {
        __debugbreak();
    }
    if ( !b2
        && _tlAssert(
                 "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\constraint_solver\\pulse_sum_point_inline.h",
                 169,
                 "b2",
                 "") )
    {
        __debugbreak();
    }
    if ( b1 == b2
        && _tlAssert(
                 "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\constraint_solver\\pulse_sum_point_inline.h",
                 170,
                 "b1 != b2",
                 "") )
    {
        __debugbreak();
    }
    if ( !b1->m_node
        && !b2->m_node
        && _tlAssert(
                 "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\constraint_solver\\pulse_sum_point_inline.h",
                 171,
                 "rbint::get_pulse_sum_node(b1) || rbint::get_pulse_sum_node(b2)",
                 "") )
    {
        __debugbreak();
    }
    if ( b1->m_node )
        v12 = (b1->m_flags & 0x30) == 0;
    else
        v12 = b1->m_flags & 0x30;
    if ( !v12 || (!b2->m_node ? (v13 = b2->m_flags & 0x30) : (v13 = (b2->m_flags & 0x30) == 0), !v13) )
    {
        if ( _tlAssert(
                     "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\constraint_solver\\pulse_sum_point_inline.h",
                     172,
                     "rbint::verify_pulse_sum_node(b1) && rbint::verify_pulse_sum_node(b2)",
                     "") )
        {
            __debugbreak();
        }
    }
    if ( !b1->m_node
        && _tlAssert(
                 "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\constraint_solver\\pulse_sum_point_inline.h",
                 174,
                 "rbint::get_pulse_sum_node(b1) != NULL",
                 "") )
    {
        __debugbreak();
    }
    this->m_b1 = b1->m_node;
    this->m_b1_r.x = b1_r->x;
    this->m_b1_r.y = b1_r->y;
    this->m_b1_r.z = b1_r->z;
    m_node = b2->m_node;
    if ( m_node )
    {
        this->m_b2 = m_node;
        this->m_b2_r.x = b2_r->x;
        this->m_b2_r.y = b2_r->y;
        z = b2_r->z;
    }
    else
    {
        this->m_b2 = 0;
        v16 = rbint::gtv(&v18, b2, b2_r);
        pulse_sum_point::set_object_vel(v16);
        v17 = this->m_b2 == 0;
        v18.x = b2_r->x + b2->m_mat.w.x;
        v18.y = b2->m_mat.w.y + b2_r->y;
        v18.z = b2->m_mat.w.z + b2_r->z;
        if ( !v17
            && _tlAssert(
                     "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\constraint_solver\\pulse_sum_point.h",
                     47,
                     "m_b2 == NULL",
                     "") )
        {
            __debugbreak();
        }
        this->m_b2_r.x = v18.x;
        this->m_b2_r.y = v18.y;
        z = v18.z;
    }
    this->m_b2_r.z = z;
    this->m_pulse_sum_cache = ps_cache;
    pulse_sum_point::calc_abs_and_setup(delta_t, is_spring, spring_k, damp_k);
}

double pulse_sum_contact_point::get_impact_vel(pulse_sum_contact *psc)
{
    pulse_sum_node *m_b2; // edi
    rigid_body *p_body; // eax
    phys_vec3 *p_m_b2_ap_n; // esi
    rigid_body *m_rb; // eax
    rigid_body *v7; // eax
    phys_vec3 *v8; // esi
    rigid_body *v9; // eax
    phys_vec3 v11; // [esp-1Ch] [ebp-5Ch] BYREF
    float v12; // [esp-Ch] [ebp-4Ch]
    float v13; // [esp-8h] [ebp-48h]
    float v14; // [esp-4h] [ebp-44h]
    phys_vec3 lv; // [esp+0h] [ebp-40h]
    phys_vec3 v; // [esp+10h] [ebp-30h]
    float v17; // [esp+2Ch] [ebp-14h]
    float v18; // [esp+30h] [ebp-10h]
    //int v19; // [esp+34h] [ebp-Ch]
    //void *v20; // [esp+38h] [ebp-8h]
    //void *retaddr; // [esp+40h] [ebp+0h]
    //
    //v19 = a2;
    //v20 = retaddr;
    m_b2 = psc->m_b2;
    if (m_b2)
    {
        p_body = m_b2->m_rb;
        p_m_b2_ap_n = &v11;
        v.y = this->m_b2_r.z * p_body->m_last_a_vel.y - this->m_b2_r.y * p_body->m_last_a_vel.z;
        v.z = this->m_b2_r.x * p_body->m_last_a_vel.z - this->m_b2_r.z * p_body->m_last_a_vel.x;
        v.w = p_body->m_last_a_vel.x * this->m_b2_r.y - p_body->m_last_a_vel.y * this->m_b2_r.x;
        v11.x = p_body->m_last_t_vel.x + v.y;
        v11.y = p_body->m_last_t_vel.y + v.z;
        v11.z = p_body->m_last_t_vel.z + v.w;
    }
    else
    {
        p_m_b2_ap_n = &this->m_b2_ap_n;
    }
    m_rb = psc->m_b1->m_rb;
    v.y = m_rb->m_last_a_vel.y * this->m_b1_r.z - m_rb->m_last_a_vel.z * this->m_b1_r.y;
    v.z = m_rb->m_last_a_vel.z * this->m_b1_r.x - this->m_b1_r.z * m_rb->m_last_a_vel.x;
    v.w = this->m_b1_r.y * m_rb->m_last_a_vel.x - m_rb->m_last_a_vel.y * this->m_b1_r.x;
    lv.y = m_rb->m_last_t_vel.x + v.y;
    lv.z = m_rb->m_last_t_vel.y + v.z;
    lv.w = m_rb->m_last_t_vel.z + v.w;
    v12 = lv.y - p_m_b2_ap_n->x;
    v13 = lv.z - p_m_b2_ap_n->y;
    v14 = lv.w - p_m_b2_ap_n->z;
    if (m_b2)
    {
        v7 = m_b2->m_rb;
        v8 = &v11;
        lv.y = this->m_b2_r.z * v7->m_a_vel.y - this->m_b2_r.y * v7->m_a_vel.z;
        lv.z = this->m_b2_r.x * v7->m_a_vel.z - this->m_b2_r.z * v7->m_a_vel.x;
        lv.w = v7->m_a_vel.x * this->m_b2_r.y - v7->m_a_vel.y * this->m_b2_r.x;
        v11.x = lv.y + v7->m_t_vel.x;
        v11.y = v7->m_t_vel.y + lv.z;
        v11.z = v7->m_t_vel.z + lv.w;
    }
    else
    {
        v8 = &this->m_b2_ap_n;
    }
    v9 = psc->m_b1->m_rb;
    lv.y = this->m_b1_r.z * v9->m_a_vel.y - this->m_b1_r.y * v9->m_a_vel.z;
    lv.z = this->m_b1_r.x * v9->m_a_vel.z - v9->m_a_vel.x * this->m_b1_r.z;
    lv.w = this->m_b1_r.y * v9->m_a_vel.x - this->m_b1_r.x * v9->m_a_vel.y;
    v.y = v9->m_t_vel.x + lv.y;
    v.z = v9->m_t_vel.y + lv.z;
    v.w = v9->m_t_vel.z + lv.w;
    lv.y = v.y - v8->x;
    lv.z = v.z - v8->y;
    lv.w = v.w - v8->z;
    v17 = psc->m_ud_n.y * lv.z + psc->m_ud_n.x * lv.y + psc->m_ud_n.z * lv.w;
    v18 = psc->m_ud_n.y * v13 + psc->m_ud_n.x * v12 + psc->m_ud_n.z * v14;
    if (v17 >= (double)v18)
        return v17;
    return v18;
}

void pulse_sum_contact::set(
                rigid_body *const b1,
                rigid_body *const b2,
                contact_point_info *cpi,
                float delta_t)
{
    unsigned int v7; // eax
    unsigned int v8; // eax
    const phys_vec3 *v9; // eax
    int m_list_pscp_count; // eax
    rigid_body *m_rb; // eax
    float y; // ecx
    float z; // edx
    float w; // eax
    double v15; // st7
    double v16; // st5
    phys_vec3 *m_list_b1_r_loc; // ecx
    phys_vec3 *m_list_b2_r_loc; // edx
    bool v19; // cc
    double v20; // st7
    pulse_sum_contact_point *v21; // esi
    phys_vec3 *v22; // edx
    double v23; // st7
    const phys_vec3 *v24; // eax
    bool v25; // zf
    phys_vec3 v26; // [esp+24h] [ebp-6Ch] BYREF
    phys_vec3 v27; // [esp+34h] [ebp-5Ch] BYREF
    float v28; // [esp+44h] [ebp-4Ch]
    float v29; // [esp+48h] [ebp-48h]
    float v30; // [esp+4Ch] [ebp-44h]
    phys_vec3 b2_r; // [esp+54h] [ebp-3Ch] BYREF
    float min_restitution_impact_vel_sq; // [esp+6Ch] [ebp-24h]
    const phys_mat44 *p_m_mat; // [esp+70h] [ebp-20h]
    int pp_i; // [esp+74h] [ebp-1Ch]
    phys_vec3 *b2_r_loc; // [esp+78h] [ebp-18h]
    phys_vec3 *b1_r_loc; // [esp+7Ch] [ebp-14h]
    unsigned int v37; // [esp+80h] [ebp-10h]
    //_UNKNOWN *v38[2]; // [esp+84h] [ebp-Ch] BYREF
    //contact_point_info *cpia; // [esp+90h] [ebp+0h]
    //
    //v38[0] = a2;
    //v38[1] = cpia;
    if (!b1
        && _tlAssert(
            "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\constraint_solver\\pulse_sum_contact_inline.h",
            236,
            "b1",
            ""))
    {
        __debugbreak();
    }
    if (!b2
        && _tlAssert(
            "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\constraint_solver\\pulse_sum_contact_inline.h",
            237,
            "b2",
            ""))
    {
        __debugbreak();
    }
    if (b1 == b2
        && _tlAssert(
            "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\constraint_solver\\pulse_sum_contact_inline.h",
            238,
            "b1 != b2",
            ""))
    {
        __debugbreak();
    }
    if (b1->m_node)
        goto LABEL_15;
    if (!b2->m_node
        && _tlAssert(
            "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\constraint_solver\\pulse_sum_contact_inline.h",
            239,
            "rbint::get_pulse_sum_node(b1) || rbint::get_pulse_sum_node(b2)",
            ""))
    {
        __debugbreak();
    }
    if (b1->m_node)
        LABEL_15:
    v7 = (b1->m_flags & 0x30) == 0;
    else
        v7 = b1->m_flags & 0x30;
    if (!v7 || (!b2->m_node ? (v8 = b2->m_flags & 0x30) : (v8 = (b2->m_flags & 0x30) == 0), !v8))
    {
        if (_tlAssert(
            "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\constraint_solver\\pulse_sum_contact_inline.h",
            240,
            "rbint::verify_pulse_sum_node(b1) && rbint::verify_pulse_sum_node(b2)",
            ""))
        {
            __debugbreak();
        }
    }
    if (!b1->m_node
        && _tlAssert(
            "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\constraint_solver\\pulse_sum_contact_inline.h",
            242,
            "rbint::get_pulse_sum_node(b1) != NULL",
            "b1 in contact constraint cannot be environment or user rigid body."))
    {
        __debugbreak();
    }
    this->m_b1 = b1->m_node;
    this->m_b2 = b2->m_node;
    this->m_ud_n.x = cpi->m_normal.x;
    this->m_ud_n.y = cpi->m_normal.y;
    this->m_ud_n.z = cpi->m_normal.z;
    v9 = construct_orth_ud(&v27, &this->m_ud_n);
    this->m_ud_f1.x = v9->x;
    this->m_ud_f1.y = v9->y;
    this->m_ud_f1.z = v9->z;
    b2_r.x = this->m_ud_n.y * this->m_ud_f1.z - this->m_ud_n.z * this->m_ud_f1.y;
    b2_r.y = this->m_ud_f1.x * this->m_ud_n.z - this->m_ud_n.x * this->m_ud_f1.z;
    b2_r.z = this->m_ud_n.x * this->m_ud_f1.y - this->m_ud_n.y * this->m_ud_f1.x;
    this->m_ud_f2.x = b2_r.x;
    this->m_ud_f2.y = b2_r.y;
    this->m_ud_f2.z = b2_r.z;
    m_list_pscp_count = this->m_list_pscp_count;
    this->m_fric_coef = cpi->m_fric_coef;
    this->m_pulse_sum_cache_list = cpi->m_list_pulse_sum_cache_info;
    if (m_list_pscp_count != cpi->m_point_pair_count
        && _tlAssert(
            "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\constraint_solver\\pulse_sum_contact_inline.h",
            251,
            "m_list_pscp_count == cpi->m_point_pair_count",
            ""))
    {
        __debugbreak();
    }
    m_rb = this->m_b1->m_rb;
    y = m_rb->m_gravity_acc_vec.y;
    b2_r.x = m_rb->m_gravity_acc_vec.x;
    z = m_rb->m_gravity_acc_vec.z;
    w = m_rb->m_gravity_acc_vec.w;
    b2_r.y = y;
    v15 = y;
    b2_r.z = z;
    v16 = z;
    m_list_b1_r_loc = cpi->m_list_b1_r_loc;
    m_list_b2_r_loc = cpi->m_list_b2_r_loc;
    b2_r.w = w;
    v19 = cpi->m_point_pair_count <= 0;
    b1_r_loc = m_list_b1_r_loc;
    b2_r_loc = m_list_b2_r_loc;
    *(float *)&pp_i = v15 * v15 + b2_r.x * b2_r.x + v16 * v16;
    v20 = *(float *)&pp_i;
    *(float *)&pp_i = 0.0;
    min_restitution_impact_vel_sq = v20 * 0.09000000357627869;
    if (!v19)
    {
        p_m_mat = &b1->m_mat;
        v37 = 0;
        do
        {
            v21 = &this->m_list_pscp[v37 / 0xD0];
            phys_multiply(&v27, p_m_mat, b1_r_loc);
            v22 = b2_r_loc;
            v21->m_b1_r.x = v27.x;
            v21->m_b1_r.y = v27.y;
            v21->m_b1_r.z = v27.z;
            phys_multiply(&b2_r, &b2->m_mat, v22);
            if (this->m_b2)
            {
                v21->m_b2_r.x = b2_r.x;
                v21->m_b2_r.y = b2_r.y;
                v23 = b2_r.z;
            }
            else
            {
                v24 = rbint::gtv(&v26, b2, &b2_r);
                v21->m_b2_ap_n.x = v24->x;
                v21->m_b2_ap_n.y = v24->y;
                v21->m_b2_ap_n.z = v24->z;
                v25 = this->m_b2 == 0;
                v28 = b2_r.x + b2->m_mat.w.x;
                v29 = b2->m_mat.w.y + b2_r.y;
                v30 = b2->m_mat.w.z + b2_r.z;
                if (!v25)
                {
                    if (_tlAssert(
                        "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\constraint_solver\\pulse_sum_contact_inline.h",
                        209,
                        "psc->m_b2 == NULL",
                        ""))
                    {
                        __debugbreak();
                    }
                }
                v21->m_b2_r.x = v28;
                v21->m_b2_r.y = v29;
                v23 = v30;
            }
            v21->m_b2_r.z = v23;
            //pulse_sum_contact_point::calc_abs(v21, (int)v38, this);
            v21->calc_abs(this);
            //pulse_sum_contact_point::setup_vel_uni_restitution(
                v21->setup_vel_uni_restitution(
                this,
                cpi->m_bounce_coef,
                cpi->m_max_restitution_vel,
                delta_t,
                50.0,
                min_restitution_impact_vel_sq);
            v37 += 208;
            ++b1_r_loc;
            ++b2_r_loc;
            v19 = ++pp_i < cpi->m_point_pair_count;
        } while (v19);
    }
}

int g_list_island_cur;
void __cdecl constraint_solver_process(
                phys_transient_allocator *transient_buffer,
                physics_system *psys,
                float outside_delta_t)
{
    rigid_body **m_list_island; // ebx
    int m_list_island_count; // edi
    constraint_solver_task_input *input; // eax
    int m_max_vel_pos_iters; // edx

    m_list_island = psys->m_list_island;
    m_list_island_count = psys->m_list_island_count;
    if (m_list_island_count > 0)
    {
        g_list_island_cur = 0;
        input = (constraint_solver_task_input *)transient_buffer->allocate(sizeof(constraint_solver_task_input), 4, 0, "phys_transient_allocator out of memory.");
        input->m_list_island = m_list_island;
        input->m_list_island_count = m_list_island_count;
        input->m_list_island_cur = &g_list_island_cur;
        input->m_psys_max_vel_iters = psys->m_max_vel_iters;
        m_max_vel_pos_iters = psys->m_max_vel_pos_iters;
        input->m_outside_delta_t = outside_delta_t;
        input->m_psys_max_vel_pos_iters = m_max_vel_pos_iters;
        phys_task_manager_process(&phys_jq_constraint_solverModule, input, m_list_island_count);
    }

    if (phys_task_manager_needs_flush())
        phys_task_manager_flush();
}

void __thiscall pulse_sum_constraint_solver::solve_iterative(int max_iters, float max_error_sq)
{
    pulse_sum_constraint_solver *v3; // edi
    pulse_sum_normal *m_first; // esi
    pulse_sum_point *i; // esi
    pulse_sum_angular *j; // esi
    pulse_sum_wheel *k; // esi
    pulse_sum_contact *m_next_link; // ebx
    pulse_sum_contact_point *m_list_pscp; // esi
    pulse_sum_contact_point *m; // edi
    int iters; // [esp+4h] [ebp-8h]
    float error_sq; // [esp+8h] [ebp-4h] BYREF
    int savedregs; // [esp+Ch] [ebp+0h] BYREF

    v3 = this;
    error_sq = max_error_sq + max_error_sq;
    iters = 0;
    while ( iters <= max_iters )
    {
        if ( max_error_sq >= (double)error_sq )
            break;
        m_first = v3->m_list_pulse_sum_normal.m_first;
        ++iters;
        for (error_sq = 0.0; m_first; m_first = m_first->m_next_link)
        {
            //pulse_sum_normal::SOLVER_apply_relaxation(m_first, &error_sq, 1);
            m_first->SOLVER_apply_relaxation(&error_sq, 1);
        }
        for (i = v3->m_list_pulse_sum_point.m_first; i; i = i->m_next_link)
        {
            //pulse_sum_point::SOLVER_apply_relaxation(i, (int)&savedregs, &error_sq);
            i->SOLVER_apply_relaxation(&error_sq);
        }
        for (j = v3->m_list_pulse_sum_angular.m_first; j; j = j->m_next_link)
        {
            //pulse_sum_angular::SOLVER_apply_relaxation(j, &error_sq);
            j->SOLVER_apply_relaxation(&error_sq);
        }
        for (k = v3->m_list_pulse_sum_wheel.m_first; k; k = k->m_next_link)
        {
            //pulse_sum_wheel::SOLVER_apply_relaxation(k, &error_sq);
            k->SOLVER_apply_relaxation(&error_sq);
        }

        m_next_link = v3->m_list_pulse_sum_contact.m_first;

        if ( m_next_link )
        {
            do
            {
                m_list_pscp = m_next_link->m_list_pscp;
                for (m = &m_list_pscp[m_next_link->m_list_pscp_count]; m_list_pscp != m; ++m_list_pscp)
                {
                    //pulse_sum_contact_point::SOLVER_apply_relaxation(m_list_pscp, (int)&savedregs, m_next_link, &error_sq);
                    m_list_pscp->SOLVER_apply_relaxation(m_next_link, &error_sq);
                }
                m_next_link = m_next_link->m_next_link;
            }
            while ( m_next_link );
            v3 = this;
        }
    }
}

void __thiscall pulse_sum_constraint_solver::add_urb(
                phys_inplace_avl_tree<user_rigid_body *,pulse_sum_constraint_solver::temp_user_rigid_body,pulse_sum_constraint_solver::temp_user_rigid_body::avl_tree_accessor> *turb_search_tree,
                phys_simple_link_list<pulse_sum_constraint_solver::temp_user_rigid_body> *list_turb,
                phys_simple_link_list<pulse_sum_constraint_solver::user_rigid_body_restore_info> *list_urbri,
                rigid_body_constraint *rbc)
{
    rigid_body **p_b1; // edi
    rigid_body *b2; // eax
    pulse_sum_constraint_solver::temp_user_rigid_body *v7; // esi
    pulse_sum_constraint_solver::user_rigid_body_restore_info *v8; // eax

    p_b1 = &rbc->b1;
    if ( rbc->b1 && (rbc->b1->m_flags & 0x20) != 0 || (b2 = rbc->b2, p_b1 = &rbc->b2, b2) && (b2->m_flags & 0x20) != 0 )
    {
        if ( p_b1 )
        {
            //v7 = phys_inplace_avl_tree<user_rigid_body *,pulse_sum_constraint_solver::temp_user_rigid_body,pulse_sum_constraint_solver::temp_user_rigid_body::avl_tree_accessor>::find(turb_search_tree, (user_rigid_body **)p_b1);
            v7 = turb_search_tree->find(*(user_rigid_body **)p_b1);
            if ( !v7 )
            {
                //v7 = (pulse_sum_constraint_solver::temp_user_rigid_body *)phys_transient_allocator::allocate(&this->m_solver_memory_allocator, 464, 16, 0, SOLVER_MEMORY_ALLOCATOR_ERROR_MSG);
                v7 = (pulse_sum_constraint_solver::temp_user_rigid_body *)this->m_solver_memory_allocator.allocate(464, 16, 0, SOLVER_MEMORY_ALLOCATOR_ERROR_MSG);
                //user_rigid_body::operator=(v7, (const user_rigid_body *)*p_b1);
                *(user_rigid_body*)v7 = (user_rigid_body *)*p_b1;
                //phys_inplace_avl_tree<user_rigid_body *,pulse_sum_constraint_solver::temp_user_rigid_body,pulse_sum_constraint_solver::temp_user_rigid_body::avl_tree_accessor>::add(turb_search_tree, (user_rigid_body **)p_b1, v7);
                turb_search_tree->add(*(user_rigid_body **)p_b1, v7);
                v7->m_next_link = list_turb->m_first;
                list_turb->m_first = v7;
            }
            //v8 = (pulse_sum_constraint_solver::user_rigid_body_restore_info *)phys_transient_allocator::allocate(&this->m_solver_memory_allocator, 12, 4, 0, SOLVER_MEMORY_ALLOCATOR_ERROR_MSG);
            v8 = (pulse_sum_constraint_solver::user_rigid_body_restore_info *)this->m_solver_memory_allocator.allocate(12, 4, 0, SOLVER_MEMORY_ALLOCATOR_ERROR_MSG);
            v8->m_rbc_urb = (user_rigid_body **)p_b1;
            v8->m_original_urb = (user_rigid_body *)*p_b1;
            *p_b1 = v7;
            v8->m_next_link = list_urbri->m_first;
            list_urbri->m_first = v8;
        }
    }
}

int __cdecl phys_jq_constraint_solver_batch_function(jqBatch *pBatch)
{
    constraint_solver_task_input *Input = (constraint_solver_task_input *)pBatch->Input;

    pulse_sum_constraint_solver solver;
    solver.m_outside_delta_t = Input->m_outside_delta_t;
    solver.m_psys_max_vel_pos_iters = Input->m_psys_max_vel_pos_iters;
    solver.m_psys_max_vel_iters = Input->m_psys_max_vel_iters;
    solver.m_memory_high_water = 0;

    for (int i = _InterlockedExchangeAdd((volatile unsigned __int32 *)Input->m_list_island_cur, 1u);
        i < Input->m_list_island_count;
        i = _InterlockedExchangeAdd((volatile unsigned __int32 *)Input->m_list_island_cur, 1u))
    {
        solver.execute_constraint_solver(Input->m_list_island[i]);
    }

    return 0;
}

double __thiscall pulse_sum_normal::get_objective()
{
    pulse_sum_node *m_b1; // eax
    pulse_sum_node *m_b2; // eax
    float v4; // [esp-2Ch] [ebp-4Ch]
    float v5; // [esp-28h] [ebp-48h]
    float v6; // [esp-24h] [ebp-44h]
    float v7; // [esp-1Ch] [ebp-3Ch]
    float v8; // [esp-1Ch] [ebp-3Ch]
    float v9; // [esp-18h] [ebp-38h]
    float v10; // [esp-18h] [ebp-38h]
    float v11; // [esp-14h] [ebp-34h]
    float v12; // [esp-14h] [ebp-34h]
    float v13; // [esp-Ch] [ebp-2Ch]
    float v14; // [esp-8h] [ebp-28h]
    float v15; // [esp-4h] [ebp-24h]

    m_b1 = this->m_b1;
    v7 = m_b1->a_vel.y * this->m_b1_r.z - m_b1->a_vel.z * this->m_b1_r.y;
    v9 = this->m_b1_r.x * m_b1->a_vel.z - m_b1->a_vel.x * this->m_b1_r.z;
    v11 = m_b1->a_vel.x * this->m_b1_r.y - this->m_b1_r.x * m_b1->a_vel.y;
    v13 = m_b1->t_vel.x + v7;
    v14 = m_b1->t_vel.y + v9;
    v15 = m_b1->t_vel.z + v11;
    if ( this->m_b2 )
    {
        m_b2 = this->m_b2;
        v8 = m_b2->a_vel.y * this->m_b2_r.z - m_b2->a_vel.z * this->m_b2_r.y;
        v10 = m_b2->a_vel.z * this->m_b2_r.x - m_b2->a_vel.x * this->m_b2_r.z;
        v12 = m_b2->a_vel.x * this->m_b2_r.y - m_b2->a_vel.y * this->m_b2_r.x;
        v4 = m_b2->t_vel.x + v8;
        v5 = m_b2->t_vel.y + v10;
        v6 = m_b2->t_vel.z + v12;
        v13 = v13 - v4;
        v14 = v14 - v5;
        v15 = v15 - v6;
    }
    return (float)(this->m_ud.y * v14 + this->m_ud.x * v13 + this->m_ud.z * v15);
}

void __thiscall pulse_sum_normal::apply(const float *s_)
{
    pulse_sum_node *m_b1; // eax
    phys_vec3 *p_a_vel; // eax
    pulse_sum_node *m_b2; // eax
    double v5; // st7
    phys_vec3 *v6; // ecx
    float v7; // [esp-20h] [ebp-2Ch]
    float v8; // [esp-20h] [ebp-2Ch]
    float v9; // [esp-20h] [ebp-2Ch]
    float v10; // [esp-20h] [ebp-2Ch]
    float v11; // [esp-1Ch] [ebp-28h]
    float v12; // [esp-1Ch] [ebp-28h]
    float v13; // [esp-1Ch] [ebp-28h]
    float v14; // [esp-1Ch] [ebp-28h]
    float v15; // [esp-18h] [ebp-24h]
    float v16; // [esp-18h] [ebp-24h]
    float v17; // [esp-18h] [ebp-24h]
    float v18; // [esp-18h] [ebp-24h]
    float v19; // [esp-4h] [ebp-10h]
    float v20; // [esp-4h] [ebp-10h]

    m_b1 = this->m_b1;
    v19 = m_b1->m_inv_mass * *s_;
    v7 = this->m_ud.x * v19;
    v11 = this->m_ud.y * v19;
    v15 = v19 * this->m_ud.z;
    m_b1->t_vel.x = m_b1->t_vel.x + v7;
    m_b1->t_vel.y = m_b1->t_vel.y + v11;
    m_b1->t_vel.z = m_b1->t_vel.z + v15;
    p_a_vel = &this->m_b1->a_vel;
    v8 = this->m_b1_ap.x * *s_;
    v12 = this->m_b1_ap.y * *s_;
    v16 = *s_ * this->m_b1_ap.z;
    p_a_vel->x = v8 + p_a_vel->x;
    p_a_vel->y = p_a_vel->y + v12;
    p_a_vel->z = p_a_vel->z + v16;
    if ( this->m_b2 )
    {
        m_b2 = this->m_b2;
        v20 = m_b2->m_inv_mass * *s_;
        v9 = this->m_ud.x * v20;
        v13 = this->m_ud.y * v20;
        v17 = v20 * this->m_ud.z;
        m_b2->t_vel.x = m_b2->t_vel.x - v9;
        m_b2->t_vel.y = m_b2->t_vel.y - v13;
        m_b2->t_vel.z = m_b2->t_vel.z - v17;
        v10 = *s_ * this->m_b2_ap.x;
        v14 = this->m_b2_ap.y * *s_;
        v5 = *s_ * this->m_b2_ap.z;
        v6 = &this->m_b2->a_vel;
        v18 = v5;
        v6->x = v6->x - v10;
        v6->y = v6->y - v14;
        v6->z = v6->z - v18;
    }
}

double __thiscall pulse_sum_normal::clamp_pulse_sum(float ps)
{
    double result; // st7
    float v4; // [esp+4h] [ebp-4h]
    float v5; // [esp+4h] [ebp-4h]

    if ( (this->m_flags & 1) != 0 )
    {
        if ( !this->m_pulse_parent
            && _tlAssert(
                     "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\constraint_solver\\pulse_sum_normal_inline.h",
                     56,
                     "m_pulse_parent",
                     "") )
        {
            __debugbreak();
        }
        if ( this->m_pulse_limit_ratio < 0.0
            && _tlAssert(
                     "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\constraint_solver\\pulse_sum_normal_inline.h",
                     57,
                     "m_pulse_limit_ratio >= 0.0f",
                     "") )
        {
            __debugbreak();
        }
        v4 = fabs(this->m_pulse_parent->m_pulse_sum);
        v5 = v4 * this->m_pulse_limit_ratio;
        this->m_pulse_sum_max = v5;
        this->m_pulse_sum_min = -v5;
    }

    if ( this->m_pulse_sum_max < (double)this->m_pulse_sum_min
        && _tlAssert(
                 "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\constraint_solver\\pulse_sum_normal_inline.h",
                 61,
                 "m_pulse_sum_min <= m_pulse_sum_max",
                 "") )
    {
        __debugbreak();
    }
    result = ps;
    if ( this->m_pulse_sum_min <= (double)ps )
    {
        if ( this->m_pulse_sum_max >= result )
        {
            this->m_flags &= ~2u;
        }
        else
        {
            this->m_flags |= 2u;
            return this->m_pulse_sum_max;
        }
    }
    else
    {
        this->m_flags |= 2u;
        return this->m_pulse_sum_min;
    }
    return result;
}

void __thiscall pulse_sum_normal::SOLVER_apply_relaxation(float *error_sq, const bool add_error)
{
    double v4; // st7
    float m_objective; // [esp+8h] [ebp-8h] BYREF
    float m_last_pulse_sum; // [esp+Ch] [ebp-4h]
    float error_sq___a; // [esp+1Ch] [ebp+Ch]
    float error_sq___; // [esp+1Ch] [ebp+Ch]

    m_objective = pulse_sum_normal::get_objective() + this->m_cfm * this->m_pulse_sum - this->m_right_side;
    m_last_pulse_sum = this->m_pulse_sum;
    m_objective = this->m_pulse_sum - m_objective / this->m_denom;
    m_objective = pulse_sum_normal::clamp_pulse_sum(m_objective);
    v4 = m_objective;
    this->m_pulse_sum = m_objective;
    if ( (this->m_pulse_sum_min - 0.00009999999747378752 > v4 || this->m_pulse_sum_max + 0.00009999999747378752 < v4)
        && _tlAssert(
                 "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\constraint_solver\\pulse_sum_normal_inline.h",
                 90,
                 "m_pulse_sum >= (m_pulse_sum_min - .0001f) && m_pulse_sum <= (m_pulse_sum_max + .0001f)",
                 "") )
    {
        __debugbreak();
    }
    m_objective = this->m_pulse_sum - m_last_pulse_sum;
    pulse_sum_normal::apply(&m_objective);
    if ( add_error )
    {
        error_sq___a = (this->m_pulse_sum - m_last_pulse_sum) * this->m_denom;
        error_sq___ = error_sq___a * error_sq___a;
        if ( *error_sq < (double)error_sq___ )
            *error_sq = error_sq___;
    }
}

void __thiscall pulse_sum_normal::SOLVER_solver_prolog(float delta_t)
{
    double vel; // st7
    pulse_sum_cache *m_pulse_sum_cache; // eax
    pulse_sum_normal *v5; // ecx
    double m_right_side; // [esp+Ch] [ebp-8h]
    int savedregs; // [esp+14h] [ebp+0h] BYREF
    float delta_ta; // [esp+1Ch] [ebp+8h]

    m_right_side = this->m_right_side;
    vel = pulse_sum_normal::get_vel();
    m_pulse_sum_cache = this->m_pulse_sum_cache;
    this->m_right_side = m_right_side - vel;
    delta_ta = m_pulse_sum_cache->m_pulse_sum * delta_t;
    this->m_pulse_sum = delta_ta;
    this->m_pulse_sum = pulse_sum_normal::clamp_pulse_sum(delta_ta);
    pulse_sum_normal::apply(&this->m_pulse_sum);
}

const phys_vec3 *__thiscall pulse_sum_point::get_objective(phys_vec3 *result)
{
    bool v2; // zf
    pulse_sum_node *m_b1; // edx
    const phys_vec3 *v4; // eax
    double v5; // st7
    double v6; // st6
    pulse_sum_node *m_b2; // edx
    float v8; // [esp-30h] [ebp-3Ch]
    float v9; // [esp-2Ch] [ebp-38h]
    float v10; // [esp-28h] [ebp-34h]
    float v11; // [esp-20h] [ebp-2Ch]
    float v12; // [esp-20h] [ebp-2Ch]
    float v13; // [esp-1Ch] [ebp-28h]
    float v14; // [esp-1Ch] [ebp-28h]
    float v15; // [esp-18h] [ebp-24h]
    float v16; // [esp-18h] [ebp-24h]
    float v17; // [esp-4h] [ebp-10h]
    float v18; // [esp-4h] [ebp-10h]
    float v19; // [esp-4h] [ebp-10h]

    v2 = this->m_b2 == 0;
    m_b1 = this->m_b1;
    v4 = result;
    v11 = m_b1->a_vel.y * this->m_b1_r.z - m_b1->a_vel.z * this->m_b1_r.y;
    v13 = this->m_b1_r.x * m_b1->a_vel.z - this->m_b1_r.z * m_b1->a_vel.x;
    v15 = this->m_b1_r.y * m_b1->a_vel.x - this->m_b1_r.x * m_b1->a_vel.y;
    v17 = m_b1->t_vel.x + v11;
    v5 = v17;
    result->x = v17;
    v18 = m_b1->t_vel.y + v13;
    v6 = v18;
    result->y = v18;
    v19 = m_b1->t_vel.z + v15;
    result->z = v19;
    if ( !v2 )
    {
        m_b2 = this->m_b2;
        v12 = m_b2->a_vel.y * this->m_b2_r.z - m_b2->a_vel.z * this->m_b2_r.y;
        v14 = m_b2->a_vel.z * this->m_b2_r.x - m_b2->a_vel.x * this->m_b2_r.z;
        v16 = m_b2->a_vel.x * this->m_b2_r.y - m_b2->a_vel.y * this->m_b2_r.x;
        v8 = m_b2->t_vel.x + v12;
        v9 = m_b2->t_vel.y + v14;
        v10 = m_b2->t_vel.z + v16;
        result->x = v5 - v8;
        result->y = v6 - v9;
        result->z = v19 - v10;
    }
    return v4;
}

void __thiscall pulse_sum_point::apply(const phys_vec3 *s_)
{
    pulse_sum_node *m_b1; // edx
    phys_vec3 *p_a_vel; // edx
    pulse_sum_node *m_b2; // edx
    double v5; // st7
    double v6; // st7
    double v7; // st7
    double x; // st7
    double v9; // st7
    phys_vec3 *v10; // ecx
    float v11; // [esp-50h] [ebp-5Ch]
    float v12; // [esp-50h] [ebp-5Ch]
    float v13; // [esp-50h] [ebp-5Ch]
    float v14; // [esp-50h] [ebp-5Ch]
    float v15; // [esp-4Ch] [ebp-58h]
    float v16; // [esp-4Ch] [ebp-58h]
    float v17; // [esp-4Ch] [ebp-58h]
    float v18; // [esp-4Ch] [ebp-58h]
    float v19; // [esp-48h] [ebp-54h]
    float v20; // [esp-48h] [ebp-54h]
    float v21; // [esp-48h] [ebp-54h]
    float v22; // [esp-48h] [ebp-54h]
    float v23; // [esp-40h] [ebp-4Ch]
    float v24; // [esp-40h] [ebp-4Ch]
    float v25; // [esp-3Ch] [ebp-48h]
    float v26; // [esp-3Ch] [ebp-48h]
    float v27; // [esp-38h] [ebp-44h]
    float v28; // [esp-38h] [ebp-44h]
    float v29; // [esp-30h] [ebp-3Ch]
    float v30; // [esp-30h] [ebp-3Ch]
    float v31; // [esp-30h] [ebp-3Ch]
    float v32; // [esp-2Ch] [ebp-38h]
    float v33; // [esp-2Ch] [ebp-38h]
    float v34; // [esp-2Ch] [ebp-38h]
    float v35; // [esp-28h] [ebp-34h]
    float v36; // [esp-28h] [ebp-34h]
    float v37; // [esp-28h] [ebp-34h]
    float v38; // [esp-20h] [ebp-2Ch]
    float v39; // [esp-20h] [ebp-2Ch]
    float v40; // [esp-20h] [ebp-2Ch]
    float v41; // [esp-1Ch] [ebp-28h]
    float v42; // [esp-1Ch] [ebp-28h]
    float v43; // [esp-1Ch] [ebp-28h]
    float v44; // [esp-18h] [ebp-24h]
    float v45; // [esp-18h] [ebp-24h]
    float v46; // [esp-18h] [ebp-24h]
    float m_inv_mass; // [esp-4h] [ebp-10h]
    float z; // [esp-4h] [ebp-10h]
    float y; // [esp-4h] [ebp-10h]

    m_b1 = this->m_b1;
    m_inv_mass = m_b1->m_inv_mass;
    v38 = s_->x * m_inv_mass;
    v41 = m_inv_mass * s_->y;
    v44 = m_inv_mass * s_->z;
    m_b1->t_vel.x = m_b1->t_vel.x + v38;
    m_b1->t_vel.y = m_b1->t_vel.y + v41;
    m_b1->t_vel.z = m_b1->t_vel.z + v44;
    p_a_vel = &this->m_b1->a_vel;
    z = s_->z;
    v11 = this->m_b1_apz.x * z;
    v15 = this->m_b1_apz.y * z;
    v19 = z * this->m_b1_apz.z;
    y = s_->y;
    v29 = y * this->m_b1_apy.x;
    v32 = this->m_b1_apy.y * y;
    v35 = y * this->m_b1_apy.z;
    v39 = this->m_b1_apx.x * s_->x;
    v42 = this->m_b1_apx.y * s_->x;
    v45 = s_->x * this->m_b1_apx.z;
    v23 = v39 + v29;
    v25 = v42 + v32;
    v27 = v45 + v35;
    v30 = v23 + v11;
    v33 = v25 + v15;
    v36 = v27 + v19;
    p_a_vel->x = p_a_vel->x + v30;
    p_a_vel->y = p_a_vel->y + v33;
    p_a_vel->z = v36 + p_a_vel->z;
    if ( this->m_b2 )
    {
        m_b2 = this->m_b2;
        v5 = m_b2->m_inv_mass;
        v12 = s_->x * v5;
        v16 = v5 * s_->y;
        v20 = v5 * s_->z;
        m_b2->t_vel.x = m_b2->t_vel.x - v12;
        m_b2->t_vel.y = m_b2->t_vel.y - v16;
        m_b2->t_vel.z = m_b2->t_vel.z - v20;
        v6 = s_->z;
        v40 = this->m_b2_apz.x * v6;
        v43 = this->m_b2_apz.y * v6;
        v46 = v6 * this->m_b2_apz.z;
        v7 = s_->y;
        v24 = v7 * this->m_b2_apy.x;
        v26 = this->m_b2_apy.y * v7;
        v28 = v7 * this->m_b2_apy.z;
        x = s_->x;
        v13 = this->m_b2_apx.x * x;
        v17 = this->m_b2_apx.y * x;
        v9 = x * this->m_b2_apx.z;
        v10 = &this->m_b2->a_vel;
        v21 = v9;
        v31 = v13 + v24;
        v34 = v17 + v26;
        v37 = v21 + v28;
        v14 = v31 + v40;
        v18 = v34 + v43;
        v22 = v37 + v46;
        v10->x = v10->x - v14;
        v10->y = v10->y - v18;
        v10->z = v10->z - v22;
    }
}

void __thiscall pulse_sum_point::SOLVER_solver_intermediate(float delta_t)
{
    double v2; // st7
    float delta_ta; // [esp+8h] [ebp+8h]
    float delta_tb; // [esp+8h] [ebp+8h]
    float delta_tc; // [esp+8h] [ebp+8h]

    v2 = delta_t;
    delta_ta = this->m_pulse_sum.x / delta_t;
    this->m_pulse_sum_cache->m_pulse_sum = delta_ta;
    delta_tb = this->m_pulse_sum.y / v2;
    this->m_pulse_sum_cache[1].m_pulse_sum = delta_tb;
    delta_tc = this->m_pulse_sum.z / v2;
    this->m_pulse_sum_cache[2].m_pulse_sum = delta_tc;
    this->m_right_side.x = this->m_big_dirt.x + this->m_right_side.x;
    this->m_right_side.y = this->m_big_dirt.y + this->m_right_side.y;
    this->m_right_side.z = this->m_big_dirt.z + this->m_right_side.z;
}

void pulse_sum_point::SOLVER_apply_relaxation(float *error_sq)
{
    const phys_vec3 *objective; // eax
    float x; // ecx
    float y; // edx
    double z; // st7
    float v8; // eax
    double v9; // st7
    double v10; // st7
    double v11; // st7
    double v12; // st7
    double v13; // st7
    phys_vec3 v14; // [esp-1Ch] [ebp-6Ch] BYREF
    float v15; // [esp-Ch] [ebp-5Ch]
    float v16; // [esp-8h] [ebp-58h]
    float v17; // [esp-4h] [ebp-54h]
    phys_vec3 error_; // [esp+0h] [ebp-50h]
    phys_vec3 v19; // [esp+14h] [ebp-3Ch] BYREF
    float m_objective_4; // [esp+24h] [ebp-2Ch]
    float m_objective_8; // [esp+28h] [ebp-28h]
    float m_objective_12; // [esp+2Ch] [ebp-24h]
    phys_vec3 m_last_pulse_sum; // [esp+30h] [ebp-20h]
    float m_cfm; // [esp+40h] [ebp-10h]
    //int v25; // [esp+44h] [ebp-Ch]
    //void *v26; // [esp+48h] [ebp-8h]
    //void *retaddr; // [esp+50h] [ebp+0h]
    //
    //v25 = a2;
    //v26 = retaddr;
    m_cfm = this->m_cfm;
    v19.x = this->m_pulse_sum.x * m_cfm;
    v19.y = this->m_pulse_sum.y * m_cfm;
    v19.z = m_cfm * this->m_pulse_sum.z;
    objective = pulse_sum_point::get_objective(&v14);
    x = this->m_pulse_sum.x;
    y = this->m_pulse_sum.y;
    m_objective_4 = objective->x + v19.x;
    m_objective_8 = objective->y + v19.y;
    z = objective->z;
    v8 = this->m_pulse_sum.z;
    m_objective_12 = z + v19.z;
    v9 = m_objective_4;
    m_objective_4 = x;
    v10 = v9 - this->m_right_side.x;
    m_last_pulse_sum.x = this->m_pulse_sum.w;
    v19.x = v10;
    v11 = m_objective_8;
    m_objective_8 = y;
    v19.y = v11 - this->m_right_side.y;
    v12 = m_objective_12;
    m_objective_12 = v8;
    v19.z = v12 - this->m_right_side.z;
    v14.x = v19.z * this->m_cr12.x;
    v14.y = this->m_cr12.y * v19.z;
    v14.z = v19.z * this->m_cr12.z;
    error_.y = this->m_cr31.x * v19.y;
    error_.z = this->m_cr31.y * v19.y;
    error_.w = v19.y * this->m_cr31.z;
    v13 = v19.x;
    v19.x = v19.x * this->m_cr23.x;
    v19.y = this->m_cr23.y * v13;
    v19.z = v13 * this->m_cr23.z;
    v15 = v19.x + error_.y;
    v16 = v19.y + error_.z;
    v17 = v19.z + error_.w;
    error_.y = v15 + v14.x;
    error_.z = v16 + v14.y;
    error_.w = v17 + v14.z;
    this->m_pulse_sum.x = this->m_pulse_sum.x - error_.y;
    this->m_pulse_sum.y = this->m_pulse_sum.y - error_.z;
    this->m_pulse_sum.z = this->m_pulse_sum.z - error_.w;
    v19.x = this->m_pulse_sum.x - m_objective_4;
    v19.y = this->m_pulse_sum.y - m_objective_8;
    v19.z = this->m_pulse_sum.z - m_objective_12;
    pulse_sum_point::apply(&v19);
    v14.x = this->m_denom.x * v19.x;
    v14.y = this->m_denom.y * v19.y;
    v14.z = this->m_denom.z * v19.z;
    v15 = v14.x * v14.x;
    v16 = v14.y * v14.y;
    v17 = v14.z * v14.z;
    if (*error_sq < (double)v15)
        *error_sq = v15;
    if (*error_sq < (double)v16)
        *error_sq = v16;
    if (*error_sq < (double)v17)
        *error_sq = v17;
}

void    pulse_sum_angular::calc_abs()
{
    phys_vec3 *p_m_ud; // edi
    const phys_vec3 *v4; // eax
    pulse_sum_node *m_b2; // eax
    const phys_vec3 *v6; // eax
    phys_vec3 v7; // [esp-20h] [ebp-2Ch] BYREF
    float v8; // [esp-4h] [ebp-10h]
    //int v9; // [esp+0h] [ebp-Ch]
    //void *v10; // [esp+4h] [ebp-8h]
    //void *retaddr; // [esp+Ch] [ebp+0h]
    //
    //v9 = a2;
    //v10 = retaddr;
    p_m_ud = &this->m_ud;
    v4 = phys_multiply(&v7, &this->m_b1->m_world_inv_inertia, &this->m_ud);
    this->m_b1_ap.x = v4->x;
    this->m_b1_ap.y = v4->y;
    this->m_b1_ap.z = v4->z;
    m_b2 = this->m_b2;
    this->m_denom = this->m_b1_ap.y * p_m_ud->y + this->m_b1_ap.x * p_m_ud->x + this->m_b1_ap.z * p_m_ud->z;
    if ( m_b2 )
    {
        v6 = phys_multiply(&v7, &m_b2->m_world_inv_inertia, p_m_ud);
        this->m_b2_ap.x = v6->x;
        this->m_b2_ap.y = v6->y;
        this->m_b2_ap.z = v6->z;
        v8 = this->m_b2_ap.y * p_m_ud->y + this->m_b2_ap.x * p_m_ud->x + this->m_b2_ap.z * p_m_ud->z;
        this->m_denom = v8 + this->m_denom;
    }
    if ( this->m_denom <= 0.0000099999997 )
    {
        if ( _tlAssert(
                     "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\constraint_solver\\pulse_sum_angular_inline.h",
                     10,
                     "m_denom > 0.00001f",
                     "") )
        {
            __debugbreak();
        }
    }
}

double __thiscall pulse_sum_angular::get_objective()
{
    //pulse_sum_node *m_b1; // eax
    //float x; // edx
    //float v3; // edx
    //pulse_sum_node *m_b2; // eax
    //float v6; // [esp-Ch] [ebp-2Ch]
    //float v7; // [esp-8h] [ebp-28h]
    //float v8; // [esp-4h] [ebp-24h]
    //
    //m_b1 = this->m_b1;
    //x = m_b1->a_vel.x;
    //m_b1 = (pulse_sum_node *)((char *)m_b1 + 96);
    //v6 = x;
    //v7 = *((float *)&m_b1->phys_link_list_base<pulse_sum_node> + 1);
    //v3 = *((float *)&m_b1->phys_link_list_base<pulse_sum_node> + 2);
    //m_b2 = this->m_b2;
    //v8 = v3;
    //if ( m_b2 )
    //{
    //    v6 = v6 - m_b2->a_vel.x;
    //    v7 = v7 - m_b2->a_vel.y;
    //    v8 = v3 - m_b2->a_vel.z;
    //}
    //return (float)(this->m_ud.y * v7 + this->m_ud.x * v6 + this->m_ud.z * v8);

    // aislop
    // Load angular velocity of body 1
    phys_vec3 rel_w = m_b1->a_vel;

    // Subtract body 2 angular velocity if present
    if (m_b2)
        rel_w -= m_b2->a_vel;

    // Return projection onto constraint axis
    return m_ud.x * rel_w.x +
        m_ud.y * rel_w.y +
        m_ud.z * rel_w.z;
}

void __thiscall pulse_sum_angular::apply(const float *s_)
{
    phys_vec3 *p_a_vel; // eax
    pulse_sum_node *m_b2; // eax
    float v4; // [esp-20h] [ebp-2Ch]
    float v5; // [esp-20h] [ebp-2Ch]
    float v6; // [esp-1Ch] [ebp-28h]
    float v7; // [esp-1Ch] [ebp-28h]
    float v8; // [esp-18h] [ebp-24h]
    float v9; // [esp-18h] [ebp-24h]

    p_a_vel = &this->m_b1->a_vel;
    v4 = this->m_b1_ap.x * *s_;
    v6 = this->m_b1_ap.y * *s_;
    v8 = *s_ * this->m_b1_ap.z;
    p_a_vel->x = p_a_vel->x + v4;
    p_a_vel->y = p_a_vel->y + v6;
    p_a_vel->z = p_a_vel->z + v8;
    m_b2 = this->m_b2;
    if ( m_b2 )
    {
        v5 = *s_ * this->m_b2_ap.x;
        v7 = this->m_b2_ap.y * *s_;
        v9 = *s_ * this->m_b2_ap.z;
        m_b2->a_vel.x = m_b2->a_vel.x - v5;
        m_b2->a_vel.y = m_b2->a_vel.y - v7;
        m_b2->a_vel.z = m_b2->a_vel.z - v9;
    }
}

double __thiscall pulse_sum_angular::clamp_pulse_sum(float ps)
{
    double result; // st7

    if ( this->m_pulse_sum_max < (double)this->m_pulse_sum_min
        && _tlAssert(
                 "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\constraint_solver\\pulse_sum_angular_inline.h",
                 40,
                 "m_pulse_sum_min <= m_pulse_sum_max",
                 "") )
    {
        __debugbreak();
    }
    result = ps;
    if ( this->m_pulse_sum_min > (double)ps )
        return this->m_pulse_sum_min;
    if ( this->m_pulse_sum_max < result )
        return this->m_pulse_sum_max;
    return result;
}

void __thiscall pulse_sum_angular::SOLVER_apply_relaxation(float *error_sq)
{
    double v3; // st7
    float error_sq___; // [esp+8h] [ebp-8h] BYREF
    float m_last_pulse_sum; // [esp+Ch] [ebp-4h]

    error_sq___ = pulse_sum_angular::get_objective() + this->m_cfm * this->m_pulse_sum - this->m_right_side;
    m_last_pulse_sum = this->m_pulse_sum;
    error_sq___ = this->m_pulse_sum - error_sq___ / this->m_denom;
    error_sq___ = pulse_sum_angular::clamp_pulse_sum(error_sq___);
    v3 = error_sq___;
    this->m_pulse_sum = error_sq___;
    if ( (this->m_pulse_sum_min - 0.00009999999747378752 > v3 || this->m_pulse_sum_max + 0.00009999999747378752 < v3)
        && _tlAssert(
                 "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\constraint_solver\\pulse_sum_angular_inline.h",
                 61,
                 "m_pulse_sum >= (m_pulse_sum_min - .0001f) && m_pulse_sum <= (m_pulse_sum_max + .0001f)",
                 "") )
    {
        __debugbreak();
    }
    error_sq___ = this->m_pulse_sum - m_last_pulse_sum;
    pulse_sum_angular::apply(&error_sq___);
    error_sq___ = (this->m_pulse_sum - m_last_pulse_sum) * this->m_denom;
    error_sq___ = error_sq___ * error_sq___;
    if ( *error_sq < (double)error_sq___ )
        *error_sq = error_sq___;
}

void __thiscall pulse_sum_wheel::SOLVER_solver_intermediate(float delta_t)
{
    double v2; // st7
    pulse_sum_normal *m_side; // eax
    pulse_sum_normal *m_fwd; // ecx
    float delta_ta; // [esp+8h] [ebp+8h]
    float delta_tb; // [esp+8h] [ebp+8h]
    float delta_tc; // [esp+8h] [ebp+8h]

    v2 = delta_t;
    delta_ta = this->m_suspension.m_pulse_sum / delta_t;
    this->m_suspension.m_pulse_sum_cache->m_pulse_sum = delta_ta;
    this->m_suspension.m_right_side = this->m_suspension.m_big_dirt + this->m_suspension.m_right_side;
    m_side = this->m_side;
    if ( m_side )
    {
        delta_tb = m_side->m_pulse_sum / v2;
        m_side->m_pulse_sum_cache->m_pulse_sum = delta_tb;
        m_side->m_right_side = m_side->m_big_dirt + m_side->m_right_side;
        m_fwd = this->m_fwd;
        if ( m_fwd )
        {
            delta_tc = m_fwd->m_pulse_sum / v2;
            m_fwd->m_pulse_sum_cache->m_pulse_sum = delta_tc;
            m_fwd->m_right_side = m_fwd->m_big_dirt + m_fwd->m_right_side;
        }
    }
}

void pulse_sum_contact_point::calc_abs(pulse_sum_contact *psc)
{
    const phys_vec3 *v5; // eax
    pulse_sum_node *m_b1; // eax
    const phys_vec3 *v7; // eax
    pulse_sum_node *v8; // ecx
    const phys_vec3 *v9; // eax
    pulse_sum_node *v10; // edx
    double y; // st6
    double x; // st7
    double z; // st5
    pulse_sum_node *m_b2; // eax
    const phys_vec3 *v15; // eax
    pulse_sum_node *v16; // eax
    const phys_vec3 *v17; // eax
    pulse_sum_node *v18; // ecx
    const phys_vec3 *v19; // eax
    pulse_sum_node *v20; // edx
    double v21; // st6
    double v22; // st7
    double v23; // st5
    double v24; // st7
    phys_mat44 *v25; // [esp-4Ch] [ebp-8Ch]
    phys_mat44 *v26; // [esp-4Ch] [ebp-8Ch]
    phys_mat44 *v27; // [esp-40h] [ebp-80h]
    phys_mat44 *v28; // [esp-40h] [ebp-80h]
    phys_mat44 *p_m_world_inv_inertia; // [esp-34h] [ebp-74h]
    phys_vec3 v30; // [esp-1Ch] [ebp-5Ch] BYREF
    phys_vec3 v31; // [esp-Ch] [ebp-4Ch] BYREF
    phys_vec3 b2_t_f2_4; // [esp+4h] [ebp-3Ch] BYREF
    phys_vec3 b2_t_f1_4; // [esp+14h] [ebp-2Ch] BYREF
    float b2_t_n_12; // [esp+2Ch] [ebp-14h]
    float v35; // [esp+30h] [ebp-10h]
    //int v36; // [esp+34h] [ebp-Ch]
    //float oo_det; // [esp+38h] [ebp-8h]
    //float retaddr; // [esp+40h] [ebp+0h]
    //
    //v36 = a2;
    //oo_det = retaddr;
    p_m_world_inv_inertia = &psc->m_b1->m_world_inv_inertia;
    b2_t_f2_4.x = psc->m_ud_n.z * this->m_b1_r.y - psc->m_ud_n.y * this->m_b1_r.z;
    b2_t_f2_4.y = this->m_b1_r.z * psc->m_ud_n.x - psc->m_ud_n.z * this->m_b1_r.x;
    b2_t_f2_4.z = psc->m_ud_n.y * this->m_b1_r.x - this->m_b1_r.y * psc->m_ud_n.x;
    v5 = phys_multiply(&v31, p_m_world_inv_inertia, &b2_t_f2_4);
    this->m_b1_ap_n.x = v5->x;
    this->m_b1_ap_n.y = v5->y;
    this->m_b1_ap_n.z = v5->z;
    m_b1 = psc->m_b1;
    b2_t_n_12 = this->m_b1_ap_n.y * b2_t_f2_4.y + this->m_b1_ap_n.x * b2_t_f2_4.x + this->m_b1_ap_n.z * b2_t_f2_4.z;
    this->m_denom_xx = b2_t_n_12 + m_b1->m_inv_mass;
    v27 = &psc->m_b1->m_world_inv_inertia;
    b2_t_f1_4.x = psc->m_ud_f1.z * this->m_b1_r.y - psc->m_ud_f1.y * this->m_b1_r.z;
    b2_t_f1_4.y = psc->m_ud_f1.x * this->m_b1_r.z - psc->m_ud_f1.z * this->m_b1_r.x;
    b2_t_f1_4.z = psc->m_ud_f1.y * this->m_b1_r.x - psc->m_ud_f1.x * this->m_b1_r.y;
    v7 = phys_multiply(&v31, v27, &b2_t_f1_4);
    this->m_b1_ap_f1.x = v7->x;
    this->m_b1_ap_f1.y = v7->y;
    this->m_b1_ap_f1.z = v7->z;
    v8 = psc->m_b1;
    b2_t_n_12 = this->m_b1_ap_f1.y * b2_t_f1_4.y + this->m_b1_ap_f1.x * b2_t_f1_4.x + this->m_b1_ap_f1.z * b2_t_f1_4.z;
    this->m_denom_yy = b2_t_n_12 + v8->m_inv_mass;
    v25 = &psc->m_b1->m_world_inv_inertia;
    v31.x = psc->m_ud_f2.z * this->m_b1_r.y - this->m_b1_r.z * psc->m_ud_f2.y;
    v31.y = psc->m_ud_f2.x * this->m_b1_r.z - psc->m_ud_f2.z * this->m_b1_r.x;
    v31.z = this->m_b1_r.x * psc->m_ud_f2.y - psc->m_ud_f2.x * this->m_b1_r.y;
    v9 = phys_multiply(&v30, v25, &v31);
    this->m_b1_ap_f2.x = v9->x;
    this->m_b1_ap_f2.y = v9->y;
    this->m_b1_ap_f2.z = v9->z;
    v10 = psc->m_b1;
    b2_t_n_12 = this->m_b1_ap_f2.y * v31.y + this->m_b1_ap_f2.x * v31.x + this->m_b1_ap_f2.z * v31.z;
    this->m_denom_zz = b2_t_n_12 + v10->m_inv_mass;
    v35 = this->m_b1_ap_f2.y * b2_t_f1_4.y + this->m_b1_ap_f2.x * b2_t_f1_4.x + this->m_b1_ap_f2.z * b2_t_f1_4.z;
    y = b2_t_f2_4.y;
    x = b2_t_f2_4.x;
    z = b2_t_f2_4.z;
    this->m_denom_xy = this->m_b1_ap_f1.z * b2_t_f2_4.z
        + this->m_b1_ap_f1.x * b2_t_f2_4.x
        + this->m_b1_ap_f1.y * b2_t_f2_4.y;
    this->m_denom_xz = x * this->m_b1_ap_f2.x + y * this->m_b1_ap_f2.y + z * this->m_b1_ap_f2.z;
    m_b2 = psc->m_b2;
    if (m_b2)
    {
        b2_t_f1_4.x = this->m_b2_r.y * psc->m_ud_n.z - this->m_b2_r.z * psc->m_ud_n.y;
        b2_t_f1_4.y = this->m_b2_r.z * psc->m_ud_n.x - this->m_b2_r.x * psc->m_ud_n.z;
        b2_t_f1_4.z = psc->m_ud_n.y * this->m_b2_r.x - psc->m_ud_n.x * this->m_b2_r.y;
        v15 = phys_multiply(&v30, &m_b2->m_world_inv_inertia, &b2_t_f1_4);
        this->m_b2_ap_n.x = v15->x;
        this->m_b2_ap_n.y = v15->y;
        this->m_b2_ap_n.z = v15->z;
        v16 = psc->m_b2;
        b2_t_n_12 = this->m_b2_ap_n.y * b2_t_f1_4.y + this->m_b2_ap_n.x * b2_t_f1_4.x + this->m_b2_ap_n.z * b2_t_f1_4.z;
        this->m_denom_xx = b2_t_n_12 + v16->m_inv_mass + this->m_denom_xx;
        v28 = &psc->m_b2->m_world_inv_inertia;
        b2_t_f2_4.x = this->m_b2_r.y * psc->m_ud_f1.z - this->m_b2_r.z * psc->m_ud_f1.y;
        b2_t_f2_4.y = psc->m_ud_f1.x * this->m_b2_r.z - this->m_b2_r.x * psc->m_ud_f1.z;
        b2_t_f2_4.z = this->m_b2_r.x * psc->m_ud_f1.y - this->m_b2_r.y * psc->m_ud_f1.x;
        v17 = phys_multiply(&v30, v28, &b2_t_f2_4);
        this->m_b2_ap_f1.x = v17->x;
        this->m_b2_ap_f1.y = v17->y;
        this->m_b2_ap_f1.z = v17->z;
        v18 = psc->m_b2;
        b2_t_n_12 = this->m_b2_ap_f1.y * b2_t_f2_4.y + this->m_b2_ap_f1.x * b2_t_f2_4.x + this->m_b2_ap_f1.z * b2_t_f2_4.z;
        this->m_denom_yy = b2_t_n_12 + v18->m_inv_mass + this->m_denom_yy;
        v26 = &psc->m_b2->m_world_inv_inertia;
        v31.x = this->m_b2_r.y * psc->m_ud_f2.z - this->m_b2_r.z * psc->m_ud_f2.y;
        v31.y = psc->m_ud_f2.x * this->m_b2_r.z - psc->m_ud_f2.z * this->m_b2_r.x;
        v31.z = psc->m_ud_f2.y * this->m_b2_r.x - psc->m_ud_f2.x * this->m_b2_r.y;
        v19 = phys_multiply(&v30, v26, &v31);
        this->m_b2_ap_f2.x = v19->x;
        this->m_b2_ap_f2.y = v19->y;
        this->m_b2_ap_f2.z = v19->z;
        v20 = psc->m_b2;
        b2_t_n_12 = this->m_b2_ap_f2.y * v31.y + v31.x * this->m_b2_ap_f2.x + this->m_b2_ap_f2.z * v31.z;
        this->m_denom_zz = b2_t_n_12 + v20->m_inv_mass + this->m_denom_zz;
        b2_t_n_12 = this->m_b2_ap_f2.y * b2_t_f2_4.y + b2_t_f2_4.x * this->m_b2_ap_f2.x + this->m_b2_ap_f2.z * b2_t_f2_4.z;
        v35 = b2_t_n_12 + v35;
        v21 = b2_t_f1_4.y;
        v22 = b2_t_f1_4.x;
        v23 = b2_t_f1_4.z;
        b2_t_n_12 = this->m_b2_ap_f1.z * b2_t_f1_4.z + this->m_b2_ap_f1.x * b2_t_f1_4.x + this->m_b2_ap_f1.y * b2_t_f1_4.y;
        this->m_denom_xy = b2_t_n_12 + this->m_denom_xy;
        b2_t_n_12 = v22 * this->m_b2_ap_f2.x + v21 * this->m_b2_ap_f2.y + v23 * this->m_b2_ap_f2.z;
        this->m_denom_xz = b2_t_n_12 + this->m_denom_xz;
    }
    b2_t_n_12 = this->m_denom_zz * this->m_denom_yy - v35 * v35;

    //if (b2_t_n_12 <= 0.0
    //    && _tlAssert(
    //        "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\constraint_solver\\pulse_sum_contact_inline.h",
    //        38,
    //        "det > 0.0f",
    //        "")
    //{
    //    __debugbreak();
    //}

    b2_t_n_12 = 1.0 / b2_t_n_12;
    v24 = b2_t_n_12;
    this->m_inv_yy = b2_t_n_12 * this->m_denom_zz;
    this->m_inv_yz = -v35 * v24;
    this->m_inv_zz = v24 * this->m_denom_yy;
}

const phys_vec3 *__thiscall pulse_sum_contact_point::get_objective(phys_vec3 *result, pulse_sum_contact *psc)
{
    pulse_sum_node *m_b1; // eax
    pulse_sum_node *m_b2; // eax
    const phys_vec3 *v5; // eax
    float v6; // [esp-2Ch] [ebp-3Ch]
    float v7; // [esp-28h] [ebp-38h]
    float v8; // [esp-24h] [ebp-34h]
    float v9; // [esp-1Ch] [ebp-2Ch]
    float v10; // [esp-1Ch] [ebp-2Ch]
    float v11; // [esp-18h] [ebp-28h]
    float v12; // [esp-18h] [ebp-28h]
    float v13; // [esp-14h] [ebp-24h]
    float v14; // [esp-14h] [ebp-24h]
    float v15; // [esp-Ch] [ebp-1Ch]
    float v16; // [esp-8h] [ebp-18h]
    float v17; // [esp-4h] [ebp-14h]

    m_b1 = psc->m_b1;
    v9 = m_b1->a_vel.y * this->m_b1_r.z - m_b1->a_vel.z * this->m_b1_r.y;
    v11 = m_b1->a_vel.z * this->m_b1_r.x - m_b1->a_vel.x * this->m_b1_r.z;
    v13 = this->m_b1_r.y * m_b1->a_vel.x - this->m_b1_r.x * m_b1->a_vel.y;
    v15 = m_b1->t_vel.x + v9;
    v16 = m_b1->t_vel.y + v11;
    v17 = m_b1->t_vel.z + v13;
    if ( psc->m_b2 )
    {
        m_b2 = psc->m_b2;
        v10 = m_b2->a_vel.y * this->m_b2_r.z - m_b2->a_vel.z * this->m_b2_r.y;
        v12 = m_b2->a_vel.z * this->m_b2_r.x - this->m_b2_r.z * m_b2->a_vel.x;
        v14 = m_b2->a_vel.x * this->m_b2_r.y - m_b2->a_vel.y * this->m_b2_r.x;
        v6 = m_b2->t_vel.x + v10;
        v7 = m_b2->t_vel.y + v12;
        v8 = m_b2->t_vel.z + v14;
        v15 = v15 - v6;
        v16 = v16 - v7;
        v17 = v17 - v8;
    }
    v5 = result;
    result->x = psc->m_ud_n.z * v17 + psc->m_ud_n.x * v15 + psc->m_ud_n.y * v16;
    result->y = psc->m_ud_f1.y * v16 + v15 * psc->m_ud_f1.x + psc->m_ud_f1.z * v17;
    result->z = v15 * psc->m_ud_f2.x + v16 * psc->m_ud_f2.y + v17 * psc->m_ud_f2.z;
    return v5;
}

void __thiscall pulse_sum_contact_point::apply(pulse_sum_contact *psc, const phys_vec3 *s_)
{
    pulse_sum_node *m_b1; // esi
    double v4; // st7
    double v5; // st5
    double v6; // st6
    phys_vec3 *p_a_vel; // esi
    pulse_sum_node *m_b2; // esi
    phys_vec3 *v9; // eax
    double v10; // st7
    double v11; // st7
    float v12; // [esp-2Ch] [ebp-5Ch]
    float v13; // [esp-2Ch] [ebp-5Ch]
    float v14; // [esp-2Ch] [ebp-5Ch]
    float v15; // [esp-2Ch] [ebp-5Ch]
    float v16; // [esp-2Ch] [ebp-5Ch]
    float v17; // [esp-2Ch] [ebp-5Ch]
    float v18; // [esp-2Ch] [ebp-5Ch]
    float v19; // [esp-28h] [ebp-58h]
    float v20; // [esp-28h] [ebp-58h]
    float v21; // [esp-28h] [ebp-58h]
    float v22; // [esp-28h] [ebp-58h]
    float v23; // [esp-28h] [ebp-58h]
    float v24; // [esp-28h] [ebp-58h]
    float v25; // [esp-28h] [ebp-58h]
    float v26; // [esp-24h] [ebp-54h]
    float v27; // [esp-24h] [ebp-54h]
    float v28; // [esp-24h] [ebp-54h]
    float v29; // [esp-24h] [ebp-54h]
    float v30; // [esp-24h] [ebp-54h]
    float v31; // [esp-24h] [ebp-54h]
    float v32; // [esp-24h] [ebp-54h]
    float v33; // [esp-1Ch] [ebp-4Ch]
    float v34; // [esp-1Ch] [ebp-4Ch]
    float v35; // [esp-1Ch] [ebp-4Ch]
    float v36; // [esp-18h] [ebp-48h]
    float v37; // [esp-18h] [ebp-48h]
    float v38; // [esp-18h] [ebp-48h]
    float v39; // [esp-14h] [ebp-44h]
    float v40; // [esp-14h] [ebp-44h]
    float v41; // [esp-14h] [ebp-44h]
    float v42; // [esp-Ch] [ebp-3Ch]
    float v43; // [esp-Ch] [ebp-3Ch]
    float v44; // [esp-Ch] [ebp-3Ch]
    float v45; // [esp-Ch] [ebp-3Ch]
    float v46; // [esp-8h] [ebp-38h]
    float v47; // [esp-8h] [ebp-38h]
    float v48; // [esp-8h] [ebp-38h]
    float v49; // [esp-8h] [ebp-38h]
    float v50; // [esp-4h] [ebp-34h]
    float v51; // [esp-4h] [ebp-34h]
    float v52; // [esp-4h] [ebp-34h]
    float v53; // [esp-4h] [ebp-34h]
    float f_4; // [esp+4h] [ebp-2Ch]
    float f_4a; // [esp+4h] [ebp-2Ch]
    float f_4b; // [esp+4h] [ebp-2Ch]
    float f_8; // [esp+8h] [ebp-28h]
    float f_8a; // [esp+8h] [ebp-28h]
    float f_8b; // [esp+8h] [ebp-28h]
    float f_12; // [esp+Ch] [ebp-24h]
    float f_12a; // [esp+Ch] [ebp-24h]
    float f_12b; // [esp+Ch] [ebp-24h]
    float z; // [esp+20h] [ebp-10h]
    float y; // [esp+20h] [ebp-10h]
    float m_inv_mass; // [esp+20h] [ebp-10h]
    float v66; // [esp+20h] [ebp-10h]
    float v67; // [esp+20h] [ebp-10h]
    float v68; // [esp+20h] [ebp-10h]

    z = s_->z;
    m_b1 = psc->m_b1;
    v12 = psc->m_ud_f2.x * z;
    v19 = psc->m_ud_f2.y * z;
    v26 = z * psc->m_ud_f2.z;
    y = s_->y;
    v42 = y * psc->m_ud_f1.x;
    v46 = psc->m_ud_f1.y * y;
    v50 = y * psc->m_ud_f1.z;
    f_4 = psc->m_ud_n.x * s_->x;
    f_8 = psc->m_ud_n.y * s_->x;
    f_12 = s_->x * psc->m_ud_n.z;
    v33 = f_4 + v42;
    v36 = f_8 + v46;
    v39 = f_12 + v50;
    v43 = v33 + v12;
    v47 = v36 + v19;
    v51 = v39 + v26;
    m_inv_mass = m_b1->m_inv_mass;
    v4 = v43;
    v13 = v43 * m_inv_mass;
    v5 = v47;
    v20 = v47 * m_inv_mass;
    v6 = v51;
    v27 = m_inv_mass * v51;
    m_b1->t_vel.x = m_b1->t_vel.x + v13;
    m_b1->t_vel.y = v20 + m_b1->t_vel.y;
    m_b1->t_vel.z = m_b1->t_vel.z + v27;
    p_a_vel = &psc->m_b1->a_vel;
    v66 = s_->z;
    f_4a = this->m_b1_ap_f2.x * v66;
    f_8a = this->m_b1_ap_f2.y * v66;
    f_12a = v66 * this->m_b1_ap_f2.z;
    v67 = s_->y;
    v34 = v67 * this->m_b1_ap_f1.x;
    v37 = this->m_b1_ap_f1.y * v67;
    v40 = v67 * this->m_b1_ap_f1.z;
    v14 = this->m_b1_ap_n.x * s_->x;
    v21 = this->m_b1_ap_n.y * s_->x;
    v28 = s_->x * this->m_b1_ap_n.z;
    v44 = v14 + v34;
    v48 = v21 + v37;
    v52 = v28 + v40;
    v15 = v44 + f_4a;
    v22 = v48 + f_8a;
    v29 = v52 + f_12a;
    p_a_vel->x = p_a_vel->x + v15;
    p_a_vel->y = v22 + p_a_vel->y;
    p_a_vel->z = p_a_vel->z + v29;
    if ( psc->m_b2 )
    {
        m_b2 = psc->m_b2;
        v68 = m_b2->m_inv_mass;
        v16 = v4 * v68;
        v23 = v5 * v68;
        v30 = v68 * v6;
        m_b2->t_vel.x = m_b2->t_vel.x - v16;
        m_b2->t_vel.y = m_b2->t_vel.y - v23;
        m_b2->t_vel.z = m_b2->t_vel.z - v30;
        v9 = &psc->m_b2->a_vel;
        v10 = s_->z;
        f_4b = this->m_b2_ap_f2.x * v10;
        f_8b = this->m_b2_ap_f2.y * v10;
        f_12b = v10 * this->m_b2_ap_f2.z;
        v11 = s_->y;
        v35 = v11 * this->m_b2_ap_f1.x;
        v38 = this->m_b2_ap_f1.y * v11;
        v41 = v11 * this->m_b2_ap_f1.z;
        v17 = this->m_b2_ap_n.x * s_->x;
        v24 = this->m_b2_ap_n.y * s_->x;
        v31 = s_->x * this->m_b2_ap_n.z;
        v45 = v17 + v35;
        v49 = v24 + v38;
        v53 = v31 + v41;
        v18 = v45 + f_4b;
        v25 = v49 + f_8b;
        v32 = v53 + f_12b;
        v9->x = v9->x - v18;
        v9->y = v9->y - v25;
        v9->z = v9->z - v32;
    }
}

void __cdecl rbint::setup_constraint(rigid_body *rb, pulse_sum_node *psn)
{
    if ( !rb
        && _tlAssert(
                 "C:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\constraint_solver\\pulse_sum_constraint_solver_inline.h",
                 123,
                 "rb",
                 "") )
    {
        __debugbreak();
    }
    if ( rb->m_inv_mass <= 0.0000099999997
        && _tlAssert(
                 "C:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\constraint_solver\\pulse_sum_constraint_solver_inline.h",
                 124,
                 "rb->get_inv_mass() > 0.00001f",
                 "") )
    {
        __debugbreak();
    }
    nuge::tensor_transform_principle(&rb->m_inv_inertia, &rb->m_mat, &psn->m_world_inv_inertia);
    rb->m_node = psn;
    psn->m_rb = rb;
    psn->m_inv_mass = rb->m_inv_mass;
}

pulse_sum_constraint_solver::~pulse_sum_constraint_solver()
{
    if ( this->m_solver_memory_allocator.m_first_block )
    {
        if ( _tlAssert(
                     "C:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\phys_transient_allocator.h",
                     69,
                     "m_first_block == NULL",
                     "") )
        {
            __debugbreak();
        }
    }
}

void __thiscall rigid_body::update_last_position()
{
    float v1; // [esp-20h] [ebp-2Ch]
    float v2; // [esp-1Ch] [ebp-28h]
    float v3; // [esp-18h] [ebp-24h]
    float m_smallest_lambda; // [esp-4h] [ebp-10h]
    float v5; // [esp-4h] [ebp-10h]
    float v6; // [esp-4h] [ebp-10h]
    float v7; // [esp-4h] [ebp-10h]

    m_smallest_lambda = this->m_smallest_lambda;
    v1 = this->m_moved_vec.x * m_smallest_lambda;
    v2 = this->m_moved_vec.y * m_smallest_lambda;
    v3 = m_smallest_lambda * this->m_moved_vec.z;
    this->m_last_position.x = this->m_last_position.x + v1;
    this->m_last_position.y = v2 + this->m_last_position.y;
    this->m_last_position.z = this->m_last_position.z + v3;
    v5 = fabs(this->m_last_position.x);
    if ( v5 > 100000.0
        || (v6 = fabs(this->m_last_position.y), v6 > 100000.0)
        || (v7 = fabs(this->m_last_position.z), v7 > 100000.0) )
    {
        phys_exec_debug_callback(this);
    }
}

user_rigid_body & user_rigid_body::operator=(const user_rigid_body *__that)
{
    *((rigid_body *)this) = *__that;
    //rigid_body::operator=(this, __that);
    this->m_dictator = __that->m_dictator;
    this->m_dictator_mat.x.x = __that->m_dictator_mat.x.x;
    this->m_dictator_mat.x.y = __that->m_dictator_mat.x.y;
    this->m_dictator_mat.x.z = __that->m_dictator_mat.x.z;
    this->m_dictator_mat.y.x = __that->m_dictator_mat.y.x;
    this->m_dictator_mat.y.y = __that->m_dictator_mat.y.y;
    this->m_dictator_mat.y.z = __that->m_dictator_mat.y.z;
    this->m_dictator_mat.z.x = __that->m_dictator_mat.z.x;
    this->m_dictator_mat.z.y = __that->m_dictator_mat.z.y;
    this->m_dictator_mat.z.z = __that->m_dictator_mat.z.z;
    this->m_dictator_mat.w.x = __that->m_dictator_mat.w.x;
    this->m_dictator_mat.w.y = __that->m_dictator_mat.w.y;
    this->m_dictator_mat.w.z = __that->m_dictator_mat.w.z;
    return *this;
}

void    rbint::substep(user_rigid_body *rb, float delta_t)
{
    phys_mat44 v3; // [esp+18h] [ebp-5Ch] BYREF
    float rv_52; // [esp+58h] [ebp-1Ch]
    float rv_56; // [esp+5Ch] [ebp-18h]
    float rv_60; // [esp+60h] [ebp-14h]
    //_DWORD v7[3]; // [esp+68h] [ebp-Ch] BYREF
    //_UNKNOWN *retaddr; // [esp+74h] [ebp+0h]
    //
    //v7[0] = a1;
    //v7[1] = retaddr;
    rv_52 = rb->m_t_vel.x * delta_t;
    rv_56 = rb->m_t_vel.y * delta_t;
    rv_60 = rb->m_t_vel.z * delta_t;
    rb->m_mat.w.x = rb->m_mat.w.x + rv_52;
    rb->m_mat.w.y = rb->m_mat.w.y + rv_56;
    rb->m_mat.w.z = rb->m_mat.w.z + rv_60;
    make_rotate(&v3, &rb->m_a_vel, delta_t, 1000.0);
    phys_multiply_mat(&rb->m_mat, &v3, &rb->m_mat);
}

void __cdecl rbint::solver_prolog(rigid_body *rb, float delta_t)
{
    double v3; // st7
    float rba; // [esp+8h] [ebp+8h]
    float rbb; // [esp+8h] [ebp+8h]
    float rbc; // [esp+8h] [ebp+8h]
    float rbd; // [esp+8h] [ebp+8h]
    float rbe; // [esp+8h] [ebp+8h]
    float rbf; // [esp+8h] [ebp+8h]
    float rbg; // [esp+8h] [ebp+8h]

    rba = 1.0 / delta_t;
    rb->m_force_sum.x = rb->m_force_sum.x * rba;
    rb->m_force_sum.y = rb->m_force_sum.y * rba;
    rb->m_force_sum.z = rba * rb->m_force_sum.z;
    rb->m_torque_sum.x = rb->m_torque_sum.x * rba;
    rb->m_torque_sum.y = rb->m_torque_sum.y * rba;
    rb->m_torque_sum.z = rba * rb->m_torque_sum.z;
    v3 = 100000.0;
    rbb = fabs(rb->m_force_sum.x);
    if ( rbb > 100000.0
        || (rbc = fabs(rb->m_force_sum.y), rbc > 100000.0)
        || (rbd = fabs(rb->m_force_sum.z), rbd > 100000.0) )
    {
        phys_exec_debug_callback(rb);
        v3 = 100000.0;
    }
    rbe = fabs(rb->m_torque_sum.x);
    if ( rbe > v3 || (rbf = fabs(rb->m_torque_sum.y), rbf > v3) || (rbg = fabs(rb->m_torque_sum.z), v3 < rbg) )
        phys_exec_debug_callback(rb);
}

// local variable allocation has failed, the output may be wrong!
void    rbint::euler_integrate_velocity(rigid_body *const rb, float delta_t)
{
    double v3; // st7
    const phys_vec3 *v4; // eax
    double x; // st7
    double y; // st6
    const phys_vec3 *v7; // eax
    double v8; // st7
    double v9; // st7
    double v10; // st7
    double v11; // st6
    double v12; // st3
    double v13; // st7
    double v14; // st7
    phys_vec3 a_vel_loc2; // [esp+14h] [ebp-3Ch] BYREF
    phys_vec3 a_mom_loc2; // [esp+24h] [ebp-2Ch] BYREF
    float factor; // [esp+38h] [ebp-18h]
    float ake_times_two_2; // [esp+3Ch] [ebp-14h]
    float navel_sq; // [esp+40h] [ebp-10h]
    //_UNKNOWN *v20[2]; // [esp+44h] [ebp-Ch] BYREF
    //int vars0; // [esp+50h] [ebp+0h]
    //
    //*v20 = a1;
    //v20[1] = vars0;
    v3 = 100000.0;
    navel_sq = fabs(rb->m_t_vel.x);
    if (navel_sq > 100000.0
        || (navel_sq = fabs(rb->m_t_vel.y), navel_sq > 100000.0)
        || (navel_sq = fabs(rb->m_t_vel.z), navel_sq > 100000.0))
    {
        phys_exec_debug_callback(rb);
        v3 = 100000.0;
    }
    navel_sq = fabs(rb->m_a_vel.x);
    if (navel_sq > v3
        || (navel_sq = fabs(rb->m_a_vel.y), navel_sq > v3)
        || (navel_sq = fabs(rb->m_a_vel.z), v3 < navel_sq))
    {
        phys_exec_debug_callback(rb);
    }
    rb->m_last_t_vel.x = rb->m_t_vel.x;
    rb->m_last_t_vel.y = rb->m_t_vel.y;
    rb->m_last_t_vel.z = rb->m_t_vel.z;
    rb->m_last_a_vel.x = rb->m_a_vel.x;
    rb->m_last_a_vel.y = rb->m_a_vel.y;
    rb->m_last_a_vel.z = rb->m_a_vel.z;
    navel_sq = rb->m_inv_mass;
    a_mom_loc2.x = navel_sq * rb->m_force_sum.x;
    a_mom_loc2.y = rb->m_force_sum.y * navel_sq;
    a_mom_loc2.z = navel_sq * rb->m_force_sum.z;
    a_vel_loc2.x = a_mom_loc2.x + rb->m_gravity_acc_vec.x;
    a_vel_loc2.y = rb->m_gravity_acc_vec.y + a_mom_loc2.y;
    a_vel_loc2.z = rb->m_gravity_acc_vec.z + a_mom_loc2.z;
    a_mom_loc2.x = a_vel_loc2.x * delta_t;
    a_mom_loc2.y = a_vel_loc2.y * delta_t;
    a_mom_loc2.z = a_vel_loc2.z * delta_t;
    rb->m_t_vel.x = rb->m_t_vel.x + a_mom_loc2.x;
    rb->m_t_vel.y = rb->m_t_vel.y + a_mom_loc2.y;
    rb->m_t_vel.z = a_mom_loc2.z + rb->m_t_vel.z;
    v4 = rbint::inv_L(&a_vel_loc2, rb, &rb->m_torque_sum, delta_t);
    rb->m_a_vel.x = rb->m_a_vel.x + v4->x;
    rb->m_a_vel.y = v4->y + rb->m_a_vel.y;
    rb->m_a_vel.z = v4->z + rb->m_a_vel.z;
    a_vel_loc2.x = rb->m_mat.x.y * rb->m_a_vel.y + rb->m_a_vel.x * rb->m_mat.x.x + rb->m_mat.x.z * rb->m_a_vel.z;
    a_vel_loc2.y = rb->m_mat.y.y * rb->m_a_vel.y + rb->m_a_vel.x * rb->m_mat.y.x + rb->m_mat.y.z * rb->m_a_vel.z;
    a_vel_loc2.z = rb->m_mat.z.y * rb->m_a_vel.y + rb->m_a_vel.x * rb->m_mat.z.x + rb->m_mat.z.z * rb->m_a_vel.z;
    x = a_vel_loc2.x;
    a_mom_loc2.x = a_vel_loc2.x / rb->m_inv_inertia.x;
    y = a_vel_loc2.y;
    a_mom_loc2.y = a_vel_loc2.y / rb->m_inv_inertia.y;
    a_mom_loc2.z = a_vel_loc2.z / rb->m_inv_inertia.z;
    factor = a_mom_loc2.y * a_vel_loc2.y + a_mom_loc2.x * a_vel_loc2.x + a_mom_loc2.z * a_vel_loc2.z;
    a_vel_loc2.x = a_mom_loc2.y * a_vel_loc2.z - a_mom_loc2.z * a_vel_loc2.y;
    a_vel_loc2.y = a_mom_loc2.z * x - a_vel_loc2.z * a_mom_loc2.x;
    a_vel_loc2.z = a_mom_loc2.x * y - x * a_mom_loc2.y;
    a_mom_loc2.x = rb->m_inv_inertia.x * a_vel_loc2.x;
    a_mom_loc2.y = rb->m_inv_inertia.y * a_vel_loc2.y;
    a_mom_loc2.z = rb->m_inv_inertia.z * a_vel_loc2.z;
    a_vel_loc2.x = a_mom_loc2.x * delta_t;
    a_vel_loc2.y = a_mom_loc2.y * delta_t;
    a_vel_loc2.z = delta_t * a_mom_loc2.z;
    v7 = phys_multiply(&a_mom_loc2, &rb->m_mat, &a_vel_loc2);
    rb->m_a_vel.x = v7->x + rb->m_a_vel.x;
    rb->m_a_vel.y = v7->y + rb->m_a_vel.y;
    rb->m_a_vel.z = v7->z + rb->m_a_vel.z;
    navel_sq = rb->m_a_vel.y * rb->m_a_vel.y + rb->m_a_vel.x * rb->m_a_vel.x + rb->m_a_vel.z * rb->m_a_vel.z;
    ake_times_two_2 = rb->m_max_avel;
    ake_times_two_2 = ake_times_two_2 * ake_times_two_2;
    if (ake_times_two_2 < navel_sq)
    {
        if (navel_sq <= 9.999999439624929e-11
            && _tlAssert(
                "C:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\rigid_body_internal.h",
                124,
                "navel_sq > phys_sqr(0.00001f)",
                ""))
        {
            __debugbreak();
        }
        *&a_mom_loc2.z = rb->m_max_avel;
        ake_times_two_2 = sqrt(navel_sq);
        ake_times_two_2 = *&a_mom_loc2.z / ake_times_two_2;
        v8 = ake_times_two_2;
        rb->m_a_vel.x = ake_times_two_2 * rb->m_a_vel.x;
        rb->m_a_vel.y = v8 * rb->m_a_vel.y;
        rb->m_a_vel.z = v8 * rb->m_a_vel.z;
    }
    a_vel_loc2.x = rb->m_mat.x.y * rb->m_a_vel.y + rb->m_mat.x.x * rb->m_a_vel.x + rb->m_mat.x.z * rb->m_a_vel.z;
    a_vel_loc2.y = rb->m_mat.y.y * rb->m_a_vel.y + rb->m_mat.y.x * rb->m_a_vel.x + rb->m_mat.y.z * rb->m_a_vel.z;
    a_vel_loc2.z = rb->m_mat.z.y * rb->m_a_vel.y + rb->m_mat.z.x * rb->m_a_vel.x + rb->m_mat.z.z * rb->m_a_vel.z;
    a_mom_loc2.x = a_vel_loc2.x / rb->m_inv_inertia.x;
    a_mom_loc2.y = a_vel_loc2.y / rb->m_inv_inertia.y;
    a_mom_loc2.z = a_vel_loc2.z / rb->m_inv_inertia.z;
    ake_times_two_2 = a_vel_loc2.x * a_mom_loc2.x + a_vel_loc2.y * a_mom_loc2.y + a_vel_loc2.z * a_mom_loc2.z;
    v9 = ake_times_two_2;
    if (factor < ake_times_two_2 && v9 > 0.0000099999997)
    {
        factor = factor / v9;
        factor = sqrt(factor);
        v10 = factor;
        rb->m_a_vel.x = rb->m_a_vel.x * factor;
        rb->m_a_vel.y = rb->m_a_vel.y * v10;
        rb->m_a_vel.z = v10 * rb->m_a_vel.z;
    }
    factor = rb->m_t_drag_coef * delta_t;
    if (factor >= 0.8999999761581421)
    {
        navel_sq = 0.89999998;
        v11 = 0.89999998;
    }
    else
    {
        v11 = 0.89999998;
        navel_sq = factor;
    }
    factor = 1.0 - navel_sq;
    v12 = factor;
    rb->m_t_vel.x = rb->m_t_vel.x * factor;
    rb->m_t_vel.y = rb->m_t_vel.y * v12;
    rb->m_t_vel.z = v12 * rb->m_t_vel.z;
    factor = delta_t * rb->m_a_drag_coef;
    if (factor < 0.8999999761581421)
        v11 = factor;
    navel_sq = v11;
    factor = 1.0 - navel_sq;
    v13 = factor;
    rb->m_a_vel.x = factor * rb->m_a_vel.x;
    rb->m_a_vel.y = v13 * rb->m_a_vel.y;
    rb->m_a_vel.z = v13 * rb->m_a_vel.z;
    factor = fabs(rb->m_t_vel.x);
    v14 = 100000.0;
    if (factor > 100000.0
        || (factor = fabs(rb->m_t_vel.y), factor > 100000.0)
        || (factor = fabs(rb->m_t_vel.z), factor > 100000.0))
    {
        phys_exec_debug_callback(rb);
        v14 = 100000.0;
    }
    factor = fabs(rb->m_a_vel.x);
    if (factor > v14 || (factor = fabs(rb->m_a_vel.y), factor > v14) || (factor = fabs(rb->m_a_vel.z), v14 < factor))
        phys_exec_debug_callback(rb);
}

void    rbint::euler_integrate_pos(rigid_body *const rb, float delta_t)
{
    phys_mat44 rv; // [esp+18h] [ebp-6Ch] BYREF
    float v4; // [esp+58h] [ebp-2Ch]
    float v5; // [esp+5Ch] [ebp-28h]
    float v6; // [esp+60h] [ebp-24h]
    float v7; // [esp+74h] [ebp-10h]
    //_UNKNOWN *v8[2]; // [esp+78h] [ebp-Ch] BYREF
    //int vars0; // [esp+84h] [ebp+0h]
    //
    //v8[0] = a1;
    //v8[1] = vars0;
    v4 = rb->m_t_vel.x * delta_t;
    v5 = rb->m_t_vel.y * delta_t;
    v6 = rb->m_t_vel.z * delta_t;
    rb->m_mat.w.x = rb->m_mat.w.x + v4;
    rb->m_mat.w.y = rb->m_mat.w.y + v5;
    rb->m_mat.w.z = rb->m_mat.w.z + v6;
    make_rotate(&rv, &rb->m_a_vel, delta_t, 1000.0);
    phys_multiply_mat(&rb->m_mat, &rv, &rb->m_mat);
    if (++rb->m_tick > 5)
    {
        rb->m_tick = 0;
        orthonormalize(&rb->m_mat);
    }
    v7 = fabs(rb->m_mat.w.x);
    if (v7 > 100000.0 || (v7 = fabs(rb->m_mat.w.y), v7 > 100000.0) || (v7 = fabs(rb->m_mat.w.z), v7 > 100000.0))
        phys_exec_debug_callback(rb);
}

void __cdecl rbint::add_vel(rigid_body *rb, const phys_vec3 *t, const phys_vec3 *a)
{
    double v4; // st7
    double v5; // st7
    float rba; // [esp+Ch] [ebp+8h]
    float rbb; // [esp+Ch] [ebp+8h]
    float rbc; // [esp+Ch] [ebp+8h]
    float rbd; // [esp+Ch] [ebp+8h]
    float rbe; // [esp+Ch] [ebp+8h]
    float rbf; // [esp+Ch] [ebp+8h]
    float rbg; // [esp+Ch] [ebp+8h]
    float rbh; // [esp+Ch] [ebp+8h]

    rb->m_t_vel.x = rb->m_t_vel.x + t->x;
    rb->m_t_vel.y = t->y + rb->m_t_vel.y;
    rb->m_t_vel.z = t->z + rb->m_t_vel.z;
    rb->m_a_vel.x = rb->m_a_vel.x + a->x;
    rb->m_a_vel.y = a->y + rb->m_a_vel.y;
    rb->m_a_vel.z = a->z + rb->m_a_vel.z;
    v4 = 100000.0;
    rba = fabs(rb->m_t_vel.x);
    if ( rba > 100000.0 || (rbb = fabs(rb->m_t_vel.y), rbb > 100000.0) || (rbc = fabs(rb->m_t_vel.z), rbc > 100000.0) )
    {
        phys_exec_debug_callback(rb);
        v4 = 100000.0;
    }
    rbd = fabs(rb->m_a_vel.x);
    if ( rbd > v4 || (rbe = fabs(rb->m_a_vel.y), rbe > v4) || (rbf = fabs(rb->m_a_vel.z), v4 < rbf) )
        phys_exec_debug_callback(rb);
    rbg = rb->m_t_vel.y * rb->m_t_vel.y + rb->m_t_vel.x * rb->m_t_vel.x + rb->m_t_vel.z * rb->m_t_vel.z;
    v5 = rbg;
    rbh = rb->m_a_vel.y * rb->m_a_vel.y + rb->m_a_vel.x * rb->m_a_vel.x + rb->m_a_vel.z * rb->m_a_vel.z;
    if ( v5 + rbh > 1.0e10 )
        rb->m_flags |= 0x80u;
}

void __thiscall rigid_body_constraint_contact::update_smallest_lambda()
{
    contact_point_info *i; // ecx
    rigid_body *b1; // edx
    double m_translation_lambda; // st7
    float v5; // [esp+4h] [ebp-4h]

    if ( (this->b1->m_flags & 0x10) != 0
        && _tlAssert(
                 "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\rbc_defs\\rbc_def_contact.h",
                 414,
                 "b1->is_environment_rigid_body() == false",
                 "") )
    {
        __debugbreak();
    }
    if ( (this->b2->m_flags & 0x10) != 0 )
    {
        for ( i = this->m_list_contact_point_info_buffer_1.m_first; i; i = i->m_next_link )
        {
            b1 = this->b1;
            if ( i->m_translation_lambda <= (double)this->b1->m_smallest_lambda )
                m_translation_lambda = i->m_translation_lambda;
            else
                m_translation_lambda = b1->m_smallest_lambda;
            v5 = m_translation_lambda;
            b1->m_smallest_lambda = v5;
        }
    }
}

void pulse_sum_point::calc_abs_and_setup(
                float delta_t,
                bool is_spring,
                float spring_k,
                float damp_k)
{
    pulse_sum_node *m_b1; // eax
    const phys_mat44 *p_m_world_inv_inertia; // edi
    const phys_vec3 *v9; // eax
    const phys_vec3 *v10; // eax
    const phys_vec3 *v11; // eax
    pulse_sum_node *m_b2; // eax
    const phys_mat44 *v13; // edi
    const phys_vec3 *v14; // eax
    const phys_vec3 *v15; // eax
    const phys_vec3 *v16; // eax
    double v17; // st6
    double v18; // st5
    double v19; // st7
    const phys_vec3 *pos; // eax
    double z; // st7
    double y; // st5
    double v23; // st4
    double x; // rtt
    double v25; // st3
    double v26; // st7
    phys_vec3 v27; // [esp-Ch] [ebp-6Ch] OVERLAPPED BYREF
    phys_vec3 v28; // [esp+4h] [ebp-5Ch] BYREF
    phys_vec3 zz; // [esp+14h] [ebp-4Ch]
    phys_vec3 zy; // [esp+24h] [ebp-3Ch]
    phys_vec3 zx; // [esp+34h] [ebp-2Ch]
    float oo_det; // [esp+4Ch] [ebp-14h]
    float m_inv_mass; // [esp+50h] [ebp-10h]
    //_UNKNOWN *v34[2]; // [esp+54h] [ebp-Ch] BYREF
    //float spring_ka; // [esp+60h] [ebp+0h]
    //
    //v34[0] = a2;
    //*&v34[1] = spring_ka;
    m_b1 = this->m_b1;
    m_inv_mass = m_b1->m_inv_mass;
    p_m_world_inv_inertia = &m_b1->m_world_inv_inertia;
    v28.x = 0.0;
    v28.y = this->m_b1_r.z;
    v28.z = -this->m_b1_r.y;
    v9 = phys_multiply(&v27, &m_b1->m_world_inv_inertia, &v28);
    this->m_b1_apx.x = v9->x;
    this->m_b1_apx.y = v9->y;
    this->m_b1_apx.z = v9->z;
    zx.x = this->m_b1_apx.y * this->m_b1_r.z - this->m_b1_apx.z * this->m_b1_r.y;
    zx.y = this->m_b1_r.x * this->m_b1_apx.z - this->m_b1_apx.x * this->m_b1_r.z;
    zx.z = this->m_b1_apx.x * this->m_b1_r.y - this->m_b1_r.x * this->m_b1_apx.y;
    zx.x = zx.x + m_inv_mass;
    v28.x = -this->m_b1_r.z;
    v28.y = 0.0;
    v28.z = this->m_b1_r.x;
    v10 = phys_multiply(&v27, p_m_world_inv_inertia, &v28);
    this->m_b1_apy.x = v10->x;
    this->m_b1_apy.y = v10->y;
    this->m_b1_apy.z = v10->z;
    zy.x = this->m_b1_apy.y * this->m_b1_r.z - this->m_b1_apy.z * this->m_b1_r.y;
    zy.y = this->m_b1_apy.z * this->m_b1_r.x - this->m_b1_r.z * this->m_b1_apy.x;
    zy.z = this->m_b1_apy.x * this->m_b1_r.y - this->m_b1_apy.y * this->m_b1_r.x;
    zy.y = zy.y + m_inv_mass;
    v28.x = this->m_b1_r.y;
    v28.y = -this->m_b1_r.x;
    v28.z = 0.0;
    v11 = phys_multiply(&v27, p_m_world_inv_inertia, &v28);
    this->m_b1_apz.x = v11->x;
    this->m_b1_apz.y = v11->y;
    this->m_b1_apz.z = v11->z;
    m_b2 = this->m_b2;
    zz.x = this->m_b1_apz.y * this->m_b1_r.z - this->m_b1_apz.z * this->m_b1_r.y;
    zz.y = this->m_b1_apz.z * this->m_b1_r.x - this->m_b1_apz.x * this->m_b1_r.z;
    oo_det = this->m_b1_apz.x * this->m_b1_r.y - this->m_b1_r.x * this->m_b1_apz.y;
    zz.z = oo_det + m_inv_mass;
    if (m_b2)
    {
        v13 = &m_b2->m_world_inv_inertia;
        m_inv_mass = m_b2->m_inv_mass;
        v28.x = 0.0;
        v28.y = this->m_b2_r.z;
        v28.z = -this->m_b2_r.y;
        v14 = phys_multiply(&v27, &m_b2->m_world_inv_inertia, &v28);
        this->m_b2_apx.x = v14->x;
        this->m_b2_apx.y = v14->y;
        this->m_b2_apx.z = v14->z;
        v28.x = this->m_b2_apx.y * this->m_b2_r.z - this->m_b2_apx.z * this->m_b2_r.y;
        v28.y = this->m_b2_apx.z * this->m_b2_r.x - this->m_b2_apx.x * this->m_b2_r.z;
        v28.z = this->m_b2_r.y * this->m_b2_apx.x - this->m_b2_apx.y * this->m_b2_r.x;
        zx.x = v28.x + zx.x;
        zx.y = v28.y + zx.y;
        zx.z = v28.z + zx.z;
        zx.x = m_inv_mass + zx.x;
        v28.x = -this->m_b2_r.z;
        v28.y = 0.0;
        v28.z = this->m_b2_r.x;
        v15 = phys_multiply(&v27, v13, &v28);
        this->m_b2_apy.x = v15->x;
        this->m_b2_apy.y = v15->y;
        this->m_b2_apy.z = v15->z;
        v28.x = this->m_b2_apy.y * this->m_b2_r.z - this->m_b2_apy.z * this->m_b2_r.y;
        v28.y = this->m_b2_apy.z * this->m_b2_r.x - this->m_b2_r.z * this->m_b2_apy.x;
        v28.z = this->m_b2_r.y * this->m_b2_apy.x - this->m_b2_r.x * this->m_b2_apy.y;
        zy.x = v28.x + zy.x;
        zy.y = v28.y + zy.y;
        zy.z = v28.z + zy.z;
        zy.y = zy.y + m_inv_mass;
        v28.x = this->m_b2_r.y;
        v28.y = -this->m_b2_r.x;
        v28.z = 0.0;
        v16 = phys_multiply(&v27, v13, &v28);
        this->m_b2_apz.x = v16->x;
        this->m_b2_apz.y = v16->y;
        this->m_b2_apz.z = v16->z;
        v28.x = this->m_b2_apz.y * this->m_b2_r.z - this->m_b2_apz.z * this->m_b2_r.y;
        v28.y = this->m_b2_apz.z * this->m_b2_r.x - this->m_b2_apz.x * this->m_b2_r.z;
        v28.z = this->m_b2_r.y * this->m_b2_apz.x - this->m_b2_apz.y * this->m_b2_r.x;
        zz.x = v28.x + zz.x;
        zz.y = v28.y + zz.y;
        oo_det = v28.z + zz.z;
        zz.z = oo_det + m_inv_mass;
    }
    if (is_spring)
    {
        pulse_sum_point::get_pos(&v27);
        oo_det = delta_t * spring_k;
        v17 = oo_det * delta_t;
        oo_det = delta_t * damp_k;
        oo_det = 1.0 / (oo_det + v17);
        v18 = oo_det;
        this->m_cfm = oo_det;
        zx.x = this->m_cfm + zx.x;
        zy.y = zy.y + this->m_cfm;
        zz.z = zz.z + this->m_cfm;
        oo_det = v17 * v18;
        oo_det = -oo_det / delta_t;
        v28.x = v27.x * oo_det;
        v28.y = v27.y * oo_det;
        v28.z = oo_det * v27.z;
        this->m_right_side.x = v28.x;
        this->m_right_side.y = v28.y;
        this->m_right_side.z = v28.z;
        this->m_big_dirt.x = PHYS_ZERO_VEC.x;
        this->m_big_dirt.y = PHYS_ZERO_VEC.y;
        this->m_big_dirt.z = PHYS_ZERO_VEC.z;
    }
    else
    {
        v19 = 0.016666668;
        if (delta_t > 0.016666668)
            v19 = delta_t;
        m_inv_mass = v19;
        pos = pulse_sum_point::get_pos(&v27);
        oo_det = -0.5 / m_inv_mass;
        v28.x = oo_det * pos->x;
        v28.y = pos->y * oo_det;
        v28.z = oo_det * pos->z;
        this->m_big_dirt.x = v28.x;
        this->m_big_dirt.y = v28.y;
        this->m_big_dirt.z = v28.z;
        this->m_right_side.x = PHYS_ZERO_VEC.x;
        this->m_right_side.y = PHYS_ZERO_VEC.y;
        this->m_right_side.z = PHYS_ZERO_VEC.z;
        this->m_cfm = 0.0;
    }
    z = zz.z;
    y = zy.y;
    v23 = zz.y;
    v28.x = zz.z * zy.y - zz.y * zy.z;
    v28.y = zy.z * zz.x - zz.z * zy.x;
    x = zz.x;
    v28.z = zy.x * zz.y - zy.y * zz.x;
    this->m_cr23.x = v28.x;
    this->m_cr23.y = v28.y;
    this->m_cr23.z = v28.z;
    v28.x = v23 * zx.z - z * zx.y;
    v25 = zx.x;
    v28.y = z * zx.x - zx.z * x;
    v28.z = zx.y * x - v23 * zx.x;
    this->m_cr31.x = v28.x;
    this->m_cr31.y = v28.y;
    this->m_cr31.z = v28.z;
    v28.x = zy.z * zx.y - y * zx.z;
    v28.y = zx.z * zy.x - zy.z * v25;
    v28.z = y * v25 - zx.y * zy.x;
    this->m_cr12.x = v28.x;
    this->m_cr12.y = v28.y;
    this->m_cr12.z = v28.z;
    this->m_denom.x = v25;
    this->m_denom.y = y;
    this->m_denom.z = z;
    oo_det = z * this->m_cr12.z + x * this->m_cr12.x + v23 * this->m_cr12.y;
    oo_det = 1.0 / oo_det;
    v26 = oo_det;
    this->m_cr23.x = this->m_cr23.x * oo_det;
    this->m_cr23.y = v26 * this->m_cr23.y;
    this->m_cr23.z = this->m_cr23.z * v26;
    this->m_cr31.x = v26 * this->m_cr31.x;
    this->m_cr31.y = this->m_cr31.y * v26;
    this->m_cr31.z = this->m_cr31.z * v26;
    this->m_cr12.x = this->m_cr12.x * v26;
    this->m_cr12.y = this->m_cr12.y * v26;
    this->m_cr12.z = v26 * this->m_cr12.z;
}

void pulse_sum_point::SOLVER_solver_prolog(float delta_t)
{
    const phys_vec3 *vel; // eax
    //pulse_sum_point *this; // ecx
    phys_vec3 v5; // [esp-10h] [ebp-1Ch] BYREF
    //_DWORD v6[3]; // [esp+0h] [ebp-Ch] BYREF
    //_UNKNOWN *retaddr; // [esp+Ch] [ebp+0h]
    //
    //v6[0] = a2;
    //v6[1] = retaddr;
    vel = pulse_sum_point::get_vel(&v5);
    this->m_right_side.x = this->m_right_side.x - vel->x;
    this->m_right_side.y = this->m_right_side.y - vel->y;
    this->m_right_side.z = this->m_right_side.z - vel->z;
    this->m_pulse_sum.x = this->m_pulse_sum_cache->m_pulse_sum * delta_t;
    this->m_pulse_sum.y = this->m_pulse_sum_cache[1].m_pulse_sum * delta_t;
    this->m_pulse_sum.z = delta_t * this->m_pulse_sum_cache[2].m_pulse_sum;
    pulse_sum_point::apply(&this->m_pulse_sum);
}

bool __thiscall pulse_sum_wheel::pulse_chain_within_limits()
{
    pulse_sum_normal *m_side; // edx
    pulse_sum_normal *m_fwd; // esi
    double v3; // st4
    double v4; // st6
    double v5; // st5
    float a2_; // [esp+4h] [ebp-10h]
    float a2_a; // [esp+4h] [ebp-10h]
    float a2_b; // [esp+4h] [ebp-10h]
    float a2_c; // [esp+4h] [ebp-10h]
    float a2_d; // [esp+4h] [ebp-10h]
    float a2_e; // [esp+4h] [ebp-10h]
    float b2_; // [esp+8h] [ebp-Ch]
    float p2_; // [esp+Ch] [ebp-8h]
    float a_; // [esp+10h] [ebp-4h]

    m_side = this->m_side;
    m_fwd = this->m_fwd;
    a_ = -m_side->m_pulse_limit_ratio * this->m_suspension.m_pulse_sum;
    if ( this->m_side_fric_max <= (double)a_ )
        a_ = this->m_side_fric_max;
    p2_ = -m_fwd->m_pulse_limit_ratio * this->m_suspension.m_pulse_sum;
    if ( this->m_side_fric_max <= (double)p2_ )
        p2_ = this->m_side_fric_max;
    a2_ = a_ * a_;
    b2_ = p2_ * p2_;
    v3 = a2_;
    a2_a = b2_ * a2_;
    v4 = a2_a;
    a2_b = m_side->m_pulse_sum * m_side->m_pulse_sum;
    v5 = b2_ * a2_b;
    a2_c = m_fwd->m_pulse_sum * m_fwd->m_pulse_sum;
    a2_d = v3 * a2_c + v5;
    a2_e = a2_d * 0.9999900000002526;
    return a2_e <= v4;
}

char __thiscall pulse_sum_wheel::clamp_pulse_sum_pulse_chain(float *ps1_, float *ps2_)
{
    pulse_sum_normal *m_fwd; // edx
    pulse_sum_normal *m_side; // esi
    double v5; // st6
    double v6; // st7
    double v7; // st7
    double v8; // st6
    char result; // al
    double v10; // st7
    float numer_sq; // [esp+4h] [ebp-14h]
    float numer_sqa; // [esp+4h] [ebp-14h]
    float numer_sqb; // [esp+4h] [ebp-14h]
    float numer_sqc; // [esp+4h] [ebp-14h]
    float denom_sq; // [esp+8h] [ebp-10h]
    float denom_sqa; // [esp+8h] [ebp-10h]
    float denom_sqb; // [esp+8h] [ebp-10h]
    float p2_; // [esp+Ch] [ebp-Ch]
    float t_; // [esp+10h] [ebp-8h]
    float t_a; // [esp+10h] [ebp-8h]
    float t_b; // [esp+10h] [ebp-8h]
    float a_; // [esp+14h] [ebp-4h]
    float a_a; // [esp+14h] [ebp-4h]

    m_fwd = this->m_fwd;
    m_side = this->m_side;
    a_ = -m_side->m_pulse_limit_ratio * this->m_suspension.m_pulse_sum;
    if ( this->m_side_fric_max <= (double)a_ )
        a_ = this->m_side_fric_max;
    t_ = -m_fwd->m_pulse_limit_ratio * this->m_suspension.m_pulse_sum;
    if ( this->m_side_fric_max <= (double)t_ )
        t_ = this->m_side_fric_max;
    numer_sq = a_ * a_;
    denom_sq = t_ * t_;
    a_a = m_side->m_pulse_sum;
    p2_ = m_fwd->m_pulse_sum;
    t_a = a_a * a_a;
    v5 = t_a * denom_sq;
    v6 = denom_sq;
    denom_sqa = p2_ * p2_;
    denom_sqb = denom_sqa * numer_sq + v5;
    numer_sqa = v6 * numer_sq;
    v7 = numer_sqa;
    v8 = denom_sqb;
    if ( denom_sqb >= (double)numer_sqa || v7 < 0.0000099999997 )
    {
        m_fwd->m_flags |= 4u;
        if ( v8 <= 0.0000099999997 )
        {
            v10 = 0.0;
        }
        else
        {
            numer_sqb = v7 / v8;
            numer_sqc = sqrt(numer_sqb);
            v10 = numer_sqc;
        }
        t_b = v10;
        *ps1_ = t_b * a_a;
        *ps2_ = t_b * p2_;
        return 1;
    }
    else
    {
        m_fwd->m_flags &= ~4u;
        *ps1_ = m_side->m_pulse_sum;
        result = 0;
        *ps2_ = m_fwd->m_pulse_sum;
    }
    return result;
}

void __thiscall pulse_sum_wheel::addp_pulse_chain()
{
    pulse_sum_normal *m_side; // edi
    pulse_sum_normal *m_fwd; // ebx
    float s_; // [esp+Ch] [ebp-Ch] BYREF
    float ps2_; // [esp+10h] [ebp-8h] BYREF
    float ps1_; // [esp+14h] [ebp-4h] BYREF

    m_side = this->m_side;
    m_fwd = this->m_fwd;
    if ( pulse_sum_wheel::clamp_pulse_sum_pulse_chain(&ps1_, &ps2_) )
    {
        s_ = ps1_ - m_side->m_pulse_sum;
        //pulse_sum_normal::apply(m_side, &s_);
        m_side->apply(&s_);
        m_side->m_pulse_sum = ps1_;
        s_ = ps2_ - m_fwd->m_pulse_sum;
        //pulse_sum_normal::apply(m_fwd, &s_);
        m_fwd->apply(&s_);
        m_fwd->m_pulse_sum = ps2_;
    }
    if ( !pulse_sum_wheel::pulse_chain_within_limits() )
    {
        if ( _tlAssert(
                     "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\constraint_solver\\pulse_sum_wheel_inline.h",
                     67,
                     "pulse_chain_within_limits()",
                     "") )
        {
            __debugbreak();
        }
    }
}

void __thiscall pulse_sum_wheel::SOLVER_apply_relaxation(float *error_sq)
{
    pulse_sum_normal *m_side; // ecx
    pulse_sum_normal *m_fwd; // ecx
    float m_fwd_last_pulse_sum; // [esp+8h] [ebp-4h]
    float error_sq___; // [esp+14h] [ebp+8h]
    float error_sq___c; // [esp+14h] [ebp+8h]
    float error_sq___a; // [esp+14h] [ebp+8h]
    float error_sq___d; // [esp+14h] [ebp+8h]
    float error_sq___b; // [esp+14h] [ebp+8h]

    //pulse_sum_normal::SOLVER_apply_relaxation(&this->m_suspension, error_sq, 1);
    this->m_suspension.SOLVER_apply_relaxation(error_sq, 1);
    m_side = this->m_side;
    if ( m_side )
    {
        error_sq___ = m_side->m_pulse_sum;
        //pulse_sum_normal::SOLVER_apply_relaxation(m_side, error_sq, this->m_fwd == 0);
        m_side->SOLVER_apply_relaxation(error_sq, this->m_fwd == 0);
        if ( this->m_fwd )
        {
            m_fwd = this->m_fwd;
            m_fwd_last_pulse_sum = m_fwd->m_pulse_sum;
            //pulse_sum_normal::SOLVER_apply_relaxation(m_fwd, error_sq, 0);
            m_fwd->SOLVER_apply_relaxation(error_sq, 0);
            pulse_sum_wheel::addp_pulse_chain();
            error_sq___c = (this->m_side->m_pulse_sum - error_sq___) * this->m_side->m_denom;
            error_sq___a = error_sq___c * error_sq___c;
            if ( *error_sq < (double)error_sq___a )
                *error_sq = error_sq___a;
            error_sq___d = (this->m_fwd->m_pulse_sum - m_fwd_last_pulse_sum) * this->m_fwd->m_denom;
            error_sq___b = error_sq___d * error_sq___d;
            if ( *error_sq < (double)error_sq___b )
                *error_sq = error_sq___b;
        }
    }
}

void __thiscall pulse_sum_wheel::SOLVER_solver_prolog(float delta_t)
{
    //pulse_sum_normal *p_m_suspension; // esi
    //double vel; // st7
    //float *p_m_pulse_sum; // eax
    //pulse_sum_normal *m_side; // ecx
    //pulse_sum_normal *m_fwd; // ecx
    //double ps; // [esp+10h] [ebp-8h]
    //int savedregs; // [esp+18h] [ebp+0h] BYREF
    //
    //p_m_suspension = &this->m_suspension;
    //ps = this->m_suspension.m_right_side;
    //vel = this->m_suspension.get_vel();
    //p_m_pulse_sum = &p_m_suspension->m_pulse_sum_cache->m_pulse_sum;
    //p_m_suspension->m_right_side = ps - vel;
    //*((float *)&ps + 1) = *p_m_pulse_sum * delta_t;
    //p_m_suspension->m_pulse_sum = *((float *)&ps + 1);
    //p_m_suspension->m_pulse_sum = v6->clamp_pulse_sum(*((float *)&ps + 1));
    //p_m_suspension->apply(&p_m_suspension->m_pulse_sum);
    //m_side = this->m_side;
    //if ( m_side )
    //{
    //    //pulse_sum_normal::SOLVER_solver_prolog(m_side, delta_t);
    //    //pulse_sum_normal::SOLVER_solver_prolog(m_side, delta_t);
    //    m_fwd = this->m_fwd;
    //    if ( m_fwd )
    //    {
    //        //pulse_sum_normal::SOLVER_solver_prolog(m_fwd, delta_t);
    //        m_fwd->SOLVER_solver_prolog(delta_t);
    //        pulse_sum_wheel::addp_pulse_chain();
    //    }
    //}

    pulse_sum_normal *suspension = &this->m_suspension;

    // Store previous value
    float prev_right_side = suspension->m_right_side;

    // Compute velocity along suspension axis
    float vel = suspension->get_vel();

    // Update right side term
    suspension->m_right_side = prev_right_side - vel;

    // Integrate cached pulse
    float pulse = suspension->m_pulse_sum_cache->m_pulse_sum * delta_t;

    suspension->m_pulse_sum = pulse;
    suspension->m_pulse_sum = suspension->clamp_pulse_sum(suspension->m_pulse_sum);

    // Apply suspension impulse
    suspension->apply(&suspension->m_pulse_sum);

    // Process side constraint
    if (this->m_side)
    {
        this->m_side->SOLVER_solver_prolog(delta_t);

        // Process forward constraint
        if (this->m_fwd)
        {
            this->m_fwd->SOLVER_solver_prolog(delta_t);

            // Add pulses together in chain
            pulse_sum_wheel::addp_pulse_chain();
        }
    }
}

void __thiscall pulse_sum_contact_point::clamp_f(pulse_sum_contact *psc)
{
    double v3; // st7
    long double v4; // st7
    float pulse_sum_fric_sqa; // [esp+4h] [ebp-8h]
    float pulse_sum_fric_sqb; // [esp+4h] [ebp-8h]
    float pulse_sum_fric_sqc; // [esp+4h] [ebp-8h]
    float pulse_sum_fric_sqd; // [esp+4h] [ebp-8h]
    float pulse_sum_fric_sq; // [esp+4h] [ebp-8h]
    float ratio; // [esp+14h] [ebp+8h]
    float ratioa; // [esp+14h] [ebp+8h]

    ratio = -psc->m_fric_coef * this->m_pulse_sum.x;
    pulse_sum_fric_sqa = this->m_pulse_sum.y * this->m_pulse_sum.y;
    v3 = pulse_sum_fric_sqa;
    pulse_sum_fric_sqb = this->m_pulse_sum.z * this->m_pulse_sum.z;
    pulse_sum_fric_sqc = v3 + pulse_sum_fric_sqb;
    v4 = pulse_sum_fric_sqc;
    pulse_sum_fric_sqd = ratio * ratio;
    if ( pulse_sum_fric_sqd < v4 )
    {
        pulse_sum_fric_sq = sqrt(v4);
        if ( pulse_sum_fric_sq <= 0.0
            && _tlAssert(
                     "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\constraint_solver\\pulse_sum_contact_inline.h",
                     100,
                     "pulse_sum_fric > 0.0f",
                     "") )
        {
            __debugbreak();
        }
        if ( ratio < 0.0 )
        {
            if ( _tlAssert(
                         "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\constraint_solver\\pulse_sum_contact_inline.h",
                         101,
                         "m_pulse_sum_fric_limit >= 0.0f",
                         "") )
            {
                __debugbreak();
            }
        }
        ratioa = ratio / pulse_sum_fric_sq;
        this->m_pulse_sum.y = ratioa * this->m_pulse_sum.y;
        this->m_pulse_sum.z = ratioa * this->m_pulse_sum.z;
    }
}

void pulse_sum_contact_point::SOLVER_solver_prolog(
                pulse_sum_contact *psc,
                pulse_sum_cache *m_pulse_sum_cache,
                float delta_t)
{
    const phys_vec3 *vel; // eax
    pulse_sum_contact_point *v7; // ecx
    phys_vec3 v8; // [esp-10h] [ebp-1Ch] BYREF
    //_UNKNOWN *v9[2]; // [esp+0h] [ebp-Ch] BYREF
    //float delta_ta; // [esp+Ch] [ebp+0h]
    //
    //v9[0] = a2;
    //*&v9[1] = delta_ta;
    vel = pulse_sum_contact_point::get_vel(&v8, psc);
    this->m_right_side.x = this->m_right_side.x - vel->x;
    this->m_right_side.y = this->m_right_side.y - vel->y;
    this->m_right_side.z = this->m_right_side.z - vel->z;
    this->m_pulse_sum.x = m_pulse_sum_cache->m_pulse_sum * delta_t;
    this->m_pulse_sum.y = m_pulse_sum_cache[1].m_pulse_sum * delta_t;
    this->m_pulse_sum.z = delta_t * m_pulse_sum_cache[2].m_pulse_sum;
    if (this->m_pulse_sum.x > 0.0)
        this->m_pulse_sum.x = 0.0;
    //pulse_sum_contact_point::clamp_f(v7, psc);
    this->clamp_f(psc);
    //pulse_sum_contact_point::apply(this, psc, &this->m_pulse_sum);
    this->apply(psc, &this->m_pulse_sum);
}

void pulse_sum_contact_point::SOLVER_apply_relaxation(pulse_sum_contact *psc, float *error_sq)
{
    float x; // eax
    float z; // edx
    float w; // eax
    const phys_vec3 *objective; // eax
    double v9; // st6
    double v10; // st7
    phys_vec3 v11; // [esp-1Ch] [ebp-4Ch] BYREF
    float m_denom_xx; // [esp-Ch] [ebp-3Ch]
    float y; // [esp-8h] [ebp-38h]
    float m_denom_zz; // [esp-4h] [ebp-34h]
    float denom; // [esp+0h] [ebp-30h]
    phys_vec3 v16; // [esp+4h] [ebp-2Ch] BYREF
    float m_objective_8; // [esp+18h] [ebp-18h]
    float m_objective_12; // [esp+1Ch] [ebp-14h]
    float v19; // [esp+20h] [ebp-10h]
    //int v20; // [esp+24h] [ebp-Ch]
    //void *v21; // [esp+28h] [ebp-8h]
    //void *retaddr; // [esp+30h] [ebp+0h]
    //
    //v20 = a2;
    //v21 = retaddr;
    x = this->m_pulse_sum.x;
    z = this->m_pulse_sum.z;
    y = this->m_pulse_sum.y;
    m_denom_xx = x;
    w = this->m_pulse_sum.w;
    m_denom_zz = z;
    denom = w;
    objective = pulse_sum_contact_point::get_objective(&v11, psc);
    v16.x = objective->x - this->m_right_side.x;
    v16.y = objective->y - this->m_right_side.y;
    v16.z = objective->z - this->m_right_side.z;
    this->m_pulse_sum.x = this->m_pulse_sum.x - v16.x / this->m_denom_xx;
    if (this->m_pulse_sum.x > 0.0)
        this->m_pulse_sum.x = 0.0;
    v19 = this->m_pulse_sum.x - m_denom_xx;
    v16.y = this->m_denom_xy * v19 + v16.y;
    v16.z = v19 * this->m_denom_xz + v16.z;
    v9 = v16.z;
    v10 = v16.y;
    this->m_pulse_sum.y = this->m_pulse_sum.y - (this->m_inv_yz * v16.z + this->m_inv_yy * v16.y);
    this->m_pulse_sum.z = this->m_pulse_sum.z - (v10 * this->m_inv_yz + v9 * this->m_inv_zz);
    pulse_sum_contact_point::clamp_f(psc);
    v16.x = this->m_pulse_sum.x - m_denom_xx;
    v16.y = this->m_pulse_sum.y - y;
    v16.z = this->m_pulse_sum.z - m_denom_zz;
    pulse_sum_contact_point::apply(psc, &v16);
    m_denom_xx = this->m_denom_xx;
    y = this->m_denom_yy;
    m_denom_zz = this->m_denom_zz;
    m_objective_8 = m_denom_zz * v16.z;
    m_objective_12 = y * v16.y;
    v19 = m_denom_xx * v16.x;
    m_denom_xx = v19 * v19;
    y = m_objective_12 * m_objective_12;
    m_denom_zz = m_objective_8 * m_objective_8;
    if (*error_sq < (double)m_denom_xx)
        *error_sq = m_denom_xx;
    if (*error_sq < (double)y)
        *error_sq = y;
    if (*error_sq < (double)m_denom_zz)
        *error_sq = m_denom_zz;
}

void pulse_sum_contact_point::setup_vel_uni_restitution(
                pulse_sum_contact *psc,
                float restitution_k,
                float max_restitution_v,
                float delta_t,
                float max_penalty_restitution_vel,
                float min_restitution_impact_vel_sq)
{
#if 0
    pulse_sum_contact_point *v8; // ecx
    double v9; // st7
    double v10; // st7
    int v11; // ecx
    double v12; // st6
    double v13; // st6
    float pos; // [esp-8h] [ebp-14h]
    float v15; // [esp-8h] [ebp-14h]
    float v16; // [esp-8h] [ebp-14h]
    float v17; // [esp-8h] [ebp-14h]
    float v18; // [esp-8h] [ebp-14h]
    float v19; // [esp-4h] [ebp-10h]
    float impact_vel; // [esp-4h] [ebp-10h]
    float v21; // [esp-4h] [ebp-10h]
    int v22; // [esp+0h] [ebp-Ch] BYREF
    float dist; // [esp+4h] [ebp-8h]
    float retaddr; // [esp+Ch] [ebp+0h]

    v22 = a2;
    dist = retaddr;
    pos = pulse_sum_contact_point::get_pos(this, (int)&v22, psc);
    v9 = 0.016666668;
    if ( delta_t > 0.016666668 )
        v9 = delta_t;
    v19 = v9;
    v10 = pos;
    v15 = -pos / v19;
    v8->m_big_dirt = v15;
    if ( v15 < 0.0 )
        v8->m_big_dirt = v15 * 0.300000011920929;
    v16 = -max_penalty_restitution_vel;
    if ( v16 > (double)v8->m_big_dirt )
        v8->m_big_dirt = v16;
    if ( v8->m_big_dirt < 0.0 )
    {
        v8->m_right_side.x = PHYS_ZERO_VEC.x;
        v8->m_right_side.y = PHYS_ZERO_VEC.y;
        v8->m_right_side.z = PHYS_ZERO_VEC.z;
    }
    else
    {
        v8->m_right_side.x = v8->m_big_dirt;
        v8->m_right_side.y = 0.0;
        v8->m_right_side.z = 0.0;
        v8->m_big_dirt = 0.0;
    }
    if ( restitution_k > 0.0000099999997 && max_restitution_v > 0.0000099999997 && v10 >= 0.0 )
    {
        impact_vel = pulse_sum_contact_point::get_impact_vel(v8, (int)&v22, psc);
        v12 = impact_vel;
        if ( impact_vel > 0.0 )
        {
            v17 = v12 * v12;
            if ( min_restitution_impact_vel_sq < (double)v17 )
            {
                v21 = v12 * restitution_k;
                v13 = v21;
                if ( max_restitution_v < (double)v21 )
                    v13 = max_restitution_v;
                if ( *(float *)(v11 + 160) > -0.0000099999997 )
                {
                    *(float *)(v11 + 144) = *(float *)(v11 + 144) - v13;
                }
                else
                {
                    v18 = -v13;
                    if ( *(float *)(v11 + 160) > (double)v18 )
                    {
                        *(float *)(v11 + 144) = v18;
                        *(float *)(v11 + 160) = 0.0;
                    }
                }
            }
        }
    }
#endif
    // aislop
    //
    // ---- Penetration bias term ----
    //

    float pos = get_pos(psc);

    // dt = max(delta_t, MIN_NERP_DELTA_T)
    float dt = delta_t;
    const float MIN_DT = 0.016666668f;   // 1/60
    if (dt < MIN_DT)
        dt = MIN_DT;

    float bias = -pos / dt;
    m_big_dirt = bias;

    // if negative, scale by 0.3
    if (m_big_dirt < 0.0f)
        m_big_dirt *= 0.3f;

    // clamp to -max_penalty_restitution_vel
    float minBias = -max_penalty_restitution_vel;
    if (m_big_dirt < minBias)
        m_big_dirt = minBias;

    //
    // ---- Setup RHS ----
    //

    if (m_big_dirt >= 0.0f)
    {
        // positive bias -> push apart
        m_right_side.x = m_big_dirt;
        m_right_side.y = m_big_dirt;
        m_right_side.z = m_big_dirt;

        m_big_dirt = 0.0f;
    }
    else
    {
        // no correction
        m_right_side = PHYS_ZERO_VEC;
    }

    //
    // ---- Restitution ----
    //

    const float EPS = 1e-5f;

    if (restitution_k > EPS &&
        max_restitution_v > EPS &&
        pos >= 0.0f)
    {
        float impact_vel = get_impact_vel(psc);

        if (impact_vel > 0.0f)
        {
            float impact_vel_sq = impact_vel * impact_vel;

            if (impact_vel_sq > min_restitution_impact_vel_sq)
            {
                float restitution = impact_vel * restitution_k;

                if (restitution > max_restitution_v)
                    restitution = max_restitution_v;

                // Apply restitution to RHS.x
                if (m_big_dirt > -EPS)
                {
                    m_right_side.x -= restitution;
                }
                else
                {
                    float negRest = -restitution;

                    if (m_big_dirt > negRest)
                    {
                        m_right_side.x = negRest;
                        m_big_dirt = 0.0f;
                    }
                }
            }
        }
    }

}

void __thiscall pulse_sum_contact::SOLVER_solver_intermediate(float delta_t)
{
    pulse_sum_contact_point *m_list_pscp; // eax
    float *m_pulse_sum_cache_list; // edx
    pulse_sum_contact_point *v4; // ecx
    double v5; // st7
    double x; // st6

    m_list_pscp = this->m_list_pscp;
    m_pulse_sum_cache_list = (float *)this->m_pulse_sum_cache_list;
    v4 = &m_list_pscp[this->m_list_pscp_count];
    if ( m_list_pscp != v4 )
    {
        v5 = delta_t;
        do
        {
            x = m_list_pscp->m_pulse_sum.x;
            ++m_list_pscp;
            m_pulse_sum_cache_list += 3;
            *(m_pulse_sum_cache_list - 3) = x / v5;
            *(m_pulse_sum_cache_list - 2) = m_list_pscp[-1].m_pulse_sum.y / v5;
            *(m_pulse_sum_cache_list - 1) = m_list_pscp[-1].m_pulse_sum.z / v5;
            m_list_pscp[-1].m_right_side.x = m_list_pscp[-1].m_big_dirt + m_list_pscp[-1].m_right_side.x;
        }
        while ( m_list_pscp != v4 );
    }
}

pulse_sum_constraint_solver::pulse_sum_constraint_solver()
{
    this->m_solver_memory_allocator.m_first_block = 0;
    this->m_solver_memory_allocator.m_cur = 0;
    this->m_solver_memory_allocator.m_end = 0;
    this->m_solver_memory_allocator.m_total_memory_allocated = 0;
    this->m_solver_memory_allocator.m_mutex.m_count = 1;
    this->m_solver_memory_allocator.m_slot_pool = 0;

    //this->m_list_pulse_sum_node.m_first = 0;
    //this->m_list_pulse_sum_node.m_alloc_count = 0;
    //this->m_list_pulse_sum_node.m_last_next_ptr = &this->m_list_pulse_sum_node.m_first;
    //this->m_list_pulse_sum_normal.m_first = 0;
    //this->m_list_pulse_sum_normal.m_alloc_count = 0;
    //this->m_list_pulse_sum_normal.m_last_next_ptr = &this->m_list_pulse_sum_normal.m_first;
    //this->m_list_pulse_sum_point.m_first = 0;
    //this->m_list_pulse_sum_point.m_alloc_count = 0;
    //this->m_list_pulse_sum_point.m_last_next_ptr = &this->m_list_pulse_sum_point.m_first;
    //this->m_list_pulse_sum_angular.m_first = 0;
    //this->m_list_pulse_sum_angular.m_alloc_count = 0;
    //this->m_list_pulse_sum_angular.m_last_next_ptr = &this->m_list_pulse_sum_angular.m_first;
    //this->m_list_pulse_sum_wheel.m_first = 0;
    //this->m_list_pulse_sum_wheel.m_last_next_ptr = &this->m_list_pulse_sum_wheel.m_first;
    //this->m_list_pulse_sum_wheel.m_alloc_count = 0;
    //this->m_list_pulse_sum_contact.m_first = 0;
    //this->m_list_pulse_sum_contact.m_last_next_ptr = &this->m_list_pulse_sum_contact.m_first;
    //this->m_list_pulse_sum_contact.m_alloc_count = 0;
}

void __thiscall pulse_sum_constraint_solver::solve_constraints(rigid_body *const head)
{
    pulse_sum_node *j; // esi
    pulse_sum_normal *k; // esi
    double vel; // st7
    float *p_m_pulse_sum; // ecx
    pulse_sum_point *m; // esi
    pulse_sum_angular *n; // esi
    double v9; // st7
    float *v10; // edx
    double m_pulse_sum_max; // st7
    pulse_sum_wheel *ii; // esi
    pulse_sum_contact_point *m_list_pscp; // esi
    pulse_sum_contact_point *v14; // edi
    rigid_body_constraint_point *jj; // eax
    rigid_body_constraint_hinge *kk; // eax
    rigid_body_constraint_distance *mm; // eax
    rigid_body_constraint_ragdoll *nn; // eax
    rigid_body_constraint_wheel *i1; // eax
    rigid_body_constraint_angular_actuator *i2; // eax
    rigid_body_constraint_upright *i3; // eax
    rigid_body_constraint_custom_orientation *i4; // eax
    rigid_body_constraint_custom_path *i5; // eax
    rigid_body_constraint_contact *i6; // eax
    pulse_sum_normal *i7; // eax
    pulse_sum_point *i8; // esi
    pulse_sum_angular *i9; // eax
    pulse_sum_wheel *i10; // esi
    pulse_sum_contact *i11; // esi
    pulse_sum_node *m_first; // esi
    bool i12; // edi
    unsigned int m_flags; // eax
    pulse_sum_node *i13; // ebx
    rigid_body *m_rb; // eax
    double m_right_side; // [esp+10h] [ebp-10h]
    double v36; // [esp+10h] [ebp-10h]
    float delta_t; // [esp+14h] [ebp-Ch]
    float m_pulse_sum_cacheb; // [esp+18h] [ebp-8h]
    float m_pulse_sum_cachec; // [esp+18h] [ebp-8h]
    float m_pulse_sum_cached; // [esp+18h] [ebp-8h]
    float m_pulse_sum_cache; // [esp+18h] [ebp-8h]
    pulse_sum_cache *m_pulse_sum_cachea; // [esp+18h] [ebp-8h]
    float ia; // [esp+1Ch] [ebp-4h]
    pulse_sum_contact *i; // [esp+1Ch] [ebp-4h]
    int savedregs; // [esp+20h] [ebp+0h] BYREF
    float heada; // [esp+28h] [ebp+8h]
    float headb; // [esp+28h] [ebp+8h]

    for ( j = this->m_list_pulse_sum_node.m_first; j; j = j->m_next_link )
    {
        rbint::euler_integrate_velocity(j->m_rb, this->m_si.m_delta_t);
        j->t_vel.x = PHYS_ZERO_VEC.x;
        j->t_vel.y = PHYS_ZERO_VEC.y;
        j->t_vel.z = PHYS_ZERO_VEC.z;
        j->a_vel.x = PHYS_ZERO_VEC.x;
        j->a_vel.y = PHYS_ZERO_VEC.y;
        j->a_vel.z = PHYS_ZERO_VEC.z;
    }
    for ( k = this->m_list_pulse_sum_normal.m_first; k; k = k->m_next_link )
    {
        m_pulse_sum_cacheb = this->m_si.m_delta_t;
        m_right_side = k->m_right_side;
        //vel = pulse_sum_normal::get_vel(k, (int)&savedregs);
        vel = k->get_vel();
        p_m_pulse_sum = &k->m_pulse_sum_cache->m_pulse_sum;
        k->m_right_side = m_right_side - vel;
        m_pulse_sum_cachec = *p_m_pulse_sum * m_pulse_sum_cacheb;
        k->m_pulse_sum = m_pulse_sum_cachec;
        //k->m_pulse_sum = pulse_sum_normal::clamp_pulse_sum(k, m_pulse_sum_cachec);
        k->m_pulse_sum = k->clamp_pulse_sum(m_pulse_sum_cachec);
        //pulse_sum_normal::apply(k, &k->m_pulse_sum);
        k->apply(&k->m_pulse_sum);
    }

    for (m = this->m_list_pulse_sum_point.m_first; m; m = m->m_next_link)
    {
        //pulse_sum_point::SOLVER_solver_prolog(m, (int)&savedregs, this->m_si.m_delta_t);
        m->SOLVER_solver_prolog(this->m_si.m_delta_t);
    }

    for ( n = this->m_list_pulse_sum_angular.m_first; n; n = n->m_next_link )
    {
        m_pulse_sum_cached = this->m_si.m_delta_t;
        v36 = n->m_right_side;
        //v9 = pulse_sum_angular::get_vel(n);
        v9 = n->get_vel();
        v10 = &n->m_pulse_sum_cache->m_pulse_sum;
        n->m_right_side = v36 - v9;
        m_pulse_sum_cache = *v10 * m_pulse_sum_cached;
        n->m_pulse_sum = m_pulse_sum_cache;
        if ( n->m_pulse_sum_max < (double)n->m_pulse_sum_min
            && _tlAssert(
                     "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\constraint_solver\\pulse_sum_angular_inline.h",
                     40,
                     "m_pulse_sum_min <= m_pulse_sum_max",
                     "") )
        {
            __debugbreak();
        }
        m_pulse_sum_max = m_pulse_sum_cache;
        if ( n->m_pulse_sum_min <= (double)m_pulse_sum_cache )
        {
            if ( n->m_pulse_sum_max < m_pulse_sum_max )
                m_pulse_sum_max = n->m_pulse_sum_max;
        }
        else
        {
            m_pulse_sum_max = n->m_pulse_sum_min;
        }
        ia = m_pulse_sum_max;
        n->m_pulse_sum = ia;
        //pulse_sum_angular::apply(n, &n->m_pulse_sum);
        n->apply(&n->m_pulse_sum);
    }

    for (ii = this->m_list_pulse_sum_wheel.m_first; ii; ii = ii->m_next_link)
    {
        //pulse_sum_wheel::SOLVER_solver_prolog(ii, this->m_si.m_delta_t);
        ii->SOLVER_solver_prolog(this->m_si.m_delta_t);
    }

    for ( i = this->m_list_pulse_sum_contact.m_first; i; i = i->m_next_link )
    {
        delta_t = this->m_si.m_delta_t;
        m_list_pscp = i->m_list_pscp;
        v14 = &m_list_pscp[i->m_list_pscp_count];
        for ( m_pulse_sum_cachea = (pulse_sum_cache *)i->m_pulse_sum_cache_list; m_list_pscp != v14; ++m_list_pscp )
        {
            //pulse_sum_contact_point::SOLVER_solver_prolog(m_list_pscp, (int)&savedregs, i, m_pulse_sum_cachea, delta_t);
            m_list_pscp->SOLVER_solver_prolog(i, m_pulse_sum_cachea, delta_t);
            m_pulse_sum_cachea += 3;
        }
    }
    pulse_sum_constraint_solver::solve_iterative(this->m_si.m_max_vel_iters, this->m_si.m_max_vel_error_sq);
    for ( jj = head->m_partition_node.m_rbc_point_first; jj; jj = (rigid_body_constraint_point *)jj->m_next )
    {
        jj->m_ps_cache_list[0].m_pulse_sum = 0.0;
        jj->m_ps_cache_list[1].m_pulse_sum = 0.0;
        jj->m_ps_cache_list[2].m_pulse_sum = 0.0;
    }
    for ( kk = head->m_partition_node.m_rbc_hinge_first; kk; kk = (rigid_body_constraint_hinge *)kk->m_next )
    {
        kk->m_ps_cache[0].m_pulse_sum = 0.0;
        kk->m_ps_cache[1].m_pulse_sum = 0.0;
        kk->m_ps_cache[2].m_pulse_sum = 0.0;
        kk->m_ps_cache[3].m_pulse_sum = 0.0;
        kk->m_ps_cache[4].m_pulse_sum = 0.0;
        kk->m_ps_cache[5].m_pulse_sum = 0.0;
        kk->m_ps_cache[6].m_pulse_sum = 0.0;
        kk->m_ps_cache[7].m_pulse_sum = 0.0;
    }
    for ( mm = head->m_partition_node.m_rbc_dist_first; mm; mm = (rigid_body_constraint_distance *)mm->m_next )
    {
        mm->m_ps_cache_list[0].m_pulse_sum = 0.0;
        mm->m_ps_cache_list[1].m_pulse_sum = 0.0;
        mm->m_ps_cache_list[2].m_pulse_sum = 0.0;
    }
    for ( nn = head->m_partition_node.m_rbc_ragdoll_first; nn; nn = (rigid_body_constraint_ragdoll *)nn->m_next )
    {
        nn->m_ps_cache_list[0].m_pulse_sum = 0.0;
        nn->m_ps_cache_list[1].m_pulse_sum = 0.0;
        nn->m_ps_cache_list[2].m_pulse_sum = 0.0;
        nn->m_ps_cache_list[3].m_pulse_sum = 0.0;
        nn->m_ps_cache_list[4].m_pulse_sum = 0.0;
        nn->m_ps_cache_list[5].m_pulse_sum = 0.0;
        nn->m_ps_cache_list[6].m_pulse_sum = 0.0;
        nn->m_ps_cache_list[7].m_pulse_sum = 0.0;
        nn->m_ps_cache_list[8].m_pulse_sum = 0.0;
        nn->m_ps_cache_list[9].m_pulse_sum = 0.0;
    }
    for ( i1 = head->m_partition_node.m_rbc_wheel_first; i1; i1 = (rigid_body_constraint_wheel *)i1->m_next )
    {
        i1->m_ps_cache_list[0].m_pulse_sum = 0.0;
        i1->m_ps_cache_list[1].m_pulse_sum = 0.0;
        i1->m_ps_cache_list[2].m_pulse_sum = 0.0;
        i1->m_ps_cache_list[3].m_pulse_sum = 0.0;
    }
    for ( i2 = head->m_partition_node.m_rbc_angular_actuator_first;
                i2;
                i2 = (rigid_body_constraint_angular_actuator *)i2->m_next )
    {
        i2->m_ps_cache_list[0].m_pulse_sum = 0.0;
        i2->m_ps_cache_list[1].m_pulse_sum = 0.0;
        i2->m_ps_cache_list[2].m_pulse_sum = 0.0;
    }
    for ( i3 = head->m_partition_node.m_rbc_upright_first; i3; i3 = (rigid_body_constraint_upright *)i3->m_next )
        i3->m_ps_cache_list[0].m_pulse_sum = 0.0;
    for ( i4 = head->m_partition_node.m_rbc_custom_orientation_first;
                i4;
                i4 = (rigid_body_constraint_custom_orientation *)i4->m_next )
    {
        i4->m_ps_cache_list[0].m_pulse_sum = 0.0;
        i4->m_ps_cache_list[1].m_pulse_sum = 0.0;
        i4->m_ps_cache_list[2].m_pulse_sum = 0.0;
        i4->m_ps_cache_list[3].m_pulse_sum = 0.0;
        i4->m_ps_cache_list[4].m_pulse_sum = 0.0;
    }
    for ( i5 = head->m_partition_node.m_rbc_custom_path_first; i5; i5 = (rigid_body_constraint_custom_path *)i5->m_next )
    {
        i5->m_list_psc[0].m_pulse_sum = 0.0;
        i5->m_list_psc[1].m_pulse_sum = 0.0;
        i5->m_list_psc[2].m_pulse_sum = 0.0;
        i5->m_list_psc[3].m_pulse_sum = 0.0;
    }
    for ( i6 = head->m_partition_node.m_rbc_contact_first; i6; i6 = (rigid_body_constraint_contact *)i6->m_next )
        ;
    for ( i7 = this->m_list_pulse_sum_normal.m_first; i7; i7 = i7->m_next_link )
    {
        heada = i7->m_pulse_sum / this->m_si.m_delta_t;
        i7->m_pulse_sum_cache->m_pulse_sum = heada;
        i7->m_right_side = i7->m_big_dirt + i7->m_right_side;
    }

    for (i8 = this->m_list_pulse_sum_point.m_first; i8; i8 = i8->m_next_link)
    {
        //pulse_sum_point::SOLVER_solver_intermediate(i8, this->m_si.m_delta_t);
        i8->SOLVER_solver_intermediate(this->m_si.m_delta_t);
    }

    for ( i9 = this->m_list_pulse_sum_angular.m_first; i9; i9 = i9->m_next_link )
    {
        headb = i9->m_pulse_sum / this->m_si.m_delta_t;
        i9->m_pulse_sum_cache->m_pulse_sum = headb;
        i9->m_right_side = i9->m_big_dirt + i9->m_right_side;
    }

    for (i10 = this->m_list_pulse_sum_wheel.m_first; i10; i10 = i10->m_next_link)
    {
        //pulse_sum_wheel::SOLVER_solver_intermediate(i10, this->m_si.m_delta_t);
        i10->SOLVER_solver_intermediate(this->m_si.m_delta_t);
    }

    for (i11 = this->m_list_pulse_sum_contact.m_first; i11; i11 = i11->m_next_link)
    {
        //pulse_sum_contact::SOLVER_solver_intermediate(i11, this->m_si.m_delta_t);
        i11->SOLVER_solver_intermediate(this->m_si.m_delta_t);
    }

    pulse_sum_constraint_solver::solve_iterative(this->m_si.m_max_vel_pos_iters, this->m_si.m_max_vel_pos_error_sq);
    m_first = this->m_list_pulse_sum_node.m_first;
    for ( i12 = 1; m_first; m_first = m_first->m_next_link )
    {
        rbint::add_vel(m_first->m_rb, &m_first->t_vel, &m_first->a_vel);
        rbint::euler_integrate_pos(m_first->m_rb, this->m_si.m_delta_t);
        rbint::update_stability(m_first->m_rb, this->m_si.m_delta_t);
        i12 = i12 && (m_flags = m_first->m_rb->m_flags, (m_flags & 4) != 0) && (m_flags & 0x40) == 0;
    }
    for ( i13 = this->m_list_pulse_sum_node.m_first; i13; i13 = i13->m_next_link )
    {
        m_rb = i13->m_rb;
        if ( i12 )
            m_rb->m_flags |= 8u;
        else
            m_rb->m_flags &= ~8u;
    }
}

void __thiscall pulse_sum_constraint_solver::execute_constraint_solver(rigid_body *const head)
{
    rigid_body *v2; // edi
    rigid_body *m_next_node; // ecx
    double i; // st7
    double m_outside_delta_t; // st6
    double v7; // st7
    rigid_body_constraint_point *j; // edi
    rigid_body_constraint_hinge *k; // edi
    rigid_body_constraint_distance *m; // edi
    rigid_body_constraint_ragdoll *n; // edi
    rigid_body_constraint_wheel *ii; // edi
    rigid_body_constraint_angular_actuator *jj; // edi
    rigid_body_constraint_upright *kk; // edi
    rigid_body_constraint_custom_orientation *mm; // edi
    rigid_body_constraint_custom_path *nn; // edi
    rigid_body_constraint_contact *i1; // edi
    phys_transient_allocator::block_header *m_first_block; // ecx
    unsigned int m_total_memory_allocated; // edx
    char *m_end; // ecx
    int m_psys_max_vel_iters; // eax
    int v22; // ecx
    int v23; // eax
    int v24; // eax
    rigid_body *v25; // ebx
    pulse_sum_node *v26; // edi
    pulse_sum_node **m_last_next_ptr; // edx
    rigid_body_constraint_point *i2; // edi
    rigid_body_constraint_hinge *i3; // edi
    rigid_body_constraint_distance *i4; // edi
    rigid_body_constraint_ragdoll *i5; // edi
    rigid_body_constraint_wheel *i6; // edi
    rigid_body_constraint_angular_actuator *i7; // edi
    rigid_body_constraint_upright *i8; // edi
    rigid_body_constraint_custom_orientation *i9; // edi
    rigid_body_constraint_custom_path *i10; // edi
    rigid_body_constraint_contact *i11; // edi
    pulse_sum_constraint_solver::temp_user_rigid_body *i12; // edi
    rigid_body_constraint_point *i13; // eax
    rigid_body_constraint_hinge *i14; // eax
    rigid_body_constraint_distance *i15; // edi
    rigid_body_constraint_ragdoll *i16; // eax
    rigid_body_constraint_wheel *i17; // eax
    rigid_body_constraint_angular_actuator *i18; // edi
    rigid_body_constraint_upright *i19; // eax
    rigid_body_constraint_custom_orientation *i20; // eax
    rigid_body_constraint_custom_path *i21; // eax
    rigid_body_constraint_contact *i22; // eax
    int m_memory_high_water; // eax
    bool v50; // zf
    pulse_sum_constraint_solver::user_rigid_body_restore_info *i23; // eax
    rigid_body_constraint_point *i24; // esi
    rigid_body_constraint_hinge *i25; // eax
    rigid_body_constraint_distance *i26; // eax
    rigid_body_constraint_ragdoll *i27; // eax
    rigid_body_constraint_wheel *i28; // esi
    rigid_body_constraint_angular_actuator *i29; // eax
    rigid_body_constraint_upright *i30; // esi
    rigid_body_constraint_custom_orientation *i31; // eax
    rigid_body_constraint_custom_path *i32; // eax
    rigid_body_constraint_contact *i33; // esi
    rigid_body *v62; // esi
    phys_transient_allocator::allocator_state saved_allocator_state; // [esp+10h] [ebp-2Ch] BYREF
    int v64; // [esp+20h] [ebp-1Ch]
    phys_transient_allocator *p_m_solver_memory_allocator; // [esp+24h] [ebp-18h]
    float max_delta_t; // [esp+28h] [ebp-14h]
    int sub_steps; // [esp+2Ch] [ebp-10h]
    phys_inplace_avl_tree<user_rigid_body *,pulse_sum_constraint_solver::temp_user_rigid_body,pulse_sum_constraint_solver::temp_user_rigid_body::avl_tree_accessor> turb_search_tree; // [esp+30h] [ebp-Ch] BYREF
    phys_simple_link_list<pulse_sum_constraint_solver::user_rigid_body_restore_info> list_urbri; // [esp+34h] [ebp-8h] BYREF
    phys_simple_link_list<pulse_sum_constraint_solver::temp_user_rigid_body> list_turb; // [esp+38h] [ebp-4h] BYREF
    int savedregs; // [esp+3Ch] [ebp+0h] BYREF

    v2 = head;
    if ( !head
        && _tlAssert(
                 "C:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\constraint_solver\\pulse_sum_constraint_solver_inline.h",
                 153,
                 "head",
                 "") )
    {
        __debugbreak();
    }
    m_next_node = head->m_partition_node.m_next_node;
    max_delta_t = head->m_max_delta_t;
    for ( i = max_delta_t; m_next_node; m_next_node = m_next_node->m_partition_node.m_next_node )
    {
        if ( m_next_node->m_max_delta_t < i )
        {
            max_delta_t = m_next_node->m_max_delta_t;
            i = max_delta_t;
        }
    }
    m_outside_delta_t = this->m_outside_delta_t;
    sub_steps = (int)(m_outside_delta_t / i);
    if ( (double)sub_steps * i < m_outside_delta_t )
        ++sub_steps;
    *(float *)&v64 = (float)sub_steps;
    if ( this->m_outside_delta_t > i * *(float *)&v64
        && _tlAssert(
                 "C:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\constraint_solver\\pulse_sum_constraint_solver_inline.h",
                 162,
                 "m_outside_delta_t <= max_delta_t * sub_steps",
                 "") )
    {
        __debugbreak();
    }
    v7 = this->m_outside_delta_t;
    turb_search_tree.m_tree_root = 0;
    max_delta_t = v7;
    list_turb.m_first = 0;
    list_urbri.m_first = 0;
    if ( this->m_solver_memory_allocator.m_cur
        && _tlAssert(
                 "C:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\constraint_solver\\pulse_sum_constraint_solver_inline.h",
                 171,
                 "m_solver_memory_allocator.is_empty()",
                 "") )
    {
        __debugbreak();
    }
    do
    {
        rbint::solver_prolog(v2, max_delta_t);
        //rigid_body::swap_last_position(v2);
        v2->swap_last_position();
        v2 = v2->m_partition_node.m_next_node;
    }
    while ( v2 );
    for ( j = head->m_partition_node.m_rbc_point_first; j; j = (rigid_body_constraint_point *)j->m_next )
        pulse_sum_constraint_solver::add_urb(&turb_search_tree, &list_turb, &list_urbri, j);
    for ( k = head->m_partition_node.m_rbc_hinge_first; k; k = (rigid_body_constraint_hinge *)k->m_next )
        pulse_sum_constraint_solver::add_urb(&turb_search_tree, &list_turb, &list_urbri, k);
    for ( m = head->m_partition_node.m_rbc_dist_first; m; m = (rigid_body_constraint_distance *)m->m_next )
        pulse_sum_constraint_solver::add_urb(&turb_search_tree, &list_turb, &list_urbri, m);
    for ( n = head->m_partition_node.m_rbc_ragdoll_first; n; n = (rigid_body_constraint_ragdoll *)n->m_next )
        pulse_sum_constraint_solver::add_urb(&turb_search_tree, &list_turb, &list_urbri, n);
    for ( ii = head->m_partition_node.m_rbc_wheel_first; ii; ii = (rigid_body_constraint_wheel *)ii->m_next )
        pulse_sum_constraint_solver::add_urb(&turb_search_tree, &list_turb, &list_urbri, ii);
    for ( jj = head->m_partition_node.m_rbc_angular_actuator_first;
                jj;
                jj = (rigid_body_constraint_angular_actuator *)jj->m_next )
    {
        pulse_sum_constraint_solver::add_urb(&turb_search_tree, &list_turb, &list_urbri, jj);
    }
    for ( kk = head->m_partition_node.m_rbc_upright_first; kk; kk = (rigid_body_constraint_upright *)kk->m_next )
        pulse_sum_constraint_solver::add_urb(&turb_search_tree, &list_turb, &list_urbri, kk);
    for ( mm = head->m_partition_node.m_rbc_custom_orientation_first;
                mm;
                mm = (rigid_body_constraint_custom_orientation *)mm->m_next )
    {
        pulse_sum_constraint_solver::add_urb(&turb_search_tree, &list_turb, &list_urbri, mm);
    }
    for ( nn = head->m_partition_node.m_rbc_custom_path_first; nn; nn = (rigid_body_constraint_custom_path *)nn->m_next )
        pulse_sum_constraint_solver::add_urb(&turb_search_tree, &list_turb, &list_urbri, nn);
    for ( i1 = head->m_partition_node.m_rbc_contact_first; i1; i1 = (rigid_body_constraint_contact *)i1->m_next )
        pulse_sum_constraint_solver::add_urb(&turb_search_tree, &list_turb, &list_urbri, i1);
    m_first_block = this->m_solver_memory_allocator.m_first_block;
    saved_allocator_state.m_cur = this->m_solver_memory_allocator.m_cur;
    m_total_memory_allocated = this->m_solver_memory_allocator.m_total_memory_allocated;
    p_m_solver_memory_allocator = &this->m_solver_memory_allocator;
    saved_allocator_state.m_first_block = m_first_block;
    m_end = this->m_solver_memory_allocator.m_end;
    m_psys_max_vel_iters = this->m_psys_max_vel_iters;
    saved_allocator_state.m_total_memory_allocated = m_total_memory_allocated;
    saved_allocator_state.m_end = m_end;
    v22 = sub_steps;
    v23 = m_psys_max_vel_iters / sub_steps;
    if ( v23 <= 1 )
        v23 = 1;
    this->m_si.m_max_vel_iters = v23;
    v24 = this->m_psys_max_vel_pos_iters / v22;
    if ( v24 <= 1 )
        v24 = 1;
    this->m_si.m_max_vel_pos_iters = v24;
    this->m_si.m_max_vel_error_sq = 0.1156;
    this->m_si.m_max_vel_pos_error_sq = 2.8900001;
    this->m_si.m_delta_t = max_delta_t / *(float *)&v64;
    if ( v22 > 0 )
    {
        v64 = v22;
        do
        {
            //phys_transient_allocator::reset_to_state(p_m_solver_memory_allocator, &saved_allocator_state);
            p_m_solver_memory_allocator->reset_to_state(&saved_allocator_state);
            v25 = head;

            this->m_list_pulse_sum_node.clear();
            this->m_list_pulse_sum_normal.clear();
            this->m_list_pulse_sum_point.clear();
            this->m_list_pulse_sum_angular.clear();
            this->m_list_pulse_sum_wheel.clear();
            this->m_list_pulse_sum_contact.clear();

            do
            {
                //v26 = (pulse_sum_node *)phys_transient_allocator::allocate(p_m_solver_memory_allocator, 128, 16, 0, SOLVER_MEMORY_ALLOCATOR_ERROR_MSG);
                v26 = (pulse_sum_node *)p_m_solver_memory_allocator->allocate(128, 16, 0, SOLVER_MEMORY_ALLOCATOR_ERROR_MSG);
                if ( !this->m_list_pulse_sum_node.m_last_next_ptr
                    && _tlAssert(
                             "C:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\phys_mem.h",
                             230,
                             "m_last_next_ptr",
                             "") )
                {
                    __debugbreak();
                }
                v26->m_next_link = 0;
                m_last_next_ptr = this->m_list_pulse_sum_node.m_last_next_ptr;
                ++this->m_list_pulse_sum_node.m_alloc_count;
                *m_last_next_ptr = v26;
                this->m_list_pulse_sum_node.m_last_next_ptr = &v26->m_next_link;
                rbint::setup_constraint(v25, v26);
                v25 = v25->m_partition_node.m_next_node;
            }
            while ( v25 );

            for (i2 = head->m_partition_node.m_rbc_point_first; i2; i2 = (rigid_body_constraint_point *)i2->m_next)
            {
                //rigid_body_constraint_point::setup_constraint(i2, (int)&savedregs, this, this->m_si.m_delta_t);
                i2->setup_constraint(this, this->m_si.m_delta_t);
            }
            for (i3 = head->m_partition_node.m_rbc_hinge_first; i3; i3 = (rigid_body_constraint_hinge *)i3->m_next)
            {
                //rigid_body_constraint_hinge::setup_constraint(i3, (int)&savedregs, this, this->m_si.m_delta_t);
                i3->setup_constraint(this, this->m_si.m_delta_t);
            }
            for (i4 = head->m_partition_node.m_rbc_dist_first; i4; i4 = (rigid_body_constraint_distance *)i4->m_next)
            {
                //rigid_body_constraint_distance::setup_constraint(i4, (int)&savedregs, this, this->m_si.m_delta_t);
                i4->setup_constraint(this, this->m_si.m_delta_t);
            }
            for (i5 = head->m_partition_node.m_rbc_ragdoll_first; i5; i5 = (rigid_body_constraint_ragdoll *)i5->m_next)
            {
                //rigid_body_constraint_ragdoll::setup_constraint(i5, (int)&savedregs, this, this->m_si.m_delta_t);
                i5->setup_constraint(this, this->m_si.m_delta_t);
            }
            for (i6 = head->m_partition_node.m_rbc_wheel_first; i6; i6 = (rigid_body_constraint_wheel *)i6->m_next)
            {
                //rigid_body_constraint_wheel::setup_constraint(i6, (int)&savedregs, this, this->m_si.m_delta_t);
                i6->setup_constraint(this, this->m_si.m_delta_t);
            }

            for ( i7 = head->m_partition_node.m_rbc_angular_actuator_first;
                        i7;
                        i7 = (rigid_body_constraint_angular_actuator *)i7->m_next )
            {
                //rigid_body_constraint_angular_actuator::setup_constraint(i7, (const phys_mat44 *)&savedregs, this, this->m_si.m_delta_t);
                i7->setup_constraint(this, this->m_si.m_delta_t);
            }
            for (i8 = head->m_partition_node.m_rbc_upright_first; i8; i8 = (rigid_body_constraint_upright *)i8->m_next)
            {
                //rigid_body_constraint_upright::setup_constraint(i8, (int)&savedregs, this, this->m_si.m_delta_t);
                i8->setup_constraint(this, this->m_si.m_delta_t);
            }
            for ( i9 = head->m_partition_node.m_rbc_custom_orientation_first;
                        i9;
                        i9 = (rigid_body_constraint_custom_orientation *)i9->m_next )
            {
                //rigid_body_constraint_custom_orientation::setup_constraint(i9, this, this->m_si.m_delta_t);
                i9->setup_constraint(this, this->m_si.m_delta_t);
            }
            for ( i10 = head->m_partition_node.m_rbc_custom_path_first;
                        i10;
                        i10 = (rigid_body_constraint_custom_path *)i10->m_next )
            {
                //rigid_body_constraint_custom_path::setup_constraint(i10, (int)&savedregs, this, this->m_si.m_delta_t);
                i10->setup_constraint(this, this->m_si.m_delta_t);
            }
            for (i11 = head->m_partition_node.m_rbc_contact_first; i11; i11 = (rigid_body_constraint_contact *)i11->m_next)
            {
                //rigid_body_constraint_contact::setup_constraint(i11, this, this->m_si.m_delta_t);
                i11->setup_constraint(this, this->m_si.m_delta_t);
            }
            pulse_sum_constraint_solver::solve_constraints(head);

            for (i12 = list_turb.m_first; i12; i12 = i12->m_next_link)
            {
                //rbint::substep((int)&savedregs, i12, this->m_si.m_delta_t);
                rbint::substep(i12, this->m_si.m_delta_t);
            }

            for ( i13 = head->m_partition_node.m_rbc_point_first; i13; i13 = (rigid_body_constraint_point *)i13->m_next )
                ;
            for ( i14 = head->m_partition_node.m_rbc_hinge_first; i14; i14 = (rigid_body_constraint_hinge *)i14->m_next )
                ;
            for (i15 = head->m_partition_node.m_rbc_dist_first; i15; i15 = (rigid_body_constraint_distance *)i15->m_next)
            {
                //rigid_body_constraint_distance::inner_update(i15, this->m_si.m_delta_t);
                i15->inner_update(this->m_si.m_delta_t);
            }
            for ( i16 = head->m_partition_node.m_rbc_ragdoll_first; i16; i16 = (rigid_body_constraint_ragdoll *)i16->m_next )
                ;
            for ( i17 = head->m_partition_node.m_rbc_wheel_first; i17; i17 = (rigid_body_constraint_wheel *)i17->m_next )
                ;
            for ( i18 = head->m_partition_node.m_rbc_angular_actuator_first;
                        i18;
                        i18 = (rigid_body_constraint_angular_actuator *)i18->m_next )
            {
                //rigid_body_constraint_angular_actuator::inner_update(i18, COERCE_FLOAT(&savedregs), this->m_si.m_delta_t);
                i18->inner_update(this->m_si.m_delta_t);
            }
            for ( i19 = head->m_partition_node.m_rbc_upright_first; i19; i19 = (rigid_body_constraint_upright *)i19->m_next )
                ;
            for ( i20 = head->m_partition_node.m_rbc_custom_orientation_first;
                        i20;
                        i20 = (rigid_body_constraint_custom_orientation *)i20->m_next )
            {
                ;
            }
            for ( i21 = head->m_partition_node.m_rbc_custom_path_first;
                        i21;
                        i21 = (rigid_body_constraint_custom_path *)i21->m_next )
            {
                ;
            }
            for ( i22 = head->m_partition_node.m_rbc_contact_first; i22; i22 = (rigid_body_constraint_contact *)i22->m_next )
                ;
            m_memory_high_water = this->m_solver_memory_allocator.m_total_memory_allocated;
            if ( this->m_memory_high_water > m_memory_high_water )
                m_memory_high_water = this->m_memory_high_water;
            v50 = v64-- == 1;
            this->m_memory_high_water = m_memory_high_water;
        }
        while ( !v50 );
    }

    for ( i23 = list_urbri.m_first; i23; i23 = i23->m_next_link )
        *i23->m_rbc_urb = i23->m_original_urb;

    for (i24 = head->m_partition_node.m_rbc_point_first; i24; i24 = (rigid_body_constraint_point *)i24->m_next)
    {
        //rigid_body_constraint_point::epilog_vel_constraint(i24, max_delta_t);
        i24->epilog_vel_constaint(max_delta_t);
    }

    for ( i25 = head->m_partition_node.m_rbc_hinge_first; i25; i25 = (rigid_body_constraint_hinge *)i25->m_next )
        ;
    for ( i26 = head->m_partition_node.m_rbc_dist_first; i26; i26 = (rigid_body_constraint_distance *)i26->m_next )
        ;
    for ( i27 = head->m_partition_node.m_rbc_ragdoll_first; i27; i27 = (rigid_body_constraint_ragdoll *)i27->m_next )
        ;
    for (i28 = head->m_partition_node.m_rbc_wheel_first; i28; i28 = (rigid_body_constraint_wheel *)i28->m_next)
    {
        //rigid_body_constraint_wheel::epilog_vel_constraint(i28, (int)&savedregs, max_delta_t);
        i28->epilog_vel_constraint(max_delta_t);
    }

    for ( i29 = head->m_partition_node.m_rbc_angular_actuator_first;
                i29;
                i29 = (rigid_body_constraint_angular_actuator *)i29->m_next )
    {
        ;
    }

    for (i30 = head->m_partition_node.m_rbc_upright_first; i30; i30 = (rigid_body_constraint_upright *)i30->m_next)
    {
        //rigid_body_constraint_upright::epilog_vel_constraint(i30, max_delta_t);
        i30->epilog_vel_constraint(max_delta_t);
    }

    for ( i31 = head->m_partition_node.m_rbc_custom_orientation_first;
                i31;
                i31 = (rigid_body_constraint_custom_orientation *)i31->m_next )
    {
        ;
    }

    for ( i32 = head->m_partition_node.m_rbc_custom_path_first; i32; i32 = (rigid_body_constraint_custom_path *)i32->m_next )
        ;

    for ( i33 = head->m_partition_node.m_rbc_contact_first; i33; i33 = (rigid_body_constraint_contact *)i33->m_next )
    {
        //rigid_body_constraint_contact::update_smallest_lambda(i33);
        i33->update_smallest_lambda();
        i33->m_list_contact_point_info_buffer_2.m_first = i33->m_list_contact_point_info_buffer_1.m_first;
        i33->m_list_contact_point_info_buffer_1.m_first = 0;
    }

    v62 = head;
    do
    {
        v62->m_force_sum.x = PHYS_ZERO_VEC.x;
        v62->m_force_sum.y = PHYS_ZERO_VEC.y;
        v62->m_force_sum.z = PHYS_ZERO_VEC.z;
        v62->m_torque_sum.x = PHYS_ZERO_VEC.x;
        v62->m_torque_sum.y = PHYS_ZERO_VEC.y;
        v62->m_torque_sum.z = PHYS_ZERO_VEC.z;
        //rigid_body::update_last_position(v62);
        v62->update_last_position();
        v62 = v62->m_partition_node.m_next_node;
    }
    while ( v62 );

    //phys_transient_allocator::reset(p_m_solver_memory_allocator);
    p_m_solver_memory_allocator->reset();
}

