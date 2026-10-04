#pragma once

#include <universal/q_shared.h>

int __cdecl VEH_ParseSpecificField(unsigned __int8 *pStruct, const char *pValue, const int fieldType, const int fieldOffset);

extern const char *s_vehicleCameraModes[6];
extern const char *s_vehicleBoostModes [2];
extern const char *s_tractionTypeNames [3];
extern const char *s_vehicleClassNames [7];

extern cspField_t s_vehicleFields[345];
const __int16 s_numVehicleFields = 345;