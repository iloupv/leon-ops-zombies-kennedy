#pragma once
#include <Windows.h>
#include <d3d9.h>
#include "r_bsp.h"
#include "r_material_load_obj.h"

#define MAX_TOTAL_ENT_COUNT 0x2000

struct __declspec(align(4)) vidConfig_t // sizeof=0x38
{                                                                             // XREF: .data:vidConfig_t vidConfig/r
        unsigned int sceneWidth;                        // XREF: R_StoreWindowSettings+3B/w
                                                                                // R_StoreWindowSettings+23E/r ...
        unsigned int sceneHeight;                     // XREF: R_StoreWindowSettings+46/w
                                                                                // R_StoreWindowSettings+225/r ...
        unsigned int displayWidth;                    // XREF: CL_GetScreenDimensions(int *,int *,float *)+81/r
                                                                                // CL_SetupViewport(void)+6/r ...
        unsigned int displayHeight;                 // XREF: CL_GetScreenDimensions(int *,int *,float *)+8C/r
                                                                                // CL_SetupViewport(void)+E/r ...
        unsigned __int16 outputDisplayWidth;
        unsigned __int16 outputDisplayHeight;
        unsigned int displayFrequency;            // XREF: R_StoreWindowSettings+69/w
                                                                                // R_ReduceWindowSettings+39/r ...
        bool isToolMode;                                        // XREF: CG_Init(int,int,int,int)+A21/r
                                                                                // CL_InitRenderer(void):loc_5643F4/w ...
        // padding byte
        // padding byte
        // padding byte
        int isFullscreen;                                     // XREF: Sys_UpdateHotkeyBlock(void):loc_7E0A25/r
                                                                                // R_StoreWindowSettings+76/w ...
        float aspectRatioWindow;                        // XREF: CL_GetScreenDimensions(int *,int *,float *)+96/r
                                                                                // CL_SetupViewport(void)+26/r ...
        float aspectRatioScenePixel;                // XREF: ScrPlace_SetupFloatViewport(ScreenPlacement *,float,float,float,float,float)+7A/r
                                                                                // R_StoreWindowSettings+252/w ...
        float aspectRatioDisplayPixel;            // XREF: CL_GetScreenAspectRatioDisplayPixel(void)+3/r
                                                                                // CL_DrawSpinnerPhysical(float,float,float,float,float const * const)+D7/r ...
        unsigned int maxTextureSize;                // XREF: R_StoreDirect3DCaps+59/w
        unsigned int maxTextureMaps;                // XREF: R_StoreDirect3DCaps+5E/w
                                                                                // R_StoreDirect3DCaps+80/r ...
        bool deviceSupportsGamma;                     // XREF: R_SetColorMappings(void)+13/r
                                                                                // R_StoreDirect3DCaps+7A/w ...
        // padding byte
        // padding byte
        // padding byte
};

struct GfxWindowTarget // sizeof=0x10
{                                       // XREF: DxGlobals/r
    HWND__ *hwnd;                       // XREF: R_ReleaseForShutdownOrReset(void)+28/r
                                        // R_CreateDevice(GfxWindowParms const *):loc_A4ED89/r ...
    IDirect3DSwapChain9 *swapChain;     // XREF: R_ResizeWindow(void)+84/r
                                        // R_FinishAttachingToWindow+85/w ...
    int width;                          // XREF: R_ReleaseForShutdownOrReset(void)+5A/r
                                        // R_ReleaseForShutdownOrReset(void)+69/w ...
    int height;                         // XREF: R_ResizeWindow(void)+C7/w
                                        // R_FinishAttachingToWindow+99/w ...
};

struct __declspec(align(8)) DxGlobals // sizeof=0x2D00
{                                       // XREF: .data:DxGlobals dx/r
    HINSTANCE__ *hinst;
    IDirect3D9 *d3d9;                   // XREF: DB_TryLoadXFileInternal+554/r
                                        // DB_TryLoadXFileInternal+574/r ...
    IDirect3DDevice9 *device;           // XREF: yuv_init_internal(void)+154/r
                                        // yuv_init_internal(void)+15B/r ...
    unsigned int adapterIndex;          // XREF: DB_TryLoadXFileInternal+56E/r
                                        // R_CreateDeviceInternal+54/r ...
    unsigned int vendorId;              // XREF: DB_TryLoadXFileInternal+544/r
                                        // R_PreCreateWindow+D6/w ...
    bool adapterNativeIsValid;          // XREF: R_CreateDeviceInternal+2E/w
                                        // R_StoreWindowSettings+AB/r ...
    // padding byte
    // padding byte
    // padding byte
    int adapterNativeWidth;             // XREF: R_CreateDeviceInternal+21/o
                                        // R_StoreWindowSettings+B6/r ...
    int adapterNativeHeight;            // XREF: R_CreateDeviceInternal:loc_A4F18C/o
                                        // R_StoreWindowSettings+BF/r ...
    int adapterFullscreenWidth;         // XREF: R_CreateDeviceInternal+F5/w
                                        // R_CreateDeviceInternal+10B/w ...
    int adapterFullscreenHeight;        // XREF: R_CreateDeviceInternal+FE/w
                                        // R_CreateDeviceInternal+117/w ...
    bool supportsSceneNullRenderTarget; // XREF: RB_StandardDrawCommands+430/r
                                        // RB_StandardDrawCommands:loc_A74235/r ...
    bool supportsIntZ;                  // XREF: R_SetDepthOfField:loc_A65966/r
                                        // R_DoesDrawSurfListInfoNeedFloatz+6/r ...
    bool nvInitialized;                 // XREF: R_PreCreateWindow+A0/w
                                        // R_PreCreateWindow+FC/w ...
    bool nvStereoActivated;             // XREF: R_CreateDeviceInternal+7C/w
                                        // R_CreateDeviceInternal+C6/w ...
    void *nvStereoHandle;               // XREF: R_CreateDeviceInternal+83/w
                                        // R_CreateDeviceInternal+8D/o ...
    void *nvDepthBufferHandle;          // XREF: R_ResolveIntZ_PC(void)+42/r
                                        // R_ResolveIntZ_PC(void)+66/r ...
    void *nvFloatZBufferHandle;         // XREF: R_ResolveIntZ_PC(void)+4B/r
                                        // R_ResolveIntZ_PC(void)+5D/r ...
    bool resizeWindow;                  // XREF: R_CheckResizeWindow(void):loc_A4F4E0/w
                                        // R_CheckResizeWindow(void):loc_A4F4E7/r ...
    // padding byte
    // padding byte
    // padding byte
    _D3DFORMAT depthStencilFormat;      // XREF: R_CreateDevice(GfxWindowParms const *)+D7/w
                                        // R_SetD3DPresentParameters+D0/r ...
    unsigned int displayModeCount;      // XREF: R_ClosestRefreshRateForMode+D/r
                                        // R_EnumDisplayModes+28/w ...
    _D3DDISPLAYMODE displayModes[256];  // XREF: R_ClosestRefreshRateForMode+34/r
                                        // R_ClosestRefreshRateForMode+48/r ...
    const char *resolutionNameTable[257];
                                        // XREF: R_EnumDisplayModes+212/w
                                        // R_EnumDisplayModes+284/w ...
    const char *refreshRateNameTable[257];
                                        // XREF: R_EnumDisplayModes+2FB/w
                                        // R_EnumDisplayModes+346/w ...
    char modeText[5120];                // XREF: R_EnumDisplayModes:loc_A50749/o
    IDirect3DQuery9 *fencePool[8];      // XREF: R_HW_InsertFence(IDirect3DQuery9 * *)+18/r
                                        // R_ReleaseForShutdownOrReset(void)+170/r ...
    unsigned int nextFence;             // XREF: R_HW_InsertFence(IDirect3DQuery9 * *)+12/r
                                        // R_HW_InsertFence(IDirect3DQuery9 * *)+21/r ...
    int gpuSync;                        // XREF: R_UpdateGpuSyncType(void)+29/w
                                        // RB_CallExecuteRenderCommands(void)+4E5/r
    int gpuCount;                       // XREF: R_InitHardware+3E/w
                                        // R_InitHardware+A6/w ...
    _D3DMULTISAMPLE_TYPE multiSampleType;
                                        // XREF: R_SetD3DPresentParameters+A5/r
                                        // R_SetupAntiAliasing+99/w ...
    unsigned int multiSampleQuality;    // XREF: R_SetD3DPresentParameters+B0/r
                                        // R_SetupAntiAliasing+EE/w ...
    int sunSpriteSamples;               // XREF: R_Init(void)+4F/w
                                        // R_Init(void)+54/r ...
    int sunShadowmapSize; // not a real name, not in IDA for some reason
    struct IDirect3DSurface9 *singleSampleDepthStencilSurface;
                                        // XREF: R_GenerateSortedDrawSurfs+381/r
                                        // R_GenerateSortedDrawSurfs:loc_A62FDD/r ...
    bool inScene;                       // XREF: R_AssignSingleSampleDepthStencilSurface+6/r
                                        // R_AssignSingleSampleDepthStencilSurface+1D/w ...
    // padding byte
    // padding byte
    // padding byte
    int targetWindowIndex;              // XREF: R_ComErrorCleanup(void)+42/r
                                        // R_ComErrorCleanup(void)+F9/w ...
    int windowCount;                    // XREF: R_CreateGameWindow:loc_A50D41/w
                                        // RB_SetGammaRamp(GfxGammaRamp const *)+123/r ...
    GfxWindowTarget windows[1];         // XREF: R_ReleaseForShutdownOrReset(void)+28/r
                                        // R_ReleaseForShutdownOrReset(void)+5A/r ...
    IDirect3DQuery9 *flushGpuQuery;     // XREF: R_ResizeWindow(void)+D0/w
                                        // R_FinishAttachingToWindow+AD/w ...
    IDirect3DQuery9 *gpuSyncDelay;      // XREF: R_ReleaseForShutdownOrReset(void):loc_A4EB31/r
                                        // R_ReleaseForShutdownOrReset(void):loc_A4EB5E/r ...
    unsigned __int64 gpuSyncStart;      // XREF: RB_CallExecuteRenderCommands(void)+4EE/w
                                        // RB_CallExecuteRenderCommands(void)+4F8/w
    unsigned __int64 gpuSyncEnd;
    _D3DTEXTUREFILTERTYPE linearNonMippedMinFilter;
    _D3DTEXTUREFILTERTYPE linearNonMippedMagFilter;
    _D3DTEXTUREFILTERTYPE linearMippedMinFilter;
    _D3DTEXTUREFILTERTYPE linearMippedMagFilter;
    _D3DTEXTUREFILTERTYPE anisotropicMinFilter;
    _D3DTEXTUREFILTERTYPE anisotropicMagFilter;
    int linearMippedAnisotropy;
    int anisotropyFor2x;
    int anisotropyFor4x;
    int mipFilterMode;
    unsigned int mipBias;
    IDirect3DQuery9 *swapFence[4];      // XREF: R_ReleaseForShutdownOrReset(void)+1F9/r
                                        // R_ReleaseForShutdownOrReset(void)+206/w ...
    // padding byte
    // padding byte
    // padding byte
    // padding byte
};

struct GfxGlobals // sizeof=0x24
{                                       // XREF: .data:GfxGlobals r_glob/r
    bool startedRenderThread;           // XREF: R_ShutdownInternal(int)+32/w
                                        // R_BeginRegistration(vidConfig_t *)+5A/r ...
    bool isMultiplayer;                 // XREF: R_SetIsMultiplayer(bool)+6/w
                                        // R_RegisterDvars(void)+4A5D/r ...
    // padding byte
    // padding byte
    volatile int endFrameFence;
    bool isRenderingRemoteUpdate;       // XREF: R_InitThreads(void)+3/r
                                        // R_ReleaseLostDeviceAssets2(void)+6C/r ...
    // padding byte
    // padding byte
    // padding byte
    volatile int screenUpdateNotify;    // XREF: R_BeginRemoteScreenUpdate(void):loc_A7D848/r
                                        // R_EndRemoteScreenUpdate(void (*)(void)):loc_A7D9E8/r ...
    volatile int remoteScreenUpdateNesting; // XREF: DB_Sleep:loc_58F143/r
                                        // R_SyncRenderThread(void):loc_A7A1B6/r ...
    volatile int remoteScreenUpdateInGame;
                                        // XREF: R_BeginRemoteScreenUpdate(void)+85/r
                                        // R_EndRemoteScreenUpdate(void (*)(void))+B9/r
    unsigned __int8 remoteScreenLastSceneResolveTarget;
                                        // XREF: R_Init(void)+13/w
                                        // R_PerMap_Init(void)+8/w
    // padding byte
    // padding byte
    // padding byte
    int backEndFrameCount;              // XREF: RB_DrawCoronaQuerySprite+53/r
                                        // RB_BeginFrame(void const *)+2D/r ...
    unsigned __int8 frameBuffer;
    unsigned __int8 displayBuffer;
    unsigned __int8 ui3dUseFrameBuffer;
    unsigned __int8 ui3dRenderTarget;
};

struct GfxConfiguration // sizeof=0x34
{                                       // XREF: .data:GfxConfiguration gfxCfg/r
                                        // ?CL_InitRef@@YAXXZ/r ...
    unsigned int maxClientViews;        // XREF: R_UnfilterEntFromCells(int,uint):loc_A416E8/r
                                        // R_FilterDObjIntoCells(int,uint,float * const,float)+3A/r ...
    unsigned int entCount;              // XREF: R_UnfilterEntFromCells(int,uint)+9E/r
                                        // R_UnfilterEntFromCells(int,uint)+E2/r ...
    unsigned int entnumNone;            // XREF: R_AddSceneDObj(uint,uint)+6/r
                                        // R_DrawAllSceneEnt(GfxViewInfo const *)+FE/r ...
    unsigned int entnumOrdinaryEnd;     // XREF: R_GenerateMarkVertsForDynamicModels+5A/r
                                        // R_GenerateMarkVertsForDynamicModels+10F/r ...
    int threadContextCount;
    int critSectCount;
    const char *codeFastFileName;
    const char *patchFastFileName;
    const char *uiFastFileName;
    const char *uiViewerFastFileName;
    const char *commonFastFileName;
    const char *modFastFileName;
    const char *devFastFileName;
};

struct GfxGammaRamp // sizeof=0x200
{                                       // XREF: ?R_SetColorMappings@@YAXXZ/r
    unsigned __int16 entries[256];
};

struct GfxWindowParms // sizeof=0x28
{                                       // XREF: ?R_ResizeWindow@@YAXXZ/r
                                        // R_InitGraphicsApi/r ...
    HWND__ *hwnd;                       // XREF: R_ResetDevice+B/w
                                        // R_ResetDevice+DF/r ...
    int hz;                             // XREF: R_ResetDevice+40/w
    bool fullscreen;                    // XREF: R_ResizeWindow(void)+D6/r
                                        // R_ResizeWindow(void)+152/r ...
    // padding byte
    // padding byte
    // padding byte
    int x;                              // XREF: R_ResizeWindow(void)+14E/r
                                        // R_ResizeWindow(void)+176/r ...
    int y;                              // XREF: R_ResizeWindow(void)+14A/r
                                        // R_ResizeWindow(void)+172/r ...
    int sceneWidth;                     // XREF: R_ResetDevice+31/w
    int sceneHeight;                    // XREF: R_ResetDevice+37/w
    int displayWidth;                   // XREF: R_ResizeWindow(void)+C4/r
                                        // R_ResizeWindow(void)+103/r ...
    int displayHeight;                  // XREF: R_ResizeWindow(void)+CD/r
                                        // R_ResizeWindow(void)+110/r ...
    int aaSamples;                      // XREF: R_ResetDevice+5B/w
    bool borderless;                    // borderless: BO1Zombies's own, not in the exe struct (see R_SetWndParms)
};

struct trViewStatistics_t // sizeof=0x18
{                                       // XREF: .data:rendererViewStats/r
    int c_indexes;
    int c_fxIndexes;
    int c_viewIndexes;
    int c_shadowIndexes;
    int c_vertexes;
    int c_batches;
};

struct Image_MemUsage // sizeof=0xC
{                                       // XREF: trStatistics_t/r
    int total;                          // XREF: CG_DrawFPS(int,ScreenPlacement const *,float,meminfo_t *)+8B6/r
    int lightmap;
    int minspec;                        // XREF: CG_DrawFPS(int,ScreenPlacement const *,float,meminfo_t *)+90D/r
                                        // CG_DrawFPS(int,ScreenPlacement const *,float,meminfo_t *)+920/r ...
};

struct trStatistics_t // sizeof=0x10
{                                       // XREF: .data:rendererStats/r
    trViewStatistics_t *views;          // XREF: CG_DrawFPS(int,ScreenPlacement const *,float,meminfo_t *)+699/w
    Image_MemUsage c_imageUsage;        // XREF: CG_DrawFPS(int,ScreenPlacement const *,float,meminfo_t *)+8B6/r
};

struct SavedScreenParams // sizeof=0x14
{                                       // XREF: r_global_permanent_t/r
    float s0;
    float t0;
    float ds;
    float dt;
    int isSet;
};

struct r_global_permap_assets_t // sizeof=0xC
{                                       // XREF: r_global_permanent_t/r
    Material *flameThrowerFXMaterial;   // XREF: R_InitAssets_PostMapFastfileLoad(void)+5/w
    Material *electrifiedFXMaterial;    // XREF: R_InitAssets_PostMapFastfileLoad(void)+A/w
    Material *transportedFXMaterial;    // XREF: R_InitAssets_PostMapFastfileLoad(void)+F/w
};

struct __declspec(align(128)) r_global_permanent_t // sizeof=0x4300
{                                       // XREF: .data:r_global_permanent_t rgp/r
    Material *sortedMaterials[4096];    // XREF: GDT_RemoteMaterialUpdate+2A1/r
                                        // Material_Add+BA/w ...
    int needSortMaterials;              // XREF: Material_DirtySort(void)+3/w
                                        // Material_Duplicate(Material *,char const *)+A7/w ...
    int materialCount;                  // XREF: GDT_RemoteMaterialUpdate+28F/r
                                        // Material_Add+79/r ...
    int needMaterialPreload;            // XREF: R_BeginFrame(void)+BE/w
    GfxImage *whiteImage;               // XREF: R_SetDLightsConstantsDefaults+6/r
                                        // R_SetDLightsConstantsDefaults+8E/r ...
    GfxImage *blackImage;               // XREF: R_GenerateSortedDrawSurfs+7A2/r
                                        // RB_StandardDrawCommandsCommon+256/r ...
    GfxImage *blankImage;               // XREF: R_InitCodeImages+9E/w
                                        // R_InitCodeImages+A3/r ...
    GfxImage *g16r16Image;              // XREF: R_InitCodeImages+E3/w
                                        // R_InitCodeImages+E8/r ...
    GfxImage *r5g6b5Image;              // XREF: R_InitCodeImages+128/w
                                        // R_InitCodeImages+12D/r ...
    GfxImage *grayImage;                // XREF: R_Cinematic_BlackRendererImages+34/r
                                        // R_Cinematic_BlackRendererImages+40/r ...
    GfxImage *identityNormalMapImage;   // XREF: RB_InitCodeImages(void)+65/r
                                        // Image_AssignDefaultTexture+1F/r ...
    GfxImage *whiteTransparentImage;    // XREF: R_InitCodeImages+1F7/w
                                        // R_InitCodeImages+1FC/r ...
    GfxImage *blackTransparentImage;    // XREF: R_InitCodeImages+23C/w
                                        // R_InitCodeImages+241/r ...
    GfxImage *outdoorImage;
    GfxImage *heatMapImage;             // XREF: RB_UI3D_RenderToTexture(void const *,GfxUI3DBackend const *,GfxCmdBufInput const *)+2F3/r
                                        // RB_UI3D_RenderToTexture(void const *,GfxUI3DBackend const *,GfxCmdBufInput const *)+2FC/r ...
    GfxImage *pixelCostColorCodeImage;  // XREF: R_InitCodeImages+281/w
                                        // R_InitCodeImages+286/r
    GfxLightDef *dlightDef;             // XREF: R_AddOmniLightToScene(float const * const,float const (* const)[3],float,float,float,float)+40E/r
                                        // R_InitLightDefs(void)+3C/w
    GfxLightDef *flashLightDef;         // XREF: R_AddSpotLightToScene(float const * const,float const (* const)[3],float,float,float,float)+61F/r
                                        // R_ShadowedSpotLightScore+F/r
    Material *defaultMaterial;          // XREF: Material_IsDefault(Material const *):loc_A4C6CF/r
                                        // Material_IsDefault(Material const *)+61/r ...
    Material *whiteMaterial;            // XREF: RB_DrawPoints2D+37/r
                                        // RB_DrawLines2D(int,int,GfxPointVertex const * const)+67/r ...
    Material *additiveMaterial;         // XREF: RB_DrawCoronaQuerySprite+264/r
                                        // RB_CalcSunSpriteSamples(void)+257/r ...
    Material *additiveMaterialNoDepth;  // XREF: RB_DrawCoronaQuerySprite+2B9/r
                                        // .rdata:s_builtInMaterials/o
    Material *pointMaterial;            // XREF: RB_DrawPoints3D+8/r
                                        // .rdata:s_builtInMaterials/o
    Material *lineMaterial;             // XREF: RB_DrawLines3D(int,int,GfxPointVertex const * const,bool)+10/r
                                        // RB_DebugPolyGetMaterialByDepthTest+2C/r ...
    Material *lineMaterialNoDepth;      // XREF: RB_DrawLines3D(int,int,GfxPointVertex const * const,bool)+23/r
                                        // RB_DebugPolyGetMaterialByDepthTest:loc_AEF99E/r ...
    Material *blendMaterial;            // XREF: RB_DebugPolyGetMaterialByDepthTest+13/r
                                        // .rdata:s_builtInMaterials/o
    Material *blendMaterialNoDepth;     // XREF: RB_StandardDrawCommands+C4F/r
                                        // RB_DebugPolyGetMaterialByDepthTest+45/r ...
    Material *whiteDebugExteriorMaterial;
                                        // XREF: RB_DrawDebugSphere:loc_AF08FD/r
                                        // .rdata:s_builtInMaterials/o
    Material *whiteDepthInteriorMaterial;
                                        // XREF: RB_DrawDebugSphere+CC2/r
                                        // .rdata:s_builtInMaterials/o
    Material *clearAlphaMaterial;       // XREF: R_DrawPointLitSurfsCallback+3C/r
                                        // .rdata:s_builtInMaterials/o
    Material *clearAlphaStencilMaterial;
                                        // XREF: .rdata:s_builtInMaterials/o
    Material *setAlphaMaterial;         // XREF: RB_SetFrameBufferAlpha+EB/r
                                        // RB_SetFrameBufferAlpha+170/r ...
    Material *shadowClearMaterial;      // XREF: R_DepthPrepassCallback+136/r
                                        // R_DrawSpotShadowMapCallback+76/r ...
    Material *shadowCasterMaterial;     // XREF: .rdata:s_builtInMaterials/o
    Material *shadowOverlayMaterial;    // XREF: RB_DrawSpotShadowOverlay(void)+1A0/r
                                        // RB_DrawSunShadowOverlay(void)+16D/r ...
    Material *depthPrepassMaterial;     // XREF: R_SunShadowMaps(GfxSunShadow *)+4C/r
                                        // R_SetPrepassMaterial+89/r ...
    Material *glareBlindMaterial;       // XREF: RB_DrawBlindAndGlare+D0/r
                                        // .rdata:s_builtInMaterials/o
    Material *lightCoronaMaterial;      // XREF: RB_DrawCoronas(int):loc_A8C212/r
                                        // .rdata:s_builtInMaterials/o
    Material *pixelCostAddDepthAlwaysMaterial;
                                        // XREF: R_PixelCost_GetAccumulationMaterial(Material const *):$LN10_118/r
                                        // .rdata:s_builtInMaterials/o
    Material *pixelCostAddDepthDisableMaterial;
                                        // XREF: R_PixelCost_GetAccumulationMaterial(Material const *)+F/r
                                        // R_PixelCost_GetAccumulationMaterial(Material const *)+22/r ...
    Material *pixelCostAddDepthEqualMaterial;
                                        // XREF: R_PixelCost_GetAccumulationMaterial(Material const *):$LN6_203/r
                                        // .rdata:s_builtInMaterials/o
    Material *pixelCostAddDepthLessMaterial;
                                        // XREF: R_PixelCost_GetAccumulationMaterial(Material const *):$LN8_146/r
                                        // .rdata:s_builtInMaterials/o
    Material *pixelCostAddDepthWriteMaterial;
                                        // XREF: R_PixelCost_GetAccumulationMaterial(Material const *):$LN3_250/r
                                        // .rdata:s_builtInMaterials/o
    Material *pixelCostAddNoDepthWriteMaterial;
                                        // XREF: R_PixelCost_GetAccumulationMaterial(Material const *):$LN4_249/r
                                        // R_PixelCost_GetAccumulationMaterial(Material const *):loc_B08C27/r ...
    Material *pixelCostColorCodeMaterial;
                                        // XREF: .rdata:s_builtInMaterials/o
    Material *stencilShadowMaterial;    // XREF: .rdata:s_builtInMaterials/o
    Material *stencilDisplayMaterial;   // XREF: .rdata:s_builtInMaterials/o
    Material *floatZDisplayMaterial;    // XREF: RB_ShowFloatZDebug+DE/r
                                        // .rdata:s_builtInMaterials/o
    Material *colorChannelMixerMaterial;
                                        // XREF: RB_ShowFbColorDebug_Feedback+297/r
                                        // RB_ShowFbColorDebug_Feedback+4AD/r ...
    Material *frameColorDebugMaterial;  // XREF: RB_ShowFbColorDebug_Screen+CF/r
                                        // RB_ShowFbColorDebug_Screen+13E/r ...
    Material *frameAlphaDebugMaterial;  // XREF: RB_ShowFbColorDebug_Screen+1FC/r
                                        // .rdata:s_builtInMaterials/o
    GfxImage *rawImage;                 // XREF: R_InitRawImage+11/w
                                        // R_InitRawImage+16/r
    GfxWorld *world;                    // XREF: TerrainScorch_AllowSurfacesCallback(int,void *)+C/r
                                        // GDT_RemoteXModelUpdate:loc_6DDAFA/r ...
    Material *feedbackReplaceMaterial;  // XREF: RB_StandardDrawCommandsCommon+1F4/r
                                        // RB_StandardDrawCommandsCommon+2DE/r ...
    Material *feedbackBlendMaterial;    // XREF: RB_BlurScreen+2BA/r
                                        // .rdata:s_builtInMaterials/o
    Material *feedbackFilmBlendMaterial;
                                        // XREF: .rdata:s_builtInMaterials/o
    Material *cinematicMaterial;        // XREF: .rdata:s_builtInMaterials/o
    Material *dofDownsampleMaterial;    // XREF: RB_GetDepthOfFieldInputImages:loc_B27F6D/r
                                        // .rdata:s_builtInMaterials/o
    Material *dofDownsampleNvIntzMaterial;
                                        // XREF: RB_GetDepthOfFieldInputImages+4C/r
                                        // .rdata:s_builtInMaterials/o
    Material *dofNearCocMaterial;       // XREF: RB_GetDepthOfFieldInputImages+CA/r
                                        // .rdata:s_builtInMaterials/o
    Material *smallBlurMaterial;        // XREF: RB_GetDepthOfFieldInputImages+11B/r
                                        // .rdata:s_builtInMaterials/o
    Material *postFxDofMaterial;        // XREF: RB_ApplyDepthOfField+3E4/r
                                        // .rdata:s_builtInMaterials/o
    Material *postFxDofNvIntzMaterial;  // XREF: RB_ApplyDepthOfField+3D0/r
                                        // .rdata:s_builtInMaterials/o
    Material *postFxDofColorMaterial;   // XREF: .rdata:s_builtInMaterials/o
    Material *postFxDofColorNvIntzMaterial;
                                        // XREF: .rdata:s_builtInMaterials/o
    Material *motionblur1Material;      // XREF: .rdata:s_builtInMaterials/o
    Material *motionblur2Material;      // XREF: .rdata:s_builtInMaterials/o
    Material *motionblur3Material;      // XREF: .rdata:s_builtInMaterials/o
    Material *motionblur4Material;      // XREF: .rdata:s_builtInMaterials/o
    Material *motionblur5Material;      // XREF: .rdata:s_builtInMaterials/o
    Material *motionblur6Material;      // XREF: .rdata:s_builtInMaterials/o
    Material *motionblur7Material;      // XREF: .rdata:s_builtInMaterials/o
    Material *motionblur8Material;      // XREF: .rdata:s_builtInMaterials/o
    Material *motionblurRadialMaterial; // XREF: .rdata:s_builtInMaterials/o
    Material *motionblurFrameBasedMaterial;
                                        // XREF: .rdata:s_builtInMaterials/o
    Material *redactMaterial1;          // XREF: DrawText2D+2195/r
                                        // SetupRedactFXVars+82/r ...
    Material *redactMaterial2;          // XREF: SetupRedactFXVars+8E/r
                                        // .rdata:s_builtInMaterials/o
    Material *redactMaterial3;          // XREF: SetupRedactFXVars+9B/r
                                        // .rdata:s_builtInMaterials/o
    Material *waterSheetingFXMaterial;  // XREF: RB_WaterSheetingFX(GfxViewInfo const *)+35A/r
                                        // .rdata:s_builtInMaterials/o
    Material *waterDropletMaterial;
    Material *reviveFXMaterial;         // XREF: RB_ReviveFX(GfxViewInfo const *)+5A3/r
                                        // .rdata:s_builtInMaterials/o
    Material *postFxColorMaterial;      // XREF: RB_BloomLDR(GfxViewInfo const *)+851/r
                                        // .rdata:s_builtInMaterials/o
    Material *postFxLutMaterial[10];    // XREF: .rdata:s_builtInMaterials/o
    Material *postFxMaterial;           // XREF: .rdata:s_builtInMaterials/o
    Material *poisonFXMaterial;         // XREF: RB_PoisonFX(GfxViewInfo const *)+545/r
                                        // RB_PickSymmetricFilterMaterial+3C/r ...
    Material *symmetricFilterMaterial[8];
                                        // XREF: .rdata:s_builtInMaterials/o
    Material *godRaysFilterMaterial;    // XREF: .rdata:s_builtInMaterials/o
    Material *shellShockBlurredMaterial;
                                        // XREF: RB_SaveScreen_BlendBlurred(GfxBlendSaveScreenBlurredParam const *,GfxViewInfo const *)+315/r
                                        // .rdata:s_builtInMaterials/o
    Material *shellShockFlashedMaterial;
                                        // XREF: RB_SaveScreen_BlendFlashed(GfxBlendSaveScreenFlashedParam const *,GfxViewInfo const *)+27C/r
                                        // .rdata:s_builtInMaterials/o
    Material *glowConsistentSetupMaterial;
                                        // XREF: .rdata:s_builtInMaterials/o
    Material *glowApplyBloomMaterial;   // XREF: .rdata:s_builtInMaterials/o
    Material *ropeMaterial;             // XREF: .rdata:s_builtInMaterials/o
    int savedScreenTimes[4];            // XREF: RB_SaveScreen_BlendBlurred(GfxBlendSaveScreenBlurredParam const *,GfxViewInfo const *)+AC/r
                                        // RB_SaveScreen(GfxSaveScreenParam const *,GfxViewInfo const *)+C3/w ...
    SavedScreenParams savedScreenParams[4];
                                        // XREF: RB_SaveScreen_BlendBlurred(GfxBlendSaveScreenBlurredParam const *,GfxViewInfo const *)+1EB/o
                                        // RB_SaveScreen_BlendFlashed(GfxBlendSaveScreenFlashedParam const *,GfxViewInfo const *)+152/o ...
    GfxImage *r32fWhiteImage;           // XREF: R_InitCodeImages+2C6/w
                                        // R_InitCodeImages+2CB/r ...
    Material *resampleFinal;            // XREF: RB_BloomLDR(GfxViewInfo const *)+1CA/r
                                        // .rdata:s_builtInMaterials/o
    Material *resampleShift;            // XREF: .rdata:s_builtInMaterials/o
    Material *bloomDownsampleLDR;       // XREF: RB_BloomLDR(GfxViewInfo const *)+164/r
                                        // .rdata:s_builtInMaterials/o
    Material *bloomDownsampleHDR;       // XREF: .rdata:s_builtInMaterials/o
    Material *bloomDownsampleConvolution;
                                        // XREF: RB_BloomLDR(GfxViewInfo const *)+328/r
                                        // .rdata:s_builtInMaterials/o
    Material *bloomRemap;               // XREF: RB_BloomLDR(GfxViewInfo const *)+3AF/r
                                        // .rdata:s_builtInMaterials/o
    Material *bloomExpansion;           // XREF: .rdata:s_builtInMaterials/o
    Material *bloomRemapStreak;         // XREF: RB_BloomStreak(GfxViewInfo const *,uchar &,uchar &)+46A/r
                                        // RB_BloomStreak(GfxViewInfo const *,uchar &,uchar &)+1F14/r ...
    Material *bloomStreakX;             // XREF: RB_BloomStreak(GfxViewInfo const *,uchar &,uchar &)+1BAF/r
                                        // .rdata:s_builtInMaterials/o
    Material *bloomStreakY;             // XREF: RB_BloomStreak(GfxViewInfo const *,uchar &,uchar &)+3448/r
                                        // .rdata:s_builtInMaterials/o
    Material *bloomApplyStreak;         // XREF: .rdata:s_builtInMaterials/o
    Material *bloomBlurX;               // XREF: RB_BloomLDR(GfxViewInfo const *)+447/r
                                        // RB_GenericFilterFX(GfxViewInfo const *)+1BF4/r ...
    Material *bloomBlurY;               // XREF: RB_BloomLDR(GfxViewInfo const *)+4DF/r
                                        // RB_GenericFilterFX(GfxViewInfo const *)+1CB4/r ...
    Material *bloomMip3Blur;            // XREF: .rdata:s_builtInMaterials/o
    Material *bloomApplyHDR;            // XREF: .rdata:s_builtInMaterials/o
    Material *bloomApplyLDR;            // XREF: RB_BloomLDR(GfxViewInfo const *)+801/r
                                        // .rdata:s_builtInMaterials/o
    Material *bloomSmoothA;             // XREF: RB_BloomLDR(GfxViewInfo const *)+761/r
                                        // .rdata:s_builtInMaterials/o
    Material *bloomSmoothB;             // XREF: .rdata:s_builtInMaterials/o
    Material *superFlareApply;          // XREF: RB_GenericFilterFX(GfxViewInfo const *)+1D4E/r
                                        // .rdata:s_builtInMaterials/o
    Material *superFlareOccluder;       // XREF: RB_DrawSuperFlareOccluders(GfxViewInfo const *):loc_B03C70/r
                                        // .rdata:s_builtInMaterials/o
    Material *superFlareOccluderDebug;  // XREF: RB_DrawSuperFlareOccluders(GfxViewInfo const *)+375/r
                                        // .rdata:s_builtInMaterials/o
    Material *superFlareOccluderNoZ;    // XREF: RB_DrawSuperFlareOccluders(GfxViewInfo const *):loc_B03C49/r
                                        // .rdata:s_builtInMaterials/o
    Material *superFlareOccluderNoZDebug;
                                        // XREF: RB_DrawSuperFlareOccluders(GfxViewInfo const *)+34E/r
                                        // .rdata:s_builtInMaterials/o
    Material *createLut2d;              // XREF: .rdata:s_builtInMaterials/o
    Material *createLut2dv;             // XREF: .rdata:s_builtInMaterials/o
    Material *applyLut2d;               // XREF: .rdata:s_builtInMaterials/o
    Material *applyLut3d;               // XREF: .rdata:s_builtInMaterials/o
    Material *dummyMaterial;            // XREF: Material_LoadBuiltIn:loc_A4E13F/w
                                        // .rdata:s_builtInMaterials/o
    Material *compositeResult;          // XREF: RB_StretchCompositeCmd+9C/r
                                        // .rdata:s_builtInMaterials/o
    Material *infraredWhite;            // XREF: R_AddDObjSurfacesCamera(GfxSceneEntity restrict *,uint,uint,GfxDrawSurf restrict * * const,GfxDrawSurf restrict * * const,uint,bool)+2FB/o
                                        // .rdata:s_builtInMaterials/o
    r_global_permap_assets_t permapAssets;
                                        // XREF: R_InitAssets_PostMapFastfileLoad(void)+5/w
                                        // R_InitAssets_PostMapFastfileLoad(void)+A/w ...
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

struct GfxClientFog // sizeof=0x194
{                                       // XREF: r_globals_t/r
    GfxFog settings[5];
    int index;                          // XREF: R_ClearFogs(void)+44/w
};

struct VertUsage // sizeof=0x8
{
    int index;
    VertUsage *next;
};

struct MaterialUsage // sizeof=0xC
{                                       // XREF: r_globals_t/r
    int memory;
    VertUsage *verts;                   // XREF: R_ShutdownMaterialUsage(void)+2D/r
                                        // R_ShutdownMaterialUsage(void)+64/w
    Material *material;
};

struct GfxLodRamp // sizeof=0xC
{                                       // XREF: GfxLodParms/r
    float scale;                        // XREF: R_AddAllStaticModelSurfacesSpotShadow(int,uint,uint)+6E/r
    float bias;                         // XREF: R_AddAllStaticModelSurfacesSpotShadow(int,uint,uint)+84/r
    float appliedInvScale;
};

struct __declspec(align(4)) GfxLodParms // sizeof=0x38
{                                       // XREF: r_globals_t/r
                                        // r_globals_t/r
    float origin[3];
    GfxLodRamp ramp[2];                 // XREF: R_AddAllStaticModelSurfacesSpotShadow(int,uint,uint)+6E/r
                                        // R_AddAllStaticModelSurfacesSpotShadow(int,uint,uint)+84/r
    bool valid;                         // XREF: R_RenderScene(refdef_s const *,int):loc_A62468/r
                                        // R_GenerateSortedDrawSurfs+453/r ...
    // padding byte
    // padding byte
    // padding byte
    float extraCamOrigin[3];            // XREF: R_GetBaseLodDist(float const * const)+25/o
    bool extraCamOriginValid;           // XREF: R_GetBaseLodDist(float const * const)+1A/r
    // padding byte
    // padding byte
    // padding byte
};

struct __declspec(align(8)) r_globals_t // sizeof=0x10A10
{                                       // XREF: .data:r_globals_t rg/r
    GfxViewParms identityViewParms;     // XREF: R_InitGlobalStructs+BD/o
                                        // R_InitGlobalStructs+CA/o ...
    bool inFrame;                       // XREF: R_AllocSceneModel(void):loc_A5BB75/r
                                        // R_AllocSceneBrush:loc_A5BED5/r ...
    bool registered;                    // XREF: R_InitSystems+30/w
                                        // R_ShutdownInternal(int)+4/r ...
    bool forbidDelayLoadImages;
    bool ignorePrecacheErrors;
    float viewOrg[3];                   // XREF: ShardGroup::GenerateVerts(bool,int)+69/o
                                        // R_RenderScene(refdef_s const *,int)+186/w ...
    float viewDir[3];                   // XREF: R_RenderScene(refdef_s const *,int)+1CA/w
                                        // R_RenderScene(refdef_s const *,int)+1DD/w ...
    unsigned int frontEndFrameCount;    // XREF: R_ToggleSmpFrame+3D/r
                                        // R_ToggleSmpFrame+46/w ...
    int totalImageMemory;
    Material *materialHashTable[4096];  // XREF: Material_Duplicate(Material *,char const *)+80/r
                                        // Material_Add+3E/r ...
    MaterialUsage materialUsage[4096];  // XREF: R_GetMaterialUsageData(Material const *)+2F/o
                                        // R_CreateMaterialList(void)+37/o ...
    GfxClientFog clientFogs[4];         // XREF: R_UpdateFrameFog+3E/o
                                        // R_ClearFogs(void)+2C/o ...
    GfxColor color_axis;                // XREF: R_UpdateTeamColors(float const * const,float const * const)+1A/o
                                        // RB_LookupColor+2A/r
    GfxColor color_allies;              // XREF: R_UpdateTeamColors(float const * const,float const * const)+9/o
                                        // RB_LookupColor+1D/r
    trStatistics_t *globalstats;        // XREF: R_TrackStatistics(trStatistics_t *)+6/w
                                        // RB_CopyBackendStats(void):loc_AAB3AF/r ...
    trViewStatistics_t *stats;          // XREF: RB_CopyBackendStats(void)+3/r
                                        // RB_CopyBackendStats(void)+C/r ...
    GfxLodParms lodParms;               // XREF: R_AddDObjSurfacesCamera(GfxSceneEntity restrict *,uint,uint,GfxDrawSurf restrict * * const,GfxDrawSurf restrict * * const,uint,bool)+783/o
                                        // R_SetLodOrigin(refdef_s const *):loc_A614A1/o ...
    GfxLodParms correctedLodParms[4];   // XREF: R_GenerateSortedDrawSurfs+48C/r
                                        // R_CorrectLodScale+F/o ...
    bool hasAnyImageOverrides;          // XREF: R_UpdateFrontEndDvarOptions+5C/w
                                        // R_SetPixelSamplerFromMaterial(GfxCmdBufContext const &,MaterialShaderArgument const *,MaterialTextureDef const *)+CC/r
    bool useSunLightOverride;           // XREF: R_UpdateFrameSun:loc_A61CC2/r
    bool useSunDirOverride;             // XREF: R_UpdateFrameSun+8B/r
                                        // R_UpdateFrameSun+1A5/r ...
    bool useSunDirLerp;                 // XREF: R_UpdateFrameSun+96/r
    float diffuseSunLightOverride[3];   // XREF: R_UpdateFrameSun+12F/r
                                        // R_UpdateFrameSun+13E/r ...
    float specularSunLightOverride[3];  // XREF: R_UpdateFrameSun+16B/r
                                        // R_UpdateFrameSun+17A/r ...
    float sunDirOverride[3];            // XREF: R_UpdateFrameSun+C7/o
                                        // R_UpdateFrameSun+E6/r ...
    float sunDirOverrideTarget[3];      // XREF: R_UpdateFrameSun+C2/o
    int sunDirLerpBeginTime;            // XREF: R_UpdateFrameSun+BB/r
    int sunDirLerpEndTime;              // XREF: R_UpdateFrameSun+B4/r
    const GfxViewParms *debugViewParms; // XREF: R_AddWorldSurfacesDpvs(GfxViewParms const *,int)+96/w
                                        // ShodLodInfo(float const * const,int,int,float,float,bool)+10C/r ...
    int endTime;                        // XREF: R_SetEndTime(int)+6/w
                                        // R_WaitEndTime(void)+1A/r
    bool distortion;                    // XREF: R_UpdateFrontEndDvarOptions+1BB/r
                                        // R_UpdateFrontEndDvarOptions+1D2/w
    // padding byte
    // padding byte
    // padding byte
    int renderHiResShot;                // XREF: CL_CGameRendering+3F7/r
                                        // CL_CGameRendering+3FF/w ...
    int hiResShotTiles;                 // XREF: ScrPlace_HiResGetScale(void)+C/r
                                        // ScrPlace_HiResApplyRect(float *,float *,float *,float *)+13/r ...
    float hiResShotRow;                 // XREF: ScrPlace_HiResApplyRect(float *,float *,float *,float *)+7E/r
                                        // ScrPlace_ApplyXinverse(ScreenPlacement const *,float,int)+21/r ...
    float hiResShotCol;                 // XREF: ScrPlace_HiResApplyRect(float *,float *,float *,float *)+A1/r
                                        // ScrPlace_ApplyYinverse(ScreenPlacement const *,float,int)+21/r ...
    bool drawWorld;                     // XREF: R_UpdateFrontEndDvarOptions+257/w
                                        // R_AddCellStaticSurfacesInFrustumCmd(void *)+67/r ...
    bool drawSModels;                   // XREF: R_UpdateFrontEndDvarOptions+274/w
                                        // R_AddAabbTreeSurfacesInFrustum_r:loc_B00B9B/r ...
    bool drawXModels;                   // XREF: R_DrawAllDynEnt(GfxViewInfo const *):loc_A41472/r
                                        // R_UpdateFrontEndDvarOptions+282/w
    bool drawBModels;                   // XREF: R_DrawAllDynEnt(GfxViewInfo const *):loc_A41634/r
                                        // R_UpdateFrontEndDvarOptions+266/w
    const char *codeImageNames[TEXTURE_SRC_CODE_COUNT]; // index = MaterialTextureSource
                                        // RB_InitCodeImages(void)+5B/w ...
    unsigned int viewInfoCount;         // XREF: R_GenerateSortedDrawSurfs+B/r
                                        // R_GenerateSortedDrawSurfs+4B/r ...
    int sunShadowFull;                  // XREF: R_SetupShadowSurfacesDpvs(GfxViewParms const *,float const (* const)[4],uint,int)+1E0/r
                                        // R_GenerateSortedDrawSurfs+185/w ...
    float sunShadowmapScale;            // XREF: R_GenerateSortedDrawSurfs+1A7/w
                                        // R_GenerateSortedDrawSurfs+217/r ...
    float sunShadowmapScaleNum;         // XREF: R_GenerateSortedDrawSurfs+37B/w
                                        // R_GenerateSortedDrawSurfs+431/w ...
    unsigned int sunShadowSize;         // XREF: R_GenerateSortedDrawSurfs+3ED/w
                                        // R_GenerateSortedDrawSurfs+413/w ...
    float sunShadowPartitionRatio;      // XREF: R_GenerateSortedDrawSurfs+403/w
                                        // R_GenerateSortedDrawSurfs+441/w ...
    int drawSunShadow;                  // XREF: R_GenerateSortedDrawSurfs:loc_A63019/w
                                        // R_GenerateSortedDrawSurfs+F2C/w
    bool isNotRenderingFullScreen;      // XREF: R_AddOmniLightToScene(float const * const,float const (* const)[3],float,float,float,float):loc_A5C9B3/r
                                        // R_GenerateSortedDrawSurfs+17F/w
    // padding byte
    // padding byte
    // padding byte
    int skinnedCacheReachedThreshold;   // XREF: R_UpdateSkinCacheUsage+4D/w
    bool enablePlayerShadowFlag;        // XREF: R_SetEnablePlayerShadowFlag(bool)+6/w
                                        // R_AddDObjSurfacesCamera(GfxSceneEntity restrict *,uint,uint,GfxDrawSurf restrict * * const,GfxDrawSurf restrict * * const,uint,bool)+86/r ...
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
};

struct __declspec(align(4)) GfxMetrics // sizeof=0x28
{                                       // XREF: .data:GfxMetrics gfxMetrics/r
    unsigned __int16 cubemapShotRes;    // XREF: R_BeginCubemapShot(int,int)+CC/w
                                        // R_CopyCubemapShot+45/r ...
    unsigned __int16 cubemapShotPixelBorder;
                                        // XREF: R_BeginCubemapShot(int,int)+D7/w
                                        // R_CubemapShotCopySurfaceToBufferHDR+49/r ...
    unsigned __int16 feedbackWidth;
    unsigned __int16 feedbackHeight;
    bool hasAnisotropicMinFilter;       // XREF: R_StoreDirect3DCaps+F4/w
                                        // R_SetTexFilter(void):loc_B0977A/r
    bool hasAnisotropicMagFilter;       // XREF: R_StoreDirect3DCaps+10C/w
                                        // R_SetTexFilter(void)+54/r
    // padding byte
    // padding byte
    int maxAnisotropy;                  // XREF: R_StoreDirect3DCaps+147/w
                                        // R_SetTexFilter(void)+14/r ...
    int maxClipPlanes;                  // XREF: R_StoreDirect3DCaps+DD/w
    bool hasHardwareShadowmap;          // XREF: R_SetShadowmapFormats_DX+139/w
                                        // R_SetShadowmapFormats_DX+178/w ...
    // padding byte
    // padding byte
    // padding byte
    _D3DFORMAT shadowmapFormatPrimary;  // XREF: R_SetShadowmapFormats_DX+123/w
                                        // R_SetShadowmapFormats_DX:loc_A5054D/w ...
    _D3DFORMAT shadowmapFormatSecondary;
                                        // XREF: R_SetShadowmapFormats_DX+12C/w
                                        // R_SetShadowmapFormats_DX+167/w ...
    unsigned __int8 shadowmapBuildTechType;
                                        // XREF: R_AddAllSceneEntSurfacesRangeSunShadow+3A/r
                                        // R_AddAllSceneEntSurfacesSpotShadow(GfxViewInfo const *,uint,uint)+6F/r ...
    unsigned __int8 shadowmapSamplerState;
                                        // XREF: R_SetShadowmapFormats_DX+140/w
                                        // R_SetShadowmapFormats_DX+17F/w ...
    bool slopeScaleDepthBias;           // XREF: R_StoreDirect3DCaps+15E/w
                                        // R_HW_SetPolygonOffset(IDirect3DDevice9 *,float,float)+6/r
    bool canMipCubemaps;                // XREF: R_StoreDirect3DCaps+175/w
                                        // Image_CreateCubeTexture_PC(GfxImage *,ushort,int,_D3DFORMAT)+89/r ...
    bool hasTransparencyMsaa;           // XREF: R_StoreDirect3DCaps+187/w
                                        // RB_UpdateBackEndDvarOptions+F1/r ...
    // padding byte
    // padding byte
    // padding byte
};

struct GfxAssets // sizeof=0x10
{                                       // XREF: .data:GfxAssets gfxAssets/r
    IDirect3DQuery9 *pixelCountQuery[4];
                                        // XREF: R_ReleaseForShutdownOrReset(void)+23D/r
                                        // R_ReleaseForShutdownOrReset(void)+26E/r ...
};

struct GfxDrawConsts // sizeof=0x60
{                                       // XREF: .data:GfxDrawConsts g_drawConsts/r
    float outdoorLookupMatrix[4][4];
    GfxScaledPlacement identityPlacement;
                                        // XREF: R_InitGlobalStructs+38/w
                                        // R_InitGlobalStructs+48/w ...
};

void __cdecl    R_FatalInitError(const char *msg);
void __cdecl    R_FatalLockError(HRESULT hr);
const char *__cdecl R_ErrorDescription(HRESULT hr);
void __cdecl R_SetColorMappings();
void __cdecl R_CalcGammaRamp(GfxGammaRamp *gammaRamp);
void __cdecl R_MakeDedicated(const GfxConfiguration *config);
void __cdecl SetGfxConfig(const GfxConfiguration *config);
void __cdecl R_ReleaseForShutdownOrReset();
void __cdecl R_UpdateGpuSyncType();
char __cdecl R_CreateDevice(const GfxWindowParms *wndParms);
void __cdecl R_SetD3DPresentParameters(_D3DPRESENT_PARAMETERS_ *d3dpp, const GfxWindowParms *wndParms);
void __cdecl R_SetupAntiAliasing(const GfxWindowParms *wndParms);
HRESULT __cdecl R_CreateDeviceInternal(HWND__ *hwnd, unsigned int behavior, _D3DPRESENT_PARAMETERS_ *d3dpp);
bool __cdecl R_GetMonitorDimensions(int *width, int *height);
int __cdecl R_GetDeviceType();
void __cdecl R_CheckResizeWindow();
void __cdecl R_ResizeWindow();
void __cdecl R_StoreWindowSettings(const GfxWindowParms *wndParms);
void __cdecl R_SetWndParms(GfxWindowParms *wndParms);
const char *__cdecl R_ClosestRefreshRateForMode(unsigned int width, unsigned int height, int refreshRate);
bool __cdecl R_SetCustomResolution(GfxWindowParms *wndParms);
void __cdecl R_AllocateMinimalResources();
void __cdecl R_SetIsMultiplayer(bool isMp);
void __cdecl R_InitThreads();
void __cdecl R_Init();
void R_InitGraphicsApi();
void R_InitSystems();
char __cdecl R_PreCreateWindow();
void __cdecl R_StoreDirect3DCaps(unsigned int adapterIndex);
void __cdecl R_GetDirect3DCaps(unsigned int adapterIndex, _D3DCAPS9 *caps);
void __cdecl R_PickRenderer(_D3DCAPS9 *caps);
bool __cdecl R_CheckTransparencyMsaa(unsigned int adapterIndex);
void __cdecl R_SetShadowmapFormats_DX(unsigned int adapterIndex);
void __cdecl R_EnumDisplayModes(unsigned int adapterIndex);
int __cdecl R_CompareDisplayModes(unsigned int *e0, unsigned int *e1);
int __cdecl R_AddValidRefreshRate(int refreshRate, int rateCount, int *availableRefreshRates);
int __cdecl R_CompareRefreshRates(unsigned int *e0, unsigned int *e1);
int __cdecl R_AddValidResolution(int width, int height, int resolutionCount, int (*availableResolutions)[2]);
unsigned int __cdecl R_ChooseAdapter();
HMONITOR__ *__cdecl R_ChooseMonitor();
int __stdcall R_MonitorEnumCallback(HMONITOR__ *monitorHandle, HDC__ *hdc, tagRECT *rect, unsigned int *userData);
char __cdecl R_CreateGameWindow(GfxWindowParms *wndParms);
char __cdecl R_InitHardware(const GfxWindowParms *wndParms);
void R_InitGamma();
void __cdecl R_FinishAttachingToWindow(const GfxWindowParms *wndParms);
char __cdecl R_CreateForInitOrReset();
IDirect3DQuery9 *__cdecl RB_HW_AllocOcclusionQuery();
char __cdecl R_CreateWindow(GfxWindowParms *wndParms);
char __cdecl R_ReduceWindowSettings();
// borderless: BO1Zombies's own full-screen borderless window (r_fullscreen 0 + r_borderless 1), see r_init.cpp
bool __cdecl R_IsBorderlessWindow();
void __cdecl R_BorderlessLogWindow(const char *when);
void R_Register();
void R_InitGlobalStructs();
void __cdecl R_ShutdownStreams();
void __cdecl R_ShutdownInternal();
void R_ShutdownDirect3D();
void R_ShutdownDirect3DInternal();
void R_UnloadGraphicsAssets();
void __cdecl R_Shutdown(int destroyWindow);
void __cdecl R_UnloadWorld();
void __cdecl R_BeginRegistrationInternal();
void __cdecl R_BeginRegistration(vidConfig_t *vidConfigOut);
void __cdecl R_EndRegistration();
void __cdecl R_TrackStatistics(trStatistics_t *stats);
void __cdecl R_SyncGpu();
void __cdecl R_UpdateTeamColors(const float *color_allies, const float *color_axis);
void __cdecl R_ConfigureRenderer(const GfxConfiguration *config);
char __cdecl R_ReleaseLostDeviceAssets2();
char __cdecl R_RecoverLostDevice();
char __cdecl R_ResetDevice();
void __cdecl R_ComErrorCleanup();
bool __cdecl R_StereoActivated();


extern GfxConfiguration gfxCfg;
extern vidConfig_t vidConfig;
extern DxGlobals dx;
extern r_global_permanent_t rgp;
extern r_globals_t rg;
extern GfxMetrics gfxMetrics;
extern GfxGlobals r_glob;

extern GfxWorldDpvsStatic *g_worldDpvs;
extern GfxWorldDraw *g_worldDraw;
extern GfxWorldDpvsPlanes *g_worldDpvsPlanes;
extern GfxAssets gfxAssets;

extern int g_disableRendering;

extern bool g_allocateMinimalResources;