#pragma once

char *__cdecl Com_LoadInfoString(char *fileName, const char *fileDesc, const char *ident, char *loadBuffer);
char *__cdecl Com_LoadInfoString_LoadObj(char *fileName, const char *fileDesc, const char *ident, char *loadBuffer);
const char *__cdecl Com_LoadInfoString_FastFile(const char *fileName, const char *fileDesc, const char *ident);
char *__cdecl Com_LoadRawTextFile(const char *fullpath);
char *__cdecl Com_LoadRawTextFile_LoadObj(const char *fullpath);
char *__cdecl Com_LoadRawTextFile_FastFile(const char *fullpath);
void __cdecl Com_UnloadRawTextFile(char *filebuf);
