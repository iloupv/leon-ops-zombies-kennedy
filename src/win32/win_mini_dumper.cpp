#include "win_mini_dumper.h"
#include <stdio.h>
#include <ctime>
#include <universal/q_shared.h>

const char *BUILD_MACHINE = "CODPCAB-V64";
const char *BUILD_TIME = "Fri Nov 05 11:33:52 2010";
const int BUILD_NUMBER = 61;
const int CHANGELIST_NUMBER = 794515;

miniDumper *miniDumper::s_pMiniDumper;
LPCRITICAL_SECTION miniDumper::s_pCriticalSection;

miniDumper *g_miniDumper;
bool g_miniDumperStarted;

void __cdecl Sys_StartMiniDump(bool prompt)
{
    static int _S1_10 = 0;
    if ( (_S1_10 & 1) == 0 )
    {
        _S1_10 |= 1u;
        //miniDumper::miniDumper(&g_miniDumper, prompt);
        //atexit(Sys_StartMiniDump_::_2_::_dynamic_atexit_destructor_for__g_miniDumper__);
        g_miniDumper = new miniDumper(prompt);
    }
    g_miniDumperStarted = 1;
}

bool __cdecl Sys_IsMiniDumpStarted()
{
    return g_miniDumperStarted;
}

miniDumper::miniDumper(bool bPromptUserForMiniDump)
{
    miniDumper::s_pMiniDumper = this;
    this->m_bPromptUserForMiniDump = bPromptUserForMiniDump;
    SetUnhandledExceptionFilter(miniDumper::unhandledExceptionHandler);
    miniDumper::s_pCriticalSection = (LPCRITICAL_SECTION)operator new(0x18u);
    if ( miniDumper::s_pCriticalSection )
        InitializeCriticalSection(miniDumper::s_pCriticalSection);
}

miniDumper::~miniDumper()
{
    if ( miniDumper::s_pCriticalSection )
    {
        DeleteCriticalSection(miniDumper::s_pCriticalSection);
        operator delete(miniDumper::s_pCriticalSection);
    }
}

LONG __stdcall miniDumper::unhandledExceptionHandler(_EXCEPTION_POINTERS *pExceptionInfo)
{
    if (miniDumper::s_pMiniDumper)
    {
        //return miniDumper::writeMiniDump(miniDumper::s_pMiniDumper, pExceptionInfo);
        return miniDumper::s_pMiniDumper->writeMiniDump(pExceptionInfo);
    }
    else
    {
        return 0;
    }
}

void miniDumper::setMiniDumpFileName()
{
    __int64 currentTime; // [esp+4h] [ebp-8h] BYREF

    _time64(&currentTime);
    sprintf(
        this->m_szMiniDumpPath,
        "%s%s.%s.#%d.CL#%d.%ld.dmp",
        this->m_szAppPath,
        this->m_szAppBaseName,
        BUILD_MACHINE,
        BUILD_NUMBER,
        CHANGELIST_NUMBER,
        (unsigned int)currentTime);
}

char miniDumper::getImpersonationToken(void **phToken)
{
    HANDLE CurrentThread; // eax
    HANDLE CurrentProcess; // eax

    *phToken = 0;
    CurrentThread = GetCurrentThread();
    if ( !OpenThreadToken(CurrentThread, 0x28u, 1, phToken) )
    {
        if ( GetLastError() != 1008 )
            return 0;
        CurrentProcess = GetCurrentProcess();
        if ( !OpenProcessToken(CurrentProcess, 0x28u, phToken) )
            return 0;
    }
    return 1;
}

bool miniDumper::enablePrivilege(
                const char *pszPriv,
                void *hToken,
                _TOKEN_PRIVILEGES *ptpOld)
{
    unsigned long cbOld; // [esp+8h] [ebp-18h] BYREF
    _TOKEN_PRIVILEGES tp; // [esp+Ch] [ebp-14h] BYREF
    int bOk; // [esp+1Ch] [ebp-4h]

    bOk = 0;
    tp.PrivilegeCount = 1;
    tp.Privileges[0].Attributes = 2;
    bOk = LookupPrivilegeValueA(0, pszPriv, &tp.Privileges[0].Luid);
    if ( bOk )
    {
        cbOld = 16;
        bOk = AdjustTokenPrivileges(hToken, 0, &tp, 0x10u, ptpOld, &cbOld);
    }
    return bOk && GetLastError() != 1300;
}

bool miniDumper::restorePrivilege(void *hToken, _TOKEN_PRIVILEGES *ptpOld)
{
    return AdjustTokenPrivileges(hToken, 0, ptpOld, 0, 0, 0) && GetLastError() != 1300;
}

int miniDumper::writeMiniDump(_EXCEPTION_POINTERS *pExceptionInfo)
{
    return 0;

    // do I look like I care about minidumps in an open src project? no, not at the moment
#if 0
    char *v3; // eax
    HANDLE CurrentProcess; // eax
    unsigned intv5; // eax
    unsigned intLastError; // eax
    HANDLE v7; // eax
    unsigned intCurrentProcessId; // [esp-18h] [ebp-1198h]
    HANDLE v9; // [esp-14h] [ebp-1194h]
    char v10; // [esp+7h] [ebp-1179h]
    unsigned int *v11; // [esp+8h] [ebp-1178h]
    char v12; // [esp+Fh] [ebp-1171h]
    char *v13; // [esp+14h] [ebp-116Ch]
    char *m_szAppPath; // [esp+18h] [ebp-1168h]
    char v15; // [esp+1Fh] [ebp-1161h]
    char *m_szAppBaseName; // [esp+24h] [ebp-115Ch]
    char *v17; // [esp+28h] [ebp-1158h]
    _TOKEN_PRIVILEGES ptpOld; // [esp+30h] [ebp-1150h] BYREF
    bool v20; // [esp+40h] [ebp-1140h]
    unsigned int v21[3]; // [esp+44h] [ebp-113Ch] BYREF
    bool v22; // [esp+50h] [ebp-1130h]
    HANDLE hObject; // [esp+54h] [ebp-112Ch]
    char string[4100]; // [esp+58h] [ebp-1128h] BYREF
    bool (__stdcall *MiniDumpWriteDump)(HANDLE, DWORD, HANDLE, MINIDUMP_TYPE, PMINIDUMP_EXCEPTION_INFORMATION, PMINIDUMP_USER_STREAM_INFORMATION, PMINIDUMP_CALLBACK_INFORMATION); // [esp+105Ch] [ebp-124h]
    int v26; // [esp+1060h] [ebp-120h]
    void *phToken; // [esp+1064h] [ebp-11Ch] BYREF
    char LibFileName[264]; // [esp+1068h] [ebp-118h] BYREF
    LPCSTR lpText; // [esp+1174h] [ebp-Ch]
    HMODULE hModule; // [esp+1178h] [ebp-8h]
    int v31; // [esp+117Ch] [ebp-4h]

    v31 = 0;
    this->m_pExceptionInfo = pExceptionInfo;
    phToken = 0;
    if ( !miniDumper::getImpersonationToken(this, &phToken) )
        return 0;
    hModule = 0;
    if ( GetModuleFileNameA(0, this->m_szAppPath, 0x104u) )
    {
        v3 = strrchr(this->m_szAppPath, 0x5Cu);
        v26 = v3;
        if ( v3 )
        {
            v17 = (char *)(v26 + 1);
            m_szAppBaseName = this->m_szAppBaseName;
            do
            {
                v15 = *v17;
                *m_szAppBaseName++ = *v17++;
            }
            while ( v15 );
            *(_BYTE *)(v26 + 1) = 0;
        }
        m_szAppPath = this->m_szAppPath;
        v13 = LibFileName;
        do
        {
            v12 = *m_szAppPath;
            *v13++ = *m_szAppPath++;
        }
        while ( v12 );
        v11 = (void **)((char *)&phToken + 3);
        do
        {
            v10 = *((_BYTE *)v11 + 1);
            v11 = (unsigned int *)((char *)v11 + 1);
        }
        while ( v10 );
        *v11 = *(unsigned int *)::LibFileName;
        v11[1] = 777014341;
        v11[2] = (char *)&loc_4C4C40 + 4;
        hModule = LoadLibraryA(LibFileName);
    }
    if ( !hModule )
        hModule = LoadLibraryA(::LibFileName);
    lpText = 0;
    if ( hModule )
    {
        MiniDumpWriteDump = (bool (__stdcall *)(HANDLE, DWORD, HANDLE, MINIDUMP_TYPE, PMINIDUMP_EXCEPTION_INFORMATION, PMINIDUMP_USER_STREAM_INFORMATION, PMINIDUMP_CALLBACK_INFORMATION))GetProcAddress(hModule, "MiniDumpWriteDump");
        if ( MiniDumpWriteDump )
        {
            miniDumper::setMiniDumpFileName(this);
            sprintf(
                string,
                "There was an unexpected error:\n\nWould you like to create a mini-dump file?\n\n%s ",
                this->m_szMiniDumpPath);
            if ( !this->m_bPromptUserForMiniDump || MessageBoxA(0, string, 0, 4u) == 6 )
            {
                hObject = CreateFileA(this->m_szMiniDumpPath, 0x40000000u, 2u, 0, 2u, 0x80u, 0);
                if ( hObject == (HANDLE)-1 )
                {
                    LastError = GetLastError();
                    sprintf(string, "Failed to create the mini-dump file '%s' (error %d)", this->m_szMiniDumpPath, LastError);
                    lpText = string;
                }
                else
                {
                    v21[0] = GetCurrentThreadId();
                    v21[1] = pExceptionInfo;
                    v21[2] = 0;
                    v22 = miniDumper::enablePrivilege(this, "SeDebugPrivilege", phToken, &ptpOld);
                    EnterCriticalSection(miniDumper::s_pCriticalSection);
                    v9 = hObject;
                    CurrentProcessId = GetCurrentProcessId();
                    CurrentProcess = GetCurrentProcess();
                    v20 = MiniDumpWriteDump(
                                    CurrentProcess,
                                    CurrentProcessId,
                                    v9,
                                    MiniDumpNormal,
                                    (PMINIDUMP_EXCEPTION_INFORMATION)v21,
                                    0,
                                    0);
                    LeaveCriticalSection(miniDumper::s_pCriticalSection);
                    if ( v22 )
                        miniDumper::restorePrivilege(this, phToken, &ptpOld);
                    if ( v20 )
                    {
                        lpText = 0;
                        v31 = 1;
                    }
                    else
                    {
                        v5 = GetLastError();
                        sprintf(string, "Failed to save the mini-dump file to '%s' (error %d)", this->m_szMiniDumpPath, v5);
                        lpText = string;
                    }
                    CloseHandle(hObject);
                }
            }
        }
        else
        {
            lpText = "Call to GetProcAddress failed to find MiniDumpWriteDump. The DBGHELP.DLL is possibly outdated.";
        }
    }
    else
    {
        lpText = "Call to LoadLibrary failed to find DBGHELP.DLL.";
    }
    if ( lpText )
    {
        if ( this->m_bPromptUserForMiniDump )
            MessageBoxA(0, lpText, 0, 0);
    }
    v7 = GetCurrentProcess();
    TerminateProcess(v7, 0);
    return v31;
#endif
}

