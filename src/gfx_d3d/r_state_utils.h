#pragma once

#include <d3d9.h>

enum MaterialUpdateFrequency : __int32;


struct GfxCmdBufSourceState;
struct GfxCmdBufInput;
struct GfxCmdBufState;
struct GfxViewParms;
struct GfxSceneDef;
struct GfxMatrix;
struct GfxViewport;

void __cdecl R_InitCmdBufSourceState(GfxCmdBufSourceState *source, const GfxCmdBufInput *input, int cameraView);
void __cdecl R_InitCmdBufState(GfxCmdBufState *state);
void __cdecl R_HW_ForceSamplerState(IDirect3DDevice9 *device, unsigned int samplerIndex, unsigned int samplerState);
void __cdecl R_SetDefaultAlphaTestFunction(GfxCmdBufState *state);
unsigned int __cdecl R_DecodeSamplerState(unsigned __int8 samplerState);
void __cdecl R_SetDefaultStateBits(unsigned int *stateBits);
void __cdecl R_InitCmdBufSamplerState(GfxCmdBufState *state);
GfxCmdBufSourceState *__cdecl R_GetActiveWorldMatrix(GfxCmdBufSourceState *source);
void __cdecl R_WorldMatrixChanged(GfxCmdBufSourceState *source);
void __cdecl R_Set2D(GfxCmdBufSourceState *source);
// local variable allocation has failed, the output may be wrong!
void    R_CmdBufSet2D(GfxCmdBufSourceState *source, GfxViewport *viewport);
void __cdecl R_MatrixIdentity44(float (*out)[4]);
void __cdecl R_CmdBufSet3D(GfxCmdBufSourceState *source);
void __cdecl R_Set3D(GfxCmdBufSourceState *source);
void __cdecl R_BeginView(GfxCmdBufSourceState *source, const GfxSceneDef *sceneDef, const GfxViewParms *viewParms);
void __cdecl R_DeriveNearPlaneConstantsForView(GfxCmdBufSourceState *source);
void __cdecl R_SetShadowLookupMatrix(GfxCmdBufSourceState *source, const GfxMatrix *matrix);
