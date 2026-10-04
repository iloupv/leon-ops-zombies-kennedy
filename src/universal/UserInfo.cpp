#include "UserInfo.h"

PublicProfileInfo::PublicProfileInfo() : bdProfileInfo()
{
    //bdProfileInfo::bdProfileInfo(this);
    //this->__vftable = (PublicProfileInfo_vtbl *)&PublicProfileInfo::`vftable';
    //return this;
}

void PublicProfileInfo::serialize(bdByteBuffer *buffer)
{
#ifdef BO1_USERINFO
    bdHashMap<bdReference<bdCommonAddr>,bdArray<bdQoSProbe::bdQoSProbeEntryWrapper>,bdHashingClass>::releaseIterator((int)buffer);
    bdByteBuffer::writeInt32(buffer, this->m_memberVERSION);
    bdByteBuffer::writeBlob(buffer, this->m_memberddl, 0x100u);
#endif
}

bool PublicProfileInfo::deserialize(bdReference<bdByteBuffer> buffer)
{
#ifdef BO1_USERINFO
    bdReference<bdCommonAddr> v3; // [esp-4h] [ebp-44h] BYREF
    bool v4; // [esp+0h] [ebp-40h]
    bool v5; // [esp+4h] [ebp-3Ch]
    bool v6; // [esp+Bh] [ebp-35h]
    volatile int m_refCount; // [esp+Ch] [ebp-34h]
    PublicProfileInfo *thisa; // [esp+10h] [ebp-30h]
    bdByteBuffer *v9; // [esp+20h] [ebp-20h]
    bdByteBuffer *v10; // [esp+24h] [ebp-1Ch]
    bdReference<bdCommonAddr> *v11; // [esp+28h] [ebp-18h]
    bdCommonAddr *m_ptr; // [esp+2Ch] [ebp-14h]
    bool v13; // [esp+33h] [ebp-Dh]
    unsigned int tempBlobLength_ddl; // [esp+38h] [ebp-8h] BYREF
    bool ok; // [esp+3Fh] [ebp-1h]

    thisa = this;
    v11 = &v3;
    v3.m_ptr = (bdCommonAddr *)buffer.m_ptr;
    if ( buffer.m_ptr )
    {
        m_ptr = v11->m_ptr;
        InterlockedIncrement(&m_ptr->m_refCount);
        m_refCount = m_ptr->m_refCount;
    }
    v6 = bdProfileInfo::deserialize((unsigned __int64 *)thisa, v3);
    ok = v6;
    v5 = v6 && (v10 = buffer.m_ptr, bdByteBuffer::readInt32(buffer.m_ptr, &thisa->m_memberVERSION));
    ok = v5;
    tempBlobLength_ddl = 256;
    v4 = v5 && (v9 = buffer.m_ptr, bdByteBuffer::readBlob(buffer.m_ptr, thisa->m_memberddl, &tempBlobLength_ddl));
    ok = v4;
    v13 = v4;
    bdReference<bdRemoteTask>::~bdReference<bdRemoteTask>((bdReference<bdCommonAddr> *)&buffer);
    return v13;
#else
    return false;
#endif
}

unsigned int PublicProfileInfo::sizeOf()
{
    return 280;
}

PrivateProfileInfo::PrivateProfileInfo() : bdProfileInfo()
{
    //bdProfileInfo::bdProfileInfo(this);
    //this->__vftable = (PrivateProfileInfo_vtbl *)&PrivateProfileInfo::`vftable';
    //return this;
}

void __thiscall PrivateProfileInfo::serialize(bdByteBuffer *buffer)
{
#ifdef BO1_USERINFO
    bdHashMap<bdReference<bdCommonAddr>,bdArray<bdQoSProbe::bdQoSProbeEntryWrapper>,bdHashingClass>::releaseIterator((int)buffer);
    bdByteBuffer::writeBlob(buffer, this->m_memberfavsblob, 0xFCu);
    bdByteBuffer::writeBlob(buffer, this->m_memberuids, 0x150u);
#endif
}

bool __thiscall PrivateProfileInfo::deserialize(bdReference<bdByteBuffer> buffer)
{
#ifdef BO1_USERINFO
    bdReference<bdCommonAddr> v3; // [esp-4h] [ebp-48h] BYREF
    bool v4; // [esp+0h] [ebp-44h]
    bool v5; // [esp+4h] [ebp-40h]
    bool v6; // [esp+Bh] [ebp-39h]
    volatile int m_refCount; // [esp+Ch] [ebp-38h]
    PrivateProfileInfo *thisa; // [esp+10h] [ebp-34h]
    bdByteBuffer *v9; // [esp+20h] [ebp-24h]
    bdByteBuffer *v10; // [esp+24h] [ebp-20h]
    bdReference<bdCommonAddr> *v11; // [esp+28h] [ebp-1Ch]
    bdCommonAddr *m_ptr; // [esp+2Ch] [ebp-18h]
    bool v13; // [esp+33h] [ebp-11h]
    unsigned int tempBlobLength_uids; // [esp+38h] [ebp-Ch] BYREF
    unsigned int tempBlobLength_favsblob; // [esp+3Ch] [ebp-8h] BYREF
    bool ok; // [esp+43h] [ebp-1h]

    thisa = this;
    v11 = &v3;
    v3.m_ptr = (bdCommonAddr *)buffer.m_ptr;
    if ( buffer.m_ptr )
    {
        m_ptr = v11->m_ptr;
        InterlockedIncrement(&m_ptr->m_refCount);
        m_refCount = m_ptr->m_refCount;
    }
    v6 = bdProfileInfo::deserialize((unsigned __int64 *)thisa, v3);
    ok = v6;
    tempBlobLength_favsblob = 252;
    v5 = v6
        && (v10 = buffer.m_ptr, bdByteBuffer::readBlob(buffer.m_ptr, thisa->m_memberfavsblob, &tempBlobLength_favsblob));
    ok = v5;
    tempBlobLength_uids = 336;
    v4 = v5 && (v9 = buffer.m_ptr, bdByteBuffer::readBlob(buffer.m_ptr, thisa->m_memberuids, &tempBlobLength_uids));
    ok = v4;
    v13 = v4;
    bdReference<bdRemoteTask>::~bdReference<bdRemoteTask>((bdReference<bdCommonAddr> *)&buffer);
    return v13;
#else
    return false;
#endif
}

unsigned int __thiscall PrivateProfileInfo::sizeOf()
{
    return 608;
}

