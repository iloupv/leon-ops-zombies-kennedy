#pragma once
#include <ui/ui_utils.h>
#include <client/cl_milestone.h>
#include <game/teams.h>
#include <qcommon/msg_mp.h>

int __cdecl UI_GetMilestoneType(float feederID);
int __cdecl UI_Project_FeederCount(int localClientNum, int contextIndex, float feederID, listBoxDef_s *listPtr);
int __cdecl UI_GetBasicTrainingGameTypeCount();
int __cdecl UI_FeederCount_WeaponGroups();
int __cdecl UI_FeederCount_CustomClasses(int localClientNum);
int __cdecl UI_FeederCount_KillstreakNumKills();
char *__cdecl UI_FeederItemText_Maps(int controllerIndex, itemDef_s *item, int index, int column, Material **handle);
char *__cdecl UI_FeederItemText_CustomGametypes(int index);
const char *__cdecl UI_FeederItemText_GametypesBase(
                int contextIndex,
                int controllerIndex,
                int index,
                int column,
                itemDef_s *item,
                listBoxDef_s *listPtr,
                Material **handle);
int __cdecl UI_GetBasicTrainingGametypeIdForNum(int num);
char *__cdecl UI_FeederItemText_GametypesInGame(
                int contextIndex,
                int controllerIndex,
                int index,
                int column,
                listBoxDef_s *listPtr,
                Material **handle);
char *__cdecl UI_FeederItemText_SystemLinkLobbyMembers(int localClientNum);
char *__cdecl UI_FeederItemText_WeaponOptions(
                int localClientNum,
                int contextIndex,
                int controllerIndex,
                float feederID,
                int index,
                int column,
                listBoxDef_s *listPtr,
                Material **handle);
char *__cdecl UI_Feeder_HandleItemList(
                int controllerIndex,
                int itemIndex,
                int column,
                bool isSelectedItem,
                bool itemLocked,
                int cost,
                const char *itemName,
                Material **handle);
void __cdecl UI_FeederItemText_CommonListColorHandler(int controllerIndex, bool isSelectedItem, bool isGrey, int cost);
char *__cdecl UI_FeederItemText_Attachments(
                int contextIndex,
                int controllerIndex,
                float feederID,
                int index,
                int column,
                itemDef_s *item,
                Material **handle);
char *__cdecl UI_ChallengeCompletionStatus(int column, int currentValue, int targetValue, bool *useOwnerDraw);
char *__cdecl UI_PrepareStatsMilestoneName(
                int challengeIndex,
                int milestone,
                const char *statName,
                const char *itemName);
struct _CustomClassDescription *__cdecl UI_Project_FeederItemText(
                int localClientNum,
                int contextIndex,
                itemDef_s *item,
                float feederID,
                itemGroup_t index,
                int column,
                Material **handle,
                float *angle,
                bool *useOwnerDraw);
char *__cdecl UI_FeederItemText_WeaponGroups(
                int controllerIndex,
                int contextIndex,
                itemDef_s *item,
                itemGroup_t index,
                int column,
                Material **handle);
const char *__cdecl UI_FeederItemText_ClanTagFeatures(
                int controllerIndex,
                int contextIndex,
                itemDef_s *item,
                int index,
                int column,
                Material **handle);
char *__cdecl UI_FeederItemText_ClanTagFeaturesEdit(int controllerIndex);
const char *__cdecl UI_FeederItemText_CustomClasses(int controllerIndex, int contextIndex, itemDef_s *item, int index);
_CustomClassDescription *__cdecl UI_FeederItemText_CustomGametypeClasses(
                int controllerIndex,
                int contextIndex,
                itemDef_s *item,
                int index);
char *__cdecl UI_FeederItemText_ChallengesPerks(
                int contextIndex,
                int controllerIndex,
                float feederID,
                int index,
                int column,
                itemDef_s *item,
                Material **handle);
char *__cdecl UI_FeederItemText_ItemInSlot(
                int contextIndex,
                int controllerIndex,
                float feederID,
                int index,
                int column,
                itemDef_s *item,
                Material **handle);
char *__cdecl UI_FeederItemText_CustomPerksInSlot(
                int contextIndex,
                int controllerIndex,
                float feederID,
                int index,
                int column,
                itemDef_s *item,
                Material **handle);
char *__cdecl UI_FeederItemText_ChallengesKillstreaks(
                int contextIndex,
                int controllerIndex,
                itemDef_s *item,
                float feederID,
                int index,
                int column,
                listBoxDef_s *listPtr,
                Material **handle);
char *__cdecl UI_FeederItemText_SortedItems(
                int contextIndex,
                int controllerIndex,
                itemDef_s *item,
                float feederID,
                int index,
                int column,
                listBoxDef_s *listPtr,
                Material **handle);
const char *__cdecl UI_FeederItemText_CustomKillstreaks(
                int contextIndex,
                int controllerIndex,
                itemDef_s *item,
                float feederID,
                int index,
                int column,
                listBoxDef_s *listPtr);
const char *__cdecl UI_FeederItemText_AARScoreboard(
                int contextIndex,
                int controllerIndex,
                int index,
                int column,
                listBoxDef_s *listPtr,
                Material **handle,
                float feederID);
const char *__cdecl UI_FeederItemText_InGamePlayerList(
                int contextIndex,
                int controllerIndex,
                int index,
                int column,
                Material **handle,
                team_t team);
char *__cdecl UI_FeederItemText_InGamePlayers(
                int contextIndex,
                int controllerIndex,
                int index,
                int column,
                Material **handle,
                int feederID,
                itemDef_s *item);
char *__cdecl UI_FeederItemText_Contracts(
                int contextIndex,
                int controllerIndex,
                int index,
                int column,
                itemDef_s *item,
                listBoxDef_s *listPtr,
                Material **handle);
char *__cdecl UI_FeederItemText_PersonalBests(int contextIndex, int controllerIndex, unsigned int index, int column);
char *__cdecl UI_FeederItemText_WeaponGroupMilestones(
                int contextIndex,
                int controllerIndex,
                unsigned int index,
                int column,
                listBoxDef_s *listPtr,
                Material **handle,
                bool *useOwnerDraw);
char *__cdecl UI_FeederItemText_StatsMilestones(
                int contextIndex,
                int controllerIndex,
                unsigned int index,
                int column,
                listBoxDef_s *listPtr,
                Material **handle,
                bool *useOwnerDraw,
                statsMilestoneTypes_t milestoneType);
char *__cdecl UI_FeederItemText_GlobalChallenges(
                int contextIndex,
                int controllerIndex,
                unsigned int index,
                int column,
                listBoxDef_s *listPtr,
                Material **handle,
                bool *useOwnerDraw);
char *__cdecl UI_FeederItemText_ChallengesAttachments(
                int contextIndex,
                int controllerIndex,
                unsigned int index,
                int column,
                listBoxDef_s *listPtr,
                Material **handle,
                bool *useOwnerDraw);
const char *__cdecl UI_FeederItemText_KillstreakNumKills(
                int contextIndex,
                int controllerIndex,
                int index,
                int column,
                listBoxDef_s *listPtr,
                Material **handle);
void __cdecl UI_Project_FeederItemOwnerDraw(
                int localClientNum,
                int contextIndex,
                float feederID,
                const rectDef_s *rect,
                unsigned int index);
char __cdecl UI_Project_FeederItemColor(
                int controllerIndex,
                int contextIndex,
                itemDef_s *item,
                float feederID,
                int index,
                int column,
                float *color,
                bool onFocus);
char __cdecl UI_FeederItemColor_Contracts(
                int controllerIndex,
                int contextIndex,
                itemDef_s *item,
                int index,
                int column,
                float *color);
char __cdecl UI_FeederItemColor_ItemList(
                int controllerIndex,
                int contextIndex,
                itemDef_s *item,
                int index,
                int column,
                int cost,
                bool itemPurchased,
                bool itemLocked,
                bool itemClassified,
                float *color);
char __cdecl UI_FeederItemColor_ItemsInSlot(
                int controllerIndex,
                int contextIndex,
                itemDef_s *item,
                int index,
                int column,
                float *color);
char __cdecl UI_FeederItemColor_WeaponOptions(
                int controllerIndex,
                int contextIndex,
                itemDef_s *item,
                int index,
                int column,
                float *color);
bool __cdecl UI_FeederItemColor_InGamePlayerList(
                int controllerIndex,
                int contextIndex,
                itemDef_s *item,
                int index,
                int column,
                float *color,
                bool onFocus,
                team_t team);
bool __cdecl UI_FeederItemColor_InGamePlayers(
                int controllerIndex,
                int contextIndex,
                itemDef_s *item,
                unsigned int index,
                int column,
                float *color,
                bool onFocus,
                int feederType);
char __cdecl UI_FeederItemColor_GlobalMilestones(
                int controllerIndex,
                int contextIndex,
                itemDef_s *item,
                int index,
                int column,
                float *color);
char __cdecl UI_FeederItemColor_PerksMilestones(
                int controllerIndex,
                int contextIndex,
                itemDef_s *item,
                int index,
                int column,
                float *color);
char __cdecl UI_FeederItemColor_StatsMilestones(
                int controllerIndex,
                int contextIndex,
                itemDef_s *item,
                unsigned int index,
                int column,
                float *color,
                bool onFocus,
                int milestoneType);
bool __cdecl UI_FeederItemColor_AARScoreboard(
                int controllerIndex,
                int contextIndex,
                float feederID,
                itemDef_s *item,
                int index,
                int column,
                float *color);
char __cdecl UI_FeederItemColor_KillstreakNumKills(
                int controllerIndex,
                int contextIndex,
                itemDef_s *item,
                int index,
                int column,
                float *color);
int __cdecl UI_FeederDoubleClick(int localClientNum, int contextIndex, float feederID, itemGroup_t index);
void __cdecl UI_FeederDoubleClickDlcMaps(int localClientNum, int contextIndex, int index);
bool __cdecl UI_FeederItemEnabled(int localClientNum, int contextIndex, float feederID, int index);
Material *__cdecl UI_FeederItemImage(float feederID, int index);
char *__cdecl UI_GetMapLoadNameForCurrentIndex(int index);
void __cdecl UI_OverrideCursorPos_Maps(int contextIndex, listBoxDef_s *listPtr);
void __cdecl UI_OverrideCursorPos_CustomGametypes(int contextIndex, listBoxDef_s *listPtr);
int __cdecl UI_GetBasicTrainingGametypeNumFromId(int id);
int __cdecl UI_CustomGametypes_GetSelectedGameTypeIndex();
bool __cdecl UI_CustomGametypes_IsInvalidCursorPos(int cursorPos);
void __cdecl UI_OverrideCursorPos_Gametypes(
                int localClientNum,
                int contextIndex,
                listBoxDef_s *listPtr,
                float feederID);
bool __cdecl UI_Project_OverrideCursorPos(int localClientNum, int contextIndex, itemDef_s *item);
void __cdecl UI_OverrideCursorPos_FriendsStatsCompare(int localClientNum, int contextIndex, listBoxDef_s *listPtr);
void __cdecl UI_OverrideCursorPos_GameVariants(
                int localClientNum,
                int contextIndex,
                listBoxDef_s *listPtr,
                int feederID);
void __cdecl UI_OverrideCursorPos_ItemsInSlot(
                int localClientNum,
                int contextIndex,
                listBoxDef_s *listPtr,
                int feederID);
void __cdecl UI_OverrideCursorPos_Contracts(
                int localClientNum,
                int contextIndex,
                itemDef_s *item,
                listBoxDef_s *listPtr);
void __cdecl UI_FeederSelection_Maps(int contextIndex, float feederID, int index);
void __cdecl UI_FeederSelection_CustomGametypes(int contextIndex, float feederID, int index);
void __cdecl UI_FeederSelection_GametypesBase(int localClientNum, int contextIndex, float feederID, int index);
void __cdecl UI_FeederSelection_GametypesInGame(int localClientNum, int contextIndex, float feederID, int index);
void __cdecl UI_Project_FeederSelection(int localClientNum, int contextIndex, float feederID, int index);
char __cdecl UI_Project_FeederDataExtended(
                int controllerIndex,
                int contextIndex,
                int feederID,
                listBoxDef_s *listBox,
                const char *field,
                char **stringResult);
char __cdecl UI_FeederDataExtended_FriendsStatsCompare(
                int controllerIndex,
                int index,
                const char *field,
                char **stringResult);
char __cdecl UI_FeederDataExtended_InGamePlayers(
                int controllerIndex,
                unsigned int index,
                int feederType,
                const char *field,
                char **stringResult);
char __cdecl UI_FeederDataExtended_AARScoreboard(
                int controllerIndex,
                int index,
                int feederType,
                const char *field,
                char **stringResult);
void __cdecl UI_FeederBackColor(
                int localClientNum,
                int contextIndex,
                itemDef_s *item,
                float feederID,
                int index,
                float *color);
