#include "rbc_def_vehicle.h"
#include <physics/phys_constraint_solver_multithreaded.h>
#include "rbc_def_generic.h"
#include <physics/phys_assert.h>

void __thiscall pulse_sum_normal::set_pulse_sum_limits_parent_ratio(float limit_ratio, pulse_sum_normal *const parent)
{
  if ( limit_ratio < 0.0
    && _tlAssert(
         "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\constraint_solver\\pulse_sum_normal.h",
         269,
         "limit_ratio >= 0.0f",
         "") )
  {
    __debugbreak();
  }
  if ( !parent
    && _tlAssert(
         "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\constraint_solver\\pulse_sum_normal.h",
         270,
         "parent",
         "") )
  {
    __debugbreak();
  }
  this->m_flags |= 1u;
  this->m_pulse_limit_ratio = limit_ratio;
  this->m_pulse_parent = parent;
  this->m_pulse_sum_min = 0.0;
  this->m_pulse_sum_max = 0.0;
}

void __thiscall pulse_sum_wheel::set_side_fwd_ratios(
        float side_ratio,
        float fwd_ratio,
        float side_fric_max)
{
  if ( !this->m_side || !this->m_fwd )
  {
    if ( _tlAssert(
           "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\constraint_solver\\pulse_sum_wheel.h",
           34,
           "m_side && m_fwd",
           "") )
    {
      __debugbreak();
    }
  }
  this->m_side->m_pulse_limit_ratio = side_ratio;
  this->m_fwd->m_pulse_limit_ratio = fwd_ratio;
  this->m_side_fric_max = side_fric_max;
}

void __thiscall rigid_body_constraint_wheel::set_wheel_state_accelerating(
        float desired_speed_k,
        float acceleration_factor_k)
{
  this->m_wheel_state = 0;
  this->m_desired_speed_k = desired_speed_k;
  this->m_acceleration_factor_k = acceleration_factor_k;
}

void __thiscall rigid_body_constraint_wheel::get_wheel_state_accelerating(
        float *desired_speed_k,
        float *acceleration_factor_k)
{
  *desired_speed_k = this->m_desired_speed_k;
  *acceleration_factor_k = this->m_acceleration_factor_k;
}

void __thiscall rigid_body_constraint_wheel::set_wheel_state_braking(
        float braking_factor_k)
{
  this->m_wheel_state = 1;
  this->m_braking_factor_k = braking_factor_k;
}

void __thiscall rigid_body_constraint_wheel::set_no_collision()
{
  this->m_wheel_flags &= ~1u;
  this->b2 = 0;
}

void __thiscall rigid_body_constraint_wheel::set_collision(
        rigid_body *const rb,
        const phys_vec3 *hitp_loc,
        const phys_vec3 *hitn_loc)
{
  this->m_wheel_flags |= 1u;
  this->b2 = rb;
  this->m_b2_hitp_loc.x = hitp_loc->x;
  this->m_b2_hitp_loc.y = hitp_loc->y;
  this->m_b2_hitp_loc.z = hitp_loc->z;
  this->m_b2_hitn_loc.x = hitn_loc->x;
  this->m_b2_hitn_loc.y = hitn_loc->y;
  this->m_b2_hitn_loc.z = hitn_loc->z;
}

double __cdecl lerp_float(const float tgt, float cur, float rate, float delta_t)
{
  double result; // st7
  double v5; // st6
  long double v6; // st5
  double v7; // st4
  double v8; // st4
  float stepa; // [esp+8h] [ebp+8h]
  float step; // [esp+8h] [ebp+8h]
  float stepb; // [esp+8h] [ebp+8h]
  float cura; // [esp+Ch] [ebp+Ch]

  result = tgt;
  v5 = cur;
  stepa = tgt - cur;
  v6 = stepa;
  if ( stepa <= 0.0 )
    v7 = -rate;
  else
    v7 = rate;
  step = v7 * delta_t;
  cura = fabs(v6);
  if ( cura > 0.05000000074505806 )
  {
    v8 = step;
    stepb = fabs(step);
    if ( stepb <= (double)cura )
      return (float)(v8 + v5);
  }
  return result;
}

void __thiscall rigid_body_constraint_wheel::set(
        const phys_vec3 *wheel_center_loc,
        const phys_vec3 *suspension_dir_loc,
        const phys_vec3 *wheel_axis_loc,
        float wheel_radius,
        float fwd_fric_k,
        float side_fric_k,
        float suspension_stiffness_k,
        float suspension_damp_k,
        float hard_limit_dist,
        float roll_stability_factor,
        float pitch_stability_factor,
        float side_fric_max)
{
  phys_vec3 *p_m_b1_suspension_dir_loc; // esi
  phys_vec3 *p_m_b1_wheel_axis_loc; // edi

  this->m_b1_wheel_center_loc.x = wheel_center_loc->x;
  p_m_b1_suspension_dir_loc = &this->m_b1_suspension_dir_loc;
  this->m_b1_wheel_center_loc.y = wheel_center_loc->y;
  p_m_b1_wheel_axis_loc = &this->m_b1_wheel_axis_loc;
  this->m_b1_wheel_center_loc.z = wheel_center_loc->z;
  this->m_b1_suspension_dir_loc.x = suspension_dir_loc->x;
  this->m_b1_suspension_dir_loc.y = suspension_dir_loc->y;
  this->m_b1_suspension_dir_loc.z = suspension_dir_loc->z;
  this->m_b1_wheel_axis_loc.x = wheel_axis_loc->x;
  this->m_b1_wheel_axis_loc.y = wheel_axis_loc->y;
  this->m_b1_wheel_axis_loc.z = wheel_axis_loc->z;
  this->m_wheel_state = 1;
  this->m_wheel_flags = 0;
  this->m_wheel_radius = wheel_radius;
  this->m_fwd_fric_k = fwd_fric_k;
  this->m_side_fric_k = side_fric_k;
  this->m_side_fric_max = side_fric_max;
  this->m_suspension_stiffness_k = suspension_stiffness_k;
  this->m_suspension_damp_k = suspension_damp_k;
  this->m_hard_limit_dist = hard_limit_dist;
  this->m_roll_stability_factor = roll_stability_factor;
  this->m_pitch_stability_factor = pitch_stability_factor;
  this->m_braking_factor_k = 0.0;
  this->m_turning_radius_ratio_max_speed = 1.0;
  this->m_turning_radius_ratio_accel = 1.0;
  this->m_wheel_vel = 0.0;
  this->m_wheel_pos = 0.0;
  this->m_wheel_fwd = 0.0;
  this->m_wheel_displaced_center_dist = 0.0;
  PHYS_ASSERT_UNIT(&this->m_b1_suspension_dir_loc);
  PHYS_ASSERT_UNIT(p_m_b1_wheel_axis_loc);
  PHYS_ASSERT_ORTHOGONAL(p_m_b1_suspension_dir_loc, p_m_b1_wheel_axis_loc);
}

void rigid_body_constraint_wheel::get_wheel_collide_segment(
        const phys_mat44 *b1_mat,
        phys_vec3 *const p0,
        phys_vec3 *const p1)
{
    const phys_vec3 *v6; // eax
    double x; // st7
    double v8; // st6
    double y; // st5
    double v10; // st4
    double z; // st3
    double v12; // st2
    phys_vec3 v13; // [esp-Ch] [ebp-4Ch] BYREF
    phys_vec3 wheel_center_4; // [esp+4h] [ebp-3Ch] BYREF
    phys_vec3 seg_dir; // [esp+14h] [ebp-2Ch]
    float m_wheel_radius; // [esp+30h] [ebp-10h]
    //_UNKNOWN *v17[2]; // [esp+34h] [ebp-Ch] BYREF
    //phys_vec3 *p1a; // [esp+40h] [ebp+0h]
    //
    //v17[0] = a2;
    //v17[1] = p1a;
    if (((unsigned __int8)p0 & 0xF) != 0
        && _tlAssert(
            "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\phys_math.h",
            444,
            "uint(v) % PHYS_ALIGNOF(phys_vec3) == 0",
            ""))
    {
        __debugbreak();
    }
    if (((unsigned __int8)p1 & 0xF) != 0
        && _tlAssert(
            "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\phys_math.h",
            444,
            "uint(v) % PHYS_ALIGNOF(phys_vec3) == 0",
            ""))
    {
        __debugbreak();
    }
    phys_full_multiply(&v13, b1_mat, &this->m_b1_wheel_center_loc);
    v6 = phys_multiply(&wheel_center_4, b1_mat, &this->m_b1_suspension_dir_loc);
    m_wheel_radius = this->m_wheel_radius;
    seg_dir.x = v6->x * m_wheel_radius;
    seg_dir.y = v6->y * m_wheel_radius;
    seg_dir.z = m_wheel_radius * v6->z;
    x = v13.x;
    v8 = seg_dir.x;
    wheel_center_4.x = v13.x - seg_dir.x;
    y = v13.y;
    v10 = seg_dir.y;
    wheel_center_4.y = v13.y - seg_dir.y;
    z = v13.z;
    v12 = seg_dir.z;
    wheel_center_4.z = v13.z - seg_dir.z;
    p0->x = wheel_center_4.x;
    p0->y = wheel_center_4.y;
    p0->z = wheel_center_4.z;
    wheel_center_4.x = v8 + x;
    wheel_center_4.y = y + v10;
    wheel_center_4.z = v12 + z;
    p1->x = wheel_center_4.x;
    p1->y = wheel_center_4.y;
    p1->z = wheel_center_4.z;
}

float velocity_clamp = 0.5f;
float lr = 1000.0f;
float lr_0 = 1.0f;

void rigid_body_constraint_wheel::epilog_vel_constraint(float delta_t)
{
    unsigned int m_wheel_flags; // edx
    rigid_body *b1; // edi
    rigid_body *b2; // eax
    rigid_body *v7; // eax
    double m_hard_limit_dist; // st7
    double v9; // st7
    double v10; // st7
    double m_wheel_displaced_center_dist; // st6
    double m_pulse_sum; // st6
    pulse_sum_normal *m_ps_side_fric; // eax
    pulse_sum_normal *m_ps_fwd_fric; // ecx
    double v15; // st6
    float v16; // [esp+Ch] [ebp-70h]
    phys_vec3 v17; // [esp+20h] [ebp-5Ch] BYREF
    phys_vec3 b1_r; // [esp+30h] [ebp-4Ch]
    phys_vec3 b2_r; // [esp+40h] [ebp-3Ch] BYREF
    phys_vec3 b1_suspension_dir; // [esp+50h] [ebp-2Ch] BYREF
    float prev_wheel_displaced_center_dist; // [esp+60h] [ebp-1Ch]
    unsigned int v22; // [esp+64h] [ebp-18h]
    float lerp_rate; // [esp+68h] [ebp-14h]
    bool apply_wheel_velocity; // [esp+6Fh] [ebp-Dh]
    //_UNKNOWN *v25; // [esp+70h] [ebp-Ch]
    //float delta_ta; // [esp+74h] [ebp-8h]
    //int vars0; // [esp+7Ch] [ebp+0h]
    //
    //v25 = a2;
    //LODWORD(delta_ta) = vars0;
    m_wheel_flags = this->m_wheel_flags;
    prev_wheel_displaced_center_dist = this->m_wheel_displaced_center_dist;
    v22 = m_wheel_flags;
    if ((m_wheel_flags & 1) != 0)
    {
        b1 = this->b1;
        phys_multiply(&b1_suspension_dir, &this->b1->m_mat, &this->m_b1_suspension_dir_loc);
        lerp_rate = this->m_wheel_radius;
        b2_r.x = b1_suspension_dir.x * lerp_rate;
        b2_r.y = b1_suspension_dir.y * lerp_rate;
        b2_r.z = lerp_rate * b1_suspension_dir.z;
        phys_multiply(&v17, &b1->m_mat, &this->m_b1_wheel_center_loc);
        b2 = this->b2;
        b1_r.x = v17.x + b2_r.x;
        b1_r.y = v17.y + b2_r.y;
        b1_r.z = v17.z + b2_r.z;
        phys_multiply(&b2_r, &b2->m_mat, &this->m_b2_hitp_loc);
        v7 = this->b2;
        v17.x = v7->m_mat.w.x + b2_r.x;
        v17.y = v7->m_mat.w.y + b2_r.y;
        v17.z = v7->m_mat.w.z + b2_r.z;
        b2_r.x = b1->m_mat.w.x + b1_r.x;
        b2_r.y = b1->m_mat.w.y + b1_r.y;
        b2_r.z = b1->m_mat.w.z + b1_r.z;
        b1_r.x = b2_r.x - v17.x;
        b1_r.y = b2_r.y - v17.y;
        b1_r.z = b2_r.z - v17.z;
        lerp_rate = b1_r.y * b1_suspension_dir.y + b1_r.x * b1_suspension_dir.x + b1_r.z * b1_suspension_dir.z;
        m_hard_limit_dist = lerp_rate;
        if (this->m_hard_limit_dist < (double)lerp_rate)
            m_hard_limit_dist = this->m_hard_limit_dist;
        m_wheel_flags = v22;
        lerp_rate = m_hard_limit_dist;
        v9 = lerp_rate;
    }
    else
    {
        v9 = 0.0;
    }
    this->m_wheel_displaced_center_dist = v9;
    if (this->m_wheel_displaced_center_dist >= (double)prev_wheel_displaced_center_dist)
    {
        v10 = delta_t;
    }
    else
    {
        v10 = delta_t;
        prev_wheel_displaced_center_dist = prev_wheel_displaced_center_dist - 51.0 * delta_t;
        m_wheel_displaced_center_dist = prev_wheel_displaced_center_dist;
        if (prev_wheel_displaced_center_dist < (double)this->m_wheel_displaced_center_dist)
            m_wheel_displaced_center_dist = this->m_wheel_displaced_center_dist;
        lerp_rate = m_wheel_displaced_center_dist;
        this->m_wheel_displaced_center_dist = lerp_rate;
    }
    if (this->m_ps_suspension)
        m_pulse_sum = this->m_ps_cache_list[1].m_pulse_sum;
    else
        m_pulse_sum = 0.0;
    this->m_wheel_normal_force = m_pulse_sum;
    prev_wheel_displaced_center_dist = this->m_wheel_vel * this->m_wheel_radius;
    prev_wheel_displaced_center_dist = fabs(prev_wheel_displaced_center_dist);
    apply_wheel_velocity = velocity_clamp < (double)prev_wheel_displaced_center_dist;
    m_ps_side_fric = this->m_ps_side_fric;
    if (m_ps_side_fric)
    {
        m_ps_fwd_fric = this->m_ps_fwd_fric;
        if (m_ps_fwd_fric)
        {
            if ((m_ps_fwd_fric->m_flags & 4) != 0)
                this->m_wheel_flags = m_wheel_flags | 4;
            //this->m_wheel_vel = pulse_sum_normal::get_unclamped_pulse_sum(m_ps_fwd_fric)
            this->m_wheel_vel = m_ps_fwd_fric->get_unclamped_pulse_sum()
                * this->m_wheel_fwd
                / this->m_wheel_radius
                + this->m_wheel_vel;
            v10 = delta_t;
        }
        else if ((m_ps_side_fric->m_flags & 2) != 0)
        {
            this->m_wheel_flags = m_wheel_flags | 4;
        }
    }
    if (apply_wheel_velocity)
        this->m_wheel_pos = this->m_wheel_vel * v10 + this->m_wheel_pos;
    if ((this->m_wheel_flags & 1) == 0)
    {
        if (this->m_braking_factor_k >= 50.0)
            v15 = lr;
        else
            v15 = lr_0;
        lerp_rate = v15;
        v16 = v10;
        this->m_wheel_vel = lerp_float(0.0, this->m_wheel_vel, lerp_rate, v16);
    }
}

// local variable allocation has failed, the output may be wrong!
void rigid_body_constraint_wheel::setup_constraint(pulse_sum_constraint_solver *psys, float delta_t)
{
    rigid_body *b1; // edi
    rigid_body *b2; // eax
    rigid_body *v7; // ecx
    double y; // st7
    double z; // st6
    double x; // st5
    double v11; // st4
    double v12; // st3
    double v13; // st2
    unsigned int v14; // eax
    pulse_sum_normal *psn; // eax
    rigid_body *v16; // ecx
    pulse_sum_normal *v17; // edi
    rigid_body *v18; // eax
    double v19; // st5
    double v20; // rt2
    double v21; // rtt
    pulse_sum_wheel *psw; // eax
    rigid_body *v23; // ecx
    pulse_sum_normal *m_ps_suspension; // eax
    pulse_sum_normal *v25; // edi
    double pos; // st7
    double v27; // st6
    pulse_sum_normal *v28; // eax
    pulse_sum_normal *m_ps_side_fric; // eax
    pulse_sum_normal *v30; // ecx
    const phys_vec3 *relative_velocity; // eax
    unsigned int m_wheel_state; // ecx
    pulse_sum_normal *v33; // eax
    pulse_sum_wheel *v34; // edx
    pulse_sum_normal *pulse_sum_wheel_fwd; // eax
    rigid_body *v36; // ecx
    rigid_body *v37; // ecx
    double v38; // st7
    double v39; // st7
    double v40; // st6
    double v41; // st7
    pulse_sum_normal *v42; // eax
    double v43; // st7
    double v44; // st5
    double v45; // st6
    double w; // st7
    pulse_sum_normal *v47; // eax
    double v48; // st6
    pulse_sum_normal *m_ps_fwd_fric; // eax
    pulse_sum_wheel *v50; // ecx
    const phys_vec3 *v51; // eax
    rigid_body *v52; // [esp-10h] [ebp-148h]
    rigid_body *v53; // [esp-8h] [ebp-140h]
    rigid_body *v54; // [esp-8h] [ebp-140h]
    rigid_body *v55; // [esp-8h] [ebp-140h]
    phys_mat44 rot_mat; // [esp+1Ch] [ebp-11Ch] BYREF
    phys_vec3 v57; // [esp+5Ch] [ebp-DCh] BYREF
    phys_vec3 fric_ud; // [esp+6Ch] [ebp-CCh] BYREF
    phys_vec3 b1_r_displace; // [esp+7Ch] [ebp-BCh] BYREF
    phys_vec3 b2_r; // [esp+8Ch] [ebp-ACh] BYREF
    phys_vec3 wheel_axis; // [esp+9Ch] [ebp-9Ch] BYREF
    phys_vec3 b1_suspension_dir; // [esp+ACh] [ebp-8Ch] BYREF
    phys_vec3 b1_r; // [esp+BCh] [ebp-7Ch] BYREF
    phys_vec3 hitn; // [esp+CCh] [ebp-6Ch] BYREF
    pulse_sum_wheel *ps_wheel; // [esp+E8h] [ebp-50h]
    phys_vec3 desired_speed; // [esp+ECh] [ebp-4Ch] BYREF
    float k_; // [esp+108h] [ebp-30h]
    phys_vec3 fwd; // [esp+10Ch] [ebp-2Ch] BYREF
    float pen_depth; // [esp+128h] [ebp-10h]
    //_UNKNOWN *v70[2]; // [esp+12Ch] [ebp-Ch] BYREF
    //int vars0; // [esp+138h] [ebp+0h]
    //
    //v70[0] = a2;
    //v70[1] = (_UNKNOWN *)vars0;
    this->m_wheel_flags &= ~4u;
    this->m_wheel_fwd = 0.0;
    this->m_ps_suspension = 0;
    this->m_ps_side_fric = 0;
    this->m_ps_fwd_fric = 0;
    ps_wheel = (pulse_sum_wheel *)this->m_wheel_flags;
    if (((unsigned __int8)ps_wheel & 1) != 0)
    {
        b1 = this->b1;
        phys_multiply(&b1_suspension_dir, &this->b1->m_mat, &this->m_b1_suspension_dir_loc);
        k_ = this->m_wheel_radius;
        fwd.x = b1_suspension_dir.x * k_;
        fwd.y = b1_suspension_dir.y * k_;
        fwd.z = k_ * b1_suspension_dir.z;
        phys_multiply(&desired_speed, &b1->m_mat, &this->m_b1_wheel_center_loc);
        b2 = this->b2;
        b1_r.x = desired_speed.x + fwd.x;
        //LODWORD(pen_depth) = &b2->m_mat;
        b1_r.y = desired_speed.y + fwd.y;
        b1_r.z = desired_speed.z + fwd.z;
        phys_multiply(&b2_r, &b2->m_mat, &this->m_b2_hitp_loc);
        v7 = this->b2;
        hitn.x = v7->m_mat.w.x + b2_r.x;
        y = b2_r.y;
        hitn.y = v7->m_mat.w.y + b2_r.y;
        z = b2_r.z;
        hitn.z = v7->m_mat.w.z + b2_r.z;
        x = b1_r.x;
        fwd.x = b1->m_mat.w.x + b1_r.x;
        v11 = b1_r.y;
        fwd.y = b1->m_mat.w.y + b1_r.y;
        v12 = b1_r.z;
        fwd.z = b1->m_mat.w.z + b1_r.z;
        desired_speed.x = fwd.x - hitn.x;
        desired_speed.y = fwd.y - hitn.y;
        desired_speed.z = fwd.z - hitn.z;
        k_ = desired_speed.x * b1_suspension_dir.x
            + desired_speed.y * b1_suspension_dir.y
            + desired_speed.z * b1_suspension_dir.z;
        v13 = k_;
        k_ = this->m_hard_limit_dist - 3.400000095367432;
        if (k_ > v13)
            v14 = (unsigned int)ps_wheel & 0xFFFFFFFD;
        else
            v14 = (unsigned int)ps_wheel | 2;
        this->m_wheel_flags = v14;
        hitn.x = v7->m_mat.w.x + b2_r.x;
        hitn.y = y + v7->m_mat.w.y;
        hitn.z = z + v7->m_mat.w.z;
        fwd.x = x + b1->m_mat.w.x;
        fwd.y = v11 + b1->m_mat.w.y;
        fwd.z = v12 + b1->m_mat.w.z;
        desired_speed.x = fwd.x - hitn.x;
        desired_speed.y = fwd.y - hitn.y;
        desired_speed.z = fwd.z - hitn.z;
        if ((v14 & 2) != 0)
        {
            //phys_multiply(&fwd, (const phys_mat44 *)LODWORD(pen_depth), &this->m_b2_hitn_loc);
            phys_multiply(&fwd, &this->b2->m_mat, &this->m_b2_hitn_loc); // unsure if this is 100% right
            v57.x = -fwd.x;
            v57.y = -fwd.y;
            v57.z = -fwd.z;
            //psn = pulse_sum_constraint_solver::create_pulse_sum_normal(psys);
            psn = psys->create_pulse_sum_normal();
            pen_depth = this->m_hard_limit_dist;
            v16 = this->b2;
            v17 = psn;
            fwd.x = pen_depth * b1_suspension_dir.x;
            v18 = this->b1;
            fwd.y = b1_suspension_dir.y * pen_depth;
            fwd.z = pen_depth * b1_suspension_dir.z;
            wheel_axis.x = b1_r.x - fwd.x;
            wheel_axis.y = b1_r.y - fwd.y;
            wheel_axis.z = b1_r.z - fwd.z;
            //pulse_sum_normal::set(v17, (int)v70, v18, &wheel_axis, v16, &b2_r, &v57, this->m_ps_cache_list, &PHYS_ZERO_VEC_76);
            v17->set(v18, &wheel_axis, v16, &b2_r, &v57, this->m_ps_cache_list, &PHYS_ZERO_VEC);
            v17->m_pulse_sum_min = -10000000.0;
            v17->m_pulse_sum_max = 0.0;
            //pulse_sum_normal::setup_vel_uni_standard(v17, delta_t, 170.0);
            v17->setup_vel_uni_standard(delta_t, 170.0);
        }
        phys_multiply(&wheel_axis, &this->b2->m_mat, &this->m_b2_hitn_loc);
        hitn.x = -wheel_axis.x;
        hitn.y = -wheel_axis.y;
        hitn.z = -wheel_axis.z;
        pen_depth = hitn.y * desired_speed.y + hitn.x * desired_speed.x + hitn.z * desired_speed.z;
        v19 = pen_depth;
        pen_depth = hitn.x * pen_depth;
        fwd.x = pen_depth;
        v20 = pen_depth;
        pen_depth = hitn.y * v19;
        fwd.y = pen_depth;
        v21 = pen_depth;
        pen_depth = v19 * hitn.z;
        b1_r.x = b1_r.x - fwd.x;
        b1_r.y = b1_r.y - fwd.y;
        b1_r.z = b1_r.z - pen_depth;
        fwd.x = v20;
        fwd.y = v21;
        fwd.z = pen_depth;
        b2_r.x = b2_r.x - fwd.x;
        b2_r.y = b2_r.y - fwd.y;
        b2_r.z = b2_r.z - pen_depth;
        pen_depth = -this->m_roll_stability_factor;
        b1_r_displace.x = pen_depth * b1_suspension_dir.x;
        b1_r_displace.y = b1_suspension_dir.y * pen_depth;
        b1_r_displace.z = pen_depth * b1_suspension_dir.z;
        //psw = pulse_sum_constraint_solver::create_pulse_sum_wheel(psys);
        psw = psys->create_pulse_sum_wheel();
        psw->m_side = 0;
        psw->m_fwd = 0;
        v53 = this->b2;
        v23 = this->b1;
        ps_wheel = psw;
        this->m_ps_suspension = &psw->m_suspension;
        //pulse_sum_normal::set(
            psw->m_suspension.set(
            v23,
            &b1_r,
            v53,
            &b2_r,
            &hitn,
            &this->m_ps_cache_list[1],
            &b1_r_displace);
        m_ps_suspension = this->m_ps_suspension;
        m_ps_suspension->m_pulse_sum_min = -10000000.0;
        m_ps_suspension->m_pulse_sum_max = 0.0;
        v25 = this->m_ps_suspension;
        pen_depth = this->m_suspension_stiffness_k * delta_t;
        *(double *)&desired_speed.z = pen_depth * delta_t;
        pen_depth = delta_t * this->m_suspension_damp_k;
        pen_depth = 1.0 / (*(double *)&desired_speed.z + pen_depth);
        //pos = pulse_sum_normal::get_pos(v25, (int)v70);
        pos = v25->get_pos();
        v27 = pen_depth;
        pen_depth = *(double *)&desired_speed.z * pen_depth;
        v25->m_right_side = pos * (-pen_depth / delta_t);
        v25->m_big_dirt = 0.0;
        v25->m_cfm = v27;
        v25->m_denom = v27 + v25->m_denom;
        phys_multiply(&wheel_axis, &this->b1->m_mat, &this->m_b1_wheel_axis_loc);
        make_rotate(&rot_mat, &b1_suspension_dir, &hitn);
        phys_multiply(&fric_ud, &rot_mat, &wheel_axis);
        //v28 = (pulse_sum_normal *)phys_transient_allocator::allocate(
        v28 = (pulse_sum_normal *)
            psys->m_solver_memory_allocator.allocate(
            160,
            16,
            0,
            SOLVER_MEMORY_ALLOCATOR_ERROR_MSG);
        ps_wheel->m_side = v28;
        v54 = this->b2;
        v52 = this->b1;
        this->m_ps_side_fric = v28;
        //pulse_sum_normal::set(v28, (int)v70, v52, &b1_r, v54, &b2_r, &fric_ud, &this->m_ps_cache_list[2], &b1_r_displace);
        v28->set(v52, &b1_r, v54, &b2_r, &fric_ud, &this->m_ps_cache_list[2], &b1_r_displace);
        m_ps_side_fric = this->m_ps_side_fric;
        m_ps_side_fric->m_right_side = 0.0;
        m_ps_side_fric->m_big_dirt = 0.0;
        m_ps_side_fric->m_cfm = 0.0;
        v30 = this->m_ps_suspension;
        desired_speed.x = fric_ud.y * hitn.z - fric_ud.z * hitn.y;
        desired_speed.y = fric_ud.z * hitn.x - hitn.z * fric_ud.x;
        desired_speed.z = hitn.y * fric_ud.x - fric_ud.y * hitn.x;
        fwd.x = -desired_speed.x;
        fwd.y = -desired_speed.y;
        fwd.z = -desired_speed.z;
        //relative_velocity = pulse_sum_normal::get_relative_velocity(v30, (int)v70, &wheel_axis);
        relative_velocity = v30->get_relative_velocity(&wheel_axis);
        m_wheel_state = this->m_wheel_state;
        pen_depth = relative_velocity->y * fwd.y + fwd.x * relative_velocity->x + relative_velocity->z * fwd.z;
        this->m_wheel_vel = pen_depth / this->m_wheel_radius;
        if (!m_wheel_state && this->m_acceleration_factor_k < 0.00009999999747378752
            || m_wheel_state == 1 && this->m_braking_factor_k < 0.00009999999747378752)
        {
            //pulse_sum_normal::set_pulse_sum_limits_parent_ratio(
                this->m_ps_side_fric->set_pulse_sum_limits_parent_ratio(
                this->m_side_fric_k,
                this->m_ps_suspension);
            return;
        }
        v33 = this->m_ps_side_fric;
        v34 = ps_wheel;
        v33->m_pulse_sum_min = -10000000.0;
        v33->m_pulse_sum_max = 10000000.0;
        //pulse_sum_wheel_fwd = pulse_sum_constraint_solver::create_pulse_sum_wheel_fwd(psys, v34);
        pulse_sum_wheel_fwd = psys->create_pulse_sum_wheel_fwd(v34);
        pen_depth = -this->m_pitch_stability_factor;
        v36 = this->b2;
        wheel_axis.x = pen_depth * b1_suspension_dir.x;
        v55 = v36;
        v37 = this->b1;
        wheel_axis.y = b1_suspension_dir.y * pen_depth;
        v38 = pen_depth * b1_suspension_dir.z;
        this->m_ps_fwd_fric = pulse_sum_wheel_fwd;
        wheel_axis.z = v38;
        //pulse_sum_normal::set(
            pulse_sum_wheel_fwd->set(
            v37,
            &b1_r,
            v55,
            &b2_r,
            &fwd,
            &this->m_ps_cache_list[3],
            &wheel_axis);
        if (this->m_wheel_state)
        {
            m_ps_fwd_fric = this->m_ps_fwd_fric;
            m_ps_fwd_fric->m_right_side = 0.0;
            m_ps_fwd_fric->m_big_dirt = 0.0;
            m_ps_fwd_fric->m_cfm = 0.0;
            v47 = this->m_ps_fwd_fric;
            desired_speed.w = this->m_braking_factor_k * delta_t;
            w = desired_speed.w;
            v48 = -desired_speed.w;
        }
        else
        {
            desired_speed.w = this->m_desired_speed_k * this->m_turning_radius_ratio_max_speed;
            pen_depth = this->m_wheel_radius;
            v39 = pen_depth;
            pen_depth = this->m_acceleration_factor_k * this->m_turning_radius_ratio_accel;
            v40 = v39 * v39;
            v41 = pen_depth * delta_t;
            pen_depth = v40;
            k_ = v41 / pen_depth;
            if (k_ <= 0.000001)
                k_ = 0.0000099999997;
            v42 = this->m_ps_fwd_fric;
            pen_depth = 1.0 / k_;
            v43 = desired_speed.w;
            v42->m_right_side = this->m_wheel_radius * desired_speed.w;
            v42->m_big_dirt = 0.0;
            v44 = pen_depth;
            v42->m_cfm = pen_depth;
            v42->m_denom = v44 + v42->m_denom;
            v45 = v43;
            w = 0.0;
            if (v45 > 0.00009999999747378752)
            {
                v47 = this->m_ps_fwd_fric;
                v47->m_pulse_sum_min = 0.0;
                w = 10000000.0;
            LABEL_22:
                v50 = ps_wheel;
                v47->m_pulse_sum_max = w;
                //pulse_sum_wheel::set_side_fwd_ratios(v50, this->m_side_fric_k, this->m_fwd_fric_k, this->m_side_fric_max);
                v50->set_side_fwd_ratios(this->m_side_fric_k, this->m_fwd_fric_k, this->m_side_fric_max);
                //v51 = pulse_sum_normal::get_relative_velocity_change_dir(this->m_ps_fwd_fric, &wheel_axis);
                v51 = this->m_ps_fwd_fric->get_relative_velocity_change_dir(&wheel_axis);
                this->m_wheel_fwd = v51->y * fwd.y + v51->x * fwd.x + v51->z * fwd.z;
                return;
            }
            v47 = this->m_ps_fwd_fric;
            if (v45 >= -0.00009999999747378752)
            {
                v47->m_pulse_sum_min = -10000000.0;
                w = 10000000.0;
                goto LABEL_22;
            }
            v48 = -10000000.0;
        }
        v47->m_pulse_sum_min = v48;
        goto LABEL_22;
    }
}

void __thiscall pulse_sum_normal::setup_vel_uni_standard(
        float delta_t,
        float max_penalty_restitution_vel)
{
  double v3; // st7
  int savedregs; // [esp+4h] [ebp+0h] BYREF
  float delta_tc; // [esp+Ch] [ebp+8h]
  float delta_ta; // [esp+Ch] [ebp+8h]
  float delta_tb; // [esp+Ch] [ebp+8h]

  v3 = 0.016666668;
  if ( delta_t > 0.016666668 )
    v3 = delta_t;
  delta_tc = v3;
  delta_ta = -pulse_sum_normal::get_pos() / delta_tc;
  this->m_big_dirt = delta_ta;
  if ( delta_ta < 0.0 )
    this->m_big_dirt = delta_ta * 0.300000011920929;
  delta_tb = -max_penalty_restitution_vel;
  if ( delta_tb > (double)this->m_big_dirt )
    this->m_big_dirt = delta_tb;
  if ( this->m_big_dirt < 0.0 )
  {
    this->m_right_side = 0.0;
  }
  else
  {
    this->m_right_side = this->m_big_dirt;
    this->m_big_dirt = 0.0;
  }
  this->m_cfm = 0.0;
}

pulse_sum_normal *__thiscall pulse_sum_constraint_solver::create_pulse_sum_wheel_fwd(pulse_sum_wheel *psw)
{
  pulse_sum_normal *result; // eax

  if ( !psw->m_side
    && _tlAssert(
         "C:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\constraint_solver\\pulse_sum_constraint_solver.h",
         196,
         "psw->m_side",
         "") )
  {
    __debugbreak();
  }
  result = (pulse_sum_normal *)//phys_transient_allocator::allocate(
                                 //&this->m_solver_memory_allocator,
                                 this->m_solver_memory_allocator.allocate(
                                 160,
                                 16,
                                 0,
                                 SOLVER_MEMORY_ALLOCATOR_ERROR_MSG);
  psw->m_fwd = result;
  return result;
}

pulse_sum_wheel *__thiscall pulse_sum_constraint_solver::create_pulse_sum_wheel()
{
  pulse_sum_wheel **v2; // edi
  pulse_sum_wheel **m_last_next_ptr; // ecx

  v2 = (pulse_sum_wheel **)//phys_transient_allocator::allocate(
                             this->m_solver_memory_allocator.allocate(
                             192,
                             16,
                             0,
                             SOLVER_MEMORY_ALLOCATOR_ERROR_MSG);
  if ( !this->m_list_pulse_sum_wheel.m_last_next_ptr
    && _tlAssert(
         "C:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\phys_mem.h",
         230,
         "m_last_next_ptr",
         "") )
  {
    __debugbreak();
  }
  *v2 = 0;
  m_last_next_ptr = this->m_list_pulse_sum_wheel.m_last_next_ptr;
  ++this->m_list_pulse_sum_wheel.m_alloc_count;
  *m_last_next_ptr = (pulse_sum_wheel *)v2;
  this->m_list_pulse_sum_wheel.m_last_next_ptr = v2;
  return (pulse_sum_wheel *)v2;
}

