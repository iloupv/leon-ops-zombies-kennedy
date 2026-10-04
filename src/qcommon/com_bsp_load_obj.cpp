#include "com_bsp_load_obj.h"
#include <universal/com_files.h>
#include <universal/com_memory.h>
#include "com_profilemapload.h"
#include <gfx_d3d/r_water_sim.h>
#include "com_bsp.h"
#include "md4.h"
#include <database/db_registry.h>

const char *connectionString_126[11] =
{
  "CA_DISCONNECTED",
  "CA_CINEMATIC",
  "CA_UICINEMATIC",
  "CA_LOGO",
  "CA_CONNECTING",
  "CA_CHALLENGING",
  "CA_CONNECTED",
  "CA_SENDINGSTATS",
  "CA_LOADING",
  "CA_PRIMED",
  "CA_ACTIVE"
};


struct $770C4B51799C51B11A150CADEE7E2360 // sizeof=0x54
{                                       // XREF: .data:comBspGlob/r
    char name[64];                      // XREF: Com_UnloadBsp(void)+8C/w
    BspHeader *header;                  // XREF: Com_IsBspLoaded(void)+5/r
                                        // Com_GetBspLump(LumpType,uint,uint *):loc_6CABF5/r ...
    unsigned int fileSize;              // XREF: Com_ValidateBspLumpData+45/r
                                        // Com_CheckVersionLumpCountError+56/r ...
    unsigned int checksum;              // XREF: Com_GetBspChecksum(void):loc_6CB265/r
                                        // Com_LoadBsp(char const *)+182/w
    LumpType loadedLumpType;            // XREF: Com_LoadBspLump(char const *,LumpType,uint,uint *)+12D/w
                                        // Com_UnloadBspLump(LumpType):loc_6CB162/r ...
    const void *loadedLumpData;         // XREF: Com_LoadBspLump(char const *,LumpType,uint,uint *)+13/r
                                        // Com_LoadBspLump(char const *,LumpType,uint,uint *)+B4/w ...
} comBspGlob;

bool __cdecl Com_IsBspLoaded()
{
    return comBspGlob.header != 0;
}

char *__cdecl Com_GetBspLump(LumpType type, unsigned int elemSize, unsigned int *count)
{
    unsigned int chunkIter; // [esp+0h] [ebp-10h]
    unsigned int offset; // [esp+4h] [ebp-Ch]

    if ( !Com_IsBspLoaded()
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\com_bsp_load_obj.cpp",
                    100,
                    0,
                    "%s",
                    "Com_IsBspLoaded()") )
    {
        __debugbreak();
    }
    if ( comBspGlob.header->version > 0x12 )
    {
        offset = 8 * comBspGlob.header->chunkCount + 12;
        for ( chunkIter = 0; chunkIter < comBspGlob.header->chunkCount; ++chunkIter )
        {
            if ( comBspGlob.header->chunks[chunkIter].type == type )
                return Com_ValidateBspLumpData(type, offset, comBspGlob.header->chunks[chunkIter].length, elemSize, count);
            offset += (comBspGlob.header->chunks[chunkIter].length + 3) & 0xFFFFFFFC;
        }
        *count = 0;
        return 0;
    }
    else if ( type < (unsigned int)Com_GetBspLumpCountForVersion(comBspGlob.header->version) )
    {
        return Com_ValidateBspLumpData(
                         type,
                         comBspGlob.header->chunks[type].type,
                         *(&comBspGlob.header->chunkCount + 2 * type),
                         elemSize,
                         count);
    }
    else
    {
        *count = 0;
        return 0;
    }
}

char *__cdecl Com_GetBspLumpCountForVersion(int version)
{
    if ( (version < 6 || version > 18)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\com_bsp_load_obj.cpp",
                    66,
                    0,
                    "version not in [OLDEST_BSP_VERSION, 18]\n\t%i not in [%i, %i]",
                    version,
                    6,
                    18) )
    {
        __debugbreak();
    }
    return (char*)connectionString_126[version + 8];
}

char *__cdecl Com_ValidateBspLumpData(
                LumpType type,
                unsigned int offset,
                unsigned int length,
                unsigned int elemSize,
                unsigned int *count)
{
    if ( !count
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\com_bsp_load_obj.cpp", 74, 0, "%s", "count") )
    {
        __debugbreak();
    }
    if ( length )
    {
        if ( length + offset > comBspGlob.fileSize )
            Com_Error(ERR_DROP, "LoadMap: lump %i extends past end of file", type);
        *count = length / elemSize;
        if ( elemSize * *count != length )
            Com_Error(ERR_DROP, "LoadMap: lump %i has funny size", type);
        return (char *)comBspGlob.header + offset;
    }
    else
    {
        *count = 0;
        return 0;
    }
}

const void *__cdecl Com_LoadBspLump(char *mapname, LumpType type, unsigned int elemSize, unsigned int *count)
{
    char *v4; // eax
    const char *v5; // eax
    char filename[260]; // [esp+28h] [ebp-110h] BYREF
    int fileSize; // [esp+130h] [ebp-8h]
    int h; // [esp+134h] [ebp-4h] BYREF

    if ( comBspGlob.loadedLumpData
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\com_bsp_load_obj.cpp",
                    212,
                    1,
                    "%s",
                    "comBspGlob.loadedLumpData == NULL") )
    {
        __debugbreak();
    }
    if ( Com_IsBspLoaded() )
    {
        v4 = strstr(comBspGlob.name, mapname);
        if ( !v4 )
        {
            v5 = va("%s, %s", comBspGlob.name, mapname);
            if ( !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\com_bsp_load_obj.cpp",
                            216,
                            0,
                            "%s\n\t%s",
                            "strstr( comBspGlob.name, mapname )",
                            v5) )
                __debugbreak();
        }
        comBspGlob.loadedLumpData = Com_GetBspLump(type, elemSize, count);
    }
    else
    {
        Com_GetBspFilename(filename, 0x100u, mapname);
        fileSize = FS_FOpenFileRead(filename, &h);
        if ( fileSize < 0 )
        {
            *count = 0;
            return 0;
        }
        Hunk_CheckTempMemoryHighClear();
        comBspGlob.loadedLumpData = Com_ReadLumpOutOfBsp(h, type, elemSize, count);
        FS_FCloseFile(h);
    }
    comBspGlob.loadedLumpType = type;
    return comBspGlob.loadedLumpData;
}

unsigned __int8 *__cdecl Com_ReadLumpOutOfBsp(int h, LumpType type, unsigned int elemSize, unsigned int *count)
{
    unsigned int ident; // [esp+0h] [ebp-28h] BYREF
    unsigned int chunkIter; // [esp+4h] [ebp-24h]
    unsigned int chunkCount; // [esp+8h] [ebp-20h] BYREF
    unsigned int version; // [esp+Ch] [ebp-1Ch] BYREF
    unsigned int offset; // [esp+10h] [ebp-18h]
    BspChunk chunk; // [esp+14h] [ebp-14h] BYREF
    BspLump lump; // [esp+1Ch] [ebp-Ch] BYREF
    int readSize; // [esp+24h] [ebp-4h]

    *count = 0;
    if ( FS_Read((unsigned __int8 *)&ident, 4u, h) != 4 || ident != 1347633737 )
        return 0;
    if ( FS_Read((unsigned __int8 *)&version, 4u, h) != 4 )
        return 0;
    if ( version > 0x12 )
    {
        if ( FS_Read((unsigned __int8 *)&chunkCount, 4u, h) == 4 )
        {
            offset = 8 * chunkCount + 12;
            for ( chunkIter = 0; chunkIter < chunkCount; ++chunkIter )
            {
                if ( FS_Read((unsigned __int8 *)&chunk, 8u, h) != 8 )
                    return 0;
                if ( chunk.type == type )
                    return Com_ReadLumpOutOfBspAtOffset(h, type, offset, chunk.length, elemSize, count);
                offset += (chunk.length + 3) & 0xFFFFFFFC;
            }
            return 0;
        }
        else
        {
            return 0;
        }
    }
    else if ( type < (unsigned int)Com_GetBspLumpCountForVersion(version) )
    {
        readSize = 8 * type;
        FS_Seek(h, 8 * type, 0);
        if ( FS_Read((unsigned __int8 *)&lump, 8u, h) == 8 )
            return Com_ReadLumpOutOfBspAtOffset(h, type, lump.offset, lump.length, elemSize, count);
        else
            return 0;
    }
    else
    {
        return 0;
    }
}

unsigned __int8 *__cdecl Com_ReadLumpOutOfBspAtOffset(
                int h,
                LumpType type,
                unsigned int offset,
                unsigned int length,
                unsigned int elemSize,
                unsigned int *count)
{
    unsigned __int8 *data; // [esp+0h] [ebp-4h]

    if ( !length )
        return 0;
    if ( length % elemSize )
        Com_Error(ERR_DROP, "LoadMap: lump %i has funny size", type);
    FS_Seek(h, offset, 2);
    data = (unsigned __int8 *)Hunk_AllocateTempMemoryHigh(length, "Com_ReadLumpOutOfBsp");
    if ( FS_Read(data, length, h) != length )
        return 0;
    *count = length / elemSize;
    return data;
}

void __cdecl Com_UnloadBspLump(LumpType type)
{
    if ( !comBspGlob.loadedLumpData
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\com_bsp_load_obj.cpp",
                    240,
                    1,
                    "%s",
                    "comBspGlob.loadedLumpData != NULL") )
    {
        __debugbreak();
    }
    if ( comBspGlob.loadedLumpType != type
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\com_bsp_load_obj.cpp",
                    241,
                    1,
                    "comBspGlob.loadedLumpType == type\n\t%i, %i",
                    comBspGlob.loadedLumpType,
                    type) )
    {
        __debugbreak();
    }
    if ( !Com_IsBspLoaded() )
        Hunk_ClearTempMemoryHigh();
    comBspGlob.loadedLumpData = 0;
}

bool __cdecl Com_BspHasLump(LumpType type)
{
    unsigned int count; // [esp+0h] [ebp-4h] BYREF

    Com_GetBspLump(type, 1u, &count);
    return count != 0;
}

unsigned int __cdecl Com_GetBspVersion()
{
    if ( !Com_IsBspLoaded()
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\com_bsp_load_obj.cpp",
                    261,
                    0,
                    "%s",
                    "Com_IsBspLoaded()") )
    {
        __debugbreak();
    }
    return comBspGlob.header->version;
}

unsigned int __cdecl Com_GetBspChecksum()
{
    if ( !Com_IsBspLoaded()
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\com_bsp_load_obj.cpp",
                    269,
                    0,
                    "%s",
                    "Com_IsBspLoaded()") )
    {
        __debugbreak();
    }
    return comBspGlob.checksum;
}

bool __cdecl Com_BspError()
{
    return comBspGlob.header->ident != 1347633737
            || comBspGlob.header->version < 6
            || comBspGlob.header->version > 0x2D
            || Com_CheckVersionLumpCountError(comBspGlob.header->version) != 0;
}

char __cdecl Com_CheckVersionLumpCountError(int version)
{
    if ( (version < 6 || version > 45)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\com_bsp_load_obj.cpp",
                    277,
                    0,
                    "version not in [OLDEST_BSP_VERSION, BSP_VERSION]\n\t%i not in [%i, %i]",
                    version,
                    6,
                    45) )
    {
        __debugbreak();
    }
    if ( comBspGlob.header->version > 0x12 )
    {
        if ( comBspGlob.fileSize < 8 * comBspGlob.header->chunkCount + 12 )
            return 1;
    }
    else if ( comBspGlob.fileSize < 8 * (int)Com_GetBspLumpCountForVersion(version) + 8 )
    {
        return 1;
    }
    return 0;
}

void __cdecl Com_LoadBsp(char *filename)
{
    const char *v1; // eax
    const char *v2; // eax
    const char *v3; // eax
    unsigned int v4; // edx
    unsigned int bytesRead; // [esp+18h] [ebp-Ch]
    unsigned int len; // [esp+1Ch] [ebp-8h]
    int h; // [esp+20h] [ebp-4h] BYREF

    if ( !filename
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\com_bsp_load_obj.cpp", 312, 0, "%s", "filename") )
    {
        __debugbreak();
    }
    if ( Com_IsBspLoaded()
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\com_bsp_load_obj.cpp",
                    313,
                    0,
                    "%s",
                    "!Com_IsBspLoaded()") )
    {
        __debugbreak();
    }
    if ( comBspGlob.loadedLumpData
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\com_bsp_load_obj.cpp",
                    314,
                    0,
                    "%s",
                    "comBspGlob.loadedLumpData == NULL") )
    {
        __debugbreak();
    }
    ProfLoad_Begin("Load bsp file");
    comBspGlob.fileSize = FS_FOpenFileRead(filename, &h);
    if ( !h )
    {
        v1 = va("EXE_ERR_COULDNT_LOAD", filename);
        Com_Error(ERR_DROP, v1);
    }
    comBspGlob.header = (BspHeader *)Z_MallocGarbage(comBspGlob.fileSize, "Com_LoadBsp", 11);
    bytesRead = FS_Read((unsigned __int8 *)comBspGlob.header, comBspGlob.fileSize, h);
    FS_FCloseFile(h);
    if ( bytesRead != comBspGlob.fileSize )
    {
        Z_Free((char *)comBspGlob.header, 11);
        v2 = va("EXE_ERR_COULDNT_LOAD", filename);
        Com_Error(ERR_DROP, v2);
    }
    ProfLoad_Begin("Bsp checksum");
    comBspGlob.checksum = Com_BlockChecksumKey32((const unsigned __int8 *)comBspGlob.header, comBspGlob.fileSize, 0);
    ProfLoad_End();
    if ( Com_BspError() )
    {
        Z_Free((char *)comBspGlob.header, 11);
        comBspGlob.header = 0;
        v3 = va("EXE_ERR_WRONG_MAP_VERSION_NUM", filename);
        Com_Error(ERR_DROP, v3);
    }
    ProfLoad_End();
    v4 = strlen(filename);
    len = v4;
    if ( v4 >= 0x40
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\com_bsp_load_obj.cpp",
                    345,
                    0,
                    "%s\n\t(len) = %i",
                    "(len < (sizeof( comBspGlob.name ) / (sizeof( comBspGlob.name[0] ) * (sizeof( comBspGlob.name ) != 4 || sizeof("
                    " comBspGlob.name[0] ) <= 4))))",
                    v4) )
    {
        __debugbreak();
    }
    memcpy((unsigned __int8 *)&comBspGlob, (unsigned __int8 *)filename, len + 1);
    if ( !Com_IsBspLoaded()
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\com_bsp_load_obj.cpp",
                    348,
                    0,
                    "%s",
                    "Com_IsBspLoaded()") )
    {
        __debugbreak();
    }
}

void __cdecl Com_UnloadBsp()
{
    if ( !Com_IsBspLoaded()
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\com_bsp_load_obj.cpp",
                    354,
                    0,
                    "%s",
                    "Com_IsBspLoaded()") )
    {
        __debugbreak();
    }
    if ( comBspGlob.loadedLumpData
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\com_bsp_load_obj.cpp",
                    355,
                    0,
                    "%s",
                    "comBspGlob.loadedLumpData == NULL") )
    {
        __debugbreak();
    }
    ProfLoad_Begin("Unload bsp file");
    Z_Free((char *)comBspGlob.header, 11);
    comBspGlob.header = 0;
    comBspGlob.name[0] = 0;
    ProfLoad_End();
    if ( Com_IsBspLoaded()
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\com_bsp_load_obj.cpp",
                    365,
                    0,
                    "%s",
                    "!Com_IsBspLoaded()") )
    {
        __debugbreak();
    }
}

void __cdecl Com_CleanupBsp()
{
    comBspGlob.loadedLumpData = 0;
    if ( Com_IsBspLoaded() )
        Com_UnloadBsp();
    if ( Com_IsBspLoaded()
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\com_bsp_load_obj.cpp",
                    376,
                    0,
                    "%s",
                    "!Com_IsBspLoaded()") )
    {
        __debugbreak();
    }
}

char *__cdecl Com_EntityString(int *numEntityChars)
{
    char *entityString; // [esp+0h] [ebp-8h]
    unsigned int count; // [esp+4h] [ebp-4h] BYREF

    if ( !Com_IsBspLoaded()
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\com_bsp_load_obj.cpp",
                    385,
                    0,
                    "%s",
                    "Com_IsBspLoaded()") )
    {
        __debugbreak();
    }
    entityString = Com_GetBspLump(LUMP_ENTITIES, 1u, &count);
    if ( numEntityChars )
        *numEntityChars = count;
    return entityString;
}

void __cdecl Com_SaveLump(LumpType type, const void *newLump, unsigned int size, ComSaveLumpBehavior behavior)
{
    char v4; // [esp+3h] [ebp-535h]
    char *v5; // [esp+8h] [ebp-530h]
    $770C4B51799C51B11A150CADEE7E2360 *v6; // [esp+Ch] [ebp-52Ch]
    char savedName[68]; // [esp+10h] [ebp-528h] BYREF
    unsigned int chunkIter; // [esp+58h] [ebp-4E0h]
    unsigned int zero; // [esp+5Ch] [ebp-4DCh] BYREF
    BspHeader newHeader; // [esp+60h] [ebp-4D8h] BYREF
    unsigned int offset; // [esp+394h] [ebp-1A4h]
    const BspChunk *chunk; // [esp+398h] [ebp-1A0h]
    bool isNewChunk; // [esp+39Fh] [ebp-199h]
    const void *chunkData[100]; // [esp+3A0h] [ebp-198h]
    int h; // [esp+530h] [ebp-8h]
    unsigned int zeroCount; // [esp+534h] [ebp-4h]

    if ( !Com_IsBspLoaded()
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\com_bsp_load_obj.cpp",
                    407,
                    0,
                    "%s",
                    "Com_IsBspLoaded()") )
    {
        __debugbreak();
    }
    if ( comBspGlob.header->version != 45
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\com_bsp_load_obj.cpp",
                    408,
                    0,
                    "%s",
                    "comBspGlob.header->version == BSP_VERSION") )
    {
        __debugbreak();
    }
    if ( comBspGlob.header->chunkCount > 0x64
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\com_bsp_load_obj.cpp",
                    410,
                    0,
                    "comBspGlob.header->chunkCount <= ARRAY_COUNT( newHeader.chunks )\n\t%i, %i",
                    comBspGlob.header->chunkCount,
                    100) )
    {
        __debugbreak();
    }
    newHeader.ident = 1347633737;
    newHeader.version = 45;
    newHeader.chunkCount = 0;
    isNewChunk = 1;
    offset = 8 * comBspGlob.header->chunkCount + 12;
    for ( chunkIter = 0; chunkIter < comBspGlob.header->chunkCount; ++chunkIter )
    {
        chunk = &comBspGlob.header->chunks[chunkIter];
        if ( chunk->type == type )
        {
            isNewChunk = 0;
            if ( size )
            {
                newHeader.chunks[newHeader.chunkCount].type = type;
                newHeader.chunks[newHeader.chunkCount].length = size;
                chunkData[newHeader.chunkCount++] = newLump;
            }
        }
        else
        {
            newHeader.chunks[newHeader.chunkCount].type = chunk->type;
            newHeader.chunks[newHeader.chunkCount].length = chunk->length;
            chunkData[newHeader.chunkCount++] = (char *)comBspGlob.header + offset;
        }
        offset += (chunk->length + 3) & 0xFFFFFFFC;
    }
    if ( isNewChunk && size )
    {
        if ( newHeader.chunkCount >= 0x64
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\com_bsp_load_obj.cpp",
                        444,
                        0,
                        "newHeader.chunkCount < ARRAY_COUNT( newHeader.chunks )\n\t%i, %i",
                        newHeader.chunkCount,
                        100) )
        {
            __debugbreak();
        }
        newHeader.chunks[newHeader.chunkCount].type = type;
        newHeader.chunks[newHeader.chunkCount].length = size;
        chunkData[newHeader.chunkCount++] = newLump;
    }
    h = FS_OpenFileOverwrite(comBspGlob.name);
    if ( h )
    {
        FS_Write((char *)&newHeader, 8 * newHeader.chunkCount + 12, h);
        for ( chunkIter = 0; chunkIter < newHeader.chunkCount; ++chunkIter )
        {
            FS_Write((char *)chunkData[chunkIter], newHeader.chunks[chunkIter].length, h);
            zeroCount = -(int)newHeader.chunks[chunkIter].length & 3;
            if ( zeroCount )
                FS_Write((char *)&zero, zeroCount, h);
        }
        FS_FCloseFile(h);
        if ( behavior )
        {
            if ( behavior != COM_SAVE_LUMP_AND_REOPEN
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\com_bsp_load_obj.cpp",
                            473,
                            1,
                            "%s\n\t(behavior) = %i",
                            "(behavior == COM_SAVE_LUMP_AND_REOPEN)",
                            behavior) )
            {
                __debugbreak();
            }
            v6 = &comBspGlob;
            v5 = savedName;
            do
            {
                v4 = v6->name[0];
                *v5 = v6->name[0];
                v6 = ($770C4B51799C51B11A150CADEE7E2360 *)((char *)v6 + 1);
                ++v5;
            }
            while ( v4 );
            Com_UnloadBsp();
            Com_LoadBsp(savedName);
        }
        else
        {
            Com_UnloadBsp();
        }
    }
    else
    {
        Com_Error(ERR_DROP, "Failed to open file %s for writing", comBspGlob.name);
    }
}

const char *__cdecl Com_GetHunkStringCopy(char *string)
{
    unsigned int v2; // [esp+0h] [ebp-18h]
    unsigned __int8 *hunkCopy; // [esp+14h] [ebp-4h]

    v2 = strlen(string);
    hunkCopy = Hunk_AllocAlign(v2 + 1, 1, "Com_GetLightDefName", 14);
    memcpy(hunkCopy, (unsigned __int8 *)string, v2 + 1);
    return (const char *)hunkCopy;
}

void __cdecl Com_LoadWaterData()
{
    ComWaterCell *out; // [esp+8h] [ebp-18h]
    const DiskWaterCell *in; // [esp+Ch] [ebp-14h]
    unsigned int n; // [esp+10h] [ebp-10h]
    char *diskHeader; // [esp+14h] [ebp-Ch]
    char *diskCells; // [esp+18h] [ebp-8h]
    unsigned int count; // [esp+1Ch] [ebp-4h] BYREF

    if ( comBspGlob.header->version >= 0x1B )
    {
        diskHeader = Com_GetBspLump(LUMP_WATERHEADER, 0x14u, &count);
        if ( count > 1
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\com_bsp_load_obj.cpp",
                        693,
                        0,
                        "%s",
                        "count <= 1") )
        {
            __debugbreak();
        }
        if ( count )
        {
            comWorld.waterHeader = *(ComWaterHeader *)diskHeader;
            diskCells = Com_GetBspLump(LUMP_WATERCELLS, 8u, &count);
            comWorld.numWaterCells = count;
            comWorld.waterCells = (ComWaterCell *)Hunk_Alloc(8 * count, "CMod_LoadWaterData", 14);
            for ( n = 0; n < count; ++n )
            {
                in = (const DiskWaterCell *)&diskCells[8 * n];
                out = &comWorld.waterCells[n];
                out->flooroffset = in->flooroffset;
                out->waterheight = in->waterheight;
                out->shoredist = in->shoreDist;
                Byte4CopyBgraToVertexColor(in->color, out->color);
            }
        }
        else
        {
            comWorld.numWaterCells = 0;
            comWorld.waterCells = 0;
            comWorld.waterHeader.minx = 0;
            comWorld.waterHeader.maxx = 0;
            comWorld.waterHeader.miny = 0;
            comWorld.waterHeader.maxy = 0;
        }
    }
}

void __cdecl Byte4CopyBgraToVertexColor(const unsigned __int8 *rgbaFrom, unsigned __int8 *nativeTo)
{
    *(unsigned int *)nativeTo = (rgbaFrom[3] << 24) | *rgbaFrom | (rgbaFrom[1] << 8) | (rgbaFrom[2] << 16);
}

void __cdecl Com_LoadBurnableData()
{
    int x; // [esp+0h] [ebp-28h]
    const DiskBurnableBlock *block; // [esp+4h] [ebp-24h]
    ComBurnableCell *out; // [esp+Ch] [ebp-1Ch]
    unsigned int n; // [esp+10h] [ebp-18h]
    char *diskHeader; // [esp+14h] [ebp-14h]
    const DiskBurnableBlock *blocks; // [esp+18h] [ebp-10h]
    char *in; // [esp+1Ch] [ebp-Ch]
    unsigned int blockCount; // [esp+20h] [ebp-8h] BYREF
    unsigned int count; // [esp+24h] [ebp-4h] BYREF

    if ( comBspGlob.header->version >= 0x18 )
    {
        diskHeader = Com_GetBspLump(LUMP_BURNABLEHEADER, 0x10u, &count);
        if ( count > 1
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\com_bsp_load_obj.cpp",
                        742,
                        0,
                        "%s",
                        "count <= 1") )
        {
            __debugbreak();
        }
        if ( count )
        {
            comWorld.burnableHeader = *(ComBurnableHeader *)diskHeader;
            in = Com_GetBspLump(LUMP_BURNABLECELLS, 0x10u, &count);
            comWorld.numBurnableCells = count;
            comWorld.burnableCells = (ComBurnableCell *)Hunk_Alloc(12 * count, "CMod_LoadBurnableData", 14);
            blocks = (const DiskBurnableBlock *)Com_GetBspLump(LUMP_BURNABLECELLDATA, 0x20u, &blockCount);
            for ( n = 0; n < count; ++n )
            {
                out = &comWorld.burnableCells[n];
                out->x = *(unsigned int *)in;
                out->y = *((unsigned int *)in + 1);
                if ( (*((int *)in + 3) < 0 || *((unsigned int *)in + 3) >= (signed int)blockCount)
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\com_bsp_load_obj.cpp",
                                772,
                                0,
                                "%s",
                                "in->dataIndex >= 0 && in->dataIndex < (int)blockCount") )
                {
                    __debugbreak();
                }
                block = &blocks[*((unsigned int *)in + 3)];
                out->data = (ComBurnableSample *)Hunk_Alloc(0x20u, "CMod_LoadBurnableData", 14);
                for ( x = 0; x < 32; ++x )
                    Com_PackBurnableSample(&block->data[x], &out->data[x]);
                in += 16;
            }
            qsort(
                comWorld.burnableCells,
                comWorld.numBurnableCells,
                0xCu,
                (int (__cdecl *)(const void *, const void *))Com_BurnableCellSort);
        }
        else
        {
            comWorld.numBurnableCells = 0;
            comWorld.burnableCells = 0;
            comWorld.burnableHeader.minx = 0;
            comWorld.burnableHeader.maxx = 0;
            comWorld.burnableHeader.miny = 0;
            comWorld.burnableHeader.maxy = 0;
        }
    }
}

int __cdecl Com_BurnableCellSort(unsigned int *va, unsigned int *vb)
{
    if ( va[1] < vb[1] )
        return -1;
    if ( va[1] > vb[1] )
        return 1;
    if ( *va >= *vb )
        return *va > *vb;
    return -1;
}

void __cdecl Com_PackBurnableSample(const DiskBurnableSample *in, ComBurnableSample *out)
{
    out->state = in->state;
}

void __cdecl Com_LoadWorld(char *name)
{
    if ( useFastFile->current.enabled )
        Com_LoadWorld_FastFile(name);
    else
        Com_LoadWorld_LoadObj(name);
}

void __cdecl Com_LoadWorld_LoadObj(char *name)
{
    if ( !Com_IsBspLoaded()
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\com_bsp_load_obj.cpp",
                    794,
                    0,
                    "%s",
                    "Com_IsBspLoaded()") )
    {
        __debugbreak();
    }
    Com_LoadPrimaryLights();
    Com_LoadWaterData();
    Com_LoadBurnableData();
    comWorld.name = Com_GetHunkStringCopy(name);
    comWorld.isInUse = 1;
}

ComPrimaryLight *Com_LoadPrimaryLights()
{
    ComPrimaryLight *result; // eax
    float *cookieControl2; // [esp+Ch] [ebp-50h]
    float *cookieControl1; // [esp+14h] [ebp-48h]
    float *cookieControl0; // [esp+1Ch] [ebp-40h]
    float *aAbB; // [esp+24h] [ebp-38h]
    float *origin; // [esp+34h] [ebp-28h]
    float *dir; // [esp+3Ch] [ebp-20h]
    float *color; // [esp+44h] [ebp-18h]
    ComPrimaryLight *out; // [esp+48h] [ebp-14h]
    const DiskPrimaryLight *diskLights; // [esp+4Ch] [ebp-10h]
    unsigned int diskLightCount; // [esp+50h] [ebp-Ch] BYREF
    DiskPrimaryLight *in; // [esp+54h] [ebp-8h]
    unsigned int lightIndex; // [esp+58h] [ebp-4h]

    if ( comBspGlob.header->version <= 0xE )
        return Com_LoadPrimaryLights_Version14();
    if ( comBspGlob.header->version <= 0x10 )
        return (ComPrimaryLight *)Com_LoadPrimaryLights_Version16();
    diskLights = (const DiskPrimaryLight *)Com_GetBspLump(LUMP_PRIMARY_LIGHTS, 0x10Cu, &diskLightCount);
    if ( diskLightCount <= 1 )
        Com_Error(ERR_DROP, "no primary lights in bsp\n");
    comWorld.primaryLightCount = diskLightCount;
    comWorld.primaryLights = (ComPrimaryLight *)Hunk_Alloc(220 * diskLightCount, "Com_LoadPrimaryLights", 14);
    in = (DiskPrimaryLight*)diskLights;
    result = comWorld.primaryLights;
    out = comWorld.primaryLights;
    lightIndex = 0;
    while ( lightIndex < diskLightCount )
    {
        out->type = in->type;
        out->canUseShadowMap = in->canUseShadowMap;
        out->cullDist = in->cullDist;
        out->exponent = truncate_cast<unsigned char>(in->exponent);
        out->priority = in->priority;
        color = in->color;
        out->color[0] = in->color[0];
        out->color[1] = color[1];
        out->color[2] = color[2];
        dir = in->dir;
        out->dir[0] = in->dir[0];
        out->dir[1] = dir[1];
        out->dir[2] = dir[2];
        origin = in->origin;
        out->origin[0] = in->origin[0];
        out->origin[1] = origin[1];
        out->origin[2] = origin[2];
        out->radius = in->radius;
        out->cosHalfFovOuter = in->cosHalfFovOuter;
        out->cosHalfFovInner = in->cosHalfFovInner;
        out->rotationLimit = in->rotationLimit;
        out->translationLimit = in->translationLimit;
        if ( in->type && in->type != 1 )
        {
            out->defName = Com_GetLightDefName((char*)in->defName, comWorld.primaryLights, lightIndex);
            if ( out->cosHalfFovOuter >= out->cosHalfFovInner )
                out->cosHalfFovInner = (float)(out->cosHalfFovOuter * 0.75) + 0.25;
            if ( out->rotationLimit == 1.0 )
            {
                out->cosHalfFovExpanded = out->cosHalfFovOuter;
            }
            //else if ( COERCE_FLOAT(LODWORD(out->cosHalfFovOuter) ^ _mask__NegFloat_) < out->rotationLimit )
            else if ( (-(out->cosHalfFovOuter)) < out->rotationLimit )
            {
                out->cosHalfFovExpanded = CosOfSumOfArcCos(out->cosHalfFovOuter, out->rotationLimit);
            }
            else
            {
                out->cosHalfFovExpanded = -1.0f;
            }
        }
        else
        {
            out->defName = 0;
            out->cosHalfFovExpanded = in->cosHalfFovOuter;
        }
        out->diffuseColor[0] = in->color[0];
        out->diffuseColor[1] = in->color[1];
        out->diffuseColor[2] = in->color[2];
        out->diffuseColor[3] = 0.0f;
        out->specularColor[0] = in->specularcolor[0];
        out->specularColor[1] = in->specularcolor[1];
        out->specularColor[2] = in->specularcolor[2];
        out->specularColor[3] = 0.0f;
        out->attenuation[0] = in->attenuation[0];
        out->attenuation[1] = in->attenuation[1];
        out->attenuation[2] = in->attenuation[2];
        out->attenuation[3] = 0.0f;
        out->falloff[0] = in->cutOn;
        out->falloff[1] = in->radius;
        out->falloff[2] = (float)((float)(in->radius - in->cutOn) * in->nearEdge) + in->cutOn;
        out->falloff[3] = (float)((float)(in->radius - in->cutOn) * in->farEdge) + in->cutOn;
        out->angle[0] = in->angle[0];
        out->angle[1] = in->angle[1];
        out->angle[2] = in->angle[2];
        out->angle[3] = in->roundness;
        out->mipDistance = in->mipDistance;
        aAbB = in->aAbB;
        out->aAbB[0] = in->aAbB[0];
        out->aAbB[1] = aAbB[1];
        out->aAbB[2] = aAbB[2];
        out->aAbB[3] = aAbB[3];
        cookieControl0 = in->cookieControl0;
        out->cookieControl0[0] = in->cookieControl0[0];
        out->cookieControl0[1] = cookieControl0[1];
        out->cookieControl0[2] = cookieControl0[2];
        out->cookieControl0[3] = cookieControl0[3];
        cookieControl1 = in->cookieControl1;
        out->cookieControl1[0] = in->cookieControl1[0];
        out->cookieControl1[1] = cookieControl1[1];
        out->cookieControl1[2] = cookieControl1[2];
        out->cookieControl1[3] = cookieControl1[3];
        cookieControl2 = in->cookieControl2;
        out->cookieControl2[0] = in->cookieControl2[0];
        out->cookieControl2[1] = cookieControl2[1];
        out->cookieControl2[2] = cookieControl2[2];
        out->cookieControl2[3] = cookieControl2[3];
        ++lightIndex;
        ++in;
        result = ++out;
    }
    return result;
}

const char *__cdecl Com_GetLightDefName(
                char *defName,
                const ComPrimaryLight *primaryLights,
                unsigned int primaryLightCount)
{
    unsigned int primaryLightIndex; // [esp+14h] [ebp-8h]

    for ( primaryLightIndex = 0; primaryLightIndex < primaryLightCount; ++primaryLightIndex )
    {
        if ( primaryLights[primaryLightIndex].defName && !strcmp(defName, primaryLights[primaryLightIndex].defName) )
            return primaryLights[primaryLightIndex].defName;
    }
    return Com_GetHunkStringCopy(defName);
}

ComPrimaryLight *Com_LoadPrimaryLights_Version14()
{
    ComPrimaryLight *result; // eax

    comWorld.primaryLightCount = 2;
    result = (ComPrimaryLight *)Hunk_Alloc(0x1B8u, "Com_LoadPrimaryLights", 14);
    comWorld.primaryLights = result;
    if ( result->type )
    {
        result = (ComPrimaryLight *)Assert_MyHandler(
                                                                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\com_bsp_load_obj.cpp",
                                                                    516,
                                                                    1,
                                                                    "%s",
                                                                    "comWorld.primaryLights[0].type == GFX_LIGHT_TYPE_NONE");
        if ( !(_BYTE)result )
            __debugbreak();
    }
    comWorld.primaryLights[1].type = 1;
    return result;
}

const DiskPrimaryLight_Version16 *Com_LoadPrimaryLights_Version16()
{
    const DiskPrimaryLight_Version16 *result; // eax
    float *origin; // [esp+4h] [ebp-28h]
    float *dir; // [esp+Ch] [ebp-20h]
    float *color; // [esp+14h] [ebp-18h]
    ComPrimaryLight *out; // [esp+18h] [ebp-14h]
    const DiskPrimaryLight_Version16 *diskLights; // [esp+1Ch] [ebp-10h]
    unsigned int diskLightCount; // [esp+20h] [ebp-Ch] BYREF
    DiskPrimaryLight_Version16 *in; // [esp+24h] [ebp-8h]
    unsigned int lightIndex; // [esp+28h] [ebp-4h]

    diskLights = (const DiskPrimaryLight_Version16 *)Com_GetBspLump(LUMP_PRIMARY_LIGHTS, 0x60u, &diskLightCount);
    if ( diskLightCount <= 1 )
        Com_Error(ERR_DROP, "no primary lights in bsp\n");
    comWorld.primaryLightCount = diskLightCount;
    comWorld.primaryLights = (ComPrimaryLight *)Hunk_Alloc(220 * diskLightCount, "Com_LoadPrimaryLights", 14);
    result = diskLights;
    in = (DiskPrimaryLight_Version16 *)diskLights;
    out = comWorld.primaryLights;
    lightIndex = 0;
    while ( lightIndex < diskLightCount )
    {
        out->type = in->type;
        out->canUseShadowMap = 0;
        out->cullDist = 1000;
        out->exponent = truncate_cast<unsigned char>(in->exponent);
        out->priority = 0;
        color = in->color;
        out->color[0] = in->color[0];
        out->color[1] = color[1];
        out->color[2] = color[2];
        dir = in->dir;
        out->dir[0] = in->dir[0];
        out->dir[1] = dir[1];
        out->dir[2] = dir[2];
        origin = in->origin;
        out->origin[0] = in->origin[0];
        out->origin[1] = origin[1];
        out->origin[2] = origin[2];
        out->radius = in->radius;
        out->cosHalfFovOuter = in->cosHalfFovOuter;
        out->cosHalfFovInner = in->cosHalfFovInner;
        out->cosHalfFovExpanded = out->cosHalfFovOuter;
        out->rotationLimit = 1.0f;
        out->translationLimit = 0.0f;
        if ( in->type && in->type != 1 )
            out->defName = Com_GetLightDefName(in->defName, comWorld.primaryLights, lightIndex);
        else
            out->defName = 0;
        ++lightIndex;
        result = ++in;
        ++out;
    }
    return result;
}

void __cdecl Com_LoadWorld_FastFile(const char *name)
{
    if ( DB_FindXAssetHeader(ASSET_TYPE_COMWORLD, (char*)name, 1, -1).comWorld != &comWorld
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\com_bsp_load_obj.cpp",
                    813,
                    0,
                    "%s",
                    "asset == &comWorld") )
    {
        __debugbreak();
    }
    if ( !comWorld.isInUse
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\com_bsp_load_obj.cpp",
                    814,
                    0,
                    "%s",
                    "comWorld.isInUse") )
    {
        __debugbreak();
    }
}

void __cdecl Com_ShutdownWorld()
{
    volatile unsigned int Target; // [esp+0h] [ebp-4h] BYREF

    comWorld.isInUse = 0;
    Target = 0;
    InterlockedExchange(&Target, 0);
}

