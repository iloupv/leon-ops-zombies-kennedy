#pragma once
#include "ui_shared.h"

struct conditionStackMember_t // sizeof=0x24
{                                       // XREF: conditionStack_t/r
    ExpressionStatement condition;
    bool fireOnTrue;
    // padding byte
    // padding byte
    // padding byte
    int level;
    int blockID;
    int constructID;
    int type;
};

struct conditionStack_t // sizeof=0x904
{                                       // XREF: .data:g_conditionStack/r
    conditionStackMember_t members[64];
    int size;                           // XREF: PC_ResetConditionStack(void)+3/w
                                        // PC_CndStackPush(int,ExpressionStatement,bool,bool,int,int,int)+3/r ...
};

struct nestingCounts_t // sizeof=0x8
{                                       // XREF: .data:g_nestingCounts/r
    int bracketNestingLevel;            // XREF: PC_EventScript_Parse+8C/w
    int ifNestingLevel;                 // XREF: PC_EventScript_Parse+96/w
};

struct loadAssets_t // sizeof=0x10
{                                       // XREF: $CD64A558AFC89A5F4974E935559855BB/r
    float fadeClamp;                    // XREF: Menu_Init+4F/r
    int fadeCycle;                      // XREF: Asset_Parse+100/o
                                        // Menu_Init+62/r
    float fadeAmount;                   // XREF: Asset_Parse+13E/o
                                        // Menu_Init+29/r
    float fadeInAmount;                 // XREF: Asset_Parse+176/o
                                        // Menu_Init+3C/r
};

void __cdecl Menus_FreeAllMemory(UiContext *dc);
void __cdecl Menu_FreeMemory(menuDef_t *menu);
void __cdecl Menu_FreeItemMemory(itemDef_s *item);
void __cdecl UI_MapLoadInfo(const char *filename);
void __cdecl UI_ParseMenuMaterial(const char *key, char *value);
void __cdecl PC_ResetConditionStack();
int __cdecl PC_CndStackPush(
                int handle,
                ExpressionStatement expressionStatement,
                bool fireOnTrue,
                bool incrementLevel,
                int blockID,
                int constructID,
                int type);
int __cdecl PC_CndStackModifyTop();
int __cdecl PC_CndStackPopLevel();
int __cdecl PC_CndStackAddConditionsToScript(GenericEventScript **baseScript, char *accumulatedScriptCommands);
int __cdecl MenuParse_name(menuDef_t *menu, int handle);
int __cdecl PC_String_Parse(int handle, const char **out);
int    MenuParse_fullscreen(menuDef_t *menu, int handle);
int    PC_Int_Parse(int handle, int *i);
void PC_SourceError(int handle, const char *format, ...);
int    PC_Int_Expression_Parse(int handle, int *i);
int    MenuParse_rect(menuDef_t *menu, int handle);
int    PC_Rect_Parse(int handle, rectDef_s *r);
int    PC_Float_Parse(int handle, float *f);
int    PC_Float_Expression_Parse(int handle, float *f);
int __cdecl PC_Flag_ParseOptional(int handle, int *i);
int    PC_Byte_Parse(int handle, unsigned __int8 *b);
int __cdecl MenuParse_visible(menuDef_t *menu, int handle);
int __cdecl MenuParse_visiblityBits(menuDef_t *menu, int handle);
int __cdecl MenuParse_onFocus(menuDef_t *menu, int handle);
int __cdecl PC_EventScript_Parse(int handle, GenericEventScript **baseScript);
int __cdecl MenuParse_leaveFocus(menuDef_t *menu, int handle);
int __cdecl MenuParse_onOpen(menuDef_t *menu, int handle);
int __cdecl MenuParse_onClose(menuDef_t *menu, int handle);
int __cdecl MenuParse_onESC(menuDef_t *menu, int handle);
int __cdecl MenuParse_execExp(menuDef_t *menu, int handle);
int    MenuParse_border(menuDef_t *menu, int handle);
int    MenuParse_borderSize(menuDef_t *menu, int handle);
int    MenuParse_backcolor(menuDef_t *menu, int handle);
int    PC_Color_Parse(int handle, float (*c)[4]);
int    MenuParse_forecolor(menuDef_t *menu, int handle);
int    MenuParse_bordercolor(menuDef_t *menu, int handle);
int    MenuParse_focuscolor(menuDef_t *menu, int handle);
int    MenuParse_disablecolor(menuDef_t *menu, int handle);
int    MenuParse_outlinecolor(menuDef_t *menu, int handle);
int __cdecl MenuParse_background(menuDef_t *menu, int handle);
int    ItemParse_ownerdrawFlag(itemDef_s *item, int handle);
int    MenuParse_ownerdraw(menuDef_t *menu, int handle);
int __cdecl MenuParse_popup(menuDef_t *menu, int handle);
int __cdecl MenuParse_outOfBounds(menuDef_t *menu, int handle);
int __cdecl MenuParse_soundLoop(menuDef_t *menu, int handle);
int    MenuParse_fadeClamp(menuDef_t *menu, int handle);
int    MenuParse_fadeAmount(menuDef_t *menu, int handle);
int    MenuParse_fadeInAmount(menuDef_t *menu, int handle);
int    MenuParse_fadeCycle(menuDef_t *menu, int handle);
int __cdecl MenuParse_itemDef(menuDef_t *menu, int handle);
void __cdecl Item_Init(itemDef_s *item, int imageTrack);
void __cdecl Window_Init(windowDef_t *w);
int __cdecl Item_Parse(int handle, itemDef_s *item);
void __cdecl Item_PostParse(itemDef_s *item);
void __cdecl Item_InitControls(itemDef_s *item);
void __cdecl Item_ValidateTypeData(itemDef_s *item, int handle);
int __cdecl MenuParse_execKey(menuDef_t *menu, int handle);
int __cdecl PC_Char_Parse(int handle, char *out);
int    MenuParse_execKeyInt(menuDef_t *menu, int handle);
int    MenuParse_blurWorld(menuDef_t *menu, int handle);
int __cdecl MenuParse_legacySplitScreenScale(menuDef_t *menu, int handle);
int __cdecl SetItemStaticFlag(menuDef_t *menu, int handle, int flag);
int __cdecl MenuParse_hiddenDuringScope(menuDef_t *menu, int handle);
int __cdecl MenuParse_hiddenDuringFlashbang(menuDef_t *menu, int handle);
int __cdecl MenuParse_hiddenDuringUI(menuDef_t *menu, int handle);
int __cdecl MenuParse_allowedBinding(menuDef_t *menu, int handle);
int __cdecl MenuParse_allowSignIn(menuDef_t *menu, int handle);
int    MenuParse_ui3dWindowId(menuDef_t *menu, int handle);
int    MenuParse_priority(menuDef_t *menu, int handle);
int    MenuParse_openSlideSpeed(menuDef_t *menu, int handle);
int    MenuParse_closeSlideSpeed(menuDef_t *menu, int handle);
int    MenuParse_openSlideDirection(menuDef_t *menu, int handle);
int    MenuParse_closeSlideDirection(menuDef_t *menu, int handle);
int    MenuParse_openFadingTime(menuDef_t *menu, int handle);
int    MenuParse_closeFadingTime(menuDef_t *menu, int handle);
int    MenuParse_control(menuDef_t *menu, int handle);
int    ItemParse_frame(itemDef_s *item, int handle);
int    WindowParse_frame(windowDef_t *window, int handle);
void __cdecl Menu_SetupKeywordHash();
int __cdecl ItemParse_name(itemDef_s *item, int handle);
int __cdecl ItemParse_focusSound(itemDef_s *item, int handle);
int __cdecl ItemParse_text(itemDef_s *item, int handle);
int __cdecl ItemParse_textfile(itemDef_s *item, int handle);
char *__cdecl UI_FileText(char *fileName);
int __cdecl ItemParse_textsavegame(itemDef_s *item, int handle);
int __cdecl ItemParse_textcinematicsubtitle(itemDef_s *item, int handle);
int __cdecl ItemParse_group(itemDef_s *item, int handle);
int    ItemParse_rect(itemDef_s *item, int handle);
int    ItemParse_origin(itemDef_s *item, int handle);
int    ItemParse_style(itemDef_s *item, int handle);
int __cdecl ItemParse_decoration(itemDef_s *item, int handle);
int __cdecl ItemParse_notselectable(itemDef_s *item, int handle);
int __cdecl ItemParse_noScrollBars(itemDef_s *item, int handle);
int __cdecl ItemParse_noBlinkingHighlight(itemDef_s *item, int handle);
int __cdecl ItemParse_usePaging(itemDef_s *item, int handle);
int __cdecl ItemParse_autowrapped(itemDef_s *item, int handle);
int __cdecl ItemParse_horizontalscroll(itemDef_s *item, int handle);
int    ItemParse_type(itemDef_s *item, int handle);
int ItemParse_elementwidth(itemDef_s *item, int handle);
int ItemParse_elementheight(itemDef_s *item, int handle);
int ItemParse_special(itemDef_s *item, int handle);
int    ItemParse_elementtype(itemDef_s *item, int handle);
int    ItemParse_columns(itemDef_s *item, int handle);
int __cdecl PC_Int_ParseLine(int handle, int *i);
int    ItemParse_menuItemsDef(itemDef_s *item, int handle);
int    ItemParse_menuColDef(
                listBoxDef_s *listPtr,
                int handle,
                MenuCell *cells,
                int cellCount);
int    ItemParse_menuarea(itemDef_s *item, int handle);
int    ItemParse_userarea(itemDef_s *item, int handle);
int ItemParse_border(itemDef_s *item, int handle);
int ItemParse_bordersize(itemDef_s *item, int handle);
int __cdecl ItemParse_visible(itemDef_s *item, int handle);
int __cdecl ItemParse_visiblityBits(itemDef_s *item, int handle);
int ItemParse_ownerdraw(itemDef_s *item, int handle);
int ItemParse_align(itemDef_s *item, int handle);
int    ItemParse_textalign(itemDef_s *item, int handle);
bool __cdecl ItemParse_IsValidTextAlignment(unsigned int textAlignMode);
int ItemParse_textalignx(itemDef_s *item, int handle);
int     ItemParse_textaligny(itemDef_s *item, int handle);
int     ItemParse_textscale(itemDef_s *item, int handle);
int     ItemParse_textstyle(itemDef_s *item, int handle);
int     ItemParse_rotation(itemDef_s *item, int handle);
int     ItemParse_textfont(itemDef_s *item, int handle);
int    ItemParse_backcolor(itemDef_s *item, int handle);
int    ItemParse_forecolor(itemDef_s *item, int handle);
int    ItemParse_bordercolor(itemDef_s *item, int handle);
int __cdecl ItemParse_modal(itemDef_s *item, int handle);
int ItemParse_outlinecolor(itemDef_s *item, int handle);
int __cdecl ItemParse_background(itemDef_s *item, int handle);
int __cdecl ItemParse_doubleClick(itemDef_s *item, int handle);
int __cdecl ItemParse_rightClick(itemDef_s *item, int handle);
int __cdecl ItemParse_onEnter(itemDef_s *item, int handle);
int __cdecl ItemParse_onFocus(itemDef_s *item, int handle);
int __cdecl ItemParse_leaveFocus(itemDef_s *item, int handle);
int __cdecl ItemParse_mouseEnter(itemDef_s *item, int handle);
int __cdecl PC_Script_Parse(int handle, const char **out);
int __cdecl ItemParse_mouseExit(itemDef_s *item, int handle);
int __cdecl ItemParse_mouseEnterText(itemDef_s *item, int handle);
int __cdecl ItemParse_mouseExitText(itemDef_s *item, int handle);
int __cdecl ItemParse_action(itemDef_s *item, int handle);
int __cdecl ItemParse_accept(itemDef_s *item, int handle);
int __cdecl ItemParse_dvarTest(itemDef_s *item, int handle);
int __cdecl ItemParse_dvar(itemDef_s *item, int handle);
int    ItemParse_maxChars(itemDef_s *item, int handle);
int __cdecl Item_Parse_maxCharsGotoNext(itemDef_s *item, int handle);
int    ItemParse_maxPaintChars(itemDef_s *item, int handle);
int ItemParse_dvarFloat(itemDef_s *item, int handle);
int __cdecl ItemParse_dvarStrList(itemDef_s *item, int handle);
int    ItemParse_dvarFloatList(itemDef_s *item, int handle);
int __cdecl ItemParse_dvarEnumList(itemDef_s *item, int handle);
int __cdecl ItemParse_actionOnEnterPressOnly(itemDef_s *item, int handle);
int __cdecl ItemParse_enableDvar(itemDef_s *item, int handle);
int __cdecl ItemParse_disableDvar(itemDef_s *item, int handle);
int __cdecl ItemParse_showDvar(itemDef_s *item, int handle);
int __cdecl ItemParse_hideDvar(itemDef_s *item, int handle);
int __cdecl ItemParse_focusDvar(itemDef_s *item, int handle);
int __cdecl ItemParse_onEvent(itemDef_s *item, int handle);
int __cdecl ItemParse_execKey(itemDef_s *item, int handle);
int    ItemParse_execKeyInt(itemDef_s *item, int handle);
int __cdecl ItemParse_execExp(itemDef_s *item, int handle);
int    ItemParse_gameMsgWindowIndex(itemDef_s *item, int handle);
int    ItemParse_gameMsgWindowMode(itemDef_s *item, int handle);
int    ItemParse_selectBorder(itemDef_s *item, int handle);
int    ItemParse_elementHighlightColor(itemDef_s *item, int handle);
int    ItemParse_elementBackgroundColor(itemDef_s *item, int handle);
int    ItemParse_disableColor(itemDef_s *item, int handle);
int    ItemParse_focusColor(itemDef_s *item, int handle);
int __cdecl ItemParse_selectIcon(itemDef_s *item, int handle);
int __cdecl ItemParse_backgroundItemListbox(itemDef_s *item, int handle);
int __cdecl ItemParse_highlightTexture(itemDef_s *item, int handle);
int __cdecl ItemParse_onListboxSelectionChange(itemDef_s *item, int handle);
int    ItemParse_ui3dWindowId(itemDef_s *item, int handle);
int __cdecl ItemParse_state(itemDef_s *item, int handle);
void __cdecl Item_SetupKeywordHash();
MenuList *__cdecl UI_LoadMenu(const char *menuFile, int imageTrack);
MenuList * UI_LoadMenu_LoadObj(char *menuFile, int imageTrack);
char    UI_ParseMenuInternal(char *menuFile, int imageTrack);
int    Asset_Parse(int handle);
char __cdecl Menu_New(int handle, int imageTrack);
void __cdecl Menu_Init(menuDef_t *menu, int imageTrack);
int __cdecl Menu_Parse(int handle, menuDef_t *menu);
void __cdecl Menu_PostParse(menuDef_t *menu);
MenuList *__cdecl UI_LoadMenus(const char *menuFile, int imageTrack);
MenuList * UI_LoadMenus_LoadObj(char *menuFile, int imageTrack);
int    Load_Menu(const char **p, int imageTrack);
MenuList *__cdecl UI_LoadMenus_FastFile(const char *menuFile);
int __cdecl KeywordHash_KeySeed(const char *keyword, int hashCount, int seed);
