#pragma once
#include "phys_local.h"
#include "phys_collision.h"
#include "phys_colgeom.h"
#include "phys_task_manager.h"
#include "phys_gjk.h"

struct phys_contact_manifold_process;

void __cdecl process_cpi(contact_point_info *cpi);
void __cdecl process_list_do_gjk_collide_and_contact_manifold(phys_link_list<phys_collision_pair> *list_pcd);
int    phys_gjk_collide_jq_batch_function(jqBatch *pBatch);
void    phys_collide_do_gjk_collide_and_contact_manifold(
    phys_collision_pair *pcp,
    phys_gjk_info *gjk_info,
    phys_contact_manifold_process *cman_process);