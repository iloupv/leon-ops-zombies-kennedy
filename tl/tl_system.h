#pragma once

union _LARGE_INTEGER;

struct tlFileBuf // sizeof=0xC
{
    unsigned __int8 *Buf;
    unsigned int Size;
    unsigned int UserData;
};

struct tlSystemCallbacks // sizeof=0x20
{                                       // XREF: .data:tlSystemCallbacks tlCurSystemCallbacks/r
    bool (__cdecl *ReadFile)(const char *, tlFileBuf *, unsigned int, unsigned int);
    void (__cdecl *ReleaseFile)(tlFileBuf *);
    void (__cdecl *CriticalError)(const char *);
    void (__cdecl *Warning)(const char *);
    void (__cdecl *DebugPrint)(const char *);
    void *(__cdecl *MemAlloc)(unsigned int, unsigned int, unsigned int);
    void *(__cdecl *MemRealloc)(void *, unsigned int, unsigned int, unsigned int);
    void (__cdecl *MemFree)(void *);    // XREF: Sys_SetupTLCallbacks(int)+55/w
};

void __cdecl tlSetSystemCallbacks(const tlSystemCallbacks *Callbacks);
void __cdecl tlSetFileServerRootPC(const char *Path);
void __cdecl tlMemFree(void *Ptr);
void __cdecl tlReleaseFile(tlFileBuf *File);
_LARGE_INTEGER __cdecl tlPcGetTick();
void __cdecl tlPrint(const char *txt);
int __cdecl tlGetVersion();
void __cdecl tlStackRangeInit();
char __cdecl tlFatalHandler(const char *Msg);
void __cdecl tlDebugPrint(const char *txt);
void __cdecl tlVPrintf(const char *Format, char *args);
void tlPrintf(const char *Format, ...);
char __cdecl _tlAssert(const char *file, int line, const char *expr, const char *desc);
void tlFatal(const char *Format, ...);
void *__cdecl tlMemAlloc(unsigned int Size, unsigned int Align, unsigned int Flags);
void *__cdecl tlMemRealloc(void *Ptr, unsigned int Size, unsigned int Align, unsigned int Flags);
unsigned int __cdecl tlGetFreeMemory();
void *__cdecl tlScratchPadInit();
void __cdecl tlScratchPadReset();
void tlWarning(const char *Format, ...);
bool __cdecl tlReadFile(const char *FileName, tlFileBuf *File, unsigned int Align, unsigned int Flags);

extern float tlPcTicksPerMS;
extern bool tlScratchpadLocked;