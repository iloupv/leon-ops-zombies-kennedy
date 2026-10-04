#pragma once
#include <steam/steam_api.h>

struct LiveSteamClient // sizeof=0x28
{
    EResult resultOnRequestEncryptedAppTicket;
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    CCallResult<LiveSteamClient,EncryptedAppTicketResponse_t> m_SteamCallResultEncryptedAppTicket;


    LiveSteamClient();
    void RequestEncryptedAppTicket(const void *data, unsigned int size);
    char GetRetrievedEncryptedAppTicket(
        void *ticketBuf,
        unsigned int ticketBufSize,
        unsigned int *ticketSize);
    void OnRequestEncryptedAppTicket(
        EncryptedAppTicketResponse_t *pEncryptedAppTicketResponse,
        bool bIOFailure);
};

