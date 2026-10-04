#pragma once
#include "ui_shared.h"

void __cdecl UILocalVar_Init(UILocalVarContext *context);
void __cdecl UILocalVar_Shutdown(UILocalVarContext *context);
UILocalVarContext *__cdecl UILocalVar_Find(UILocalVarContext *context, const char *name);
char __cdecl UILocalVar_FindLocation(UILocalVarContext *context, const char *name, unsigned int *hashForName);
unsigned int __cdecl UILocalVar_HashName(const char *name);
UILocalVarContext *__cdecl UILocalVar_FindOrCreate(UILocalVarContext *context, char *name);
inline UILocalVarContext *__cdecl UILocalVar_FindOrCreate(UILocalVarContext *context, const char *name)
{
    return UILocalVar_FindOrCreate(context, (char *)name);
}

bool __cdecl UILocalVar_GetBool(const UILocalVar *var);
int __cdecl UILocalVar_GetInt(const UILocalVar *var);
double __cdecl UILocalVar_GetFloat(const UILocalVar *var);
char *__cdecl UILocalVar_GetString(const UILocalVar *var, char *stringBuf, unsigned int size);
void __cdecl UILocalVar_SetBool( UILocalVar *var, bool b);
void __cdecl UILocalVar_SetInt( UILocalVar *var, int i);
void __cdecl UILocalVar_SetFloat( UILocalVar *var, float f);
void __cdecl UILocalVar_SetString( UILocalVar *var, char *s);
