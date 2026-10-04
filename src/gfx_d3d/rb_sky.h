#pragma once
#include "rb_backend.h"

struct SunFlareDynamic // sizeof=0x2C
{                                       // XREF: .data:SunFlareDynamic * sunFlareArray/r
    float flareIntensity;
    float sunVisibilityAdjust;
    float currentBlind;
    float currentGlare;
    int lastTime;
    float lastVisibility;
    float lastDot;
    bool error;
    bool sunQueryIssued[2];             // XREF: RB_AllocSunSpriteQueries(void)+59/w
    // padding byte
    IDirect3DQuery9 *sunQuery[2];       // XREF: RB_CalcSunSpriteSamples(void):loc_B0161A/r
                                        // RB_AllocSunSpriteQueries(void)+49/w ...
    int hitNum;
};

unsigned int __cdecl RB_CalcSunSpriteSamples();
void __cdecl RB_DrawSun(unsigned int localClientNum);
void __cdecl RB_DrawSunQuerySprite(SunFlareDynamic *sunFlare);
void __cdecl RB_TessSunBillboard(float widthInClipSpace, float heightInClipSpace, GfxColor color);
void __cdecl RB_UpdateSunVisibilityWithoutQuery(SunFlareDynamic *sunFlare);
double __cdecl RB_GetSunSampleRectRelativeArea(int widthInPixels, int heightInPixels);
void __cdecl RB_AddSunEffects(SunFlareDynamic *sunFlare);
void __cdecl RB_DrawSunPostEffects(unsigned int localClientNum, float sunVisibilityAdjust);
void __cdecl RB_DrawSunFlare(SunFlareDynamic *sunFlare, int frameTime);
void __cdecl RB_DrawSunFlareCore(float alpha, float sizeIn640x480);
void __cdecl RB_DrawBlindAndGlare(SunFlareDynamic *sunFlare, int frameTime);
void __cdecl RB_CalcSunBlind(SunFlareDynamic *sunFlare, int frameTime, float *blind, float *glare);
void __cdecl RB_AllocSunSpriteQueries();
void __cdecl RB_FreeSunSpriteQueries();

extern SunFlareDynamic sunFlareArray[4];
