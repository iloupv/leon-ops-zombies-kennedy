#include "cg_pose_utils.h"

void __cdecl CG_UsedDObjCalcPose(cpose_t *pose)
{
    if ( !pose && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_pose_utils.cpp", 11, 0, "%s", "pose") )
        __debugbreak();
    _InterlockedCompareExchange(&pose->cullIn, 1, 0);
}

void __cdecl CG_CullIn(cpose_t *pose)
{
    if ( !pose && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_pose_utils.cpp", 24, 0, "%s", "pose") )
        __debugbreak();
    pose->cullIn = 2;
}

