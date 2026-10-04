#ifdef BO1_DW

#include "dwStorage.h"
#include <win32/win_shared.h>
#include <qcommon/common.h>
#include <ui/ui_main.h>
#include "dwUtils.h"
#include "dwUtils_pc.h"

//dwFileTask::dwFileTask()
//{
//    this->m_filename = 0;
//    //bdFileData::bdFileData(&this->m_fileData, 0, 0);
//    this->m_error = BD_NO_ERROR;
//    this->m_fileID = 0;
//    this->m_fileSize = 0;
//    this->m_bufferSize = 0;
//    //bdFileInfo::bdFileInfo(&this->m_fileInfo);
//    this->m_buffer = 0;
//    this->m_optional = 0;
//}

void __cdecl dwReadFileFailure(TaskRecord *task)
{
#ifdef BO1_DEMON
    unsigned int v1; // eax
    unsigned int v2; // eax
    char *v3; // eax
    dwFileTask *fileTask; // [esp+Ch] [ebp-4Ch]
    char errorStr[64]; // [esp+10h] [ebp-48h] BYREF
    bdLobbyErrorCode errorCode; // [esp+54h] [ebp-4h]

    if ( (!task || !task->payload || !task->remoteTask.m_ptr)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\Dw\\dwStorage.cpp",
                    20,
                    0,
                    "%s",
                    "task && task->payload && task->remoteTask") )
    {
        __debugbreak();
    }
    if ( task && task->payload && task->remoteTask.m_ptr )
    {
        fileTask = (dwFileTask *)task->payload;
        errorCode = bdRemoteTask::getErrorCode(task->remoteTask.m_ptr);
        fileTask->m_error = errorCode;
        dwLobbyErrorCodeToString(errorCode, errorStr, 0x40u);
        v1 = Sys_Milliseconds();
        Com_Printf(
            16,
            "Failed to fetch DW file '%s': err %i '%s'. (%dms)\n",
            fileTask->m_filename,
            errorCode,
            errorStr,
            v1 - task->startMS);
        if ( !fileTask->m_optional )
        {
            v2 = Sys_Milliseconds();
            v3 = va(
                         "DEBUG POPUP: Failed to fetch DW file '%s': err %i '%s'. (%dms)\n",
                         fileTask->m_filename,
                         errorCode,
                         errorStr,
                         v2 - task->startMS);
            Com_SetErrorMessage(v3);
            UI_OpenMenu(0, "error_dw_fetch_file");
        }
    }
#endif
}

void __cdecl dwReadFileSuccess(TaskRecord *task)
{
    dwFileTask *fileTask; // [esp+0h] [ebp-8h]
    bool foundFile; // [esp+7h] [ebp-1h]

    foundFile = 0;
    if ( (!task || !task->payload)
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\Dw\\dwStorage.cpp", 45, 0, "%s", "task && task->payload") )
    {
        __debugbreak();
    }
    if ( task && task->payload )
    {
        fileTask = (dwFileTask *)task->payload;
        if ( fileTask->m_fileData.m_fileSize )
            foundFile = 1;
        fileTask->m_fileSize = fileTask->m_fileData.m_fileSize;
    }
    if ( !foundFile )
        dwReadFileFailure(task);
}

TaskRecord *__cdecl dwReadFile(int localControllerIndex, dwFileTask *const fileTask)
{
    TaskRecord *retval; // [esp+0h] [ebp-4h]

    retval = 0;
    if ( fileTask )
        return dwReadFileInternal(localControllerIndex, fileTask, 0);
    return retval;
}

TaskRecord *__cdecl dwReadFileInternal(int localControllerIndex, dwFileTask *const fileTask, unsigned __int64 ownerUID)
{
#ifdef BO1_DEMON
    bdReference<bdCommonAddr> v4; // [esp+30h] [ebp-14h] BYREF
    bdReference<bdCommonAddr> v5; // [esp+34h] [ebp-10h] BYREF
    bdRemoteTask *remoteTask; // [esp+38h] [ebp-Ch]
    bdStorage *storage; // [esp+3Ch] [ebp-8h]
    TaskRecord *task; // [esp+40h] [ebp-4h]

    storage = dwGetStorage(localControllerIndex);
    if ( !fileTask->m_buffer
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\Dw\\dwStorage.cpp", 67, 0, "%s", "fileTask->m_buffer") )
    {
        __debugbreak();
    }
    if ( !fileTask->m_bufferSize
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\Dw\\dwStorage.cpp", 68, 0, "%s", "fileTask->m_bufferSize") )
    {
        __debugbreak();
    }
    task = 0;
    if ( storage && fileTask && fileTask->m_buffer && fileTask->m_bufferSize )
    {
        remoteTask = 0;
        fileTask->m_fileData.m_fileData = fileTask->m_buffer;
        fileTask->m_fileData.m_fileSize = fileTask->m_bufferSize;
        if ( ownerUID )
        {
            remoteTask = *(bdRemoteTask **)bdStorage::getFile(
                                                                             storage,
                                                                             (int)&v5,
                                                                             (unsigned __int8 *)fileTask->m_filename,
                                                                             &fileTask->m_fileData,
                                                                             ownerUID);
            bdReference<bdRemoteTask>::~bdReference<bdRemoteTask>(&v5);
        }
        else
        {
            remoteTask = *(bdRemoteTask **)bdStorage::getPublisherFile(
                                                                             storage,
                                                                             (int)&v4,
                                                                             (unsigned __int8 *)fileTask->m_filename,
                                                                             &fileTask->m_fileData);
            bdReference<bdRemoteTask>::~bdReference<bdRemoteTask>(&v4);
        }
        if ( remoteTask )
        {
            task = TaskManager2_CreateTask(task_ReadFileByID, localControllerIndex, 0, 0);
            bdReference<bdCommonAddr>::operator=(&task->remoteTask, remoteTask);
            TaskManager2_StartTask(task);
            task->payload = fileTask;
        }
        else
        {
            dwConnectionErrorHandler(15);
        }
    }
    return task;
#else 
    return NULL;
#endif
}

TaskRecord *__cdecl dwReadUserFile(int localControllerIndex, dwFileTask *const fileTask)
{
    unsigned __int64 userID; // [esp+0h] [ebp-10h] BYREF
    TaskRecord *retval; // [esp+Ch] [ebp-4h]

    retval = 0;
    if ( fileTask && dwGetOnlineUserID(localControllerIndex, &userID) )
        return dwReadFileInternal(localControllerIndex, fileTask, userID);
    return retval;
}

TaskRecord *__cdecl dwReadUserFileByUserID(int localControllerIndex, dwFileTask *const fileTask, unsigned __int64 xuid)
{
    TaskRecord *retval; // [esp+0h] [ebp-4h]

    retval = 0;
    if ( fileTask )
        return dwReadFileInternal(localControllerIndex, fileTask, xuid);
    return retval;
}

void __cdecl dwDeleteFileFailure(TaskRecord *task)
{
#ifdef BO1_DEMON
    char errorStr[64]; // [esp+8h] [ebp-48h] BYREF
    bdLobbyErrorCode errorCode; // [esp+4Ch] [ebp-4h]

    if ( task->remoteTask.m_ptr )
    {
        errorCode = bdRemoteTask::getErrorCode(task->remoteTask.m_ptr);
        dwLobbyErrorCodeToString(errorCode, errorStr, 0x40u);
        Com_PrintError(23, "DW delete file error %i - '%s'\n", errorCode, errorStr);
    }
    else
    {
        Com_PrintWarning(32, "failure with no remote task\n");
    }
#endif
}

TaskRecord *__cdecl dwDeleteFile(int localControllerIndex, char *filename)
{
#ifdef BO1_DEMON
    const bdReference<bdCommonAddr> *v2; // eax
    bdReference<bdCommonAddr> v4; // [esp+1Ch] [ebp-Ch] BYREF
    bdStorage *storage; // [esp+20h] [ebp-8h]
    TaskRecord *task; // [esp+24h] [ebp-4h]

    if ( !filename && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\Dw\\dwStorage.cpp", 170, 0, "%s", "filename") )
        __debugbreak();
    storage = dwGetStorage(localControllerIndex);
    task = 0;
    if ( storage )
    {
        Com_Printf(16, "Starting deleting file: %s.\n", filename);
        task = TaskManager2_CreateTask(task_DeleteFile, localControllerIndex, 0, 0);
        v2 = (const bdReference<bdCommonAddr> *)bdStorage::removeFile(storage, (int)&v4, (unsigned __int8 *)filename, 0);
        bdReference<bdCommonAddr>::operator=((bdReference<bdCommonAddr> *)&task->remoteTask, v2);
        bdReference<bdRemoteTask>::~bdReference<bdRemoteTask>(&v4);
        TaskManager2_StartTask(task);
    }
    return task;
#else
    return NULL;
#endif
}

void __cdecl dwWriteFileFailure(TaskRecord *task)
{
#ifdef BO1_DEMON
    char errorStr[64]; // [esp+8h] [ebp-48h] BYREF
    bdLobbyErrorCode errorCode; // [esp+4Ch] [ebp-4h]

    if ( task->remoteTask.m_ptr )
    {
        errorCode = bdRemoteTask::getErrorCode(task->remoteTask.m_ptr);
        dwLobbyErrorCodeToString(errorCode, errorStr, 0x40u);
        Com_PrintError(23, "DW upload file error %i - '%s'\n", errorCode, errorStr);
    }
    else
    {
        Com_PrintWarning(32, "failure with no remote task\n");
    }
#endif
}

TaskRecord *__cdecl dwWriteFile(
                int localControllerIndex,
                char *filename,
                unsigned __int8 *data,
                unsigned int dataSize,
                bdFileInfo *fileInfo,
                unsigned __int64 ownerID)
{
#ifdef BO1_DEMON
    const bdReference<bdCommonAddr> *v6; // eax
    bdReference<bdCommonAddr> v8; // [esp+1Ch] [ebp-Ch] BYREF
    bdStorage *storage; // [esp+20h] [ebp-8h]
    TaskRecord *task; // [esp+24h] [ebp-4h]

    if ( !fileInfo && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\Dw\\dwStorage.cpp", 213, 0, "%s", "fileInfo") )
        __debugbreak();
    storage = dwGetStorage(localControllerIndex);
    task = 0;
    if ( storage && fileInfo )
    {
        Com_Printf(16, "Starting upload of file: %s.\n", filename);
        task = TaskManager2_CreateTask(task_UploadFile, localControllerIndex, 0, 0);
        v6 = (const bdReference<bdCommonAddr> *)bdStorage::uploadFile(
                                                                                            storage,
                                                                                            (int)&v8,
                                                                                            (unsigned __int8 *)filename,
                                                                                            data,
                                                                                            dataSize,
                                                                                            0,
                                                                                            ownerID,
                                                                                            fileInfo);
        bdReference<bdCommonAddr>::operator=((bdReference<bdCommonAddr> *)&task->remoteTask, v6);
        bdReference<bdRemoteTask>::~bdReference<bdRemoteTask>(&v8);
        TaskManager2_StartTask(task);
    }
    return task;
#else
    return NULL;
#endif
}

TaskRecord *__cdecl dwFileShareGetListing(int localControllerIndex, dwFileShareListTask *const fsTask)
{
#ifdef BO1_DEMON
    bdReference<bdCommonAddr> v3; // [esp+20h] [ebp-10h] BYREF
    bdRemoteTask *remoteTask; // [esp+24h] [ebp-Ch]
    TaskRecord *task; // [esp+28h] [ebp-8h]
    bdContentStreaming *contentStreaming; // [esp+2Ch] [ebp-4h]

    if ( !fsTask && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\Dw\\dwStorage.cpp", 252, 0, "%s", "fsTask") )
        __debugbreak();
    contentStreaming = dwGetContentStreaming(localControllerIndex);
    task = 0;
    if ( contentStreaming )
    {
        if ( fsTask )
        {
            remoteTask = 0;
            remoteTask = *(bdRemoteTask **)bdContentStreaming::listFilesByOwner(
                                                                             contentStreaming,
                                                                             (int)&v3,
                                                                             fsTask->ownerID,
                                                                             fsTask->startDate,
                                                                             fsTask->category,
                                                                             fsTask->descriptors,
                                                                             fsTask->maxNumResults,
                                                                             fsTask->offset,
                                                                             (unsigned __int8 *)fsTask->fileName);
            bdReference<bdRemoteTask>::~bdReference<bdRemoteTask>(&v3);
            if ( remoteTask )
            {
                task = TaskManager2_CreateTask(task_ReadFileShare, localControllerIndex, 0, 0);
                bdReference<bdCommonAddr>::operator=(&task->remoteTask, remoteTask);
                TaskManager2_StartTask(task);
                task->payload = fsTask;
            }
        }
    }
    return task;
#else
    return NULL;
#endif
}

TaskRecord *__cdecl dwFileShareGetRating(int localControllerIndex, dwFileShareGetRatingTask *const fsTask)
{
#ifdef BO1_DEMON
    bdReference<bdCommonAddr> v3; // [esp+20h] [ebp-14h] BYREF
    bdRemoteTask *remoteTask; // [esp+24h] [ebp-10h]
    TaskRecord *task; // [esp+28h] [ebp-Ch]
    int leaderboardID; // [esp+2Ch] [ebp-8h]
    bdStats *stats; // [esp+30h] [ebp-4h]

    if ( !fsTask && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\Dw\\dwStorage.cpp", 276, 0, "%s", "fsTask") )
        __debugbreak();
    stats = dwGetStats(localControllerIndex);
    task = 0;
    if ( stats )
    {
        if ( fsTask )
        {
            remoteTask = 0;
            leaderboardID = fsTask->leaderboardID - 294967296;
            remoteTask = *(bdRemoteTask **)bdStats::readStatsByEntityID(
                                                                             stats,
                                                                             (int)&v3,
                                                                             leaderboardID,
                                                                             (int)fsTask->fileIDs,
                                                                             fsTask->maxFilesToFetch,
                                                                             fsTask->outStatsInfo);
            bdReference<bdRemoteTask>::~bdReference<bdRemoteTask>(&v3);
            if ( remoteTask )
            {
                task = TaskManager2_CreateTask(task_GetFileRating, localControllerIndex, 0, 0);
                bdReference<bdCommonAddr>::operator=(&task->remoteTask, remoteTask);
                TaskManager2_StartTask(task);
                task->payload = fsTask;
            }
        }
    }
    return task;
#else
    return NULL;
#endif
}

TaskRecord *__cdecl dwFileShareGetTopRated(int localControllerIndex, dwFileShareGetTopRatedTask *const fsTask)
{
#ifdef BO1_DEMON
    bdReference<bdCommonAddr> v3; // [esp+20h] [ebp-10h] BYREF
    bdRemoteTask *remoteTask; // [esp+24h] [ebp-Ch]
    TaskRecord *task; // [esp+28h] [ebp-8h]
    bdStats *stats; // [esp+2Ch] [ebp-4h]

    if ( !fsTask && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\Dw\\dwStorage.cpp", 302, 0, "%s", "fsTask") )
        __debugbreak();
    stats = dwGetStats(localControllerIndex);
    task = 0;
    if ( stats )
    {
        if ( fsTask )
        {
            remoteTask = 0;
            remoteTask = *(bdRemoteTask **)bdStats::readStatsByRank(
                                                                             stats,
                                                                             (int)&v3,
                                                                             fsTask->leaderboardID,
                                                                             fsTask->startRank + 1,
                                                                             &fsTask->statsInfo[0],
                                                                             fsTask->maxFiles);
            bdReference<bdRemoteTask>::~bdReference<bdRemoteTask>(&v3);
            if ( remoteTask )
            {
                task = TaskManager2_CreateTask(task_GetTopRated, localControllerIndex, 0, 0);
                bdReference<bdCommonAddr>::operator=(&task->remoteTask, remoteTask);
                TaskManager2_StartTask(task);
                task->payload = fsTask;
            }
        }
    }
    return task;
#else
    return NULL;
#endif
}

char __cdecl dwFileShareLocationToCollectionID(fileShareLocation location, unsigned int *collectionID)
{
    if ( location == FILESHARE_LOCATION_USERSTORAGE )
    {
        *collectionID = 1;
        return 1;
    }
    else if ( location == FILESHARE_LOCATION_POOLEDSTORAGE )
    {
        *collectionID = 2;
        return 1;
    }
    else
    {
        if ( !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\Dw\\dwStorage.cpp",
                        337,
                        0,
                        "Invalid file share location") )
            __debugbreak();
        return 0;
    }
}

TaskRecord *__cdecl dwFileShareSearch(int localControllerIndex, dwFileShareSearchTask *const fsTask)
{
#ifdef BO1_DEMON
    bdReference<bdCommonAddr> v3; // [esp+20h] [ebp-14h] BYREF
    bdRemoteTask *remoteTask; // [esp+24h] [ebp-10h]
    unsigned int collectionID; // [esp+28h] [ebp-Ch] BYREF
    TaskRecord *task; // [esp+2Ch] [ebp-8h]
    bdTags *tagService; // [esp+30h] [ebp-4h]

    if ( !fsTask && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\Dw\\dwStorage.cpp", 357, 0, "%s", "fsTask") )
        __debugbreak();
    tagService = dwGetTagService(localControllerIndex);
    task = 0;
    if ( tagService && fsTask )
    {
        remoteTask = 0;
        if ( dwFileShareLocationToCollectionID(fsTask->location, &collectionID) )
        {
            remoteTask = *(bdRemoteTask **)bdTags::searchByTags(
                                                                             tagService,
                                                                             (int)&v3,
                                                                             collectionID,
                                                                             fsTask->startIndex,
                                                                             fsTask->maxNumResults,
                                                                             fsTask->orderNewestFirst,
                                                                             fsTask->numTags,
                                                                             (int)fsTask->tags,
                                                                             fsTask->resultFileIDs);
            bdReference<bdRemoteTask>::~bdReference<bdRemoteTask>(&v3);
        }
        if ( remoteTask )
        {
            task = TaskManager2_CreateTask(task_SearchFileShareByTag, localControllerIndex, 0, 0);
            bdReference<bdCommonAddr>::operator=(&task->remoteTask, remoteTask);
            TaskManager2_StartTask(task);
            task->payload = fsTask;
        }
    }
    return task;
#else
    return NULL;
#endif
}

TaskRecord *__cdecl dwFileShareGetDescriptors(int localControllerIndex, dwFileShareDescriptorsTask *const fsTask)
{
    if ( !fsTask && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\Dw\\dwStorage.cpp", 449, 0, "%s", "fsTask") )
        __debugbreak();
    if ( fsTask )
    {
        if ( fsTask->location == FILESHARE_LOCATION_USERSTORAGE )
            return dwFileShareGetDescriptorsFromUserStorage(localControllerIndex, fsTask);
        if ( fsTask->location == FILESHARE_LOCATION_POOLEDSTORAGE )
            return dwFileShareGetDescriptorsFromPooledStorage(localControllerIndex, fsTask);
        if ( !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\Dw\\dwStorage.cpp",
                        462,
                        0,
                        "Invalid file share location") )
            __debugbreak();
    }
    return 0;
}

TaskRecord *__cdecl dwFileShareGetDescriptorsFromPooledStorage(
                int localControllerIndex,
                dwFileShareDescriptorsTask *const fsTask)
{
#ifdef BO1_DEMON
    unsigned int v2; // eax
    bdFileID *v3; // edx
    unsigned int v4; // ecx
    bdReference<bdCommonAddr> v6; // [esp+24h] [ebp-B8h] BYREF
    unsigned int i; // [esp+28h] [ebp-B4h]
    bdRemoteTask *remoteTask; // [esp+2Ch] [ebp-B0h]
    TaskRecord *task; // [esp+30h] [ebp-ACh]
    unsigned __int64 fileIDs[20]; // [esp+34h] [ebp-A8h] BYREF
    bdPooledStorage *pooledStorage; // [esp+D8h] [ebp-4h]

    if ( !fsTask && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\Dw\\dwStorage.cpp", 389, 0, "%s", "fsTask") )
        __debugbreak();
    pooledStorage = dwGetPooledStorage(localControllerIndex);
    task = 0;
    if ( pooledStorage )
    {
        if ( fsTask )
        {
            remoteTask = 0;
            for ( i = 0; i < fsTask->numFiles; ++i )
            {
                v2 = i;
                v3 = fsTask->fileIDs;
                v4 = i;
                LODWORD(fileIDs[i]) = v3[i].m_fileID;
                HIDWORD(fileIDs[v4]) = HIDWORD(v3[v2].m_fileID);
            }
            remoteTask = *(bdRemoteTask **)bdPooledStorage::getPooledMetaDataByID(
                                                                             pooledStorage,
                                                                             (int)&v6,
                                                                             fsTask->numFiles,
                                                                             (int)fileIDs,
                                                                             fsTask->descriptors);
            bdReference<bdRemoteTask>::~bdReference<bdRemoteTask>(&v6);
            if ( remoteTask )
            {
                task = TaskManager2_CreateTask(task_SearchFileShareByFileID, localControllerIndex, 0, 0);
                bdReference<bdCommonAddr>::operator=(&task->remoteTask, remoteTask);
                TaskManager2_StartTask(task);
                task->payload = fsTask;
            }
        }
    }
    return task;
#else
    return NULL;
#endif
}

TaskRecord *__cdecl dwFileShareGetDescriptorsFromUserStorage(
                int localControllerIndex,
                dwFileShareDescriptorsTask *const fsTask)
{
#ifdef BO1_DEMON
    unsigned int v2; // eax
    bdFileID *v3; // edx
    unsigned int v4; // ecx
    bdReference<bdCommonAddr> v6; // [esp+24h] [ebp-B8h] BYREF
    unsigned int i; // [esp+28h] [ebp-B4h]
    bdRemoteTask *remoteTask; // [esp+2Ch] [ebp-B0h]
    TaskRecord *task; // [esp+30h] [ebp-ACh]
    unsigned __int64 fileIDs[20]; // [esp+34h] [ebp-A8h] BYREF
    bdContentStreaming *contentStreaming; // [esp+D8h] [ebp-4h]

    if ( !fsTask && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\Dw\\dwStorage.cpp", 420, 0, "%s", "fsTask") )
        __debugbreak();
    contentStreaming = dwGetContentStreaming(localControllerIndex);
    task = 0;
    if ( contentStreaming )
    {
        if ( fsTask )
        {
            remoteTask = 0;
            for ( i = 0; i < fsTask->numFiles; ++i )
            {
                v2 = i;
                v3 = fsTask->fileIDs;
                v4 = i;
                LODWORD(fileIDs[i]) = v3[i].m_fileID;
                HIDWORD(fileIDs[v4]) = HIDWORD(v3[v2].m_fileID);
            }
            remoteTask = *(bdRemoteTask **)bdContentStreaming::getFileMetaDataByID(
                                                                             contentStreaming,
                                                                             (int)&v6,
                                                                             fsTask->numFiles,
                                                                             (int)fileIDs,
                                                                             fsTask->descriptors);
            bdReference<bdRemoteTask>::~bdReference<bdRemoteTask>(&v6);
            if ( remoteTask )
            {
                task = TaskManager2_CreateTask(task_SearchFileShareByFileID, localControllerIndex, 0, 0);
                bdReference<bdCommonAddr>::operator=(&task->remoteTask, remoteTask);
                TaskManager2_StartTask(task);
                task->payload = fsTask;
            }
        }
    }
    return task;
#else
    return NULL;
#endif
}

TaskRecord *__cdecl dwFileShareReadFile(int controllerIndex, dwFileShareReadFileTask *const fsTask)
{
    if ( !fsTask && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\Dw\\dwStorage.cpp", 527, 0, "%s", "fsTask") )
        __debugbreak();
    if ( fsTask )
    {
        if ( fsTask->location == FILESHARE_LOCATION_USERSTORAGE )
            return dwFileShareReadUserStorageFile(controllerIndex, fsTask);
        if ( fsTask->location == FILESHARE_LOCATION_POOLEDSTORAGE )
            return dwFileShareReadPooledStorageFile(controllerIndex, fsTask);
        if ( !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\Dw\\dwStorage.cpp",
                        540,
                        0,
                        "Invalid file share location") )
            __debugbreak();
    }
    return 0;
}

TaskRecord *__cdecl dwFileShareReadPooledStorageFile(int controllerIndex, dwFileShareReadFileTask *const fsTask)
{
#ifdef BO1_DEMON
    bdReference<bdCommonAddr> v3; // [esp+20h] [ebp-10h] BYREF
    bdRemoteTask *remoteTask; // [esp+24h] [ebp-Ch]
    TaskRecord *task; // [esp+28h] [ebp-8h]
    bdPooledStorage *pooledStorage; // [esp+2Ch] [ebp-4h]

    pooledStorage = dwGetPooledStorage(controllerIndex);
    task = 0;
    if ( pooledStorage )
    {
        remoteTask = 0;
        if ( fsTask->downloadHandler )
        {
            remoteTask = *(bdRemoteTask **)bdPooledStorage::download(
                                                                             (int)pooledStorage,
                                                                             (int)&v3,
                                                                             fsTask->fileID,
                                                                             HIDWORD(fsTask->fileID),
                                                                             fsTask->downloadHandler,
                                                                             &fsTask->descriptor,
                                                                             0,
                                                                             0);
            bdReference<bdRemoteTask>::~bdReference<bdRemoteTask>(&v3);
        }
        if ( remoteTask )
        {
            task = TaskManager2_CreateTask(task_ReadContentServerFile, controllerIndex, 0, 0);
            bdReference<bdCommonAddr>::operator=(&task->remoteTask, remoteTask);
            TaskManager2_StartTask(task);
            task->payload = fsTask;
        }
    }
    return task;
#else
    return NULL;
#endif
}

TaskRecord *__cdecl dwFileShareReadUserStorageFile(int controllerIndex, dwFileShareReadFileTask *const fsTask)
{
#ifdef BO1_DEMON
    bdReference<bdCommonAddr> v3; // [esp+30h] [ebp-14h] BYREF
    bdReference<bdCommonAddr> v4; // [esp+34h] [ebp-10h] BYREF
    bdRemoteTask *remoteTask; // [esp+38h] [ebp-Ch]
    TaskRecord *task; // [esp+3Ch] [ebp-8h]
    bdContentStreaming *contentStreaming; // [esp+40h] [ebp-4h]

    contentStreaming = dwGetContentStreaming(controllerIndex);
    task = 0;
    if ( contentStreaming )
    {
        remoteTask = 0;
        if ( fsTask->downloadHandler )
        {
            remoteTask = *(bdRemoteTask **)bdContentStreaming::download(
                                                                             (int)contentStreaming,
                                                                             (int)&v4,
                                                                             fsTask->fileID,
                                                                             fsTask->downloadHandler,
                                                                             &fsTask->descriptor,
                                                                             0,
                                                                             0);
            bdReference<bdRemoteTask>::~bdReference<bdRemoteTask>(&v4);
        }
        else
        {
            remoteTask = *(bdRemoteTask **)bdContentStreaming::download(
                                                                             (int)contentStreaming,
                                                                             (int)&v3,
                                                                             fsTask->fileID,
                                                                             fsTask->buffer,
                                                                             fsTask->bufferSize,
                                                                             &fsTask->descriptor,
                                                                             0,
                                                                             0);
            bdReference<bdRemoteTask>::~bdReference<bdRemoteTask>(&v3);
        }
        if ( remoteTask )
        {
            task = TaskManager2_CreateTask(task_ReadContentServerFile, controllerIndex, 0, 0);
            bdReference<bdCommonAddr>::operator=(&task->remoteTask, remoteTask);
            TaskManager2_StartTask(task);
            task->payload = fsTask;
        }
    }
    return task;
#else
    return NULL;
#endif
}

TaskRecord *__cdecl dwFileShareRemoveFile(int controllerIndex, dwFileShareRemoveTask *const fsTask)
{
#ifdef BO1_DEMON
    bdReference<bdCommonAddr> v3; // [esp+20h] [ebp-10h] BYREF
    bdRemoteTask *remoteTask; // [esp+24h] [ebp-Ch]
    TaskRecord *task; // [esp+28h] [ebp-8h]
    bdContentStreaming *contentStreaming; // [esp+2Ch] [ebp-4h]

    if ( !fsTask && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\Dw\\dwStorage.cpp", 549, 0, "%s", "fsTask") )
        __debugbreak();
    contentStreaming = dwGetContentStreaming(controllerIndex);
    task = 0;
    if ( contentStreaming )
    {
        if ( fsTask )
        {
            remoteTask = 0;
            remoteTask = *(bdRemoteTask **)bdContentStreaming::removeFile((int)contentStreaming, (int)&v3, fsTask->fileSlot);
            bdReference<bdRemoteTask>::~bdReference<bdRemoteTask>(&v3);
            if ( remoteTask )
            {
                task = TaskManager2_CreateTask(task_FileShareRemove, controllerIndex, 0, 0);
                bdReference<bdCommonAddr>::operator=(&task->remoteTask, remoteTask);
                TaskManager2_StartTask(task);
                task->payload = fsTask;
            }
        }
    }
    return task;
#else
    return NULL;
#endif
}

TaskRecord *__cdecl dwFileShareTransferFile(int controllerIndex, dwFileShareTransferTask *const fsTask)
{
#ifdef BO1_DEMON
    fileShareLocation location; // [esp+0h] [ebp-48h]
    bdReference<bdCommonAddr> v4; // [esp+34h] [ebp-14h] BYREF
    bdReference<bdCommonAddr> v5; // [esp+38h] [ebp-10h] BYREF
    bdRemoteTask *remoteTask; // [esp+3Ch] [ebp-Ch]
    TaskRecord *task; // [esp+40h] [ebp-8h]
    bdContentStreaming *contentStreaming; // [esp+44h] [ebp-4h]

    contentStreaming = dwGetContentStreaming(controllerIndex);
    task = 0;
    if ( contentStreaming && fsTask )
    {
        remoteTask = 0;
        location = fsTask->location;
        if ( location == FILESHARE_LOCATION_USERSTORAGE )
        {
            remoteTask = *(bdRemoteTask **)bdContentStreaming::copyFromUserStorage(
                                                                             (int)contentStreaming,
                                                                             (int)&v5,
                                                                             fsTask->fileID,
                                                                             fsTask->fileSlot,
                                                                             fsTask->fileName,
                                                                             fsTask->category,
                                                                             fsTask->metaData,
                                                                             fsTask->metaDataSize,
                                                                             fsTask->numTags,
                                                                             fsTask->tags,
                                                                             &fsTask->outFileID);
            bdReference<bdRemoteTask>::~bdReference<bdRemoteTask>(&v5);
        }
        else if ( location == FILESHARE_LOCATION_POOLEDSTORAGE )
        {
            remoteTask = *(bdRemoteTask **)bdContentStreaming::copyFromPooledStorage(
                                                                             (int)contentStreaming,
                                                                             (int)&v4,
                                                                             fsTask->fileID,
                                                                             fsTask->fileSlot,
                                                                             fsTask->fileName,
                                                                             fsTask->category,
                                                                             fsTask->metaData,
                                                                             fsTask->metaDataSize,
                                                                             fsTask->numTags,
                                                                             fsTask->tags,
                                                                             &fsTask->outFileID);
            bdReference<bdRemoteTask>::~bdReference<bdRemoteTask>(&v4);
        }
        if ( remoteTask )
        {
            task = TaskManager2_CreateTask(task_FileShareTransfer, controllerIndex, 0, 0);
            bdReference<bdCommonAddr>::operator=(&task->remoteTask, remoteTask);
            TaskManager2_StartTask(task);
            task->payload = fsTask;
        }
    }
    return task;
#else
    return NULL;
#endif
}

TaskRecord *__cdecl dwFileShareGetSummary(int controllerIndex, dwFileShareSummaryTask *const fsTask)
{
#ifdef BO1_DEMON
    bdReference<bdCommonAddr> v3; // [esp+20h] [ebp-10h] BYREF
    bdRemoteTask *remoteTask; // [esp+24h] [ebp-Ch]
    TaskRecord *task; // [esp+28h] [ebp-8h]
    bdPooledStorage *pooledStorage; // [esp+2Ch] [ebp-4h]

    pooledStorage = dwGetPooledStorage(controllerIndex);
    task = 0;
    if ( pooledStorage )
    {
        remoteTask = 0;
        remoteTask = *(bdRemoteTask **)bdPooledStorage::downloadSummary(
                                                                         (int)pooledStorage,
                                                                         (int)&v3,
                                                                         fsTask->fileID,
                                                                         HIDWORD(fsTask->fileID),
                                                                         fsTask->summaryData,
                                                                         fsTask->summaryDataSize,
                                                                         0,
                                                                         0);
        bdReference<bdRemoteTask>::~bdReference<bdRemoteTask>(&v3);
        if ( remoteTask )
        {
            task = TaskManager2_CreateTask(task_FileShareGetSummary, controllerIndex, 0, 0);
            bdReference<bdCommonAddr>::operator=(&task->remoteTask, remoteTask);
            TaskManager2_StartTask(task);
            task->payload = fsTask;
        }
    }
    return task;
#else
    return NULL;
#endif
}

TaskRecord *__cdecl dwFileShareSetPooledSummary(int controllerIndex, dwFileShareSummaryTask *const fsTask)
{
#ifdef BO1_DEMON
    bdReference<bdCommonAddr> v3; // [esp+20h] [ebp-10h] BYREF
    bdRemoteTask *remoteTask; // [esp+24h] [ebp-Ch]
    TaskRecord *task; // [esp+28h] [ebp-8h]
    bdPooledStorage *pooledStorage; // [esp+2Ch] [ebp-4h]

    pooledStorage = dwGetPooledStorage(controllerIndex);
    task = 0;
    if ( pooledStorage )
    {
        remoteTask = 0;
        remoteTask = *(bdRemoteTask **)bdPooledStorage::uploadSummaryMetaData(
                                                                         (int)pooledStorage,
                                                                         (int)&v3,
                                                                         fsTask->fileID,
                                                                         HIDWORD(fsTask->fileID),
                                                                         fsTask->summaryData,
                                                                         fsTask->summaryDataSize,
                                                                         (void *)fsTask->metaData,
                                                                         fsTask->metaDataSize,
                                                                         fsTask->numTags,
                                                                         fsTask->tags);
        bdReference<bdRemoteTask>::~bdReference<bdRemoteTask>(&v3);
        if ( remoteTask )
        {
            task = TaskManager2_CreateTask(task_FileShareSetSummary, controllerIndex, 0, 0);
            bdReference<bdCommonAddr>::operator=(&task->remoteTask, remoteTask);
            TaskManager2_StartTask(task);
            task->payload = fsTask;
        }
    }
    return task;
#else
    return NULL;
#endif
}

TaskRecord *__cdecl dwFileShareSetUserSummary(int controllerIndex, dwFileShareSummaryTask *const fsTask)
{
#ifdef BO1_DEMON
    bdReference<bdCommonAddr> v3; // [esp+20h] [ebp-10h] BYREF
    bdRemoteTask *remoteTask; // [esp+24h] [ebp-Ch]
    TaskRecord *task; // [esp+28h] [ebp-8h]
    bdContentStreaming *contentStreaming; // [esp+2Ch] [ebp-4h]

    contentStreaming = dwGetContentStreaming(controllerIndex);
    task = 0;
    if ( contentStreaming )
    {
        remoteTask = 0;
        remoteTask = *(bdRemoteTask **)bdContentStreaming::uploadUserSummaryMetaData(
                                                                         (int)contentStreaming,
                                                                         (int)&v3,
                                                                         fsTask->fileID,
                                                                         HIDWORD(fsTask->fileID),
                                                                         fsTask->summaryData,
                                                                         fsTask->summaryDataSize,
                                                                         (void *)fsTask->metaData,
                                                                         fsTask->metaDataSize,
                                                                         fsTask->numTags,
                                                                         fsTask->tags);
        bdReference<bdRemoteTask>::~bdReference<bdRemoteTask>(&v3);
        if ( remoteTask )
        {
            task = TaskManager2_CreateTask(task_FileShareSetSummary, controllerIndex, 0, 0);
            bdReference<bdCommonAddr>::operator=(&task->remoteTask, remoteTask);
            TaskManager2_StartTask(task);
            task->payload = fsTask;
        }
    }
    return task;
#else
    return NULL;
#endif
}

TaskRecord *__cdecl dwFileShareSetSummary(int controllerIndex, dwFileShareSummaryTask *const fsTask)
{
    fileShareLocation location; // [esp+0h] [ebp-4h]

    location = fsTask->location;
    if ( location == FILESHARE_LOCATION_USERSTORAGE )
        return dwFileShareSetUserSummary(controllerIndex, fsTask);
    if ( location == FILESHARE_LOCATION_POOLEDSTORAGE )
        return dwFileShareSetPooledSummary(controllerIndex, fsTask);
    return 0;
}

TaskRecord *__cdecl dwFileShareWriteFile(int controllerIndex, dwFileShareWriteFileTask *const fsTask)
{
    if ( !fsTask && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\Dw\\dwStorage.cpp", 751, 0, "%s", "fsTask") )
        __debugbreak();
    if ( fsTask )
    {
        if ( fsTask->location == FILESHARE_LOCATION_USERSTORAGE )
            return dwFileShareWriteUserStorageFile(controllerIndex, fsTask);
        if ( fsTask->location == FILESHARE_LOCATION_POOLEDSTORAGE )
            return dwFileShareWritePooledStorageFile(controllerIndex, fsTask);
        if ( !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\Dw\\dwStorage.cpp",
                        764,
                        0,
                        "Invalid file share location") )
            __debugbreak();
    }
    return 0;
}

TaskRecord *__cdecl dwFileShareWritePooledStorageFile(int controllerIndex, dwFileShareWriteFileTask *const fsTask)
{
#ifdef BO1_DEMON
    bdReference<bdCommonAddr> v3; // [esp+20h] [ebp-10h] BYREF
    bdRemoteTask *remoteTask; // [esp+24h] [ebp-Ch]
    TaskRecord *task; // [esp+28h] [ebp-8h]
    bdPooledStorage *pooledStorage; // [esp+2Ch] [ebp-4h]

    pooledStorage = dwGetPooledStorage(controllerIndex);
    task = 0;
    if ( pooledStorage )
    {
        remoteTask = 0;
        if ( !fsTask->uploadHandler
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\Dw\\dwStorage.cpp",
                        694,
                        0,
                        "%s",
                        "fsTask->uploadHandler") )
        {
            __debugbreak();
        }
        if ( fsTask->uploadHandler )
        {
            remoteTask = *(bdRemoteTask **)bdPooledStorage::upload(
                                                                             (int)pooledStorage,
                                                                             (int)&v3,
                                                                             fsTask->uploadHandler,
                                                                             fsTask->category,
                                                                             (char *)fsTask->fileName,
                                                                             fsTask->numTags,
                                                                             fsTask->tags,
                                                                             &fsTask->outFileID);
            bdReference<bdRemoteTask>::~bdReference<bdRemoteTask>(&v3);
        }
        if ( remoteTask )
        {
            task = TaskManager2_CreateTask(task_WriteContentServerFile, controllerIndex, 0, -1);
            bdReference<bdCommonAddr>::operator=(&task->remoteTask, remoteTask);
            TaskManager2_StartTask(task);
            task->payload = fsTask;
        }
    }
    return task;
#else
    return NULL;
#endif
}

TaskRecord *__cdecl dwFileShareWriteUserStorageFile(int controllerIndex, dwFileShareWriteFileTask *const fsTask)
{
#ifdef BO1_DEMON
    bdReference<bdCommonAddr> v3; // [esp+30h] [ebp-14h] BYREF
    bdReference<bdCommonAddr> v4; // [esp+34h] [ebp-10h] BYREF
    bdRemoteTask *remoteTask; // [esp+38h] [ebp-Ch]
    TaskRecord *task; // [esp+3Ch] [ebp-8h]
    bdContentStreaming *contentStreaming; // [esp+40h] [ebp-4h]

    if ( !fsTask && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\Dw\\dwStorage.cpp", 715, 0, "%s", "fsTask") )
        __debugbreak();
    contentStreaming = dwGetContentStreaming(controllerIndex);
    task = 0;
    if ( contentStreaming && fsTask )
    {
        remoteTask = 0;
        if ( fsTask->uploadHandler )
        {
            remoteTask = *(bdRemoteTask **)bdContentStreaming::upload(
                                                                             (int)contentStreaming,
                                                                             (int)&v4,
                                                                             fsTask->fileSlot,
                                                                             fsTask->uploadHandler,
                                                                             0,
                                                                             (char *)fsTask->fileName,
                                                                             fsTask->category,
                                                                             (void *)fsTask->thumbData,
                                                                             fsTask->thumbDataSize,
                                                                             fsTask->numTags,
                                                                             fsTask->tags,
                                                                             &fsTask->outFileID);
            bdReference<bdRemoteTask>::~bdReference<bdRemoteTask>(&v4);
        }
        else
        {
            remoteTask = *(bdRemoteTask **)bdContentStreaming::upload(
                                                                             (int)contentStreaming,
                                                                             (int)&v3,
                                                                             fsTask->fileSlot,
                                                                             fsTask->fileData,
                                                                             fsTask->fileSize,
                                                                             (char *)fsTask->fileName,
                                                                             fsTask->category,
                                                                             (void *)fsTask->thumbData,
                                                                             fsTask->thumbDataSize,
                                                                             fsTask->numTags,
                                                                             fsTask->tags,
                                                                             &fsTask->outFileID);
            bdReference<bdRemoteTask>::~bdReference<bdRemoteTask>(&v3);
        }
        if ( remoteTask )
        {
            task = TaskManager2_CreateTask(task_WriteContentServerFile, controllerIndex, 0, -1);
            bdReference<bdCommonAddr>::operator=(&task->remoteTask, remoteTask);
            TaskManager2_StartTask(task);
            task->payload = fsTask;
        }
    }
    return task;
#else
    return NULL;
#endif
}

void __cdecl dwFileShareGetProgress(
                int localControllerIndex,
                unsigned int *bytes,
                float *dataRate,
                fileShareLocation location)
{
    if ( !bytes && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\Dw\\dwStorage.cpp", 805, 0, "%s", "bytes") )
        __debugbreak();
    if ( !dataRate && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\Dw\\dwStorage.cpp", 806, 0, "%s", "dataRate") )
        __debugbreak();
    if ( bytes && dataRate )
    {
        if ( location == FILESHARE_LOCATION_USERSTORAGE )
        {
            dwFileShareGetProgressUser(localControllerIndex, bytes, dataRate);
        }
        else if ( location == FILESHARE_LOCATION_POOLEDSTORAGE )
        {
            dwFileShareGetProgressPooled(localControllerIndex, bytes, dataRate);
        }
        else
        {
            Com_PrintWarning(16, "Invalid fileshare location\n");
        }
    }
}

void __cdecl dwFileShareGetProgressPooled(int localControllerIndex, unsigned int *bytes, float *dataRate)
{
#ifdef BO1_DEMON
    bdPooledStorage *pooledStorage; // [esp+0h] [ebp-4h]

    pooledStorage = dwGetPooledStorage(localControllerIndex);
    if ( pooledStorage )
    {
        bdContentStreamingBase::checkProgress(pooledStorage, bytes, dataRate);
    }
    else
    {
        Com_PrintError(16, "Error getting HTTP operation progress. No handle to pooled storage.\n");
        *bytes = 0;
        *dataRate = 0.0f;
    }
#endif
}

void __cdecl dwFileShareGetProgressUser(int localControllerIndex, unsigned int *bytes, float *dataRate)
{
#ifdef BO1_DEMON
    bdContentStreaming *contentStreaming; // [esp+0h] [ebp-4h]

    contentStreaming = dwGetContentStreaming(localControllerIndex);
    if ( contentStreaming )
    {
        bdContentStreamingBase::checkProgress(contentStreaming, bytes, dataRate);
    }
    else
    {
        Com_PrintError(16, "Error getting HTTP operation progress. No handle to content streaming.\n");
        *bytes = 0;
        *dataRate = 0.0f;
    }
#endif
}

void __cdecl dwFileShareAbortOperation(int localControllerIndex, fileShareLocation location)
{
    if ( location == FILESHARE_LOCATION_USERSTORAGE )
    {
        dwFileShareAbortOperationUser(localControllerIndex);
    }
    else if ( location == FILESHARE_LOCATION_POOLEDSTORAGE )
    {
        dwFileShareAbortOperationPooled(localControllerIndex);
    }
    else if ( !Assert_MyHandler(
                             "C:\\projects_pc\\cod\\codsrc\\src\\Dw\\dwStorage.cpp",
                             863,
                             0,
                             "Invalid file share location") )
    {
        __debugbreak();
    }
}

void __cdecl dwFileShareAbortOperationPooled(int localControllerIndex)
{
#ifdef BO1_DEMON
    bdPooledStorage *pooledStorage; // [esp+0h] [ebp-4h]

    pooledStorage = dwGetPooledStorage(localControllerIndex);
    if ( pooledStorage )
        bdContentStreamingBase::abortHTTPOperation(pooledStorage);
    else
        Com_PrintError(16, "Error aborting HTTP operation. No handle to pooled storage.\n");
#endif
}

void __cdecl dwFileShareAbortOperationUser(int localControllerIndex)
{
#ifdef BO1_DEMON
    bdContentStreaming *contentStreaming; // [esp+0h] [ebp-4h]

    contentStreaming = dwGetContentStreaming(localControllerIndex);
    if ( contentStreaming )
        bdContentStreamingBase::abortHTTPOperation(contentStreaming);
    else
        Com_PrintError(16, "Error aborting HTTP operation. No handle to content streaming.\n");
#endif
}

unsigned int __cdecl dwFileShareGetLastHTTPError(int localControllerIndex, fileShareLocation location)
{
    if ( location == FILESHARE_LOCATION_USERSTORAGE )
        return dwFileShareGetLastHTTPErrorUser(localControllerIndex);
    if ( location == FILESHARE_LOCATION_POOLEDSTORAGE )
        return dwFileShareGetLastHTTPErrorPooled(localControllerIndex);
    if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\Dw\\dwStorage.cpp", 908, 0, "Invalid file share locatin") )
        __debugbreak();
    return 0;
}

unsigned int __cdecl dwFileShareGetLastHTTPErrorPooled(int localControllerIndex)
{
#ifdef BO1_DEMON
    bdPooledStorage *pooledStorage; // [esp+0h] [ebp-4h]

    pooledStorage = dwGetPooledStorage(localControllerIndex);
    if ( pooledStorage )
        return bdContentStreamingBase::getLastHTTPError(pooledStorage);
    Com_PrintError(16, "Error aborting HTTP operation. No handle to pooled storage.\n");
    return 0;
#else
    return 0;
#endif
}

unsigned int __cdecl dwFileShareGetLastHTTPErrorUser(int localControllerIndex)
{
#ifdef BO1_DEMON
    bdContentStreaming *contentStreaming; // [esp+0h] [ebp-4h]

    contentStreaming = dwGetContentStreaming(localControllerIndex);
    if ( contentStreaming )
        return bdContentStreamingBase::getLastHTTPError(contentStreaming);
    Com_PrintError(16, "Error aborting HTTP operation. No handle to content streaming.\n");
    return 0;
#else
    return 0;
#endif
}

TaskRecord *__cdecl dwFileShareSubmitRating(int localControllerIndex, dwFileShareSubmitRatingTask *const fsTask)
{
#ifdef BO1_DEMON
    bdReference<bdCommonAddr> v3; // [esp+20h] [ebp-10h] BYREF
    bdRemoteTask *remoteTask; // [esp+24h] [ebp-Ch]
    bdVoteRank *voteRankService; // [esp+28h] [ebp-8h]
    TaskRecord *task; // [esp+2Ch] [ebp-4h]

    if ( !fsTask && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\Dw\\dwStorage.cpp", 943, 0, "%s", "fsTask") )
        __debugbreak();
    voteRankService = dwGetVoteRankService(localControllerIndex);
    task = 0;
    if ( voteRankService )
    {
        if ( fsTask )
        {
            remoteTask = 0;
            remoteTask = *(bdRemoteTask **)bdVoteRank::submitRating(
                                                                             voteRankService,
                                                                             (int)&v3,
                                                                             (int)fsTask,
                                                                             fsTask->numRatings);
            bdReference<bdRemoteTask>::~bdReference<bdRemoteTask>(&v3);
            if ( remoteTask )
            {
                task = TaskManager2_CreateTask(task_FileShareSubmitRating, localControllerIndex, 0, 0);
                bdReference<bdCommonAddr>::operator=(&task->remoteTask, remoteTask);
                TaskManager2_StartTask(task);
                task->payload = fsTask;
            }
        }
    }
    return task;
#else
    return NULL;
#endif
}


#endif