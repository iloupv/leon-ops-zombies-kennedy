#pragma once

struct uiInfo_s;
struct listBoxDef_s;
struct rectDef_s;
struct Font_s;

void __cdecl UI_ClearDisplayedServers();
void __cdecl UI_BuildServerDisplayList(int localClientNum, uiInfo_s *uiInfo, int force);
int __cdecl UI_BinaryServerInsertion(int num, int nameNum);
void __cdecl UI_InsertServerIntoDisplayList(int num, int position, int nameNum);
void __cdecl UI_BuildServerStatus(int localClientNum, uiInfo_s *uiInfo, int force);
void __cdecl UI_BuildServerStatusScoreBoard(int localClientNum, uiInfo_s *uiInfo, int force);
void __cdecl UI_DoServerRefresh(int localClientNum, uiInfo_s *uiInfo);
void __cdecl UI_StartServerRefresh(int localClientNum, int contextIndex, int full);
void __cdecl UI_StopServerRefresh();
void __cdecl UI_ServersSort(__int64 column);
int __cdecl UI_ServersQsortCompare(int *arg1, int *arg2);
void __cdecl UI_AddServerToFavoritesList(const char *pszName, const char *pszAddress);
void __cdecl UI_AddServerToFavoritesList(unsigned __int64 uid);
void __cdecl UI_RemoveServerFromFavoritesList();
// local variable allocation has failed, the output may be wrong!
void __cdecl UI_DrawServerRefreshDate(
                int contextIndex,
                rectDef_s *rect,
                Font_s *font,
                float scale,
                float *color,
                int textStyle);
void __cdecl UI_UpdateServerToolTips(int localClientNum, listBoxDef_s *listPtr);

extern int ui_serverFilterType;