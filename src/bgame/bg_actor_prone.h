#pragma once

enum proneCheckType_t : __int32
{                                       // XREF: ?BG_CheckProne@@YA_NPBUplayerState_s@@HQBMMMMPAM2_N33EW4proneCheckType_t@@M@Z/r
    PCT_CLIENT = 0x0,
    PCT_ACTOR  = 0x1,
};

struct actor_prone_info_s // sizeof=0x18
{                                                                             // XREF: actor_s/r
                                                                                // actor_prone_info_t/r ...
        bool bCorpseOrientation;                        // XREF: Actor_BecomeCorpse(gentity_s *)+4A/w
        bool orientPitch;
        bool prone;
        // padding byte
        int iProneTime;                                         // XREF: Actor_BecomeCorpse(gentity_s *)+50/w
        int iProneTrans;                                        // XREF: Actor_BecomeCorpse(gentity_s *)+56/w
        float fBodyHeight;                                    // XREF: Actor_BecomeCorpse(gentity_s *)+5C/w
        ///$A899A4A44C693354E5CF33C9EDFF92AE ___u6;
        union //$A899A4A44C693354E5CF33C9EDFF92AE // sizeof=0x4
        {                                                                             // XREF: Actor_BecomeCorpse(gentity_s *)+62/w
                                                                                        // actor_prone_info_s/r
                float fTorsoPitch;
                float fBodyPitch;
        };
                                                                                // XREF: Actor_BecomeCorpse(gentity_s *)+62/w
        //$8F7A1F2A0E788339D3BE9A175DA5EAEF ___u7;
        union //$8F7A1F2A0E788339D3BE9A175DA5EAEF // sizeof=0x4
        {                                                                             // XREF: Actor_BecomeCorpse(gentity_s *)+68/w
                                                                                        // actor_prone_info_s/r
                float fWaistPitch;
                float fBodyRoll;
        };
                                                                                // XREF: Actor_BecomeCorpse(gentity_s *)+68/w
};

int __cdecl BG_ActorIsProne(actor_prone_info_s *pInfo, int iCurrentTime);
double __cdecl BG_GetActorProneFraction(actor_prone_info_s *pInfo, int iCurrentTime);
