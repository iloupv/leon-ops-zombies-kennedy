#pragma once
#include <database/db_registry.h>
#include <universal/dvar.h>

int __cdecl UI_ParseInfos(const char *buf, int max, char **infos);
void __cdecl UI_LoadArenas();
char __cdecl UI_LoadModArenas();
void __cdecl UI_LoadModsMap(FF_DIR source);
int UI_LoadArenasFromFile();
const char *UI_LoadArenasFromFile_LoadObj();
void UI_LoadArenasFromFile_FastFile();
void __cdecl UI_LoadMaps();
void __cdecl UI_LoadCustomMatchGameTypes();

extern const dvar_t *ui_browserMap;