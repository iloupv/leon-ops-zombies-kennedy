#pragma once

#include <stdio.h> // FILE?
#include <win32/win_common.h>
#include <universal/dvar.h>

enum FS_SERVER_COMPARE_RESULT : __int32
{                                       // XREF: ?FS_CompareWithServerFiles@@YA?AW4FS_SERVER_COMPARE_RESULT@@PADHH@Z/r
                                        // ?FS_CompareWithServerFiles@@YA?AW4FS_SERVER_COMPARE_RESULT@@PADHH@Z/r ...
    FILES_MATCH      = 0x0,
    NEED_DOWNLOAD    = 0x1,
    NOT_DOWNLOADABLE = 0x2,
};

enum fsMode_t : __int32
{                                       // XREF: ?FS_FOpenFileByMode@@YAHPBDPAHW4fsMode_t@@@Z/r
    FS_READ        = 0x0,
    FS_WRITE       = 0x1,
    FS_APPEND      = 0x2,
    FS_APPEND_SYNC = 0x3,
};

enum FsListBehavior_e : __int32
{                                       // XREF: FsListBehavior/r
                                        // ?FS_ListFilteredFiles@@YAPAPBDPAUsearchpath_s@@PBD11W4FsListBehavior_e@@PAHH@Z/r ...
    FS_LIST_PURE_ONLY = 0x0,
    FS_LIST_ALL       = 0x1,
};

enum FsThread : __int32
{                                       // XREF: ?FS_HandleForFile@@YAHPBDW4FsThread@@@Z/r
                                        // FS_BuildOSPathForThread/r ...
    FS_THREAD_MAIN     = 0x0,
    FS_THREAD_STREAM   = 0x1,
    FS_THREAD_DATABASE = 0x2,
    FS_THREAD_BACKEND  = 0x3,
    FS_THREAD_SERVER   = 0x4,
    FS_THREAD_COUNT    = 0x5,
    FS_THREAD_INVALID  = 0x6,
};

union qfile_gus // sizeof=0x4
{                                       // XREF: FS_SV_FOpenFileRead(char const *,char const *,int *)+BE/w
                                        // FS_SV_FOpenFileRead(char const *,char const *,int *)+CD/r ...
    FILE *o;
    unsigned __int8 *z;
};

struct qfile_us // sizeof=0x8
{                                       // XREF: fileHandleData_t/r
                                        // qfile_ut/r
    qfile_gus file;                     // XREF: FS_SV_FOpenFileRead(char const *,char const *,int *)+BE/w
                                        // FS_SV_FOpenFileRead(char const *,char const *,int *)+CD/r ...
    int iwdIsClone;                     // XREF: FS_FCloseFile(int)+AC/r
                                        // FS_FOpenFileReadForThread+43F/w ...
};

struct fileHandleData_t // sizeof=0x11C
{                                       // XREF: .data:fileHandleData_t * fsh/r
    qfile_us handleFiles;               // XREF: FS_SV_FOpenFileRead(char const *,char const *,int *)+BE/w
                                        // FS_SV_FOpenFileRead(char const *,char const *,int *)+CD/r ...
    int handleSync;                     // XREF: FS_SV_FOpenFileRead(char const *,char const *,int *)+9C/w
                                        // FS_GetHandleAndOpenFile+88/w ...
    int fileSize;                       // XREF: FS_Shutdown(int)+2A/r
                                        // FS_FOpenFileByMode(char const *,int *,fsMode_t)+C8/w
    int zipFilePos;                     // XREF: FS_FOpenFileReadForThread+5ED/w
                                        // FS_Seek(int,long,int)+6E/r ...
    struct iwd_t *zipFile;                     // XREF: FS_SV_FOpenFileRead(char const *,char const *,int *)+89/w
                                        // FS_FileForHandle(int)+42/r ...
    int streamed;                       // XREF: FS_FCloseFile(int)+12/r
                                        // FS_Seek(int,long,int)+14/r ...
    char name[256];                     // XREF: FS_SV_FOpenFileRead(char const *,char const *,int *)+217/o
                                        // FS_HandleForFile(char const *,FsThread)+1F7/o ...
};

struct fileInIwd_s // sizeof=0xC
{                                       // XREF: fileInIwd_t/r
    unsigned int pos;
    char *name;
    fileInIwd_s *next;
};

struct iwd_t // sizeof=0x324
{
    char iwdFilename[256];
    char iwdBasename[256];
    char iwdGamename[256];
    unsigned __int8 *handle;
    int checksum;
    int pure_checksum;
    //volatile int hasOpenFile;
    volatile unsigned int hasOpenFile;
    int numFiles;
    unsigned __int8 referenced;
    // padding byte
    // padding byte
    // padding byte
    unsigned int hashSize;
    fileInIwd_s **hashTable;
    fileInIwd_s *buildBuffer;
};

struct directory_t // sizeof=0x200
{
    char path[256];
    char gamedir[256];
};

struct searchpath_s // sizeof=0x1C
{                                       // XREF: searchpath_t/r
    searchpath_s *next;
    iwd_t *iwd;
    directory_t *dir;
    int bLocalized;
    int ignore;
    int ignorePureCheck;
    int language;
};

struct iwd_pure_check_s // sizeof=0x208
{                                       // XREF: iwd_pure_check_t/r
    iwd_pure_check_s *next;
    int checksum;
    char iwdBasename[256];
    char iwdGamename[256];
};

char *__cdecl FS_GetOsFolderPath(int folder, char *ospath);
bool __cdecl FS_Initialized();
void __cdecl FS_CheckFileSystemStarted();
int __cdecl FS_IwdIsPure(const iwd_t *iwd);
void __cdecl FS_DisablePureCheck(bool disable);
int __cdecl FS_LoadStack();
bool __cdecl FS_UseSearchPath(const searchpath_s *pSearch);
int __cdecl FS_LanguageHasAssets(int iLanguage);
int __cdecl FS_HashFileName(const char *fname, int hashSize);
int __cdecl FS_HandleForFile(const char *name, FsThread thread);
int __cdecl FS_HandleForFileCurrentThread(const char *filename);
FsThread __cdecl FS_GetCurrentThread();
FILE *__cdecl FS_FileForHandle(int f);
int __cdecl FS_filelength(int f);
void __cdecl FS_ReplaceSeparators(char *path);
void __cdecl FS_BuildOSPath(char *base, char *game, char *qpath, char *ospath);
void __cdecl FS_BuildOSPathForThread(char *base, char *game, char *qpath, char *ospath, FsThread thread);
int __cdecl FS_CreatePath(char *OSPath);
int __cdecl FS_FileExists(char *file);
int __cdecl FS_OSFPathExists(const char *file);
void __cdecl FS_CopyFile(char *fromOSPath, char *toOSPath);
void __cdecl FS_Remove(const char *osPath);
void __cdecl FS_FCloseFile(int h);
void __cdecl FS_FCloseLogFile(int h);
int __cdecl FS_FOpenFileWriteToDir(char *filename, char *dir, const char *osbasepath);
int __cdecl FS_FOpenFileWriteToDirForThread(char *filename, char *dir, const char *osbasepath, FsThread thread);
int __cdecl FS_GetHandleAndOpenFile(const char *filename, const char *ospath, FsThread thread);
int __cdecl FS_FOpenFileWrite(char *filename);
int __cdecl FS_FOpenFileWriteCurrentThread(char *filename, char *dir, const char *osbasepath);
int __cdecl FS_FOpenTextFileWrite(char *filename);
int __cdecl FS_FOpenFileAppend(char *filename);
int __cdecl FS_FilenameCompare(const char *s1, const char *s2);
bool __cdecl FS_PureIgnoreFiles(const char *filename);
int __cdecl FS_FOpenFileRead(const char *filename, int *file);
unsigned int __cdecl FS_FOpenFileReadCurrentThread(const char *filename, int *file);
unsigned int __cdecl FS_FOpenFileReadForThread(const char *filename, int *file, FsThread thread);
char __cdecl FS_SanitizeFilename(const char *filename, char *sanitizedName, int sanitizedNameSize);
bool __cdecl FS_IsBackupSubStr(const char *filenameSubStr);
char __cdecl FS_FilesAreLoadedGlobally(const char *filename);
bool __cdecl FS_TouchFile(const char *name);
bool __cdecl FS_Delete(char *filename);
unsigned int __cdecl FS_Read(unsigned __int8 *buffer, unsigned int len, int h);
unsigned int __cdecl FS_Write(char *buffer, unsigned int len, int h);
inline unsigned int __cdecl FS_Write(const char *buffer, unsigned int len, int h)
{
    return FS_Write((char *)buffer, len, h);
}
unsigned int __cdecl FS_WriteToDemo(char *buffer, unsigned int len, int h);
void FS_Printf(int h, char *fmt, ...);
int __cdecl FS_Seek(int f, int offset, int origin);
int __cdecl FS_ReadFile(const char *qpath, void **buffer);
char *__cdecl FS_AllocMem(int bytes);
void __cdecl FS_ResetFiles();
void __cdecl FS_FreeFile(void *buffer);
void __cdecl FS_FreeMem(void *buffer);
int __cdecl FS_WriteFile(char *filename, char *buffer, unsigned int size);
int __cdecl FS_GetFileOsPath(const char *filename, char *ospath);
int __cdecl FS_OpenFileOverwrite(const char *qpath);
const char **__cdecl FS_ListFilteredFiles(
                searchpath_s *searchPath,
                const char *path,
                char *extension,
                char *filter,
                FsListBehavior_e behavior,
                int *numfiles);
int __cdecl FS_ReturnPath(const char *zname, char *zpath, int *depth);
int __cdecl FS_AddFileToList(HunkUser *user, const char *name, const char **list, int nfiles);
const char **__cdecl FS_ListFiles(const char *path, char *extension, FsListBehavior_e behavior, int *numfiles);
const char **__cdecl FS_ListFilesInLocation(
                const char *path,
                char *extension,
                FsListBehavior_e behavior,
                int *numfiles,
                int lookInFlags);
const char **__cdecl FS_ListFilteredFilesInLocation(
                const char *path,
                char *extension,
                char *filter,
                FsListBehavior_e behavior,
                int *numfiles,
                int lookInFlags);
bool __cdecl FS_CheckLocation(const char *path, int lookInFlags);
void __cdecl FS_FreeFileList(const char **list);
int __cdecl FS_GetFileList(const char *path, char *extension, FsListBehavior_e behavior, char *listbuf, int bufsize);
void __cdecl FS_ConvertPath(char *s);
int __cdecl FS_PathCmp(const char *s1, const char *s2);
void __cdecl FS_SortFileList(const char **filelist, int numfiles);
void __cdecl FS_DisplayPath(int bLanguageCull);
void __cdecl FS_FullPath_f();
void __cdecl FS_Path_f();
void __cdecl FS_AddUserMapDirIWDs(char *pszGameFolder);
void __cdecl FS_AddIwdFilesForGameDirectory(char *path, char *pszGameFolder);
iwd_t *__cdecl FS_LoadZipFile(char *zipfile, const char *basename);
iwd_t *__cdecl FS_LoadFakeIwdFile(char *fakeiwdfile, const char *basename);
char *__cdecl IwdFileLanguage(const char *pszIwdFileName);
int __cdecl iwdsort(const char **a, char **b);
void __cdecl FS_AddSearchPath(searchpath_s *search);
void __cdecl FS_ShutdownServerIwdNames();
void __cdecl FS_ShutdownServerFileReferences(int *numFiles, const char **fileNames);
void __cdecl FS_ShutdownServerReferencedIwds();
void __cdecl FS_ShutdownServerReferencedFFs();
void __cdecl FS_Shutdown();
void __cdecl FS_ShutdownSearchPaths(searchpath_s *p);
void __cdecl FS_Startup(const char *gameName, bool allow_devraw);
void __cdecl FS_AddGameDirectory(char *path, const char *dir, int bLanguageDirectory, int iLanguage);
void __cdecl FS_AddLocalizedGameDirectory(char *path, const char *dir);
void FS_RegisterDvars();
void __cdecl FS_AddDevGameDirs(char *path, bool allow_devraw);
void __cdecl FS_ClearIwdReferences();
void __cdecl FS_AddIwdPureCheckReference(const searchpath_s *search);
void __cdecl FS_ShutDownIwdPureCheckReferences();
bool __cdecl FS_IsBasePathValid();
void __cdecl FS_InitFilesystem(bool allow_devraw);
void __cdecl FS_Restart(int localClientNum, int checksumFeed);
bool __cdecl FS_NeedRestart(int checksumFeed);
unsigned int __cdecl FS_FOpenFileByMode(char *qpath, int *f, fsMode_t mode);
unsigned int __cdecl FS_FTell(int f);
void __cdecl FS_Flush(int f);
void __cdecl Com_GetBspFilename(char *filename, unsigned int size, const char *mapname);

extern int fs_numServerReferencedIwds;
extern const char *fs_serverIwdNames[1024];
extern int fs_numServerIwds;

extern const dvar_t *fs_debug;
extern const dvar_t *fs_copyfiles;
extern const dvar_t *fs_cdpath;
extern const dvar_t *fs_basepath;
extern const dvar_t *fs_basegame;
extern const dvar_t *fs_gameDirVar;
extern const dvar_t *fs_modsVar; // mod: fs_mods (a stack of mods/<name> folders, see FS_Startup)
bool FS_ModsActive(); // mod: fs_game or fs_mods set (the retail 'fs_game set' gates for reading mod files)
bool FS_ModStackActive(); // mod: fs_mods set
int FS_ModStackNames(char (*names)[64], int max); // mod: fs_game's mod (if not listed), then fs_mods, in load order
extern const dvar_t *fs_usermapDir;
extern const dvar_t *fs_ignoreLocalized;
extern const dvar_t *fs_homepath;
extern const dvar_t *fs_userDocuments;
extern const dvar_t *fs_restrict;
extern const dvar_t *fs_usedevdir;

extern char fs_gamedir[256];


extern fileHandleData_t fsh[70];

extern searchpath_s *fs_searchpaths;

extern const char *fs_serverReferencedIwdNames[1024];
extern int fs_serverReferencedIwds[1024];
extern int fs_numServerReferencedFFs;
extern const char *fs_serverReferencedFFNames[32];
extern int fs_serverReferencedFFCheckSums[32];
extern iwd_pure_check_s *fs_iwdPureChecks;
extern int fs_checksumFeed;
extern int fs_fakeChkSum;
extern int fs_serverIwds[1024];

extern int com_fileAccessed;
