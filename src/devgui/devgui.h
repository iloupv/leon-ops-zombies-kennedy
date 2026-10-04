#pragma once
#include <universal/dvar.h>
#include <qcommon/graph.h>

#include <cstdint>

enum DevGuiTokenResult : __int32
{                                                                             // XREF: DevGui_ConstructPath_r/r
                                                                                // DevGui_IsValidPath/r ...
        DEVGUI_TOKEN_ERROR = 0x0,
        DEVGUI_TOKEN_MORE    = 0x1,
        DEVGUI_TOKEN_LAST    = 0x2,
};

union DevMenuChild // sizeof=0x4
{                                                                             // XREF: DevGui_ChooseOrigin:loc_598083/r
                                                                                // DevGui_Init(void)+7D/w ...
        const char *command;
        const dvar_s *dvar;
        DevGraph *graph;
        unsigned __int16 menu;
};

struct DevMenuItem // sizeof=0x28
{                                                                             // XREF: devguiGlob_t/r
                                                                                // devguiGlob_t/r
        char label[26];                                         // XREF: DevGui_Init(void)+4B/w
                                                                                // DevGui_Init(void)+59/w
        unsigned __int8 childType;                    // XREF: DevGui_Init(void)+6D/w
        unsigned __int8 childMenuMemory;        // XREF: DevGui_Init(void)+74/w
        __int16 sortKey;
        unsigned __int16 nextSibling;             // XREF: DevGui_Init(void)+86/w
        unsigned __int16 prevSibling;             // XREF: DevGui_Init(void)+8F/w
        unsigned __int16 parent;                        // XREF: DevGui_Init(void)+97/w
        DevMenuChild child;                                 // XREF: DevGui_ChooseOrigin:loc_598083/r
                                                                                // DevGui_Init(void)+7D/w ...
};

struct devguiGlob_t // sizeof=0x14050
{                                                                             // XREF: .data:devguiGlob/r
        DevMenuItem menus[2048];                        // XREF: DevGui_Init(void)+4B/w
                                                                                // DevGui_Init(void)+59/w
        DevMenuItem *nextFreeMenu;                    // XREF: DevGui_CreateMenu:loc_596776/r
                                                                                // DevGui_CreateMenu+3E/w ...
        DevMenuItem topmostMenu;                        // XREF: DevGui_CreateMenu:loc_596834/o
                                                                                // DevGui_FindMenu:loc_596A50/o ...
        bool bindNextKey;                                     // XREF: DevGui_Draw(int):loc_5977B1/r
                                                                                // DevGui_KeyPressed(int)+13/r ...
        bool isActive;                                            // XREF: DevGui_FreeMenu_r+28/w
                                                                                // DevGui_OpenMenu(char const *):loc_5975FC/w ...
        bool isInitialized;                                 // XREF: DevGui_Init(void)+109/w
                                                                                // DevGui_IsInitialized(void)+3/r
        bool editingMenuItem;                             // XREF: DevGui_FreeMenu_r+2F/w
                                                                                // DevGui_OpenMenu(char const *)+EA/w ...
        unsigned __int16 selectedMenu;            // XREF: DevGui_FreeMenu_r:loc_597461/r
                                                                                // DevGui_FreeMenu_r+22/w ...
        // padding byte
        // padding byte
        int selRow;                                                 // XREF: DevGui_OpenMenu(char const *):loc_597643/w
                                                                                // DevGui_DrawSliders+215/r ...
        bool editingKnot;                                     // XREF: DevGui_DrawGraph+345/r
                                                                                // DevGui_Init(void)+B6/w ...
        // padding byte
        // padding byte
        // padding byte
        int top;                                                        // XREF: DevGui_Draw(int)+AF/r
                                                                                // DevGui_ChooseOrigin+E8/r ...
        int bottom;                                                 // XREF: DevGui_DrawMenuVertically+FC/r
                                                                                // DevGui_DrawSliders+10E/r ...
        int left;                                                     // XREF: DevGui_Draw(int):loc_597766/r
                                                                                // DevGui_ChooseOrigin+DE/r ...
        int right;                                                    // XREF: DevGui_ChooseOrigin+F1/r
                                                                                // DevGui_ChooseOrigin+101/r ...
        int sliderWidth;                                        // XREF: DevGui_DrawSliders:loc_5981AE/r
                                                                                // DevGui_DrawGraph:loc_599562/r ...
};

void __cdecl DevGui_AddDvar(const char *path, const dvar_s *dvar);
devguiGlob_t *__cdecl DevGui_GetMenu(unsigned __int16 handle);
unsigned __int16 __cdecl DevGui_ConstructPath_r(unsigned __int16 parent, const char *path);
unsigned __int16 __cdecl DevGui_RegisterMenu(unsigned __int16 parentHandle, const char *label, __int16 sortKey);
int __cdecl DevGui_CreateMenu(uint16_t parentHandle, const char *label, __int16 sortKey);
unsigned __int16 __cdecl DevGui_GetMenuHandle(DevMenuItem *menu);
int __cdecl DevGui_CompareMenus(const DevMenuItem *menu0, const DevMenuItem *menu1);
unsigned __int16 __cdecl DevGui_FindMenu(unsigned __int16 parentHandle, const char *label);
DevGuiTokenResult __cdecl DevGui_PathToken(const char **pathInOut, char *label, __int16 *sortKeyOut);
char __cdecl DevGui_IsValidPath(const char *path);
void __cdecl DevGui_AddCommand(const char *path, char *command);
void __cdecl DevGui_AddGraph(const char *path, DevGraph *graph);
void __cdecl DevGui_RemoveMenu(const char *path);
void __cdecl DevGui_FreeMenu_r(unsigned __int16 handle);
void __cdecl DevGui_OpenMenu(const char *path);
bool __cdecl DevGui_EditableMenuItem(const DevMenuItem *menu);
void __cdecl DevGui_Draw(int localClientNum);
unsigned __int16 __cdecl DevGui_GetMenuParent(unsigned __int16 handle);
void __cdecl DevGui_DrawMenu(unsigned __int16 menuHandle, unsigned __int16 activeChild, int *origin);
void __cdecl DevGui_DrawMenuVertically(const DevMenuItem *menu, unsigned __int16 activeChild, int *origin);
bool __cdecl DevGui_MenuItemDisabled(const DevMenuItem *menu);
int __cdecl DevGui_SubMenuTextWidth();
int __cdecl DevGui_MaxChildMenuWidth(const DevMenuItem *menu);
int __cdecl DevGui_MenuItemWidth(const DevMenuItem *menu);
void __cdecl DevGui_DrawMenuHorizontally(const DevMenuItem *menu, unsigned __int16 activeChild, int *origin);
void __cdecl DevGui_ChooseOrigin(int *origin);
void __cdecl DevGui_DrawSliders(const DevMenuItem *menu);
void __cdecl DevGui_DrawSliderPath(int x, int y);
int __cdecl DevGui_GetSliderPath(unsigned __int16 menuHandle, char *path, int pathLen);
void __cdecl DevGui_DrawDvarDescription(int x, int y, const dvar_s *dvar);
void __cdecl DevGui_DrawSingleSlider(
                int x,
                int y,
                int rowWidth,
                int rowHeight,
                float fraction,
                const unsigned __int8 *knobColor);
void __cdecl DevGui_DrawDvarValue(int x, int y, const dvar_s *dvar);
int __cdecl DevGui_DvarRowCount(const dvar_s *dvar);
void DevGui_DrawBindNextKey();
void __cdecl DevGui_DrawGraph(const DevMenuItem *menu, int localClientNum);
void __cdecl DevGui_Init();
void DevGui_RegisterDvars();
void __cdecl DevGui_Shutdown();
void DevGui_MenuShutdown();
void __cdecl DevGui_KeyPressed(int key);
void __cdecl DevGui_Update(int localClientNum, float deltaTime);
void DevGui_MoveSelectionHorizontally();
void DevGui_MoveLeft();
void DevGui_SelectPrevMenuItem();
void DevGui_SelectTopLevelChild();
void __cdecl DevGui_AdvanceChildNum(int numberToAdvance);
void DevGui_MoveRight();
void DevGui_SelectNextMenuItem();
void DevGui_MoveSelectionVertically();
void DevGui_MoveUp();
void DevGui_MoveDown();
void __cdecl DevGui_Accept(int localClientNum);
void DevGui_Reject();
int DevGui_UpdateSelection();
int DevGui_ScrollUp();
const dvar_s *__cdecl DevGui_SelectedDvar();
int DevGui_ScrollUpInternal();
int DevGui_ScrollDown();
int DevGui_ScrollDownInternal();
void __cdecl DevGui_UpdateDvar(float deltaTime, bool channelLink, bool dvarReset);
double __cdecl DevGui_PickFloatScrollStep(float min, float max);
void __cdecl DevGui_UpdateGraph(int localClientNum, float deltaTime);
void __cdecl DevGui_AddGraphKnot(DevGraph *graph, int localClientNum);
void __cdecl DevGui_RemoveGraphKnot(DevGraph *graph, int localClientNum);
void __cdecl DevGui_Toggle();
bool __cdecl DevGui_IsActive();
bool __cdecl DevGui_IsInitialized();
