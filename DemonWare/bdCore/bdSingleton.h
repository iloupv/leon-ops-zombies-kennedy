#pragma once


template <typename T>
struct bdSingleton
{
    inline static T *getInstance()
    {
        if (!m_instance)
        {
            m_instance = new T;
        }

        // some more bloaty crap here prob, impl this if this whole lib isn't nuked later

        return m_instance;
    }
    inline static void destroyInstance()
    {
        if (m_instance)
        {
            delete m_instance;
            m_instance = 0;
        }
    }

    bdSingleton()
    {
    }

    static T *m_instance;
};