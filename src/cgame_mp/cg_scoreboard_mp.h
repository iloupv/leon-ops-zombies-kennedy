#pragma once
#include <game/teams.h>
#include <universal/dvar.h>

enum listColumnTypes_t : __int32
{                                       // XREF: listColumnInfo_t/r
                                        // CG_AddSBColumnToMatchScoreBoard/r
    LCT_RANK_ICON    = 0x0,             // XREF: .rdata:columnInfoParty/s
    LCT_STATUS_ICON  = 0x1,             // XREF: .rdata:columnInfoParty/s
    LCT_NAME         = 0x2,             // XREF: .rdata:columnInfoParty/s
    LCT_TALKING_ICON = 0x3,             // XREF: .rdata:columnInfoParty/s
    LCT_SCORE        = 0x4,
    LCT_SB_COLUMN    = 0x5,
    LCT_PING         = 0x6,
    LCT_NUM          = 0x7,
};

enum scoreboardColumnType_t : __int32
{                                                                             // XREF: UnarchivedMatchState/r
    SB_TYPE_INVALID = 0x0,                 // XREF: .rdata:columnInfoParty/s
    SB_TYPE_NONE = 0x1,
    SB_TYPE_KILLS = 0x2,
    SB_TYPE_DEATHS = 0x3,
    SB_TYPE_ASSISTS = 0x4,
    SB_TYPE_DEFENDS = 0x5,
    SB_TYPE_PLANTS = 0x6,
    SB_TYPE_DEFUSES = 0x7,
    SB_TYPE_RETURNS = 0x8,
    SB_TYPE_CAPTURES = 0x9,
    SB_TYPE_DESTRUCTIONS = 0xA,
    SB_TYPE_KDRATIO = 0xB,
    SB_TYPE_SURVIVED = 0xC,
    SB_TYPE_STABS = 0xD,
    SB_TYPE_TOMAHAWKS = 0xE,
    SB_TYPE_HUMILIATED = 0xF,
    SB_TYPE_X2SCORE = 0x10,
    SB_TYPE_HEADSHOTS = 0x11,
    NUM_SB_TYPES = 0x12,
};

struct score_s // sizeof=0x2C
{                                                                             // XREF: clientState_s/r
    int ping;
    int status_icon;
    int place;
    int score;
    int kills;
    int assists;
    int deaths;
    int scoreboardColumns[4];
};

struct listColumnInfo_t // sizeof=0x14
{                                       // XREF: .rdata:columnInfoParty/r
    listColumnTypes_t type;
    float fWidth;
    const char *pszName;
    int iAlignment;
    scoreboardColumnType_t sbColumnType;
};

struct matchClientScoreData_t // sizeof=0x70
{                                       // XREF: matchScoreBoardData_t/r
    int rank;                           // XREF: CG_UpdateMatchScoreboard(int)+172/w
                                        // CG_GetMatchScoreboardInfo(int,int,int,team_t)+9D/r ...
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    unsigned __int64 xuid;              // XREF: CG_UpdateMatchScoreboard(int)+13B/w
                                        // CG_UpdateMatchScoreboard(int)+149/w ...
    int clientNum;                      // XREF: CG_UpdateMatchScoreboard(int)+112/w
                                        // CG_GetInGamePlayerListIcon(int,int,int,team_t,Material * *)+B1/r ...
    struct Material *hRankIcon;                // XREF: CG_UpdateMatchScoreboard(int)+19B/w
                                        // CG_GetMatchScoreboardRankIcon(int,int,team_t,Material * *)+49/r
    char name[32];                      // XREF: CG_UpdateMatchScoreboard(int)+220/o
                                        // CG_GetMatchScoreboardInfo(int,int,int,team_t)+DC/o ...
    char clanAbbrev[8];                 // XREF: CG_UpdateMatchScoreboard(int)+255/o
                                        // CG_GetMatchScoreboardInfo(int,int,int,team_t)+C1/r ...
    score_s score;                      // XREF: CG_UpdateMatchScoreboard(int)+1C4/o
                                        // CG_GetMatchScoreboardInfo(int,int,int,team_t)+139/r ...
    team_t team;                        // XREF: CG_UpdateMatchScoreboard(int)+1F4/w
                                        // CG_UpdateMatchScoreboard(int)+274/r ...
};

struct __declspec(align(8)) matchScoreBoardData_t // sizeof=0xF10
{                                       // XREF: .data:matchScoreBoardData_t * matchScoreBoardData/r
    matchClientScoreData_t matchClientScoreData[32];
                                        // XREF: CG_UpdateMatchScoreboard(int)+112/w
                                        // CG_UpdateMatchScoreboard(int)+13B/w ...
    scoreboardColumnType_t scoreboardColumnTypes[4];
                                        // XREF: CG_GetNameForScoreboardColumn(int,int)+54/r
                                        // CG_UpdateMatchScoreboard(int)+A2/w ...
    listColumnInfo_t inGameScoreboardColumnInfo[12];
                                        // XREF: CG_AddSBColumnToMatchScoreBoard+21/w
                                        // CG_AddSBColumnToMatchScoreBoard+48/w ...
    int numClients;                     // XREF: CG_UpdateMatchScoreboard(int)+54/w
                                        // CG_UpdateMatchScoreboard(int)+2A8/r ...
    int numAllies;                      // XREF: CG_UpdateMatchScoreboard(int)+41/w
                                        // CG_UpdateMatchScoreboard(int)+287/r ...
    int numInGameScoreboardColumns;     // XREF: CG_UpdateMatchScoreboard(int)+67/w
                                        // CG_AddSBColumnToMatchScoreBoard+15/r ...
    // padding byte
    // padding byte
    // padding byte
    // padding byte
};

struct cg_s;
struct Font_s;

const char *__cdecl CG_GetNameForScoreboardColumn(int localClientNum, unsigned int columnNumber);
void __cdecl CG_UpdateMatchScoreboard(int localClientNum);
void __cdecl CG_AddSBColumnToMatchScoreBoard(
                int localClientNum,
                listColumnTypes_t columnType,
                float columnWidth,
                const char *columnName,
                int alignment,
                scoreboardColumnType_t sbColumnType);
int __cdecl CG_GetMatchScoreboardClientCount(int localClientNum, team_t team);
team_t __cdecl CG_GetMatchscoreboardTeam(int localClientNum, int index);
int __cdecl CG_GetMatchScoreboardIndexForTeam(int localClientNum, int index, team_t team);
void __cdecl CG_GetInGamePlayerListIcon(int localClientNum, int column, int index, team_t team, Material **handle);
char *__cdecl CG_GetMatchScoreboardInfo(int localClientNum, int column, int index, team_t team);
char *__cdecl CG_GetColumnValueString(int localClientNum, const score_s *score, scoreboardColumnType_t columnType);
int __cdecl CG_GetColumnValue(int localClientNum, const score_s *score, scoreboardColumnType_t columnType);
char *__cdecl CG_GetMatchInGamePlayerListInfo(
                int localClientNum,
                int column,
                int index,
                team_t team,
                Material **handle);
const char *__cdecl CG_IsInGamePlayerListPlayerTalking(int localClientNum, unsigned int clientNum, Material **handle);
char *__cdecl CG_GetMatchInGamePlayersInfo(
                int localClientNum,
                int column,
                unsigned int index,
                int feederType,
                Material **handle);
const char *__cdecl CG_GetMatchInGamePlayerXuid(int localClientNum, int index, team_t team);
char __cdecl CG_GetMatchInGamePlayersXUID(
                int localClientNum,
                unsigned int index,
                int feederType,
                unsigned __int64 *retXUID);
void __cdecl CG_GetMatchScoreboardRankIcon(int localClientNum, int index, team_t team, Material **handle);
int __cdecl CG_GetIndexIntoMatchScoreboard(int localClientNum, int controllerIndex);
int __cdecl CG_GetWagerPlaceForMatchScoreboard(int localClientNum, int matchScoreboardIndex);
const score_s *__cdecl GetClientScore(int localClientNum, int clientNum);
const score_s *__cdecl UI_GetOurClientScore(int localClientNum);
const score_s *__cdecl UI_GetScoreAtRank(int localClientNum, int rank);
char *__cdecl CG_GetGametypeDescription(int localClientNum);
char __cdecl CG_DrawScoreboard_GetTeamColorIndex(int team, int localClientNum);
int __cdecl CG_DrawScoreboard(int localClientNum);
void __cdecl CG_DrawScoreboard_Backdrop(int localClientNum, float alpha);
double __cdecl CG_BackdropWidth();
double __cdecl CG_BackdropLeft(int localClientNum);
double __cdecl CG_BackdropTop();
void __cdecl CG_DrawBackdropServerInfo(int localClientNum, float alpha);
void __cdecl CG_DrawScoreboard_ScoresList(int localClientNum, float alpha);
double __cdecl CG_DrawScoreboard_ListColumnHeaders(
                int localClientNum,
                const float *color,
                float y,
                float h,
                float listWidth);
void __cdecl CG_GetScoreboardInfo(int localClientNum, const listColumnInfo_t **colInfo, int *numFields);
int __cdecl CG_ScoreboardTotalLines(int localClientNum);
double __cdecl CG_DrawTeamOfClientScore(
                int localClientNum,
                const float *color,
                float y,
                int team,
                float listWidth,
                int *drawLine);
int __cdecl CG_CheckDrawScoreboardLine(int localClientNum, int *drawLine, float y, float lineHeight);
double __cdecl CG_DrawScoreboard_ListBanner(
                int localClientNum,
                const float *color,
                float y,
                float w,
                float h,
                int team,
                int *piDrawLine);
int __cdecl CG_GetLivePlayersOnTeam(int localClientNum, int team);
double __cdecl CG_DrawClientScore(
                int localClientNum,
                unsigned int clientNum,
                const float *color,
                float y,
                const score_s *score,
                float listWidth);
double __cdecl CalcXAdj(int align, float maxw, float w);
void __cdecl DrawListString(
                int localClientNum,
                char *string,
                float x,
                float y,
                float width,
                int alignment,
                Font_s *font,
                float scale,
                int style,
                const float *color);
void __cdecl CG_DrawClientPing(int localClientNum, int ping, float x, float y, float maxWidth, float maxHeight);
void __cdecl CG_DrawScrollbar(int localClientNum, float top);
void __cdecl CenterViewOnClient(int localClientNum);
bool __cdecl CG_IsScoreboardDisplayed(int localClientNum);
bool __cdecl CG_IsIntermission(int localClientNum);
void __cdecl CG_ScrollScoreboardUp(cg_s *cgameGlob);
void __cdecl CG_ScrollScoreboardDown(cg_s *cgameGlob);
void __cdecl CG_InitScoreboard();
void __cdecl CG_SetFocusScoreboardCmd();
void __cdecl CG_RegisterScoreboardDvars();
void __cdecl CG_RegisterScoreboardGraphics();
bool __cdecl Scoreboard_HandleInput(int localClientNum, int key);
bool __cdecl MuteError_HandleInput(int localClientNum);
int __cdecl CG_GetPlaceWithTiesForScore(int localClientNum, int score);
int __cdecl CG_GetKills(int localClientNum, const score_s *score);
int __cdecl CG_GetDeaths(int localClientNum, const score_s *score);

extern const dvar_t *cg_ScoresPing_MaxBars;
extern const dvar_t *cg_ScoresPing_Interval;
extern const dvar_t *cg_ScoresPing_HighColor;
extern const dvar_t *cg_ScoresPing_MedColor;
extern const dvar_t *cg_ScoresPing_LowColor;
extern const dvar_t *cg_ScoresPing_BgColor;
extern const dvar_t *cg_scoreboardScrollStep;
extern const dvar_t *cg_scoreboardBannerHeight;
extern const dvar_t *cg_scoreboardItemHeight;
extern const dvar_t *cg_scoreboardPingWidth;
extern const dvar_t *cg_scoreboardPingHeight;
extern const dvar_t *cg_scoreboardWidth;
extern const dvar_t *cg_scoreboardSplitscreenWidth;
extern const dvar_t *cg_scoreboardQuarterscreenWidth;
extern const dvar_t *cg_scoreboardHeight;
extern const dvar_t *cg_scoreboardMyColor;
extern const dvar_t *cg_scoreboardRankFontScale;
extern const dvar_t *cg_scoreboardTextOffset;
extern const dvar_t *cg_scoreboardFont;
extern const dvar_t *cg_scoreboardHeaderFontScale;
extern const dvar_t *cg_scoreboardPingText;
extern const dvar_t *cg_scoreboardPingGraph;