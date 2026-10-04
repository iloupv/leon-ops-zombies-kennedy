#pragma once
#include <bgame/bg_public.h>
#include <bgame/bg_local.h>

struct corpseInfo_t // sizeof=0x5E0
{
    XAnimTree_s *tree;
    int entnum;
    int time;
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    clientInfo_t ci;
    bool falling;
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
};

int __cdecl G_GetPlayerCorpseIndex(gentity_s *ent, const char *error_msg);
int __cdecl G_GetFreePlayerCorpseIndex();
void __cdecl PlayerCorpse_Free(gentity_s *ent);
void __cdecl G_RunCorpseMove(gentity_s *ent);
void __cdecl G_BounceCorpse(gentity_s *ent, corpseInfo_t *corpseInfo, trace_t *trace, float *endpos);
char __cdecl G_GetAnimDeltaForCorpse(gentity_s *ent, float *originChange);
void __cdecl G_RunCorpse(gentity_s *ent);
void __cdecl G_RunCorpseAnimate(gentity_s *ent);
