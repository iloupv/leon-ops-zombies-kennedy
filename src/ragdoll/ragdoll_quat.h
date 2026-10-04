#pragma once

void __cdecl Ragdoll_QuatMul(const float *qa, const float *qb, float *dest);
void __cdecl Ragdoll_QuatMulInvSecond(const float *qa, const float *qb, float *dest);
void __cdecl Ragdoll_QuatConjugate(const float *src, float *dest);
void __cdecl Ragdoll_QuatInverse(const float *src, float *dest);
void __cdecl Ragdoll_QuatPointRotate(const float *p, const float *q, float *dest);
void __cdecl Ragdoll_Mat33ToQuat(const float (*axis)[3], float *quat);
void __cdecl Ragdoll_QuatToAxisAngle(const float *quat, float *axisAngle);
void __cdecl Ragdoll_QuatLerp(const float *qa, const float *qb, float t, float *dest);
