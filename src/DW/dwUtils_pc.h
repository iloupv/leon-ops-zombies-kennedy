#pragma once

#ifdef BO1_DW

#include <Windows.h>
#include <DemonWare/bdCore/bdSingleton.h>
#include <universal/dvar.h>

void __cdecl bdGetRandomUChar8(BYTE *pbBuffer, DWORD dwLen);

// garbage ass class that exists to wrap a 1-liner WinAPI. "Truly" just the worst part of this codebase is all the demonware crap.
// If I had a slur for Irish people off the top of my head, it would go in here. 
struct bdTrulyRandomImpl : bdSingleton<bdTrulyRandomImpl>// sizeof=0x0
{
    void getRandomUByte8(BYTE *pbBuffer, DWORD dwLen)
    {
        bdGetRandomUChar8(pbBuffer, dwLen);
    }

    unsigned int getRandomUInt()
    {
        int pbBuffer; // [esp+8h] [ebp-8h] BYREF

        bdTrulyRandomImpl::getRandomUByte8((BYTE *)&pbBuffer, 4u);
        return pbBuffer;
    }
};

struct netadr_t;

char __cdecl dwGetOnlineUserID(int controllerIndex, unsigned __int64 *userID);
char __cdecl dwGetOnlineUserName(int controllerIndex, char *buf, unsigned int bufsize);
void __cdecl dwXnaddrtonetadr(unsigned __int8 *xnaddr, netadr_t *privadr, netadr_t *pubadr);
unsigned int __cdecl SV_SendClientChallenge(unsigned int nonce, unsigned int challenge, unsigned __int64 userID);
void __cdecl CL_HandleSVDWChallenge(unsigned int nonce, unsigned int serverchallenge, unsigned int clientchallenge);
unsigned int __cdecl Com_GametypeToInt(const char *gametype);
void __cdecl Com_IntToGametype(char *gametype_out, unsigned int gametype_in);

extern unsigned __int64 g_bdUserID;

extern const dvar_t *cl_senddwchallenge;

#endif