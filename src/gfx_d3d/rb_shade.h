#pragma once
#include "r_material.h"
#include "rb_state.h"
#include "rb_tess.h"

void __cdecl RB_ClearPixelShader();
void __cdecl R_HW_SetPixelShader(IDirect3DDevice9 *device, const MaterialPixelShader *mtlShader);
void __cdecl RB_ClearVertexShader();
void __cdecl R_HW_SetVertexShader(IDirect3DDevice9 *device, const MaterialVertexShader *mtlShader);
void __cdecl RB_ClearVertexDecl();
void __cdecl R_SetVertexDecl(GfxCmdBufPrimState *primState, const MaterialVertexDeclaration *vertexDecl);
void __cdecl R_BeginPixMaterial(GfxCmdBufState *state);
void __cdecl R_EndPixMaterial(GfxCmdBufState *state);
void __cdecl R_EndPixMaterialInternal(GfxCmdBufState *state);
void __cdecl R_SetPixPrimarySortKey(GfxCmdBufState *state, unsigned int primarySortKey);
void __cdecl R_BeginPixMaterials(GfxCmdBufState *state);
void __cdecl R_EndPixMaterials(GfxCmdBufState *state);
void __cdecl RB_SetTessTechnique(const Material *material, unsigned __int8 techType);
void __cdecl RB_BeginSurface(const Material *material, unsigned __int8 techType);
void __cdecl RB_EndTessSurface();
GfxPrimStats *RB_EndSurfacePrologue();
void RB_EndSurfaceEpilogue();
void RB_DrawTessSurface();
void __cdecl R_DrawTessTechnique(GfxCmdBufContext context, const GfxDrawPrimArgs *args);
void __cdecl R_CheckVertexDataOverflow(int dataSize);
void __cdecl R_SetStreamSource(
                GfxCmdBufPrimState *primState,
                IDirect3DVertexBuffer9 *vb,
                unsigned int vertexOffset,
                unsigned int vertexStride);
void __cdecl RB_TessOverflow();
