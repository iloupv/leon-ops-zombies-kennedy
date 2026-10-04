#pragma once
#include "fx_draw.h"
#include <universal/com_math.h>

// local variable allocation has failed, the output may be wrong!
void __cdecl FX_Beam_GenerateVerts(FxGenerateVertsCmd *cmd);
void __cdecl CreateClipMatrix(float4x4 *clipMtx, const float *vieworg, const float (*viewaxis)[3]);
void __cdecl Float4x4ForViewer(float4x4 *mtx, const float *origin3, const float axis3[3][3]);
void __cdecl Float4x4InfinitePerspectiveMatrix(float4x4 *mtx, float tanHalfFovX, float tanHalfFovY, float zNear);
// local variable allocation has failed, the output may be wrong!
char    FX_GenerateBeam_GetFlatDelta(
                const float4x4 *clipMtx,
                const float4x4 *invClipMtx,
                const float4 *beamWorldBegin,
                const float4 *beamWorldEnd,
                float4 *outFlatDelta);
bool __cdecl Vec4HomogenousClipBothZ(float4 *pt0, float4 *pt1);
bool __cdecl Vec4HomogenousClipZW(float4 *pt0, float4 *pt1, const float4 *coeffZW);
double __cdecl I_fres(float val);
void __cdecl FX_Beam_Begin();
void __cdecl FX_Beam_Add(FxBeam *beam);
FxBeamInfo *__cdecl FX_Beam_GetInfo();
