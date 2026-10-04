#pragma once
#include "r_material.h"
#include "rb_state.h"

enum GfxPixelCostMode : __int32
{                                       // XREF: $24C327571DE1B1A743908CE1FB1197BD/r
    GFX_PIXEL_COST_MODE_OFF          = 0x0,
    GFX_PIXEL_COST_MODE_MEASURE_COST = 0x1,
    GFX_PIXEL_COST_MODE_MEASURE_MSEC = 0x2,
    GFX_PIXEL_COST_MODE_ADD_COST_USE_DEPTH = 0x3,
    GFX_PIXEL_COST_MODE_ADD_COST_IGNORE_DEPTH = 0x4,
    GFX_PIXEL_COST_MODE_ADD_PASSES_USE_DEPTH = 0x5,
    GFX_PIXEL_COST_MODE_ADD_PASSES_IGNORE_DEPTH = 0x6,
};

union GfxPixelCostKey // sizeof=0x8
{                                       // XREF: R_PixelCost_PackedKeyForMaterial+35/w
                                        // R_PixelCost_PackedKeyForMaterial+3B/w ...
    //GfxPixelCostKey::<unnamed_type_mtl> mtl;
    struct
    {
        const Material *material;
        unsigned __int8 techType;
    } mtl;
    unsigned __int64 packed;
};

struct GfxPixelCostRecord // sizeof=0x20
{                                       // XREF: $24C327571DE1B1A743908CE1FB1197BD/r
    unsigned __int16 costHistory[12];   // XREF: RB_PixelCost_GetCostForRecordIndex+35/r
                                        // R_PixelCost_EndSurface(GfxCmdBufContext)+129/w ...
    GfxPixelCostKey key;
};

const Material *__cdecl R_PixelCost_GetAccumulationMaterial(const Material *material);
void __cdecl R_PixelCost_BeginSurface(GfxCmdBufContext context);
void __cdecl R_PixelCost_SetConstant(GfxCmdBufSourceState *source, int cost);
int __cdecl RB_PixelCost_GetCostForRecordIndex(int recordIndex);
unsigned __int64 __cdecl R_PixelCost_PackedKeyForMaterial(const Material *material, unsigned __int8 techType);
bool __cdecl RB_PixelCost_DoesPrimMatch(unsigned __int64 packedKey);
void __cdecl RB_PixelCost_ResetPrim(unsigned __int64 packedKey);
void __cdecl R_PixelCost_EndSurface(GfxCmdBufContext context);
int RB_PixelCost_AccumulateMsec();
GfxRenderTargetId __cdecl RB_PixelCost_OverrideRenderTarget(GfxRenderTargetId targetId);

extern GfxPixelCostMode pixelCostMode;
