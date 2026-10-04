#include "rbc_def_custom.h"
#include "rbc_def_generic.h"
#include <physics/physics_system_internal.h>
#include <physics/phys_constraint_solver_multithreaded.h>

float psa_spring_d = 5.0;
float psa_spring_k = 100.0;
float psa_spring_d_0 = 5.0;
float psa_spring_k_0 = 100.0;
float psn_spring_d = 12.0;
float psn_spring_k = 50.0;


const phys_vec3 * rbint::mul_L(
        phys_vec3 *result,
        const rigid_body *rb,
        const phys_vec3 *t)
{
  phys_vec3 v5; // [esp-20h] [ebp-2Ch] BYREF
  float v6; // [esp-10h] [ebp-1Ch]
  float v7; // [esp-Ch] [ebp-18h]
  float v8; // [esp-8h] [ebp-14h]
  //int v9; // [esp+0h] [ebp-Ch]
  //void *v10; // [esp+4h] [ebp-8h]
  //void *retaddr; // [esp+Ch] [ebp+0h]
  //
  //v9 = a1;
  //v10 = retaddr;
  v6 = rb->m_mat.x.y * t->y + rb->m_mat.x.x * t->x + rb->m_mat.x.z * t->z;
  v7 = rb->m_mat.y.y * t->y + rb->m_mat.y.x * t->x + rb->m_mat.y.z * t->z;
  v8 = rb->m_mat.z.y * t->y + t->x * rb->m_mat.z.x + rb->m_mat.z.z * t->z;
  v5.x = v6 / rb->m_inv_inertia.x;
  v5.y = v7 / rb->m_inv_inertia.y;
  v5.z = v8 / rb->m_inv_inertia.z;
  phys_multiply(result, &rb->m_mat, &v5);
  return result;
}

void __thiscall rigid_body_constraint_custom_orientation::setup_constraint(
        pulse_sum_constraint_solver *psys,
        float delta_t)
{
    double v3; // st7
    bool v6; // c0
    phys_mat44 *p_m_mat; // ecx
    phys_mat44 *v8; // edi
    pulse_sum_angular *psa; // eax
    pulse_sum_angular *psa2; // eax
    pulse_sum_angular *psa3; // eax
    pulse_sum_angular *psa4; // eax
    double v13; // st7
    double pos; // st7
    double v15; // st6
    double v16; // st5
    pulse_sum_angular *psa5; // eax
    float *v18; // ebx
    double v19; // st7
    double v20; // st7
    double v21; // st6
    phys_vec3 *b2_r; // [esp+Ch] [ebp-Ch]
    float roll; // [esp+10h] [ebp-8h]
    const phys_mat44 *b2_mat; // [esp+14h] [ebp-4h]
    float b2_matb; // [esp+14h] [ebp-4h]
    phys_mat44 *b2_mata; // [esp+14h] [ebp-4h]
    float pitchd; // [esp+20h] [ebp+8h]
    float pitche; // [esp+20h] [ebp+8h]
    float pitchf; // [esp+20h] [ebp+8h]
    float pitchg; // [esp+20h] [ebp+8h]
    float pitchh; // [esp+20h] [ebp+8h]
    float pitch; // [esp+20h] [ebp+8h]
    float pitcha; // [esp+20h] [ebp+8h]
    float pitchi; // [esp+20h] [ebp+8h]
    float pitchb; // [esp+20h] [ebp+8h]
    float pitchc; // [esp+20h] [ebp+8h]

    v3 = 0.0;
    v6 = this->m_torque_resistance_pitch_roll > 0.0;
    p_m_mat = &this->b2->m_mat;
    v8 = &this->b1->m_mat;
    b2_mat = p_m_mat;
    if (v6)
    {
        pitchd = this->m_torque_resistance_pitch_roll * 10.0;
        psa = //pulse_sum_constraint_solver::create_pulse_sum_angular(
            psys->create_pulse_sum_angular(
            this->b1,
            &this->b1->m_mat.z,
            this->b2,
            &p_m_mat->z,
            &this->b1->m_mat.y,
            this->m_ps_cache_list);
        psa->m_pulse_sum_min = -10000000.0;
        psa->m_pulse_sum_max = 10000000.0;
        pitche = 1.0 / (pitchd * delta_t);
        psa->m_right_side = 0.0;
        psa->m_big_dirt = 0.0;
        psa->m_cfm = pitche;
        psa->m_denom = pitche + psa->m_denom;
        psa2 = //pulse_sum_constraint_solver::create_pulse_sum_angular(
            psys->create_pulse_sum_angular(
            this->b1,
            &v8->z,
            this->b2,
            &b2_mat->z,
            &v8->x,
            &this->m_ps_cache_list[1]);
        p_m_mat = (phys_mat44 *)b2_mat;
        psa2->m_pulse_sum_min = -10000000.0;
        psa2->m_pulse_sum_max = 10000000.0;
        v3 = 0.0;
        psa2->m_right_side = 0.0;
        psa2->m_big_dirt = 0.0;
        psa2->m_cfm = pitche;
        psa2->m_denom = pitche + psa2->m_denom;
    }
    if (v3 < this->m_torque_resistance_yaw)
    {
        pitchf = this->m_torque_resistance_yaw * 10.0;
        psa3 = //pulse_sum_constraint_solver::create_pulse_sum_angular(
            psys->create_pulse_sum_angular(
            this->b1,
            &v8->x,
            this->b2,
            &p_m_mat->x,
            &v8->z,
            &this->m_ps_cache_list[2]);
        psa3->m_pulse_sum_min = -10000000.0;
        p_m_mat = (phys_mat44 *)b2_mat;
        psa3->m_pulse_sum_max = 10000000.0;
        pitchg = 1.0 / (pitchf * delta_t);
        v3 = 0.0;
        psa3->m_right_side = 0.0;
        psa3->m_big_dirt = 0.0;
        psa3->m_cfm = pitchg;
        psa3->m_denom = pitchg + psa3->m_denom;
    }
    if (this->m_active && v3 < this->m_upright_strength)
    {
        pitchh = fabs(this->b1->m_mat.y.z);
        roll = pitchh;
        pitch = fabs(this->b1->m_mat.x.z);
        if (v3 > this->b1->m_mat.z.z)
        {
            pitch = 1.0;
            roll = 1.0;
        }
        b2_r = &p_m_mat->z;
        b2_matb = this->m_upright_strength * 30.0 * delta_t;
        pitcha = b2_matb * pitch;
        psa4 = //pulse_sum_constraint_solver::create_pulse_sum_angular(
            psys->create_pulse_sum_angular(
            this->b1,
            &v8->z,
            this->b2,
            &p_m_mat->z,
            &v8->y,
            &this->m_ps_cache_list[3]);
        b2_mata = (phys_mat44 *)psa4;
        if (this->m_no_orientation_correction)
        {
            v13 = 0.0;
            if (v8->x.z >= 0.0)
            {
                psa4->m_pulse_sum_min = 0.0;
                v13 = pitcha;
            }
            else
            {
                psa4->m_pulse_sum_min = -pitcha;
            }
            psa4->m_pulse_sum_max = v13;
            //pos = pulse_sum_angular::get_pos(psa4);
            pos = psa4->get_pos();
            v15 = delta_t;
            v16 = -0.0 / delta_t;
        }
        else
        {
            psa4->m_pulse_sum_min = -pitcha;
            psa4->m_pulse_sum_max = pitcha;
            //pos = pulse_sum_angular::get_pos(psa4);
            pos = psa4->get_pos();
            v15 = delta_t;
            v16 = -1.0 / delta_t;
        }
        b2_mata[1].z.w = pos * v16;
        b2_mata[1].w.x = 0.0;
        b2_mata[1].w.y = 0.0;
        b2_mata[1].w.z = b2_mata[1].w.z + 0.0;
        pitchi = v15 * (this->m_upright_strength * 100.0);
        pitchb = pitchi * roll;
        psa5 = //pulse_sum_constraint_solver::create_pulse_sum_angular(
            psys->create_pulse_sum_angular(
            this->b1,
            &v8->z,
            this->b2,
            b2_r,
            &v8->x,
            &this->m_ps_cache_list[4]);
        v18 = (float *)psa5;
        if (this->m_no_orientation_correction)
        {
            pitchc = pitchb * 0.5;
            v19 = 0.0;
            if (v8->y.z <= 0.0)
            {
                psa5->m_pulse_sum_min = 0.0;
                v19 = pitchc;
            }
            else
            {
                psa5->m_pulse_sum_min = -pitchc;
            }
            psa5->m_pulse_sum_max = v19;
            //v20 = pulse_sum_angular::get_pos(psa5);
            v20 = psa5->get_pos();
            v21 = -0.0 / delta_t;
        }
        else
        {
            psa5->m_pulse_sum_min = -pitchb;
            psa5->m_pulse_sum_max = pitchb;
            //v20 = pulse_sum_angular::get_pos(psa5);
            v20 = psa5->get_pos();
            v21 = -1.0 / delta_t;
        }
        v18[27] = v20 * v21;
        v18[28] = 0.0;
        v18[29] = 0.0;
        v18[30] = v18[30] + 0.0;
    }
}

// local variable allocation has failed, the output may be wrong!
void rigid_body_constraint_custom_path::setup_constraint(pulse_sum_constraint_solver *psys, float delta_t)
{
    rigid_body *b1; // ecx
    double v6; // st6
    double v7; // st4
    double v8; // st6
    pulse_sum_normal *psn; // edi
    double pos; // st7
    double v11; // st7
    pulse_sum_normal *pulse_sum_normal; // edi
    double v13; // st7
    double v14; // st7
    rigid_body *v15; // edi
    rigid_body *b2; // eax
    float x; // ecx
    const phys_vec3 *v18; // eax
    double v19; // st7
    pulse_sum_angular *v20; // edi
    double v21; // st7
    double v22; // st6
    double v23; // st7
    float v24; // ecx
    phys_vec3 *p_z; // eax
    float z; // edx
    float w; // eax
    double v28; // st6
    double v29; // st5
    double v30; // rt0
    const phys_vec3 *v31; // eax
    rigid_body *v32; // ecx
    double v33; // st7
    pulse_sum_angular *v34; // esi
    double v35; // st7
    double v36; // st6
    rigid_body *v37; // [esp-2Ch] [ebp-ACh]
    phys_vec3 v38; // [esp-Ch] [ebp-8Ch] BYREF
    phys_vec3 b1_r_4; // [esp+4h] [ebp-7Ch] BYREF
    phys_vec3 b2_r_4; // [esp+14h] [ebp-6Ch] BYREF
    phys_vec3 b1_r_; // [esp+24h] [ebp-5Ch] BYREF
    phys_vec3 b1_r__4; // [esp+34h] [ebp-4Ch] BYREF
    phys_vec3 b1_ud; // [esp+44h] [ebp-3Ch] BYREF
    float v44[2]; // [esp+5Ch] [ebp-24h]
    float v45; // [esp+64h] [ebp-1Ch]
    float y; // [esp+68h] [ebp-18h]
    float psysa[2]; // [esp+6Ch] [ebp-14h]
    //int v48[2]; // [esp+74h] [ebp-Ch] BYREF
    //int vars0_1; // [esp+80h] [ebp+0h]
    //
    //v48[0] = a2;
    //v48[1] = vars0_1;
    b1_r_.x = 0.0;
    b1 = this->b1;
    b1_r_.y = 0.0;
    b1_r_.z = 0.0;
    phys_multiply(&v38, &b1->m_mat, &this->b1_r_loc);
    phys_multiply(&b1_r_4, &this->b2->m_mat, &b1_r_);
    if (!this->m_urb && _tlAssert("source/rbc_def_custom.cpp", 148, "m_urb", ""))
        __debugbreak();
    psysa[1] = 1.0 / this->b1->m_inv_mass;
    v6 = psysa[1];
    psysa[1] = psn_spring_k * psysa[1];
    psysa[1] = psysa[1] * delta_t;
    v7 = v6 * psn_spring_d;
    v8 = psysa[1] * delta_t;
    psysa[1] = v7;
    psysa[1] = delta_t * psysa[1];
    psysa[1] = 1.0 / (psysa[1] + v8);
    v44[1] = v8 * psysa[1];
//    psn = pulse_sum_constraint_solver::create_pulse_sum_normal(psys);
    psn = psys->create_pulse_sum_normal();
    //pulse_sum_normal::set(
        psn->set(
        this->b1,
        &v38,
        this->b2,
        &b1_r_4,
        &PHYS_X_VEC,
        this->m_list_psc,
        &PHYS_ZERO_VEC);
    *(double *)v44 = -v44[1] / delta_t;
    //pos = pulse_sum_normal::get_pos(psn, (int)v48);
    pos = psn->get_pos();
    psn->m_right_side = pos * *(double *)v44;
    psn->m_big_dirt = 0.0;
    v11 = psysa[1];
    psn->m_cfm = psysa[1];
    psn->m_denom = v11 + psn->m_denom;
    psn->m_pulse_sum_min = -10000000.0;
    psn->m_pulse_sum_max = 10000000.0;
    //pulse_sum_normal = pulse_sum_constraint_solver::create_pulse_sum_normal(psys);
    pulse_sum_normal = psys->create_pulse_sum_normal();
    //pulse_sum_normal::set(
        pulse_sum_normal->set(
        this->b1,
        &v38,
        this->b2,
        &b1_r_4,
        &PHYS_Y_VEC,
        &this->m_list_psc[1],
        &PHYS_ZERO_VEC);
    //v13 = pulse_sum_normal::get_pos(pulse_sum_normal, (int)v48);
    v13 = pulse_sum_normal->get_pos();
    pulse_sum_normal->m_right_side = v13 * *(double *)v44;
    pulse_sum_normal->m_big_dirt = 0.0;
    v14 = psysa[1];
    pulse_sum_normal->m_cfm = psysa[1];
    pulse_sum_normal->m_denom = v14 + pulse_sum_normal->m_denom;
    pulse_sum_normal->m_pulse_sum_min = -10000000.0;
    pulse_sum_normal->m_pulse_sum_max = 10000000.0;
    v15 = this->b1;
    b1_r__4.x = this->b1->m_mat.x.x;
    b1_r__4.y = v15->m_mat.x.y;
    b2 = this->b2;
    b1_r__4.z = v15->m_mat.x.z;
    b2 = (rigid_body *)((char *)b2 + 64);
    b1_r__4.w = v15->m_mat.x.w;
    x = b2->m_last_position.x;
    y = b2->m_last_position.y;
    v45 = x;
    psysa[0] = b2->m_last_position.z;
    psysa[1] = b2->m_last_position.w;
    v44[1] = x * b1_r__4.x + y * b1_r__4.y + psysa[0] * b1_r__4.z;
    b1_r_.x = x * v44[1];
    b1_r_.y = y * v44[1];
    b1_r_.z = psysa[0] * v44[1];
    b1_ud.x = b1_r__4.x - b1_r_.x;
    b1_ud.y = b1_r__4.y - b1_r_.y;
    b1_ud.z = b1_r__4.z - b1_r_.z;
    v44[1] = b1_ud.x * b1_ud.x + b1_ud.y * b1_ud.y + b1_ud.z * b1_ud.z;
    v44[1] = sqrt(v44[1]);
    if (v44[1] <= 0.00009999999747378752)
    {
        v23 = 0.0;
    }
    else
    {
        v44[1] = 1.0 / v44[1];
        b1_ud.x = b1_ud.x * v44[1];
        b1_ud.y = b1_ud.y * v44[1];
        b1_ud.z = v44[1] * b1_ud.z;
        b1_r_.x = b1_ud.z * y - b1_ud.y * psysa[0];
        b1_r_.y = psysa[0] * b1_ud.x - b1_ud.z * v45;
        b1_r_.z = v45 * b1_ud.y - b1_ud.x * y;
        v18 = rbint::mul_L(&b2_r_4, v15, &b1_r_);
        v44[1] = v18->y * b1_r_.y + v18->x * b1_r_.x + v18->z * b1_r_.z;
        v19 = v44[1];
        v44[1] = psa_spring_k_0 * v44[1];
        v44[1] = v44[1] * delta_t;
        *(double *)psysa = v44[1] * delta_t;
        v44[1] = v19 * psa_spring_d_0;
        v44[1] = delta_t * v44[1];
        v44[1] = 1.0 / (*(double *)psysa + v44[1]);
        //v20 = pulse_sum_constraint_solver::create_pulse_sum_angular(
        v20 = psys->create_pulse_sum_angular(
            this->b1,
            &b1_r__4,
            this->b2,
            &b1_ud,
            &b1_r_,
            &this->m_list_psc[2]);
        //v21 = pulse_sum_angular::get_pos(v20);
        v21 = v20->get_pos();
        v22 = v44[1];
        v44[1] = *(double *)psysa * v44[1];
        v20->m_right_side = v21 * (-v44[1] / delta_t);
        v20->m_big_dirt = 0.0;
        v23 = 0.0;
        v20->m_cfm = v22;
        v20->m_denom = v22 + v20->m_denom;
        v20->m_pulse_sum_min = -10000000.0;
        v20->m_pulse_sum_max = 10000000.0;
    }
    v24 = this->b1->m_mat.z.y;
    p_z = &this->b1->m_mat.z;
    b1_ud.x = p_z->x;
    z = p_z->z;
    w = p_z->w;
    b2_r_4.x = v23;
    b2_r_4.y = v23;
    b1_ud.y = v24;
    b1_ud.z = z;
    b2_r_4.z = 1.0;
    b1_ud.w = w;
    v44[1] = v24 * 0.0 + b1_ud.x * 0.0 + z;
    b1_r_.x = b1_ud.x * v44[1];
    b1_r_.y = v24 * v44[1];
    b1_r_.z = v44[1] * z;
    b1_r__4.x = 0.0 - b1_r_.x;
    b1_r__4.y = 0.0 - b1_r_.y;
    b1_r__4.z = 1.0 - b1_r_.z;
    v44[1] = b1_r__4.x * b1_r__4.x + b1_r__4.y * b1_r__4.y + b1_r__4.z * b1_r__4.z;
    v44[1] = sqrt(v44[1]);
    if (v44[1] >= 0.00009999999747378752)
    {
        v28 = b1_ud.x;
        v45 = b1_ud.x * 0.5;
        y = b1_ud.y * 0.5;
        v29 = b1_ud.z;
        psysa[0] = 0.5 * b1_ud.z;
        v30 = b1_ud.y;
        v44[1] = 0.8660253882408142 / v44[1];
        b1_ud.x = b1_r__4.x * v44[1];
        b1_ud.y = b1_r__4.y * v44[1];
        b1_ud.z = v44[1] * b1_r__4.z;
        b1_r_.x = b1_ud.x + v45;
        b1_r_.y = b1_ud.y + y;
        b1_r_.z = b1_ud.z + psysa[0];
        b1_ud.x = b1_r_.y * v29 - b1_r_.z * v30;
        b1_ud.y = b1_r_.z * v28 - v29 * b1_r_.x;
        b1_ud.z = b1_r_.x * v30 - v28 * b1_r_.y;
        b1_r__4.x = b1_ud.x * 1.154700517654419;
        b1_r__4.y = b1_ud.y * 1.154700517654419;
        b1_r__4.z = 1.154700517654419 * b1_ud.z;
        PHYS_ASSERT_UNIT(&b1_r__4);
        v31 = rbint::mul_L(&b1_ud, this->b1, &b1_r__4);
        v32 = this->b1;
        v37 = this->b2;
        v44[1] = v31->y * b1_r__4.y + v31->x * b1_r__4.x + v31->z * b1_r__4.z;
        v33 = v44[1];
        v44[1] = v44[1] * psa_spring_k;
        v44[1] = v44[1] * delta_t;
        *(double *)psysa = v44[1] * delta_t;
        v44[1] = v33 * psa_spring_d;
        v44[1] = delta_t * v44[1];
        v44[1] = 1.0 / (*(double *)psysa + v44[1]);
        //v34 = pulse_sum_constraint_solver::create_pulse_sum_angular(
        v34 = psys->create_pulse_sum_angular(
            v32,
            &b1_r_,
            v37,
            &b2_r_4,
            &b1_r__4,
            &this->m_list_psc[3]);
        v34->m_pulse_sum_min = -10000000.0;
        v34->m_pulse_sum_max = 0.0;
        //v35 = pulse_sum_angular::get_pos(v34);
        v35 = v34->get_pos();
        v36 = v44[1];
        v44[1] = *(double *)psysa * v44[1];
        v34->m_right_side = v35 * (-v44[1] / delta_t);
        v34->m_big_dirt = 0.0;
        v34->m_cfm = v36;
        v34->m_denom = v36 + v34->m_denom;
    }
}

