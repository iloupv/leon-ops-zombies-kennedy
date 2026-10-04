#pragma once
#include "assertive.h"
#include <cstring>

struct MemoryFile // sizeof=0x8024
{                                       // XREF: SaveGame/r
                                        // ?UI_Gametype_UploadToFileShare_f@@YAXXZ/r ...
    unsigned __int8 *buffer;
    int bufferSize;
    int bytesUsed;
    int cacheBufferUsed;
    int segmentIndex;
    int segmentStart;
    int nonZeroCount;
    int zeroCount;
    bool errorOnOverflow;
    bool memoryOverflow;
    bool compress_enabled;
    bool rle_enabled;
    bool is_writing;
    // padding byte
    // padding byte
    // padding byte
    int cacheBufferAvail;
    unsigned __int8 cacheBuffer[32760];
};

void __cdecl MemFile_CommonInit(
                MemoryFile *memFile,
                int size,
                unsigned __int8 *buffer,
                bool errorOnOverflow,
                bool compress_enabled);
void __cdecl MemFile_InitForReading(MemoryFile *memFile, int size, unsigned __int8 *buffer, bool compress_enabled);
void __cdecl MemFile_InitForWriting(
                MemoryFile *memFile,
                int size,
                unsigned __int8 *buffer,
                bool errorOnOverflow,
                bool compress_enabled);
void __cdecl MemFile_StartSegment(MemoryFile *memFile, int index);
void __cdecl MemFile_EndSegment(MemoryFile *memFile);
void __cdecl MemFile_MoveToSegment(MemoryFile *memFile, int index);
void __cdecl MemFile_inflateInit(MemoryFile *memFile);
unsigned __int8 *__cdecl MemFile_GetSegmentAddess(MemoryFile *memFile, unsigned int index);
bool __cdecl MemFile_IsReading(MemoryFile *memFile);
void __cdecl MemFile_WriteDataFlushInternal(MemoryFile *memFile);
void __cdecl MemFile_WriteDataInternal(MemoryFile *memFile, unsigned int byteCount, unsigned __int8 *p);
char *__cdecl MemFile_ReadCString(MemoryFile *memFile);
void __cdecl MemFile_ReadData(MemoryFile *memFile, int byteCount, unsigned __int8 *p);

void __cdecl MemFile_WriteInt(MemoryFile *memFile, int value);
void __cdecl MemFile_WriteFloat(MemoryFile *memFile, float valuef);


inline void __cdecl MemFile_WriteData(MemoryFile *memFile, unsigned int byteCount, unsigned __int8 *p)
{
    if ((int)(byteCount + memFile->cacheBufferUsed) < 32760)
    {
        memcpy(&memFile->cacheBuffer[memFile->cacheBufferUsed], p, byteCount);
        memFile->cacheBufferUsed += byteCount;
    }
    else
    {
        MemFile_WriteDataInternal(memFile, byteCount, p);
    }
}

inline void __cdecl MemFile_WriteCString(MemoryFile *memFile, char *string)
{
    if (!string
        && !Assert_MyHandler(
            "c:\\projects_pc\\cod\\codsrc\\src\\effectscore\\../universal/memfile.h",
            144,
            0,
            "%s",
            "string"))
    {
        __debugbreak();
    }
    MemFile_WriteData(memFile, strlen(string) + 1, (unsigned __int8 *)string);
}
