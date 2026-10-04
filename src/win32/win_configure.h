#pragma once
#include <qcommon/common.h>

int __cdecl Sys_SystemMemoryMB();
void __cdecl Sys_DetectVideoCard(int descLimit, char *description);
void __cdecl Sys_DetectCpuVendorAndName(char *vendor, char *name);
void __cdecl Sys_CopyCpuidString(char *dest, const char *source, unsigned int maxLen);
bool __cdecl Sys_SupportsSSE();
void __cdecl Sys_SetAutoConfigureGHz(SysInfo *sysInfo);
void __cdecl Sys_GetPhysicalCpuCount(SysInfo *sysInfo);
unsigned int __cdecl Sys_AddApicIdIfUnique(
                unsigned int apicId,
                unsigned int *existingApicId,
                unsigned int existingCount);
long double __cdecl Sys_BenchmarkGHz();
