#include "cl_devgui.h"
#include <universal/dvar.h>
#include <universal/q_shared.h>
#include <qcommon/com_clients.h>
#include <qcommon/cmd.h>
#include <gfx_d3d/r_devgui.h>
#include <cgame/cg_visionsets.h>
#include <universal/com_files.h>
#include <devgui/devgui.h>

const char *emptyEnumList[1];

struct $C9973E8CB98117584CC11D7AF6800567 // sizeof=0x248
{                                       // XREF: .data:clGuiGlob/r
    bool inited;                        // XREF: CL_CreateDevGui(void)+3/r
                                        // CL_CreateDevGui(void):loc_54F664/w ...
    // padding byte
    // padding byte
    // padding byte
    int mapDirCount;                    // XREF: CL_DevGuiFrame(int)+1B/r
                                        // CL_CreateMapMenuEntries:loc_54F824/w ...
    char mapDirs[8][64];                // XREF: CL_DevGuiFrame(int)+60/o
                                        // CL_AddMapDirSlider+10F/o
    const char **mapNames[8];           // XREF: CL_AddMapDirSlider+127/w
                                        // CL_DestroyDevGui(void)+70/r
    const dvar_s *mapEnumDvar[8];       // XREF: CL_DevGuiFrame(int)+2A/r
                                        // CL_RegisterDevGuiDvars+21/w ...
};

$C9973E8CB98117584CC11D7AF6800567 clGuiGlob;

void __cdecl CL_DevGuiFrame(int localClientNum)
{
    int ControllerIndex; // eax
    int dvarIndex; // [esp+0h] [ebp-14h]
    const char *name; // [esp+8h] [ebp-Ch]
    const dvar_s *dvar; // [esp+Ch] [ebp-8h]
    char *cmd; // [esp+10h] [ebp-4h]

    for ( dvarIndex = 0; dvarIndex != clGuiGlob.mapDirCount; ++dvarIndex )
    {
        dvar = clGuiGlob.mapEnumDvar[dvarIndex];
        if ( dvar->modified )
        {
            Dvar_ClearModified(dvar);
            name = Dvar_EnumToString(dvar);
            if ( clGuiGlob.mapDirs[dvarIndex][0] )
                cmd = va("devmap %s/%s", clGuiGlob.mapDirs[dvarIndex], name);
            else
                cmd = va("devmap %s", name);
            ControllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
            Cmd_ExecuteSingleCommand(localClientNum, ControllerIndex, cmd);
        }
    }
}

void __cdecl CL_CreateDevGui()
{
    if ( clGuiGlob.inited
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_devgui.cpp", 173, 0, "%s", "!clGuiGlob.inited") )
    {
        __debugbreak();
    }
    clGuiGlob.inited = 1;
    CL_RegisterDevGuiDvars();
    CL_CreateMapMenuEntries();
    R_CreateDevGui();
    Cbuf_InsertText(0, "exec devgui_main");
    Cbuf_InsertText(0, "exec devgui_maps_mp");
    Cbuf_InsertText(0, "exec devgui_mp");
    CG_InitVisionSetsMenu();
    Cbuf_InsertText(0, "exec devgui_anims");
    Cbuf_InsertText(0, "exec devgui_art");
}

void CL_RegisterDevGuiDvars()
{
    clGuiGlob.mapEnumDvar[0] = _Dvar_RegisterEnum("mapEnum0", emptyEnumList, 0, 0x840u, "");
    clGuiGlob.mapEnumDvar[1] = _Dvar_RegisterEnum("mapEnum1", emptyEnumList, 0, 0x840u, "");
    clGuiGlob.mapEnumDvar[2] = _Dvar_RegisterEnum("mapEnum2", emptyEnumList, 0, 0x840u, "");
    clGuiGlob.mapEnumDvar[3] = _Dvar_RegisterEnum("mapEnum3", emptyEnumList, 0, 0x840u, "");
    clGuiGlob.mapEnumDvar[4] = _Dvar_RegisterEnum("mapEnum4", emptyEnumList, 0, 0x840u, "");
    clGuiGlob.mapEnumDvar[5] = _Dvar_RegisterEnum("mapEnum5", emptyEnumList, 0, 0x840u, "");
    clGuiGlob.mapEnumDvar[6] = _Dvar_RegisterEnum("mapEnum6", emptyEnumList, 0, 0x840u, "");
    clGuiGlob.mapEnumDvar[7] = _Dvar_RegisterEnum("mapEnum7", emptyEnumList, 0, 0x840u, "");
}

void CL_CreateMapMenuEntries()
{
    if ( !clGuiGlob.inited
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_devgui.cpp", 148, 0, "%s", "clGuiGlob.inited") )
    {
        __debugbreak();
    }
    clGuiGlob.mapDirCount = 0;
    CL_CreateMapMenuEntriesForLocation(1, "main*");
    CL_CreateMapMenuEntriesForLocation(2, "dev*");
    CL_CreateMapMenuEntriesForLocation(4, "temp*");
}

void __cdecl CL_CreateMapMenuEntriesForLocation(int locationFlags, const char *locationName)
{
    int dirCount; // [esp+0h] [ebp-Ch] BYREF
    const char **dirList; // [esp+4h] [ebp-8h]
    int dirIndex; // [esp+8h] [ebp-4h]

    CL_AddMapDirSlider("", locationFlags, locationName);
    dirList = FS_ListFiles("maps", (char*)"/", FS_LIST_PURE_ONLY, &dirCount);
    for ( dirIndex = 0; dirIndex != dirCount; ++dirIndex )
        CL_AddMapDirSlider(dirList[dirIndex], locationFlags, locationName);
    FS_FreeFileList(dirList);
}

void __cdecl CL_AddMapDirSlider(const char *dir, int locationFlags, const char *locationName)
{
    char *devguiPath; // [esp+0h] [ebp-18h]
    char *fullDir; // [esp+4h] [ebp-14h]
    int fileIndex; // [esp+8h] [ebp-10h]
    const char **fileList; // [esp+Ch] [ebp-Ch]
    dvar_s *dvar; // [esp+10h] [ebp-8h]
    int fileCount; // [esp+14h] [ebp-4h] BYREF

    fullDir = va("%s%s", "maps/", dir);
    fileList = FS_ListFilesInLocation(fullDir, (char*)"d3dbsp", FS_LIST_PURE_ONLY, &fileCount, locationFlags);
    if ( fileCount < 0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_devgui.cpp", 83, 0, "%s", "fileCount >= 0") )
    {
        __debugbreak();
    }
    if ( fileCount )
    {
        if ( !fileList
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_devgui.cpp", 87, 0, "%s", "fileList") )
        {
            __debugbreak();
        }
        if ( clGuiGlob.mapDirCount >= 8
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_devgui.cpp",
                        88,
                        0,
                        "%s",
                        "clGuiGlob.mapDirCount < MAX_MAP_DIRS") )
        {
            __debugbreak();
        }
        for ( fileIndex = 0; fileIndex != fileCount; ++fileIndex )
            Com_StripExtension((char *)fileList[fileIndex], (char *)fileList[fileIndex]);
        I_strncpyz(clGuiGlob.mapDirs[clGuiGlob.mapDirCount], dir, 64);
        clGuiGlob.mapNames[clGuiGlob.mapDirCount] = fileList;
        dvar = (dvar_s *)clGuiGlob.mapEnumDvar[clGuiGlob.mapDirCount];
        Dvar_UpdateEnumDomain(dvar, fileList);
        Dvar_SetInt(dvar, 0);
        devguiPath = va("Main:1/Maps:3/%s//maps//%s", locationName, dir);
        DevGui_AddDvar(devguiPath, dvar);
        ++clGuiGlob.mapDirCount;
    }
}

void __cdecl CL_DestroyDevGui()
{
    int dvarIndex; // [esp+0h] [ebp-4h]

    if ( !clGuiGlob.inited
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_devgui.cpp", 220, 0, "%s", "clGuiGlob.inited") )
    {
        __debugbreak();
    }
    clGuiGlob.inited = 0;
    DevGui_RemoveMenu("Main:1/Maps:3");
    CL_UnregisterDevGuiDvars();
    for ( dvarIndex = 0; dvarIndex != clGuiGlob.mapDirCount; ++dvarIndex )
        FS_FreeFileList(clGuiGlob.mapNames[dvarIndex]);
}

int CL_UnregisterDevGuiDvars()
{
    int result; // eax
    int dvarIndex; // [esp+0h] [ebp-4h]

    for ( dvarIndex = 0; dvarIndex < 8; ++dvarIndex )
    {
        Dvar_UpdateEnumDomain((dvar_s *)clGuiGlob.mapEnumDvar[dvarIndex], emptyEnumList);
        result = dvarIndex + 1;
    }
    return result;
}

