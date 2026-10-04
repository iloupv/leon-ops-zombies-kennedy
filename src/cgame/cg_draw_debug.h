#pragma once

struct ScreenPlacement;
struct snd_overlay_info;
struct meminfo_t;

double __cdecl CG_DrawAnimTagInfo(const ScreenPlacement *scrPlace, float posY);
double __cdecl CG_CornerDebugPrint(
                const ScreenPlacement *sP,
                float posX,
                float posY,
                float labelWidth,
                char *text,
                char *label,
                const float *color);
void __cdecl CG_DrawUpperRightDebugInfo(int localClientNum);
double __cdecl CG_DrawSnapshot(int localClientNum, float posY);
double __cdecl CG_CornerDebugPrintCaption(
                const ScreenPlacement *sP,
                float posX,
                float posY,
                float labelWidth,
                char *text,
                const float *color);
double __cdecl CG_DrawStatmon(const ScreenPlacement *scrPlace, float y);
double __cdecl CG_DrawViewpos(const ScreenPlacement *scrPlace, float y, int localClientNum);
double __cdecl DrawEntityCounts(const ScreenPlacement *scrPlace, float posY);
double __cdecl DrawSnapshotInfo(const ScreenPlacement *scrPlace, float posY);
int __cdecl CG_CompareSndInfoChannel(unsigned int *va, unsigned int *vb);
int __cdecl CG_CompareSndInfoPriority(float *va, float *vb);
int __cdecl CG_CompareSndInfoAlias(snd_overlay_info *va, snd_overlay_info *vb);
int __cdecl CG_CompareSndIsActive(snd_overlay_info *a, snd_overlay_info *b);
int __cdecl CG_CompareSndInfoDryLevel(snd_overlay_info *va, snd_overlay_info *vb);
int __cdecl CG_CompareSndInfoDistance(snd_overlay_info *va, snd_overlay_info *vb);
int __cdecl CG_CompareSndInfoEntity(snd_overlay_info *va, snd_overlay_info *vb);
void __cdecl CG_DrawPerformanceWarnings();
void __cdecl CG_DrawDebugOverlays(int localClientNum);
void __cdecl CG_DrawModelBoneAxis(int localClientNum);
void __cdecl CG_DrawDebugPlayerHealth(int localClientNum);
void __cdecl CG_DrawDebugPlayerSprint(int localClientNum);
void __cdecl CG_DrawFullScreenDebugOverlays();
void __cdecl CG_DrawScriptUsage(const ScreenPlacement *scrPlace);
void __cdecl CG_DrawSoundOverlay(const ScreenPlacement *scrPlace);
void __cdecl CG_DrawFxProfile(int localClientNum);
void __cdecl CG_DrawFxText(char *text, float *profilePos);
void __cdecl CG_DrawFxMarkProfile(unsigned int localClientNum);
void __cdecl CG_DrawFxPriorityDebug(int localClientNum);
void __cdecl CG_DrawFxPriorityText(char *text, float *profilePos);
double __cdecl CG_DrawFPS(int localClientNum, const ScreenPlacement *scrPlace, float y, meminfo_t *meminfo);
bool __cdecl CG_Flash(int timeMs);
void __cdecl CG_DrawVersion();
void __cdecl CG_DrawShaderWarming();
void CG_QuickPrint(const char *fmt, ...);
void __cdecl CG_QuickPrintFlush();


extern float g_TagColors[6][4];