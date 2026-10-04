#pragma once

enum ParseTokenType : __int32
{                                                                             // XREF: parseInfo_t/r
                                                                                // ?Expression_Parse@@YA_NPAPBDPAUExpressionStatement@@PAXH@Z/r
        PARSE_TOKEN_UNKNOWN         = 0x0,
        PARSE_TOKEN_NUMBER            = 0x1,
        PARSE_TOKEN_STRING            = 0x2,
        PARSE_TOKEN_NAME                = 0x3,
        PARSE_TOKEN_HASH                = 0x4,
        PARSE_TOKEN_PUNCTUATION = 0x5,
};

struct parseInfo_t // sizeof=0x424
{                                                                             // XREF: ParseThreadInfo/r
        char token[1024];
        ParseTokenType tokenType;
        int lines;
        bool ungetToken;
        bool spaceDelimited;
        bool keepStringQuotes;
        bool csv;
        bool negativeNumbers;
        // padding byte
        // padding byte
        // padding byte
        const char *errorPrefix;
        const char *warningPrefix;
        int backup_lines;
        const char *backup_text;
        const char *parseFile;
};

struct ParseThreadInfo // sizeof=0x464C
{                                                                             // XREF: .data:g_parse/r
        parseInfo_t parseInfo[16];
        int parseInfoNum;
        const char *tokenPos;
        const char *prevTokenPos;
        char line[1024];
};

struct com_parse_mark_t // sizeof=0x14
{                                                                             // XREF: .data:com_parse_mark_t g_cs_file_linemark/r
                                                                                // level_locals_t/r ...
        int lines;                                                    // XREF: CScr_OpenFile(void)+12D/w
                                                                                // CScr_FReadLn+A3/r ...
        const char *text;
        int ungetToken;
        int backup_lines;
        const char *backup_text;
};

void __cdecl Com_InitParse();
void __cdecl Com_InitParseInfo(parseInfo_t *pi);
void __cdecl Com_BeginParseSession(const char *filename);
ParseThreadInfo *__cdecl Com_GetParseThreadInfo();
void __cdecl Com_EndParseSession();
void __cdecl Com_ResetParseSessions();
void __cdecl Com_SetSpaceDelimited(int spaceDelimited);
void __cdecl Com_SetKeepStringQuotes(int keepStringQuotes);
void __cdecl Com_SetCSV(int csv);
void __cdecl Com_SetParseNegativeNumbers(int negativeNumbers);
int __cdecl Com_GetCurrentParseLine();
void __cdecl Com_SetScriptErrorPrefix(const char *prefix);
const char *__cdecl Com_GetScriptErrorPrefix();
void __cdecl Com_SetScriptWarningPrefix(const char *prefix);
const char *__cdecl Com_GetScriptWarningPrefix();
void Com_ScriptErrorDrop(const char *msg, ...);
void Com_ScriptError(const char *msg, ...);
void Com_ScriptWarning(const char *msg, ...);
void __cdecl Com_UngetToken();
void __cdecl Com_ParseSetMark(const char **text, com_parse_mark_t *mark);
void __cdecl Com_ParseReturnToMark(const char **text, com_parse_mark_t *mark);
int __cdecl Com_Compress(char *data_p);
const char *__cdecl Com_GetLastTokenPos();
parseInfo_t *__cdecl Com_Parse(const char **data_p);
parseInfo_t *__cdecl Com_ParseExt(const char **data_p, int allowLineBreaks);
const char *__cdecl SkipWhitespace(const char *data, int *newLines);
parseInfo_t *__cdecl Com_ParseCSV(const char **data_p, int allowLineBreaks);
parseInfo_t *__cdecl Com_ParseOnLine(const char **data_p);
ParseTokenType __cdecl Com_GetTokenType();
int __cdecl Com_MatchToken(const char **buf_p, const char *match, int warning);
int __cdecl Com_SkipBracedSection(const char **program, unsigned int startDepth, int iMaxNesting);
void __cdecl Com_SkipRestOfLine(const char **data);
int __cdecl Com_GetArgCountOnLine(const char **data_p);
char *__cdecl Com_ParseRestOfLine(const char **data_p);
double __cdecl Com_ParseFloat(const char **buf_p);
double __cdecl Com_ParseFloatOnLine(const char **buf_p);
int __cdecl Com_ParseInt(const char **buf_p);
void __cdecl Com_Parse1DMatrix(const char **buf_p, int x, float *m);

extern const char GLOBAL_EMPTY_STRING;