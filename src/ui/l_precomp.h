#pragma once
#include "l_script.h"
#include "ui_shared.h"

void __cdecl PrintSourceStack(const script_s *scriptstack);
void SourceError(source_s *source, const char *str, ...);
void SourceWarning(source_s *source, const char *str, ...);
void __cdecl PC_PushScript(source_s *source, script_s *script);
token_s *__cdecl PC_CopyToken(token_s *token);
void __cdecl PC_FreeToken(token_s *token);
int __cdecl PC_ReadSourceToken(source_s *source, token_s *token);
void __cdecl PC_PopIndent(source_s *source, int *type, parseSkip_t *skip);
int __cdecl PC_UnreadSourceToken(source_s *source, token_s *token);
int __cdecl PC_ReadDefineParms(source_s *source, define_s *define, token_s **parms, int maxparms);
int __cdecl PC_StringizeTokens(token_s *tokens, token_s *token);
int __cdecl PC_MergeTokens(token_s *t1, token_s *t2);
int __cdecl PC_NameHash(char *name);
void __cdecl PC_AddDefineToHash(define_s *define, define_s **definehash);
define_s *__cdecl PC_FindHashedDefine(define_s **definehash, char *name);
int __cdecl PC_FindDefineParm(define_s *define, const char *name);
void __cdecl PC_FreeDefine(define_s *define);
int __cdecl PC_ExpandBuiltinDefine(
                source_s *source,
                token_s *deftoken,
                define_s *define,
                token_s **firsttoken,
                token_s **lasttoken);
int __cdecl PC_ExpandDefine(
                source_s *source,
                token_s *deftoken,
                define_s *define,
                token_s **firsttoken,
                token_s **lasttoken);
void __cdecl PC_ConvertPath(char *path);
int __cdecl PC_Directive_include(source_s *source);
bool __cdecl PC_WhiteSpaceBeforeToken(token_s *token);
void __cdecl PC_ClearTokenWhiteSpace(token_s *token);
int __cdecl PC_Directive_undef(source_s *source);
int __cdecl PC_ReadLine(source_s *source, token_s *token, bool expandDefines);
int __cdecl PC_ExpandDefineIntoSource(source_s *source, token_s *deftoken, define_s *define);
int __cdecl PC_Directive_define(source_s *source);
define_s *__cdecl PC_DefineFromString(char *string);
int __cdecl PC_AddDefine(source_s *source, char *string);
define_s *__cdecl PC_CopyDefine(source_s *source, define_s *define);
void __cdecl PC_AddGlobalDefinesToSource(source_s *source);
int __cdecl PC_Directive_if_def(source_s *source, int type);
void __cdecl PC_PushIndent(source_s *source, int type, parseSkip_t skip);
int __cdecl PC_Directive_ifdef(source_s *source);
int __cdecl PC_Directive_ifndef(source_s *source);
int __cdecl PC_Directive_else(source_s *source);
int __cdecl PC_Directive_endif(source_s *source);
int __cdecl PC_OperatorPriority(int op);
int __cdecl PC_EvaluateTokens(source_s *source, token_s *tokens, int *intvalue, long double *floatvalue, int integer);
int __cdecl PC_DollarEvaluate(source_s *source, int *intvalue, long double *floatvalue, int integer);
int __cdecl PC_Directive_elif(source_s *source);
int __cdecl PC_Evaluate(source_s *source, int *intvalue, long double *floatvalue, int integer);
int __cdecl PC_Directive_if(source_s *source);
int __cdecl PC_Directive_line(source_s *source);
int __cdecl PC_Directive_error(source_s *source);
int __cdecl PC_Directive_pragma(source_s *source);
void __cdecl UnreadSignToken(source_s *source);
int __cdecl PC_Directive_eval(source_s *source);
int __cdecl PC_Directive_evalfloat(source_s *source);
int __cdecl PC_ReadDirective(source_s *source);
int __cdecl PC_DollarDirective_evalint(source_s *source);
int __cdecl PC_DollarDirective_evalfloat(source_s *source);
int __cdecl PC_ReadDollarDirective(source_s *source);
int __cdecl PC_ReadToken(source_s *source, token_s *token);
int __cdecl PC_CheckTokenString(source_s *source, char *string);
inline int __cdecl PC_CheckTokenString(source_s *source, const char *string)
{
    return PC_CheckTokenString(source, (char *)string);
}
void __cdecl PC_UnreadLastToken(source_s *source);
void __cdecl PC_UnreadToken(source_s *source, token_s *token);
source_s *__cdecl LoadSourceFile(char *filename);
void __cdecl FreeSource(source_s *source);
int __cdecl PC_LoadSourceHandle(char *filename, const char **builtinDefines);
int __cdecl PC_FreeSourceHandle(int handle);
int __cdecl PC_ReadTokenHandle(int handle, pc_token_s *pc_token);
int __cdecl PC_ReadLineHandle(int handle, pc_token_s *pc_token);
void __cdecl PC_UnreadLastTokenHandle(int handle);
int __cdecl PC_SourceFileAndLine(int handle, char *filename, int *line);
