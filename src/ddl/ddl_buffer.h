#pragma once

struct ddlBufferHeader_t // sizeof=0x28
{                                       // XREF: ?DDL_AssociateBuffer@@YAHPADHPAUddlDef_t@@@Z/r
                                        // ?DDL_FixBufferVersion@@YAHPADPAUddlDef_t@@PBD0H@Z/r ...
    unsigned int checksum;
    int version;                        // XREF: DDL_AssociateBuffer(char *,int,ddlDef_t *)+A9/r
    // DDL_AssociateBuffer(char *,int,ddlDef_t *)+B2/r ...
    unsigned __int8 flags;              // XREF: DDL_SetUserFlag(char *,int,bool)+5E/r
    // DDL_SetUserFlag(char *,int,bool)+64/w ...
    char associatedDDL[31];
};

void __cdecl DDL_Buffer_WriteHeader(char *buffer, ddlBufferHeader_t *header);
void __cdecl DDL_Buffer_ReadHeader(char *buffer, ddlBufferHeader_t *header);
int __cdecl DDL_Buffer_ReadBits(char *buffer, int offset, int size, unsigned int *value);
int __cdecl DDL_Buffer_WriteBits(char *buffer, int offset, int size, unsigned int value);
int __cdecl DDL_Buffer_ClearBits(char *buffer, int offset, int size);
