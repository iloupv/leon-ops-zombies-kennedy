#pragma once
#include "flame_system.h"

struct flameDrips_t // sizeof=0x64
{
    flameGeneric_s gen;
    flameRender_s *flameRend;
};

void __cdecl Flame_Class_Drips_Init();
flameDrips_t *__cdecl Flame_Class_Drips_Alloc(bool is_server);
void __cdecl Flame_Class_Drips_Free(bool is_server, flameDrips_t *fire);
void __cdecl Flame_Class_Drips_Free_If_Stream(bool is_server, flameStream_s *stream);
void __cdecl Flame_Class_Drips_Age(bool is_server, int time);
flameDrips_t *__cdecl Flame_Class_Drips_Spawn(bool is_server, flameChunk_s *fromChunk, int curTime, float minLifeFrac);
void __cdecl Flame_Class_Drips_Render_Item(int localClientNum, flameDrips_t *fire);
void __cdecl Flame_Class_Drips_Render_All(int localClientNum);
void __cdecl Flame_Class_Drips_Render_Local_List(int localClientNum, flameDrips_t *start);
