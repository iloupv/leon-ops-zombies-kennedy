#pragma once
#include <cgame_mp/cg_local_mp.h>
#include <universal/memfile.h>

struct CameraShake // sizeof=0x24
{                                       // XREF: CameraShakeSet/r
                                        // ?CG_StartShakeCamera@@YAXHMHQAMM@Z/r
    int time;                           // XREF: CG_StartShakeCamera(int,float,int,float * const,float)+D9/w
    float scale;                        // XREF: CG_StartShakeCamera(int,float,int,float * const,float)+C1/w
    float length;                       // XREF: CG_StartShakeCamera(int,float,int,float * const,float)+CB/w
    float radius;                       // XREF: CG_StartShakeCamera(int,float,int,float * const,float)+107/w
    float src[3];                       // XREF: CG_StartShakeCamera(int,float,int,float * const,float)+E3/w
                                        // CG_StartShakeCamera(int,float,int,float * const,float)+F0/w ...
    float size;                         // XREF: CG_StartShakeCamera(int,float,int,float * const,float)+AA/w
                                        // CG_StartShakeCamera(int,float,int,float * const,float):loc_480884/r
    float rumbleScale;                  // XREF: CG_StartShakeCamera(int,float,int,float * const,float)+B7/w
};

struct CameraShakeSet // sizeof=0x94
{                                       // XREF: .data:s_cameraShakeSet/r
    CameraShake shakes[4];
    float phase;
};

void __cdecl CG_StartShakeCamera(int localClientNum, float p, int duration, float *src, float radius);
int __cdecl CG_UpdateCameraShake(const cg_s *cgameGlob, CameraShake *shake);
void __cdecl CG_ShakeCamera(int localClientNum);
void __cdecl MemFile_WriteData(MemoryFile *memFile, unsigned int byteCount, unsigned __int8 *p);
void __cdecl CG_ClearCameraShakes(int localClientNum);
