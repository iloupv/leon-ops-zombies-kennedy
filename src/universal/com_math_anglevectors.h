#pragma once

void __cdecl AngleVectors(const float *angles, float *forward, float *right, float *up);
void __cdecl AnglesToAxis(const float *angles, float axis[3][3]);
void __cdecl AnglesToQuat(const float *angles, float *quat);
void __cdecl QuatToAxis(const float *quat, float axis[3][3]);
void __cdecl AxisToSignedAngles(const float (*axis)[3], float *angles);
void __cdecl vectosignedangles(const float *vec, float *angles);
double __cdecl vectosignedpitch(const float *vec);
