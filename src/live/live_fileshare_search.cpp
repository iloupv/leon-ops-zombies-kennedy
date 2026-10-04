#include "live_fileshare_search.h"
#include <win32/win_shared.h>
#include <cgame/cg_compass.h>
#include "live_storage.h"
#include "live_fileshare.h"
#include <universal/com_shared.h>

unsigned __int64 s_summaryFileID;
bool s_summaryFileLoaded;
int s_summaryFileRequestTime;

fileShareSearchBackoff_t s_searchBackoff;
fshSearch_t s_fshSearch;

void __cdecl Live_FileShareSearch_LoadSummary(int controllerIndex, int itemIndex)
{
    //if ( (_S6 & 1) == 0 )
    //{
    //    _S6 |= 1u;
    //    bdFileMetaData::bdFileMetaData(&descriptor);
    //    atexit(Live_FileShareSearch_LoadSummary_::_2_::_dynamic_atexit_destructor_for__descriptor__);
    //}
    static bdFileMetaData descriptor;
    if ( Live_FileShareSearch_GetDescriptor(controllerIndex, itemIndex, &descriptor)
        && (!descriptor.m_fileID || descriptor.m_fileID != s_summaryFileID || !s_summaryFileLoaded)
        && !CG_IsShowingZombieMap() )
    {
        if ( s_summaryFileID != descriptor.m_fileID )
        {
            s_summaryFileID = descriptor.m_fileID;
            s_summaryFileRequestTime = Sys_Milliseconds();
            s_summaryFileLoaded = 0;
        }
        if ( Live_FileShareSearch_GetDescriptor(controllerIndex, itemIndex, &descriptor)
            && (int)(Sys_Milliseconds() - s_summaryFileRequestTime) >= 2000
            && s_summaryFileID )
        {
            if ( Live_FileShareSearch_IsTaskPermitted() )
                LiveStorage_FileShare_ReadSummary(
                    controllerIndex,
                    &descriptor,
                    Live_FileShareSearch_LoadSummarySuccess,
                    Live_FileShareSearch_LoadSummaryFailure);
        }
    }
}

int s_lastTaskTime;
char __cdecl Live_FileShareSearch_IsTaskPermitted()
{
    signed int v0; // esi

    if ( s_lastTaskTime )
    {
        v0 = Sys_Milliseconds() - s_lastTaskTime;
        if ( v0 <= Live_FileShareSearch_GetNextTaskDelay() )
            return 0;
    }
    s_lastTaskTime = Sys_Milliseconds();
    return 1;
}

int __cdecl Live_FileShareSearch_GetNextTaskDelay()
{
    int taskGap; // [esp+0h] [ebp-4h]

    taskGap = Live_FileShareSearch_GetTaskGap();
    if ( s_searchBackoff.isActive )
        return taskGap + 10000 * s_searchBackoff.level;
    else
        return taskGap;
}

int __cdecl Live_FileShareSearch_GetTaskGap()
{
    return fshSearchTaskDelay->current.integer;
}

void __cdecl Live_FileShareSearch_LoadSummarySuccess()
{
    s_summaryFileLoaded = 1;
    Live_FileShareSearch_ReportSuccess();
}

void __cdecl Live_FileShareSearch_ReportSuccess()
{
    if ( s_searchBackoff.isActive )
    {
        s_searchBackoff.isActive = 0;
        s_searchBackoff.level = 0;
    }
}

void __cdecl Live_FileShareSearch_LoadSummaryFailure()
{
    s_summaryFileID = 0;
    s_summaryFileLoaded = 0;
    s_summaryFileRequestTime = 0;
    Live_FileShareSearch_ReportFailure();
}

void __cdecl Live_FileShareSearch_ReportFailure()
{
    s_searchBackoff.isActive = 1;
    if ( ++s_searchBackoff.level > 6 )
        s_searchBackoff.level = 6;
    Com_PrintError(16, "CommunitySearch: Setting backoff level to %d.\n", s_searchBackoff.level);
}

void __cdecl Live_FileShareSearch_SetIndexOverride(int controllerIndex, int itemIndex)
{
    qtime_s realTime; // [esp+0h] [ebp-874h] BYREF
    const char *stringResult; // [esp+24h] [ebp-850h] BYREF
    int now; // [esp+28h] [ebp-84Ch]
    int createTime; // [esp+2Ch] [ebp-848h] BYREF
    int maxDiff; // [esp+30h] [ebp-844h]
    bdFileMetaData descriptor; // [esp+34h] [ebp-840h] BYREF

    //bdFileMetaData::bdFileMetaData(&descriptor);
    if ( s_fshSearch.context.maxPastDays == -1 )
    {
        s_fshSearch.context.totalOverride = -1;
        ////bdFileMetaData::~bdFileMetaData(&descriptor);
    }
    else
    {
        if ( Live_FileShareSearch_GetDescriptor(controllerIndex, itemIndex, &descriptor) )
        {
            Live_FileShare_ReadFromMetaData(2, &descriptor, (char **)&stringResult, &createTime);
            now = Com_RealTime(&realTime, 1);
            maxDiff = 86400 * s_fshSearch.context.maxPastDays;
            if ( now - createTime > 86400 * s_fshSearch.context.maxPastDays
                && (s_fshSearch.context.totalOverride == -1 || itemIndex < s_fshSearch.context.totalOverride) )
            {
                s_fshSearch.context.totalOverride = itemIndex;
            }
        }
        ////bdFileMetaData::~bdFileMetaData(&descriptor);
    }
}

int __cdecl Live_FileShareSearch_GetTotalFiles()
{
    return s_fshSearch.totalFiles;
}

void __cdecl Live_FileShareSearch_SetContext(
                int controllerIndex,
                bdTag *tags,
                int numTags,
                fileShareLocation location,
                bool sortHighestRated,
                int numPastDays)
{
    unsigned int *v6; // ecx
    bdTag *v7; // edx
    const char *v8; // [esp+4h] [ebp-10h]
    int i; // [esp+10h] [ebp-4h]

    if ( sortHighestRated )
        v8 = "true";
    else
        v8 = "false";
    if ( location == FILESHARE_LOCATION_POOLEDSTORAGE )
        Com_Printf(16, "\tSearch: Setting Context: \n %d tags, %s storage, sortHighestRated: %s.\n", numTags, "POOLED", v8);
    else
        Com_Printf(16, "\tSearch: Setting Context: \n %d tags, %s storage, sortHighestRated: %s.\n", numTags, "USER", v8);
    if ( Live_FileShareSearch_NeedToPerformSearch(
                 tags,
                 numTags,
                 location,
                 fsSearchFileType->current.integer,
                 sortHighestRated,
                 numPastDays) )
    {
        Live_FileShareSearch_ClearSearch();
        s_fshSearch.context.timestamp = Sys_Milliseconds();
        s_fshSearch.context.numTags = numTags;
        s_fshSearch.context.location = location;
        s_fshSearch.context.sortHighestRated = sortHighestRated;
        s_fshSearch.context.fileType = fsSearchFileType->current.integer;
        s_fshSearch.context.isValid = 0;
        s_fshSearch.context.maxPastDays = numPastDays;
        //for ( i = 0; i < numTags; ++i )
        //{
        //    v6 = &s_fshSearch.context.tags[i].__vftable;
        //    v7 = &tags[i];
        //    v6[2] = v7->m_priTag;
        //    v6[3] = HIDWORD(v7->m_priTag);
        //    v6[4] = v7->m_secTag;
        //    v6[5] = HIDWORD(v7->m_secTag);
        //}
        Live_FileShareSearch_LoadBook(controllerIndex, 0, 1);
    }
    else
    {
        Com_Printf(16, "\tSearch: Search skipped. Context unchanged or timeout not yet reached.\n");
    }
}

int Live_FileShareSearch_ClearSearch()
{
    int result; // eax
    int i; // [esp+0h] [ebp-4h]

    s_fshSearch.totalFiles = 0;
    for ( i = 0; i < 3; ++i )
    {
        s_fshSearch.pages[i].numDescriptors = 0;
        s_fshSearch.pages[i].startIndex = -1;
        s_fshSearch.pages[i].timestamp = 0;
        result = i + 1;
    }
    s_fshSearch.bookFiles.startIndex = 0;
    s_fshSearch.bookFiles.numFiles = 0;
    s_fshSearch.bookFiles.numRatings = 0;
    s_fshSearch.context.location = FILESHARE_LOCATION_INVALID;
    s_fshSearch.context.isValid = 0;
    s_fshSearch.context.numTags = 0;
    s_fshSearch.context.totalOverride = -1;
    s_summaryFileID = 0;
    s_summaryFileRequestTime = 0;
    s_summaryFileLoaded = 0;
    return result;
}

char __cdecl Live_FileShareSearch_NeedToPerformSearch(
                bdTag *tags,
                int numTags,
                fileShareLocation location,
                int fileType,
                bool sortHighestRated,
                int numPastDays)
{
    int i; // [esp+18h] [ebp-4h]

    if ( !s_fshSearch.context.isValid
        || s_fshSearch.context.location != location
        || s_fshSearch.context.numTags != numTags
        || s_fshSearch.context.fileType != fileType
        || s_fshSearch.context.sortHighestRated != sortHighestRated
        || s_fshSearch.context.maxPastDays != numPastDays
        || (int)(Sys_Milliseconds() - s_fshSearch.context.timestamp) >= 60000 )
    {
        return 1;
    }
    for ( i = 0; i < numTags; ++i )
    {
        if ( LODWORD(tags[i].m_priTag) != LODWORD(s_fshSearch.context.tags[i].m_priTag)
            || *(unsigned __int64 *)((char *)&tags[i].m_priTag + 4) != __PAIR64__(
                                                                                                                                     s_fshSearch.context.tags[i].m_secTag,
                                                                                                                                     HIDWORD(s_fshSearch.context.tags[i].m_priTag))
            || HIDWORD(tags[i].m_secTag) != HIDWORD(s_fshSearch.context.tags[i].m_secTag) )
        {
            return 1;
        }
    }
    return 0;
}

void __cdecl Live_FileShareSearch_LoadBook(int controllerIndex, int itemIndex, bool forceTask)
{
    fileShareSearchInfo_t taskInfo; // [esp+0h] [ebp-30h] BYREF

    if ( !Live_FileShareSearch_IsBookLoading(controllerIndex) && (forceTask || Live_FileShareSearch_IsTaskPermitted()) )
    {
        if ( (LiveStorage_FileShare_IsPerformingSearch(controllerIndex)
             || LiveStorage_FileShare_IsReadingTopRated(controllerIndex))
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_fileshare_search.cpp",
                        435,
                        0,
                        "%s",
                        "!LiveStorage_FileShare_IsPerformingSearch( controllerIndex ) && !LiveStorage_FileShare_IsReadingTopRated( controllerIndex )") )
        {
            __debugbreak();
        }
        s_fshSearch.bookFiles.startIndex = Live_FileShareSearch_GetFirstBookItem(itemIndex);
        s_fshSearch.bookFiles.numFiles = 0;
        s_fshSearch.bookFiles.needToReadRatings = 1;
        if ( s_fshSearch.context.sortHighestRated )
        {
            LiveStorage_FileShare_ReadTopRated(
                controllerIndex,
                s_fshSearch.context.fileType,
                s_fshSearch.bookFiles.startIndex,
                100,
                s_fshSearch.bookFiles.fileIDs,
                &s_fshSearch.bookFiles.numFiles,
                &s_fshSearch.totalFiles,
                Live_FileShareSearch_LoadBookSuccess,
                Live_FileShareSearch_LoadRatingsFailure);
        }
        else
        {
            taskInfo.tags = s_fshSearch.context.tags;
            taskInfo.numTags = s_fshSearch.context.numTags;
            taskInfo.location = s_fshSearch.context.location;
            taskInfo.startIndex = s_fshSearch.bookFiles.startIndex;
            taskInfo.maxFileIDs = 100;
            taskInfo.outFileIDs = s_fshSearch.bookFiles.fileIDs;
            taskInfo.outFileCount = &s_fshSearch.bookFiles.numFiles;
            taskInfo.outRatings = s_fshSearch.bookFiles.ratings;
            taskInfo.outRatingCount = &s_fshSearch.bookFiles.numRatings;
            taskInfo.outTotalFileCount = (int *)&s_fshSearch;
            taskInfo.successCallback = Live_FileShareSearch_LoadBookSuccess;
            taskInfo.failureCallback = Live_FileShareSearch_LoadRatingsFailure;
            LiveStorage_FileShare_PerformSearch(controllerIndex, &taskInfo);
        }
    }
}

int __cdecl Live_FileShareSearch_GetFirstBookItem(int fileIndex)
{
    int startFileIndex; // [esp+0h] [ebp-4h]

    startFileIndex = fileIndex - 50;
    if ( fileIndex - 50 < 0 )
        return 0;
    return startFileIndex;
}

bool __cdecl Live_FileShareSearch_IsBookLoading(int controllerIndex)
{
    return LiveStorage_FileShare_IsPerformingSearch(controllerIndex)
            || LiveStorage_FileShare_IsReadingTopRated(controllerIndex);
}

void __cdecl Live_FileShareSearch_LoadBookSuccess()
{
    s_fshSearch.context.isValid = 1;
    Live_FileShareSearch_ReportSuccess();
}

void __cdecl Live_FileShareSearch_LoadRatingsFailure()
{
    Live_FileShareSearch_ReportFailure();
}

bool __cdecl Live_FileShareSearch_LoadDescriptor(int controllerIndex, int startIndex, int selectedIndex, int itemIndex)
{
    return s_fshSearch.context.isValid
            && Live_FileShareSearch_LoadItem(controllerIndex, startIndex, selectedIndex, itemIndex);
}

char __cdecl Live_FileShareSearch_LoadItem(int controllerIndex, int startIndex, int selectedIndex, int itemIndex)
{
    if ( Live_FileShareSearch_IsBookLoaded(itemIndex) )
    {
        if ( Live_FileShareSearch_IsPageLoaded(itemIndex) )
        {
            if ( s_fshSearch.context.location != FILESHARE_LOCATION_USERSTORAGE || Live_FileShareSearch_AreRatingsLoaded() )
            {
                if ( s_fshSearch.context.location == FILESHARE_LOCATION_POOLEDSTORAGE && selectedIndex == itemIndex )
                    Live_FileShareSearch_LoadSummary(controllerIndex, itemIndex);
            }
            else
            {
                Live_FileShareSearch_LoadRatings(controllerIndex);
            }
            Live_FileShareSearch_SetIndexOverride(controllerIndex, itemIndex);
            return 1;
        }
        else
        {
            Live_FileShareSearch_LoadPage(controllerIndex, startIndex, itemIndex);
            return 0;
        }
    }
    else
    {
        Live_FileShareSearch_LoadBook(controllerIndex, itemIndex, 0);
        return 0;
    }
}

char __cdecl Live_FileShareSearch_IsPageLoaded(int fileIndex)
{
    int i; // [esp+0h] [ebp-4h]

    for ( i = 0; i < 3; ++i )
    {
        if ( s_fshSearch.pages[i].numDescriptors > 0
            && s_fshSearch.pages[i].startIndex <= fileIndex
            && fileIndex < s_fshSearch.pages[i].numDescriptors + s_fshSearch.pages[i].startIndex )
        {
            return 1;
        }
    }
    return 0;
}

void __cdecl Live_FileShareSearch_LoadPage(int controllerIndex, int startIndex, int itemIndex)
{
    fshSearchPage_t *page; // [esp+4h] [ebp-Ch]
    int bookFilesRemaining; // [esp+8h] [ebp-8h]
    int bookOffset; // [esp+Ch] [ebp-4h]

    if ( !Live_FileShareSearch_IsPageLoading(controllerIndex) && Live_FileShareSearch_IsTaskPermitted() )
    {
        page = Live_FileShareSearch_AllocatePage(startIndex);
        page->startIndex = Live_FileShareSearch_GetFirstPageItem(itemIndex);
        bookOffset = page->startIndex - s_fshSearch.bookFiles.startIndex;
        bookFilesRemaining = s_fshSearch.bookFiles.numFiles - bookOffset;
        if ( bookOffset < 0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_fileshare_search.cpp",
                        362,
                        0,
                        "%s",
                        "bookOffset >= 0") )
        {
            __debugbreak();
        }
        if ( bookFilesRemaining < 10 )
            LiveStorage_FileShare_LoadPage(
                controllerIndex,
                bookFilesRemaining,
                &s_fshSearch.bookFiles.fileIDs[bookOffset],
                s_fshSearch.context.location,
                page->descriptors,
                &page->numDescriptors,
                Live_FileShareSearch_ReportSuccess,
                Live_FileShareSearch_ReportFailure);
        else
            LiveStorage_FileShare_LoadPage(
                controllerIndex,
                10,
                &s_fshSearch.bookFiles.fileIDs[bookOffset],
                s_fshSearch.context.location,
                page->descriptors,
                &page->numDescriptors,
                Live_FileShareSearch_ReportSuccess,
                Live_FileShareSearch_ReportFailure);
    }
}

int __cdecl Live_FileShareSearch_GetFirstPageItem(int fileIndex)
{
    return fileIndex - fileIndex % 10;
}

bool __cdecl Live_FileShareSearch_IsPageLoading(int controllerIndex)
{
    return LiveStorage_FileShare_IsPageLoading(controllerIndex);
}

fshSearchPage_t *__cdecl Live_FileShareSearch_AllocatePage(int startIndex)
{
    int i; // [esp+0h] [ebp-Ch]
    fshSearchPage_t *resultPage; // [esp+4h] [ebp-8h]

    resultPage = s_fshSearch.pages;
    for ( i = 0; i < 3; ++i )
    {
        if ( s_fshSearch.pages[i].startIndex == -1 )
        {
            resultPage = &s_fshSearch.pages[i];
            break;
        }
        if ( s_fshSearch.pages[i].startIndex > startIndex + 17 )
        {
            resultPage = &s_fshSearch.pages[i];
            break;
        }
        if ( s_fshSearch.pages[i].startIndex + 10 < startIndex )
        {
            resultPage = &s_fshSearch.pages[i];
            break;
        }
        if ( s_fshSearch.pages[i].timestamp < resultPage->timestamp )
            resultPage = &s_fshSearch.pages[i];
    }
    resultPage->numDescriptors = 0;
    resultPage->startIndex = -1;
    resultPage->timestamp = Sys_Milliseconds();
    return resultPage;
}

bool __cdecl Live_FileShareSearch_IsBookLoaded(int itemIndex)
{
    int pageStartIndex; // [esp+0h] [ebp-4h]

    pageStartIndex = Live_FileShareSearch_GetFirstPageItem(itemIndex);
    return s_fshSearch.bookFiles.numFiles > 0
            && pageStartIndex >= s_fshSearch.bookFiles.startIndex
            && pageStartIndex < s_fshSearch.bookFiles.numFiles + s_fshSearch.bookFiles.startIndex;
}

bool __cdecl Live_FileShareSearch_AreRatingsLoaded()
{
    return !s_fshSearch.bookFiles.needToReadRatings;
}

void __cdecl Live_FileShareSearch_LoadRatings(int controllerIndex)
{
    if ( !Live_FileShareSearch_AreRatingsLoading(controllerIndex)
        && Live_FileShareSearch_IsTaskPermitted()
        && fsSearchFileType
        && fsSearchFileType->current.integer > 0 )
    {
        LiveStorage_FileShare_ReadRatingsCommunity(
            controllerIndex,
            s_fshSearch.bookFiles.fileIDs,
            s_fshSearch.bookFiles.numFiles,
            s_fshSearch.context.location,
            s_fshSearch.bookFiles.ratings,
            &s_fshSearch.bookFiles.numRatings,
            Live_FileShareSearch_LoadRatingsSuccess,
            Live_FileShareSearch_LoadRatingsFailure);
    }
}

bool __cdecl Live_FileShareSearch_AreRatingsLoading(int controllerIndex)
{
    return LiveStorage_FileShare_IsReadingRatings(controllerIndex);
}

void __cdecl Live_FileShareSearch_LoadRatingsSuccess()
{
    s_fshSearch.bookFiles.needToReadRatings = 0;
    Live_FileShareSearch_ReportSuccess();
}

char __cdecl Live_FileShareSearch_GetDescriptor(int controllerIndex, int itemIndex, bdFileMetaData *outDescriptor)
{
    fshSearchPage_t *page; // [esp+0h] [ebp-4h] BYREF

    return Live_FileShareSearch_FindDescriptor(controllerIndex, itemIndex, outDescriptor, &page);
}

char __cdecl Live_FileShareSearch_FindDescriptor(
                int controllerIndex,
                int itemIndex,
                bdFileMetaData *outDescriptor,
                fshSearchPage_t **page)
{
    int i; // [esp+20h] [ebp-4h]

    for ( i = 0; i < 3; ++i )
    {
        if ( s_fshSearch.pages[i].numDescriptors > 0
            && s_fshSearch.pages[i].startIndex <= itemIndex
            && itemIndex < s_fshSearch.pages[i].numDescriptors + s_fshSearch.pages[i].startIndex )
        {
            //bdFileMetaData::operator=(
            //    outDescriptor,
            //    (const bdFileMetaData *)(21056 * i + 2104 * (itemIndex - s_fshSearch.pages[i].startIndex) + 172815696));
            *outDescriptor = s_fshSearch.pages[i].descriptors[itemIndex - s_fshSearch.pages[i].startIndex];
            *page = &s_fshSearch.pages[i];
            return 1;
        }
    }
    return 0;
}

char __cdecl Live_FileShareSearch_GetDesriptorByID(
                int controllerIndex,
                unsigned __int64 fileID,
                fileShareLocation location,
                bdFileMetaData *outDescriptor)
{
    return Live_FileShareSearch_FindDescriptorByID(controllerIndex, fileID, location, outDescriptor);
}

char __cdecl Live_FileShareSearch_FindDescriptorByID(
                int controllerIndex,
                unsigned __int64 fileID,
                fileShareLocation location,
                bdFileMetaData *outDescriptor)
{
    int j; // [esp+24h] [ebp-8h]
    int i; // [esp+28h] [ebp-4h]

    if ( s_fshSearch.context.location != location )
        return 0;
    for ( i = 0; i < 3; ++i )
    {
        for ( j = 0; j < s_fshSearch.pages[i].numDescriptors; ++j )
        {
            if ( __PAIR64__(
                         HIDWORD(s_fshSearch.pages[i].descriptors[j].m_fileID),
                         s_fshSearch.pages[i].descriptors[j].m_fileID) == fileID )
            {
                //bdFileMetaData::operator=(outDescriptor, (const bdFileMetaData *)(21056 * i + 2104 * j + 172815696));
                *outDescriptor = s_fshSearch.pages[i].descriptors[j];
                return 1;
            }
        }
    }
    return 0;
}

void __cdecl Live_FileShareSearch_GetRatings(bdVoteRankStatsInfo **ratings, int *numRatings)
{
    *ratings = s_fshSearch.bookFiles.ratings;
    *numRatings = s_fshSearch.bookFiles.numRatings;
}

void __cdecl Live_FileShareSearch_MarkItemCorrupt(int controllerIndex, int itemIndex)
{
    int v2; // eax
    int v3; // ecx
    int i; // [esp+4h] [ebp-4h]

    for ( i = 0; i < 3; ++i )
    {
        if ( s_fshSearch.pages[i].numDescriptors > 0
            && s_fshSearch.pages[i].startIndex <= itemIndex
            && itemIndex < s_fshSearch.pages[i].numDescriptors + s_fshSearch.pages[i].startIndex )
        {
            v2 = 21056 * i;
            v3 = 2104 * (itemIndex - s_fshSearch.pages[i].startIndex);
            *(unsigned int *)(v2 + v3 + 172815704) = 0;
            *(unsigned int *)(v2 + v3 + 172815708) = 0;
            return;
        }
    }
}

bool __cdecl Live_FileShareSearch_IsSummaryLoaded()
{
    return s_summaryFileLoaded;
}

char __cdecl Live_FileShareSearch_FeederItemEnabled(int index)
{
    qtime_s realTime; // [esp+4h] [ebp-874h] BYREF
    const char *stringResult; // [esp+28h] [ebp-850h] BYREF
    int now; // [esp+2Ch] [ebp-84Ch]
    int createTime; // [esp+30h] [ebp-848h] BYREF
    int maxDiff; // [esp+34h] [ebp-844h]
    bdFileMetaData descriptor; // [esp+38h] [ebp-840h] BYREF

    //bdFileMetaData::bdFileMetaData(&descriptor);
    if ( s_fshSearch.context.maxPastDays == -1 )
    {
        ////bdFileMetaData::~bdFileMetaData(&descriptor);
        return 1;
    }
    else if ( Live_FileShareSearch_GetDescriptor(0, index, &descriptor)
                 && (Live_FileShare_ReadFromMetaData(2, &descriptor, (char **)&stringResult, &createTime),
                         now = Com_RealTime(&realTime, 1),
                         maxDiff = 86400 * s_fshSearch.context.maxPastDays,
                         now - createTime > 86400 * s_fshSearch.context.maxPastDays) )
    {
        //bdFileMetaData::~bdFileMetaData(&descriptor);
        return 0;
    }
    else
    {
        //bdFileMetaData::~bdFileMetaData(&descriptor);
        return 1;
    }
}

char __cdecl Live_FileShareSearch_FeederColor(
                listBoxDef_s *listPtr,
                itemDef_s *item,
                int contextIndex,
                int index,
                int column,
                DvarValue *color)
{
    if ( !column )
    {
        if ( Live_FileShareSearch_FeederItemEnabled(index) )
        {
            color->value = 1.0f;
            color->vector[1] = 1.0f;
            color->vector[2] = 1.0f;
            color->vector[3] = 1.0f;
        }
        else
        {
            *color = fshOldItemColor->current;
        }
    }
    return 1;
}
