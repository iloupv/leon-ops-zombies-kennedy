#pragma once

#include <universal/dvar.h>
#include <bgame/bg_local.h>
#include <game/g_hudelem.h>

struct cg_hudelem_t // sizeof=0x23C
{                                       // XREF: DrawSingleHudElem2d/r
                                        // DrawOffscreenViewableWaypoint/r
    float x;                            // XREF: DrawSingleHudElem2d+1C7/r
                                        // DrawSingleHudElem2d+1D7/w
    float y;
    float rotation;
    float width;
    float height;
    char hudElemLabel[256];             // XREF: DrawSingleHudElem2d+192/r
                                        // DrawSingleHudElem2d+1A8/o
    float labelWidth;                   // XREF: DrawSingleHudElem2d+1CF/r
    char hudElemText[256];              // XREF: DrawSingleHudElem2d:$LN8_10/r
                                        // DrawSingleHudElem2d+232/o ...
    float textWidth;
    Font_s *font;
    float fontScale;
    float fontHeight;
    float color[4];                     // XREF: DrawSingleHudElem2d:loc_4BAB0E/o
    int timeNow;                        // XREF: DrawSingleHudElem2d+117/w
};

struct ScopedScrPlaceViewStack // sizeof=0x4
{                                       // XREF: ?Menu_Paint@@YA_NHPAUUiContext@@PAUScreenPlacementStack@@PAUmenuDef_t@@H@Z/r
    ScreenPlacementStack *mStack;

    ScopedScrPlaceViewStack(ScreenPlacementStack *stack, const ScreenPlacement *newPlacement);
    ~ScopedScrPlaceViewStack();
};

void __cdecl CG_HudElemRegisterDvars();
void __cdecl CG_TranslateHudElemMessage(
                int localClientNum,
                const char *message,
                const char *messageType,
                char *hudElemString);
char __cdecl ReplaceDirective(int localClientNum, unsigned int *searchPos, unsigned int *dstLen, char *dstString);
void __cdecl GetHudelemDirective(int localClientNum, char *directive, char *result);
void __cdecl DirectiveFakeIntroSeconds(int localClientNum, const char *arg0, char *result);
void __cdecl ParseDirective(char *directive, char *resultName, char *resultArg0);
char __cdecl WorldPosToScreenPos(int localClientNum, const float *worldPos, float *outScreenPos);
void __cdecl GetViewAxisProjections(
                const float *vieworg,
                const float (*viewaxis)[3],
                const float *worldPoint,
                float *projections);
char __cdecl WorldPosToExtraCamScreenPos(int localClientNum, const float *worldPos, float *outScreenPos);
bool __cdecl ClampScreenPosToEdges(
                int localClientNum,
                float *point,
                float padLeft,
                float padRight,
                float padTop,
                float padBottom,
                float *resultNormal,
                float *resultDist);
void __cdecl CG_Draw2dHudElems(int localClientNum, int foreground);
int __cdecl GetSortedHudElems(int localClientNum, hudelem_s **elems);
void __cdecl CopyInUseHudElems(hudelem_s **elems, int *elemCount, hudelem_s *elemSrcArray, int elemSrcArrayCount);
int __cdecl compare_hudelems(const void *pe0, const void *pe1);
int __cdecl GetSortedDebugHudElems(hudelem_s **elems);
void __cdecl CopyInUseDebugHudElems(
                hudelem_s **elems,
                int *elemCount,
                game_hudelem_s *elemSrcArray,
                int elemSrcArrayCount);
void __cdecl CG_Draw2dHudElemList(
                hudelem_s **elems,
                int elemCount,
                int localClientNum,
                int foreground,
                bool playerAlive,
                bool inKillCam);
void __cdecl DrawSingleHudElem2d(int localClientNum, const hudelem_s *elem);
void __cdecl GetHudElemInfo(int localClientNum, const hudelem_s *elem, cg_hudelem_t *cghe, char *hudElemString);
void __cdecl SafeTranslateHudElemString(int localClientNum, int index, char *hudElemString);
double __cdecl HudElemStringWidth(const char *string, const hudelem_s *elem, const cg_hudelem_t *cghe);
char *__cdecl HudElemTimerString(const hudelem_s *elem, int timeNow);
int __cdecl GetHudElemTime(const hudelem_s *elem, int timeNow);
char *__cdecl HudElemTenthsTimerString(const hudelem_s *elem, int timeNow);
double __cdecl HudElemWidth(const ScreenPlacement *scrPlace, const hudelem_s *elem, const cg_hudelem_t *cghe);
double __cdecl HudElemMaterialWidth(const ScreenPlacement *scrPlace, const hudelem_s *elem, const cg_hudelem_t *cghe);
double __cdecl HudElemMaterialSpecifiedWidth(
                const ScreenPlacement *scrPlace,
                char alignScreen,
                int sizeVirtual,
                const cg_hudelem_t *cghe);
double __cdecl HudElemHeight(const ScreenPlacement *scrPlace, const hudelem_s *elem, const cg_hudelem_t *cghe);
double __cdecl HudElemMaterialHeight(const ScreenPlacement *scrPlace, const hudelem_s *elem, const cg_hudelem_t *cghe);
double __cdecl HudElemMaterialSpecifiedHeight(
                const ScreenPlacement *scrPlace,
                char alignScreen,
                int sizeVirtual,
                const cg_hudelem_t *cghe);
void __cdecl SetHudElemPos(const ScreenPlacement *scrPlace, const hudelem_s *elem, cg_hudelem_t *cghe);
void __cdecl GetHudElemOrg(
                const ScreenPlacement *scrPlace,
                int alignOrg,
                int alignScreen,
                float xVirtual,
                float yVirtual,
                float width,
                float height,
                float *orgX,
                float *orgY);
double __cdecl AlignHudElemX(int alignOrg, float x, float width);
double __cdecl AlignHudElemY(int alignOrg, float y, float height);
double __cdecl HudElemMovementFrac(const hudelem_s *elem, int timeNow);
void __cdecl ConsolidateHudElemText(const hudelem_s *elem, cg_hudelem_t *cghe, char *hudElemString);
void __cdecl CopyStringToHudElemString(char *string, char *hudElemString);
void __cdecl HudElemColorToVec4(const hudelem_color_t *hudElemColor, float *resultColor);
void __cdecl DrawHudElemString(
                int localClientNum,
                const ScreenPlacement *scrPlace,
                char *text,
                const hudelem_s *elem,
                cg_hudelem_t *cghe);
double __cdecl OffsetHudElemY(const hudelem_s *elem, const cg_hudelem_t *cghe, float offsetY);
void __cdecl DrawHudElemClock(int localClientNum, const hudelem_s *elem, const cg_hudelem_t *cghe);
void __cdecl DrawHudElemMaterial(int localClientNum, const hudelem_s *elem, cg_hudelem_t *cghe);
void __cdecl UpdateHudElemWarGameData(int localClientNum, const hudelem_s *elem);
void    DrawOffscreenViewableWaypoint(int localClientNum, const hudelem_s *elem);
double __cdecl GetScaleForDistance(int localClientNum, const float *worldPos);
void __cdecl WaypointTargetFade(int localClientNum, const hudelem_s *elem, const float *screenPos, float *color);
double __cdecl WaypointFadeGetTimeFrac(const cg_s *cgameGlob, int currentTime);
void __cdecl CG_AddDrawSurfsFor3dHudElems(int localClientNum);
void    AddDrawSurfForHudElemWaypoint(int localClientNum, const hudelem_s *elem);
double __cdecl HudElemWaypointHeight(int localClientNum, const hudelem_s *elem);
void __cdecl RB_DrawWaypoints(int localClientNum);
void __cdecl RB_AddWaypoint(int localClientNum, const hudelem_s *elem);

extern const dvar_t *waypointDebugDraw;
extern const dvar_t *waypointIconWidth;
extern const dvar_t *waypointIconHeight;
extern const dvar_t *waypointOffscreenPointerWidth;
extern const dvar_t *waypointOffscreenPointerHeight;
extern const dvar_t *waypointOffscreenPointerDistance;
extern const dvar_t *waypointOffscreenDistanceThresholdAlpha;
extern const dvar_t *waypointOffscreenPadLeft;
extern const dvar_t *waypointOffscreenPadRight;
extern const dvar_t *waypointOffscreenPadTop;
extern const dvar_t *waypointOffscreenPadBottom;
extern const dvar_t *waypointOffscreenRoundedCorners;
extern const dvar_t *waypointOffscreenCornerRadius;
extern const dvar_t *waypointOffscreenScaleLength;
extern const dvar_t *waypointOffscreenScaleSmallest;
extern const dvar_t *waypointDistScaleRangeMin;
extern const dvar_t *waypointDistScaleRangeMax;
extern const dvar_t *waypointDistScaleSmallest;
extern const dvar_t *waypointDistFade;
extern const dvar_t *waypointTimeFade;
extern const dvar_t *waypointSplitscreenScale;
extern const dvar_t *waypointTweakY;
extern const dvar_t *hudElemPausedBrightness;
extern const dvar_t *waypointPlayerOffsetProne;
extern const dvar_t *waypointPlayerOffsetCrouch;
extern const dvar_t *waypointPlayerOffsetStand;
extern const dvar_t *waypointPlayerOffsetRevive;

extern game_hudelem_s g_hudelems[1024];