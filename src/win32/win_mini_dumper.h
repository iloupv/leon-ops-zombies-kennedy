#pragma once

#include <Windows.h>

struct __declspec(align(4)) miniDumper // sizeof=0x314
{                                       // XREF: .data:g_miniDumper/r
    _EXCEPTION_POINTERS *m_pExceptionInfo;
    char m_szMiniDumpPath[260];
    char m_szAppPath[260];
    char m_szAppBaseName[260];
    bool m_bPromptUserForMiniDump;
    // padding byte
    // padding byte
    // padding byte


    static miniDumper *s_pMiniDumper;
    static LPCRITICAL_SECTION s_pCriticalSection;

    miniDumper(bool bPromptUserForMiniDump);
    ~miniDumper();


    //typedef LONG(WINAPI *PTOP_LEVEL_EXCEPTION_FILTER)(
    //    _In_ struct _EXCEPTION_POINTERS *ExceptionInfo
    //    );

    static LONG __stdcall unhandledExceptionHandler(_EXCEPTION_POINTERS *pExceptionInfo);

    void setMiniDumpFileName();
    char getImpersonationToken(void **phToken);
    bool enablePrivilege(
        const char *pszPriv,
        void *hToken,
        _TOKEN_PRIVILEGES *ptpOld);
    bool restorePrivilege(void *hToken, _TOKEN_PRIVILEGES *ptpOld);
    int writeMiniDump(_EXCEPTION_POINTERS *pExceptionInfo);

};

void __cdecl Sys_StartMiniDump(bool prompt);
bool __cdecl Sys_IsMiniDumpStarted();

