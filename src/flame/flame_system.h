#pragma once
#include <gfx_d3d/r_material.h>
#include <universal/dvar.h>
#include "flame_physics.h"

struct cg_s;
struct centity_s;
struct flameStream_s;

struct flameSize_t // sizeof=0x8
{                                                                             // XREF: flameGeneric_s/r
        float current;
        float rate;
};

struct flameAge_t // sizeof=0xC
{                                                                             // XREF: flameGeneric_s/r
        int lastUpdateTime;
        int startTime;
        int endTime;
};

struct flameList_t // sizeof=0x8
{                                                                             // XREF: flameGeneric_s/r
                                                                                // flameGeneric_s/r ...
        struct flameGeneric_s *prev;                             // XREF: Flame_Item_Init(flameGeneric_s *,int)+F/w
                                                                                // Flame_Item_Init(flameGeneric_s *,int)+1E/w ...
        struct flameGeneric_s *next;                             // XREF: Flame_Item_Init(flameGeneric_s *,int)+12/w
                                                                                // Flame_Item_Init(flameGeneric_s *,int)+21/w ...
};

struct flameGeneric_s // sizeof=0x60
{                                                                             // XREF: .data:flameChunk_s * sv_flameChunks/r
                                                                                // .data:flameChunk_s * flameChunks/r ...
    flamePhysics_t phys;
    flameSize_t size;
    flameAge_t age;
    flameList_t listGlobal;
    flameList_t listLocal;
    flameStream_s *stream;
    unsigned __int32 type : 3;
    unsigned __int32 delete_chunk : 1;
    unsigned __int32 id : 28;
};

struct flameRender_s // sizeof=0x88
{                                                                             // XREF: .data:flameRender_s * flameVarList/r
        char name[128];
        flameGeneric_s *fireList;                     // XREF: Flame_Generate_Verts(int)+9C/r
        flameGeneric_s *dripsList;                    // XREF: Flame_Generate_Verts(int)+B8/r
};

struct flameRenderList_s // sizeof=0xC04
{                                                                             // XREF: .data:flameStreamRenderList/r
        float fuelVerts[128][3];
        float flameVerts[128][3];
        flameRender_s *flameRend;
};

struct flameChunk_s // sizeof=0x70
{                                                                             // XREF: flameChunk_t/r
        flameGeneric_s gen;
        int spawnFireInterval;
        int lastSpawnFire;
        int spawnDripsInterval;
        int spawnSmokeInterval;
};

struct __declspec(align(4)) flameSource_t // sizeof=0x58
{                                                                             // XREF: .data:flameSource_t * sv_flameSources/r
                                                                                // .data:flameSource_t * flameSources/r
        float origin[3];
        float angle[3];
        float entityOrigin[3];
        flameStream_s *currentStream;
        int lastUsedTime;
        int isKillcamFlame;
        int firingThroughGeo;
        int entityNum;
        int firstDobjHandle;
        int thirdDobjHandle;
        int lastSoundTime;
        float flameSoundPoint[3];
        bool bIsFiring;
        bool inUse;
        bool is_server_alloc;                             // XREF: Flame_Init_Sources(void)+77/w
        bool wasFiring;
        unsigned __int8 firstBoneIndex;
        unsigned __int8 thirdBoneIndex;
        // padding byte
        // padding byte
};

struct flameTable // sizeof=0x1DC
{                                                                             // XREF: .data:flameTable bg_DVarFlameTable/r
        float flameVar_streamChunkGravityStart;
        float flameVar_streamChunkGravityEnd;
        float flameVar_streamChunkMaxSize;
        float flameVar_streamChunkStartSize;
        float flameVar_streamChunkEndSize;
        float flameVar_streamChunkStartSizeRand;
        float flameVar_streamChunkEndSizeRand;
        float flameVar_streamChunkDistScalar;
        float flameVar_streamChunkDistSwayScale;
        float flameVar_streamChunkDistSwayVelMax;
        float flameVar_streamChunkSpeed;
        float flameVar_streamChunkDecel;
        float flameVar_streamChunkVelocityAddScale;
        float flameVar_streamChunkDuration;
        float flameVar_streamChunkDurationScaleMaxVel;
        float flameVar_streamChunkDurationVelScalar;
        float flameVar_streamChunkSizeSpeedScale;
        float flameVar_streamChunkSizeAgeScale;
        float flameVar_streamChunkSpawnFireIntervalStart;
        float flameVar_streamChunkSpawnFireIntervalEnd;
        float flameVar_streamChunkSpawnFireMinLifeFrac;
        float flameVar_streamChunkSpawnFireMaxLifeFrac;
        float flameVar_streamChunkFireMinLifeFrac;
        float flameVar_streamChunkFireMinLifeFracStart;
        float flameVar_streamChunkFireMinLifeFracEnd;
        float flameVar_streamChunkDripsMinLifeFrac;
        float flameVar_streamChunkDripsMinLifeFracStart;
        float flameVar_streamChunkDripsMinLifeFracEnd;
        float flameVar_streamChunkRotationRange;
        float flameVar_streamSizeRandSinWave;
        float flameVar_streamSizeRandCosWave;
        float flameVar_streamDripsChunkInterval;
        float flameVar_streamDripsChunkMinFrac;
        float flameVar_streamDripsChunkRandFrac;
        float flameVar_streamSmokeChunkInterval;
        float flameVar_streamSmokeChunkMinFrac;
        float flameVar_streamSmokeChunkRandFrac;
        float flameVar_streamChunkCullDistSizeFrac;
        float flameVar_streamChunkCullMinLife;
        float flameVar_streamChunkCullMaxLife;
        float flameVar_streamFuelSizeStart;
        float flameVar_streamFuelSizeEnd;
        float flameVar_streamFuelLength;
        float flameVar_streamFuelNumSegments;
        float flameVar_streamFuelAnimLoopTime;
        float flameVar_streamFlameSizeStart;
        float flameVar_streamFlameSizeEnd;
        float flameVar_streamFlameLength;
        float flameVar_streamFlameNumSegments;
        float flameVar_streamFlameAnimLoopTime;
        float flameVar_streamPrimaryLightRadius;
        float flameVar_streamPrimaryLightRadiusFlutter;
        float flameVar_streamPrimaryLightR;
        float flameVar_streamPrimaryLightG;
        float flameVar_streamPrimaryLightB;
        float flameVar_streamPrimaryLightFlutterR;
        float flameVar_streamPrimaryLightFlutterG;
        float flameVar_streamPrimaryLightFlutterB;
        float flameVar_fireLife;
        float flameVar_fireLifeRand;
        float flameVar_fireSpeedScale;
        float flameVar_fireSpeedScaleRand;
        float flameVar_fireVelocityAddZ;
        float flameVar_fireVelocityAddZRand;
        float flameVar_fireVelocityAddSideways;
        float flameVar_fireGravity;
        float flameVar_fireGravityEnd;
        float flameVar_fireMaxRotVel;
        float flameVar_fireFriction;
        float flameVar_fireEndSizeAdd;
        float flameVar_fireStartSizeScale;
        float flameVar_fireEndSizeScale;
        float flameVar_dripsLife;
        float flameVar_dripsLifeRand;
        float flameVar_dripsSpeedScale;
        float flameVar_dripsSpeedScaleRand;
        float flameVar_dripsVelocityAddZ;
        float flameVar_dripsVelocityAddZRand;
        float flameVar_dripsVelocityAddSideways;
        float flameVar_dripsGravity;
        float flameVar_dripsGravityEnd;
        float flameVar_dripsMaxRotVel;
        float flameVar_dripsFriction;
        float flameVar_dripsEndSizeAdd;
        float flameVar_dripsStartSizeScale;
        float flameVar_dripsEndSizeScale;
        float flameVar_smokeLife;
        float flameVar_smokeLifeRand;
        float flameVar_smokeSpeedScale;
        float flameVar_smokeVelocityAddZ;
        float flameVar_smokeGravity;
        float flameVar_smokeGravityEnd;
        float flameVar_smokeMaxRotation;
        float flameVar_smokeMaxRotVel;
        float flameVar_smokeFriction;
        float flameVar_smokeEndSizeAdd;
        float flameVar_smokeStartSizeAdd;
        float flameVar_smokeOriginSizeOfsZScale;
        float flameVar_smokeOriginOfsZ;
        float flameVar_smokeFadein;
        float flameVar_smokeFadeout;
        float flameVar_smokeMaxAlpha;
        float flameVar_smokeBrightness;
        float flameVar_smokeOriginOffset;
        float flameVar_collisionSpeedScale;
        float flameVar_collisionVolumeScale;
        const char *name;
        Material *fire;
        Material *smoke;
        Material *heat;
        Material *drips;
        Material *streamFuel;
        Material *streamFuel2;
        Material *streamFlame;
        Material *streamFlame2;
        const char *flameOffLoopSound;
        const char *flameIgniteSound;
        const char *flameOnLoopSound;
        const char *flameCooldownSound;
};

struct flameWeaponConfig_t // sizeof=0x44
{                                                                             // XREF: ?CG_Flame_Update_Source@@YAXH@Z/r
                                                                                // ?Weapon_Flamethrower_Update@@YAXPAUgentity_s@@PAUweaponParms@@@Z/r ...
        bool bIsFiring;                                         // XREF: Weapon_Flamethrower_Update(gentity_s *,weaponParms *)+1B7/w
                                                                                // Weapon_Flamethrower_Fire(gentity_s *,weaponParms *)+1E4/w ...
        bool bFireWhileIdle;                                // XREF: Weapon_Flamethrower_Update(gentity_s *,weaponParms *)+1B0/w
                                                                                // Weapon_Flamethrower_Fire(gentity_s *,weaponParms *):loc_69734D/w ...
        // padding byte
        // padding byte
        float origin[3];                                        // XREF: Weapon_Flamethrower_Update(gentity_s *,weaponParms *)+15E/w
                                                                                // Weapon_Flamethrower_Update(gentity_s *,weaponParms *)+16E/w ...
        float angle[3];                                         // XREF: Weapon_Flamethrower_Update(gentity_s *,weaponParms *)+188/w
                                                                                // Weapon_Flamethrower_Update(gentity_s *,weaponParms *)+192/w ...
        float strength;                                         // XREF: Weapon_Flamethrower_Update(gentity_s *,weaponParms *)+1CF/w
                                                                                // Weapon_Flamethrower_Fire(gentity_s *,weaponParms *)+1FC/w ...
        float thickness;                                        // XREF: Weapon_Flamethrower_Update(gentity_s *,weaponParms *)+1DC/w
                                                                                // Weapon_Flamethrower_Fire(gentity_s *,weaponParms *)+209/w ...
        float burnRate;                                         // XREF: Weapon_Flamethrower_Update(gentity_s *,weaponParms *)+1C2/w
                                                                                // Weapon_Flamethrower_Fire(gentity_s *,weaponParms *)+1EF/w ...
        float entityOrigin[3];                            // XREF: Weapon_Flamethrower_Update(gentity_s *,weaponParms *)+1FA/w
                                                                                // Weapon_Flamethrower_Update(gentity_s *,weaponParms *)+20A/w ...
        flameTable *fTable;                                 // XREF: Weapon_Flamethrower_Update(gentity_s *,weaponParms *)+1AD/w
                                                                                // Weapon_Flamethrower_Fire(gentity_s *,weaponParms *)+1A0/w ...
        int damage;                                                 // XREF: Weapon_Flamethrower_Update(gentity_s *,weaponParms *)+22B/w
                                                                                // Weapon_Flamethrower_Fire(gentity_s *,weaponParms *)+258/w ...
        float damageDuration;                             // XREF: Weapon_Flamethrower_Update(gentity_s *,weaponParms *)+23C/w
                                                                                // Weapon_Flamethrower_Fire(gentity_s *,weaponParms *)+269/w
        float damageInterval;                             // XREF: Weapon_Flamethrower_Update(gentity_s *,weaponParms *)+24F/w
                                                                                // Weapon_Flamethrower_Fire(gentity_s *,weaponParms *)+27C/w
};

struct flameVarDef_t // sizeof=0x18
{
    const char *dvarName;
    const char *name;
    float defaultVal;
    float minVal;
    float maxVal;
    const char *description;
};

void __cdecl Flame_Init_FlameVars();
void __cdecl Flame_Reset_FlameVars();
void __cdecl Flame_Init_Classes();
void __cdecl Flame_Init_Sources();
void __cdecl Flame_Init_DVars();
char __cdecl Flame_TableInDevGui(const char *tableName);
void __cdecl Flame_InitDevGui();
void __cdecl Flame_Init();
void __cdecl Flame_Reset();
void __cdecl Flame_CMD_PrintDVarsToConsol();
void __cdecl Flame_CMD_Test_Toggle();
void __cdecl Flame_CMD_Use_Dvars_Toggle();
void __cdecl Flame_DVarsToFlameVars(flameTable *fTable);
flameTable *__cdecl Flame_FindFlameTable(const char *tableName);
float __cdecl Flame_SwayRand(float x, float y, int time);
flameSource_t *__cdecl SV_Flame_Source_Get(int entityNum);
flameSource_t *__cdecl Flame_Source_Get(int entityNum);
flameRender_s *__cdecl Flame_Get_FlameRender(const char *name);
void __cdecl Flame_Item_Init(flameGeneric_s *item, unsigned int itemSize);
void __cdecl Flame_List_Init(flameGeneric_s *list, int itemSize, int listSize);
void __cdecl Flame_List_Move_Global(flameGeneric_s *item, flameGeneric_s **fromList, flameGeneric_s **toList);
void __cdecl Flame_List_Add_Local(flameGeneric_s *item, flameGeneric_s **localList);
void __cdecl Flame_List_Sub_Local(flameGeneric_s *item, flameGeneric_s **localList);
void __cdecl CG_Flame_Age_All_Objects(int time);
void __cdecl SV_Flame_Age_All_Objects(int time);
flameSource_t *__cdecl Flame_Source_Alloc(int entityNum);
flameSource_t *__cdecl SV_Flame_Source_Alloc(int entityNum);
void __cdecl Flame_Render_Sprite(
                cg_s *clientGlobals,
                Material *material,
                flameGeneric_s *flameGeneric,
                float alpha,
                float frame,
                unsigned __int8 col);
void    Flame_Render_Sprites(
                cg_s *clientGlobals,
                Material *material,
                flameGeneric_s **flameGenericList,
                int numItems);
int __cdecl Flame_GetLocalClientFlameSource(int localClientNum, int EntNum);
bool __cdecl Flame_GetLocalClientSourceRange(const char *__formal = 0);
void __cdecl Flame_Source_Update(flameSource_t *source, flameWeaponConfig_t *weaponConfig);
void __cdecl SV_Flame_Update_Source_Internal(
                flameSource_t *source,
                int entityNum,
                flameWeaponConfig_t *weaponConfig,
                int time);
void __cdecl CG_Flame_Update_Source_Internal(
                flameSource_t *source,
                int entityNum,
                flameWeaponConfig_t *weaponConfig,
                int time,
                flameRender_s *flameRend);
void __cdecl SV_Flame_Update_Source(int entityNum, flameWeaponConfig_t *weaponConfig, int time);
void __cdecl CG_Flame_Update_Source(int localClientNum);
void __cdecl Flame_Generate_Verts(int localClientNum);
void __cdecl CG_Flame_Update_ViewModel(int localClientNum, centity_s *cent);
double __cdecl Flame_Random(bool is_server);
double __cdecl Flame_CRandom(bool is_server);
double __cdecl Flame_CalcTimeScale(int startTime, int endTime);
double __cdecl Flame_CalcInvStartSpeed(float invInitialSpeed, float speedScale);

extern phys_static_array<flameGeneric_s *, 1000> sv_flames;
extern phys_static_array<flameGeneric_s *, 1000> cl_flames;

extern const dvar_t *flame_test;
extern const dvar_t *flame_use_dvars;
extern const dvar_t *flame_render;
extern const dvar_t *flame_team_damage;
extern const dvar_t *flame_debug_render;
extern const dvar_t *flame_config_valid;
extern const dvar_t *default_flameVars_initialHitDamage;
extern const dvar_t *default_flameVars_timedDamageDuration;
extern const dvar_t *default_flameVars_timedDamageInterval;
extern const dvar_t *flameVar_editingFlameTable;
extern const dvar_t *flameVar_lastFlameTable;
extern const dvar_t *flame_kick_offset;
extern const dvar_t *flame_kick_speed;
extern const dvar_t *flame_kick_recover_speed;

extern unsigned int flame_spawn_id;
extern unsigned int flame_freeze_id;

extern cspField_t flameTableFields[119];