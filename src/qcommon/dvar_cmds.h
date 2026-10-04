#pragma once
#include <universal/dvar.h>

struct DvarDumpInfo // sizeof=0xC
{                                       // XREF: ?Com_DvarDump@@YAXHPBD@Z/r
    int count;                          // XREF: Com_DvarDump(int,char const *)+45/w
                                        // Com_DvarDump(int,char const *)+7C/r
    int channel;                        // XREF: Com_DvarDump(int,char const *)+52/w
    const char *match;                  // XREF: Com_DvarDump(int,char const *)+5B/w
};

struct DvarSetConfigInfo // sizeof=0xC
{                                       // XREF: ?SV_SetConfig@@YAXHHH@Z/r
    int start;                          // XREF: SV_SetConfig(int,int,int)+9/w
    int max;                            // XREF: SV_SetConfig(int,int,int)+F/w
    int bit;                            // XREF: SV_SetConfig(int,int,int)+15/w
};

int __cdecl Dvar_Command(bool restricted);
void __cdecl Dvar_GetCombinedString(char *combined, int first);
void __cdecl Dvar_Toggle_f();
bool __cdecl Dvar_ToggleInternal();
bool __cdecl Dvar_ToggleSimple(dvar_s *dvar);
void __cdecl Dvar_TogglePrint_f();
void __cdecl Dvar_Set_f();
void __cdecl Dvar_SetU_f();
void __cdecl Dvar_SetS_f();
void __cdecl Dvar_SetA_f();
void __cdecl Dvar_SetAdmin_f();
void __cdecl Dvar_SetFromDvar_f();
void __cdecl Dvar_Reset_f();
void __cdecl Dvar_WriteVariables(int f);
void __cdecl Dvar_WriteSingleVariable(const dvar_s *dvar, int *userData);
void __cdecl Dvar_WriteDefaults(int f);
void __cdecl Dvar_WriteSingleDefault(const dvar_s *dvar, int *userData);
void __cdecl Dvar_List_f();
void __cdecl Dvar_ListSingle(const dvar_s *dvar, const char *userData);
void __cdecl Com_DvarDump(int channel, const char *match);
void __cdecl Com_DvarDumpSingle(const dvar_s *dvar, void *userData);
void __cdecl Dvar_Dump_f();
void __cdecl SV_SetConfig(int start, int max, int bit);
void __cdecl SV_SetConfigDvar(const dvar_s *dvar, int *userData);
char *__cdecl Dvar_InfoString(int localClientNum, int bit);
void __cdecl Dvar_InfoStringSingle(const dvar_s *dvar, unsigned int *userData);
char *__cdecl Dvar_InfoString_Big(int bit);
void __cdecl Dvar_InfoStringSingle_Big(const dvar_s *dvar, unsigned int *userData);
void __cdecl Dvar_AddCommands();
void __cdecl Dvar_RegisterBool_f();
void __cdecl Dvar_RegisterInt_f();
void __cdecl Dvar_RegisterFloat_f();
void __cdecl Dvar_RegisterColor_f();
void __cdecl Dvar_SetFromLocalizedStr_f();
void __cdecl Dvar_SetToTime_f();
