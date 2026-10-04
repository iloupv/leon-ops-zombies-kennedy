#include "profanityfilter.h"

bool __cdecl isBadWord(int controllerIndex, char *testString)
{
    return false; // fuck off guys, it's an M-rated game, suck my balls.
#if 0
    char *v2; // eax
    char *v4; // eax
    char *v5; // eax
    char *v6; // eax
    bdReference<bdCommonAddr> v7; // [esp+24h] [ebp-2Ch] BYREF
    int maxTaskWaits; // [esp+28h] [ebp-28h]
    bdRemoteTask::bdStatus remoteTaskStatus; // [esp+2Ch] [ebp-24h]
    bdVerifyString verified; // [esp+30h] [ebp-20h] BYREF
    bdRemoteTask *remoteTask; // [esp+38h] [ebp-18h]
    int testStringLength; // [esp+3Ch] [ebp-14h]
    unsigned __int64 userID; // [esp+40h] [ebp-10h] BYREF
    bdTitleUtilities *titleUtil; // [esp+4Ch] [ebp-4h]

    titleUtil = dwGetTitleUtilities(controllerIndex);
    if ( titleUtil && dwGetOnlineUserID(controllerIndex, &userID) )
    {
        testStringLength = strlen(testString);
        bdVerifyString::bdVerifyString(&verified);
        remoteTask = *(bdRemoteTask **)bdTitleUtilities::verifyString(
                                                                         titleUtil,
                                                                         (int)&v7,
                                                                         (unsigned __int8 *)testString,
                                                                         testStringLength + 1,
                                                                         &verified);
        bdReference<bdRemoteTask>::~bdReference<bdRemoteTask>(&v7);
        if ( remoteTask )
        {
            remoteTaskStatus = bdRemoteTask::getStatus(remoteTask);
            maxTaskWaits = 200;
            while ( remoteTaskStatus == BD_PENDING )
            {
                remoteTaskStatus = bdRemoteTask::getStatus(remoteTask);
                dwNetPump();
                dwLobbyPump(controllerIndex);
                NET_Sleep(0xAu);
                if ( !--maxTaskWaits )
                {
                    v2 = va("Timeout whilst checking string %s\n", testString);
                    Com_Printf(16, v2);
                    bdTaskResult::~bdTaskResult(&verified);
                    return 0;
                }
            }
            if ( remoteTaskStatus == BD )
            {
                if ( !verified.m_verified )
                {
                    v4 = va("Profanity Detected %s\n", testString);
                    Com_Printf(16, v4);
                    bdTaskResult::~bdTaskResult(&verified);
                    return 1;
                }
                v5 = va("String verified by DW '%s'\n", testString);
                Com_Printf(16, v5);
            }
        }
        else
        {
            v6 = va("Error Occurred whilst checking string %s\n", testString);
            Com_Printf(16, v6);
        }
        bdTaskResult::~bdTaskResult(&verified);
    }
    return 0;
#endif
}

