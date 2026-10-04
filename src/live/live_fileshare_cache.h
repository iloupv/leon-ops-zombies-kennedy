#pragma once
#include <demo/demo_common.h>
#include "live_storage.h"

struct __declspec(align(8)) fshCacheFileDetails_t // sizeof=0x840
{
    bdFileMetaData descriptor;
    fileShareLocation location;
    // padding byte
    // padding byte
    // padding byte
    // padding byte
};

struct __declspec(align(8)) fshCacheRatingDetails_t // sizeof=0xD8
{
    unsigned __int64 fileID;
    bdVoteRankStatsInfo rating;
    fileShareLocation location;
    // padding byte
    // padding byte
    // padding byte
    // padding byte
};

bool __cdecl Live_FileShare_CacheInit();
void __cdecl Live_FileShare_CacheShutdown();
int __cdecl Live_FileShare_Hash(unsigned __int64 fileID, fileShareLocation location);
char __cdecl Live_FileShare_IsCached(unsigned __int64 fileID, fileShareLocation location, int *index);
void __cdecl Live_FileShareCache_AddDescriptor(
                unsigned __int64 fileID,
                fileShareLocation location,
                bdFileMetaData *descriptor);
char __cdecl Live_FileShareCache_GetDescriptor(
                unsigned __int64 fileID,
                fileShareLocation location,
                bdFileMetaData *descriptor);
char __cdecl Live_FileShare_IsRatingCached(unsigned __int64 fileID, fileShareLocation location, int *index);
void __cdecl Live_FileShareCache_AddRating(
                unsigned __int64 fileID,
                fileShareLocation location,
                bdVoteRankStatsInfo *rating);
char __cdecl Live_FileShareCache_GetRating(
                unsigned __int64 fileID,
                fileShareLocation location,
                bdVoteRankStatsInfo *rating);
