#pragma once

struct trajectory_t;

int __cdecl CG_irand(int min, int max);
double __cdecl CG_flrand(float min, float max);
int __cdecl CG_rand();
float __cdecl CG_random();
bool __cdecl CG_HasClientSystemBeenInitialzed();
void __cdecl CG_ClientDoneWithInitialization();
void __cdecl CG_ResetClientInitializationState();
int __cdecl CG_GetWeaponIndexForName(char *name);
void __cdecl CG_RegisterVehicle(const char *name, __int16 index);
void __cdecl CG_RegisterVehicleMaterial(__int16 index);
void __cdecl CG_Veh_Init();
void __cdecl CG_Veh_RegisterMaterials();
char __cdecl CG_VehicleUpdateField(const char *vehicleInfoName, char *keyValue);
bool __cdecl CG_IsVehicleRemoteControl(int index);
void __cdecl CG_UpdateUIDeviceContexts(int localClientNum, int time);
double __cdecl CG_GetDefaultFovForView();
void __cdecl CG_UpdateFov(float value);
void __cdecl CG_ModFov_Update(); // mod: bo1_mod_fov
void __cdecl CG_SetThirdPerson(int value);
bool __cdecl CG_IsEntWalkable(int localClientNum, int entityNum);
int __cdecl CScriptMover_UpdateMove(
                trajectory_t *pTr,
                float *vCurrPos,
                float fSpeed,
                float fMidTime,
                float fDecelTime,
                const float *vPos1,
                const float *vPos2,
                const float *vPos3);
