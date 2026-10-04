#pragma once
#include "live_storage.h"

struct WADEntry // sizeof=0x2C
{
    char name[32];
    unsigned int compressedSize;
    unsigned int size;
    unsigned int offset;
};

bool __cdecl LiveStorage_DoWeHaveContracts();
bool __cdecl LiveStorage_DoWeHavePlaylists();
void __cdecl LiveStorage_SetHavePlaylists(bool val);
TaskRecord *__cdecl LiveStorage_FetchOnlineWAD(int controllerIndex);
int __cdecl LiveStorage_FetchOnlineWADNotFound();
bool __cdecl LiveStorage_DoWeHaveOnlineWAD();
void __cdecl LiveStorage_RefetchOnlineWAD();
bool __cdecl LiveStorage_CompletedOnlineWAD();
bool __cdecl LiveStorage_FetchingOnlineWAD();
void __cdecl LiveStorage_ForceWADFetch();
void __cdecl LiveStorage_HandlePublisherStorageTasks();
void LiveStorage_ProcessOnlineWAD();
void __cdecl tlEndianSwapMemory32(int *Data, unsigned int Bytes);
bool __cdecl LiveStorage_DecompressBuffer(
                unsigned __int8 *compressedBuffer,
                unsigned int compressedSize,
                unsigned __int8 *uncompressedBuffer,
                unsigned int uncompressedSize);
