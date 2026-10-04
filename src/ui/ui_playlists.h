#pragma once

#include <universal/dvar.h>

enum PlaylistLockState : __int32
{                                       // XREF: UI_DrawPlaylistDescription/r
    PLS_UNLOCKED                   = 0x0,
    PLS_FETCHING                   = 0x1,
    PLS_XP                         = 0x2,
    PLS_PLEVEL                     = 0x3,
    PLS_MAXPARTY                   = 0x4,
    PLS_MINPARTY                   = 0x5,
    PLS_GAMETYPEBAN                = 0x6,
    PLS_COLLECTORS                 = 0x7,
    PLS_FINISHEDSOLO               = 0x8,
    PLS_REQUIRED_DLC_DISABLED      = 0x9,
    PLS_REQUIRED_DLC_NOT_AVAILABLE = 0xA,
    PLS_CODPOINTS                  = 0xB,
    PLS_VISDVAR                    = 0xC,
    PLS_LOCALPLAYERS               = 0xD,
    PLS_NOGUESTS                   = 0xE,
    PLS_COUNT                      = 0xF,
};

enum parseBlockTypes : __int32
{                                       // XREF: ?LiveContracts_ParseContracts@@YAXPAD@Z/r
                                        // ?Playlist_ParsePlaylists@@YAXPBD@Z/r
    PARSE_UNKNOWN  = 0x0,
    PARSE_CONTRACT = 0x1,
};

struct playlistEntry // sizeof=0x14
{                                       // XREF: .data:nullPlaylistEntry/r
    const char *mapname;
    const char *gametype;
    int fullWeight;
    int remainingWeight;
    int ignore;
};

struct __declspec(align(4)) playlistGametype // sizeof=0x14
{                                       // XREF: .data:gametypes/r
    const char *scriptName;             // XREF: Playlist_ParsePlaylists(char const *)+2D0/r
                                        // Playlist_ParsePlaylists(char const *)+3C1/w ...
    const char *internalName;           // XREF: Playlist_ParsePlaylists(char const *)+31F/r
                                        // Playlist_ParsePlaylists(char const *)+39F/w ...
    const char *localizedName;          // XREF: Playlist_ParsePlaylists(char const *)+2E6/r
                                        // Playlist_ParsePlaylists(char const *)+30A/r ...
    const char *rules;                  // XREF: Playlist_ParsePlaylists(char const *)+3B3/w
                                        // Playlist_ParsePlaylists(char const *)+9E5/r
    bool teamBased;                     // XREF: Playlist_ParsePlaylists(char const *)+3E7/w
                                        // Playlist_ParsePlaylists(char const *)+1256/w
    // padding byte
    // padding byte
    // padding byte
};

struct categoryInfo // sizeof=0x34
{                                       // XREF: .data:categories/r
    const char *internalName;           // XREF: Playlist_GetCategoryIdByName(char const *)+57/r
                                        // Playlist_ParsePlaylists(char const *)+4C3/w ...
    const char *filter;                 // XREF: Playlist_ParsePlaylists(char const *)+4E6/w
                                        // Playlist_ParsePlaylists(char const *)+E30/w ...
    const char *localizedName;          // XREF: Playlist_ParsePlaylists(char const *)+4F8/w
                                        // Playlist_ParsePlaylists(char const *)+BF3/w ...
    const char *description;            // XREF: Playlist_ParsePlaylists(char const *)+50B/w
                                        // Playlist_ParsePlaylists(char const *)+F80/w ...
    const char *lockdescription;        // XREF: Playlist_ParsePlaylists(char const *)+51E/w
                                        // Playlist_ParsePlaylists(char const *)+1164/w ...
    const char *iconName;               // XREF: Playlist_ParsePlaylists(char const *)+530/w
                                        // Playlist_ParsePlaylists(char const *)+DA9/w ...
    int unlockXp;                       // XREF: Playlist_ParsePlaylists(char const *)+1605/w
                                        // Playlist_CategoryIsLocked(int,int,int,int,int,int)+75/r
    int unlockPLevel;                   // XREF: Playlist_ParsePlaylists(char const *)+16A5/w
                                        // Playlist_CategoryIsLocked(int,int,int,int,int,int)+93/r ...
    bool collectorsEditionRequired;     // XREF: Playlist_ParsePlaylists(char const *)+14E8/w
                                        // Playlist_CategoryIsLocked(int,int,int,int,int,int)+F8/r
    bool finishedSoloRequired;          // XREF: Playlist_ParsePlaylists(char const *)+1553/w
                                        // Playlist_CategoryIsLocked(int,int,int,int,int,int)+11D/r
    // padding byte
    // padding byte
    int playlistCount;                  // XREF: Playlist_ParsePlaylists(char const *)+543/w
                                        // Playlist_ParsePlaylists(char const *)+1B5F/r ...
    const char *rules;                  // XREF: Playlist_ParsePlaylists(char const *)+4D7/w
                                        // Playlist_ParsePlaylists(char const *)+A2F/r ...
    bool visible_dvar;                  // XREF: Playlist_ParsePlaylists(char const *)+1434/w
                                        // Playlist_CategoryIsLocked(int,int,int,int,int,int)+BD/r
    // padding byte
    // padding byte
    // padding byte
    const dvar_s *visDvar;              // XREF: Playlist_ParsePlaylists(char const *)+1453/w
                                        // Playlist_ParsePlaylists(char const *)+1462/r ...
};

struct playlistInfo // sizeof=0x90
{                                       // XREF: .data:playlists/r
    const char *name;                   // XREF: Playlist_ParsePlaylists(char const *)+6AB/w
                                        // Playlist_ParsePlaylists(char const *)+BB9/w ...
    const char *description;            // XREF: Playlist_ParsePlaylists(char const *)+6BE/w
                                        // Playlist_ParsePlaylists(char const *)+F63/w ...
    const char *lockdescription;        // XREF: Playlist_ParsePlaylists(char const *)+6D1/w
                                        // Playlist_ParsePlaylists(char const *)+1147/w ...
    const char *iconName;               // XREF: Playlist_ParsePlaylists(char const *)+6E4/w
                                        // Playlist_ParsePlaylists(char const *)+D77/w ...
    const char *rules;                  // XREF: Playlist_ParsePlaylists(char const *)+69C/w
                                        // Playlist_ParsePlaylists(char const *)+9A1/r ...
    const char *category[16];           // XREF: Playlist_ParsePlaylists(char const *)+7DF/w
                                        // Playlist_ParsePlaylists(char const *)+1AF2/w ...
    int baseEntry;                      // XREF: Playlist_ParsePlaylists(char const *)+6FD/w
                                        // Playlist_GetEntry(int,int)+61/r
    int numEntries;                     // XREF: Playlist_ParsePlaylists(char const *)+70C/w
                                        // Playlist_ParsePlaylists(char const *)+1DEB/r ...
    int unlockXp;                       // XREF: Playlist_ParsePlaylists(char const *)+72F/w
                                        // Playlist_ParsePlaylists(char const *)+15D6/w ...
    int unlockPLevel;                   // XREF: Playlist_ParsePlaylists(char const *)+742/w
                                        // Playlist_ParsePlaylists(char const *)+1676/w ...
    int gametypeBan;                    // XREF: Playlist_ParsePlaylists(char const *)+755/w
                                        // Playlist_ParsePlaylists(char const *)+170F/w ...
    int maxPartySize;                   // XREF: Playlist_ParsePlaylists(char const *)+768/w
                                        // Playlist_ParsePlaylists(char const *)+198C/w ...
    int minPartySize;                   // XREF: Playlist_ParsePlaylists(char const *)+77B/w
                                        // Playlist_ParsePlaylists(char const *)+19F3/w ...
    unsigned int requiredDLCMask;       // XREF: Playlist_ParsePlaylists(char const *)+1916/r
                                        // Playlist_ParsePlaylists(char const *)+1925/w ...
    bool ranked;                        // XREF: Playlist_ParsePlaylists(char const *)+71F/w
                                        // Playlist_ParsePlaylists(char const *)+1354/w ...
    bool collectorsEditionRequired;     // XREF: Playlist_ParsePlaylists(char const *)+14D1/w
                                        // Playlist_IsLocked(int,int,int,int,int,int)+13E/r
    bool finishedSoloRequired;          // XREF: Playlist_ParsePlaylists(char const *)+153B/w
                                        // Playlist_IsLocked(int,int,int,int,int,int)+16A/r
    // padding byte
    int wagerBet;                       // XREF: Playlist_ParsePlaylists(char const *)+1779/w
                                        // Playlist_GetPlaylistDescription(int,int)+71/r ...
    int wagerEntry;                     // XREF: Playlist_ParsePlaylists(char const *)+17E3/w
    int maxLocalPlayers;                // XREF: Playlist_ParsePlaylists(char const *)+7A1/w
                                        // Playlist_ParsePlaylists(char const *)+184D/w ...
    bool disableGuests;                 // XREF: Playlist_ParsePlaylists(char const *)+7B4/w
                                        // Playlist_ParsePlaylists(char const *)+189F/w ...
    bool visible_dvar;                  // XREF: Playlist_ParsePlaylists(char const *)+13D2/w
                                        // Playlist_IsLocked(int,int,int,int,int,int)+F7/r
    // padding byte
    // padding byte
    const dvar_s *visDvar;              // XREF: Playlist_ParsePlaylists(char const *)+13F1/w
                                        // Playlist_ParsePlaylists(char const *)+1400/r ...
    int categoryCount;                  // XREF: Playlist_ParsePlaylists(char const *)+78E/w
                                        // Playlist_ParsePlaylists(char const *)+1A43/w ...
};

int __cdecl Playlist_GetCategoryIdByName(const char *name);
void __cdecl Playlist_ParsePlaylists(const char *buffer);
char *__cdecl Playlist_Strdup(const char *src);
char *__cdecl Playlist_Strdup_Newline(const char *src);
char *__cdecl Playlist_BeginRules();
char __cdecl Playlist_AppendRules(const char *base, const char *token);
int __cdecl ParseIntRange(const char *field, const char **pData, int min, int max);
char __cdecl Playlist_HasCategory(int playlistId, const char *category);
int __cdecl Playlist_GetPlaylistCount();
int __cdecl Playlist_GetCategoryCount();
int __cdecl Playlist_GetPlaylistCount(int category);
void __cdecl Playlist_ValidatePlaylistNum();
int __cdecl Playlist_GetPlaylistIdForNum(int controllerIndex, int num);
int __cdecl Playlist_GetPlaylistIdForNum(int controllerIndex, unsigned int categoryPlaylistNum, int categoryNum);
int __cdecl Playlist_GetPlaylistNumFromId(int controllerIndex, int id);
int __cdecl Playlist_GetCategoryIdForNum(int num);
int __cdecl Playlist_GetCategoryIdForPlaylistId(int num);
int __cdecl Playlist_GetNumEntries(unsigned int playlistId);
playlistEntry *__cdecl Playlist_GetEntry(unsigned int playlistId, int entry);
void __cdecl Playlist_ResetCategoryFilter();
void __cdecl Playlist_EnableCategoryInFilter(const char *filterName);
const char *__cdecl Playlist_GetCategoryLocalizedName(int controllerIndex, unsigned int categoryId);
const char *__cdecl Playlist_GetPlaylistName(int controllerIndex, unsigned int playlistId);
const char *__cdecl Playlist_GetPlaylistDescription(int controllerIndex, unsigned int playlistId);
const char *__cdecl Playlist_GetPlaylistIconName(int controllerIndex, unsigned int playlistId);
const char *__cdecl Playlist_GetCategoryIconName(int controllerIndex, unsigned int categoryId);
int __cdecl Playlist_IsLocked(
                int controllerIndex,
                unsigned int playlistId,
                int xp,
                int prestige,
                int gametypeBan,
                int partyCount);
int __cdecl Playlist_CategoryIsLocked(int controllerIndex, unsigned int categoryId, int xp, int prestige);
int __cdecl Playlist_GetMapPacksUsedByPlaylist(unsigned int playlistId);
bool __cdecl Playlist_DoWeHaveRequiredDLC(int controllerIndex, unsigned int playlistId);
const char *__cdecl Playlist_GetCategoryInternalName(int categoryId);
playlistGametype *__cdecl Playlist_FindGametype(const char *gametypeInternalName);
int __cdecl Playlist_GetGametypeCount();
const char *__cdecl Playlist_GetGametypeLocalizedName(unsigned int index);
const char *__cdecl Playlist_GetGametypeInternalName(unsigned int index);
int __cdecl Playlist_ChooseEntry(int localControllerIndex, int lastSelection);
void __cdecl Playlist_ResetWeights(int playlistId);
int __cdecl Playlist_CalcTotalWeights(int playlistId, int lastSelection);
int __cdecl Playlist_FindEntryByWeight(int playlistId, int selectedWeight, int lastSelection);
void __cdecl Playlist_MarkEntryPlayed(int playlistId, int entryNum);
void __cdecl Playlist_RunRules(int localControllerIndex, int entryNum);
int __cdecl Playlist_GetVersionNumber();
int __cdecl Playlist_GetUnlockXp(int localControllerIndex, unsigned int playlistId);
int __cdecl Playlist_GetUnlockPLevel(int localControllerIndex, unsigned int playlistId);
int __cdecl Playlist_GetMaxPartySize(int localControllerIndex, unsigned int playlistId);
int __cdecl Playlist_GetMinPartySize(int localControllerIndex, unsigned int playlistId);
int __cdecl Playlist_GetWagerBet(int localControllerIndex, unsigned int playlistId);
void __cdecl Playlist_SetPrevPlaylist();
void __cdecl Playlist_GetPrevPlaylist();
void __cdecl Playlist_SetPlaylistEntry(int num);
void __cdecl Playlist_SetSVMapRotation();
void __cdecl Playlist_SetSVMapRotationCurrent(int entryNum);
void __cdecl Playlist_SVMapRotate();
void __cdecl Playlist_Init();
void Playlist_RegisterDvars();
void __cdecl Playlist_Fetch_f();
void __cdecl Playlist_Pick_f();
int __cdecl Playlist_GetWagerAmount(int version, int index, int *wagerAmount);


extern const dvar_t *party_linearMapCycle;
extern const dvar_t *party_linearMapCycleRandom;
extern const dvar_t *party_maxplayers;
extern const dvar_t *playlist_excludeGametype;
extern const dvar_t *playlist_excludeMap;
extern const dvar_t *playlist_excludeGametypeMap;
extern const dvar_t *playlist_enabled;
extern const dvar_t *playlist_next;
extern const dvar_t *playlist_entry;