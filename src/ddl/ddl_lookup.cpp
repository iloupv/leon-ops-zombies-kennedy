#include "ddl_lookup.h"
#include <universal/q_shared.h>
#include "ddl_converter.h"

int __cdecl DDL_Lookup_IsMemberAtOffset(
                ddlMemberDef_t *member,
                int parentOffset,
                ddlDef_t *ddl,
                int searchOffset,
                int *arrayIndex)
{
    int currArr; // [esp+0h] [ebp-4h]

    for ( currArr = 0; ; ++currArr )
    { 
        if ( currArr >= member->arraySize )
            return 0;
        if ( searchOffset == member->offset + parentOffset )
            break;
        parentOffset += member->size / member->arraySize;
    }
    if ( member->arraySize <= 1 )
        *arrayIndex = -1;
    else
        *arrayIndex = currArr;
    return 1;
}

ddlMemberDef_t *__cdecl DDL_Lookup_FindInStruct(
                ddlStructDef_t *structDef,
                ddlDef_t *ddl,
                int parentOffset,
                int searchOffset,
                int *arrayIndex)
{
    int j; // [esp+8h] [ebp-Ch]
    int i; // [esp+Ch] [ebp-8h]
    ddlMemberDef_t *member; // [esp+10h] [ebp-4h]

    for ( i = 0; i < structDef->memberCount; ++i )
    {
        if ( structDef->members[i].type == 6 )
        {
            for ( j = 0; j < structDef->members[i].arraySize; ++j )
            {
                member = DDL_Lookup_FindInStruct(
                                     &ddl->structList[structDef->members[i].externalIndex],
                                     ddl,
                                     structDef->members[i].offset
                                 + parentOffset
                                 + j * (structDef->members[i].size / structDef->members[i].arraySize),
                                     searchOffset,
                                     arrayIndex);
                if ( member )
                    return member;
            }
        }
        else if ( DDL_Lookup_IsMemberAtOffset(&structDef->members[i], parentOffset, ddl, searchOffset, arrayIndex) )
        {
            return &structDef->members[i];
        }
    }
    return 0;
}

ddlMemberDef_t *__cdecl DDL_Lookup_GetMemberFromOffset(int offset, const ddlState_t *searchState, int *arrayIndex)
{
    return DDL_Lookup_FindInStruct(searchState->ddl->structList, searchState->ddl, 0, offset, arrayIndex);
}

void __cdecl DDL_Lookup_ClearState(ddlState_t *searchState)
{
    searchState->member = 0;
    searchState->absoluteOffset = 0;
    searchState->arrayIndex = -1;
    searchState->ddl = 0;
}

ddlMemberDef_t *__cdecl DDL_Lookup_FindMemberDef(const char *memberName, ddlStructDef_t *structDef)
{
    int low; // [esp+0h] [ebp-118h]
    int result; // [esp+4h] [ebp-114h]
    char lowercaseMemberName[260]; // [esp+8h] [ebp-110h] BYREF
    int high; // [esp+110h] [ebp-8h]
    int mid; // [esp+114h] [ebp-4h]

    low = 0;
    high = structDef->memberCount - 1;
    I_strncpyz(lowercaseMemberName, memberName, 256);
    I_strlwr(lowercaseMemberName);
    while ( low <= high )
    {
        mid = (high + low) / 2;
        result = I_strcmp(structDef->members[mid].name, lowercaseMemberName);
        if ( result >= 0 )
        {
            if ( result <= 0 )
                return &structDef->members[mid];
            high = mid - 1;
        }
        else
        {
            low = mid + 1;
        }
    }
    DDL_PrintError("DDL: Could not find '%s' in '%s'.\n", memberName, structDef->name);
    return 0;
}

int __cdecl DDL_Lookup_MoveToIndex(
                const ddlState_t *searchState,
                int arrayIndex,
                ddlState_t *resultState,
                int checkValidIndex)
{
    if ( !searchState->member
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ddl\\ddl_lookup.cpp", 193, 0, "%s", "searchState->member") )
    {
        __debugbreak();
    }
    if ( searchState->member->arraySize <= 1
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\ddl\\ddl_lookup.cpp",
                    194,
                    0,
                    "%s",
                    "searchState->member->arraySize > 1") )
    {
        __debugbreak();
    }
    if ( searchState->member->enumIndex != -1
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\ddl\\ddl_lookup.cpp",
                    195,
                    0,
                    "%s",
                    "searchState->member->enumIndex == DDL_INVALID_ENUM_INDEX") )
    {
        __debugbreak();
    }
    if ( searchState->arrayIndex != -1
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\ddl\\ddl_lookup.cpp",
                    196,
                    0,
                    "%s",
                    "searchState->arrayIndex == DDL_INVALID_ARRAY_INDEX") )
    {
        __debugbreak();
    }
    if ( checkValidIndex )
    {
        if ( arrayIndex >= searchState->member->arraySize
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ddl\\ddl_lookup.cpp", 202, 0, "%s", "isValidIndex") )
        {
            __debugbreak();
        }
    }
    else if ( arrayIndex >= searchState->member->arraySize )
    {
        return 0;
    }
    resultState->member = searchState->member;
    resultState->absoluteOffset = searchState->absoluteOffset
                                                            + arrayIndex * (resultState->member->size / resultState->member->arraySize);
    resultState->arrayIndex = arrayIndex;
    resultState->ddl = searchState->ddl;
    return 1;
}

int __cdecl DDL_Lookup_MoveTo(const ddlState_t *searchState, const char *name, ddlState_t *resultState)
{
    char lowercaseMemberName[256]; // [esp+4h] [ebp-118h] BYREF
    int low; // [esp+108h] [ebp-14h]
    int result; // [esp+10Ch] [ebp-10h]
    int arrayIndex; // [esp+110h] [ebp-Ch]
    int high; // [esp+114h] [ebp-8h]
    int mid; // [esp+118h] [ebp-4h]

    if ( searchState->member )
    {
        if ( searchState->member->arraySize > 1 && searchState->arrayIndex == -1 )
        {
            if ( searchState->member->enumIndex <= -1 )
            {
                arrayIndex = atoi(name);
                if ( arrayIndex < searchState->member->arraySize )
                {
                    resultState->member = searchState->member;
                    resultState->absoluteOffset = searchState->absoluteOffset
                                                                            + arrayIndex * (resultState->member->size / resultState->member->arraySize);
                    resultState->arrayIndex = arrayIndex;
                    resultState->ddl = searchState->ddl;
                    return 1;
                }
                else
                {
                    DDL_PrintError("DDL: Array index overflow.\n");
                    return 0;
                }
            }
            else
            {
                low = 0;
                high = searchState->ddl->enumList[searchState->member->enumIndex].memberCount - 1;
                I_strncpyz(lowercaseMemberName, name, 256);
                I_strlwr(lowercaseMemberName);
                while ( low <= high )
                {
                    mid = (high + low) / 2;
                    result = I_strcmp(
                                         searchState->ddl->enumList[searchState->member->enumIndex].members[mid],
                                         lowercaseMemberName);
                    if ( result >= 0 )
                    {
                        if ( result <= 0 )
                        {
                            resultState->member = searchState->member;
                            resultState->absoluteOffset = searchState->absoluteOffset
                                                                                    + mid * (resultState->member->size / resultState->member->arraySize);
                            resultState->arrayIndex = mid;
                            resultState->ddl = searchState->ddl;
                            return 1;
                        }
                        high = mid - 1;
                    }
                    else
                    {
                        low = mid + 1;
                    }
                }
                DDL_PrintError("DDL: Enum Index '%s' not found.\n", name);
                return 0;
            }
        }
        else if ( DDL_Converter_IsLeaf(searchState->member) )
        {
            DDL_PrintError(
                "DDL: Trying to move into a leaf member. '%s' expected to be a struct.\n",
                searchState->member->name);
            return 0;
        }
        else
        {
            if ( searchState->member->type != 6
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\ddl\\ddl_lookup.cpp",
                            303,
                            0,
                            "%s",
                            "searchState->member->type == DDL_STRUCT_TYPE") )
            {
                __debugbreak();
            }
            resultState->member = DDL_Lookup_FindMemberDef(
                                                            name,
                                                            &searchState->ddl->structList[searchState->member->externalIndex]);
            if ( resultState->member )
            {
                resultState->arrayIndex = -1;
                resultState->absoluteOffset = resultState->member->offset + searchState->absoluteOffset;
                resultState->ddl = searchState->ddl;
                return 1;
            }
            else
            {
                return 0;
            }
        }
    }
    else
    {
        resultState->member = DDL_Lookup_FindMemberDef(name, searchState->ddl->structList);
        if ( resultState->member )
        {
            resultState->arrayIndex = -1;
            resultState->absoluteOffset = resultState->member->offset;
            resultState->ddl = searchState->ddl;
            return 1;
        }
        else
        {
            return 0;
        }
    }
}

int __cdecl DDL_Lookup_MoveNext(const ddlState_t *searchState, ddlState_t *resultState)
{
    ddlMemberDef_t *member; // [esp+4h] [ebp-Ch]
    int nextOffset; // [esp+8h] [ebp-8h]
    int arrayIndex; // [esp+Ch] [ebp-4h] BYREF

    nextOffset = searchState->absoluteOffset + searchState->member->size / searchState->member->arraySize;
    member = DDL_Lookup_GetMemberFromOffset(nextOffset, searchState, &arrayIndex);
    resultState->ddl = searchState->ddl;
    if ( member )
    {
        resultState->member = member;
        resultState->absoluteOffset = nextOffset;
        resultState->arrayIndex = arrayIndex;
        return 1;
    }
    else
    {
        resultState->member = searchState->member;
        resultState->absoluteOffset = searchState->absoluteOffset;
        resultState->arrayIndex = searchState->arrayIndex;
        return 0;
    }
}

bool __cdecl DDL_Lookup_MoveToStart(ddlState_t *resultState, ddlDef_t *ddl)
{
    int arrayIndex; // [esp+0h] [ebp-4h] BYREF

    resultState->ddl = ddl;
    resultState->absoluteOffset = 0;
    resultState->member = DDL_Lookup_GetMemberFromOffset(0, resultState, &arrayIndex);
    resultState->arrayIndex = arrayIndex;
    return resultState->member != 0;
}

