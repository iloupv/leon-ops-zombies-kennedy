#include "stringed_remote.h"
#include <win32/win_common.h>
#include <string.h>
#include <qcommon/common.h>

int StringEdCommandCount;
char StringEdCommands[4][128];

EXT_List<LocalizeEntry> overrideLocalizedEntryList;

char __cdecl ProcessStringEdCmds()
{
    EXT_Link<LocalizeEntry> *Current; // [esp+78h] [ebp-18h]
    char *Value; // [esp+7Ch] [ebp-14h]
    LocalizeEntry *newEntry; // [esp+80h] [ebp-10h]
    LocalizeEntry *newEntrya; // [esp+80h] [ebp-10h]
    char *Reference; // [esp+84h] [ebp-Ch]
    int i; // [esp+88h] [ebp-8h]

    Sys_EnterCriticalSection(CRITSECT_STRINGED_COMMAND);
    for ( i = 0; i < StringEdCommandCount; ++i )
    {
        Reference = strtok(StringEdCommands[i], "!");
        Value = strtok(0, "\n");
        newEntry = 0;
        for ( Current = overrideLocalizedEntryList.m_Head; Current; Current = Current->m_Next )
        {
            if ( !strcmp(Reference, Current->m_Data->name) )
            {
                newEntry = Current->m_Data;
                break;
            }
        }
        if ( newEntry )
        {
            Com_LiveDeallocate((void *)newEntry->value);
            newEntry->value = (const char *)Com_LiveAllocate(strlen(Value) + 1);
            I_strncpyz((char *)newEntry->value, Value, strlen(Value) + 1);
        }
        else
        {
            newEntrya = (LocalizeEntry *)Com_LiveAllocate(8u);
            newEntrya->name = (const char *)Com_LiveAllocate(strlen(Reference) + 1);
            I_strncpyz((char *)newEntrya->name, Reference, strlen(Reference) + 1);
            newEntrya->value = (const char *)Com_LiveAllocate(strlen(Value) + 1);
            I_strncpyz((char *)newEntrya->value, Value, strlen(Value) + 1);
            //EXT_List<LocalizeEntry>::Add(&overrideLocalizedEntryList, newEntrya, 0);
            overrideLocalizedEntryList.Add(newEntrya, 0);
        }
    }
    StringEdCommandCount = 0;
    Sys_LeaveCriticalSection(CRITSECT_STRINGED_COMMAND);
    return 1;
}

char *__cdecl GetString(const char *PackageAndStringReference)
{
    EXT_Link<LocalizeEntry> *Current; // [esp+18h] [ebp-8h]
    char *value; // [esp+1Ch] [ebp-4h]

    value = 0;
    Sys_EnterCriticalSection(CRITSECT_STRINGED_COMMAND);
    for ( Current = overrideLocalizedEntryList.m_Head; Current; Current = Current->m_Next )
    {
        if ( !strcmp(Current->m_Data->name, PackageAndStringReference) )
        {
            value = (char *)Current->m_Data->value;
            break;
        }
    }
    Sys_LeaveCriticalSection(CRITSECT_STRINGED_COMMAND);
    return value;
}