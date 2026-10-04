#include "phys_util.h"

Phys_UnitQuaternion *__cdecl Phys_GetQuaternion(Phys_UnitQuaternion *result, const phys_mat44 *M)
{
    double v3; // st7
    double v4; // st7
    double y; // st7
    int v6; // esi
    double x; // st6
    int v8; // edi
    Phys_UnitQuaternion *v9; // eax
    float q[4]; // [esp+4h] [ebp-1Ch]
    float a; // [esp+14h] [ebp-Ch]
    float d; // [esp+18h] [ebp-8h]
    int k; // [esp+1Ch] [ebp-4h]
    float s; // [esp+2Ch] [ebp+Ch]
    float sc; // [esp+2Ch] [ebp+Ch]
    float sd; // [esp+2Ch] [ebp+Ch]
    float se; // [esp+2Ch] [ebp+Ch]
    float sa; // [esp+2Ch] [ebp+Ch]
    float sf; // [esp+2Ch] [ebp+Ch]
    float sb; // [esp+2Ch] [ebp+Ch]

    s = M->x.x + M->y.y + M->z.z;
    if ( s <= 0.0 )
    {
        y = M->y.y;
        v6 = 0;
        x = M->x.x;
        v8 = 1;
        k = 2;
        if ( x < y )
        {
            v6 = 1;
            v8 = 2;
            *(float *)&k = 0.0;
        }
        if ( *(&M->x.x + 5 * v6) < (double)M->z.z )
        {
            v6 = 2;
            v8 = 0;
            k = 1;
        }
        sf = *(&M->x.x + 5 * v6) - *(&M->x.x + 5 * v8) - *(&M->x.x + 5 * k) + 1.0;
        sb = sqrt(sf);
        q[v6 + 1] = sb * 0.5;
        if ( 0.0 != sb )
            sb = 0.5 / sb;
        if ( (unsigned int)k > 2
            && _tlAssert(
                     "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\old_phys_math.h",
                     32,
                     "i >= 0 && i < 3",
                     "") )
        {
            __debugbreak();
        }
        q[0] = (*(&M->x.x + 4 * k + v8) - *(&M->x.x + 4 * v8 + k)) * sb;
        q[v8 + 1] = (*(&M->x.x + 4 * v8 + v6) + *(&M->x.x + 4 * v6 + v8)) * sb;
        if ( (unsigned int)k > 2
            && _tlAssert(
                     "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\old_phys_math.h",
                     32,
                     "i >= 0 && i < 3",
                     "") )
        {
            __debugbreak();
        }
        q[k + 1] = (*(&M->x.x + 4 * k + v6) + *(&M->x.x + 4 * v6 + k)) * sb;
        a = q[0];
        sa = q[1];
        k = SLODWORD(q[2]);
        v4 = q[3];
    }
    else
    {
        sc = s + 1.0;
        sd = sqrt(sc);
        a = sd * 0.5;
        se = 0.5 / sd;
        v3 = se;
        sa = (M->z.y - M->y.z) * se;
        *(float *)&k = (M->x.z - M->z.x) * v3;
        v4 = v3 * (M->y.x - M->x.y);
    }
    v9 = result;
    d = v4;
    result->x = -sa;
    result->y = -*(float *)&k;
    result->z = -d;
    result->w = a;
    return v9;
}

void __cdecl nuge::get_ballistic_info(
                rigid_body *const *list_rigid_body,
                int rbodies_count,
                phys_vec3 *center_of_mass,
                phys_vec3 *total_momentum,
                float *total_mass)
{
    phys_vec3 *v5; // ecx
    phys_vec3 *v6; // eax
    int v7; // edx
    int v8; // edi
    double v9; // st7
    rigid_body *const *v10; // edi
    float *v11; // edx
    float *v12; // edx
    float *v13; // edx
    float *v14; // edx
    rigid_body *v15; // edx
    float v16; // [esp-30h] [ebp-3Ch]
    float v17; // [esp-30h] [ebp-3Ch]
    float v18; // [esp-30h] [ebp-3Ch]
    float v19; // [esp-30h] [ebp-3Ch]
    float v20; // [esp-30h] [ebp-3Ch]
    float v21; // [esp-2Ch] [ebp-38h]
    float v22; // [esp-2Ch] [ebp-38h]
    float v23; // [esp-2Ch] [ebp-38h]
    float v24; // [esp-2Ch] [ebp-38h]
    float v25; // [esp-2Ch] [ebp-38h]
    float v26; // [esp-28h] [ebp-34h]
    float v27; // [esp-28h] [ebp-34h]
    float v28; // [esp-28h] [ebp-34h]
    float v29; // [esp-28h] [ebp-34h]
    float v30; // [esp-28h] [ebp-34h]
    float v31; // [esp-20h] [ebp-2Ch]
    float v32; // [esp-20h] [ebp-2Ch]
    float v33; // [esp-20h] [ebp-2Ch]
    float v34; // [esp-20h] [ebp-2Ch]
    float v35; // [esp-20h] [ebp-2Ch]
    float v36; // [esp-1Ch] [ebp-28h]
    float v37; // [esp-1Ch] [ebp-28h]
    float v38; // [esp-1Ch] [ebp-28h]
    float v39; // [esp-1Ch] [ebp-28h]
    float v40; // [esp-1Ch] [ebp-28h]
    float v41; // [esp-18h] [ebp-24h]
    float v42; // [esp-18h] [ebp-24h]
    float v43; // [esp-18h] [ebp-24h]
    float v44; // [esp-18h] [ebp-24h]
    float v45; // [esp-18h] [ebp-24h]
    int v46; // [esp-Ch] [ebp-18h]
    float v47; // [esp-Ch] [ebp-18h]
    unsigned int v48; // [esp-8h] [ebp-14h]
    float v49; // [esp-4h] [ebp-10h]
    float v50; // [esp-4h] [ebp-10h]
    float v51; // [esp-4h] [ebp-10h]
    float v52; // [esp-4h] [ebp-10h]
    float v53; // [esp-4h] [ebp-10h]

    v5 = center_of_mass;
    if ( ((unsigned __int8)center_of_mass & 0xF) != 0 )
    {
        if ( _tlAssert(
                     "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\phys_math.h",
                     444,
                     "uint(v) % PHYS_ALIGNOF(phys_vec3) == 0",
                     "") )
        {
            __debugbreak();
        }
        v5 = center_of_mass;
    }
    v6 = total_momentum;
    if ( ((unsigned __int8)total_momentum & 0xF) != 0 )
    {
        if ( _tlAssert(
                     "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\phys_math.h",
                     444,
                     "uint(v) % PHYS_ALIGNOF(phys_vec3) == 0",
                     "") )
        {
            __debugbreak();
        }
        v5 = center_of_mass;
        v6 = total_momentum;
    }
    v7 = rbodies_count;
    v5->x = PHYS_ZERO_VEC.x;
    v8 = 0;
    v5->y = PHYS_ZERO_VEC.y;
    v5->z = PHYS_ZERO_VEC.z;
    v6->x = PHYS_ZERO_VEC.x;
    v6->y = PHYS_ZERO_VEC.y;
    v6->z = PHYS_ZERO_VEC.z;
    *total_mass = 0.0;
    v9 = 1.0;
    if ( rbodies_count >= 4 )
    {
        v48 = ((unsigned int)(rbodies_count - 4) >> 2) + 1;
        v10 = list_rigid_body + 2;
        v46 = 4 * v48;
        do
        {
            v11 = (float *)*((unsigned int *)v10 - 2);
            if ( v11 )
            {
                v49 = 1.0 / v11[60];
                v31 = v49 * v11[24];
                v36 = v11[25] * v49;
                v41 = v11[26] * v49;
                v5->x = v5->x + v31;
                v5->y = v5->y + v36;
                v5->z = v5->z + v41;
                v16 = v11[36] * v49;
                v21 = v11[37] * v49;
                v26 = v11[38] * v49;
                v6->x = v6->x + v16;
                v6->y = v6->y + v21;
                v6->z = v26 + v6->z;
                *total_mass = v49 + *total_mass;
            }
            v12 = (float *)*((unsigned int *)v10 - 1);
            if ( v12 )
            {
                v50 = 1.0 / v12[60];
                v32 = v50 * v12[24];
                v37 = v12[25] * v50;
                v42 = v12[26] * v50;
                v5->x = v5->x + v32;
                v5->y = v5->y + v37;
                v5->z = v5->z + v42;
                v17 = v12[36] * v50;
                v22 = v12[37] * v50;
                v27 = v12[38] * v50;
                v6->x = v6->x + v17;
                v6->y = v6->y + v22;
                v6->z = v27 + v6->z;
                *total_mass = v50 + *total_mass;
            }
            v13 = (float *)*v10;
            if ( *v10 )
            {
                v51 = 1.0 / v13[60];
                v33 = v51 * v13[24];
                v38 = v13[25] * v51;
                v43 = v13[26] * v51;
                v5->x = v5->x + v33;
                v5->y = v5->y + v38;
                v5->z = v5->z + v43;
                v18 = v13[36] * v51;
                v23 = v13[37] * v51;
                v28 = v13[38] * v51;
                v6->x = v6->x + v18;
                v6->y = v6->y + v23;
                v6->z = v28 + v6->z;
                *total_mass = v51 + *total_mass;
            }
            v14 = (float *)*((unsigned int *)v10 + 1);
            if ( v14 )
            {
                v52 = 1.0 / v14[60];
                v34 = v52 * v14[24];
                v39 = v14[25] * v52;
                v44 = v14[26] * v52;
                v5->x = v5->x + v34;
                v5->y = v5->y + v39;
                v5->z = v5->z + v44;
                v19 = v14[36] * v52;
                v24 = v14[37] * v52;
                v29 = v14[38] * v52;
                v6->x = v6->x + v19;
                v6->y = v6->y + v24;
                v6->z = v29 + v6->z;
                *total_mass = v52 + *total_mass;
            }
            v10 += 4;
            --v48;
        }
        while ( v48 );
        v8 = v46;
        v7 = rbodies_count;
    }
    if ( v8 < v7 )
    {
        do
        {
            v15 = list_rigid_body[v8];
            if ( v15 )
            {
                v53 = 1.0 / v15->m_inv_mass;
                v35 = v53 * v15->m_mat.w.x;
                v40 = v15->m_mat.w.y * v53;
                v45 = v15->m_mat.w.z * v53;
                v5->x = v5->x + v35;
                v5->y = v5->y + v40;
                v5->z = v5->z + v45;
                v20 = v15->m_t_vel.x * v53;
                v25 = v15->m_t_vel.y * v53;
                v30 = v15->m_t_vel.z * v53;
                v6->x = v6->x + v20;
                v6->y = v6->y + v25;
                v6->z = v30 + v6->z;
                *total_mass = v53 + *total_mass;
            }
            ++v8;
        }
        while ( v8 < rbodies_count );
    }
    if ( *total_mass <= 0.0000099999997 )
    {
        if ( _tlAssert("source/phys_util.cpp", 23, "*total_mass > 0.00001f", "") )
            __debugbreak();
        v5 = center_of_mass;
        v9 = 1.0;
    }
    v47 = v9 / *total_mass;
    v5->x = v5->x * v47;
    v5->y = v5->y * v47;
    v5->z = v47 * v5->z;
}

void    nuge::apply_ballistic_target(
                rigid_body *const *list_rigid_body,
                int rbodies_count,
                const phys_vec3 *target,
                float *const dist_sq)
{
    double v5; // st6
    double v6; // st7
    rigid_body *v7; // eax
    float w; // eax
    int v9; // ecx
    int v10; // edx
    double v11; // st6
    double v12; // st4
    double v13; // st4
    double v14; // st2
    double v15; // st3
    long double v16; // st5
    long double v17; // st4
    double v18; // st5
    int v19; // ecx
    int v20; // esi
    double v21; // st7
    unsigned int v22; // ecx
    rigid_body *const *v23; // eax
    int v24; // edx
    int v25; // edx
    int v26; // edx
    rigid_body *v27; // eax
    int v28; // esi
    rigid_body *v29; // ecx
    float v30; // [esp-Ch] [ebp-DCh]
    float vt_8; // [esp+8h] [ebp-C8h]
    float dvs_8; // [esp+18h] [ebp-B8h]
    float ddvs_4; // [esp+38h] [ebp-98h]
    float ddvs_4a; // [esp+38h] [ebp-98h]
    double v35; // [esp+4Ch] [ebp-84h]
    float dir; // [esp+54h] [ebp-7Ch]
    float dir_4; // [esp+58h] [ebp-78h]
    float dir_8; // [esp+5Ch] [ebp-74h]
    float total_velocity; // [esp+64h] [ebp-6Ch]
    float total_velocity_4; // [esp+68h] [ebp-68h]
    float total_velocity_8; // [esp+6Ch] [ebp-64h]
    float delta_pos; // [esp+74h] [ebp-5Ch]
    float delta_pos_4; // [esp+78h] [ebp-58h]
    float delta_pos_8; // [esp+7Ch] [ebp-54h]
    float total_mass; // [esp+90h] [ebp-40h] BYREF
    phys_vec3 center_of_mass; // [esp+94h] [ebp-3Ch] BYREF
    phys_vec3 total_momentum; // [esp+A4h] [ebp-2Ch] BYREF
    float delta_x; // [esp+B4h] [ebp-1Ch]
    float s_; // [esp+B8h] [ebp-18h]
    float B; // [esp+BCh] [ebp-14h]
    float s1_; // [esp+C0h] [ebp-10h]
    //_UNKNOWN *v52; // [esp+C4h] [ebp-Ch]
    //rigid_body *const *list_rigid_bodya; // [esp+C8h] [ebp-8h]
    //const phys_vec3 *targeta; // [esp+D0h] [ebp+0h]
    //
    //v52 = a1;
    //list_rigid_bodya = (rigid_body *const *)targeta;
    nuge::get_ballistic_info(list_rigid_body, rbodies_count, &center_of_mass, &total_momentum, &total_mass);
    delta_x = 1.0 / total_mass;
    total_velocity = total_momentum.x * delta_x;
    total_velocity_4 = total_momentum.y * delta_x;
    total_velocity_8 = delta_x * total_momentum.z;
    delta_pos = target->x - center_of_mass.x;
    delta_pos_4 = target->y - center_of_mass.y;
    delta_pos_8 = target->z - center_of_mass.z;
    v5 = delta_pos * delta_pos;
    *dist_sq = delta_pos_8 * delta_pos_8 + delta_pos_4 * delta_pos_4 + v5;
    delta_x = delta_pos_8 * delta_pos_8 + v5;
    delta_x = sqrt(delta_x);
    v6 = delta_x;
    if (delta_x >= 0.001)
    {
        v7 = *list_rigid_body;
        total_momentum.y = (*list_rigid_body)->m_gravity_acc_vec.y;
        total_momentum.x = v7->m_gravity_acc_vec.x;
        w = v7->m_gravity_acc_vec.w;
        v9 = 0;
        s1_ = total_momentum.y * 0.5 * v6;
        total_momentum.w = w;
        v10 = 0;
        B = delta_pos_4 / v6;
        delta_x = 1.0 / v6;
        dir = delta_x * delta_pos;
        dir_4 = delta_x * 0.0;
        dir_8 = delta_x * delta_pos_8;
        s_ = 1.0;
        v11 = s1_;
        v35 = -2.0 * s1_;
        delta_x = 0.0 - 0.0;
        total_momentum.z = delta_x;
        do
        {
            v12 = s_;
            dvs_8 = s_ * B - v11 / s_;
            vt_8 = B + v11 / (v12 * v12);
            ddvs_4 = v35 / (v12 * v12 * v12);
            v30 = dir_4 * total_velocity_4 + dir * total_velocity + dir_8 * total_velocity_8;
            s1_ = s_ - v30;
            v13 = s1_;
            center_of_mass.x = s1_;
            s1_ = dvs_8 - total_velocity_4;
            center_of_mass.y = s1_;
            center_of_mass.z = delta_x;
            total_momentum.x = v13;
            total_momentum.y = s1_;
            s1_ = vt_8 * s1_ + center_of_mass.x + delta_x * 0.0;
            v14 = s1_;
            s1_ = total_momentum.y * ddvs_4 + total_momentum.x * 0.0 + delta_x * 0.0;
            v15 = s1_;
            s1_ = vt_8 * vt_8 + 1.0 + 0.0 * 0.0;
            s1_ = v15 + s1_;
            s1_ = s_ - v14 / s1_;
            v16 = s1_;
            s1_ = s1_ - s_;
            s1_ = fabs(s1_);
            if (s1_ < 0.001)
                v10 = 1;
            ++v9;
            v17 = v16;
            s_ = v16;
        } while (v9 <= 10 && !v10);
        s1_ = fabs(v16);
        if (s1_ >= 1.0)
        {
            v18 = 0.0;
        }
        else
        {
            v18 = 0.0;
            if (s1_ > 0.001)
            {
                if (v17 < 0.0)
                    s_ = -1.0;
                else
                    s_ = 1.0;
            }
        }
        v19 = rbodies_count;
        v20 = 0;
        ddvs_4a = s_ * B - v11 / s_;
        total_momentum.x = dir * s_;
        total_momentum.y = dir_4 * s_;
        total_momentum.z = s_ * dir_8;
        center_of_mass.x = total_momentum.x + 0.0;
        center_of_mass.y = total_momentum.y + ddvs_4a;
        center_of_mass.z = total_momentum.z + 0.0;
        B = v18;
        v21 = 1.0;
        if (rbodies_count >= 4)
        {
            v22 = ((unsigned int)(rbodies_count - 4) >> 2) + 1;
            v23 = list_rigid_body + 2;
            v20 = 4 * v22;
            do
            {
                v24 = *((_DWORD *)v23 - 2);
                if (v24)
                {
                    s_ = *(float *)(v24 + 240);
                    s1_ = s_ * s_;
                    B = 1.0 / s1_ + B;
                }
                v25 = *((_DWORD *)v23 - 1);
                if (v25)
                {
                    s_ = *(float *)(v25 + 240);
                    s1_ = s_ * s_;
                    B = 1.0 / s1_ + B;
                }
                if (*v23)
                {
                    s_ = (*v23)->m_inv_mass;
                    s1_ = s_ * s_;
                    B = 1.0 / s1_ + B;
                }
                v26 = *((_DWORD *)v23 + 1);
                if (v26)
                {
                    s_ = *(float *)(v26 + 240);
                    s1_ = s_ * s_;
                    B = 1.0 / s1_ + B;
                }
                v23 += 4;
                --v22;
            } while (v22);
            v19 = rbodies_count;
        }
        for (; v20 < v19; ++v20)
        {
            v27 = list_rigid_body[v20];
            if (v27)
            {
                s_ = v27->m_inv_mass;
                s1_ = s_ * s_;
                B = 1.0 / s1_ + B;
            }
        }
        if (B <= 0.0000099999997)
        {
            if (_tlAssert("source/phys_util.cpp", 82, "total_square_mass > 0.00001f", ""))
                __debugbreak();
            v19 = rbodies_count;
            v21 = 1.0;
        }
        v28 = 0;
        total_momentum.x = center_of_mass.x - total_velocity;
        total_momentum.y = center_of_mass.y - total_velocity_4;
        total_momentum.z = center_of_mass.z - total_velocity_8;
        s1_ = total_mass / B;
        center_of_mass.x = total_momentum.x * s1_;
        center_of_mass.y = total_momentum.y * s1_;
        for (center_of_mass.z = s1_ * total_momentum.z; v28 < v19; ++v28)
        {
            if (list_rigid_body[v28])
            {
                v29 = list_rigid_body[v28];
                s1_ = v29->m_inv_mass;
                s1_ = s1_ * s1_;
                s1_ = v21 / s1_;
                total_momentum.x = s1_ * center_of_mass.x;
                total_momentum.y = center_of_mass.y * s1_;
                total_momentum.z = s1_ * center_of_mass.z;
                //rigid_body::add_force(v29, &total_momentum);
                v29->add_force(&total_momentum);
                v21 = 1.0;
                v19 = rbodies_count;
            }
        }
    }
}

void    nuge::calc_velocities(
                const phys_mat44 *mat0,
                const phys_mat44 *mat1,
                float delta_t,
                phys_vec3 *t_vel,
                phys_vec3 *a_vel)
{
    Phys_UnitQuaternion *Quaternion; // eax
    float y; // ecx
    float z; // edx
    long double w; // st7
    double v10; // st7
    double v11; // st6
    double v12; // st7
    double v13; // st6
    phys_mat44 v14; // [esp-Ch] [ebp-7Ch] BYREF
    Phys_UnitQuaternion vel_mat_52; // [esp+34h] [ebp-3Ch] BYREF
    Phys_UnitQuaternion qvel; // [esp+44h] [ebp-2Ch]
    float oo_delta_t; // [esp+58h] [ebp-18h]
    float nqvec; // [esp+5Ch] [ebp-14h]
    float v19; // [esp+60h] [ebp-10h]
    //_UNKNOWN *v20[2]; // [esp+64h] [ebp-Ch] BYREF
    //float delta_ta; // [esp+70h] [ebp+0h]
    //
    //*(float *)v20 = a1;
    //*(float *)&v20[1] = delta_ta;
    if (((unsigned __int8)t_vel & 0xF) != 0
        && _tlAssert(
            "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\phys_math.h",
            444,
            "uint(v) % PHYS_ALIGNOF(phys_vec3) == 0",
            ""))
    {
        __debugbreak();
    }
    if (((unsigned __int8)a_vel & 0xF) != 0
        && _tlAssert(
            "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\phys_math.h",
            444,
            "uint(v) % PHYS_ALIGNOF(phys_vec3) == 0",
            ""))
    {
        __debugbreak();
    }
    oo_delta_t = 1.0 / delta_t;
    phys_transpose(&v14, mat0);
    phys_multiply_mat(&v14, mat1, &v14);
    Quaternion = Phys_GetQuaternion(&vel_mat_52, &v14);
    y = Quaternion->y;
    qvel.x = Quaternion->x;
    z = Quaternion->z;
    qvel.w = Quaternion->w;
    qvel.y = y;
    qvel.z = z;
    w = qvel.w;
    if (qvel.w < 1.0)
    {
        if (w > -1.0)
        {
            v19 = acos(w);
            v10 = v19;
        }
        else
        {
            v10 = 3.1415927;
        }
    }
    else
    {
        v10 = 0.0;
    }
    v19 = v10;
    nqvec = qvel.y * qvel.y + qvel.x * qvel.x + qvel.z * qvel.z;
    nqvec = sqrt(nqvec);
    v11 = oo_delta_t;
    if (nqvec <= 0.00009999999747378752)
    {
        v12 = oo_delta_t;
        a_vel->x = PHYS_ZERO_VEC.x;
        a_vel->y = PHYS_ZERO_VEC.y;
        v13 =      PHYS_ZERO_VEC.z;
    }
    else
    {
        oo_delta_t = v19 + v19;
        v12 = v11;
        oo_delta_t = v11 / nqvec * oo_delta_t;
        vel_mat_52.x = oo_delta_t * qvel.x;
        vel_mat_52.y = qvel.y * oo_delta_t;
        vel_mat_52.z = oo_delta_t * qvel.z;
        a_vel->x = vel_mat_52.x;
        a_vel->y = vel_mat_52.y;
        v13 = vel_mat_52.z;
    }
    a_vel->z = v13;
    vel_mat_52.x = mat1->w.x - mat0->w.x;
    vel_mat_52.y = mat1->w.y - mat0->w.y;
    vel_mat_52.z = mat1->w.z - mat0->w.z;
    qvel.x = vel_mat_52.x * v12;
    qvel.y = vel_mat_52.y * v12;
    qvel.z = v12 * vel_mat_52.z;
    t_vel->x = qvel.x;
    t_vel->y = qvel.y;
    t_vel->z = qvel.z;
}

void    nuge::calc_velocities(
                const phys_mat44 *mat0,
                const phys_mat44 *mat1,
                const phys_vec3 *center_offset_loc,
                float delta_t,
                phys_vec3 *t_vel,
                phys_vec3 *a_vel)
{
    const phys_vec3 *v7; // eax
    phys_vec3 v9; // [esp+1Ch] [ebp-2Ch] BYREF
    float v10; // [esp+2Ch] [ebp-1Ch]
    float v11; // [esp+30h] [ebp-18h]
    float v12; // [esp+34h] [ebp-14h]
    //unsigned int v13[3]; // [esp+3Ch] [ebp-Ch] BYREF
    //_UNKNOWN *retaddr; // [esp+48h] [ebp+0h]
    //
    //v13[0] = a1;
    //v13[1] = retaddr;
    if ( ((unsigned __int8)t_vel & 0xF) != 0
        && _tlAssert(
                 "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\phys_math.h",
                 444,
                 "uint(v) % PHYS_ALIGNOF(phys_vec3) == 0",
                 "") )
    {
        __debugbreak();
    }
    if ( ((unsigned __int8)a_vel & 0xF) != 0
        && _tlAssert(
                 "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\phys_math.h",
                 444,
                 "uint(v) % PHYS_ALIGNOF(phys_vec3) == 0",
                 "") )
    {
        __debugbreak();
    }
    nuge::calc_velocities(mat0, mat1, delta_t, t_vel, a_vel);
    v7 = phys_multiply(&v9, mat1, center_offset_loc);
    v10 = v7->z * a_vel->y - v7->y * a_vel->z;
    v11 = a_vel->z * v7->x - a_vel->x * v7->z;
    v12 = v7->y * a_vel->x - v7->x * a_vel->y;
    t_vel->x = t_vel->x + v10;
    t_vel->y = v11 + t_vel->y;
    t_vel->z = t_vel->z + v12;
}

void __cdecl nuge::calc_sphere_inertia(float radius, phys_vec3 *unit_inertia, float *volume)
{
    phys_vec3 *v3; // eax
    double v4; // st7
    float v5; // [esp-4h] [ebp-10h]
    float v6; // [esp-4h] [ebp-10h]

    v3 = unit_inertia;
    if ( ((unsigned __int8)unit_inertia & 0xF) != 0 )
    {
        if ( _tlAssert(
                     "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\phys_math.h",
                     444,
                     "uint(v) % PHYS_ALIGNOF(phys_vec3) == 0",
                     "") )
        {
            __debugbreak();
        }
        v3 = unit_inertia;
    }
    v4 = radius;
    v5 = 3.141592502593994 * v4 * v4 * v4 * 4.0 / 3.0;
    *volume = v5;
    v6 = (v4 * (v5 * radius) + v4 * (v5 * radius)) / 5.0;
    v3->x = v6;
    v3->y = v6;
    v3->z = v6;
}

void __cdecl nuge::calc_box_inertia(const phys_vec3 *dim, phys_vec3 *unit_inertia, float *volume)
{
    phys_vec3 *v3; // ecx
    float v4; // [esp-Ch] [ebp-3Ch]
    float v5; // [esp-8h] [ebp-38h]
    float v6; // [esp-4h] [ebp-34h]
    float moments_4; // [esp+4h] [ebp-2Ch]
    float moments_4a; // [esp+4h] [ebp-2Ch]
    float moments_8; // [esp+8h] [ebp-28h]
    float moments_8a; // [esp+8h] [ebp-28h]
    float moments_12; // [esp+Ch] [ebp-24h]
    float moments_12a; // [esp+Ch] [ebp-24h]
    float v13; // [esp+18h] [ebp-18h]

    v3 = unit_inertia;
    if ( ((unsigned __int8)unit_inertia & 0xF) != 0 )
    {
        if ( _tlAssert(
                     "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\phys_math.h",
                     444,
                     "uint(v) % PHYS_ALIGNOF(phys_vec3) == 0",
                     "") )
        {
            __debugbreak();
        }
        v3 = unit_inertia;
    }
    *volume = dim->x * dim->y * dim->z;
    moments_4 = dim->x * dim->x;
    moments_8 = dim->y * dim->y;
    moments_12 = dim->z * dim->z;
    v13 = dim->x * 0.0833333358168602 * dim->y * dim->z;
    v4 = v13 * moments_4;
    v5 = moments_8 * v13;
    v6 = v13 * moments_12;
    moments_4a = v5 + v6;
    moments_8a = v6 + v4;
    moments_12a = v5 + v4;
    v3->x = moments_4a;
    v3->y = moments_8a;
    v3->z = moments_12a;
}

void __cdecl nuge::calc_bound_sphere(const phys_vec3 *vert_list, int vert_count, float *radius, phys_vec3 *com)
{
    int v4; // edx
    const float *p_z; // eax
    unsigned int v6; // ecx
    double v7; // st7
    const float *v8; // eax
    int v9; // ecx
    double v10; // st7
    int v11; // eax
    double v12; // st7
    double v13; // st6
    double v14; // st5
    const float *v15; // ecx
    unsigned int v16; // edx
    const float *v17; // ecx
    int v18; // edi
    float v19; // [esp-1Ch] [ebp-3Ch]
    float v20; // [esp-1Ch] [ebp-3Ch]
    float v21; // [esp-1Ch] [ebp-3Ch]
    float v22; // [esp-1Ch] [ebp-3Ch]
    float v23; // [esp-1Ch] [ebp-3Ch]
    float v24; // [esp-18h] [ebp-38h]
    float v25; // [esp-18h] [ebp-38h]
    float v26; // [esp-18h] [ebp-38h]
    float v27; // [esp-18h] [ebp-38h]
    float v28; // [esp-18h] [ebp-38h]
    float v29; // [esp-14h] [ebp-34h]
    float v30; // [esp-14h] [ebp-34h]
    float v31; // [esp-14h] [ebp-34h]
    float v32; // [esp-14h] [ebp-34h]
    float v33; // [esp-14h] [ebp-34h]
    float x; // [esp-Ch] [ebp-2Ch]
    float v35; // [esp-Ch] [ebp-2Ch]
    float v36; // [esp-Ch] [ebp-2Ch]
    float v37; // [esp-Ch] [ebp-2Ch]
    float v38; // [esp-Ch] [ebp-2Ch]
    float y; // [esp-8h] [ebp-28h]
    float v40; // [esp-8h] [ebp-28h]
    float v41; // [esp-8h] [ebp-28h]
    float v42; // [esp-8h] [ebp-28h]
    float v43; // [esp-8h] [ebp-28h]
    float z; // [esp-4h] [ebp-24h]
    float v45; // [esp-4h] [ebp-24h]
    float v46; // [esp-4h] [ebp-24h]
    float v47; // [esp-4h] [ebp-24h]
    float v48; // [esp-4h] [ebp-24h]
    int dir3_com_12; // [esp+Ch] [ebp-14h]
    float dir3_com_12a; // [esp+Ch] [ebp-14h]
    float v51; // [esp+10h] [ebp-10h]
    float v52; // [esp+10h] [ebp-10h]
    float v53; // [esp+10h] [ebp-10h]
    float v54; // [esp+10h] [ebp-10h]
    float v55; // [esp+10h] [ebp-10h]
    float v56; // [esp+10h] [ebp-10h]

    if ( ((unsigned __int8)vert_list & 0xF) != 0
        && _tlAssert(
                 "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\phys_math.h",
                 444,
                 "uint(v) % PHYS_ALIGNOF(phys_vec3) == 0",
                 "") )
    {
        __debugbreak();
    }
    if ( ((unsigned __int8)com & 0xF) != 0
        && _tlAssert(
                 "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\phys_math.h",
                 444,
                 "uint(v) % PHYS_ALIGNOF(phys_vec3) == 0",
                 "") )
    {
        __debugbreak();
    }
    x = PHYS_ZERO_VEC.x;
    z = PHYS_ZERO_VEC.z;
    v4 = 0;
    y = PHYS_ZERO_VEC.y;
    if ( vert_count >= 4 )
    {
        p_z = &vert_list[1].z;
        v6 = ((unsigned int)(vert_count - 4) >> 2) + 1;
        v4 = 4 * v6;
        do
        {
            v7 = *(p_z - 6);
            p_z += 16;
            --v6;
            v35 = v7 + x;
            v40 = *(p_z - 21) + y;
            v45 = *(p_z - 20) + z;
            v36 = *(p_z - 18) + v35;
            v41 = *(p_z - 17) + v40;
            v46 = v45 + *(p_z - 16);
            v37 = *(p_z - 14) + v36;
            v42 = *(p_z - 13) + v41;
            v47 = *(p_z - 12) + v46;
            x = *(p_z - 10) + v37;
            y = *(p_z - 9) + v42;
            z = *(p_z - 8) + v47;
        }
        while ( v6 );
    }
    if ( v4 < vert_count )
    {
        v8 = &vert_list[v4].z;
        v9 = vert_count - v4;
        do
        {
            v10 = *(v8 - 2);
            v8 += 4;
            --v9;
            x = v10 + x;
            y = *(v8 - 5) + y;
            z = *(v8 - 4) + z;
        }
        while ( v9 );
    }
    v11 = 0;
    v51 = 1.0 / (double)vert_count;
    v38 = v51 * x;
    v43 = y * v51;
    v48 = v51 * z;
    *radius = 0.0;
    v12 = v48;
    v13 = v43;
    v14 = v38;
    if ( vert_count >= 4 )
    {
        v15 = &vert_list[1].z;
        v16 = ((unsigned int)(vert_count - 4) >> 2) + 1;
        dir3_com_12 = 4 * v16;
        do
        {
            v19 = *(v15 - 6) - v14;
            v24 = *(v15 - 5) - v13;
            v29 = *(v15 - 4) - v12;
            v52 = v24 * v24 + v19 * v19 + v29 * v29;
            if ( *radius < (double)v52 )
                *radius = v52;
            v20 = *(v15 - 2) - v14;
            v25 = *(v15 - 1) - v13;
            v30 = *v15 - v12;
            v53 = v25 * v25 + v20 * v20 + v30 * v30;
            if ( *radius < (double)v53 )
                *radius = v53;
            v21 = v15[2] - v14;
            v26 = v15[3] - v13;
            v31 = v15[4] - v12;
            v54 = v26 * v26 + v21 * v21 + v31 * v31;
            if ( *radius < (double)v54 )
                *radius = v54;
            v22 = v15[6] - v14;
            v27 = v15[7] - v13;
            v32 = v15[8] - v12;
            v55 = v27 * v27 + v22 * v22 + v32 * v32;
            if ( *radius < (double)v55 )
                *radius = v55;
            v15 += 16;
            --v16;
        }
        while ( v16 );
        v11 = dir3_com_12;
    }
    if ( v11 < vert_count )
    {
        v17 = &vert_list[v11].z;
        v18 = vert_count - v11;
        do
        {
            v23 = *(v17 - 2) - v14;
            v28 = *(v17 - 1) - v13;
            v33 = *v17 - v12;
            v56 = v28 * v28 + v23 * v23 + v33 * v33;
            if ( *radius < (double)v56 )
                *radius = v56;
            v17 += 4;
            --v18;
        }
        while ( v18 );
    }
    dir3_com_12a = sqrt(*radius);
    *radius = dir3_com_12a;
    com->x = v38;
    com->y = v43;
    com->z = v48;
}

void __cdecl nuge::calc_bound_box(const phys_vec3 *vert_list, int vert_count, phys_vec3 *dim, phys_vec3 *com)
{
    int v4; // edx
    const phys_vec3 *v5; // ecx
    unsigned int v6; // esi
    double x; // st7
    double v8; // st7
    double v9; // st7
    double v10; // st7
    const phys_vec3 *v11; // ecx
    int v12; // edi
    double v13; // st7
    double v14; // st6
    double v15; // st4
    double v16; // st2
    float v17; // [esp-Ch] [ebp-3Ch]
    float v18; // [esp-Ch] [ebp-3Ch]
    float y; // [esp-8h] [ebp-38h]
    float v20; // [esp-8h] [ebp-38h]
    float v21; // [esp-8h] [ebp-38h]
    float v22; // [esp-8h] [ebp-38h]
    float v23; // [esp-8h] [ebp-38h]
    float v24; // [esp-8h] [ebp-38h]
    float v25; // [esp-8h] [ebp-38h]
    float z; // [esp-4h] [ebp-34h]
    float v27; // [esp-4h] [ebp-34h]
    float v28; // [esp-4h] [ebp-34h]
    float v29; // [esp-4h] [ebp-34h]
    float v30; // [esp-4h] [ebp-34h]
    float v31; // [esp-4h] [ebp-34h]
    float v32; // [esp-4h] [ebp-34h]
    float v_4; // [esp+4h] [ebp-2Ch]
    float v_4a; // [esp+4h] [ebp-2Ch]
    float v_8; // [esp+8h] [ebp-28h]
    float v_8a; // [esp+8h] [ebp-28h]
    float v_12; // [esp+Ch] [ebp-24h]
    float v_12a; // [esp+Ch] [ebp-24h]
    float vmin_4; // [esp+14h] [ebp-1Ch]
    float vmin_8; // [esp+18h] [ebp-18h]
    float vmin_12; // [esp+1Ch] [ebp-14h]

    if ( ((unsigned __int8)vert_list & 0xF) != 0
        && _tlAssert(
                 "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\phys_math.h",
                 444,
                 "uint(v) % PHYS_ALIGNOF(phys_vec3) == 0",
                 "") )
    {
        __debugbreak();
    }
    if ( ((unsigned __int8)dim & 0xF) != 0
        && _tlAssert(
                 "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\phys_math.h",
                 444,
                 "uint(v) % PHYS_ALIGNOF(phys_vec3) == 0",
                 "") )
    {
        __debugbreak();
    }
    if ( ((unsigned __int8)com & 0xF) != 0
        && _tlAssert(
                 "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\phys_math.h",
                 444,
                 "uint(v) % PHYS_ALIGNOF(phys_vec3) == 0",
                 "") )
    {
        __debugbreak();
    }
    v_4 = 10000000.0;
    v4 = 0;
    v_8 = 10000000.0;
    v_12 = 10000000.0;
    vmin_4 = -10000000.0;
    vmin_8 = -10000000.0;
    vmin_12 = -10000000.0;
    if ( vert_count >= 4 )
    {
        v5 = vert_list + 2;
        v6 = ((unsigned int)(vert_count - 4) >> 2) + 1;
        v4 = 4 * v6;
        do
        {
            x = v5[-2].x;
            y = v5[-2].y;
            z = v5[-2].z;
            if ( v_4 > x )
                v_4 = v5[-2].x;
            if ( vmin_4 < x )
                vmin_4 = v5[-2].x;
            if ( v_8 > (double)y )
                v_8 = y;
            if ( vmin_8 < (double)y )
                vmin_8 = y;
            if ( v_12 > (double)z )
                v_12 = z;
            if ( vmin_12 < (double)z )
                vmin_12 = z;
            v8 = v5[-1].x;
            v20 = v5[-1].y;
            v27 = v5[-1].z;
            if ( v_4 > v8 )
                v_4 = v5[-1].x;
            if ( vmin_4 < v8 )
                vmin_4 = v5[-1].x;
            if ( v_8 > (double)v20 )
                v_8 = v20;
            if ( vmin_8 < (double)v20 )
                vmin_8 = v20;
            if ( v_12 > (double)v27 )
                v_12 = v27;
            if ( vmin_12 < (double)v27 )
                vmin_12 = v27;
            v9 = v5->x;
            v21 = v5->y;
            v28 = v5->z;
            if ( v_4 > v9 )
                v_4 = v5->x;
            if ( vmin_4 < v9 )
                vmin_4 = v5->x;
            if ( v_8 > (double)v21 )
                v_8 = v21;
            if ( vmin_8 < (double)v21 )
                vmin_8 = v21;
            if ( v_12 > (double)v28 )
                v_12 = v28;
            if ( vmin_12 < (double)v28 )
                vmin_12 = v28;
            v10 = v5[1].x;
            v22 = v5[1].y;
            v29 = v5[1].z;
            if ( v_4 > v10 )
                v_4 = v5[1].x;
            if ( vmin_4 < v10 )
                vmin_4 = v5[1].x;
            if ( v_8 > (double)v22 )
                v_8 = v22;
            if ( vmin_8 < (double)v22 )
                vmin_8 = v22;
            if ( v_12 > (double)v29 )
                v_12 = v29;
            if ( vmin_12 < (double)v29 )
                vmin_12 = v29;
            v5 += 4;
            --v6;
        }
        while ( v6 );
    }
    if ( v4 < vert_count )
    {
        v11 = &vert_list[v4];
        v12 = vert_count - v4;
        do
        {
            v13 = v11->x;
            v23 = v11->y;
            v30 = v11->z;
            if ( v_4 > v13 )
                v_4 = v11->x;
            if ( vmin_4 < v13 )
                vmin_4 = v11->x;
            if ( v_8 > (double)v23 )
                v_8 = v23;
            if ( vmin_8 < (double)v23 )
                vmin_8 = v23;
            if ( v_12 > (double)v30 )
                v_12 = v30;
            if ( vmin_12 < (double)v30 )
                vmin_12 = v30;
            ++v11;
            --v12;
        }
        while ( v12 );
    }
    v14 = v_4;
    v17 = vmin_4 + v_4;
    v15 = v_8;
    v24 = vmin_8 + v_8;
    v16 = v_12;
    v31 = vmin_12 + v_12;
    v_4a = v17 * 0.5;
    v_8a = v24 * 0.5;
    v_12a = 0.5 * v31;
    com->x = v_4a;
    com->y = v_8a;
    com->z = v_12a;
    v18 = vmin_4 - v14;
    v25 = vmin_8 - v15;
    v32 = vmin_12 - v16;
    dim->x = v18;
    dim->y = v25;
    dim->z = v32;
}

