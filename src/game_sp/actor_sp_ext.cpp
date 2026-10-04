#include "actor_sp_ext.h"

#include <game/actor_animapi.h>
#include <game/actor_fields.h>
#include <game/actor_zombie_exposed.h>
#include <game_mp/g_scr_main_mp.h>
#include <game_mp/g_spawn_mp.h>
#include <game_mp/actor_mp.h>
#include <clientscript/cscr_variable.h>
#include <clientscript/cscr_stringlist.h>
#include <universal/q_shared.h>
#include <universal/dvar.h>
#include <cgame_mp/cg_local_mp.h>
#include "g_sp_levelstart.h"

// zombies: 32 actor animation records without changing bgs_t (SP actor limit 0x0048b9f0).
static actorInfo_t g_actorInfoServer[MAX_ACTORS_CAP - MAX_ACTORS_MP];
static actorInfo_t g_actorInfoClient[MAX_LOCAL_CLIENTS][MAX_ACTORS_CAP - MAX_ACTORS_MP];

static actorInfo_t *BG_SP_ExtraActorInfo(bgs_t *context)
{
    if ( context == &level_bgs )
        return g_actorInfoServer;
    if ( cgArray )
    {
        for ( int i = 0; i < MAX_LOCAL_CLIENTS; ++i )
        {
            if ( context == &cgArray[i].bgs )
                return g_actorInfoClient[i];
        }
    }
    Com_Error(ERR_DROP, "BG_SP_ExtraActorInfo: unknown bgs context");
    return NULL;
}

actorInfo_t *BG_SP_GetActorInfo(bgs_t *context, unsigned int actorNum)
{
    iassert(context);
    iassert(actorNum < MAX_ACTORS);
    if ( actorNum < MAX_ACTORS_MP )
        return &context->actorinfo[actorNum];
    return &BG_SP_ExtraActorInfo(context)[actorNum - MAX_ACTORS_MP];
}

void BG_SP_ClearActorInfo(bgs_t *context)
{
    memset(BG_SP_ExtraActorInfo(context), 0, sizeof(g_actorInfoServer));
}

// --- dvars -------------------------------------------------------------------------------------
const dvar_s *zombiemode_path_minz_bias;
const dvar_s *bg_moonGravity;
const dvar_s *ai_useCheapSight;
const dvar_s *ai_coverScore_distance;
const dvar_s *ai_ShowCanshootChecks;
const dvar_s *ai_accuracyDistScale;
const dvar_s *ai_playerLOSRange;

void Actor_SP_RegisterDvars()
{
    Actor_SP_ResetGrenadeLandingCaches(); // Called once by G_InitActors on each level.
    // SP 0x0082bcca: Dvar_RegisterFloat("zombiemode_path_minz_bias", 50 (.rdata 0x009b0fc4), 0,
    // 200 (.rdata 0x00a187a0), 0x1000, "")
    zombiemode_path_minz_bias = _Dvar_RegisterFloat("zombiemode_path_minz_bias", 50.0f, 0.0f, 200.0f, 0x1000u, "");
    // zombies: BG_RegisterDvars (SP 0x0065e6b0), DAT_00bcd0b8; read by Actor_PhysicsMove.
    bg_moonGravity = _Dvar_RegisterFloat("bg_moonGravity", 136.0f, 1.0f, FLT_MAX, 0x2000u, "");
    // zombies: AI_RegisterDvars_B (SP 0x007e1c10), DAT_01c01840.
    ai_useCheapSight = _Dvar_RegisterBool("ai_useCheapSight", false, 0x5000u, "");
    // zombies: AI_RegisterDvars_A (SP 0x007e0c00), DAT_01c018f8.
    ai_coverScore_distance = _Dvar_RegisterFloat("ai_coverScore_distance", 16.0f, 0.0f, 50.0f, 0x2080u, "");
    // zombies: AI_RegisterDvars_B (SP 0x007e2658 -> DAT_01bda378); read by canshoot (SP 0x007cac7f).
    ai_ShowCanshootChecks = _Dvar_RegisterBool("ai_ShowCanshootChecks", false, 1u, "");
    // SP 0x007e15eb: Dvar_RegisterFloat("ai_accuracyDistScale", 1, FLT_MIN, FLT_MAX, 0x3080), read by getweaponaccuracy.
    ai_accuracyDistScale = _Dvar_RegisterFloat("ai_accuracyDistScale", 1.0f, FLT_MIN, FLT_MAX, 0x3080u, "");
    // zombies: SP 0x007e10ad, DAT_01b4c744 (150, 0..500, 0x2080); read by the player look-at update.
    ai_playerLOSRange = _Dvar_RegisterFloat("ai_playerLOSRange", 150.0f, 0.0f, 500.0f, 0x2080u, "");
    Actor_SP_RegisterExposedDvars();
}

// --- species ------------------------------------------------------------------------------------
// zombies: P6a - the SP scr_const slots for species 0, 2 and 3 (init 0x005eba72; slot table .data
// 0x00b74f14). Same allocation and lifetime as scr_const.dog (GScr_AllocString, game system).
unsigned __int16 g_actorSpScrConstHuman;
unsigned __int16 g_actorSpScrConstZombie;
unsigned __int16 g_actorSpScrConstZombieDog;

void Actor_SP_LoadConsts()
{
    g_actorSpScrConstHuman = GScr_AllocString("human");
    g_actorSpScrConstZombie = GScr_AllocString("zombie");
    g_actorSpScrConstZombieDog = GScr_AllocString("zombie_dog");
}

// zombies: human cover consumers in P6c still need this list (SP 0x01c79b64).
// Dog storage belongs to g_scr_data; P3 owns both zombie lists.
static AnimScriptList g_actorSpHumanAnim;

AnimScriptList *Actor_SP_GetAnimScriptStorage(AISpecies species)
{
    iassert(species >= AI_SPECIES_HUMAN && species < MAX_AI_SPECIES);
    switch ( species )
    {
    case AI_SPECIES_DOG: return &g_scr_data.dogAnim;
    case AI_SPECIES_ZOMBIE: return &g_scr_data_sp.zombieAnim;
    case AI_SPECIES_ZOMBIE_DOG: return &g_scr_data_sp.zombieDogAnim;
    default: return &g_actorSpHumanAnim;
    }
}

// --- per-actor SP fields ------------------------------------------------------------------------
static actor_sp_ext_t g_actorSpExt[MAX_ACTORS_CAP];
static unsigned short g_actorProneAnimNodes[MAX_ACTORS_CAP][3]; // SP actor+0xd00/02/04
static scripted_anim_sp_t g_scriptedAnims[MAX_GENTITIES_SV];
static bool g_scriptedAnimInUse[MAX_GENTITIES_SV];

scripted_anim_sp_t *G_SP_GetScriptedAnim(const gentity_s *ent)
{
    int index = ent - g_entities;
    iassert(index >= 0 && index < MAX_GENTITIES);
    return g_scriptedAnimInUse[index] ? &g_scriptedAnims[index] : NULL;
}

scripted_anim_sp_t *G_SP_AllocScriptedAnim(const gentity_s *ent)
{
    int index = ent - g_entities;
    iassert(index >= 0 && index < MAX_GENTITIES);
    if (!g_scriptedAnimInUse[index])
        memset(&g_scriptedAnims[index], 0, sizeof(scripted_anim_sp_t));
    g_scriptedAnimInUse[index] = true;
    return &g_scriptedAnims[index];
}

void G_SP_ClearScriptedAnim(const gentity_s *ent)
{
    int index = ent - g_entities;
    iassert(index >= 0 && index < MAX_GENTITIES);
    g_scriptedAnimInUse[index] = false;
}

unsigned short *Actor_SP_ProneAnimNodes(const actor_s *actor)
{
    int index = G_GetActorIndex((actor_s *)actor);
    iassert(index >= 0 && index < MAX_ACTORS);
    return g_actorProneAnimNodes[index];
}

actor_sp_ext_t *Actor_SP_Ext(const actor_s *actor)
{
    iassert(actor);
    int index = G_GetActorIndex((actor_s *)actor);
    iassert(index >= 0 && index < (int)ARRAY_COUNT(g_actorSpExt));
    return &g_actorSpExt[index];
}

void Actor_SP_InitExt(actor_s *actor)
{
    actor_sp_ext_t *ext = Actor_SP_Ext(actor);
    memset(ext, 0, sizeof(*ext));
    memset(Actor_SP_ProneAnimNodes(actor), 0, sizeof(g_actorProneAnimNodes[0]));
    ext->linkYawOnly = 1; // zombies: Actor_SetDefaults (SP 0x004eff50), actor+0x2248
    ext->allowReact = 1; // G_SpawnActorEntity 0x004f5a70: actor+0xc68 = 1
    ext->lookAtEntNum = ENTITYNUM_NONE; // zombies: actor+0x88 = 0x3ff (SP 0x007bbf8e, called by 0x004f5a70)
}

void Actor_SP_FreeExt(actor_s *actor)
{
    actor_sp_ext_t *ext = Actor_SP_Ext(actor);
    Scr_SetString(&ext->name, 0, SCRIPTINSTANCE_SERVER);
    Scr_SetString(&ext->primaryWeapon, 0, SCRIPTINSTANCE_SERVER);
    Scr_SetString(&ext->secondaryWeapon, 0, SCRIPTINSTANCE_SERVER);
    Scr_SetString(&ext->sidearm, 0, SCRIPTINSTANCE_SERVER);
}

struct actor_sp_ext_field_t
{
    const char *name;
    int ofs;
};

static const actor_sp_ext_field_t g_actorSpExtFields[] =
{
    { "name", offsetof(actor_sp_ext_t, name) },
    { "primaryweapon", offsetof(actor_sp_ext_t, primaryWeapon) },
    { "secondaryweapon", offsetof(actor_sp_ext_t, secondaryWeapon) },
    { "sidearm", offsetof(actor_sp_ext_t, sidearm) },
    { "perfectaim", offsetof(actor_sp_ext_t, perfectAim) },
    { "ignorelocationaldamage", offsetof(actor_sp_ext_t, ignoreLocationalDamage) },
    { "allowreact", offsetof(actor_sp_ext_t, allowReact) },
    { "engagemindist", offsetof(actor_sp_ext_t, engageMinDist) },
    { "engageminfalloffdist", offsetof(actor_sp_ext_t, engageMinFalloffDist) },
    { "engagemaxdist", offsetof(actor_sp_ext_t, engageMaxDist) },
    { "engagemaxfalloffdist", offsetof(actor_sp_ext_t, engageMaxFalloffDist) },
};

static int Actor_SP_ExtFieldOfs(const actor_fields_s *pField)
{
    for ( unsigned int i = 0; i < ARRAY_COUNT(g_actorSpExtFields); ++i )
    {
        if ( !strcmp(g_actorSpExtFields[i].name, pField->name) )
            return g_actorSpExtFields[i].ofs;
    }
    Com_Error(ERR_DROP, "actor field %s has no side-table storage", pField->name);
    return 0;
}

// The generic field code does the type work (strings keep their script refcounts), on the side table.
void __cdecl ActorScr_SP_SetExtField(actor_s *pSelf, const actor_fields_s *pField)
{
    iassert(pSelf);
    iassert(pField && pField->ofs == AF_SP_EXT);
    GScr_SetGenericField((unsigned __int8 *)Actor_SP_Ext(pSelf), pField->type, Actor_SP_ExtFieldOfs(pField), 0);
}

void __cdecl ActorScr_SP_GetExtField(actor_s *pSelf, const actor_fields_s *pField)
{
    iassert(pSelf);
    iassert(pField && pField->ofs == AF_SP_EXT);
    GScr_GetGenericField((unsigned __int8 *)Actor_SP_Ext(pSelf), pField->type, Actor_SP_ExtFieldOfs(pField), 0);
}

// --- AI state rows SP has and KB has not ported ---------------------------------------------------
static const char *Actor_SP_StateName(ai_state_t state)
{
    static const char *names[AIS_COUNT] = {
        "invalid", "exposed", "turret", "grenade_response", "badplace_flee", "coverarrival",
        "death", "pain", "react", "scriptedanim", "customanim", "negotiation" };
    return (unsigned)state < AIS_COUNT ? names[state] : "?";
}

static void Actor_SP_NotPorted(actor_s *self, ai_state_t state, const char *slot)
{
    // AIFuncTable comments (game/actor_function_table.cpp) name the SP function for each slot
    Com_Error(ERR_DROP,
        "AI state not ported: species %i, state %i (%s), %s - see AIFuncTable in actor_function_table.cpp",
        self ? self->species : -1, state, Actor_SP_StateName(state), slot);
}

bool __fastcall Actor_SP_NotPorted_Start(actor_s *self, ai_state_t ePrevState)
{
    Actor_SP_NotPorted(self, self ? self->eState[self->stateLevel] : AIS_INVALID, "pfnStart");
    return false;
}

void __fastcall Actor_SP_NotPorted_Finish(actor_s *self, ai_state_t eNextState)
{
    Actor_SP_NotPorted(self, self ? self->eState[self->stateLevel] : AIS_INVALID, "pfnFinish/pfnSuspend");
}

bool __fastcall Actor_SP_NotPorted_Resume(actor_s *self, ai_state_t ePrevState)
{
    Actor_SP_NotPorted(self, self ? self->eState[self->stateLevel] : AIS_INVALID, "pfnResume");
    return false;
}

actor_think_result_t __fastcall Actor_SP_NotPorted_Think(actor_s *self)
{
    Actor_SP_NotPorted(self, self ? self->eState[self->stateLevel] : AIS_INVALID, "pfnThink");
    return ACTOR_THINK_DONE;
}
