#pragma once
#include "ik.h"

void    IK_Layer_TerrainMapping(IKState *ikState);
void    IK_Layer_ApplyFootIK(
                IKState *ikState,
                IKJointBones *jointBones,
                IKJointVars *jointVars,
                float (*childMat)[4]);
void    IK_Layer_ApplyHandIK(
                IKState *ikState,
                IKJointBones *jointBones,
                IKJointVars *jointVars,
                float (*childMat)[4]);
void    IK_Layer_LeftHandOnGun(IKState *ikState);
void __cdecl IK_Layer_PlayerControllers(IKState *ikState);
void    IK_Layer_PlayerPitch(IKState *ikState, bool preControllers);
void    IK_Layer_PreventHeadClip(IKState *ikState);
void    IK_ProcessLayers(IKState *ikState);
