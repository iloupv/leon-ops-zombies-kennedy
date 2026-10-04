#pragma once
#include <game/teams.h>

enum CompassType : __int32;

struct CompassScrambler // sizeof=0x10
{                                       // XREF: cg_s/r
    float pos[2];
    int startTime;
    int handle;
};

struct CompassActor // sizeof=0x4C
{                                       // XREF: .data:s_compassActors/r
    int lastUpdate;
    float lastPos[2];
    float lastEnemyPos[2];
    float satelliteEnemyPos[2];
    float lastYaw;
    int pingTime;
    int beginFadeTime;
    int beginRadarFadeTime;
    int beginSatelliteFadeTime;
    int beginStaticImageFadeTime;
    int beginVoiceFadeTime;
    bool enemy;
    bool inVehicle;
    // padding byte
    // padding byte
    unsigned int perks[2];
    int needsRevive;
    float currentFadeTimeAmount;
};

struct __declspec(align(4)) CompassTurrets // sizeof=0x24
{                                       // XREF: .data:s_compassTurrets/r
    int entityNum;                      // XREF: GetCompassTurrets+5B/r
                                        // GetCompassTurrets+15E/w
    int lastUpdate;                     // XREF: GetCompassTurrets+AB/r
                                        // GetCompassTurrets+FC/r ...
    int beginFadeTime;
    float lastPos[2];
    float lastYaw;
    team_t team;
    int ownerIndex;
    bool enemy;
    bool firing;
    // padding byte
    // padding byte
};

struct CompassVehicle // sizeof=0x1C
{                                       // XREF: .data:s_compassVehicles/r
    int entityNum;                      // XREF: GetVehicle+2E/r
                                        // GetVehicle+12B/w
    int lastUpdate;                     // XREF: GetVehicle+7E/r
                                        // GetVehicle+CF/r ...
    float lastPos[2];
    float lastYaw;
    team_t team;
    int ownerIndex;
};

struct CompassHelicopter // sizeof=0x1C
{                                       // XREF: .data:s_compassHelicopter/r
    int entityNum;                      // XREF: GetHelicopter+2E/r
                                        // GetHelicopter+12B/w
    int lastUpdate;                     // XREF: GetHelicopter+7E/r
                                        // GetHelicopter+CF/r ...
    float lastPos[2];
    float lastYaw;
    team_t team;
    int ownerIndex;
};

struct CompassGuidedMissile // sizeof=0x1C
{                                       // XREF: .data:s_compassGuidedMissiles/r
    int entityNum;                      // XREF: GetGuidedMissile+2E/r
                                        // GetGuidedMissile+12B/w
    int lastUpdate;                     // XREF: GetGuidedMissile+7E/r
                                        // GetGuidedMissile+CF/r ...
    float lastPos[2];
    float lastYaw;
    team_t team;
    int ownerIndex;
};

struct __declspec(align(4)) CompassDogs // sizeof=0x20
{                                       // XREF: .data:s_compassDogs/r
    int entityNum;                      // XREF: GetDogs+58/r
                                        // GetDogs+146/w
    int lastUpdate;                     // XREF: GetDogs+A2/r
                                        // GetDogs+ED/r ...
    float lastPos[2];
    float lastYaw;
    team_t team;
    int ownerIndex;
    bool enemy;
    // padding byte
    // padding byte
    // padding byte
};

struct __declspec(align(4)) CompassFakeFire // sizeof=0x18
{                                       // XREF: .data:s_compassFakeFire/r
    float pos[2];
    float yaw;
    int entityNum;
    int beginFadeTime;
    bool enemy;
    bool satellite;
    // padding byte
    // padding byte
};

struct CompassArtillery // sizeof=0x14
{                                       // XREF: .data:s_compassArtillery/r
    float pos[2];                       // XREF: CG_AddArtilleryPing(int,float * const)+170/r
    float realPos[2];                   // XREF: CG_AddArtilleryPing(int,float * const)+129/o
    int fireTime;                       // XREF: CG_AddArtilleryPing(int,float * const)+88/r
};

struct cg_s;
struct centity_s;
struct rectDef_s;
struct Material;
struct clientInfo_t;


void __cdecl CG_ClearCompassPingData();
void __cdecl RadarPingEnemyPlayer(CompassActor *actor, int time, unsigned int PerkToIgnore);
bool __cdecl BG_HasPerk(const unsigned int *perks, unsigned int perkIndex);
void __cdecl RadarPingEnemyTurrets(CompassTurrets *turret, int time);
void __cdecl SatellitePingEnemyPlayer(CompassActor *actor, int time, unsigned int PerkToIgnore);
void __cdecl CG_CompassUpdateVehicleInfo(int localClientNum, int entityIndex);
CompassVehicle *__cdecl GetVehicle(int localClientNum, int entityNum);
void __cdecl CG_CompassUpdateHelicopterInfo(int localClientNum, int entityIndex);
CompassHelicopter *__cdecl GetHelicopter(int localClientNum, int entityNum);
void __cdecl CG_CompassUpdateGuidedMissileInfo(int localClientNum, int entityIndex);
CompassGuidedMissile *__cdecl GetGuidedMissile(int localClientNum, int entityNum);
void __cdecl CG_CompassUpdateVehicleOccupantInfo(int localClientNum, int entityIndex);
void __cdecl ActorUpdatePos(int localClientNum, CompassActor *actor, const float *newPos, int actorClientIndex);
bool __cdecl DoesMovementCrossRadar(cg_s *cgameGlob, float radarProgress, const float *p1, const float *p2);
void __cdecl GetRadarLine(cg_s *cgameGlob, float radarProgress, float *line);
double __cdecl GetRadarLineMargin(cg_s *cgameGlob);
bool __cdecl DoesMovementCrossLocalRadar(
                const float *radarPos,
                float localRadarProgress,
                const float *p1,
                const float *p2);
double __cdecl GetLocalRadarRadius(float radarProgress);
bool __cdecl CanLocalPlayerHearActorFootsteps(int localClientNum, const float *actorPos, int actorClientIndex);
void __cdecl CG_CompassUpdateDogInfo(int localClientNum, int entityIndex);
CompassDogs *__cdecl GetDogs(int localClientNum, int entityNum, int time);
void __cdecl CG_CompassUpdateTurretInfo(int localClientNum, int entityIndex);
CompassTurrets *__cdecl GetCompassTurrets(int localClientNum, int entityNum, int time);
void __cdecl CG_CompassRadarPingEnemyTurrets(
                int localClientNum,
                float oldRadarProgress,
                float newRadarProgress,
                float oldLocalRadarProgress,
                float newLocalRadarProgress);
bool __cdecl DoLinesSurroundPoint(cg_s *cgameGlob, float *radarLine1, float *radarLine2, float *pos);
bool __cdecl DoRadiiSurroundPoint(const float *radarPos, float radius1, float radius2, float *pos);
void __cdecl CG_CompassRadarPingEnemyPlayers(
                int localClientNum,
                float oldRadarProgress,
                float newRadarProgress,
                float oldLocalRadarProgress,
                float newLocalRadarProgress);
void __cdecl CG_CompassIncreaseRadarTime(int localClientNum);
void __cdecl CG_AddTurretWeaponPingInfo(int localClientNum, const centity_s *cent, const float *origin, int msec);
void __cdecl CG_CompassAddWeaponPingInfo(int localClientNum, const centity_s *cent, const float *origin, int msec);
void __cdecl CG_CompassAddVehicleWeaponPingInfo(
                int localClientNum,
                const centity_s *cent,
                const float *origin,
                int msec);
void __cdecl CG_CompassAddFakeFirePingInfo(
                int localClientNum,
                unsigned int ownerNum,
                int entNum,
                const float *origin,
                const float *dir,
                int msec);
CompassFakeFire *__cdecl CG_FindFakeFireActor(int localClientNum, int playerIndex, int entNum);
void __cdecl CG_AddEnemyScrambler(int localClientNum, centity_s *scramblerEnt);
void __cdecl CG_ClearNearestEnemyScrambler(int localClientNum);
void __cdecl CG_AddFriendlyScrambler(int localClientNum, float x, float y, int handle);
void __cdecl CG_RemoveFriendlyScrambler(int localClientNum, int handle);
void __cdecl CG_RemoveAllFriendlyScramblers(int localClientNum);
void __cdecl CG_CompassUpdateActors(int localClientNum);
void __cdecl CG_CompassDrawFriendlies(
                int localClientNum,
                CompassType compassType,
                const rectDef_s *parentRect,
                const rectDef_s *rect,
                float *color);
void __cdecl CG_AddArtilleryPing(int localClientNum, float *position);
void __cdecl CG_CompassDrawArtilleryIcon(
                int localClientNum,
                CompassType compassType,
                const rectDef_s *parentRect,
                const rectDef_s *rect,
                float *color);
void __cdecl CG_CompassDrawIconAtLocation(
                int localClientNum,
                CompassType compassType,
                const rectDef_s *parentRect,
                const rectDef_s *rect,
                float *color,
                float *vPos,
                Material *material);
void __cdecl CG_CompassDrawIncomingArtilleryIcon(
                int localClientNum,
                CompassType compassType,
                const rectDef_s *parentRect,
                const rectDef_s *rect,
                float *color);
void __cdecl GetPositionFromPackedBits(int localClientNum, char bitsPerAxis, int bitMask, int bits, float *vPos);
void __cdecl CG_CompassDrawEnemies(
                int localClientNum,
                CompassType compassType,
                const rectDef_s *parentRect,
                const rectDef_s *rect,
                float *color);
Material *__cdecl GetCompassPingMaterialForClient(int localClientNum, const clientInfo_t *clientInfo);
Material *__cdecl GetCompassFiringMaterialForClient(int localClientNum, const clientInfo_t *clientInfo);
void __cdecl CG_CompassDrawFakeFire(
                int localClientNum,
                CompassType compassType,
                const rectDef_s *parentRect,
                const rectDef_s *rect,
                float *color);
void __cdecl CG_CompassDrawRadarEffects(
                int localClientNum,
                CompassType compassType,
                const rectDef_s *parentRect,
                const rectDef_s *rect,
                float *color);
void __cdecl CG_CompassDrawFlicker(
                int localClientNum,
                CompassType compassType,
                const rectDef_s *parentRect,
                const rectDef_s *rect,
                float *color,
                Material *material);
void __cdecl CG_CompassDrawVehicles(
                int localClientNum,
                CompassType compassType,
                int eType,
                const rectDef_s *parentRect,
                const rectDef_s *rect,
                const float *color);
void __cdecl CG_CompassDrawDogs(
                int localClientNum,
                CompassType compassType,
                int eType,
                const rectDef_s *parentRect,
                const rectDef_s *rect,
                Material *dogMaterial,
                const float *color);
void __cdecl CG_CompassDrawTurrets(
                int localClientNum,
                CompassType compassType,
                int eType,
                const rectDef_s *parentRect,
                const rectDef_s *rect,
                const float *color);
Material *__cdecl CG_GetTurretCompassMaterial(centity_s *cent, bool friendly, bool isFiring);
void __cdecl CG_CompassDrawHelicopter(
                int localClientNum,
                CompassType compassType,
                int eType,
                const rectDef_s *parentRect,
                const rectDef_s *rect,
                const float *color);
void __cdecl CG_CompassDrawGuidedMissile(
                int localClientNum,
                CompassType compassType,
                int eType,
                const rectDef_s *parentRect,
                const rectDef_s *rect,
                const float *color);
void __cdecl CG_CompassDrawScramblerFriendly(
                int localClientNum,
                CompassType compassType,
                const rectDef_s *parentRect,
                const rectDef_s *rect,
                const float *color);
void __cdecl CG_ResetCompassData(int localClientNum, int time);
