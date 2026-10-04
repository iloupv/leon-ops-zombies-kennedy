#pragma once
#include "fxprimitives.h"

struct FxCmd // sizeof=0x34
{                                       // XREF: ?CG_DrawActiveFrame@@YAHHHW4DemoType@@W4CubemapShot@@HH@Z/r
    FxSystem *system;
    int localClientNum;                 // XREF: R_GenerateSortedDrawSurfs+1316/w
    volatile int *spawnLock;
    bool useReflection;
    // padding byte
    // padding byte
    // padding byte
    float reflectionHeight;
    unsigned int smodelCount;
    unsigned __int8 *smodelVisData;
    float elemRemoveChance;
    unsigned __int8 updateCameraType;   // XREF: CG_DrawExtraCamFrame(int,int,DemoType,CubemapShot,int,int)+565/w
    // padding byte
    // padding byte
    // padding byte
    const GfxLight *visibleLights;      // XREF: R_GenerateSortedDrawSurfs+1322/w
    int visibleLightCount;              // XREF: R_GenerateSortedDrawSurfs+132E/w
    int updateEffectBegin;
    int updateEffectEnd;
};

struct GfxSceneEntity;

void __cdecl R_ProcessCmd_UpdateFxSpotLight(FxCmd *cmd);
void __cdecl R_ProcessCmd_UpdateFxNonDependent(FxCmd *cmd);
void __cdecl R_DrawEffects(FxCmd *cmd);
void __cdecl R_ProcessCmd_UpdateFxRemaining(FxCmd *cmd);
void __cdecl R_UpdateSpotLightEffect(FxCmd *cmd);
void __cdecl R_UpdateNonDependentEffects(FxCmd *cmd);
void __cdecl R_UpdateRemainingEffects(FxCmd *cmd);
void __cdecl R_UpdateXModelBoundsDelayed(GfxSceneEntity *sceneEnt);
void __cdecl R_SkinGfxEntityDelayed(GfxSceneEntity *sceneEnt);
