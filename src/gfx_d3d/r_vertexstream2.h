#pragma once

#include <d3d9.h>

void __cdecl R_VertexStream2_Init(unsigned int stream2ByteSize);
bool __cdecl AllocateVertexStream2Buffer(unsigned int stream2ByteSize);
void __cdecl R_VertexStream2_Shutdown();
void FreeVertexStream2Buffer();
unsigned __int8 *__cdecl R_VertexStream2_Lock(unsigned int *stream2LockedSize);
void __cdecl R_VertexStream2_Unlock();
void __cdecl R_VertexStream2_Clear();
IDirect3DVertexBuffer9 **__cdecl R_VertexStream2_GetVB();
