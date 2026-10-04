#pragma once
#include "ik.h"

void __cdecl IK_GeneratePreIKMatrices(IKState *ikState, bool isLocalBones);
void    IK_GenerateIKXformMatrices(IKState *ikState, bool isLocalBones);
char __cdecl IK_HasAnimatedBones(IKState *ikState);
void    IK_Process(IKState *ikState, bool isLocalBones);
