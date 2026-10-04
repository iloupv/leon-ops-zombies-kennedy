#include "r_workercmds_common.h"
#include <EffectsCore/fx_update.h>
#include <qcommon/threads.h>
#include <EffectsCore/fx_draw.h>
#include <universal/com_workercmds.h>
#include "r_workercmds.h"
#include <cgame_mp/cg_draw_mp.h>
#include <EffectsCore/fx_marks.h>
#include "r_scene.h"

void __cdecl R_ProcessCmd_UpdateFxSpotLight(FxCmd *cmd)
{
    PROF_SCOPED("R_ProcessCmd_UpdateFxSpotLight");
    //BLOPS_NULLSUB();
    FX_UpdateSpotLight(cmd);
    Sys_SetUpdateSpotLightEffectEvent();
}

void __cdecl R_ProcessCmd_UpdateFxNonDependent(FxCmd *cmd)
{
    PROF_SCOPED("R_ProcessCmd_UpdateFxNonDependent");
    FX_UpdateNonDependent(cmd);
    Sys_SetUpdateNonDependentEffectsEvent();
}

void __cdecl R_DrawEffects(FxCmd *cmd)
{
    FxGenerateVertsCmd genVertsCmd; // [esp+10h] [ebp-58h] BYREF

    PROF_SCOPED("R_DrawEffects");

    {
        PROF_SCOPED("fx surfs");
        FX_AddNonSpriteDrawSurfs(cmd);
    }
    
    FX_FillGenerateVertsCmd(cmd->localClientNum, &genVertsCmd);
    genVertsCmd.useReflection = cmd->useReflection;
    genVertsCmd.reflectionHeight = cmd->reflectionHeight;
    genVertsCmd.genVertsCameraType = cmd->updateCameraType;
    Sys_AddWorkerCmdInternal(&fx_drawWorkerCmd, (unsigned __int8 *)&genVertsCmd, 0);
}

void __cdecl R_ProcessCmd_UpdateFxRemaining(FxCmd *cmd)
{
    PROF_SCOPED("R_ProcessCmd_UpdateFxRemaining");
    if ( !Sys_QueryD3DDeviceOKEvent() )
    {
        return;
    }
    CG_AddSceneTracerBeams(cmd->localClientNum);
    CG_GenerateSceneVerts(cmd->localClientNum);

    {
        PROF_SCOPED("wait for fx_update");
        if (sys_smp_allowed->current.enabled)
            Sys_WaitUpdateNonDependentEffectsCompleted();
    }
    {
        PROF_SCOPED("wait for fx_add_mark");
        Sys_AssistAndWaitWorkerCmdInternal(&fx_add_markWorkerCmd);
    }
    
    FX_EndUpdate(cmd->localClientNum);
    R_DrawEffects(cmd);
}

void __cdecl R_UpdateSpotLightEffect(FxCmd *cmd)
{
    PROF_SCOPED("R_UpdateSpotLightEffect");
    Sys_ResetUpdateSpotLightEffectEvent();
    Sys_AddWorkerCmdInternal(&fx_update_spotWorkerCmd, (unsigned __int8 *)cmd, 0);
}

void __cdecl R_UpdateNonDependentEffects(FxCmd *cmd)
{
    PROF_SCOPED("R_UpdateNonDependentEffects");
    Sys_ResetUpdateNonDependentEffectsEvent();
    Sys_AddWorkerCmdInternal(&fx_updateWorkerCmd, (unsigned __int8 *)cmd, 0);
}

void __cdecl R_UpdateRemainingEffects(FxCmd *cmd)
{
    PROF_SCOPED("R_UpdateRemainingEffects");
    Sys_AddWorkerCmdInternal(&fx_update_remainingWorkerCmd, (unsigned __int8 *)cmd, 0);
    Sys_AddWorkerCmdInternal(&fx_update_remaining_ppuWorkerCmd, (unsigned __int8 *)cmd, 0);
}

void __cdecl R_UpdateXModelBoundsDelayed(GfxSceneEntity *sceneEnt)
{
    Sys_AddWorkerCmdInternal(&dobj_skelWorkerCmd, (unsigned __int8 *)&sceneEnt, 0);
}

void __cdecl R_SkinGfxEntityDelayed(GfxSceneEntity *sceneEnt)
{
    Sys_AddWorkerCmdInternal(&dobj_skinWorkerCmd, (unsigned __int8 *)&sceneEnt, 0);
}

