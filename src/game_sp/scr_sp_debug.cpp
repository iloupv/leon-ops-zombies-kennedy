#include "scr_sp_debug.h"

#include <algorithm>
#include <string>
#include <vector>

#include <clientscript/cscr_animtree.h>
#include <clientscript/cscr_compiler.h>
#include <clientscript/cscr_stringlist.h>
#include <clientscript/cscr_variable.h>
#include <clientscript/cscr_vm.h>
#include <qcommon/common.h>
#include <universal/dvar.h>
#include <universal/q_shared.h>
#include <win32/win_main.h>

bool g_scrSPTraceEnabled;

namespace
{
struct CodePosEntry
{
    unsigned int offset;
    std::string name; // file::function
};

std::vector<CodePosEntry> s_codePosMaps[2]; // per script instance (x6: the client VM too)
const dvar_s *bo1_scripterrors;
int s_reported;
int s_suppressed;
std::vector<const char *> s_threadPositions;

void RegisterDvar()
{
    if (!bo1_scripterrors)
        bo1_scripterrors = _Dvar_RegisterInt(
            "bo1_scripterrors",
            0,
            0,
            0x7FFFFFFF,
            0,
            "zombies: print the location (file::function+offset and callers) of the first n server script runtime errors");
}
} // namespace

void Scr_SP_BuildCodePosMap(scriptInstance_t inst)
{
    unsigned int fileVar;
    unsigned int fileObj;
    unsigned int funcVar;
    const char *programBuffer = gScrVarPub[inst].programBuffer;

    if (!Sys_IsHeadless())
        return;
    RegisterDvar();
    std::vector<CodePosEntry> &s_codePosMap = s_codePosMaps[inst];
    s_codePosMap.clear();
    if (inst == SCRIPTINSTANCE_SERVER)
    {
        s_reported = 0;
        s_suppressed = 0;
        s_threadPositions.clear();
        g_scrSPTraceEnabled = bo1_scripterrors->current.integer > 0;
    }
    if (bo1_scripterrors->current.integer <= 0 || !gScrCompilePub[inst].scriptsPos || !programBuffer)
        return;

    for (fileVar = FindFirstSibling(inst, gScrCompilePub[inst].scriptsPos); fileVar; fileVar = FindNextSibling(inst, fileVar))
    {
        unsigned int fileName = GetVariableName(inst, fileVar);
        if (fileName >= 0x10000 || GetValueType(inst, fileVar) != VAR_POINTER)
            continue;
        fileObj = FindObject(inst, fileVar);
        if (!fileObj)
            continue;
        const char *file = SL_ConvertToString(fileName, inst);
        for (funcVar = FindFirstSibling(inst, fileObj); funcVar; funcVar = FindNextSibling(inst, funcVar))
        {
            unsigned int funcName = GetVariableName(inst, funcVar);
            if (funcName >= 0x10000)
                continue;
            VariableValue pos = Scr_EvalVariable(inst, funcVar);
            if (pos.type != VAR_CODEPOS && pos.type != VAR_DEVELOPER_CODEPOS)
                continue;
            if (!Scr_IsInOpcodeMemory(inst, pos.u.codePosValue))
                continue;
            CodePosEntry e;
            e.offset = (unsigned int)(pos.u.codePosValue - programBuffer);
            e.name = std::string(file ? file : "?") + "::" + SL_ConvertToString(funcName, inst);
            s_codePosMap.push_back(e);
        }
    }
    std::sort(s_codePosMap.begin(), s_codePosMap.end(), [](const CodePosEntry &a, const CodePosEntry &b) { return a.offset < b.offset; });
    if (bo1_scripterrors->current.integer > 0)
        Com_Printf(24, "bo1_scripterrors: %d script functions mapped (instance %d)\n", (int)s_codePosMap.size(), inst);
}

const char *Scr_SP_DescribeCodePos(scriptInstance_t inst, const char *codePos)
{
    static char buf[2][320];
    static int which;
    char *out = buf[which];
    which ^= 1;

    if (!codePos)
        return "<frozen thread>";
    if (!gScrVarPub[inst].programBuffer || !Scr_IsInOpcodeMemory(inst, codePos))
    {
        Com_sprintf(out, sizeof(buf[0]), "<not script code %p>", codePos);
        return out;
    }
    unsigned int offset = (unsigned int)(codePos - gScrVarPub[inst].programBuffer);
    const std::vector<CodePosEntry> &s_codePosMap = s_codePosMaps[inst];
    auto it = std::upper_bound(
        s_codePosMap.begin(), s_codePosMap.end(), offset, [](unsigned int o, const CodePosEntry &e) { return o < e.offset; });
    if (it == s_codePosMap.begin())
    {
        Com_sprintf(out, sizeof(buf[0]), "@ %u", offset);
        return out;
    }
    --it;
    Com_sprintf(out, sizeof(buf[0]), "%s+%u (@ %u)", it->name.c_str(), offset - it->offset, offset);
    return out;
}

const char *Scr_SP_CodePosFunction(scriptInstance_t inst, const char *codePos)
{
    if (!codePos || !gScrVarPub[inst].programBuffer || !Scr_IsInOpcodeMemory(inst, codePos))
        return nullptr;
    unsigned int offset = (unsigned int)(codePos - gScrVarPub[inst].programBuffer);
    const std::vector<CodePosEntry> &s_codePosMap = s_codePosMaps[inst];
    auto it = std::upper_bound(
        s_codePosMap.begin(), s_codePosMap.end(), offset, [](unsigned int o, const CodePosEntry &e) { return o < e.offset; });
    if (it == s_codePosMap.begin())
        return nullptr;
    --it;
    return it->name.c_str();
}

bool Scr_SP_ReportRuntimeError(scriptInstance_t inst, const char *codePos, const char *msg)
{
    int i;

    if (!Sys_IsHeadless())
        return false;
    RegisterDvar();
    if (bo1_scripterrors->current.integer <= 0)
        return false;
    if (s_reported >= bo1_scripterrors->current.integer)
    {
        if (++s_suppressed == 1)
            Com_Printf(24, "bo1_scripterrors: limit of %d reports reached; further errors are not located\n", s_reported);
        return false;
    }
    ++s_reported;
    Com_Printf(24, "******* %sscript runtime error #%d *******\n%s: %s\n", inst == SCRIPTINSTANCE_CLIENT ? "client " : "", s_reported, msg ? msg : "?", Scr_SP_DescribeCodePos(inst, codePos - 1));
    for (i = gScrVmPub[inst].function_count - 1; i >= 1; --i)
        Com_Printf(24, "  called from: %s\n", Scr_SP_DescribeCodePos(inst, gScrVmPub[inst].function_frame_start[i].fs.pos - 1));
    if (gScrVmPub[inst].function_count)
        Com_Printf(24, "  started from: %s\n", Scr_SP_DescribeCodePos(inst, gScrVmPub[inst].function_frame_start[0].fs.pos));
    return true;
}

void Scr_SP_TraceThread(scriptInstance_t inst, unsigned int localId, const char *codePos,
    const char *reason, unsigned int value)
{
    if (!g_scrSPTraceEnabled || inst != SCRIPTINSTANCE_SERVER)
        return;
    if (s_threadPositions.size() >= (unsigned int)bo1_scripterrors->current.integer
        || std::find(s_threadPositions.begin(), s_threadPositions.end(), codePos) != s_threadPositions.end())
        return;
    s_threadPositions.push_back(codePos);
    Com_Printf(24, "bo1_scriptthread: thread %u local %u vmTime %u %s %u at %s\n",
        GetStartLocalId(inst, localId), localId, gScrVarPub[inst].time,
        reason, value, Scr_SP_DescribeCodePos(inst, codePos - 1));
    if (!strcmp(reason, "notify"))
        Com_Printf(24, "  waiting for: %s\n", SL_ConvertToString(value, inst));
}
