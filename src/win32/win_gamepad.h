#pragma once

#include <universal/dvar.h>
#include <client/cl_gamepad.h>

#include <XInput.h>

struct GamePad // sizeof=0x7C
{                                       // XREF: .data:s_gamePads/r
    bool enabled;
    bool keyboardEnabled;
    __int16 digitals;
    __int16 lastDigitals;
    // padding byte
    // padding byte
    float analogs[2];
    float lastAnalogs[2];
    float sticks[4];
    float lastSticks[4];
    bool stickDown[4][2];
    bool stickDownLast[4][2];
    float lowRumble;
    float highRumble;
    //GamePad::<unnamed_type_feedback> feedback;
    struct //GamePad::<unnamed_type_feedback> // sizeof=0x4
    {                                       // XREF: GamePad/r
        _XINPUT_VIBRATION Rumble;
    } feedback;
    _XINPUT_CAPABILITIES caps;
    _XINPUT_CAPABILITIES keyboardCaps;
};

void GPad_InitAll();
void __cdecl GPad_UpdateAll();
int GPad_RefreshAll();
void __cdecl GPad_Check(int portIndex);
void __cdecl GPad_UpdateDigitals(int portIndex, const _XINPUT_GAMEPAD *xpad);
void __cdecl GPad_UpdateAnalogs(int portIndex, const _XINPUT_GAMEPAD *xpad);
void __cdecl GPad_UpdateSticks(int portIndex, const _XINPUT_GAMEPAD *xpad);
void __cdecl GPad_ConvertStickToFloat(__int16 inX, __int16 inY, float *outX, float *outY);
void __cdecl GPad_UpdateSticksDown(GamePad *gPad);
void __cdecl GPad_ResetState(int portIndex);
bool __cdecl GPad_IsActive(int portIndex);
double __cdecl GPad_GetButton(int portIndex, GamePadButton button);
bool __cdecl GPad_ButtonRequiresUpdates(int portIndex, GamePadButton button);
bool __cdecl GPad_IsButtonPressed(int portIndex, GamePadButton button);
bool __cdecl GPad_IsButtonReleased(int portIndex, GamePadButton button);
double __cdecl GPad_GetStick(int portIndex, GamePadStick stick);
bool __cdecl GPad_IsStickPressed(int portIndex, GamePadStick stick, GamePadStickDir stickDir);
bool __cdecl GPad_GetStickChangedToPressedState(
                int portIndex,
                GamePadStick stick,
                GamePadStickDir stickDir,
                bool pressedState);
bool __cdecl GPad_IsStickReleased(int portIndex, GamePadStick stick, GamePadStickDir stickDir);


extern const dvar_t *gpad_debug;
extern const dvar_t *gpad_button_lstick_deflect_max;
extern const dvar_t *gpad_button_rstick_deflect_max;
extern const dvar_t *gpad_button_deadzone;
extern const dvar_t *gpad_stick_deadzone_min;
extern const dvar_t *gpad_stick_deadzone_max;
extern const dvar_t *gpad_stick_pressed;
extern const dvar_t *gpad_stick_pressed_hysteresis;
extern const dvar_t *gpad_rumble;
extern const dvar_t *gpad_menu_scroll_delay_first;
extern const dvar_t *gpad_menu_scroll_delay_rest;
extern const dvar_t *gpad_buttonsConfig;
extern const dvar_t *gpad_sticksConfig;
extern const dvar_t *gpad_enabled;
extern const dvar_t *gpad_present;