#include "gdt_remote.h"

#include <cstring>
#include <bgame/bg_weapons_load_obj.h>
#include <ctype.h>
#include <physics/physconstraints_load_obj.h>
#include <game/g_scr_vehicle.h>
#include <cgame/cg_main.h>
#include <gfx_d3d/r_scene.h>
#include <xanim/xmodel.h>
#include <gfx_d3d/r_utils.h>
#include <game_mp/g_scr_main_mp.h>
#include <win32/win_common.h>

int GDTCommandCount;
char GDTCommands[4][128];

struct gdt_remote_cmd_handler // sizeof=0x8
{                                       // XREF: .data:gdt_remote_handlers/r
    const char *assetType;              // XREF: ProcessGDTCmds(void)+A2/r
    bool (__cdecl *action)(const char *);
};

gdt_remote_cmd_handler gdt_remote_handlers[17] =
{
  { "bulletweapon", GDT_RemoteWeaponUpdate },
  { "grenadeweapon", GDT_RemoteWeaponUpdate },
  { "projectileweapon", GDT_RemoteWeaponUpdate },
  { "gasweapon", GDT_RemoteWeaponUpdate },
  { "turretweapon", GDT_RemoteWeaponUpdate },
  { "vehicleweapon", GDT_RemoteWeaponUpdate },
  { "meleeweapon", GDT_RemoteWeaponUpdate },
  { "dualwieldweapon", GDT_RemoteWeaponUpdate },
  { "vehicle", GDT_RemoteVehicleUpdate },
  { "physpreset", Flame_GetLocalClientSourceRange },
  { "physconstraints", GDT_RemotePhysConstraintsUpdate },
  { "rumble", Flame_GetLocalClientSourceRange },
  { "xmodel", GDT_RemoteXModelUpdate },
  { "material", GDT_RemoteMaterialUpdate },
  { "mphead", GDT_RemoteMptypeUpdate },
  { "mpbody", GDT_RemoteMptypeUpdate },
  { "flametable", GDT_RemoteFlametableUpdate }
};



bool __cdecl GDT_RemoteWeaponUpdate(const char *in)
{
  char weaponName[128]; // [esp+0h] [ebp-88h] BYREF
  const char *keyValue; // [esp+84h] [ebp-4h]

  memset(weaponName, 0, sizeof(weaponName));
  keyValue = GDTCmd_NextToken(in, weaponName, 0x80u);
  return BG_WeaponUpdateField(weaponName, (char *)keyValue) != 0;
}

const char *__cdecl GDTCmd_NextToken(const char *src, char *dst, unsigned int len)
{
  unsigned int start; // [esp+0h] [ebp-8h]
  unsigned int i; // [esp+4h] [ebp-4h]
  unsigned int ia; // [esp+4h] [ebp-4h]

  if ( !src )
    return 0;
  start = 0;
  for ( i = 0; i < len && isspace(src[i]); ++i )
    ++start;
  if ( !src[start] )
    return 0;
  for ( ia = start; ia < len; ++ia )
  {
    if ( isspace(src[ia]) || !src[ia] )
    {
      dst[ia - start] = 0;
      break;
    }
    dst[ia - start] = src[ia];
  }
  while ( ia < len && isspace(src[ia]) )
    ++ia;
  if ( src[ia] )
    return &src[ia];
  else
    return 0;
}

bool __cdecl GDT_RemoteFlametableUpdate(const char *in)
{
  char *keyValue; // [esp+0h] [ebp-8Ch]
  char flameTableName[132]; // [esp+4h] [ebp-88h] BYREF

  memset(flameTableName, 0, 128);
  keyValue = (char *)GDTCmd_NextToken(in, flameTableName, 0x80u);
  return BG_FlameTableUpdateField(flameTableName, keyValue);
}

bool __cdecl GDT_RemotePhysConstraintsUpdate(const char *in)
{
  char *keyValue; // [esp+0h] [ebp-8Ch]
  char assetName[132]; // [esp+4h] [ebp-88h] BYREF

  memset(assetName, 0, 128);
  keyValue = (char *)GDTCmd_NextToken(in, assetName, 0x80u);
  PhysConstraintsUpdateField(assetName, keyValue);
  return 1;
}

bool __cdecl GDT_RemoteVehicleUpdate(const char *in)
{
  char vehicleInfoName[128]; // [esp+0h] [ebp-88h] BYREF
  const char *keyValue; // [esp+84h] [ebp-4h]

  memset(vehicleInfoName, 0, sizeof(vehicleInfoName));
  keyValue = GDTCmd_NextToken(in, vehicleInfoName, 0x80u);
  return G_VehicleUpdateField(vehicleInfoName, (char *)keyValue)
      && CG_VehicleUpdateField(vehicleInfoName, (char *)keyValue) != 0;
}

bool __cdecl GDT_RemoteXModelUpdate(const char *in)
{
  float lodDistance; // [esp+0h] [ebp-358h]
  XModel *v3; // [esp+4h] [ebp-354h]
  volatile int m; // [esp+8h] [ebp-350h]
  XModel *v5; // [esp+Ch] [ebp-34Ch]
  int j; // [esp+10h] [ebp-348h]
  int modelCount; // [esp+14h] [ebp-344h]
  volatile int k; // [esp+18h] [ebp-340h]
  XModel *model; // [esp+1Ch] [ebp-33Ch]
  unsigned int i; // [esp+20h] [ebp-338h]
  char c; // [esp+2Fh] [ebp-329h]
  int lodIndex; // [esp+30h] [ebp-328h]
  GfxScene *scene; // [esp+34h] [ebp-324h]
  char ModelVariable[256]; // [esp+38h] [ebp-320h] BYREF
  int DstIndex; // [esp+138h] [ebp-220h]
  XModel *foundModel; // [esp+13Ch] [ebp-21Ch]
  int SrcIndex; // [esp+140h] [ebp-218h]
  int TokenIndex; // [esp+144h] [ebp-214h]
  char ModelName[256]; // [esp+148h] [ebp-210h] BYREF
  char VariableValue[256]; // [esp+248h] [ebp-110h] BYREF
  char *TokenArray[3]; // [esp+34Ch] [ebp-Ch]

  SrcIndex = 0;
  DstIndex = 0;
  TokenArray[0] = ModelName;
  TokenArray[1] = ModelVariable;
  TokenArray[2] = VariableValue;
  TokenIndex = 0;
  while ( 1 )
  {
    c = in[SrcIndex++];
    if ( !c )
      break;
    if ( c == 92 )
    {
      TokenArray[TokenIndex][DstIndex] = 0;
      DstIndex = 0;
      ++TokenIndex;
    }
    else if ( c != 32 )
    {
      TokenArray[TokenIndex][DstIndex++] = c;
    }
  }
  if ( I_stricmp(TokenArray[1], "highLodDist") )
  {
    if ( I_stricmp(TokenArray[1], "mediumLodDist") )
    {
      if ( I_stricmp(TokenArray[1], "lowLodDist") )
      {
        if ( I_stricmp(TokenArray[1], "lowestLodDist") )
          return 1;
        lodIndex = 3;
      }
      else
      {
        lodIndex = 2;
      }
    }
    else
    {
      lodIndex = 1;
    }
  }
  else
  {
    lodIndex = 0;
  }
  foundModel = 0;
  for ( i = 0; i < rgp.world->dpvs.smodelCount; ++i )
  {
    model = rgp.world->dpvs.smodelDrawInsts[i].model;
    if ( !I_stricmp(model->name, TokenArray[0]) )
    {
      foundModel = model;
      break;
    }
  }
  scene = R_GetScene();
  if ( !scene )
    return 1;
  if ( !foundModel )
  {
    for ( k = 0; k < scene->sceneDObjCount; ++k )
    {
      modelCount = scene->sceneDObj[k].obj->numModels;
      for ( j = 0; j < modelCount; ++j )
      {
        v5 = scene->sceneDObj[k].obj->localModels[j];
        if ( !I_stricmp(v5->name, TokenArray[0]) )
        {
          foundModel = v5;
          break;
        }
      }
      if ( foundModel )
        break;
    }
  }
  if ( !foundModel )
  {
    for ( m = 0; m < scene->sceneModelCount; ++m )
    {
      v3 = (XModel *)scene->sceneModel[m].model;
      if ( !I_stricmp(v3->name, TokenArray[0]) )
      {
        foundModel = v3;
        break;
      }
    }
  }
  if ( foundModel )
  {
    if ( foundModel->numLods > lodIndex )
    {
      lodDistance = atof(VariableValue);
      if ( lodDistance > 0.0 )
        foundModel->lodInfo[lodIndex].dist = lodDistance;
    }
  }
  return 1;
}

bool __cdecl GDT_RemoteMaterialUpdate(const char *in)
{
  float v2; // [esp+0h] [ebp-370h]
  float v3; // [esp+4h] [ebp-36Ch]
  float v4; // [esp+8h] [ebp-368h]
  float v5; // [esp+Ch] [ebp-364h]
  int j; // [esp+10h] [ebp-360h]
  Material *material; // [esp+14h] [ebp-35Ch]
  char *CleanMaterialName; // [esp+18h] [ebp-358h]
  char v9; // [esp+26h] [ebp-34Ah]
  char c; // [esp+27h] [ebp-349h]
  char MaterialVariable[260]; // [esp+28h] [ebp-348h] BYREF
  char RequiredPrefix[12]; // [esp+12Ch] [ebp-244h] BYREF
  char *VariableName; // [esp+138h] [ebp-238h]
  bool IsW; // [esp+13Fh] [ebp-231h]
  unsigned int VariableNameHash; // [esp+140h] [ebp-230h]
  int DstIndex; // [esp+144h] [ebp-22Ch]
  bool IsY; // [esp+14Bh] [ebp-225h]
  int SrcIndex; // [esp+14Ch] [ebp-224h]
  char MaterialName[256]; // [esp+150h] [ebp-220h] BYREF
  int TokenIndex; // [esp+250h] [ebp-120h]
  bool IsZ; // [esp+257h] [ebp-119h]
  int i; // [esp+258h] [ebp-118h]
  bool IsX; // [esp+25Fh] [ebp-111h]
  char VariableValue[256]; // [esp+260h] [ebp-110h] BYREF
  char *TokenArray[3]; // [esp+364h] [ebp-Ch]

  SrcIndex = 0;
  DstIndex = 0;
  TokenArray[0] = MaterialName;
  TokenArray[1] = MaterialVariable;
  TokenArray[2] = VariableValue;
  TokenIndex = 0;
  while ( 1 )
  {
    c = in[SrcIndex++];
    if ( !c )
      break;
    if ( c == 92 )
    {
      TokenArray[TokenIndex][DstIndex] = 0;
      DstIndex = 0;
      ++TokenIndex;
    }
    else if ( c != 32 )
    {
      TokenArray[TokenIndex][DstIndex++] = c;
    }
  }
  strcpy(RequiredPrefix, "SW4_Float_");
  for ( i = 0; i < 256; ++i )
  {
    if ( i < 10 && MaterialVariable[i] != RequiredPrefix[i] )
      return 1;
    if ( !MaterialVariable[i] )
      break;
  }
  *(&v9 + i) = 0;
  VariableName = &MaterialVariable[10];
  IsX = MaterialVariable[i - 1] == 120;
  IsY = MaterialVariable[i - 1] == 121;
  IsZ = MaterialVariable[i - 1] == 122;
  IsW = MaterialVariable[i - 1] == 119;
  VariableNameHash = R_HashString(&MaterialVariable[10]);
  for ( i = 0; i < rgp.materialCount; ++i )
  {
    material = rgp.sortedMaterials[i];
    CleanMaterialName = (char *)material->info.name;
    if ( *material->info.name == 47 )
    {
      ++CleanMaterialName;
    }
    else if ( CleanMaterialName[1] == 47 )
    {
      CleanMaterialName += 2;
    }
    else if ( CleanMaterialName[2] == 47 )
    {
      CleanMaterialName += 3;
    }
    else if ( CleanMaterialName[3] == 47 )
    {
      CleanMaterialName += 4;
    }
    if ( !I_stricmp(CleanMaterialName, MaterialName) )
    {
      for ( j = 0; j < material->constantCount; ++j )
      {
        if ( !strncmp(material->localConstantTable[j].name, VariableName, 0xCu)
          && material->localConstantTable[j].nameHash == VariableNameHash )
        {
          if ( IsX )
          {
            v5 = atof(VariableValue);
            material->localConstantTable[j].literal[0] = v5;
          }
          if ( IsY )
          {
            v4 = atof(VariableValue);
            material->localConstantTable[j].literal[1] = v4;
          }
          if ( IsZ )
          {
            v3 = atof(VariableValue);
            material->localConstantTable[j].literal[2] = v3;
          }
          if ( IsW )
          {
            v2 = atof(VariableValue);
            material->localConstantTable[j].literal[3] = v2;
          }
        }
      }
    }
  }
  return 1;
}

bool __cdecl GDT_RemoteMptypeUpdate(const char *in)
{
  const char *Token; // eax
  char asset[128]; // [esp+0h] [ebp-108h] BYREF
  char keyValue[132]; // [esp+80h] [ebp-88h] BYREF

  Token = GDTCmd_NextToken(in, asset, 0x80u);
  GDTCmd_NextToken(Token, keyValue, 0x80u);
  GScr_Gdt_Update(asset, keyValue);
  return 1;
}

char __cdecl ProcessGDTCmds()
{
  int j; // [esp+0h] [ebp-90h]
  const char *strCommand; // [esp+4h] [ebp-8Ch]
  char assetType[128]; // [esp+8h] [ebp-88h] BYREF
  int i; // [esp+8Ch] [ebp-4h]

  Sys_EnterCriticalSection(CRITSECT_GDT_COMMAND);
  for ( j = 0; j < GDTCommandCount; ++j )
  {
    strCommand = GDTCmd_NextToken(GDTCommands[j], assetType, 0x80u);
    for ( i = 0; (unsigned int)i < 0x11; ++i )
    {
      if ( !I_stricmp(gdt_remote_handlers[i].assetType, assetType) )
      {
        Com_Printf(5, "GDT CMD: %s\n", strCommand);
        gdt_remote_handlers[i].action(strCommand);
        break;
      }
    }
    if ( i == 17 )
      Com_Printf(5, "Unknown GDT CMD: %s for asset type %s\n", strCommand, assetType);
  }
  GDTCommandCount = 0;
  Sys_LeaveCriticalSection(CRITSECT_GDT_COMMAND);
  return 1;
}

