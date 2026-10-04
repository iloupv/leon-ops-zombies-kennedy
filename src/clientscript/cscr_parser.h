#pragma once
#include "cscr_main.h"

struct OpcodeLookup // sizeof=0xC
{
    const char *codePos;
    unsigned int sourcePosIndex;
    unsigned int sourcePosCount;
};

struct SourceLookup // sizeof=0x8
{
    unsigned int sourcePos;
    int type;
};

struct SaveSourceBufferInfo // sizeof=0x8
{
    char *sourceBuf;
    int len;
};

struct Scr_SourcePos_t;

struct __declspec(align(4)) SourceBufferInfo // sizeof=0x18
{
    const char *codePos;
    char *buf;
    const char *sourceBuf;
    int len;
    int sortedIndex;
    bool archive;
    // padding byte
    // padding byte
    // padding byte
};

struct scrParserPub_t // sizeof=0x10
{                                       // XREF: .data:scrParserPub_t * gScrParserPub/r
    SourceBufferInfo *sourceBufferLookup;
                                        // XREF: CheckThreadPosition+8C/r
                                        // Scr_ScriptWindow::SetScriptFile(scriptInstance_t,char const *)+38/r ...
    unsigned int sourceBufferLookupLen; // XREF: Scr_ScriptWindow::SetScriptFile(scriptInstance_t,char const *)+24/r
                                        // Scr_ScriptWindow::Init(scriptInstance_t)+41/r ...
    const char *scriptfilename;         // XREF: Scr_LoadAnimTreeInternal+D8/r
                                        // Scr_LoadAnimTreeInternal+EA/w ...
    const char *sourceBuf;              // XREF: Scr_LoadAnimTreeInternal+A3/r
                                        // Scr_LoadAnimTreeInternal+120/w ...
};

struct scrParserGlob_t // sizeof=0x34
{
    OpcodeLookup *opcodeLookup;
    unsigned int opcodeLookupMaxLen;
    unsigned int opcodeLookupLen;
    SourceLookup *sourcePosLookup;
    unsigned int sourcePosLookupMaxLen;
    unsigned int sourcePosLookupLen;
    unsigned int sourceBufferLookupMaxLen;
    const unsigned __int8 *currentCodePos;
    unsigned int currentSourcePosCount;
    SaveSourceBufferInfo *saveSourceBufferLookup;
    unsigned int saveSourceBufferLookupLen;
    int delayedSourceIndex;
    int threadStartSourceIndex;
};

void __cdecl Scr_InitOpcodeLookup(scriptInstance_t inst);
void __cdecl Scr_ShutdownOpcodeLookup(scriptInstance_t inst);
void __cdecl AddOpcodePos(scriptInstance_t inst, unsigned int sourcePos, int type);
void __cdecl RemoveOpcodePos(scriptInstance_t inst);
void __cdecl AddThreadStartOpcodePos(scriptInstance_t inst, unsigned int sourcePos);
const char *__cdecl Scr_GetOpcodePosOfType(
                scriptInstance_t inst,
                unsigned int bufferIndex,
                unsigned int startSourcePos,
                unsigned int endSourcePos,
                int type,
                unsigned int *sourcePos);
unsigned int __cdecl Scr_GetClosestSourcePosOfType(
                scriptInstance_t inst,
                unsigned int bufferIndex,
                unsigned int sourcePos,
                int type);
unsigned int __cdecl Scr_GetPrevSourcePos(scriptInstance_t inst, const char *codePos, unsigned int index);
OpcodeLookup *__cdecl Scr_GetPrevSourcePosOpcodeLookup(scriptInstance_t inst, const char *codePos);
unsigned int __cdecl Scr_GetLineNum(scriptInstance_t inst, unsigned int bufferIndex, unsigned int sourcePos);
unsigned int __cdecl Scr_GetLineNumInternal(const char *buf, unsigned int sourcePos, const char **startLine, int *col);
unsigned int __cdecl Scr_GetFunctionLineNumInternal(const char *buf, unsigned int sourcePos, const char **startLine);
int __cdecl Scr_GetSourcePosOfType(scriptInstance_t inst, const char *codePos, int type, Scr_SourcePos_t *pos);
OpcodeLookup *__cdecl Scr_GetSourcePosOpcodeLookup(scriptInstance_t inst, const char *codePos);
void __cdecl Scr_SendSource(scriptInstance_t inst);
char *__cdecl Scr_AddSourceBuffer(
                scriptInstance_t inst,
                const char *filename,
                char *extFilename,
                const char *codePos,
                bool archive);
void __cdecl Scr_AddSourceBufferInternal(
                scriptInstance_t inst,
                const char *extFilename,
                const char *codePos,
                char *sourceBuf,
                int len,
                bool doEolFixup,
                bool archive);
SourceBufferInfo *__cdecl Scr_GetNewSourceBuffer(scriptInstance_t inst);
char *__cdecl Scr_ReadFile(
                scriptInstance_t inst,
                const char *filename,
                char *extFilename,
                const char *codePos,
                bool archive);
char *__cdecl Scr_ReadFile_FastFile(
                scriptInstance_t inst,
                const char *filename,
                const char *extFilename,
                const char *codePos,
                bool archive);
char *__cdecl Scr_ReadFile_LoadObj(
                scriptInstance_t inst,
                const char *filename,
                char *extFilename,
                const char *codePos,
                bool archive);
unsigned int __cdecl Scr_GetSourcePos(
                scriptInstance_t inst,
                unsigned int bufferIndex,
                unsigned int sourcePos,
                char *outBuf,
                unsigned int outBufLen);
unsigned int __cdecl Scr_GetLineInfo(const char *buf, unsigned int sourcePos, int *col, char *line);
void __cdecl Scr_CopyFormattedLine(char *line, const char *rawLine);
unsigned int __cdecl Scr_GetSourceBuffer(scriptInstance_t inst, const char *codePos);
void __cdecl Scr_PrintPrevCodePos(scriptInstance_t inst, int channel, char *codePos, unsigned int index);
void __cdecl Scr_PrintSourcePos(
                scriptInstance_t inst,
                int channel,
                const char *filename,
                const char *buf,
                unsigned int sourcePos);
char *__cdecl Scr_PrevCodePosFileName(scriptInstance_t inst, char *codePos);
const char *__cdecl Scr_PrevCodePosFunctionName(scriptInstance_t inst, char *codePos);
bool __cdecl Scr_PrevCodePosFileNameMatches(scriptInstance_t inst, char *codePos, const char *fileName);
void __cdecl Scr_PrintPrevCodePosSpreadSheet(
                scriptInstance_t inst,
                int channel,
                char *codePos,
                bool summary,
                bool functionSummary);
void __cdecl Scr_PrintSourcePosSpreadSheet(
                scriptInstance_t inst,
                int channel,
                const char *filename,
                const char *buf,
                unsigned int sourcePos);
void __cdecl Scr_PrintFunctionPosSpreadSheet(
                scriptInstance_t inst,
                int channel,
                const char *filename,
                const char *buf,
                unsigned int sourcePos);
unsigned int __cdecl Scr_GetFunctionInfo(const char *buf, unsigned int sourcePos, char *line);
void __cdecl Scr_PrintSourcePosSummary(scriptInstance_t inst, int channel, const char *filename);
void __cdecl Scr_GetCodePos(
                scriptInstance_t inst,
                const char *codePos,
                unsigned int index,
                char *outBuf,
                unsigned int outBufLen);
void __cdecl Scr_GetFileAndLine(scriptInstance_t inst, const char *codePos, char **filename, int *linenum);
void CompileError(scriptInstance_t inst, unsigned int sourcePos, const char *msg, ...);
void __cdecl Scr_IgnoreLeaks(scriptInstance_t inst);
void CompileError2(scriptInstance_t inst, char *codePos, const char *msg, ...);
void __cdecl Scr_GetTextSourcePos(scriptInstance_t inst, const char *buf, char *codePos, char *line);
void __cdecl RuntimeError(
                scriptInstance_t inst,
                char *codePos,
                unsigned int index,
                const char *msg,
                const char *dialogMessage);
void __cdecl RuntimeErrorInternal(
                scriptInstance_t inst,
                int channel,
                char *codePos,
                unsigned int index,
                const char *msg);
void __cdecl Scr_SetLoadedImpureScript(bool loadedImpureScript);


extern scrParserPub_t gScrParserPub[2];
extern scrParserGlob_t gScrParserGlob[2];

extern char g_EndPos;