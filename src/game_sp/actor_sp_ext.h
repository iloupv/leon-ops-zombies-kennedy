#pragma once
// zombies: actor lane side storage for SP actor state KB's MP structs have no room for.
// KB's structs keep their MP size and layout (decompiled code hard-codes strides), so SP-only
// actor state lives here.

#include <game/actor.h>

struct AnimScriptList;
struct bgs_t;
struct actorInfo_t;

// Keep bgs_t's embedded 16 entries and use separate server/client storage for slots 16..31.
actorInfo_t *BG_SP_GetActorInfo(bgs_t *context, unsigned int actorNum);
void BG_SP_ClearActorInfo(bgs_t *context);

// --- species ------------------------------------------------------------------------------------
// Script-string handles for the species names scr_const_t lacks ("dog" is scr_const.dog).
// Allocated with the game-system constants each level (G_InitActors).
extern unsigned __int16 g_actorSpScrConstHuman;
extern unsigned __int16 g_actorSpScrConstZombie;
extern unsigned __int16 g_actorSpScrConstZombieDog;
void Actor_SP_LoadConsts();

// Canonical species lists: human side storage (used by P6c), scr_data_t's dogAnim,
// and P3's g_scr_data_sp zombie lists. Human/dog lists stay empty in zombiemode.
AnimScriptList *Actor_SP_GetAnimScriptStorage(AISpecies species);

// --- dvars -------------------------------------------------------------------------------------
// zombies: V4 SP registers zombiemode_path_minz_bias next to zombiemode (0x0082bcca): float 50, 0..200,
// flags 0x1000, no description. Read by Path_NearestNodeNotCrossPlanes (pathnode.cpp) in zombiemode.
struct dvar_s;
extern const dvar_s *zombiemode_path_minz_bias;
extern const dvar_s *bg_moonGravity;
extern const dvar_s *ai_useCheapSight;
extern const dvar_s *ai_playerLOSRange;
extern const dvar_s *ai_coverScore_distance;
extern const dvar_s *ai_ShowCanshootChecks;
extern const dvar_s *ai_accuracyDistScale;
void Actor_SP_RegisterDvars();
// zombies: SP 0x007bdb00 (g_scr_sp_ai_cmds.cpp)
float Actor_GetWeaponAccuracyFromGraph_SP(const struct WeaponVariantDef *weapVariantDef, int weaponIndex, int type,
    float distance);

// --- per-actor SP fields ------------------------------------------------------------------------
// One slot per level.actors entry: set to SP's spawn defaults on Actor_Alloc (Actor_SetDefaults),
// strings released on Actor_Free (Scr_FreeActorFields).
struct actor_sp_ext_t
{
    char zombieName[32];                // SP actorState name, actor+0x50 (setzombiename SP 0x008060e0)
    int hudWarningType;                 // SP actorState hudwarningType, actor+0x84 (sethudwarningtype SP 0x00805ff0)
    int allowPitchAngle;                // SP actor+0x224c; Actor_SP_InitExt zeroes this at spawn
    int linkYawOnly;                    // SP actor+0x2248, G_SetFixedLink angle mode; 1 at spawn
    unsigned __int16 name;              // SP actor+0xc94 "name"
    unsigned __int16 primaryWeapon;     // SP actor+0xc98 "primaryweapon"
    unsigned __int16 secondaryWeapon;   // SP actor+0xc9a "secondaryweapon"
    unsigned __int16 sidearm;           // SP actor+0xc9c "sidearm"
    int perfectAim;                     // SP actor+0xc78 "perfectaim", 0 at spawn
    int ignoreLocationalDamage;         // SP actor+0xc7c "ignorelocationaldamage", 0 at spawn
    unsigned __int8 allowReact;         // SP actor+0xc68 "allowreact", 1 at spawn (G_SpawnActorEntity 0x004f5a70)
    float engageMinDist;                // SP actor+0x1ad8..0x1ae4, read-only fields, set by
    float engageMinFalloffDist;         //   setengagementmin/maxdist (0x007ce220 / 0x007ce2c0)
    float engageMaxDist;
    float engageMaxFalloffDist;
    // SP actor+0x20c8: four 0x18-byte suppression records, absent from MP actor_s.
    // Only the time/normal/dist fields read by the exposed path helpers are stored here.
    struct { int time; float normal[2]; float dist; } suppression[4];
    gentity_s *turret;                  // SP actor+0x21a8; no MP actor turret member
    int lookAtEntNum;                   // SP actor+0x88, lookatentity; ENTITYNUM_NONE at spawn (SP 0x007bbf50)
    unsigned short sightTraceHitNum[MAX_SENTIENTS_CAP]; // SP +0x31f8: per-sentient collision-brush hints
};

actor_sp_ext_t *Actor_SP_Ext(const actor_s *actor);
void Actor_SP_InitExt(actor_s *actor);
void Actor_SP_FreeExt(actor_s *actor);

// SP entity+0x2a0 points at this 100-byte scripted animation record. Separate storage
// preserves gentity_s and actor_s layouts; the unused deathplant tail stays opaque.
struct scripted_anim_sp_t
{
    float axis[4][3];
    float originError[3];
    float anglesError[3];
    unsigned short anim;
    unsigned short root;
    unsigned char started;
    unsigned char deathplant;
    unsigned short padding;
    int startTime;
    unsigned char deathplantData[16];
};
static_assert(sizeof(scripted_anim_sp_t) == 100, "SP scripted animation record");
scripted_anim_sp_t *G_SP_GetScriptedAnim(const gentity_s *ent);
scripted_anim_sp_t *G_SP_AllocScriptedAnim(const gentity_s *ent);
void G_SP_ClearScriptedAnim(const gentity_s *ent);
unsigned short *Actor_SP_ProneAnimNodes(const actor_s *actor);

// actor_fields_s::ofs of a row whose storage is actor_sp_ext_t (the row's name selects the member)
#define AF_SP_EXT (-1)
struct actor_fields_s;
void __cdecl ActorScr_SP_SetExtField(actor_s *pSelf, const actor_fields_s *pField);
void __cdecl ActorScr_SP_GetExtField(actor_s *pSelf, const actor_fields_s *pField);

// --- AI state rows SP has and KB has not ported ---------------------------------------------------
// Entered only through AIFuncTable; each errors loudly (ERR_DROP) naming the species and state.
bool __fastcall Actor_SP_NotPorted_Start(actor_s *self, ai_state_t ePrevState);
void __fastcall Actor_SP_NotPorted_Finish(actor_s *self, ai_state_t eNextState);
bool __fastcall Actor_SP_NotPorted_Resume(actor_s *self, ai_state_t ePrevState);
actor_think_result_t __fastcall Actor_SP_NotPorted_Think(actor_s *self);
