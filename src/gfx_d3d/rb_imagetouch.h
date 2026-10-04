#pragma once
#include "r_material.h"

void __cdecl RB_TouchAllImages();
void __cdecl RB_TouchImage(GfxImage *image);
int __cdecl RB_CompareTouchImages(int *e0, int *e1);
