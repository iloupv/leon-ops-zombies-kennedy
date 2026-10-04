#pragma once

#include <Windows.h>

template <typename T>
struct bdReference//<bdByteBuffer> // sizeof=0x4
{                                                                             // XREF: bdRemoteTask/r
    T *m_ptr;

    const bdReference<T> *operator=(const bdReference<T> &other)
    {
        if (&other != this)
        {
            if (m_ptr)
            {
                T *oldPtr = m_ptr;

                if (InterlockedDecrement(&m_ptr->m_refCount) == 0)
                {
                    // Demonware-style explicit destructor call
                    oldPtr->~T();
                }
            }

            m_ptr = other.m_ptr;

            if (m_ptr)
            {
                InterlockedIncrement(&m_ptr->m_refCount);
            }
        }

        return this;
    }

    bdReference &operator=(T *ptr)
    {
        if (m_ptr != ptr)
        {
            if (m_ptr)
            {
                T *old = m_ptr;
                InterlockedDecrement(&old->m_refCount);
                if (!old->m_refCount)
                {
                    old->~T();
                }
            }

            m_ptr = ptr;

            if (m_ptr)
                InterlockedIncrement(&m_ptr->m_refCount);
        }
        return *this;
    }
};

struct bdReferencable // sizeof=0x8
{                                                                             // XREF: bdByteBuffer/r
    volatile unsigned int m_refCount;

    virtual ~bdReferencable() = default;
};