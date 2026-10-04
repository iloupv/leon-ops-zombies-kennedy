#pragma once
#include <bgame/bg_public.h>

int __cdecl G_GetActorCorpseIndex(gentity_s *ent);
int __cdecl G_GetFreeActorCorpseIndex(int reuse);
void __cdecl G_RemoveOneActorCorpse();
void __cdecl G_RemoveAllActorCorpses();
void __cdecl G_RemoveActorCorpses(unsigned int allowedCorpseCount);
void __cdecl G_UpdateActorCorpses();
void __cdecl ActorCorpse_Free(gentity_s *ent);
void __cdecl Actor_GetBodyPlantAngles(
                int iEntNum,
                int iClipMask,
                const float *vOrigin,
                float fYaw,
                float *pfPitch,
                float *pfRoll,
                float *pfHeight);
double __cdecl Actor_SetBodyPlantAngle(
                int iEntNum,
                int iClipMask,
                const float *vOrigin,
                const float *vCenter,
                const float *vDir,
                float *pfAngle);
void __cdecl Actor_OrientCorpseToGround(gentity_s *self, int bLerp);
void __cdecl Actor_OrientPitchToGround(gentity_s *self, int bLerp);
int __cdecl Actor_BecomeCorpse(gentity_s *self);
int __cdecl G_CorpseFromActor(gentity_s *ent);
int __cdecl G_FindActorCorpseSlot(int entnum);
struct XAnimTree_s *__cdecl G_GetActorCorpseTree(gentity_s *ent);
void __cdecl ActorCorpse_Think(gentity_s *ent);
