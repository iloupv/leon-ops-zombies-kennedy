#include "com_fileaccess.h"
#include <qcommon/com_profilemapload.h>
#include <string.h>
#include "assertive.h"
#include "q_shared.h"

unsigned int __cdecl FS_FileRead(void *ptr, unsigned int len, FILE *stream)
{
    unsigned int read_size; // [esp+0h] [ebp-4h]

    ProfLoad_BeginTrackedValue(MAP_PROFILE_FILE_READ);
    read_size = fread(ptr, 1u, len, stream);
    ProfLoad_EndTrackedValue(MAP_PROFILE_FILE_READ);
    return read_size;
}

unsigned int __cdecl FS_FileWrite(const void *ptr, unsigned int len, FILE *stream)
{
    return fwrite(ptr, 1u, len, stream);
}

FILE *__cdecl FileWrapper_Open(const char *ospath, const char *mode)
{
    int *v3; // eax
    const char *v4; // eax
    const char *v5; // eax
    FILE *file; // [esp+0h] [ebp-4h]

    *_errno() = 0;
    file = fopen(ospath, mode);
    if ( file != (FILE *)-1 )
        return file;
    v3 = _errno();
    v4 = strerror(*v3);
    v5 = va("Couldn't open file: %s %s", ospath, v4);
    if ( !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\qcommon\\../universal/com_files_wrapper_stdio.h",
                    24,
                    0,
                    v5) )
        __debugbreak();
    return 0;
}

FILE *__cdecl FS_FileOpenReadBinary(const char *filename)
{
    FILE *file; // [esp+4h] [ebp-4h]

    ProfLoad_BeginTrackedValue(MAP_PROFILE_FILE_OPEN);
    file = FileWrapper_Open(filename, "rb");
    ProfLoad_EndTrackedValue(MAP_PROFILE_FILE_OPEN);
    return file;
}

FILE *__cdecl FS_FileOpenReadText(const char *filename)
{
    FILE *file; // [esp+4h] [ebp-4h]

    ProfLoad_BeginTrackedValue(MAP_PROFILE_FILE_OPEN);
    file = FileWrapper_Open(filename, "rt");
    ProfLoad_EndTrackedValue(MAP_PROFILE_FILE_OPEN);
    return file;
}

FILE *__cdecl FS_FileOpenWriteBinary(const char *filename)
{
    FILE *file; // [esp+4h] [ebp-4h]

    ProfLoad_BeginTrackedValue(MAP_PROFILE_FILE_OPEN);
    file = FileWrapper_Open(filename, "wb");
    ProfLoad_EndTrackedValue(MAP_PROFILE_FILE_OPEN);
    return file;
}

FILE *__cdecl FS_FileOpenAppendText(const char *filename)
{
    FILE *file; // [esp+4h] [ebp-4h]

    ProfLoad_BeginTrackedValue(MAP_PROFILE_FILE_OPEN);
    file = FileWrapper_Open(filename, "at");
    ProfLoad_EndTrackedValue(MAP_PROFILE_FILE_OPEN);
    return file;
}

FILE *__cdecl FS_FileOpenWriteText(const char *filename)
{
    FILE *file; // [esp+4h] [ebp-4h]

    ProfLoad_BeginTrackedValue(MAP_PROFILE_FILE_OPEN);
    file = FileWrapper_Open(filename, "w+t");
    ProfLoad_EndTrackedValue(MAP_PROFILE_FILE_OPEN);
    return file;
}

void __cdecl FS_FileClose(FILE *stream)
{
    fclose(stream);
}

int __cdecl FS_FileSeek(FILE *file, int offset, int whence)
{
    int seek; // [esp+4h] [ebp-4h]

    ProfLoad_BeginTrackedValue(MAP_PROFILE_FILE_SEEK);
    seek = FileWrapper_Seek(file, offset, whence);
    ProfLoad_EndTrackedValue(MAP_PROFILE_FILE_SEEK);
    return seek;
}

int __cdecl FileWrapper_Seek(FILE *h, int offset, int origin)
{
    const char *v4; // eax

    switch ( origin )
    {
        case 0:
            return fseek(h, offset, 1);
        case 1:
            return fseek(h, offset, 2);
        case 2:
            return fseek(h, offset, 0);
    }
    v4 = va("Bad origin %i in FS_Seek", origin);
    if ( !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\qcommon\\../universal/com_files_wrapper_stdio.h",
                    111,
                    0,
                    v4) )
        __debugbreak();
    return 0;
}

int __cdecl FS_FileGetFileSize(FILE *file)
{
    return FileWrapper_GetFileSize(file);
}

int __cdecl FileWrapper_GetFileSize(FILE *h)
{
    int startPos; // [esp+0h] [ebp-8h]
    int fileSize; // [esp+4h] [ebp-4h]

    startPos = ftell(h);
    fseek(h, 0, 2);
    fileSize = ftell(h);
    fseek(h, startPos, 0);
    return fileSize;
}

