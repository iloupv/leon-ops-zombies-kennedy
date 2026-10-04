#pragma once

#include "phys_local.h"

struct phys_convex_hull // sizeof=0x20D60
{                                       // XREF: .data:phys_convex_hull g_hull/r
    struct __declspec(align(8)) ch_triangle // sizeof=0x20
    {
        phys_vec3 m_normal;
        phys_vec3 *m_verts[3];
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        double get_dist(const phys_vec3 *vert);

        //ch_triangle &operator=(const phys_convex_hull::ch_triangle *__that);

        phys_convex_hull::ch_triangle &operator=(const phys_convex_hull::ch_triangle &rhs)
        {
            if (this == &rhs)
                return *this;

            m_normal = rhs.m_normal;

            for (int i = 0; i < 3; ++i)
                m_verts[i] = rhs.m_verts[i];

            return *this;
        }
    };
    struct ch_edge // sizeof=0x8
    {
        phys_vec3 *m_verts[2];
    };
    phys_static_array<phys_vec3, 6144> m_vertex_buffer;
    // XREF: generate_brush_chull(cbrush_t const *)+3B/r
    // generate_brush_chull(cbrush_t const *):loc_84E685/w ...
    phys_static_array<phys_vec3 *, 6144> m_intermediate_vertex_list;
    phys_static_array<phys_convex_hull::ch_triangle, 256> m_intermediate_triangle_list;
    phys_static_array<phys_convex_hull::ch_edge, 128> m_intermediate_edge_list;
    phys_static_array<phys_vec3 *, 64> m_convex_hull_vert_list;
    phys_static_array<phys_convex_hull::ch_triangle, 64> m_convex_hull_triangle_list;

    phys_convex_hull() = default;
    ~phys_convex_hull() = default;

    void compute_convex_hull(int max_verts, float min_expansion_volume_percent);
    void add_convex_hull_vert(phys_vec3 **vert);
    void create_intermediate_triangle(phys_vec3 *v0, phys_vec3 *v1, phys_vec3 *v2);
    phys_vec3 **support_intermediate_verts(const phys_vec3 *dir);
    void init_convex_hull();
    void calculate_initial_triangle_vertices();
    double calc_expansion_volume(const phys_vec3 *vert);
    double tetrahedron_volume(
        const phys_vec3 *a,
        const phys_vec3 *b,
        const phys_vec3 *c,
        const phys_vec3 *d);
    void create_edge_list(const phys_vec3 *vert);
    void add_triangle_edges(phys_convex_hull::ch_triangle *tri);
    void add_intermediate_edge(phys_vec3 *v0, phys_vec3 *v1);
    void remove_inside_verts();

};

struct cbrush_t;
struct CollisionAabbTree;
struct cLeafBrushNode_s;
struct PhysGeomList;

chull_t *__cdecl create_chull(phys_convex_hull *pch);
void __cdecl free_chull(chull_t *first);
void __cdecl chull_list_add(chull_t *chull, unsigned int key);
void __cdecl free_chull_lists();
chull_t *generate_brush_chull(const cbrush_t *brush);

chull_t *generate_partition_chull(const CollisionAabbTree *tree);
void __cdecl generate_brushmodel_chull_r(cLeafBrushNode_s *node, chull_t **hull);
chull_t *__cdecl generate_brushmodel_chull(unsigned int brushmodel);
chull_t *__cdecl generate_collmap_chull(PhysGeomList *geomList);
chull_t *__cdecl get_collmap_chull(PhysGeomList *geomList, unsigned int key);
chull_t *__cdecl get_brushmodel_chull(unsigned int brushmodel, unsigned int key);
chull_t *__cdecl get_brush_chull(const cbrush_t *brush);
chull_t *__cdecl get_partition_chull(const CollisionAabbTree *tree);