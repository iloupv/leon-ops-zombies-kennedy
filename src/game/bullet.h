#pragma once
#include <bgame/bg_weapons.h>
#include <bgame/bg_local.h>
#include <bgame/bg_public.h>

double __cdecl G_GoodRandomFloat(int *idum);
void __cdecl Bullet_Endpos(int randSeed, float spread, float *end, float *dir, const weaponParms *wp, float maxRange);
void __cdecl Bullet_RandomDir(int time, float *x, float *y);
bool __cdecl BG_WeaponBulletFire_ExplodeOnImpact(const WeaponDef *weapDef);
bool __cdecl BG_WeaponBulletFire_ShouldPenetrate(const WeaponDef *weapDef);
void __cdecl Bullet_Fire(
                gentity_s *attacker,
                float spread,
                const weaponParms *wp,
                const gentity_s *weaponEnt,
                int gameTime);
void __cdecl Bullet_FireExtended(
                BulletFireParams *bp,
                const WeaponVariantDef *weapVariantDef,
                gentity_s *attacker,
                int gameTime);
char __cdecl Bullet_Trace(
                const BulletFireParams *bp,
                const WeaponVariantDef *weapVariantDef,
                gentity_s *attacker,
                BulletTraceResults *br,
                unsigned int lastSurfaceType);
bool __cdecl Bullet_IgnoreHitEntity(const BulletFireParams *bp, const BulletTraceResults *br, gentity_s *attacker);
void __cdecl Bullet_Process(
                BulletFireParams *bp,
                BulletTraceResults *br,
                const WeaponVariantDef *weapVariantDef,
                gentity_s *attacker,
                int dFlags,
                int gameTime,
                int *outImpactFlags,
                bool processFx);
int __cdecl Bullet_GetDamage(
                const BulletFireParams *bp,
                const BulletTraceResults *br,
                const WeaponDef *weapDef,
                gentity_s *attacker);
void __cdecl Bullet_NofifyActor(
                const BulletFireParams *bp,
                gentity_s *attacker,
                float *start,
                const float *end,
                gentity_s *hitEnt,
                const WeaponVariantDef *weapVariantDef);
team_t __cdecl Sentient_EnemyTeam(team_t eTeam);
void __cdecl Bullet_ImpactEffect(
                const BulletFireParams *bp,
                const BulletTraceResults *br,
                const float *normal,
                const WeaponVariantDef *weapVariantDef,
                gentity_s *attacker,
                int impactEffectFlags,
                gentity_s **outTempEnt);
void __cdecl Bullet_ImpactEffect_SetAdditionalFlags(gentity_s *tempEnt, char impactEffectFlags);
void __cdecl Bullet_FirePenetrate(
                BulletFireParams *bp,
                const WeaponVariantDef *weapVariantDef,
                gentity_s *attacker,
                int gameTime);


extern unsigned __int8 scr_playerdamage_boneindex;

