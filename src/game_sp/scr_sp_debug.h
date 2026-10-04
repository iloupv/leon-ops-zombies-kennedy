#pragma once

// zombies: script error locations without developer mode. KB runs with developer 0 (its debug hunk is
// never created), so a runtime error prints only "throwing script exception: <msg>". With
// `+set bo1_scripterrors <n>` the first n runtime errors of the server VM also print where they
// happened as file::function+offset for the error and every caller, from a table of function starts
// taken from the compiler's scriptsPos before the script load ends. No file/line: the retail compiler
// keeps no source positions.

#include <clientscript/cscr_main.h>

// Called at the end of a server script load, before Scr_EndLoadScripts frees scriptsPos.
void Scr_SP_BuildCodePosMap(scriptInstance_t inst);

// From RuntimeError (cscr_parser.cpp) when developer is off. Returns true if it printed a report.
bool Scr_SP_ReportRuntimeError(scriptInstance_t inst, const char *codePos, const char *msg);

// file::function+offset for a code position, or "@ offset" when unknown. Uses a static buffer.
const char *Scr_SP_DescribeCodePos(scriptInstance_t inst, const char *codePos);

// k1: the "file::function" containing a code position, or nullptr when unknown (map not built).
const char *Scr_SP_CodePosFunction(scriptInstance_t inst, const char *codePos);

// Opt-in, bounded reports for distinct suspension / long-loop positions. No VM state changes.
extern bool g_scrSPTraceEnabled;
void Scr_SP_TraceThread(scriptInstance_t inst, unsigned int localId, const char *codePos,
    const char *reason, unsigned int value);
