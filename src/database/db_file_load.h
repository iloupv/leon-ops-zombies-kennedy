#pragma once

#include <Windows.h>
#include <zlib/zlib.h>
#include "db_registry.h"

struct DB_LoadData // sizeof=0x807C
{                                       // XREF: .data:g_load/r
    void *f;                            // XREF: DB_CancelLoadXFile(void)+2E/r
                                        // DB_CancelLoadXFile(void):loc_57A38D/r ...
    const char *filename;               // XREF: DB_LoadXFileData(uchar *,int)+263/r
                                        // DB_LoadXFileData(uchar *,int)+304/r ...
    struct XBlock *blocks;                     // XREF: DB_LoadXFileInternal(void)+33B/r
                                        // DB_LoadXFileInternal(void)+354/r ...
    int flags;                          // XREF: DB_LoadXFileInternal(void)+32D/r
                                        // DB_LoadXFileInternal(void)+426/r ...
    int outstandingReads;               // XREF: DB_CancelLoadXFile(void):loc_57A341/r
                                        // DB_WaitXFileStage:loc_57A405/r ...
    OVERLAPPED overlapped;             // XREF: DB_ReadData:loc_57AA33/r
                                        // DB_ReadData+9B/o ...
    z_stream_s stream;                  // XREF: DB_CancelLoadXFile(void):loc_57A351/o
                                        // DB_WaitXFileStage+AD/r ...
    unsigned __int8 *compressBufferStart;
                                        // XREF: DB_CancelLoadXFile(void)+3/r
                                        // DB_LoadXFileData(uchar *,int)+288/r ...
    unsigned __int8 *compressBufferEnd; // XREF: DB_LoadXFileData(uchar *,int)+2C2/r
                                        // DB_LoadXFile(char const *,void *,char const *,XBlock *,void (*)(void),uchar *,int,int)+121/w
    void (__cdecl *interrupt)();        // XREF: DB_ReadData:loc_57AA24/r
                                        // DB_ReadData+6D/r ...
    int allocType;                      // XREF: DB_LoadXFileInternal(void)+334/r
                                        // DB_LoadXFile(char const *,void *,char const *,XBlock *,void (*)(void),uchar *,int,int)+73/w
    bool abort;                         // XREF: Load_XAssetArrayCustom+70/r
    // padding byte
    // padding byte
    // padding byte
    int startTime;                      // XREF: DB_LoadXFileInternal(void)+409/r
                                        // DB_LoadXFile(char const *,void *,char const *,XBlock *,void (*)(void),uchar *,int,int)+7E/w
    int deflateBufferPos;               // XREF: DB_LoadXFileDataNullTerminated(uchar *):loc_57A4BA/r
                                        // DB_LoadXFileDataNullTerminated(uchar *)+29/r ...
    int deflateRemainingFileSize;       // XREF: DB_LoadXFileSetSize(int)+3/r
                                        // DB_LoadXFileSetSize(int)+35/w ...
    unsigned __int8 deflateBuffer[32768];
                                        // XREF: DB_LoadXFileDataNullTerminated(uchar *)+2E/r
                                        // DB_LoadXFileData(uchar *,int)+C3/r ...
};

struct XFile // sizeof=0x24
{                                       // XREF: ?DB_LoadXFileInternal@@YAXXZ/r
    unsigned int size;                  // XREF: DB_LoadXFileInternal(void)+28B/r
    unsigned int externalSize;          // XREF: DB_LoadXFileInternal(void)+2B4/r
                                        // DB_LoadXFileInternal(void)+2EF/r
    unsigned int blockSize[7];          // XREF: DB_LoadXFileInternal(void)+348/o
};

double __cdecl DB_GetLoadedFraction();
bool __cdecl DB_FinishedLoadingAssets();
void __cdecl DB_ResetZoneSize(int trackLoadProgress);
bool __cdecl DB_IsMinimumFastFileLoaded();
void __cdecl DB_ResetMinimumFastFileLoaded();
void __cdecl DB_CancelLoadXFile();
signed __int32 DB_WaitXFileStage();
void __cdecl DB_LoadedExternalData(unsigned int size);
unsigned __int8 *__cdecl DB_LoadXFileDataNullTerminated(unsigned __int8 *pos);
void __cdecl DB_LoadXFileSetSize(int size);
void __cdecl DB_LoadXFileData(unsigned __int8 *pos, int size);
void DB_ReadXFileStage();
int __cdecl DB_ReadData();

void __cdecl DB_LoadXFileInternal();
void Load_XAssetListCustom();
void __cdecl Load_XAssetArrayCustom(int count);
char __cdecl DB_LoadXFile(
                const char *path,
                void *f,
                const char *filename,
                XBlock *blocks,
                void (__cdecl *interrupt)(),
                unsigned __int8 *buf,
                int allocType,
                int flags);

extern XAssetList *varXAssetList;

extern volatile unsigned int g_loadingAssets;
