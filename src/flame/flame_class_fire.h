#pragma once

#include "flame_system.h"

struct flameFire_t // sizeof=0x64
{
    flameGeneric_s gen;
    flameRender_s *flameRend;
};

void __cdecl Flame_Class_Fire_Init();
flameFire_t *__cdecl Flame_Class_Fire_Alloc();
void __cdecl Flame_Class_Fire_Free(bool is_server, flameFire_t *fire);
void __cdecl Flame_Class_Fire_Age(bool is_server, int time);
flameFire_t *__cdecl Flame_Class_Fire_Spawn(bool is_server, flameChunk_s *fromChunk, int curTime, float minLifeFrac);
void __cdecl Flame_Class_Fire_Render_Item(int localClientNum, flameFire_t *fire);
void __cdecl Flame_Class_Fire_Render_All(int localClientNum);
void __cdecl Flame_Class_Fire_Render_Local_List(int localClientNum, flameFire_t *start);
