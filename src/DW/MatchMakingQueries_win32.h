#pragma once

#ifdef BO1_DW

#include <DemonWare/bdCore/bdContainers/bdByteBuffer.h>

struct bdSessionParams // sizeof=0x4
{                                       // XREF: MatchMaking_PC_QUICKMATCH_Query/r
                                        // MatchMaking_PC_ZOMBIE_Query/r ...
    //bdSessionParams_vtbl *__vftable;
    virtual ~bdSessionParams() = default;
    virtual unsigned int sizeOf()
    {
        return 4;
    }
};

struct MatchMaking_PC_QUICKMATCH_Query : bdSessionParams // sizeof=0x1C
{                                       // XREF: ?CL_QuickMatch_Start@@YAXHPBDHHHH@Z/r
    unsigned int m_memberPARAM_SERVERTYPE;
                                        // XREF: CL_QuickMatch_Start(int,char const *,int,int,int,int)+D1/w
    int m_memberPARAM_NETCODE_VERSION;  // XREF: CL_QuickMatch_Start(int,char const *,int,int,int,int)+D4/w
    unsigned int m_memberPARAM_MAXPLAYERS;
    unsigned int m_memberPARAM_PLAYLIST;
                                        // XREF: CL_QuickMatch_Start(int,char const *,int,int,int,int)+DE/w
    int m_memberPARAM_PLAYLIST_VERSION; // XREF: CL_QuickMatch_Start(int,char const *,int,int,int,int)+E6/w
    unsigned int m_memberPARAM_LICENSE; // XREF: CL_QuickMatch_Start(int,char const *,int,int,int,int)+E9/w

    MatchMaking_PC_QUICKMATCH_Query();
    void serialize(bdByteBuffer *buffer);
};

struct MatchMaking_PC_WAGER_Query : bdSessionParams // sizeof=0x1C
{                                       // XREF: ?CL_QuickWager_Start@@YAXXZ/r
    int m_memberPARAM_SERVERTYPE;       // XREF: CL_QuickWager_Start(void)+D7/w
    int m_memberPARAM_LICENSE;          // XREF: CL_QuickWager_Start(void)+105/w
    int m_memberPARAM_PLAYLIST_MIN;     // XREF: CL_QuickWager_Start(void)+EE/w
    int m_memberPARAM_PLAYLIST_MAX;     // XREF: CL_QuickWager_Start(void)+FA/w
    int m_memberPARAM_NETCODE_VERSION;  // XREF: CL_QuickWager_Start(void)+DE/w
    int m_memberPARAM_PLAYLIST_VERSION; // XREF: CL_QuickWager_Start(void)+102/w

    MatchMaking_PC_WAGER_Query();
    void serialize(bdByteBuffer *buffer);
};


struct MatchMaking_PC_GEO_Query : bdSessionParams // sizeof=0x20
{                                       // XREF: ?dwFindSessionsPaged@@YAPAUTaskRecord@@HH_N0@Z/r
    unsigned int m_memberPARAM_NETCODE_VERSION;
                                        // XREF: dwFindSessionsPaged(int,int,bool,bool)+68/w
    unsigned int m_memberPARAM_GAMETYPE;
                                        // XREF: dwFindSessionsPaged(int,int,bool,bool)+6B/w
    unsigned int m_memberPARAM_SERVERTYPE;
                                        // XREF: dwFindSessionsPaged(int,int,bool,bool)+75/w
    unsigned int m_memberPARAM_DDL_VERSION;
                                        // XREF: dwFindSessionsPaged(int,int,bool,bool)+7F/w
    unsigned int m_memberPARAM_RANKED;
    unsigned int m_memberPARAM_LICENSE; // XREF: dwFindSessionsPaged(int,int,bool,bool)+82/w
    unsigned int m_memberPARAM_GEO;     // XREF: dwFindSessionsPaged(int,int,bool,bool)+AE/w


    MatchMaking_PC_GEO_Query();
    void serialize(bdByteBuffer *buffer);
};

#endif