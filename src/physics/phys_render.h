#pragma once
#include "phys_local.h"
#include "phys_colgeom.h"
#include "phys_main.h"
#include "phys_traverse.h"

struct __declspec(align(4)) debug_patch_t // sizeof=0xC
{                                       // XREF: debug_patch_info_t/r
    const CollisionAabbTree *tree;
    unsigned int first_vertex_index;
    unsigned __int16 num_indices;
    // padding byte
    // padding byte
};

struct debug_patch_info_t // sizeof=0x704E8
{
    debug_patch_t patches[5000];
    unsigned int indices[100000];
    unsigned int num_patches;
    unsigned int num_indices;

    void add_patch(const CollisionAabbTree *tree);
    unsigned int find_index_in_clipmap(const CollisionAabbTree *tree, const float *v);
};

struct debug_brush_t // sizeof=0x10
{                                       // XREF: debug_brush_info_t/r
    const cbrush_t *brush;
    unsigned int first_vertex_index;
    unsigned int first_winding_index;
    unsigned __int16 num_indices;
    unsigned __int16 num_windings;
};

struct debug_brush_info_t // sizeof=0x77A1C
{
    debug_brush_t brushes[5000];
    unsigned __int8 windings[50000];
    unsigned __int8 indices[360000];
    int num_brushes;
    int num_windings;
    int num_indices;

    void add_brush(const cbrush_t *brush, const phys_mat44 *mat);
    __int16 find_index_in_brush(const cbrush_t *brush, const float *v);
};

struct GjkTraceGeom // sizeof=0x50
{
    const cbrush_t *brush;
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    phys_mat44 mat;

    static GjkTraceGeom geoms[50];
    static int nGeoms;

    static void Render();
};

struct __declspec(align(4)) colgeom_debug_renderer_t : colgeom_visitor_t // sizeof=0x80
{                                       // XREF: ?Phys_RenderWorldCollMesh@@YAXQBM_N1@Z/r
    float lightPos[3];                  // XREF: Phys_RenderWorldCollMesh(float const * const,bool,bool)+1BB/w
                                        // Phys_RenderWorldCollMesh(float const * const,bool,bool)+1C8/w ...
    bool bRenderStaticCollision;        // XREF: Phys_RenderWorldCollMesh(float const * const,bool,bool)+1AB/w
    bool bRenderBrushes;                // XREF: Phys_RenderWorldCollMesh(float const * const,bool,bool)+1B1/w
    // padding byte
    // padding byte

    void reset();
    void visit(const CollisionAabbTree *tree);
    void visit(const cbrush_t *brush);
    void update(
        const float *_mn,
        const float *_mx,
        int mask,
        const float *expand_vec);
};


void __cdecl DebugPatchesAndBrushesProlog();
void __cdecl DebugPatchesAndBrushesEpilog();
void __cdecl clear_debug_brushes_and_patches();
void __cdecl add_debug_brush(const cbrush_t *brush, const phys_mat44 *mat);


void __cdecl add_debug_patch(const CollisionAabbTree *tree);
void __cdecl render_debug_brushes();
void __cdecl RB_CheckTessOverflow(int vertexCount, int indexCount);
void __cdecl render_debug_patches_full();
void __cdecl render_debug_brushes_and_patches();
void    render_brush(
                const cbrush_t *brush,
                const phys_mat44 *xform,
                const float *color,
                bool bLinesOnly,
                const float *lightPos,
                bool persistent,
                bool ztest,
                const float *edgeColor);
void __cdecl calc_color(unsigned int id, float alpha, float *color);
void __cdecl render_convex_partition(const CollisionAabbTree *tree);
void    render_chull(const chull_t *first, const phys_mat44 *xform, const float *color);
void __cdecl render_xmodel_chull(const XModel *model, unsigned int key, const phys_mat44 *xform, const float *color);
void __cdecl render_brushmodel_chull(
                unsigned int brushmodel,
                unsigned int key,
                const phys_mat44 *xform,
                const float *color);
void __cdecl render_line(const float *p0, const float *p1, const float *color, int duration, int ztest);
void __cdecl render_line(const phys_vec3 *p0, const phys_vec3 *p1, const float *color, int duration, int ztest);
void    render_box(
                const phys_vec3 *mins,
                const phys_vec3 *maxs,
                const phys_mat44 *xform,
                const float *color,
                int duration);
void    render_box(float *_mn, float *_mx, const float *color, int duration);
void __cdecl render_box(const phys_vec3 *mins, const phys_vec3 *maxs, const float *color, int duration);
void    render_gjk_geom(gjk_base_t *geom, const phys_mat44 *cg2w);
void    debug_render(PhysObjUserData *userData);
void    clip_winding(phys_static_array<phys_vec3, 512> &winding, const plane_lt &clip);
void    init_winding(const plane_lt *plane, phys_static_array<phys_vec3,512> *winding);
void    calc_winding(
                const phys_static_array<plane_lt,512> &planes,
                int plane_index,
                phys_static_array<phys_vec3,512> *winding);
void __cdecl Phys_DebugPoint(const phys_vec3 *pos, float radius, const float *color);
void __cdecl Phys_DebugBox(const phys_mat44 *mat, const float *color, float scale);
void __cdecl Phys_DebugBox(const phys_mat44 *mat, const phys_vec3 *dims, const float *color);
void    Phys_RenderBase(const phys_mat44 *mat, float scale);
void    Phys_JointDebugRender(int id);
void    render_contact(rigid_body_constraint_contact *rbc);
void __cdecl render_collision_tree(const CollisionAabbTree *tree, const float *color);
void __cdecl render_prims(col_prim_t *prims, int nprims);
void __cdecl render_debug_draw_gjk_trace_geom();


