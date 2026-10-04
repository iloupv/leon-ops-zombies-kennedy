#include "r_light_load_obj.h"
#include <universal/com_files.h>
#include <universal/q_shared.h>
#include <universal/com_memory.h>
#include "r_image.h"

GfxLightDef *__cdecl R_LoadLightDef(const char *name)
{
    char v2; // [esp+3h] [ebp-31h]
    char *v3; // [esp+8h] [ebp-2Ch]
    const char *v4; // [esp+Ch] [ebp-28h]
    char *filename; // [esp+20h] [ebp-14h]
    unsigned __int8 *def; // [esp+24h] [ebp-10h]
    unsigned __int8 *file; // [esp+28h] [ebp-Ch] BYREF
    int fileSize; // [esp+2Ch] [ebp-8h]
    const unsigned __int8 *readPos; // [esp+30h] [ebp-4h]

    if ( !name
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_light_load_obj.cpp", 95, 0, "%s", "name") )
    {
        __debugbreak();
    }
    filename = va("lights/%s", name);
    fileSize = FS_ReadFile(filename, (void **)&file);
    if ( fileSize < 0 )
        return 0;
    if ( fileSize )
    {
        def = Hunk_Alloc(0x10u, "R_RegisterLightDef", 21);
        *(unsigned int *)def = (unsigned int)Hunk_Alloc(strlen(name) + 1, "R_RegisterLightDef", 21);
        if ( !def
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_light_load_obj.cpp", 110, 0, "%s", "def") )
        {
            __debugbreak();
        }
        readPos = file;
        readPos = R_LoadLightImage(file, (GfxLightImage *)(def + 4));
        v4 = name;
        v3 = *(char **)def;
        do
        {
            v2 = *v4;
            *v3++ = *v4++;
        }
        while ( v2 );
        I_strlwr(*(char **)def);
        FS_FreeFile(file);
        *((unsigned int *)def + 3) = 0;
        return (GfxLightDef *)def;
    }
    else
    {
        FS_FreeFile(file);
        return 0;
    }
}

unsigned __int8 *__cdecl R_LoadLightImage(unsigned __int8 *readPos, GfxLightImage *lightImage)
{
    unsigned int v3; // [esp+0h] [ebp-18h]
    unsigned __int8 *readPosa; // [esp+20h] [ebp+8h]

    lightImage->samplerState = *readPos;
    readPosa = readPos + 1;
    v3 = strlen((const char *)readPosa);
    if ( v3 )
        lightImage->image = Image_Register((char *)readPosa, 1u, 5);
    else
        lightImage->image = 0;
    return &readPosa[v3 + 1];
}

