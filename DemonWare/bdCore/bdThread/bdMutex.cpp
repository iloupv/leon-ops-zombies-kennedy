#include "bdMutex.h"


bdMutex::bdMutex()
{
    this->m_handle = bdPlatformMutex::createMutex();
    //return this;
}

bdMutex::~bdMutex()
{
    bdPlatformMutex::destroy(this->m_handle);
}

void bdMutex::lock()
{
    bdPlatformMutex::lock(this->m_handle);
}

void bdMutex::unlock()
{
    bdPlatformMutex::unlock(this->m_handle);
}
