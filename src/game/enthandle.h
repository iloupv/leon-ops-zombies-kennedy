#pragma once

struct gentity_s;
struct sentient_s;

struct EntHandle // sizeof=0x4
{                                       // XREF: entityShared_t/r
                                        // gclient_s/r ...
    static void Init();
    static void Shutdown();

    void setEnt(gentity_s *ent);
    bool isDefined() const;
    gentity_s *ent();
    const gentity_s *ent() const;

    int entnum() const;

    unsigned __int16 number;
    unsigned __int16 infoIndex;
};

struct SentientHandle // sizeof=0x4
{
    static void Init();

    void setSentient(sentient_s *sentient);
    bool isDefined() const;
    sentient_s *sentient();
    const sentient_s *sentient() const;

    unsigned __int16 number;
    unsigned __int16 infoIndex;
};

struct EntHandleList // sizeof=0x2
{                                       // XREF: .data:g_entitiesHandleList/r
                                        // .data:g_sentientsHandleList/r
    unsigned __int16 infoIndex;
};

struct EntHandleInfo // sizeof=0x8
{                                       // XREF: .data:g_entHandleInfoArray/r
    void *handle;
    unsigned __int16 next;              // XREF: EntHandle::Init(void)+48/w
                                        // RemoveEntHandleInfo+1F/r
    unsigned __int16 prev;              // XREF: EntHandleDissociateInternal+E7/r
                                        // AddEntHandleInfo+BD/w
};


void __cdecl EntHandleDissociate(gentity_s *ent);
void __cdecl EntHandleDissociateInternal(EntHandleList *entHandleList);
void __cdecl SentientHandleDissociate(sentient_s *sentient);
void __cdecl RemoveEntHandleInfo(EntHandleList *entHandleList, unsigned int oldInfoIndex);
unsigned int __cdecl AddEntHandleInfo(EntHandleList *entHandleList, void *handle);
