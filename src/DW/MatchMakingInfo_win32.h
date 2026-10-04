#pragma once

#ifdef BO1_DW


#include <demo/demo_common.h>

struct bdMatchMakingInfo : bdTaskResult // sizeof=0x11C
{                                       // XREF: MatchMakingInfo/r
                                        // ?dwCreateSession@@YAXQAUoverlappedTask@@QAVMatchMakingInfo@@@Z/r
    bdSecurityID m_sessionID;           // XREF: Live_JoinSessionInProgressComplete(TaskRecord *)+47/o
                                        // Live_JoinSessionInProgressComplete(TaskRecord *)+F9/r ...
    unsigned __int8 m_hostAddr[255];    // XREF: Live_JoinSessionInProgressComplete(TaskRecord *)+10F/o
    // padding byte
    unsigned int m_hostAddrSize;
    unsigned int m_gameType;
    unsigned int m_maxPlayers;
    unsigned int m_numPlayers;
};

struct MatchMakingInfo : bdMatchMakingInfo // sizeof=0x228
{                                       // XREF: .data:MatchMakingInfo * g_sessionResults/r
                                        // .data:s_friendServersInfo/r ...
    int m_memberNETCODE_VERSION;
    unsigned __int8 m_membersecKey[17]; // XREF: Live_JoinSessionInProgressComplete(TaskRecord *)+CB/r
                                        // Live_JoinSessionInProgressComplete(TaskRecord *)+D7/r ...
    char m_memberservername[65];        // XREF: Live_JoinSessionInProgressComplete(TaskRecord *)+126/o
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    unsigned __int64 m_membermaprotation;
    char m_membermapname[33];
    // padding byte
    // padding byte
    // padding byte
    int m_memberGAME_TYPE;
    int m_memberPLAYLIST;               // XREF: Live_JoinSessionInProgressComplete(TaskRecord *)+160/r
    int m_memberPLAYLIST_VERSION;       // XREF: Live_JoinSessionInProgressComplete(TaskRecord *)+167/r
    int m_memberPARTY_STATE;
    int m_memberMAPPACKS;
    int m_memberGAME_MODE;
    int m_memberSKILL;
    int m_memberTEAM_SIZE_MIN;
    int m_memberTEAM_SIZE_MAX;
    int m_memberGEOGRAPHICAL_LOCATION;
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    unsigned __int64 m_memberdemonwareID;
    int m_memberserverType;             // XREF: Live_JoinSessionInProgressComplete(TaskRecord *)+141/r
                                        // Live_JoinSessionInProgressComplete(TaskRecord *)+14E/r
    int m_memberroundTime;
    char m_membermod[33];
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    unsigned __int64 m_memberaddressport;
    unsigned int m_memberDDL_VERSION;
    unsigned int m_memberLICENSE;
    unsigned int m_pingedtime;
    int m_dirty;
    int m_active;
    int m_membertimesincelastupdate;

    MatchMakingInfo();
    void serialize(bdByteBuffer *buffer);
    bool deserialize(bdReference<bdByteBuffer> buffer);
    unsigned int sizeOf();
    char setSessionKey(const bdSecurityKey *secKey);
    char getSessionKey(bdSecurityKey *secKey);
    bool doUpdate(
        int msec,
        unsigned int updateIntervalMsec,
        unsigned int forceUpdateIntervalMsec);
};



#endif