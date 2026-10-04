#pragma once
#include <bgame/bg_public.h>
#include <qcommon/common.h>

void __cdecl G_Trigger(gentity_s *self, gentity_s *other);
char __cdecl InitTrigger(gentity_s *self);
void __cdecl InitSentientTrigger(gentity_s *self);
void __cdecl multi_trigger(gentity_s *ent);
void __cdecl Touch_Multi(gentity_s *self, gentity_s *other, int);
void __cdecl SP_trigger_multiple(gentity_s *ent, SpawnVar *spawnVar);
void __cdecl InitTriggerWait(gentity_s *ent, int spawnflag, SpawnVar *spawnVar);
void __cdecl SP_trigger_radius(gentity_s *ent, SpawnVar *spawnVar);
void __cdecl init_trigger_radius(gentity_s *ent, float radius, float height);
void __cdecl SP_trigger_radius_use(gentity_s *ent, SpawnVar *v = NULL);
void __cdecl SP_trigger_disk(gentity_s *ent, SpawnVar *spawnVar);
void __cdecl hurt_touch(gentity_s *self, gentity_s *other, int);
void __cdecl hurt_use(gentity_s *self, gentity_s *__formal, gentity_s *__formal2);
void __cdecl SP_trigger_hurt(gentity_s *self, SpawnVar *spawnVar);
void __cdecl SP_trigger_once(gentity_s *ent, SpawnVar *spawnVar = NULL);
bool __cdecl Respond_trigger_damage(gentity_s *pEnt, int iMOD);
void __cdecl Activate_trigger_damage(gentity_s *pEnt, gentity_s *pOther, int iDamage, int iMOD);
void __cdecl Use_trigger_damage(gentity_s *pEnt, gentity_s *pOther, gentity_s *);
void __cdecl Pain_trigger_damage(gentity_s *pSelf, gentity_s *pAttacker, int iDamage, const float *vPoint, const int iMod, const float *__formal, const hitLocation_t __formal2, const int __formal3);
void __cdecl Flame_trigger_damage(
                gentity_s *pSelf,
                gentity_s *pAttacker,
                int iDamage,
                float *vPoint,
                int iMod,
                float *vDir);

void __cdecl Die_trigger_damage(gentity_s *pSelf, gentity_s *pInflictor, gentity_s *pAttacker, int iDamage, int iMod, const int __formal, const float *__formal2, const hitLocation_t __formal3, int __formal4);
void __cdecl SP_trigger_damage(gentity_s *pSelf, SpawnVar *spawnVar);
void __cdecl G_CheckHitTriggerDamage(gentity_s *pActivator, float *vStart, float *vEnd, int iDamage, unsigned int iMOD);
void __cdecl G_GrenadeTouchTriggerDamage(gentity_s *pActivator, float *vStart, float *vEnd, int iDamage, int iMOD);
void __cdecl SP_trigger_lookat(gentity_s *self, SpawnVar *v = NULL);
void __cdecl trigger_ik_playerclip_terrain_touch(gentity_s *ent, gentity_s *other, int);
void __cdecl SP_trigger_ik_playerclip_terrain(gentity_s *self, SpawnVar *v = NULL);
void __cdecl trigger_ik_disable_terrain_mapping_touch(gentity_s *ent, gentity_s *other, int);
