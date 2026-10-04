#pragma once

#include <cstdint>

#include <DemonWare/bdCore/bdMemory/bdMemory.h>

// aislop

/*
===============================================================================
  bdArray<T> — binary-faithful implementation
===============================================================================
*/

template <typename T>
struct bdArray
{
    T *m_data;      // +0x00
    unsigned int m_capacity;  // +0x04
    unsigned int m_size;      // +0x08

    // ---------------------------------------------------------------------
    // ctor / dtor
    // ---------------------------------------------------------------------

    bdArray()
        : m_data(nullptr)
        , m_capacity(0)
        , m_size(0)
    {
    }

    ~bdArray()
    {
        clear();
    }

    // ---------------------------------------------------------------------
    // clear (destroys elements + frees buffer)
    // ---------------------------------------------------------------------

    inline int clear()
    {
        unsigned int count = m_size;
        T *data = m_data;

        for (unsigned int i = 0; i < count; ++i)
        {
            // NOTE: binary explicitly destroys bdInetAddr only
            data[i].m_address.~bdInetAddr();
        }

        bdMemory::deallocate(m_data);

        m_data = nullptr;
        m_capacity = 0;
        m_size = 0;
        return (int)this;
    }

    // ---------------------------------------------------------------------
    // capacity decrease (matches shrink heuristic exactly)
    // ---------------------------------------------------------------------

    inline int decreaseCapacity(unsigned int a2)
    {
        if (m_capacity <= 4 * m_size)
            return (int)this;

        unsigned int reduce;
        if (a2 <= m_capacity - m_size)
            reduce = a2;
        else
            reduce = m_capacity - m_size;

        unsigned int delta;
        if (reduce <= (m_capacity >> 1))
            delta = m_capacity >> 1;
        else
            delta = reduce;

        m_capacity -= delta;

        T *newData = nullptr;
        if (m_capacity)
        {
            newData = static_cast<T *>(bdMemory::allocate(sizeof(T) * m_capacity));
            for (unsigned int i = 0; i < m_size; ++i)
                new (&newData[i]) T(&m_data[i]);
        }

        for (unsigned int i = 0; i < m_size; ++i)
            m_data[i].m_address.~bdInetAddr();

        bdMemory::deallocate(m_data);
        m_data = newData;
        return (int)this;
    }

    // ---------------------------------------------------------------------
    // capacity increase
    // ---------------------------------------------------------------------

    inline int increaseCapacity(unsigned int a2)
    {
        unsigned int grow = (a2 <= m_capacity) ? m_capacity : a2;
        unsigned int newCap = grow + m_capacity;

        T *newData = nullptr;
        if (newCap)
        {
            newData = static_cast<T *>(bdMemory::allocate(sizeof(T) * newCap));
            for (unsigned int i = 0; i < m_size; ++i)
                new (&newData[i]) T(&m_data[i]);
        }

        for (unsigned int i = 0; i < m_size; ++i)
            m_data[i].m_address.~bdInetAddr();

        bdMemory::deallocate(m_data);

        m_data = newData;
        m_capacity = newCap;
        return (int)this;
    }

    // ---------------------------------------------------------------------
    // assignment operator
    // ---------------------------------------------------------------------

    inline int operator=(const bdArray &rhs)
    {
        if (this == &rhs)
            return (int)this;

        unsigned int rhsSize = rhs.m_size;

        if (rhsSize <= m_capacity)
        {
            if (rhsSize <= m_size)
            {
                for (unsigned int i = 0; i < rhsSize; ++i)
                    m_data[i] = rhs.m_data[i];

                for (unsigned int i = rhsSize; i < m_size; ++i)
                    m_data[i].m_address.~bdInetAddr();

                m_size = rhsSize;
                return decreaseCapacity(0);
            }
            else
            {
                unsigned int i = 0;
                for (; i < m_size; ++i)
                    m_data[i] = rhs.m_data[i];

                for (; i < rhsSize; ++i)
                    new (&m_data[i]) T(&rhs.m_data[i]);

                m_size = rhsSize;
                return rhsSize;
            }
        }

        clear();
        m_data = uninitializedCopy(rhs);
        m_capacity = rhs.m_capacity;
        m_size = rhsSize;
        return rhsSize;
    }

    // ---------------------------------------------------------------------
    // pushBack
    // ---------------------------------------------------------------------

    inline int pushBack(const T *value)
    {
        if (m_size == m_capacity)
            increaseCapacity(1);

        new (&m_data[m_size]) T(value);
        return ++m_size;
    }

    // ---------------------------------------------------------------------
    // uninitializedCopy (static helper)
    // ---------------------------------------------------------------------

    static inline T *uninitializedCopy(const bdArray &src)
    {
        T *out = nullptr;
        if (src.m_capacity)
        {
            out = static_cast<T *>(
                bdMemory::allocate(sizeof(T) * src.m_capacity));

            for (unsigned int i = 0; i < src.m_size; ++i)
                new (&out[i]) T(&src.m_data[i]);
        }
        return out;
    }
};
