#pragma once

bool __cdecl GDT_RemoteWeaponUpdate(const char *in);
const char *__cdecl GDTCmd_NextToken(const char *src, char *dst, unsigned int len);
bool __cdecl GDT_RemoteFlametableUpdate(const char *in);
bool __cdecl GDT_RemotePhysConstraintsUpdate(const char *in);
bool __cdecl GDT_RemoteVehicleUpdate(const char *in);
bool __cdecl GDT_RemoteXModelUpdate(const char *in);
bool __cdecl GDT_RemoteMaterialUpdate(const char *in);
bool __cdecl GDT_RemoteMptypeUpdate(const char *in);
char __cdecl ProcessGDTCmds();
