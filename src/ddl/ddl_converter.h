#pragma once
#include "ddl_api.h"

bool __cdecl DDL_Converter_IsLeaf(ddlMemberDef_t *member);
void __cdecl DDL_Converter_CopyStates(
                const ddlState_t *const stateFrom,
                char *bufferFrom,
                const ddlState_t *const stateTo,
                char *bufferTo);
void __cdecl DDL_Converter_ProcessStates(
                const ddlStructDef_t *const structDef,
                int structMemberItr,
                const ddlState_t *const oldState,
                const ddlState_t *const newState,
                char *oldBuffer,
                char *newBuffer);
void __cdecl DDL_Converter_TraverseStruct(
                const ddlStructDef_t *const structDef,
                const ddlState_t *const oldBaseState,
                const ddlState_t *const newBaseState,
                char *oldBuffer,
                char *newBuffer);
int __cdecl DDL_Converter_Versions(
                ddlDef_t *ddlVersionFrom,
                ddlDef_t *ddlVersionTo,
                char *buffer,
                char *backupBuffer,
                int bufferSize);
