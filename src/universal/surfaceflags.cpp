#include "surfaceflags.h"
#include "q_shared.h"

const char *Dtp_SurfaceTypeNames[9] =
{
  "default",
  "metal",
  "concrete",
  "wood",
  "snow",
  "dirt",
  "gravel",
  "grass",
  "mud"
};

infoParm_t infoParms[66] =
{
  { "bark", 0, 1048576, 0, 0 },
  { "brick", 0, 2097152, 0, 0 },
  { "carpet", 0, 3145728, 0, 0 },
  { "cloth", 0, 4194304, 0, 0 },
  { "concrete", 0, 5242880, 0, 0 },
  { "dirt", 0, 6291456, 0, 0 },
  { "flesh", 0, 7340032, 0, 0 },
  { "foliage", 1, 8388608, 2, 0 },
  { "glass", 1, 9437184, 16, 0 },
  { "grass", 0, 10485760, 0, 0 },
  { "gravel", 0, 11534336, 0, 0 },
  { "ice", 0, 12582912, 0, 0 },
  { "metal", 0, 13631488, 0, 0 },
  { "mud", 0, 14680064, 0, 0 },
  { "paper", 0, 15728640, 0, 0 },
  { "plaster", 0, 16777216, 0, 0 },
  { "rock", 0, 17825792, 0, 0 },
  { "sand", 0, 18874368, 0, 0 },
  { "snow", 0, 19922944, 0, 0 },
  { "water", 1, 20971520, 32, 0 },
  { "wood", 0, 22020096, 0, 0 },
  { "asphalt", 0, 23068672, 0, 0 },
  { "ceramic", 0, 24117248, 0, 0 },
  { "plastic", 0, 25165824, 0, 0 },
  { "rubber", 0, 26214400, 0, 0 },
  { "cushion", 0, 27262976, 0, 0 },
  { "fruit", 0, 28311552, 0, 0 },
  { "paintedmetal", 0, 29360128, 0, 0 },
  { "player", 0, 30408704, 0, 0 },
  { "tallgrass", 0, 31457280, 0, 0 },
  { "opaqueglass", 0, 9437184, 0, 0 },
  { "clipmissile", 1, 0, 128, 0 },
  { "ai_nosight", 1, 0, 4096, 0 },
  { "clipshot", 1, 0, 8192, 0 },
  { "playerclip", 1, 0, 65536, 0 },
  { "monsterclip", 1, 0, 131072, 0 },
  { "vehicleclip", 1, 0, 512, 0 },
  { "itemclip", 1, 0, 1024, 0 },
  { "nodrop", 1, 0, 2147483648, 0 },
  { "nonsolid", 1, 16384, 0, 0 },
  { "detail", 0, 0, 134217728, 0 },
  { "structural", 0, 0, 268435456, 0 },
  { "portal", 1, 2147483648, 0, 0 },
  { "canshootclip", 0, 0, 64, 0 },
  { "origin", 1, 0, 0, 4 },
  { "sky", 0, 4, 2048, 0 },
  { "nocastshadow", 0, 262144, 0, 0 },
  { "onlycastshadow", 0, 524288, 0, 0 },
  { "physicsGeom", 0, 0, 0, 1024 },
  { "lightPortal", 0, 0, 0, 8192 },
  { "slick", 0, 2, 0, 0 },
  { "noimpact", 0, 16, 0, 0 },
  { "nomarks", 0, 32, 0, 0 },
  { "nopenetrate", 0, 256, 0, 0 },
  { "ladder", 0, 8, 0, 0 },
  { "nodamage", 0, 1, 0, 0 },
  { "mantleOn", 0, 67108864, 16777216, 0 },
  { "mantleOver", 0, 134217728, 16777216, 0 },
  { "mount", 0, 268435456, 16777216, 0 },
  { "nosteps", 0, 8192, 0, 0 },
  { "nodraw", 0, 128, 0, 0 },
  { "nolightmap", 0, 1024, 0, 0 },
  { "noReceiveDynamicShadow", 0, 2048, 0, 0 },
  { "nodlight", 0, 131072, 0, 0 },
  { "receiveSeeThruDecal", 0, 512, 0, 0 },
  { NULL, 0, 0, 0, 0 }
};



int __cdecl Com_SurfaceTypeFromName(const char *name)
{
    int i; // [esp+0h] [ebp-4h]

    if ( !name )
        return -1;
    if ( !I_stricmp(name, "default") )
        return 0;
    for ( i = 0; i < 30; ++i )
    {
        if (!I_stricmp(name, infoParms[i].name))
            return (infoParms[i].surfaceFlags & 0x3F00000) >> 20;
    }
    return -1;
}

const char *__cdecl Com_SurfaceTypeToName(int iTypeIndex)
{
    if (iTypeIndex <= 0 || iTypeIndex >= 31)
        return "default";
    if ((infoParms[iTypeIndex - 1].surfaceFlags & 0x3F00000) >> 20 != iTypeIndex
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\universal\\surfaceflags.cpp",
            149,
            0,
            "%s",
            "SURF_TYPEINDEX( infoParms[iTypeIndex - 1].surfaceFlags ) == iTypeIndex"))
    {
        __debugbreak();
    }
    return infoParms[iTypeIndex - 1].name;
}

bool __cdecl Com_SurfaceBurns(int iTypeIndex)
{
    bool burns; // [esp+7h] [ebp-1h]

    switch ( iTypeIndex )
    {
        case 1:
        case 3:
        case 4:
        case 7:
        case 8:
        case 10:
        case 15:
        case 21:
        case 23:
        case 24:
        case 25:
            burns = 1;
            break;
        default:
            burns = 0;
            break;
    }
    return burns;
}

bool __cdecl Com_SurfaceFloats(int iTypeIndex)
{
    return iTypeIndex == 21 || iTypeIndex > 23 && iTypeIndex <= 27;
}

double __cdecl Com_SurfaceDensity(int iTypeIndex)
{
    float density; // [esp+4h] [ebp-4h]

    switch ( iTypeIndex )
    {
        case 16:
            density = 0.3f;
            break;
        case 21:
            density = 0.5f;
            break;
        case 24:
            density = 0.25f;
            break;
        case 25:
            density = 0.6f;
            break;
        case 26:
            density = 0.2f;
            break;
        case 27:
            density = 0.15f;
            break;
        default:
            density = 1.0f;
            break;
    }
    return density;
}

double __cdecl Com_SurfaceFrictionScale(int iTypeIndex)
{
    float friction; // [esp+4h] [ebp-4h]

    switch ( iTypeIndex )
    {
        case 6:
        case 10:
        case 11:
            friction = 0.9f;
            break;
        case 12:
            friction = 0.75f;
            break;
        case 14:
        case 19:
            friction = 0.8f;
            break;
        default:
            friction = 1.0f;
            break;
    }
    return friction;
}

double __cdecl Com_SurfaceBounceScale(int iTypeIndex)
{
    float bounce; // [esp+4h] [ebp-4h]

    switch ( iTypeIndex )
    {
        case 6:
        case 10:
        case 11:
            bounce = 0.3f;
            break;
        case 14:
            bounce = 0.1f;
            break;
        case 19:
            bounce = 0.2f;
            break;
        default:
            bounce = 1.0f;
            break;
    }
    return bounce;
}

