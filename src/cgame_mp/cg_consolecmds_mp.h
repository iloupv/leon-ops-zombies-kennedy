#pragma once

void __cdecl CG_ScoresUp(int localClientNum);
void __cdecl CG_InitConsoleCommands();
void __cdecl CG_Viewpos_f();
void __cdecl CG_ScoresUp_f();
void __cdecl CG_ScoresDown_f();
void __cdecl CG_ScoresDown(int localClientNum);
void __cdecl CG_ShellShock_f();
void __cdecl CG_ShellShock_Load_f();
void __cdecl CG_ShellShock_Save_f();
void __cdecl CG_MovieCamera_f();
char __cdecl CG_CheatsOK(const char *cmdName);
void __cdecl CG_FollowCamera_f();
void __cdecl CG_QuickMessage_f();
void __cdecl CG_VoiceChat_f();
void __cdecl CG_TeamVoiceChat_f();
void __cdecl CG_UpdateVehicleBindings_f();
bool __cdecl CL_LocalClient_IsCUIFlagSet(int localClientNum, int flag);
void __cdecl CG_RestartSmokeGrenades_f();
void __cdecl UpdateFilmTweaks_f();
void __cdecl UpdateCharPrimaryTweaks_f();
void __cdecl UpdateBloomTweaks_f();
void __cdecl CG_ShutdownConsoleCommands();
