#pragma once

void __cdecl yuv_init_internal();
void __cdecl yuv_init(int width, int height, bool double_buffer);
char __cdecl yuv_encode_frame();
void __cdecl yuv_get_buffers(unsigned __int8 **y, unsigned __int8 **u, unsigned __int8 **v);
void __cdecl yuv_shutdown_internal();
void __cdecl yuv_shutdown();
void __cdecl yuv_lost_device();
void __cdecl yuv_recover_device();
