#pragma once

void mjpeg_initonce();
void __cdecl mjpeg_set_callback(void (__cdecl *callback)(unsigned __int8 *, unsigned __int8 *, unsigned __int8 *));
void __cdecl mjpeg_init();
bool __cdecl mjpeg_is_encoding();
void __stdcall    mjpeg_thread(void *context);
void __cdecl mjpeg_close();
void __cdecl mjpeg_draw();

extern bool mjpeg_run_encoder;
