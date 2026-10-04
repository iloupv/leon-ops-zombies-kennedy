#include "physconstraints_load_obj.h"
#include <cstring>
#include <universal/q_shared.h>
#include <universal/com_files.h>
#include <qcommon/common.h>
#include <universal/com_memory.h>
#include <database/db_registry.h>

const char *s_constraintTypeNames[10] =
{
  "none",
  "point",
  "distance",
  "hinge",
  "joint",
  "actuator",
  "fake_shake",
  "launch",
  "rope",
  "light"
};

cspField_t physConstraintsFields[58] =
{
  { "c1_type", 12, 15 },
  { "c1_bone1_name", 28, 0 },
  { "c1_bone2_name", 44, 0 },
  { "c1_offsetX", 48, 7 },
  { "c1_offsetY", 52, 7 },
  { "c1_offsetZ", 56, 7 },
  { "c1_timeout", 100, 4 },
  { "c1_min_health", 104, 4 },
  { "c1_max_health", 108, 4 },
  { "c1_damp", 116, 7 },
  { "c1_power", 120, 7 },
  { "c1_spin_scale", 136, 7 },
  { "c1_shakescalex", 124, 7 },
  { "c1_shakescaley", 128, 7 },
  { "c1_shakescalez", 132, 7 },
  { "c1_min_angle", 140, 7 },
  { "c1_max_angle", 144, 7 },
  { "c1_yaw", 128, 7 },
  { "c1_pitch", 124, 7 },
  { "c2_type", 180, 15 },
  { "c2_bone1_name", 196, 0 },
  { "c2_bone2_name", 212, 0 },
  { "c2_offsetX", 216, 7 },
  { "c2_offsetY", 220, 7 },
  { "c2_offsetZ", 224, 7 },
  { "c2_timeout", 268, 4 },
  { "c2_min_health", 272, 4 },
  { "c2_max_health", 276, 4 },
  { "c2_damp", 284, 7 },
  { "c2_power", 288, 7 },
  { "c2_min_angle", 308, 7 },
  { "c2_max_angle", 312, 7 },
  { "c3_type", 348, 15 },
  { "c3_bone1_name", 364, 0 },
  { "c3_bone2_name", 380, 0 },
  { "c3_offsetX", 384, 7 },
  { "c3_offsetY", 388, 7 },
  { "c3_offsetZ", 392, 7 },
  { "c3_timeout", 436, 4 },
  { "c3_min_health", 440, 4 },
  { "c3_max_health", 444, 4 },
  { "c3_damp", 452, 7 },
  { "c3_power", 456, 7 },
  { "c3_min_angle", 476, 7 },
  { "c3_max_angle", 480, 7 },
  { "c4_type", 516, 15 },
  { "c4_bone1_name", 532, 0 },
  { "c4_bone2_name", 548, 0 },
  { "c4_offsetX", 552, 7 },
  { "c4_offsetY", 556, 7 },
  { "c4_offsetZ", 560, 7 },
  { "c4_timeout", 604, 4 },
  { "c4_min_health", 608, 4 },
  { "c4_max_health", 612, 4 },
  { "c4_damp", 620, 7 },
  { "c4_power", 624, 7 },
  { "c4_min_angle", 644, 7 },
  { "c4_max_angle", 648, 7 }
};



PhysConstraints *__cdecl PhysConstraintsLoadFile(const char *name, void *(__cdecl *Alloc)(int))
{
    int i; // [esp+20h] [ebp-405Ch]
    char dest[64]; // [esp+24h] [ebp-4058h] BYREF
    char buffer[16384]; // [esp+64h] [ebp-4018h] BYREF
    const char *last; // [esp+4068h] [ebp-14h]
    signed int v7; // [esp+406Ch] [ebp-10h]
    int f; // [esp+4070h] [ebp-Ch] BYREF
    unsigned __int8 *dst; // [esp+4074h] [ebp-8h]
    int len; // [esp+4078h] [ebp-4h]

    last = "PHYSCONSTRAINTS";
    len = strlen("PHYSCONSTRAINTS");
    if ( !strlen(name) )
        return 0;
    if ( Com_sprintf(dest, 0x40u, "physconstraints/%s", name) >= 0 )
    {
        v7 = FS_FOpenFileByMode(dest, &f, FS_READ);
        if ( v7 >= 0 )
        {
            FS_Read((unsigned __int8 *)buffer, len, f);
            buffer[len] = 0;
            if ( !strncmp(buffer, last, len) )
            {
                if ( v7 - len < 0x4000 )
                {
                    FS_Read((unsigned __int8 *)buffer, v7 - len, f);
                    buffer[v7 - len] = 0;
                    FS_FCloseFile(f);
                    if ( Info_Validate(buffer) )
                    {
                        dst = (unsigned __int8 *)Alloc(2696);
                        if ( !dst
                            && !Assert_MyHandler(
                                        "C:\\projects_pc\\cod\\codsrc\\src\\physics\\physconstraints_load_obj.cpp",
                                        234,
                                        0,
                                        "%s",
                                        "physConstraints") )
                        {
                            __debugbreak();
                        }
                        memset(dst, 0, 0xA88u);
                        for ( i = 0; i < 16; ++i )
                        {
                            *(unsigned int *)&dst[168 * i + 28] = (unsigned int)"";
                            *(unsigned int *)&dst[168 * i + 16] = 3;
                            *(unsigned int *)&dst[168 * i + 44] = (unsigned int)"";
                            *(unsigned int *)&dst[168 * i + 32] = 3;
                        }
                        if ( ParseConfigStringToStruct(
                                     dst,
                                     physConstraintsFields,
                                     58,
                                     buffer,
                                     16,
                                     (int (__cdecl *)(unsigned __int8 *, const char *, const int, const int))CON_ParseSpecificField,
                                     PhysConstraints_Strcpy) )
                        {
                            PhysConstraintsPostLoadFixup((PhysConstraints *)dst);
                            return (PhysConstraints *)dst;
                        }
                        else
                        {
                            return 0;
                        }
                    }
                    else
                    {
                        Com_PrintError(20, "ERROR: physics constraints file [%s] is not valid\n", name);
                        return 0;
                    }
                }
                else
                {
                    Com_PrintError(20, "ERROR: physics constraints file [%s] is to big\n", name);
                    FS_FCloseFile(f);
                    return 0;
                }
            }
            else
            {
                Com_PrintError(20, "ERROR: file [%s] is not a physics constraints file\n", name);
                FS_FCloseFile(f);
                return 0;
            }
        }
        else
        {
            Com_PrintError(20, "ERROR: physics constraints '%s' not found\n", name);
            return 0;
        }
    }
    else
    {
        Com_PrintError(20, "ERROR: filename '%s' too long\n", dest);
        return 0;
    }
}

void __cdecl PhysConstraints_Strcpy(unsigned __int8 *member, const char *keyValue)
{
    char v2; // [esp+3h] [ebp-25h]
    unsigned __int8 *v3; // [esp+8h] [ebp-20h]
    const char *v4; // [esp+Ch] [ebp-1Ch]
    unsigned __int8 *buf; // [esp+20h] [ebp-8h]

    if ( *keyValue )
    {
        buf = Hunk_AllocLowAlign(strlen(keyValue) + 1, 1, "SetPhysConstraintsString", 0);
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

int __cdecl CON_ParseSpecificField(unsigned __int8 *pStruct, const char *pValue, int fieldType)
{
    const char *v3; // eax
    PhysConstraint *constraint; // [esp+4h] [ebp-Ch]
    int i; // [esp+Ch] [ebp-4h]

    constraint = (PhysConstraint *)&pStruct[168 * *((unsigned int *)pStruct + 1) + 8];
    if ( fieldType == 15 )
    {
        for ( i = 0; i < 10; ++i )
        {
            if ( !I_stricmp(pValue, s_constraintTypeNames[i]) )
            {
                if ( i )
                {
                    constraint->type = (ConstraintType)i;
                    if ( *((unsigned int *)pStruct + 1) >= 0x10u )
                        Com_Error(ERR_DROP, "Cosntraints array overflow");
                    ++*((unsigned int *)pStruct + 1);
                }
                break;
            }
        }
        if ( i == 10 )
            Com_Error(ERR_DROP, "Unknown constraint type [%s]", pValue);
        return 1;
    }
    else
    {
        v3 = va("Bad constraint field type %i\n", fieldType);
        if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\physics\\physconstraints_load_obj.cpp", 147, 0, v3) )
            __debugbreak();
        Com_Error(ERR_DROP, "Bad constraint field type %i", fieldType);
        return 0;
    }
}

void __cdecl PhysConstraintsPostLoadFixup(PhysConstraints *physConstraints)
{
    PhysConstraint *point_constraint; // [esp+8h] [ebp-10h]
    int ci; // [esp+10h] [ebp-8h]
    signed int count; // [esp+14h] [ebp-4h]

    count = physConstraints->count;
    for ( ci = 0; ci < count; ++ci )
    {
        if ( physConstraints->data[ci].type == CONSTRAINT_ACTUATOR )
        {
            if ( physConstraints->count >= 0x10 )
                Com_Error(ERR_DROP, "Cosntraints array overflow");
            point_constraint = &physConstraints->data[physConstraints->count++];
            memcpy(point_constraint, &physConstraints->data[ci], sizeof(PhysConstraint));
            point_constraint->type = CONSTRAINT_POINT;
        }
        else if ( physConstraints->data[ci].type == CONSTRAINT_LAUNCH )
        {
            physConstraints->data[ci].scale[2] = 0.0f;
        }
    }
}

PhysConstraints *__cdecl PhysConstraintsPrecache(const char *name, void *(__cdecl *Alloc)(int))
{
    PhysConstraints *physConstraints; // [esp+0h] [ebp-4h]
    PhysConstraints *physConstraintsa; // [esp+0h] [ebp-4h]

    if ( !name
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\physics\\physconstraints_load_obj.cpp",
                    262,
                    0,
                    "%s",
                    "name") )
    {
        __debugbreak();
    }
    if ( !*name
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\physics\\physconstraints_load_obj.cpp",
                    263,
                    0,
                    "%s",
                    "name[0]") )
    {
        __debugbreak();
    }
    physConstraints = (PhysConstraints *)Hunk_FindDataForFile(8, name);
    if ( physConstraints )
        return physConstraints;
    if ( !Alloc )
        return 0;
    physConstraintsa = PhysConstraintsLoadFile(name, Alloc);
    if ( physConstraintsa )
    {
        physConstraintsa->name = Hunk_SetDataForFile(8, name, physConstraintsa, Alloc);
        return physConstraintsa;
    }
    else
    {
        Com_PrintError(20, "ERROR: Cannot find physics constraints '%s'.\n", name);
        return 0;
    }
}

void __cdecl PhysConstraintsUpdateField(char *name, char *keyvalue)
{
    PhysConstraints *physConstraints; // [esp+0h] [ebp-4h]

    physConstraints = GetPhysConstraints(name);
    if ( physConstraints )
    {
        if ( ParseConfigStringToStruct(
                     (unsigned __int8 *)physConstraints,
                     physConstraintsFields,
                     58,
                     keyvalue,
                     16,
                     (int (__cdecl *)(unsigned __int8 *, const char *, const int, const int))CON_ParseSpecificField,
                     PhysConstraints_Strcpy) )
        {
            PhysConstraintsPostLoadFixup(physConstraints);
        }
    }
}

PhysConstraints *__cdecl GetPhysConstraints(char *name)
{
    return DB_FindXAssetHeader(ASSET_TYPE_PHYSCONSTRAINTS, name, 1, -1).physConstraints;
}

