#pragma once

enum DevGuiInputState : __int32
{                                       // XREF: DevGuiInput/r
                                        // DevGuiInput/r
    SCROLL_NONE    = 0x0,
    SCROLL_PRESSED = 0x1,
    SCROLL_STALLED = 0x2,
    SCROLL_HELD    = 0x3,
};

enum DevGuiInputAxis : __int32
{                                       // XREF: ?DevGui_GetMenuScroll@@YAFW4DevGuiInputAxis@@@Z/r
                                        // ?DevGui_UpdateIntScroll@@YA_JM_J00W4DevGuiInputAxis@@@Z/r ...
    SCROLL_XAXIS      = 0x0,
    SCROLL_YAXIS      = 0x1,
    SCROLL_AXIS_COUNT = 0x2,
};

enum DevGuiInputButton : __int32
{                                       // XREF: ?DevGui_IsButtonDown@@YA_NW4DevGuiInputButton@@@Z/r
                                        // ?DevGui_IsButtonPressed@@YA_NW4DevGuiInputButton@@@Z/r ...
    INPUT_UP           = 0x0,
    INPUT_DOWN         = 0x1,
    INPUT_LEFT         = 0x2,
    INPUT_RIGHT        = 0x3,
    INPUT_ACCEPT       = 0x4,
    INPUT_REJECT       = 0x5,
    INPUT_BIND         = 0x6,
    INPUT_GRAPH_EDIT   = 0x7,
    INPUT_GRAPH_ADD    = 0x8,
    INPUT_GRAPH_REMOVE = 0x9,
    INPUT_GRAPH_SAVE   = 0xA,
    INPUT_LINK         = 0xB,
    INPUT_RESET        = 0xC,
    INPUT_COUNT        = 0xD,
};

struct DevGuiInput // sizeof=0x7C
{                                       // XREF: .data:s_input/r
    int selectedGamePadIndex;           // XREF: DevGui_SelectGamepad(int)+6/w
                                        // DevGui_InputUpdateGamepad+3/r
    int gamePadIndex;                   // XREF: DevGui_InputUpdate(int,float)+5F/r
                                        // DevGui_InputUpdate(int,float)+73/r ...
    float mousePos[2];                  // XREF: DevGui_UpdateScrollInputs+22A/r
                                        // DevGui_UpdateScrollInputs+237/r ...
    bool buttonDown[13];                // XREF: DevGui_InputUpdate(int,float)+53/r
                                        // DevGui_InputUpdate(int,float)+A2/w ...
    bool prevButtonDown[13];            // XREF: DevGui_InputUpdate(int,float)+59/w
                                        // DevGui_IsButtonPressed(DevGuiInputButton)+15/r ...
    // padding byte
    // padding byte
    float scrollScale;                  // XREF: DevGui_UpdateScrollInputs+325/w
                                        // DevGui_UpdateMenuScroll+11B/r ...
    DevGuiInputState digitalStates[2];  // XREF: DevGui_InputUpdate(int,float)+E9/o
                                        // DevGui_UpdateMenuScroll+A9/r ...
    float digitalAxis[2];               // XREF: DevGui_InputUpdate(int,float)+E4/o
                                        // DevGui_UpdateScrollInputs+337/w ...
    float digitalTimes[2];              // XREF: DevGui_InputUpdate(int,float)+DF/o
    DevGuiInputState analogStates[2];   // XREF: DevGui_InputUpdate(int,float)+110/o
                                        // DevGui_UpdateMenuScroll+B6/r ...
    float analogAxis[2];                // XREF: DevGui_InputUpdate(int,float)+10B/o
                                        // DevGui_UpdateScrollInputs+356/w ...
    float analogTimes[2];               // XREF: DevGui_InputUpdate(int,float)+106/o
    __int16 menuScroll[2];              // XREF: DevGui_UpdateMenuScroll+9E/w
                                        // DevGui_UpdateMenuScroll+1A9/r ...
    float menuScrollTime[2];            // XREF: DevGui_UpdateMenuScroll+152/r
                                        // DevGui_UpdateMenuScroll+15E/w ...
    float digitalSliderTime;            // XREF: DevGui_UpdateIntScroll(float,__int64,__int64,__int64,DevGuiInputAxis):loc_59D157/r
                                        // DevGui_UpdateIntScroll(float,__int64,__int64,__int64,DevGuiInputAxis)+E4/w ...
    float analogSliderTime;             // XREF: DevGui_UpdateIntScroll(float,__int64,__int64,__int64,DevGuiInputAxis):loc_59D2BD/r
                                        // DevGui_UpdateIntScroll(float,__int64,__int64,__int64,DevGuiInputAxis)+24A/w ...
    float sliderScrollTime;             // XREF: DevGui_InputUpdateGamepad+42/w
                                        // DevGui_InputUpdateMouse+B/w ...
    float sliderScrollMaxTimeStep;      // XREF: DevGui_InputUpdateGamepad+52/w
                                        // DevGui_InputUpdateMouse+1B/w ...
};

void __cdecl DevGui_InputInit();
void __cdecl DevGui_InputShutdown();
void __cdecl DevGui_SelectGamepad(int gamePadIndex);
char __cdecl DevGui_InputUpdate(int localClientNum, float deltaTime);
void __cdecl DevGui_UpdateScrollInputs(int localClientNum);
void __cdecl DevGui_UpdateScrollStates(float deltaTime, DevGuiInputState *states, float *axis, float *times);
void __cdecl DevGui_UpdateMenuScroll(float deltaTime);
char __cdecl DevGui_InputUpdateGamepad();
void DevGui_InputUpdateMouse();
void __cdecl DevGui_MouseEvent(int dx, int dy);
__int16 __cdecl DevGui_GetMenuScroll(DevGuiInputAxis axis);
__int64 __cdecl DevGui_UpdateIntScroll(float deltaTime, __int64 value, __int64 min, __int64 max, DevGuiInputAxis axis);
double __cdecl DevGui_UpdateFloatScroll(
                float deltaTime,
                float value,
                float min,
                float max,
                float step,
                DevGuiInputAxis axis);
bool __cdecl DevGui_IsButtonDown(DevGuiInputButton button);
bool __cdecl DevGui_IsButtonPressed(DevGuiInputButton button);
bool __cdecl DevGui_IsButtonReleased(DevGuiInputButton button);
