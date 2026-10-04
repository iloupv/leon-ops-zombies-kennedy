#include "rbc_def_generic.h"
#include <physics/phys_util.h>
#include <physics/phys_assert.h>
#include <physics/phys_constraint_solver_multithreaded.h>

const phys_vec3 *__cdecl construct_orth_ud(phys_vec3 *result, const phys_vec3 *ud)
{
  double v2; // st7
  const phys_vec3 *v3; // eax
  float v4; // [esp-20h] [ebp-2Ch]
  float y; // [esp-20h] [ebp-2Ch]
  float v6; // [esp-1Ch] [ebp-28h]
  float x; // [esp-18h] [ebp-24h]
  double v8; // [esp-10h] [ebp-1Ch]
  float v9; // [esp-4h] [ebp-10h]
  float v10; // [esp-4h] [ebp-10h]
  float v11; // [esp-4h] [ebp-10h]
  float v12; // [esp-4h] [ebp-10h]
  float z; // [esp-4h] [ebp-10h]
  float v14; // [esp-4h] [ebp-10h]
  float v15; // [esp-4h] [ebp-10h]
  float v16; // [esp-4h] [ebp-10h]
  float v17; // [esp-4h] [ebp-10h]
  float v18; // [esp-4h] [ebp-10h]
  float v19; // [esp-4h] [ebp-10h]

  v9 = ud->y * ud->y + ud->x * ud->x + ud->z * ud->z;
  v10 = sqrt(v9);
  v11 = v10 - 1.0;
  v12 = fabs(v11);
  if ( v12 >= 0.00009999999747378752
    && _tlAssert(
         "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\old_phys_math.h",
         298,
         "fabsf(nud - 1.0f) < .0001f",
         "") )
  {
    __debugbreak();
  }
  result->x = 0.0;
  z = ud->z;
  v2 = z;
  result->y = z;
  v14 = -ud->y;
  result->z = v14;
  v8 = 0.0 * 0.0;
  v15 = v2 * v2 + 0.0 * 0.0 + v14 * v14;
  v16 = sqrt(v15);
  if ( v16 < 0.00009999999747378752 )
  {
    v4 = -ud->z;
    x = ud->x;
    result->x = v4;
    result->y = 0.0;
    result->z = x;
    v17 = x * x + v4 * v4 + v8;
    v16 = sqrt(v17);
    if ( v16 < 0.00009999999747378752 )
    {
      y = ud->y;
      v6 = -ud->x;
      result->x = y;
      result->y = v6;
      result->z = 0.0;
      v18 = y * y + v6 * v6 + v8;
      v16 = sqrt(v18);
      if ( v16 < 0.00009999999747378752 )
      {
        if ( _tlAssert("c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\old_phys_math.h", 313, "0", "not possible") )
          __debugbreak();
      }
    }
  }
  v3 = result;
  v19 = 1.0 / v16;
  result->x = result->x * v19;
  result->y = v19 * result->y;
  result->z = v19 * result->z;
  return v3;
}

void __cdecl PHYS_ASSERT_UNIT(const phys_vec3 *v)
{
  float va; // [esp+8h] [ebp+8h]
  float vb; // [esp+8h] [ebp+8h]
  float vc; // [esp+8h] [ebp+8h]
  float vd; // [esp+8h] [ebp+8h]

  va = v->y * v->y + v->x * v->x + v->z * v->z;
  vb = sqrt(va);
  vc = vb - 1.0;
  vd = fabs(vc);
  if ( vd >= 0.001 )
  {
    if ( _tlAssert(
           "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\phys_math.h",
           420,
           "fabsf(nv - 1.0f) < 0.001f",
           "") )
    {
      __debugbreak();
    }
  }
}

void __cdecl PHYS_ASSERT_ORTHOGONAL(const phys_vec3 *v1, const phys_vec3 *v2)
{
  float v1a; // [esp+8h] [ebp+8h]
  float v1b; // [esp+8h] [ebp+8h]

  v1a = v1->y * v2->y + v1->x * v2->x + v1->z * v2->z;
  v1b = fabs(v1a);
  if ( v1b >= 0.001 )
  {
    if ( _tlAssert(
           "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\phys_math.h",
           425,
           "fabsf(d) < 0.001f",
           "") )
    {
      __debugbreak();
    }
  }
}

void __cdecl PHYS_ASSERT_ORTHONORMAL(const phys_mat44 *m)
{
  float dyz; // [esp+4h] [ebp-14h]
  float dxz; // [esp+8h] [ebp-10h]
  float dxy; // [esp+Ch] [ebp-Ch]
  float nz; // [esp+10h] [ebp-8h]
  float ny; // [esp+14h] [ebp-4h]
  float ma; // [esp+20h] [ebp+8h]
  float mb; // [esp+20h] [ebp+8h]
  float mc; // [esp+20h] [ebp+8h]
  float md; // [esp+20h] [ebp+8h]
  float me; // [esp+20h] [ebp+8h]
  float mf; // [esp+20h] [ebp+8h]
  float mg; // [esp+20h] [ebp+8h]
  float mh; // [esp+20h] [ebp+8h]
  float mi; // [esp+20h] [ebp+8h]
  float mj; // [esp+20h] [ebp+8h]
  float mk; // [esp+20h] [ebp+8h]
  float ml; // [esp+20h] [ebp+8h]
  float mm; // [esp+20h] [ebp+8h]
  float mn; // [esp+20h] [ebp+8h]
  float mo; // [esp+20h] [ebp+8h]

  ma = m->y.y * m->y.y + m->y.x * m->y.x + m->y.z * m->y.z;
  mb = sqrt(ma);
  ny = mb;
  mc = m->z.y * m->z.y + m->z.x * m->z.x + m->z.z * m->z.z;
  md = sqrt(mc);
  nz = md;
  dxy = m->y.y * m->x.y + m->y.x * m->x.x + m->y.z * m->x.z;
  dxz = m->z.y * m->x.y + m->z.x * m->x.x + m->z.z * m->x.z;
  dyz = m->y.y * m->z.y + m->z.x * m->y.x + m->y.z * m->z.z;
  me = m->x.z * m->x.z + m->x.x * m->x.x + m->x.y * m->x.y;
  mf = sqrt(me);
  mg = mf - 1.0;
  mh = fabs(mg);
  if ( mh >= 0.001
    && _tlAssert(
         "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\phys_math.h",
         435,
         "fabsf(nx - 1.0f) < 0.001f",
         "") )
  {
    __debugbreak();
  }
  mi = ny - 1.0;
  mj = fabs(mi);
  if ( mj >= 0.001
    && _tlAssert(
         "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\phys_math.h",
         436,
         "fabsf(ny - 1.0f) < 0.001f",
         "") )
  {
    __debugbreak();
  }
  mk = nz - 1.0;
  ml = fabs(mk);
  if ( ml >= 0.001
    && _tlAssert(
         "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\phys_math.h",
         437,
         "fabsf(nz - 1.0f) < 0.001f",
         "") )
  {
    __debugbreak();
  }
  mm = fabs(dxy);
  if ( mm >= 0.001
    && _tlAssert(
         "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\phys_math.h",
         438,
         "fabsf(dxy) < 0.001f",
         "") )
  {
    __debugbreak();
  }
  mn = fabs(dxz);
  if ( mn >= 0.001
    && _tlAssert(
         "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\phys_math.h",
         439,
         "fabsf(dxz) < 0.001f",
         "") )
  {
    __debugbreak();
  }
  mo = fabs(dyz);
  if ( mo >= 0.001 )
  {
    if ( _tlAssert(
           "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\phys_math.h",
           440,
           "fabsf(dyz) < 0.001f",
           "") )
    {
      __debugbreak();
    }
  }
}

const phys_vec3 *__cdecl rbint::sub_pos(phys_vec3 *result, rigid_body *const b, const phys_vec3 *p)
{
  const phys_vec3 *v3; // eax

  v3 = result;
  result->x = p->x - b->m_mat.w.x;
  result->y = p->y - b->m_mat.w.y;
  result->z = p->z - b->m_mat.w.z;
  return v3;
}

void __thiscall rigid_body_constraint_point::set(
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

void rigid_body_constraint_point::epilog_vel_constaint(float __formal)
{
    this->m_stress = this->m_ps_cache_list[1].m_pulse_sum * this->m_ps_cache_list[1].m_pulse_sum
        + this->m_ps_cache_list[0].m_pulse_sum * this->m_ps_cache_list[0].m_pulse_sum
        + this->m_ps_cache_list[2].m_pulse_sum * this->m_ps_cache_list[2].m_pulse_sum;
}

//void __thiscall rigid_body_constraint_point::epilog_vel_constraint(float __formal)
//{
//  this->m_stress = this->m_ps_cache_list[1].m_pulse_sum * this->m_ps_cache_list[1].m_pulse_sum
//                 + this->m_ps_cache_list[0].m_pulse_sum * this->m_ps_cache_list[0].m_pulse_sum
//                 + this->m_ps_cache_list[2].m_pulse_sum * this->m_ps_cache_list[2].m_pulse_sum;
//}

void __thiscall rigid_body_constraint_distance::set(
        const phys_vec3 *b1_r_loc,
        const phys_vec3 *b2_r_loc,
        float min_distance,
        float max_distance)
{
  this->m_b1_r_loc.x = b1_r_loc->x;
  this->m_b1_r_loc.y = b1_r_loc->y;
  this->m_b1_r_loc.z = b1_r_loc->z;
  this->m_b2_r_loc.x = b2_r_loc->x;
  this->m_b2_r_loc.y = b2_r_loc->y;
  this->m_b2_r_loc.z = b2_r_loc->z;
  this->m_flags = 1;
  this->m_min_distance = min_distance;
  this->m_max_distance = max_distance;
  this->m_next_max_distance = max_distance;
  this->m_max_distance_vel = 0.0;
  this->m_damp_coef = 0.0;
}

void __thiscall rigid_body_constraint_distance::outer_prolog_update(float delta_t)
{
  this->m_max_distance_vel = (this->m_next_max_distance - this->m_max_distance) / delta_t;
}

void __thiscall rigid_body_constraint_distance::inner_update(float delta_t)
{
  this->m_max_distance = this->m_max_distance_vel * delta_t + this->m_max_distance;
}

void __thiscall rigid_body_constraint_distance::outer_epilog_update(const float __formal)
{
  this->m_max_distance = this->m_next_max_distance;
}

void __thiscall rigid_body_constraint_angular_actuator::set(
        float power,
        const phys_mat44 *target_mat)
{
  this->m_power = power;
  this->m_power_scale = 1.0;
  this->m_target_mat.x.x = target_mat->x.x;
  this->m_target_mat.x.y = target_mat->x.y;
  this->m_target_mat.x.z = target_mat->x.z;
  this->m_target_mat.y.x = target_mat->y.x;
  this->m_target_mat.y.y = target_mat->y.y;
  this->m_target_mat.y.z = target_mat->y.z;
  this->m_target_mat.z.x = target_mat->z.x;
  this->m_target_mat.z.y = target_mat->z.y;
  this->m_target_mat.z.z = target_mat->z.z;
  this->m_target_mat.w.x = target_mat->w.x;
  this->m_target_mat.w.y = target_mat->w.y;
  this->m_target_mat.w.z = target_mat->w.z;
  this->m_next_target_mat.x.x = target_mat->x.x;
  this->m_next_target_mat.x.y = target_mat->x.y;
  this->m_next_target_mat.x.z = target_mat->x.z;
  this->m_next_target_mat.y.x = target_mat->y.x;
  this->m_next_target_mat.y.y = target_mat->y.y;
  this->m_next_target_mat.y.z = target_mat->y.z;
  this->m_next_target_mat.z.x = target_mat->z.x;
  this->m_next_target_mat.z.y = target_mat->z.y;
  this->m_next_target_mat.z.z = target_mat->z.z;
  this->m_next_target_mat.w.x = target_mat->w.x;
  this->m_next_target_mat.w.y = target_mat->w.y;
  this->m_next_target_mat.w.z = target_mat->w.z;
  this->m_a_vel.x = 0.0;
  this->m_a_vel.y = 0.0;
  this->m_a_vel.z = 0.0;
  this->m_enabled = 1;
  PHYS_ASSERT_ORTHONORMAL(&this->m_target_mat);
}

void rigid_body_constraint_angular_actuator::outer_prolog_update(float delta_t)
{
    phys_vec3 t_vel; // [esp+Ch] [ebp-1Ch] BYREF
    //_UNKNOWN *v4[2]; // [esp+1Ch] [ebp-Ch] BYREF
    //int vars0; // [esp+28h] [ebp+0h]
    //
    //*(float *)v4 = a2;
    //v4[1] = (_UNKNOWN *)vars0;
    this->m_target_mat.w.x = 0.0;
    this->m_target_mat.w.y = 0.0;
    this->m_target_mat.w.z = 0.0;
    this->m_next_target_mat.w.x = 0.0;
    this->m_next_target_mat.w.y = 0.0;
    this->m_next_target_mat.w.z = 0.0;
    nuge::calc_velocities(
        &this->m_target_mat,
        &this->m_next_target_mat,
        delta_t,
        &t_vel,
        &this->m_a_vel);
}

void __thiscall rigid_body_constraint_angular_actuator::outer_epilog_update(const float __formal)
{
  this->m_target_mat.x.x = this->m_next_target_mat.x.x;
  this->m_target_mat.x.y = this->m_next_target_mat.x.y;
  this->m_target_mat.x.z = this->m_next_target_mat.x.z;
  this->m_target_mat.y.x = this->m_next_target_mat.y.x;
  this->m_target_mat.y.y = this->m_next_target_mat.y.y;
  this->m_target_mat.y.z = this->m_next_target_mat.y.z;
  this->m_target_mat.z.x = this->m_next_target_mat.z.x;
  this->m_target_mat.z.y = this->m_next_target_mat.z.y;
  this->m_target_mat.z.z = this->m_next_target_mat.z.z;
  this->m_target_mat.w.x = this->m_next_target_mat.w.x;
  this->m_target_mat.w.y = this->m_next_target_mat.w.y;
  this->m_target_mat.w.z = this->m_next_target_mat.w.z;
}

double __thiscall rigid_body_constraint_upright::calc_current_lean_angle()
{
  float v2; // [esp-Ch] [ebp-2Ch]
  float v3; // [esp-8h] [ebp-28h]
  float v4; // [esp-4h] [ebp-24h]
  float b2_up_axis_12; // [esp+Ch] [ebp-14h]
  float v7; // [esp+10h] [ebp-10h]
  float v8; // [esp+10h] [ebp-10h]

  v2 = this->b1->m_mat.x.y * this->m_b2_up_axis_loc.y
     + this->b1->m_mat.x.x * this->m_b2_up_axis_loc.x
     + this->b1->m_mat.x.z * this->m_b2_up_axis_loc.z;
  v3 = this->b1->m_mat.y.y * this->m_b2_up_axis_loc.y
     + this->m_b2_up_axis_loc.x * this->b1->m_mat.y.x
     + this->b1->m_mat.y.z * this->m_b2_up_axis_loc.z;
  v4 = this->b1->m_mat.z.y * this->m_b2_up_axis_loc.y
     + this->m_b2_up_axis_loc.x * this->b1->m_mat.z.x
     + this->b1->m_mat.z.z * this->m_b2_up_axis_loc.z;
  b2_up_axis_12 = this->m_b1_up_axis_loc.z * v4 + this->m_b1_up_axis_loc.x * v2 + this->m_b1_up_axis_loc.y * v3;
  v7 = v2 * this->m_b1_right_axis_loc.x + v3 * this->m_b1_right_axis_loc.y + v4 * this->m_b1_right_axis_loc.z;
  v8 = -v7;
  if ( v8 * v8 + b2_up_axis_12 * b2_up_axis_12 <= 0.000000009999999051046871 )
    return 0.0;
  return (float)atan2(v8, b2_up_axis_12);
}

const phys_vec3 *__thiscall rigid_body_constraint_upright::calc_b1_lean_axis_loc(phys_vec3 *result, float lean_angle)
{
  const phys_vec3 *v3; // eax
  float v4; // [esp-30h] [ebp-3Ch]
  float v5; // [esp-2Ch] [ebp-38h]
  float v6; // [esp-28h] [ebp-34h]
  float v7; // [esp-20h] [ebp-2Ch]
  float v8; // [esp-1Ch] [ebp-28h]
  float v9; // [esp-18h] [ebp-24h]
  float v10; // [esp-4h] [ebp-10h]
  float v11; // [esp-4h] [ebp-10h]

  v10 = sin(lean_angle);
  v4 = this->m_b1_right_axis_loc.x * v10;
  v5 = this->m_b1_right_axis_loc.y * v10;
  v6 = v10 * this->m_b1_right_axis_loc.z;
  v11 = cos(lean_angle);
  v3 = result;
  v7 = this->m_b1_up_axis_loc.x * v11;
  v8 = this->m_b1_up_axis_loc.y * v11;
  v9 = v11 * this->m_b1_up_axis_loc.z;
  result->x = v7 - v4;
  result->y = v8 - v5;
  result->z = v9 - v6;
  return v3;
}

void rigid_body_constraint_hinge::set(
        const phys_vec3 *b1_r_loc,
        const phys_vec3 *b2_r_loc,
        const phys_vec3 *b1_axis_loc,
        const phys_vec3 *b2_axis_loc,
        const phys_vec3 *b1_ref_loc,
        const phys_vec3 *b2_ref_loc,
        float theta_min,
        float theta_max,
        float damp_k)
{
    phys_vec3 *v12; // eax
    phys_vec3 *p_m_b1_axis_loc; // edi
    phys_vec3 *v14; // eax
    phys_vec3 *v15; // eax
    double y; // st7
    const phys_vec3 *v17; // eax
    double v18; // st7
    double v19; // st7
    const phys_vec3 *v20; // ecx
    double v21; // st6
    const phys_vec3 *v22; // eax
    phys_vec3 *v23; // eax
    const phys_vec3 *v24; // eax
    phys_vec3 *v25; // eax
    const phys_vec3 *v26; // esi
    phys_mat44 rv; // [esp+18h] [ebp-9Ch] BYREF
    phys_vec3 v28; // [esp+58h] [ebp-5Ch] BYREF
    phys_vec3 v29; // [esp+68h] [ebp-4Ch] BYREF
    const phys_vec3 *p_m_b1_ref_loc; // [esp+7Ch] [ebp-38h]
    const phys_vec3 *p_m_b2_ref_max_loc; // [esp+80h] [ebp-34h]
    const phys_vec3 *p_m_b2_ref_min_loc; // [esp+84h] [ebp-30h]
    phys_vec3 v33; // [esp+88h] [ebp-2Ch] BYREF
    const phys_vec3 *p_m_b1_a1_loc; // [esp+9Ch] [ebp-18h]
    const phys_vec3 *p_m_b1_a2_loc; // [esp+A0h] [ebp-14h]
    const phys_vec3 *p_m_b2_axis_loc; // [esp+A4h] [ebp-10h]
    //_UNKNOWN *v37[2]; // [esp+A8h] [ebp-Ch] BYREF
    //const phys_vec3 *b1_axis_loca; // [esp+B4h] [ebp+0h]
    //
    //v37[0] = a2;
    //v37[1] = b1_axis_loca;
    this->m_flags = 0;
    this->m_b1_r_loc.x = b1_r_loc->x;
    this->m_b1_r_loc.y = b1_r_loc->y;
    this->m_b1_r_loc.z = b1_r_loc->z;
    this->m_b2_r_loc.x = b2_r_loc->x;
    this->m_b2_r_loc.y = b2_r_loc->y;
    this->m_b2_r_loc.z = b2_r_loc->z;
    v12 = phys_Unitize(&v33, b1_axis_loc);
    this->m_b1_axis_loc.x = v12->x;
    p_m_b1_axis_loc = &this->m_b1_axis_loc;
    this->m_b1_axis_loc.y = v12->y;
    this->m_b1_axis_loc.z = v12->z;
    v14 = phys_Unitize(&v33, b2_axis_loc);
    this->m_b2_axis_loc.x = v14->x;
    p_m_b2_axis_loc = &this->m_b2_axis_loc;
    this->m_b2_axis_loc.y = v14->y;
    this->m_b2_axis_loc.z = v14->z;
    v15 = phys_Unitize(&v33, b1_ref_loc);
    this->m_b1_ref_loc.x = v15->x;
    y = v15->y;
    p_m_b1_ref_loc = &this->m_b1_ref_loc;
    this->m_b1_ref_loc.y = y;
    this->m_b1_ref_loc.z = v15->z;
    v17 = construct_orth_ud(&v33, &this->m_b1_axis_loc);
    this->m_b1_a1_loc.x = v17->x;
    v18 = v17->y;
    p_m_b1_a1_loc = &this->m_b1_a1_loc;
    this->m_b1_a1_loc.y = v18;
    this->m_b1_a1_loc.z = v17->z;
    v33.x = this->m_b1_axis_loc.y * this->m_b1_a1_loc.z - this->m_b1_axis_loc.z * this->m_b1_a1_loc.y;
    v33.y = this->m_b1_a1_loc.x * this->m_b1_axis_loc.z - this->m_b1_a1_loc.z * this->m_b1_axis_loc.x;
    v19 = this->m_b1_a1_loc.y * this->m_b1_axis_loc.x;
    v20 = p_m_b2_axis_loc;
    v21 = this->m_b1_a1_loc.x * this->m_b1_axis_loc.y;
    p_m_b1_a2_loc = &this->m_b1_a2_loc;
    v33.z = v19 - v21;
    this->m_b1_a2_loc.x = v33.x;
    this->m_b1_a2_loc.y = v33.y;
    this->m_b1_a2_loc.z = v33.z;
    make_rotate(&rv, v20, theta_min, 1000.0);
    v22 = phys_multiply(&v33, &rv, b2_ref_loc);
    v23 = phys_Unitize(&v29, v22);
    this->m_b2_ref_min_loc.x = v23->x;
    this->m_b2_ref_min_loc.y = v23->y;
    p_m_b2_ref_min_loc = &this->m_b2_ref_min_loc;
    this->m_b2_ref_min_loc.z = v23->z;
    make_rotate(&rv, p_m_b2_axis_loc, theta_max, 1000.0);
    v24 = phys_multiply(&v29, &rv, b2_ref_loc);
    v25 = phys_Unitize(&v28, v24);
    this->m_b2_ref_max_loc.x = v25->x;
    this->m_b2_ref_max_loc.y = v25->y;
    p_m_b2_ref_max_loc = &this->m_b2_ref_max_loc;
    this->m_b2_ref_max_loc.z = v25->z;
    this->m_damp_k = damp_k;
    PHYS_ASSERT_UNIT(&this->m_b1_axis_loc);
    PHYS_ASSERT_UNIT(p_m_b2_axis_loc);
    v26 = p_m_b1_ref_loc;
    PHYS_ASSERT_UNIT(p_m_b1_ref_loc);
    PHYS_ASSERT_UNIT(p_m_b1_a1_loc);
    PHYS_ASSERT_UNIT(p_m_b1_a2_loc);
    PHYS_ASSERT_UNIT(p_m_b2_ref_min_loc);
    PHYS_ASSERT_UNIT(p_m_b2_ref_max_loc);
    PHYS_ASSERT_ORTHOGONAL(p_m_b1_axis_loc, v26);
    PHYS_ASSERT_ORTHOGONAL(p_m_b1_a1_loc, p_m_b1_a2_loc);
}

void rigid_body_constraint_angular_actuator::inner_update(float delta_t)
{
    phys_mat44 rv; // [esp+18h] [ebp-4Ch] BYREF
    //_UNKNOWN *v5[2]; // [esp+58h] [ebp-Ch] BYREF
    //int vars0; // [esp+64h] [ebp+0h]
    //
    //*(float *)v5 = a2;
    //v5[1] = (_UNKNOWN *)vars0;
    make_rotate(&rv, &this->m_a_vel, delta_t, 1000.0);
    phys_multiply_mat(&this->m_target_mat, &rv, &this->m_target_mat);
}

void __thiscall rigid_body_constraint_upright::epilog_vel_constraint(float delta_t)
{
  if ( this->m_enabled )
  {
    this->m_lean_angle = rigid_body_constraint_upright::calc_current_lean_angle();
    this->m_lean_angle_calc_delta_t = this->m_lean_angle_calc_delta_t + delta_t;
  }
}

void rigid_body_constraint_upright::update_lean_axis(
        const phys_vec3 *b1_lean_center,
        const phys_vec3 *b1_lean_axis_loc)
{
    phys_vec3 *p_m_b1_lean_axis_loc; // edi
    float x; // eax
    float z; // edx
    const phys_vec3 *v8; // eax
    double v9; // st7
    phys_mat44 *v10; // eax
    rigid_body *b1; // esi
    phys_mat44 v12; // [esp-Ch] [ebp-7Ch] BYREF
    phys_vec3 b1_lean_axis; // [esp+34h] [ebp-3Ch] BYREF
    phys_vec3 b2_up_axis; // [esp+44h] [ebp-2Ch] BYREF
    const phys_mat44 *p_m_mat; // [esp+5Ch] [ebp-14h]
    float nb2_up_axis; // [esp+60h] [ebp-10h]
    //_UNKNOWN *v17[2]; // [esp+64h] [ebp-Ch] BYREF
    //int vars0; // [esp+70h] [ebp+0h]
    //
    //v17[0] = a2;
    //v17[1] = (_UNKNOWN *)vars0;
    this->m_b1_lean_axis_loc.x = b1_lean_axis_loc->x;
    p_m_b1_lean_axis_loc = &this->m_b1_lean_axis_loc;
    this->m_b1_lean_axis_loc.y = b1_lean_axis_loc->y;
    this->m_b1_lean_axis_loc.z = b1_lean_axis_loc->z;
    if (this->m_enabled)
    {
        p_m_mat = &this->b1->m_mat;
        phys_multiply(&b1_lean_axis, p_m_mat, &this->m_b1_forward_axis_loc);
        x = this->m_b2_up_axis_loc.x;
        z = this->m_b2_up_axis_loc.z;
        b2_up_axis.y = this->m_b2_up_axis_loc.y;
        b2_up_axis.x = x;
        b2_up_axis.z = z;
        b2_up_axis.w = this->m_b2_up_axis_loc.w;
        nb2_up_axis = b1_lean_axis.x * x + b1_lean_axis.y * b2_up_axis.y + b1_lean_axis.z * z;
        b1_lean_axis.x = b1_lean_axis.x * nb2_up_axis;
        b1_lean_axis.y = b1_lean_axis.y * nb2_up_axis;
        b1_lean_axis.z = b1_lean_axis.z * nb2_up_axis;
        b2_up_axis.x = x - b1_lean_axis.x;
        b2_up_axis.y = b2_up_axis.y - b1_lean_axis.y;
        b2_up_axis.z = z - b1_lean_axis.z;
        nb2_up_axis = b2_up_axis.y * b2_up_axis.y + b2_up_axis.x * b2_up_axis.x + b2_up_axis.z * b2_up_axis.z;
        nb2_up_axis = sqrt(nb2_up_axis);
        if (nb2_up_axis > 0.0000099999997)
        {
            nb2_up_axis = 1.0 / nb2_up_axis;
            b2_up_axis.x = nb2_up_axis * b2_up_axis.x;
            b2_up_axis.y = b2_up_axis.y * nb2_up_axis;
            b2_up_axis.z = nb2_up_axis * b2_up_axis.z;
            phys_multiply(&b1_lean_axis, p_m_mat, p_m_b1_lean_axis_loc);
            make_rotate(&v12, &b1_lean_axis, &b2_up_axis);
            v8 = phys_multiply(&b2_up_axis, &v12, b1_lean_center);
            b1_lean_axis.x = b1_lean_center->x - v8->x;
            b1_lean_axis.y = b1_lean_center->y - v8->y;
            v9 = b1_lean_center->z - v8->z;
            v10 = &this->b1->m_mat;
            b1_lean_axis.z = v9;
            v12.w.x = b1_lean_axis.x;
            v12.w.y = b1_lean_axis.y;
            v12.w.z = b1_lean_axis.z;
            phys_full_multiply_mat(v10, &v12, v10);
            b1 = this->b1;
            *(float *)&p_m_mat = fabs(b1->m_mat.w.x);
            if (*(float *)&p_m_mat > 100000.0
                || (*(float *)&p_m_mat = fabs(b1->m_mat.w.y), *(float *)&p_m_mat > 100000.0)
                || (*(float *)&p_m_mat = fabs(b1->m_mat.w.z), *(float *)&p_m_mat > 100000.0))
            {
                phys_exec_debug_callback(b1);
            }
        }
    }
}

void rigid_body_constraint_point::setup_constraint(
        pulse_sum_constraint_solver *psys,
        float delta_t)
{
  rigid_body *b2; // edi
  phys_vec3 v6; // [esp+20h] [ebp-3Ch] BYREF
  phys_vec3 v7; // [esp+30h] [ebp-2Ch] BYREF
  rigid_body *b1; // [esp+4Ch] [ebp-10h]
  //int v9; // [esp+50h] [ebp-Ch]
  //void *v10; // [esp+54h] [ebp-8h]
  //void *retaddr; // [esp+5Ch] [ebp+0h]
  //
  //v9 = a2;
  //v10 = retaddr;
  b2 = this->b2;
  phys_multiply(&v7, &b2->m_mat, &this->m_b2_r_loc);
  b1 = this->b1;
  phys_multiply(&v6, &b1->m_mat, &this->m_b1_r_loc);
  //pulse_sum_constraint_solver::create_point(
    psys->create_point(
    b1,
    &v6,
    b2,
    &v7,
    this->m_ps_cache_list,
    delta_t,
    this->m_spring_enabled,
    this->m_spring_k,
    this->m_damp_k);
}

void rigid_body_constraint_hinge::setup_constraint(
        pulse_sum_constraint_solver *psys,
        float delta_t)
{
    rigid_body *b2; // edi
    phys_mat44 *p_m_mat; // edi
    pulse_sum_angular *v7; // eax
    double v8; // st7
    phys_mat44 *v9; // edi
    double z; // st7
    unsigned int m_flags; // ecx
    double y; // st6
    double v13; // st5
    double v14; // st4
    double v15; // st2
    double v16; // st5
    unsigned int v17; // ecx
    double v18; // st2
    double v19; // st4
    double x; // st3
    unsigned int v21; // ecx
    double v22; // st2
    rigid_body *v23; // eax
    pulse_sum_angular *v24; // eax
    pulse_sum_angular *v25; // eax
    phys_vec3 b1_axis; // [esp+20h] [ebp-8Ch] BYREF
    phys_vec3 v27; // [esp+30h] [ebp-7Ch] BYREF
    phys_vec3 v28; // [esp+40h] [ebp-6Ch] BYREF
    phys_vec3 b1_ref; // [esp+50h] [ebp-5Ch] BYREF
    phys_vec3 b2_axis; // [esp+60h] [ebp-4Ch] BYREF
    phys_vec3 b2_ref_max; // [esp+70h] [ebp-3Ch] BYREF
    phys_vec3 b2_ref_min; // [esp+80h] [ebp-2Ch] BYREF
    float v33; // [esp+9Ch] [ebp-10h]
    //_UNKNOWN *v34; // [esp+A0h] [ebp-Ch]
    //pulse_sum_constraint_solver *psysa; // [esp+A4h] [ebp-8h]
    //int vars0; // [esp+ACh] [ebp+0h]
    //
    //v34 = a2;
    //psysa = (pulse_sum_constraint_solver *)vars0;
    if ((this->m_flags & 0x10) == 0)
    {
        b2 = this->b2;
        phys_multiply(&v28, &b2->m_mat, &this->m_b2_r_loc);
        phys_multiply(&v27, &this->b1->m_mat, &this->m_b1_r_loc);
        //pulse_sum_constraint_solver::create_point(psys, this->b1, &v27, b2, &v28, this->m_ps_cache, delta_t, 0, 0.0, 0.0);
        psys->create_point(this->b1, &v27, b2, &v28, this->m_ps_cache, delta_t, 0, 0.0, 0.0);
    }
    p_m_mat = &this->b1->m_mat;
    phys_multiply(&b1_axis, p_m_mat, &this->m_b1_axis_loc);
    phys_multiply(&b2_axis, &this->b2->m_mat, &this->m_b2_axis_loc);
    phys_multiply(&v27, p_m_mat, &this->m_b1_a2_loc);
    phys_multiply(&v28, p_m_mat, &this->m_b1_a1_loc);
    //pulse_sum_constraint_solver::create_hinge(
        psys->create_hinge(
        this->b1,
        &b1_axis,
        this->b2,
        &b2_axis,
        &v28,
        &v27,
        &this->m_ps_cache[4],
        delta_t);
    if (this->m_damp_k > 0.0000099999997)
    {
        v7 = //pulse_sum_constraint_solver::create_pulse_sum_angular(
            psys->create_pulse_sum_angular(
            this->b1,
            &PHYS_ZERO_VEC,
            this->b2,
            &PHYS_ZERO_VEC,
            &b2_axis,
            &this->m_ps_cache[3]);
        v7->m_right_side = 0.0;
        v7->m_big_dirt = 0.0;
        v7->m_cfm = 0.0;
        v33 = delta_t * this->m_damp_k;
        v8 = v33;
        v7->m_pulse_sum_min = -v33;
        v7->m_pulse_sum_max = v8;
    }
    phys_multiply(&b1_ref, &this->b1->m_mat, &this->m_b1_ref_loc);
    v9 = &this->b2->m_mat;
    phys_multiply(&b2_ref_min, v9, &this->m_b2_ref_min_loc);
    phys_multiply(&b2_ref_max, v9, &this->m_b2_ref_max_loc);
    z = b2_axis.z;
    m_flags = this->m_flags;
    y = b2_axis.y;
    v13 = b1_ref.y;
    v14 = b1_ref.y;
    if ((m_flags & 8) != 0)
    {
        v33 = b2_ref_min.x * b1_ref.x + v14 * b2_ref_min.y + b1_ref.z * b2_ref_min.z;
        v15 = v13 * b2_ref_max.y;
        v16 = v33;
        v33 = b1_ref.x * b2_ref_max.x + v15 + b1_ref.z * b2_ref_max.z;
        if (v33 > v16)
        {
            this->m_flags = m_flags & 0xFFFFFFFC | 2;
        }
        else
        {
            v17 = m_flags | 1;
            this->m_flags = v17;
            this->m_flags = v17 & 0xFFFFFFFD;
        }
    }
    else
    {
        v18 = v14 * b2_ref_min.z - b1_ref.z * b2_ref_min.y;
        v19 = b1_ref.z;
        v28.x = v18;
        x = b1_ref.x;
        v28.y = b1_ref.z * b2_ref_min.x - b2_ref_min.z * b1_ref.x;
        v28.z = b1_ref.x * b2_ref_min.y - v13 * b2_ref_min.x;
        v33 = y * v28.y + v28.x * b2_axis.x + z * v28.z;
        if (v33 < -0.043618999)
            v21 = m_flags & 0xFFFFFFFE;
        else
            v21 = m_flags | 1;
        v22 = b2_ref_max.z;
        this->m_flags = v21;
        v28.x = v22 * v13 - b2_ref_max.y * v19;
        v28.y = v19 * b2_ref_max.x - b2_ref_max.z * x;
        v28.z = x * b2_ref_max.y - b2_ref_max.x * v13;
        v33 = v28.y * y + v28.x * b2_axis.x + v28.z * z;
        if (v33 > 0.043618999)
            this->m_flags &= ~2u;
        else
            this->m_flags |= 2u;
    }
    if ((this->m_flags & 1) != 0)
    {
        v28.x = -b2_axis.x;
        v23 = this->b2;
        v28.y = -y;
        v28.z = -z;
        v24 = //pulse_sum_constraint_solver::create_pulse_sum_angular(
            psys->create_pulse_sum_angular(
            this->b1,
            &b1_ref,
            v23,
            &b2_ref_min,
            &v28,
            &this->m_ps_cache[6]);
        v24->m_pulse_sum_min = -10000000.0;
        v24->m_pulse_sum_max = 0.0;
        //pulse_sum_angular::setup_vel_uni_standard(v24, delta_t, 5.0);
        v24->setup_vel_uni_standard(delta_t, 5.0);
    }
    if ((this->m_flags & 2) != 0)
    {
        v25 = //pulse_sum_constraint_solver::create_pulse_sum_angular(
            psys->create_pulse_sum_angular(
            this->b1,
            &b1_ref,
            this->b2,
            &b2_ref_max,
            &b2_axis,
            &this->m_ps_cache[7]);
        v25->m_pulse_sum_min = -10000000.0;
        v25->m_pulse_sum_max = 0.0;
        //pulse_sum_angular::setup_vel_uni_standard(v25, delta_t, 5.0);
        v25->setup_vel_uni_standard(delta_t, 5.0);
    }
}

void rigid_body_constraint_distance::setup_constraint(pulse_sum_constraint_solver *psys, float delta_t)
{
    rigid_body *b1; // edi
    rigid_body *b2; // eax
    pulse_sum_normal *psn; // edi
    rigid_body *v8; // eax
    pulse_sum_normal *v9; // eax
    pulse_sum_normal *v10; // ecx
    phys_vec3 *v11; // eax
    double v12; // st7
    const phys_vec3 *v13; // eax
    phys_vec3 *v14; // eax
    pulse_sum_normal *v15; // edi
    const phys_vec3 *v16; // eax
    double z; // st7
    rigid_body *v18; // ecx
    const phys_vec3 *v19; // eax
    double v20; // st7
    rigid_body *v21; // [esp-10h] [ebp-C8h]
    rigid_body *v22; // [esp-8h] [ebp-C0h]
    float pos_adjust; // [esp+4h] [ebp-B4h]
    float pos_adjusta; // [esp+4h] [ebp-B4h]
    float pos_adjustb; // [esp+4h] [ebp-B4h]
    phys_vec3 r2; // [esp+1Ch] [ebp-9Ch] BYREF
    phys_vec3 b2_pt; // [esp+2Ch] [ebp-8Ch] BYREF
    phys_vec3 b1_pt; // [esp+3Ch] [ebp-7Ch] BYREF
    phys_vec3 b1_r; // [esp+4Ch] [ebp-6Ch] BYREF
    phys_vec3 b2_r; // [esp+5Ch] [ebp-5Ch] BYREF
    phys_vec3 ud; // [esp+6Ch] [ebp-4Ch] BYREF
    phys_vec3 r1; // [esp+7Ch] [ebp-3Ch] BYREF
    phys_vec3 damp_dir; // [esp+8Ch] [ebp-2Ch] BYREF
    float ndamp_dir; // [esp+A4h] [ebp-14h]
    pulse_sum_normal *ps_min; // [esp+A8h] [ebp-10h]
    //_UNKNOWN *v36[2]; // [esp+ACh] [ebp-Ch] BYREF
    //int vars0; // [esp+B8h] [ebp+0h]
    //
    //v36[0] = a2;
    //v36[1] = (_UNKNOWN *)vars0;
    if ((this->m_flags & 1) != 0)
    {
        if (this->m_min_distance < 0.0
            && _tlAssert("source/rbc_def_generic.cpp", 137, "m_min_distance >= 0.0f", ""))
        {
            __debugbreak();
        }
        if (this->m_max_distance < (double)this->m_min_distance
            && _tlAssert("source/rbc_def_generic.cpp", 138, "m_min_distance <= m_max_distance", ""))
        {
            __debugbreak();
        }
        b1 = this->b1;
        phys_multiply(&b1_r, &this->b1->m_mat, &this->m_b1_r_loc);
        phys_multiply(&b2_r, &this->b2->m_mat, &this->m_b2_r_loc);
        b2 = this->b2;
        b1_pt.x = b1->m_mat.w.x + b1_r.x;
        b1_pt.y = b1->m_mat.w.y + b1_r.y;
        b1_pt.z = b1->m_mat.w.z + b1_r.z;
        b2_pt.x = b2->m_mat.w.x + b2_r.x;
        b2_pt.y = b2->m_mat.w.y + b2_r.y;
        b2_pt.z = b2->m_mat.w.z + b2_r.z;
        damp_dir.x = b2_pt.x - b1_pt.x;
        damp_dir.y = b2_pt.y - b1_pt.y;
        damp_dir.z = b2_pt.z - b1_pt.z;
        ndamp_dir = damp_dir.y * damp_dir.y + damp_dir.x * damp_dir.x + damp_dir.z * damp_dir.z;
        if (ndamp_dir >= 0.0000010000001)
        {
            *(float *)&ps_min = sqrt(ndamp_dir);
            *(float *)&ps_min = 1.0 / *(float *)&ps_min;
            ud.x = damp_dir.x * *(float *)&ps_min;
            ud.y = damp_dir.y * *(float *)&ps_min;
            ud.z = *(float *)&ps_min * damp_dir.z;
            //psn = pulse_sum_constraint_solver::create_pulse_sum_normal(psys);
            psn = psys->create_pulse_sum_normal();
            r1.x = -ud.x;
            v8 = this->b2;
            r1.y = -ud.y;
            r1.z = -ud.z;
            //pulse_sum_normal::set(psn, (int)v36, this->b1, &b1_r, v8, &b2_r, &r1, this->m_ps_cache_list, &PHYS_ZERO_VEC_74);
            psn->set(this->b1, &b1_r, v8, &b2_r, &r1, this->m_ps_cache_list, &PHYS_ZERO_VEC);
            psn->m_pulse_sum_min = -10000000.0;
            psn->m_pulse_sum_max = 0.0;
            pos_adjust = -this->m_max_distance;
            //pulse_sum_normal::setup_vel_uni_standard_pos_adjust(psn, delta_t, pos_adjust, 1700.0);
            psn->setup_vel_uni_standard_pos_adjust(delta_t, pos_adjust, 1700.0);
            psn->m_right_side = this->m_max_distance_vel + psn->m_right_side;
            if (this->m_min_distance > 0.0099999998)
            {
                //v9 = pulse_sum_constraint_solver::create_pulse_sum_normal(psys);
                v9 = psys->create_pulse_sum_normal();
                v22 = this->b2;
                v21 = this->b1;
                ps_min = v9;
                //pulse_sum_normal::set(v9, (int)v36, v21, &b1_r, v22, &b2_r, &ud, &this->m_ps_cache_list[1], &PHYS_ZERO_VEC_74);
                v9->set(v21, &b1_r, v22, &b2_r, &ud, &this->m_ps_cache_list[1], &PHYS_ZERO_VEC);
                v10 = ps_min;
                ps_min->m_pulse_sum_min = -10000000.0;
                v10->m_pulse_sum_max = 0.0;
                //pulse_sum_normal::setup_vel_uni_standard_pos_adjust(v10, delta_t, this->m_min_distance, 1700.0);
                v10->setup_vel_uni_standard_pos_adjust(delta_t, this->m_min_distance, 1700.0);
            }
            if (this->m_damp_coef > 0.0000099999997)
            {
                *(float *)&ps_min = this->m_max_distance - 0.1000000014901161;
                *(float *)&ps_min = *(float *)&ps_min * *(float *)&ps_min;
                if (*(float *)&ps_min <= (double)ndamp_dir)
                {
                    *(float *)&ps_min = this->m_max_distance + 0.1000000014901161;
                    *(float *)&ps_min = *(float *)&ps_min * *(float *)&ps_min;
                    if (*(float *)&ps_min >= (double)ndamp_dir)
                    {
                        //pulse_sum_normal::get_relative_velocity(psn, (int)v36, &damp_dir);
                        psn->get_relative_velocity(&damp_dir);
                        pos_adjusta = phys_dot(&damp_dir, &ud);
                        //v11 = operator*(&r1, pos_adjusta, &ud);
                        r1 = ud * pos_adjusta;
                        //phys_vec3::operator-=(&damp_dir, v11);
                        damp_dir -= r1;

                        ndamp_dir = Vec3Length(&damp_dir.x);
                        v12 = ndamp_dir;
                        if (ndamp_dir < 0.00009999999747378752)
                        {
                            //v13 = pulse_sum_normal::get_relative_velocity_change_dir(psn, &r1);
                            v13 = psn->get_relative_velocity_change_dir(&r1);
                            damp_dir.x = v13->x;
                            damp_dir.y = v13->y;
                            damp_dir.z = v13->z;
                            pos_adjustb = phys_dot(&damp_dir, &ud);
                            //v14 = operator*(&r1, pos_adjustb, &ud);
                            r1 = ud * pos_adjustb;
                            //phys_vec3::operator-=(&damp_dir, v14);
                            damp_dir -= r1;
                            ndamp_dir = Vec3Length(&damp_dir.x);
                            v12 = ndamp_dir;
                        }
                        if (v12 > 0.00009999999747378752)
                        {
                            *(float *)&ps_min = 1.0 / v12;
                            //phys_vec3::operator*=(&damp_dir, *(float *)&ps_min);
                            damp_dir *= (1.0f / v12);
                            //v15 = pulse_sum_constraint_solver::create_pulse_sum_normal(psys);
                            v15 = psys->create_pulse_sum_normal();
                            if ((this->m_flags & 2) != 0)
                            {
                                v16 = rbint::sub_pos(&b1_pt, this->b1, &b2_pt);
                                r1.x = v16->x;
                                r1.y = v16->y;
                                r1.z = v16->z;
                                r2.x = b2_r.x;
                                r2.y = b2_r.y;
                                z = b2_r.z;
                            }
                            else
                            {
                                v18 = this->b2;
                                r1.x = b1_r.x;
                                r1.y = b1_r.y;
                                r1.z = b1_r.z;
                                v19 = rbint::sub_pos(&b2_pt, v18, &b1_pt);
                                r2.x = v19->x;
                                r2.y = v19->y;
                                z = v19->z;
                            }
                            r2.z = z;
                            //pulse_sum_normal::set(
                                v15->set(
                                this->b1,
                                &r1,
                                this->b2,
                                &r2,
                                &damp_dir,
                                &this->m_ps_cache_list[2],
                                &PHYS_ZERO_VEC);
                            *(float *)&ps_min = 1.0 / (this->m_damp_coef * delta_t);
                            v15->m_right_side = 0.0;
                            v15->m_big_dirt = 0.0;
                            v20 = *(float *)&ps_min;
                            v15->m_cfm = *(float *)&ps_min;
                            v15->m_denom = v20 + v15->m_denom;
                            v15->m_pulse_sum_min = -10000000.0;
                            v15->m_pulse_sum_max = 10000000.0;
                        }
                    }
                }
            }
        }
    }
}

// local variable allocation has failed, the output may be wrong!
void rigid_body_constraint_angular_actuator::setup_constraint(
        pulse_sum_constraint_solver *psys,
        float delta_t)
{
    // aislop
    if (!this->m_enabled)
        return;

    rigid_body *b1 = this->b1;
    rigid_body *b2 = this->b2;

    //
    // Power limit
    //
    float ps_limit = this->m_power * delta_t * this->m_power_scale;
    float ps_min = -ps_limit;

    //
    // Build target orientation in world space
    //
    phys_mat44 target_ws;
    phys_multiply_mat(
        &target_ws,
        &b1->m_mat,
        &this->m_target_mat);

    //
    // Compute angular velocity in world space
    //
    phys_vec3 a_vel;
    phys_multiply(
        &a_vel,
        &b1->m_mat,
        &this->m_a_vel);

    float inv_dt = -1.0f / delta_t;

    //
    // ---------------------------------------------------------------------
    // AXIS 0
    // ---------------------------------------------------------------------
    //
    pulse_sum_angular *psa0 =
        psys->create_pulse_sum_angular(
            b1,
            &target_ws.x,
            b2,
            &b2->m_mat.x,
            &target_ws.y,
            &this->m_ps_cache_list[0]);

    psa0->m_pulse_sum_min = ps_min;
    psa0->m_pulse_sum_max = ps_limit;

    psa0->m_big_dirt =
        psa0->get_pos() * inv_dt;

    psa0->m_right_side = 0.0f;
    psa0->m_cfm = 0.0f;
    psa0->m_denom += 0.0f;

    psa0->m_right_side -=
        target_ws.y.x * a_vel.x +
        target_ws.y.y * a_vel.y +
        target_ws.y.z * a_vel.z;

    //
    // ---------------------------------------------------------------------
    // AXIS 1
    // ---------------------------------------------------------------------
    //
    pulse_sum_angular *psa1 =
        psys->create_pulse_sum_angular(
            b1,
            &target_ws.y,
            b2,
            &b2->m_mat.y,
            &target_ws.z,
            &this->m_ps_cache_list[1]);

    psa1->m_pulse_sum_min = ps_min;
    psa1->m_pulse_sum_max = ps_limit;

    psa1->m_big_dirt =
        psa1->get_pos() * inv_dt;

    psa1->m_right_side = 0.0f;
    psa1->m_cfm = 0.0f;
    psa1->m_denom += 0.0f;

    psa1->m_right_side -=
        target_ws.z.x * a_vel.x +
        target_ws.z.y * a_vel.y +
        target_ws.z.z * a_vel.z;

    //
    // ---------------------------------------------------------------------
    // AXIS 2
    // ---------------------------------------------------------------------
    //
    pulse_sum_angular *psa2 =
        psys->create_pulse_sum_angular(
            b1,
            &target_ws.z,
            b2,
            &b2->m_mat.z,
            &target_ws.x,
            &this->m_ps_cache_list[2]);

    psa2->m_pulse_sum_min = ps_min;
    psa2->m_pulse_sum_max = ps_limit;

    psa2->m_big_dirt =
        psa2->get_pos() * inv_dt;

    psa2->m_right_side = 0.0f;
    psa2->m_cfm = 0.0f;
    psa2->m_denom += 0.0f;

    psa2->m_right_side -=
        target_ws.x.x * a_vel.x +
        target_ws.x.y * a_vel.y +
        target_ws.x.z * a_vel.z;
}

void rigid_body_constraint_upright::setup_constraint(
        pulse_sum_constraint_solver *psys,
        float delta_t)
{
    rigid_body *b1; // edi
    pulse_sum_angular *v6; // esi
    double v7; // st7
    double pos; // st7
    phys_vec3 v9; // [esp-Ch] [ebp-3Ch] BYREF
    phys_vec3 b1_lean_axis_4; // [esp+4h] [ebp-2Ch] BYREF
    float v11; // [esp+20h] [ebp-10h]
    //_UNKNOWN *v12; // [esp+24h] [ebp-Ch]
    //pulse_sum_constraint_solver *psysa; // [esp+28h] [ebp-8h]
    //int vars0; // [esp+30h] [ebp+0h]
    //
    //v12 = a2;
    //psysa = (pulse_sum_constraint_solver *)vars0;
    if (this->m_enabled)
    {
        b1 = this->b1;
        phys_multiply(&b1_lean_axis_4, &this->b1->m_mat, &this->m_b1_forward_axis_loc);
        phys_multiply(&v9, &b1->m_mat, &this->m_b1_lean_axis_loc);
        v6 = //pulse_sum_constraint_solver::create_pulse_sum_angular(
            psys->create_pulse_sum_angular(
            b1,
            &v9,
            this->b2,
            &this->m_b2_up_axis_loc,
            &b1_lean_axis_4,
            this->m_ps_cache_list);
        v6->m_pulse_sum_min = -10000000.0;
        v6->m_pulse_sum_max = 10000000.0;
        v7 = 0.016666668;
        if (delta_t > 0.016666668)
            v7 = delta_t;
        v11 = v7;
        //pos = pulse_sum_angular::get_pos(v6);
        pos = v6->get_pos();
        v6->m_big_dirt = pos * (-0.5 / v11);
        v6->m_right_side = 0.0;
        v6->m_cfm = 0.0;
    }
}

void __thiscall pulse_sum_normal::setup_vel_uni_standard_pos_adjust(
        float delta_t,
        float pos_adjust,
        float max_penalty_restitution_vel)
{
  double v4; // st7
  int savedregs; // [esp+4h] [ebp+0h] BYREF
  float delta_tc; // [esp+Ch] [ebp+8h]
  float delta_ta; // [esp+Ch] [ebp+8h]
  float delta_tb; // [esp+Ch] [ebp+8h]

  v4 = 0.016666668;
  if ( delta_t > 0.016666668 )
    v4 = delta_t;
  delta_tc = v4;
  delta_ta = -((pulse_sum_normal::get_pos() + pos_adjust) / delta_tc);
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

void __thiscall pulse_sum_angular::setup_vel_uni_standard(
        float delta_t,
        float max_penalty_restitution_vel)
{
  double v3; // st7
  float delta_tc; // [esp+Ch] [ebp+8h]
  float delta_ta; // [esp+Ch] [ebp+8h]
  float delta_tb; // [esp+Ch] [ebp+8h]

  v3 = 0.016666668;
  if ( delta_t > 0.016666668 )
    v3 = delta_t;
  delta_tc = v3;
  delta_ta = -pulse_sum_angular::get_pos() / delta_tc;
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

pulse_sum_normal *__thiscall pulse_sum_constraint_solver::create_pulse_sum_normal()
{
  pulse_sum_normal **v2; // edi
  pulse_sum_normal **m_last_next_ptr; // ecx

  //v2 = (pulse_sum_normal **)phys_transient_allocator::allocate(&this->m_solver_memory_allocator, 160, 16, 0, SOLVER_MEMORY_ALLOCATOR_ERROR_MSG);
  v2 = (pulse_sum_normal **)this->m_solver_memory_allocator.allocate(160, 16, 0, SOLVER_MEMORY_ALLOCATOR_ERROR_MSG);
  if ( !this->m_list_pulse_sum_normal.m_last_next_ptr
    && _tlAssert(
         "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\phys_mem.h",
         230,
         "m_last_next_ptr",
         "") )
  {
    __debugbreak();
  }
  *v2 = 0;
  m_last_next_ptr = this->m_list_pulse_sum_normal.m_last_next_ptr;
  ++this->m_list_pulse_sum_normal.m_alloc_count;
  *m_last_next_ptr = (pulse_sum_normal *)v2;
  this->m_list_pulse_sum_normal.m_last_next_ptr = v2;
  return (pulse_sum_normal *)v2;
}

void __thiscall pulse_sum_constraint_solver::create_point(
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
  pulse_sum_point *v11; // edi
  pulse_sum_point **m_last_next_ptr; // ecx
  int savedregs; // [esp+18h] [ebp+0h] BYREF

  //v11 = (pulse_sum_point *)phys_transient_allocator::allocate(&this->m_solver_memory_allocator, 272, 16, 0, SOLVER_MEMORY_ALLOCATOR_ERROR_MSG);
  v11 = (pulse_sum_point *)this->m_solver_memory_allocator.allocate(272, 16, 0, SOLVER_MEMORY_ALLOCATOR_ERROR_MSG);
  if ( !this->m_list_pulse_sum_point.m_last_next_ptr
    && _tlAssert(
         "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\phys_mem.h",
         230,
         "m_last_next_ptr",
         "") )
  {
    __debugbreak();
  }
  v11->m_next_link = 0;
  m_last_next_ptr = this->m_list_pulse_sum_point.m_last_next_ptr;
  ++this->m_list_pulse_sum_point.m_alloc_count;
  *m_last_next_ptr = v11;
  this->m_list_pulse_sum_point.m_last_next_ptr = &v11->m_next_link;
  //pulse_sum_point::set(v11, (int)&savedregs, b1, b1_r, b2, b2_r, ps_cache, delta_t, is_spring, spring_k, damp_k);
  v11->set(b1, b1_r, b2, b2_r, ps_cache, delta_t, is_spring, spring_k, damp_k);
}

pulse_sum_angular *__thiscall pulse_sum_constraint_solver::create_pulse_sum_angular(
        rigid_body *const b1,
        const phys_vec3 *b1_r,
        rigid_body *const b2,
        const phys_vec3 *b2_r,
        const phys_vec3 *ud,
        pulse_sum_cache *const ps_cache)
{
  pulse_sum_angular *v8; // edi
  pulse_sum_angular **m_last_next_ptr; // ecx
  int savedregs; // [esp+8h] [ebp+0h] BYREF

  //v8 = (pulse_sum_angular *)phys_transient_allocator::allocate(&this->m_solver_memory_allocator, 144, 16, 0, SOLVER_MEMORY_ALLOCATOR_ERROR_MSG);
  v8 = (pulse_sum_angular *)this->m_solver_memory_allocator.allocate(144, 16, 0, SOLVER_MEMORY_ALLOCATOR_ERROR_MSG);
  if ( !this->m_list_pulse_sum_angular.m_last_next_ptr
    && _tlAssert(
         "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\phys_mem.h",
         230,
         "m_last_next_ptr",
         "") )
  {
    __debugbreak();
  }
  v8->m_next_link = 0;
  m_last_next_ptr = this->m_list_pulse_sum_angular.m_last_next_ptr;
  ++this->m_list_pulse_sum_angular.m_alloc_count;
  *m_last_next_ptr = v8;
  this->m_list_pulse_sum_angular.m_last_next_ptr = &v8->m_next_link;
  //pulse_sum_angular::set(v8, (int)&savedregs, b1, b1_r, b2, b2_r, ud, ps_cache);
  v8->set(b1, b1_r, b2, b2_r, ud, ps_cache);
  return v8;
}

void __thiscall pulse_sum_constraint_solver::create_hinge(
        rigid_body *const b1,
        const phys_vec3 *b1_axis,
        rigid_body *const b2,
        const phys_vec3 *b2_axis,
        const phys_vec3 *a1,
        const phys_vec3 *a2,
        pulse_sum_cache *const ps_cache,
        float delta_t)
{
    pulse_sum_angular *psa; // esi
    double v12; // st7
    pulse_sum_angular *v13; // esi
    double v14; // st7
    float b2_axisa; // [esp+20h] [ebp+14h]
    float ps_cachea; // [esp+2Ch] [ebp+20h]

    psa = pulse_sum_constraint_solver::create_pulse_sum_angular(b1, b1_axis, b2, b2_axis, a1, ps_cache);
    psa->m_pulse_sum_min = -10000000.0;
    psa->m_pulse_sum_max = 10000000.0;
    v12 = 0.016666668;
    if (delta_t > 0.016666668)
        v12 = delta_t;
    ps_cachea = v12;
    //psa->m_big_dirt = pulse_sum_angular::get_pos(psa) * (-0.5 / ps_cachea);
    psa->m_big_dirt = psa->get_pos() * (-0.5 / ps_cachea);
    psa->m_right_side = 0.0;
    psa->m_cfm = 0.0;
    v13 = pulse_sum_constraint_solver::create_pulse_sum_angular(b1, b1_axis, b2, b2_axis, a2, ps_cache + 1);
    v13->m_pulse_sum_min = -10000000.0;
    v13->m_pulse_sum_max = 10000000.0;
    v14 = 0.016666668;
    if (delta_t > 0.016666668)
        v14 = delta_t;
    b2_axisa = v14;
    //v13->m_big_dirt = pulse_sum_angular::get_pos(v13) * (-0.5 / b2_axisa);
    v13->m_big_dirt = v13->get_pos() * (-0.5 / b2_axisa);
    v13->m_right_side = 0.0;
    v13->m_cfm = 0.0;
}

