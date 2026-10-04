#pragma once
#include "enthandle.h"

enum fieldtype_t : __int32
{                                       // XREF: sentient_fields_s/r
    F_INT            = 0x0,             // XREF: .rdata:cent_fields/s
                                        // .rdata:fields/s ...
    F_SHORT          = 0x1,             // XREF: .data:fields_3/s
    F_BYTE           = 0x2,             // XREF: .rdata:fields_0/s
                                        // .rdata:aifields/s ...
    F_FLOAT          = 0x3,             // XREF: .rdata:fields/s
                                        // .rdata:fields_0/s ...
    F_LSTRING        = 0x4,             // XREF: .rdata:cent_fields/s
                                        // .rdata:fields/s ...
    F_STRING         = 0x5,             // XREF: .rdata:cent_fields/s
                                        // .rdata:client_spawn_fields/s ...
    F_VECTOR         = 0x6,             // XREF: .rdata:cent_fields/s
                                        // .rdata:client_spawn_fields/s ...
    F_ENTITY         = 0x7,             // XREF: .rdata:cent_fields/s
                                        // .rdata:fields/s
    F_ENTHANDLE      = 0x8,             // XREF: .rdata:aifields/s
                                        // .rdata:sentientfields/s ...
    F_ACTOR          = 0x9,
    F_SENTIENT       = 0xA,
    F_SENTIENTHANDLE = 0xB,             // XREF: .rdata:aifields/s
    F_CLIENT         = 0xC,
    F_PATHNODE       = 0xD,             // XREF: .rdata:sentientfields/s
                                        // .rdata:fields_2/s
    F_ACTORGROUP     = 0xE,
    F_VECTORHACK     = 0xF,             // XREF: .data:fields_3/s
    F_OBJECT         = 0x10,            // XREF: .rdata:fields/s
    F_MODEL          = 0x11,            // XREF: .rdata:client_spawn_fields/s
                                        // .rdata:fields_1/s ...
    F_BITFLAG        = 0x12,            // XREF: .rdata:fields/s
};

struct sentient_fields_s // sizeof=0x18
{                                       // XREF: .rdata:fields_2/r
    const char *name;
    int ofs;
    int size[1];
    fieldtype_t type;
    void (__cdecl *setter)(sentient_s *, const sentient_fields_s *);
    void (__cdecl *getter)(sentient_s *, const sentient_fields_s *);
};

void __cdecl SentientScr_ReadOnly(sentient_s *pSelf, const sentient_fields_s *pField);
void __cdecl SentientScr_SetTeam(sentient_s *pSelf, const sentient_fields_s *pField);
void __cdecl SentientScr_GetTeam(sentient_s *pSelf, const sentient_fields_s *pField);
void __cdecl GScr_AddFieldsForSentient();
void __cdecl Scr_SetSentientField(sentient_s *sentient, unsigned int offset);
void __cdecl Scr_GetSentientField(sentient_s *sentient, unsigned int offset);
