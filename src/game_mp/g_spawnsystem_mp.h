#pragma once
#include <universal/dvar.h>
#include <bgame/bg_public.h>

enum eSpawnSystemDebugMode : __int32
{
    SS_DEBUG_OFF                   = 0x0,
    SS_DEBUG_LIVE                  = 0x1,
    SS_DEBUG_ARCHIVE_LOADING       = 0x2,
    SS_DEBUG_ARCHIVE_VIEW          = 0x3,
    SS_DEBUG_ARCHIVE_SIMULATE      = 0x4,
    SS_DEBUG_ARCHIVE_SIMULATE_LIVE = 0x5,
    SS_DEBUG_ARCHIVE_MAX           = 0x5,
    SS_DEBUG_COUNT                 = 0x6,
};

enum eInfluencerType : __int32
{                                       // XREF: SpawnInfluencerPreset/r
                                        // ?SpawnSystem_SetPresetData@@YAPAUSpawnInfluencerPreset@@HW4eInfluencerType@@W4eInfluencerShape@@MMMW4eInfluencerScoreCurve@@PBD@Z/r ...
    INFLUENCER_TYPE_NORMAL    = 0x0,
    INFLUENCER_TYPE_PLAYER    = 0x1,
    INFLUENCER_TYPE_WEAPON    = 0x2,
    INFLUENCER_TYPE_DOG       = 0x3,
    INFLUENCER_TYPE_VEHICLE   = 0x4,
    INFLUENCER_TYPE_SQUAD     = 0x5,
    INFLUENCER_TYPE_GAME_MODE = 0x6,
    INFLUENCER_TYPE_UNUSED    = 0x7,
    MAX_INFLUENCER_TYPE       = 0x8,
};

enum eInfluencerShape : __int32
{                                       // XREF: SpawnInfluencerPreset/r
                                        // ?SpawnSystem_SetPresetData@@YAPAUSpawnInfluencerPreset@@HW4eInfluencerType@@W4eInfluencerShape@@MMMW4eInfluencerScoreCurve@@PBD@Z/r ...
    INFLUENCER_SHAPE_SPHERE   = 0x0,
    INFLUENCER_SHAPE_CYLINDER = 0x1,
    INFLUENCER_SHAPE_BOX      = 0x2,
    MAX_INFLUENCER_SHAPE      = 0x3,
};

enum eInfluencerScoreCurve : __int32
{                                       // XREF: SpawnInfluencerPreset/r
                                        // ?SpawnSystem_SetPresetData@@YAPAUSpawnInfluencerPreset@@HW4eInfluencerType@@W4eInfluencerShape@@MMMW4eInfluencerScoreCurve@@PBD@Z/r ...
    INFLUENCER_SC_CONSTANT       = 0x0,
    INFLUENCER_SC_LINEAR         = 0x1,
    INFLUENCER_SC_STEEP          = 0x2,
    INFLUENCER_SC_INVERSE_LINEAR = 0x3,
    INFLUENCER_SC_NEGATIVE_TO_POSITIVE = 0x4,
    MAX_INFLUENCER_SCORE_CURVE   = 0x5,
};

struct SortedPointArray // sizeof=0x328
{                                       // XREF: .data:SortedPointArray * g_sortedTeamSpawnPoints/r
    int spawnPointsByIndex[200];        // XREF: SpawnSystem_ComputePointWeights(gentity_s *,int,int)+106/w
    int count;                          // XREF: SpawnSystem_SortPointsForTeamByScore(int)+21/r
    int sortTime;                       // XREF: SpawnSystem_UpdateSpawnPointsForTeam(int,int)+76/w
};

struct SpawnPoint // sizeof=0x34
{                                       // XREF: .data:SpawnPoint * g_spawnPoints/r
                                        // ??$_Insertion_sort1@PAUSpawnPoint@@Vcompare_spawnpoint_sort@@U1@@std@@YAXPAUSpawnPoint@@0Vcompare_spawnpoint_sort@@0@Z/r ...
    float origin[3];                    // XREF: SpawnSystem_AddPoint(int,gentity_s *)+C5/r
                                        // SpawnSystem_AddPoint(int,gentity_s *)+DE/w ...
    float weight[3];                    // XREF: SpawnSystem_AddPoint(int,gentity_s *)+104/o
                                        // std::_Unguarded_partition<int *,compare_spawnpoint_score_sort>(int *,int *,compare_spawnpoint_score_sort)+6E/r ...
    float baseWeight[3];                // XREF: SpawnSystem_AddPoint(int,gentity_s *)+11C/o
    int entNum;                         // XREF: SpawnSystem_FindPointByEnt(int)+2E/r
                                        // SpawnSystem_AddPoint(int,gentity_s *)+F3/w ...
    int teamMask;                       // XREF: SpawnSystem_AddPoint(int,gentity_s *)+138/w
                                        // SpawnSystem_AddPoint(int,gentity_s *)+15F/r ...
    float yaw;
    int orderNum;
};

struct SpawnInfluencerPreset // sizeof=0x1C
{                                       // XREF: .data:SpawnInfluencerPreset * g_spawnInfluencerPresets/r
    int index;
    eInfluencerType type;               // XREF: SpawnSystem_Init(bool)+CC/w
    eInfluencerShape shape;
    float radius;
    float originalLength;
    float score;
    eInfluencerScoreCurve score_curve;
};

struct SpawnInfluencer // sizeof=0x7C
{
    bool used;
    bool active;
    // padding byte
    // padding byte
    int expireTime;
    int disableTime;
    int teamMask;
    EntHandle entity;
    SpawnInfluencerPreset *preset;
    float boundingRadius;
    float bounds[2];
    float length;
    float origin[3];
    float forward[3];
    float up[3];
    float quat[4];
    float worldQuat[4];
    float midPoint[3];
    int lastUpdateTime;
};

void __cdecl SpawnSystem_SortPoints();
void __cdecl SpawnSystem_SortPointsForTeamByScore(int team);
void __cdecl SpawnSystem_Init();
const dvar_s *SpawnSystem_RegisterDvars();
int __cdecl SpawnSystem_FindPointByEnt(int entNum);
void __cdecl SpawnSystem_SetRandomVariation(float variation);
void __cdecl SpawnSystem_ClearPoints();
char __cdecl SpawnSystem_AddPoint(unsigned int team, gentity_s *ent);
void __cdecl SpawnSystem_ClearPointsBaseWeight(int teammask);
void __cdecl SpawnSystem_SetPointBaseWeight(SpawnPoint *point, char teammask, float weight);
void __cdecl SpawnSystem_SetPointsBaseWeight(int teammask, float *objective_position, float angle, float score);
SpawnInfluencerPreset *__cdecl SpawnSystem_SetPresetData(
                int influencerIndex,
                eInfluencerType type,
                eInfluencerShape shape,
                float length,
                float radius,
                float score,
                eInfluencerScoreCurve score_curve);
void __cdecl SpawnSystem_InitInfluencer(
                SpawnInfluencer *influencer,
                eInfluencerType type,
                eInfluencerShape shape,
                const float *origin,
                const float *forward,
                const float *up,
                float radius,
                float axis_length,
                float score,
                eInfluencerScoreCurve score_curve,
                int teamMask,
                unsigned int entNum,
                int timeout);
int __cdecl SpawnSystem_AddSphereInfluencer(
                eInfluencerType type,
                float *origin,
                float radius,
                float score,
                eInfluencerScoreCurve score_curve,
                int teamMask,
                unsigned int entNum,
                int timeout);
int __cdecl SpawnSystem_FindFreeInfluencer();
void __cdecl SpawnSystem_ComputeInfluencerBounds(SpawnInfluencer *influencer);
void __cdecl SpawnSystem_InsertSortedInfluencer(SpawnInfluencer *influencer);
int __cdecl SpawnSystem_AddCylinderInfluencer(
                eInfluencerType type,
                float *origin,
                float *forward,
                float *up,
                float radius,
                float axis_length,
                float score,
                eInfluencerScoreCurve score_curve,
                int teamMask,
                unsigned int entNum,
                int timeout);
void __cdecl SpawnSystem_UpdateCylinderInfluencer(SpawnInfluencer *influencer);
void __cdecl SpawnSystem_CalculateCylinderLength(SpawnInfluencer *influencer);
void __cdecl SpawnSystem_CalculateCylinderAxis(SpawnInfluencer *influencer);
void __cdecl QuatMultiply(const float *in1, const float *in2, float *out);
void __cdecl SpawnSystem_CalculateCylinderAxisMidpoint(SpawnInfluencer *influencer);
char __cdecl SpawnSystem_RemoveInfluencer(unsigned int influencer_index);
void __cdecl SpawnSystem_RemoveSortedInfluencer(SpawnInfluencer *influencer);
void __cdecl SpawnSystem_RemoveSortedInfluencerByIndex(unsigned int influencer_index);
char __cdecl SpawnSystem_EnableInfluencer(unsigned int influencer_index, bool enabled);
char __cdecl SpawnSystem_SetInfluencerTeamMask(unsigned int influencer_index, int team_mask);
void __cdecl SpawnSystem_ClearEntityInfluencers(gentity_s *ent);
void __cdecl SpawnSystem_ClearTimedOutInfluencers();
SpawnInfluencer *__cdecl SpawnSystem_GetInfluencerList();
void __cdecl SpawnSystem_Update();
void SpawnSystem_SortInfluencers();
void SpawnSystem_UpdateInfluencerBounds();
void SpawnSystem_UpdateInfluencerOriginAngles();
double __cdecl SpawnSystem_InfluenceWeight(
                eInfluencerScoreCurve curve_type,
                float influencer_radius_sq,
                float distance_sq,
                float score);
double __cdecl SpawnSystem_ComputeSphereInfluence(SpawnInfluencer *influencer, SpawnPoint *point);
double __cdecl SpawnSystem_ComputeCylinderInfluence(SpawnInfluencer *influencer, SpawnPoint *point);
void __cdecl SpawnSystem_ComputePointWeight(
                gentity_s *player,
                unsigned int point_team,
                unsigned int influencer_team,
                SpawnPoint *point,
                bool bIsArchiveCall);
void __cdecl SpawnSystem_ComputePointWeights(gentity_s *player, unsigned int point_team, unsigned int influencer_team);
SpawnPoint *__cdecl SpawnSystem_GetSpawnPoints();
int __cdecl SpawnSystem_GetSpawnPointCount();
void __cdecl SpawnSystem_SortPointsByScore(unsigned int team);
int __cdecl SpawnSystem_UpdateSpawnPointsForTeam(unsigned int point_team, unsigned int influencer_team);
int __cdecl SpawnSystem_UpdateSpawnPointsForPlayer(
                gentity_s *ent,
                unsigned int point_team,
                unsigned int influencer_team);
int __cdecl SpawnSystem_GetSortedPointEntNum(unsigned int team, unsigned int index);
char __cdecl SpawnSystem_IsSpawnPointVisible(float *origin, float *angles, int other_team, gentity_s *ignore_entity);
char __cdecl SpawnSystem_IsSpawnPointVisible(
                float *origin,
                float *angles,
                int other_team,
                gentity_s *ignore_entity,
                bool render_mode_success);
void __cdecl SpawnSystem_DebugRenderVisibilityBox(float *point1, float *angles1, const float *color);
void __cdecl SpawnSystem_DebugRenderVisibilityCheck(
                float *point1,
                float *angles1,
                float *point2,
                float *angles2,
                const float *color,
                float height_offset,
                gentity_s *ignore_entity,
                bool collision_test);
void __cdecl SpawnSystem_DebugSideSwitch(bool enabled);
void SpawnSystem_DebugRender();
void __cdecl SpawnSystem_DebugRenderTeamPointWeights(int teamMask);
void __cdecl SpawnSystem_DebugRenderTeamPoints(int teamMask);
void __cdecl SpawnSystem_DebugRenderSpawnPointBox(SpawnPoint *sp, const float *color, int bDepthTest);
void __cdecl SpawnSystem_SetSpawnPointsOrder(int team);
void __cdecl SpawnSystem_DebugUpdateSpawnPoints(
                gentity_s *player,
                unsigned int point_team,
                unsigned int influencer_team);
