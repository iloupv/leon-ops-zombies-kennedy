#include "db_assetnames.h"
#include "db_registry.h"

#include <gfx_d3d/r_material.h>

#include <xanim/xmodel.h>
#include <universal/q_shared.h>

#include <physics/physpreset_load_obj.h>
#include <physics/physconstraints_load_obj.h>
#include <glass/glass.h>
#include "db_load.h"
#include <bgame/bg_weapons.h>
#include <bgame/bg_local.h>
#include <gfx_d3d/r_bsp_load_obj.h>
#include <bgame/bg_emblems.h>
#include <qcommon/com_bsp.h>
#include <gfx_d3d/r_extracam.h>

const char *(__cdecl *DB_XAssetGetNameHandler[43])(const XAssetHeader *) =
{
  &DB_DDLGetName,
  &DB_DDLGetName,
  &DB_DDLGetName,
  &DB_DDLGetName,
  &DB_DDLGetName,
  &DB_DDLGetName,
  &DB_DDLGetName,
  &DB_DDLGetName,
  &DB_ImageGetName,
  &DB_DDLGetName,
  &DB_DDLGetName,
  &DB_DDLGetName,
  &DB_DDLGetName,
  &DB_DDLGetName,
  &DB_DDLGetName,
  &DB_DDLGetName,
  &DB_DDLGetName,
  &DB_DDLGetName,
  &DB_DDLGetName,
  NULL,
  &DB_DDLGetName,
  &DB_DDLGetName,
  &DB_DDLGetName,
  &DB_LocalizeEntryGetName,
  &DB_DDLGetName,
  NULL,
  NULL,
  &DB_DDLGetName,
  &DB_DDLGetName,
  &DB_DDLGetName,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  &DB_DDLGetName,
  &DB_DDLGetName,
  &DB_DDLGetName,
  &DB_DDLGetName,
  &DB_DDLGetName,
  &DB_DDLGetName,
  &DB_GetEmblemSetName
};

const char *g_assetNames[43] =
{
  "xmodelpieces",
  "physpreset",
  "physconstraints",
  "destructibledef",
  "xanim",
  "xmodel",
  "material",
  "techset",
  "image",
  "sound",
  "sound_patch",
  "col_map_sp",
  "col_map_mp",
  "com_map",
  "game_map_sp",
  "game_map_mp",
  "map_ents",
  "gfx_map",
  "lightdef",
  "ui_map",
  "font",
  "menufile",
  "menu",
  "localize",
  "weapon",
  "weapondef",
  "weaponvariant",
  "snddriverglobals",
  "fx",
  "impactfx",
  "aitype",
  "mptype",
  "mpbody",
  "mphead",
  "character",
  "xmodelalias",
  "rawfile",
  "stringtable",
  "packindex",
  "xGlobals",
  "ddl",
  "glasses",
  "emblemset"
};

void(__cdecl *DB_XAssetSetNameHandler[43])(XAssetHeader *, const char *) =
{
  &DB_DDLSetname,
  &DB_DDLSetname,
  &DB_DDLSetname,
  &DB_DDLSetname,
  &DB_DDLSetname,
  &DB_DDLSetname,
  &DB_DDLSetname,
  &DB_DDLSetname,
  (void(*)(XAssetHeader*, const char*))&DB_ImageSetName,
  &DB_DDLSetname,
  NULL,
  &DB_DDLSetname,
  &DB_DDLSetname,
  &DB_DDLSetname,
  &DB_DDLSetname,
  &DB_DDLSetname,
  &DB_DDLSetname,
  &DB_DDLSetname,
  &DB_DDLSetname,
  NULL,
  &DB_DDLSetname,
  &DB_DDLSetname,
  &DB_DDLSetname,
  &DB_LocalizeEntrySetName,
  &DB_DDLSetname,
  NULL,
  NULL,
  &DB_DDLSetname,
  &DB_DDLSetname,
  &DB_DDLSetname,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  &DB_DDLSetname,
  &DB_DDLSetname,
  &DB_DDLSetname,
  &DB_DDLSetname,
  &DB_DDLSetname,
  NULL,
  NULL
};

static_assert(sizeof(RawFile) == 12);
int __cdecl DB_SizeofXAsset_RawFile_()
{
    return 12;
}

static_assert(sizeof(PhysPreset) == 84);
int __cdecl DB_SizeofXAsset_PhysPreset_()
{
    return 84;
}

static_assert(sizeof(PhysConstraints) == 2696);
int __cdecl DB_SizeofXAsset_PhysConstraints_()
{
    return 2696;
}

static_assert(sizeof(DestructibleDef) == 24);
//int __cdecl SV_GetMaxAttachCount()
int __cdecl DB_SizeofXAsset_DestructibleDef_()
{
    return 24;
}

static_assert(sizeof(Font_s) == 24);
int __cdecl DB_SizeofXAsset_Font_s_()
{
    return 24;
}

static_assert(sizeof(XAnimParts) == 104);
//int __cdecl PM_MediumLandingForSurface()
int __cdecl DB_SizeofXAsset_XAnimParts_()
{
    return 104;
}

static_assert(sizeof(XModel) == 252);
int __cdecl DB_SizeofXAsset_XModel_()
{
    return 252;
}

// (for some reason the intellisense size is wrong unless it's right next to the struct definition)
int __cdecl DB_SizeofXAsset_Material_()
{
    return 192;
}

static_assert(sizeof(MaterialTechniqueSet) == 528);
int __cdecl DB_SizeofXAsset_MaterialTechniqueSet_()
{
    return 528;
}

static_assert(sizeof(SndDriverGlobals) == 52);
int __cdecl DB_SizeofXAsset_SndDriverGlobals_()
{
    return 52;
}

static_assert(sizeof(XGlobals) == 40);
int __cdecl DB_SizeofXAsset_XGlobals_()
{
    return 40;
}

static_assert(sizeof(StringTable) == 20);
int __cdecl DB_SizeofXAsset_StringTable_()
{
    return 20;
}

static_assert(sizeof(clipMap_t) == 332);
int __cdecl DB_SizeofXAsset_clipMap_t_()
{
    return 332;
}

static_assert(sizeof(ComWorld) == 64);
int __cdecl DB_SizeofXAsset_ComWorld_()
{
    return 64;
}

static_assert(sizeof(EmblemSet) == 44);
int __cdecl DB_SizeofXAsset_EmblemSet_()
{
    return 44;
}

static_assert(sizeof(GfxWorld) == 1084);
int __cdecl DB_SizeofXAsset_GfxWorld_()
{
    return 1084;
}

static_assert(sizeof(GfxLightDef) == 16);
int __cdecl DB_SizeofXAsset_GfxLightDef_()
{
    return 16;
}

static_assert(sizeof(menuDef_t) == 400);
int __cdecl DB_SizeofXAsset_menuDef_t_()
{
    return 400;
}

static_assert(sizeof(XAnimTree_s) == 8);
//int __cdecl XAnimTreeSize()
//{
//    return 8;
//}

static_assert(sizeof(WeaponVariantDef) == 228);
int __cdecl DB_SizeofXAsset_WeaponVariantDef_()
{
    return 228;
}

static_assert(sizeof(FxEffectDef) == 60);
int __cdecl DB_SizeofXAsset_FxEffectDef_()
{
    return 60;
}

static_assert(sizeof(PackIndex) == 28);
int __cdecl DB_SizeofXAsset_PackIndex_()
{
    return 28;
}

static_assert(sizeof(Glasses) == 56);
int __cdecl DB_SizeofXAsset_Glasses_()
{
    return 56;
}

int(__cdecl *DB_GetXAssetSizeHandler[43])() =
{
  &DB_SizeofXAsset_RawFile_,
  &DB_SizeofXAsset_PhysPreset_,
  &DB_SizeofXAsset_PhysConstraints_,
  &DB_SizeofXAsset_DestructibleDef_,
  //&PM_MediumLandingForSurface,
  DB_SizeofXAsset_XAnimParts_,
  &DB_SizeofXAsset_XModel_,
  &DB_SizeofXAsset_Material_,
  &DB_SizeofXAsset_MaterialTechniqueSet_,
  &DB_SizeofXAsset_SndDriverGlobals_,
  &DB_SizeofXAsset_XGlobals_,
  &DB_SizeofXAsset_StringTable_,
  &DB_SizeofXAsset_clipMap_t_,
  &DB_SizeofXAsset_clipMap_t_,
  &DB_SizeofXAsset_ComWorld_,
  &DB_SizeofXAsset_EmblemSet_,
  &DB_SizeofXAsset_EmblemSet_,
  &DB_SizeofXAsset_RawFile_,
  &DB_SizeofXAsset_GfxWorld_,
  &DB_SizeofXAsset_GfxLightDef_,
  NULL,
  &DB_SizeofXAsset_Font_s_,
  &DB_SizeofXAsset_RawFile_,
  &DB_SizeofXAsset_menuDef_t_,
  &XAnimTreeSize,
  &DB_SizeofXAsset_WeaponVariantDef_,
  NULL,
  NULL,
  &DB_SizeofXAsset_SndDriverGlobals_,
  &DB_SizeofXAsset_FxEffectDef_,
  &XAnimTreeSize,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  &DB_SizeofXAsset_RawFile_,
  &DB_SizeofXAsset_StringTable_,
  &DB_SizeofXAsset_PackIndex_,
  &DB_SizeofXAsset_XGlobals_,
  &XAnimTreeSize,
  &DB_SizeofXAsset_Glasses_,
  &DB_SizeofXAsset_EmblemSet_
};





const char *__cdecl DB_ImageGetName(const XAssetHeader *header)
{
    return header->image->name;
}

void __cdecl DB_ImageSetName(XAssetHeader *header, const char *name)
{
    //header->xmodelPieces[3].pieces = name;
    header->image->name = name;
}

const char *__cdecl DB_LocalizeEntryGetName(const XAssetHeader *header)
{
    return (const char *)header->xmodelPieces->numpieces;
}

void __cdecl DB_LocalizeEntrySetName(XAssetHeader *header, const char *name)
{
    header->xmodelPieces->numpieces = (int)name;
}

void __cdecl DB_DDLSetname(XAssetHeader *header, const char *name)
{
    header->xmodelPieces->name = name;
}

const char *__cdecl DB_DDLGetName(const XAssetHeader *header)
{
    return header->xmodelPieces->name;
}

const char *__cdecl DB_GetEmblemSetName(const XAssetHeader *header)
{
    return "emblemset";
}

const char *__cdecl DB_GetXAssetHeaderName(int type, const XAssetHeader *header)
{
    const char *v2; // eax
    const char *name; // [esp+0h] [ebp-4h]

    if ( !header
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\database\\db_assetnames.cpp", 687, 0, "%s", "header") )
    {
        __debugbreak();
    }
    if ( !DB_XAssetGetNameHandler[type]
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\database\\db_assetnames.cpp",
                    688,
                    0,
                    "%s",
                    "DB_XAssetGetNameHandler[type]") )
    {
        __debugbreak();
    }
    if ( !header->xmodelPieces
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\database\\db_assetnames.cpp", 689, 0, "%s", "header->data") )
    {
        __debugbreak();
    }
    name = DB_XAssetGetNameHandler[type](header);
    if ( !name )
    {
        v2 = va("Name \"%s\" not found for asset type %s\n", 0, g_assetNames[type]);
        if ( !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\database\\db_assetnames.cpp",
                        691,
                        0,
                        "%s\n\t%s",
                        "name",
                        v2) )
            __debugbreak();
    }
    return name;
}

const char *__cdecl DB_GetXAssetName(const XAsset *asset)
{
    if ( !asset
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\database\\db_assetnames.cpp", 698, 0, "%s", "asset") )
    {
        __debugbreak();
    }
    return DB_GetXAssetHeaderName(asset->type, &asset->header);
}

void __cdecl DB_SetXAssetName(XAsset *asset, const char *name)
{
    if ( !DB_XAssetSetNameHandler[asset->type]
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\database\\db_assetnames.cpp",
                    705,
                    0,
                    "%s",
                    "DB_XAssetSetNameHandler[asset->type]") )
    {
        __debugbreak();
    }
    DB_XAssetSetNameHandler[asset->type](&asset->header, name);
}

int __cdecl DB_GetXAssetTypeSize(int type)
{
    if ( !DB_GetXAssetSizeHandler[type]
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\database\\db_assetnames.cpp",
                    712,
                    0,
                    "%s",
                    "DB_GetXAssetSizeHandler[type]") )
    {
        __debugbreak();
    }
    return DB_GetXAssetSizeHandler[type]();
}

const char *__cdecl DB_GetXAssetTypeName(unsigned int type)
{
    if ( type > 0x2A
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\database\\db_assetnames.cpp",
                    718,
                    0,
                    "%s",
                    "type >= 0 && type < ASSET_TYPE_COUNT") )
    {
        __debugbreak();
    }
    return g_assetNames[type];
}

