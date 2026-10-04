#pragma once

#include <tl/jobqueue/jobqueue_all.h>

struct jqBatch;

int __cdecl r_dpvs_entityCallback(jqBatch *batch);
int __cdecl r_model_skinCallback(jqBatch *batch);
bool __cdecl R_DynamicFenceBusy();
int __cdecl r_dpvs_staticCallback(jqBatch *batch);
int __cdecl r_dpvs_dynmodelCallback(jqBatch *batch);
int __cdecl r_dpvs_sceneentCallback(jqBatch *batch);
int __cdecl r_dpvs_dynbrushCallback(jqBatch *batch);
int __cdecl r_waterCallback(jqBatch *batch);
int __cdecl R_UploadWaterPending();
signed __int32 R_ExecWaterFailed();
int __cdecl r_water_simCallback(jqBatch *batch);
bool __cdecl R_EndFenceBusy();
int __cdecl r_spot_shadow_entCallback(jqBatch *batch);
int __cdecl fx_updateCallback(jqBatch *batch);
int __cdecl fx_update_remainingCallback(jqBatch *batch);
bool __cdecl R_FXUpdateRemainingWait();
bool __cdecl R_FXSpotLightPending();
int __cdecl fx_update_remaining_ppuCallback(jqBatch *batch);
bool __cdecl R_FXUpdateRemainingPPUWait();
bool __cdecl R_FXDependentPending();
int __cdecl fx_drawCallback(jqBatch *batch);
int __cdecl dobj_skelCallback(jqBatch *batch);
int __cdecl dobj_skinCallback(jqBatch *batch);
int __cdecl r_model_lightingCallback(jqBatch *batch);
int __cdecl r_add_sceneentCallback(jqBatch *batch);
int __cdecl fx_marks_drawCallback(jqBatch *batch);
int __cdecl fx_update_spotCallback(jqBatch *batch);
int __cdecl r_skin_cached_staticmodelCallback(jqBatch *batch);

extern jqModule fx_update_spotModule;
extern jqModule fx_updateModule;
extern jqModule fx_update_remainingModule;
extern jqModule r_dpvs_staticModule;
extern jqModule r_dpvs_sceneentModule;
extern jqModule r_dpvs_dynmodelModule;
extern jqModule r_dpvs_dynbrushModule;
extern jqModule r_dpvs_entityModule;
extern jqModule r_add_sceneentModule;
extern jqModule r_spot_shadow_entModule;
extern jqModule dobj_skelModule;
extern jqModule dobj_skinModule;
extern jqModule fx_drawModule;
extern jqModule fx_marks_drawModule;
extern jqModule r_model_skinModule;
extern jqModule nuge_physicsModule;
extern jqModule r_skin_cached_staticmodelModule;

extern jqWorkerCmd r_water_simWorkerCmd;

extern jqWorkerCmd fx_update_spotWorkerCmd;
extern jqWorkerCmd fx_updateWorkerCmd;
extern jqWorkerCmd fx_update_remainingWorkerCmd;
extern jqWorkerCmd r_dpvs_staticWorkerCmd;
extern jqWorkerCmd r_dpvs_sceneentWorkerCmd;
extern jqWorkerCmd r_dpvs_dynmodelWorkerCmd;
extern jqWorkerCmd r_dpvs_dynbrushWorkerCmd;
extern jqWorkerCmd r_dpvs_entityWorkerCmd;
extern jqWorkerCmd r_add_sceneentWorkerCmd;
extern jqWorkerCmd r_spot_shadow_entWorkerCmd;
extern jqWorkerCmd dobj_skelWorkerCmd;
extern jqWorkerCmd dobj_skinWorkerCmd;
extern jqWorkerCmd fx_drawWorkerCmd;
extern jqWorkerCmd fx_marks_drawWorkerCmd;
extern jqWorkerCmd r_model_skinWorkerCmd;
extern jqWorkerCmd nuge_physicsWorkerCmd;
extern jqWorkerCmd r_skin_cached_staticmodelWorkerCmd;
extern jqWorkerCmd fx_update_remaining_ppuWorkerCmd;
extern jqWorkerCmd r_model_lightingWorkerCmd;