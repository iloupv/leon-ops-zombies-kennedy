#pragma once
#include <client/screen_placement.h>

struct Material;

void __cdecl UI_DrawHandlePic(
                const ScreenPlacement *scrPlace,
                float x,
                float y,
                float w,
                float h,
                int horzAlign,
                int vertAlign,
                const float *color,
                Material *material);
void __cdecl UI_DrawHandlePicRotated(
                const ScreenPlacement *scrPlace,
                float x,
                float y,
                float w,
                float h,
                int horzAlign,
                int vertAlign,
                const float *color,
                float angle,
                Material *material);
void __cdecl UI_DrawHandlePicFramed(
                const ScreenPlacement *scrPlace,
                float x,
                float y,
                float w,
                float h,
                int horzAlign,
                int vertAlign,
                float frameSize,
                float frameTexSize,
                int frameSides,
                const float *color,
                Material *material);
void __cdecl UI_DrawLoadBar(
                const ScreenPlacement *scrPlace,
                float x,
                float y,
                float w,
                float h,
                int horzAlign,
                int vertAlign,
                const float *color,
                Material *material);
double __cdecl UI_LoadBarProgress_LoadObj();
double __cdecl UI_LoadBarProgress_FastFile();
void __cdecl UI_FillRectPhysical(float x, float y, float width, float height, const float *color);
void __cdecl UI_FillRect(
                const ScreenPlacement *scrPlace,
                float x,
                float y,
                float width,
                float height,
                int horzAlign,
                int vertAlign,
                const float *color);
void __cdecl UI_DrawSpinner(
                const ScreenPlacement *scrPlace,
                float x,
                float y,
                float width,
                float height,
                int horzAlign,
                int vertAlign,
                const float *color);
void __cdecl UI_DrawSpinnerLoadbar(
                const ScreenPlacement *scrPlace,
                float x,
                float y,
                float width,
                float height,
                int horzAlign,
                int vertAlign,
                const float *color);
