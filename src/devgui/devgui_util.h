#pragma once

unsigned int __cdecl DevGui_GetScreenWidth();
unsigned int __cdecl DevGui_GetScreenHeight();
void __cdecl DevGui_DrawBox(int x, int y, int w, int h, const unsigned __int8 *color);
void __cdecl DevGui_DrawBoxCentered(int centerX, int centerY, int w, int h, const unsigned __int8 *color);
void __cdecl DevGui_DrawBevelBox(int x, int y, int w, int h, float shade, const unsigned __int8 *color);
void __cdecl DevGui_DrawQuad(const int (*vtxs)[2], const float *color);
void    DevGui_DrawLine(float *start, float *end, int width, const unsigned __int8 *color);
void __cdecl DevGui_DrawFont(int x, int y, const unsigned __int8 *color, char *text, float xScale, float yScale);
int __cdecl DevGui_GetFontWidth(const char *text);
int __cdecl DevGui_GetFontHeight();
