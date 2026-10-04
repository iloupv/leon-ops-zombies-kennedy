#pragma once

struct StateBitsTable // sizeof=0x8
{                                       // XREF: RB_LogPrintState_0(int,int)+14/w
                                        // RB_LogPrintState_0(int,int)+4C/w ...
    int stateBits;                      // XREF: RB_LogPrintState_0(int,int)+6/w
                                        // RB_LogPrintState_0(int,int)+22/w ...
    const char *name;                   // XREF: RB_LogPrintState_0(int,int)+D/w
                                        // RB_LogPrintState_0(int,int)+1B/w ...
};

struct StencilLogBits // sizeof=0x18
{                                       // XREF: ?RB_LogPrintState_1@@YAXHH@Z/r
                                        // ?RB_LogPrintState_1@@YAXHH@Z/r
    const char *description;            // XREF: RB_LogPrintState_1(int,int)+84/w
                                        // RB_LogPrintState_1(int,int)+AE/w
    int enableMask;                     // XREF: RB_LogPrintState_1(int,int)+8B/w
                                        // RB_LogPrintState_1(int,int)+B5/w
    int passShift;                      // XREF: RB_LogPrintState_1(int,int)+92/w
                                        // RB_LogPrintState_1(int,int)+BC/w
    int failShift;                      // XREF: RB_LogPrintState_1(int,int)+99/w
                                        // RB_LogPrintState_1(int,int)+C3/w
    int zfailShift;                     // XREF: RB_LogPrintState_1(int,int)+A0/w
                                        // RB_LogPrintState_1(int,int)+CA/w
    int funcShift;                      // XREF: RB_LogPrintState_1(int,int)+A7/w
                                        // RB_LogPrintState_1(int,int)+D1/w
};

void __cdecl RB_UpdateLogging();
void RB_CloseLogFile();
void RB_OpenLogFile();
bool __cdecl RB_IsLogging();
void __cdecl RB_LogPrint(const char *text);
const char *__cdecl RB_LogTechniqueType(unsigned __int8 techType);
void __cdecl RB_LogPrintState_0(int stateBits0, int changedBits0);
void __cdecl RB_LogBlend(const char *format, unsigned int blend);
void __cdecl RB_LogBlendOp(const char *format, unsigned int blendOp);
void __cdecl RB_LogFromTable(
                const char *keyName,
                int stateBits,
                int changedBits,
                int bitMask,
                char bitShift,
                const StateBitsTable *table,
                int tableCount);
void __cdecl RB_LogPrintState_1(int stateBits1, int changedBits1);
void __cdecl RB_LogBool(
                const char *keyName,
                int stateBits,
                int changedBits,
                int bitMask,
                const char *trueName,
                const char *falseName);
void __cdecl RB_LogStencilState(int stateBits1, int changedBits1, const StencilLogBits *desc);
