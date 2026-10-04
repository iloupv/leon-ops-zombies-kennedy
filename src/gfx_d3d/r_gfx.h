#pragma once
#include <universal/com_math.h>

enum GfxDrawSceneMethod : __int32
{                                                                             // XREF: GfxDrawMethod/r
        GFX_DRAW_SCENE_NONE                = 0x0,
        GFX_DRAW_SCENE_FULLBRIGHT    = 0x1,
        GFX_DRAW_SCENE_DEBUGSHADER = 0x2,
        GFX_DRAW_SCENE_STANDARD        = 0x3,
};

enum $13F9397E79D9CCECD04953BDD0966834 : __int32
{
    DRAW_SURF_CAMERA_LIT_BEGIN       = 0x0,
    DRAW_SURF_BSP_CAMERA_LIT         = 0x0,
    DRAW_SURF_SMODEL_CAMERA_LIT      = 0x1,
    DRAW_SURF_ENT_CAMERA_LIT         = 0x2,
    DRAW_SURF_CAMERA_LIT_END         = 0x3,
    DRAW_SURF_CAMERA_DECAL_BEGIN     = 0x3,
    DRAW_SURF_BSP_CAMERA_DECAL       = 0x3,
    DRAW_SURF_SMODEL_CAMERA_DECAL    = 0x4,
    DRAW_SURF_ENT_CAMERA_DECAL       = 0x5,
    DRAW_SURF_FX_CAMERA_LIT          = 0x6,
    DRAW_SURF_FX_CAMERA_LIT_AUTO     = 0x7,
    DRAW_SURF_FX_CAMERA_LIT_DECAL    = 0x8,
    DRAW_SURF_CODEMESH_CAMERA_LIT    = 0x9,
    DRAW_SURF_CAMERA_DECAL_END       = 0xA,
    DRAW_SURF_CAMERA_EMISSIVE_BEGIN  = 0xA,
    DRAW_SURF_BSP_CAMERA_EMISSIVE    = 0xA,
    DRAW_SURF_SMODEL_CAMERA_EMISSIVE = 0xB,
    DRAW_SURF_ENT_CAMERA_EMISSIVE    = 0xC,
    DRAW_SURF_FX_CAMERA_EMISSIVE     = 0xD,
    DRAW_SURF_FX_CAMERA_EMISSIVE_AUTO = 0xE,
    DRAW_SURF_FX_CAMERA_EMISSIVE_DECAL = 0xF,
    DRAW_SURF_CAMERA_EMISSIVE_END    = 0x10,
    DRAW_SURF_REFLECTED_EMISSIVE_BEGIN = 0x10,
    DRAW_SURF_FX_REFLECTED_EMISSIVE  = 0x10,
    DRAW_SURF_FX_REFLECTED_EMISSIVE_AUTO = 0x11,
    DRAW_SURF_FX_REFLECTED_EMISSIVE_DECAL = 0x12,
    DRAW_SURF_REFLECTED_EMISSIVE_END = 0x13,
    DRAW_SURF_SUNSHADOW_0_BEGIN      = 0x13,
    DRAW_SURF_BSP_SUNSHADOW_0        = 0x13,
    DRAW_SURF_SMODEL_SUNSHADOW_0     = 0x14,
    DRAW_SURF_ENT_SUNSHADOW_0        = 0x15,
    DRAW_SURF_CODEMESH_SUNSHADOW_0   = 0x16,
    DRAW_SURF_SUNSHADOW_1_BEGIN      = 0x17,
    DRAW_SURF_BSP_SUNSHADOW_1        = 0x17,
    DRAW_SURF_SMODEL_SUNSHADOW_1     = 0x18,
    DRAW_SURF_ENT_SUNSHADOW_1        = 0x19,
    DRAW_SURF_CODEMESH_SUNSHADOW_1   = 0x1A,
    DRAW_SURF_SPOTSHADOW_0_BEGIN     = 0x1B,
    DRAW_SURF_BSP_SPOTSHADOW_0       = 0x1B,
    DRAW_SURF_SMODEL_SPOTSHADOW_0    = 0x1C,
    DRAW_SURF_ENT_SPOTSHADOW_0       = 0x1D,
    DRAW_SURF_SPOTSHADOW_1_BEGIN     = 0x1E,
    DRAW_SURF_BSP_SPOTSHADOW_1       = 0x1E,
    DRAW_SURF_SMODEL_SPOTSHADOW_1    = 0x1F,
    DRAW_SURF_ENT_SPOTSHADOW_1       = 0x20,
    DRAW_SURF_SPOTSHADOW_2_BEGIN     = 0x21,
    DRAW_SURF_BSP_SPOTSHADOW_2       = 0x21,
    DRAW_SURF_SMODEL_SPOTSHADOW_2    = 0x22,
    DRAW_SURF_ENT_SPOTSHADOW_2       = 0x23,
    DRAW_SURF_SPOTSHADOW_3_BEGIN     = 0x24,
    DRAW_SURF_BSP_SPOTSHADOW_3       = 0x24,
    DRAW_SURF_SMODEL_SPOTSHADOW_3    = 0x25,
    DRAW_SURF_ENT_SPOTSHADOW_3       = 0x26,
    DRAW_SURF_TYPE_COUNT             = 0x27,
};

struct __declspec(align(4)) GfxDrawMethod // sizeof=0x1C8
{                                                                             // XREF: .data:GfxDrawMethod gfxDrawMethod/r
        GfxDrawSceneMethod drawScene;             // XREF: R_GenerateSortedDrawSurfs+E1E/r
                                                                                // R_GenerateSortedDrawSurfs:loc_A63D75/r ...
        unsigned __int8 baseTechType;             // XREF: R_GenerateSortedDrawSurfs+181E/r
                                                                                // R_GenerateSortedDrawSurfs+18C4/r ...
        unsigned __int8 emissiveTechType;     // XREF: R_AddBModelSurfacesCamera(BModelDrawInfo restrict *,GfxBrushModel const restrict *,GfxDrawSurf restrict * * const,GfxDrawSurf restrict * * const,uint,float,float,uint):loc_A5D7E8/r
                                                                                // R_AddBModelSurfacesCamera(BModelDrawInfo restrict *,GfxBrushModel const restrict *,GfxDrawSurf restrict * * const,GfxDrawSurf restrict * * const,uint,float,float,uint)+8C/r ...
        unsigned __int8 litTechType[16][7][2][2];
                                                                                // XREF: R_AddMarkMeshDrawSurf(Material *,GfxMarkContext const *,ushort *,uint,float const * const,uint):loc_ACAC4B/r
                                                                                // R_SetDefaultLitTechTypes+D/o ...
        // padding byte
        // padding byte
};

struct GfxMatrix // sizeof=0x40
{                                       // ...
    float m[4][4];                      // ...

    inline GfxMatrix &operator=(const vector4 &rhs)
    {
        m[0][0] = rhs.x.v[0];
        m[0][1] = rhs.x.v[1];
        m[0][2] = rhs.x.v[2];
        m[0][3] = rhs.x.v[3];

        m[1][0] = rhs.y.v[0];
        m[1][1] = rhs.y.v[1];
        m[1][2] = rhs.y.v[2];
        m[1][3] = rhs.y.v[3];

        m[2][0] = rhs.z.v[0];
        m[2][1] = rhs.z.v[1];
        m[2][2] = rhs.z.v[2];
        m[2][3] = rhs.z.v[3];

        m[3][0] = rhs.w.v[0];
        m[3][1] = rhs.w.v[1];
        m[3][2] = rhs.w.v[2];
        m[3][3] = rhs.w.v[3];

        return *this;
    }
};

struct GfxViewParms // sizeof=0x140
{                                       // XREF: .data:lockPvsViewParms/r
    GfxMatrix viewMatrix;
    GfxMatrix projectionMatrix;         // XREF: R_InitGlobalStructs+BD/o
    GfxMatrix viewProjectionMatrix;     // XREF: R_InitGlobalStructs+CA/o
    GfxMatrix inverseViewProjectionMatrix;
    // XREF: R_InitGlobalStructs+D7/o
    float origin[4];                    // XREF: R_UpdateLodParms+16/r
    // R_UpdateLodParms+25/r ...
    float axis[3][3];                   // XREF: R_PvsLock_GetViewAxis(float (* const)[3])+7/o
    float depthHackNearClip;
    float zNear;
    float zFar;
};

struct GfxDrawSurfFields // sizeof=0x8
{                                                                             // XREF: GfxDrawSurf/r
        unsigned __int64 objectId : 16;
        unsigned __int64 fade : 4;
        unsigned __int64 customIndex : 5;
        unsigned __int64 reflectionProbeIndex : 3;
        unsigned __int64 hdrBits : 1;
        unsigned __int64 glightRender : 1;
        unsigned __int64 dlightRender : 1;
        unsigned __int64 materialSortedIndex : 12;
        unsigned __int64 primaryLightIndex : 8;
        unsigned __int64 surfType : 4;
        unsigned __int64 prepass : 2;
        unsigned __int64 noDynamicShadow : 1;
        unsigned __int64 primarySortKey : 6;
};

struct GfxFog // sizeof=0x50
{                                       // XREF: GfxBackEndData/r
    int startTime;
    int finishTime;
    float color[4];
    float fogStart;
    float density;
    float heightDensity;
    float baseHeight;
    float sunFogColor[4];
    float sunFogDir[3];
    float sunFogStartAng;
    float sunFogEndAng;
    float maxDensity;
};

union GfxDrawSurf // sizeof=0x8
{                                                                             // XREF: R_AddBModelSurfacesCamera(BModelDrawInfo restrict *,GfxBrushModel const restrict *,GfxDrawSurf restrict * * const,GfxDrawSurf restrict * * const,uint,float,float,uint)+1FC/w
                                                                                // R_AddBModelSurfacesCamera(BModelDrawInfo restrict *,GfxBrushModel const restrict *,GfxDrawSurf restrict * * const,GfxDrawSurf restrict * * const,uint,float,float,uint)+1FF/w ...
        GfxDrawSurfFields fields;
        unsigned __int64 packed;
};

struct GfxDelayedCmdBuf // sizeof=0x10
{                                                                             // XREF: GfxBspDrawSurfData/r
                                                                                // GfxSModelDrawSurfData/r ...
        int primDrawSurfPos;
        unsigned int primDrawSurfSize;
        GfxDrawSurf drawSurfKey;
};

struct GfxDrawSurfList // sizeof=0x8
{                                                                             // XREF: GfxBspDrawSurfData/r
        GfxDrawSurf *current;                             // XREF: R_GetStaticModelLightSurfs(GfxLight const *,int)+282/w
        GfxDrawSurf *end;                                     // XREF: R_GetStaticModelLightSurfs(GfxLight const *,int)+297/w
};

struct GfxBspDrawSurfData // sizeof=0x18
{                                                                             // XREF: R_GetBspLightSurfs(GfxLight const *,int)+6D/o
        GfxDelayedCmdBuf delayedCmdBuf;
        GfxDrawSurfList drawSurfList;             // XREF: R_AddAllBspDrawSurfacesRangeCamera+10D/w
};

struct GfxSModelDrawSurfLightingData // sizeof=0x28
{                                                                             // XREF: ?R_AddAllStaticModelSurfacesCamera@@YAXHHPBUGfxLight@@H_N@Z/r
        GfxDelayedCmdBuf delayedCmdBuf;
        GfxDrawSurfList drawSurf[3];                // XREF: R_AddAllStaticModelSurfacesCamera(int,int,GfxLight const *,int,bool)+15E/w
};

struct __declspec(align(4)) GfxLightImage // sizeof=0x8
{                                                                             // XREF: GfxLightDef/r
        struct GfxImage *image;
        unsigned __int8 samplerState;
        // padding byte
        // padding byte
        // padding byte
};

struct GfxLightDef // sizeof=0x10
{                                                                             // XREF: XAssetPoolEntry<GfxLightDef>/r
        const char *name;
        GfxLightImage attenuation;
        int lmapLookupStart;
};

struct __declspec(align(16)) GfxLight // sizeof=0x170
{                                                                             // XREF: refdef_s/r
                                                                                // PointLightPartition/r ...
        unsigned __int8 type;
        unsigned __int8 canUseShadowMap;
        __int16 cullDist;
        float color[3];
        float dir[3];
        float origin[3];
        float radius;
        float cosHalfFovOuter;
        float cosHalfFovInner;
        int exponent;
        unsigned int spotShadowIndex;
        float angles[3];
        float spotShadowHiDistance;
        float diffuseColor[4];
        float specularColor[4];
        float shadowColor[4];
        float falloff[4];
        float attenuation[4];
        float aAbB[4];
        float cookieControl0[4];
        float cookieControl1[4];
        float cookieControl2[4];
        // zombies: the exe's 4 padding bytes are real (viewMatrix at +0xE0, def at +0x160, sizeof 0x170). Without them
        // BO1Zombies's GfxLight was 0x160, so every exe-sized use (Load_GfxLight's 368-byte stream, the 368-stride
        // light copies, R_CullDynamicPointLightsInCameraView's decompiled offsets) read the wrong bytes: fx omni
        // lights (the perk machines' glow) were frustum-culled by a garbage origin (L2, a playtest 2026-09-24).
        // SP: Load_GfxWorld 0x00542360 streams the sun light as 0x170 and sets def at +0x160; R_AddOmniLightToScene
        // 0x006C02A0 and R_CullDynamicPointLightsInCameraView 0x006B51F0 step addedLight by 0x170; the spot-light adders
        // build viewMatrix at +0xE0 and projMatrix at +0x120.
        unsigned char pad_viewMatrix[4];
        float44 viewMatrix;
        float44 projMatrix;
        GfxLightDef *def;
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
static_assert(sizeof(GfxLight) == 0x170, "GfxLight must match the exe (zone stream, 368-stride copies)");
static_assert(offsetof(GfxLight, origin) == 0x1C && offsetof(GfxLight, radius) == 0x28, "GfxLight origin/radius");
static_assert(offsetof(GfxLight, viewMatrix) == 0xE0 && offsetof(GfxLight, def) == 0x160, "GfxLight viewMatrix/def");

struct GfxViewport // sizeof=0x10
{                                                                             // XREF: refdef_s/r
                                                                                // GfxCmdBufState/r ...
        int x;                                                            // XREF: RB_StandardDrawCommands+B5/w
                                                                                // RB_StandardDrawCommands+DA6/r ...
        int y;                                                            // XREF: RB_StandardDrawCommands+BB/w
                                                                                // RB_StandardDrawCommands+DAB/r ...
        int width;                                                    // XREF: RB_EndSceneRendering+C9/r
                                                                                // RB_EndSceneRendering+D2/r ...
        int height;                                                 // XREF: RB_StandardDrawCommands+C7/w
                                                                                // RB_StandardDrawCommands+DB7/r ...
};

struct GfxExposureValue // sizeof=0xB0
{                                                                             // XREF: refdef_s/r
                                                                                // GfxViewInfo/r ...
        float blackPoint[4];                                // XREF: R_UpdateExposureValue+A9/r
                                                                                // R_DefaultExposureValue(int)+14/w ...
        float whitePoint[4];                                // XREF: R_DefaultExposureValue(int)+78/w
                                                                                // R_DefaultExposureValue(int)+91/w ...
        float linearStart[4];                             // XREF: R_DefaultExposureValue(int)+DC/w
                                                                                // R_DefaultExposureValue(int)+F5/w ...
        float linearEnd[4];                                 // XREF: R_DefaultExposureValue(int)+140/w
                                                                                // R_DefaultExposureValue(int)+159/w ...
        float remapStart[4];                                // XREF: R_DefaultExposureValue(int)+1A4/w
                                                                                // R_DefaultExposureValue(int)+1BD/w ...
        float remapEnd[4];                                    // XREF: R_DefaultExposureValue(int)+208/w
                                                                                // R_DefaultExposureValue(int)+221/w ...
        float scurveStart[4];                             // XREF: R_DefaultExposureValue(int)+26C/w
                                                                                // R_DefaultExposureValue(int)+285/w ...
        float scurveEnd[4];                                 // XREF: R_DefaultExposureValue(int)+2D0/w
                                                                                // R_DefaultExposureValue(int)+2E9/w ...
        float bloomCurveLDR[4];                         // XREF: R_DefaultExposureValue(int)+334/w
                                                                                // R_DefaultExposureValue(int)+34D/w ...
        float bloomCurveHDR[4];                         // XREF: R_DefaultExposureValue(int)+398/w
                                                                                // R_DefaultExposureValue(int)+3B1/w ...
        float bloomScale[4];                                // XREF: R_DefaultExposureValue(int)+3FC/w
                                                                                // R_DefaultExposureValue(int)+415/w ...
};

struct GfxDepthOfField // sizeof=0x20
{                                       // XREF: refdef_s/r
                                        // GfxViewInfo/r ...
    float viewModelStart;
    float viewModelEnd;
    float nearStart;
    float nearEnd;
    float farStart;
    float farEnd;
    float nearBlur;
    float farBlur;
};

struct GfxDoubleVision // sizeof=0x1C
{                                       // XREF: refdef_s/r
                                        // GfxViewInfo/r ...
    float direction[3];
    float motionBlurMagnitude;
    float deltaPerMS;
    float cur;
    float targ;
};

struct __declspec(align(4)) GfxCompositeFx // sizeof=0x2C
{                                       // XREF: refdef_s/r refdef_s/r ...
    float distortionScale[2];
    float blurRadius;
    float distortionMagnitude;
    float frameRate;
    int lastUpdate;
    int frame;
    int startMSec;
    int currentTime;
    int duration;
    bool enabled;
    bool scriptEnabled;
    // padding byte
    // padding byte
};

struct GfxGenericFilter // sizeof=0x10BC
{                                       // XREF: refdef_s/r
                                        // GfxViewInfo/r ...
    bool passEnabled[3][16];
    struct Material *passMaterial[3][16];
    int passTarget[3][16];
    int passSampler0[3][16];
    int passSampler1[3][16];
    float passParam[3][16][16];
    int passQuads[3][16];
    float passFlareOcclusion[3][16];
    float sunPosition[3];
};

struct GfxPoison // sizeof=0x8
{                                       // XREF: refdef_s/r
                                        // GfxViewInfo/r ...
    float curAmountTarget;
    float curAmount;
};

struct __declspec(align(4)) GfxSaveScreenParam // sizeof=0x18
{                                       // XREF: GfxSaveScreenFx/r
    float s0;
    float t0;
    float ds;
    float dt;
    int screenTimerId;
    unsigned __int8 mode;
    // padding byte
    // padding byte
    // padding byte
};

struct __declspec(align(4)) GfxBlendSaveScreenBlurredParam // sizeof=0x1C
{                                       // XREF: GfxSaveScreenFx/r
    int fadeMsec;
    float s0;
    float t0;
    float ds;
    float dt;
    int screenTimerId;
    unsigned __int8 enabled;
    // padding byte
    // padding byte
    // padding byte
};

struct __declspec(align(4)) GfxBlendSaveScreenFlashedParam // sizeof=0x20
{                                       // XREF: GfxSaveScreenFx/r
    float intensityWhiteout;
    float intensityScreengrab;
    float s0;
    float t0;
    float ds;
    float dt;
    int screenTimerId;
    unsigned __int8 enabled;
    // padding byte
    // padding byte
    // padding byte
};

struct GfxSaveScreenFx // sizeof=0x54
{                                       // XREF: refdef_s/r
                                        // GfxViewInfo/r ...
    GfxSaveScreenParam saveScreenParam;
    GfxBlendSaveScreenBlurredParam blendBlurredParam;
    GfxBlendSaveScreenFlashedParam blendFlashedParam;
};

struct WaterFogDef // sizeof=0x4C
{                                       // XREF: refdef_s/r
    int startTime;
    int finishTime;
    float color[4];
    float fogStart;
    float density;
    float heightDensity;
    float baseHeight;
    float sunFogColor[4];
    float sunFogDir[3];
    float sunFogStartAng;
    float sunFogEndAng;
};

struct GfxPlacement // sizeof=0x1C
{                                       // XREF: GfxScaledPlacement/r
    float quat[4];                      // XREF: R_InitGlobalStructs+38/w
    float origin[3];                    // XREF: CG_Player(int,centity_s *)+D8D/w
};

struct GfxScaledPlacement // sizeof=0x20
{                                       // XREF: .data:GfxScaledPlacement s_manualObjectPlacement/r
    GfxPlacement base;                  // XREF: CG_Player(int,centity_s *)+D8D/w
    float scale;                        // XREF: CG_Player(int,centity_s *)+DD9/w
};

union GfxColor // sizeof=0x4
{                                                                             // XREF: RB_AddWaypoint+286/w
    operator unsigned int()
    {
        return packed;
    }
    GfxColor()
    {
        packed = 0;
    }
    GfxColor(int i)
    {
        packed = i;
    }
    GfxColor(unsigned int i)
    {
        packed = i;
    }
    unsigned int packed;
    unsigned __int8 array[4];
};

struct GfxWorldVertex // sizeof=0x2C
{                                       // XREF: GfxWorldVertex0/r
                                        // GfxWorldVertex1/r
    float xyz[3];
    float binormalSign;
    GfxColor color;
    float texCoord[2];
    float lmapCoord[2];
    PackedUnitVec normal;
    PackedUnitVec tangent;
};

struct GfxBrushModelWritable // sizeof=0x1C
{                                       // XREF: GfxBrushModel/r
    float mins[3];
    float maxs[3];
    float mip1radiusSq;
};

struct GfxBrushModel // sizeof=0x3C
{
    GfxBrushModelWritable writable;
    float bounds[2][3];
    unsigned int surfaceCount;
    unsigned int startSurfIndex;
};

struct srfTriangles_t // sizeof=0x30
{                                       // XREF: GfxSurface/r
    float mins[3];
    int vertexLayerData;
    float maxs[3];
    int firstVertex;
    unsigned __int16 vertexCount;
    unsigned __int16 triCount;
    int baseIndex;
    float himipRadiusSq;
    int stream2ByteOffset;
};

struct GfxSurface // sizeof=0x50
{                                       // XREF: ??$_Insertion_sort1@PAUGfxSurface@@P6A_NABU1@0@ZU1@@std@@YAXPAUGfxSurface@@0P6A_NABU1@1@Z0@Z/r
                                        // ??$_Adjust_heap@PAUGfxSurface@@HU1@P6A_NABU1@0@Z@std@@YAXPAUGfxSurface@@HHU1@P6A_NABU1@2@Z@Z/r ...
    srfTriangles_t tris;
    Material *material;
    unsigned __int8 lightmapIndex;
    unsigned __int8 reflectionProbeIndex;
    unsigned __int8 primaryLightIndex;
    unsigned __int8 flags;
    float bounds[2][3];
};

struct GfxLightmapArray // sizeof=0xC
{
    GfxImage *primary;
    GfxImage *secondary;
    GfxImage *secondaryB;
};

struct GfxSceneDef // sizeof=0x14
{                                       // XREF: GfxViewInfo/r
    int time;                           // XREF: R_SetLodOrigin(refdef_s const *)+80/w
    // R_SetLodOrigin(refdef_s const *)+86/r ...
    float floatTime;                    // XREF: R_SetLodOrigin(refdef_s const *)+96/w
    // R_RenderScene(refdef_s const *,int)+264/r ...
    float viewOffset[3];                // XREF: R_SetLodOrigin(refdef_s const *)+52/w
    // R_SetLodOrigin(refdef_s const *)+62/w ...
};