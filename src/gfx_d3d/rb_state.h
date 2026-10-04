#pragma once
#include "r_material.h"
#include "r_material_load_obj.h"

//enum GfxRenderTargetId : __int32
enum GfxRenderTargetId : unsigned __int8
{
    R_RENDERTARGET_NULL = 0,
    R_RENDERTARGET_NONE = 0,
    R_RENDERTARGET_SAVED_SCREEN = 1,
    R_RENDERTARGET_FRAME_BUFFER = 2,
    R_RENDERTARGET_SCENE = 3,
    R_RENDERTARGET_SCENE_NULLCOLOR = 4,
    R_RENDERTARGET_RESOLVED_POST_SUN = 5,
    R_RENDERTARGET_RESOLVED_SCENE = 6,
    R_RENDERTARGET_FLOAT_Z = 7,
    R_RENDERTARGET_PINGPONG_0 = 8,
    R_RENDERTARGET_PINGPONG_1 = 9,
    R_RENDERTARGET_POST_EFFECT_SRC = 10,
    R_RENDERTARGET_POST_EFFECT_GODRAYS = 11,
    R_RENDERTARGET_POST_EFFECT_0 = 12,
    R_RENDERTARGET_POST_EFFECT_1 = 13,
    R_RENDERTARGET_SHADOWMAP_SUN = 14,
    R_RENDERTARGET_SHADOWMAP_SPOT = 15,
    R_RENDERTARGET_16BIT_SYSTEM = 16,
    R_RENDERTARGET_8BIT_SYSTEM = 17,
    R_RENDERTARGET_8BIT_SWAPCHAIN_BACKBUFFER = 18,
    R_RENDERTARGET_SEETHRU_DECAL = 19,
    R_RENDERTARGET_UI3D = 20,
    R_RENDERTARGET_UI3D_PING_PONG = 21,
    R_RENDERTARGET_MISSILE_CAM = 22,
    R_RENDERTARGET_FLOAT_Z_MISSILE_CAM = 23,
    R_RENDERTARGET_COMPOSITE = 24,
    R_RENDERTARGET_BLOOM_MIP1 = 25,
    R_RENDERTARGET_BLOOM_MIP2 = 26,
    R_RENDERTARGET_BLOOM_MIP3 = 27,
    R_RENDERTARGET_BLOOM_MIP3_PING = 28,
    R_RENDERTARGET_BLOOM_MIP3_PONG = 29,
    R_RENDERTARGET_BLOOM_STREAK = 30,
    R_RENDERTARGET_SHADOWMAP_SUN_HI_FULL = 31,
    R_RENDERTARGET_SHADOWMAP_SUN_HI_PARTITION_0 = 32,
    R_RENDERTARGET_SHADOWMAP_SUN_HI_PARTITION_1 = 33,
    R_RENDERTARGET_SHADOWMAP_SUN_LO_FULL = 34,
    R_RENDERTARGET_SHADOWMAP_SUN_LO_PARTITION_0 = 35,
    R_RENDERTARGET_SHADOWMAP_SUN_LO_PARTITION_1 = 36,
    R_RENDERTARGET_SHADOWMAP_SPOT_FULL = 37,
    R_RENDERTARGET_SHADOWMAP_SPOT_0 = 38,
    R_RENDERTARGET_SHADOWMAP_SPOT_1 = 39,
    R_RENDERTARGET_SHADOWMAP_SPOT_2 = 40,
    R_RENDERTARGET_SHADOWMAP_SPOT_3 = 41,
    R_RENDERTARGET_SHADOWMAP_SPOT_HI_0 = 42,
    R_RENDERTARGET_SHADOWMAP_SPOT_HI_1 = 43,
    R_RENDERTARGET_COUNT = 44,
};

enum GfxViewMode : __int32
{                                       // XREF: GfxCmdBufSourceState/r
    VIEW_MODE_NONE     = 0x0,
    VIEW_MODE_3D       = 0x1,
    VIEW_MODE_2D       = 0x2,
    VIEW_MODE_IDENTITY = 0x3,
};

enum GfxViewportBehavior : __int32
{                                       // XREF: GfxCmdBufSourceState/r
    GFX_USE_VIEWPORT_FOR_VIEW = 0x0,    // XREF: .rdata:s_viewportBehaviorForRenderTarget/s
    GFX_USE_VIEWPORT_FULL     = 0x1,    // XREF: .rdata:s_viewportBehaviorForRenderTarget/s
};

struct GfxPrimStats // sizeof=0x18
{                                       // XREF: GfxViewStats/r
    int primCount;                      // XREF: RB_CopyBackendStats(void)+3D/o
                                        // RB_Stats_Summarize+52/w ...
    int triCount;                       // XREF: RB_Stats_Summarize+55/w
    int staticIndexCount;               // XREF: RB_Stats_Summarize+58/w
    int staticVertexCount;              // XREF: RB_Stats_Summarize+5B/w
    int dynamicIndexCount;              // XREF: RB_Stats_Summarize+5E/w
    int dynamicVertexCount;             // XREF: RB_Stats_Summarize+61/w
};

struct GfxViewStats // sizeof=0x138
{                                       // XREF: GfxFrameStats/r
    GfxPrimStats primStats[10];         // XREF: RB_CopyBackendStats(void)+3D/o
    int drawSurfCount;
    int drawMatCount;
    int drawPrimHistogram[16];          // XREF: RB_DrawPrimHistogramOverlay(void)+59/r
                                        // RB_DrawPrimHistogramOverlay(void)+6E/r ...
};

struct GfxFrameStats // sizeof=0x27C
{                                       // XREF: .data:GfxFrameStats g_frameStatsCur/r
                                        // GfxCmdBufPrimState/r ...
    GfxViewStats viewStats[2];          // XREF: RB_CopyBackendStats(void)+3D/o
                                        // RB_DrawPrimHistogramOverlay(void)+59/r ...
    int gfxEntCount;
    int geoIndexCount;                  // XREF: RB_CopyBackendStats(void)+11/r
    int fxIndexCount;                   // XREF: RB_CopyBackendStats(void)+1F/r
};

struct GfxCmdBufPrimState // sizeof=0x2BC
{                                       // XREF: GfxCmdBufState/r
    //$0548B6ED447A4690AAB8EE4D4B1790A5 ___u0;
    union //$0548B6ED447A4690AAB8EE4D4B1790A5 // sizeof=0x4
    {                                       // XREF: R_SetAndClearSceneTarget+6D/r
        IDirect3DDevice9 *device;
        IDirect3DDevice9 *localDevice;
    };
                                        // XREF: R_SetAndClearSceneTarget+6D/r
                                        // R_SetAndClearSceneTarget+7F/r ...
    IDirect3DIndexBuffer9 *indexBuffer; // XREF: RB_CallExecuteRenderCommands(void)+372/r
                                        // RB_SetInitialState(void)+1C5/r
    MaterialVertexDeclType vertDeclType;
                                        // XREF: RB_BeginSurface(Material const *,uchar)+126/w
                                        // RB_DrawTessSurface+C3/r ...
    struct //GfxCmdBufPrimState::<unnamed_type_streams> // sizeof=0xC
    {                                       // XREF: GfxCmdBufPrimState/r
        unsigned int stride;                // XREF: RB_SetInitialState(void):loc_AC7AC9/r
        // RB_SetInitialState(void):loc_AC7B4D/r
        IDirect3DVertexBuffer9 *vb;         // XREF: RB_SetInitialState(void):loc_AC7A71/r
        // RB_SetInitialState(void):loc_AC7AF5/r
        unsigned int offset;                // XREF: RB_SetInitialState(void):loc_AC7A9D/r
        // RB_SetInitialState(void):loc_AC7B21/r
    } streams[3];
    //GfxCmdBufPrimState::<unnamed_type_streams> streams[3];
                                        // XREF: RB_SetInitialState(void):loc_AC7A71/r
                                        // RB_SetInitialState(void):loc_AC7A9D/r ...
    IDirect3DVertexDeclaration9 *vertexDecl;
                                        // XREF: RB_ClearVertexDecl(void)+6/r
                                        // RB_ClearVertexDecl(void):loc_AC6A5E/r
    GfxFrameStats frameStats;
    GfxPrimStats *primStats;            // XREF: RB_ExecuteRenderCommandsLoop(void const *,int *)+2A5/r
                                        // RB_BeginSurface(Material const *,uchar):loc_AC706C/r ...
    GfxPrimStats *backupPrimStats;
    GfxViewStats *viewStats;
};

struct GfxCmdBufState // sizeof=0x13D0
{                                       // XREF: .data:GfxCmdBufState gfxCmdBufState/r
    unsigned __int8 refSamplerState[16];
    unsigned int samplerState[16];
    const GfxTexture *samplerTexture[16];
    GfxCmdBufPrimState prim;            // XREF: R_ShutdownStreams(void):loc_A515AE/o
                                        // R_SetAndClearSceneTarget+6D/r ...
    const Material *material;           // XREF: RB_BeginSurface(Material const *,uchar)+118/w
                                        // RB_BeginSurface(Material const *,uchar)+161/w ...
    unsigned __int8 techType;           // XREF: RB_BeginSurface(Material const *,uchar)+121/w
                                        // RB_BeginSurface(Material const *,uchar)+166/w ...
    // padding byte
    // padding byte
    // padding byte
    const MaterialTechnique *technique; // XREF: RB_BeginSurface(Material const *,uchar)+184/w
                                        // RB_BeginSurface(Material const *,uchar)+189/r ...
    const MaterialPass *pass;
    unsigned int passIndex;
    GfxDepthRangeType depthRangeType;
    float depthRangeNear;
    float depthRangeFar;
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    unsigned __int64 vertexShaderConstState[256];
    unsigned __int64 pixelShaderConstState[256];
    unsigned __int8 alphaRef;           // XREF: RB_SetInitialState(void):loc_AC7BFD/r
    // padding byte
    // padding byte
    // padding byte
    unsigned int refStateBits[2];
    unsigned int activeStateBits[2];    // XREF: RB_UpdateBackEndDvarOptions+FC/r
    const MaterialPixelShader *pixelShader;
                                        // XREF: RB_ClearPixelShader(void)+6/r
                                        // RB_ClearPixelShader(void):loc_AC67FF/w
    const MaterialVertexShader *vertexShader;
                                        // XREF: RB_ClearVertexShader(void)+6/r
                                        // RB_ClearVertexShader(void):loc_AC692F/w ...
    unsigned int pixPrimarySortKey;
    const Material *pixMaterial;
    const MaterialTechnique *pixTechnique;
    int pixCombine;
    GfxViewport viewport;
    GfxViewport scissor;
    int scissorEnabled;
    unsigned __int8 renderTargetId;     // XREF: RB_GetResolvedScene+3/r
                                        // RB_GetResolvedScene+11/r
    // padding byte
    // padding byte
    // padding byte
    const Material *origMaterial;       // XREF: RB_SetTessTechnique(Material const *,uchar):loc_AC6F8F/r
                                        // RB_BeginSurface(Material const *,uchar)+133/w ...
    unsigned __int8 origTechType;       // XREF: RB_SetTessTechnique(Material const *,uchar)+3A/r
                                        // RB_BeginSurface(Material const *,uchar)+13C/w ...
    // padding byte
    // padding byte
    // padding byte
    int stateOverride;
};

struct GfxCodeMatrices // sizeof=0x800
{                                       // XREF: GfxCmdBufSourceState/r
    GfxMatrix matrix[32];
};

struct GfxCodeImageRenderTargetFields // sizeof=0x8
{                                       // XREF: GfxCodeImageRenderTarget/r
    unsigned __int64 renderTargetId : 8;
    unsigned __int64 enable : 1;
    unsigned __int64 fbufferTexture : 3;
    unsigned __int64 filtering : 20;
};

union GfxCodeImageRenderTarget // sizeof=0x8
{                                       // XREF: RB_InitCodeImages(void)+55B/w
                                        // R_SetTextureSamplerCodeImageRenderTarget(GfxCmdBufContext,uint,GfxCodeImageRenderTarget)+16/r ...
    GfxCodeImageRenderTargetFields fields;
    unsigned int packed;
};

struct GfxBackEndData;
struct __declspec(align(16)) GfxCmdBufInput // sizeof=0xE90
{                                       // XREF: .data:GfxCmdBufInput gfxCmdBufInput/r
    float consts[197][4];
    const GfxImage *codeImages[TEXTURE_SRC_CODE_COUNT];     // XREF: R_UpdateFrontEndDvarOptions+249/w
    unsigned __int8 codeImageSamplerStates[TEXTURE_SRC_CODE_COUNT];
    // padding byte
    GfxCodeImageRenderTarget codeImageRenderTargetControl[TEXTURE_SRC_CODE_COUNT];
    const GfxBackEndData *data;
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
};

struct GfxCmdBufSourceState // sizeof=0x1A90
{                                       // XREF: .data:GfxCmdBufSourceState gfxCmdBufSourceState/r
    GfxCodeMatrices matrices;
    GfxCmdBufInput input;               // XREF: RB_ShowFbColorDebug_Feedback+D2/o
                                        // RB_ShowFbColorDebug_Feedback+166/o ...
    GfxViewParms viewParms;             // XREF: RB_EndSceneRendering:loc_A72FBF/o
                                        // RB_EndSceneRendering+10C/o ...
    GfxMatrix shadowLookupMatrix;
    unsigned __int16 constVersions[229/*CONST_SRC_TOTAL_COUNT*/];
    unsigned __int16 matrixVersions[8];
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    float eyeOffset[4];                 // XREF: RB_SetIdentity(void)+41/w
                                        // RB_SetIdentity(void)+51/w ...
    unsigned int shadowableLightForShadowLookupMatrix;
    const GfxScaledPlacement *objectPlacement;
    const GfxViewParms *viewParms3D;    // XREF: RB_TessCoronaBillboard+9/r
                                        // RB_TessCoronaBillboard+432/r ...
    unsigned int depthHackFlags;
    GfxScaledPlacement skinnedPlacement;
    int cameraView;
    GfxViewMode viewMode;               // XREF: RB_EndSceneRendering+C9/r
                                        // RB_EndSceneRendering+D2/r ...
    GfxSceneDef sceneDef;               // XREF: RB_DrawCoronas(int)+C7/r
                                        // RB_DrawCoronas(int):loc_A8C158/r ...
    GfxViewport sceneViewport;          // XREF: RB_InitSceneViewport(void)+3/w
                                        // RB_InitSceneViewport(void)+D/w ...
    GfxViewport scissorViewport;
    float materialTime;
    float destructibleBurnAmount;
    float destructibleFadeAmount;
    float wetness;
    GfxViewportBehavior viewportBehavior;
    int renderTargetWidth;
    int renderTargetHeight;
    bool viewportIsDirty;
    bool scissorEnabled;
    // padding byte
    // padding byte
    unsigned int shadowableLightIndex;
};

struct GfxCmdBuf // sizeof=0x4
{                                       // XREF: RB_FullbrightDrawCommands/r
                                        // RB_DebugShaderDrawCommands/r ...
    IDirect3DDevice9 *device;
};

struct GfxCmdBufContext // sizeof=0x8
{                                       // XREF: .rdata:GfxCmdBufContext const gfxCmdBufContext/r
    GfxCmdBufSourceState *source;       // ...
    GfxCmdBufState *state;              // ...
};

void __cdecl RB_SetInitialState();
void __cdecl RB_UnbindAllImages();
void __cdecl RB_InitSceneViewport();
void __cdecl RB_BindDefaultImages();
void __cdecl RB_InitImages();

extern GfxCmdBufState gfxCmdBufState;
extern GfxCmdBufInput gfxCmdBufInput;
extern GfxCmdBufSourceState gfxCmdBufSourceState;
extern GfxCmdBufContext gfxCmdBufContext;