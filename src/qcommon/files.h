#pragma once

enum FS_SERVER_COMPARE_RESULT : __int32;

int __cdecl FS_SV_FileExists(char *file, char *dir);
int __cdecl FS_SV_FOpenFileWrite(char *filename, char *dir);
int __cdecl FS_SV_FOpenFileRead(char *filename, char *dir, int *fp);
void __cdecl FS_SV_Rename(char *from, char *to, char *dir);
int __cdecl FS_GetModList(char *listbuf, int bufsize);
int __cdecl Sys_CountFileList(char **list);
void __cdecl FS_Dir_f();
void __cdecl FS_NewDir_f();
void __cdecl FS_TouchFile_f();
int __cdecl FS_iwIwd(char *iwd, char *base);
bool __cdecl FS_serverPak(const char *pak);
FS_SERVER_COMPARE_RESULT __cdecl FS_CompareWithServerFiles(char *neededFiles, int len, int dlstring);
FS_SERVER_COMPARE_RESULT __cdecl FS_CompareIwds(char *needediwds, int len, int dlstring);
FS_SERVER_COMPARE_RESULT __cdecl FS_CompareFFs(char *neededFFs, int len, int dlstring);
void __cdecl FS_RemoveCommands();
void __cdecl FS_AddCommands();
void __cdecl FS_SetRestrictions();
void __cdecl FS_LoadedIwds(const char **checksums, const char **names);
char *__cdecl FS_LoadedIwdPureChecksums();
void __cdecl FS_ReferencedIwds(const char **checksums, const char **names);
char *__cdecl FS_ReferencedIwdPureChecksums();
char __cdecl FS_PureServerSetLoadedIwds(char *iwdSums, char *iwdNames);
void __cdecl FS_ServerSetReferencedIwds(const char *iwdSums, const char *iwdNames);
int __cdecl FS_ServerSetReferencedFiles(
                const char *fileSums,
                const char *fileNames,
                int maxFiles,
                int *fs_sums,
                const char **fs_names);
void __cdecl FS_ServerSetReferencedFFs(const char *FFSums, const char *FFNames);
char *__cdecl FS_GetMapBaseName(char *mapname);
