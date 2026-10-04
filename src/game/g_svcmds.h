#pragma once

struct ipFilter_s // sizeof=0x8
{                                       // XREF: .data:ipFilters/r
                                        // ipFilter_t/r ...
    unsigned int mask;                  // XREF: Svcmd_RemoveIP_f(void)+80/r
                                        // Svcmd_RemoveIP_f(void)+87/r
    unsigned int compare;               // XREF: AddIP+24/r AddIP+87/w ...
};

void __cdecl G_ProcessIPBans();
void __cdecl AddIP(char *str);
int __cdecl StringToFilter(char *s, ipFilter_s *f);
void UpdateIPBans();
void __cdecl Svcmd_AddIP_f();
void __cdecl Svcmd_RemoveIP_f();
void __cdecl Svcmd_EntityList_f();
int __cdecl ConsoleCommand();
