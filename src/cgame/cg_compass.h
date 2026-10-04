#pragma once
#include <cgame_mp/cg_local_mp.h>
#include <universal/dvar.h>
#include <qcommon/cmd.h>

enum CompassType : __int32
{                                       // XREF: ?CG_CompassDrawFriendlies@@YAXHW4CompassType@@PBUrectDef_s@@1QAM@Z/r
                                        // ?CG_CompassDrawArtilleryIcon@@YAXHW4CompassType@@PBUrectDef_s@@1QAM@Z/r ...
    COMPASS_TYPE_PARTIAL = 0x0,
    COMPASS_TYPE_FULL    = 0x1,
};

void __cdecl CG_CompassRegisterDvars();
char __cdecl CG_IsSelectingLocation(int localClientNum);
bool __cdecl CG_WorldPosToCompass(
                CompassType compassType,
                const cg_s *cgameGlob,
                const rectDef_s *mapRect,
                const float *north,
                const float *playerWorldPos,
                const float *in,
                float *out,
                float *outClipped);
char __cdecl CG_TextClippedToCompass(
                CompassType compassType,
                const cg_s *cgameGlob,
                const rectDef_s *mapRect,
                const float *north,
                const float *playerWorldPos,
                const float *in,
                float w,
                float h);
void __cdecl CG_CompassCalcDimensions(
                CompassType compassType,
                const cg_s *cgameGlob,
                const rectDef_s *parentRect,
                const rectDef_s *rect,
                float *x,
                float *y,
                float *w,
                float *h);
double __cdecl CG_FadeCompass(int localClientNum, int displayStartTime, CompassType compassType);
double __cdecl CG_FadeCompassIcons(int localClientNum, bool ignoreGlobalScrambler);
void __cdecl CG_CompassDrawPlayerBack(
                int localClientNum,
                CompassType compassType,
                const rectDef_s *parentRect,
                const rectDef_s *rect,
                Material *material,
                float *color);
void __cdecl CG_DrawPlayerPopUps(
                int localClientNum,
                const rectDef_s *rect,
                Font_s *font,
                float scale,
                float *color,
                Material *material,
                int textStyle,
                itemDef_s *item,
                char *text,
                bool doSwing);
void __cdecl CG_DrawPlayerCOD7TypeWriter(
                int localClientNum,
                const rectDef_s *rect,
                Font_s *font,
                float scale,
                float *color,
                Material *material,
                int textStyle,
                itemDef_s *item,
                char *text);
void __cdecl CG_DrawPlayerRedationText(
                int localClientNum,
                const rectDef_s *rect,
                Font_s *font,
                float scale,
                float *color,
                Material *material,
                int textStyle,
                itemDef_s *item,
                char *text);
void __cdecl CG_DrawPlayerPopInText(
                int localClientNum,
                const rectDef_s *rect,
                Font_s *font,
                float scale,
                float *color,
                Material *material,
                int textStyle,
                itemDef_s *item,
                char *text);
void __cdecl CG_CompassDrawPlayerNorthCoord(
                int localClientNum,
                CompassType compassType,
                const rectDef_s *parentRect,
                const rectDef_s *rect,
                Font_s *font,
                Material *material,
                float *color,
                int textStyle);
void __cdecl CG_CompassDrawPlayerEastCoord(
                int localClientNum,
                CompassType compassType,
                const rectDef_s *parentRect,
                const rectDef_s *rect,
                Font_s *font,
                Material *material,
                float *color,
                int textStyle);
void __cdecl CG_CompassDrawPlayerNCoordScroll(
                int localClientNum,
                CompassType compassType,
                const rectDef_s *parentRect,
                const rectDef_s *rect,
                Font_s *font,
                Material *material,
                float *color,
                int textStyle);
void __cdecl CG_CompassDrawPlayerECoordScroll(
                int localClientNum,
                CompassType compassType,
                const rectDef_s *parentRect,
                const rectDef_s *rect,
                Font_s *font,
                Material *material,
                float *color,
                int textStyle);
void __cdecl CG_CompassDrawStatic(
                int localClientNum,
                CompassType compassType,
                const rectDef_s *parentRect,
                const rectDef_s *rect,
                Material *material,
                float *color);
void __cdecl CG_UpdateEnemyScramblerAlpha(int localClientNum);
double __cdecl CG_GetScramblerEnemyAlpha(int localClientNum);
void __cdecl CG_CompassDrawScrambler(
                int localClientNum,
                CompassType compassType,
                const rectDef_s *parentRect,
                const rectDef_s *rect,
                Material *material,
                float *color);
void __cdecl CG_CompassDrawRound(int localClientNum, const rectDef_s *rect, Material *material, float *color);
void __cdecl CG_CompassDrawPlayerMap(
                int localClientNum,
                CompassType compassType,
                const rectDef_s *parentRect,
                const rectDef_s *rect,
                Material *material,
                float *color,
                bool grid);
void __cdecl CG_CompassDrawPlayerMapLocationSelector(
                int localClientNum,
                CompassType compassType,
                const rectDef_s *parentRect,
                const rectDef_s *rect,
                Material *material,
                float *color);
void __cdecl CG_CompassDrawPlayerSelectedLocations(
                int localClientNum,
                CompassType compassType,
                const rectDef_s *parentRect,
                const rectDef_s *rect,
                Material *material,
                float *color);
void __cdecl CG_CompassDrawPlayer(
                int localClientNum,
                CompassType compassType,
                const rectDef_s *parentRect,
                rectDef_s *rect,
                Material *material,
                float *color);
void __cdecl CG_CompassDrawBorder(
                int localClientNum,
                CompassType compassType,
                const rectDef_s *parentRect,
                rectDef_s *rect,
                Material *material,
                float *color);
void __cdecl CG_CompassUpYawVector(const cg_s *cgameGlob, float *result);
void    CG_CompassDrawTickertape(
                int localClientNum,
                CompassType compassType,
                const rectDef_s *parentRect,
                const rectDef_s *rect,
                Material *material,
                const float *color,
                Font_s *textFont,
                float textScale,
                int textStyle,
                bool drawObjectives);
void __cdecl CalcCompassPointerSize(CompassType compassType, float *w, float *h);
void __cdecl DrawIconDistanceText(
                int localClientNum,
                float distance,
                float iconX,
                float iconY,
                float iconH,
                const rectDef_s *rect,
                const float *color,
                Font_s *textFont,
                float textScale,
                int textStyle);
double __cdecl CutFloat(float original);
void __cdecl CG_SetGridTable();
void __cdecl CG_ScaleCompass();
void __cdecl CG_SetGridOffsets(
                const float *east,
                const float *south,
                float *eastOffsetLetter,
                float *southOffsetLetter);
void __cdecl CG_CompassDrawGridPoints(
                int localClientNum,
                CompassType compassType,
                const rectDef_s *parentRect,
                const rectDef_s *rect,
                const float *color,
                Font_s *textFont,
                float textScale,
                int textStyle);
void __cdecl CG_GetGridFromPos(int localClientNum, float *pos, char *gridName);
float __cdecl CG_GetHudAlphaCompass(int localClientNum);
void __cdecl CalcCompassFriendlySize(CompassType compassType, float *w, float *h);
void __cdecl CG_CompassDrawPlayerPointers_MiniMap(
                int localClientNum,
                CompassType compassType,
                const rectDef_s *parentRect,
                const rectDef_s *rect,
                Material *material,
                const float *color);
double __cdecl GetObjectiveFade(const rectDef_s *clipRect, float x, float y, float width, float height);
char __cdecl CalcCompassPointerSizeObjective(
                CompassType compassType,
                float *w,
                float *h,
                const objective_t *objective,
                const rectDef_s *mapRect);
bool __cdecl CG_IsShowingZombieMap();

extern const dvar_t *compass;
extern const dvar_t *compassSize;
extern const dvar_t *compassMaxRange;
extern const dvar_t *compassMinRange;
extern const dvar_t *compassMinRadius;
extern const dvar_t *compassSoundPingFadeTime;
extern const dvar_t *compassClampIcons;
extern const dvar_t *compassFriendlyWidth;
extern const dvar_t *compassFriendlyHeight;
extern const dvar_t *compassPlayerWidth;
extern const dvar_t *compassPlayerHeight;
extern const dvar_t *compassCoords;
extern const dvar_t *compassECoordCutoff;
extern const dvar_t *compassRotation;
extern const dvar_t *compassTickertapeStretch;
extern const dvar_t *compassRadarPingFadeTime;
extern const dvar_t *compassRadarUpdateTime;
extern const dvar_t *compassRadarUpdateFastTime;
extern const dvar_t *compassSatelliteStaticImageFadeTime;
extern const dvar_t *compassSatellitePingFadeTime;
extern const dvar_t *compassStaticImageUpdateTime;
extern const dvar_t *compassSatelliteScanTime;
extern const dvar_t *compassRadarLineThickness;
extern const dvar_t *compassLocalRadarUpdateTime;
extern const dvar_t *compassLocalRadarRadius;
extern const dvar_t *compassObjectiveWidth;
extern const dvar_t *compassObjectiveHeight;
extern const dvar_t *compassObjectiveArrowWidth;
extern const dvar_t *compassObjectiveArrowHeight;
extern const dvar_t *compassObjectiveArrowOffset;
extern const dvar_t *compassObjectiveArrowRotateDist;
extern const dvar_t *compassObjectiveMaxRange;
extern const dvar_t *compassObjectiveMinAlpha;
extern const dvar_t *compassObjectiveNumRings;
extern const dvar_t *compassObjectiveRingTime;
extern const dvar_t *compassObjectiveRingSize;
extern const dvar_t *compassObjectiveTextScale;
extern const dvar_t *compassObjectiveTextHeight;
extern const dvar_t *compassObjectiveDrawLines;
extern const dvar_t *compassObjectiveIconWidth;
extern const dvar_t *compassObjectiveIconHeight;
extern const dvar_t *compassObjectiveIconWidthZombie;
extern const dvar_t *compassObjectiveIconHeightZombie;
extern const dvar_t *compassObjectiveNearbyDist;
extern const dvar_t *compassObjectiveMinDistRange;
extern const dvar_t *compassObjectiveDetailDist;
extern const dvar_t *compassObjectiveMinHeight;
extern const dvar_t *compassObjectiveMaxHeight;
extern const dvar_t *compassPartialType;
extern const dvar_t *compassSpectatorsSeeEnemies;
extern const dvar_t *compassEnemyFootstepMaxRange;
extern const dvar_t *compassEnemyFootstepMaxZ;
extern const dvar_t *compassEnemyFootstepMinSpeed;
extern const dvar_t *compassEnemyFootstepEnabled;
extern const dvar_t *compassShowEnemies;
extern const dvar_t *compassForcePlayerIcon;
extern const dvar_t *compassGridEnabled;
extern const dvar_t *compassGridRows;
extern const dvar_t *compassGridCols;
extern const dvar_t *compassGridAlign;
extern const dvar_t *compassScaleDuration;
extern const dvar_t *compassScaleDiff;
extern const dvar_t *compassScaleTimer;
extern const dvar_t *compassScaleReset;
extern const dvar_t *typeWriterCod7LetterFXTime;
extern const dvar_t *popInLetterFXTime;
extern const dvar_t *redactionDisplayTime;
extern const dvar_t *redactionFadeDuration;
extern const dvar_t *redactionStartStrokeTime;
extern const dvar_t *redactionStrokeTime;
extern const dvar_t *compassDebug;
extern const dvar_t *forceFullScreenMap;
extern const dvar_t *cg_showZombieMap;
extern const dvar_t *cg_hudMapRadarLineThickness;
extern const dvar_t *cg_hudMapFriendlyWidth;
extern const dvar_t *cg_hudMapFriendlyHeight;
extern const dvar_t *cg_hudMapPlayerWidth;
extern const dvar_t *cg_hudMapPlayerHeight;
extern const dvar_t *cg_hudMapBorderWidth;
extern const dvar_t *cg_showZombieControls;