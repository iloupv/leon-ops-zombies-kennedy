#pragma once

enum DemoType : __int32;

struct windowDef_t;

void __cdecl SCR_DrawSmallStringExt(int x, int y, char *string, const float *setColor);
void __cdecl SCR_Init();
void __cdecl CL_DrawScreen(int localClientNum);
double __cdecl CL_GetMenuBlurRadius(int localClientNum);
void __cdecl CL_UpdateUIVisibilityBits(int localClientNum);
void    SCR_UpdateScreen();
void    SCR_UpdateFrame();
int    CL_CGameRendering(int localClientNum);
int __cdecl Window_GetDynamicFlags(int contextIndex, const windowDef_t *w);
DemoType __cdecl CL_GetDemoType();
void __cdecl SCR_DrawScreenField(int localClientNum, int refreshedUI);
void SCR_DrawDemoRecording();
void SCR_ClearScreen();
void SCR_DrawLoadingAndLogo();
char __cdecl SCR_ShouldSkipUpdateScreen();
void __cdecl SCR_UpdateLoadScreen();
void    CL_CubemapShot_f();
void CL_CubemapShotUsage();
void    CL_TakeHiResShot();
void    HiResScreenshot(int tiles);
void    CL_TakeHiResShot2();
void    CL_TakeHiResShot3();
void    CL_TakeHiResShot4();
void    CL_TakeHiResShot8();

extern bool updateScreenCalled;

extern int dword_98DADAC[4];