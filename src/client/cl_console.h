#pragma once
#include <gfx_d3d/r_font.h>
#include <gfx_d3d/r_material.h>
#include "screen_placement.h"
#include <universal/com_stringtable.h>
#include <universal/dvar.h>

enum
{
    COLOR_FIRST = 48,
    //... BO1TODO
    COLOR_LAST = 64,
};

enum e_restricted_initState : __int32
{                                       // XREF: ?Con_Restricted_SetState@@YAXW4e_restricted_initState@@@Z/r
    RESTRICTED_DISABLED    = 0x0,
    RESTRICTED_CONFIGURING = 0x1,
    RESTRICTED_PROTECTED   = 0x2,
    RESTRICTED_STATE_COUNT = 0x3,
};

enum msgwnd_mode_t : __int32
{                                       // XREF: ?Con_DrawGameMessageWindow@@YAXHHHHHHPAUFont_s@@MQAMHHW4msgwnd_mode_t@@@Z/r
                                        // Con_DrawMessageWindow/r
    MWM_BOTTOMUP_ALIGN_TOP    = 0x0,
    MWM_BOTTOMUP_ALIGN_BOTTOM = 0x1,
    MWM_TOPDOWN_ALIGN_TOP     = 0x2,
    MWM_TOPDOWN_ALIGN_BOTTOM  = 0x3,
};

enum print_msg_dest_t : __int32
{                                       // XREF: ?Con_IsChannelVisible@@YA_NW4print_msg_dest_t@@HH@Z/r
                                        // Con_InitChannelsForDestFromList/r ...
    CON_DEST_CONSOLE    = 0x0,
    CON_DEST_MINICON    = 0x1,
    CON_DEST_ERROR      = 0x2,
    CON_DEST_GAME_FIRST = 0x3,
    CON_DEST_GAME1      = 0x3,
    CON_DEST_GAME2      = 0x4,
    CON_DEST_GAME3      = 0x5,
    CON_DEST_GAME_LAST  = 0x5,
    CON_DEST_COUNT      = 0x6,
};
inline print_msg_dest_t &operator++(print_msg_dest_t &t)
{
    t = static_cast<print_msg_dest_t>((static_cast<int>(t) + 1));
    return t;
}
inline print_msg_dest_t operator++(print_msg_dest_t &t, int)
{
    print_msg_dest_t old = t;
    t = static_cast<print_msg_dest_t>((static_cast<int>(t) + 1));
    return old;
}

struct s_restricted_single // sizeof=0x41
{                                       // XREF: .data:g_restricted/r
    char name[64];
    bool valid;                         // XREF: Con_Restricted_Add(char const *)+E3/w
};

struct MessageLine // sizeof=0x18
{                                       // XREF: MessageBuffer/r
                                        // MessageBuffer/r ...
    int messageIndex;
    int textBufPos;
    int textBufSize;
    int typingStartTime;
    int lastTypingSoundTime;
    int flags;
};

struct Message // sizeof=0x8
{                                       // XREF: MessageBuffer/r
                                        // MessageBuffer/r ...
    int startTime;
    int endTime;
};

struct MessageWindow // sizeof=0x34
{                                       // XREF: MessageBuffer/r
                                        // MessageBuffer/r ...
    MessageLine *lines;                 // XREF: Con_GetTextCopy(char *,int)+23/r
                                        // Con_DrawOutputText+BE/r
    Message *messages;
    char *circularTextBuffer;           // XREF: Con_GetTextCopy(char *,int)+9B/r
                                        // Con_GetTextCopy(char *,int)+BC/r ...
    int textBufSize;                    // XREF: Con_GetTextCopy(char *,int)+47/r
                                        // Con_GetTextCopy(char *,int)+6F/r ...
    int lineCount;                      // XREF: Con_DrawMiniConsole(int,int,int,float)+7F/r
                                        // Con_DrawMiniConsole(int,int,int,float)+92/w ...
    int padding;
    int scrollTime;
    int fadeIn;
    int fadeOut;
    int textBufPos;                     // XREF: Con_GetTextCopy(char *,int)+30/r
    int firstLineIndex;                 // XREF: Con_GetTextCopy(char *,int):loc_54555A/r
                                        // Con_DrawOutputText+A3/r
    int activeLineCount;                // XREF: Con_GetTextCopy(char *,int)+6/r
                                        // Con_FreeFirstMessageWindowLine+8F/r ...
    int messageIndex;
};

struct MessageBuffer // sizeof=0x3EA4
{                                       // XREF: Console/r
    char gamemsgText[3][2048];
    MessageWindow gamemsgWindows[3];
    MessageLine gamemsgLines[3][12];
    Message gamemsgMessages[3][12];
    char miniconText[4096];
    MessageWindow miniconWindow;        // XREF: Con_DrawMiniConsole(int,int,int,float)+7F/r
                                        // Con_DrawMiniConsole(int,int,int,float)+92/w
    MessageLine miniconLines[100];
    Message miniconMessages[100];
    char errorText[1024];
    MessageWindow errorWindow;
    MessageLine errorLines[5];
    Message errorMessages[5];
};

struct Console // sizeof=0x14118
{                                       // XREF: .data:con/r
    int initialized;                    // XREF: Con_OneTimeInit(void)+C47/w
                                        // Con_Init(void)+15F/r ...
    MessageWindow consoleWindow;        // XREF: Con_GetTextCopy(char *,int)+6/r
                                        // Con_GetTextCopy(char *,int):loc_54555A/r ...
    MessageLine consoleLines[1024];     // XREF: Con_OneTimeInit(void)+BE6/o
    Message consoleMessages[1024];      // XREF: Con_OneTimeInit(void)+BEB/o
    char consoleText[32768];            // XREF: Con_OneTimeInit(void)+BE1/o
    char textTempLine[512];             // XREF: CL_ConsolePrint_AddLine+369/w
                                        // CL_ConsolePrint_AddLine+386/w ...
    unsigned int lineOffset;            // XREF: Con_Clear_f+10/w
                                        // CL_ConsolePrint_AddLine+B1/r ...
    int displayLineOffset;              // XREF: Con_Clear_f+1A/w
                                        // Con_FreeFirstMessageWindowLine+72/r ...
    int prevChannel;                    // XREF: CL_ConsolePrint_AddLine+A9/r
                                        // CL_ConsolePrint_AddLine+BE/r ...
    bool outputVisible;                 // XREF: Con_ToggleConsole(void)+3F/w
                                        // Con_ToggleConsoleOutput(void)+3/r ...
    // padding byte
    // padding byte
    // padding byte
    int fontHeight;                     // XREF: Con_CheckResize(void)+11B/w
                                        // Con_CheckResize(void)+120/r ...
    int visibleLineCount;               // XREF: Con_CheckResize(void)+18D/w
                                        // Con_CheckResize(void)+1D4/w ...
    int visiblePixelWidth;              // XREF: Con_CheckResize(void)+1C2/w
                                        // Con_CheckResize(void)+1DE/w ...
    float screenMin[2];                 // XREF: Con_CheckResize(void)+39/w
                                        // Con_CheckResize(void)+72/w ...
    float screenMax[2];                 // XREF: Con_CheckResize(void)+B6/w
                                        // Con_CheckResize(void)+FA/w ...
    MessageBuffer messageBuffer[1];     // XREF: Con_InitMessageBuffer(void)+2B/o
                                        // Con_DrawMiniConsole(int,int,int,float)+7F/r ...
    float color[4];                     // XREF: Con_OneTimeInit(void)+C0A/w
                                        // Con_OneTimeInit(void)+C1A/w ...
};

struct ConDrawInputGlob // sizeof=0x64
{                                       // XREF: .data:conDrawInputGlob/r
    char autoCompleteChoice[64];        // XREF: Con_HasActiveAutoComplete(void)+D/r
                                        // Con_CancelAutoComplete(void)+1D/w ...
    int matchIndex;                     // XREF: Con_Init(void):loc_546C19/w
                                        // Con_CycleAutoComplete(int):loc_5497F0/r ...
    int matchCount;                     // XREF: Con_HasTooManyMatchesToShow(void)+3/r
                                        // Con_CycleAutoComplete(int)+E/r ...
    const char *inputText;              // XREF: Con_DrawInput+149/w
                                        // Con_DrawInput+14E/r ...
    int inputTextLen;                   // XREF: Con_DrawInput+13B/r
                                        // Con_DrawInput+183/w ...
    bool hasExactMatch;                 // XREF: Con_CycleAutoComplete(int)+25/r
                                        // Con_DrawInput+290/w ...
    bool mayAutoComplete;               // XREF: Con_CycleAutoComplete(int)+3/r
                                        // Con_AllowAutoCompleteCycling(bool)+6/w
    // padding byte
    // padding byte
    float x;                            // XREF: Con_DrawInput+B4/w
                                        // Con_DrawInput+D4/r ...
    float y;                            // XREF: Con_DrawInput+CC/w
                                        // Con_DrawInput:loc_54B5B8/r ...
    float leftX;                        // XREF: Con_DrawInput+DC/w
                                        // Con_DrawInput+111/w ...
    float fontHeight;                   // XREF: Con_DrawInput+9C/w
                                        // Con_DrawInput+3E0/r ...
};



void __cdecl Con_ToggleConsole();
void __cdecl Con_GetTextCopy(char *text, int maxSize);
void __cdecl Con_TimeJumped(int localClientNum, int serverTime);
void __cdecl Con_ResetMessageWindowTimes(MessageWindow *msgwnd, int serverTime);
void __cdecl Con_TimeNudged(int localClientNum, int serverTimeNudge);
void __cdecl Con_NudgeMessageWindowTimes(MessageWindow *msgwnd, int serverTimeNudge, int serverTime);
void __cdecl Con_ClearNotify(int localClientNum);
void __cdecl Con_ClearMessageWindow(MessageWindow *msgwnd);
void __cdecl Con_ClearErrors(int localClientNum);
void __cdecl Con_CheckResize();
void __cdecl Con_OneTimeInit();
void __cdecl Con_InitMessageWindow(
                MessageWindow *msgwnd,
                Message *messages,
                MessageLine *lines,
                char *text,
                int lineCount,
                int padding,
                int textPoolSize,
                int scrollTime,
                int fadeIn,
                int fadeOut);
void __cdecl Con_Init();
void __cdecl Con_ChatModePublic_f();
void __cdecl SetupChatField(int localClientNum, int teamChat, int widthInPixels);
void __cdecl Con_ChatModeTeam_f();
void __cdecl Con_Echo_f();
void __cdecl Con_Clear_f();
void __cdecl Con_InitClientAssets();
void __cdecl Con_InitMessageBuffer();
void __cdecl CL_ConsolePrint(int localClientNum, int channel, const char *txt, int duration, int pixelWidth, int flags);
void __cdecl CL_ConsolePrint_AddLine(
                int localClientNum,
                int channel,
                const char *txt,
                int duration,
                int pixelWidth,
                char color,
                int flags);
void __cdecl Con_UpdateNotifyMessage(int localClientNum, unsigned int channel, int duration, int flags);
void __cdecl Con_UpdateNotifyMessageWindow(
                int localClientNum,
                unsigned int channel,
                int duration,
                int flags,
                print_msg_dest_t dest);
int __cdecl Con_GetDefaultMsgDuration(print_msg_dest_t dest);
void __cdecl Con_UpdateMessage(int localClientNum, MessageWindow *msgwnd, int duration);
MessageWindow *__cdecl Con_GetDestWindow(int localClientNum, print_msg_dest_t dest);
void __cdecl Con_UpdateNotifyLine(int localClientNum, unsigned int channel, bool lineFeed, int flags);
void __cdecl Con_UpdateNotifyLineWindow(
                int localClientNum,
                unsigned int channel,
                bool lineFeed,
                int flags,
                print_msg_dest_t dest);
void __cdecl Con_UpdateMessageWindowLine(int localClientNum, MessageWindow *msgwnd, int linefeed, int flags);
void __cdecl Con_FreeFirstMessageWindowLine(MessageWindow *msgwnd);
void __cdecl Con_CopyCurrentConsoleLineText(MessageWindow *msgwnd, MessageLine *msgLine);
bool __cdecl Con_NeedToFreeMessageWindowLine(MessageWindow *msgwnd, int charCount);
int __cdecl PrintTimeTotal(MessageWindow *msgwnd, MessageLine *line);
int __cdecl PrintableCharsCount(const MessageWindow *msgwnd, MessageLine *line);
int __cdecl GetNextValidPrintTimeForLine(int localClientNum, MessageWindow *msgwnd, char flags);
int __cdecl LatestActiveTypewrittenLineIdx(MessageWindow *msgwnd);
int __cdecl PrintTimeWriteOut(MessageWindow *msgwnd, MessageLine *line);
void __cdecl Con_Linefeed(int localClientNum, unsigned int channel, int flags);
const char *__cdecl CL_TextLineWrapPosition(
                const char *txt,
                int bufferSize,
                int pixelsAvailable,
                Font_s *font,
                float scale,
                int lineBroken);
void __cdecl CL_ConsoleFixPosition();
void __cdecl CL_ReviveMessagePrint(
                int localClientNum,
                char *reviveString,
                Material *iconShader,
                float iconWidth,
                float iconHeight,
                bool horzFlipIcon);
int __cdecl CL_AddMessageChar(char *msg, unsigned int msgLen, unsigned int msgMaxLen, char c);
unsigned int __cdecl CL_AddMessageIcon(
                char *msg,
                unsigned int msgLen,
                unsigned int msgMaxLen,
                Material *iconShader,
                float iconWidth,
                float iconHeight,
                bool horzFlipIcon);
void __cdecl CL_DeathMessagePrint(
                int localClientNum,
                char *attackerName,
                char attackerColorIndex,
                char *victimName,
                char victimColorIndex,
                Material *iconShader,
                float iconWidth,
                float iconHeight,
                bool horzFlipIcon);
void __cdecl Con_AutoCompleteFromList(
                const char **strings,
                unsigned int stringCount,
                const char *prefix,
                char *completed,
                unsigned int sizeofCompleted);
const char *__cdecl Con_TokenizeInput();
char __cdecl Con_AnySpaceAfterCommand();
bool __cdecl Con_IsAutoCompleteMatch(const char *query, const char *matchToText, int matchTextLen);
bool __cdecl Con_HasTooManyMatchesToShow();
bool __cdecl Con_IsDvarCommand(const char *cmd);
void __cdecl Con_ForEach(
                bool isDvarCommand,
                void (__cdecl *callbackDvar)(const char *),
                void (__cdecl *callbackCmd)(const char *));
char __cdecl Con_CycleAutoComplete(int step);
bool __cdecl Con_HasActiveAutoComplete();
char __cdecl Con_CommitToAutoComplete();
char __cdecl Con_CancelAutoComplete();
void __cdecl Con_AllowAutoCompleteCycling(bool isAllowed);
void __cdecl Con_DrawGameMessageWindow(
                int localClientNum,
                unsigned int windowIndex,
                int xPos,
                int yPos,
                int horzAlign,
                int vertAlign,
                Font_s *font,
                float fontScale,
                float *color,
                int textStyle,
                char textAlignMode,
                msgwnd_mode_t mode);
void __cdecl Con_DrawMessageWindow(
                int localClientNum,
                MessageWindow *msgwnd,
                int x,
                int y,
                int charHeight,
                int horzAlign,
                int vertAlign,
                Font_s *font,
                float *color,
                int textStyle,
                float msgwndScale,
                msgwnd_mode_t mode,
                char textAlignMode);
void __cdecl Con_DrawMessageWindowNewToOld(
                int localClientNum,
                MessageWindow *msgwnd,
                int x,
                int y,
                int hudCharHeight,
                int horzAlign,
                int vertAlign,
                bool up,
                Font_s *font,
                float *color,
                int textStyle,
                float msgwndScale,
                char textAlignMode);
bool __cdecl Con_ShouldProcessMessage(Message *message, int serverTime);
void __cdecl Con_DrawMessageLineOnHUD(
                int localClientNum,
                const ScreenPlacement *scrPlace,
                int x,
                int y,
                int charHeight,
                int horzAlign,
                int vertAlign,
                Font_s *font,
                const MessageWindow *msgwnd,
                int lineIdx,
                float *color,
                int textStyle,
                float msgwndScale,
                char textAlignMode);
bool __cdecl LineVisible(const MessageWindow *msgwnd, int lineIdx, int time);
void __cdecl TypewriterSounds(int localClientNum, const MessageWindow *msgwnd, MessageLine *line);
void __cdecl Con_CullFinishedLines(int serverTime, MessageWindow *msgwnd);
double __cdecl Con_GetMessageAlpha(Message *message, MessageWindow *msgwnd, int serverTime, bool scrollsIntoPlace);
void __cdecl Con_DrawMessageWindowOldToNew(
                int localClientNum,
                MessageWindow *msgwnd,
                int x,
                int y,
                int charHeight,
                int horzAlign,
                int vertAlign,
                bool up,
                Font_s *font,
                float *color,
                int textStyle,
                float msgwndScale,
                char textAlignMode);
bool __cdecl CL_ShouldntDrawMessageWindow(int localClientNum);
void __cdecl Con_DrawMiniConsole(int localClientNum, int xPos, int yPos, float alpha);
void __cdecl Con_ClearMiniConsole(int localClientNum);
void __cdecl Con_DrawErrors(int localClientNum, int xPos, int yPos, float alpha);
bool __cdecl Con_IsValidGameMessageWindow(unsigned int windowIndex);
bool __cdecl Con_IsGameMessageWindowActive(int localClientNum, unsigned int windowIndex);
void __cdecl Con_DrawSay(int localClientNum, int x, int y);
void __cdecl Con_ToggleConsoleOutput();
void __cdecl Con_DrawConsole(int localClientNum);
void __cdecl Con_DrawSolidConsole(int localClientNum);
void __cdecl Con_DrawInput(int localClientNum);
void __cdecl ConDrawInput_Text(char *str, const float *color);
void __cdecl ConDrawInput_TextAndOver(char *str, const float *color);
int __cdecl ConDrawInput_TextWidth(const char *text);
void __cdecl ConDrawInput_Box(int lines, const float *color);
void __cdecl ConDraw_Box(float x, float y, float w, float h, const float *color);
void __cdecl ConDrawInput_IncrMatchCounter(const char *str);
void __cdecl ConDrawInput_DvarMatch(char *str);
void __cdecl ConDrawInput_TextLimitChars(char *str, int maxChars, const float *color);
void __cdecl ConDrawInput_DetailedDvarMatch(char *str);
void __cdecl ConDrawInput_AutoCompleteArg(const char **stringList, int stringCount);
int __cdecl ConDrawInput_TextFieldFirstArgChar();
int __cdecl ConDrawInput_GetDvarDescriptionLines(const dvar_s *dvar);
void __cdecl ConDrawInput_DetailedCmdMatch(char *str);
void __cdecl ConDrawInput_CmdMatch(char *str);
void __cdecl Con_DrawAutoCompleteChoice(int localClientNum, bool isDvarCommand, const char *originalCommand);
unsigned int __cdecl Con_GetAutoCompleteColorCodedString(
                char *query,
                const char *matchToText,
                int matchTextLen,
                bool isDvarCommand,
                const char *originalCommand,
                char *colorCoded);
int __cdecl Con_GetAutoCompleteColorCodedStringDiscontiguous(
                const char *query,
                const char *matchToText,
                int matchTextLen,
                char *colorCoded);
int __cdecl Con_GetAutoCompleteColorCodedStringContiguous(
                char *query,
                const char *matchToText,
                int matchTextLen,
                char *colorCoded);
void __cdecl Con_DrawInputPrompt(int localClientNum);
void Con_DrawOuputWindow();
void __cdecl Con_DrawOutputScrollBar(float x, float y, float width, float height);
void __cdecl Con_DrawOutputText(float x, float y);
void __cdecl Con_DrawOutputVersion(float x, float y, float width, float height);
char *__cdecl Con_GetVersionString();
void __cdecl Con_PageUp();
void __cdecl Con_PageDown();
void __cdecl Con_Top();
void __cdecl Con_Bottom();
void __cdecl Con_Close(int localClientNum);
bool __cdecl Con_IsActive(int localClientNum);
void __cdecl CL_PlayTextFXPulseSounds(
                int localClientNum,
                int currentTime,
                int strLength,
                int fxBirthTime,
                int fxLetterTime,
                int fxDecayStartTime,
                int *soundTimeKeeper);
void __cdecl Con_Restricted_Add(const char *name);
int __cdecl Con_Restricted_SigleExist(const char *name);
void __cdecl Con_Restricted_SetState(e_restricted_initState state);
void __cdecl Con_Restricted_ShutDown();
e_restricted_initState __cdecl Con_Restricted_GetState();
void __cdecl Con_Restricted_SecureConfigs();
void __cdecl Con_Restricted_InitLists();
void __cdecl Con_Restricted_LoadTable(const StringTable *table);
void __cdecl Con_Restricted_ResetLists();
void Con_Restricted_SetLists();
void Con_Restricted_Reset();
void __cdecl Con_Restricted_ForEach(
                bool isDvarCommand,
                void (__cdecl *callbackDvar)(const char *),
                void (__cdecl *callbackCmd)(const char *));
void __cdecl Con_Restricted_ExecuteBuf(int localClientNum, int controllerIndex, char *buffer);
void __cdecl Con_Restricted_AddBuf(char *buf);
int __cdecl Con_Restricted_RestrictBuf(const char *buf, int start, int length, char *buffer, int buffer_size);
char __cdecl Con_Restricted_ValidateConfig(const char *fileName);
int __cdecl ConDrawInput_CompareStrings(const char **e0, const char **e1);

extern const dvar_t *con_inputBoxColor;
extern const dvar_t *con_inputHintBoxColor;
extern const dvar_t *con_outputBarColor;
extern const dvar_t *con_outputSliderColor;
extern const dvar_t *con_outputWindowColor;
extern const dvar_t *con_gameMsgWindowNMsgTime[3];
extern const dvar_t *con_gameMsgWindowNLineCount[3];
extern const dvar_t *con_gameMsgWindowNScrollTime[3];
extern const dvar_t *con_gameMsgWindowNFadeInTime[3];
extern const dvar_t *con_gameMsgWindowNFadeOutTime[3];
extern const dvar_t *con_gameMsgWindowNSplitscreenScale[3];
extern const dvar_t *con_errormessagetime;
extern const dvar_t *con_minicontime;
extern const dvar_t *con_miniconlines;
extern const dvar_t *con_typewriterEnabledSounds;
extern const dvar_t *con_typewriterPrintSpeed;
extern const dvar_t *con_typewriterDecayStartTime;
extern const dvar_t *con_typewriterDecayDuration;
extern const dvar_t *con_typewriterColorBase;
extern const dvar_t *con_typewriterColorGlowUpdated;
extern const dvar_t *con_typewriterColorGlowCompleted;
extern const dvar_t *con_typewriterColorGlowFailed;
extern const dvar_t *con_typewriterColorGlowCheckpoint;

extern const dvar_t *con_restricted;
extern const dvar_t *con_restricted_access;
extern const dvar_t *con_matchPrefixOnly;

extern field_t g_consoleField;
extern field_t historyEditLines[32];

extern bool con_ignoreMatchPrefixOnly;

extern int g_console_field_width;
extern float g_console_char_height;

static const float con_screenPadding = 4.0f;
static const int con_inputMaxMatchesShown = 24;
static const float defaultGameMessageTimes[3] = { 5.0, 8.0, 5.0 };
static const int defaultGameMessageWindowLineCounts[3] = { 4, 1, 7 };