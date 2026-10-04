#pragma once
#include "ragdoll.h"

void __cdecl Ragdoll_Clear_f();
void __cdecl Ragdoll_ResetBodiesUsingDef();
bool __cdecl Ragdoll_BodyBound(RagdollBody *body);
void __cdecl Ragdoll_InitCommands();
void __cdecl Ragdoll_Bone_f();
char __cdecl Ragdoll_ReadGeomType(int arg, BoneDef *bone);
void __cdecl Ragdoll_Joint_f();
char __cdecl Ragdoll_ReadJointType(int arg, JointDef *joint);
void __cdecl Ragdoll_Limit_f();
char __cdecl Ragdoll_ReadAxis(int arg, float *dest);
void __cdecl Ragdoll_Selfpair_f();
void __cdecl Ragdoll_BaseLerpBone_f();
void __cdecl Ragdoll_PinBone_f();
