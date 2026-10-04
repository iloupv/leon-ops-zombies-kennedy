#pragma once
#include "ddl_api.h"

int __cdecl DDL_Lookup_IsMemberAtOffset(
                ddlMemberDef_t *member,
                int parentOffset,
                ddlDef_t *ddl,
                int searchOffset,
                int *arrayIndex);
ddlMemberDef_t *__cdecl DDL_Lookup_FindInStruct(
                ddlStructDef_t *structDef,
                ddlDef_t *ddl,
                int parentOffset,
                int searchOffset,
                int *arrayIndex);
ddlMemberDef_t *__cdecl DDL_Lookup_GetMemberFromOffset(int offset, const ddlState_t *searchState, int *arrayIndex);
void __cdecl DDL_Lookup_ClearState(ddlState_t *searchState);
ddlMemberDef_t *__cdecl DDL_Lookup_FindMemberDef(const char *memberName, ddlStructDef_t *structDef);
int __cdecl DDL_Lookup_MoveToIndex(
                const ddlState_t *searchState,
                int arrayIndex,
                ddlState_t *resultState,
                int checkValidIndex);
int __cdecl DDL_Lookup_MoveTo(const ddlState_t *searchState, const char *name, ddlState_t *resultState);
int __cdecl DDL_Lookup_MoveNext(const ddlState_t *searchState, ddlState_t *resultState);
bool __cdecl DDL_Lookup_MoveToStart(ddlState_t *resultState, ddlDef_t *ddl);
