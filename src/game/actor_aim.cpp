#include "actor_aim.h"
#include "actor_senses.h"
#include "g_weapon.h"
#include <clientscript/cscr_stringlist.h>
#include <game_mp/g_main_mp.h>
#include <game_mp/actor_mp.h>

void __cdecl Actor_FillWeaponParms(actor_s *self, weaponParms *wp)
{
    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_aim.cpp", 437, 0, "%s", "self") )
        __debugbreak();
    if ( !wp && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_aim.cpp", 438, 0, "%s", "wp") )
        __debugbreak();
    if ( ((LODWORD(self->vLookForward[0]) & 0x7F800000) == 0x7F800000
         || (LODWORD(self->vLookForward[1]) & 0x7F800000) == 0x7F800000
         || (LODWORD(self->vLookForward[2]) & 0x7F800000) == 0x7F800000)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_aim.cpp",
                    440,
                    0,
                    "%s",
                    "!IS_NAN((self->vLookForward)[0]) && !IS_NAN((self->vLookForward)[1]) && !IS_NAN((self->vLookForward)[2])") )
    {
        __debugbreak();
    }
    if ( ((LODWORD(self->vLookRight[0]) & 0x7F800000) == 0x7F800000
         || (LODWORD(self->vLookRight[1]) & 0x7F800000) == 0x7F800000
         || (LODWORD(self->vLookRight[2]) & 0x7F800000) == 0x7F800000)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_aim.cpp",
                    441,
                    0,
                    "%s",
                    "!IS_NAN((self->vLookRight)[0]) && !IS_NAN((self->vLookRight)[1]) && !IS_NAN((self->vLookRight)[2])") )
    {
        __debugbreak();
    }
    if ( ((LODWORD(self->vLookUp[0]) & 0x7F800000) == 0x7F800000
         || (LODWORD(self->vLookUp[1]) & 0x7F800000) == 0x7F800000
         || (LODWORD(self->vLookUp[2]) & 0x7F800000) == 0x7F800000)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_aim.cpp",
                    442,
                    0,
                    "%s",
                    "!IS_NAN((self->vLookUp)[0]) && !IS_NAN((self->vLookUp)[1]) && !IS_NAN((self->vLookUp)[2])") )
    {
        __debugbreak();
    }
    wp->forward[0] = self->vLookForward[0];
    wp->forward[1] = self->vLookForward[1];
    wp->forward[2] = self->vLookForward[2];
    wp->right[0] = self->vLookRight[0];
    wp->right[1] = self->vLookRight[1];
    wp->right[2] = self->vLookRight[2];
    wp->up[0] = self->vLookUp[0];
    wp->up[1] = self->vLookUp[1];
    wp->up[2] = self->vLookUp[2];
    if ( Actor_GetMuzzleInfo(self, wp->muzzleTrace, wp->gunForward) )
    {
        wp->forward[0] = wp->gunForward[0];
        wp->forward[1] = wp->gunForward[1];
        wp->forward[2] = wp->gunForward[2];
    }
    else
    {
        Actor_GetEyePosition(self, wp->muzzleTrace);
        wp->gunForward[0] = wp->forward[0];
        wp->gunForward[1] = wp->forward[1];
        wp->gunForward[2] = wp->forward[2];
    }
    if ( ((LODWORD(wp->muzzleTrace[0]) & 0x7F800000) == 0x7F800000
         || (LODWORD(wp->muzzleTrace[1]) & 0x7F800000) == 0x7F800000
         || (LODWORD(wp->muzzleTrace[2]) & 0x7F800000) == 0x7F800000)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_aim.cpp",
                    458,
                    0,
                    "%s",
                    "!IS_NAN((wp->muzzleTrace)[0]) && !IS_NAN((wp->muzzleTrace)[1]) && !IS_NAN((wp->muzzleTrace)[2])") )
    {
        __debugbreak();
    }
    if ( ((LODWORD(wp->gunForward[0]) & 0x7F800000) == 0x7F800000
         || (LODWORD(wp->gunForward[1]) & 0x7F800000) == 0x7F800000
         || (LODWORD(wp->gunForward[2]) & 0x7F800000) == 0x7F800000)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_aim.cpp",
                    459,
                    0,
                    "%s",
                    "!IS_NAN((wp->gunForward)[0]) && !IS_NAN((wp->gunForward)[1]) && !IS_NAN((wp->gunForward)[2])") )
    {
        __debugbreak();
    }
}

void __cdecl Actor_InitWeaponParms(actor_s *self, unsigned int weapon, weaponParms *wp)
{
    Actor_FillWeaponParms(self, wp);
    Weapon_SetWeaponParamsWeapon(wp, weapon);
}

gentity_s *__cdecl Actor_Melee(actor_s *self, const float *direction)
{
    char *v2; // eax
    float v4; // [esp+10h] [ebp-8Ch]
    float v5; // [esp+2Ch] [ebp-70h]
    float enemyEye[3]; // [esp+38h] [ebp-64h] BYREF
    int weapon; // [esp+44h] [ebp-58h]
    sentient_s *enemy; // [esp+48h] [ebp-54h]
    weaponParms wp; // [esp+4Ch] [ebp-50h] BYREF
    gentity_s *ent; // [esp+98h] [ebp-4h]

    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_aim.cpp", 955, 0, "%s", "self") )
        __debugbreak();
    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_aim.cpp", 961, 0, "%s", "self") )
        __debugbreak();
    ent = self->ent;
    v2 = SL_ConvertToString(self->weaponName, SCRIPTINSTANCE_SERVER);
    weapon = G_GetWeaponIndexForName(v2);
    if ( self->lastShotTime == level.time )
    {
        Com_PrintError(
            18,
            "ERROR: Attempt for same actor (entnum %d) to shoot/melee more than once in a frame.\n",
            ent->s.number);
        return 0;
    }
    else
    {
        self->lastShotTime = level.time;
        enemy = Actor_GetTargetSentient(self);
        Actor_InitWeaponParms(self, weapon, &wp);
        Actor_GetEyePosition(self, wp.muzzleTrace);
        if ( enemy )
        {
            Sentient_GetThirdPersonEyePosition(enemy, enemyEye);
            wp.forward[0] = enemyEye[0] - wp.muzzleTrace[0];
            wp.forward[1] = enemyEye[1] - wp.muzzleTrace[1];
            wp.forward[2] = enemyEye[2] - wp.muzzleTrace[2];
            if ( direction )
            {
                v4 = Vec2Length(direction);
                v5 = Vec2Length(wp.forward) / v4;
                wp.forward[0] = v5 * *direction;
                wp.forward[1] = v5 * direction[1];
            }
            Vec3Normalize(wp.forward);
        }
        else if ( direction )
        {
            wp.forward[0] = *direction;
            wp.forward[1] = direction[1];
            wp.forward[2] = direction[2];
            Vec3Normalize(wp.forward);
        }
        ent->s.weapon = weapon;
        if ( ent->s.weapon != weapon
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_aim.cpp",
                        1014,
                        0,
                        "%s",
                        "ent->s.weapon == weapon") )
        {
            __debugbreak();
        }
        // zombies: V5 only dogs (species 1 or 3) use the ai_melee* box; every other species swings a
        // single ray, reach 64, width 0, height 0 (SP Actor_Melee 0x005846d0: Actor_IsDogSpecies at
        // 0x005848a7, else 64 from .rdata 0x00a50ce8 and fldz x2 at 0x005848e2). MP actors are dogs.
        if ( !Actor_IsDogSpecies(self->species) )
            return Weapon_Melee(self->ent, &wp, 64.0f, 0.0f, 0.0f, level.time);
        return Weapon_Melee(
                         self->ent,
                         &wp,
                         ai_meleeRange->current.value,
                         ai_meleeWidth->current.value,
                         ai_meleeHeight->current.value,
                         level.time);
    }
}

