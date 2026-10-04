#include "r_adszscale.h"
#include "rb_state.h"
#include "r_foliage.h"

int activeAdsZScale;

float adsZScale[2][1] = { { 1.0 }, { 1.0 } };

void __cdecl R_StartADSZScaleFrame()
{
    ++activeAdsZScale;
}

void __cdecl R_SetADSZScale(unsigned int viewport, float pos)
{
    adsZScale[viewport][(activeAdsZScale + 1) % 2] = pos;
}

void __cdecl R_SetADSZScaleConstants(unsigned int viewport, GfxCmdBufSourceState *source)
{
    R_SetCodeConstant(source,
        CONST_SRC_CODE_ADSZSCALE,
        adsZScale[viewport][activeAdsZScale % 2],
        0.0f,
        0.0f,
        0.0f
        );
}

