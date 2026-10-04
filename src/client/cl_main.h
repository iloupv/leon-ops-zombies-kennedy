#pragma once
#include <gfx_d3d/r_material.h>
#include "screen_placement.h"

enum connstate_t : __int32
{                                       // XREF: clientUIActive_t/r
                                        // uiClientState_s/r ...
    CA_DISCONNECTED = 0x0,
    CA_CINEMATIC    = 0x1,
    CA_UICINEMATIC  = 0x2,
    CA_LOGO         = 0x3,
    CA_CONNECTING   = 0x4,
    CA_CHALLENGING  = 0x5,
    CA_CONNECTED    = 0x6,
    CA_SENDINGSTATS = 0x7,
    CA_LOADING      = 0x8,
    CA_PRIMED       = 0x9,
    CA_ACTIVE       = 0xA,
};

struct clientUIActive_t // sizeof=0x10
{                                       // XREF: SwapClients/r
    int flags;                          // XREF: SwapClients+2B/w
                                        // SwapClients+79/r
    int keyCatchers;                    // XREF: SwapClients+31/w
                                        // SwapClients+7E/r
    connstate_t connectionState;        // XREF: SwapClients+37/w
                                        // SwapClients+84/r
    int nextScrollTime;                 // XREF: SwapClients+3D/w
                                        // SwapClients+8A/r
};

void __cdecl CL_RunNetworkFrame(int localClientNum);
char __cdecl CL_AnyLocalClientsRunning();
void __cdecl CL_DontUnlockControllers(bool flag);
void __cdecl CL_Command_ProvisionallyDisableAllClients();
void __cdecl CL_Command_DisableAllButPrimaryClients();
void __cdecl CL_Command_DisableAllClients();
void __cdecl CL_Command_SetClientBeingUsedAndPrimary();
void __cdecl CL_Command_SetClientBeingUsedAndPrimaryAndActive();
void __cdecl CL_Command_SetClientPrimary();
void __cdecl CL_Command_SetClientBeingUsed();
void __cdecl CL_Command_SetClientBeingUsedAndActive();
void __cdecl CL_Command_SetClientNotBeingUsed();
void __cdecl CL_SetupClientsForIngame();
void __cdecl CL_SetupClientsForFrontend();
void __cdecl CL_DrawFramedPicPhysical(
                float x,
                float y,
                float w,
                float h,
                float thicknessW,
                float thicknessH,
                float thicknessTex,
                int sides,
                const float *color,
                Material *material);
void __cdecl CL_RegisterCommands();
void __cdecl CL_UpdateDvarsFromProfile();
void __cdecl CL_UpdateGamerProfile();
void __cdecl CL_CommitDvarToProfiles();
void __cdecl CL_UpdateProfileFromStats();
void __cdecl CL_DrawSpinnerPhysical(float x, float y, float w, float h, const float *color);
void __cdecl CL_DrawSpinner(
                const ScreenPlacement *scrPlace,
                float x,
                float y,
                float w,
                float h,
                int horzAlign,
                int vertAlign,
                const float *color);
void __cdecl CL_DrawSpinnerLoadbarPhysical(float x, float y, float w, float h, const float *color, float percentDone);
void __cdecl CL_DrawSpinnerLoadbar(
                const ScreenPlacement *scrPlace,
                float x,
                float y,
                float w,
                float h,
                int horzAlign,
                int vertAlign,
                const float *color,
                float percentDone);
void __cdecl CL_RegisterDvars();


extern clientUIActive_t clientUIActives[1];
