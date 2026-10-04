#pragma once
#include <physics/phys_local.h>
#include <qcommon/msg_mp.h>

struct NitrousVehicle;

struct VehicleStun // sizeof=0x18
{                                       // XREF: NitrousVehicleController/r
    bool m_changed_this_frame;
    // padding byte
    // padding byte
    // padding byte
    float m_stun_time;
    float m_last_throttle;
    float m_throttle_stun_time;
    float m_last_brake;
    float m_brake_stun_time;
};

struct NitrousVehicleController // sizeof=0xA0
{                                       // XREF: NitrousVehicle/r
    phys_vec3 m_script_goal_position;
    float m_script_goal_radius;
    float m_script_goal_speed;
    float m_drive_path_speed;
    float m_stopped_time;
    float m_stuck_time;
    phys_vec3 m_stuck_position;
    usercmd_s m_cmd;
    int m_cmd_local_client_num;
    bool m_cmd_valid;
    bool m_stop_at_goal;
    bool m_jump_button_pressed;
    VehicleStun m_stun;

    void Init();

    void UpdateScriptVehicleControl(NitrousVehicle *rbveh, float delta_t);
    void UpdateControlsStrafeMode(NitrousVehicle *rbveh);
    void UpdateVehicleInputs(
        NitrousVehicle *rbveh,
        float target_yaw,
        float target_accel,
        float forward_preference);

    void UpdateStun(NitrousVehicle *rbveh, float delta_t);

    void UpdateControls(NitrousVehicle *rbveh, float delta_t);
    void UpdateControlsDefault(NitrousVehicle *rbveh, float delta_t);
    void UpdateControlsTank(NitrousVehicle *rbveh);
    void SetScriptTarget(
        NitrousVehicle *rbveh,
        phys_vec3 *goal_position,
        float goal_radius,
        float goal_speed,
        bool stopAtGoal);
    void UpdateJump(NitrousVehicle *rbveh, float delta_t);
    void UpdateApplyBoatAccel(NitrousVehicle *rbveh, float delta_t);
};





double __cdecl StunRandomBrake(const usercmd_s *cmd, float desired_brake, float last_brake);
double __cdecl RecalibrateInputCustom(float val, float threshold);
