#pragma once
#include "rb_state.h"

struct GfxShaderConstantState_s
{
    MaterialArgumentCodeConst codeConst;
    unsigned int version;
};

union GfxShaderConstantState // sizeof=0x8
{                                       // XREF: R_IsShaderMatrixUpToDate+C/w
    GfxShaderConstantState_s fields;
    unsigned __int64 packed;
};

void __cdecl R_SetupPassPerPrimArgs(GfxCmdBufContext context);
void __cdecl R_SetPassShaderPrimArguments(const GfxCmdBufContext context, unsigned int argCount, const MaterialShaderArgument *arg);
void __cdecl R_SetVertexShaderConstantFromCode_Old(GfxCmdBufContext context, const MaterialShaderArgument *routingData);
float *__cdecl R_GetCodeConstant(const GfxCmdBufContext context, unsigned int constant);
char __cdecl R_IsVertexShaderConstantUpToDate(GfxCmdBufContext context, const MaterialShaderArgument *routingData);
bool __cdecl R_IsShaderMatrixUpToDate(
                const GfxCmdBufSourceState *source,
                GfxShaderConstantState *constant,
                const MaterialShaderArgument *routingData);
bool __cdecl R_IsShaderConstantUpToDate(
                const GfxCmdBufSourceState *source,
                GfxShaderConstantState *constant,
                const MaterialShaderArgument *routingData);
void __cdecl R_SetVertexShaderConstantFromCode_New(const GfxCmdBufContext context, const MaterialShaderArgument *routingData);
void __cdecl R_SetupPassPerObjectArgs(GfxCmdBufContext context);
void __cdecl R_SetPassShaderObjectArguments(const GfxCmdBufContext context, unsigned int argCount, const MaterialShaderArgument *arg);
void __cdecl R_SetPassPixelShaderStableArguments( const GfxCmdBufContext context, unsigned int argCount, const MaterialShaderArgument *arg);
void __cdecl R_SetPixelShaderConstantFromLiteral(GfxCmdBufState *state, unsigned int dest, const float *literal);
void __cdecl R_HW_SetPixelShaderConstant(
                IDirect3DDevice9 *device,
                unsigned int dest,
                const float *data,
                unsigned int rowCount);
int __cdecl R_IsPixelShaderConstantUpToDate(const GfxCmdBufContext context, const MaterialShaderArgument *routingData);
void __cdecl R_SetPixelShaderConstantFromCode(const GfxCmdBufContext context, const MaterialShaderArgument *routingData);
void __cdecl R_SetupPassCriticalPixelShaderArgs(GfxCmdBufContext context);
void __cdecl R_SetupPassVertexShaderArgs(GfxCmdBufContext context);
void __cdecl R_SetPassVertexShaderStableArguments(const GfxCmdBufContext context, unsigned int argCount, const MaterialShaderArgument *arg);
const MaterialTextureDef *__cdecl R_SetPixelSamplerFromMaterial(
                const GfxCmdBufContext context,
                const MaterialShaderArgument *arg,
                const MaterialTextureDef *texDef);
void __cdecl R_OverrideImage(GfxImage **image, const MaterialTextureDef *texdef);
void __cdecl R_SetPassShaderStableArguments(
                const GfxCmdBufContext context,
                unsigned int argCount,
                const MaterialShaderArgument *arg);
void __cdecl R_SetVertexShaderConstantFromLiteral(GfxCmdBufState *state, unsigned int dest, const float *literal);
int __cdecl R_ReserveIndexData(GfxCmdBufPrimState *state, int triCount);
int __cdecl R_SetIndexData(GfxCmdBufPrimState *state, unsigned __int8 *indices, int triCount);
void __cdecl R_SetPixelShader(GfxCmdBufState *state, const MaterialPixelShader *pixelShader);
void __cdecl R_UpdateVertexDecl(GfxCmdBufState *state);
void __cdecl R_SetVertexShader(GfxCmdBufState *state, const MaterialVertexShader *vertexShader);
void __cdecl R_SetupPass(GfxCmdBufContext context, unsigned int passIndex);
void __cdecl R_SetState(GfxCmdBufState *state, unsigned int *stateBits);
int __cdecl R_SetVertexData(GfxCmdBufState *state, const void *data, int vertexCount, int stride);
