#pragma once
#include <qcommon/common.h>

void __cdecl G_SetEntityParsePoint(const char *beginParsePoint);
void __cdecl G_ResetEntityParsePoint();
const char *__cdecl G_GetEntityParsePoint();
int __cdecl G_GetEntityToken(char *buffer, int bufferSize);
int __cdecl G_ParseSpawnVars(SpawnVar *spawnVar);
char *__cdecl G_AddSpawnVarToken(char *string, SpawnVar *spawnVar);
int __cdecl G_SpawnString(const SpawnVar *spawnVar, const char *key, const char *defaultString, const char **out);
unsigned int __cdecl G_NewString(const char *string);
char *__cdecl vtos(const float *v);
