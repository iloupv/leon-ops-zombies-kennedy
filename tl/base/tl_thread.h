#pragma once

#include <Windows.h>

struct __declspec(align(8)) tlAtomicReadWriteMutex // sizeof=0x18
{                                       // XREF: .data:tlAtomicReadWriteMutex g_auto_rigid_body_map_mutex/r
                                        // broad_phase_memory/r
    volatile unsigned __int64 WriteThreadId;
                                        // XREF: _dynamic_initializer_for__g_auto_rigid_body_map_mutex__+3/w
                                        // _dynamic_initializer_for__g_auto_rigid_body_map_mutex__+D/w
    volatile unsigned int ReadLockCount;         // XREF: _dynamic_initializer_for__g_auto_rigid_body_map_mutex__+17/w
    volatile unsigned int WriteLockCount;        // XREF: _dynamic_initializer_for__g_auto_rigid_body_map_mutex__+21/w
    tlAtomicReadWriteMutex *ThisPtr;    // XREF: auto_rigid_body::add(centity_s const *,gjk_physics_collision_visitor *,int)+15B/r
                                        // _dynamic_initializer_for__g_auto_rigid_body_map_mutex__+2B/w ...
    // padding byte
    // padding byte
    // padding byte
    // padding byte

    tlAtomicReadWriteMutex()
    {
        WriteThreadId = 0;
        ReadLockCount = 0;
        WriteLockCount = 0;
        ThisPtr = this;
    }

    ~tlAtomicReadWriteMutex()
    {
        this->ThisPtr = 0;
    }

    inline void WriteLock() 
    {
        LONG Target; // [esp+30h] [ebp-24h] BYREF
        volatile unsigned __int32 *p_WriteLockCount; // [esp+34h] [ebp-20h]
        tlAtomicReadWriteMutex *v4; // [esp+38h] [ebp-1Ch]
        volatile unsigned __int32 *p_ReadLockCount; // [esp+3Ch] [ebp-18h]
        tlAtomicReadWriteMutex *v6; // [esp+40h] [ebp-14h]
        tlAtomicReadWriteMutex *ThisPtr; // [esp+48h] [ebp-Ch]
        unsigned __int64 CurThread; // [esp+4Ch] [ebp-8h]

        CurThread = GetCurrentThreadId();
        ThisPtr = this->ThisPtr;
        if (_InterlockedCompareExchange64((volatile signed __int64 *)ThisPtr, CurThread, CurThread) == CurThread)
        {
            _InterlockedExchangeAdd(&this->ThisPtr->WriteLockCount, 1u);
            return;
        }
        while (1)
        {
            v6 = this->ThisPtr;
            if (_InterlockedCompareExchange64((volatile signed __int64 *)v6, CurThread, 0))
                goto LABEL_4;
            p_ReadLockCount = &this->ThisPtr->ReadLockCount;
            if (!_InterlockedCompareExchange(p_ReadLockCount, 0, 0))
                break;
            do
                v4 = this->ThisPtr;
            while (_InterlockedCompareExchange64((volatile signed __int64 *)v4, 0, CurThread) != CurThread);
        LABEL_4:
            SwitchToThread();
        }
        p_WriteLockCount = &this->ThisPtr->WriteLockCount;
        _InterlockedExchangeAdd(p_WriteLockCount, 1u);
        Target = 0;
        InterlockedExchange(&Target, 0);
    }

    inline void ReadLock()
    {
        LONG Target; // [esp+30h] [ebp-20h] BYREF
        tlAtomicReadWriteMutex *v3; // [esp+34h] [ebp-1Ch]
        volatile unsigned __int32 *v4; // [esp+38h] [ebp-18h]
        tlAtomicReadWriteMutex *v5; // [esp+3Ch] [ebp-14h]
        volatile unsigned __int32 *p_ReadLockCount; // [esp+40h] [ebp-10h]
        tlAtomicReadWriteMutex *ThisPtr; // [esp+44h] [ebp-Ch]
        unsigned __int64 CurThread; // [esp+48h] [ebp-8h]

        CurThread = GetCurrentThreadId();
        ThisPtr = this->ThisPtr;
        if (_InterlockedCompareExchange64((volatile signed __int64 *)ThisPtr, CurThread, CurThread) == CurThread)
        {
            p_ReadLockCount = &this->ThisPtr->ReadLockCount;
            _InterlockedExchangeAdd(p_ReadLockCount, 1u);
        }
        else
        {
            while (1)
            {
                v5 = this->ThisPtr;
                if (!_InterlockedCompareExchange64((volatile signed __int64 *)v5, CurThread, 0))
                    break;
                SwitchToThread();
            }
            v4 = &this->ThisPtr->ReadLockCount;
            _InterlockedExchangeAdd(v4, 1u);
            do
                v3 = this->ThisPtr;
            while (_InterlockedCompareExchange64((volatile signed __int64 *)v3, 0, CurThread) != CurThread);
        }
        Target = 0;
        InterlockedExchange(&Target, 0);
    }
    inline void WriteUnlock()
    {
        signed __int64 v1; // [esp+Ch] [ebp-20h]
        LONG Target; // [esp+1Ch] [ebp-10h] BYREF
        volatile unsigned __int32 *p_WriteLockCount; // [esp+20h] [ebp-Ch]
        unsigned __int64 CurThread; // [esp+24h] [ebp-8h]

        CurThread = GetCurrentThreadId();
        p_WriteLockCount = &this->ThisPtr->WriteLockCount;
        if (!_InterlockedDecrement(p_WriteLockCount))
        {
            Target = 0;
            InterlockedExchange(&Target, 0);
            do
                v1 = _InterlockedCompareExchange64((volatile signed __int64 *)this->ThisPtr, 0, CurThread);
            while (v1 != CurThread);
        }
    }
};