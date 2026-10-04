#pragma once

int __cdecl Win_InitLocalization();
void __cdecl Win_ShutdownLocalization();
char *__cdecl Win_LocalizeRef(const char *ref);
char *__cdecl Win_CopyLocalizationString(const char *string);
char *__cdecl Win_GetLanguage();
