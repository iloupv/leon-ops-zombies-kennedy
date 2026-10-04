#pragma once
#include <bgame/bg_local.h>

enum LaserOwnerEnum : __int32
{                                       // XREF: ?CG_Laser_Add@@YAXPAUcentity_s@@PAUDObj@@PAUcpose_t@@QBMW4LaserOwnerEnum@@@Z/r
                                        // CG_Laser_Add_Core/r
    LASER_OWNER_NON_PLAYER = 0x0,
    LASER_OWNER_PLAYER     = 0x1,
};

void __cdecl CG_Laser_Add(centity_s *cent, DObj *obj, cpose_t *pose, const float *viewerPos, LaserOwnerEnum laserOwner);
void __cdecl CG_Laser_Add_Core(
                centity_s *cent,
                DObj *obj,
                orientation_t *orient,
                const float *viewerPos,
                LaserOwnerEnum laserOwner);
