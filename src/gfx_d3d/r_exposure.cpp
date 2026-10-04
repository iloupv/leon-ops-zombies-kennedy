#include "r_exposure.h"
#include "r_globalgfxdata.h"

void __cdecl R_DefaultExposureValue(int index)
{
    exposureSettings.exposureValues[index].blackPoint[0] = 0.0f;
    exposureSettings.exposureValues[index].blackPoint[1] = 0.0f;
    exposureSettings.exposureValues[index].blackPoint[2] = 0.0f;
    exposureSettings.exposureValues[index].blackPoint[3] = 8.0f;
    exposureSettings.exposureValues[index].whitePoint[0] = 8.0f;
    exposureSettings.exposureValues[index].whitePoint[1] = 8.0f;
    exposureSettings.exposureValues[index].whitePoint[2] = 8.0f;
    exposureSettings.exposureValues[index].whitePoint[3] = 0.0f;
    exposureSettings.exposureValues[index].linearStart[0] = 0.0f;
    exposureSettings.exposureValues[index].linearStart[1] = 0.0f;
    exposureSettings.exposureValues[index].linearStart[2] = 0.0f;
    exposureSettings.exposureValues[index].linearStart[3] = 0.0f;
    exposureSettings.exposureValues[index].linearEnd[0] = 1.0f;
    exposureSettings.exposureValues[index].linearEnd[1] = 1.0f;
    exposureSettings.exposureValues[index].linearEnd[2] = 1.0f;
    exposureSettings.exposureValues[index].linearEnd[3] = 0.0f;
    exposureSettings.exposureValues[index].remapStart[0] = 0.0f;
    exposureSettings.exposureValues[index].remapStart[1] = 0.0f;
    exposureSettings.exposureValues[index].remapStart[2] = 0.0f;
    exposureSettings.exposureValues[index].remapStart[3] = 0.0f;
    exposureSettings.exposureValues[index].remapEnd[0] = 1.0f;
    exposureSettings.exposureValues[index].remapEnd[1] = 1.0f;
    exposureSettings.exposureValues[index].remapEnd[2] = 1.0f;
    exposureSettings.exposureValues[index].remapEnd[3] = 0.0f;
    exposureSettings.exposureValues[index].scurveStart[0] = -0.5f;
    exposureSettings.exposureValues[index].scurveStart[1] = -0.5f;
    exposureSettings.exposureValues[index].scurveStart[2] = -0.5f;
    exposureSettings.exposureValues[index].scurveStart[3] = 0.0f;
    exposureSettings.exposureValues[index].scurveEnd[0] = -0.5f;
    exposureSettings.exposureValues[index].scurveEnd[1] = -0.5f;
    exposureSettings.exposureValues[index].scurveEnd[2] = -0.5f;
    exposureSettings.exposureValues[index].scurveEnd[3] = 0.0f;
    exposureSettings.exposureValues[index].bloomCurveLDR[0] = 1.0f;
    exposureSettings.exposureValues[index].bloomCurveLDR[1] = 1.0f;
    exposureSettings.exposureValues[index].bloomCurveLDR[2] = 1.0f;
    exposureSettings.exposureValues[index].bloomCurveLDR[3] = 1.0f;
    exposureSettings.exposureValues[index].bloomCurveHDR[0] = 1.0f;
    exposureSettings.exposureValues[index].bloomCurveHDR[1] = 1.0f;
    exposureSettings.exposureValues[index].bloomCurveHDR[2] = 1.0f;
    exposureSettings.exposureValues[index].bloomCurveHDR[3] = 1.0f;
    exposureSettings.exposureValues[index].bloomScale[0] = 0.0f;
    exposureSettings.exposureValues[index].bloomScale[1] = 0.0f;
    exposureSettings.exposureValues[index].bloomScale[2] = 0.0f;
    exposureSettings.exposureValues[index].bloomScale[3] = 0.0f;
}

void __cdecl R_SetExposureToDefault()
{
    int i; // [esp+0h] [ebp-4h]

    for ( i = 0; i < 256; ++i )
        R_DefaultExposureValue(i);
}

void __cdecl R_LoadExposureData(unsigned int *data)
{
    unsigned int i; // [esp+4h] [ebp-Ch]

    for ( i = 0; i < 0x2C04; ++i )
        *(&exposureSettings.header.identifier + i) = data[i];
}

