#pragma once

struct GfxCmdBufSourceState;

void __cdecl R_StartADSZScaleFrame();
void __cdecl R_SetADSZScale(unsigned int viewport, float pos);
void __cdecl R_SetADSZScaleConstants(unsigned int viewport, GfxCmdBufSourceState *source);
