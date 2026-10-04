#ifdef BO1_DW
#include "dwMatchMaking.h"

bdLANDiscoveryServer *__cdecl dwGetLanDiscoveryServer()
{
#ifdef BO1_DEMON
    if ( (_S1_6 & 1) == 0 )
    {
        _S1_6 |= 1u;
        bdLANDiscoveryServer::bdLANDiscoveryServer(&lanDiscoveryServer);
        atexit(dwGetLanDiscoveryServer_::_2_::_dynamic_atexit_destructor_for__lanDiscoveryServer__);
    }
    return &lanDiscoveryServer;
#else
    return NULL;
#endif
}

void __cdecl dwCreateLanSession()
{
    bdNetImpl *Instance; // eax
    bdGameInfoFactoryImpl *v1; // eax
    bdLANDiscoveryServer *LanDiscoveryServer; // eax
    bdReference<bdCommonAddr> v3; // [esp-Ch] [ebp-B8h] BYREF
    struct bdInetAddr *m_iaddr; // [esp-8h] [ebp-B4h]
    bdReference<bdCommonAddr> v5[3]; // [esp-4h] [ebp-B0h] BYREF
    char *v6; // [esp+8h] [ebp-A4h]
    volatile int m_refCount; // [esp+Ch] [ebp-A0h]
    unsigned int *v8; // [esp+10h] [ebp-9Ch]
    bdReference<bdCommonAddr> *v9; // [esp+2Ch] [ebp-80h]
    bdCommonAddr *v10; // [esp+30h] [ebp-7Ch]
    bdGameInfo *v11; // [esp+34h] [ebp-78h]
    bdGameInfo *v12; // [esp+38h] [ebp-74h]
    bdReference<bdCommonAddr> *v13; // [esp+3Ch] [ebp-70h]
    bdCommonAddr *m_ptr; // [esp+40h] [ebp-6Ch]
    bdInetAddr v15; // [esp+78h] [ebp-34h] BYREF
    char *v16; // [esp+80h] [ebp-2Ch]
    unsigned int *v17; // [esp+84h] [ebp-28h]
    bdReference<bdCommonAddr> myAddress; // [esp+88h] [ebp-24h] BYREF
    bdSecurityID secID; // [esp+8Ch] [ebp-20h] BYREF
    bdSecurityKey secKey; // [esp+94h] [ebp-18h] BYREF
    bdReference<bdGameInfo> game; // [esp+A8h] [ebp-4h] BYREF

    Instance = bdSingleton<bdNetImpl>::getInstance();
    bdNetImpl::getLocalCommonAddr(Instance, (int)&myAddress);
    bdSecurityID::bdSecurityID(&secID);
    bdSecurityKey::bdSecurityKey(&secKey);
    dwCreateSecIDAndKey(&secID, &secKey);
    dwRegisterSecIDAndKey(&secID, &secKey);
    v17 = bdMemory::allocate(4u);
    if ( v17 )
    {
        *v17 = &bdCreatorBase<bdGameInfo>::`vftable';
        *v17 = &bdCreator<bdGameInfo,bdGameInfo>::`vftable';
        v8 = v17;
    }
    else
    {
        v8 = 0;
    }
    v1 = bdSingleton<bdGameInfoFactoryImpl>::getInstance();
    bdGameInfoFactoryImpl::setClass(v1, (int)v8);
    v16 = (char *)bdMemory::allocate(0x28u);
    if ( v16 )
    {
        v13 = v5;
        v5[0] = myAddress;
        if ( myAddress.m_ptr )
        {
            m_ptr = v13->m_ptr;
            InterlockedIncrement(&m_ptr->m_refCount);
            m_refCount = m_ptr->m_refCount;
        }
        v6 = bdGameInfo::bdGameInfo(v16, 1668244535, &secID, &secKey, v5[0]);
        v11 = (bdGameInfo *)v6;
    }
    else
    {
        v11 = 0;
    }
    game.m_ptr = v11;
    if ( v11 )
    {
        v12 = game.m_ptr;
        InterlockedIncrement(&game.m_ptr->m_refCount);
        v5[2] = (bdReference<bdCommonAddr>)v12->m_refCount;
    }
    v5[0].m_ptr = (bdCommonAddr *)50000;
    m_iaddr = (struct bdInetAddr *)bdInetAddr::Any(&v15).m_addr.inUn.m_iaddr;
    v9 = &v3;
    v3.m_ptr = (bdCommonAddr *)game.m_ptr;
    if ( game.m_ptr )
    {
        v10 = v9->m_ptr;
        InterlockedIncrement(&v10->m_refCount);
        v5[1] = (bdReference<bdCommonAddr>)v10->m_refCount;
    }
    LanDiscoveryServer = dwGetLanDiscoveryServer();
    bdLANDiscoveryServer::start(LanDiscoveryServer, v3, m_iaddr, (unsigned __int16)v5[0].m_ptr);
    bdInetAddr::~bdInetAddr(&v15);
    bdReference<bdRemoteTask>::~bdReference<bdRemoteTask>((bdReference<bdCommonAddr> *)&game);
    bdReference<bdRemoteTask>::~bdReference<bdRemoteTask>(&myAddress);
}

void __cdecl dwUpdateLanSession()
{
    bdLANDiscoveryServer *LanDiscoveryServer; // eax
    bdLANDiscoveryServer *s; // [esp+0h] [ebp-4h]

    s = dwGetLanDiscoveryServer();
    if ( bdSocketRouter::getStatus((bdSocketRouter *)s) == BD_SOCKET_ROUTER_INITIALIZED )
    {
        LanDiscoveryServer = dwGetLanDiscoveryServer();
        bdLANDiscoveryServer::update(LanDiscoveryServer);
    }
}

LanDiscoveryListener *__cdecl dwGetLanDiscoveryListener()
{
    if ( (_S3 & 1) == 0 )
    {
        _S3 |= 1u;
        LanDiscoveryListener::LanDiscoveryListener(&listener);
        atexit(dwGetLanDiscoveryListener_::_2_::_dynamic_atexit_destructor_for__listener__);
    }
    return &listener;
}

LanDiscoveryListener *__thiscall LanDiscoveryListener::LanDiscoveryListener(LanDiscoveryListener *this)
{
    bdGameInfoFactoryImpl *Instance; // eax
    int v3; // [esp+0h] [ebp-68h]
    unsigned int *v5; // [esp+64h] [ebp-4h]

    bdLANDiscoveryListener::bdLANDiscoveryListener(this);
    this->__vftable = (LanDiscoveryListener_vtbl *)&LanDiscoveryListener::`vftable';
    this->sessions.m_data = 0;
    this->sessions.m_capacity = 32;
    this->sessions.m_size = 0;
    if ( this->sessions.m_capacity )
        this->sessions.m_data = (bdReference<bdGameInfo> *)bdMemory::allocate(4 * this->sessions.m_capacity);
    v5 = bdMemory::allocate(4u);
    if ( v5 )
    {
        *v5 = &bdCreatorBase<bdGameInfo>::`vftable';
        *v5 = &bdCreator<bdGameInfo,bdGameInfo>::`vftable';
        v3 = (int)v5;
    }
    else
    {
        v3 = 0;
    }
    Instance = bdSingleton<bdGameInfoFactoryImpl>::getInstance();
    bdGameInfoFactoryImpl::setClass(Instance, v3);
    return this;
}

void __thiscall LanDiscoveryListener::onDiscovery(LanDiscoveryListener *this, bdReference<bdGameInfo> info)
{
    bdArray<bdReference<bdGameInfo>>::pushBack(&this->sessions, &info);
    bdReference<bdRemoteTask>::~bdReference<bdRemoteTask>((bdReference<bdCommonAddr> *)&info);
}

void __thiscall LanDiscoveryListener::onDiscoveryFinished(LanDiscoveryListener *this)
{
    bdAddr *LocalAddrByIndex; // eax
    bdCommonAddr *v3; // [esp+20h] [ebp-4Ch]
    bdReference<bdCommonAddr> v4; // [esp+3Ch] [ebp-30h] BYREF
    bdReference<bdGameInfo> info; // [esp+40h] [ebp-2Ch] BYREF
    char localaddr[32]; // [esp+44h] [ebp-28h] BYREF
    unsigned int i; // [esp+68h] [ebp-4h]

    for ( i = 0; i < this->sessions.m_size; ++i )
    {
        info.m_ptr = this->sessions.m_data[i].m_ptr;
        if ( info.m_ptr )
            InterlockedIncrement(&info.m_ptr->m_refCount);
        v3 = *(bdCommonAddr **)bdGameInfo::getHostAddr(info.m_ptr, (int)&v4);
        LocalAddrByIndex = (bdAddr *)bdCommonAddr::getLocalAddrByIndex(v3, 0);
        bdAddr::toString(LocalAddrByIndex, localaddr, 0x20u);
        bdReference<bdRemoteTask>::~bdReference<bdRemoteTask>(&v4);
        Com_Printf(0, "%d ip=%s\n", i, localaddr);
        bdReference<bdRemoteTask>::~bdReference<bdRemoteTask>((bdReference<bdCommonAddr> *)&info);
    }
}

void __cdecl dwStartLanDiscovery()
{
    bdLANDiscoveryClient *LanDiscoveryClient; // eax
    struct bdInetAddr v1; // eax
    bdInetAddr v2; // [esp+2Ch] [ebp-Ch] BYREF
    bdLANDiscoveryClient *discoveryClient; // [esp+30h] [ebp-8h]
    LanDiscoveryListener *listener; // [esp+34h] [ebp-4h]

    discoveryClient = dwGetLanDiscoveryClient();
    listener = dwGetLanDiscoveryListener();
    bdArray<bdReference<bdGameInfo>>::clear(&listener->sessions);
    LanDiscoveryClient = dwGetLanDiscoveryClient();
    bdLANDiscoveryClient::unregisterListener(LanDiscoveryClient, listener);
    bdLANDiscoveryClient::registerListener(discoveryClient, listener);
    v1 = bdInetAddr::Broadcast(&v2);
    bdLANDiscoveryClient::discover(
        discoveryClient,
        0x636F6437u,
        2.0,
        (const struct bdInetAddr *)v1.m_addr.inUn.m_iaddr,
        0xC350u);
    bdInetAddr::~bdInetAddr(&v2);
}

bdLANDiscoveryClient *__cdecl dwGetLanDiscoveryClient()
{
    if ( (_S2_4 & 1) == 0 )
    {
        _S2_4 |= 1u;
        bdLANDiscoveryClient::bdLANDiscoveryClient(&discoveryClient);
        atexit(dwGetLanDiscoveryClient_::_2_::_dynamic_atexit_destructor_for__discoveryClient__);
    }
    return &discoveryClient;
}

char __cdecl dwGetLanSession(int index, bdReference<bdCommonAddr> *addr, bdSecurityID *id, bdSecurityKey *key)
{
    const bdReference<bdCommonAddr> *HostAddr; // eax
    unsigned int *AddressHandle; // eax
    int v6; // edx
    bdReference<bdCommonAddr> v8; // [esp+50h] [ebp-8h] BYREF
    bdReference<bdGameInfo> session; // [esp+54h] [ebp-4h] BYREF

    if ( index >= (signed int)dwGetLanDiscoveryListener()->sessions.m_size )
        return 0;
    session.m_ptr = dwGetLanDiscoveryListener()->sessions.m_data[index].m_ptr;
    if ( session.m_ptr )
        InterlockedIncrement(&session.m_ptr->m_refCount);
    HostAddr = (const bdReference<bdCommonAddr> *)bdGameInfo::getHostAddr(session.m_ptr, (int)&v8);
    bdReference<bdCommonAddr>::operator=(addr, HostAddr);
    bdReference<bdRemoteTask>::~bdReference<bdRemoteTask>(&v8);
    AddressHandle = (unsigned int *)bdConnection::getAddressHandle(session.m_ptr);
    v6 = AddressHandle[1];
    *(unsigned int *)id->ab = *AddressHandle;
    *(unsigned int *)&id->ab[4] = v6;
    *key = *bdGameInfo::getSecurityKey(session.m_ptr);
    bdReference<bdRemoteTask>::~bdReference<bdRemoteTask>((bdReference<bdCommonAddr> *)&session);
    return 1;
}

const bdSecurityKey *__cdecl dwGetSessionSecKey()
{
    if ( (_S5 & 1) == 0 )
    {
        _S5 |= 1u;
        bdSecurityKey::bdSecurityKey(&g_secKey);
    }
    return &g_secKey;
}

void __cdecl dwCreateSession(bdReference<bdCommonAddr> *task, MatchMakingInfo *const sessionInfo)
{
    bdReference<bdCommonAddr> v2[12]; // [esp-4h] [ebp-184h] BYREF
    bdReference<bdCommonAddr> *v3; // [esp+2Ch] [ebp-154h]
    bdCommonAddr *m_ptr; // [esp+30h] [ebp-150h]
    bdSecurityKey secKey; // [esp+38h] [ebp-148h] BYREF
    bdReference<bdRemoteTask> remoteTask; // [esp+48h] [ebp-138h] BYREF
    const bdSecurityKey *secKeyStore; // [esp+4Ch] [ebp-134h]
    bool localAddressValid; // [esp+53h] [ebp-12Dh]
    bdReference<bdCommonAddr> localAddr; // [esp+54h] [ebp-12Ch] BYREF
    bdMatchMakingInfo mmInfo; // [esp+58h] [ebp-128h] BYREF
    bdMatchMaking *matchmaking; // [esp+17Ch] [ebp-4h]

    if ( (sessionInfo->m_maxPlayers > 0x20 || sessionInfo->m_numPlayers > 0x20)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\Dw\\dwMatchMaking.cpp",
                    348,
                    0,
                    "%s",
                    "sessionInfo->m_maxPlayers <= MAX_CLIENTS && sessionInfo->m_numPlayers <= MAX_CLIENTS") )
    {
        __debugbreak();
    }
    matchmaking = dwGetMatchmaking((int)task[5].m_ptr);
    bdMatchMakingInfo::bdMatchMakingInfo(&mmInfo);
    dwGetLocalCommonAddr(&localAddr);
    localAddressValid = localAddr.m_ptr != 0;
    if ( matchmaking && localAddressValid && task )
    {
        v3 = v2;
        v2[0] = localAddr;
        if ( localAddr.m_ptr )
        {
            m_ptr = v3->m_ptr;
            InterlockedIncrement(&m_ptr->m_refCount);
            v2[1] = (bdReference<bdCommonAddr>)m_ptr->m_refCount;
        }
        bdMatchMakingInfo::setHostAddr((int)sessionInfo, v2[0]);
        secKeyStore = dwGetSessionSecKey();
        bdSecurityKey::bdSecurityKey(&secKey);
        MatchMakingInfo::getSessionKey(sessionInfo, &secKey);
        *secKeyStore = secKey;
        bdMatchMaking::createSession(matchmaking, (int)&remoteTask, (int)sessionInfo, &g_sessionID);
        if ( remoteTask.m_ptr )
        {
            bdReference<bdCommonAddr>::operator=(task, (const bdReference<bdCommonAddr> *)&remoteTask);
        }
        else
        {
            TaskManager_ClearTask((overlappedTask *)task);
            dwConnectionErrorHandler(2);
        }
        bdReference<bdRemoteTask>::~bdReference<bdRemoteTask>((bdReference<bdCommonAddr> *)&remoteTask);
    }
    bdReference<bdRemoteTask>::~bdReference<bdRemoteTask>(&localAddr);
    bdMatchMakingInfo::~bdMatchMakingInfo(&mmInfo);
}

const bdSecurityID *__cdecl dwGetSessionSecID()
{
    return &g_sessionID.m_sessionID;
}

const bdSessionID *__cdecl dwGetSessionID()
{
    return &g_sessionID;
}

void __cdecl dwSetSessionID(const bdSessionID *sessionID)
{
    int v1; // edx

    v1 = *(unsigned int *)&sessionID->m_sessionID.ab[4];
    *(unsigned int *)g_sessionID.m_sessionID.ab = *(unsigned int *)sessionID->m_sessionID.ab;
    *(unsigned int *)&g_sessionID.m_sessionID.ab[4] = v1;
}

int __cdecl dwCreateSessionComplete(overlappedTask *const task)
{
    enum bdLobbyErrorCode errorCode; // [esp+10h] [ebp-24h]
    bdSecurityKey key; // [esp+18h] [ebp-1Ch] BYREF
    bdRemoteTask::bdStatus taskStatus; // [esp+2Ch] [ebp-8h]
    bdMatchMaking *matchmaking; // [esp+30h] [ebp-4h]

    matchmaking = dwGetMatchmaking(task->controllerIndex);
    if ( matchmaking && task && task->overlappedIO.m_ptr )
    {
        dwLobbyPump(task->controllerIndex);
        taskStatus = bdRemoteTask::getStatus(task->overlappedIO.m_ptr);
        if ( taskStatus == BD )
        {
            bdSecurityKey::bdSecurityKey(&key);
            if ( MatchMakingInfo::getSessionKey(g_matchmakingInfo, &key) )
                dwRegisterSecIDAndKey(&g_sessionID.m_sessionID, &key);
        }
        if ( taskStatus == BD_PENDING )
        {
            return dwTaskStatusConvert(BD_PENDING, BD_NO_ERROR);
        }
        else
        {
            errorCode = bdRemoteTask::getErrorCode(task->overlappedIO.m_ptr);
            Com_Printf(0, "SESSION CREATE ERROR %i\n", errorCode);
            return dwTaskStatusConvert(taskStatus, errorCode);
        }
    }
    else
    {
        Com_Printf(16, "Failed to retrieve session create status\n");
        return 2;
    }
}

void __cdecl dwUpdateSession(overlappedTask *const task, MatchMakingInfo *const sessionInfo)
{
    bdReference<bdCommonAddr> v2; // [esp+3Ch] [ebp-18h] BYREF
    bdReference<bdCommonAddr> result; // [esp+40h] [ebp-14h] BYREF
    bdRemoteTask *remoteTask; // [esp+44h] [ebp-10h]
    bool localAddressValid; // [esp+4Bh] [ebp-9h]
    bdReference<bdCommonAddr> localAddr; // [esp+4Ch] [ebp-8h] BYREF
    bdMatchMaking *matchmaking; // [esp+50h] [ebp-4h]

    matchmaking = dwGetMatchmaking(task->controllerIndex);
    dwGetLocalCommonAddr(&localAddr);
    localAddressValid = 1;
    localAddressValid = dwGetLocalCommonAddr(&result)->m_ptr != 0;
    bdReference<bdRemoteTask>::~bdReference<bdRemoteTask>(&result);
    if ( matchmaking && localAddressValid && task )
    {
        remoteTask = *(bdRemoteTask **)bdMatchMaking::updateSession(
                                                                         matchmaking,
                                                                         (int)&v2,
                                                                         (int)&g_sessionID,
                                                                         (int)sessionInfo);
        bdReference<bdRemoteTask>::~bdReference<bdRemoteTask>(&v2);
        if ( remoteTask )
        {
            bdReference<bdCommonAddr>::operator=(&task->overlappedIO, remoteTask);
            g_matchmakingInfo->m_dirty = 0;
        }
        else
        {
            TaskManager_ClearTask(task);
            dwConnectionErrorHandler(3);
        }
    }
    bdReference<bdRemoteTask>::~bdReference<bdRemoteTask>(&localAddr);
}

taskCompleteResults __cdecl dwUpdateSessionComplete(overlappedTask *const task)
{
    char errorStr[68]; // [esp+10h] [ebp-58h] BYREF
    bdLobbyErrorCode errorCode; // [esp+58h] [ebp-10h]
    bdRemoteTask::bdStatus taskStatus; // [esp+5Ch] [ebp-Ch]
    taskCompleteResults status; // [esp+60h] [ebp-8h]
    bdMatchMaking *matchmaking; // [esp+64h] [ebp-4h]

    status = TASK_ERROR;
    matchmaking = dwGetMatchmaking(task->controllerIndex);
    if ( matchmaking && task && task->overlappedIO.m_ptr )
    {
        dwLobbyPump(task->controllerIndex);
        if ( task->overlappedIO.m_ptr )
        {
            taskStatus = bdRemoteTask::getStatus(task->overlappedIO.m_ptr);
            errorCode = BD_NO_ERROR;
            if ( taskStatus == BD )
            {
                errorCode = bdRemoteTask::getErrorCode(task->overlappedIO.m_ptr);
                if ( errorCode )
                {
                    TaskManager_ClearTask(task);
                    dwLobbyErrorCodeToString(errorCode, errorStr, 0x40u);
                    Com_Printf(16, "DW session update error %i - '%s'\n", errorCode, errorStr);
                    if ( g_matchmakingInfo->m_active )
                        g_matchmakingInfo->m_dirty = 1;
                }
            }
            return dwTaskStatusConvert(taskStatus, errorCode);
        }
    }
    else
    {
        Com_Printf(16, "Failed to retrieve session update status\n");
    }
    return status;
}

void __cdecl dwDeleteSession(int controllerIndex, overlappedTask *const task)
{
    bdReference<bdCommonAddr> v2; // [esp+44h] [ebp-10h] BYREF
    bdRemoteTask *remoteTask; // [esp+48h] [ebp-Ch]
    bdNetImpl *net; // [esp+4Ch] [ebp-8h]
    bdMatchMaking *matchmaking; // [esp+50h] [ebp-4h]

    net = bdSingleton<bdNetImpl>::getInstance();
    matchmaking = dwGetMatchmaking(controllerIndex);
    if ( matchmaking && net && task )
    {
        remoteTask = *(bdRemoteTask **)bdMatchMaking::deleteSession(matchmaking, (int)&v2, (int)&g_sessionID);
        bdReference<bdRemoteTask>::~bdReference<bdRemoteTask>(&v2);
        if ( remoteTask )
        {
            bdReference<bdCommonAddr>::operator=(&task->overlappedIO, remoteTask);
        }
        else
        {
            TaskManager_ClearTask(task);
            dwConnectionErrorHandler(4);
        }
    }
}

serverInfo_t *__cdecl FindServerBySecID(bdSecurityID *secid)
{
    int i; // [esp+10h] [ebp-Ch]
    int *count; // [esp+14h] [ebp-8h] BYREF
    serverInfo_t *servers; // [esp+18h] [ebp-4h] BYREF

    servers = 0;
    if ( CL_GetServerList(cls.pingUpdateSource, &servers, &count) )
    {
        for ( i = 0; i < *count; ++i )
        {
            if ( !memcmp((const char *)secid, (const char *)&servers[i].xnkid, 8) )
                return &servers[i];
        }
    }
    return 0;
}

void __cdecl CL_SetPingFromQOS(int ping, bdSecurityID *secid)
{
    serverInfo_t *server; // [esp+0h] [ebp-4h]

    server = FindServerBySecID(secid);
    if ( server )
    {
        Com_DPrintf(14, "Setting server %s ping to %ims\n", server->hostName, ping);
        server->ping = ping;
    }
}

void __cdecl dwMatchMaking_InviteDWUserSuccess()
{
    Com_Printf(23, "Invite success\n");
}

void __cdecl dwMatchMaking_InviteDWUserFailure()
{
    Com_Printf(23, "Invite failure\n");
}

TaskRecord *__cdecl dwMatchMaking_InviteDWUserToSession(unsigned __int64 frienduid, bdSessionID sessionID)
{
    const bdReference<bdCommonAddr> *v2; // eax
    int v4; // [esp+2Ch] [ebp-54h]
    signed int v5; // [esp+30h] [ebp-50h]
    TaskRecord *v6; // [esp+34h] [ebp-4Ch]
    bdReference<bdCommonAddr> v7; // [esp+38h] [ebp-48h] BYREF
    const char *pass; // [esp+3Ch] [ebp-44h]
    unsigned __int64 userIDs[1]; // [esp+40h] [ebp-40h] BYREF
    TaskRecord *v10; // [esp+48h] [ebp-38h]
    int protocolversion; // [esp+4Ch] [ebp-34h]
    unsigned __int8 payload[36]; // [esp+50h] [ebp-30h] BYREF
    TaskRecord *task; // [esp+74h] [ebp-Ch]
    bdMatchMaking *matchmaking; // [esp+78h] [ebp-8h]

    matchmaking = dwGetMatchmaking(0);
    task = 0;
    if ( matchmaking )
    {
        v10 = TaskManager2_CreateTask(task_inviteDWUserToSession, 0, 0, 0);
        userIDs[0] = frienduid;
        protocolversion = 1044;
        memset(&payload[4], 0, 32);
        *(unsigned int *)payload = 1044;
        if ( CL_GetLocalClientConnectionState(0) == 10 )
        {
            if ( com_sv_running && com_sv_running->current.enabled )
                pass = Dvar_GetString("g_password");
            else
                pass = Dvar_GetString("password");
            v4 = strlen(pass);
            if ( v4 < 31 )
                v5 = v4;
            else
                v5 = 31;
            if ( v5 > 0 )
                memcpy(&payload[4], (unsigned __int8 *)pass, v5);
        }
        v2 = (const bdReference<bdCommonAddr> *)bdMatchMaking::inviteToSession(
                                                                                            matchmaking,
                                                                                            (int)&v7,
                                                                                            (int)&sessionID,
                                                                                            (int)userIDs,
                                                                                            1u,
                                                                                            payload,
                                                                                            0x24u);
        bdReference<bdCommonAddr>::operator=((bdReference<bdCommonAddr> *)&v10->remoteTask, v2);
        bdReference<bdRemoteTask>::~bdReference<bdRemoteTask>(&v7);
        TaskManager2_StartTask(v10);
    }
    v6 = task;
    bdTaskResult::~bdTaskResult(&sessionID);
    return v6;
}

void __cdecl dwMatchMaking_GetSessionByIDSuccess()
{
    Com_DPrintf(23, "GetSessionByID success!\n");
}

TaskRecord *__cdecl dwMatchMaking_FindSessionByID(bdSessionID *sessionID)
{
    const bdReference<bdCommonAddr> *SessionFromID; // eax
    MatchMakingInfo *matched; // [esp+0h] [ebp-34h]
    bdReference<bdCommonAddr> v4; // [esp+20h] [ebp-14h] BYREF
    MatchMakingInfo *payload; // [esp+24h] [ebp-10h]
    MatchMakingInfo *mminfo; // [esp+28h] [ebp-Ch]
    TaskRecord *task; // [esp+2Ch] [ebp-8h]
    bdMatchMaking *matchmaking; // [esp+30h] [ebp-4h]

    if ( !sessionID )
        return 0;
    matchmaking = dwGetMatchmaking(0);
    if ( !matchmaking )
        return 0;
    task = TaskManager2_CreateTask(task_getSessionByID, 0, 0, 0);
    if ( task )
    {
        payload = (MatchMakingInfo *)task->payload;
        if ( payload )
            matched = MatchMakingInfo::MatchMakingInfo(payload);
        else
            matched = 0;
        mminfo = matched;
        SessionFromID = (const bdReference<bdCommonAddr> *)bdMatchMaking::findSessionFromID(
                                                                                                                 matchmaking,
                                                                                                                 (int)&v4,
                                                                                                                 (int)sessionID,
                                                                                                                 matched);
        bdReference<bdCommonAddr>::operator=((bdReference<bdCommonAddr> *)&task->remoteTask, SessionFromID);
        bdReference<bdRemoteTask>::~bdReference<bdRemoteTask>(&v4);
        TaskManager2_StartTask(task);
    }
    return task;
}

void __cdecl dwFindSessionsByEntityIDsFailure()
{
    Com_PrintWarning(14, "Find by entityID failure\n");
}

TaskRecord *__cdecl dwFindSessionsByEntityIDs(
                const unsigned __int64 *serverIDs,
                unsigned int numservers,
                MatchMakingInfo *serverinfos)
{
    const bdReference<bdCommonAddr> *SessionsByEntityIDs; // eax
    bdReference<bdCommonAddr> v5; // [esp+1Ch] [ebp-Ch] BYREF
    TaskRecord *task; // [esp+20h] [ebp-8h]
    bdMatchMaking *matchmaking; // [esp+24h] [ebp-4h]

    task = 0;
    matchmaking = dwGetMatchmaking(0);
    if ( matchmaking )
    {
        task = TaskManager2_CreateTask(task_FindByEntityIDs, 0, 0, 0);
        SessionsByEntityIDs = (const bdReference<bdCommonAddr> *)bdMatchMaking::findSessionsByEntityIDs(
                                                                                                                             matchmaking,
                                                                                                                             (int)&v5,
                                                                                                                             (int)serverIDs,
                                                                                                                             numservers,
                                                                                                                             serverinfos);
        bdReference<bdCommonAddr>::operator=((bdReference<bdCommonAddr> *)&task->remoteTask, SessionsByEntityIDs);
        bdReference<bdRemoteTask>::~bdReference<bdRemoteTask>(&v5);
        TaskManager2_StartTask(task);
    }
    return task;
}

int __cdecl dwRegionCodeFromCountryCode(const char *countrycode)
{
    int i; // [esp+0h] [ebp-8h]
    int retval; // [esp+4h] [ebp-4h]

    retval = -1;
    for ( i = 0; i < 13; ++i )
    {
        if ( !I_stricmp(countries[i], countrycode) )
            return i + 1;
    }
    return retval;
}

bool __cdecl dwIsPagedFindInProgress()
{
    return s_pagingToken != 0;
}

void __cdecl dwFindSessionsPagedSuccess(TaskRecord *task)
{
    int HeaderSize; // eax
    int v2; // eax
    int v3; // eax
    bool geo; // [esp-4h] [ebp-44h]
    bool v6; // [esp-4h] [ebp-44h]
    bdPagingToken *v7; // [esp+0h] [ebp-40h]
    bdPagingToken *v8; // [esp+24h] [ebp-1Ch]
    struct bdRemoteTaskManager *tokenStatus; // [esp+38h] [ebp-8h]
    findsessionparams_t *params; // [esp+3Ch] [ebp-4h]

    tokenStatus = bdLobbyService::getTaskManager((bdLobbyService *)s_pagingToken);
    params = (findsessionparams_t *)task->payload;
    if ( tokenStatus == (struct bdRemoteTaskManager *)1 )
    {
        if ( params->random )
        {
            Com_Printf(14, "One-shot query, pagingtoken is finished\n");
            if ( bdTaskByteBuffer::getHeaderSize((bdTaskByteBuffer *)task->remoteTask.m_ptr) )
            {
                HeaderSize = bdTaskByteBuffer::getHeaderSize((bdTaskByteBuffer *)task->remoteTask.m_ptr);
                CL_ServersResponsePacket((MatchMakingInfo *)&g_sessionResults, HeaderSize, 1);
            }
            if ( s_pagingToken )
                ((void (__thiscall *)(bdPagingToken *, int))s_pagingToken->~bdPagingToken)(s_pagingToken, 1);
            s_pagingToken = 0;
        }
        else
        {
            Com_DPrintf(14, "PagedSuccess, calling again, gametype %i\n", params->servertype);
            geo = params->geo;
            v2 = bdTaskByteBuffer::getHeaderSize((bdTaskByteBuffer *)task->remoteTask.m_ptr);
            CL_ServersResponsePacket((MatchMakingInfo *)&g_sessionResults, v2, geo);
            dwFindSessionsPaged(1044, params->servertype, 0, params->geo);
        }
    }
    else if ( tokenStatus == (struct bdRemoteTaskManager *)2 )
    {
        Com_Printf(14, "Pagingtoken is finished\n");
        if ( bdTaskByteBuffer::getHeaderSize((bdTaskByteBuffer *)task->remoteTask.m_ptr) )
        {
            v6 = params->geo;
            v3 = bdTaskByteBuffer::getHeaderSize((bdTaskByteBuffer *)task->remoteTask.m_ptr);
            CL_ServersResponsePacket((MatchMakingInfo *)&g_sessionResults, v3, v6);
        }
        if ( s_pagingToken )
            ((void (__thiscall *)(bdPagingToken *, int))s_pagingToken->~bdPagingToken)(s_pagingToken, 1);
        s_pagingToken = 0;
        if ( params->geo && !params->random )
        {
            v8 = (bdPagingToken *)bdMemory::allocate(0x14u);
            if ( v8 )
                v7 = bdPagingToken::bdPagingToken(v8, 0x12Cu);
            else
                v7 = 0;
            s_pagingToken = v7;
            dwFindSessionsPaged(1044, params->servertype, 0, 0);
        }
    }
    else if ( !Assert_MyHandler(
                             "C:\\projects_pc\\cod\\codsrc\\src\\Dw\\dwMatchMaking.cpp",
                             1263,
                             0,
                             "shouldn't be reaching here in this callback!\n") )
    {
        __debugbreak();
    }
}

void __cdecl dwFindSessionsPagedFailure()
{
    Com_PrintWarning(0, "Findsessionspaged task failed!\n");
    if ( s_pagingToken )
    {
        ((void (__thiscall *)(bdPagingToken *, int))s_pagingToken->~bdPagingToken)(s_pagingToken, 1);
        s_pagingToken = 0;
    }
}

bool __cdecl dwIsPagedFindTaskAlreadyStarted()
{
    return TaskManager2_TaskIsInProgress(task_findSessionsPaged);
}

TaskRecord *__cdecl dwFindSessionsPaged(int netCodeVersion, int servertype, bool random, bool geo)
{
    const char *CountryCode; // eax
    int v6; // [esp+0h] [ebp-64h]
    bdPagingToken *v7; // [esp+4h] [ebp-60h]
    unsigned int v8; // [esp+8h] [ebp-5Ch]
    TaskRecord *v9; // [esp+2Ch] [ebp-38h]
    bdReference<bdCommonAddr> v10; // [esp+30h] [ebp-34h] BYREF
    bdPagingToken *v11; // [esp+34h] [ebp-30h]
    MatchMaking_PC_GEO_Query query; // [esp+38h] [ebp-2Ch] BYREF
    TaskRecord *task; // [esp+58h] [ebp-Ch]
    bdRemoteTask *remoteTask; // [esp+5Ch] [ebp-8h]
    bdMatchMaking *matchmaking; // [esp+60h] [ebp-4h]

    if ( TaskManager2_TaskIsInProgress(task_findSessionsPaged) )
        return 0;
    s_findsessionparams.servertype = servertype;
    s_findsessionparams.random = random;
    s_findsessionparams.geo = geo;
    task = 0;
    matchmaking = dwGetMatchmaking(0);
    if ( !matchmaking )
        return task;
    MatchMaking_PC_GEO_Query::MatchMaking_PC_GEO_Query(&query);
    query.m_memberPARAM_NETCODE_VERSION = netCodeVersion;
    query.m_memberPARAM_GAMETYPE = -1;
    query.m_memberPARAM_SERVERTYPE = servertype;
    query.m_memberPARAM_DDL_VERSION = LiveStats_GetStatsDDL()->version;
    query.m_memberPARAM_LICENSE = -1;
    if ( geo )
    {
        CountryCode = LiveSteam_GetCountryCode();
        v8 = dwRegionCodeFromCountryCode(CountryCode);
    }
    else
    {
        v8 = -1;
    }
    query.m_memberPARAM_GEO = v8;
    if ( !s_pagingToken )
    {
        page = 0;
        v11 = (bdPagingToken *)bdMemory::allocate(0x14u);
        v7 = v11 ? bdPagingToken::bdPagingToken(v11, 0x12Cu) : 0;
        s_pagingToken = v7;
        if ( random )
            bdPagingToken::setRandomSessionID(s_pagingToken);
    }
    v6 = page++;
    Com_Printf(0, "finding sessions with netcodeversion %i, type %i, page %i\n", netCodeVersion, servertype, v6);
    remoteTask = *(bdRemoteTask **)bdMatchMaking::findSessionsPaged(
                                                                     matchmaking,
                                                                     (int)&v10,
                                                                     6u,
                                                                     (int)&query,
                                                                     (bdLobbyService *)s_pagingToken,
                                                                     &g_sessionResults);
    bdReference<bdRemoteTask>::~bdReference<bdRemoteTask>(&v10);
    if ( remoteTask )
    {
        task = TaskManager2_CreateTask(task_findSessionsPaged, 0, 0, 0);
        task->payload = &s_findsessionparams;
        bdReference<bdCommonAddr>::operator=(&task->remoteTask, remoteTask);
        TaskManager2_StartTask(task);
    }
    else
    {
        dwConnectionErrorHandler(5);
    }
    v9 = task;
    bdSessionParams::~bdSessionParams(&query);
    return v9;
}

//bdNetImpl *__cdecl bdSingleton<bdNetImpl>::getInstance()
//{
//    bool m_cleaningUp; // edx
//    bdNetImpl *v2; // [esp+4h] [ebp-6Ch]
//    void (__cdecl *value)(); // [esp+8h] [ebp-68h] BYREF
//    bdSingletonRegistryImpl *Instance; // [esp+Ch] [ebp-64h]
//    bool v5; // [esp+2Bh] [ebp-45h]
//    bdNetImpl *v6; // [esp+64h] [ebp-Ch]
//    bdNetImpl *v7; // [esp+68h] [ebp-8h]
//    bdNetImpl *v8; // [esp+6Ch] [ebp-4h]
//
//    if ( !bdSingleton<bdNetImpl>::m_instance )
//    {
//        v8 = (bdNetImpl *)bdMemory::allocate(0x4760u);
//        if ( v8 )
//            v2 = bdNetImpl::bdNetImpl(v8);
//        else
//            v2 = 0;
//        bdSingleton<bdNetImpl>::m_instance = v2;
//        if ( v2 )
//        {
//            value = bdSingleton<bdNetImpl>::destroyInstance;
//            Instance = bdSingleton<bdSingletonRegistryImpl>::getInstance();
//            m_cleaningUp = Instance->m_cleaningUp;
//            v5 = !m_cleaningUp;
//            if ( !m_cleaningUp )
//                bdFastArray<void (__cdecl *)(void)>::pushBack(&Instance->m_destroyFunctions, &value);
//            if ( !v5 )
//            {
//                v6 = bdSingleton<bdNetImpl>::m_instance;
//                v7 = bdSingleton<bdNetImpl>::m_instance;
//                if ( bdSingleton<bdNetImpl>::m_instance )
//                    ((void (__thiscall *)(bdNetImpl *, int))v7->~bdNetImpl)(v7, 1);
//                bdSingleton<bdNetImpl>::m_instance = 0;
//                DebugBreak();
//            }
//        }
//        else
//        {
//            DebugBreak();
//        }
//    }
//    return bdSingleton<bdNetImpl>::m_instance;
//}
//
//bdGameInfoFactoryImpl *__cdecl bdSingleton<bdGameInfoFactoryImpl>::getInstance()
//{
//    bdGameInfoFactoryImpl *v0; // eax
//    bool m_cleaningUp; // edx
//    bdGameInfoFactoryImpl *v3; // [esp+4h] [ebp-64h]
//    void (__cdecl *value)(); // [esp+8h] [ebp-60h] BYREF
//    bdSingletonRegistryImpl *Instance; // [esp+Ch] [ebp-5Ch]
//    bool v6; // [esp+2Bh] [ebp-3Dh]
//    bdGameInfoFactoryImpl *v7; // [esp+5Ch] [ebp-Ch]
//    bdGameInfoFactoryImpl *v8; // [esp+60h] [ebp-8h]
//    void *v9; // [esp+64h] [ebp-4h]
//
//    if ( !bdSingleton<bdGameInfoFactoryImpl>::m_instance )
//    {
//        v9 = bdMemory::allocate(4u);
//        if ( v9 )
//        {
//            bdGameInfoFactoryImpl::bdGameInfoFactoryImpl(v9);
//            v3 = v0;
//        }
//        else
//        {
//            v3 = 0;
//        }
//        bdSingleton<bdGameInfoFactoryImpl>::m_instance = v3;
//        if ( v3 )
//        {
//            value = bdSingleton<bdGameInfoFactoryImpl>::destroyInstance;
//            Instance = bdSingleton<bdSingletonRegistryImpl>::getInstance();
//            m_cleaningUp = Instance->m_cleaningUp;
//            v6 = !m_cleaningUp;
//            if ( !m_cleaningUp )
//                bdFastArray<void (__cdecl *)(void)>::pushBack(&Instance->m_destroyFunctions, &value);
//            if ( !v6 )
//            {
//                v7 = bdSingleton<bdGameInfoFactoryImpl>::m_instance;
//                v8 = bdSingleton<bdGameInfoFactoryImpl>::m_instance;
//                if ( bdSingleton<bdGameInfoFactoryImpl>::m_instance )
//                {
//                    bdGameInfoFactoryImpl::~bdGameInfoFactoryImpl(v8);
//                    bdMemory::deallocate(v8);
//                }
//                bdSingleton<bdGameInfoFactoryImpl>::m_instance = 0;
//                DebugBreak();
//            }
//        }
//        else
//        {
//            DebugBreak();
//        }
//    }
//    return bdSingleton<bdGameInfoFactoryImpl>::m_instance;
//}
//
//void __thiscall bdCreatorBase<bdGameInfo>::~bdCreatorBase<bdGameInfo>(bdCreatorBase<bdGameInfo> *this)
//{
//    this->__vftable = (bdCreatorBase<bdGameInfo>_vtbl *)&bdCreatorBase<bdGameInfo>::`vftable';
//}
//
//void __thiscall bdCreator<bdGameInfo,bdGameInfo>::~bdCreator<bdGameInfo,bdGameInfo>(
//                bdCreator<bdGameInfo,bdGameInfo> *this)
//{
//    this->__vftable = (bdCreator<bdGameInfo,bdGameInfo>_vtbl *)&bdCreator<bdGameInfo,bdGameInfo>::`vftable';
//    bdCreatorBase<bdGameInfo>::~bdCreatorBase<bdGameInfo>(this);
//}
//
//bdGameInfo *__thiscall bdCreator<bdGameInfo,bdGameInfo>::create(bdCreator<bdGameInfo,bdGameInfo> *this)
//{
//    bdGameInfo *v3; // [esp+8h] [ebp-4h]
//
//    v3 = (bdGameInfo *)bdMemory::allocate(0x28u);
//    if ( v3 )
//        return bdGameInfo::bdGameInfo(v3);
//    else
//        return 0;
//}
//
//unsigned int __thiscall bdCreator<bdGameInfo,bdGameInfo>::getSizeOf(bdCreator<bdGameInfo,bdGameInfo> *this)
//{
//    return 40;
//}
//
//void __thiscall bdArray<bdReference<bdGameInfo>>::clear(bdArray<bdReference<bdGameInfo> > *this)
//{
//    unsigned int m_size; // [esp+8h] [ebp-1Ch]
//    bdReference<bdGameInfo> *m_data; // [esp+Ch] [ebp-18h]
//    unsigned int i; // [esp+20h] [ebp-4h]
//
//    m_size = this->m_size;
//    m_data = this->m_data;
//    for ( i = 0; i < m_size; ++i )
//        bdReference<bdRemoteTask>::~bdReference<bdRemoteTask>((bdReference<bdCommonAddr> *)&m_data[i]);
//    bdMemory::deallocate(this->m_data);
//    this->m_data = 0;
//    this->m_size = 0;
//    this->m_capacity = 0;
//}
//
//void __thiscall bdArray<bdReference<bdGameInfo>>::pushBack(
//                bdArray<bdReference<bdGameInfo> > *this,
//                const bdReference<bdGameInfo> *value)
//{
//    bdReference<bdGameInfo> *v3; // [esp+14h] [ebp-40h]
//
//    if ( this->m_size == this->m_capacity )
//        bdArray<bdReference<bdGameInfo>>::increaseCapacity(this, 1u);
//    v3 = &this->m_data[this->m_size];
//    if ( v3 )
//    {
//        v3->m_ptr = value->m_ptr;
//        if ( v3->m_ptr )
//            InterlockedIncrement(&v3->m_ptr->m_refCount);
//    }
//    ++this->m_size;
//}
//
//void bdSingleton<bdNetImpl>::destroyInstance()
//{
//    if ( bdSingleton<bdNetImpl>::m_instance )
//    {
//        ((void (__thiscall *)(bdNetImpl *, int))bdSingleton<bdNetImpl>::m_instance->~bdNetImpl)(
//            bdSingleton<bdNetImpl>::m_instance,
//            1);
//        bdSingleton<bdNetImpl>::m_instance = 0;
//    }
//}
//
//void bdSingleton<bdGameInfoFactoryImpl>::destroyInstance()
//{
//    bdGameInfoFactoryImpl *v0; // [esp+8h] [ebp-4h]
//
//    if ( bdSingleton<bdGameInfoFactoryImpl>::m_instance )
//    {
//        v0 = bdSingleton<bdGameInfoFactoryImpl>::m_instance;
//        bdGameInfoFactoryImpl::~bdGameInfoFactoryImpl(bdSingleton<bdGameInfoFactoryImpl>::m_instance);
//        bdMemory::deallocate(v0);
//        bdSingleton<bdGameInfoFactoryImpl>::m_instance = 0;
//    }
//}
//
//void __thiscall bdArray<bdReference<bdGameInfo>>::increaseCapacity(
//                bdArray<bdReference<bdGameInfo> > *this,
//                unsigned int increase)
//{
//    unsigned int m_capacity; // [esp+0h] [ebp-44h]
//    unsigned int m_size; // [esp+Ch] [ebp-38h]
//    bdReference<bdGameInfo> *m_data; // [esp+10h] [ebp-34h]
//    unsigned int i; // [esp+24h] [ebp-20h]
//    bdReference<bdGameInfo> *newData; // [esp+3Ch] [ebp-8h]
//    unsigned int newCapacity; // [esp+40h] [ebp-4h]
//
//    if ( increase <= this->m_capacity )
//        m_capacity = this->m_capacity;
//    else
//        m_capacity = increase;
//    newCapacity = m_capacity + this->m_capacity;
//    newData = 0;
//    if ( newCapacity )
//    {
//        newData = (bdReference<bdGameInfo> *)bdMemory::allocate(4 * newCapacity);
//        bdArray<bdReference<bdGameInfo>>::copyConstructArrayArray(this, newData, this->m_data, this->m_size);
//    }
//    m_size = this->m_size;
//    m_data = this->m_data;
//    for ( i = 0; i < m_size; ++i )
//        bdReference<bdRemoteTask>::~bdReference<bdRemoteTask>((bdReference<bdCommonAddr> *)&m_data[i]);
//    bdMemory::deallocate(this->m_data);
//    this->m_data = newData;
//    this->m_capacity = newCapacity;
//}
//
//void __thiscall bdArray<bdReference<bdGameInfo>>::copyConstructArrayArray(
//                bdArray<bdReference<bdGameInfo> > *this,
//                bdReference<bdGameInfo> *dest,
//                const bdReference<bdGameInfo> *src,
//                unsigned int n)
//{
//    bdReference<bdGameInfo> *v4; // [esp+10h] [ebp-10h]
//    unsigned int i; // [esp+1Ch] [ebp-4h]
//
//    for ( i = 0; i < n; ++i )
//    {
//        v4 = &dest[i];
//        if ( v4 )
//        {
//            v4->m_ptr = (bdGameInfo *)src[i];
//            if ( v4->m_ptr )
//                InterlockedIncrement(&v4->m_ptr->m_refCount);
//        }
//    }
//}
//
//
#endif