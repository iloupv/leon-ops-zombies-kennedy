#pragma once

#include <physics/rope.h>

struct RopeRenderInfo // sizeof=0xC04
{                                       // XREF: .data:g_ropeInfo/r
    VisualRope ropes[96];
    int ropeCount;                      // XREF: R_Rope_ClearAll(void)+3/w
                                        // R_Rope_Add(VisualRope const *)+6/r ...
};

struct RopeGenerateVertsCmd // sizeof=0x40
{                                       // XREF: ?CG_GenerateSceneCodeMeshes@@YAXH@Z/r
    RopeRenderInfo *ropeInfo;           // XREF: CG_GenerateSceneCodeMeshes(int)+2F/w
    int localClientNum;                 // XREF: CG_GenerateSceneCodeMeshes(int)+27/w
    float vieworg[3];                   // XREF: CG_GenerateSceneCodeMeshes(int)+54/w
                                        // CG_GenerateSceneCodeMeshes(int)+61/w ...
    float viewaxis[3][3];               // XREF: CG_GenerateSceneCodeMeshes(int)+73/o
    float refdef_tanHalfFovX;           // XREF: CG_GenerateSceneCodeMeshes(int)+94/w
    float refdef_tanHalfFovY;           // XREF: CG_GenerateSceneCodeMeshes(int)+A4/w
};

void __cdecl R_Rope_GenerateVerts_Camera(RopeGenerateVertsCmd *cmd);
void __cdecl R_Rope_GenerateVerts_Internal(
                const RopeGenerateVertsCmd *cmd,
                char sceneIndex,
                unsigned int drawsurfRegion,
                bool forceLowResLOD);
void __cdecl R_Rope_ClearAll();
void __cdecl R_Rope_Add(const VisualRope *vropeIn);
RopeRenderInfo *__cdecl R_Rope_GetInfo();
unsigned __int8 __cdecl R_Rope_CalcVisibility(const float *minmax, unsigned int localClientNum);
const Material *__cdecl R_Rope_GetMaterialFromGfxWorld();
