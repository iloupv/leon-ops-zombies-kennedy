#pragma once
#include "fx_draw.h"

void __cdecl FX_SpriteGenerateVerts(FxGenerateVertsCmd *cmd);
void __cdecl FX_GenerateSpriteCodeMeshVerts(FxSprite *sprite, FxGenerateVertsCmd *cmd);
void __cdecl FX_GenerateSpriteCodeMeshVertsFixedScreenSize(
                Material *material,
                const float *pos,
                float radius,
                const unsigned __int8 *rgbaColor,
                char spriteFlags,
                FxGenerateVertsCmd *cmd);
void __cdecl FX_BuildSpriteCodeMeshVerts(
                Material *material,
                const float *pos,
                float worldRadius,
                const unsigned __int8 *rgbaColor,
                char spriteFlags);
void __cdecl FX_BuildQuadStampCodeMeshVerts(
                Material *material,
                const float *viewAxis,
                const float *origin,
                const float *left,
                const float *up,
                const unsigned __int8 *rgbaColor,
                int s0,
                int t0,
                int s1,
                int t1);
void __cdecl Byte4CopyRgbaToVertexColor(const unsigned __int8 *rgbaFrom, unsigned __int8 *nativeTo);
char __cdecl FX_HeightScreenToWorld(
                const float *worldOrigin,
                float screenHeight,
                float *worldHeight,
                FxGenerateVertsCmd *cmd);
double __cdecl FX_GetClipSpaceW(const float *worldPoint, float *vieworg, float (*viewaxis)[3]);
void __cdecl FX_GenerateSpriteCodeMeshVertsFixedWorldSize(
                Material *material,
                const float *pos,
                float radius,
                float minScreenRadius,
                const unsigned __int8 *rgbaColor,
                char spriteFlags,
                FxGenerateVertsCmd *cmd);
char __cdecl FX_HeightWorldToScreen(
                const float *worldOrigin,
                float worldHeight,
                float *screenHeight,
                FxGenerateVertsCmd *cmd);
void __cdecl FX_SpriteBegin();
void __cdecl FX_SpriteAdd(FxSprite *sprite);
FxSpriteInfo *__cdecl FX_SpriteGetInfo();
