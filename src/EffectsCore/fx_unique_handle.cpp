#include "fx_unique_handle.h"
#include "fx_system.h"
#include "fx_archive.h"
#include <win32/win_common.h>

FxEffectContainer *__cdecl UniqueHandleToEffect(int localClientNum, unsigned int h)
{
    FxSystemContainer *s; // [esp+0h] [ebp-4h]

    s = FX_GetSystem(localClientNum);
    return UniqueHandleToEffect(&s->system, h);
}

FxEffectContainer *__cdecl UniqueHandleToEffect(FxSystem *system, unsigned int handle)
{
    FxUniqueHandleDb *uniqueHandleDb; // [esp+10h] [ebp-1Ch]
    FxUniqueHandleDb::Node *localNode; // [esp+24h] [ebp-8h]

    uniqueHandleDb = system->uniqueHandleDb;
    if ( !uniqueHandleDb )
        return 0;
    if ( !handle )
        return 0;
    if ( HIWORD(handle) >= 0x401u
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_unique_handle.cpp",
                    35,
                    0,
                    "%s",
                    "shandle[1] < ARRAY_COUNT(uniqueHandleDb->nodes)+1") )
    {
        __debugbreak();
    }
    localNode = &uniqueHandleDb->nodes[HIWORD(handle) - 1];
    if ( localNode->handle != handle )
        return 0;
    if ( localNode->effect == 0xFFFF )
        return 0;
    if ( FX_EffectFromHandle(system, localNode->effect)->effect.uniqueHandle != handle )
        return 0;
    if ( localNode->nextNode != -1
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_unique_handle.cpp",
                    50,
                    0,
                    "%s",
                    "localNode->nextNode == FxUniqueHandleDb::INVALID_NODE") )
    {
        __debugbreak();
    }
    return FX_EffectFromHandle(system, localNode->effect);
}

void __cdecl UniqueHandleAssignHandle(FxSystem *system, FxEffectContainer *effect, FxEffectContainer *remoteEffect)
{
    FxUniqueHandleDb *uniqueHandleDb; // [esp+0h] [ebp-18h]
    int firstFreeNode; // [esp+4h] [ebp-14h]
    FxUniqueHandleDb::Node *localNode; // [esp+10h] [ebp-8h]

    if ( effect != remoteEffect
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_unique_handle.cpp",
                    57,
                    0,
                    "%s",
                    "effect == remoteEffect") )
    {
        __debugbreak();
    }
    uniqueHandleDb = system->uniqueHandleDb;
    if ( !uniqueHandleDb
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_unique_handle.cpp",
                    61,
                    0,
                    "%s",
                    "uniqueHandleDb") )
    {
        __debugbreak();
    }
    if ( UniqueHandleToEffect(system, effect->effect.uniqueHandle)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_unique_handle.cpp",
                    63,
                    0,
                    "%s",
                    "!UniqueHandleToEffect(system,effect->uniqueHandle)") )
    {
        __debugbreak();
    }
    Sys_EnterCriticalSection(CRITSECT_FX_UNIQUE_HANDLE);
    firstFreeNode = uniqueHandleDb->firstFreeNode;
    if ( firstFreeNode == -1
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_unique_handle.cpp",
                    69,
                    0,
                    "%s",
                    "firstFreeNode != FxUniqueHandleDb::INVALID_NODE") )
    {
        __debugbreak();
    }
    localNode = &uniqueHandleDb->nodes[firstFreeNode];
    if ( localNode->effect != 0xFFFF
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_unique_handle.cpp",
                    75,
                    0,
                    "%s",
                    "localNode->effect == FX_HANDLE_NONE") )
    {
        __debugbreak();
    }
    uniqueHandleDb->firstFreeNode = localNode->nextNode;
    localNode->effect = FX_EffectToHandle(system, remoteEffect);
    localNode->nextNode = -1;
    ++LOWORD(localNode->handle);
    if ( HIWORD(localNode->handle) != ((16 * firstFreeNode) >> 4) + 1
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_unique_handle.cpp",
                    88,
                    0,
                    "%s",
                    "shandle[1] == (remoteNode - &uniqueHandleDb->nodes[0])+1") )
    {
        __debugbreak();
    }
    effect->effect.uniqueHandle = localNode->handle;
    Sys_LeaveCriticalSection(CRITSECT_FX_UNIQUE_HANDLE);
    _InterlockedExchangeAdd(&uniqueHandleDb->numNodesInUse, 1u);
    if ( !UniqueHandleToEffect(system, effect->effect.uniqueHandle)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_unique_handle.cpp",
                    105,
                    0,
                    "%s",
                    "UniqueHandleToEffect(system,effect->uniqueHandle)") )
    {
        __debugbreak();
    }
}

void __cdecl UniqueHandleRelease(FxSystem *system, FxEffect *effect)
{
    FxUniqueHandleDb *uniqueHandleDb; // [esp+4h] [ebp-18h]
    FxUniqueHandleDb::Node *remoteNode; // [esp+Ch] [ebp-10h]

    uniqueHandleDb = system->uniqueHandleDb;
    if ( !uniqueHandleDb
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_unique_handle.cpp",
                    111,
                    0,
                    "%s",
                    "uniqueHandleDb") )
    {
        __debugbreak();
    }
    if ( HIWORD(effect->uniqueHandle) >= 0x401u
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_unique_handle.cpp",
                    120,
                    0,
                    "%s",
                    "shandle[1] < ARRAY_COUNT(uniqueHandleDb->nodes)+1") )
    {
        __debugbreak();
    }
    if ( UniqueHandleToEffect(system, effect->uniqueHandle) )
    {
        Sys_EnterCriticalSection(CRITSECT_FX_UNIQUE_HANDLE);
        remoteNode = &uniqueHandleDb->nodes[HIWORD(effect->uniqueHandle) - 1];
        if ( remoteNode->nextNode != -1
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_unique_handle.cpp",
                        131,
                        0,
                        "%s",
                        "localNode->nextNode == FxUniqueHandleDb::INVALID_NODE") )
        {
            __debugbreak();
        }
        if ( remoteNode->effect == 0xFFFF
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_unique_handle.cpp",
                        132,
                        0,
                        "%s",
                        "localNode->effect != FX_HANDLE_NONE") )
        {
            __debugbreak();
        }
        remoteNode->nextNode = uniqueHandleDb->firstFreeNode;
        remoteNode->effect = -1;
        effect->uniqueHandle = 0;
        uniqueHandleDb->firstFreeNode = ((char *)remoteNode - (char *)uniqueHandleDb) >> 4;
        ++LOWORD(remoteNode->handle);
        if ( HIWORD(remoteNode->handle) != (((char *)remoteNode - (char *)uniqueHandleDb) >> 4) + 1
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_unique_handle.cpp",
                        143,
                        0,
                        "%s",
                        "shandle[1] == (remoteNode - &uniqueHandleDb->nodes[0])+1") )
        {
            __debugbreak();
        }
        Sys_LeaveCriticalSection(CRITSECT_FX_UNIQUE_HANDLE);
        _InterlockedExchangeAdd(&uniqueHandleDb->numNodesInUse, 0xFFFFFFFF);
    }
}

void __cdecl UniqueHandleInit(FxUniqueHandleDb *uniqueHandleDb)
{
    FxUniqueHandleDb::Node *node; // [esp+0h] [ebp-10h]
    int i; // [esp+8h] [ebp-8h]
    int last; // [esp+Ch] [ebp-4h]

    uniqueHandleDb->numNodesInUse = 0;
    uniqueHandleDb->firstFreeNode = 0;
    last = -1;
    for ( i = 1023; i >= 0; --i )
    {
        node = &uniqueHandleDb->nodes[i];
        node->nextNode = last;
        node->effect = -1;
        last = i;
        ++LOWORD(node->handle);
        HIWORD(node->handle) = i + 1;
    }
}

