#ifdef BO1_DW

#include "MatchMakingQueries_win32.h"

MatchMaking_PC_QUICKMATCH_Query::MatchMaking_PC_QUICKMATCH_Query() : bdSessionParams()
{
    //bdSessionParams::bdSessionParams(this);
    //this->__vftable = (MatchMaking_PC_QUICKMATCH_Query_vtbl *)&MatchMaking_PC_QUICKMATCH_Query::`vftable';
    //return this;
}

void MatchMaking_PC_QUICKMATCH_Query::serialize(bdByteBuffer *buffer)
{
#ifdef BO1_DW_STUBS
    bdHashMap<bdReference<bdCommonAddr>,bdArray<bdQoSProbe::bdQoSProbeEntryWrapper>,bdHashingClass>::releaseIterator((int)buffer);
    bdByteBuffer::writeUInt32(buffer, this->m_memberPARAM_SERVERTYPE);
    bdByteBuffer::writeInt32(buffer, this->m_memberPARAM_NETCODE_VERSION);
    bdByteBuffer::writeUInt32(buffer, this->m_memberPARAM_MAXPLAYERS);
    bdByteBuffer::writeUInt32(buffer, this->m_memberPARAM_PLAYLIST);
    bdByteBuffer::writeInt32(buffer, this->m_memberPARAM_PLAYLIST_VERSION);
    bdByteBuffer::writeUInt32(buffer, this->m_memberPARAM_LICENSE);
#endif
}



MatchMaking_PC_WAGER_Query::MatchMaking_PC_WAGER_Query() : bdSessionParams()
{
    //bdSessionParams::bdSessionParams(this);
    //this->__vftable = (MatchMaking_PC_WAGER_Query_vtbl *)&MatchMaking_PC_WAGER_Query::`vftable';
    //return this;
}

void MatchMaking_PC_WAGER_Query::serialize(bdByteBuffer *buffer)
{
#ifdef BO1_DW_STUBS
    bdHashMap<bdReference<bdCommonAddr>,bdArray<bdQoSProbe::bdQoSProbeEntryWrapper>,bdHashingClass>::releaseIterator((int)buffer);
    bdByteBuffer::writeInt32(buffer, this->m_memberPARAM_SERVERTYPE);
    bdByteBuffer::writeInt32(buffer, this->m_memberPARAM_LICENSE);
    bdByteBuffer::writeInt32(buffer, this->m_memberPARAM_PLAYLIST_MIN);
    bdByteBuffer::writeInt32(buffer, this->m_memberPARAM_PLAYLIST_MAX);
    bdByteBuffer::writeInt32(buffer, this->m_memberPARAM_NETCODE_VERSION);
    bdByteBuffer::writeInt32(buffer, this->m_memberPARAM_PLAYLIST_VERSION);
#endif
}




MatchMaking_PC_GEO_Query::MatchMaking_PC_GEO_Query() :bdSessionParams()
{
    //bdSessionParams::bdSessionParams(this);
    //this->__vftable = (MatchMaking_PC_GEO_Query_vtbl *)&MatchMaking_PC_GEO_Query::`vftable';
    //return this;
}

void MatchMaking_PC_GEO_Query::serialize(bdByteBuffer *buffer)
{
#ifdef BO1_DW_STUBS
    bdHashMap<bdReference<bdCommonAddr>,bdArray<bdQoSProbe::bdQoSProbeEntryWrapper>,bdHashingClass>::releaseIterator((int)buffer);
    bdByteBuffer::writeUInt32(buffer, this->m_memberPARAM_NETCODE_VERSION);
    bdByteBuffer::writeUInt32(buffer, this->m_memberPARAM_GAMETYPE);
    bdByteBuffer::writeUInt32(buffer, this->m_memberPARAM_SERVERTYPE);
    bdByteBuffer::writeUInt32(buffer, this->m_memberPARAM_DDL_VERSION);
    bdByteBuffer::writeUInt32(buffer, this->m_memberPARAM_RANKED);
    bdByteBuffer::writeUInt32(buffer, this->m_memberPARAM_LICENSE);
    bdByteBuffer::writeUInt32(buffer, this->m_memberPARAM_GEO);
#endif
}


#endif