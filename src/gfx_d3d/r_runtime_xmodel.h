#pragma once

// Runtime models: XModels built at level load from a model file (rtmodels/<name>.rlm) instead of a fastfile,
// for mods whose models are converted on the player's PC from another game the player owns (see
// r_runtime_xmodel.cpp). The model names come from the bo1_rtmodels dvar.
void __cdecl R_RuntimeXModel_RegisterDvars();
void __cdecl R_RuntimeXModel_LoadForLevel(const char *mapName);
bool __cdecl R_RuntimeXModel_IsAnimated(const struct XModel *model);
void __cdecl R_RuntimeXModel_PoseDObj(struct DObj *obj, const float *origin, const float *angles, int timeMs,
                                      bool isPlayer, float aimFrac, float aimPitch);
