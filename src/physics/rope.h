#pragma once
#include <gfx_d3d/r_material.h>

enum rope_constraint_e : __int32
{                                       // XREF: constraint_t/r
                                        // ?Rope_AddAnchor@@YAXHHQBMHHW4rope_constraint_e@@@Z/r
    ROPE_PAIR_CONSTRAINT    = 0x0,
    ROPE_WORLD_CONSTRAINT   = 0x1,
    ROPE_DENTITY_CONSTRAINT = 0x2,
    ROPE_CENTITY_CONSTRAINT = 0x3,
};

struct __declspec(align(4)) VisualRope // sizeof=0x20
{                                       // XREF: RopeRenderInfo/r
                                        // Rope_Draw/r
    float beginRadius;                  // XREF: Rope_Draw+DD/w
    float endRadius;                    // XREF: Rope_Draw+F5/w
    const Material *material;           // XREF: Rope_Draw+103/w
    int segmentCount;                   // XREF: Rope_Draw+114/w
    float *segmentVerts;                // XREF: Rope_Draw+10C/w
    GfxDrawSurf *drawSurf;
    unsigned __int16 *lightingHandle;   // XREF: Rope_Draw+120/w
    unsigned __int8 visibilityFlags;    // XREF: Rope_Draw+89/w
                                        // Rope_Draw+8C/r
    // padding byte
    // padding byte
    // padding byte
};

struct contact_t // sizeof=0x1C
{                                       // XREF: .data:contact_t * g_contacts/r
    float point[3];
    float normal[3];
    int particle_index;
};

struct par_t // sizeof=0x28
{                                       // XREF: rope_t/r
    float p[3];
    float p0[3];
    float p_prev[3];
    int flags;
};

struct __declspec(align(4)) constraint_t // sizeof=0x1C
{                                       // XREF: rope_t/r
    float p[3];
    rope_constraint_e type;
    int entity_index;
    int bone_name_hash;
    unsigned __int8 pi1;
    unsigned __int8 pi2;
    // padding byte
    // padding byte
};

struct rope_frame_verts_t // sizeof=0x25C
{                                       // XREF: rope_client_verts_t/r
    int num_verts;
    float v[50][3];
};

struct rope_client_verts_t // sizeof=0x4BC
{                                       // XREF: rope_t/r
    rope_frame_verts_t frame_verts[2];
    unsigned int frame_index;
};

struct __declspec(align(4)) rope_t // sizeof=0xC74
{
    par_t m_particles[25];
    constraint_t m_constraints[30];
    int m_entity_anchors[3];
    int m_num_particles;
    int m_num_constraints;
    int m_num_entity_anchors;
    int m_num_draw_verts;
    rope_client_verts_t m_client_verts;
    float m_min[3];
    float m_max[3];
    float m_start[3];
    float m_end[3];
    int m_in_use;
    int m_visible;
    int m_dist_constraint;
    int m_flags;
    const Material *m_material;
    float m_seglen;
    float m_length;
    float m_width;
    float m_scale;
    float m_force_scale;
    int m_health;
    int m_frame;
    int m_stable_count;
    int m_static_rope;
    unsigned __int16 m_lightingHandle;
    // padding byte
    // padding byte
};

struct __declspec(align(16)) RopeUpdateCmdData // sizeof=0x60
{                                       // XREF: ?Rope_Update@@YAXHH@Z/r
    float viewOrg[3];
    int ropeCount;
    float screenMtx[4][4];
    rope_t *ropes;
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
};

bool __cdecl Rope_IsValid(int rope_index);
void __cdecl Rope_SetFlag(int rope_index, int flags, int onoff);
void __cdecl Rope_Reset();
void __cdecl Rope_Reset(int rope_index);
void __cdecl Rope_SetMaterial(int rope_index, Material *const material);
int __cdecl Rope_GetNumParticles(int rope_index);
void __cdecl Rope_AddAnchor(
                int rope_index,
                char particle_index,
                const float *p,
                int entity_index,
                int bone_name_hash,
                rope_constraint_e type);
void __cdecl Rope_Activate(int rope_index);
void __cdecl Rope_AddDynEntAnchor(int rope_index, char particle_index, const float *p, int entity_index);
void __cdecl Rope_AddEntAnchor(
                int rope_index,
                char particle_index,
                const float *p,
                int entity_index,
                int bone_name_hash);
void __cdecl Rope_RemoveAnchor(int rope_index, int particle_index);
void __cdecl Rope_Create(
                int rope_index,
                const float *start,
                const float *end,
                float length,
                float scale,
                float width,
                float force_scale,
                int static_rope);
void __cdecl Rope_Create(int rope_index);
void __cdecl Rope_Init(
                int rope_index,
                int npars,
                const float *p0,
                const float *p1,
                float rope_len,
                float rope_width,
                float scale,
                float force_scale,
                int static_rope);
void __cdecl Rope_AddParticle(int rope_index, const float *pos);
void __cdecl Rope_UpdateInternal(const RopeUpdateCmdData *cmd);
void __cdecl Rope_Tick(const RopeUpdateCmdData *cmd, int rope_index, float dt, bool force_update);
void __cdecl Rope_ApplyPhysics(int rope_index, float dt);
void __cdecl Rope_ApplyConstraints(int rope_index, bool forward);
void __cdecl Rope_ApplyContactConstraint(int rope_index, int contact_index);
void __cdecl Rope_ApplyConstraint(int rope_index, int constraint_index);
void __cdecl Rope_UpdateVisible(int rope_index);
void __cdecl Rope_BuildCurve(const RopeUpdateCmdData *cmd, int rope_index);
void    Rope_Update(int localClientNum, int curtime);
void __cdecl Rope_Render(unsigned int localClientNum);
void __cdecl Rope_DebugDraw(int rope_index);
void __cdecl Rope_Draw(int rope_index, unsigned int localClientNum);
void __cdecl Rope_Settle(int rope_index);
void __cdecl Rope_ExplosionEvent(
                const float *origin,
                float innerRadius,
                float outerRadius,
                int innerDamage,
                int outerDamage);
void __cdecl Rope_ExplosionEvent(
                int rope_index,
                const float *origin,
                float innerRadius,
                float outerRadius,
                int innerDamage,
                int outerDamage);
double __cdecl PointToBoxDistSq(const float *pt, const float *mins, const float *maxs);
int __cdecl trace_point_through_sphere(
                const float *p,
                const float *ud,
                const float *ctr,
                float r,
                float *t,
                float *hitp);
void __cdecl Rope_Trace(const float *p0, const float *p1);
void __cdecl Rope_CollideWorld(int rope_index);
void __cdecl Rope_CollideEntitiesHelper(int rope_index, float *origin);
void __cdecl Rope_CollideEntities(int rope_index);

extern rope_t *g_ropes;
extern int g_ropeCount;
extern int g_ropesWithEntsAnchorsCount;
extern int g_totalRopeCount;
extern int g_mapRopeCount;
