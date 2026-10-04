#include "FxCurve.h"

void __cdecl FxCurveIterator_Create(FxCurveIterator *createe, const FxCurve *master)
{
    if ( !createe
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\FxCurve.cpp", 62, 0, "%s", "createe") )
    {
        __debugbreak();
    }
    if ( !master
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\FxCurve.cpp", 64, 0, "%s", "master") )
    {
        __debugbreak();
    }
    if ( master->keyCount <= 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\FxCurve.cpp",
                    65,
                    0,
                    "%s\n\t(master->keyCount) = %i",
                    "(master->keyCount > 0)",
                    master->keyCount) )
    {
        __debugbreak();
    }
    if ( master->dimensionCount <= 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\FxCurve.cpp",
                    66,
                    0,
                    "%s\n\t(master->dimensionCount) = %i",
                    "(master->dimensionCount > 0)",
                    master->dimensionCount) )
    {
        __debugbreak();
    }
    if ( master == (const FxCurve *)-8
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\FxCurve.cpp", 67, 0, "%s", "master->keys") )
    {
        __debugbreak();
    }
    createe->master = master;
    createe->currentKeyIndex = 0;
}

void __cdecl FxCurveIterator_Release(FxCurveIterator *releasee)
{
    if ( !releasee
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\FxCurve.cpp", 83, 0, "%s", "releasee") )
    {
        __debugbreak();
    }
    if ( !releasee->master
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\FxCurve.cpp", 84, 0, "%s", "releasee->master") )
    {
        __debugbreak();
    }
    releasee->master = 0;
}

