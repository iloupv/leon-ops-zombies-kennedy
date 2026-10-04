#pragma once
#include <bgame/bg_local.h>
#include <bgame/bg_public.h>
#include <bgame/bg_weapons.h>

enum itemType_t : __int32
{                                       // XREF: gitem_s/r
    IT_BAD = 0x0,
    IT_WEAPON = 0x1,
};

struct gitem_s // sizeof=0x4
{                                       // XREF: gitem_t/r
    itemType_t giType;
};

void __cdecl Fill_Clip(playerState_s *ps, unsigned int weapon);
int __cdecl Add_Ammo(gentity_s *ent, unsigned int weaponIndex, unsigned __int8 weaponModel, int count, int fillClip);
void __cdecl Touch_Item_Auto(gentity_s *ent, gentity_s *other, int bTouched);
void __cdecl Touch_Item(gentity_s *ent, gentity_s *other, int touched);
int __cdecl WeaponPickup(gentity_s *weaponEnt, gentity_s *player, int *pickupEvent, int touched);
int __cdecl WeaponPickup_Grab(gentity_s *weaponEnt, gentity_s *player, int weapIdx, int *pickupEvent);
int __cdecl WeaponPickup_AddWeapon(
                gentity_s *ent,
                gentity_s *other,
                int weapon,
                unsigned __int8 weaponModel,
                gentity_s **pDroppedWeapon);
int __cdecl CurrentPrimaryWeapon(playerState_s *ps);
int __cdecl G_ItemClipMask(gentity_s *ent);
bool __cdecl WeaponPickup_LeechFromWeaponEnt(
                gentity_s *weaponEnt,
                gentity_s *player,
                int haveExactWeapon,
                int *pickupEvent,
                bool suppressNotifies);
void __cdecl PrintPlayerPickupMessage(gentity_s *player, unsigned int weapIdx);
void __cdecl WeaponPickup_AddAmmoForNewWeapon(gentity_s *weaponEnt, gentity_s *player);
void __cdecl WeaponPickup_Notifies(
                gentity_s *thisItem,
                gentity_s *newDroppedItem,
                gentity_s *player,
                const WeaponDef *weapDef);
bool __cdecl WeaponPickup_Touch(gentity_s *weaponEnt, gentity_s *player, unsigned int weapIdx, int *pickupEvent);
void __cdecl PrintMessage_CannotGrabItem(
                gentity_s *ent,
                gentity_s *player,
                int touched,
                gitem_s *item,
                unsigned int weapIndex);
void __cdecl DroppedItemClearOwner(gentity_s *pSelf);
void __cdecl G_GetItemClassname(const gitem_s *item, unsigned __int16 *out);
gentity_s *__cdecl Drop_Item(gentity_s *ent, const gitem_s *item, float angle, int novelocity);
gentity_s *__cdecl LaunchItem(const gitem_s *item, float *origin, float *angles, float *velocity, int ownerNum);
int __cdecl GetFreeDropCueIdx();
gentity_s *__cdecl ThrowClientWeapon(gentity_s *ent, const gitem_s *item, float heightIncrease, float velocityIncrease);
bool __cdecl PlayerHasAnyAmmoToTransferToWeapon(gentity_s *player, unsigned int transferWeapon);
int __cdecl GetNonClipAmmoToTransferToWeaponEntity(gentity_s *player, unsigned int transferWeapon);
gentity_s *__cdecl ThrowDownWeapon(
                gentity_s *ent,
                int weapIdx,
                unsigned __int8 weaponModel,
                unsigned int tag,
                float throwHeight,
                float throwDistance);
int __cdecl TransferPlayerAmmoToWeaponEntity(gentity_s *player, gentity_s *weaponEnt, int transferWeapon);
int __cdecl TransferRandomAmmoToWeaponEntity(gentity_s *weaponEnt, int transferWeapon);
int __cdecl BG_GetDropMinAmmo(unsigned int weaponIndex);
int __cdecl BG_GetDropMaxAmmo(unsigned int weaponIndex);
gentity_s *__cdecl Drop_Weapon(gentity_s *ent, int weapIdx, unsigned __int8 weaponModel, unsigned int tag);
void __cdecl FinishSpawningItem(gentity_s *ent);
void __cdecl ClearRegisteredItems();
void __cdecl SaveRegisteredWeapons();
void __cdecl SaveRegisteredItems();
void __cdecl G_RegisterWeapon(unsigned int weapIndex);
int __cdecl IsItemRegistered(unsigned int iItemIndex);
void __cdecl G_SpawnItem(gentity_s *ent, const gitem_s *item);
int __cdecl TransferSpecifiedAmmoToWeaponEntity(gentity_s *weaponEnt, int transferWeapon);
void __cdecl G_RunItem(gentity_s *ent);
void __cdecl G_OrientItemToGround(gentity_s *ent, trace_t *trace);
