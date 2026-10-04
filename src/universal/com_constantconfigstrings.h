#pragma once

void __cdecl CCS_ClearConstantConfigStrings();
void __cdecl CCS_ValidateChecksums(int serverChecksum, int clientChecksum);
int __cdecl CCS_GetChecksum();
char __cdecl CCS_ShouldLoadConstConfigStrings(int party);
void __cdecl CCS_LoadConstantConfigStrings(const char *mapname, const char *gametype);
const char *__cdecl CCS_GetConfigStringValue(int rowNum);
int __cdecl CCS_GetConfigStringNum(int rowNum);
