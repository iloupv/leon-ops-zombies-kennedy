#pragma once

#include <qcommon/bitarray.h>

void __cdecl CG_HandleSpecialStateInput(int localClientNum, bitarray<51> *button_bits);
void __cdecl CG_HandleSpectatorInput(int localClientNum, bitarray<51> *button_bits);
bool __cdecl CG_ShouldUpdateViewAngles(int localClientNum);
char __cdecl CG_TranslateGamepadButton(int localClientNum, int button, const char *cmd);
char __cdecl CG_GamepadSpectatorInput(int localClientNum, int button, const char *cmd);
char __cdecl CG_GamepadCheckStartAndBack(int localClientNum, int button, const char *cmd);
char __cdecl CG_GamepadKillCamInput(int localClientNum, int button, const char *cmd);
