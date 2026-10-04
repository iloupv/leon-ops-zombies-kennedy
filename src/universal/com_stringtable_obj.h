#pragma once

struct StringTable;

int __cdecl StringTable_HashString(const char *string);
void __cdecl StringTable_ParseFileIntoTable(const char *filename, StringTable *table);
unsigned __int8 *__cdecl StringTable_Alloc(unsigned int size);
int __cdecl CellCompare(__int16 *a, __int16 *b);
void __cdecl StringTable_GetAsset_LoadObj(const char *filename, StringTable **tablePtr);
