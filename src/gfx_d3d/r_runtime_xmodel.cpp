// Runtime models: XModels built at level load from rtmodels/<name>.rlm (the bo1_rtmodels dvar lists the names)
// instead of from a fastfile. A mod uses this for models it converts on the player's PC from another game the
// player owns, so the mod never ships that game's assets.
//
// The model, its materials and its images are linked into the asset DB as the level zone's own assets
// (DB_LinkXAssetEntry with g_zoneIndex set to the level zone): GSC precache / setModel find them by name, the
// renderer's per-pool tables (material sort, streaming) see real pool entries, the level's
// R_FinishLoadingModels -> XModelOptimize makes the vertex / index buffers, and the zone unload releases the
// GPU side with the level. The CPU arrays are this file's, freed when the next level builds its models.
//
// Version 1 is rigid: one bone (tag_origin), every surface one vertex list on it. Materials are clones of a
// template material from an always-loaded zombies model with the colour map replaced (normal map flat,
// specular black); a texture marked alpha masked clones an alpha-tested lit material instead.
//
// .rlm (little-endian; written by the converter): 'RLM1', u32 version 1, u32 texture count, u32 surface count;
// texture: char name[32], u16 width, u16 height, u8 mip count, u8 alpha masked, u16 0, every mip's BGRA8
// pixels, largest first; surface: u32 texture, u32 vertex count, u32 triangle count, vertex count x {f32 pos[3]
// (inches, Z up), f32 normal[3], f32 uv[2]}, triangle count x 3 u16 indices, zero pad to 4.
//
// Version 2 adds a skeleton and skinned vertices, and optionally, after the surfaces, baked animations: 'ANM2',
// u32 count, per animation char name[32], u32 frame count, f32 frames per second, u32 bone count (the model's),
// f32 move speed (inches per second at its own rate, 0: in place), frame count x bone count x {f32 local rotation
// quaternion[4], f32 translation offset from the rest pose[3]}. In place of an XAnim tree, every DObj of the
// model plays the locomotion cycle that fits how its entity moves (R_RuntimeXModel_PoseDObj): "idle", "walk",
// "run" or "back", at the rate matching its speed, crossfaded.

#include "r_runtime_xmodel.h"
#include "r_material.h"
#include "r_image.h"
#include "r_init.h"
#include "r_dvars.h"
#include "r_utils.h"
#include "r_xsurface.h"
#include "r_xsurface_load_obj.h"
#include "rb_resource.h"
#include <xanim/xmodel.h>
#include <xanim/xanim.h>
#include <xanim/dobj.h>
#include <database/database.h>
#include <database/db_registry.h>
#include <clientscript/cscr_main.h>
#include <clientscript/cscr_stringlist.h>
#include <qcommon/common.h>
#include <qcommon/threads.h>
#include <universal/com_files.h>
#include <universal/com_memory.h>
#include <universal/dvar.h>
#include <universal/q_shared.h>
#include <universal/com_math.h>
#include <universal/com_math_anglevectors.h>
#include <cgame_mp/cg_pose_mp.h>
#include <win32/win_shared.h>
#include <malloc.h>
#include <string.h>
#include <math.h>
#include <intrin.h>
#include <vector>
#include <string>
#include <stdexcept>

extern XZoneName g_zoneNames[33];
extern unsigned int g_zoneIndex;

static const dvar_t *bo1_rtmodels;
static const dvar_t *bo1_rtmodels_loaded;

// the template's material: in common_zombie, so on every zombies level
static const char *const s_templateModels[] = { "t5_weapon_m14_world", "defaultactor" };

static std::vector<void *> s_allocs; // this level's runtime model memory

struct RtAnim
{
    const XModel *model; // the DB pool copy DObjs point at
    char name[32];
    int frameCount;
    float fps;
    int boneCount;
    float moveSpeed;     // inches per second at the cycle's own rate
    const float *frames; // frameCount x boneCount x {quat[4], trans offset[3]}
    const unsigned char *upperBody; // per bone: 1 from j_spinelower up (the aim poses take these), 0 below
    const unsigned char *headSide;  // per bone: 1 from j_neck up (the head: the knife slash leaves it to the aim), 0 else
    int spineBone;       // j_spinelower (the aim correction turns it), -1: none
    int neckBone;        // j_neck (the head's look turns it), -1: none
    int eyelidBone;      // RE4's part 0x1C, the eyelid (j_re4_028: the blink turns it; its motions hold it), -1: none
    int neckChain[8];    // its parents below the root, root side first (their local rotations: its parent's frame)
    int neckChainCount;
    int recoilPeak;      // a shot's motion: the frame its kick peaks (the upper body turned most from its first); 0
    int reloadRaise;     // a reload's motion: the frame its raise back to RE4's aim starts (its last half's farthest
                         // from its end pose); 0
    const float *runBob; // the run: its spine's held-back bob per frame (RtSoftenRunBob, 3 floats a frame); 0
};
// the local player's head look (R_RuntimeXModel_SetLookYaw): degrees about up, for the next PoseDObj only
static thread_local float s_lookYaw;
static std::vector<RtAnim> s_anims; // this level's
// this level's laser sight beam segment and dot (R_RuntimeXModel_Laser), red and blue (the wonder weapons')
static const XModel *s_laserModels[2][2];
// the beam's shorter pieces: 8, 4, 2 and 1 units (R_RuntimeXModel_LaserPiece), red and blue
static const XModel *s_laserPieces[2][4];
static const XModel *s_haloModel;      // the kick's halo on a staggered zombie (R_RuntimeXModel_Halo)
// RE4's HUD's images as 2D materials (R_RuntimeXModel_HudMaterial: cgame_mp/cg_re4hud_mp.cpp), by their name in the file
struct RtHudMaterial
{
    char name[32];
    Material *material;
};
static std::vector<RtHudMaterial> s_hudMaterials;

// Per entity locomotion state (by DObj entnum): the measured ground speed and the playing / fading-out cycles.
struct RtAnimState
{
    const DObj *obj; // the DObj the state belongs to (a new one restarts it)
    int lastTime;
    float lastOrigin[3];
    float vel[2];      // smoothed ground velocity, inches per second
    float speed;       // its length
    float moveRel;     // its direction relative to the facing, degrees (0: forward, +-180: backward)
    float bodyYaw;     // the body's turn toward the motion (a strafing player), degrees
    int cur, prev;     // s_anims indices (-1: none)
    float curPhase, prevPhase; // frames
    float prevRate;
    float blend;       // 0: prev only .. 1: cur only
    float slowTime;    // seconds the motion has been (nearly) still
    float lastYaw;     // the facing at the last update, degrees
    float yawRate;     // its smoothed turn, degrees a second (+: to his left)
    float turnStill;   // seconds a turn in place has been (nearly) not turning
};
static const int RT_MAX_ANIM_ENTS = 2048;

// A player's switch between motions (R_RuntimeXModel_PoseDObj): each action, and a weapon's set, came in against his
// locomotion and aim alone, not from the pose he was in - into the throw from the grenade's ready pose, an action cut
// short by another, a new gun's set of motions: the pose jumped. On a change of the action, its motion or the set, his
// last pose eases into the new one over RT_POSE_FADE_MS (not the root, which carries his facing in the world; not
// RE4's quick turn or the death, blended their own way).
static const int RT_POSE_FADE_MS = 150;
static const int RT_POSE_FADE_BONES = 192;
struct RtPoseFade
{
    const DObj *obj;
    int lastTime, start;
    int action, anim; // the last pose's
    int cycle;        // its locomotion cycle (RtAnimState::cur) and how far that cycle's crossfade had come
    float cycleBlend;
    char set[32];
    int bones;
    float last[RT_POSE_FADE_BONES][7]; // the last pose: each bone's quat, trans
    float from[RT_POSE_FADE_BONES][7]; // the pose faded from
};
static RtPoseFade s_poseFades[64]; // by client
static RtAnimState s_animStates[RT_MAX_ANIM_ENTS];
static volatile long s_animStateLock; // the renderer's worker threads pose DObjs too

static void *RtAlloc(int size)
{
    void *p = _aligned_malloc(size ? size : 16, 16);
    if ( !p )
        throw std::bad_alloc(); // (R_RuntimeXModel_LoadForLevel reports it)
    memset(p, 0, size ? size : 16);
    s_allocs.push_back(p);
    return p;
}

// the characters' envMapParms (RtCreateMaterial), their template's min / max kept to scale live (leon_matrefl)
struct RtEnvParms
{
    MaterialConstantDef *constant;
    float base[2];
};
static std::vector<RtEnvParms> s_envParms;
static char s_envScale[32];

static void RtApplyEnvScale()
{
    const dvar_s *refl = Dvar_FindVar("leon_matrefl");
    const char *text = refl && *refl->current.string ? refl->current.string : "0.5";
    if ( !strcmp(text, s_envScale) )
        return;
    I_strncpyz(s_envScale, text, sizeof(s_envScale));
    const float scale = (float)atof(text);
    for ( RtEnvParms &e : s_envParms )
    {
        e.constant->literal[0] = e.base[0] * scale;
        e.constant->literal[1] = e.base[1] * scale;
    }
}

static void RtFreeAll()
{
    s_envParms.clear();
    s_envScale[0] = 0;
    s_anims.clear();
    memset(s_animStates, 0, sizeof(s_animStates));
    memset(s_laserModels, 0, sizeof(s_laserModels));
    memset(s_laserPieces, 0, sizeof(s_laserPieces));
    s_haloModel = 0;
    s_hudMaterials.clear();
    for ( void *p : s_allocs )
        _aligned_free(p);
    s_allocs.clear();
}

static char *RtString(const char *s)
{
    char *p = (char *)RtAlloc((int)strlen(s) + 1);
    strcpy(p, s);
    return p;
}

void __cdecl R_RuntimeXModel_RegisterDvars()
{
    if ( !bo1_rtmodels )
        bo1_rtmodels = _Dvar_RegisterString("bo1_rtmodels", "", 0,
            "mod: runtime models built at level load from rtmodels/<name>.rlm (space-separated names)");
    if ( !bo1_rtmodels_loaded )
        bo1_rtmodels_loaded = _Dvar_RegisterString("bo1_rtmodels_loaded", "", 0,
            "mod: the bo1_rtmodels built for this level (the Leon mod's scripts offer these characters)");
    // (archived: the player config keeps the last character picked)
    static bool s_characterRegistered;
    if ( !s_characterRegistered )
        s_characterRegistered = !!_Dvar_RegisterString("leon_character", "leon", 1,
            "mod: the Leon mod's character (maps\\leon\\_character.gsc): leon, ada, hunk or luis");
    static bool s_reflRegistered;
    if ( !s_reflRegistered )
        s_reflRegistered = !!_Dvar_RegisterString("leon_matrefl", "0.5", 0,
            "mod: the characters' reflection probe strength (RtCreateMaterial), live: lower is more matte and darker");
}

static int RtLevelZoneIndex(const char *mapName)
{
    for ( int i = 1; i < 33; ++i )
    {
        if ( g_zoneNames[i].name[0] && !I_stricmp(g_zoneNames[i].name, mapName) )
            return i;
    }
    return 0;
}

// links a header into the DB as the level zone's asset; returns the pool copy
static XAssetHeader RtLink(XAssetType type, XAssetHeader header, int zoneIndex)
{
    XAssetEntry entry;
    memset(&entry, 0, sizeof(entry));
    entry.asset.type = type;
    entry.asset.header = header;
    entry.zoneIndex = (unsigned __int8)zoneIndex;
    Sys_EnterCriticalSection(CRITSECT_DBHASH);
    unsigned int savedZone = g_zoneIndex;
    g_zoneIndex = zoneIndex;
    XAssetEntryPoolEntry *linked = DB_LinkXAssetEntry(&entry, 0);
    g_zoneIndex = savedZone;
    Sys_LeaveCriticalSection(CRITSECT_DBHASH);
    return linked->entry.asset.header;
}

struct RtReader
{
    const unsigned __int8 *base, *p, *end;
    bool ok = true;
    const void *take(size_t n)
    {
        if ( !ok || (size_t)(end - p) < n )
        {
            ok = false;
            return 0;
        }
        const void *r = p;
        p += n;
        return r;
    }
    unsigned int u32() { const void *r = take(4); return r ? *(const unsigned int *)r : 0; }
    unsigned short u16() { const void *r = take(2); return r ? *(const unsigned short *)r : 0; }
    unsigned char u8() { const void *r = take(1); return r ? *(const unsigned char *)r : 0; }
};

static Material *RtTemplateMaterial()
{
    for ( const char *name : s_templateModels )
    {
        if ( !DB_FindXAssetEntry(ASSET_TYPE_XMODEL, name) )
            continue;
        XModel *model = DB_FindXAssetHeader(ASSET_TYPE_XMODEL, (char *)name, 1, -1).model;
        if ( model && model->numsurfs && model->materialHandles && model->materialHandles[0] )
            return model->materialHandles[0];
    }
    return 0;
}

// how strongly a shot plays RE4's fire motion on the body (1: fully; the light machine guns less: their recoil was
// too big for a gun fired in bursts), set per frame by the player's weapon (CG_RuntimeXModel_Pose)
static thread_local float s_fireWeight = 1.0f;

void __cdecl R_RuntimeXModel_SetFireWeight(float weight)
{
    s_fireWeight = weight < 0.0f ? 0.0f : (weight > 1.0f ? 1.0f : weight);
}

// how fast a shot's motion plays its kick, up to the kick's peak, and on from there (1: RE4's own; most guns settle at
// 2.3 - the gun floated back onto the aim -, the automatics kick and settle at 3: RE4's kick, made for one shot, never
// peaked between their shots and pushed slowly), set per frame by the player's weapon (CG_RuntimeXModel_Pose)
static thread_local float s_fireKick = 1.0f, s_fireSettle = 1.0f;

void __cdecl R_RuntimeXModel_SetFireRates(float kick, float settle)
{
    s_fireKick = kick < 0.5f ? 0.5f : (kick > 4.0f ? 4.0f : kick);
    s_fireSettle = settle < 0.5f ? 0.5f : (settle > 4.0f ? 4.0f : settle);
}

// seconds since the previous shot's start, for the next R_RuntimeXModel_PoseDObj's shot only (-1: none): a shot fired
// while the last one's motion still plays crossfades from where that one is (RT_FIRE_CROSSFADE), instead of snapping
// back to the motion's start - an automatic's, the Ray Gun's heavy magnum kick, jerked at every shot
static thread_local float s_firePrevTime = -1.0f;
static const float RT_FIRE_CROSSFADE = 0.08f;

void __cdecl R_RuntimeXModel_SetFirePrev(float seconds)
{
    s_firePrevTime = seconds;
}

// a shot's motion's phase (frames) at its time (seconds): the kick up to its peak at s_fireKick, then the settle at
// s_fireSettle (R_RuntimeXModel_SetFireRates)
static float RtFirePhase(const RtAnim &motion, float time)
{
    const float peak = (float)motion.recoilPeak;
    const float kicked = time * motion.fps * s_fireKick;
    return kicked <= peak ? kicked : peak + (time * motion.fps - peak / s_fireKick) * s_fireSettle;
}

// The settings for the next R_RuntimeXModel_PoseDObj (these, the look's and the fire's above) are per thread: the
// renderer's worker threads pose DObjs too, and one's pose of a zombie between another's setting and its pose of the
// player would take them (and clear them: the player posed without his own for that frame).
// the local player's locomotion under RE4's tank controls, for the next R_RuntimeXModel_PoseDObj only: strafeLegs -
// aimed in and stepping sideways (PM_TankControls), his legs turn toward the step and his torso stays on the aim;
// turnInPlace - turning on the spot (A / D, the mouse) plays RE4's turn in place (pl_R1_Turn: turn_left / turn_right)
static thread_local bool s_strafeLegs, s_turnInPlace, s_stepWatch;
static thread_local int s_moveIntent; // the player's move keys (1 forward, -1 back, 0 neither): his cycle's way under tank controls
// a player's run under RE4's controls (R_RuntimeXModel_SetRunning: bg_leon.h LEON_EF2_RUNNING), for the next pose only:
// 1 the run, 0 a walk, -1 not told (the cycle by his speed)
static thread_local int s_running = -1;
static int s_playerTurn; // the last such pose's turn in place: 1 left, -1 right, 0 none
static bool s_playerBodyAction; // the last such pose's quick turn or kick still on him (R_RuntimeXModel_PlayerBodyAction)
static bool s_playerEasing; // the last such pose eased from another (RtPoseFade: R_RuntimeXModel_PlayerEasing)
// his footsteps: the cycle's foot plants passed since the client last took them, and whether the last was a run's
// (counted on whichever thread poses him - the renderer's workers pose DObjs too - and taken on the client's)
static volatile long s_playerSteps;
static volatile bool s_playerStepRun;

// where in RE4's reload the next R_RuntimeXModel_PoseDObj's reload action starts (0..1: 0 its start; a new gun's first
// raise plays its end, the gun worked)
static thread_local float s_reloadFrom;

void __cdecl R_RuntimeXModel_SetReloadFrom(float from)
{
    s_reloadFrom = from < 0.0f ? 0.0f : (from > 0.95f ? 0.95f : from);
}

// the motion set the next R_RuntimeXModel_PoseDObj's reload comes from when it is not its animSet's (a gun posed with
// one RE4 weapon's motions and reloaded with another's: the Ray Gun's magnum stance, the handgun's reload); 0 its own
static thread_local const char *s_reloadSet;
static thread_local bool s_reloadSetGiven;
// which roundhouse the next pose's kick plays (R_RuntimeXModel_SetKickVariant): 0 em10's, 1 the character's own
// second ("kick2"; the first where a model has none)
static thread_local int s_kickVariant;

void __cdecl R_RuntimeXModel_SetKickVariant(int variant)
{
    s_kickVariant = variant;
}

// the motion the next R_RuntimeXModel_PoseDObj's RT_ACTION_MOTION plays (an enemy's: its swing, its flinch), by name
static thread_local const char *s_actionMotion;

void __cdecl R_RuntimeXModel_SetActionMotion(const char *name)
{
    s_actionMotion = name;
}

void __cdecl R_RuntimeXModel_SetReloadSet(const char *set)
{
    s_reloadSet = set;
    s_reloadSetGiven = true;
}

void __cdecl R_RuntimeXModel_SetRunning(int running)
{
    s_running = running;
}

void __cdecl R_RuntimeXModel_SetPlayerMotion(bool strafeLegs, bool turnInPlace, int moveIntent)
{
    s_strafeLegs = strafeLegs;
    s_turnInPlace = turnInPlace;
    s_moveIntent = moveIntent;
    s_stepWatch = true;
}

int __cdecl R_RuntimeXModel_TakePlayerSteps(bool *run)
{
    *run = s_playerStepRun;
    return (int)_InterlockedExchange(&s_playerSteps, 0);
}


int __cdecl R_RuntimeXModel_PlayerTurn()
{
    return s_playerTurn;
}

bool __cdecl R_RuntimeXModel_PlayerBodyAction()
{
    return s_playerBodyAction;
}

bool __cdecl R_RuntimeXModel_PlayerEasing()
{
    return s_playerEasing;
}

// RE4's turn in place turns him 175.6 degrees in its 40 frames at 30 Hz: the rate its steps match
static const float RT_TURN_IN_PLACE_RATE = 175.6f / 40.0f * 30.0f;

// masked: the texture's alpha is a cut-out mask (hair strands, lashes)
static GfxImage *RtCreateImage(RtReader &r, const char *modelName, int zoneIndex, bool gpu, bool *masked)
{
    char texName[33] = {};
    if ( const void *n = r.take(32) )
        memcpy(texName, n, 32);
    unsigned int width = r.u16(), height = r.u16(), mips = r.u8();
    *masked = r.u8() != 0;
    r.u16();
    size_t bytes = 0;
    for ( unsigned int m = 0; m < mips; ++m )
        bytes += (size_t)((width >> m) ? (width >> m) : 1) * ((height >> m) ? (height >> m) : 1) * 4;
    const void *pixels = r.take(bytes);
    if ( !r.ok || !width || !height || !mips )
        return 0;

    GfxImage image;
    memset(&image, 0, sizeof(image));
    image.mapType = 3; // MAPTYPE_2D
    image.semantic = 2; // colour map
    image.category = 3;
    image.noPicmip = 1;
    image.width = (unsigned __int16)width;
    image.height = (unsigned __int16)height;
    image.depth = 1;
    image.levelCount = (unsigned __int8)mips;
    image.name = RtString(va("%s/%s", modelName, texName));
    GfxImage *pooled = RtLink(ASSET_TYPE_IMAGE, (XAssetHeader)&image, zoneIndex).image;
    if ( gpu )
    {
        void *buffer = Z_VirtualAlloc((int)bytes, "Image_AllocTempMemory", 20);
        memcpy(buffer, pixels, bytes);
        RB_Resource_CreateTexture(pooled, buffer, (int)mips, 0, D3DFMT_A8R8G8B8);
    }
    return pooled;
}

// Template materials: a loaded lit model material of the plainest technique set available (colour, normal and
// specular maps; no weapon reflection or colour detail), preferring the earlier names. "r0" sets are opaque, "t0"
// ones alpha-test the colour map (the perk machines' mc_l_sm_t0c0n0s0 is in common_zombie, so on every level).
static const char *const s_opaqueTechsets[] = { "mc_l_sm_r0c0n0s0", "mc_l_sm_r0c0n0s0_hero", "mc_l_sm_r0c0n0s0x0",
                                                 "mc_l_sm_r0c0s0", "mc_l_sm_r0c0" };
static const char *const s_alphaTestTechsets[] = { "mc_l_sm_t0c0n0s0", "mc_l_sm_t0c0n0s0x0", "mc_l_sm_t0c0s0",
                                                    "mc_l_sm_t0c0" };

struct RtTemplateSearch
{
    const char *const *techsets;
    int count;
    int best; // index into techsets of the found one
    const Material *found;
};

static void RtFindTemplate(XAssetHeader header, void *data)
{
    RtTemplateSearch *search = (RtTemplateSearch *)data;
    const Material *m = header.material;
    if ( !m || !m->techniqueSet || !m->techniqueSet->name || !m->stateBitsTable )
        return;
    for ( int t = 0; t < search->count && t < search->best; ++t )
    {
        if ( strcmp(m->techniqueSet->name, search->techsets[t]) )
            continue;
        for ( int i = 0; i < m->textureCount; ++i )
        {
            if ( m->textureTable[i].nameHash == R_HashString("colorMap") )
            {
                search->best = t;
                search->found = m;
                return;
            }
        }
        return;
    }
}

static const Material *RtFindTemplateMaterial(const char *const *techsets, int count)
{
    RtTemplateSearch search = { techsets, count, count, 0 };
    DB_EnumXAssets_FastFile(ASSET_TYPE_MATERIAL, RtFindTemplate, &search, false);
    return search.found;
}

// Culling stays the template's: drawn two-sided, the back of a hair card is lit as if it faced the camera (the lit
// shaders do not flip the normal), which turns whole cards white under real lighting; RE4's hair shell is built to
// be drawn one-sided.
static Material *RtCreateMaterial(const Material *templ, GfxImage *colorMap, const char *name, int zoneIndex)
{
    Material material;
    memcpy(&material, templ, sizeof(material));
    material.info.name = RtString(name);
    MaterialTextureDef *table = (MaterialTextureDef *)RtAlloc(sizeof(MaterialTextureDef) * (templ->textureCount ? templ->textureCount : 1));
    const unsigned int colorMapHash = R_HashString("colorMap");
    for ( int i = 0; i < templ->textureCount; ++i )
    {
        table[i] = templ->textureTable[i];
        if ( table[i].nameHash == colorMapHash )
            table[i].u.image = colorMap;
        else if ( table[i].semantic == 5 ) // normal map: flat
            table[i].u.image = rgp.identityNormalMapImage;
        else if ( table[i].semantic == 8 ) // specular: none, at the roughest gloss (its alpha; a transparent black
            table[i].u.image = rgp.blackImage;  // was glossier, a grey one chrome)
    }
    material.textureTable = table;
    // the template's environment reflection (envMapParms: the reflection probe's min / max by the view's angle, its
    // falloff and the sun's specular) is its own surface's (the zombies templates are polished steel and the like):
    // Leon's cloth, leather and skin take no sun specular, or they look wet - and the probe's reflection scaled
    // (leon_matrefl, 0.5, live: RtApplyEnvScale). With a black specular map it is most of their light (at 0 they went
    // black), at its full strength a polished sheen: under a bright light a character looked like metal
    if ( templ->constantCount && templ->constantTable )
    {
        MaterialConstantDef *constants = (MaterialConstantDef *)RtAlloc(sizeof(MaterialConstantDef) * templ->constantCount);
        for ( int i = 0; i < templ->constantCount; ++i )
        {
            constants[i] = templ->constantTable[i];
            if ( !strncmp(constants[i].name, "envMapParms", 11) )
            {
                constants[i].literal[3] = 0.0f;
                s_envParms.push_back({ &constants[i], { constants[i].literal[0], constants[i].literal[1] } });
            }
        }
        material.constantTable = constants;
    }
    return RtLink(ASSET_TYPE_MATERIAL, (XAssetHeader)&material, zoneIndex).material;
}

// The vertex list's collision tree (impact marks: XSurfaceVisitTrianglesInAabb). The engine's builder
// (XModelReadSurface_BuildCollisionTree) does not return on these surfaces, so build the smallest valid tree:
// one root node over the whole surface whose children are leaves (childCount | 0x8000), one triangle per leaf.
// Queries then test every triangle, which is fine for the few marks a character takes.
static XSurfaceCollisionTree *RtBuildCollisionTree(const XSurface *surf)
{
    float lo[3] = { 1e9f, 1e9f, 1e9f }, hi[3] = { -1e9f, -1e9f, -1e9f };
    for ( unsigned int v = 0; v < surf->vertCount; ++v )
    {
        for ( int k = 0; k < 3; ++k )
        {
            const float x = surf->verts0[v].xyz[k];
            lo[k] = x < lo[k] ? x : lo[k];
            hi[k] = x > hi[k] ? x : hi[k];
        }
    }
    XSurfaceCollisionTree *tree = (XSurfaceCollisionTree *)RtAlloc(sizeof(XSurfaceCollisionTree));
    for ( int k = 0; k < 3; ++k )
    {
        tree->trans[k] = -lo[k];
        tree->scale[k] = hi[k] > lo[k] ? 65535.0f / (hi[k] - lo[k]) : 1.0f;
    }
    const unsigned int leafCount = surf->triCount < 0x7FFF ? surf->triCount : 0x7FFF;
    tree->nodeCount = 1;
    tree->nodes = (XSurfaceCollisionNode *)RtAlloc(sizeof(XSurfaceCollisionNode));
    for ( int k = 0; k < 3; ++k )
    {
        tree->nodes[0].aabb.mins[k] = 0;
        tree->nodes[0].aabb.maxs[k] = 0xFFFF;
    }
    tree->nodes[0].childBeginIndex = 0;
    tree->nodes[0].childCount = (unsigned __int16)(0x8000 | leafCount);
    tree->leafCount = leafCount;
    tree->leafs = (XSurfaceCollisionLeaf *)RtAlloc(sizeof(XSurfaceCollisionLeaf) * (leafCount ? leafCount : 1));
    for ( unsigned int t = 0; t < leafCount; ++t )
        tree->leafs[t].triangleBeginIndex = (unsigned __int16)t;
    return tree;
}

static void RtBuildSurface(RtReader &r, XSurface *surf, float *mins, float *maxs)
{
    unsigned int vertCount = r.u32(), triCount = r.u32();
    const float *src = (const float *)r.take((size_t)vertCount * 32);
    const unsigned short *idx = (const unsigned short *)r.take((size_t)triCount * 6);
    while ( r.ok && ((r.p - r.base) & 3) )
        r.take(1);
    if ( !r.ok || !vertCount || vertCount > 0xFFFF || triCount > 0xFFFF )
    {
        r.ok = false;
        return;
    }

    XVertexInfo_s *info = (XVertexInfo_s *)_aligned_malloc(sizeof(XVertexInfo_s) * vertCount, 16);
    memset(info, 0, sizeof(XVertexInfo_s) * vertCount);
    for ( unsigned int v = 0; v < vertCount; ++v )
    {
        const float *f = src + 8 * v;
        XVertexInfo_s &x = info[v];
        for ( int k = 0; k < 3; ++k )
        {
            x.offset[k] = f[k];
            x.normal[k] = f[3 + k];
            mins[k] = f[k] < mins[k] ? f[k] : mins[k];
            maxs[k] = f[k] > maxs[k] ? f[k] : maxs[k];
        }
        // a tangent frame around the normal (version 1 uses a flat normal map, so any frame will do)
        float up[3] = { 0.0f, 0.0f, 1.0f };
        if ( fabsf(x.normal[2]) > 0.9f )
            up[0] = 1.0f, up[2] = 0.0f;
        float t[3] = { up[1] * x.normal[2] - up[2] * x.normal[1], up[2] * x.normal[0] - up[0] * x.normal[2],
                       up[0] * x.normal[1] - up[1] * x.normal[0] };
        float len = sqrtf(t[0] * t[0] + t[1] * t[1] + t[2] * t[2]);
        len = len > 0.0f ? 1.0f / len : 0.0f;
        for ( int k = 0; k < 3; ++k )
            x.tangent[k] = t[k] * len;
        x.binormal[0] = x.normal[1] * x.tangent[2] - x.normal[2] * x.tangent[1];
        x.binormal[1] = x.normal[2] * x.tangent[0] - x.normal[0] * x.tangent[2];
        x.binormal[2] = x.normal[0] * x.tangent[1] - x.normal[1] * x.tangent[0];
        x.texCoordX = f[6];
        x.texCoordY = f[7];
        x.color[0] = x.color[1] = x.color[2] = x.color[3] = 0xFF;
        x.numWeights = 0;
        x.boneOffset = 0;
    }

    surf->tileMode = 0;
    surf->flags = 0;
    surf->vertCount = (unsigned __int16)vertCount;
    surf->triCount = (unsigned __int16)triCount;
    surf->verts0 = (GfxPackedVertex *)RtAlloc(sizeof(GfxPackedVertex) * vertCount);
    XSurfaceTransfer((const XVertexBuffer *)info, surf->verts0, surf->verts0, (int)vertCount);
    _aligned_free(info);
    surf->triIndices = (unsigned __int16 *)RtAlloc((int)triCount * 6);
    memcpy(surf->triIndices, idx, (size_t)triCount * 6);
    surf->vertListCount = 1;
    surf->vertList = (XRigidVertList *)RtAlloc(sizeof(XRigidVertList));
    surf->vertList->boneOffset = 0; // bone 0 << 6
    surf->vertList->vertCount = (unsigned __int16)vertCount;
    surf->vertList->triOffset = 0;
    surf->vertList->triCount = (unsigned __int16)triCount;
    surf->vertList->collisionTree = RtBuildCollisionTree(surf);
    surf->partBits[0] = (int)0x80000000; // bone 0
}

struct RtSkinVert // the .rlm v2 vertex
{
    float pos[3];
    float normal[3];
    float uv[2];
    unsigned __int16 bone[4];
    float weight[4];
};

static void RtFillVertexInfo(XVertexInfo_s &x, const float *pos, const float *normal, const float *uv)
{
    for ( int k = 0; k < 3; ++k )
    {
        x.offset[k] = pos[k];
        x.normal[k] = normal[k];
    }
    // a tangent frame around the normal (the materials use a flat normal map, so any frame will do)
    float up[3] = { 0.0f, 0.0f, 1.0f };
    if ( fabsf(x.normal[2]) > 0.9f )
        up[0] = 1.0f, up[2] = 0.0f;
    float t[3] = { up[1] * x.normal[2] - up[2] * x.normal[1], up[2] * x.normal[0] - up[0] * x.normal[2],
                   up[0] * x.normal[1] - up[1] * x.normal[0] };
    float len = sqrtf(t[0] * t[0] + t[1] * t[1] + t[2] * t[2]);
    len = len > 0.0f ? 1.0f / len : 0.0f;
    for ( int k = 0; k < 3; ++k )
        x.tangent[k] = t[k] * len;
    x.binormal[0] = x.normal[1] * x.tangent[2] - x.normal[2] * x.tangent[1];
    x.binormal[1] = x.normal[2] * x.tangent[0] - x.normal[0] * x.tangent[2];
    x.binormal[2] = x.normal[0] * x.tangent[1] - x.normal[1] * x.tangent[0];
    x.texCoordX = uv[0];
    x.texCoordY = uv[1];
    x.color[0] = x.color[1] = x.color[2] = x.color[3] = 0xFF;
    x.numWeights = 0;
    x.boneOffset = 0;
}

// A weighted (deformed) surface, skinned on the CPU from the DObj's bone matrices (R_SkinXSurfaceWeight*): the
// vertices sorted by extra-weight count (0..3: vertInfo.vertCount[k]), vertsBlend per vertex {bone0 << 6, then
// (bone << 6, weight * 65536) per extra weight} - bone 0's weight is the rest, and it alone turns the normal, so
// it is the heaviest. No rigid vertex lists, so no collision tree (marks skip deformed surfaces).
static void RtBuildSkinnedSurface(RtReader &r, XSurface *surf, int numBones, const float (*boneWorld)[3],
                                  float (*boneMins)[3], float (*boneMaxs)[3], float *mins, float *maxs)
{
    unsigned int vertCount = r.u32(), triCount = r.u32();
    const RtSkinVert *src = (const RtSkinVert *)r.take((size_t)vertCount * sizeof(RtSkinVert));
    const unsigned short *idx = (const unsigned short *)r.take((size_t)triCount * 6);
    while ( r.ok && ((r.p - r.base) & 3) )
        r.take(1);
    if ( !r.ok || !vertCount || vertCount > 0xFFFF || triCount > 0xFFFF )
    {
        r.ok = false;
        return;
    }

    // extra-weight count per vertex, then the new order (stable within a group)
    std::vector<int> group(vertCount), order, newIndex(vertCount);
    int groupCount[4] = {};
    for ( unsigned int v = 0; v < vertCount; ++v )
    {
        int n = 0;
        for ( int k = 1; k < 4; ++k )
            if ( src[v].weight[k] > 0.0f && src[v].bone[k] < numBones )
                n = k;
        group[v] = n;
        ++groupCount[n];
    }
    for ( int g = 0; g < 4; ++g )
        for ( unsigned int v = 0; v < vertCount; ++v )
            if ( group[v] == g )
            {
                newIndex[v] = (int)order.size();
                order.push_back((int)v);
            }

    XVertexInfo_s *info = (XVertexInfo_s *)_aligned_malloc(sizeof(XVertexInfo_s) * vertCount, 16);
    memset(info, 0, sizeof(XVertexInfo_s) * vertCount);
    size_t blendCount = 0;
    for ( int g = 0; g < 4; ++g )
        blendCount += (size_t)groupCount[g] * (1 + 2 * g);
    surf->vertInfo.vertsBlend = (unsigned __int16 *)RtAlloc((int)(blendCount * 2));
    unsigned __int16 *blend = surf->vertInfo.vertsBlend;
    for ( unsigned int n = 0; n < vertCount; ++n )
    {
        const RtSkinVert &v = src[order[n]];
        RtFillVertexInfo(info[n], v.pos, v.normal, v.uv);
        const int bone0 = v.bone[0] < numBones ? v.bone[0] : 0;
        *blend++ = (unsigned __int16)(bone0 << 6);
        surf->partBits[bone0 >> 5] |= (int)(0x80000000u >> (bone0 & 31));
        for ( int k = 1; k <= group[order[n]]; ++k )
        {
            const int b = v.bone[k];
            const float w = v.weight[k] * 65536.0f + 0.5f;
            *blend++ = (unsigned __int16)(b << 6);
            *blend++ = (unsigned __int16)(w > 65535.0f ? 65535.0f : w);
            surf->partBits[b >> 5] |= (int)(0x80000000u >> (b & 31));
        }
        for ( int k = 0; k < 3; ++k )
        {
            mins[k] = v.pos[k] < mins[k] ? v.pos[k] : mins[k];
            maxs[k] = v.pos[k] > maxs[k] ? v.pos[k] : maxs[k];
            const float local = v.pos[k] - boneWorld[bone0][k]; // bone space: the bind pose has no rotation
            boneMins[bone0][k] = local < boneMins[bone0][k] ? local : boneMins[bone0][k];
            boneMaxs[bone0][k] = local > boneMaxs[bone0][k] ? local : boneMaxs[bone0][k];
        }
    }
    for ( int g = 0; g < 4; ++g )
        surf->vertInfo.vertCount[g] = (__int16)groupCount[g];

    surf->tileMode = 0;
    surf->flags = 0x82; // XSURFACE_FLAG_DEFORMED | XSURFACE_FLAG_SKINNED
    surf->vertCount = (unsigned __int16)vertCount;
    surf->triCount = (unsigned __int16)triCount;
    surf->verts0 = (GfxPackedVertex *)RtAlloc(sizeof(GfxPackedVertex) * vertCount);
    XSurfaceTransfer((const XVertexBuffer *)info, surf->verts0, surf->verts0, (int)vertCount);
    _aligned_free(info);
    surf->triIndices = (unsigned __int16 *)RtAlloc((int)triCount * 6);
    for ( unsigned int i = 0; i < triCount * 3; ++i )
        surf->triIndices[i] = (unsigned __int16)newIndex[idx[i] < vertCount ? idx[i] : 0];
    surf->vertListCount = 0;
    surf->vertList = 0;
}

// RE4's run bobs his pelvis 3.3" (its walk 2"), and the game's pace plays it ~1.4 times as fast: the upper body
// (j_spinelower, on the pelvis) is held 40 % of the way back toward the pelvis's mean height, its bob near the walk's;
// the legs keep the pelvis's, so the feet stay planted. frames: a cycle's (this load's own copy). Returns the offsets
// added to the spine, 3 floats a frame: an upper body action over the run (a reload, a shot) takes the spine from the
// cycle and with it this, and is given it back (R_RuntimeXModel_PoseDObj) - the bob came back with every reload.
static const float *RtSoftenRunBob(float *frames, int frameCount, int boneCount, int pelvis, int spine)
{
    float *added = (float *)RtAlloc(3 * frameCount * (int)sizeof(float));
    float mean = 0.0f;
    for ( int f = 0; f < frameCount; ++f )
        mean += frames[7 * (f * boneCount + pelvis) + 6];
    mean /= (float)frameCount;
    for ( int f = 0; f < frameCount; ++f )
    {
        const float *p = &frames[7 * (f * boneCount + pelvis)];
        // the height given back (the root's Z, up), turned into the pelvis's frame: by its rotation's inverse
        const float v[3] = { 0.0f, 0.0f, (mean - p[6]) * 0.4f };
        const float ux = -p[0], uy = -p[1], uz = -p[2], w = p[3];
        const float tx = 2.0f * (uy * v[2] - uz * v[1]), ty = 2.0f * (uz * v[0] - ux * v[2]), tz = 2.0f * (ux * v[1] - uy * v[0]);
        float *a = &added[3 * f];
        a[0] = v[0] + w * tx + (uy * tz - uz * ty);
        a[1] = v[1] + w * ty + (uz * tx - ux * tz);
        a[2] = v[2] + w * tz + (ux * ty - uy * tx);
        float *s = &frames[7 * (f * boneCount + spine)];
        for ( int k = 0; k < 3; ++k )
            s[4 + k] += a[k];
    }
    return added;
}

// a cycle's held-back bob at phase (frames): the run's (RtSoftenRunBob), else none
static void RtSampleRunBob(const RtAnim &anim, float phase, float *out)
{
    out[0] = out[1] = out[2] = 0.0f;
    if ( !anim.runBob )
        return;
    int f0 = (int)phase;
    if ( f0 >= anim.frameCount )
        f0 = anim.frameCount - 1;
    const int f1 = (f0 + 1) % anim.frameCount;
    const float frac = phase - (float)f0;
    for ( int k = 0; k < 3; ++k )
        out[k] = anim.runBob[3 * f0 + k] + (anim.runBob[3 * f1 + k] - anim.runBob[3 * f0 + k]) * frac;
}

// builds a model from .rlm data (a file's, or one made in memory); returns the DB's copy
static const XModel *RtBuildModel(const char *name, const void *data, int size, int zoneIndex, const Material *templ,
                                  const Material *atestTempl, bool gpu)
{
    RtReader r{ (const unsigned __int8 *)data, (const unsigned __int8 *)data, (const unsigned __int8 *)data + size };
    const void *magic = r.take(4);
    unsigned int version = r.u32(), texCount = r.u32(), surfCount = r.u32();
    unsigned int boneCount = version >= 2 ? r.u32() : 1;
    if ( !r.ok || memcmp(magic, "RLM1", 4) || (version != 1 && version != 2) || !surfCount || surfCount > 255 || !boneCount
         || boneCount >= 160 )
    {
        Com_PrintWarning(0, "runtime model '%s': not a version 1 or 2 .rlm file\n", name);
        return 0;
    }

    std::vector<Material *> materials;
    for ( unsigned int t = 0; t < texCount && r.ok; ++t )
    {
        bool masked = false;
        GfxImage *image = RtCreateImage(r, name, zoneIndex, gpu, &masked);
        const Material *from = masked && atestTempl ? atestTempl : templ;
        materials.push_back(image ? RtCreateMaterial(from, image, va("%s_mtl%u", name, t), zoneIndex) : 0);
    }

    // the skeleton (version 1: only tag_origin); roots first, parents before children
    std::vector<int> parent(boneCount, -1);
    std::vector<float> localTrans(boneCount * 3, 0.0f), localQuat(boneCount * 4, 0.0f);
    std::vector<std::string> boneName(boneCount, "tag_origin");
    localQuat[3] = 1.0f;
    for ( unsigned int b = 0; version >= 2 && b < boneCount && r.ok; ++b )
    {
        char boneStr[33] = {};
        if ( const void *n = r.take(32) )
            memcpy(boneStr, n, 32);
        boneName[b] = boneStr;
        parent[b] = (int)r.u32();
        const float *f = (const float *)r.take(7 * sizeof(float));
        for ( int k = 0; f && k < 3; ++k )
            localTrans[3 * b + k] = f[k];
        for ( int k = 0; f && k < 4; ++k )
            localQuat[4 * b + k] = f[3 + k];
    }
    int numRoot = 0;
    while ( numRoot < (int)boneCount && parent[numRoot] < 0 )
        ++numRoot;
    for ( unsigned int b = numRoot; b < boneCount; ++b )
        if ( parent[b] < 0 || parent[b] >= (int)b || (int)b - parent[b] > 255 )
            r.ok = false; // roots first, parents before children, within the u8 parent offset
    if ( !r.ok || !numRoot )
    {
        Com_PrintWarning(0, "runtime model '%s': bad skeleton\n", name);
        return 0;
    }
    // bind pose: the vertices' rest pose, every bone unrotated at its accumulated offset
    std::vector<float> world(boneCount * 3, 0.0f);
    for ( unsigned int b = numRoot; b < boneCount; ++b )
        for ( int k = 0; k < 3; ++k )
            world[3 * b + k] = world[3 * parent[b] + k] + localTrans[3 * b + k];

    XModel model;
    memset(&model, 0, sizeof(model));
    model.name = RtString(name);
    model.numBones = (unsigned __int8)boneCount;
    model.numRootBones = (unsigned __int8)numRoot;
    model.numsurfs = (unsigned __int8)surfCount;
    model.boneNames = (unsigned __int16 *)RtAlloc(sizeof(unsigned __int16) * boneCount);
    for ( unsigned int b = 0; b < boneCount; ++b )
        model.boneNames[b] = (unsigned __int16)SL_GetString((char *)boneName[b].c_str(), 4, SCRIPTINSTANCE_SERVER);
    const int nonRoot = (int)boneCount - numRoot;
    if ( nonRoot )
    {
        // the local pose DObjCalcBaseAnim builds the skeleton from: parent offset (child - parent), the rotation in
        // s16 (x 32767) and the translation, per non-root bone
        model.parentList = (unsigned __int8 *)RtAlloc(nonRoot);
        model.quats = (__int16 *)RtAlloc(sizeof(__int16) * 4 * nonRoot);
        model.trans = (float *)RtAlloc(sizeof(float) * 3 * nonRoot);
        for ( int i = 0; i < nonRoot; ++i )
        {
            const int b = numRoot + i;
            model.parentList[i] = (unsigned __int8)(b - parent[b]);
            const float *q = &localQuat[4 * b];
            const float len = sqrtf(q[0] * q[0] + q[1] * q[1] + q[2] * q[2] + q[3] * q[3]);
            for ( int k = 0; k < 4; ++k )
            {
                const float c = len > 0.0f ? q[k] / len : (k == 3 ? 1.0f : 0.0f);
                model.quats[4 * i + k] = (__int16)floorf(c * 32767.0f + 0.5f);
            }
            for ( int k = 0; k < 3; ++k )
                model.trans[3 * i + k] = localTrans[3 * b + k];
        }
    }
    // the hit locations (hitLocation_t) of the bones named as BO1's, as a BO1 character's: a server trace (an AI's
    // melee, a bullet: DObjTraceline) only tests bones whose location ranks, so an unclassified body is never hit; the
    // other bones (fingers, hair, RE4's extra ones) take their parent's
    static const struct { const char *bone; unsigned char loc; } s_hitLocations[] = {
        { "j_mainroot", HITLOC_TORSO_LWR }, { "j_spinelower", HITLOC_TORSO_LWR }, { "j_spine4", HITLOC_TORSO_UPR },
        { "j_neck", HITLOC_NECK }, { "j_head", HITLOC_HEAD },
        { "j_clavicle_ri", HITLOC_TORSO_UPR }, { "j_shoulder_ri", HITLOC_R_ARM_UPR }, { "j_elbow_ri", HITLOC_R_ARM_LWR },
        { "j_wristtwist_ri", HITLOC_R_ARM_LWR }, { "j_wrist_ri", HITLOC_R_HAND },
        { "j_clavicle_le", HITLOC_TORSO_UPR }, { "j_shoulder_le", HITLOC_L_ARM_UPR }, { "j_elbow_le", HITLOC_L_ARM_LWR },
        { "j_wristtwist_le", HITLOC_L_ARM_LWR }, { "j_wrist_le", HITLOC_L_HAND },
        { "j_hip_ri", HITLOC_R_LEG_UPR }, { "j_knee_ri", HITLOC_R_LEG_LWR }, { "j_ankle_ri", HITLOC_R_FOOT },
        { "j_ball_ri", HITLOC_R_FOOT },
        { "j_hip_le", HITLOC_L_LEG_UPR }, { "j_knee_le", HITLOC_L_LEG_LWR }, { "j_ankle_le", HITLOC_L_FOOT },
        { "j_ball_le", HITLOC_L_FOOT },
    };
    model.partClassification = (unsigned __int8 *)RtAlloc(boneCount);
    for ( unsigned int b = 0; b < boneCount; ++b )
    {
        for ( const auto &hit : s_hitLocations )
        {
            if ( boneName[b] == hit.bone )
                model.partClassification[b] = hit.loc;
        }
    }
    model.baseMat = (DObjAnimMat *)RtAlloc(sizeof(DObjAnimMat) * boneCount);
    for ( unsigned int b = 0; b < boneCount; ++b )
    {
        model.baseMat[b].quat[3] = 1.0f;
        model.baseMat[b].transWeight = 2.0f;
        for ( int k = 0; k < 3; ++k )
            model.baseMat[b].trans[k] = world[3 * b + k];
    }

    model.surfs = (XSurface *)RtAlloc(sizeof(XSurface) * surfCount);
    model.materialHandles = (Material **)RtAlloc(sizeof(Material *) * surfCount);
    float mins[3] = { 1e9f, 1e9f, 1e9f }, maxs[3] = { -1e9f, -1e9f, -1e9f };
    std::vector<float> boneMins(boneCount * 3, 1e9f), boneMaxs(boneCount * 3, -1e9f);
    int baseTri = 0, baseVert = 0;
    for ( unsigned int s = 0; s < surfCount && r.ok; ++s )
    {
        unsigned int tex = r.u32();
        XSurface *surf = &model.surfs[s];
        if ( version >= 2 )
        {
            RtBuildSkinnedSurface(r, surf, (int)boneCount, (const float (*)[3])world.data(), (float (*)[3])boneMins.data(),
                                  (float (*)[3])boneMaxs.data(), mins, maxs);
        }
        else
        {
            RtBuildSurface(r, surf, mins, maxs);
            for ( int k = 0; k < 3; ++k )
            {
                boneMins[k] = mins[k];
                boneMaxs[k] = maxs[k];
            }
        }
        surf->baseTriIndex = (unsigned __int16)baseTri;
        surf->baseVertIndex = (unsigned __int16)baseVert;
        baseTri += surf->triCount;
        baseVert += surf->vertCount;
        for ( int k = 0; k < 5; ++k )
            model.lodInfo[0].partBits[k] |= surf->partBits[k];
        if ( surf->flags & 0x80 )
            model.lodRampType = 1;
        model.materialHandles[s] = tex < materials.size() && materials[tex] ? materials[tex] : (Material *)templ;
    }
    std::vector<RtAnim> anims;
    if ( r.ok && version >= 2 && r.end - r.p >= 8 && !memcmp(r.p, "ANM2", 4) )
    {
        r.take(4);
        const unsigned int animCount = r.u32();
        for ( unsigned int a = 0; a < animCount && r.ok; ++a )
        {
            RtAnim anim = {};
            if ( const void *n = r.take(32) )
                memcpy(anim.name, n, sizeof(anim.name) - 1);
            anim.frameCount = (int)r.u32();
            const void *fps = r.take(4);
            anim.fps = fps ? *(const float *)fps : 0.0f;
            anim.boneCount = (int)r.u32();
            const void *speed = r.take(4);
            anim.moveSpeed = speed ? *(const float *)speed : 0.0f;
            if ( anim.frameCount <= 0 || anim.frameCount > 100000 || anim.boneCount != (int)boneCount || !(anim.fps > 0.0f)
                 || !(anim.moveSpeed >= 0.0f) )
            {
                r.ok = false;
                break;
            }
            const int size = anim.frameCount * anim.boneCount * 7 * (int)sizeof(float);
            if ( const void *src = r.take(size) )
            {
                float *frames = (float *)RtAlloc(size);
                memcpy(frames, src, size);
                anim.frames = frames;
                anims.push_back(anim);
            }
        }
    }
    if ( !r.ok )
    {
        Com_PrintWarning(0, "runtime model '%s': truncated or invalid .rlm file\n", name);
        return 0;
    }

    model.lodInfo[0].dist = 0.0f; // never switched out
    model.lodInfo[0].numsurfs = (unsigned __int16)surfCount;
    model.lodInfo[0].surfIndex = 0;
    model.numLods = 1;
    model.collLod = -1;
    model.boneInfo = (XBoneInfo *)RtAlloc(sizeof(XBoneInfo) * boneCount);
    for ( unsigned int b = 0; b < boneCount; ++b )
    {
        XBoneInfo *bi = &model.boneInfo[b];
        if ( boneMins[3 * b] > boneMaxs[3 * b] )
            continue; // no vertices: an empty box at the bone
        float radiusSq = 0.0f;
        for ( int k = 0; k < 3; ++k )
        {
            bi->bounds[0][k] = boneMins[3 * b + k];
            bi->bounds[1][k] = boneMaxs[3 * b + k];
            bi->offset[k] = 0.5f * (boneMins[3 * b + k] + boneMaxs[3 * b + k]);
            const float half = 0.5f * (boneMaxs[3 * b + k] - boneMins[3 * b + k]);
            radiusSq += half * half;
        }
        bi->radiusSquared = radiusSq;
    }
    float radiusSq = 0.0f;
    for ( int k = 0; k < 3; ++k )
    {
        model.mins[k] = mins[k];
        model.maxs[k] = maxs[k];
        const float half = 0.5f * (maxs[k] - mins[k]);
        radiusSq += half * half;
    }
    model.radius = sqrtf(radiusSq);
    model.streamInfo.highMipBounds = (XModelHighMipBounds *)RtAlloc(sizeof(XModelHighMipBounds) * surfCount);
    const XModel *linked = RtLink(ASSET_TYPE_XMODEL, (XAssetHeader)&model, zoneIndex).model;
    // the upper body: j_spinelower and everything under it (the aim poses' bones; the legs keep the cycle)
    unsigned char *upperBody = (unsigned char *)RtAlloc((int)boneCount);
    for ( unsigned int b = 0; b < boneCount; ++b )
    {
        for ( int a = (int)b; a >= 0 && !upperBody[b]; a = parent[a] )
            upperBody[b] = boneName[a] == "j_spinelower";
    }
    // the head: j_neck and everything under it
    unsigned char *headSide = (unsigned char *)RtAlloc((int)boneCount);
    for ( unsigned int b = 0; b < boneCount; ++b )
    {
        for ( int a = (int)b; a >= 0 && !headSide[b]; a = parent[a] )
            headSide[b] = boneName[a] == "j_neck";
    }
    int spineBone = -1, neckBone = -1, eyelidBone = -1, pelvisBone = -1;
    for ( unsigned int b = 0; b < boneCount; ++b )
    {
        if ( boneName[b] == "j_mainroot" )
            pelvisBone = (int)b;
        if ( boneName[b] == "j_spinelower" )
            spineBone = (int)b;
        if ( boneName[b] == "j_neck" )
            neckBone = (int)b;
        if ( boneName[b] == "j_re4_028" )
            eyelidBone = (int)b;
    }
    for ( RtAnim &anim : anims )
    {
        anim.model = linked;
        anim.upperBody = upperBody;
        anim.headSide = headSide;
        anim.spineBone = spineBone;
        anim.neckBone = neckBone;
        anim.eyelidBone = eyelidBone;
        anim.neckChainCount = 0;
        if ( neckBone > 0 )
        {
            int chain[8], count = 0;
            for ( int a = parent[neckBone]; a > 0 && count < 8; a = parent[a] )
                chain[count++] = a;
            for ( int k = 0; k < count; ++k )
                anim.neckChain[k] = chain[count - 1 - k];
            anim.neckChainCount = count;
        }
        // a shot's kick peak: in its first 12 frames (0.4 s), the frame whose upper body is turned most from the first
        anim.recoilPeak = 0;
        const char *colon = strchr(anim.name, ':');
        if ( !strcmp(colon ? colon + 1 : anim.name, "run") && pelvisBone > 0 && spineBone > 0 && parent[spineBone] == pelvisBone )
        {
            anim.runBob = RtSoftenRunBob(const_cast<float *>(anim.frames), anim.frameCount, anim.boneCount, pelvisBone,
                                         spineBone);
            if ( R_RuntimeXModel_DvarOn("leon_actionlog") ) // (tests)
                Com_Printf(0, "runtime model: '%s' bob softened\n", anim.name);
        }
        if ( !strncmp(colon ? colon + 1 : anim.name, "fire", 4) && anim.frameCount > 1 )
        {
            float most = 0.0f;
            for ( int f = 1; f < anim.frameCount && f < 12; ++f )
            {
                float turned = 0.0f;
                for ( int b = 0; b < anim.boneCount; ++b )
                {
                    if ( !upperBody[b] )
                        continue;
                    const float *q0 = anim.frames + (size_t)b * 7, *q = anim.frames + ((size_t)f * anim.boneCount + b) * 7;
                    turned += 1.0f - fabsf(q0[0] * q[0] + q0[1] * q[1] + q0[2] * q[2] + q0[3] * q[3]);
                }
                if ( turned > most )
                {
                    most = turned;
                    anim.recoilPeak = f;
                }
            }
            if ( R_RuntimeXModel_DvarOn("leon_actionlog") ) // (tests)
                Com_Printf(0, "runtime model: '%s' kick peaks at frame %d of %d\n", anim.name, anim.recoilPeak,
                           anim.frameCount);
        }
        // a reload's raise back to the aim: in its last half, the frame whose upper body is turned farthest from its
        // last (RE4 reloads from its aim and ends raising the gun back up to it)
        anim.reloadRaise = 0;
        if ( !strcmp(colon ? colon + 1 : anim.name, "reload") && anim.frameCount > 4 )
        {
            const float *last = anim.frames + (size_t)(anim.frameCount - 1) * anim.boneCount * 7;
            float most = -1.0f;
            for ( int f = anim.frameCount / 2; f < anim.frameCount - 1; ++f )
            {
                float turned = 0.0f;
                for ( int b = 0; b < anim.boneCount; ++b )
                {
                    if ( !upperBody[b] )
                        continue;
                    const float *q0 = last + (size_t)b * 7, *q = anim.frames + ((size_t)f * anim.boneCount + b) * 7;
                    turned += 1.0f - fabsf(q0[0] * q[0] + q0[1] * q[1] + q0[2] * q[2] + q0[3] * q[3]);
                }
                if ( turned > most )
                {
                    most = turned;
                    anim.reloadRaise = f;
                }
            }
            // (farthest at the half's very start: no raise of its own found - it moves toward its end all through the
            // half, the magazine still going in there (the Killer7's at 44 of 55) - played whole, as before)
            if ( anim.reloadRaise <= anim.frameCount / 2 + 1 )
                anim.reloadRaise = 0;
            if ( R_RuntimeXModel_DvarOn("leon_actionlog") ) // (tests)
                Com_Printf(0, "runtime model: '%s' raises back to the aim from frame %d of %d\n", anim.name,
                           anim.reloadRaise, anim.frameCount);
        }
        s_anims.push_back(anim);
    }
    Com_Printf(0, "runtime model '%s': %u surfaces, %d vertices, %d triangles, %u textures, %u bones, %u animations"
        " (level zone %d)\n", name, surfCount, baseVert, baseTri, texCount, boneCount, (unsigned int)anims.size(), zoneIndex);
    return linked;
}

// false: no rtmodels/<name>.rlm
static bool RtLoadModel(const char *name, int zoneIndex, const Material *templ, const Material *atestTempl, bool gpu)
{
    void *file = 0;
    int size = FS_ReadFile(va("rtmodels/%s.rlm", name), &file);
    if ( size <= 0 || !file )
    {
        Com_PrintWarning(0, "runtime model '%s': rtmodels/%s.rlm not found\n", name, name);
        return false;
    }
    RtBuildModel(name, file, size, zoneIndex, templ, atestTempl, gpu);
    FS_FreeFile(file);
    return true;
}

// ---- the laser sight's models (the over-the-shoulder aim, cg_view_mp.cpp) ---------------------------------------
// Made here, not converted: a red beam segment (16 units along +X, two crossed thin quads, both faces) and a dot
// (three crossed quads), on an unlit effect material, so the laser glows in the dark as RE4's does.
static const char *const s_effectTechsets[] = { "mc_effect", "mc_effect_zfeather", "mc_unlit_alphatest" };

static void RtPut(std::vector<unsigned char> &out, const void *p, size_t n)
{
    out.insert(out.end(), (const unsigned char *)p, (const unsigned char *)p + n);
}

static void RtPutU32(std::vector<unsigned char> &out, unsigned int v)
{
    RtPut(out, &v, 4);
}

enum RtGlowKind
{
    RT_GLOW_BEAM, // the laser's beam
    RT_GLOW_DOT,  // the laser's dot
    RT_GLOW_HALO, // the kick's halo: a dull yellow, soft all the way, no core
};

// the laser's glow at texture coordinates (u, v) in 0..1: radial (the dot: a white-hot core, a red bloom fading
// out) or across v (the beam: bright along its middle, soft at its edges), red or blue (the wonder weapons'); the halo's
// dull yellow; BGRA
static void RtLaserTexel(RtGlowKind kind, bool blue, float u, float v, unsigned char *bgra)
{
    if ( kind == RT_GLOW_HALO )
    {
        float d = sqrtf((u - 0.5f) * (u - 0.5f) + (v - 0.5f) * (v - 0.5f)) * 2.0f;
        d = d > 1.0f ? 1.0f : d;
        const float glow = (1.0f - d) * sqrtf(1.0f - d); // soft out to its edge
        bgra[0] = 50;  // B
        bgra[1] = 200; // G
        bgra[2] = 255; // R
        bgra[3] = (unsigned char)(255.0f * 0.4f * glow);
        return;
    }
    const bool radial = kind == RT_GLOW_DOT;
    float d = radial ? sqrtf((u - 0.5f) * (u - 0.5f) + (v - 0.5f) * (v - 0.5f)) * 2.0f : fabsf(v - 0.5f) * 2.0f;
    d = d > 1.0f ? 1.0f : d;
    const float core = radial ? (d < 0.18f ? 1.0f - d / 0.18f : 0.0f) : (d < 0.35f ? 1.0f - d / 0.35f : 0.0f);
    float glow = 1.0f - d;
    glow = radial ? glow * glow * glow : glow * glow; // the bloom falls off fast
    const float a = core > glow ? core : glow;
    const float hot = radial ? 200.0f : 20.0f; // the dot's core white-hot; the beam keeps its colour (thinned far out)
    const float red = blue ? 40.0f : 255.0f, green = blue ? 110.0f : 40.0f, bluish = blue ? 255.0f : 40.0f;
    const float add = hot * core;
    bgra[0] = (unsigned char)(bluish + add < 255.0f ? bluish + add : 255.0f); // B
    bgra[1] = (unsigned char)(green + add < 255.0f ? green + add : 255.0f);   // G
    bgra[2] = (unsigned char)(red + add < 255.0f ? red + add : 255.0f);       // R
    bgra[3] = (unsigned char)(255.0f * a * (radial ? 1.0f : 0.15f)); // the beam dull (at a quarter it glared), the dot bright
}

// a version 1 .rlm with one glow texture (the kind's, red or blue) and one surface of quads (4 corners each, drawn
// both ways)
static std::vector<unsigned char> RtMakeQuadModel(const float (*corners)[3], int quadCount, RtGlowKind kind,
                                                  bool blue = false)
{
    std::vector<unsigned char> out;
    RtPut(out, "RLM1", 4);
    RtPutU32(out, 1);
    RtPutU32(out, 1);
    RtPutU32(out, 1);
    char texName[32] = {};
    strcpy(texName, kind == RT_GLOW_DOT ? "laser_dot" : (kind == RT_GLOW_HALO ? "kick_halo" : "laser_beam"));
    if ( blue )
        strcat(texName, "_blue");
    RtPut(out, texName, 32);
    const unsigned short size = 32;
    RtPut(out, &size, 2);
    RtPut(out, &size, 2);
    const unsigned char mips = 6, masked = 0;
    RtPut(out, &mips, 1);
    RtPut(out, &masked, 1);
    const unsigned short pad = 0;
    RtPut(out, &pad, 2);
    for ( int m = 0, w = 32; m < 6; ++m, w /= 2 )
    {
        for ( int y = 0; y < w; ++y )
        {
            for ( int x = 0; x < w; ++x )
            {
                unsigned char bgra[4];
                RtLaserTexel(kind, blue, ((float)x + 0.5f) / (float)w, ((float)y + 0.5f) / (float)w, bgra);
                RtPut(out, bgra, 4);
            }
        }
    }
    RtPutU32(out, 0);
    RtPutU32(out, (unsigned int)quadCount * 4);
    RtPutU32(out, (unsigned int)quadCount * 4);
    for ( int q = 0; q < quadCount; ++q )
    {
        const float *c = corners[4 * q];
        float e1[3], e2[3], n[3];
        for ( int k = 0; k < 3; ++k )
        {
            e1[k] = corners[4 * q + 1][k] - c[k];
            e2[k] = corners[4 * q + 3][k] - c[k];
        }
        n[0] = e1[1] * e2[2] - e1[2] * e2[1];
        n[1] = e1[2] * e2[0] - e1[0] * e2[2];
        n[2] = e1[0] * e2[1] - e1[1] * e2[0];
        const float len = sqrtf(n[0] * n[0] + n[1] * n[1] + n[2] * n[2]);
        for ( int k = 0; k < 3; ++k )
            n[k] = len > 0.0f ? n[k] / len : 0.0f;
        const float uv[4][2] = { { 0, 0 }, { 1, 0 }, { 1, 1 }, { 0, 1 } };
        for ( int v = 0; v < 4; ++v )
        {
            RtPut(out, corners[4 * q + v], 12);
            RtPut(out, n, 12);
            RtPut(out, uv[v], 8);
        }
    }
    for ( int q = 0; q < quadCount; ++q )
    {
        const unsigned short b = (unsigned short)(4 * q);
        const unsigned short tris[12] = { b, (unsigned short)(b + 1), (unsigned short)(b + 2),
                                          b, (unsigned short)(b + 2), (unsigned short)(b + 3),
                                          b, (unsigned short)(b + 2), (unsigned short)(b + 1),
                                          b, (unsigned short)(b + 3), (unsigned short)(b + 2) };
        RtPut(out, tris, sizeof(tris));
    }
    while ( out.size() % 4 )
        out.push_back(0);
    return out;
}

static void RtBuildLaserModels(int zoneIndex, bool gpu)
{
    memset(s_laserModels, 0, sizeof(s_laserModels));
    memset(s_laserPieces, 0, sizeof(s_laserPieces));
    s_haloModel = 0;
    const Material *effect = RtFindTemplateMaterial(s_effectTechsets, ARRAYSIZE(s_effectTechsets));
    if ( !effect )
    {
        Com_PrintWarning(0, "runtime models: no unlit effect material; no laser sight\n");
        return;
    }
    // the beam's glow is soft at its edges; the dot is one quad facing +X (CG_ShoulderLaser turns it to the camera
    // and scales it, as RE4's drawPoint), its radial glow round with a bloom
    const float w = 0.3f, L = 16.0f, d = 2.0f;
    const float beam[4][4][3] = {
        { { 0, -w, 0 }, { L, -w, 0 }, { L, w, 0 }, { 0, w, 0 } },
        { { 0, 0, -w }, { L, 0, -w }, { L, 0, w }, { 0, 0, w } },
        { { 0, -w * 0.5f, -w * 0.5f }, { L, -w * 0.5f, -w * 0.5f }, { L, w * 0.5f, w * 0.5f }, { 0, w * 0.5f, w * 0.5f } },
        { { 0, -w * 0.5f, w * 0.5f }, { L, -w * 0.5f, w * 0.5f }, { L, w * 0.5f, -w * 0.5f }, { 0, w * 0.5f, -w * 0.5f } } };
    const float dot[1][4][3] = { { { 0, -d, -d }, { 0, d, -d }, { 0, d, d }, { 0, -d, d } } };
    std::vector<unsigned char> data;
    // each in red and in blue (the wonder weapons': CG_DrawLaser)
    for ( int c = 0; c < 2; ++c )
    {
        const bool blue = c == 1;
        const char *const color = blue ? "_blue" : "";
        char name[32]; // (not va's: RtBuildModel's own va calls would write over it)
        sprintf_s(name, "leon_laser%s", color);
        data = RtMakeQuadModel(&beam[0][0], 4, RT_GLOW_BEAM, blue);
        s_laserModels[c][0] = RtBuildModel(name, data.data(), (int)data.size(), zoneIndex, effect, 0, gpu);
        // the beam's shorter pieces (halving: 8, 4, 2, 1), to end it at the dot
        for ( int p = 0; p < 4; ++p )
        {
            const float l = L / (float)(2 << p);
            const float piece[4][4][3] = {
                { { 0, -w, 0 }, { l, -w, 0 }, { l, w, 0 }, { 0, w, 0 } },
                { { 0, 0, -w }, { l, 0, -w }, { l, 0, w }, { 0, 0, w } },
                { { 0, -w * 0.5f, -w * 0.5f }, { l, -w * 0.5f, -w * 0.5f }, { l, w * 0.5f, w * 0.5f }, { 0, w * 0.5f, w * 0.5f } },
                { { 0, -w * 0.5f, w * 0.5f }, { l, -w * 0.5f, w * 0.5f }, { l, w * 0.5f, -w * 0.5f }, { 0, w * 0.5f, -w * 0.5f } } };
            data = RtMakeQuadModel(&piece[0][0], 4, RT_GLOW_BEAM, blue);
            sprintf_s(name, "leon_laser%s_%d", color, 8 >> p);
            s_laserPieces[c][p] = RtBuildModel(name, data.data(), (int)data.size(), zoneIndex, effect, 0, gpu);
        }
        data = RtMakeQuadModel(&dot[0][0], 1, RT_GLOW_DOT, blue);
        sprintf_s(name, "leon_laser%s_dot", color);
        s_laserModels[c][1] = RtBuildModel(name, data.data(), (int)data.size(), zoneIndex, effect, 0, gpu);
    }
    // the kick's halo: one quad facing +X, 20 units across (CG_LeonKickHalos turns it to the camera and scales it)
    const float h = 10.0f;
    const float halo[1][4][3] = { { { 0, -h, -h }, { 0, h, -h }, { 0, h, h }, { 0, -h, h } } };
    data = RtMakeQuadModel(&halo[0][0], 1, RT_GLOW_HALO);
    s_haloModel = RtBuildModel("leon_kick_halo", data.data(), (int)data.size(), zoneIndex, effect, 0, gpu);
    Com_Printf(0, "runtime models: laser sight on '%s' (%s)\n", effect->info.name, effect->techniqueSet->name);
}

// ---- RE4's HUD's images (cgame_mp/cg_re4hud_mp.cpp) -----------------------------------------------------------------
// rtmodels/re4hud.rlh (tools/leon-converter/re4_hud.py, from the player's RE4 at setup): each image a 2D material, a copy
// of the HUD's own "white" (its 2D technique, blended by the colour's alpha) with the image as its colour map. Made
// once per level zone, as the laser's models (linked into it again, they asserted): one there already is taken as it is.
static void RtLoadHud(int zoneIndex, bool gpu)
{
    s_hudMaterials.clear();
    void *file = 0;
    const int size = FS_ReadFile("rtmodels/re4hud.rlh", &file);
    if ( size <= 0 || !file )
        return; // (not converted: BO1's own ammo counter stays)
    const Material *templ = DB_FindXAssetEntry(ASSET_TYPE_MATERIAL, "white")
        ? DB_FindXAssetHeader(ASSET_TYPE_MATERIAL, (char *)"white", 1, -1).material : 0;
    const unsigned int colorMapHash = R_HashString("colorMap");
    bool colorMap = false;
    for ( int i = 0; templ && i < templ->textureCount; ++i )
        colorMap |= templ->textureTable[i].nameHash == colorMapHash;
    if ( !colorMap )
    {
        Com_PrintWarning(0, "runtime models: no 2D material to draw RE4's HUD with\n");
        FS_FreeFile(file);
        return;
    }
    RtReader r = { (const unsigned __int8 *)file, (const unsigned __int8 *)file, (const unsigned __int8 *)file + size };
    const void *magic = r.take(4);
    const unsigned int count = magic && !memcmp(magic, "RLH1", 4) ? r.u32() : 0;
    for ( unsigned int i = 0; i < count && r.ok; ++i )
    {
        RtHudMaterial hud;
        memset(hud.name, 0, sizeof(hud.name));
        if ( r.end - r.p >= 32 )
            memcpy(hud.name, r.p, 31); // (the record's name, read again by RtCreateImage)
        const char *materialName = va("re4hud_%s", hud.name);
        const XAssetEntryPoolEntry *made = DB_FindXAssetEntry(ASSET_TYPE_MATERIAL, materialName);
        if ( made && made->entry.zoneIndex == zoneIndex )
        {
            // (this zone has it: its record skipped)
            r.take(32);
            const unsigned int width = r.u16(), height = r.u16(), mips = r.u8();
            r.u8();
            r.u16();
            size_t bytes = 0;
            for ( unsigned int m = 0; m < mips; ++m )
                bytes += (size_t)((width >> m) ? (width >> m) : 1) * ((height >> m) ? (height >> m) : 1) * 4;
            r.take(bytes);
            hud.material = DB_FindXAssetHeader(ASSET_TYPE_MATERIAL, (char *)materialName, 1, -1).material;
        }
        else
        {
            bool masked;
            GfxImage *image = RtCreateImage(r, "re4hud", zoneIndex, gpu, &masked);
            if ( !image )
                break;
            hud.material = RtCreateMaterial(templ, image, materialName, zoneIndex);
        }
        if ( r.ok && hud.material )
            s_hudMaterials.push_back(hud);
    }
    FS_FreeFile(file);
    Com_Printf(0, "runtime models: RE4's HUD, %u images, on '%s' (%s)\n", (unsigned int)s_hudMaterials.size(),
               templ->info.name, templ->techniqueSet->name);
}

// RE4's HUD's image `name` as a 2D material (re4_hud.py's names: frame, digit0..9, empty, disc, name_<character>), or 0
Material *__cdecl R_RuntimeXModel_HudMaterial(const char *name)
{
    for ( const RtHudMaterial &hud : s_hudMaterials )
    {
        if ( !strcmp(hud.name, name) )
            return hud.material;
    }
    return 0;
}

// the laser sight's beam segment (16 units along +X) or dot, red or blue, or 0 (no runtime models, or no effect
// material)
const XModel *__cdecl R_RuntimeXModel_Laser(bool dot, bool blue)
{
    return s_laserModels[blue ? 1 : 0][dot ? 1 : 0];
}

// the beam's shorter pieces: 0..3, 8 / 4 / 2 / 1 units along +X, red or blue, or 0
const XModel *__cdecl R_RuntimeXModel_LaserPiece(int piece, bool blue)
{
    return piece >= 0 && piece < 4 ? s_laserPieces[blue ? 1 : 0][piece] : 0;
}

// the kick's halo on a staggered zombie (a dull yellow glow quad facing +X, 20 units across), or 0
const XModel *__cdecl R_RuntimeXModel_Halo()
{
    return s_haloModel;
}

bool __cdecl R_RuntimeXModel_DvarOn(const char *name)
{
    const dvar_s *dvar = Dvar_FindVar(name);
    if ( !dvar )
        return false;
    if ( dvar->type == DVAR_TYPE_BOOL )
        return dvar->current.enabled;
    if ( dvar->type == DVAR_TYPE_STRING )
        return dvar->current.string && atoi(dvar->current.string) != 0;
    return dvar->current.integer != 0;
}

void __cdecl R_RuntimeXModel_SetLookYaw(float degrees)
{
    s_lookYaw = degrees;
}

bool __cdecl R_RuntimeXModel_HasAnim(const char *name)
{
    for ( const RtAnim &anim : s_anims )
        if ( !strcmp(anim.name, name) )
            return true;
    return false;
}

bool __cdecl R_RuntimeXModel_RootYaw(const char *name, float time, float *yaw, float *duration)
{
    for ( const RtAnim &anim : s_anims )
    {
        if ( strcmp(anim.name, name) || anim.frameCount < 2 || anim.fps <= 0.0f )
            continue;
        *duration = (float)(anim.frameCount - 1) / anim.fps;
        float phase = time * anim.fps;
        phase = phase < 0.0f ? 0.0f : (phase > (float)(anim.frameCount - 1) ? (float)(anim.frameCount - 1) : phase);
        const int f0 = (int)phase, f1 = f0 + 1 < anim.frameCount ? f0 + 1 : f0;
        // bone 0's rotation (about up): the frame's yaw, unwrapped toward the previous one across +-180
        const float *q0 = anim.frames + (size_t)f0 * anim.boneCount * 7, *q1 = anim.frames + (size_t)f1 * anim.boneCount * 7;
        const float y0 = 2.0f * atan2f(q0[2], q0[3]) * 57.29578f;
        float y1 = 2.0f * atan2f(q1[2], q1[3]) * 57.29578f;
        y1 = y0 + AngleNormalize180(y1 - y0);
        *yaw = y0 + (y1 - y0) * (phase - (float)f0);
        return true;
    }
    return false;
}

// bo1_rtmodels_loaded: the names of bo1_rtmodels built in the level's zone (one without its .rlm is left out)
static void RtPublishLoaded(int zoneIndex)
{
    char names[1024], loaded[1024] = "";
    I_strncpyz(names, bo1_rtmodels->current.string, sizeof(names));
    for ( char *tok = strtok(names, " ,"); tok; tok = strtok(0, " ,") )
    {
        const XAssetEntryPoolEntry *e = DB_FindXAssetEntry(ASSET_TYPE_XMODEL, tok);
        if ( !e || e->entry.zoneIndex != zoneIndex )
            continue;
        if ( loaded[0] )
            I_strncat(loaded, sizeof(loaded), " ");
        I_strncat(loaded, sizeof(loaded), tok);
    }
    Dvar_SetString((dvar_s *)bo1_rtmodels_loaded, loaded);
}

// Called by CL_InitCGame once the level's fastfiles are queued, before CG_Init registers the models.
static void RtLoadForLevel(const char *mapName)
{
    R_RuntimeXModel_RegisterDvars();
    Dvar_SetString((dvar_s *)bo1_rtmodels_loaded, "");
    if ( !bo1_rtmodels->current.string[0] || IsDedicatedServer() )
        return;
    DB_SyncXAssets();
    const int zoneIndex = RtLevelZoneIndex(mapName);
    if ( !zoneIndex )
    {
        Com_PrintWarning(0, "runtime models: no level zone '%s'\n", mapName);
        return;
    }

    char names[1024];
    I_strncpyz(names, bo1_rtmodels->current.string, sizeof(names));
    // already built for this level (a restart keeps the zone; the client's load follows the server's)? Each one in the
    // zone, or with no file to build it from: one missing was left out the first time too - counted as unbuilt, the
    // client's load built the others again, their assets linked into the zone twice (DB_LinkXAssetEntry's assert)
    bool built = true;
    for ( char *tok = strtok(names, " ,"); tok; tok = strtok(0, " ,") )
    {
        XAssetEntryPoolEntry *e = DB_FindXAssetEntry(ASSET_TYPE_XMODEL, tok);
        if ( e && e->entry.zoneIndex == zoneIndex )
            continue;
        int file;
        if ( FS_FOpenFileRead(va("rtmodels/%s.rlm", tok), &file) >= 0 )
        {
            FS_FCloseFile(file);
            built = false;
        }
    }
    if ( built )
    {
        RtPublishLoaded(zoneIndex);
        return;
    }

    // a character's cloth material first (the zombies' gear: the characters' own lit shader, matte where a prop's is
    // shiny), else a plain lit prop's
    static const char *const s_clothTechsets[] = { "mc_l_sm_r0c0n0s0_clothcharr_hero", "mc_l_sm_r0c0n0s0_clothcharr" };
    static const char *const s_clothPlainTechsets[] = { "mc_l_sm_r0c0n0s0_clothcharr", "mc_l_sm_r0c0n0s0_clothcharr_hero" };
    const bool plainTest = R_RuntimeXModel_DvarOn("leon_clothplain"); // tests: the cloth shader without its hero lighting
    const bool propTest = R_RuntimeXModel_DvarOn("leon_matprop"); // tests: a plain lit prop's shader, no cloth sheen
    const Material *templ = propTest ? 0 : RtFindTemplateMaterial(plainTest ? s_clothPlainTechsets : s_clothTechsets,
                                                                  ARRAYSIZE(s_clothTechsets));
    if ( !templ )
        templ = RtFindTemplateMaterial(s_opaqueTechsets, ARRAYSIZE(s_opaqueTechsets));
    if ( !templ )
        templ = RtTemplateMaterial(); // a weapon's: lit, but with its reflection and colour detail
    if ( !templ )
    {
        Com_PrintWarning(0, "runtime models: no template material on this level\n");
        return;
    }
    const Material *atestTempl = RtFindTemplateMaterial(s_alphaTestTechsets, ARRAYSIZE(s_alphaTestTechsets));
    Com_Printf(0, "runtime models: templates '%s' (%s), alpha-tested '%s' (%s)\n", templ->info.name,
        templ->techniqueSet->name, atestTempl ? atestTempl->info.name : "none: masked textures draw opaque",
        atestTempl ? atestTempl->techniqueSet->name : "-");
    if ( R_RuntimeXModel_DvarOn("leon_matlog") ) // (tests: what the template brings besides the colour map)
    {
        for ( int i = 0; i < templ->textureCount; ++i )
        {
            const MaterialTextureDef &t = templ->textureTable[i];
            Com_Printf(0, "runtime models: template texture %c..%c hash %08x semantic %d image %s\n", t.nameStart,
                       t.nameEnd, t.nameHash, t.semantic, t.semantic != 1 && t.u.image ? t.u.image->name : "-");
        }
        for ( int i = 0; i < templ->constantCount; ++i )
        {
            const MaterialConstantDef &c = templ->constantTable[i];
            Com_Printf(0, "runtime models: template constant %.12s %g %g %g %g\n", c.name, c.literal[0], c.literal[1],
                       c.literal[2], c.literal[3]);
        }
    }
    RtFreeAll(); // the previous level's arrays; its zone (and the GPU side) is gone
    const bool gpu = !r_loadForRenderer || r_loadForRenderer->current.enabled;
    I_strncpyz(names, bo1_rtmodels->current.string, sizeof(names));
    bool loaded = false;
    for ( char *tok = strtok(names, " ,"); tok; tok = strtok(0, " ,") )
        loaded |= RtLoadModel(tok, zoneIndex, templ, atestTempl, gpu);
    // the laser sight only with a model to hold it, and once per level zone: linking its assets into the zone again
    // asserted in DB_LinkXAssetEntry (0.1.1's crash, after a trip to the menu had turned the mod off - fs_game - so
    // the model was not found and only the lasers were made again)
    const XAssetEntryPoolEntry *laser = DB_FindXAssetEntry(ASSET_TYPE_XMODEL, "leon_laser");
    if ( loaded && !(laser && laser->entry.zoneIndex == zoneIndex) )
        RtBuildLaserModels(zoneIndex, gpu);
    RtLoadHud(zoneIndex, gpu);
    RtPublishLoaded(zoneIndex);
    if ( gpu )
        RB_Resource_Flush();
}

void __cdecl R_RuntimeXModel_LoadForLevel(const char *mapName)
{
    // The characters are built with C++ containers (RtAlloc's list, the motions, a surface's vertex groups) in a
    // 32-bit process: an allocation that fails throws, and uncaught it ended the process from the C runtime with no
    // word in the log (players' crashes in vcruntime140 / ucrtbase, 0xC0000409, as a map started). The level's load
    // drops to the menu with the reason instead.
    try
    {
        RtLoadForLevel(mapName);
    }
    catch ( const std::exception &e )
    {
        RtFreeAll();
        Com_Error(ERR_DROP, "The characters could not be built for %s: %s (out of memory?)", mapName, e.what());
    }
}

// the model's animation `name` (0: its first), an s_anims index or -1
static int RtFindAnim(const XModel *model, const char *name)
{
    for ( int i = 0; i < (int)s_anims.size(); ++i )
    {
        if ( s_anims[i].model == model && (!name || !strcmp(s_anims[i].name, name)) )
            return i;
    }
    return -1;
}

// A motion of a set (the converter's weapon sets: "<set>:<name>", e.g. "shotgun:aim"), or the default set's
// (unprefixed: the handgun's) when the set has none or no set is given.
static int RtFindSetAnim(const XModel *model, const char *set, const char *name)
{
    if ( set && *set )
    {
        char full[64];
        Com_sprintf(full, sizeof(full), "%s:%s", set, name);
        const int index = RtFindAnim(model, full);
        if ( index >= 0 )
            return index;
    }
    return RtFindAnim(model, name);
}

// a motion's name without its set
static const char *RtAnimBaseName(const RtAnim &anim)
{
    const char *colon = strchr(anim.name, ':');
    return colon ? colon + 1 : anim.name;
}

// R_AddDObjToScene: a one-model DObj without an XAnim tree normally draws as a scene model in the model's base
// pose; one with a baked animation takes the posed (scene DObj) path, which calls R_RuntimeXModel_PoseDObj.
bool __cdecl R_RuntimeXModel_IsAnimated(const XModel *model)
{
    return !s_anims.empty() && RtFindAnim(model, 0) >= 0;
}

static float RtWrapPhase(float phase, int frameCount)
{
    phase = fmodf(phase, (float)frameCount);
    return phase < 0.0f ? phase + (float)frameCount : phase;
}

// bone b of a cycle at `phase` (frames), the neighbouring frames blended
static void RtSampleBone(const RtAnim &anim, float phase, int b, float *quat, float *trans)
{
    int f0 = (int)phase;
    if ( f0 >= anim.frameCount )
        f0 = anim.frameCount - 1;
    const int f1 = (f0 + 1) % anim.frameCount;
    const float frac = phase - (float)f0;
    const float *a = &anim.frames[7 * (f0 * anim.boneCount + b)];
    const float *c = &anim.frames[7 * (f1 * anim.boneCount + b)];
    const float sign = a[0] * c[0] + a[1] * c[1] + a[2] * c[2] + a[3] * c[3] < 0.0f ? -1.0f : 1.0f;
    for ( int k = 0; k < 4; ++k )
        quat[k] = a[k] + (sign * c[k] - a[k]) * frac;
    for ( int k = 0; k < 3; ++k )
        trans[k] = a[4 + k] + (c[4 + k] - a[4 + k]) * frac;
}

// out = a * b (Hamilton; x, y, z, w): b turned by a
// RE4's blink (game/pl_class.cpp moveEyeNormal): the eyelid (part 0x1C) turned about its X axis - BO1's Y, positive
// closing - by a table, a value a 30 Hz frame: a blink every 89 frames (~3 s), 7 frames long, one in four the long one
// (9, closing again half way). Degrees at timeMs, eased between RE4's frames; each entity on its own phase.
static float RtBlinkAngle(int timeMs, int entnum)
{
    static const float s_short[7] = { 5.0f, 10.0f, 20.0f, 18.0f, 14.0f, 10.0f, 5.0f };
    static const float s_long[9] = { 5.0f, 10.0f, 20.0f, 17.0f, 19.0f, 20.0f, 15.0f, 10.0f, 5.0f };
    const unsigned int seed = (unsigned int)entnum * 2654435761u;
    const float frame = (float)(timeMs < 0 ? 0 : timeMs) * 0.03f + (float)(seed % 89u);
    const int cycle = (int)(frame / 89.0f);
    const float in = frame - (float)cycle * 89.0f;
    const bool longBlink = ((((unsigned int)cycle * 2246822519u) ^ seed) >> 13) % 4u == 0;
    const float *table = longBlink ? s_long : s_short;
    const int count = longBlink ? 9 : 7;
    // (frame i of the blink: the table's i - 1, open at 0 and after the last)
    const int f0 = (int)in;
    if ( f0 > count )
        return 0.0f;
    const float v0 = f0 >= 1 && f0 <= count ? table[f0 - 1] : 0.0f;
    const float v1 = f0 + 1 <= count ? table[f0] : 0.0f;
    return v0 + (v1 - v0) * (in - (float)f0);
}

static void RtQuatMul(const float *a, const float *b, float *out)
{
    out[0] = a[3] * b[0] + a[0] * b[3] + a[1] * b[2] - a[2] * b[1];
    out[1] = a[3] * b[1] - a[0] * b[2] + a[1] * b[3] + a[2] * b[0];
    out[2] = a[3] * b[2] + a[0] * b[1] - a[1] * b[0] + a[2] * b[3];
    out[3] = a[3] * b[3] - a[0] * b[0] - a[1] * b[1] - a[2] * b[2];
}

// a = a + (b - a) * t for a bone pose: the rotation by the shorter way (normalised later), the translation linearly
static void RtBlendPose(float *quat, float *trans, const float *bq, const float *bt, float t)
{
    const float sign = quat[0] * bq[0] + quat[1] * bq[1] + quat[2] * bq[2] + quat[3] * bq[3] < 0.0f ? -1.0f : 1.0f;
    float lenSq = 0.0f;
    for ( int k = 0; k < 4; ++k )
    {
        quat[k] += (sign * bq[k] - quat[k]) * t;
        lenSq += quat[k] * quat[k];
    }
    const float inv = lenSq > 0.0f ? 1.0f / sqrtf(lenSq) : 0.0f;
    for ( int k = 0; k < 4; ++k )
        quat[k] *= inv;
    for ( int k = 0; k < 3; ++k )
        trans[k] += (bt[k] - trans[k]) * t;
}

// playback rate of a cycle at a ground speed: a moving cycle's feet keep pace with the entity
static float RtCycleRate(const RtAnim &anim, float speed)
{
    if ( anim.moveSpeed <= 0.0f )
        return 1.0f;
    const float rate = speed / anim.moveSpeed;
    return rate < 0.5f ? 0.5f : (rate > 2.0f ? 2.0f : rate);
}

// The cycle for the entity's motion: idle when (nearly) still, back when moving against its facing, run past the
// geometric mean of the walk and run speeds, else walk; hysteresis keeps it from flickering at the thresholds.
// where a cycle's feet plant, as fractions of it (the frames each ankle is lowest in RE4's motions, the same in nearly
// every weapon's set: walk 0 / 15 of 28, run 0 / 11 of 21, the back step 3 / 19 of 21); 0 when not a cycle
// The frame of cycle `to` whose pose is nearest `from`'s at fromPhase (its bones' rotations, 24 frames tried): for a
// reversal between a step forward and a back step, whose feet plant at other points of their cycles - carried over by
// proportion, his legs swapped through the crossfade
static float RtMatchPhase(const RtAnim &from, float fromPhase, const RtAnim &to)
{
    const int bones = from.boneCount < to.boneCount ? from.boneCount : to.boneCount;
    float best = 0.0f, bestDist = 1e30f;
    for ( int k = 0; k < 24; ++k )
    {
        const float phase = (float)k / 24.0f * (float)to.frameCount;
        float dist = 0.0f;
        for ( int b = 1; b < bones; ++b ) // (not the root: his facing)
        {
            float qa[4], ta[3], qb[4], tb[3];
            RtSampleBone(from, fromPhase, b, qa, ta);
            RtSampleBone(to, phase, b, qb, tb);
            const float dot = qa[0] * qb[0] + qa[1] * qb[1] + qa[2] * qb[2] + qa[3] * qb[3];
            dist += 1.0f - fabsf(dot);
        }
        if ( dist < bestDist )
        {
            bestDist = dist;
            best = phase;
        }
    }
    return best;
}

static int RtFootPlants(const RtAnim &anim, float *plants)
{
    const char *name = RtAnimBaseName(anim);
    if ( !strcmp(name, "walk") )
    {
        plants[0] = 0.0f;
        plants[1] = 15.0f / 28.0f;
        return 2;
    }
    if ( !strcmp(name, "run") )
    {
        plants[0] = 0.0f;
        plants[1] = 11.0f / 21.0f;
        return 2;
    }
    if ( !strcmp(name, "back") )
    {
        plants[0] = 3.0f / 21.0f;
        plants[1] = 19.0f / 21.0f;
        return 2;
    }
    return 0;
}

static bool RtIsTurnInPlace(int anim)
{
    if ( anim < 0 )
        return false;
    const char *name = RtAnimBaseName(s_anims[anim]);
    return !strcmp(name, "turn_left") || !strcmp(name, "turn_right");
}

static int RtChooseCycle(const XModel *model, const RtAnimState &st, bool turnToMotion, const char *set)
{
    const int idle = RtFindSetAnim(model, set, "idle"), walk = RtFindSetAnim(model, set, "walk");
    const int run = RtFindSetAnim(model, set, "run"), back = RtFindSetAnim(model, set, "back");
    const int still = idle >= 0 ? idle : RtFindAnim(model, 0);
    const bool moving = st.cur >= 0 && strcmp(RtAnimBaseName(s_anims[st.cur]), "idle") && !RtIsTurnInPlace(st.cur);
    if ( st.speed < (moving ? 6.0f : 12.0f) || (walk < 0 && run < 0) )
    {
        // standing and turning (the player's, s_turnInPlace): RE4's turn in place, to that side; in past 45 degrees
        // a second, out under 20 held 0.12 s - a turn the other way passes through 0 and goes straight from one turn
        // to the other (through the idle, two crossfades and a restart hitched in the middle of it)
        if ( s_turnInPlace && RtIsTurnInPlace(st.cur) && fabsf(st.yawRate) <= 20.0f && st.turnStill < 0.12f )
            return st.cur;
        if ( s_turnInPlace && fabsf(st.yawRate) > (RtIsTurnInPlace(st.cur) ? 20.0f : 45.0f) )
        {
            const int turn = RtFindSetAnim(model, set, st.yawRate > 0.0f ? "turn_left" : "turn_right");
            if ( turn >= 0 )
                return turn;
        }
        return still;
    }
    // under tank controls (no sideways motion but an aimed strafe) the back step is past 110 degrees from the facing,
    // kept down to 45: a turn while stepping back leaves the measured motion trailing the facing, and must not flip it
    // to a walk; a strafe goes sideways on the walk (also as it slows down, the aim let go: at 90 it flickered)
    const bool sideways = turnToMotion || s_strafeLegs;
    const float backIn = turnToMotion ? 115.0f : 110.0f;
    const float backOut = sideways ? (turnToMotion ? 100.0f : 95.0f) : 45.0f;
    const bool backNow = st.cur >= 0 && !strcmp(RtAnimBaseName(s_anims[st.cur]), "back");
    // the player under tank controls, not strafing (s_stepWatch): the back step while he presses back, or still slows
    // out of one, never else - his measured motion trails a fast turn (a flick of the mouse while running read as a
    // step back and out), and reversing it passed through a walk and the idle on the way (run, walk, idle, back)
    if ( s_stepWatch && !s_strafeLegs )
    {
        if ( back >= 0 && (s_moveIntent < 0 || (s_moveIntent == 0 && backNow)) )
            return back;
    }
    else if ( back >= 0 && fabsf(st.moveRel) > (backNow ? backOut : backIn) )
        return back;
    if ( walk < 0 )
        return run;
    if ( run < 0 )
        return walk;
    // a player under RE4's controls: the run while his run key runs him, else the walk (his pace aside: by speed a
    // quick character's walk stayed past the cut below, and from a run to a walk he played the run slowed down)
    if ( s_running >= 0 )
        return s_running ? run : walk;
    const bool runNow = st.cur >= 0 && !strcmp(RtAnimBaseName(s_anims[st.cur]), "run");
    const float runCut = sqrtf(s_anims[walk].moveSpeed * s_anims[run].moveSpeed) * (runNow ? 0.9f : 1.1f);
    return st.speed > runCut ? run : walk;
}

// Advances the entity's locomotion state to timeMs (once per game time; the pose functions run several times a
// frame) and returns a copy.
static RtAnimState RtUpdateAnimState(const DObj *obj, const XModel *model, const float *origin, const float *angles,
                                     int timeMs, bool turnToMotion, const char *set)
{
    while ( _InterlockedCompareExchange(&s_animStateLock, 1, 0) )
        _mm_pause();
    RtAnimState &st = s_animStates[obj->entnum % RT_MAX_ANIM_ENTS];
    // a new entity (or a stale state) restarts; the same one's DObj rebuilt (a grenade attached: the player's dobjDirty)
    // keeps its motion going
    if ( st.cur < 0 || st.cur >= (int)s_anims.size() || s_anims[st.cur].model != model || timeMs < st.lastTime
         || (st.obj != obj && timeMs - st.lastTime > 500) )
    {
        memset(&st, 0, sizeof(st));
        st.obj = obj;
        st.lastTime = timeMs;
        for ( int k = 0; k < 3; ++k )
            st.lastOrigin[k] = origin[k];
        const int idle = RtFindSetAnim(model, set, "idle");
        st.cur = idle >= 0 ? idle : RtFindAnim(model, 0);
        st.prev = -1;
        st.blend = 1.0f;
        st.lastYaw = angles[1];
    }
    else if ( timeMs != st.lastTime )
    {
        const float dt = (float)(timeMs - st.lastTime) * 0.001f;
        if ( dt > 0.0f && dt <= 0.5f )
        {
            float vx = (origin[0] - st.lastOrigin[0]) / dt, vy = (origin[1] - st.lastOrigin[1]) / dt;
            if ( vx * vx + vy * vy > 2000.0f * 2000.0f ) // a teleport
                vx = vy = 0.0f;
            const float k = dt * 10.0f < 1.0f ? dt * 10.0f : 1.0f;
            st.vel[0] += (vx - st.vel[0]) * k;
            st.vel[1] += (vy - st.vel[1]) * k;
            st.speed = sqrtf(st.vel[0] * st.vel[0] + st.vel[1] * st.vel[1]);
            if ( st.speed > 1.0f )
                st.moveRel = AngleNormalize180(atan2f(st.vel[1], st.vel[0]) * 57.29578f - angles[1]);
            st.yawRate += (AngleNormalize180(angles[1] - st.lastYaw) / dt - st.yawRate) * k;
            st.turnStill = fabsf(st.yawRate) <= 20.0f ? st.turnStill + dt : 0.0f;

            int target = RtChooseCycle(model, st, turnToMotion, set);
            // a reversal (back step <-> walk) passes through standing still for a moment: the cycle goes to the
            // idle only once he has stood for 0.15 s, so it goes straight from one step to the other, in stride
            st.slowTime = st.speed < 6.0f ? st.slowTime + dt : 0.0f;
            if ( target >= 0 && st.cur >= 0 && target != st.cur && !strcmp(RtAnimBaseName(s_anims[target]), "idle")
                 && strcmp(RtAnimBaseName(s_anims[st.cur]), "idle") && st.slowTime < 0.15f )
                target = st.cur;
            if ( target != st.cur && target >= 0 )
            {
                const RtAnim &from = s_anims[st.cur], &to = s_anims[target];
                if ( R_RuntimeXModel_DvarOn("leon_cyclelog") ) // tests: the cycles an entity goes through
                    Com_Printf(0, "leon_cyclelog: ent %d t %d %s -> %s speed %.0f moveRel %.0f yawRate %.0f\n",
                               obj->entnum, timeMs, from.name, to.name, st.speed, st.moveRel, st.yawRate);
                st.prev = st.cur;
                st.prevPhase = st.curPhase;
                st.prevRate = RtCycleRate(from, st.speed);
                // walk <-> run keep the stride's phase (the feet stay in step); a reversal (the back step <-> a step
                // forward) starts where its pose is nearest (RtMatchPhase); others start at the top
                const bool stride = from.moveSpeed > 0.0f && to.moveSpeed > 0.0f;
                const bool reversal = stride && (!strcmp(RtAnimBaseName(from), "back") != !strcmp(RtAnimBaseName(to), "back"));
                const float fromPhase = st.curPhase;
                st.curPhase = reversal ? RtMatchPhase(from, st.curPhase, to)
                              : (stride ? st.curPhase / (float)from.frameCount * (float)to.frameCount : 0.0f);
                if ( R_RuntimeXModel_DvarOn("leon_cyclelog") )
                    Com_Printf(0, "leon_cyclelog: ent %d phase %.1f/%d -> %.1f/%d%s\n", obj->entnum, fromPhase,
                               from.frameCount, st.curPhase, to.frameCount, reversal ? " (matched)" : "");
                st.cur = target;
                st.blend = 0.0f;
            }
            // the body turns toward a sideways motion (backward: the back step, turned the other way); not for a
            // player under RE4's tank controls, who never moves sideways (the noise in his measured motion twisted
            // him on the back step)
            // (aimed in, the player's legs alone turn toward his strafe, at most 60 degrees: the torso turned back
            // onto the aim - R_RuntimeXModel_PoseDObj - twists no further than that)
            float bodyTarget = 0.0f;
            if ( (turnToMotion || s_strafeLegs) && s_anims[st.cur].moveSpeed > 0.0f )
            {
                const bool backStep = !strcmp(RtAnimBaseName(s_anims[st.cur]), "back");
                bodyTarget = backStep ? AngleNormalize180(st.moveRel - 180.0f) : st.moveRel;
                const float limit = turnToMotion ? (backStep ? 70.0f : 100.0f) : 60.0f;
                bodyTarget = bodyTarget > limit ? limit : (bodyTarget < -limit ? -limit : bodyTarget);
            }
            st.bodyYaw += (bodyTarget - st.bodyYaw) * (dt * 8.0f < 1.0f ? dt * 8.0f : 1.0f);
            const RtAnim &cur = s_anims[st.cur];
            // a turn in place steps at the pace of the turn (its own: RT_TURN_IN_PLACE_RATE); the player under tank
            // controls (not strafing) steps at his speed along his facing, backward for the back step (out of a run
            // into it his speed still went forward, and raced the back step at twice its pace)
            float cycleSpeed = st.speed;
            if ( s_stepWatch && !s_strafeLegs && cur.moveSpeed > 0.0f )
            {
                const float along = st.speed * cosf(st.moveRel * 0.017453292f);
                cycleSpeed = !strcmp(RtAnimBaseName(cur), "back") ? -along : along;
                cycleSpeed = cycleSpeed > 0.0f ? cycleSpeed : 0.0f;
            }
            float rate = RtCycleRate(cur, cycleSpeed);
            if ( RtIsTurnInPlace(st.cur) )
            {
                rate = fabsf(st.yawRate) / RT_TURN_IN_PLACE_RATE;
                rate = rate < 0.5f ? 0.5f : (rate > 1.6f ? 1.6f : rate);
            }
            const float oldPhase = st.curPhase;
            st.curPhase = RtWrapPhase(st.curPhase + dt * cur.fps * rate, cur.frameCount);
            // the player's footsteps on the cycle's own plants (s_stepWatch: the local player's): each one passed
            float plants[2];
            const int plantCount = s_stepWatch && st.speed > 20.0f ? RtFootPlants(cur, plants) : 0;
            for ( int p = 0; p < plantCount; ++p )
            {
                const float at = plants[p] * (float)cur.frameCount;
                const bool crossed = st.curPhase >= oldPhase ? (oldPhase < at && at <= st.curPhase)
                                                             : (at > oldPhase || at <= st.curPhase); // (wrapped)
                if ( crossed )
                {
                    s_playerStepRun = !strcmp(RtAnimBaseName(cur), "run");
                    _InterlockedIncrement(&s_playerSteps);
                }
            }
            if ( st.prev >= 0 )
            {
                const RtAnim &prev = s_anims[st.prev];
                st.prevPhase = RtWrapPhase(st.prevPhase + dt * prev.fps * st.prevRate, prev.frameCount);
            }
            st.blend += dt / 0.2f; // the crossfade time
            if ( st.blend >= 1.0f )
            {
                st.blend = 1.0f;
                st.prev = -1;
            }
        }
        st.lastTime = timeMs;
        st.lastYaw = angles[1];
        for ( int k = 0; k < 3; ++k )
            st.lastOrigin[k] = origin[k];
    }
    st.obj = obj;
    const RtAnimState copy = st;
    _InterlockedExchange(&s_animStateLock, 0);
    return copy;
}

// Called by CG_DObjCalcPose / CG_DObjCalcBone before DObjCalcSkel, with the entity's pose origin and angles. A DObj
// whose model has baked animations gets every bone's local pose from its locomotion cycle (crossfaded from the
// previous one), marked as animated so DObjCalcAnim leaves the bones alone; DObjCalcSkel then builds the skeleton as
// for an XAnim pose (the rotation absolute, the translation added to the model's rest offset).
float __cdecl R_RuntimeXModel_PoseDObj(DObj *obj, const float *origin, const float *angles, int timeMs,
                                      bool isPlayer, float aimFrac, float aimPitch, RtAction action, float actionTime,
                                      float actionDuration, const float *aimCorrection, bool turnToMotion,
                                      const char *animSet)
{
    RtApplyEnvScale(); // (leon_matrefl changed from the console)
    const float lookYaw = s_lookYaw; // for this call only
    s_lookYaw = 0.0f;
    struct PlayerMotionReset // (s_strafeLegs / s_turnInPlace: for this call only)
    {
        ~PlayerMotionReset() { s_strafeLegs = s_turnInPlace = s_stepWatch = false; s_moveIntent = 0; s_running = -1; }
    } playerMotionReset;
    const bool strafeLegs = s_strafeLegs, turnInPlace = s_turnInPlace;
    const bool tankPlayer = s_stepWatch; // the local player under RE4's tank controls (R_RuntimeXModel_SetPlayerMotion)
    const float reloadFrom = s_reloadFrom;
    s_reloadFrom = 0.0f;
    const int kickVariant = s_kickVariant; // (for this call only)
    s_kickVariant = 0;
    const char *const actionMotion = s_actionMotion; // (for this call only)
    s_actionMotion = 0;
    const char *const reloadSet = s_reloadSetGiven ? s_reloadSet : animSet; // (for this call only)
    const float firePrevTime = s_firePrevTime;
    s_firePrevTime = -1.0f;
    float firePrevPhase = -1.0f, fireCross = 1.0f; // the previous shot's motion, crossfaded out (RT_FIRE_CROSSFADE)
    s_reloadSetGiven = false;
    s_reloadSet = 0;
    if ( s_anims.empty() || !obj->numModels || !obj->skel.mat )
        return 0.0f;
    const XModel *model = obj->localModels[0];
    if ( RtFindAnim(model, 0) < 0 )
        return 0.0f;
    const RtAnimState st = RtUpdateAnimState(obj, model, origin, angles, timeMs, turnToMotion, animSet);
    if ( st.cur < 0 )
        return 0.0f;
    if ( turnInPlace )
        s_playerTurn = RtIsTurnInPlace(st.cur) ? (!strcmp(RtAnimBaseName(s_anims[st.cur]), "turn_left") ? 1 : -1) : 0;
    const RtAnim &cur = s_anims[st.cur];
    const RtAnim *prev = st.prev >= 0 && st.blend < 1.0f ? &s_anims[st.prev] : 0;
    // the blink (RtBlinkAngle; not once dead, as RE4's moveEye); leon_blinktest holds the lid at an angle (tests)
    float blink = cur.eyelidBone >= 0 && action != RT_ACTION_DEATH ? RtBlinkAngle(timeMs, obj->entnum) : 0.0f;
    {
        const dvar_s *blinkTest = Dvar_FindVar("leon_blinktest");
        if ( blinkTest && blinkTest->type == DVAR_TYPE_STRING && blinkTest->current.string && blinkTest->current.string[0] )
            blink = (float)atof(blinkTest->current.string);
    }
    // aiming (a player's ADS): the upper body takes the aim pose, level blended toward up or down by the view pitch
    // (RE4 blends its three by the aim pitch, cMot3), in by the ADS fraction
    const int aimLevel = aimFrac > 0.0f ? RtFindSetAnim(model, animSet, "aim") : -1;
    int aimPitched = -1;
    float aimPitchBlend = 0.0f, aimPhase = 0.0f;
    if ( aimLevel >= 0 )
    {
        aimPitchBlend = -aimPitch / 60.0f; // view pitch is positive looking down; the up / down poses are ~60 degrees
        if ( aimPitchBlend > 1.0f )
            aimPitchBlend = 1.0f;
        if ( aimPitchBlend < -1.0f )
            aimPitchBlend = -1.0f;
        aimPitched = RtFindSetAnim(model, animSet, aimPitchBlend >= 0.0f ? "aim_up" : "aim_down");
        aimPitchBlend = fabsf(aimPitchBlend);
        const RtAnim &level = s_anims[aimLevel];
        aimPhase = RtWrapPhase((float)(timeMs % 3600000) * 0.001f * level.fps, level.frameCount);
        if ( aimFrac > 1.0f )
            aimFrac = 1.0f;
    }
    // the action: a shot plays RE4's fire motion (level blended toward up / down by the pitch, as the aim) from the
    // shot at its own rate; a reload RE4's reload stretched over BO1's; both fade in and out on the upper body
    int actionAnim = -1, actionPitched = -1;
    float actionPhase = 0.0f, actionWeight = 0.0f, actionPitchBlend = 0.0f;
    if ( action == RT_ACTION_FIRE || action == RT_ACTION_KNIFE || action == RT_ACTION_THROW || action == RT_ACTION_KICK )
    {
        // a shot, a knife slash or a grenade throw, from its start at the motion's own rate; the shot and the slash
        // blend toward their up / down versions by the pitch
        const char *level = action == RT_ACTION_FIRE ? "fire"
                            : (action == RT_ACTION_KNIFE ? "knife"
                                                          : (action == RT_ACTION_KICK ? (kickVariant ? "kick2" : "kick") : "throw"));
        actionAnim = RtFindSetAnim(model, animSet, level);
        if ( actionAnim < 0 && action == RT_ACTION_KICK && kickVariant )
            actionAnim = RtFindSetAnim(model, animSet, "kick"); // (a model converted before the second kick)
        if ( actionAnim >= 0 )
        {
            const RtAnim &motion = s_anims[actionAnim];
            actionPhase = actionTime * motion.fps;
            // a shot: the kick up to its peak, then settling back, each at the weapon's rate (s_fireKick, s_fireSettle);
            // one fired while the last one's motion still plays comes in from where that one is
            if ( action == RT_ACTION_FIRE )
            {
                actionPhase = RtFirePhase(motion, actionTime);
                if ( firePrevTime >= 0.0f && actionTime < RT_FIRE_CROSSFADE )
                {
                    const float prevPhase = RtFirePhase(motion, firePrevTime);
                    const float prevLast = motion.frameCount - 1 > 0 ? (float)(motion.frameCount - 1) : 0.0f;
                    if ( prevPhase < prevLast )
                    {
                        firePrevPhase = prevPhase;
                        fireCross = actionTime / RT_FIRE_CROSSFADE;
                    }
                }
            }
            // the kick ends at frame 34, blending back over its last 8 (from frame 30 to its end at 49 he stays down in
            // the recovery, 4 cm lower and leaning: it ended hunched); the others at their end, over 4
            const float last = action == RT_ACTION_KICK && motion.frameCount - 1 > 34 ? 34.0f : (float)(motion.frameCount - 1);
            const float blendFrames = action == RT_ACTION_KICK ? 8.0f : 4.0f;
            if ( actionPhase >= last )
                actionAnim = -1;
            else
            {
                const float left = last - actionPhase;
                actionWeight = left < blendFrames ? left / blendFrames : 1.0f; // the last frames blend back into the aim
                if ( action != RT_ACTION_FIRE && actionPhase < 2.0f )
                    actionWeight *= actionPhase / 2.0f; // the slash and the throw come in over 2 frames
                if ( action == RT_ACTION_FIRE )
                    actionWeight *= s_fireWeight; // the weapon's (R_RuntimeXModel_SetFireWeight)
                if ( action != RT_ACTION_THROW && action != RT_ACTION_KICK )
                {
                    float r = -aimPitch / 60.0f;
                    r = r > 1.0f ? 1.0f : (r < -1.0f ? -1.0f : r);
                    const bool up = r >= 0.0f;
                    actionPitched = RtFindSetAnim(model, animSet, action == RT_ACTION_FIRE ? (up ? "fire_up" : "fire_down")
                                                                                           : (up ? "knife_up" : "knife_down"));
                    actionPitchBlend = fabsf(r);
                }
            }
        }
    }
    else if ( action == RT_ACTION_THROW_READY )
    {
        // the grenade readied: RE4's ready pose, looped, in over 0.15 s (the throw follows from it)
        actionAnim = RtFindSetAnim(model, animSet, "throw_ready");
        if ( actionAnim >= 0 )
        {
            const RtAnim &ready = s_anims[actionAnim];
            actionPhase = RtWrapPhase(actionTime * ready.fps, ready.frameCount);
            actionWeight = actionTime < 0.15f ? actionTime / 0.15f : 1.0f;
        }
    }
    else if ( action == RT_ACTION_HIT_FRONT || action == RT_ACTION_HIT_BACK )
    {
        // a hit: from it at the motion's own rate, in over 2 frames, back into the locomotion over the last 6
        actionAnim = RtFindSetAnim(model, animSet, action == RT_ACTION_HIT_FRONT ? "hit_front" : "hit_back");
        if ( actionAnim >= 0 )
        {
            const RtAnim &hit = s_anims[actionAnim];
            actionPhase = actionTime * hit.fps;
            const float left = (float)(hit.frameCount - 1) - actionPhase;
            if ( left <= 0.0f )
                actionAnim = -1;
            else
                actionWeight = (left < 6.0f ? left / 6.0f : 1.0f) * (actionPhase < 2.0f ? actionPhase / 2.0f : 1.0f);
        }
    }
    else if ( action == RT_ACTION_DEATH )
    {
        // down: the fall, then its last frame for as long as he stays down; in over 0.2 s
        actionAnim = RtFindSetAnim(model, animSet, "death");
        if ( actionAnim >= 0 )
        {
            const RtAnim &death = s_anims[actionAnim];
            actionPhase = actionTime * death.fps;
            if ( actionPhase > (float)(death.frameCount - 1) )
                actionPhase = (float)(death.frameCount - 1);
            actionWeight = actionTime < 0.2f ? actionTime / 0.2f : 1.0f;
        }
    }
    else if ( action == RT_ACTION_MOTION && actionMotion )
    {
        // a named motion (an enemy's swing, its flinch): once from its start at its own rate, in over 3 frames, back into
        // the locomotion over its last 6
        actionAnim = RtFindSetAnim(model, animSet, actionMotion);
        if ( actionAnim >= 0 )
        {
            const RtAnim &motion = s_anims[actionAnim];
            actionPhase = actionTime * motion.fps;
            const float left = (float)(motion.frameCount - 1) - actionPhase;
            if ( left <= 0.0f || actionPhase < 0.0f )
                actionAnim = -1;
            else
                actionWeight = (left < 6.0f ? left / 6.0f : 1.0f) * (actionPhase < 3.0f ? actionPhase / 3.0f : 1.0f);
        }
    }
    else if ( action == RT_ACTION_TURN )
    {
        // the quick turn: the whole body, at its own rate (the view turns along its root yaw: CL_LeonQuickTurn), to
        // frame 20 (the turn is done at 13; CL_LeonQuickTurn ends it there), blending back into the locomotion from
        // frame 14, as he may move again - over 2 frames, a run started out of it popped from the turn's last pose
        actionAnim = RtFindSetAnim(model, animSet, "turn180");
        if ( actionAnim >= 0 )
        {
            const RtAnim &turn = s_anims[actionAnim];
            const float full = (float)(turn.frameCount - 1) / turn.fps;
            const float length = full < 20.0f / turn.fps ? full : 20.0f / turn.fps;
            const float blend = length > 16.0f / turn.fps ? length - 14.0f / turn.fps : 2.0f / turn.fps;
            actionPhase = actionTime * turn.fps;
            if ( actionTime >= length )
                actionAnim = -1;
            else
                actionWeight = length - actionTime < blend ? (length - actionTime) / blend : 1.0f;
        }
    }
    else if ( action == RT_ACTION_RELOAD && actionDuration > 0.0f )
    {
        actionAnim = RtFindSetAnim(model, reloadSet, "reload");
        if ( actionAnim >= 0 )
        {
            const RtAnim &reload = s_anims[actionAnim];
            float t = actionTime / actionDuration;
            t = t < 0.0f ? 0.0f : (t > 1.0f ? 1.0f : t);
            actionPhase = (reloadFrom + t * (1.0f - reloadFrom)) * (float)(reload.frameCount - 1);
            const float fade = 0.15f / actionDuration; // 0.15 s in and out
            actionWeight = t < fade ? t / fade : (1.0f - t < fade ? (1.0f - t) / fade : 1.0f);
            // RE4's reload ends raising the gun back to its aim (it reloads aimed in): BO1 reloads unaimed, so it holds
            // where that raise starts and eases back into his stance over the rest of the reload - he put the gun up as
            // if to aim before lowering it (the magazine's frames, before it, keep their times)
            if ( reload.reloadRaise > 0 && actionPhase > (float)reload.reloadRaise )
            {
                const float span = (float)(reload.frameCount - 1 - reload.reloadRaise);
                float k = span > 0.0f ? (actionPhase - (float)reload.reloadRaise) / span : 1.0f;
                k = k > 1.0f ? 1.0f : k;
                actionPhase = (float)reload.reloadRaise;
                const float out = 1.0f - k * k * (3.0f - 2.0f * k);
                actionWeight = actionWeight < out ? actionWeight : out;
            }
        }
    }
    else if ( (action == RT_ACTION_RELOAD_START || action == RT_ACTION_RELOAD_LOOP || action == RT_ACTION_RELOAD_END)
              && actionDuration > 0.0f )
    {
        // a reload stage over its own time (BO1's state's): the open blends in over 0.15 s, the close out over its
        // last 0.15 s, a round runs into the next (each one starts on the pose the last ended on)
        actionAnim = RtFindSetAnim(model, animSet, action == RT_ACTION_RELOAD_START ? "reload_start"
                                                    : (action == RT_ACTION_RELOAD_LOOP ? "reload_loop" : "reload_end"));
        if ( actionAnim >= 0 )
        {
            const RtAnim &stage = s_anims[actionAnim];
            float t = actionTime / actionDuration;
            t = t < 0.0f ? 0.0f : (t > 1.0f ? 1.0f : t);
            actionPhase = t * (float)(stage.frameCount - 1);
            actionWeight = 1.0f;
            if ( action == RT_ACTION_RELOAD_START && actionTime < 0.15f )
                actionWeight = actionTime / 0.15f;
            if ( action == RT_ACTION_RELOAD_END && actionDuration - actionTime < 0.15f )
                actionWeight = (actionDuration - actionTime) / 0.15f;
            actionWeight = actionWeight < 0.0f ? 0.0f : actionWeight;
        }
    }
    // the quick turn and the kick take the whole body; aimed in their last frames, the aim takes the upper body from
    // them (they blended out over it: the gun came up from the turn's low arms, the kick's swing pulled it aside). So
    // does a grenade's throw, whose follow-through runs 1.3 s past the release: aimed in it, the throw played on over
    // the raised gun (its arm only: no hips swing the gun after it, so the laser need not wait for it)
    const bool bodyAction = actionAnim >= 0 && actionWeight > 0.0f && (action == RT_ACTION_TURN || action == RT_ACTION_KICK);
    if ( tankPlayer )
        s_playerBodyAction = bodyAction;
    const bool aimTakes = bodyAction || (actionAnim >= 0 && actionWeight > 0.0f && action == RT_ACTION_THROW);
    const float bodyAim = aimTakes && aimLevel >= 0 ? aimFrac : 0.0f;
    // moving (a walk / run / back step cycle, not standing): a hit reaction takes the upper body alone, the legs keep
    // their stride (a hit as he ran stopped his legs dead under him)
    const bool moving = cur.moveSpeed > 0.0f && st.speed > 20.0f;
    const bool wholeBodyAction = action == RT_ACTION_TURN || ((action == RT_ACTION_HIT_FRONT || action == RT_ACTION_HIT_BACK) && !moving)
                                 || action == RT_ACTION_KICK || action == RT_ACTION_DEATH || action == RT_ACTION_MOTION;
    int numBones =cur.boneCount < model->numBones ? cur.boneCount : model->numBones;
    if ( numBones > obj->numBones )
        numBones = obj->numBones;
    // the switch between motions eased (RtPoseFade)
    RtPoseFade *fade = isPlayer && obj->entnum > 0 ? &s_poseFades[(obj->entnum - 1) & 63] : 0;
    float fadeK = 1.0f;
    if ( fade )
    {
        const char *setName = animSet ? animSet : "";
        if ( fade->obj != obj || timeMs < fade->lastTime || timeMs - fade->lastTime > 500 || fade->bones != numBones )
        {
            fade->obj = obj; // (a new body, or a gap: nothing to ease from)
            fade->start = -100000;
            fade->bones = 0;
        }
        else if ( (action != fade->action && (actionAnim >= 0 || fade->anim >= 0)) || actionAnim != fade->anim
                  || strncmp(setName, fade->set, sizeof(fade->set) - 1) )
        {
            // (an action's end with its motion already over - the kick's past its frame 34 - changes nothing on him)
            // (and not into a shot, a slash or a kick: they hit at once - eased, the shot's kick went soft)
            const bool own = action == RT_ACTION_TURN || action == RT_ACTION_DEATH || fade->action == RT_ACTION_TURN
                             || fade->action == RT_ACTION_DEATH || action == RT_ACTION_FIRE || action == RT_ACTION_KNIFE
                             || action == RT_ACTION_KICK;
            if ( !own )
            {
                memcpy(fade->from, fade->last, sizeof(fade->from));
                fade->start = timeMs;
            }
            else
                fade->start = -100000; // (an ease under way ends too)
            if ( R_RuntimeXModel_DvarOn("leon_fadelog") ) // tests: each ease of his motions, and what started it
                Com_Printf(0, "leon_fadelog: ent %d t %d action %d -> %d anim %d -> %d set %s -> %s%s\n", obj->entnum, timeMs,
                           (int)fade->action, (int)action, fade->anim, actionAnim, fade->set, setName, own ? " (none)" : "");
        }
        else if ( st.cur != fade->cycle && fade->cycleBlend < 1.0f && action != RT_ACTION_FIRE && action != RT_ACTION_KNIFE
                  && action != RT_ACTION_KICK && action != RT_ACTION_TURN && action != RT_ACTION_DEATH )
        {
            // a walk / run / idle switch before the last one's crossfade had finished (RtUpdateAnimState): the cycle
            // it came from was dropped mid-blend - his legs jumped; eased from the pose shown instead
            memcpy(fade->from, fade->last, sizeof(fade->from));
            fade->start = timeMs;
            if ( R_RuntimeXModel_DvarOn("leon_fadelog") )
                Com_Printf(0, "leon_fadelog: ent %d t %d cycle %s -> %s (its blend %.2f)\n", obj->entnum, timeMs,
                           fade->cycle >= 0 ? RtAnimBaseName(s_anims[fade->cycle]) : "-", RtAnimBaseName(s_anims[st.cur]),
                           fade->cycleBlend);
        }
        fade->cycle = st.cur;
        fade->cycleBlend = st.blend;
        fade->action = action;
        fade->anim = actionAnim;
        I_strncpyz(fade->set, setName, sizeof(fade->set));
        fade->lastTime = timeMs;
        if ( timeMs >= fade->start && timeMs - fade->start < RT_POSE_FADE_MS )
        {
            const float k = (float)(timeMs - fade->start) / (float)RT_POSE_FADE_MS;
            fadeK = k * k * (3.0f - 2.0f * k);
        }
    }
    if ( numBones > RT_POSE_FADE_BONES )
        fade = 0;
    if ( tankPlayer )
        s_playerEasing = fade && fadeK < 1.0f;
    for ( int b = 0; b < numBones; ++b )
    {
        DObjAnimMat *mat = &obj->skel.mat[b];
        // The root: the skeleton is in world space, CG_DoBaseOriginController (in CG_DoControllers, before this) puts
        // the entity's origin and angles on it, composed with a controller's root. A player's is rebuilt the same way
        // with the body's turn toward the motion in place of CoD's leg yaw (BG_Player_DoControllers' tag_origin, for
        // CoD's leg animations); another entity's stays as set. Not once the skeleton is built from it.
        if ( b == 0 && !(obj->skel.partBits.skel[0] & 0x80000000)
             && (isPlayer || !(obj->skel.partBits.anim[0] & 0x80000000)) )
        {
            // aiming: facing the aim (but for the strafing legs: the torso is turned back onto it at the spine - for
            // the player under tank controls always, aimed or not, his legs easing back under him)
            const float half = st.bodyYaw * (aimLevel >= 0 && !strafeLegs && !tankPlayer ? 1.0f - aimFrac : 1.0f) * 0.5f
                               * 0.017453292f;
            float quat[4] = { 0.0f, 0.0f, sinf(half), cosf(half) };
            if ( isPlayer )
            {
                float baseQuat[4];
                AnglesToQuat(angles, baseQuat);
                QuatMultiplyEquals(baseQuat, quat); // as CG_DoBaseOriginController composes a controller's root
            }
            for ( int k = 0; k < 4; ++k )
                mat->quat[k] = quat[k];
            for ( int k = 0; k < 3; ++k )
                mat->trans[k] = isPlayer ? origin[k] : 0.0f;
            mat->transWeight = 2.0f;
            obj->skel.partBits.anim[0] |= 0x80000000;
            continue;
        }
        if ( obj->skel.partBits.anim[b >> 5] & (0x80000000 >> (b & 31)) )
            continue; // posed already this frame (an earlier CG_DObjCalcBone / CG_DObjCalcPose)
        RtSampleBone(cur, st.curPhase, b, mat->quat, mat->trans);
        if ( prev )
        {
            float quat[4], trans[3];
            RtSampleBone(*prev, st.prevPhase, b, quat, trans);
            const float sign = quat[0] * mat->quat[0] + quat[1] * mat->quat[1] + quat[2] * mat->quat[2]
                + quat[3] * mat->quat[3] < 0.0f ? -1.0f : 1.0f;
            for ( int k = 0; k < 4; ++k )
                mat->quat[k] = sign * quat[k] + (mat->quat[k] - sign * quat[k]) * st.blend;
            for ( int k = 0; k < 3; ++k )
                mat->trans[k] = trans[k] + (mat->trans[k] - trans[k]) * st.blend;
        }
        float bobTaken = 0.0f; // the spine: the share of the run's held-back bob the upper body's blends took with it
        float bobGiven[3] = {}; // and what of it is given back
        if ( aimLevel >= 0 && cur.upperBody && cur.upperBody[b] )
        {
            bobTaken = aimFrac;
            float quat[4], trans[3];
            RtSampleBone(s_anims[aimLevel], aimPhase, b, quat, trans);
            if ( aimPitched >= 0 && aimPitchBlend > 0.0f )
            {
                float pq[4], pt[3];
                const RtAnim &pitched = s_anims[aimPitched];
                RtSampleBone(pitched, RtWrapPhase(aimPhase, pitched.frameCount), b, pq, pt);
                RtBlendPose(quat, trans, pq, pt, aimPitchBlend);
            }
            RtBlendPose(mat->quat, mat->trans, quat, trans, aimFrac);
        }
        if ( actionAnim >= 0 && actionWeight > 0.0f && (wholeBodyAction || (cur.upperBody && cur.upperBody[b])) )
        {
            float quat[4], trans[3];
            RtSampleBone(s_anims[actionAnim], actionPhase, b, quat, trans);
            if ( actionPitched >= 0 && actionPitchBlend > 0.0f )
            {
                float pq[4], pt[3];
                RtSampleBone(s_anims[actionPitched], actionPhase, b, pq, pt);
                RtBlendPose(quat, trans, pq, pt, actionPitchBlend);
            }
            if ( firePrevPhase >= 0.0f )
            {
                // the shot comes in from where the last one's motion was (RT_FIRE_CROSSFADE)
                float prevQuat[4], prevTrans[3];
                RtSampleBone(s_anims[actionAnim], firePrevPhase, b, prevQuat, prevTrans);
                if ( actionPitched >= 0 && actionPitchBlend > 0.0f )
                {
                    float pq[4], pt[3];
                    RtSampleBone(s_anims[actionPitched], firePrevPhase, b, pq, pt);
                    RtBlendPose(prevQuat, prevTrans, pq, pt, actionPitchBlend);
                }
                RtBlendPose(prevQuat, prevTrans, quat, trans, fireCross);
                for ( int k = 0; k < 4; ++k )
                    quat[k] = prevQuat[k];
                for ( int k = 0; k < 3; ++k )
                    trans[k] = prevTrans[k];
            }
            // the knife slash's own turn of the torso, at the lower spine, half taken: RE4's slash swung his whole
            // upper body round, odd over the shoulder camera (the arm still sweeps); and none of its head: RE4's
            // slash ducks and swings the head with the swing, odd again over the shoulder (the head stays with the
            // aim or the cycle)
            const bool slashHead = isPlayer && action == RT_ACTION_KNIFE && cur.headSide && cur.headSide[b];
            const float weight = (isPlayer && action == RT_ACTION_KNIFE && b == cur.spineBone ? actionWeight * 0.5f
                                                                                              : actionWeight)
                                 * (cur.upperBody && cur.upperBody[b] ? 1.0f - bodyAim : 1.0f) * (slashHead ? 0.0f : 1.0f);
            RtBlendPose(mat->quat, mat->trans, quat, trans, weight);
            // (a whole body action takes the pelvis's bob with it: none of its share is given back)
            bobTaken = wholeBodyAction ? bobTaken * (1.0f - weight) : 1.0f - (1.0f - bobTaken) * (1.0f - weight);
        }
        if ( b == cur.spineBone && bobTaken > 0.0f && (cur.runBob || (prev && prev->runBob)) )
        {
            // the run's bob held back at the spine (RtSoftenRunBob), given back for what the aim and an upper body
            // action took of it: under a reload on the run his upper body bounced with the pelvis's full bob
            float bob[3];
            RtSampleRunBob(cur, st.curPhase, bob);
            if ( prev )
            {
                float prevBob[3];
                RtSampleRunBob(*prev, st.prevPhase, prevBob);
                for ( int k = 0; k < 3; ++k )
                    bob[k] = prevBob[k] + (bob[k] - prevBob[k]) * st.blend;
            }
            for ( int k = 0; k < 3; ++k )
            {
                bobGiven[k] = bob[k] * bobTaken;
                mat->trans[k] += bobGiven[k];
            }
        }
        if ( b == cur.spineBone && isPlayer && action == RT_ACTION_KNIFE && actionAnim >= 0 && actionWeight > 0.0f )
        {
            // RE4's slash holds the torso ~17 degrees turned against the hips (its knife stance; the handgun's is
            // square, measured from the shoulders against the hips in both motions): over the legs he already has, the
            // slash swung the torso aside - taken back out, as the slash comes in (half: half its turn is taken, above)
            float twist = -8.5f;
            const dvar_s *twistTest = Dvar_FindVar("leon_knifetwist"); // tests: another twist (degrees)
            if ( twistTest && twistTest->type == DVAR_TYPE_STRING && twistTest->current.string && twistTest->current.string[0] )
                twist = (float)atof(twistTest->current.string);
            const float yaw = twist * actionWeight * 0.5f * 0.017453292f;
            const float qy[4] = { 0.0f, 0.0f, sinf(yaw), cosf(yaw) };
            float out[4];
            RtQuatMul(qy, mat->quat, out);
            for ( int k = 0; k < 4; ++k )
                mat->quat[k] = out[k];
        }
        if ( b == cur.eyelidBone && blink != 0.0f )
        {
            // the blink: the lid about its parent's (the head's) left axis
            const float half = blink * 0.5f * 0.017453292f;
            const float qy[4] = { 0.0f, sinf(half), 0.0f, cosf(half) };
            float out[4];
            RtQuatMul(qy, mat->quat, out);
            for ( int k = 0; k < 4; ++k )
                mat->quat[k] = out[k];
        }
        if ( b == cur.neckBone && isPlayer && lookYaw != 0.0f )
        {
            // the head's look (RE4's cPlNeck): the neck turned about the model's up axis - in its parent's frame
            // P (the chain's local rotations below the root): local' = P^-1 qz P local
            float p[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
            for ( int k = 0; k < cur.neckChainCount; ++k )
            {
                float t[4];
                RtQuatMul(p, obj->skel.mat[cur.neckChain[k]].quat, t);
                const float len = sqrtf(t[0] * t[0] + t[1] * t[1] + t[2] * t[2] + t[3] * t[3]);
                for ( int c = 0; c < 4; ++c )
                    p[c] = len > 0.0f ? t[c] / len : t[c];
            }
            const float half = lookYaw * 0.5f * 0.017453292f;
            const float qz[4] = { 0.0f, 0.0f, sinf(half), cosf(half) };
            const float pInv[4] = { -p[0], -p[1], -p[2], p[3] };
            float t1[4], t2[4], out[4];
            RtQuatMul(pInv, qz, t1);
            RtQuatMul(t1, p, t2);
            RtQuatMul(t2, mat->quat, out);
            for ( int k = 0; k < 4; ++k )
                mat->quat[k] = out[k];
        }
        const float lenSq = mat->quat[0] * mat->quat[0] + mat->quat[1] * mat->quat[1] + mat->quat[2] * mat->quat[2]
            + mat->quat[3] * mat->quat[3];
        if ( lenSq > 0.0f )
        {
            const float inv = 1.0f / sqrtf(lenSq);
            for ( int k = 0; k < 4; ++k )
                mat->quat[k] *= inv;
        }
        else
        {
            mat->quat[0] = mat->quat[1] = mat->quat[2] = 0.0f;
            mat->quat[3] = 1.0f;
        }
        if ( fade )
        {
            if ( b > 0 && fadeK < 1.0f && fade->bones == numBones )
                RtBlendPose(mat->quat, mat->trans, fade->from[b], fade->from[b] + 4, 1.0f - fadeK);
            for ( int k = 0; k < 4; ++k )
                fade->last[b][k] = mat->quat[k];
            for ( int k = 0; k < 3; ++k )
                fade->last[b][4 + k] = mat->trans[k];
        }
        // the torso's turns at the spine, after the ease (RtPoseFade) and kept out of the pose it eases from: they
        // answer the root's turn and the aim, as they are now (eased, the torso still turned back by the hips' old turn
        // while the root, never eased, had the new one: a step switched under the aim swung the gun 20 degrees, and the
        // aim correction, eased too, chased it and wobbled the laser)
        if ( b == cur.spineBone && aimCorrection && aimLevel >= 0 && (aimCorrection[0] != 0.0f || aimCorrection[1] != 0.0f) )
        {
            // the aim correction: the upper body turned (yaw about up, pitch about the left axis, in its parent's
            // frame) so the gun's barrel points where the shots land; in by the aim
            const float yaw = aimCorrection[1] * aimFrac * 0.5f * 0.017453292f;
            const float pitch = aimCorrection[0] * aimFrac * 0.5f * 0.017453292f;
            const float qy[4] = { 0.0f, 0.0f, sinf(yaw), cosf(yaw) };
            const float qp[4] = { 0.0f, sinf(pitch), 0.0f, cosf(pitch) };
            float qc[4];
            RtQuatMul(qy, qp, qc);
            float out[4];
            RtQuatMul(qc, mat->quat, out);
            for ( int k = 0; k < 4; ++k )
                mat->quat[k] = out[k];
        }
        if ( b == cur.spineBone && isPlayer && (strafeLegs || tankPlayer) && st.bodyYaw != 0.0f )
        {
            // the strafe: the hips turned toward the step (the root, above), the torso turned back onto the aim; not
            // only while strafing (aimed past half way): the aim let go, the hips' turn went to his torso at once - his
            // head and torso jumped by half of it and eased back (a strafe's, or a zombie's or a wall's nudge's)
            const float yaw = -st.bodyYaw * 0.5f * 0.017453292f;
            const float qy[4] = { 0.0f, 0.0f, sinf(yaw), cosf(yaw) };
            float out[4];
            RtQuatMul(qy, mat->quat, out);
            for ( int k = 0; k < 4; ++k )
                mat->quat[k] = out[k];
        }
        if ( b == cur.spineBone && tankPlayer && b > model->numRootBones && R_RuntimeXModel_DvarOn("leon_boblog") )
        {
            // tests: the spine's height over the root (the pelvis's offset and turn), and without the bob given back
            const int pelvis = b - model->parentList[b - model->numRootBones];
            const DObjAnimMat *pm = &obj->skel.mat[pelvis];
            const float *pelvisBase = pelvis >= model->numRootBones ? &model->trans[3 * (pelvis - model->numRootBones)] : 0;
            const float *spineBase = &model->trans[3 * (b - model->numRootBones)];
            float heights[2];
            for ( int pass = 0; pass < 2; ++pass )
            {
                float v[3];
                for ( int k = 0; k < 3; ++k )
                    v[k] = spineBase[k] + mat->trans[k] - (pass ? bobGiven[k] : 0.0f);
                const float *q = pm->quat;
                const float t[3] = { 2.0f * (q[1] * v[2] - q[2] * v[1]), 2.0f * (q[2] * v[0] - q[0] * v[2]),
                                     2.0f * (q[0] * v[1] - q[1] * v[0]) };
                heights[pass] = (pelvisBase ? pelvisBase[2] : 0.0f) + pm->trans[2] + v[2] + q[3] * t[2] + (q[0] * t[1] - q[1] * t[0]);
            }
            Com_Printf(0, "leon_boblog: t %d %s action %d taken %.2f height %.2f without %.2f\n", timeMs,
                       RtAnimBaseName(s_anims[st.cur]), (int)action, bobTaken, heights[0], heights[1]);
        }
        mat->transWeight = 2.0f;
        obj->skel.partBits.anim[b >> 5] |= 0x80000000 >> (b & 31);
    }
    if ( fade )
        fade->bones = numBones; // (its last pose complete)
    return action == RT_ACTION_FIRE && actionAnim >= 0 ? actionWeight : 0.0f;
}
