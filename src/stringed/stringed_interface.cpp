#include "stringed_interface.h"
#include <universal/com_files.h>
#include <universal/assertive.h>

int giFilesFound;

unsigned __int8 *__cdecl SE_LoadFileData(const char *psFileName)
{
    int len; // [esp+0h] [ebp-8h]
    unsigned __int8 *pvLoadedData; // [esp+4h] [ebp-4h] BYREF

    len = FS_ReadFile(psFileName, (void **)&pvLoadedData);
    return len <= 0 ? 0 : pvLoadedData;
}

void __cdecl SE_FreeFileDataAfterLoad(unsigned __int8 *psLoadedFile)
{
    if ( !psLoadedFile
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\stringed\\stringed_interface.cpp",
                    33,
                    0,
                    "%s",
                    "psLoadedFile") )
    {
        __debugbreak();
    }
    FS_FreeFile(psLoadedFile);
}

int __cdecl SE_BuildFileList(
                const char *psStartDir,
                std::string *strResults)
{
    giFilesFound = 0;
    //std::basic_string<char,std::char_traits<char>,Allocator<char,LocalizeStringName>>::assign(
    //    strResults,
    //    "",
    //    "" + strlen("") + 1 - algn_C60BFA);
    strResults->assign("");
    SE_R_ListFiles((char*)"str", psStartDir, strResults);
    return giFilesFound;
}

void __cdecl SE_R_ListFiles(
                char *psExtension,
                const char *psDir,
                std::string *strResults)
{
    char sFilename[64]; // [esp+54h] [ebp-98h] BYREF
    char sDirName[64]; // [esp+94h] [ebp-58h] BYREF
    int numdirs; // [esp+D8h] [ebp-14h] BYREF
    int numSysFiles; // [esp+DCh] [ebp-10h] BYREF
    int i; // [esp+E0h] [ebp-Ch]
    const char **sysFiles; // [esp+E4h] [ebp-8h]
    const char **dirFiles; // [esp+E8h] [ebp-4h]

    dirFiles = FS_ListFiles(psDir, (char*)"/", FS_LIST_PURE_ONLY, &numdirs);
    for ( i = 0; i < numdirs; ++i )
    {
        if ( *dirFiles[i] )
        {
            if ( *dirFiles[i] != 46 )
            {
                sprintf(sDirName, "%s/%s", psDir, dirFiles[i]);
                SE_R_ListFiles(psExtension, sDirName, strResults);
            }
        }
    }
    sysFiles = FS_ListFiles(psDir, psExtension, FS_LIST_PURE_ONLY, &numSysFiles);
    for ( i = 0; i < numSysFiles; ++i )
    {
        sprintf(sFilename, "%s/%s", psDir, sysFiles[i]);
        strResults->append(sFilename, &sFilename[strlen(sFilename) + 1] - &sFilename[1]);
        //std::basic_string<char,std::char_traits<char>,Allocator<char,LocalizeStringName>>::append(
        //    strResults,
        //    sFilename,
        //    &sFilename[strlen(sFilename) + 1] - &sFilename[1]);
        //std::basic_string<char,std::char_traits<char>,Allocator<char,LocalizeStringName>>::append(strResults, 1u, 59);
        strResults->append(1, ';');
        ++giFilesFound;
    }
    FS_FreeFileList(sysFiles);
    FS_FreeFileList(dirFiles);
}

