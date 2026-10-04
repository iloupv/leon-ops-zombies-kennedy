#pragma once

struct CG_PerfData // sizeof=0xA0
{                                       // XREF: CG_PerfInfo/r
                                        // CG_PerfInfo/r ...
    int history[32];
    int count;                          // XREF: CG_PerfInit(void)+24/w
                                        // CG_PerfInit(void)+2E/w ...
    int index;
    int instant;
    int total;
    float average;                      // XREF: CG_DrawFPS(int,ScreenPlacement const *,float,meminfo_t *)+ED/r
                                        // CG_DrawFPS(int,ScreenPlacement const *,float,meminfo_t *)+278/r ...
    float variance;                     // XREF: CG_DrawFPS(int,ScreenPlacement const *,float,meminfo_t *)+203/r
    int min;                            // XREF: CG_DrawFPS(int,ScreenPlacement const *,float,meminfo_t *)+A4/r
                                        // CG_DrawFPS(int,ScreenPlacement const *,float,meminfo_t *)+28E/r ...
    int max;                            // XREF: CG_DrawFPS(int,ScreenPlacement const *,float,meminfo_t *)+6F/r
};

struct CG_PerfInfo // sizeof=0x3C4
{                                       // XREF: .data:CG_PerfInfo cg_perfInfo/r
    bool initialized;                   // XREF: CG_PerfInit(void)+3/r
                                        // CG_PerfInit(void)+60/w
    // padding byte
    // padding byte
    // padding byte
    CG_PerfData frame;                  // XREF: CG_DrawFPS(int,ScreenPlacement const *,float,meminfo_t *)+6F/r
                                        // CG_DrawFPS(int,ScreenPlacement const *,float,meminfo_t *)+A4/r ...
    CG_PerfData script;                 // XREF: CG_DrawFPS(int,ScreenPlacement const *,float,meminfo_t *)+386/r
                                        // CG_DrawFPS(int,ScreenPlacement const *,float,meminfo_t *)+396/r ...
    CG_PerfData cscript;                // XREF: CG_DrawFPS(int,ScreenPlacement const *,float,meminfo_t *)+5FD/r
                                        // CG_DrawFPS(int,ScreenPlacement const *,float,meminfo_t *)+60D/r ...
    CG_PerfData server;                 // XREF: CG_DrawFPS(int,ScreenPlacement const *,float,meminfo_t *)+2EA/r
                                        // CG_DrawFPS(int,ScreenPlacement const *,float,meminfo_t *)+2FA/r ...
    CG_PerfData renderExec;             // XREF: CG_PerfInit(void)+4C/w
                                        // CG_PerfUpdate(void)+B6/o
    CG_PerfData renderSwap;             // XREF: CG_PerfInit(void)+56/w
                                        // CG_PerfUpdate(void)+C9/o
};

void __cdecl CG_PerfInit();
void __cdecl CG_PerfUpdate();
void __cdecl UpdateData(CG_PerfData *data, int value);

extern CG_PerfInfo cg_perfInfo;

extern unsigned __int64 gRunFrameTicks;
