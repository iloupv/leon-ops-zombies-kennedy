#pragma once
#include <gfx_d3d/r_primarylights.h>

struct ComWaterHeader // sizeof=0x10
{                                       // XREF: ComWorld/r
    int minx;                           // XREF: Com_LoadWaterData(void)+6F/w
                                        // Com_LoadWaterData(void)+A1/w ...
    int miny;                           // XREF: Com_LoadWaterData(void)+83/w
                                        // Com_LoadWaterData(void)+B9/w ...
    int maxx;                           // XREF: Com_LoadWaterData(void)+79/w
                                        // Com_LoadWaterData(void)+AD/w ...
    int maxy;                           // XREF: Com_LoadWaterData(void)+8D/w
                                        // Com_LoadWaterData(void)+C4/w ...
};

struct ComWaterCell // sizeof=0x8
{                                       // XREF: R_WaterSimulationFillTile/r
                                        // R_WaterSimulationFillTile/r
    __int16 waterheight;                // XREF: R_WaterSimulationFillTile+69/w
                                        // R_WaterSimulationFillTile+290/w ...
    char flooroffset;                   // XREF: R_WaterSimulationFillTile+6D/w
                                        // R_WaterSimulationFillTile+2E4/w ...
    unsigned __int8 shoredist;          // XREF: R_WaterSimulationFillTile+60/w
                                        // R_WaterSimulationFillTile+337/w ...
    unsigned __int8 color[4];           // XREF: R_WaterSimulationFillTile+50/w
                                        // R_WaterSimulationFillTile+54/w ...
};

struct ComBurnableHeader // sizeof=0x10
{                                       // XREF: ComWorld/r
    int minx;                           // XREF: Com_LoadBurnableData(void)+6F/w
                                        // Com_LoadBurnableData(void)+A1/w
    int miny;                           // XREF: Com_LoadBurnableData(void)+83/w
                                        // Com_LoadBurnableData(void)+B9/w
    int maxx;                           // XREF: Com_LoadBurnableData(void)+79/w
                                        // Com_LoadBurnableData(void)+AD/w
    int maxy;                           // XREF: Com_LoadBurnableData(void)+8D/w
                                        // Com_LoadBurnableData(void)+C4/w
};

struct ComBurnableSample // sizeof=0x1
{
    unsigned __int8 state;
};

struct ComBurnableCell // sizeof=0xC
{
    int x;
    int y;
    ComBurnableSample *data;
};

struct ComWorld // sizeof=0x40
{                                       // XREF: .data:ComWorld comWorld/r
    const char *name;                   // XREF: Com_LoadWorld_LoadObj+50/w
                                        // UIViewer::LoadMap(void)+44/r ...
    int isInUse;                        // XREF: Com_GetPrimaryLightCount(void)+3/r
                                        // Com_GetPrimaryLight(uint)+3/r ...
    unsigned int primaryLightCount;     // XREF: Com_GetPrimaryLightCount(void):loc_4AB85F/r
                                        // Com_GetPrimaryLight(uint)+32/r ...
    ComPrimaryLight *primaryLights;     // XREF: Com_GetPrimaryLight(uint)+6B/r
                                        // Com_LoadPrimaryLights+80/w ...
    ComWaterHeader waterHeader;         // XREF: Com_LoadWaterData(void)+6F/w
                                        // Com_LoadWaterData(void)+79/w ...
    unsigned int numWaterCells;         // XREF: Com_LoadWaterData(void)+5B/w
                                        // Com_LoadWaterData(void)+E0/w ...
    ComWaterCell *waterCells;           // XREF: Com_LoadWaterData(void)+65/w
                                        // Com_LoadWaterData(void)+FC/w ...
    ComBurnableHeader burnableHeader;   // XREF: Com_LoadBurnableData(void)+6F/w
                                        // Com_LoadBurnableData(void)+79/w ...
    unsigned int numBurnableCells;      // XREF: Com_LoadBurnableData(void)+5B/w
                                        // Com_LoadBurnableData(void)+E0/w ...
    ComBurnableCell *burnableCells;     // XREF: Com_LoadBurnableData(void)+65/w
                                        // Com_LoadBurnableData(void)+FC/w ...
};

char __cdecl Com_CanPrimaryLightAffectPoint(const ComPrimaryLight *light, const float *point);
double __cdecl CosOfSumOfArcCos(float cos0, float cos1);
void __cdecl Com_UnloadWorld();

extern ComWorld comWorld;