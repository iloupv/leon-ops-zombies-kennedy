#include "rbc_def_ragdoll.h"
#include "rbc_def_generic.h"
#include <physics/phys_assert.h>
#include <physics/phys_constraint_solver_multithreaded.h>

void ragdoll_joint_limit_info::set_b1_ud_loc(const phys_vec3 *b1_ud_loc)
{
  phys_vec3 *v4; // eax
  phys_vec3 v5; // [esp-10h] [ebp-1Ch] BYREF
  //int v6; // [esp+0h] [ebp-Ch]
  //void *v7; // [esp+4h] [ebp-8h]
  //void *retaddr; // [esp+Ch] [ebp+0h]
  //
  //v6 = a2;
  //v7 = retaddr;
  v4 = phys_Unitize(&v5, b1_ud_loc);
  this->m_b1_ud_loc.x = v4->x;
  this->m_b1_ud_loc.y = v4->y;
  this->m_b1_ud_loc.z = v4->z;
  PHYS_ASSERT_UNIT(&this->m_b1_ud_loc);
}

void __thiscall rigid_body_constraint_ragdoll::set(
        const phys_vec3 *b1_r_loc,
        const phys_vec3 *b2_r_loc)
{
  this->m_b1_r_loc.x = b1_r_loc->x;
  this->m_b1_r_loc.y = b1_r_loc->y;
  this->m_b1_r_loc.z = b1_r_loc->z;
  this->m_b2_r_loc.x = b2_r_loc->x;
  this->m_b2_r_loc.y = b2_r_loc->y;
  this->m_b2_r_loc.z = b2_r_loc->z;
}

void rigid_body_constraint_ragdoll::set_snider_style(
        const phys_vec3 *b1_axis_loc,
        const phys_vec3 *b1_ref_loc)
{
  phys_vec3 *v5; // eax
  phys_vec3 *v6; // eax
  double y; // st7
  const phys_vec3 *v8; // eax
  phys_vec3 v9; // [esp-20h] [ebp-2Ch] BYREF
  const phys_vec3 *p_m_b1_ref_loc; // [esp-4h] [ebp-10h]
  //int v11; // [esp+0h] [ebp-Ch]
  //void *v12; // [esp+4h] [ebp-8h]
  //void *retaddr; // [esp+Ch] [ebp+0h]
  //
  //v11 = a2;
  //v12 = retaddr;
  v5 = phys_Unitize(&v9, b1_axis_loc);
  this->m_b1_axis_loc.x = v5->x;
  this->m_b1_axis_loc.y = v5->y;
  this->m_b1_axis_loc.z = v5->z;
  v6 = phys_Unitize(&v9, b1_ref_loc);
  this->m_b1_ref_loc.x = v6->x;
  y = v6->y;
  p_m_b1_ref_loc = &this->m_b1_ref_loc;
  this->m_b1_ref_loc.y = y;
  this->m_b1_ref_loc.z = v6->z;
  if ( (this->m_flags & 4) != 0 )
  {
    v8 = construct_orth_ud(&v9, &this->m_b1_axis_loc);
    this->m_b1_a1_loc.x = v8->x;
    this->m_b1_a1_loc.y = v8->y;
    this->m_b1_a1_loc.z = v8->z;
    v9.x = this->m_b1_axis_loc.y * this->m_b1_a1_loc.z - this->m_b1_axis_loc.z * this->m_b1_a1_loc.y;
    v9.y = this->m_b1_a1_loc.x * this->m_b1_axis_loc.z - this->m_b1_axis_loc.x * this->m_b1_a1_loc.z;
    v9.z = this->m_b1_axis_loc.x * this->m_b1_a1_loc.y - this->m_b1_axis_loc.y * this->m_b1_a1_loc.x;
    this->m_b1_a2_loc.x = v9.x;
    this->m_b1_a2_loc.y = v9.y;
    this->m_b1_a2_loc.z = v9.z;
  }
  PHYS_ASSERT_UNIT(&this->m_b1_axis_loc);
  PHYS_ASSERT_UNIT(p_m_b1_ref_loc);
}

void __thiscall rigid_body_constraint_ragdoll::set_damp_k(float damp_k)
{
  this->m_damp_k = damp_k;
  if ( damp_k <= 0.0 )
    this->m_flags &= ~0x40u;
  else
    this->m_flags |= 0x40u;
}

void ragdoll_joint_limit_info::set(
        const phys_vec3 *b1_ud_loc,
        float theta_limit)
{
  phys_vec3 *v5; // eax
  double v6; // st7
  double v7; // st7
  phys_vec3 v8; // [esp-20h] [ebp-2Ch] BYREF
  const phys_vec3 *v9; // [esp-4h] [ebp-10h]
  //int v10; // [esp+0h] [ebp-Ch]
  //void *v11; // [esp+4h] [ebp-8h]
  //void *retaddr; // [esp+Ch] [ebp+0h]
  //
  //v10 = a2;
  //v11 = retaddr;
  if ( theta_limit <= 0.0 && _tlAssert("source/rbc_def_ragdoll.cpp", 7, "theta_limit > 0.0f", "") )
    __debugbreak();
  v5 = phys_Unitize(&v8, b1_ud_loc);
  this->m_b1_ud_loc.x = v5->x;
  this->m_b1_ud_loc.y = v5->y;
  this->m_b1_ud_loc.z = v5->z;
  *(float *)&v9 = cos(theta_limit);
  this->m_b1_ud_limit_co_ = *(float *)&v9;
  *(float *)&v9 = sin(theta_limit);
  this->m_b1_ud_limit_si_ = *(float *)&v9;
  *(float *)&v9 = theta_limit - 0.04363323003053665;
  v6 = *(float *)&v9;
  if ( *(float *)&v9 > 0.0 )
    v6 = 0.0;
  *(float *)&v9 = v6;
  *(float *)&v9 = cos(*(float *)&v9);
  v7 = *(float *)&v9;
  v9 = (const phys_vec3 *)this;
  this->m_b1_ud_active_limit_co_ = v7;
  PHYS_ASSERT_UNIT(v9);
}

void __thiscall ragdoll_joint_limit_info::set_theta_limit(float theta_limit)
{
  double v3; // st7
  float v4; // [esp+4h] [ebp-4h]
  float v5; // [esp+4h] [ebp-4h]
  float theta_limita; // [esp+10h] [ebp+8h]
  float theta_limitb; // [esp+10h] [ebp+8h]
  float theta_limitc; // [esp+10h] [ebp+8h]

  if ( theta_limit <= 0.0 && _tlAssert("source/rbc_def_ragdoll.cpp", 23, "theta_limit > 0.0f", "") )
    __debugbreak();
  v4 = cos(theta_limit);
  this->m_b1_ud_limit_co_ = v4;
  v5 = sin(theta_limit);
  this->m_b1_ud_limit_si_ = v5;
  theta_limita = theta_limit - 0.04363323003053665;
  v3 = theta_limita;
  if ( theta_limita > 0.0 )
    v3 = 0.0;
  theta_limitb = v3;
  theta_limitc = cos(theta_limitb);
  this->m_b1_ud_active_limit_co_ = theta_limitc;
}

void rigid_body_constraint_ragdoll::set_theta_min_max(
        const phys_vec3 *b2_ref_loc,
        float theta_min,
        float theta_max)
{
    phys_vec3 *p_m_b2_axis_loc; // edi
    const phys_vec3 *v7; // eax
    phys_vec3 *v8; // eax
    const phys_vec3 *v9; // eax
    phys_vec3 *v10; // eax
    phys_mat44 v11; // [esp+18h] [ebp-7Ch] BYREF
    phys_vec3 rv_52; // [esp+58h] [ebp-3Ch] BYREF
    phys_vec3 v13; // [esp+68h] [ebp-2Ch] BYREF
    phys_vec3 v14; // [esp+78h] [ebp-1Ch] BYREF
    //_DWORD v15[3]; // [esp+88h] [ebp-Ch] BYREF
    //_UNKNOWN *retaddr; // [esp+94h] [ebp+0h]
    //
    //v15[0] = a2;
    //v15[1] = retaddr;
    p_m_b2_axis_loc = &this->m_b2_axis_loc;
    make_rotate(&v11, &this->m_b2_axis_loc, theta_min, 1000.0);
    v7 = phys_multiply(&v14, &v11, b2_ref_loc);
    v8 = phys_Unitize(&v13, v7);
    this->m_b2_ref_min_loc.x = v8->x;
    this->m_b2_ref_min_loc.y = v8->y;
    this->m_b2_ref_min_loc.z = v8->z;
    make_rotate(&v11, p_m_b2_axis_loc, theta_max, 1000.0);
    v9 = phys_multiply(&v13, &v11, b2_ref_loc);
    v10 = phys_Unitize(&rv_52, v9);
    this->m_b2_ref_max_loc.x = v10->x;
    this->m_b2_ref_max_loc.y = v10->y;
    this->m_b2_ref_max_loc.z = v10->z;
}

void rigid_body_constraint_ragdoll::set_hinge(
        const phys_vec3 *b1_axis_loc,
        const phys_vec3 *b2_axis_loc,
        const phys_vec3 *b1_ref_loc,
        const phys_vec3 *b2_ref_loc,
        float theta_min,
        float theta_max)
{
    phys_vec3 *v9; // eax
    phys_vec3 *v10; // eax
    phys_vec3 *v11; // eax
    const phys_vec3 *v12; // eax
    const phys_vec3 *v13; // eax
    phys_vec3 *v14; // eax
    const phys_vec3 *v15; // eax
    phys_vec3 *v16; // eax
    phys_mat44 v17; // [esp+18h] [ebp-7Ch] BYREF
    phys_vec3 rv_52; // [esp+58h] [ebp-3Ch] BYREF
    phys_vec3 v19; // [esp+68h] [ebp-2Ch] BYREF
    phys_vec3 v20; // [esp+78h] [ebp-1Ch] BYREF
    //_DWORD v21[3]; // [esp+88h] [ebp-Ch] BYREF
    //_UNKNOWN *retaddr; // [esp+94h] [ebp+0h]
    //
    //v21[0] = a2;
    //v21[1] = retaddr;
    v9 = phys_Unitize(&v20, b1_axis_loc);
    this->m_b1_axis_loc.x = v9->x;
    this->m_b1_axis_loc.y = v9->y;
    this->m_b1_axis_loc.z = v9->z;
    v10 = phys_Unitize(&v20, b2_axis_loc);
    this->m_b2_axis_loc.x = v10->x;
    this->m_b2_axis_loc.y = v10->y;
    this->m_b2_axis_loc.z = v10->z;
    v11 = phys_Unitize(&v20, b1_ref_loc);
    this->m_b1_ref_loc.x = v11->x;
    this->m_b1_ref_loc.y = v11->y;
    this->m_b1_ref_loc.z = v11->z;
    this->m_flags |= 4u;
    v12 = construct_orth_ud(&v20, &this->m_b1_axis_loc);
    this->m_b1_a1_loc.x = v12->x;
    this->m_b1_a1_loc.y = v12->y;
    this->m_b1_a1_loc.z = v12->z;
    v20.x = this->m_b1_axis_loc.y * this->m_b1_a1_loc.z - this->m_b1_axis_loc.z * this->m_b1_a1_loc.y;
    v20.y = this->m_b1_axis_loc.z * this->m_b1_a1_loc.x - this->m_b1_axis_loc.x * this->m_b1_a1_loc.z;
    v20.z = this->m_b1_axis_loc.x * this->m_b1_a1_loc.y - this->m_b1_axis_loc.y * this->m_b1_a1_loc.x;
    this->m_b1_a2_loc.x = v20.x;
    this->m_b1_a2_loc.y = v20.y;
    this->m_b1_a2_loc.z = v20.z;
    make_rotate(&v17, &this->m_b2_axis_loc, theta_min, 1000.0);
    v13 = phys_multiply(&v20, &v17, b2_ref_loc);
    v14 = phys_Unitize(&v19, v13);
    this->m_b2_ref_min_loc.x = v14->x;
    this->m_b2_ref_min_loc.y = v14->y;
    this->m_b2_ref_min_loc.z = v14->z;
    make_rotate(&v17, &this->m_b2_axis_loc, theta_max, 1000.0);
    v15 = phys_multiply(&v19, &v17, b2_ref_loc);
    v16 = phys_Unitize(&rv_52, v15);
    this->m_b2_ref_max_loc.x = v16->x;
    this->m_b2_ref_max_loc.y = v16->y;
    this->m_b2_ref_max_loc.z = v16->z;
}

void rigid_body_constraint_ragdoll::set_swivel(
        const phys_vec3 *b1_axis_loc,
        const phys_vec3 *b2_axis_loc,
        const phys_vec3 *b1_ref_loc,
        const phys_vec3 *b2_ref_loc,
        float theta_min,
        float theta_max)
{
    phys_vec3 *v9; // eax
    phys_vec3 *v10; // eax
    phys_vec3 *v11; // eax
    const phys_vec3 *v12; // eax
    phys_vec3 *v13; // eax
    const phys_vec3 *v14; // eax
    phys_vec3 *v15; // eax
    phys_mat44 v16; // [esp+18h] [ebp-7Ch] BYREF
    phys_vec3 rv_52; // [esp+58h] [ebp-3Ch] BYREF
    phys_vec3 v18; // [esp+68h] [ebp-2Ch] BYREF
    phys_vec3 v19; // [esp+78h] [ebp-1Ch] BYREF
    //_DWORD v20[3]; // [esp+88h] [ebp-Ch] BYREF
    //_UNKNOWN *retaddr; // [esp+94h] [ebp+0h]
    //
    //v20[0] = a2;
    //v20[1] = retaddr;
    v9 = phys_Unitize(&v19, b1_axis_loc);
    this->m_b1_axis_loc.x = v9->x;
    this->m_b1_axis_loc.y = v9->y;
    this->m_b1_axis_loc.z = v9->z;
    v10 = phys_Unitize(&v19, b2_axis_loc);
    this->m_b2_axis_loc.x = v10->x;
    this->m_b2_axis_loc.y = v10->y;
    this->m_b2_axis_loc.z = v10->z;
    v11 = phys_Unitize(&v19, b1_ref_loc);
    this->m_b1_ref_loc.x = v11->x;
    this->m_b1_ref_loc.y = v11->y;
    this->m_b1_ref_loc.z = v11->z;
    this->m_flags |= 8u;
    make_rotate(&v16, &this->m_b2_axis_loc, theta_min, 1000.0);
    v12 = phys_multiply(&v19, &v16, b2_ref_loc);
    v13 = phys_Unitize(&v18, v12);
    this->m_b2_ref_min_loc.x = v13->x;
    this->m_b2_ref_min_loc.y = v13->y;
    this->m_b2_ref_min_loc.z = v13->z;
    make_rotate(&v16, &this->m_b2_axis_loc, theta_max, 1000.0);
    v14 = phys_multiply(&v18, &v16, b2_ref_loc);
    v15 = phys_Unitize(&rv_52, v14);
    this->m_b2_ref_max_loc.x = v15->x;
    this->m_b2_ref_max_loc.y = v15->y;
    this->m_b2_ref_max_loc.z = v15->z;
}

void __thiscall rigid_body_constraint_ragdoll::add_joint_limit(
        const phys_vec3 *b1_ud_loc,
        float theta_limit)
{
  int savedregs; // [esp+8h] [ebp+0h] BYREF

  if ( this->m_joint_limits_count >= 2
    && _tlAssert("source/rbc_def_ragdoll.cpp", 102, "m_joint_limits_count < MAX_JOINT_LIMITS", "") )
  {
    __debugbreak();
  }
  //ragdoll_joint_limit_info::set(
    this->m_joint_limits[this->m_joint_limits_count].set(
    b1_ud_loc,
    theta_limit);
  ++this->m_joint_limits_count;
}

float rigid_body_constraint_ragdoll::pull_together()
{
    rigid_body *b2; // edi
    const phys_vec3 *v4; // eax
    rigid_body *b1; // ecx
    const phys_vec3 *v6; // eax
    phys_vec3 v8; // [esp-2Ch] [ebp-4Ch] BYREF
    phys_vec3 v9; // [esp-1Ch] [ebp-3Ch] BYREF
    float v10; // [esp-Ch] [ebp-2Ch]
    float v11; // [esp-8h] [ebp-28h]
    float v12; // [esp-4h] [ebp-24h]
    float *v13; // [esp+10h] [ebp-10h]
    //_UNKNOWN *v14[2]; // [esp+14h] [ebp-Ch] BYREF
    //int vars0; // [esp+20h] [ebp+0h]
    //
    //v14[0] = a2;
    //v14[1] = (_UNKNOWN *)vars0;
    b2 = this->b2;
    //*(float *)&v4 = COERCE_FLOAT();
    b1 = this->b1;
    //v13 = (float *)v4;
    v13 = (float*)phys_full_multiply(&v9, &b2->m_mat, &this->m_b2_r_loc);
    v6 = phys_full_multiply(&v8, &b1->m_mat, &this->m_b1_r_loc);
    v10 = v6->x - *v13;
    v11 = v6->y - v13[1];
    v12 = v6->z - v13[2];
    b2->m_mat.w.x = b2->m_mat.w.x + v10;
    b2->m_mat.w.y = b2->m_mat.w.y + v11;
    b2->m_mat.w.z = b2->m_mat.w.z + v12;
    //(float *)&v13 = fabs(b2->m_mat.w.x);
    float absval = fabs(b2->m_mat.w.x);
    if (absval > 100000.0
        || (absval = fabs(b2->m_mat.w.y), absval > 100000.0)
        || (absval = fabs(b2->m_mat.w.z), absval > 100000.0))
    {
        phys_exec_debug_callback(b2);
    }
    //*(float *)&v13 = v12 * v12 + v10 * v10 + v11 * v11;
    //return *(float *)&v13;

    return v12 * v12 + v10 * v10 + v11 * v11;
}

void rigid_body_constraint_ragdoll::setup_hinge(
        pulse_sum_constraint_solver *psys,
        const phys_vec3 *b1_ref,
        const phys_vec3 *b2_axis,
        float delta_t)
{
    rigid_body *b2; // eax
    rigid_body *b1; // edx
    rigid_body *v8; // ecx
    pulse_sum_angular *psa; // eax
    pulse_sum_angular *v10; // eax
    phys_vec3 b2_ref_max; // [esp+18h] [ebp-4Ch] BYREF
    phys_vec3 b2_ref_min; // [esp+28h] [ebp-3Ch] BYREF
    phys_vec3 v13; // [esp+38h] [ebp-2Ch] BYREF
    float v14; // [esp+50h] [ebp-14h]
    rigid_body_constraint_ragdoll *v15; // [esp+54h] [ebp-10h]
    //_UNKNOWN *v16; // [esp+58h] [ebp-Ch]
    //pulse_sum_constraint_solver *psysa; // [esp+5Ch] [ebp-8h]
    //const phys_vec3 *b2_axisa; // [esp+64h] [ebp+0h]
    //
    //v16 = a2;
    //psysa = (pulse_sum_constraint_solver *)b2_axisa;
    b2 = this->b2;
    v15 = this;
    phys_multiply(&b2_ref_min, &b2->m_mat, &this->m_b2_ref_min_loc);
    v13.x = b1_ref->y * b2_ref_min.z - b1_ref->z * b2_ref_min.y;
    v13.y = b1_ref->z * b2_ref_min.x - b2_ref_min.z * b1_ref->x;
    v13.z = b2_ref_min.y * b1_ref->x - b2_ref_min.x * b1_ref->y;
    v14 = b2_axis->y * v13.y + b2_axis->x * v13.x + b2_axis->z * v13.z;
    if (v14 >= -0.043618999)
    {
        v13.x = -b2_axis->x;
        b1 = v15->b1;
        v13.y = -b2_axis->y;
        v8 = v15->b2;
        v13.z = -b2_axis->z;
        psa = //pulse_sum_constraint_solver::create_pulse_sum_angular(
            psys->create_pulse_sum_angular(
            b1,
            b1_ref,
            v8,
            &b2_ref_min,
            &v13,
            &v15->m_ps_cache_list[6]);
        psa->m_pulse_sum_min = -10000000.0;
        psa->m_pulse_sum_max = 0.0;
        //pulse_sum_angular::setup_vel_uni_standard(psa, delta_t, 5.0);
        psa->setup_vel_uni_standard(delta_t, 5.0);
    }
    phys_multiply(&b2_ref_max, &v15->b2->m_mat, &v15->m_b2_ref_max_loc);
    v13.x = b1_ref->y * b2_ref_max.z - b1_ref->z * b2_ref_max.y;
    v13.y = b1_ref->z * b2_ref_max.x - b2_ref_max.z * b1_ref->x;
    v13.z = b2_ref_max.y * b1_ref->x - b2_ref_max.x * b1_ref->y;
    v14 = b2_axis->x * v13.x + b2_axis->y * v13.y + b2_axis->z * v13.z;
    if (v14 <= 0.043618999)
    {
        v10 = //pulse_sum_constraint_solver::create_pulse_sum_angular(
            psys->create_pulse_sum_angular(
            v15->b1,
            b1_ref,
            v15->b2,
            &b2_ref_max,
            b2_axis,
            &v15->m_ps_cache_list[7]);
        v10->m_pulse_sum_min = -10000000.0;
        v10->m_pulse_sum_max = 0.0;
        //pulse_sum_angular::setup_vel_uni_standard(v10, delta_t, 5.0);
        v10->setup_vel_uni_standard(delta_t, 5.0);
    }
}

void rigid_body_constraint_ragdoll::setup_constraint(pulse_sum_constraint_solver *psys, float delta_t)
{
    rigid_body *b2; // edi
    rigid_body *b1; // edi
    rigid_body *v7; // eax
    double z; // st7
    pulse_sum_angular *v9; // eax
    double v10; // st7
    double v11; // st7
    rigid_body *v12; // edi
    unsigned int m_flags; // eax
    bool v14; // cc
    float *p_m_b1_ud_limit_si; // edi
    double v16; // st2
    double v17; // st7
    pulse_sum_angular *v18; // eax
    const phys_vec3 *v19; // eax
    phys_mat44 rv; // [esp+20h] [ebp-FCh] BYREF
    phys_vec3 b1_axis; // [esp+60h] [ebp-BCh] BYREF
    float v22; // [esp+70h] [ebp-ACh]
    float v23; // [esp+74h] [ebp-A8h]
    float v24; // [esp+78h] [ebp-A4h]
    float v25; // [esp+80h] [ebp-9Ch]
    float v26; // [esp+84h] [ebp-98h]
    float v27; // [esp+88h] [ebp-94h]
    float v28; // [esp+90h] [ebp-8Ch]
    float v29; // [esp+94h] [ebp-88h]
    float v30; // [esp+98h] [ebp-84h]
    phys_vec3 b1_r_; // [esp+A0h] [ebp-7Ch] BYREF
    phys_vec3 b1_ud; // [esp+B0h] [ebp-6Ch] BYREF
    phys_vec3 v33; // [esp+C0h] [ebp-5Ch] BYREF
    phys_vec3 b2_axis; // [esp+D0h] [ebp-4Ch] BYREF
    phys_vec3 axis; // [esp+E0h] [ebp-3Ch] BYREF
    phys_vec3 b1_r_length; // [esp+F0h] [ebp-2Ch] BYREF
    int i; // [esp+104h] [ebp-18h]
    pulse_sum_cache *v38; // [esp+108h] [ebp-14h]
    float nb1_r_length; // [esp+10Ch] [ebp-10h]
    //_UNKNOWN *v40[2]; // [esp+110h] [ebp-Ch] BYREF
    //int vars0; // [esp+11Ch] [ebp+0h]
    //
    //v40[0] = a2;
    //v40[1] = (_UNKNOWN *)vars0;
    b2 = this->b2;
    phys_multiply(&axis, &b2->m_mat, &this->m_b2_r_loc);
    phys_multiply(&v33, &this->b1->m_mat, &this->m_b1_r_loc);
    //pulse_sum_constraint_solver::create_point(
        psys->create_point(
        this->b1,
        &v33,
        b2,
        &axis,
        this->m_ps_cache_list,
        delta_t,
        0,
        0.0,
        0.0);
    b1 = this->b1;
    phys_multiply(&b1_axis, &this->b1->m_mat, &this->m_b1_axis_loc);
    phys_multiply(&b2_axis, &this->b2->m_mat, &this->m_b2_axis_loc);
    if ((this->m_flags & 0x40) != 0)
    {
        v7 = this->b2;
        *(float *)&v38 = v7->m_a_vel.y - b1->m_a_vel.y;
        *(float *)&i = v7->m_a_vel.z - b1->m_a_vel.z;
        b1_r_length.x = v7->m_a_vel.x - b1->m_a_vel.x;
        nb1_r_length = *(float *)&v38 * *(float *)&v38 + b1_r_length.x * b1_r_length.x + *(float *)&i * *(float *)&i;
        nb1_r_length = sqrt(nb1_r_length);
        if (nb1_r_length <= 0.00009999999747378752)
        {
            b1_r_length.x = b2_axis.x;
            b1_r_length.y = b2_axis.y;
            z = b2_axis.z;
        }
        else
        {
            nb1_r_length = 1.0 / nb1_r_length;
            b1_r_length.x = nb1_r_length * b1_r_length.x;
            b1_r_length.y = nb1_r_length * *(float *)&v38;
            z = nb1_r_length * *(float *)&i;
        }
        b1_r_length.z = z;
        v9 = //pulse_sum_constraint_solver::create_pulse_sum_angular(
            psys->create_pulse_sum_angular(
            b1,
            &PHYS_ZERO_VEC,
            this->b2,
            &PHYS_ZERO_VEC,
            &b1_r_length,
            &this->m_ps_cache_list[3]);
        if ((this->m_flags & 0x200) != 0)
        {
            nb1_r_length = 1.0 / (this->m_damp_k * delta_t);
            v9->m_right_side = 0.0;
            v9->m_big_dirt = 0.0;
            v10 = nb1_r_length;
            v9->m_cfm = nb1_r_length;
            v9->m_denom = v10 + v9->m_denom;
            v9->m_pulse_sum_min = -10000000.0;
            v11 = 10000000.0;
        }
        else
        {
            v9->m_right_side = 0.0;
            v9->m_big_dirt = 0.0;
            v9->m_cfm = 0.0;
            nb1_r_length = this->m_damp_k * delta_t;
            v11 = nb1_r_length;
            v9->m_pulse_sum_min = -nb1_r_length;
        }
        v9->m_pulse_sum_max = v11;
    }
    if ((this->m_flags & 4) != 0)
    {
        v12 = this->b1;
        phys_multiply(&axis, &this->b1->m_mat, &this->m_b1_a2_loc);
        phys_multiply(&v33, &v12->m_mat, &this->m_b1_a1_loc);
        //pulse_sum_constraint_solver::create_hinge(
            psys->create_hinge(
            v12,
            &b1_axis,
            this->b2,
            &b2_axis,
            &v33,
            &axis,
            &this->m_ps_cache_list[4],
            delta_t);
        phys_multiply(&axis, &this->b1->m_mat, &this->m_b1_ref_loc);
        rigid_body_constraint_ragdoll::setup_hinge(psys, &axis, &b2_axis, delta_t);
    }
    m_flags = this->m_flags;
    if ((m_flags & 8) != 0)
    {
        if ((m_flags & 0x100) == 0)
        {
            v14 = this->m_joint_limits_count <= 0;
            *(float *)&i = 0.0;
            if (!v14)
            {
                v38 = &this->m_ps_cache_list[8];
                p_m_b1_ud_limit_si = &this->m_joint_limits[0].m_b1_ud_limit_si_;
                do
                {
                    phys_multiply(&b1_ud, &this->b1->m_mat, (const phys_vec3 *)(p_m_b1_ud_limit_si - 5));
                    nb1_r_length = b1_ud.z * b2_axis.z + b1_ud.x * b2_axis.x + b1_ud.y * b2_axis.y;
                    v16 = nb1_r_length;
                    if (p_m_b1_ud_limit_si[1] >= (double)nb1_r_length)
                    {
                        v22 = b1_ud.x * v16;
                        v23 = b1_ud.y * v16;
                        v24 = v16 * b1_ud.z;
                        b1_r_length.x = b2_axis.x - v22;
                        b1_r_length.y = b2_axis.y - v23;
                        b1_r_length.z = b2_axis.z - v24;
                        nb1_r_length = b1_r_length.y * b1_r_length.y + b1_r_length.x * b1_r_length.x + b1_r_length.z * b1_r_length.z;
                        nb1_r_length = sqrt(nb1_r_length);
                        v17 = nb1_r_length;
                        if (nb1_r_length >= 0.00009999999747378752)
                        {
                            nb1_r_length = *(p_m_b1_ud_limit_si - 1);
                            v25 = nb1_r_length * b1_ud.x;
                            v26 = b1_ud.y * nb1_r_length;
                            v27 = nb1_r_length * b1_ud.z;
                            nb1_r_length = *p_m_b1_ud_limit_si / v17;
                            v28 = b1_r_length.x * nb1_r_length;
                            v29 = b1_r_length.y * nb1_r_length;
                            v30 = nb1_r_length * b1_r_length.z;
                            b1_r_.x = v28 + v25;
                            b1_r_.y = v29 + v26;
                            b1_r_.z = v30 + v27;
                            v33.x = b1_r_.y * b1_ud.z - b1_r_.z * b1_ud.y;
                            v33.y = b1_r_.z * b1_ud.x - b1_ud.z * b1_r_.x;
                            v33.z = b1_r_.x * b1_ud.y - b1_r_.y * b1_ud.x;
                            nb1_r_length = 1.0 / *p_m_b1_ud_limit_si;
                            axis.x = v33.x * nb1_r_length;
                            axis.y = v33.y * nb1_r_length;
                            axis.z = nb1_r_length * v33.z;
                            PHYS_ASSERT_UNIT(&axis);
                            v18 = //pulse_sum_constraint_solver::create_pulse_sum_angular(
                                psys->create_pulse_sum_angular(
                                this->b1,
                                &b1_r_,
                                this->b2,
                                &b2_axis,
                                &axis,
                                v38);
                            v18->m_pulse_sum_min = -10000000.0;
                            v18->m_pulse_sum_max = 0.0;
                            //pulse_sum_angular::setup_vel_uni_standard(v18, delta_t, 5.0);
                            v18->setup_vel_uni_standard(delta_t, 5.0);
                        }
                    }
                    ++v38;
                    p_m_b1_ud_limit_si += 8;
                    v14 = ++i < this->m_joint_limits_count;
                } while (v14);
            }
        }
        make_rotate(&rv, &b1_axis, &b2_axis);
        phys_multiply(&axis, &this->b1->m_mat, &this->m_b1_ref_loc);
        v19 = phys_multiply(&v33, &rv, &axis);
        rigid_body_constraint_ragdoll::setup_hinge(psys, v19, &b2_axis, delta_t);
    }
}

