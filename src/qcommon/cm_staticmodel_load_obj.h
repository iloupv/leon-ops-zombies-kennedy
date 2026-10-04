#pragma once

struct cStaticModel_s;

void __cdecl CM_LoadStaticModels();
char __cdecl CM_CreateStaticModel(cStaticModel_s *staticModel, char *name, float *origin, float *angles, float scale);
void __cdecl CM_InitStaticModel(cStaticModel_s *staticModel, float *origin, float *angles, float scale);
