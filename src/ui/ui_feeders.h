#pragma once
#include <qcommon/cmd.h>
#include "ui_shared.h"
#include <ui_mp/ui_gametype_variants_mp.h>

int __cdecl UI_FeederCount(int localClientNum, int contextIndex, float feederID, listBoxDef_s *listPtr);
void __cdecl UI_BuildPlayerList(int localClientNum);
_CustomClassDescription *__cdecl UI_FeederItemText(
                int localClientNum,
                int contextIndex,
                itemDef_s *item,
                float feederID,
                itemGroup_t index,
                int column,
                Material **handle,
                float *angle,
                bool *useOwnerDraw);
char *__cdecl UI_FeederItemText_Servers(int localClientNum, int contextIndex, int index, int column, Material **handle);
char *__cdecl UI_FeederItemText_ServerStatus(int index, unsigned int column);
char *__cdecl UI_FeederItemText_ServerStatusScoreboard(int index, unsigned int column);
const char *__cdecl UI_FeederItemText_Mods(int index);
const char *__cdecl UI_FeederItemText_CommmonPlayerListHandler(
                int controllerIndex,
                int feederId,
                int index,
                int column,
                Material **handle,
                bool *useOwnerDraw);
char *__cdecl GetRecentPlayerInfoString(int minutes);
const char *__cdecl UI_FeederItemText_Invites(
                int controllerIndex,
                int feederId,
                int index,
                int column,
                Material **handle,
                bool *useOwnerDraw);
char *__cdecl UI_FeederItemText_Playlists(int controllerIndex, int index);
const char *__cdecl UI_FeederItemText_NewCategories(
                int controllerIndex,
                itemDef_s *item,
                int index,
                int column,
                Material **handle);
bool __cdecl Window_HasFocus(int contextIndex, const windowDef_t *w);
bool __cdecl Window_IsVisible(int contextIndex, const windowDef_t *w);
char *__cdecl UI_FeederItemText_NewPlaylists(
                int controllerIndex,
                itemDef_s *item,
                int index,
                int column,
                Material **handle);
char *__cdecl UI_FeederItemText_MuteList(int localClientNum, int contextIndex, int index, int column);
char *__cdecl UI_FeederItemText_DynamicMenu(
                int contextIndex,
                int index,
                int column,
                listBoxDef_s *listPtr,
                Material **handle);
void __cdecl UI_FeederItemOwnerDraw(
                int localClientNum,
                int contextIndex,
                itemDef_s *item,
                float feederID,
                const rectDef_s *rect,
                int index,
                int col,
                const float *color);
void __cdecl UI_FeederItemOwnerDraw_Invites(
                int controllerIndex,
                int contextIndex,
                itemDef_s *item,
                const rectDef_s *rect,
                int index,
                int col,
                const float *color);
void __cdecl UI_FeederItemOwnerDraw_OnlineFriends(
                int controllerIndex,
                int contextIndex,
                itemDef_s *item,
                const rectDef_s *rect,
                int index,
                int col,
                const float *color);
void __cdecl UI_FeederItemOwnerDraw_RecentPlayers(
                int controllerIndex,
                int contextIndex,
                itemDef_s *item,
                const rectDef_s *rect,
                int index,
                int col,
                const float *color);
void __cdecl UI_FeederItemOwnerDraw_FileShareSearchResults(
                int controllerIndex,
                int contextIndex,
                itemDef_s *item,
                const rectDef_s *rect,
                int index,
                int col,
                const float *color);
void __cdecl UI_FeederItemColor(
                int controllerIndex,
                int contextIndex,
                itemDef_s *item,
                float feederID,
                int index,
                int column,
                DvarValue *color,
                bool onFocus);
void __cdecl UI_GetListDetails(
                int localClientNum,
                int contextIndex,
                itemDef_s *item,
                char *feederName,
                itemDef_s **itemToFocus,
                int *lastIndex,
                int *viewMax);
bool __cdecl UI_OverrideKeyPress(int localClientNum, int contextIndex, itemDef_s *item, int key);
void __cdecl UI_OverrideCursorPos(int localClientNum, int contextIndex, itemDef_s *item);
void __cdecl UI_OverrideCursorPos_RecentPlayers(
                int controllerIndex,
                int contextIndex,
                itemDef_s *item,
                listBoxDef_s *listPtr);
void __cdecl UI_OverrideCursorPos_OnlineFriends(
                int controllerIndex,
                int contextIndex,
                itemDef_s *item,
                listBoxDef_s *listPtr);
void __cdecl UI_OverrideCursorPos_FileShareSearchResults(int localClientNum, int contextIndex, listBoxDef_s *listPtr);
void __cdecl UI_OverrideCursorPos_FileShareMySlots(int localClientNum, int contextIndex, listBoxDef_s *listPtr);
void __cdecl UI_OverrideCursorPos_Invites(int contextIndex, listBoxDef_s *listPtr);
void __cdecl UI_OverrideCursorPos_Playlists(int contextIndex, listBoxDef_s *listPtr);
void __cdecl UI_OverrideCursorPos_Categories(int contextIndex, listBoxDef_s *listPtr);
void __cdecl UI_OverrideCursorPos_Gamemodes(int contextIndex, listBoxDef_s *listPtr);
void __cdecl UI_OverrideCursorPos_Servers(int localClientNum, int contextIndex, itemDef_s *item, listBoxDef_s *listPtr);
void __cdecl UI_OverrideCursorPos_Leaderboards(
                int localClientNum,
                int contextIndex,
                itemDef_s *item,
                listBoxDef_s *listPtr);
void __cdecl UI_OverrideCursorPos_DynamicMenu(int contextIndex, listBoxDef_s *listPtr);
void __cdecl UI_OverrideCursorPos_Default(int localClientNum, int contextIndex, float feederID, listBoxDef_s *listPtr);
void __cdecl UI_FeederSelection(int localClientNum, int contextIndex, float feederID, int index);
void __cdecl UI_FeederSelection_AllMaps(int contextIndex, float feederID, int index);
void __cdecl UI_FeederSelection_Servers(int contextIndex, float feederID, int index);
void __cdecl UI_FeederSelection_Invites(int contextIndex, float feederID, int index);
void __cdecl UI_FeederSelection_Playlists(int localClientNum, int contextIndex, float feederID, int index);
void __cdecl UI_FeederSelection_Categories(int contextIndex, float feederID, int index);
char __cdecl UI_FeederDataExtended(
                int localClientNum,
                itemDef_s *item,
                const char *field,
                char **stringResult,
                float *floatResult);
char __cdecl UI_FeederDataExtended_OnlineFriends(
                int controllerIndex,
                int index,
                const char *field,
                char **stringResult);
char __cdecl UI_FeederDataExtended_RecentPlayers(
                int controllerIndex,
                int index,
                const char *field,
                char **stringResult);
char __cdecl UI_FeederDataExtended_DynamicMenu(
                int index,
                listBoxDef_s *listBox,
                const char *field,
                char **stringResult);
void __cdecl UI_AddToFeeder(itemDef_s *item, const char *eventName, int argCount, const char **args);
void __cdecl UI_AddToFeederExtended(
                itemDef_s *item,
                const char *onActionEvent,
                const char *onFocusEvent,
                bool disableCondition,
                int argCount,
                const char **args);
void __cdecl UI_ChangeRowStatus(itemDef_s *item, int rowToChange, int newStatus);
void __cdecl UI_ClearFeeder(int contextIndex, itemDef_s *item, bool resetCursorPos);
void __cdecl UI_RaiseFeederEvent(int localClientNum, itemDef_s *item, const char *eventName);
char __cdecl UI_FeederData(
                int localClientNum,
                itemDef_s *item,
                const char *field,
                char **stringResult,
                float *numberResult);


static const float newUIfeederHighlight[4] = { 1.0, 1.0, 1.0, 0.2 };
static const float newUIFeederBackgroundColor[4] = { 0.0, 0.0, 0.0, 0.2 };
static const float newUIGrey[4] = { 0.40000001, 0.40000001, 0.40000001, 1.0 };
static const float newUIRed[4] = { 0.73000002, 0.19, 0.19, 1.0 };
static const float newUIGreen[4] = { 0.41999999, 0.68000001, 0.46000001, 1.0 };
