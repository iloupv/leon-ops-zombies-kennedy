#pragma once

#include "live_storage.h"
#include <universal/com_stringtable.h>
#include <universal/dvar.h>
#include <gfx_d3d/r_material.h>

enum fileShareKeyIndex : __int32
{                                       // XREF: ?Live_FileShare_GetItemInfo@@YA_NHW4fileShareKeyIndex@@HW4fileShareInfoLocation@@PAPBDPAM@Z/r
                                        // ?Live_FileShare_GetMySlotInfo@@YA_NHHPBDPAPBDPAM@Z/r ...
    FILESHARE_KEY_SLOT               = 0x0,
    FILESHARE_KEY_INGAMESLOT         = 0x1,
    FILESHARE_KEY_SLOTOCCUPIED       = 0x2,
    FILESHARE_KEY_DATETIME           = 0x3,
    FILESHARE_KEY_LENGTH             = 0x4,
    FILESHARE_KEY_SIZE               = 0x5,
    FILESHARE_KEY_AUTHOR             = 0x6,
    FILESHARE_KEY_MAP                = 0x7,
    FILESHARE_KEY_MAPNAME            = 0x8,
    FILESHARE_KEY_GAMETYPE           = 0x9,
    FILESHARE_KEY_GAMETYPENAME       = 0xA,
    FILESHARE_KEY_FILEID             = 0xB,
    FILESHARE_KEY_FILESIZE           = 0xC,
    FILESHARE_KEY_FILETYPE           = 0xD,
    FILESHARE_KEY_FILENAME           = 0xE,
    FILESHARE_KEY_HASSUMMARY         = 0xF,
    FILESHARE_KEY_GAMETYPEANDMAPNAME = 0x10,
    FILESHARE_KEY_RATING             = 0x11,
    FILESHARE_KEY_TYPEICONNAME       = 0x12,
    FILESHARE_KEY_SELECTEDIMAGENAME  = 0x13,
    FILESHARE_KEY_NAME               = 0x14,
    FILESHARE_KEY_ISMODIFIEDNAME     = 0x15,
    FILESHARE_KEY_DESCRIPTION        = 0x16,
    FILESHARE_KEY_ISMODIFIEDDESCRIPTION = 0x17,
    FILESHARE_KEY_USERTAG            = 0x18,
    FILESHARE_KEY_USERTAGINDEX       = 0x19,
    FILESHARE_KEY_GAMETYPEIMAGE      = 0x1A,
    FILESHARE_KEY_TOTALVOTES         = 0x1B,
    FILESHARE_KEY_ISSUMMARYCACHED    = 0x1C,
    FILESHARE_KEY_COUNT              = 0x1D,
};

enum fileShareInfoLocation : __int32
{                                       // XREF: Live_FileShare_GetDate/r
                                        // Live_FileShare_GetLength/r ...
    FILESHARE_INFOLOCATION_INVALID = 0x0,
    FILESHARE_INFOLOCATION_SEARCHRESULTS = 0x1,
    FILESHARE_INFOLOCATION_MYFILESHARE = 0x2,
    FILESHARE_INFOLOCATION_MYFILESHARE_INGAME = 0x3,
    FILESHARE_INFOLOCATION_OTHERFILESHARE = 0x4,
};

enum fileSharePooledFileContext_t : __int32
{                                       // XREF: fileSharePooledDetails_t/r
                                        // ?Live_FileShare_SetPooledFileDetailsContext@@YAXW4fileSharePooledFileContext_t@@@Z/r ...
    FILESHARE_POOLEDFILEDETAILS_INVALID = 0x0,
    FILESHARE_POOLEDFILEDETAILS_SELF = 0x1,
    FILESHARE_POOLEDFILEDETAILS_FRIEND = 0x2,
};

struct __declspec(align(8)) ratingHistory_t // sizeof=0xC88
{                                       // XREF: .data:s_fileShareRatingHistory/r
    _BYTE files[3200];                  // XREF: Live_FileShare_SaveRating(int,unsigned __int64,uchar)+43/w
                                        // Live_FileShare_IsRated(int,unsigned __int64,uchar *)+64/r
    int next;
    // padding byte
    // padding byte
    // padding byte
    // padding byte
};

struct fileShareLastPlayedGame_t // sizeof=0x848
{                                       // XREF: .data:s_lastPlayedGame/r
    bdFileID dwFileID;                  // XREF: Live_FileShare_GetLastPlayedGameDetails(int)+34/w
                                        // Live_FileShare_GetLastPlayedGameDetails(int)+39/w ...
    bdFileMetaData descriptor;          // XREF: Live_FileShare_PopulateLastPlayedGameDvars(void)+1B/r
};

struct fileSharePooledDetails_t // sizeof=0x34
{                                       // XREF: .data:s_pooledFileDetails/r
    int kills;                          // XREF: Live_FileShare_GetPooledMapKills+E/r
    int deaths;                         // XREF: Live_FileShare_GetPooledMapDeaths+E/r
    int alliesScore;                    // XREF: Live_FileShare_GetPooledMapAlliesScore+E/r
                                        // Live_FileShare_GetPooledMapVictory+E/r ...
    int axisScore;                      // XREF: Live_FileShare_GetPooledMapAxisScore+E/r
                                        // Live_FileShare_GetPooledMapVictory+14/r ...
    int mapID;                          // XREF: Live_FileShare_GetPooledMapTeam+15/r
                                        // Live_FileShare_GetPooledTeamColorIcon+8/r ...
    int team;                           // XREF: Live_FileShare_GetPooledMapTeam+E/r
                                        // Live_FileShare_GetPooledMapVictory:loc_961313/r ...
    int score;                          // XREF: Live_FileShare_GetPooledScore+E/r
    int xpEarned;
    int wagerPayout;                    // XREF: Live_FileShare_GetPooledWagerPayout+E/r
    int position;                       // XREF: Live_FileShare_GetPooledPosition+E/r
    int isWager;                        // XREF: Live_FileShare_GetPooledIsWager+E/r
    bool isValid;                       // XREF: Live_FileShare_ExpirePooledFileDetails(void)+3/w
                                        // Live_FileShare_GetPooledFileInfo(char const *)+18/r ...
    // padding byte
    // padding byte
    // padding byte
    fileSharePooledFileContext_t context;
                                        // XREF: Live_FileShare_SetPooledFileDetailsContext(fileSharePooledFileContext_t)+3/r
                                        // Live_FileShare_SetPooledFileDetailsContext(fileSharePooledFileContext_t)+21/w
};

struct fileShareFilter // sizeof=0x1C
{                                       // XREF: .data:s_fileShareFilterBackup/r
    int fileType;
    int mapName;                        // XREF: Live_FileShare_FilterBackup_f+B/w
                                        // Live_FileShare_FilterRestore_f+8/r
    int gameType;                       // XREF: Live_FileShare_FilterBackup_f+1A/w
                                        // Live_FileShare_FilterRestore_f+22/r
    int age;                            // XREF: Live_FileShare_FilterBackup_f+28/w
                                        // Live_FileShare_FilterRestore_f+3B/r
    int userTag;                        // XREF: Live_FileShare_FilterBackup_f+36/w
                                        // Live_FileShare_FilterRestore_f+54/r
    int playerPref;                     // XREF: Live_FileShare_FilterBackup_f+45/w
                                        // Live_FileShare_FilterRestore_f+6E/r
    int sortBy;                         // XREF: Live_FileShare_FilterBackup_f+53/w
};

struct listBoxDef_s;
struct itemDef_s;
struct rectDef_s;

fileShareLastPlayedGame_t *__cdecl Live_FileShare_GetLastPlayedGame();
void __cdecl Live_FileShare_SetLiveBlurb(const char *blurb);
char __cdecl Live_FileShare_GetTag(
                bdFileMetaData *descriptors,
                int itemIndex,
                unsigned __int64 tagType,
                unsigned __int64 *tag);
fileSharePooledDetails_t *__cdecl Live_FileShare_GetFilePooledDetailsBuffer();
struct bdFileMetaData *__cdecl Live_FileShare_FindFileInPrivateData(
                unsigned __int64 fileID,
                fileShareBufferLocation bufferLocation);
bool __cdecl Live_FileShare_ValidatePublicDDLBuffer(char *buffer);
char __cdecl Live_FileShare_ReadPublicMetaDataCreateTime(bdFileMetaData *descriptor, int *createTime);
char __cdecl Live_FileShare_WritePublicMetaDataCreateTime(char *buffer, unsigned int createTime);
char __cdecl Live_FileShare_ReadPublicMetaDataLength(bdFileMetaData *descriptor, int *duration);
char __cdecl Live_FileShare_WritePublicMetaDataLength(char *buffer, unsigned int duration);
char __cdecl Live_FileShare_PublicMetaDataToTags(bdFileMetaData *descriptor);
char __cdecl Live_FileShare_WritePublicMetaDataTags(char *buffer, bdTag *tags, int numTags);
bool __cdecl Live_FileShare_ReadFromMetaData(
                int index,
                bdFileMetaData *descriptor,
                char **stringResult,
                int *intResult);
bool __cdecl Live_FileShare_ValidateDDLBuffer(char *buffer);
void __cdecl Live_FileShare_AddTag(
                unsigned __int64 priTag,
                unsigned __int64 secTag,
                int *numTags,
                bdTag *tags,
                int maxNumTags);
char *__cdecl Live_FileShare_GetLocalizedUserTagFromIndex(int index);
const char *__cdecl Live_FileShare_GetGameTypeFromIndex(int index);
const char *__cdecl Live_FileShare_GetGameTypeImageFromIndex(int index);
char *__cdecl Live_FileShare_GetLocalizedGameTypeFromIndex(int index, bool upperCase);
char *__cdecl Live_FileShare_GetLocalizedMapNameFromIndex(int index);
int __cdecl Live_FileShare_GetGameTypeCount();
int __cdecl Live_FileShare_GetIndexFromGameType(const char *gameType);
void __cdecl Live_FileShare_PopulateLastPlayedGameDvars();
void __cdecl Live_FileShare_GetLastPlayedGameDetails(int controllerIndex);
char *__cdecl Live_FileShare_GetFilterList();
void __cdecl Live_FileShare_AppendToFilterList(int checkValue, const char *textValue, bool *filterPresent);
unsigned int __cdecl Live_FileShare_FeederToSlot(unsigned int row, unsigned int col);
char __cdecl Live_FileShare_FileIDFromSlot(
                bdFileMetaData *descriptors,
                int descriptorCount,
                int slot,
                unsigned __int64 *fileID);
//char __cdecl Live_FileShare_IsSlotOccupied(int slot, fileShareBufferLocation location);
const char *__cdecl Live_FileShare_GetSelectedImage(bdFileMetaData *descriptors, int index);
//unsigned int __cdecl Live_FileShare_GetRating();
char *__cdecl Live_FileShare_LocalizedGameTypeAndMap(int gameTypeIndex, int mapIndex);
void __cdecl Live_FileShare_InitRatingHistory();
void __cdecl Live_FileShare_SaveRating(int controllerIndex, unsigned __int64 fileID, unsigned __int8 rating);
char __cdecl Live_FileShare_IsRated(int controllerIndex, unsigned __int64 fileID, unsigned __int8 *rating);
char __cdecl Live_FileShare_Theater_SetFile(unsigned __int64 fileID, bool isUserFile);
char __cdecl Live_FileShare_Theater_GetSize(int *size);
char __cdecl Live_FileShare_Theater_GetType(int *type);
char __cdecl Live_FileShare_Theater_GetName(char **name);
char __cdecl Live_FileShare_Theater_GetLength(int *length);
char __cdecl Live_FileShare_Theater_GetDescription(char **description);
char __cdecl Live_FileShare_Theater_GetMap(char **mapName);
char __cdecl Live_FileShare_Theater_GetGameType(char **gameTypeName);
int __cdecl Live_FileShare_GetSearchResultsCount();
char *__cdecl Live_FileShare_SearchResultsItemText(
                int controllerIndex,
                int startIndex,
                int selectedIndex,
                int itemIndex,
                bool *useOwnerDraw);
int __cdecl Live_FileShare_GetMyPrivateSlotsCount();
int __cdecl Live_FileShare_GetOtherPrivateSlotsCount();
char *__cdecl Live_FileShare_PrivateSlotsText(
                int controllerIndex,
                int feederID,
                unsigned int itemIndex,
                int column,
                Material **handle);
const char *__cdecl Live_FileShare_PrivateSlotsIngameText(
                int controllerIndex,
                int itemIndex,
                int column,
                Material **handle);
void __cdecl Live_FileShare_FeederColor(
                listBoxDef_s *listPtr,
                itemDef_s *item,
                int contextIndex,
                unsigned int index,
                int column,
                fileShareBufferLocation bufferLocation,
                DvarValue *color);
void __cdecl Live_FileShare_ColorEmptySlot(int column, int slotNum, DvarValue *color, bool isHighlighted);
void __cdecl Live_FileShare_ColorOccupiedSlot(unsigned int column, DvarValue *color, bool isHighlighted);
char __cdecl Live_FileShare_GetDate(
                bdFileMetaData *descriptors,
                int descriptorCount,
                int index,
                fileShareInfoLocation location,
                const char **stringResult);
char __cdecl Live_FileShare_GetLength(
                bdFileMetaData *descriptors,
                int descriptorCount,
                int index,
                fileShareInfoLocation location,
                const char **stringResult);
char __cdecl Live_FileShare_GetSize(
                bdFileMetaData *descriptors,
                int descriptorCount,
                int index,
                fileShareInfoLocation location,
                char **stringResult);
char __cdecl Live_FileShare_GetMap(
                bdFileMetaData *descriptors,
                int descriptorCount,
                int index,
                fileShareInfoLocation location,
                const char **stringResult);
char __cdecl Live_FileShare_GetMapName(
                bdFileMetaData *descriptors,
                int descriptorCount,
                int index,
                fileShareInfoLocation location,
                const char **stringResult);
char __cdecl Live_FileShare_GetGameType(
                bdFileMetaData *descriptors,
                int descriptorCount,
                int index,
                fileShareInfoLocation location,
                const char **stringResult);
char __cdecl Live_FileShare_GetGameTypeName(
                bdFileMetaData *descriptors,
                int descriptorCount,
                int index,
                fileShareInfoLocation location,
                char **stringResult);
char __cdecl Live_FileShare_GetGameTypeImage(
                bdFileMetaData *descriptors,
                int descriptorCount,
                int index,
                fileShareInfoLocation location,
                const char **stringResult);
char __cdecl Live_FileShare_GetGameTypeAndMapName(
                bdFileMetaData *descriptors,
                int descriptorCount,
                int index,
                fileShareInfoLocation location,
                char **stringResult);
char __cdecl Live_FileShare_HasSummary(
                bdFileMetaData *descriptors,
                int descriptorCount,
                int index,
                fileShareInfoLocation location,
                const char **stringResult,
                float *floatResult);
char __cdecl Live_FileShare_IsSummaryCached(
                bdFileMetaData *descriptors,
                int descriptorCount,
                int index,
                fileShareInfoLocation location,
                const char **stringResult,
                float *floatResult);
char __cdecl Live_FileShare_IsSummaryLoaded(
                bdFileMetaData *descriptors,
                int descriptorCount,
                int index,
                fileShareInfoLocation location,
                const char **stringResult,
                float *floatResult);
bool __cdecl Live_FileShare_GetFileType(
                bdFileMetaData *descriptors,
                int descriptorCount,
                int index,
                fileShareInfoLocation location,
                const char **stringResult);
char __cdecl Live_FileShare_GetUserTag(
                bdFileMetaData *descriptors,
                int descriptorCount,
                int index,
                fileShareInfoLocation location,
                char **stringResult);
char __cdecl Live_FileShare_GetUserTagIndex(
                bdFileMetaData *descriptors,
                int descriptorCount,
                int index,
                fileShareInfoLocation location,
                const char **stringResult,
                float *floatResult);
char __cdecl Live_FileShare_GetFileID(
                bdFileMetaData *descriptors,
                int descriptorCount,
                int index,
                fileShareInfoLocation location,
                const char **stringResult);
char __cdecl Live_FileShare_LookupRating(
                unsigned __int64 fileID,
                bdVoteRankStatsInfo *ratings,
                int numRatings,
                float *rating);
char __cdecl Live_FileShare_LookupRank(
                unsigned __int64 fileID,
                bdVoteRankStatsInfo *ratings,
                int numRatings,
                int *rank);
double __cdecl Live_FileShare_LookupTotalVotes(unsigned __int64 fileID, bdVoteRankStatsInfo *ratings, int numRatings);
char __cdecl Live_FileShare_GetRating(
                bdFileMetaData *descriptors,
                int descriptorCount,
                int index,
                fileShareInfoLocation location,
                const char **stringResult,
                float *floatResult);
char __cdecl Live_FileShare_GetTotalVotes(
                bdFileMetaData *descriptors,
                int descriptorCount,
                int index,
                fileShareInfoLocation location,
                const char **stringResult,
                float *floatResult);
char __cdecl Live_FileShare_GetOwnerName(
                bdFileMetaData *descriptors,
                int descriptorCount,
                int index,
                fileShareInfoLocation location,
                char **stringResult);
char __cdecl Live_FileShare_GetFileSize(
                bdFileMetaData *descriptors,
                int descriptorCount,
                int index,
                fileShareInfoLocation location,
                const char **stringResult);
char __cdecl Live_FileShare_GetFileName(
                bdFileMetaData *descriptors,
                int descriptorCount,
                int index,
                fileShareInfoLocation location,
                char **stringResult);
bool __cdecl Live_FileShare_GetTypeIconName(
                bdFileMetaData *descriptors,
                int descriptorCount,
                int index,
                fileShareInfoLocation location,
                const char **stringResult);
char __cdecl Live_FileShare_GetSlot(
                bdFileMetaData *descriptors,
                int descriptorCount,
                unsigned int index,
                fileShareInfoLocation location,
                const char **stringResult,
                float *floatResult);
char __cdecl Live_FileShare_GetIngameSlot(
                bdFileMetaData *descriptors,
                int descriptorCount,
                int index,
                fileShareInfoLocation location,
                const char **stringResult,
                float *floatResult);
bool __cdecl Live_FileShare_IsSlotOccupied(
                bdFileMetaData *descriptors,
                int descriptorCount,
                int index,
                fileShareInfoLocation location,
                const char **stringResult,
                float *floatResult);
bool __cdecl Live_FileShare_GetSelectedImageName(
                bdFileMetaData *descriptors,
                int descriptorCount,
                int index,
                fileShareInfoLocation location,
                const char **stringResult);
char __cdecl Live_FileShare_GetIsModifiedName(
                bdFileMetaData *descriptors,
                int descriptorCount,
                int index,
                fileShareInfoLocation location,
                char **stringResult,
                float *floatResult);
char __cdecl Live_FileShare_GetIsModifiedDescription(
                bdFileMetaData *descriptors,
                int descriptorCount,
                int index,
                fileShareInfoLocation location,
                char **stringResult,
                float *floatResult);
char __cdecl Live_FileShare_GetName(
                bdFileMetaData *descriptors,
                int descriptorCount,
                int index,
                fileShareInfoLocation location,
                char **stringResult,
                float *floatResult);
char __cdecl Live_FileShare_GetDescription(
                bdFileMetaData *descriptors,
                int descriptorCount,
                int index,
                fileShareInfoLocation location,
                char **stringResult,
                float *floatResult);
const char *__cdecl Live_FileShare_GetPooledTeamIcon(int mapID, int team);
char *__cdecl Live_FileShare_GetPooledTeamName(int mapID, int team);
int __cdecl Live_FileShare_GetSortBy();
void __cdecl Live_FileShare_ExpirePooledFileDetails();
void __cdecl Live_FileShare_SetPooledFileDetailsContext(fileSharePooledFileContext_t context);
const char *__cdecl Live_FileShare_GetPooledFileInfo(const char *field);
const char *__cdecl Live_FileShare_GetPooledMapKills();
const char *__cdecl Live_FileShare_GetPooledMapDeaths();
const char *__cdecl Live_FileShare_GetPooledMapTeam();
const char *__cdecl Live_FileShare_GetPooledMapAxisScore();
const char *__cdecl Live_FileShare_GetPooledMapAxisIcon();
const char *__cdecl Live_FileShare_GetPooledTeamColorIcon(int teamID);
char *__cdecl Live_FileShare_GetPooledMapAxisName();
const char *__cdecl Live_FileShare_GetPooledMapAlliesScore();
const char *__cdecl Live_FileShare_GetPooledMapAlliesIcon();
char *__cdecl Live_FileShare_GetPooledMapAlliesName();
const char *__cdecl Live_FileShare_GetPooledMapVictory();
const char *__cdecl Live_FileShare_IsPooledTeamBased();
const char *__cdecl Live_FileShare_GetPooledScore();
const char *__cdecl Live_FileShare_GetPooledWagerPayout();
const char *__cdecl Live_FileShare_GetPooledPosition();
char *__cdecl Live_FileShare_GetPooledIsWager();
char __cdecl Live_FileShare_GetItemInfo(
                int controllerIndex,
                fileShareKeyIndex keyIndex,
                int index,
                fileShareInfoLocation location,
                const char **stringResult,
                float *floatResult);
bool __cdecl Live_FileShare_AreInfoParamsValid(
                bdFileMetaData *descriptors,
                int descriptorCount,
                int index,
                fileShareInfoLocation location);
char __cdecl Live_FileShare_GetMySlotInfo(
                int controllerIndex,
                int slotNum,
                const char *field,
                const char **stringResult,
                float *floatResult);
char __cdecl Live_FileShare_GetFeederData(
                int controllerIndex,
                int feederID,
                int index,
                const char *field,
                const char **stringResult,
                float *floatResult);
void __cdecl Live_FileShare_RatingOwnerdraw(
                int contextIndex,
                const rectDef_s *rect,
                const itemDef_s *item,
                const float *color);
void __cdecl Live_FileShare_DrawBackgroundStars(
                int contextIndex,
                float x,
                float y,
                int w,
                int h,
                int hA,
                int vA,
                const float *color,
                Material *mtl);
void __cdecl Live_FileShare_DrawRatingStars(
                int contextIndex,
                float rating,
                float x,
                float y,
                int w,
                int h,
                int hA,
                int vA,
                const float *color,
                Material *mtl);
double __cdecl Live_FileShare_GetStarFractionToDraw(float rating, int imageNumber);
void __cdecl Live_FileShare_PrivateRatingOwnerdraw(
                int contextIndex,
                const rectDef_s *rect,
                const itemDef_s *item,
                const float *color);
void __cdecl Live_FileShare_SubmitRatingOwnerdraw(
                int contextIndex,
                const rectDef_s *rect,
                const itemDef_s *item,
                const float *color);
void __cdecl Live_FileShare_MyRatingOwnerdraw(
                int localClientNum,
                int contextIndex,
                const rectDef_s *rect,
                const itemDef_s *item,
                const float *color);
void __cdecl Live_FileShare_AvgSubmitRatingOwnerdraw(
                int contextIndex,
                const rectDef_s *rect,
                const itemDef_s *item,
                const float *color);
void __cdecl Live_FileShare_SearchDvarsToTags(int controllerIndex, bdTag *tags, int *numTags, int maxTags);
void __cdecl Live_FileShare_ClearSearchState();
int __cdecl Live_FileShare_SetSpinnerDvars(int offset, int max, dvar_s *valueDvar, dvar_s *textDvar);
void __cdecl Live_FileShare_Init();
void __cdecl Live_FileShare_Read_f();
void __cdecl Live_FileShare_ClearSearchDvars_f();
void __cdecl Live_FileShare_SearchShiftRight_f();
void __cdecl Live_FileShare_SetSearchRowOffset(int row, int offset);
void __cdecl Live_FileShare_SearchShiftLeft_f();
void __cdecl Live_FileShare_TransferFromPooled_f();
void __cdecl Live_FileShare_TransferFromUser_f();
void __cdecl Live_FileShare_TransferFromCommunity_f();
void __cdecl Live_FileShare_TransferLastUploaded_f();
void __cdecl Live_FileShare_GetRecentGames_f();
void __cdecl Live_FileShare_GetRecentGames(int localControllerIndex, unsigned __int64 playerXuid);
void __cdecl Live_FileShare_Remove_f();
void __cdecl Live_FileShare_EditInfo_f();
void __cdecl Live_FileShare_AbortOperation_f();
void __cdecl Live_FileShare_Search_f();
void __cdecl Live_FileShare_DvarSearch(int controllerIndex);
void __cdecl Live_FileShare_SubmitRating_f();
void __cdecl Live_FileShare_ResetRating_f();
void __cdecl Live_FileShare_IncreaseRating_f();
void __cdecl Live_FileShare_DecreaseRating_f();
void __cdecl Live_FileShare_SetRating_f();
void __cdecl Live_FileShare_SetRatingPreview_f();
void __cdecl Live_FileShare_ResetRatingPreview_f();
void __cdecl Live_FileShare_ReadUserTag_f();
void Live_FileShare_UserTagLeft_f(const StringTable *table);
void Live_FileShare_UserTagRight_f(const StringTable *table);
void __cdecl Live_FileShare_FilterBackup_f();
void __cdecl Live_FileShare_FilterRestore_f();
void __cdecl Live_FileShare_GetTopRated_f();
void __cdecl Live_FileShare_ExpireFileDetails_f();
void __cdecl Live_FileShare_GetLastPlayedGame_f();
void __cdecl Live_FileShare_SetPooledFileContext_f();


extern const dvar_t *fsSearchFileType;
extern const dvar_t *fsSearchSelectedRow;
extern const dvar_t *fsShowStreamingGraph;
extern const dvar_t *fsPrivateSlotCol;
extern const dvar_t *fsOtherUserPrivateSlotCol;
extern const dvar_t *fsMaxPrivateSlotRowsOther;
extern const dvar_t *fsOtherUserSlotSelected;
extern const dvar_t *fsSlotEmptyHiddenColor;
extern const dvar_t *fsSlotEmptyShowColorBg;
extern const dvar_t *fsSlotEmptyMainColor;
extern const dvar_t *fsSlotEmptyNotSubscribedColor;
extern const dvar_t *fsSlotNumMainColor;
extern const dvar_t *fsSlotNumNoSubsribeColor;
extern const dvar_t *fsSlotEmptyShowColor;
extern const dvar_t *fsSlotMainColor;
extern const dvar_t *fsSlotHighlightedColor;
extern const dvar_t *fsSlotHighlightedColorNoSel;
extern const dvar_t *fsDebugRatingValue;
extern const dvar_t *fsStarHighlightColor;
extern const dvar_t *fsStarPreviewColor;
extern const dvar_t *fsStarAvgColor;
extern const dvar_t *fshOldItemColor;
extern const dvar_t *fsSelectedFileID;
extern const dvar_t *fsSelectedFileName;
extern const dvar_t *fsIsSelectedFileNameModified;
extern const dvar_t *fsSelectedFileDescription;
extern const dvar_t *fsIsSelectedFileDescriptionModified;
extern const dvar_t *fsSelectedFileTagIndex;
extern const dvar_t *fshSelectLastSlotRow;
extern const dvar_t *fshSelectFirstSlotRow;
extern const dvar_t *fshLiveBlurb;
extern const dvar_t *fshDebugFileList;
extern const dvar_t *fsRecents;
extern const dvar_t *fsRecentsCount;
extern const dvar_t *s_fsSearchRowTextDvars[7];
extern const dvar_t *s_fsSearchRowValueDvars[7];