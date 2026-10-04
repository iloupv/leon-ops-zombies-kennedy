#pragma once
#include <ddl/ddl_api.h>
#include "live_storage.h"
#include "live_pcache.h"
#include <bgame/bg_local.h>


void __cdecl PCache_ProfileInit();
ddlDef_t *__cdecl PCache_GetPublicProfileDDL();
void __cdecl PCache_GetPublicProfilesCompleted(TaskRecord *task);
void __cdecl PCache_GetPublicProfilesFailed(TaskRecord *task);
void __cdecl PCache_SetPublicProfileCompleted(TaskRecord *task);
void __cdecl PCache_SetPublicProfileFailed(TaskRecord *task);
void __cdecl PCache_GetPublicProfileCompleted(TaskRecord *task);
void __cdecl PCache_GetPublicProfileFailed(TaskRecord *task);
void __cdecl PCache_NukeProfile(int controlleridx);
void __cdecl PCache_BatchUpdatePublicProfiles(int controllerIndex, PCachePublicProfile *profiles, int count);
char __cdecl PCache_GetRankInternal(PCachePublicProfile *profile, int *rank, int *prestige);
char __cdecl PCache_GetRank(int controllerIndex, unsigned __int64 xuid, int *rank, int *prestige);
char __cdecl PCache_GetCodpoints(int controllerIndex, unsigned __int64 xuid, int *codpoints);
void __cdecl PCache_UpdateProfileData(int controllerIndex, PCachePublicProfile *profile);
void __cdecl PCache_GetProfileEmblem(
                PCachePublicProfile *profile,
                CompositeEmblemLayer *layers,
                int layerCount,
                __int16 *backgroundID);
double __cdecl GetItemFloat(const ddlState_t *state, char *buffer, const char *item, float min, float max, int range);
void __cdecl PCache_SetProfileEmblem(
                int controllerIndex,
                const CompositeEmblemLayer *layers,
                int layerCount,
                __int16 backgroundID);
void __cdecl SetItemFloat(
                const ddlState_t *state,
                char *buffer,
                const char *item,
                float value,
                float min,
                float max,
                int range);
