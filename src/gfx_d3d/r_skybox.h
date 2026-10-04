#pragma once

struct XModel;

void R_RegisterSkyboxModel();
void __cdecl R_ClearSkyboxModel();
void __cdecl R_AddSkyboxModel(const float *eyePos);
XModel *__cdecl R_MapGetSkyboxModel();

extern XModel *g_skyboxModel;
