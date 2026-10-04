#pragma once
#include <universal/com_expressions.h>
#include <ui/ui_main.h>

struct Material;

struct textExp_s // sizeof=0x10
{                                                                             // XREF: textExp_t/r
        ExpressionStatement textExp;
};

struct gameMsgDef_s // sizeof=0x8
{                                                                             // XREF: gameMsgDef_t/r
        int gameMsgWindowIndex;
        int gameMsgWindowMode;
};


struct CmdArgs // sizeof=0x28EC
{                                                                             // XREF: Cmd_Init(void)+10/o
                                                                                // Com_InitThreadData(int)+50/o ...
        int nesting;                                                // XREF: SV_Cmd_Argc(void)+3/r
                                                                                // SV_Cmd_Argc(void)+E/r ...
        int localClientNum[8];
        int controllerIndex[8];
        struct itemDef_s *itemDef[8];
        int argshift[8];
        int argc[8];                                                // XREF: SV_Cmd_Argc(void)+3A/r
                                                                                // SV_Cmd_Argv(int)+6E/r
        const char **argv[8];                             // XREF: SV_Cmd_Argv(int)+7D/r
        char textPool[8192];
        const char *argvPool[512];
        int usedTextPool[8];
        int totalUsedArgvPool;                            // XREF: SV_Cmd_TokenizeString(char const *)+F/r
        int totalUsedTextPool;
};


struct __declspec(align(4)) cmd_function_s // sizeof=0x18
{                                                                             // XREF: .data:BG_EmblemsFlushResults_VAR/r
                                                                                // .data:BG_EmblemsPurchaseBackgroundByIndex_f_VAR/r ...
        cmd_function_s *next;
        const char *name;
        const char *autoCompleteDir;
        const char *autoCompleteExt;
        void (__cdecl *function)();
        bool consoleAccess;
        // padding byte
        // padding byte
        // padding byte
};

struct CmdText // sizeof=0xC
{                                       // XREF: .data:sv_cmd_text/r
    unsigned __int8 *data;              // XREF: Cbuf_Init(void)+38/w
    int maxsize;                        // XREF: Cbuf_Init(void)+44/w
    int cmdsize;                        // XREF: Cbuf_Init(void)+54/w
};

struct itemDef_s;

void __cdecl Cmd_Wait_f();
void __cdecl Cbuf_Init();
void __cdecl Cbuf_AddText(int localClientNum, const char *text);
void __cdecl Cbuf_InsertText(int localClientNum, const char *text);
void __cdecl Cbuf_AddServerText_f();
void __cdecl Cmd_ExecuteServerString(char *text);
void __cdecl Cmd_ShiftArgs(int count, CmdArgs *args);
void __cdecl Cbuf_SV_Execute();
void __cdecl Cmd_AddServerCommandInternal(const char *cmdName, void (__cdecl *function)(), cmd_function_s *allocedCmd);
void __cdecl Cbuf_ExecuteBuffer_Restricted(int localClientNum, int controllerIndex, char *buffer);
void __cdecl Cbuf_ExecuteBufferInternal(
                int localClientNum,
                int controllerIndex,
                itemDef_s *item,
                char *buffer,
                char ***dvarWhitelistArray,
                bool restrict);
void __cdecl Cmd_ExecSafeDvarSingleCommmandOnly(
                int localClientNum,
                int controllerIndex,
                char *singleCommand,
                const char ***dvarWhitelistArray);
void __cdecl Cbuf_ExecuteBuffer(int localClientNum, int controllerIndex, char *buffer);
void __cdecl Cbuf_ExecuteBufferUI(int localClientNum, int controllerIndex, itemDef_s *item, char *buffer);
void __cdecl Cbuf_Execute(int localClientNum, int controllerIndex);
void __cdecl Cbuf_ExecuteInternal(int localClientNum, int controllerIndex);
void __cdecl Cmd_Vstr_f();
void __cdecl SV_Cmd_ArgvBuffer(int arg, char *buffer, int bufferLength);
void __cdecl Cmd_ArgsBuffer(int start, char *buffer, int bufLength);
void __cdecl Cmd_TokenizeStringWithLimit(const char *text_in, int max_tokens);
void __cdecl Cmd_TokenizeStringKernel(
                int localClientNum,
                int localControllerIndex,
                itemDef_s *item,
                const char *text_in,
                int max_tokens,
                bool evalExpressions,
                CmdArgs *args);
int __cdecl Cmd_TokenizeStringInternal(
                const char *text_in,
                int max_tokens,
                bool evalExpressions,
                const char **argv,
                CmdArgs *argsPriv);
bool __cdecl Cmd_IsWhiteSpaceChar(unsigned __int8 letter);
void __cdecl Cmd_EmitString(const char *str, CmdArgs *argsPriv);
void __cdecl Cmd_EvaluateExpression(const char **text_in, CmdArgs *argsPriv);
itemDef_s *__cdecl Cmd_ItemDef();
void __cdecl AssertCmdArgsConsistency(const CmdArgs *args);
void __cdecl Cmd_TokenizeStringWithLimitNoEval(const char *text_in, int max_tokens);
void __cdecl Cmd_TokenizeString(const char *text_in);
void __cdecl Cmd_TokenizeStringNoEval(const char *text_in);
void __cdecl Cmd_EndTokenizedString();
void __cdecl Cmd_EndTokenizedStringKernel(CmdArgs *args);
void __cdecl SV_Cmd_TokenizeString(const char *text_in);
void __cdecl SV_Cmd_EndTokenizedString();
cmd_function_s *__cdecl Cmd_FindCommand(const char *cmdName);
void __cdecl Cmd_AddCommandInternal(const char *cmdName, void (__cdecl *function)(), cmd_function_s *allocedCmd);
void __cdecl Cmd_RemoveCommand(const char *cmdName);
void __cdecl Cmd_SetAutoComplete(const char *cmdName, const char *dir, const char *ext);
void __cdecl Cmd_Shutdown();
void __cdecl Cmd_ForEach(void (__cdecl *callback)(const char *));
void __cdecl Cmd_ResetConAccessCommands();
char __cdecl Cmd_SetConAccess(const char *str);
void __cdecl Cmd_ForEachConsoleAccessName(void (__cdecl *callback)(const char *));
const char **__cdecl Cmd_GetAutoCompleteFileList(const char *cmdName, int *fileCount);
void __cdecl Cmd_ComErrorCleanup();
void __cdecl Cmd_ResetArgs(CmdArgs *args);
void __cdecl Cmd_ExecuteSingleCommandInternal(
                int localClientNum,
                int controllerIndex,
                itemDef_s *item,
                char *text,
                bool restrict);
void __cdecl Cmd_ExecuteSingleCommand(int localClientNum, int controllerIndex, char *text);
void __cdecl Cmd_List_f();
void __cdecl Cmd_Init();
void __cdecl Cmd_Exec_f();
char __cdecl Cmd_ExecFromDisk(int localClientNum, int controllerIndex, const char *filename);
char __cdecl Cmd_ExecFromDiskAddText(int localClientNum, int controllerIndex, const char *filename);
char __cdecl Cmd_ExecFromFastFile(int localClientNum, int controllerIndex, char *filename);
void __cdecl Cmd_ExecAddText_f();

const char *__cdecl Cmd_Argv(int argIndex);
CmdArgs *__cdecl Cmd_Args();
int __cdecl Cmd_Argc();

extern CmdArgs sv_cmd_args;
extern CmdArgs g_cmd_args[2];
