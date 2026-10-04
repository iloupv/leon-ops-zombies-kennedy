#pragma once
#include "teams.h"
#include "enthandle.h"
#include "pathnode.h"

#define SENTIENT_TEAM_DEAD 3
// mod: MAX_SENTIENTS is the runtime sentient count, g_maxSentients = 48 + (g_maxActors - 32); default 48 = retail
#define MAX_SENTIENTS_RETAIL 48
#define MAX_SENTIENTS_CAP (MAX_SENTIENTS_RETAIL + 1024 - 32) // mod: MAX_ACTORS_CAP 1024 (L25)
extern int g_maxSentients;
#define MAX_SENTIENTS (g_maxSentients)

struct vis_cache_t // sizeof=0xC
{                                       // XREF: sentient_info_t/r
    bool bVisible;
    // padding byte
    // padding byte
    // padding byte
    int iLastUpdateTime;
    int iLastVisTime;
};

struct sentient_info_t // sizeof=0x28
{                                       // XREF: actor_s/r
    vis_cache_t VisCache;
    int iLastAttackMeTime;
    int lastKnownPosTime;
    int attackTime;
    float vLastKnownPos[3];
    pathnode_t *pLastKnownNode;
};

struct __declspec(align(4)) sentient_s // sizeof=0x90
{                                       // XREF: sentient_t/r
    gentity_s *ent;
    team_t eTeam;
    EntHandle scriptOwner;
    int iThreatBias;
    int iThreatBiasGroupIndex;
    bool bIgnoreMe;
    bool bIgnoreAll;
    bool bIgnoreForFriendlyFire;
    bool originChanged;
    float oldOrigin[3];
    float maxVisibleDist;
    int iEnemyNotifyTime;
    int attackerCount;
    EntHandle lastAttacker;
    EntHandle syncedMeleeEnt;
    EntHandle targetEnt;
    EntHandle scriptTargetEnt;
    unsigned __int16 scriptTargetTag;
    // padding byte
    // padding byte
    float entityTargetThreat;
    int meleeAttackerSpot[4];
    float attackerAccuracy;
    bool ignoreRandomBulletDamage;
    bool turretInvulnerability;
    // padding byte
    // padding byte
    pathnode_t *pClaimedNode;
    pathnode_t *pPrevClaimedNode;
    pathnode_t *pActualChainPos;
    int iActualChainPosTime;
    pathnode_t *pNearestNode;
    unsigned __int8 bNearestNodeValid;
    unsigned __int8 bNearestNodeBad;
    // padding byte
    // padding byte
    float vNearestNodeCheckPos[3];
    bool inuse;
    // padding byte
    // padding byte
    // padding byte
    int banNodeTime;
    bool bInMeleeCharge;
    // padding byte
    // padding byte
    // padding byte
};

sentient_s *__cdecl Sentient_Alloc();
void __cdecl Sentient_Free(sentient_s *sentient);
void __cdecl Sentient_Dissociate(sentient_s *pSentient);
void __cdecl Sentient_DissociateSentient(sentient_s *self, sentient_s *other);
void __fastcall Sentient_GetOrigin(const sentient_s *self, float *vOriginOut);
void __fastcall Sentient_GetVelocity(const sentient_s *self, float *vVelOut);
void __fastcall Sentient_GetEyePosition(const sentient_s *self, float *vEyePosOut);
void __fastcall Sentient_GetThirdPersonEyePosition(const sentient_s *self, float *vEyePosOut);
void __fastcall Sentient_GetDebugEyePosition(const sentient_s *self, float *vEyePosOut);
pathnode_t *__fastcall Sentient_NearestNode(sentient_s *self);
pathnode_t *__fastcall Sentient_NearestNodeSuppressed(
                sentient_s *self,
                float (*vNormal)[2],
                float *fDist,
                int iPlaneCount);
void __fastcall Sentient_InvalidateNearestNode(sentient_s *self);
void __fastcall Sentient_SetEnemy(sentient_s *self, gentity_s *enemy, int bNotify);
sentient_s *__fastcall Sentient_FirstSentient(int iTeamFlags);
sentient_s *__fastcall Sentient_NextSentient(sentient_s *pPrevSentient, int iTeamFlags);
const char *__fastcall Sentient_NameForTeam(unsigned int eTeam);
void __fastcall Sentient_SetTeam(sentient_s *self, team_t eTeam);
void __fastcall Sentient_ClaimNode(sentient_s *self, pathnode_t *node);
void __fastcall Sentient_NodeClaimRevoked(sentient_s *self, pathnode_t *node);
void __fastcall Sentient_StealClaimNode(sentient_s *self, sentient_s *other);
void __fastcall Sentient_BanNearNodes(sentient_s *self);
void __cdecl G_InitSentients();
struct MemoryFile;
void Sentient_WriteGlobals(MemoryFile *memFile);
void Sentient_ReadGlobals(MemoryFile *memFile);
bool __fastcall Sentient_NearestNodeDirty(sentient_s *self, bool originChanged);
