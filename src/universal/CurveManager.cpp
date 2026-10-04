#include "CurveManager.h"
#include <win32/win_common.h>
#include "assertive.h"
#include <universal/com_math.h>

// really dont see much point to this class, guys

cCurve cCurveManager::mCurves[4];
int cCurveManagerLLlevelNotifyFunc;

int __cdecl cCurveManager::GetFreeCurve()
{
    int i; // [esp+4h] [ebp-4h]

    Sys_EnterCriticalSection(CRITSECT_CURVEALLOC);
    for ( i = 0; i < 4; ++i )
    {
        if ( !cCurveManager::mCurves[i].mActive )
        {
            cCurveManager::mCurves[i].mActive = 1;
            Sys_LeaveCriticalSection(CRITSECT_CURVEALLOC);
            return i;
        }
    }
    Sys_LeaveCriticalSection(CRITSECT_CURVEALLOC);
    if ( !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\CurveManager.cpp",
                    94,
                    0,
                    "CurveManager - No unused curves available.") )
        __debugbreak();
    return -1;
}

void __cdecl cCurveManager::AddNodeToCurve(unsigned int curve, float *p)
{
    if ( curve >= 4
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\CurveManager.cpp",
                    101,
                    0,
                    "%s",
                    "curve >= 0 && curve < MAX_CURVES") )
    {
        __debugbreak();
    }
    if ( !cCurveManager::mCurves[curve].mActive
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\CurveManager.cpp",
                    102,
                    0,
                    "%s",
                    "mCurves[curve].IsActive()") )
    {
        __debugbreak();
    }
    //cCurve::AddNode(&cCurveManager::mCurves[curve], p);
    mCurves[curve].AddNode(p);
}

void __cdecl cCurveManager::AddNodeToCurve(unsigned int curve, float *p, float t)
{
    if ( curve >= 4
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\CurveManager.cpp",
                    109,
                    0,
                    "%s",
                    "curve >= 0 && curve < MAX_CURVES") )
    {
        __debugbreak();
    }
    if ( !cCurveManager::mCurves[curve].mActive
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\CurveManager.cpp",
                    110,
                    0,
                    "%s",
                    "mCurves[curve].IsActive()") )
    {
        __debugbreak();
    }
    //cCurve::AddNode(&cCurveManager::mCurves[curve], p, t);
    mCurves[curve].AddNode(p, t);
}

void __cdecl cCurveManager::SortCurve(unsigned int curve, float *p, bool is_increasing_sort_order)
{
    if ( curve >= 4
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\CurveManager.cpp",
                    117,
                    0,
                    "%s",
                    "curve >= 0 && curve < MAX_CURVES") )
    {
        __debugbreak();
    }
    if ( !cCurveManager::mCurves[curve].mActive
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\CurveManager.cpp",
                    118,
                    0,
                    "%s",
                    "mCurves[curve].IsActive()") )
    {
        __debugbreak();
    }
    //cCurve::Sort(&cCurveManager::mCurves[curve], p, is_increasing_sort_order);
    mCurves[curve].Sort(p, is_increasing_sort_order);
}

void __cdecl cCurveManager::BuildCurve(unsigned int curve)
{
    if ( curve >= 4
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\CurveManager.cpp",
                    125,
                    0,
                    "%s",
                    "curve >= 0 && curve < MAX_CURVES") )
    {
        __debugbreak();
    }
    if ( !cCurveManager::mCurves[curve].mActive
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\CurveManager.cpp",
                    126,
                    0,
                    "%s",
                    "mCurves[curve].IsActive()") )
    {
        __debugbreak();
    }
    //cCurve::Build(&cCurveManager::mCurves[curve]);
    mCurves[curve].Build();
}

void __cdecl cCurveManager::FreeCurve(unsigned int curve)
{
    if ( curve >= 4
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\CurveManager.cpp",
                    133,
                    0,
                    "%s",
                    "curve >= 0 && curve < MAX_CURVES") )
    {
        __debugbreak();
    }
    if ( !cCurveManager::mCurves[curve].mActive
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\CurveManager.cpp",
                    134,
                    0,
                    "%s",
                    "mCurves[curve].IsActive()") )
    {
        __debugbreak();
    }
    //cCurve::Reset(&cCurveManager::mCurves[curve]);
    mCurves[curve].Reset();
}

void __cdecl cCurveManager::SetCurveBSpline(unsigned int curve)
{
    if ( curve >= 4
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\CurveManager.cpp",
                    165,
                    0,
                    "%s",
                    "curve >= 0 && curve < MAX_CURVES") )
    {
        __debugbreak();
    }
    if ( !cCurveManager::mCurves[curve].mActive
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\CurveManager.cpp",
                    166,
                    0,
                    "%s",
                    "mCurves[curve].IsActive()") )
    {
        __debugbreak();
    }

    //if ( cCurveManager::mCurves[curve].mCurveType != CURVE_BSPLINE )
    //    cCurveManager::mCurves[curve].mCurveType = CURVE_BSPLINE;
    if (mCurves[curve].mCurveType != cCurve::eCurveType::CURVE_BSPLINE)
    {
        cCurveManager::mCurves[curve].mCurveType = cCurve::eCurveType::CURVE_BSPLINE;
    }
}

void __cdecl cCurveManager::SetCurveDraw(unsigned int curve, const float *color)
{
    if ( curve >= 4
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\CurveManager.cpp",
                    173,
                    0,
                    "%s",
                    "curve >= 0 && curve < MAX_CURVES") )
    {
        __debugbreak();
    }
    if ( !cCurveManager::mCurves[curve].mActive
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\CurveManager.cpp",
                    174,
                    0,
                    "%s",
                    "mCurves[curve].IsActive()") )
    {
        __debugbreak();
    }
    //cCurve::SetDraw(&cCurveManager::mCurves[curve], color);
    mCurves[curve].SetDraw(color);
}

void __cdecl cCurveManager::GetPos(unsigned int curve, float t, float *p)
{
    if ( curve >= 4
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\CurveManager.cpp",
                    277,
                    0,
                    "%s",
                    "curve >= 0 && curve < MAX_CURVES") )
    {
        __debugbreak();
    }
    if ( !cCurveManager::mCurves[curve].mActive
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\CurveManager.cpp",
                    278,
                    0,
                    "%s",
                    "mCurves[curve].IsActive()") )
    {
        __debugbreak();
    }
    //cCurve::GetPos(&cCurveManager::mCurves[curve], t, p);
    mCurves[curve].GetPos(t, p);

    nanassertvec3(p);
}

double __cdecl cCurveManager::GetCurveLength(unsigned int curve)
{
    if ( curve >= 4
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\CurveManager.cpp",
                    287,
                    0,
                    "%s",
                    "curve >= 0 && curve < MAX_CURVES") )
    {
        __debugbreak();
    }
    if ( !cCurveManager::mCurves[curve].mActive
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\CurveManager.cpp",
                    288,
                    0,
                    "%s",
                    "mCurves[curve].IsActive()") )
    {
        __debugbreak();
    }
    //return cCurve::GetLength(&cCurveManager::mCurves[curve]);

    return mCurves[curve].GetLength();
}

