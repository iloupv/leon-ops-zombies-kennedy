// The ordered SP builtin table lists (see scr_sp_tables.h) and their lookups.
#include "scr_sp_tables.h"
#include <string.h>

namespace
{
struct ScrSpFunctionTable
{
    const BuiltinFunctionDef *defs;
    const unsigned int *count;
};

struct ScrSpMethodTable
{
    const BuiltinMethodDef *defs;
    const unsigned int *count;
};

#define SCR_SP_FUNCTION_ROW(prefix) { prefix##_functions, &prefix##_function_count },
#define SCR_SP_METHOD_ROW(prefix) { prefix##_methods, &prefix##_method_count },

const ScrSpFunctionTable s_serverFunctionTables[] = { SCR_SP_SERVER_TABLES(SCR_SP_FUNCTION_ROW) };
const ScrSpMethodTable s_serverMethodTables[] = { SCR_SP_SERVER_TABLES(SCR_SP_METHOD_ROW) };
const ScrSpFunctionTable s_clientFunctionTables[] = { SCR_SP_CLIENT_TABLES(SCR_SP_FUNCTION_ROW) };
const ScrSpMethodTable s_clientMethodTables[] = { SCR_SP_CLIENT_TABLES(SCR_SP_METHOD_ROW) };

#undef SCR_SP_FUNCTION_ROW
#undef SCR_SP_METHOD_ROW

template <typename Table, unsigned int N>
auto FindBuiltin(const Table (&tables)[N], const char **pName, int *type) -> decltype(tables[0].defs->actionFunc)
{
    for (unsigned int t = 0; t < N; ++t)
    {
        for (unsigned int i = 0; i < *tables[t].count; ++i)
        {
            if (!strcmp(*pName, tables[t].defs[i].actionString))
            {
                *pName = tables[t].defs[i].actionString;
                *type = tables[t].defs[i].type;
                return tables[t].defs[i].actionFunc;
            }
        }
    }
    return 0;
}
} // namespace

void (__cdecl *__cdecl Scr_SP_GetFunction(const char **pName, int *type))()
{
    return FindBuiltin(s_serverFunctionTables, pName, type);
}

void (__cdecl *__cdecl Scr_SP_GetMethod(const char **pName, int *type))(scr_entref_t)
{
    return FindBuiltin(s_serverMethodTables, pName, type);
}

void (__cdecl *__cdecl CScr_SP_GetFunction(const char **pName, int *type))()
{
    return FindBuiltin(s_clientFunctionTables, pName, type);
}

void (__cdecl *__cdecl CScr_SP_GetMethod(const char **pName, int *type))(scr_entref_t)
{
    return FindBuiltin(s_clientMethodTables, pName, type);
}
