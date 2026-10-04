#include "r_cmds.h"
#include <qcommon/cmd.h>
#include <qcommon/threads.h>
#include "r_material.h"
#include "r_screenshot.h"
#include "r_sky.h"
#include "r_model.h"
#include "rb_stats.h"
#include "r_staticmodelcache.h"

cmd_function_s R_Cmd_Screenshot_VAR;
cmd_function_s R_Cmd_ScreenshotJpeg_VAR;
cmd_function_s R_ImageList_f_VAR;
cmd_function_s R_Cmd_ApplyPicmip_VAR;
cmd_function_s R_Cmd_ReloadMaterialTextures_VAR;
cmd_function_s R_Cmd_LoadSun_VAR;
cmd_function_s R_Cmd_SaveSun_VAR;
cmd_function_s R_MaterialList_f_VAR;
cmd_function_s R_MaterialEdit_f_VAR;
cmd_function_s R_MaterialParameterEdit_f_VAR;
cmd_function_s R_MaterialParameterTweak_f_VAR;
cmd_function_s R_ModelList_f_VAR;
cmd_function_s RB_Stats_f_VAR;
cmd_function_s R_StaticModelCacheStats_f_VAR;
cmd_function_s R_StaticModelCacheFlush_f_VAR;

void __cdecl R_RegisterCmds()
{
    Cmd_AddCommandInternal("screenshot", R_Cmd_Screenshot, &R_Cmd_Screenshot_VAR);
    Cmd_AddCommandInternal("screenshotJpeg", R_Cmd_ScreenshotJpeg, &R_Cmd_ScreenshotJpeg_VAR);
    Cmd_AddCommandInternal("imagelist", BLOPS_NULLSUB, &R_ImageList_f_VAR);
    Cmd_AddCommandInternal("r_applyPicmip", R_Cmd_ApplyPicmip, &R_Cmd_ApplyPicmip_VAR);
    Cmd_AddCommandInternal("reloadmaterialtextures", R_Cmd_ReloadMaterialTextures, &R_Cmd_ReloadMaterialTextures_VAR);
    Cmd_AddCommandInternal("r_loadsun", R_Cmd_LoadSun, &R_Cmd_LoadSun_VAR);
    Cmd_AddCommandInternal("r_savesun", R_Cmd_SaveSun, &R_Cmd_SaveSun_VAR);
    Cmd_AddCommandInternal("gfx_world", R_MaterialList_f, &R_MaterialList_f_VAR);
    Cmd_AddCommandInternal("material", R_MaterialEdit_f, &R_MaterialEdit_f_VAR);
    Cmd_AddCommandInternal("parameter", R_MaterialParameterEdit_f, &R_MaterialParameterEdit_f_VAR);
    Cmd_AddCommandInternal("tweak_parameter", R_MaterialParameterTweak_f, &R_MaterialParameterTweak_f_VAR);
    Cmd_AddCommandInternal("gfx_model", R_ModelList_f, &R_ModelList_f_VAR);
    Cmd_AddCommandInternal("gfx_stats", RB_Stats_f, &RB_Stats_f_VAR);
    Cmd_AddCommandInternal("r_smc_stats", R_StaticModelCacheStats_f, &R_StaticModelCacheStats_f_VAR);
    Cmd_AddCommandInternal("r_smc_flush", R_StaticModelCacheFlush_f, &R_StaticModelCacheFlush_f_VAR);
}

void __cdecl R_Cmd_ApplyPicmip()
{
    Sys_WaitD3DDeviceOKEvent();
    Material_UpdatePicmipAll();
}

void __cdecl R_Cmd_Screenshot()
{
    R_ScreenshotCommand(R_SCREENSHOT_TGA);
}

void __cdecl R_Cmd_ScreenshotJpeg()
{
    R_ScreenshotCommand(R_SCREENSHOT_JPG);
}

void __cdecl R_UnregisterCmds()
{
    Cmd_RemoveCommand("r_applyPicmip");
    Cmd_RemoveCommand("screenshot");
    Cmd_RemoveCommand("screenshotJpeg");
    Cmd_RemoveCommand("imagelist");
    Cmd_RemoveCommand("reloadmaterialtextures");
    Cmd_RemoveCommand("r_loadsun");
    Cmd_RemoveCommand("r_savesun");
    Cmd_RemoveCommand("gfx_world");
    Cmd_RemoveCommand("material");
    Cmd_RemoveCommand("parameter");
    Cmd_RemoveCommand("tweak_parameter");
    Cmd_RemoveCommand("gfx_model");
    Cmd_RemoveCommand("gfx_stats");
    Cmd_RemoveCommand("r_smc_stats");
    Cmd_RemoveCommand("r_smc_flush");
}

