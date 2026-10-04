#pragma once

enum dlStatus_t : __int32
{                                       // XREF: ?CL_WWWDownload@@YAXXZ/r
    DL_CONTINUE = 0x0,
    DL_DONE     = 0x1,
    DL_FAILED   = 0x2,
};

// HACK
typedef void *_HTRequest;
typedef int _HTAlertOpcode;
// HACK END

int __cdecl DL_VPrintf(const char *fmt, char *argptr);
int __cdecl terminate_handler_WINDOWSNAMINGCLASH(_HTRequest *request, struct _HTResponse *response, void *param, int status);
int __cdecl HTAlertCallback_progress(_HTRequest *request, _HTAlertOpcode op);
int __cdecl HTAlertCallback_confirm(_HTRequest *request, _HTAlertOpcode op, int msgnum);
int __cdecl HTAlertCallback_prompt(_HTRequest *request, _HTAlertOpcode op, int msgnum);
void __cdecl DL_CancelDownload();
void __cdecl DL_InitDownload();
int __cdecl DL_BeginDownload(char *localName, char *remoteName);
int __cdecl DL_DownloadLoop();
bool __cdecl DL_InProgress();
bool __cdecl DL_DLIsMotd();
