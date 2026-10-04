#pragma once
#include <physics/phys_local.h>
#include <bgame/bg_public.h>
#include <ragdoll/ragdoll.h>
#include <physics/physconstraints_load_obj.h>
#include <clientscript/cscr_variable.h>

struct __declspec(align(16)) mover_info_t // sizeof=0xC0
{
    phys_mat44 m_mat;
    phys_mat44 m_prev_mat;
    float m_origin[3];
    float m_prev_origin[3];
    float m_angles[3];
    float m_prev_angles[3];
    gentity_s *m_ent;
    bool m_do_collision;
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

    void init(float *origin, const float *angles, bool do_collision);
    void init(
        float *origin,
        const float *angles,
        float *prev_origin,
        const float *prev_angles,
        bool do_collision);

    void update(float *origin, const float *angles);

    void apply_rotation(float *v);
    void apply_rotation_translation(float *v);
};

mover_info_t *__cdecl get_mover_info(gentity_s *ent);
mover_info_t *__cdecl create_mover_info(gentity_s *ent);
bool __cdecl entity_is_a_mover(int entnum);

void __cdecl G_PushEntity(gentity_s *check, gentity_s *pusher, const float *move, const float *amove, mover_info_t *mi);
char __cdecl IsEntityLinkedToMe(gentity_s *self, gentity_s *other);
gentity_s *__cdecl GetEntity(const unsigned __int16 *targetname);
void    CreateConstraint(PhysConstraint *constraint);
void __cdecl DeleteConstraint(int rope_index);
void __cdecl G_MoverTeam(gentity_s *ent, mover_info_t *mi);
void __cdecl G_MoverPush(gentity_s *pusher, mover_info_t *mi);
void __cdecl G_CalcEntityPhysicsPositions(gentity_s *ent);
void __cdecl G_CreatePhysicsObject(gentity_s *ent);
void __cdecl G_UpdatePhysicsPose(gentity_s *ent);
void __cdecl G_MoverTeam_New(gentity_s *ent);

void __cdecl G_RunMover(gentity_s *ent);
void __cdecl trigger_use_shared(gentity_s *self, SpawnVar *spawnVar);
void __cdecl trigger_use_touch(gentity_s *ent, SpawnVar *spawnVar);
//mover_info_t *__thiscall phys_simple_allocator<mover_info_t>::allocate(phys_simple_allocator<mover_info_t> *this);

extern const char *hintStrings[8];