#pragma once

#include <cstring>
#include <cstdint>

#include <DemonWare/bdCore/bdMemory/bdMemory.h>

// (aislop)
template <typename T>
struct bdFastArray
{
    // ===== Layout (matches decomp) =====
    T *m_data;      // +0x00
    unsigned int m_capacity;  // +0x04
    unsigned int m_size;      // +0x08

    // ===== Ctors / Dtor =====
    inline bdFastArray()
        : m_data(nullptr)
        , m_capacity(0)
        , m_size(0)
    {
    }

    inline ~bdFastArray()
    {
        bdMemory::deallocate(m_data);
        m_data = nullptr;
        m_capacity = 0;
        m_size = 0;
    }

    // ===== Core helpers =====

    inline unsigned int increaseCapacity(unsigned int additional)
    {
        unsigned int grow;

        if (additional <= m_capacity)
            grow = m_capacity;
        else
            grow = additional;

        unsigned int newCapacity = grow + m_capacity;

        T *newData = nullptr;
        if (newCapacity)
        {
            newData = static_cast<T *>(bdMemory::allocate(newCapacity * sizeof(T)));
            if (m_size)
                std::memcpy(newData, m_data, m_size * sizeof(T));
        }

        bdMemory::deallocate(m_data);

        m_data = newData;
        m_capacity = newCapacity;
        return newCapacity;
    }

    // ===== Element access / mutation =====

    inline T setGrow(unsigned int index, const T &value)
    {
        if (index >= m_size)
        {
            if (m_capacity < index + 1)
                increaseCapacity(index + 1 - m_capacity);

            m_size = index + 1;
        }

        m_data[index] = value;
        return value;
    }

    inline bdFastArray *pushBack(const T *src, unsigned int count)
    {
        unsigned int newSize = m_size + count;

        if (m_capacity < newSize)
            increaseCapacity(newSize - m_capacity);

        if (count)
            std::memcpy(m_data + m_size, src, count * sizeof(T));

        m_size = newSize;
        return this;
    }

    // ===== Convenience =====

    inline void clear()
    {
        m_size = 0;
    }

    inline T &operator[](unsigned int i)
    {
        return m_data[i];
    }

    inline const T &operator[](unsigned int i) const
    {
        return m_data[i];
    }
};
