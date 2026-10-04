#pragma once
#include "cscr_debugger.h"
#include <cstdio>

struct yy_buffer_state // sizeof=0x28
{                                       // XREF: ?ScriptParse@@YAXW4scriptInstance_t@@PATsval_u@@E@Z/r
    FILE *yy_input_file;
    char *yy_ch_buf;                    // XREF: ScriptParse(scriptInstance_t,sval_u *,uchar)+46/w
    char *yy_buf_pos;
    unsigned int yy_buf_size;           // XREF: ScriptParse(scriptInstance_t,sval_u *,uchar)+3F/w
    int yy_n_chars;
    int yy_is_our_buffer;               // XREF: ScriptParse(scriptInstance_t,sval_u *,uchar)+4D/w
    int yy_is_interactive;
    int yy_at_bol;
    int yy_fill_buffer;
    int yy_buffer_status;
};

struct stype_t // sizeof=0x8
{                                       // XREF: .data:stype_t yylval/r
    sval_u val;                         // XREF: yyparse(void)+381/r
    unsigned int pos;                   // XREF: yyparse(void)+67/w
};

int __cdecl yyparse();
unsigned int __cdecl LowerCase(unsigned int stringValue);
void __cdecl _yy_memcpy(char *to, char *from, unsigned int count);
int __cdecl yylex();
void __cdecl TextValue(char *str, int len);
unsigned int __cdecl HashValue(char *str, int len);
int __cdecl StringValue(char *str, int len);
void __cdecl IntegerValue(char *str);
void __cdecl FloatValue(char *str);
int __cdecl yy_get_next_buffer();
int __cdecl yy_get_previous_state();
int __cdecl yy_try_NUL_trans(int yy_current_state);
void __cdecl yyrestart(FILE *input_file);
void __cdecl yy_load_buffer_state();
yy_buffer_state *__cdecl yy_create_buffer(_iobuf *file, unsigned int size);
void __cdecl yy_init_buffer(yy_buffer_state *b, _iobuf *file);
void __cdecl yy_flush_buffer(yy_buffer_state *b);
void __cdecl    yy_fatal_error(const char *msg);
void *__cdecl yy_flex_alloc(unsigned int size);
void *__cdecl yy_flex_realloc(void *ptr, unsigned int size);
int __cdecl yyerror();
void __cdecl ScriptParse(scriptInstance_t inst, sval_u *parseData, unsigned __int8 user);
