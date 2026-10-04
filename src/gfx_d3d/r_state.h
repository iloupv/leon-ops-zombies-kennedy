#pragma once
#include "rb_tess.h"
#include "r_state_utils.h"
#include "r_material_load_obj.h"

enum CodeConstant : __int32 // not a real enum name
{
    CONST_SRC_CODE_MAYBE_DIRTY_PS_BEGIN = 0,
    CONST_SRC_CODE_LIGHT_POSITION = 0,
    CONST_SRC_CODE_LIGHT_DIFFUSE = 1,
    CONST_SRC_CODE_LIGHT_SPECULAR = 2,
    CONST_SRC_CODE_LIGHT_SPOTDIR = 3,
    CONST_SRC_CODE_LIGHT_SPOTFACTORS = 4,
    CONST_SRC_CODE_LIGHT_ATTENUATION = 5,
    CONST_SRC_CODE_LIGHT_FALLOFF_A = 6,
    CONST_SRC_CODE_LIGHT_FALLOFF_B = 7,
    CONST_SRC_CODE_LIGHT_SPOT_MATRIX0 = 8,
    CONST_SRC_CODE_LIGHT_SPOT_MATRIX1 = 9,
    CONST_SRC_CODE_LIGHT_SPOT_MATRIX2 = 10,
    CONST_SRC_CODE_LIGHT_SPOT_MATRIX3 = 11,
    CONST_SRC_CODE_LIGHT_SPOT_AABB = 12,
    CONST_SRC_CODE_LIGHT_CONE_CONTROL1 = 13,
    CONST_SRC_CODE_LIGHT_CONE_CONTROL2 = 14,
    CONST_SRC_CODE_LIGHT_SPOT_COOKIE_SLIDE_CONTROL = 15,
    CONST_SRC_CODE_NEARPLANE_ORG = 16,
    CONST_SRC_CODE_NEARPLANE_DX = 17,
    CONST_SRC_CODE_NEARPLANE_DY = 18,
    CONST_SRC_CODE_SHADOW_PARMS = 19,
    CONST_SRC_CODE_SHADOWMAP_POLYGON_OFFSET = 20,
    CONST_SRC_CODE_RENDER_TARGET_SIZE = 21,
    CONST_SRC_VPOSX_TO_WORLD = 22,
    CONST_SRC_VPOSY_TO_WORLD = 23,
    CONST_SRC_VPOS1_TO_WORLD = 24,
    CONST_SRC_CODE_LIGHT_FALLOFF_PLACEMENT = 25,
    CONST_SRC_CODE_DOF_EQUATION_VIEWMODEL_AND_FAR_BLUR = 26,
    CONST_SRC_CODE_DOF_EQUATION_SCENE = 27,
    CONST_SRC_CODE_DOF_LERP_SCALE = 28,
    CONST_SRC_CODE_DOF_LERP_BIAS = 29,
    CONST_SRC_CODE_DOF_ROW_DELTA = 30,
    CONST_SRC_CODE_PARTICLE_CLOUD_COLOR = 31,
    CONST_SRC_CODE_GAMETIME = 32,
    CONST_SRC_CODE_ALPHA_FADE = 33,
    CONST_SRC_CODE_MAYBE_DIRTY_PS_END = 34,
    CONST_SRC_CODE_ALWAYS_DIRTY_PS_BEGIN = 34,
    CONST_SRC_CODE_PIXEL_COST_FRACS = 34,
    CONST_SRC_CODE_PIXEL_COST_DECODE = 35,
    CONST_SRC_CODE_FILTER_TAP_0 = 36,
    CONST_SRC_CODE_FILTER_TAP_1 = 37,
    CONST_SRC_CODE_FILTER_TAP_2 = 38,
    CONST_SRC_CODE_FILTER_TAP_3 = 39,
    CONST_SRC_CODE_FILTER_TAP_4 = 40,
    CONST_SRC_CODE_FILTER_TAP_5 = 41,
    CONST_SRC_CODE_FILTER_TAP_6 = 42,
    CONST_SRC_CODE_FILTER_TAP_7 = 43,
    CONST_SRC_CODE_COLOR_MATRIX_R = 44,
    CONST_SRC_CODE_COLOR_MATRIX_G = 45,
    CONST_SRC_CODE_COLOR_MATRIX_B = 46,
    CONST_SRC_CODE_ALWAYS_DIRTY_PS_END = 47,
    CONST_SRC_CODE_NEVER_DIRTY_PS_BEGIN = 47,
    CONST_SRC_CODE_SHADOWMAP_SWITCH_PARTITION = 47,
    CONST_SRC_CODE_SHADOWMAP_SCALE = 48,
    CONST_SRC_CODE_ZNEAR = 49,
    CONST_SRC_CODE_SUN_POSITION = 50,
    CONST_SRC_CODE_SUN_DIFFUSE = 51,
    CONST_SRC_CODE_SUN_SPECULAR = 52,
    CONST_SRC_CODE_LIGHTING_LOOKUP_SCALE = 53,
    CONST_SRC_CODE_DEBUG_BUMPMAP = 54,
    CONST_SRC_CODE_MATERIAL_COLOR = 55,
    CONST_SRC_CODE_FOG = 56,
    CONST_SRC_CODE_FOG2 = 57,
    CONST_SRC_CODE_FOG_COLOR = 58,
    CONST_SRC_CODE_SUN_FOG = 59,
    CONST_SRC_CODE_SUN_FOG_DIR = 60,
    CONST_SRC_CODE_SUN_FOG_COLOR = 61,
    CONST_SRC_CODE_GLOW_SETUP = 62,
    CONST_SRC_CODE_GLOW_APPLY = 63,
    CONST_SRC_CODE_COLOR_BIAS = 64,
    CONST_SRC_CODE_COLOR_TINT_BASE = 65,
    CONST_SRC_CODE_COLOR_TINT_DELTA = 66,
    CONST_SRC_CODE_OUTDOOR_FEATHER_PARMS = 67,
    CONST_SRC_CODE_SKY_TRANSITION = 68,
    CONST_SRC_CODE_ENVMAP_PARMS = 69,
    CONST_SRC_CODE_SPOT_SHADOWMAP_PIXEL_ADJUST = 70,
    CONST_SRC_CODE_DLIGHT_SPOT_SHADOWMAP_PIXEL_ADJUST = 71,
    CONST_SRC_CODE_CLIP_SPACE_LOOKUP_SCALE = 72,
    CONST_SRC_CODE_CLIP_SPACE_LOOKUP_OFFSET = 73,
    CONST_SRC_CODE_PARTICLE_CLOUD_MATRIX = 74,
    CONST_SRC_CODE_DEPTH_FROM_CLIP = 75,
    CONST_SRC_CODE_CODE_MESH_ARG_0 = 76,
    CONST_SRC_CODE_CODE_MESH_ARG_1 = 77,
    CONST_SRC_CODE_CODE_MESH_ARG_LAST = 77,
    CONST_SRC_CODE_BASE_LIGHTING_COORDS = 78,
    CONST_SRC_CODE_WIND_DIRECTION = 79,
    CONST_SRC_CODE_WATER_PARMS = 80,
    CONST_SRC_CODE_GRASS_PARMS = 81,
    CONST_SRC_CODE_GRASS_FORCE0 = 82,
    CONST_SRC_CODE_GRASS_FORCE1 = 83,
    CONST_SRC_CODE_GRASS_WIND_FORCE0 = 84,
    CONST_SRC_CODE_MOTIONBLUR_DIRECTION_AND_MAGNITUDE = 85,
    CONST_SRC_CODE_COMPOSITE_FX_DISTORTION = 86,
    CONST_SRC_CODE_GLOW_BLOOM_SCALE = 87,
    CONST_SRC_CODE_COMPOSITE_FX_OVERLAY_TEXCOORD = 88,
    CONST_SRC_CODE_COLOR_BIAS1 = 89,
    CONST_SRC_CODE_COLOR_TINT_BASE1 = 90,
    CONST_SRC_CODE_COLOR_TINT_DELTA1 = 91,
    CONST_SRC_CODE_POSTFX_FADE_EFFECT = 92,
    CONST_SRC_CODE_VIEWPORT_DIMENSIONS = 93,
    CONST_SRC_CODE_FRAMEBUFFER_READ = 94,
    CONST_SRC_CODE_RESIZE_PARAMS1 = 95,
    CONST_SRC_CODE_RESIZE_PARAMS2 = 96,
    CONST_SRC_CODE_RESIZE_PARAMS3 = 97,
    CONST_SRC_CODE_VARIANT_WIND_SPRING_0 = 98,
    CONST_SRC_CODE_VARIANT_WIND_SPRING_1 = 99,
    CONST_SRC_CODE_VARIANT_WIND_SPRING_2 = 100,
    CONST_SRC_CODE_VARIANT_WIND_SPRING_3 = 101,
    CONST_SRC_CODE_VARIANT_WIND_SPRING_4 = 102,
    CONST_SRC_CODE_VARIANT_WIND_SPRING_5 = 103,
    CONST_SRC_CODE_VARIANT_WIND_SPRING_6 = 104,
    CONST_SRC_CODE_VARIANT_WIND_SPRING_7 = 105,
    CONST_SRC_CODE_VARIANT_WIND_SPRING_8 = 106,
    CONST_SRC_CODE_VARIANT_WIND_SPRING_9 = 107,
    CONST_SRC_CODE_VARIANT_WIND_SPRING_10 = 108,
    CONST_SRC_CODE_VARIANT_WIND_SPRING_11 = 109,
    CONST_SRC_CODE_VARIANT_WIND_SPRING_12 = 110,
    CONST_SRC_CODE_VARIANT_WIND_SPRING_13 = 111,
    CONST_SRC_CODE_VARIANT_WIND_SPRING_14 = 112,
    CONST_SRC_CODE_VARIANT_WIND_SPRING_15 = 113,
    CONST_SRC_CODE_DESTRUCTIBLE_PARMS = 114,
    CONST_SRC_CODE_CLOUD_WORLD_AREA = 115,
    CONST_SRC_CODE_WATER_SCROLL = 116,
    CONST_SRC_CODE_CROSSFADE_PARMS = 117,
    CONST_SRC_CODE_CHARACTER_CHARRED_AMOUNT = 118,
    CONST_SRC_CODE_TREECANOPY_PARMS = 119,
    CONST_SRC_CODE_MARKS_HIT_NORMAL = 120,
    CONST_SRC_CODE_POSTFX_CONTROL0 = 121,
    CONST_SRC_CODE_POSTFX_CONTROL1 = 122,
    CONST_SRC_CODE_POSTFX_CONTROL2 = 123,
    CONST_SRC_CODE_POSTFX_CONTROL3 = 124,
    CONST_SRC_CODE_POSTFX_CONTROL4 = 125,
    CONST_SRC_CODE_POSTFX_CONTROL5 = 126,
    CONST_SRC_CODE_POSTFX_CONTROL6 = 127,
    CONST_SRC_CODE_POSTFX_CONTROL7 = 128,
    CONST_SRC_CODE_POSTFX_CONTROL8 = 129,
    CONST_SRC_CODE_POSTFX_CONTROL9 = 130,
    CONST_SRC_CODE_POSTFX_CONTROLA = 131,
    CONST_SRC_CODE_POSTFX_CONTROLB = 132,
    CONST_SRC_CODE_POSTFX_CONTROLC = 133,
    CONST_SRC_CODE_POSTFX_CONTROLD = 134,
    CONST_SRC_CODE_POSTFX_CONTROLE = 135,
    CONST_SRC_CODE_POSTFX_CONTROLF = 136,
    CONST_SRC_CODE_HDRCONTROL_0 = 137,
    CONST_SRC_CODE_HDRCONTROL_1 = 138,
    CONST_SRC_CODE_GLIGHT_POSXS = 139,
    CONST_SRC_CODE_GLIGHT_POSYS = 140,
    CONST_SRC_CODE_GLIGHT_POSZS = 141,
    CONST_SRC_CODE_GLIGHT_FALLOFFS = 142,
    CONST_SRC_CODE_GLIGHT_REDS = 143,
    CONST_SRC_CODE_GLIGHT_GREENS = 144,
    CONST_SRC_CODE_GLIGHT_BLUES = 145,
    CONST_SRC_CODE_DLIGHT_POSITION = 146,
    CONST_SRC_CODE_DLIGHT_DIFFUSE = 147,
    CONST_SRC_CODE_DLIGHT_SPECULAR = 148,
    CONST_SRC_CODE_DLIGHT_ATTENUATION = 149,
    CONST_SRC_CODE_DLIGHT_FALLOFF = 150,
    CONST_SRC_CODE_DLIGHT_SPOT_MATRIX_0 = 151,
    CONST_SRC_CODE_DLIGHT_SPOT_MATRIX_1 = 152,
    CONST_SRC_CODE_DLIGHT_SPOT_MATRIX_2 = 153,
    CONST_SRC_CODE_DLIGHT_SPOT_MATRIX_3 = 154,
    CONST_SRC_CODE_DLIGHT_SPOT_DIR = 155,
    CONST_SRC_CODE_DLIGHT_SPOT_FACTORS = 156,
    CONST_SRC_CODE_DLIGHT_SHADOW_LOOKUP_MATRIX_0 = 157,
    CONST_SRC_CODE_DLIGHT_SHADOW_LOOKUP_MATRIX_1 = 158,
    CONST_SRC_CODE_DLIGHT_SHADOW_LOOKUP_MATRIX_2 = 159,
    CONST_SRC_CODE_DLIGHT_SHADOW_LOOKUP_MATRIX_3 = 160,
    CONST_SRC_CODE_CLOUD_LAYER_CONTROL0 = 161,
    CONST_SRC_CODE_CLOUD_LAYER_CONTROL1 = 162,
    CONST_SRC_CODE_CLOUD_LAYER_CONTROL2 = 163,
    CONST_SRC_CODE_CLOUD_LAYER_CONTROL3 = 164,
    CONST_SRC_CODE_CLOUD_LAYER_CONTROL4 = 165,
    CONST_SRC_CODE_HERO_LIGHTING_R = 166,
    CONST_SRC_CODE_HERO_LIGHTING_G = 167,
    CONST_SRC_CODE_HERO_LIGHTING_B = 168,
    CONST_SRC_CODE_LIGHT_HERO_SCALE = 169,
    CONST_SRC_CODE_CINEMATIC_BLUR_BOX = 170,
    CONST_SRC_CODE_CINEMATIC_BLUR_BOX2 = 171,
    CONST_SRC_CODE_ADSZSCALE = 172,
    CONST_SRC_CODE_UI3D_UV_SETUP_0 = 173,
    CONST_SRC_CODE_UI3D_UV_SETUP_1 = 174,
    CONST_SRC_CODE_UI3D_UV_SETUP_2 = 175,
    CONST_SRC_CODE_UI3D_UV_SETUP_3 = 176,
    CONST_SRC_CODE_UI3D_UV_SETUP_4 = 177,
    CONST_SRC_CODE_UI3D_UV_SETUP_5 = 178,
    CONST_SRC_CODE_CHARACTER_DISSOLVE_COLOR = 179,
    CONST_SRC_CODE_CAMERA_LOOK = 180,
    CONST_SRC_CODE_CAMERA_UP = 181,
    CONST_SRC_CODE_CAMERA_SIDE = 182,
    CONST_SRC_CODE_GENERIC_PARAM0 = 183,
    CONST_SRC_CODE_GENERIC_PARAM1 = 184,
    CONST_SRC_CODE_GENERIC_PARAM2 = 185,
    CONST_SRC_CODE_GENERIC_PARAM3 = 186,
    CONST_SRC_CODE_GENERIC_PARAM4 = 187,
    CONST_SRC_CODE_GENERIC_PARAM5 = 188,
    CONST_SRC_CODE_GENERIC_PARAM6 = 189,
    CONST_SRC_CODE_GENERIC_PARAM7 = 190,
    CONST_SRC_CODE_EYEOFFSET = 191,
    CONST_SRC_CODE_CUSTOMWIND_CENTER = 192,
    CONST_SRC_CODE_CUSTOMWIND_SPRING = 193,
    CONST_SRC_CODE_SKY_COLOR_MULTIPLIER = 194,
    CONST_SRC_CODE_EXTRA_CAM_PARAM = 195,
    CONST_SRC_CODE_EMBLEM_LUT_SELECTOR = 196,
    CONST_SRC_CODE_NEVER_DIRTY_PS_END = 197,
    CONST_SRC_CODE_COUNT_FLOAT4 = 197,

    CONST_SRC_FIRST_CODE_MATRIX = 197,
    CONST_SRC_CODE_WORLD_MATRIX = 197,
    CONST_SRC_CODE_INVERSE_WORLD_MATRIX = 198,
    CONST_SRC_CODE_TRANSPOSE_WORLD_MATRIX = 199,
    CONST_SRC_CODE_INVERSE_TRANSPOSE_WORLD_MATRIX = 200,
    CONST_SRC_CODE_VIEW_MATRIX = 201,
    CONST_SRC_CODE_INVERSE_VIEW_MATRIX = 202,
    CONST_SRC_CODE_TRANSPOSE_VIEW_MATRIX = 203,
    CONST_SRC_CODE_INVERSE_TRANSPOSE_VIEW_MATRIX = 204,
    CONST_SRC_CODE_PROJECTION_MATRIX = 205,
    CONST_SRC_CODE_INVERSE_PROJECTION_MATRIX = 206,
    CONST_SRC_CODE_TRANSPOSE_PROJECTION_MATRIX = 207,
    CONST_SRC_CODE_INVERSE_TRANSPOSE_PROJECTION_MATRIX = 208,
    CONST_SRC_CODE_WORLD_VIEW_MATRIX = 209,
    CONST_SRC_CODE_INVERSE_WORLD_VIEW_MATRIX = 210,
    CONST_SRC_CODE_TRANSPOSE_WORLD_VIEW_MATRIX = 211,
    CONST_SRC_CODE_INVERSE_TRANSPOSE_WORLD_VIEW_MATRIX = 212,
    CONST_SRC_CODE_VIEW_PROJECTION_MATRIX = 213,
    CONST_SRC_CODE_INVERSE_VIEW_PROJECTION_MATRIX = 214,
    CONST_SRC_CODE_TRANSPOSE_VIEW_PROJECTION_MATRIX = 215,
    CONST_SRC_CODE_INVERSE_TRANSPOSE_VIEW_PROJECTION_MATRIX = 216,
    CONST_SRC_CODE_WORLD_VIEW_PROJECTION_MATRIX = 217,
    CONST_SRC_CODE_INVERSE_WORLD_VIEW_PROJECTION_MATRIX = 218,
    CONST_SRC_CODE_TRANSPOSE_WORLD_VIEW_PROJECTION_MATRIX = 219,
    CONST_SRC_CODE_INVERSE_TRANSPOSE_WORLD_VIEW_PROJECTION_MATRIX = 220,
    CONST_SRC_CODE_SHADOW_LOOKUP_MATRIX = 221,
    CONST_SRC_CODE_INVERSE_SHADOW_LOOKUP_MATRIX = 222,
    CONST_SRC_CODE_TRANSPOSE_SHADOW_LOOKUP_MATRIX = 223,
    CONST_SRC_CODE_INVERSE_TRANSPOSE_SHADOW_LOOKUP_MATRIX = 224,
    CONST_SRC_CODE_WORLD_OUTDOOR_LOOKUP_MATRIX = 225,
    CONST_SRC_CODE_INVERSE_WORLD_OUTDOOR_LOOKUP_MATRIX = 226,
    CONST_SRC_CODE_TRANSPOSE_WORLD_OUTDOOR_LOOKUP_MATRIX = 227,
    CONST_SRC_CODE_INVERSE_TRANSPOSE_WORLD_OUTDOOR_LOOKUP_MATRIX = 228,

    CONST_SRC_TOTAL_COUNT = 229,
    CONST_SRC_NONE = 230,
};

// 31 Matrix const's, divide by 4 to go into matrixVersions[8] in `GfxCmdBufSourceState` (and ty to opent5)
#define MATRIX_VERSIONS_INDEX(idx) ((idx - CONST_SRC_FIRST_CODE_MATRIX) / 4)
#define MATRIX_INDEX(idx) (idx - CONST_SRC_FIRST_CODE_MATRIX)

#define CONST_SRC_MATRIX_INVERSE_BIT 1
#define CONST_SRC_MATRIX_TRANSPOSE_BIT 2

// credits to opent5
static inline unsigned int
R_GetMatrixConstantVersion(GfxCmdBufSourceState *source, unsigned int codeConstIndex)
{
    return source->matrixVersions[MATRIX_VERSIONS_INDEX(codeConstIndex)];
}
static inline bool
R_IsMatrixConstantUpToDate(GfxCmdBufSourceState *source, unsigned int codeConstIndex)
{
    return R_GetMatrixConstantVersion(source, codeConstIndex) == source->constVersions[codeConstIndex];
}

void __cdecl R_PixStartNamedRenderTarget(unsigned __int8 renderTargetId);
void R_PixEndNamedRenderTarget();
void __cdecl R_SetTexFilter();
void __cdecl R_SetInitialContextState(IDirect3DDevice9 *device);
void __cdecl R_ChangeDepthHackNearClip(GfxCmdBufSourceState *source, float depthHackFlags);
void __cdecl R_DepthHackNearClipChanged(GfxCmdBufSourceState *source);
void __cdecl R_ChangeObjectPlacement(GfxCmdBufSourceState *source, const GfxScaledPlacement *placement);
void __cdecl R_ChangeObjectPlacement_Core(GfxCmdBufSourceState *source, const GfxScaledPlacement *placement);
void __cdecl R_ChangeObjectPlacementRemote(GfxCmdBufSourceState *source, const GfxScaledPlacement *remotePlacement);
float *__cdecl R_GetCodeMatrix(
                GfxCmdBufSourceState *source,
                unsigned int sourceIndex,
                unsigned int firstRow);
void __cdecl R_DeriveCodeMatrix(GfxCmdBufSourceState *source, GfxCodeMatrices *activeMatrices, unsigned int baseIndex);
void __cdecl R_DeriveViewMatrix(GfxCmdBufSourceState *source);
void    R_DeriveWorldViewMatrix(GfxCmdBufSourceState *source);
void __cdecl R_DeriveProjectionMatrix(GfxCmdBufSourceState *source);
void __cdecl R_DeriveViewProjectionMatrix(GfxCmdBufSourceState *source);
void    R_DeriveWorldViewProjectionMatrix(GfxCmdBufSourceState *source);
void __cdecl R_DeriveShadowLookupMatrix(GfxCmdBufSourceState *source);
// local variable allocation has failed, the output may be wrong!
void    R_GenerateWorldOutdoorLookupMatrix(
                GfxCmdBufSourceState *source,
                float (*outMatrix)[4]);
const GfxImage *__cdecl R_GetTextureFromCode(
                GfxCmdBufSourceState *source,
                unsigned int codeTexture,
                unsigned __int8 *samplerState);
void __cdecl R_TextureFromCodeError(const GfxCmdBufContext context, unsigned int codeTexture);
const GfxImage *__cdecl R_OverrideGrayscaleImage(const dvar_s *dvar);
void __cdecl R_SetLightmap(GfxCmdBufContext context, unsigned int lmapIndex);
void __cdecl R_SetReflectionProbe(GfxCmdBufContext context, unsigned int reflectionProbeIndex);
void __cdecl R_ChangeDepthRange(GfxCmdBufState *state, GfxDepthRangeType depthRangeType);
void __cdecl R_HW_SetViewport(IDirect3DDevice9 *device, const GfxViewport *viewport, float nearValue, float farValue);
int __cdecl R_BeginMaterial(GfxCmdBufState *state, const Material *material, unsigned __int8 techType);
void __cdecl R_ChangeIndices(GfxCmdBufPrimState *state, IDirect3DIndexBuffer9 *ib);
void __cdecl R_ChangeStreamSource(
                GfxCmdBufPrimState *state,
                unsigned int streamIndex,
                IDirect3DVertexBuffer9 *vb,
                unsigned int vertexOffset,
                unsigned int vertexStride);
void __cdecl R_ClearAllStreamSources(GfxCmdBufPrimState *state);
void __cdecl R_DrawIndexedPrimitive(GfxCmdBufPrimState *state, const GfxDrawPrimArgs *args);
void __cdecl R_SetAlphaAntiAliasingState(IDirect3DDevice9 *device, __int16 stateBits0);
void __cdecl R_ChangeState_0(GfxCmdBufState *state, unsigned int stateBits0);
void __cdecl R_HW_SetAlphaTestEnable(IDirect3DDevice9 *device, __int16 stateBits0);
void __cdecl R_HW_SetColorMask(IDirect3DDevice9 *device, unsigned int stateBits0);
void __cdecl R_HW_SetCullFace(IDirect3DDevice9 *device, __int16 stateBits0);
void __cdecl R_HW_SetPolygonMode(IDirect3DDevice9 *device, signed int stateBits0);
void __cdecl R_HW_DisableBlend(IDirect3DDevice9 *device);
void __cdecl R_HW_SetBlend(
                IDirect3DDevice9 *device,
                bool blendWasEnabled,
                unsigned int changedBits,
                unsigned int stateBits0);
void __cdecl R_SetAlphaTestFunction(GfxCmdBufState *state, __int16 stateBits0);
void __cdecl R_ChangeState_1(GfxCmdBufState *state, unsigned int stateBits1);
void __cdecl R_HW_SetDepthWriteEnable(IDirect3DDevice9 *device, char stateBits1);
void __cdecl R_HW_SetDepthTestEnable(IDirect3DDevice9 *device, char stateBits1);
void __cdecl R_HW_SetDepthTestFunction(IDirect3DDevice9 *device, char stateBits1);
void __cdecl R_HW_EnableStencil(IDirect3DDevice9 *device);
void __cdecl R_HW_DisableStencil(IDirect3DDevice9 *device);
void __cdecl R_HW_SetFrontStencilOp(
                IDirect3DDevice9 *device,
                unsigned int stencilOpPass,
                unsigned int stencilOpFail,
                unsigned int stencilOpZFail);
void __cdecl R_HW_SetBackStencilOp(
                IDirect3DDevice9 *device,
                unsigned int stencilOpPass,
                unsigned int stencilOpFail,
                unsigned int stencilOpZFail);
void __cdecl R_HW_SetFrontStencilFunc(IDirect3DDevice9 *device, unsigned int stencilFunc);
void __cdecl R_HW_SetBackStencilFunc(IDirect3DDevice9 *device, unsigned int stencilFunc);
void __cdecl R_ForceSetPolygonOffset(IDirect3DDevice9 *device, char stateBits1);
void __cdecl R_HW_SetPolygonOffset(IDirect3DDevice9 *device, float scale, float bias);
void __cdecl R_SetTextureSamplerCodeImageRenderTarget(
                GfxCmdBufContext context,
                unsigned int samplerIndex,
                GfxCodeImageRenderTarget textureControl);
unsigned int __cdecl R_HW_SetSamplerState(
                IDirect3DDevice9 *device,
                unsigned int samplerIndex,
                unsigned int samplerState,
                unsigned int oldSamplerState);
void __cdecl R_SetSampler(
                GfxCmdBufContext context,
                unsigned int samplerIndex,
                unsigned __int8 samplerState,
                const GfxImage *image);
void __cdecl R_HW_SetSamplerTexture(IDirect3DDevice9 *device, unsigned int samplerIndex, const GfxTexture *texture);
void __cdecl R_SetSamplerState(GfxCmdBufState *state, unsigned int samplerIndex, unsigned __int8 samplerState);
void __cdecl R_TextureOverride(
                const GfxBackEndData *data,
                GfxCmdBufContext context,
                char modelIndex,
                int textureOverride);
void __cdecl R_GetViewport(GfxCmdBufSourceState *source, GfxViewport *outViewport);
void __cdecl R_SetViewport(GfxCmdBufState *state, const GfxViewport *viewport);
void __cdecl R_SetViewportStruct(GfxCmdBufSourceState *source, const GfxViewport *viewport);
void __cdecl R_SetScissorStruct(GfxCmdBufSourceState *source, const GfxViewport *scissor);
void __cdecl R_ClearScissorStruct(GfxCmdBufSourceState *source);
char __cdecl R_GetScissor(GfxCmdBufSourceState *source, GfxViewport *outScissor);
void __cdecl R_SetScissor(GfxCmdBufState *state, const GfxViewport *_scissor);
void __cdecl R_ClearScissor(GfxCmdBufState *state);
void __cdecl R_SetViewportValues(GfxCmdBufSourceState *source, int x, int y, int width, int height);
void __cdecl R_UpdateViewport(GfxCmdBufSourceState *source, GfxViewport *viewport);
void __cdecl R_DisableSampler(GfxCmdBufState *state, unsigned int samplerIndex);
void __cdecl R_HW_DisableSampler(IDirect3DDevice9 *device, unsigned int samplerIndex);
void __cdecl UpdateVPosToWorld(GfxCmdBufSourceState *source);
void __cdecl R_SetRenderTargetSize(GfxCmdBufSourceState *source, GfxRenderTargetId newTargetId);
GfxViewportBehavior __cdecl R_ViewportBehaviorForRenderTarget(GfxRenderTargetId renderTargetId);
void R_SetRenderTarget(GfxCmdBufContext context, GfxRenderTargetId newTargetId);
void __cdecl R_HW_SetRenderTarget(GfxCmdBufState *state, GfxRenderTargetId newTargetId);
void __cdecl R_UpdateStatsTarget(GfxCmdBufContext context);
void __cdecl R_UnbindImage(GfxCmdBufState *state, const GfxImage *image);
void __cdecl R_ClearRenderTargetForMultiGpu(GfxCmdBufContext context, GfxRenderTargetId targetId);
void __cdecl R_ClearScreenInternal(
                IDirect3DDevice9 *device,
                unsigned __int8 whichToClear,
                const float *color,
                float depth,
                unsigned __int8 stencil,
                const GfxViewport *viewport);
void __cdecl Byte4PackPixelColor(const float *from, unsigned __int8 *to);
void __cdecl R_ClearScreen(
                IDirect3DDevice9 *device,
                unsigned __int8 whichToClear,
                const float *color,
                float depth,
                unsigned __int8 stencil,
                const GfxViewport *viewport);
void __cdecl R_SetMeshStream(GfxCmdBufState *state, GfxMeshData *mesh);
void __cdecl R_SetCompleteState(IDirect3DDevice9 *device, unsigned int *stateBits);
void __cdecl R_ForceSetBlendState(IDirect3DDevice9 *device, unsigned int stateBits0);
void __cdecl R_ForceSetStencilState(IDirect3DDevice9 *device, unsigned int stateBits1);
void __cdecl R_Set_Texture_SeeThruDecal(GfxCmdBufSourceState *source);

void    R_DrawCall(
                void(__cdecl *callback)(const void *, GfxCmdBufContext, GfxCmdBufContext),
                const void *userData,
                GfxCmdBufSourceState *source,
                const GfxViewInfo *viewInfo,
                GfxDrawSurfListInfo *info,
                const GfxViewParms *viewParms,
                GfxCmdBuf *cmdBufEA,
                GfxCmdBuf *prepassCmdBufEA);

inline void R_DrawCall(
    void(__cdecl *callback)(const void *, GfxCmdBufContext, GfxCmdBufContext),
    const void *userData,
    GfxCmdBufSourceState *source,
    const GfxViewInfo *viewInfo,
    const GfxDrawSurfListInfo *info, // inline version to de-const this arg :(
    const GfxViewParms *viewParms,
    GfxCmdBuf *cmdBufEA,
    GfxCmdBuf *prepassCmdBufEA)
{
    R_DrawCall(callback, userData, source, viewInfo, (GfxDrawSurfListInfo *)info, viewParms, cmdBufEA, prepassCmdBufEA); 
}

static const MaterialUpdateFrequency s_codeSamplerUpdateFreq[43] =
{
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_CUSTOM,
  MTL_UPDATE_CUSTOM,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_PER_OBJECT,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_PER_OBJECT,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_PER_OBJECT,
  MTL_UPDATE_PER_OBJECT,
  MTL_UPDATE_PER_OBJECT,
  MTL_UPDATE_PER_OBJECT,
  MTL_UPDATE_PER_OBJECT,
  MTL_UPDATE_CUSTOM,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_PER_OBJECT,
  MTL_UPDATE_PER_OBJECT,
  MTL_UPDATE_PER_OBJECT,
  MTL_UPDATE_PER_OBJECT,
  MTL_UPDATE_CUSTOM,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_PER_OBJECT,
  MTL_UPDATE_RARELY
};


static const MaterialUpdateFrequency s_codeConstUpdateFreq[] =
{
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_PER_OBJECT,
  MTL_UPDATE_PER_OBJECT,
  MTL_UPDATE_PER_OBJECT,
  MTL_UPDATE_PER_OBJECT,
  MTL_UPDATE_PER_OBJECT,
  MTL_UPDATE_PER_OBJECT,
  MTL_UPDATE_PER_PRIM,
  MTL_UPDATE_PER_OBJECT,
  MTL_UPDATE_PER_PRIM,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_PER_PRIM,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_PER_PRIM,
  MTL_UPDATE_PER_PRIM,
  MTL_UPDATE_PER_PRIM,
  MTL_UPDATE_PER_PRIM,
  MTL_UPDATE_PER_OBJECT,
  MTL_UPDATE_PER_OBJECT,
  MTL_UPDATE_PER_OBJECT,
  MTL_UPDATE_PER_OBJECT,
  MTL_UPDATE_PER_OBJECT,
  MTL_UPDATE_PER_OBJECT,
  MTL_UPDATE_PER_OBJECT,
  MTL_UPDATE_PER_OBJECT,
  MTL_UPDATE_PER_PRIM,
  MTL_UPDATE_PER_PRIM,
  MTL_UPDATE_PER_PRIM,
  MTL_UPDATE_PER_PRIM,
  MTL_UPDATE_PER_OBJECT,
  MTL_UPDATE_PER_OBJECT,
  MTL_UPDATE_PER_OBJECT,
  MTL_UPDATE_PER_OBJECT,
  MTL_UPDATE_PER_PRIM,
  MTL_UPDATE_PER_PRIM,
  MTL_UPDATE_PER_PRIM,
  MTL_UPDATE_PER_PRIM,
  MTL_UPDATE_PER_OBJECT,
  MTL_UPDATE_PER_OBJECT,
  MTL_UPDATE_PER_OBJECT,
  MTL_UPDATE_PER_OBJECT,
  MTL_UPDATE_PER_PRIM,
  MTL_UPDATE_PER_PRIM,
  MTL_UPDATE_PER_PRIM,
  MTL_UPDATE_PER_PRIM,
  MTL_UPDATE_PER_PRIM,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_CUSTOM,
  MTL_UPDATE_CUSTOM,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_PER_OBJECT,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_PER_OBJECT,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_PER_OBJECT,
  MTL_UPDATE_PER_OBJECT,
  MTL_UPDATE_PER_OBJECT,
  MTL_UPDATE_PER_OBJECT,
  MTL_UPDATE_PER_OBJECT,
  MTL_UPDATE_CUSTOM,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_PER_OBJECT,
  MTL_UPDATE_PER_OBJECT,
  MTL_UPDATE_PER_OBJECT,
  MTL_UPDATE_PER_OBJECT,
  MTL_UPDATE_CUSTOM,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_RARELY,
  MTL_UPDATE_PER_OBJECT,
  MTL_UPDATE_RARELY
};

// name inferred - inlined
inline void R_SetInputCodeImage(GfxCmdBufInput *input, MaterialTextureSource source, GfxImage *image)
{
    iassert(input);
    input->codeImages[source] = image;
}

inline void R_SetInputCodeImageSamplerState(
    GfxCmdBufInput *input,
    unsigned int codeTexture,
    unsigned __int8 samplerState)
{
    bcassert(codeTexture, TEXTURE_SRC_CODE_COUNT);
    iassert(samplerState & SAMPLER_FILTER_MASK);

    input->codeImageSamplerStates[codeTexture] = samplerState;
}

extern unsigned int s_decodeSamplerFilterState[24];
