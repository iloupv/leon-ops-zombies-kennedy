#pragma once

#include <qcommon/bitarray.h>

enum $E73F4D5E40210A2851F956A075585D03 : __int32
{
    KB_LEFT                  = 0x0,
    KB_RIGHT                 = 0x1,
    KB_FORWARD               = 0x2,
    KB_BACK                  = 0x3,
    KB_LOOKUP                = 0x4,
    KB_LOOKDOWN              = 0x5,
    KB_MOVELEFT              = 0x6,
    KB_MOVERIGHT             = 0x7,
    KB_STRAFE                = 0x8,
    KB_SPEED                 = 0x9,
    KB_UP                    = 0xA,
    KB_DOWN                  = 0xB,
    KB_ANYUP                 = 0xC,
    KB_MLOOK                 = 0xD,
    KB_SPECTATE              = 0xE,
    KB_SPECNEXT              = 0xF,
    KB_SPECPREV              = 0x10,
    KB_ATTACK                = 0x11,
    KB_BREATH                = 0x12,
    KB_FRAG                  = 0x13,
    KB_OFFHANDSECONDARY      = 0x14,
    KB_MELEE                 = 0x15,
    KB_ACTIVATE              = 0x16,
    KB_RELOAD                = 0x17,
    KB_USE_RELOAD            = 0x18,
    KB_LEANLEFT              = 0x19,
    KB_LEANRIGHT             = 0x1A,
    KB_PRONE                 = 0x1B,
    KB_CROUCH                = 0x1C,
    KB_THROW                 = 0x1D,
    KB_SPRINT                = 0x1E,
    KB_NIGHTVISION           = 0x1F,
    KB_GAS                   = 0x20,
    KB_REVERSE               = 0x21,
    KB_HANDBRAKE             = 0x22,
    KB_SWITCHSEAT            = 0x23,
    KB_VEHICLEATTACK         = 0x24,
    KB_VEHICLEATTACKSECOND   = 0x25,
    KB_VEHICLEMOVEUP         = 0x26,
    KB_VEHICLEMOVEDOWN       = 0x27,
    KB_VEHICLESPECIALABILITY = 0x28,
    KB_VEHICLEFIREPICKUP     = 0x29,
    KB_VEHICLESWAPPICKUP     = 0x2A,
    KB_VEHICLEDROPDEPLOYABLE = 0x2B,
    KB_VEHICLEBOOST          = 0x2C,
    KB_JAM                   = 0x2D,
    KB_TALK                  = 0x2E,
    NUM_BUTTONS              = 0x2F,
};

struct kbutton_t // sizeof=0x18
{                                       // XREF: .data:playersKb/r
    int down[2];                        // XREF: IN_TalkDown+3/o
                                        // IN_TalkUp+3/o
    unsigned int downtime;
    unsigned int msec;
    bool active;                        // XREF: IN_IsTalkKeyHeld(void)+11/r
    bool wasPressed;
    // padding byte
    // padding byte
    float val;
};

enum StanceState : __int32;

struct usercmd_s;
struct clientActive_t;



void __cdecl CL_SetStance(int localClientNum, StanceState stance);
void __cdecl IN_UpdateUseCount();
void __cdecl IN_UpdateUseHeld();
void __cdecl IN_CenterView();
bool __cdecl IN_IsTalkKeyHeld();
int __cdecl CL_MouseEvent(int x, int y, int dx, int dy);
void CL_SP_HeadlessMouseTest(); // p1 c30 TEST SWITCH bo1_mousetest (headless client only)
void __cdecl CL_SetCursorPos(unsigned int x, unsigned int y);
void __cdecl CL_ShowSystemCursor(bool show);
bool __cdecl CL_KeyActiveOrPressed(int localClientNum, unsigned int kbButton);
void __cdecl CL_UpdateCmdButton(
                int localClientNum,
                bitarray<51> *cmdButton_bits,
                int kbButton,
                int buttonBit,
                int numBits);
void __cdecl CL_WritePacket(int localClientNum);
bool __cdecl CG_IsRemoteGuidingMissile(int localClientNum);
void __cdecl CL_SendCmd(int localClientNum);
bool __cdecl CL_ReadyToSendPacket(int localClientNum);
void __cdecl CL_CreateCmdsDuringConnection(int localClientNum);
void __cdecl CL_CreateNewCommands(int localClientNum);
usercmd_s *__cdecl CL_CreateCmd(usercmd_s *result, int localClientNum);
void __cdecl CL_AdjustAngles(int localClientNum);
double __cdecl CL_KeyState(kbutton_t *key);
void __cdecl CL_KeyMove(int localClientNum, usercmd_s *cmd);
void __cdecl CL_StanceButtonUpdate(int localClientNum);
bool __cdecl CG_IsSprinting(int localClientNum);
void __cdecl CL_AddCurrentStanceToCmd(int localClientNum, usercmd_s *cmd);
void __cdecl CL_DoubleTapButtonUpdate(int localClientNum, usercmd_s *cmd);
void __cdecl CL_GamepadMove(int localClientNum, usercmd_s *cmd);
void __cdecl CL_RandomMove(usercmd_s *cmd);
void __cdecl CL_MouseMove(int localClientNum, usercmd_s *cmd);
void __cdecl CL_GetMouseMovement(clientActive_t *cl, float *mx, float *my);
void __cdecl CL_CmdButtons(int localClientNum, usercmd_s *cmd);
void __cdecl CL_FinishMove(int localClientNum, usercmd_s *cmd);
char __cdecl CG_HandleLocationSelectionInput(int localClientNum, usercmd_s *cmd);
void __cdecl CL_Input(int localClientNum);
void __cdecl CL_InitInput();
void __cdecl IN_MLookDown();
void __cdecl IN_MLookUp();
void __cdecl IN_UpDown();
void __cdecl IN_KeyDown(kbutton_t *b);
void __cdecl IN_UpUp();
void __cdecl IN_KeyUp(kbutton_t *b);
void __cdecl IN_DownDown();
void __cdecl IN_DownUp();
void __cdecl IN_LeftDown();
void __cdecl IN_LeftUp();
void __cdecl IN_RightDown();
void __cdecl IN_RightUp();
void __cdecl IN_ForwardDown();
void __cdecl IN_ForwardUp();
void __cdecl IN_BackDown();
void __cdecl IN_BackUp();
void __cdecl IN_LookupDown();
void __cdecl IN_LookupUp();
void __cdecl IN_LookdownDown();
void __cdecl IN_LookdownUp();
void __cdecl IN_MoveleftDown();
void __cdecl IN_MoveleftUp();
void __cdecl IN_MoverightDown();
void __cdecl IN_MoverightUp();
void __cdecl IN_SpeedDown();
void __cdecl IN_LeaveADS();
void __cdecl IN_SpeedUp();
void __cdecl IN_StrafeDown();
void __cdecl IN_StrafeUp();
void __cdecl IN_Attack_Down();
void __cdecl IN_Attack_Up();
void __cdecl IN_Breath_Down();
void __cdecl IN_Breath_Up();
void __cdecl IN_MeleeBreath_Down();
void __cdecl IN_MeleeBreath_Up();
void __cdecl IN_Frag_Down();
void __cdecl IN_Frag_Up();
void __cdecl IN_Smoke_Down();
void __cdecl IN_Smoke_Up();
void __cdecl IN_Gas_Down();
void __cdecl IN_Gas_Up();
void __cdecl IN_Reverse_Down();
void __cdecl IN_Reverse_Up();
void __cdecl IN_Handbrake_Down();
void __cdecl IN_Handbrake_Up();
void __cdecl IN_SwitchSeat_Down();
void __cdecl IN_SwitchSeat_Up();
void __cdecl IN_VehicleBoost_Down();
void __cdecl IN_VehicleBoost_Up();
void __cdecl IN_VehicleAttack_Down();
void __cdecl IN_VehicleAttack_Up();
void __cdecl IN_VehicleAttackSecond_Down();
void __cdecl IN_VehicleAttackSecond_Up();
void __cdecl IN_VehicleMoveUp_Down();
void __cdecl IN_VehicleMoveUp_Up();
void __cdecl IN_VehicleMoveDown_Down();
void __cdecl IN_VehicleMoveDown_Up();
void __cdecl IN_VehicleSpecialAbility_Down();
void __cdecl IN_VehicleSpecialAbility_Up();
void __cdecl IN_VehicleFirePickup_Down();
void __cdecl IN_VehicleFirePickup_Up();
void __cdecl IN_VehicleSwapPickup_Down();
void __cdecl IN_VehicleSwapPickup_Up();
void __cdecl IN_VehicleDropDeployable_Down();
void __cdecl IN_VehicleDropDeployable_Up();
void __cdecl IN_BreathSprint_Down();
void __cdecl IN_BreathSprint_Up();
void __cdecl IN_Melee_Down();
void __cdecl IN_Melee_Up();
void __cdecl IN_Activate_Down();
void __cdecl IN_Activate_Up();
void __cdecl IN_Reload_Down();
void __cdecl IN_Reload_Up();
void __cdecl IN_UseReload_Down();
void __cdecl IN_UseReload_Up();
void __cdecl IN_LeanLeft_Down();
void __cdecl IN_LeanLeft_Up();
void __cdecl IN_LeanRight_Down();
void __cdecl IN_LeanRight_Up();
void __cdecl IN_Prone_Down();
void __cdecl IN_Prone_Up();
void __cdecl IN_Stance_Down();
void __cdecl IN_Stance_Up();
void __cdecl IN_ToggleADS();
void __cdecl IN_Throw_Down();
void __cdecl IN_Throw_Up();
void __cdecl IN_ToggleADS_Throw_Down();
void __cdecl IN_ToggleADS_Throw_Up();
void __cdecl IN_Speed_Throw_Down();
void __cdecl IN_Speed_Throw_Up();
void __cdecl IN_LowerStance();
void __cdecl IN_RaiseStance();
void __cdecl IN_ToggleCrouch();
void __cdecl CL_ToggleStance(int localClientNum, StanceState preferredStance);
void __cdecl IN_ToggleProne();
void __cdecl IN_GoProne();
void __cdecl IN_GoCrouch();
void __cdecl IN_GoStandDown();
void __cdecl IN_GoStandUp();
void __cdecl IN_SprintDown();
void __cdecl IN_SprintUp();
void __cdecl IN_SpecNext_Down();
void __cdecl IN_SpecNext_Up();
void __cdecl IN_SpecPrev_Down();
void __cdecl IN_SpecPrev_Up();
void __cdecl IN_ToggleSpec_Down();
void __cdecl IN_ToggleSpec_Up();
void __cdecl IN_ToggleView();
void __cdecl IN_TalkDown();
void __cdecl IN_TalkUp();
void __cdecl CL_ShutdownInput();
void __cdecl CL_ClearKeys(int localClientNum);