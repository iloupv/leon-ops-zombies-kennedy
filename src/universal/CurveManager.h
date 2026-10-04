#pragma once
#include "curve.h"

struct cCurveManager // sizeof=0x0
{
    static int __cdecl GetFreeCurve();

    static void __cdecl AddNodeToCurve(unsigned int curve, float *p);
    static void __cdecl AddNodeToCurve(unsigned int curve, float *p, float t);
    static void __cdecl SortCurve(unsigned int curve, float *p, bool is_increasing_sort_order);
    static void __cdecl BuildCurve(unsigned int curve);
    static void __cdecl FreeCurve(unsigned int curve);
    static void __cdecl SetCurveBSpline(unsigned int curve);
    static void __cdecl SetCurveDraw(unsigned int curve, const float *color);
    static void __cdecl GetPos(unsigned int curve, float t, float *p);
    static double __cdecl GetCurveLength(unsigned int curve);

    static cCurve mCurves[4];
    static int levelNotifyFunc;
};