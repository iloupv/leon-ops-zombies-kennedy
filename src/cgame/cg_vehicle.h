#pragma once
#include <bgame/bg_local.h>

enum VehicleSound : __int32
{                                       // XREF: CG_UpdateVehicleEngineSounds/r
                                        // CG_UpdateVehicleEngineSounds/r ...
    VEH_LOW_IDLE_SND                 = 0x0,
    VEH_LOW_IDLE_SND_PLYR            = 0x1,
    VEH_HIGH_IDLE_SND                = 0x2,
    VEH_LOW_ENGINE_SND               = 0x3,
    VEH_HIGH_ENGINE_SND              = 0x4,
    VEH_TURRET_SPIN_SND              = 0x5,
    VEH_TURRET_STOP_SND              = 0x6,
    VEH_ENGINE_MODLOOP_NOLOAD_SND    = 0x7,
    VEH_ENGINE_MODLOOP_NOLOAD_SND_PLYR = 0x8,
    VEH_ENGINE_MODLOOP_LOAD_SND      = 0x9,
    VEH_ENGINE_MODLOOP_LOAD_SND_PLYR = 0xA,
    VEH_BODYDMG_LOOP_LEFT_SND        = 0xB,
    VEH_BODYDMG_LOOP_RIGHT_SND       = 0xC,
    VEH_TANK_GRIND_LEFT_SND          = 0xD,
    VEH_TANK_GRIND_RIGHT_SND         = 0xE,
    VEH_TANK_GRIND_LEFT_DMG_SND      = 0xF,
    VEH_TANK_GRIND_RIGHT_DMG_SND     = 0x10,
    VEH_TANK_BOOSTPERK_SND           = 0x11,
    VEH_TANK_BOOSTPERK_SND_PLYR      = 0x12,
    NUM_VEHICLE_SNDS                 = 0x13,
};

void __cdecl CG_UpdateVehicleWaterFX(int localClientNum, centity_s *cent);
// local variable allocation has failed, the output may be wrong!
void    CG_Vehicle_PreControllers(
                int localClientNum,
                const DObj *obj,
                centity_s *cent);
void    VEH_UpdateMaterialTime(centity_s *cent, int dt, float speed);
int __cdecl GetSoundInfoForVehicle(int localClientNum, const centity_s *cent, float *slideRate);
void __cdecl GetSoundOriginForVehicle(const centity_s *cent, float *origin);
void __cdecl CG_CrossFadeVehicleMaterialEffects(
                int localClientNum,
                const centity_s *cent,
                const vehicle_info_t *info,
                unsigned int groundEffectIndex,
                int surfType,
                float ratio,
                int fadeTime);
void __cdecl CG_UpdateVehicleNotifies(int localClientNum, centity_s *cent);
void __cdecl CG_UpdateVehicleSounds(int localClientNum, centity_s *cent);
void __cdecl CG_UpdateSurfaceSounds(int localClientNum, centity_s *cent, const vehicle_info_t *info);
void    CG_UpdateWheeledSurfaceSounds(
                int localClientNum,
                centity_s *cent,
                const vehicle_info_t *info);
// local variable allocation has failed, the output may be wrong!
void    CG_UpdateTankSurfaceSounds(
                int localClientNum,
                centity_s *cent,
                const vehicle_info_t *info);
void __cdecl CG_UpdateFireSounds(
                int localClientNum,
                centity_s *cent,
                const vehicle_info_t *info,
                bool isLocalPlayersVehicle);
void __cdecl CG_UpdateBoostSounds(
                int localClientNum,
                centity_s *cent,
                const vehicle_info_t *info,
                bool isLocalPlayersVehicle);
void    CG_UpdateVehicleEngineSounds(
                int localClientNum,
                centity_s *cent,
                const vehicle_info_t *info,
                bool isLocalPlayersVehicle);
void __cdecl CG_UpdateTurretAndOverheatSounds(
                int localClientNum,
                centity_s *cent,
                const vehicle_info_t *info,
                bool isLocalPlayersVehicle);
