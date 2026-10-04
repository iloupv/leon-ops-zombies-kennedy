#pragma once
#include "flame_system.h"

struct flameChunkSpawnVars_t // sizeof=0x48
{                                       // XREF: ?Flame_Class_Stream_Fire_Chunks@@YAXPAUflameSource_t@@PAUflameWeaponConfig_t@@HPAUflameRender_s@@H@Z/r
    int time;                           // XREF: Flame_Class_Stream_Fire_Chunks(flameSource_t *,flameWeaponConfig_t *,int,flameRender_s *,int)+434/w
    int duration;                       // XREF: Flame_Class_Stream_Fire_Chunks(flameSource_t *,flameWeaponConfig_t *,int,flameRender_s *,int)+43A/w
    float origin[3];                    // XREF: Flame_Class_Stream_Fire_Chunks(flameSource_t *,flameWeaponConfig_t *,int,flameRender_s *,int)+48D/w
                                        // Flame_Class_Stream_Fire_Chunks(flameSource_t *,flameWeaponConfig_t *,int,flameRender_s *,int)+4A7/w ...
    float angle[3];                     // XREF: Flame_Class_Stream_Fire_Chunks(flameSource_t *,flameWeaponConfig_t *,int,flameRender_s *,int)+519/w
                                        // Flame_Class_Stream_Fire_Chunks(flameSource_t *,flameWeaponConfig_t *,int,flameRender_s *,int)+539/w ...
    float speed;                        // XREF: Flame_Class_Stream_Fire_Chunks(flameSource_t *,flameWeaponConfig_t *,int,flameRender_s *,int)+5DB/w
    float decel;                        // XREF: Flame_Class_Stream_Fire_Chunks(flameSource_t *,flameWeaponConfig_t *,int,flameRender_s *,int)+5E8/w
    float gravityStart;                 // XREF: Flame_Class_Stream_Fire_Chunks(flameSource_t *,flameWeaponConfig_t *,int,flameRender_s *,int)+5F4/w
    float gravityEnd;                   // XREF: Flame_Class_Stream_Fire_Chunks(flameSource_t *,flameWeaponConfig_t *,int,flameRender_s *,int)+601/w
    float sizeMax;                      // XREF: Flame_Class_Stream_Fire_Chunks(flameSource_t *,flameWeaponConfig_t *,int,flameRender_s *,int)+695/w
    float sizeStart;                    // XREF: Flame_Class_Stream_Fire_Chunks(flameSource_t *,flameWeaponConfig_t *,int,flameRender_s *,int)+65F/w
                                        // Flame_Class_Stream_Fire_Chunks(flameSource_t *,flameWeaponConfig_t *,int,flameRender_s *,int)+69F/r
    float sizeEnd;                      // XREF: Flame_Class_Stream_Fire_Chunks(flameSource_t *,flameWeaponConfig_t *,int,flameRender_s *,int)+688/w
                                        // Flame_Class_Stream_Fire_Chunks(flameSource_t *,flameWeaponConfig_t *,int,flameRender_s *,int)+69A/r
    float sizeRate;                     // XREF: Flame_Class_Stream_Fire_Chunks(flameSource_t *,flameWeaponConfig_t *,int,flameRender_s *,int)+6AD/w
    float spawnFireIntervalStart;
    float spawnFireIntervalEnd;
};

void __cdecl Flame_Class_Chunk_Init();
flameChunk_s *__cdecl Flame_Class_Chunk_Alloc(bool is_server);
void __cdecl Flame_Class_Chunk_Free(bool is_server, flameChunk_s *chunk);
void __cdecl Flame_Phys_Spawn_Items(bool is_server, int curTime, flameChunk_s *trav);
void __cdecl Flame_Class_Chunk_Age(bool is_server, int time);
flameChunk_s *__cdecl Flame_Class_Chunk_Spawn(
                bool is_server,
                flameChunkSpawnVars_t *spawnVars,
                flameGeneric_s **chunkList,
                float *velocityAdd,
                flameTable *fTable);
void __cdecl Flame_Class_Chunk_Render_Item(int localClientNum, flameChunk_s *chunk);
void __cdecl Flame_Class_Chunk_Render_All(int localClientNum);
