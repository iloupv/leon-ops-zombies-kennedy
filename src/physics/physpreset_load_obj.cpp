#include "physpreset_load_obj.h"
#include <universal/assertive.h>
#include <universal/com_memory.h>
#include <universal/com_files.h>
#include <qcommon/common.h>
#include <database/db_registry.h>

cspField_t physPresetFields[19] =
{
  { "mass", 0, 7 },
  { "bounce", 4, 7 }, // bouncin' back off the captain... Index numba 3 sir.
  { "friction", 8, 7 },
  { "isFrictionInfinity", 12, 6 },
  { "bulletForceScale", 16, 7 },
  { "explosiveForceScale", 20, 7 },
  { "piecesSpreadFraction", 24, 7 },
  { "piecesUpwardVelocity", 28, 7 },
  { "canFloat", 32, 4 },
  { "gravityScale", 36, 7 },
  { "massOffsetX", 40, 7 },
  { "massOffsetY", 44, 7 },
  { "massOffsetZ", 48, 7 },
  { "buoyancyMinX", 52, 7 },
  { "buoyancyMinY", 56, 7 },
  { "buoyancyMinZ", 60, 7 },
  { "buoyancyMaxX", 64, 7 },
  { "buoyancyMaxY", 68, 7 },
  { "buoyancyMaxZ", 72, 7 }
};

void *(__cdecl *physAlloc)(int);

PhysPreset *__cdecl PhysPresetLoadFile(const char *name, void *(__cdecl *Alloc)(int))
{
    float v3; // [esp+0h] [ebp-40F0h]
    float v4; // [esp+4h] [ebp-40ECh]
    float v5; // [esp+8h] [ebp-40E8h]
    char *v6; // [esp+2Ch] [ebp-40C4h]
    char *v7; // [esp+30h] [ebp-40C0h]
    char *v8; // [esp+34h] [ebp-40BCh]
    float v9; // [esp+38h] [ebp-40B8h]
    float v10; // [esp+3Ch] [ebp-40B4h]
    char dest[64]; // [esp+40h] [ebp-40B0h] BYREF
    char buffer[16388]; // [esp+80h] [ebp-4070h] BYREF
    unsigned __int8 dst[84]; // [esp+4088h] [ebp-68h] BYREF
    float *v14; // [esp+40DCh] [ebp-14h]
    const char *last; // [esp+40E0h] [ebp-10h]
    signed int v16; // [esp+40E4h] [ebp-Ch]
    int f; // [esp+40E8h] [ebp-8h] BYREF
    int len; // [esp+40ECh] [ebp-4h]

    last = "PHYSIC";
    len = strlen("PHYSIC");
    if ( !strlen(name) )
        return 0;
    if ( Com_sprintf(dest, 0x40u, "physic/%s", name) >= 0 )
    {
        v16 = FS_FOpenFileByMode(dest, &f, FS_READ);
        if ( v16 >= 0 )
        {
            FS_Read((unsigned __int8 *)buffer, len, f);
            buffer[len] = 0;
            if ( !strncmp(buffer, last, len) )
            {
                if ( v16 - len < 0x4000 )
                {
                    FS_Read((unsigned __int8 *)buffer, v16 - len, f);
                    buffer[v16 - len] = 0;
                    FS_FCloseFile(f);
                    if ( Info_Validate(buffer) )
                    {
                        memset(dst, 0, 0x4Cu);
                        *(float *)&dst[36] = 1.0f;
                        physAlloc = Alloc;
                        if ( ParseConfigStringToStruct(dst, physPresetFields, 19, buffer, 0, 0, PhysPreset_Strcpy) )
                        {
                            v14 = (float *)Alloc(84);
                            if ( !v14
                                && !Assert_MyHandler(
                                            "C:\\projects_pc\\cod\\codsrc\\src\\physics\\physpreset_load_obj.cpp",
                                            140,
                                            0,
                                            "%s",
                                            "physPreset") )
                            {
                                __debugbreak();
                            }
                            if ( (float)(*(float *)dst - 2000.0) < 0.0 )
                                v10 = *(float *)dst;
                            else
                                v10 = 2000.0f;
                            if ( (float)(1.0 - *(float *)dst) < 0.0 )
                                v5 = v10;
                            else
                                v5 = 1.0f;
                            *(float *)dst = v5;
                            if ( (float)(*(float *)&dst[36] - 10.0) < 0.0 )
                                v9 = *(float *)&dst[36];
                            else
                                v9 = 10.0f;
                            if ( (float)(0.0099999998 - *(float *)&dst[36]) < 0.0 )
                                v4 = v9;
                            else
                                v4 = 0.01f;
                            *(float *)&dst[36] = v4;
                            v14[2] = *(float *)dst * 0.001;
                            v14[3] = *(float *)&dst[4];
                            if ( *(unsigned int *)&dst[12] )
                                v3 = 1.0e10f;
                            else
                                v3 = *(float *)&dst[8];
                            v14[4] = v3;
                            v14[5] = *(float *)&dst[16];
                            v14[6] = *(float *)&dst[20];
                            v14[8] = *(float *)&dst[24];
                            v14[9] = *(float *)&dst[28];
                            v14[10] = *(float *)&dst[32];
                            v14[11] = *(float *)&dst[36];
                            v8 = (char *)(v14 + 12);
                            v14[12] = *(float *)&dst[40];
                            *((unsigned int *)v8 + 1) = *(unsigned int *)&dst[44];
                            *((unsigned int *)v8 + 2) = *(unsigned int *)&dst[48];
                            v7 = (char *)(v14 + 15);
                            v14[15] = *(float *)&dst[52];
                            *((unsigned int *)v7 + 1) = *(unsigned int *)&dst[56];
                            *((unsigned int *)v7 + 2) = *(unsigned int *)&dst[60];
                            v6 = (char *)(v14 + 18);
                            v14[18] = *(float *)&dst[64];
                            *((unsigned int *)v6 + 1) = *(unsigned int *)&dst[68];
                            *((unsigned int *)v6 + 2) = *(unsigned int *)&dst[72];
                            return (PhysPreset *)v14;
                        }
                        else
                        {
                            return 0;
                        }
                    }
                    else
                    {
                        Com_PrintError(20, "ERROR: physics preset file [%s] is not valid\n", name);
                        return 0;
                    }
                }
                else
                {
                    Com_PrintError(20, "ERROR: physics preset file [%s] is to big\n", name);
                    FS_FCloseFile(f);
                    return 0;
                }
            }
            else
            {
                Com_PrintError(20, "ERROR: file [%s] is not a physics preset file\n", name);
                FS_FCloseFile(f);
                return 0;
            }
        }
        else
        {
            Com_PrintError(20, "ERROR: physics preset '%s' not found\n", name);
            return 0;
        }
    }
    else
    {
        Com_PrintError(20, "ERROR: filename '%s' too long\n", dest);
        return 0;
    }
}

void __cdecl PhysPreset_Strcpy(unsigned __int8 *member, const char *keyValue)
{
    char v2; // [esp+3h] [ebp-25h]
    char *v3; // [esp+8h] [ebp-20h]
    const char *v4; // [esp+Ch] [ebp-1Ch]
    char *buf; // [esp+20h] [ebp-8h]

    if ( *keyValue )
    {
        buf = (char *)physAlloc(strlen(keyValue) + 1);
        v4 = keyValue;
        v3 = buf;
        do
        {
            v2 = *v4;
            *v3++ = *v4++;
        }
        while ( v2 );
        *(unsigned int *)member = (unsigned int)buf;
    }
    else
    {
        *(unsigned int *)member = (unsigned int)"";
    }
}

PhysPreset *__cdecl PhysPresetPrecache(const char *name, void *(__cdecl *Alloc)(int))
{
    PhysPreset *physPreset; // [esp+0h] [ebp-4h]
    PhysPreset *physPreseta; // [esp+0h] [ebp-4h]

    if ( !name
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\physics\\physpreset_load_obj.cpp", 165, 0, "%s", "name") )
    {
        __debugbreak();
    }
    if ( !*name
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\physics\\physpreset_load_obj.cpp", 166, 0, "%s", "name[0]") )
    {
        __debugbreak();
    }
    physPreset = (PhysPreset *)Hunk_FindDataForFile(7, name);
    if ( physPreset )
        return physPreset;
    physPreseta = PhysPresetLoadFile(name, Alloc);
    if ( physPreseta )
    {
        physPreseta->name = Hunk_SetDataForFile(7, name, physPreseta, Alloc);
        return physPreseta;
    }
    else
    {
        Com_PrintError(20, "ERROR: Cannot find physics preset '%s'.\n", name);
        return 0;
    }
}

PhysPreset *__cdecl PhysPreset_Register(const char *name)
{
    if ( useFastFile->current.enabled )
        return PhysPreset_Register_FastFile(name);
    else
        return PhysPreset_Register_LoadObj(name);
}

PhysPreset *__cdecl PhysPreset_Register_FastFile(const char *name)
{
    if ( !I_strncmp(name, "physpreset/", 11)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\physics\\physpreset_load_obj.cpp",
                    194,
                    0,
                    "%s\n\t(name) = %s",
                    "(I_strncmp( name, \"physpreset/\", 11 ))",
                    name) )
    {
        __debugbreak();
    }
    return DB_FindXAssetHeader(ASSET_TYPE_PHYSPRESET, (char*)name, 1, -1).physPreset;
}

extern void *__cdecl Hunk_AllocPhysPresetPrecache(int size);

PhysPreset *__cdecl PhysPreset_Register_LoadObj(const char *name)
{
    return PhysPresetPrecache(name, Hunk_AllocPhysPresetPrecache);
}

