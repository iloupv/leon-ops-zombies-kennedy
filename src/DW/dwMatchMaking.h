#pragma once

#ifdef BO1_DW

#include <DemonWare/bdCore/bdReference/bdReferencable.h>

struct bdLANDiscoveryServer
{
    //bdReference<bdGameInfo> m_gameInfo;
    //bdSocket m_socket;
    //bdFastArray<bdLANDiscoveryListener *> m_listeners;
    //bdLANDiscoveryServer::bdStatus m_status;


};

struct bdLANDiscoveryClient // sizeof=0x38
{                                       // XREF: .data:discoveryClient/r
    //bdLANDiscoveryClient_vtbl *__vftable;
    //// padding byte
    //// padding byte
    //// padding byte
    //// padding byte
    //float m_timeout;
    //// padding byte
    //// padding byte
    //// padding byte
    //// padding byte
    //bdStopwatch m_timer;
    //bdSocket m_socket;
    //unsigned __int8 m_nonce[8];
    //bdFastArray<bdLANDiscoveryListener *> m_listeners;
    //bdLANDiscoveryClient::bdStatus m_status;
};

struct bdLANDiscoveryListener // sizeof=0x4
{                                       // XREF: LanDiscoveryListener/r
    //bdLANDiscoveryListener_vtbl *__vftable;
};

struct bdGameInfo : bdReferencable // sizeof=0x28
{                                       // XREF: ?sendStage1@bdNATTravClient@@IAE_NAAVbdNATTravClientData@@@Z/r
                                        // ?sendStage2@bdNATTravClient@@IAE_NAAVbdNATTravClientData@@@Z/r
    unsigned int m_titleId;
    bdSecurityID m_secID;
    bdSecurityKey m_secKey;             // XREF: bdNATTravClient::sendStage1(bdNATTravClientData &)+3C2/o
                                        // bdNATTravClient::sendStage1(bdNATTravClientData &)+3DA/o ...
    bdReference<bdCommonAddr> m_hostAddr;
                                        // XREF: bdNATTravClient::sendStage1(bdNATTravClientData &)+262/o
                                        // bdNATTravClient::sendStage1(bdNATTravClientData &)+296/o ...
};

struct LanDiscoveryListener : bdLANDiscoveryListener // sizeof=0x10
{                                       // XREF: .data:listener/r
    //bdArray<bdReference<bdGameInfo> > sessions;
                                        // XREF: _dwGetLanDiscoveryListener____2____dynamic_atexit_destructor_for__listener__+6/o

    LanDiscoveryListener();
    void onDiscovery(bdReference<bdGameInfo> info);
    void onDiscoveryFinished();
};

struct bdLANDiscoveryListener // sizeof=0x4
{                                       // XREF: LanDiscoveryListener/r
    //bdLANDiscoveryListener_vtbl *__vftable;

};


bdLANDiscoveryServer *__cdecl dwGetLanDiscoveryServer();
void __cdecl dwCreateLanSession();
void __cdecl dwUpdateLanSession();
LanDiscoveryListener *__cdecl dwGetLanDiscoveryListener();
void __cdecl dwStartLanDiscovery();
bdLANDiscoveryClient *__cdecl dwGetLanDiscoveryClient();
char __cdecl dwGetLanSession(int index, bdReference<bdCommonAddr> *addr, bdSecurityID *id, bdSecurityKey *key);
const bdSecurityKey *__cdecl dwGetSessionSecKey();
void __cdecl dwCreateSession(bdReference<bdCommonAddr> *task, MatchMakingInfo *const sessionInfo);
const bdSecurityID *__cdecl dwGetSessionSecID();
const bdSessionID *__cdecl dwGetSessionID();
void __cdecl dwSetSessionID(const bdSessionID *sessionID);
int __cdecl dwCreateSessionComplete(overlappedTask *const task);
void __cdecl dwUpdateSession(overlappedTask *const task, MatchMakingInfo *const sessionInfo);
taskCompleteResults __cdecl dwUpdateSessionComplete(overlappedTask *const task);
void __cdecl dwDeleteSession(int controllerIndex, overlappedTask *const task);
serverInfo_t *__cdecl FindServerBySecID(bdSecurityID *secid);
void __cdecl CL_SetPingFromQOS(int ping, bdSecurityID *secid);
void __cdecl dwMatchMaking_InviteDWUserSuccess();
void __cdecl dwMatchMaking_InviteDWUserFailure();
TaskRecord *__cdecl dwMatchMaking_InviteDWUserToSession(unsigned __int64 frienduid, bdSessionID sessionID);
void __cdecl dwMatchMaking_GetSessionByIDSuccess();
TaskRecord *__cdecl dwMatchMaking_FindSessionByID(bdSessionID *sessionID);
void __cdecl dwFindSessionsByEntityIDsFailure();
TaskRecord *__cdecl dwFindSessionsByEntityIDs(
                const unsigned __int64 *serverIDs,
                unsigned int numservers,
                MatchMakingInfo *serverinfos);
int __cdecl dwRegionCodeFromCountryCode(const char *countrycode);
bool __cdecl dwIsPagedFindInProgress();
void __cdecl dwFindSessionsPagedSuccess(TaskRecord *task);
void __cdecl dwFindSessionsPagedFailure();
bool __cdecl dwIsPagedFindTaskAlreadyStarted();
TaskRecord *__cdecl dwFindSessionsPaged(int netCodeVersion, int servertype, bool random, bool geo);

//bdNetImpl *__cdecl bdSingleton<bdNetImpl>::getInstance();
//bdGameInfoFactoryImpl *__cdecl bdSingleton<bdGameInfoFactoryImpl>::getInstance();
//void __thiscall bdCreatorBase<bdGameInfo>::~bdCreatorBase<bdGameInfo>(bdCreatorBase<bdGameInfo> *this);
//void __thiscall bdCreator<bdGameInfo,bdGameInfo>::~bdCreator<bdGameInfo,bdGameInfo>(
//                bdCreator<bdGameInfo,bdGameInfo> *this);
//bdGameInfo *__thiscall bdCreator<bdGameInfo,bdGameInfo>::create(bdCreator<bdGameInfo,bdGameInfo> *this);
//unsigned int __thiscall bdCreator<bdGameInfo,bdGameInfo>::getSizeOf(bdCreator<bdGameInfo,bdGameInfo> *this);
//void __thiscall bdArray<bdReference<bdGameInfo>>::clear(bdArray<bdReference<bdGameInfo> > *this);
//void __thiscall bdArray<bdReference<bdGameInfo>>::pushBack(
//                bdArray<bdReference<bdGameInfo> > *this,
//                const bdReference<bdGameInfo> *value);
//void bdSingleton<bdNetImpl>::destroyInstance();
//void bdSingleton<bdGameInfoFactoryImpl>::destroyInstance();
//void __thiscall bdArray<bdReference<bdGameInfo>>::increaseCapacity(
//                bdArray<bdReference<bdGameInfo> > *this,
//                unsigned int increase);
//void __thiscall bdArray<bdReference<bdGameInfo>>::copyConstructArrayArray(
//                bdArray<bdReference<bdGameInfo> > *this,
//                bdReference<bdGameInfo> *dest,
//                const bdReference<bdGameInfo> *src,
//                unsigned int n);
//

#endif