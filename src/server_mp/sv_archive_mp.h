#pragma once

struct playerState_s;
struct msg_t;
struct gentity_s;
struct clientState_s;
struct MatchState;

void __cdecl SV_ArchiveSnapshot(msg_t *msg);
gentity_s *__cdecl SV_GentityNumLocal(int num);
const clientState_s *__cdecl G_GetClientStateLocal(int clientNum);
const MatchState *__cdecl G_GetMatchStateLocal();
int __cdecl GetFollowPlayerStateLocal(int clientNum, playerState_s *ps);
