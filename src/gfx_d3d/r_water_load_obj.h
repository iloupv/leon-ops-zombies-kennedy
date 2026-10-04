#pragma once

struct water_t;

water_t *__cdecl R_LoadWaterSetup(const water_t *water);
bool __cdecl R_WatersEquivalent(const water_t *w0, const water_t *w1);
void __cdecl R_CreateWaterSetup(const water_t *source, int waterMapSetupIndex, water_t *destination);
void __cdecl R_PickWaterFrequencies(water_t *water);
void __cdecl R_InitLoadWater();
void __cdecl R_ShutdownLoadWater();
