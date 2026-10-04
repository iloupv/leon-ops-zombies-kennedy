#include "ddl_cmd.h"
#include "ddl_api.h"
#include <qcommon/cmd.h>

void __cdecl DDL_Cmd_GetAsset_f()
{
    const char *v0; // eax
    const char *v1; // eax
    const char *v2; // eax
    int version; // [esp-4h] [ebp-28h]
    ddlDef_t *ddl; // [esp+20h] [ebp-4h]

    if ( Cmd_Argc() == 2 )
    {
        v0 = Cmd_Argv(1);
        ddl = DDL_LoadAsset(v0);
        if ( ddl )
        {
            version = ddl->version;
            v1 = Cmd_Argv(1);
            DDL_NoCheckPrintf("DDL: Asset %s loaded at version %d.\n", v1, version);
        }
        else
        {
            v2 = Cmd_Argv(1);
            DDL_NoCheckPrintError("DDL: Asset %s could not be loaded.\n", v2);
        }
    }
    else
    {
        DDL_NoCheckPrintf("USAGE: ddlLoadAsset <DDL Asset Name>\n");
    }
}

void __cdecl DDL_Cmd_Show_f()
{
    const char *v0; // eax
    const char *v1; // eax
    ddlDef_t *ddl; // [esp+1Ch] [ebp-14h]
    ddlState_t searchState; // [esp+20h] [ebp-10h] BYREF

    if ( Cmd_Argc() == 2 )
    {
        v0 = Cmd_Argv(1);
        ddl = DDL_LoadAsset(v0);
        if ( ddl )
        {
            DDL_Reset(&searchState, ddl);
            DDL_NoCheckPrintf("DDL Contents:\n");
            DDL_MoveToStart(&searchState, ddl);
            if ( searchState.arrayIndex <= -1 )
                goto LABEL_6;
LABEL_5:
            DDL_NoCheckPrintf(
                "%d \t%s[%d] (%d)\n",
                searchState.absoluteOffset,
                searchState.member->name,
                searchState.arrayIndex,
                searchState.member->size / searchState.member->arraySize);
            while ( DDL_MoveNext(&searchState, &searchState) )
            {
                if ( searchState.arrayIndex > -1 )
                    goto LABEL_5;
LABEL_6:
                DDL_NoCheckPrintf(
                    "%d \t%s (%d)\n",
                    searchState.absoluteOffset,
                    searchState.member->name,
                    searchState.member->size / searchState.member->arraySize);
            }
        }
        else
        {
            v1 = Cmd_Argv(1);
            DDL_NoCheckPrintError("DDL: Asset %s could not be loaded.\n", v1);
        }
    }
    else
    {
        DDL_NoCheckPrintf("USAGE: ddlShow <DDL Asset Name>\n");
    }
}

cmd_function_s DDL_Cmd_GetAsset_f_VAR;
cmd_function_s DDL_Cmd_Show_f_VAR;

void __cdecl DDL_Cmd_Init()
{
    Cmd_AddCommandInternal("ddlLoadAsset", DDL_Cmd_GetAsset_f, &DDL_Cmd_GetAsset_f_VAR);
    Cmd_AddCommandInternal("ddlShow", DDL_Cmd_Show_f, &DDL_Cmd_Show_f_VAR);
}

