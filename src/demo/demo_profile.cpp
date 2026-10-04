#include "demo_profile.h"
#include "demo_common.h"
#include <cgame_mp/cg_local_mp.h>
#include <cgame_mp/cg_consolecmds_mp.h>
#include "demo_playback.h"
#include <game/g_debug.h>
#include <client/cl_debugdata.h>

const char *demo_profile_enum_string_72[8] =
{
  "Gamestate",
  "Snapshots",
  "MatchStates",
  "PlayerStates",
  "Entities",
  "Clients",
  "Actors",
  "AnimCmds"
};

bool __cdecl Demo_GetAnalyzePrintDataType(int fieldBit)
{
    bool result; // [esp+7h] [ebp-1h]

    switch ( fieldBit )
    {
        case -107:
        case -106:
        case -105:
        case -104:
        case -103:
        case -102:
        case -101:
        case -100:
        case -99:
        case -92:
        case -91:
        case -90:
        case -89:
        case -88:
        case -87:
        case -86:
        case -81:
        case -80:
        case -77:
        case -76:
        case -75:
        case -74:
        case -73:
        case -72:
        case -66:
        case -65:
        case -64:
        case -63:
        case -62:
        case -61:
        case 0:
            result = 0;
            break;
        default:
            result = 1;
            break;
    }
    return result;
}

void __cdecl Demo_RecordProfileData(int profileDataType, int size)
{
    demo.profile.memUsed[profileDataType] += size;
    ++demo.profile.count[profileDataType];
}

void __cdecl Demo_PrintProfileData()
{
    const char *v0; // eax
    const char *v1; // eax
    const char *v2; // eax
    const char *v3; // eax
    const char *v4; // eax
    int i; // [esp+8h] [ebp-8h]

    v0 = va("Total Demo Filesize: %d bytes\n", demo.totalSize);
    Demo_Printf(2, v0);
    v1 = va("--------------------Demo Profile--------------------\n");
    Demo_Printf(2, v1);
    for ( i = 0; i < 8; ++i )
    {
        v2 = va("%s : Count: %d Size: ", demo_profile_enum_string_72[i], demo.profile.count[i]);
        Demo_Printf(2, v2);
        if ( demo.profile.memUsed[i] < 1024 )
            v3 = va("%d Bytes\n", demo.profile.memUsed[i]);
        else
            v3 = va("%.2f KB\n", (float)((float)demo.profile.memUsed[i] / 1024.0));
        Demo_Printf(2, v3);
    }
    v4 = va("-----------------End of Demo Profile-----------------\n");
    Demo_Printf(2, v4);
}

void __cdecl Demo_DrawDebugInformation(int localClientNum)
{
    double v1; // st7
    char *v2; // eax
    char *v3; // eax
    char *v4; // eax
    double v5; // st7
    char *v6; // eax
    int v7; // [esp+8h] [ebp-74h]
    int v8; // [esp+8h] [ebp-74h]
    int v9; // [esp+8h] [ebp-74h]
    int v10; // [esp+8h] [ebp-74h]
    float cameraOrigin[3]; // [esp+4Ch] [ebp-30h] BYREF
    int entityIndex; // [esp+58h] [ebp-24h]
    float demoInfoCullDistance; // [esp+5Ch] [ebp-20h]
    float origin[3]; // [esp+60h] [ebp-1Ch] BYREF
    cg_s *cgameGlob; // [esp+6Ch] [ebp-10h]
    centity_s *cent; // [esp+70h] [ebp-Ch]
    int debugInfoDrawTime; // [esp+74h] [ebp-8h]
    int entityCount; // [esp+78h] [ebp-4h]

    demoInfoCullDistance = 512.0f;
    debugInfoDrawTime = 50;
    entityCount = 0;
    entityIndex = -1;
    if ( CL_LocalClient_IsCUIFlagSet(localClientNum, 32) )
    {
        cgameGlob = CG_GetLocalClientGlobals(localClientNum);
        if ( Demo_IsMovieCamera() )
        {
            cameraOrigin[0] = cgameGlob->movieCameraOrigin[0];
            cameraOrigin[1] = cgameGlob->movieCameraOrigin[1];
            cameraOrigin[2] = cgameGlob->movieCameraOrigin[2];
        }
        else
        {
            cameraOrigin[0] = cgameGlob->refdef.vieworg[0];
            cameraOrigin[1] = cgameGlob->refdef.vieworg[1];
            cameraOrigin[2] = cgameGlob->refdef.vieworg[2];
        }
        if ( demo_drawdebuginformation->current.integer == 1 )
        {
            while ( entityCount < demo.prevNumEntities )
            {
                if ( demo.ent[++entityIndex].eType >= 0 )
                {
                    cent = CG_GetEntity(localClientNum, entityIndex);
                    origin[0] = cent->pose.origin[0];
                    origin[1] = cent->pose.origin[1];
                    origin[2] = cent->pose.origin[2];
                    v1 = Vec3Distance(cameraOrigin, origin);
                    if ( demoInfoCullDistance > v1 )
                    {
                        G_DebugStar(origin, colorBlue, debugInfoDrawTime);
                        origin[2] = origin[2] + 10.0;
                        v7 = debugInfoDrawTime;
                        v2 = va("Ent Mask: %x %x", demo.entClientMask[entityIndex][0], demo.entClientMask[entityIndex][1]);
                        CL_AddDebugString(origin, colorWhite, 0.69999999, v2, v7);
                        origin[2] = origin[2] + 10.0;
                        v8 = debugInfoDrawTime;
                        v3 = va("Ent Type: %d", demo.ent[entityIndex].eType);
                        CL_AddDebugString(origin, colorWhite, 0.69999999, v3, v8);
                        origin[2] = origin[2] + 10.0;
                        v9 = debugInfoDrawTime;
                        v4 = va("Ent Num: %d", entityIndex);
                        CL_AddDebugString(origin, colorWhite, 0.69999999, v4, v9);
                    }
                    ++entityCount;
                }
            }
        }
        else if ( demo_drawdebuginformation->current.integer == 2 )
        {
            for ( entityIndex = 1024; entityIndex < 1536; ++entityIndex )
            {
                cent = CG_GetEntity(localClientNum, entityIndex);
                origin[0] = cent->pose.origin[0];
                origin[1] = cent->pose.origin[1];
                origin[2] = cent->pose.origin[2];
                v5 = Vec3Distance(cameraOrigin, origin);
                if ( demoInfoCullDistance > v5 )
                {
                    G_DebugStar(origin, colorBlue, debugInfoDrawTime);
                    origin[2] = origin[2] + 10.0;
                    v10 = debugInfoDrawTime;
                    v6 = va("Ent Num: %d", entityIndex);
                    CL_AddDebugString(origin, colorWhite, 0.69999999, v6, v10);
                }
            }
        }
    }
}

