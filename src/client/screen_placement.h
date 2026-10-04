#pragma once

struct ScreenPlacement // sizeof=0x78
{                                                                             // XREF: .data:ScreenPlacement * scrPlaceView/r
                                                                                // .data:ScreenPlacement scrPlaceFull/r ...
        float scaleVirtualToReal[2];                // XREF: AimAssist_UpdateTweakables+98/r
                                                                                // AimAssist_UpdateTweakables+E8/r ...
        float scaleVirtualToFull[2];                // XREF: UI_Refresh(int)+32B/r
                                                                                // UI_Refresh(int)+348/r ...
        float scaleRealToVirtual[2];
        float virtualViewableMin[2];
        float virtualViewableMax[2];
        float virtualTweakableMin[2];
        float virtualTweakableMax[2];
        float realViewportBase[2];
        float realViewportSize[2];                    // XREF: CG_DrawGenericOverlay+13F/r
                                                                                // CG_DrawGenericOverlay+152/r ...
        float realViewportMid[2];
        float realViewableMin[2];
        float realViewableMax[2];
        float realTweakableMin[2];
        float realTweakableMax[2];
        float subScreen[2];
};

struct ScreenPlacementStack // sizeof=0xF8
{                                       // XREF: ?Menu_PaintAll@@YAXHPAUUiContext@@@Z/r
                                        // DrawSingleHudElem2d/r
    ScreenPlacement stack[2];
    int size;                           // XREF: DrawSingleHudElem2d+9B/w
                                        // Menu_PaintAll(int,UiContext *)+1E/w
    int contextIndex;                   // XREF: DrawSingleHudElem2d+A5/w
                                        // Menu_PaintAll(int,UiContext *)+2E/w
};

double __cdecl ScrPlace_HiResGetScale();
void __cdecl ScrPlace_HiResApplyRect(float *x, float *y, float *w, float *h);
void __cdecl ScrPlace_SetupUI3DForFullscreen(ScreenPlacement *ui3DScrPlace, ScreenPlacement *fullScrPlace);
void __cdecl ScrPlace_SetupFloatViewport(
                ScreenPlacement *scrPlace,
                float viewportX,
                float viewportY,
                float viewportWidth,
                float viewportHeight);
void __cdecl ScrPlace_CalcSafeAreaOffsets(
                float viewportX,
                float viewportY,
                float viewportWidth,
                float viewportHeight,
                float horzAspectScale,
                float vertAspectScale,
                float safeAreaRatioHorz,
                float safeAreaRatioVert,
                float *realViewableMin,
                float *realViewableMax,
                float *virtualViewableMin,
                float *virtualViewableMax);
void __cdecl ScrPlace_SetupViewport(
                ScreenPlacement *scrPlace,
                int viewportX,
                int viewportY,
                int viewportWidth,
                int viewportHeight);
void __cdecl ScrPlace_SetupUnsafeViewport(
                ScreenPlacement *scrPlace,
                int viewportX,
                int viewportY,
                int viewportWidth,
                int viewportHeight);
double __cdecl ScrPlace_ApplyXinverse(const ScreenPlacement *scrPlace, float x, int horzAlign);
double __cdecl ScrPlace_ApplyYinverse(const ScreenPlacement *scrPlace, float y, int vertAlign);
double __cdecl ScrPlace_ApplyX(const ScreenPlacement *scrPlace, float x, int horzAlign);
double __cdecl ScrPlace_ApplyY(const ScreenPlacement *scrPlace, float y, int vertAlign);
double __cdecl ScrPlace_ApplyW(const ScreenPlacement *scrPlace, float w, int horzAlign);
double __cdecl ScrPlace_ApplyH(const ScreenPlacement *scrPlace, float h, int vertAlign);
void __cdecl ScrPlace_ApplyRect(
                const ScreenPlacement *scrPlace,
                float *x,
                float *y,
                float *w,
                float *h,
                int horzAlign,
                int vertAlign);

extern ScreenPlacement scrPlaceView[1];
extern ScreenPlacement scrPlaceFull;
extern ScreenPlacement scrPlaceFullUnsafe;