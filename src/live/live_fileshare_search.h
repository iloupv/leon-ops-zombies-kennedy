#pragma once
#include <demo/demo_common.h>
#include "live_storage.h"

struct listBoxDef_s;
struct itemDef_s;


struct fshSearchPage_t // sizeof=0x5240
{                                       // XREF: fshSearch_t/r
    int startIndex;                     // XREF: Live_FileShareSearch_ClearSearch+42/w
                                        // Live_FileShareSearch_IsPageLoaded+37/r ...
    int timestamp;
    int numDescriptors;
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    bdFileMetaData descriptors[10];
};

struct fileShareSearchBackoff_t // sizeof=0x8
{                                       // XREF: .data:s_searchBackoff/r
    bool isActive;                      // XREF: Live_FileShareSearch_GetNextTaskDelay+C/r
                                        // Live_FileShareSearch_ReportSuccess+3/r ...
    // padding byte
    // padding byte
    // padding byte
    int level;                          // XREF: Live_FileShareSearch_GetNextTaskDelay+17/r
                                        // Live_FileShareSearch_ReportSuccess+15/w ...
};

struct fshSearchBookFiles_t // sizeof=0x5470
{                                       // XREF: fshSearch_t/r
    int startIndex;                     // XREF: Live_FileShareSearch_ClearSearch:loc_964B51/w
                                        // Live_FileShareSearch_LoadBook+8F/w ...
    int numFiles;                       // XREF: Live_FileShareSearch_ClearSearch+6B/w
                                        // Live_FileShareSearch_LoadBook+94/w ...
    bdFileID fileIDs[100];              // XREF: Live_FileShareSearch_LoadBook+C4/o
                                        // Live_FileShareSearch_LoadBook+10E/o ...
    bool needToReadRatings;             // XREF: Live_FileShareSearch_LoadBook+9E/w
                                        // Live_FileShareSearch_AreRatingsLoaded+3/r ...
    // padding byte
    // padding byte
    // padding byte
    int numRatings;                     // XREF: Live_FileShareSearch_ClearSearch+75/w
                                        // Live_FileShareSearch_LoadBook+123/o ...
    bdVoteRankStatsInfo ratings[100];   // XREF: Live_FileShareSearch_LoadBook+11C/o
                                        // Live_FileShareSearch_LoadRatings+49/o ...

    fshSearchBookFiles_t() = default;
    ~fshSearchBookFiles_t() = default;
};

struct __declspec(align(8)) fshSearchContext_t // sizeof=0x3E0
{                                       // XREF: fshSearch_t/r
    bdTag tags[40];
    fileShareLocation location;         // XREF: Live_FileShareSearch_SetContext(int,bdTag * const,int,fileShareLocation,bool,int)+9A/w
                                        // Live_FileShareSearch_ClearSearch+7F/w ...
    int numTags;                        // XREF: Live_FileShareSearch_SetContext(int,bdTag * const,int,fileShareLocation,bool,int)+92/w
                                        // Live_FileShareSearch_ClearSearch+90/w ...
    int timestamp;                      // XREF: Live_FileShareSearch_SetContext(int,bdTag * const,int,fileShareLocation,bool,int)+8A/w
                                        // Live_FileShareSearch_NeedToPerformSearch+67/r
    int fileType;                       // XREF: Live_FileShareSearch_SetContext(int,bdTag * const,int,fileShareLocation,bool,int)+B1/w
                                        // Live_FileShareSearch_NeedToPerformSearch+33/r ...
    bool sortHighestRated;              // XREF: Live_FileShareSearch_SetContext(int,bdTag * const,int,fileShareLocation,bool,int)+A3/w
                                        // Live_FileShareSearch_NeedToPerformSearch+41/r ...
    bool isValid;                       // XREF: Live_FileShareSearch_SetContext(int,bdTag * const,int,fileShareLocation,bool,int)+B7/w
                                        // Live_FileShareSearch_ClearSearch+89/w ...
    // padding byte
    // padding byte
    int maxPastDays;                    // XREF: Live_FileShareSearch_SetIndexOverride(int,int)+1E/r
                                        // Live_FileShareSearch_SetIndexOverride(int,int)+95/r ...
    int totalOverride;                  // XREF: Live_FileShareSearch_SetIndexOverride(int,int)+27/w
                                        // Live_FileShareSearch_SetIndexOverride(int,int)+BE/r ...
    // padding byte
    // padding byte
    // padding byte
    // padding byte

    //~fshSearchContext_t();
};

struct fshSearch_t // sizeof=0x14F18
{                                       // XREF: .data:s_fshSearch/r
    int totalFiles;                     // XREF: Live_FileShareSearch_GetTotalFiles(int)+3/r
                                        // Live_FileShareSearch_ClearSearch+4/w
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    fshSearchBookFiles_t bookFiles;     // XREF: Live_FileShareSearch_ClearSearch:loc_964B51/w
                                        // Live_FileShareSearch_ClearSearch+6B/w ...
    fshSearchPage_t pages[3];           // XREF: Live_FileShareSearch_ClearSearch+42/w
                                        // Live_FileShareSearch_IsPageLoaded+37/r ...
    fshSearchContext_t context;         // XREF: Live_FileShareSearch_SetIndexOverride(int,int)+1E/r
                                        // Live_FileShareSearch_SetIndexOverride(int,int)+27/w ...

    fshSearch_t() = default;
    ~fshSearch_t() = default;

    //fshSearch_t *__thiscall fshSearch_t::fshSearch_t(fshSearch_t *this);
    //void __thiscall fshSearch_t::~fshSearch_t(fshSearch_t *this);
};

void __cdecl Live_FileShareSearch_LoadSummary(int controllerIndex, int itemIndex);
char __cdecl Live_FileShareSearch_IsTaskPermitted();
int __cdecl Live_FileShareSearch_GetNextTaskDelay();
int __cdecl Live_FileShareSearch_GetTaskGap();
void __cdecl Live_FileShareSearch_LoadSummarySuccess();
void __cdecl Live_FileShareSearch_ReportSuccess();
void __cdecl Live_FileShareSearch_LoadSummaryFailure();
void __cdecl Live_FileShareSearch_ReportFailure();
void __cdecl Live_FileShareSearch_SetIndexOverride(int controllerIndex, int itemIndex);
int __cdecl Live_FileShareSearch_GetTotalFiles();
void __cdecl Live_FileShareSearch_SetContext(
                int controllerIndex,
                bdTag *tags,
                int numTags,
                fileShareLocation location,
                bool sortHighestRated,
                int numPastDays);
int Live_FileShareSearch_ClearSearch();
char __cdecl Live_FileShareSearch_NeedToPerformSearch(
                bdTag *tags,
                int numTags,
                fileShareLocation location,
                int fileType,
                bool sortHighestRated,
                int numPastDays);
void __cdecl Live_FileShareSearch_LoadBook(int controllerIndex, int itemIndex, bool forceTask);
int __cdecl Live_FileShareSearch_GetFirstBookItem(int fileIndex);
bool __cdecl Live_FileShareSearch_IsBookLoading(int controllerIndex);
void __cdecl Live_FileShareSearch_LoadBookSuccess();
void __cdecl Live_FileShareSearch_LoadRatingsFailure();
bool __cdecl Live_FileShareSearch_LoadDescriptor(int controllerIndex, int startIndex, int selectedIndex, int itemIndex);
char __cdecl Live_FileShareSearch_LoadItem(int controllerIndex, int startIndex, int selectedIndex, int itemIndex);
char __cdecl Live_FileShareSearch_IsPageLoaded(int fileIndex);
void __cdecl Live_FileShareSearch_LoadPage(int controllerIndex, int startIndex, int itemIndex);
int __cdecl Live_FileShareSearch_GetFirstPageItem(int fileIndex);
bool __cdecl Live_FileShareSearch_IsPageLoading(int controllerIndex);
fshSearchPage_t *__cdecl Live_FileShareSearch_AllocatePage(int startIndex);
bool __cdecl Live_FileShareSearch_IsBookLoaded(int itemIndex);
bool __cdecl Live_FileShareSearch_AreRatingsLoaded();
void __cdecl Live_FileShareSearch_LoadRatings(int controllerIndex);
bool __cdecl Live_FileShareSearch_AreRatingsLoading(int controllerIndex);
void __cdecl Live_FileShareSearch_LoadRatingsSuccess();
char __cdecl Live_FileShareSearch_GetDescriptor(int controllerIndex, int itemIndex, bdFileMetaData *outDescriptor);
char __cdecl Live_FileShareSearch_FindDescriptor(
                int controllerIndex,
                int itemIndex,
                bdFileMetaData *outDescriptor,
                fshSearchPage_t **page);
char __cdecl Live_FileShareSearch_GetDesriptorByID(
                int controllerIndex,
                unsigned __int64 fileID,
                fileShareLocation location,
                bdFileMetaData *outDescriptor);
char __cdecl Live_FileShareSearch_FindDescriptorByID(
                int controllerIndex,
                unsigned __int64 fileID,
                fileShareLocation location,
                bdFileMetaData *outDescriptor);
void __cdecl Live_FileShareSearch_GetRatings(bdVoteRankStatsInfo **ratings, int *numRatings);
void __cdecl Live_FileShareSearch_MarkItemCorrupt(int controllerIndex, int itemIndex);
bool __cdecl Live_FileShareSearch_IsSummaryLoaded();
char __cdecl Live_FileShareSearch_FeederItemEnabled(int index);
char __cdecl Live_FileShareSearch_FeederColor(
                listBoxDef_s *listPtr,
                itemDef_s *item,
                int contextIndex,
                int index,
                int column,
                DvarValue *color);


