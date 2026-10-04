#pragma once
#include "phys_local.h"
#include "phys_gjk.h"

struct gjk_sep_dir // sizeof=0x0
{
    static const phys_vec3 *comp_sep_dir(phys_vec3 *result, const phys_gjk_input *m_pcd, phys_gjk_info *m_gjk_info);
    double dist_seg(const phys_vec3 *v0, const phys_vec3 *v1, const phys_vec3 *mesh_center, phys_vec3 *direction);
};
