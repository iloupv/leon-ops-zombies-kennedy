#pragma once
#include <database/db_registry.h>
#include "cscr_main.h"
#include "cscr_debugger.h"

enum ParseToken_t : __int32;

sval_u __cdecl node_pos(unsigned int pool);
sval_u __cdecl node1_(scriptInstance_t inst, int val1);
void __cdecl Scr_InitAllocNode(scriptInstance_t inst);
void __cdecl Scr_ShutdownAllocNode(scriptInstance_t inst);
sval_u __cdecl node0(scriptInstance_t inst, ParseToken_t type);
sval_u *__cdecl Scr_AllocNode(scriptInstance_t inst, int size);
sval_u __cdecl node1(scriptInstance_t inst, ParseToken_t type, sval_u val1);
sval_u __cdecl node2(scriptInstance_t inst, ParseToken_t type, sval_u val1, sval_u val2);
sval_u __cdecl node2_(scriptInstance_t inst, sval_u val1, sval_u val2);
sval_u __cdecl node3(scriptInstance_t inst, ParseToken_t type, sval_u val1, sval_u val2, sval_u val3);
sval_u __cdecl node4(scriptInstance_t inst, ParseToken_t type, sval_u val1, sval_u val2, sval_u val3, sval_u val4);
sval_u __cdecl node5(
                scriptInstance_t inst,
                ParseToken_t type,
                sval_u val1,
                sval_u val2,
                sval_u val3,
                sval_u val4,
                sval_u val5);
sval_u __cdecl node6(
                scriptInstance_t inst,
                ParseToken_t type,
                sval_u val1,
                sval_u val2,
                sval_u val3,
                sval_u val4,
                sval_u val5,
                sval_u val6);
sval_u __cdecl node7(
                scriptInstance_t inst,
                ParseToken_t type,
                sval_u val1,
                sval_u val2,
                sval_u val3,
                sval_u val4,
                sval_u val5,
                sval_u val6,
                sval_u val7);
sval_u __cdecl node8(
                scriptInstance_t inst,
                ParseToken_t type,
                sval_u val1,
                sval_u val2,
                sval_u val3,
                sval_u val4,
                sval_u val5,
                sval_u val6,
                sval_u val7,
                sval_u val8);
sval_u __cdecl linked_list_end(scriptInstance_t inst, sval_u val);
sval_u __cdecl prepend_node(scriptInstance_t inst, sval_u val1, sval_u val2);
sval_u __cdecl append_node(scriptInstance_t inst, sval_u val1, sval_u val2);
void __cdecl Scr_ClearDebugExpr(scriptInstance_t inst, debugger_sval_s *debugExprHead);
sval_u *__cdecl Scr_AllocDebugExpr(scriptInstance_t inst, ParseToken_t type, int size, const char *name);
void __cdecl Scr_FreeDebugExpr(scriptInstance_t inst, ScriptExpression_t *expr);
sval_u __cdecl debugger_node0(scriptInstance_t inst, ParseToken_t type);
sval_u __cdecl debugger_node1(scriptInstance_t inst, ParseToken_t type, sval_u val1);
sval_u __cdecl debugger_node2(scriptInstance_t inst, ParseToken_t type, sval_u val1, sval_u val2);
sval_u __cdecl debugger_node3(scriptInstance_t inst, ParseToken_t type, sval_u val1, sval_u val2, sval_u val3);
sval_u __cdecl debugger_node4(
                scriptInstance_t inst,
                ParseToken_t type,
                sval_u val1,
                sval_u val2,
                sval_u val3,
                sval_u val4);
sval_u __cdecl debugger_prepend_node(scriptInstance_t inst, sval_u val1, sval_u val2);
sval_u __cdecl debugger_buffer(
                scriptInstance_t inst,
                ParseToken_t type,
                char *buf,
                unsigned int size,
                int alignment);
sval_u __cdecl debugger_string(scriptInstance_t inst, ParseToken_t type, char *s);


extern debugger_sval_s *g_debugExprHead;