#pragma once

#include <universal/dvar.h>

enum streamName_t : __int32
{                                       // XREF: streams_t/r ticker_t/r ...
    TICKER_STREAM_COD       = 0x0,
    TICKER_STREAM_SQUADS    = 0x1,
    TICKER_STREAM_FRIENDS   = 0x2,
    TICKER_STREAM_EVENTS    = 0x3,
    TICKER_STREAM_CONTRACTS = 0x4,
    TICKER_STREAM_NONE      = 0x5,
    TICKER_STREAM_COUNT     = 0x5,
};

enum messageState_e : __int32
{                                       // XREF: messages_t/r
    MESSAGE_NOT_DISPLAYING  = 0x0,
    MESSAGE_SCROLLING_UP_A  = 0x1,
    MESSAGE_PAUSED_A        = 0x2,
    MESSAGE_SCROLLING_LEFT  = 0x3,
    MESSAGE_PAUSED_B        = 0x4,
    MESSAGE_SCROLLING_UP_B  = 0x5,
    MESSAGE_DONE_DISPLAYING = 0x6,
};

struct messages_t // sizeof=0x204
{                                       // XREF: streams_t/r
    char text[512];
    messageState_e state;
};

struct streams_t // sizeof=0x1434
{                                       // XREF: ticker_t/r
    streamName_t streamName;            // XREF: LiveTicker_DumpTickerInternal(void)+60/r
                                        // LiveTicker_InitializeInternal(void)+46/w ...
    messages_t messages[10];
    int messageCount;
    int currentMessageIndex;
};

struct __declspec(align(4)) ticker_t // sizeof=0x656C
{                                       // XREF: .data:s_ticker/r
    streams_t streams[5];               // XREF: LiveTicker_DumpTickerInternal(void)+60/r
                                        // LiveTicker_InitializeInternal(void)+46/w ...
    streamName_t streamDisplayOrder[5]; // XREF: LiveTicker_SetDefaultDisplayOrder(void)+4/w
                                        // LiveTicker_SetDefaultDisplayOrder(void)+E/w ...
    int streamBatchSize[5];             // XREF: LiveTicker_SetDefaultDisplayOrder(void)+51/w
                                        // LiveTicker_MoveToNextMessage(streamName_t)+61/r ...
    streamName_t currentStream;         // XREF: LiveTicker_InitializeInternal(void)+57/w
                                        // LiveTicker_MoveToFirstStream(void)+4/r ...
    streamName_t previousStream;        // XREF: LiveTicker_InitializeInternal(void)+61/w
                                        // LiveTicker_MoveToNextStream(void)+74/w ...
    int totalMessageCount;              // XREF: LiveTicker_DumpTickerInternal(void)+26/r
                                        // LiveTicker_InitializeInternal(void)+6B/w ...
    int currentStreamIndex;
    bool isHeaderAnimating;             // XREF: LiveTicker_InitializeInternal(void)+75/w
                                        // LiveTicker_GetCurrentMessage(float,float,float,float,Font_s * const,float,float &,float &,float &):loc_98E924/r ...
    // padding byte
    // padding byte
    // padding byte
    int pauseStartTimeA;                // XREF: LiveTicker_InitializeInternal(void)+7C/w
                                        // LiveTicker_GetCurrentMessage(float,float,float,float,Font_s * const,float,float &,float &,float &)+163/w ...
    int scrollLeftStartTime;            // XREF: LiveTicker_InitializeInternal(void)+86/w
                                        // LiveTicker_GetCurrentMessage(float,float,float,float,Font_s * const,float,float &,float &,float &)+16D/w ...
    int scrollUpStartTimeB;             // XREF: LiveTicker_InitializeInternal(void)+90/w
                                        // LiveTicker_GetCurrentMessage(float,float,float,float,Font_s * const,float,float &,float &,float &)+177/w ...
    int pauseStartTimeB;                // XREF: LiveTicker_InitializeInternal(void)+9A/w
                                        // LiveTicker_GetCurrentMessage(float,float,float,float,Font_s * const,float,float &,float &,float &)+181/w ...
    float scrollLeftDuration;           // XREF: LiveTicker_InitializeInternal(void)+AC/w
                                        // LiveTicker_GetCurrentMessage(float,float,float,float,Font_s * const,float,float &,float &,float &)+193/w ...
    float scrollLeftFinalX;             // XREF: LiveTicker_InitializeInternal(void)+BC/w
                                        // LiveTicker_GetCurrentMessage(float,float,float,float,Font_s * const,float,float &,float &,float &)+1A3/w ...
    bool hasMessageScrolledLeft;        // XREF: LiveTicker_InitializeInternal(void)+C4/w
                                        // LiveTicker_GetCurrentMessage(float,float,float,float,Font_s * const,float,float &,float &,float &)+1AB/w ...
    // padding byte
    // padding byte
    // padding byte
    int scrollUpStartTimeA;             // XREF: LiveTicker_InitializeInternal(void)+CB/w
                                        // LiveTicker_GetCurrentMessage(float,float,float,float,Font_s * const,float,float &,float &,float &)+1B7/w ...
    int messagedWidthInPixels;          // XREF: LiveTicker_InitializeInternal(void)+D5/w
                                        // LiveTicker_GetCurrentMessage(float,float,float,float,Font_s * const,float,float &,float &,float &)+1D5/w ...
    int headerAnimationStartTime;       // XREF: LiveTicker_GetCurrentMessageCategory(float,float,float,Font_s *,float,float &,float &,float &)+8C/r
                                        // LiveTicker_GetCurrentMessageCategory(float,float,float,Font_s *,float,float &,float &,float &)+9F/w ...
    bool isUserOnline;                  // XREF: LiveTicker_InitializeInternal(void)+E6/w
    // padding byte
    // padding byte
    // padding byte
};

struct Font_s;

char *__cdecl LiveTicker_GetStreamNameTextInternal(streamName_t streamName);
void __cdecl LiveTicker_DumpTickerInternal();
void __cdecl LiveTicker_SetDefaultDisplayOrder();
void __cdecl LiveTicker_InitializeInternal();
void __cdecl LiveTicker_InsertMessageInternal(const char *text, streamName_t streamName);
char __cdecl LiveTicker_IsStreamPopulated(streamName_t streamName);
int __cdecl LiveTicker_GetTickerMessageTextWidth(const char *text, int maxChars, Font_s *font, float scale);
void __cdecl LiveTicker_MoveToFirstStream();
void __cdecl LiveTicker_MoveToNextStream();
void __cdecl LiveTicker_MoveToNextMessage(streamName_t streamName);
void __cdecl LiveTicker_ReplaceMessages(const char *text, streamName_t streamName, bool isMultiple);
void __cdecl LiveTicker_AddMessages(const char *text, streamName_t streamName, bool isMultiple);
void __cdecl LiveTicker_RemoveStream(streamName_t streamName);
char *__cdecl LiveTicker_GetCurrentMessage(
                float scrX,
                float baseX,
                float baseY,
                float baseAlpha,
                Font_s *const font,
                float scale,
                float *xCoord,
                float *yCoord,
                float *alpha);
char *__cdecl LiveTicker_GetCurrentMessageCategory(
                float baseAlpha,
                float baseX,
                float baseWidth,
                Font_s *font,
                float baseScale,
                float *outXCoord,
                float *outAlpha,
                float *outScale);
void __cdecl LiveTicker_SetBatchSize(streamName_t streamName, int batchSize);
void __cdecl LiveTicker_SetContractMsgsBatchSize_f();
void __cdecl LiveTicker_SetEventMsgsBatchSize_f();
void __cdecl LiveTicker_SetCODMsgsBatchSize_f();
void __cdecl LiveTicker_SetFriendMsgsBatchSize_f();
void __cdecl LiveTicker_SetClanMsgsBatchSize_f();
void __cdecl LiveTicker_DumpTickerInternal_f();
void __cdecl LiveTicker_Init();


extern const dvar_t *tickerWidescreenWidth;
extern const dvar_t *tickerStandardWidth;