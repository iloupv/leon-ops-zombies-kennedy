#include "phys_contact_manifold.h"
#include <tl/tl_system.h>

#include <utility>
#include <algorithm>

const phys_vec3 *__cdecl phys_v2_to_v3_multiply(phys_vec3 *result, const phys_mat44 *m, const phys_vec2 *v)
{
    const phys_vec3 *v3; // eax
    float v4; // [esp-30h] [ebp-3Ch]
    float v5; // [esp-2Ch] [ebp-38h]
    float v6; // [esp-28h] [ebp-34h]
    float v7; // [esp-20h] [ebp-2Ch]
    float v8; // [esp-1Ch] [ebp-28h]
    float v9; // [esp-18h] [ebp-24h]
    float y; // [esp-4h] [ebp-10h]

    y = v->y;
    v4 = m->y.x * y;
    v5 = m->y.y * y;
    v6 = y * m->y.z;
    v7 = m->x.x * v->x;
    v8 = m->x.y * v->x;
    v3 = result;
    v9 = v->x * m->x.z;
    result->x = v7 + v4;
    result->y = v8 + v5;
    result->z = v9 + v6;
    return v3;
}

void    displace_contact_p(
                contact_manifold_mesh_point **mp,
                const phys_vec2 *d,
                const phys_mat44 *contact_mat)
{
    float *v4; // esi
    const phys_vec3 *v5; // eax
    phys_vec3 v6; // [esp-10h] [ebp-1Ch] BYREF
    //int v7; // [esp+0h] [ebp-Ch]
    //void *v8; // [esp+4h] [ebp-8h]
    //void *retaddr; // [esp+Ch] [ebp+0h]
    //
    //v7 = a1;
    //v8 = retaddr;
    v4 = (float *)*mp;
    v4[4] = (*mp)->m_contact_p.x + d->x;
    v4[5] = d->y + v4[5];
    v5 = phys_v2_to_v3_multiply(&v6, contact_mat, d);
    *v4 = v5->x + *v4;
    v4[1] = v5->y + v4[1];
    v4[2] = v5->z + v4[2];
}

double __thiscall phys_contact_manifold::compute_convex_poly_area()
{
    contact_manifold_mesh_point **m_list_contact_point; // edx
    contact_manifold_mesh_point *v2; // esi
    int *v3; // eax
    int v4; // edi
    int v5; // edx
    int v6; // ecx
    double v7; // st6
    double result; // st7
    float v9; // [esp+0h] [ebp-18h]
    float v10; // [esp+4h] [ebp-14h]
    float v11; // [esp+8h] [ebp-10h]
    float v12; // [esp+Ch] [ebp-Ch]
    float v13; // [esp+10h] [ebp-8h]
    float area; // [esp+14h] [ebp-4h]
    float areaa; // [esp+14h] [ebp-4h]

    m_list_contact_point = this->m_list_contact_point;
    area = 0.0;
    v2 = *m_list_contact_point;
    v3 = (int *)(m_list_contact_point + 1);
    v4 = (int)&m_list_contact_point[this->m_list_contact_point_count - 1];
    if ( m_list_contact_point + 1 != (contact_manifold_mesh_point **)v4 )
    {
        do
        {
            v5 = v3[1];
            v6 = *v3;
            v7 = *(float *)(v5 + 16) - *(float *)(*v3++ + 16);
            v9 = v7;
            v10 = *(float *)(v5 + 20) - *(float *)(v6 + 20);
            v11 = *(float *)(v6 + 16) - v2->m_contact_p.x;
            v12 = *(float *)(v6 + 20) - v2->m_contact_p.y;
            v13 = v11 * v10 - v12 * v9;
            area = v13 + area;
        }
        while ( v3 != (int *)v4 );
    }
    areaa = area * 0.5;
    result = areaa;
    if ( areaa < 0.0 )
        return (float)-result;
    return result;
}

double __thiscall phys_contact_manifold::compute_convex_poly_perimeter()
{
    contact_manifold_mesh_point **m_list_contact_point; // esi
    contact_manifold_mesh_point **v2; // edi
    contact_manifold_mesh_point **i; // eax
    contact_manifold_mesh_point *v4; // eax
    float v6; // [esp+4h] [ebp-10h]
    float v7; // [esp+8h] [ebp-Ch]
    float v8; // [esp+Ch] [ebp-8h]
    float v9; // [esp+Ch] [ebp-8h]
    float perimeter; // [esp+10h] [ebp-4h]

    perimeter = 0.0;
    m_list_contact_point = this->m_list_contact_point;
    v2 = &m_list_contact_point[this->m_list_contact_point_count];
    for ( i = v2 - 1; m_list_contact_point != v2; perimeter = v9 + perimeter )
    {
        v4 = *i;
        v6 = (*m_list_contact_point)->m_contact_p.x - v4->m_contact_p.x;
        v7 = (*m_list_contact_point)->m_contact_p.y - v4->m_contact_p.y;
        v8 = v6 * v6 + v7 * v7;
        v9 = sqrt(v8);
        i = m_list_contact_point++;
    }
    return perimeter;
}

void __cdecl feature_qsort(contact_manifold_mesh_point **list, int list_count)
{
    int v2; // eax
    bool v3; // cc
    contact_manifold_mesh_point **v4; // edx
    contact_manifold_mesh_point *v5; // edi
    contact_manifold_mesh_point **v6; // ebx
    contact_manifold_mesh_point **v7; // ecx
    contact_manifold_mesh_point **v8; // esi
    contact_manifold_mesh_point *v9; // edx
    contact_manifold_mesh_point *v10; // eax
    contact_manifold_mesh_point *v11; // edx
    contact_manifold_mesh_point *v12; // eax
    contact_manifold_mesh_point *v13; // edx
    contact_manifold_mesh_point *v14; // eax
    contact_manifold_mesh_point *v15; // edx
    contact_manifold_mesh_point *v16; // eax
    contact_manifold_mesh_point *v17; // edx
    contact_manifold_mesh_point *v18; // eax
    contact_manifold_mesh_point *v19; // edx

    v2 = list_count;
    v3 = list_count <= 2;
    if ( list_count == 2 )
    {
LABEL_20:
        v19 = *list;
        if ( (*list)->m_contact_p.x < (double)list[1]->m_contact_p.x )
        {
            *list = list[1];
            list[1] = v19;
        }
    }
    else
    {
        while ( !v3 )
        {
            v4 = list;
            v5 = *list;
            v6 = &list[v2 - 1];
            v7 = list + 1;
            v8 = list;
            if ( list + 1 <= v6 )
            {
                if ( (4 * v2 - 4) / 4 >= 4 )
                {
                    do
                    {
                        v9 = *v7;
                        if ( v5->m_contact_p.x < (double)(*v7)->m_contact_p.x )
                        {
                            v10 = v8[1];
                            ++v8;
                            *v7 = v10;
                            *v8 = v9;
                        }
                        v11 = v7[1];
                        if ( v5->m_contact_p.x < (double)v11->m_contact_p.x )
                        {
                            v12 = v8[1];
                            ++v8;
                            v7[1] = v12;
                            *v8 = v11;
                        }
                        v13 = v7[2];
                        if ( v5->m_contact_p.x < (double)v13->m_contact_p.x )
                        {
                            v14 = v8[1];
                            ++v8;
                            v7[2] = v14;
                            *v8 = v13;
                        }
                        v15 = v7[3];
                        if ( v5->m_contact_p.x < (double)v15->m_contact_p.x )
                        {
                            v16 = v8[1];
                            ++v8;
                            v7[3] = v16;
                            *v8 = v15;
                        }
                        v7 += 4;
                    }
                    while ( (int)v7 <= (int)(v6 - 3) );
                }
                for ( ; v7 <= v6; ++v7 )
                {
                    v17 = *v7;
                    if ( v5->m_contact_p.x < (double)(*v7)->m_contact_p.x )
                    {
                        v18 = v8[1];
                        ++v8;
                        *v7 = v18;
                        *v8 = v17;
                    }
                }
                v4 = list;
            }
            *v4 = *v8;
            *v8 = v5;
            feature_qsort(v4, v8 - v4);
            v2 = v6 - v8;
            list = v8 + 1;
            v3 = v2 <= 2;
            if ( v2 == 2 )
                goto LABEL_20;
        }
    }
}

void __cdecl convex_poly_qsort(contact_manifold_mesh_point **list, int list_count)
{
    int v2; // eax
    bool v3; // cc
    contact_manifold_mesh_point **v4; // edx
    contact_manifold_mesh_point *v5; // edi
    contact_manifold_mesh_point **v6; // ebx
    contact_manifold_mesh_point **v7; // ecx
    contact_manifold_mesh_point **v8; // esi
    contact_manifold_mesh_point *v9; // edx
    contact_manifold_mesh_point *v10; // eax
    contact_manifold_mesh_point *v11; // edx
    contact_manifold_mesh_point *v12; // eax
    contact_manifold_mesh_point *v13; // edx
    contact_manifold_mesh_point *v14; // eax
    contact_manifold_mesh_point *v15; // edx
    contact_manifold_mesh_point *v16; // eax
    contact_manifold_mesh_point *v17; // edx
    contact_manifold_mesh_point *v18; // eax
    contact_manifold_mesh_point *v19; // ecx
    contact_manifold_mesh_point *v20; // edx
    double x; // st7
    float *p_x; // ecx

    v2 = list_count;
    v3 = list_count <= 2;
    if ( list_count == 2 )
    {
LABEL_30:
        v19 = list[1];
        v20 = *list;
        x = v19->m_contact_p.x;
        p_x = &v19->m_contact_p.x;
        if ( (*list)->m_contact_p.x > x || v20->m_contact_p.x == *p_x && v20->m_contact_p.y > (double)p_x[1] )
        {
            *list = list[1];
            list[1] = v20;
        }
    }
    else
    {
        while ( !v3 )
        {
            v4 = list;
            v5 = *list;
            v6 = &list[v2 - 1];
            v7 = list + 1;
            v8 = list;
            if ( list + 1 <= v6 )
            {
                if ( (4 * v2 - 4) / 4 >= 4 )
                {
                    do
                    {
                        v9 = *v7;
                        if ( v5->m_contact_p.x > (double)(*v7)->m_contact_p.x
                            || v5->m_contact_p.x == v9->m_contact_p.x && v5->m_contact_p.y > (double)v9->m_contact_p.y )
                        {
                            v10 = v8[1];
                            ++v8;
                            *v7 = v10;
                            *v8 = v9;
                        }
                        v11 = v7[1];
                        if ( v5->m_contact_p.x > (double)v11->m_contact_p.x
                            || v5->m_contact_p.x == v11->m_contact_p.x && v5->m_contact_p.y > (double)v11->m_contact_p.y )
                        {
                            v12 = v8[1];
                            ++v8;
                            v7[1] = v12;
                            *v8 = v11;
                        }
                        v13 = v7[2];
                        if ( v5->m_contact_p.x > (double)v13->m_contact_p.x
                            || v5->m_contact_p.x == v13->m_contact_p.x && v5->m_contact_p.y > (double)v13->m_contact_p.y )
                        {
                            v14 = v8[1];
                            ++v8;
                            v7[2] = v14;
                            *v8 = v13;
                        }
                        v15 = v7[3];
                        if ( v5->m_contact_p.x > (double)v15->m_contact_p.x
                            || v5->m_contact_p.x == v15->m_contact_p.x && v5->m_contact_p.y > (double)v15->m_contact_p.y )
                        {
                            v16 = v8[1];
                            ++v8;
                            v7[3] = v16;
                            *v8 = v15;
                        }
                        v7 += 4;
                    }
                    while ( (int)v7 <= (int)(v6 - 3) );
                }
                for ( ; v7 <= v6; ++v7 )
                {
                    v17 = *v7;
                    if ( v5->m_contact_p.x > (double)(*v7)->m_contact_p.x
                        || v5->m_contact_p.x == v17->m_contact_p.x && v5->m_contact_p.y > (double)v17->m_contact_p.y )
                    {
                        v18 = v8[1];
                        ++v8;
                        *v7 = v18;
                        *v8 = v17;
                    }
                }
                v4 = list;
            }
            *v4 = *v8;
            *v8 = v5;
            convex_poly_qsort(v4, v8 - v4);
            v2 = v6 - v8;
            list = v8 + 1;
            v3 = v2 <= 2;
            if ( v2 == 2 )
                goto LABEL_30;
        }
    }
}

extern const char *g_contact_manifold_error_msg;
void __thiscall phys_contact_manifold::generate_convex_poly_internal()
{
    phys_contact_manifold *v1; // ebx
    int m_list_mesh_point_count; // eax
    contact_manifold_mesh_point **m_list_sorted_mesh_point; // edi
    contact_manifold_mesh_point **m_list_contact_point; // esi
    contact_manifold_mesh_point **v5; // eax
    contact_manifold_mesh_point *v6; // ebx
    int v7; // eax
    unsigned int v8; // edi
    float *p_x; // ecx
    phys_memory_heap *m_allocator; // eax
    contact_manifold_mesh_point **v11; // eax
    contact_manifold_mesh_point **v12; // edi
    contact_manifold_mesh_point *v13; // ebx
    int v14; // eax
    unsigned int v15; // edi
    float *v16; // ecx
    phys_memory_heap *v17; // eax
    int v18; // esi
    bool v19; // cc
    contact_manifold_mesh_point **min_length2; // [esp+0h] [ebp-34h]
    phys_vec2 e1; // [esp+14h] [ebp-20h] BYREF
    phys_vec2 e2; // [esp+1Ch] [ebp-18h] BYREF
    contact_manifold_mesh_point **last_i_smp; // [esp+24h] [ebp-10h]
    phys_contact_manifold *v24; // [esp+28h] [ebp-Ch]
    contact_manifold_mesh_point **i_smp; // [esp+2Ch] [ebp-8h]
    contact_manifold_mesh_point **min_cp_mp; // [esp+30h] [ebp-4h]

    v1 = this;
    m_list_mesh_point_count = this->m_list_mesh_point_count;
    min_length2 = this->m_list_sorted_mesh_point;
    v24 = this;
    convex_poly_qsort(min_length2, m_list_mesh_point_count);
    m_list_sorted_mesh_point = v1->m_list_sorted_mesh_point;
    m_list_contact_point = v1->m_list_contact_point;
    v5 = &m_list_sorted_mesh_point[v1->m_list_mesh_point_count];
    i_smp = m_list_sorted_mesh_point;
    min_cp_mp = m_list_contact_point + 1;
    last_i_smp = v5;
    if ( m_list_sorted_mesh_point < v5 )
    {
        do
        {
            v6 = *m_list_sorted_mesh_point;
            if ( m_list_contact_point > min_cp_mp )
            {
                do
                {
                    v7 = (int)*(m_list_contact_point - 1);
                    v8 = (unsigned int)(m_list_contact_point - 1);
                    p_x = &(*(m_list_contact_point - 2))->m_contact_p.x;
                    e2.x = v6->m_contact_p.x - *(float *)(v7 + 16);
                    e2.y = v6->m_contact_p.y - *(float *)(v7 + 20);
                    e1.x = *(float *)(v7 + 16) - *p_x;
                    e1.y = *(float *)(v7 + 20) - p_x[1];
                    if ( phys_contact_manifold::rht(&e1, &e2, 0.1156, 0.00030458649) )
                        break;
                    --m_list_contact_point;
                }
                while ( v8 > (unsigned int)min_cp_mp );
                m_list_sorted_mesh_point = i_smp;
            }
            m_allocator = v24->m_allocator;
            if ( ((char *)m_list_contact_point < m_allocator->m_buffer_start
                 || (char *)(m_list_contact_point + 1) > m_allocator->m_buffer_end)
                && _tlAssert(
                         "source/phys_contact_manifold.cpp",
                         205,
                         "m_allocator->fast_is_within_buffer_limits(cp_mp,sizeof(contact_manifold_mesh_point*))",
                         g_contact_manifold_error_msg) )
            {
                __debugbreak();
            }
            ++m_list_sorted_mesh_point;
            *m_list_contact_point++ = v6;
            i_smp = m_list_sorted_mesh_point;
        }
        while ( m_list_sorted_mesh_point < last_i_smp );
        v1 = v24;
    }
    v11 = v1->m_list_sorted_mesh_point;
    v12 = m_list_sorted_mesh_point - 2;
    min_cp_mp = m_list_contact_point;
    last_i_smp = v11;
    i_smp = v12;
    if ( v12 >= v11 )
    {
        do
        {
            v13 = *v12;
            if ( m_list_contact_point > min_cp_mp )
            {
                do
                {
                    v14 = (int)*(m_list_contact_point - 1);
                    v15 = (unsigned int)(m_list_contact_point - 1);
                    v16 = &(*(m_list_contact_point - 2))->m_contact_p.x;
                    e1.x = v13->m_contact_p.x - *(float *)(v14 + 16);
                    e1.y = v13->m_contact_p.y - *(float *)(v14 + 20);
                    e2.x = *(float *)(v14 + 16) - *v16;
                    e2.y = *(float *)(v14 + 20) - v16[1];
                    if ( phys_contact_manifold::rht(&e2, &e1, 0.1156, 0.00030458649) )
                        break;
                    --m_list_contact_point;
                }
                while ( v15 > (unsigned int)min_cp_mp );
                v12 = i_smp;
            }
            v17 = v24->m_allocator;
            if ( ((char *)m_list_contact_point < v17->m_buffer_start || (char *)(m_list_contact_point + 1) > v17->m_buffer_end)
                && _tlAssert(
                         "source/phys_contact_manifold.cpp",
                         226,
                         "m_allocator->fast_is_within_buffer_limits(cp_mp,sizeof(contact_manifold_mesh_point*))",
                         g_contact_manifold_error_msg) )
            {
                __debugbreak();
            }
            --v12;
            *m_list_contact_point++ = v13;
            i_smp = v12;
        }
        while ( v12 >= last_i_smp );
        v1 = v24;
    }
    v18 = ((char *)m_list_contact_point - (char *)v1->m_list_contact_point - 4) >> 2;
    v19 = v18 <= v1->m_list_mesh_point_count;
    v1->m_list_contact_point_count = v18;
    if ( !v19 )
    {
        if ( _tlAssert(
                     "source/phys_contact_manifold.cpp",
                     232,
                     "m_list_contact_point_count <= m_list_mesh_point_count",
                     "") )
        {
            __debugbreak();
        }
    }
}

void __thiscall phys_contact_manifold_process::intersect_poly_segment(
                phys_contact_manifold *cman,
                const phys_vec2 *p0,
                const phys_vec2 *p1)
{
    contact_manifold_mesh_point **m_list_contact_point; // ecx
    contact_manifold_mesh_point **v7; // edi
    contact_manifold_mesh_point *v8; // eax
    double v9; // st7
    double v10; // st5
    double v11; // rt0
    contact_manifold_mesh_point *v12; // edx
    double v13; // st4
    double v14; // rt2
    double v15; // st5
    double v16; // st7
    bool v17; // c0
    double v18; // st4
    double v19; // rtt
    contact_manifold_mesh_point **v20; // eax
    contact_manifold_mesh_point *v21; // eax
    double v22; // st5
    int v23; // eax
    double v24; // st7
    float normal; // [esp+Ch] [ebp-28h]
    float v26; // [esp+14h] [ebp-20h]
    float v27; // [esp+14h] [ebp-20h]
    float v28; // [esp+14h] [ebp-20h]
    float v29; // [esp+18h] [ebp-1Ch]
    float v30; // [esp+18h] [ebp-1Ch]
    float v31; // [esp+18h] [ebp-1Ch]
    float v32; // [esp+1Ch] [ebp-18h]
    float v33; // [esp+1Ch] [ebp-18h]
    float v34; // [esp+1Ch] [ebp-18h]
    float v35; // [esp+20h] [ebp-14h]
    float v36; // [esp+20h] [ebp-14h]
    float v37; // [esp+20h] [ebp-14h]
    float dir; // [esp+24h] [ebp-10h]
    float dir_4; // [esp+28h] [ebp-Ch]
    float denom; // [esp+2Ch] [ebp-8h]
    float t_; // [esp+30h] [ebp-4h]
    float t_a; // [esp+30h] [ebp-4h]
    float t_exit; // [esp+40h] [ebp+Ch]
    float t_enter; // [esp+44h] [ebp+10h]

    dir = p1->x - p0->x;
    m_list_contact_point = cman->m_list_contact_point;
    v7 = &m_list_contact_point[cman->m_list_contact_point_count - 1];
    v8 = *v7;
    dir_4 = p1->y - p0->y;
    v9 = 0.0;
    t_enter = 0.0;
    t_exit = 1.0;
    if ( m_list_contact_point <= v7 )
    {
        v10 = 0.00009999999747378752;
        while ( 1 )
        {
            v12 = *m_list_contact_point;
            v26 = v8->m_contact_p.x - (*m_list_contact_point)->m_contact_p.x;
            v29 = v8->m_contact_p.y - (*m_list_contact_point)->m_contact_p.y;
            normal = -v29;
            v32 = v8->m_contact_p.x - p0->x;
            v35 = v8->m_contact_p.y - p0->y;
            t_ = v35 * v26 + v32 * normal;
            denom = v26 * dir_4 + normal * dir;
            v13 = denom;
            if ( denom <= -0.00009999999747378752 || v13 >= v10 )
            {
                t_a = t_ / v13;
                v17 = v13 < v9;
                v18 = t_a;
                if ( v17 )
                {
                    if ( t_enter < v18 )
                        t_enter = t_a;
                }
                else if ( t_exit > v18 )
                {
                    t_exit = t_a;
                }
                if ( t_exit < (double)t_enter )
                    return;
                v19 = v10;
                v15 = v9;
                v16 = v19;
            }
            else
            {
                v14 = v10;
                v15 = v9;
                v16 = v14;
                if ( v15 > t_ )
                    return;
            }
            ++m_list_contact_point;
            v8 = v12;
            if ( m_list_contact_point > v7 )
                break;
            v11 = v15;
            v10 = v16;
            v9 = v11;
        }
    }
    if ( t_exit < (double)t_enter
        && _tlAssert("source/phys_contact_manifold.cpp", 294, "t_enter <= t_exit", "") )
    {
        __debugbreak();
    }
    if ( cman->m_list_contact_point_count < 2 )
    {
        if ( _tlAssert("source/phys_contact_manifold.cpp", 295, "cman->m_list_contact_point_count >= 2", "") )
            __debugbreak();
    }
    v20 = cman->m_list_contact_point;
    this->m_list_isect_point = v20;
    this->m_contact_point_count = 2;
    v21 = *v20;
    v27 = dir * t_enter;
    v30 = t_enter * dir_4;
    v33 = p0->x + v27;
    v22 = v30 + p0->y;
    v21->m_contact_p.x = v33;
    v36 = v22;
    v21->m_contact_p.y = v36;
    v23 = *((unsigned int *)this->m_list_isect_point + 1);
    v28 = dir * t_exit;
    v31 = t_exit * dir_4;
    v34 = p0->x + v28;
    v24 = v31 + p0->y;
    *(float *)(v23 + 16) = v34;
    v37 = v24;
    *(float *)(v23 + 20) = v37;
}

// aislop
bool phys_contact_manifold_process::find_bottom(
                phys_contact_manifold_process::bridge *b,
                phys_contact_manifold_process::isect_info *left_cman,
                phys_contact_manifold_process::isect_info *right_cman)
{
    contact_manifold_mesh_point **left_next;
    contact_manifold_mesh_point **right_prev;
    phys_vec2 left_edge;
    phys_vec2 right_edge;
    phys_vec2 left_base_p;   // saved base point of left edge (left_pt->m_contact_p)
    phys_vec2 right_base_p;  // saved base point of right edge (right_prev_pt->m_contact_p)
    float det;
    float t;

    contact_manifold_mesh_point **start_left_i = b->m_left_i;
    contact_manifold_mesh_point **start_right_i = b->m_right_i;

    contact_manifold_mesh_point **l_i, **r_i;
    do
    {
        l_i = b->m_left_i;
        r_i = b->m_right_i;

        // Compute left edge: current left point -> next left point
        contact_manifold_mesh_point **left_i = b->m_left_i;
        contact_manifold_mesh_point **left_next_i = (left_i == left_cman->m_last_i)
            ? left_cman->m_cman->m_list_contact_point
            : left_i + 1;

        contact_manifold_mesh_point *left_pt = *left_i;
        contact_manifold_mesh_point *left_next_pt = *left_next_i;

        left_base_p = left_pt->m_contact_p;
        left_edge.x = left_next_pt->m_contact_p.x - left_base_p.x;
        left_edge.y = left_next_pt->m_contact_p.y - left_base_p.y;

        // Advance right pointer while right_prev is below the left edge
        while (true)
        {
            contact_manifold_mesh_point **right_i = b->m_right_i;
            right_prev = (right_i == right_cman->m_cman->m_list_contact_point)
                ? right_cman->m_last_i
                : right_i - 1;

            phys_vec2 to_right_prev;
            to_right_prev.x = (*right_prev)->m_contact_p.x - left_base_p.x;
            to_right_prev.y = (*right_prev)->m_contact_p.y - left_base_p.y;

            float cross = to_right_prev.y * left_edge.x - left_edge.y * to_right_prev.x;
            if (cross >= 0.0f)
                break;

            b->m_right_i = right_prev;
            if (right_prev == start_right_i)
                return false;
        }

        // Compute right edge: right_prev_pt -> current right point (i.e. prev -> cur on right poly)
        contact_manifold_mesh_point **right_i = b->m_right_i;
        right_prev = (right_i == right_cman->m_cman->m_list_contact_point)
            ? right_cman->m_last_i
            : right_i - 1;

        contact_manifold_mesh_point *right_pt = *right_i;
        contact_manifold_mesh_point *right_prev_pt = *right_prev;

        right_base_p = right_prev_pt->m_contact_p;
        right_edge.x = right_prev_pt->m_contact_p.x - right_pt->m_contact_p.x;
        right_edge.y = right_prev_pt->m_contact_p.y - right_pt->m_contact_p.y;

        // Advance left pointer while left_next is above the right edge
        while (true)
        {
            contact_manifold_mesh_point **left_i2 = b->m_left_i;
            left_next = (left_i2 == left_cman->m_last_i)
                ? left_cman->m_cman->m_list_contact_point
                : left_i2 + 1;

            phys_vec2 to_left_next;
            to_left_next.x = (*left_next)->m_contact_p.x - right_base_p.x;
            to_left_next.y = (*left_next)->m_contact_p.y - right_base_p.y;

            float cross = to_left_next.y * right_edge.x - right_edge.y * to_left_next.x;
            if (cross <= 0.0f)
                break;

            b->m_left_i = left_next;
            if (left_next == start_left_i)
                return false;
        }

    } while (l_i != b->m_left_i || r_i != b->m_right_i);

    // Compute intersection of the two supporting lines
    det = right_edge.y * left_edge.x - left_edge.y * right_edge.x;

    if (det <= 0.0001f)
    {
        // Nearly parallel — use right point directly
        b->m_intersection_p = (*b->m_right_i)->m_contact_p;
    }
    else
    {
        // Cramer's rule for intersection parameter t along left_edge
        phys_vec2 delta;
        delta.x = right_base_p.x - left_base_p.x;
        delta.y = right_base_p.y - left_base_p.y;

        float numer = right_edge.y * delta.x - right_edge.x * delta.y;
        t = numer / det;

        b->m_intersection_p.x = left_base_p.x + left_edge.x * t;
        b->m_intersection_p.y = left_base_p.y + left_edge.y * t;
    }

    return true;
}

void __thiscall phys_contact_manifold::comp_feature_normal()
{
    phys_memory_heap *m_allocator; // esi
    const char *v3; // ecx
    unsigned int v4; // eax
    int m_list_mesh_point_count; // ecx
    phys_memory_heap *v6; // eax
    const char *v7; // edx
    contact_manifold_mesh_point **m_list_sorted_mesh_point; // eax
    contact_manifold_mesh_point *m_list_mesh_point; // ecx
    unsigned int v10; // esi
    bool v11; // cc
    double v12; // st7
    contact_manifold_mesh_point **v13; // edx
    float *v14; // ecx
    double v15; // st6
    double v16; // st4
    double v17; // st5
    double v18; // st2
    float *v19; // eax
    float *v20; // eax
    contact_manifold_mesh_point **v21; // eax
    int m_close_mesh_point_count; // edx
    unsigned int v23; // ecx
    float *v24; // eax
    float *v25; // ecx
    double v26; // st6
    double v27; // st4
    double v28; // st5
    double v29; // st3
    double v30; // st2
    char v31; // cl
    float **v32; // esi
    float *v33; // edx
    float *v34; // edx
    float *v35; // edx
    float *v36; // edx
    float *v37; // edx
    double v38; // st6
    contact_manifold_mesh_point **v39; // edx
    float **v40; // esi
    float *v41; // ecx
    double v42; // st5
    double v43; // st5
    float v44; // [esp-1Ch] [ebp-5Ch]
    float v45; // [esp-1Ch] [ebp-5Ch]
    float v46; // [esp-1Ch] [ebp-5Ch]
    float v47; // [esp-18h] [ebp-58h]
    float v48; // [esp-18h] [ebp-58h]
    float v49; // [esp-18h] [ebp-58h]
    float v50; // [esp-14h] [ebp-54h]
    float v51; // [esp-14h] [ebp-54h]
    float v52; // [esp-14h] [ebp-54h]
    float v53; // [esp-Ch] [ebp-4Ch]
    float v54; // [esp-Ch] [ebp-4Ch]
    float v55; // [esp-Ch] [ebp-4Ch]
    float v56; // [esp-Ch] [ebp-4Ch]
    float v57; // [esp-8h] [ebp-48h]
    float v58; // [esp-8h] [ebp-48h]
    float v59; // [esp-8h] [ebp-48h]
    float v60; // [esp-8h] [ebp-48h]
    float v61; // [esp-4h] [ebp-44h]
    float v62; // [esp-4h] [ebp-44h]
    float v63; // [esp-4h] [ebp-44h]
    float v64; // [esp-4h] [ebp-44h]
    unsigned int v65; // [esp+10h] [ebp-30h]
    float v66; // [esp+10h] [ebp-30h]
    float v67; // [esp+10h] [ebp-30h]
    float v68; // [esp+10h] [ebp-30h]
    float v69; // [esp+10h] [ebp-30h]
    float v70; // [esp+10h] [ebp-30h]
    float v71; // [esp+14h] [ebp-2Ch]
    float v72; // [esp+14h] [ebp-2Ch]
    float v73; // [esp+14h] [ebp-2Ch]
    float v74; // [esp+14h] [ebp-2Ch]
    float **v75; // [esp+14h] [ebp-2Ch]
    float v76; // [esp+14h] [ebp-2Ch]
    contact_manifold_mesh_point ***v77; // [esp+18h] [ebp-28h]
    float v78; // [esp+18h] [ebp-28h]
    float dot_p_fn; // [esp+1Ch] [ebp-24h]
    contact_manifold_mesh_point **start_mp; // [esp+20h] [ebp-20h]
    contact_manifold_mesh_point **mp_j_it; // [esp+24h] [ebp-1Ch]
    float best_cos_sqa; // [esp+28h] [ebp-18h]
    float best_cos_sqb; // [esp+28h] [ebp-18h]
    float best_cos_sqc; // [esp+28h] [ebp-18h]
    float best_cos_sq; // [esp+28h] [ebp-18h]
    unsigned int i; // [esp+2Ch] [ebp-14h]
    float v87; // [esp+30h] [ebp-10h]
    float v88; // [esp+30h] [ebp-10h]
    float v89; // [esp+30h] [ebp-10h]
    float v90; // [esp+30h] [ebp-10h]
    float v91; // [esp+30h] [ebp-10h]
    float v92; // [esp+30h] [ebp-10h]
    float v93; // [esp+30h] [ebp-10h]
    float v94; // [esp+30h] [ebp-10h]
    float v95; // [esp+30h] [ebp-10h]
    float v96; // [esp+30h] [ebp-10h]
    float v97; // [esp+30h] [ebp-10h]
    float v98; // [esp+30h] [ebp-10h]
    float v99; // [esp+30h] [ebp-10h]
    float v100; // [esp+30h] [ebp-10h]
    float v101; // [esp+30h] [ebp-10h]

    if ( !this->m_list_mesh_point
        && _tlAssert("source/phys_contact_manifold.cpp", 49, "m_list_mesh_point != NULL", "") )
    {
        __debugbreak();
    }
    if ( this->m_list_sorted_mesh_point
        && _tlAssert("source/phys_contact_manifold.cpp", 50, "m_list_sorted_mesh_point == NULL", "") )
    {
        __debugbreak();
    }
    m_allocator = this->m_allocator;
    v3 = g_contact_manifold_error_msg;
    v4 = (int)(m_allocator->m_buffer_cur + 3) & 0xFFFFFFFC;
    m_allocator->m_buffer_cur = (char *)v4;
    if ( (char *)v4 >= m_allocator->m_buffer_end
        && _tlAssert(
                 "C:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\phys_mem.h",
                 114,
                 "m_buffer_cur < m_buffer_end",
                 v3) )
    {
        __debugbreak();
    }
    m_list_mesh_point_count = this->m_list_mesh_point_count;
    this->m_list_sorted_mesh_point = (contact_manifold_mesh_point **)m_allocator->m_buffer_cur;
    v6 = this->m_allocator;
    v7 = g_contact_manifold_error_msg;
    v6->m_buffer_cur += 4 * m_list_mesh_point_count;
    if ( v6->m_buffer_cur > v6->m_buffer_end
        && _tlAssert(
                 "C:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\phys_mem.h",
                 122,
                 "m_buffer_cur <= m_buffer_end",
                 v7) )
    {
        __debugbreak();
    }
    m_list_sorted_mesh_point = this->m_list_sorted_mesh_point;
    m_list_mesh_point = this->m_list_mesh_point;
    v10 = (unsigned int)&m_list_sorted_mesh_point[this->m_list_mesh_point_count];
    for ( i = v10; (unsigned int)m_list_sorted_mesh_point < v10; ++m_list_mesh_point )
        *m_list_sorted_mesh_point++ = m_list_mesh_point;
    v11 = this->m_list_mesh_point_count < 2;
    this->m_list_mesh_point = 0;
    if ( v11 && _tlAssert("source/phys_contact_manifold.cpp", 61, "m_list_mesh_point_count >= 2", "") )
        __debugbreak();
    v12 = 0.0;
    if ( this->m_list_mesh_point_count == 2 )
    {
        v13 = this->m_list_sorted_mesh_point;
        v14 = (float *)v13[1];
        v53 = *v14 - (*v13)->m_p.x;
        v57 = v14[1] - (*v13)->m_p.y;
        v61 = v14[2] - (*v13)->m_p.z;
        v15 = v57;
        v16 = v53;
        v17 = v61;
        v71 = v61 * v61 + v57 * v57 + v53 * v53;
        v18 = v71;
        if ( v71 <= 0.000011559999 )
        {
            this->m_feature_normal.x = this->m_feature_hitn.x;
            this->m_feature_normal.y = this->m_feature_hitn.y;
            this->m_feature_normal.z = this->m_feature_hitn.z;
        }
        else
        {
            v72 = this->m_feature_hitn.y * v15 + v16 * this->m_feature_hitn.x + this->m_feature_hitn.z * v17;
            v73 = v72 / v18;
            v54 = v16 * v73;
            v58 = v15 * v73;
            v62 = v73 * v17;
            v44 = this->m_feature_hitn.x - v54;
            v47 = this->m_feature_hitn.y - v58;
            v50 = this->m_feature_hitn.z - v62;
            this->m_feature_normal.x = v44;
            this->m_feature_normal.y = v47;
            this->m_feature_normal.z = v50;
            v74 = v50 * v50 + v47 * v47 + v44 * v44;
            if ( v74 <= 0.000011559999 )
            {
                this->m_feature_normal.x = this->m_feature_hitn.x;
                this->m_feature_normal.y = this->m_feature_hitn.y;
                this->m_feature_normal.z = this->m_feature_hitn.z;
            }
            v12 = 0.0;
        }
        v19 = (float *)*v13;
        *v19 = this->m_feature_hitp.x + (*v13)->m_p.x;
        v19[1] = this->m_feature_hitp.y + v19[1];
        v19[2] = this->m_feature_hitp.z + v19[2];
        v20 = (float *)*((unsigned int *)this->m_list_sorted_mesh_point + 1);
        *v20 = this->m_feature_hitp.x + *v20;
        v20[1] = this->m_feature_hitp.y + v20[1];
        v20[2] = this->m_feature_hitp.z + v20[2];
    }
    else
    {
        feature_qsort(this->m_list_sorted_mesh_point, this->m_list_mesh_point_count);
        v21 = this->m_list_sorted_mesh_point;
        this->m_feature_normal.x = this->m_feature_hitn.x;
        m_close_mesh_point_count = this->m_close_mesh_point_count;
        this->m_feature_normal.y = this->m_feature_hitn.y;
        v23 = (unsigned int)&v21[m_close_mesh_point_count];
        this->m_feature_normal.z = this->m_feature_hitn.z;
        v75 = (float **)v23;
        dot_p_fn = 0.11697778;
        v12 = 0.0;
        if ( v23 < v10 - 4 )
        {
            do
            {
                mp_j_it = *(contact_manifold_mesh_point ***)v23;
                if ( dot_p_fn > (double)*(float *)(*(unsigned int *)v23 + 16) )
                    break;
                v65 = v23 + 4;
                v77 = (contact_manifold_mesh_point ***)(v23 + 4);
                if ( v23 + 4 < v10 )
                {
                    do
                    {
                        start_mp = *v77;
                        if ( dot_p_fn > (double)*((float *)*v77 + 4) )
                            break;
                        v24 = (float *)*v77;
                        v25 = (float *)mp_j_it;
                        v55 = v24[2] * v25[1] - v24[1] * v25[2];
                        v59 = v25[2] * *v24 - v24[2] * *v25;
                        v63 = v24[1] * *v25 - *v24 * v25[1];
                        v26 = v55;
                        v27 = v59;
                        v28 = v63;
                        best_cos_sqa = v63 * v63 + v55 * v55 + v59 * v59;
                        v29 = best_cos_sqa;
                        if ( best_cos_sqa > 0.000011559999 )
                        {
                            best_cos_sqb = this->m_feature_hitn.y * v27 + v26 * this->m_feature_hitn.x + this->m_feature_hitn.z * v28;
                            v30 = best_cos_sqb;
                            best_cos_sqc = best_cos_sqb * best_cos_sqb;
                            best_cos_sq = best_cos_sqc / v29;
                            if ( dot_p_fn < (double)best_cos_sq )
                            {
                                v31 = 1;
                                if ( v30 < 0.0 )
                                {
                                    v45 = -v26;
                                    v48 = -v27;
                                    v51 = -v28;
                                    v28 = v51;
                                    v27 = v48;
                                    v26 = v45;
                                }
                                v32 = v75;
                                if ( (unsigned int)v75 >= i )
                                    goto LABEL_94;
                                if ( (int)(i - (unsigned int)v75 + 3) / 4 < 4 )
                                {
LABEL_68:
                                    while ( (unsigned int)v32 < i )
                                    {
                                        v37 = *v32;
                                        if ( best_cos_sq > (double)(*v32)[4] )
                                            break;
                                        if ( !v31 )
                                            goto LABEL_79;
                                        if ( v37 != (float *)mp_j_it && v37 != (float *)start_mp )
                                        {
                                            v99 = v37[1] * v27 + *v37 * v26 + v37[2] * v28;
                                            if ( v99 < 0.0 )
                                            {
                                                v100 = v99 * v99;
                                                v101 = v100 / v29;
                                                if ( v101 > 0.0011560001 && v37[5] * 0.000006853876129753189 < v101 )
                                                    v31 = 0;
                                            }
                                        }
                                        ++v32;
                                    }
                                }
                                else
                                {
                                    while ( 1 )
                                    {
                                        v33 = *v32;
                                        if ( best_cos_sq > (double)(*v32)[4] )
                                            break;
                                        if ( !v31 )
                                            goto LABEL_79;
                                        if ( v33 != (float *)mp_j_it && v33 != (float *)start_mp )
                                        {
                                            v87 = v33[1] * v27 + *v33 * v26 + v33[2] * v28;
                                            if ( v87 < 0.0 )
                                            {
                                                v88 = v87 * v87;
                                                v89 = v88 / v29;
                                                if ( v89 > 0.0011560001 && v33[5] * 0.000006853876129753189 < v89 )
                                                    v31 = 0;
                                            }
                                        }
                                        v34 = v32[1];
                                        if ( best_cos_sq > (double)v34[4] )
                                            break;
                                        if ( !v31 )
                                            goto LABEL_79;
                                        if ( v34 != (float *)mp_j_it && v34 != (float *)start_mp )
                                        {
                                            v90 = v34[1] * v27 + *v34 * v26 + v34[2] * v28;
                                            if ( v90 < 0.0 )
                                            {
                                                v91 = v90 * v90;
                                                v92 = v91 / v29;
                                                if ( v92 > 0.0011560001 && v34[5] * 0.000006853876129753189 < v92 )
                                                    v31 = 0;
                                            }
                                        }
                                        v35 = v32[2];
                                        if ( best_cos_sq > (double)v35[4] )
                                            break;
                                        if ( !v31 )
                                            goto LABEL_79;
                                        if ( v35 != (float *)mp_j_it && v35 != (float *)start_mp )
                                        {
                                            v93 = v35[1] * v27 + *v35 * v26 + v35[2] * v28;
                                            if ( v93 < 0.0 )
                                            {
                                                v94 = v93 * v93;
                                                v95 = v94 / v29;
                                                if ( v95 > 0.0011560001 && v35[5] * 0.000006853876129753189 < v95 )
                                                    v31 = 0;
                                            }
                                        }
                                        v36 = v32[3];
                                        if ( best_cos_sq > (double)v36[4] )
                                            break;
                                        if ( !v31 )
                                            goto LABEL_79;
                                        if ( v36 != (float *)mp_j_it && v36 != (float *)start_mp )
                                        {
                                            v96 = v36[1] * v27 + *v36 * v26 + v36[2] * v28;
                                            if ( v96 < 0.0 )
                                            {
                                                v97 = v96 * v96;
                                                v98 = v97 / v29;
                                                if ( v98 > 0.0011560001 && v36[5] * 0.000006853876129753189 < v98 )
                                                    v31 = 0;
                                            }
                                        }
                                        v32 += 4;
                                        if ( (int)v32 >= (int)(i - 12) )
                                            goto LABEL_68;
                                    }
                                }
                                if ( v31 )
                                {
LABEL_94:
                                    dot_p_fn = best_cos_sq;
                                    this->m_feature_normal.x = v26;
                                    this->m_feature_normal.y = v27;
                                    this->m_feature_normal.z = v28;
                                }
                            }
                        }
LABEL_79:
                        ++v77;
                    }
                    while ( (unsigned int)v77 < i );
                }
                v10 = i;
                v23 = v65;
            }
            while ( v65 < i - 4 );
        }
        v78 = this->m_feature_normal.y * this->m_feature_normal.y
                + this->m_feature_normal.x * this->m_feature_normal.x
                + this->m_feature_normal.z * this->m_feature_normal.z;
        v38 = v78;
        if ( v78 <= 0.0 )
        {
            if ( _tlAssert("source/phys_contact_manifold.cpp", 132, "nfeature_normal_sq > 0.0f", "") )
                __debugbreak();
            v12 = 0.0;
            v38 = v78;
        }
        v39 = this->m_list_sorted_mesh_point;
        v40 = (float **)v39;
        if ( (unsigned int)v39 < i )
        {
            do
            {
                v41 = *v40;
                v66 = **v40 * this->m_feature_normal.x
                        + (*v40)[1] * this->m_feature_normal.y
                        + (*v40)[2] * this->m_feature_normal.z;
                v42 = v66;
                v67 = v66 * v66;
                v76 = v67 / v38;
                v68 = this->m_feature_distance_eps * this->m_feature_distance_eps;
                if ( v68 >= (double)v76 || v41[5] * this->m_sin_feautre_angular_eps_sq >= v76 )
                {
                    ++v39;
                    v70 = v42 / v38;
                    v46 = v70 * this->m_feature_normal.x;
                    v49 = v70 * this->m_feature_normal.y;
                    v43 = v70 * this->m_feature_normal.z;
                    *(v39 - 1) = (contact_manifold_mesh_point *)v41;
                    v52 = v43;
                    v56 = this->m_feature_hitp.x - v46;
                    v60 = this->m_feature_hitp.y - v49;
                    v64 = this->m_feature_hitp.z - v52;
                    *v41 = v56 + *v41;
                    v41[1] = v60 + v41[1];
                    v41[2] = v41[2] + v64;
                }
                ++v40;
            }
            while ( (unsigned int)v40 < i );
        }
        this->m_list_mesh_point_count = v39 - this->m_list_sorted_mesh_point;
    }
    v69 = this->m_feature_normal.y * this->m_feature_normal.y
            + this->m_feature_normal.x * this->m_feature_normal.x
            + this->m_feature_normal.z * this->m_feature_normal.z;
    if ( v12 >= v69 )
    {
        if ( _tlAssert("source/phys_contact_manifold.cpp", 149, "AbsSquared(m_feature_normal) > 0.0f", "") )
            __debugbreak();
    }
}

void __thiscall phys_contact_manifold::generate_convex_poly(const phys_mat44 *contact_mat)
{
    phys_memory_heap *m_allocator; // edi
    const char *v4; // ecx
    unsigned int v5; // eax
    contact_manifold_mesh_point **m_buffer_cur; // edi
    phys_memory_heap *v7; // eax
    contact_manifold_mesh_point **m_list_sorted_mesh_point; // edx
    contact_manifold_mesh_point **i; // edi
    float *v10; // eax
    double x; // st7
    double v12; // st7
    double z; // st6
    contact_manifold_mesh_point **m_list_contact_point; // ecx
    contact_manifold_mesh_point *v15; // eax
    contact_manifold_mesh_point *v16; // ecx
    phys_memory_heap *v17; // eax
    contact_manifold_mesh_point **v18; // edi
    const char *v19; // edx
    char *v20; // ecx
    float v21; // [esp+8h] [ebp-8h]
    float v22; // [esp+8h] [ebp-8h]
    float v23; // [esp+Ch] [ebp-4h]
    float v24; // [esp+Ch] [ebp-4h]
    float contact_mata; // [esp+18h] [ebp+8h]

    if ( !this->m_list_sorted_mesh_point
        && _tlAssert("source/phys_contact_manifold.cpp", 237, "m_list_sorted_mesh_point != NULL", "") )
    {
        __debugbreak();
    }
    if ( this->m_list_contact_point
        && _tlAssert("source/phys_contact_manifold.cpp", 238, "m_list_contact_point == NULL", "") )
    {
        __debugbreak();
    }
    if ( this->m_list_mesh_point_count <= 1
        && _tlAssert("source/phys_contact_manifold.cpp", 239, "m_list_mesh_point_count > 1", "") )
    {
        __debugbreak();
    }
    m_allocator = this->m_allocator;
    v4 = g_contact_manifold_error_msg;
    v5 = (int)(m_allocator->m_buffer_cur + 3) & 0xFFFFFFFC;
    m_allocator->m_buffer_cur = (char *)v5;
    if ( (char *)v5 >= m_allocator->m_buffer_end
        && _tlAssert(
                 "C:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\phys_mem.h",
                 114,
                 "m_buffer_cur < m_buffer_end",
                 v4) )
    {
        __debugbreak();
    }
    m_buffer_cur = (contact_manifold_mesh_point **)m_allocator->m_buffer_cur;
    v7 = this->m_allocator;
    this->m_list_contact_point = m_buffer_cur;
    if ( ((char *)m_buffer_cur < v7->m_buffer_start
         || (char *)&m_buffer_cur[this->m_list_mesh_point_count] > v7->m_buffer_end)
        && _tlAssert(
                 "source/phys_contact_manifold.cpp",
                 241,
                 "m_allocator->fast_is_within_buffer_limits(m_list_contact_point,sizeof(contact_manifold_mesh_point*)*m_list_mesh_point_count)",
                 g_contact_manifold_error_msg) )
    {
        __debugbreak();
    }
    m_list_sorted_mesh_point = this->m_list_sorted_mesh_point;
    for ( i = &m_list_sorted_mesh_point[this->m_list_mesh_point_count]; m_list_sorted_mesh_point != i; v10[5] = v23 )
    {
        v10 = (float *)*m_list_sorted_mesh_point;
        x = (*m_list_sorted_mesh_point++)->m_p.x;
        v21 = x * contact_mat->x.x + v10[1] * contact_mat->x.y + v10[2] * contact_mat->x.z;
        v12 = *v10 * contact_mat->y.x + contact_mat->y.y * v10[1];
        z = contact_mat->y.z;
        v10[4] = v21;
        v23 = v12 + z * v10[2];
    }
    phys_contact_manifold::generate_convex_poly_internal();
    if ( this->m_list_contact_point_count == 2 )
    {
        m_list_contact_point = this->m_list_contact_point;
        v15 = m_list_contact_point[1];
        v16 = *m_list_contact_point;
        v22 = v16->m_contact_p.x - v15->m_contact_p.x;
        v24 = v16->m_contact_p.y - v15->m_contact_p.y;
        contact_mata = v24 * v24 + v22 * v22;
        if ( contact_mata < 0.1156000047922134 )
            this->m_list_contact_point_count = 1;
    }
    if ( this->m_list_contact_point_count <= 0
        && _tlAssert("source/phys_contact_manifold.cpp", 254, "m_list_contact_point_count > 0", "") )
    {
        __debugbreak();
    }
    v17 = this->m_allocator;
    v18 = (contact_manifold_mesh_point **)v17->m_buffer_cur;
    v19 = g_contact_manifold_error_msg;
    v20 = (char *)&v18[this->m_list_contact_point_count];
    v17->m_buffer_cur = v20;
    if ( v20 > v17->m_buffer_end
        && _tlAssert(
                 "C:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\phys_mem.h",
                 122,
                 "m_buffer_cur <= m_buffer_end",
                 v19) )
    {
        __debugbreak();
    }
    if ( this->m_list_contact_point != v18 )
    {
        if ( _tlAssert("source/phys_contact_manifold.cpp", 259, "m_list_contact_point == temp", "") )
            __debugbreak();
    }
}

// aislop 
void phys_contact_manifold_process::intersect_poly_poly()
{
    // Precondition checks
    iassert(cman1.m_list_contact_point_count >= 2 && cman2.m_list_contact_point_count >= 2);
    iassert(cman1.m_list_contact_point_count > 2 || cman2.m_list_contact_point_count > 2);

    m_list_isect_point = nullptr;
    m_contact_point_count = 0;

    // --- Degenerate case: one manifold is a line segment ---
    if (cman1.m_list_contact_point_count == 2)
    {
        // Get index-1 contact point's m_contact_p (at offset +16 within mesh point)
        iassert(cman1.m_list_contact_point_count > 1);
        const phys_vec2 *p1 = &cman1.m_list_contact_point[1]->m_contact_p;
        iassert(cman1.m_list_contact_point_count > 0);
        intersect_poly_segment(&cman2,
            &cman1.m_list_contact_point[0]->m_contact_p,
            p1);
        return;
    }
    else if (cman2.m_list_contact_point_count == 2)
    {
        iassert(cman2.m_list_contact_point_count > 1);
        const phys_vec2 *p1 = &cman2.m_list_contact_point[1]->m_contact_p;
        iassert(cman2.m_list_contact_point_count > 0);
        intersect_poly_segment(&cman1,
            &cman2.m_list_contact_point[0]->m_contact_p,
            p1);
        return;
    }

    // --- General polygon-polygon case: rotating calipers intersection ---

    isect_info gb_cman1, gb_cman2;
    gb_cman1.init(&cman1);
    gb_cman2.init(&cman2);

    isect_info *left_gb = &gb_cman1;
    isect_info *right_gb = &gb_cman2;

    // Allocate bridge array from bump allocator (4-byte aligned)
    bridge *list_bridge = (bridge *)m_allocator.fast_align_start(4, g_contact_manifold_error_msg);
    bridge *b = list_bridge;

    int total_verts = gb_cman1.m_cman->m_list_contact_point_count
        + gb_cman2.m_cman->m_list_contact_point_count;

    // Rotating calipers loop — runs total_verts+1 times
    int bridge_i = 0;
    if (total_verts >= 0)
    {
        do
        {
            // Cross product of the two current caliper edges to determine which to advance
            float cross = right_gb->m_edge.y * left_gb->m_edge.x
                - left_gb->m_edge.y * right_gb->m_edge.x;

            b->m_right_i = right_gb->m_i;
            b->m_left_i = left_gb->m_i;

            // Advance the caliper with the smaller angle (more clockwise edge)
            isect_info *to_advance;
            phys_vec2 caliper_dir;
            if (cross < 0.0f)
            {
                caliper_dir = right_gb->m_edge;
                to_advance = right_gb;
            }
            else
            {
                caliper_dir = left_gb->m_edge;
                to_advance = left_gb;
            }
            to_advance->update();

            // Compute displacement vector between current caliper vertices
            phys_vec2 disp;
            disp.x = (*right_gb->m_i)->m_contact_p.x - (*left_gb->m_i)->m_contact_p.x;
            disp.y = (*right_gb->m_i)->m_contact_p.y - (*left_gb->m_i)->m_contact_p.y;

            // cr = signed area / cross product test
            float cr = disp.y * caliper_dir.x - caliper_dir.y * disp.x;

            if (cr <= 0.0f)
            {
                if (cr >= 0.0f)
                {
                    // cr == 0: parallel calipers — displace the right vertex slightly
                    phys_vec2 vdisplace;
                    vdisplace.x = -caliper_dir.y;
                    vdisplace.y = caliper_dir.x;

                    float nvdisplace = sqrtf(vdisplace.x * vdisplace.x + vdisplace.y * vdisplace.y);
                    iassert(nvdisplace > 0.0001f);

                    float scale = 0.034f / nvdisplace;
                    vdisplace.x *= scale;
                    vdisplace.y = scale * caliper_dir.x;  // note: y recomputed from original caliper_dir.x

                    displace_contact_p(right_gb->m_i, &vdisplace, &contact_mat);
                }
                else
                {
                    // cr < 0: potential bridge found
                    if (bridge_i > 0)
                    {
                        //iassert(m_allocator.fast_is_within_buffer_limits(b, sizeof(bridge)));

                        if (!find_bottom(b, left_gb, right_gb))
                            return;

                        ++b;
                    }

                    // Swap left/right for next iteration
                    std::swap(left_gb, right_gb);
                }
            }

            ++bridge_i;
        } while (bridge_i <= total_verts);
    }

    int num_bridges = (int)((char *)b - (char *)list_bridge) >> 4;

    if (num_bridges % 2)
    {
        // Odd number of bridges — degenerate, fall back to larger perimeter polygon
        tlWarning("contact manifold intersect poly poly failed.\n");

        float perim1 = cman1.compute_convex_poly_perimeter();
        float perim2 = cman2.compute_convex_poly_perimeter();

        phys_contact_manifold *best = (perim1 > perim2) ? &cman2 : &cman1;

        m_list_isect_point = best->m_list_contact_point;
        m_contact_point_count = best->m_list_contact_point_count;
    }
    else
    {
        // Even number of bridges — normal case

        // Advance allocator past the bridge array we just filled
        char *temp_ptr = m_allocator.m_buffer_cur;
        m_allocator.m_buffer_cur = temp_ptr + 16 * num_bridges;
        iassert(m_allocator.m_buffer_cur <= m_allocator.m_buffer_end);
        iassert(temp_ptr == (char *)list_bridge);

        if (num_bridges == 0)
        {
            // No bridges — entire right polygon is the intersection
            m_list_isect_point = right_gb->m_cman->m_list_contact_point;
            m_contact_point_count = right_gb->m_cman->m_list_contact_point_count;
        }
        else
        {
            // Build intersection point list by stitching together bridge arcs
            contact_manifold_mesh_point **ip_list =
                (contact_manifold_mesh_point **)m_allocator.fast_align_start(4, g_contact_manifold_error_msg);

            m_list_isect_point = ip_list;

            char *ip_cur = (char *)ip_list;

            for (int k = 0; k < num_bridges; ++k)
            {
                bridge *bk = &list_bridge[k];

                // Emit bk->m_left_i's vertex into the intersection list
                //iassert(m_allocator.fast_is_within_buffer_limits(ip_cur, sizeof(contact_manifold_mesh_point *)));

                // Duplicate check
                for (contact_manifold_mesh_point **chk = ip_list;
                    (char *)chk != ip_cur;
                    ++chk)
                {
                    if (*chk == *bk->m_left_i)
                        tlWarning("contact manifold failure.");
                }

                // Write the vertex pointer
                *(contact_manifold_mesh_point **)ip_cur = *bk->m_left_i;

                // Copy the bridge's intersection point into the vertex's m_contact_p
                contact_manifold_mesh_point *vtx = *bk->m_left_i;
                vtx->m_contact_p = bk->m_intersection_p;

                ip_cur += sizeof(contact_manifold_mesh_point *);

                // Walk the arc along left_gb's polygon from bk->m_left_i up to
                // list_bridge[(k+1) % num_bridges]->m_right_i
                contact_manifold_mesh_point **arc_cur = bk->m_left_i;
                contact_manifold_mesh_point **arc_last = left_gb->m_last_i;
                contact_manifold_mesh_point **arc_end = list_bridge[(k + 1) % num_bridges].m_right_i;

                // Advance one step (past the bridge vertex we just emitted)
                if (arc_cur == arc_last)
                    arc_cur = left_gb->m_cman->m_list_contact_point;
                else
                    arc_cur = arc_cur + 1;

                while (arc_cur != arc_end)
                {
                    //iassert(m_allocator.fast_is_within_buffer_limits(ip_cur, sizeof(contact_manifold_mesh_point *)));

                    for (contact_manifold_mesh_point **chk = ip_list;
                        (char *)chk != ip_cur;
                        ++chk)
                    {
                        if (*chk == *arc_cur)
                            tlWarning("contact manifold failure.");
                    }

                    *(contact_manifold_mesh_point **)ip_cur = *arc_cur;
                    ip_cur += sizeof(contact_manifold_mesh_point *);

                    if (arc_cur == arc_last)
                        arc_cur = left_gb->m_cman->m_list_contact_point;
                    else
                        arc_cur = arc_cur + 1;
                }

                // Swap left/right for next bridge
                std::swap(left_gb, right_gb);
            }

            m_contact_point_count =
                (int)((char *)ip_cur - (char *)ip_list) / sizeof(contact_manifold_mesh_point *);

            // Commit the intersection point array into the allocator
            char *temp2 = m_allocator.m_buffer_cur;
            m_allocator.m_buffer_cur = temp2 + sizeof(contact_manifold_mesh_point *) * m_contact_point_count;
            iassert(m_allocator.m_buffer_cur <= m_allocator.m_buffer_end);
            iassert(temp2 == (char *)ip_list);
        }
    }
}


bool __cdecl phys_contact_manifold::rht(const phys_vec2 *e1, const phys_vec2 *e2, float min_length2, float min_sin_sq)
{
    double v5; // st7
    double v6; // st6
    bool result; // al
    float cra; // [esp+Ch] [ebp+Ch]
    float crb; // [esp+Ch] [ebp+Ch]
    float cr; // [esp+Ch] [ebp+Ch]
    float crc; // [esp+Ch] [ebp+Ch]

    cra = e1->x * e2->y - e2->x * e1->y;
    v5 = cra;
    result = 0;
    if ( cra > 0.0 )
    {
        crb = e1->y * e1->y + e1->x * e1->x;
        v6 = crb;
        if ( min_length2 < (double)crb )
        {
            cr = e2->y * e2->y + e2->x * e2->x;
            if ( cr > (double)min_length2 )
            {
                crc = v5 * v5 / (v6 * cr);
                if ( min_sin_sq < (double)crc )
                    return 1;
            }
        }
    }
    return result;
}

void __thiscall phys_contact_manifold_process::isect_info::init(phys_contact_manifold *cman)
{
    contact_manifold_mesh_point **m_list_contact_point; // edx
    contact_manifold_mesh_point **m_i; // edx
    float *v4; // eax
    contact_manifold_mesh_point *v5; // edx
    phys_vec2 v6; // [esp+0h] [ebp-8h]

    this->m_cman = cman;
    m_list_contact_point = cman->m_list_contact_point;
    this->m_i = m_list_contact_point;
    this->m_next_i = m_list_contact_point + 1;
    m_i = this->m_i;
    this->m_last_i = &m_i[cman->m_list_contact_point_count - 1];
    v4 = (float *)*m_i;
    v5 = *this->m_next_i;
    v4 += 4;
    v6.x = v5->m_contact_p.x - *v4;
    v6.y = v5->m_contact_p.y - v4[1];
    this->m_edge = v6;
}

void __thiscall phys_contact_manifold_process::isect_info::update()
{
    contact_manifold_mesh_point **m_next_i; // eax
    contact_manifold_mesh_point **m_list_contact_point; // edx
    contact_manifold_mesh_point **m_i; // eax
    contact_manifold_mesh_point *v4; // edx
    float *p_x; // eax
    phys_vec2 v6; // [esp+0h] [ebp-8h]

    m_next_i = this->m_next_i;
    this->m_i = m_next_i;
    if ( m_next_i == this->m_last_i )
        m_list_contact_point = this->m_cman->m_list_contact_point;
    else
        m_list_contact_point = m_next_i + 1;
    m_i = this->m_i;
    this->m_next_i = m_list_contact_point;
    v4 = *m_list_contact_point;
    p_x = &(*m_i)->m_contact_p.x;
    v6.x = v4->m_contact_p.x - *p_x;
    v6.y = v4->m_contact_p.y - p_x[1];
    this->m_edge = v6;
}

char *__thiscall phys_memory_heap::fast_align_start(int alignment, const char *error_msg)
{
    char *result; // eax

    result = (char *)(~(alignment - 1) & (int)&this->m_buffer_cur[alignment - 1]);
    this->m_buffer_cur = result;
    if ( result >= this->m_buffer_end )
    {
        if ( _tlAssert(
                     "C:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\phys_mem.h",
                     114,
                     "m_buffer_cur < m_buffer_end",
                     error_msg) )
        {
            __debugbreak();
        }
        return this->m_buffer_cur;
    }
    return result;
}

