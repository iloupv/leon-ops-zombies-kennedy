#pragma once

#include "r_material.h"
#include "r_rendercmds.h"
#include "rb_state.h"
#include "r_init.h"
#include "r_font.h"
#include "r_rendertarget.h"

#include <nvapi/nvapi.h>
#include "r_shader_constant_set.h"

#include <qcommon/threads.h>

#define SAMPLER_FILTER_MASK 7

enum GfxPrimStatsTarget : __int32
{                                       // XREF: ?RB_DrawFullSceneQuad@@YAXPBUMaterial@@EMMMMW4GfxPrimStatsTarget@@@Z/r
                                        // ?RB_DrawStretchPic@@YAXPBUMaterial@@MMMMMMMMKW4GfxPrimStatsTarget@@@Z/r ...
    GFX_PRIM_STATS_WORLD         = 0x0,
    GFX_PRIM_STATS_SMODELCACHED  = 0x1,
    GFX_PRIM_STATS_SMODELRIGID   = 0x2,
    GFX_PRIM_STATS_XMODELRIGID   = 0x3,
    GFX_PRIM_STATS_XMODELSKINNED = 0x4,
    GFX_PRIM_STATS_BMODEL        = 0x5,
    GFX_PRIM_STATS_FX            = 0x6,
    GFX_PRIM_STATS_HUD           = 0x7,
    GFX_PRIM_STATS_DEBUG         = 0x8,
    GFX_PRIM_STATS_CODE          = 0x9,
    GFX_PRIM_STATS_COUNT         = 0xA,
};

struct GfxRenderCommandExecState // sizeof=0x4
{                                       // XREF: ?RB_ExecuteRenderCommandsLoop@@YAXPBXPAH@Z/r
    const void *cmd;                    // XREF: RB_ExecuteRenderCommandsLoop(void const *,int *)+C1/w
};

struct GfxVertex // sizeof=0x20
{                                       // XREF: materialCommands_t/r
    float xyzw[4];                      // XREF: RB_SetPolyVert(float const * const,GfxColor,int)+44/w
    GfxColor color;                     // XREF: RB_SetPolyVert(float const * const,GfxColor,int)+65/w
                                        // RB_DrawDebugSphere+EAF/w
    float texCoord[2];                  // XREF: RB_SetPolyVert(float const * const,GfxColor,int)+71/o
                                        // RB_DrawDebugSphere+EBE/o
    PackedUnitVec normal;               // XREF: RB_SetPolyVert(float const * const,GfxColor,int)+52/w
};

struct GfxDrawSurfListArgs // sizeof=0x10
{                                       // XREF: ?R_DrawSurfs@@YAXUGfxCmdBufContext@@PAUGfxCmdBufState@@PBUGfxDrawSurfListInfo@@@Z/r
    GfxCmdBufContext context;           // XREF: R_DrawSurfs(GfxCmdBufContext,GfxCmdBufState *,GfxDrawSurfListInfo const *)+4FE/w
                                        // R_DrawSurfs(GfxCmdBufContext,GfxCmdBufState *,GfxDrawSurfListInfo const *)+504/w
    unsigned int firstDrawSurfIndex;    // XREF: R_DrawSurfs(GfxCmdBufContext,GfxCmdBufState *,GfxDrawSurfListInfo const *)+507/w
                                        // R_DrawSurfs(GfxCmdBufContext,GfxCmdBufState *,GfxDrawSurfListInfo const *):loc_AAF03B/r ...
    const GfxDrawSurfListInfo *info;    // XREF: R_DrawSurfs(GfxCmdBufContext,GfxCmdBufState *,GfxDrawSurfListInfo const *)+511/w
};

struct GfxPointVertex // sizeof=0x10
{                                       // XREF: .data:g_debugExternLineVerts/r
    float xyz[3];
    unsigned __int8 color[4];           // XREF: RB_DrawSunShadowOverlay(void)+32F/w
};

struct GfxCmdDrawPoints // sizeof=0x18
{
    GfxCmdHeader header;
    __int16 pointCount;
    unsigned __int8 size;
    unsigned __int8 dimensions;
    GfxPointVertex verts[1];
};

struct r_backEndGlobals_t // sizeof=0x28C
{                                       // XREF: .data:r_backEndGlobals_t backEnd/r
    int glowCount;
    GfxImage *glowImage;
    GfxImage *godRaysImage;
    Font_s *debugFont;                  // XREF: RB_RegisterBackendAssets(void)+12/w
                                        // RB_DrawDebugStrings+128/r ...
    GfxFrameStats frameStatsMax;        // XREF: RB_ResetStatTracking(int)+3/o
                                        // RB_Stats_f(void)+5E/o ...
};

struct __declspec(align(4)) materialCommands_t // sizeof=0x22A95C
{                                       // XREF: .data:materialCommands_t tess/r
    GfxVertex verts[5450];              // XREF: RB_SetPolyVert(float const * const,GfxColor,int)+44/w
                                        // RB_SetPolyVert(float const * const,GfxColor,int)+52/w ...
    unsigned __int16 indices[1048576];  // XREF: RB_SetTessQuad+75/w
                                        // RB_SetTessQuad+87/w ...
    MaterialVertexDeclType vertDeclType;
    unsigned int vertexSize;
    int indexCount;                     // XREF: RB_CheckTessOverflow(int,int)+78/r
                                        // RB_SetFrameBufferAlpha+20/r ...
    int vertexCount;                    // XREF: RB_CheckTessOverflow(int,int):loc_857149/r
                                        // RB_SetTessQuad+6/r ...
    int firstVertex;                    // XREF: RB_BeginSurface(Material const *,uchar):loc_AC70D1/w
    int lastVertex;                     // XREF: RB_BeginSurface(Material const *,uchar)+10B/w
    bool finishedFilling;               // XREF: RB_EndSurfacePrologue:loc_AC71E2/w
                                        // RB_EndSurfaceEpilogue+6B/w
    // padding byte
    // padding byte
    // padding byte
};

struct GfxCmdStretchComposite // sizeof=0x2C
{
    GfxCmdHeader header;
    GfxImage *image;
    float x;
    float y;
    float w;
    float h;
    float s0;
    float t0;
    float s1;
    float t1;
    GfxColor color;
};

struct GfxCmdStretchPic // sizeof=0x30
{
    GfxCmdHeader header;
    const Material *material;
    float x;
    float y;
    float w0;
    float w;
    float h;
    float s0;
    float t0;
    float s1;
    float t1;
    GfxColor color;
};

struct GfxCmdStretchPicRotateXY // sizeof=0x34
{
    GfxCmdHeader header;
    const Material *material;
    float x;
    float y;
    float w0;
    float w;
    float h;
    float s0;
    float t0;
    float s1;
    float t1;
    GfxColor color;
    float rotation;
};

struct GfxCmdStretchPicRotateST // sizeof=0x34
{
    GfxCmdHeader header;
    const Material *material;
    float x;
    float y;
    float w;
    float h;
    float centerS;
    float centerT;
    float radiusST;
    float scaleFinalS;
    float scaleFinalT;
    GfxColor color;
    float rotation;
};

struct GfxCmdDrawQuadPic // sizeof=0x30
{
    GfxCmdHeader header;
    const Material *material;
    float verts[4][2];
    float w;
    GfxColor color;
};

struct GfxCmdDrawEmblemLayer // sizeof=0x68
{
    GfxCmdHeader header;
    const Material *material;
    const GfxImage *image;
    int colorIdx;
    float outlineSize;
    float smoothSize;
    GfxQuadVertex verts[4];
};

struct GfxCmdDrawFramed2D // sizeof=0x2C
{
    GfxCmdHeader header;
    float x;
    float y;
    float w;
    float h;
    float thicknessW;
    float thicknessH;
    float thicknessTex;
    int sides;
    GfxColor color;
    const Material *material;
};
         
struct GfxCmdPCCopyImageGenMIP // sizeof=0x10
{
    GfxCmdHeader header;
    void (__cdecl *callback)(void *);
    GfxImage *image;
    void *job;
};

struct GfxCachedSModelSurf // sizeof=0x8
{                                       // XREF: static_model_leaf_t/r
    unsigned int baseVertIndex;
    unsigned __int16 lodIndex;
    unsigned __int16 smodelIndex;
};

struct static_model_node_list_t // sizeof=0x8
{                                       // XREF: static_model_leaf_t/r
    static_model_node_list_t *prev;     // XREF: SMC_GetFreeBlockOfSize+94/r
    static_model_node_list_t *next;
};

union static_model_leaf_t // sizeof=0x8
{                                       // XREF: static_model_cache_t/r
    GfxCachedSModelSurf cachedSurf;
    static_model_node_list_t freenode;
};

struct static_model_tree_list_t // sizeof=0x8
{                                       // XREF: static_model_tree_t/r
    static_model_tree_list_t *prev;     // XREF: SMC_ForceFreeBlock+9/r
    static_model_tree_list_t *next;     // XREF: SMC_ClearCache+90/w
};

struct static_model_node_t // sizeof=0x4
{                                       // XREF: static_model_tree_t/r
    __int16 usedVerts;
    bool inuse;
    unsigned __int8 reserved;
};

struct static_model_tree_t // sizeof=0x108
{                                       // XREF: static_model_cache_t/r
    static_model_tree_list_t usedlist;
    unsigned int frameCount;
    static_model_node_t nodes[63];
};

struct ScopedShaderConstantSetUndo;

struct GfxCmdResolveComposite // sizeof=0x8
{
    GfxCmdHeader header;
    void (__cdecl *callback)(GfxImage *);
};

struct GfxCmdSetMaterialColor // sizeof=0x14
{
    GfxCmdHeader header;
    float color[4];
};

enum CodeConstant : __int32;
struct GfxCmdSetCustomConstant // sizeof=0x18
{
    GfxCmdHeader header;
    //unsigned int type;
    CodeConstant type;
    float vec[4];
};

struct GfxCmdDrawLines // sizeof=0x28
{
    GfxCmdHeader header;
    __int16 lineCount;
    unsigned __int8 width;
    unsigned __int8 dimensions;
    GfxPointVertex verts[2];
};

void __cdecl RB_CopyBackendStats();
void __cdecl RB_SetIdentity();
void __cdecl R_SetVertex2d(GfxVertex *vert, float x, float y, float s, float t, unsigned int color);
void __cdecl RB_DrawFullSceneQuad(
                const Material *material,
                unsigned __int8 renderTargetId,
                float s0,
                float t0,
                float s1,
                float t1,
                GfxPrimStatsTarget statsTarget);
void __cdecl RB_DrawSW4Quads(const Material *material, int ParticleCount);
void __cdecl RB_DrawStretchPic(
                const Material *material,
                float x,
                float y,
                float w,
                float h,
                float s0,
                float t0,
                float s1,
                float t1,
                unsigned int color,
                GfxPrimStatsTarget statsTarget);
void __cdecl RB_DrawStretchPicW(
                const Material *material,
                float x,
                float y,
                float w0,
                float w,
                float h,
                float s0,
                float t0,
                float s1,
                float t1,
                unsigned int color,
                GfxPrimStatsTarget statsTarget);
void __cdecl R_SetVertex4d(GfxVertex *vert, float x, float y, float z, float w, float s, float t, unsigned int color);
void __cdecl RB_DrawStretchPicZ(
                const Material *material,
                float x,
                float y,
                float z,
                float w,
                float h,
                float s0,
                float t0,
                float s1,
                float t1,
                unsigned int color,
                GfxPrimStatsTarget statsTarget);
void __cdecl R_SetVertex3d(GfxVertex *vert, float x, float y, float z, float s, float t, const unsigned __int8 *color);
void __cdecl RB_DrawFullScreenColoredQuad(
                const Material *material,
                float s0,
                float t0,
                float s1,
                float t1,
                unsigned int color);
void __cdecl RB_FullScreenColoredFilter(const Material *material, unsigned int color);
void __cdecl RB_FullScreenFilter(const Material *material);
void __cdecl RB_Filter(const Material *material, const GfxViewInfo *viewInfo);
void __cdecl RB_ColoredFilter(const Material *material, const GfxViewInfo *viewInfo, unsigned int color);
void __cdecl RB_SplitScreenFilter(const Material *material, const GfxViewInfo *viewInfo, unsigned int color);
void __cdecl RB_SplitScreenTexCoords(float x, float y, float w, float h, float *s0, float *t0, float *s1, float *t1);
void R_ResolveIntZ_PC();
void __cdecl R_Resolve(GfxCmdBufContext context, GfxImage *image);
void __cdecl RB_StretchCompositeCmd(GfxRenderCommandExecState *execState);
void __cdecl R_SetCodeImageSamplerState(
                GfxCmdBufSourceState *source,
                unsigned int codeTexture,
                unsigned __int8 samplerState);
void __cdecl RB_StretchPicCmd(GfxRenderCommandExecState *execState);
void __cdecl RB_StretchPicCmdFlipST(GfxRenderCommandExecState *execState);
void __cdecl RB_DrawStretchPicFlipST(
                const Material *material,
                float x,
                float y,
                float w,
                float h,
                float s0,
                float t0,
                float s1,
                float t1,
                unsigned int color,
                GfxPrimStatsTarget statsTarget);
void __cdecl RB_StretchPicRotateXYCmd(GfxRenderCommandExecState *execState);
void __cdecl RB_StretchPicRotateSTCmd(GfxRenderCommandExecState *execState);
void __cdecl RB_DrawQuadPicCmd(GfxRenderCommandExecState *execState);
void __cdecl RB_DrawQuadList2DCmd(GfxRenderCommandExecState *execState);
void __cdecl RB_DrawEmblemLayer(GfxRenderCommandExecState *execState);
void __cdecl RB_DrawFullScreenColoredQuadCmd(GfxRenderCommandExecState *execState);
void __cdecl RB_StretchRawCmd(GfxRenderCommandExecState *execState);
void __cdecl RB_StretchRaw(int x, int y, int w, int h, int cols, int rows, const unsigned __int8 *data);
void __cdecl RB_DrawFramedCmd(GfxRenderCommandExecState *execState);
void __cdecl TessQuad(unsigned __int16 v0, unsigned __int16 v1, unsigned __int16 v2, unsigned __int16 v3);
void __cdecl RB_SetEyeOffsetConstant(GfxCmdBufSourceState *source, const float *drawSurfListViewOrigin);
void __cdecl R_DrawSurfs(GfxCmdBufContext context, GfxCmdBufState *prepassState, const GfxDrawSurfListInfo *info);
unsigned int __cdecl R_RenderDrawSurfListMaterial(const GfxDrawSurfListArgs *listArgs, GfxCmdBufContext prepassContext);
unsigned int __cdecl R_SkipDrawSurfListMaterial(const GfxDrawSurf *drawSurfList, unsigned int drawSurfCount);
void __cdecl R_TessEnd(GfxCmdBufContext context, GfxCmdBufContext prepassContext);
void __cdecl RB_ClearScreenCmd(GfxRenderCommandExecState *execState);
void __cdecl RB_SetGammaRamp(const GfxGammaRamp *gammaTable);
void __cdecl RB_BlendSavedScreenFlashedCmd(GfxRenderCommandExecState *execState);
void __cdecl RB_DrawPointsCmd(GfxRenderCommandExecState *execState);
void __cdecl RB_DrawPoints2D(const GfxCmdDrawPoints *cmd);
void __cdecl RB_DrawPoints3D(const GfxCmdDrawPoints *cmd);
void __cdecl RB_DrawLines2D(int count, int width, const GfxPointVertex *verts);
void __cdecl RB_DrawLines3D(int count, int width, const GfxPointVertex *verts, bool depthTest);
void __cdecl RB_DrawLinesCmd(GfxRenderCommandExecState *execState);
void __cdecl RB_DrawTrianglesCmd(GfxRenderCommandExecState *execState);
void __cdecl RB_DrawTriangles_Internal(
                const Material *material,
                unsigned __int8 techType,
                __int16 indexCount,
                const unsigned __int16 *indices,
                __int16 vertexCount,
                const float (*xyzw)[4],
                const float (*normal)[3],
                const GfxColor *color,
                const float (*st)[2]);
void __cdecl R_SetVertex4dWithNormal(
                GfxVertex *vert,
                float x,
                float y,
                float z,
                float w,
                float nx,
                float ny,
                float nz,
                float s,
                float t,
                const unsigned __int8 *color);
void __cdecl RB_SetCustomConstantCmd(GfxRenderCommandExecState *execState);
void __cdecl RB_SetMaterialColorCmd(GfxRenderCommandExecState *execState);
void __cdecl RB_SetViewportCmd(GfxRenderCommandExecState *execState);
void __cdecl RB_SetScissorCmd(GfxRenderCommandExecState *execState);
void __cdecl RB_ResolveCompositeCmd(GfxRenderCommandExecState *execState);
void __cdecl RB_PCCopyImageGenMIPCmd(GfxRenderCommandExecState *execState);
void __cdecl GetShiftColor(
                GfxColor targetColor,
                int fxBirthTime,
                int fxDecayStart,
                int fxDecayDuration,
                GfxColor startColor,
                GfxColor *shiftColor);
void __cdecl RB_DrawText(const char *text, Font_s *font, float x, float y, GfxColor color);
void __cdecl DrawText2D(
                const char *text,
                float x,
                float y,
                float w,
                Font_s *font,
                float xScale,
                float yScale,
                float sinAngle,
                float cosAngle,
                GfxColor color,
                int maxLength,
                int renderFlags,
                int cursorPos,
                char cursorLetter,
                float padding,
                GfxColor glowForcedColor,
                int fxBirthTime,
                int fxLetterTime,
                int fxDecayStartTime,
                int fxDecayDuration,
                int fxRedactDecayStartTime,
                int fxRedactDecayDuration,
                const Material *fxMaterial,
                const Material *fxMaterialGlow);
void __cdecl RB_DrawStretchPicRotate(
                const Material *material,
                float x,
                float y,
                float w,
                float width,
                float height,
                float s0,
                float t0,
                float s1,
                float t1,
                float sinAngle,
                float cosAngle,
                unsigned int color,
                GfxPrimStatsTarget statsTarget);
void __cdecl RB_LookupColor(unsigned __int8 c, unsigned __int8 *color);
double __cdecl RB_DrawHudIcon(
                const char *text,
                float x,
                float y,
                float sinAngle,
                float cosAngle,
                Font_s *font,
                float xScale,
                float yScale,
                unsigned int color);
GfxColor __cdecl RB_ProcessLetterColor(unsigned int color, unsigned int letter);
void __cdecl RB_DrawCursor(
                const Material *material,
                unsigned __int8 cursor,
                float x,
                float y,
                float sinAngle,
                float cosAngle,
                Font_s *font,
                float xScale,
                float yScale,
                unsigned int color);
void __cdecl RotateXY(
                float cosAngle,
                float sinAngle,
                float pivotX,
                float pivotY,
                float x,
                float y,
                float *outX,
                float *outY);
double __cdecl GetMonospaceWidth(Font_s *font, char renderFlags);
void __cdecl GlowColor(GfxColor *result, GfxColor baseColor, GfxColor forcedGlowColor, char renderFlags);
char __cdecl SetupFadeinFXVars(
                const char *text,
                int maxLength,
                int renderFlags,
                int fxBirthTime,
                int fxLetterTime,
                int fxDecayStartTime,
                int fxDecayDuration,
                bool *resultDrawRandChar,
                int *resultRandSeed,
                bool *resultDecaying,
                int *resultdecayTimeElapsed);
char __cdecl SetupCOD7DecodeFXVars(
                const char *text,
                int maxLength,
                int fxBirthTime,
                int fxLetterTime,
                int fxDecayStartTime,
                int fxDecayDuration,
                int *randSeed,
                int *resultRandomCharsLength,
                int *resultMaxLength,
                bool *resultDecaying,
                int *resultDecayTimeElapsed);
char __cdecl SetupTypewriterFXVars(
                const char *text,
                int maxLength,
                int renderFlags,
                int fxBirthTime,
                int fxLetterTime,
                int fxDecayStartTime,
                int fxDecayDuration,
                bool *resultDrawRandChar,
                int *resultRandSeed,
                int *resultMaxLength,
                bool *resultDecaying,
                int *resultdecayTimeElapsed);
char __cdecl SetupPopInFXVars(
                const char *text,
                int maxLength,
                int renderFlags,
                int fxBirthTime,
                int fxLetterTime,
                int fxDecayStartTime,
                int fxDecayDuration,
                float *sizeIncrease,
                int *resultRandSeed,
                int *resultMaxLength,
                bool *resultDecaying,
                int *resultdecayTimeElapsed);
char __cdecl SetupPulseFXVars(
                const char *text,
                int maxLength,
                char renderFlags,
                int fxBirthTime,
                int fxLetterTime,
                int fxDecayStartTime,
                int fxDecayDuration,
                bool *resultDrawRandChar,
                int *resultRandSeed,
                int *resultMaxLength,
                bool *resultDecaying,
                int *resultdecayTimeElapsed);
char __cdecl SetupRedactTextFXVars(
                const char *text,
                int maxLength,
                int renderFlags,
                int fxBirthTime,
                int fxLetterTime,
                int fxDecayStartTime,
                int fxDecayDuration,
                bool *resultDrawRandChar,
                int *resultRandSeed,
                int *resultMaxLength,
                bool *resultDecaying,
                int *resultdecayTimeElapsed);
void __cdecl SetupRedactFXVars(
                int *randSeed,
                int fxBirthTime,
                int maxLength,
                int fxRedactDecayStartTime,
                int fxRedactDecayDuration,
                int mode,
                float *src_rect,
                float *max_rect,
                float *out_rect,
                float *u,
                Material **mat);
void __cdecl GetDecayingStringAlphaInfo(
                int decayTimeElapsed,
                int fxDecayDuration,
                unsigned __int8 alpha,
                unsigned __int8 *resultAlpha);
void __cdecl DrawTextFxExtraCharacter(
                const Material *material,
                int charIndex,
                float x,
                float y,
                float w,
                float h,
                float sinAngle,
                float cosAngle,
                unsigned int color);
int __cdecl ModulateByteColors(unsigned __int8 colorA, unsigned __int8 colorB);
void __cdecl RB_DrawTextInSpace(
                const char *text,
                Font_s *font,
                const float *org,
                const float *xPixelStep,
                const float *yPixelStep,
                unsigned int color);
void __cdecl RB_DrawCharInSpace(
                const Material *material,
                float *xyz,
                const float *dx,
                const float *dy,
                const Glyph *glyph,
                unsigned int color);
void __cdecl RB_DrawText2DCmd(GfxRenderCommandExecState *execState);
void __cdecl RB_DrawText3DCmd(GfxRenderCommandExecState *execState);
void __cdecl RB_ProjectionSetCmd(GfxRenderCommandExecState *execState);
void __cdecl RB_ResetStatTracking(int viewIndex);
void __cdecl RB_BeginFrame(GfxBackEndData *data);
const GfxBackEndData *RB_PatchStaticModelCache();
void __cdecl RB_EndFrame(char drawType);
void RB_SwapBuffers();
void RB_UpdateBackEndDvarOptions();
void __cdecl RB_ExecuteRenderCommandsLoop(const void *cmds, int *ui3dTextureWindow);
void __cdecl RB_Draw3D();
void __cdecl RB_CallExecuteRenderCommands();
void __cdecl RB_UpdateDynamicBuffers(GfxBackEndData *backendData);
void     RB_RenderThread(unsigned int threadContext);
void __cdecl RB_RenderCommandFrame(const GfxBackEndData *data);
void __cdecl RB_InitBackendGlobalStructs();
void __cdecl RB_SetBspImages();
void __cdecl RB_InitCodeImages();
void __cdecl RB_InitSingleCodeImage(
                unsigned int codeTexture,
                const GfxImage *image,
                unsigned __int8 samplerState,
                const char *name);
void __cdecl RB_RegisterBackendAssets();
void __cdecl RB_SaveScreen_BlendBlurred(const GfxBlendSaveScreenBlurredParam *p, const GfxViewInfo *viewInfo);
void __cdecl RB_SaveScreen_BlendFlashed(const GfxBlendSaveScreenFlashedParam *p, const GfxViewInfo *viewInfo);
void __cdecl RB_SaveScreen(const GfxSaveScreenParam *p, const GfxViewInfo *viewInfo);
void __cdecl R_ResolveSection(GfxCmdBufContext context, GfxImage *image);


extern GfxRenderTarget gfxRenderTargets[44];
extern GfxDrawConsts g_drawConsts;
extern r_backEndGlobals_t backEnd;
extern materialCommands_t tess;
extern GfxBackEndData *backEndData;

extern int rb_execCmdsMS;
extern int rb_swapMS;

//extern int time;
extern int g_msgTime; // originally called 'time' but this name clashes!
extern bool g_showCursor;

void RB_BO1GpuMark(int idx); // p1: bo1_frameperf GPU phase mark 0..BO1_GPU_SEGS-2 (measurement only)