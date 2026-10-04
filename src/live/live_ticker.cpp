#include "live_ticker.h"
#include "live_news.h"
#include <ui/ui_main.h>
#include <qcommon/common.h>
#include <cstring>
#include <win32/win_shared.h>

ticker_t s_ticker;
bool s_isTickerInitialized;

const dvar_t *tickerWidescreenWidth;
const dvar_t *tickerStandardWidth;

char *__cdecl LiveTicker_GetStreamNameTextInternal(streamName_t streamName)
{
    char *result; // eax

    switch ( streamName )
    {
        case TICKER_STREAM_COD:
            result = UI_SafeTranslateString("TICKER_MESSAGE_CATEGORY_COD_CAPS");
            break;
        case TICKER_STREAM_SQUADS:
            result = UI_SafeTranslateString("TICKER_MESSAGE_CATEGORY_CLAN_CAPS");
            break;
        case TICKER_STREAM_FRIENDS:
            result = UI_SafeTranslateString("TICKER_MESSAGE_CATEGORY_FRIENDS_CAPS");
            break;
        case TICKER_STREAM_EVENTS:
            result = UI_SafeTranslateString("TICKER_MESSAGE_CATEGORY_EVENTS_CAPS");
            break;
        case TICKER_STREAM_CONTRACTS:
            result = UI_SafeTranslateString("TICKER_MESSAGE_CATEGORY_CONTRACTS_CAPS");
            break;
        case TICKER_STREAM_NONE:
            result = UI_SafeTranslateString("TICKER_MESSAGE_CATEGORY_NONE_CAPS");
            break;
        default:
            result = (char *)"";
            break;
    }
    return result;
}

void __cdecl LiveTicker_DumpTickerInternal()
{
#ifdef BO1_LIVE_STUBS
    char *StreamNameTextInternal; // eax
    streamName_t j; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    Com_Printf(16, "**** Start Ticker Dump ****\n");
    Com_Printf(16, "Number of streams: %d\n", 5);
    Com_Printf(16, "Number of messages: %d\n", s_ticker.totalMessageCount);
    for ( i = 0; i < 5; ++i )
    {
        StreamNameTextInternal = LiveTicker_GetStreamNameTextInternal(s_ticker.streams[i].streamName);
        Com_Printf(
            16,
            "Stream %i (%d messages): %s\n",
            i,
            s_ticker.streamDisplayOrder[1293 * i - 5174],
            StreamNameTextInternal);
        for ( j = TICKER_STREAM_COD; j < s_ticker.streamDisplayOrder[1293 * i - 5174]; ++j )
            Com_Printf(16, "\t%d: %s\n", j, (const char *)(5172 * i + 516 * j + 174152212));
    }
    Com_Printf(16, "**** End Ticker Dump ****\n");
#endif
}

void __cdecl LiveTicker_SetDefaultDisplayOrder()
{
    int i; // [esp+0h] [ebp-4h]

    s_ticker.streamDisplayOrder[0] = TICKER_STREAM_COD;
    s_ticker.streamDisplayOrder[1] = TICKER_STREAM_CONTRACTS;
    s_ticker.streamDisplayOrder[2] = TICKER_STREAM_SQUADS;
    s_ticker.streamDisplayOrder[3] = TICKER_STREAM_EVENTS;
    s_ticker.streamDisplayOrder[4] = TICKER_STREAM_FRIENDS;
    for ( i = 0; i < 5; ++i )
        s_ticker.streamBatchSize[i] = 10;
}

void __cdecl LiveTicker_InitializeInternal()
{
    int i; // [esp+0h] [ebp-4h]

    for ( i = 0; i < 5; ++i )
    {
        s_ticker.streamDisplayOrder[1293 * i - 5174] = TICKER_STREAM_COD;
        s_ticker.streamDisplayOrder[i] = TICKER_STREAM_NONE;
        s_ticker.streams[i].streamName = TICKER_STREAM_NONE;
    }
    LiveTicker_SetDefaultDisplayOrder();
    s_ticker.currentStream = TICKER_STREAM_NONE;
    s_ticker.previousStream = TICKER_STREAM_NONE;
    s_ticker.totalMessageCount = 0;
    s_ticker.isHeaderAnimating = 0;
    s_ticker.pauseStartTimeA = 0;
    s_ticker.scrollLeftStartTime = 0;
    s_ticker.scrollUpStartTimeB = 0;
    s_ticker.pauseStartTimeB = 0;
    s_ticker.scrollLeftDuration = 0.0f;
    s_ticker.scrollLeftFinalX = 0.0f;
    s_ticker.hasMessageScrolledLeft = 0;
    s_ticker.scrollUpStartTimeA = 0;
    s_ticker.messagedWidthInPixels = 0;
    s_isTickerInitialized = 1;
    s_ticker.isUserOnline = 1;
}

void __cdecl LiveTicker_InsertMessageInternal(const char *text, streamName_t streamName)
{
#ifdef BO1_LIVE_STUBS
    if ( !s_isTickerInitialized )
        LiveTicker_InitializeInternal();
    if ( text && *text && I_strncmp(text, "\n", 4) && I_strncmp(text, "\r", 4) )
    {
        s_ticker.streams[streamName].streamName = streamName;
        if ( s_ticker.streamDisplayOrder[1293 * streamName - 5174] < 10 )
        {
            I_strncpyz(
                (char *)(5172 * streamName + 516 * s_ticker.streamDisplayOrder[1293 * streamName - 5174] + 174152212),
                text,
                strlen(text) + 1);
            ++s_ticker.streamDisplayOrder[1293 * streamName - 5174];
            ++s_ticker.totalMessageCount;
        }
    }
#endif
}

char __cdecl LiveTicker_IsStreamPopulated(streamName_t streamName)
{
    int i; // [esp+0h] [ebp-4h]

    for ( i = 0; i < 5; ++i )
    {
        if ( s_ticker.streams[i].streamName == streamName && s_ticker.streamDisplayOrder[1293 * i - 5174] )
            return 1;
    }
    return 0;
}

int __cdecl LiveTicker_GetTickerMessageTextWidth(const char *text, int maxChars, Font_s *font, float scale)
{
    float actualScale; // [esp+4h] [ebp-4h]

    actualScale = R_NormalizedTextScale(font, scale);
    return (int)(float)((float)R_TextWidth(text, maxChars, font) * actualScale);
}

void __cdecl LiveTicker_MoveToFirstStream()
{
    int i; // [esp+0h] [ebp-4h]

    if ( s_ticker.currentStream != TICKER_STREAM_NONE )
        s_ticker.streamDisplayOrder[1293 * s_ticker.currentStream
                                                            - 6336
                                                            + 129 * s_ticker.streamDisplayOrder[1293 * s_ticker.currentStream - 5173]] = TICKER_STREAM_COD;
    for ( i = 0; i < 5; ++i )
    {
        if ( s_ticker.streamDisplayOrder[1293 * s_ticker.streamDisplayOrder[i] - 5174] > TICKER_STREAM_COD )
        {
            s_ticker.currentStream = s_ticker.streamDisplayOrder[i];
            s_ticker.streamDisplayOrder[1293 * s_ticker.currentStream - 5173] = TICKER_STREAM_COD;
            return;
        }
    }
}

void __cdecl LiveTicker_MoveToNextStream()
{
    int j; // [esp+0h] [ebp-Ch]
    int k; // [esp+4h] [ebp-8h]
    int i; // [esp+8h] [ebp-4h]

    if ( s_ticker.currentStream == TICKER_STREAM_NONE )
    {
        for ( i = 0; i < 5; ++i )
        {
            if ( s_ticker.streamDisplayOrder[1293 * s_ticker.streamDisplayOrder[i] - 5174] > TICKER_STREAM_COD )
            {
                s_ticker.currentStream = s_ticker.streamDisplayOrder[i];
                s_ticker.streamDisplayOrder[1293 * s_ticker.currentStream - 5173] = TICKER_STREAM_COD;
                return;
            }
        }
    }
    else
    {
        s_ticker.previousStream = s_ticker.currentStream;
        for ( k = 0; k < 5; ++k )
        {
            if ( s_ticker.streamDisplayOrder[k] == s_ticker.currentStream )
            {
                for ( j = k + 1; j != k; ++j )
                {
                    if ( j == 5 )
                        j = 0;
                    s_ticker.currentStream = s_ticker.streamDisplayOrder[j];
                    if ( s_ticker.streamDisplayOrder[1293 * s_ticker.currentStream - 5174] > TICKER_STREAM_COD )
                        break;
                }
                return;
            }
        }
    }
}

void __cdecl LiveTicker_MoveToNextMessage(streamName_t streamName)
{
    streams_t *currStream; // [esp+0h] [ebp-4h]

    currStream = &s_ticker.streams[streamName];
    if ( ++currStream->currentMessageIndex < currStream->messageCount )
    {
        if ( !(currStream->currentMessageIndex % s_ticker.streamBatchSize[currStream->streamName]) )
            LiveTicker_MoveToNextStream();
    }
    else
    {
        currStream->currentMessageIndex = 0;
        LiveTicker_MoveToNextStream();
    }
}

void __cdecl LiveTicker_ReplaceMessages(const char *text, streamName_t streamName, bool isMultiple)
{
    s_ticker.totalMessageCount -= s_ticker.streamDisplayOrder[1293 * streamName - 5174];
    s_ticker.streamDisplayOrder[1293 * streamName - 5174] = TICKER_STREAM_COD;
    LiveTicker_AddMessages(text, streamName, isMultiple);
}

void __cdecl LiveTicker_AddMessages(const char *text, streamName_t streamName, bool isMultiple)
{
    char changingFileContents[512]; // [esp+10h] [ebp-210h] BYREF
    char *token; // [esp+214h] [ebp-Ch]
    int len; // [esp+218h] [ebp-8h]
    char *delimiter; // [esp+21Ch] [ebp-4h]

    if ( isMultiple )
    {
        len = strlen(text);
        delimiter = (char*)"\n\r";
        if ( len > 0 )
        {
            I_strncpyz(changingFileContents, text, 512);
            token = strtok(changingFileContents, delimiter);
            if ( streamName == TICKER_STREAM_COD )
                Dvar_SetStringByName("motd", token);
            while ( token )
            {
                LiveTicker_InsertMessageInternal(token, streamName);
                token = strtok(0, delimiter);
            }
        }
    }
    else
    {
        LiveTicker_InsertMessageInternal(text, streamName);
    }
}

void __cdecl LiveTicker_RemoveStream(streamName_t streamName)
{
    s_ticker.totalMessageCount -= s_ticker.streamDisplayOrder[1293 * streamName - 5174];
    s_ticker.streamDisplayOrder[1293 * streamName - 5174] = TICKER_STREAM_COD;
    LiveTicker_MoveToFirstStream();
}

char *__cdecl LiveTicker_GetCurrentMessage(
                float scrX,
                float baseX,
                float baseY,
                float baseAlpha,
                Font_s *const font,
                float scale,
                float *xCoord,
                float *yCoord,
                float *alpha)
{
    char *result; // eax
    float delta; // [esp+30h] [ebp-18h]
    float deltaa; // [esp+30h] [ebp-18h]
    messages_t *currMessage; // [esp+34h] [ebp-14h]
    streams_t *currStream; // [esp+38h] [ebp-10h]
    float moveBy; // [esp+3Ch] [ebp-Ch]
    signed int currentTime; // [esp+40h] [ebp-8h]
    int xThreshold; // [esp+44h] [ebp-4h]

    currentTime = Sys_Milliseconds();
    if ( Dvar_GetInt("ticker_menu_context") > 0 )
    {
        Dvar_GetInt("ticker_menu_context");
        Dvar_SetIntByName("ticker_menu_context", -1);
    }

#ifdef BO1_LIVE_SERVICE
    if ( com_wideScreen && com_wideScreen->current.enabled )
        xThreshold = tickerWidescreenWidth->current.integer;
    else
        xThreshold = tickerStandardWidth->current.integer;
#else
    xThreshold = 405; // widescreen (HACK, these dvars are null)
#endif
    if ( s_isTickerInitialized && s_ticker.totalMessageCount >= 1 )
    {
        if ( s_ticker.isHeaderAnimating )
        {
            *xCoord = baseX;
            *yCoord = baseY;
            *alpha = baseAlpha;
            return (char *)"";
        }
        else
        {
            if ( s_ticker.currentStream == TICKER_STREAM_NONE )
                LiveTicker_MoveToNextStream();
            currStream = &s_ticker.streams[s_ticker.currentStream];
            currMessage = &currStream->messages[currStream->currentMessageIndex];
            switch ( currStream->messages[currStream->currentMessageIndex].state )
            {
                case MESSAGE_NOT_DISPLAYING:
                    s_ticker.isHeaderAnimating = 0;
                    s_ticker.pauseStartTimeA = 0;
                    s_ticker.scrollLeftStartTime = 0;
                    s_ticker.scrollUpStartTimeB = 0;
                    s_ticker.pauseStartTimeB = 0;
                    s_ticker.scrollLeftDuration = 0.0f;
                    s_ticker.scrollLeftFinalX = 0.0f;
                    s_ticker.hasMessageScrolledLeft = 0;
                    s_ticker.scrollUpStartTimeA = Sys_Milliseconds();
                    s_ticker.messagedWidthInPixels = LiveTicker_GetTickerMessageTextWidth(currMessage->text, 0, font, scale);
                    currMessage->state = MESSAGE_SCROLLING_UP_A;
                    goto $LN16_68;
                case MESSAGE_SCROLLING_UP_A:
$LN16_68:
                    delta = (float)(currentTime - s_ticker.scrollUpStartTimeA) / 250.0;
                    *yCoord = (float)(8.0 - (float)(8.0 * delta)) + baseY;
                    *alpha = delta;
                    *xCoord = baseX;
                    if ( (float)(currentTime - s_ticker.scrollUpStartTimeA) > 250.0 )
                    {
                        currMessage->state = MESSAGE_PAUSED_A;
                        s_ticker.pauseStartTimeA = currentTime;
                    }
                    break;
                case MESSAGE_PAUSED_A:
                    *xCoord = baseX;
                    *yCoord = baseY;
                    *alpha = baseAlpha;
                    if ( (float)(currentTime - s_ticker.pauseStartTimeA) > 5000.0 )
                    {
                        if ( s_ticker.messagedWidthInPixels <= xThreshold )
                        {
                            currMessage->state = MESSAGE_SCROLLING_UP_B;
                            s_ticker.scrollUpStartTimeB = currentTime;
                        }
                        else
                        {
                            currMessage->state = MESSAGE_SCROLLING_LEFT;
                            s_ticker.scrollLeftStartTime = currentTime;
                            s_ticker.scrollLeftDuration = (float)(s_ticker.messagedWidthInPixels - xThreshold) / 0.039999999;
                        }
                    }
                    break;
                case MESSAGE_SCROLLING_LEFT:
                    moveBy = (float)(s_ticker.messagedWidthInPixels - xThreshold)
                                 * (float)((float)((float)currentTime - (float)s_ticker.scrollLeftStartTime)
                                                 / s_ticker.scrollLeftDuration);
                    *yCoord = baseY;
                    *alpha = baseAlpha;
                    *xCoord = baseX - moveBy;
                    if ( (float)(currentTime - s_ticker.scrollLeftStartTime) > s_ticker.scrollLeftDuration )
                    {
                        currMessage->state = MESSAGE_PAUSED_B;
                        s_ticker.pauseStartTimeB = currentTime;
                        s_ticker.scrollLeftFinalX = *xCoord;
                        s_ticker.hasMessageScrolledLeft = 1;
                    }
                    break;
                case MESSAGE_PAUSED_B:
                    *xCoord = s_ticker.scrollLeftFinalX;
                    *yCoord = baseY;
                    *alpha = baseAlpha;
                    if ( (float)(currentTime - s_ticker.pauseStartTimeB) > 3000.0 )
                    {
                        currMessage->state = MESSAGE_SCROLLING_UP_B;
                        s_ticker.scrollUpStartTimeB = currentTime;
                    }
                    break;
                case MESSAGE_SCROLLING_UP_B:
                    deltaa = (float)(currentTime - s_ticker.scrollUpStartTimeB) / 250.0;
                    *yCoord = baseY - (float)(8.0 * deltaa);
                    *alpha = 1.0 - deltaa;
                    if ( s_ticker.hasMessageScrolledLeft )
                        *xCoord = s_ticker.scrollLeftFinalX;
                    else
                        *xCoord = baseX;
                    if ( (float)(currentTime - s_ticker.scrollUpStartTimeB) > 250.0 )
                        currMessage->state = MESSAGE_DONE_DISPLAYING;
                    break;
                case MESSAGE_DONE_DISPLAYING:
                    s_ticker.streamDisplayOrder[1293 * s_ticker.currentStream
                                                                        - 6336
                                                                        + 129 * s_ticker.streamDisplayOrder[1293 * s_ticker.currentStream - 5173]] = TICKER_STREAM_COD;
                    LiveTicker_MoveToNextMessage(currStream->streamName);
                    break;
                default:
                    break;
            }
            if ( currMessage->state )
                return (char *)currMessage;
            else
                return (char *)"";
        }
    }
    else
    {
        result = UI_SafeTranslateString("TICKER_MESSAGE_DEFAULT_OFFLINE");
        *xCoord = baseX;
        *yCoord = baseY;
        *alpha = baseAlpha;
    }
    return result;
}

char *__cdecl LiveTicker_GetCurrentMessageCategory(
                float baseAlpha,
                float baseX,
                float baseWidth,
                Font_s *font,
                float baseScale,
                float *outXCoord,
                float *outAlpha,
                float *outScale)
{
    streams_t *currStream; // [esp+1Ch] [ebp-Ch]
    unsigned int curTime; // [esp+20h] [ebp-8h]
    char *text; // [esp+24h] [ebp-4h]
    char *texta; // [esp+24h] [ebp-4h]

    text = UI_SafeTranslateString("TICKER_MESSAGE_CATEGORY_COD_CAPS");
    curTime = Sys_Milliseconds();
    if ( s_ticker.totalMessageCount >= 1 )
    {
        currStream = &s_ticker.streams[s_ticker.currentStream];
        if ( currStream->streamName != s_ticker.previousStream && !s_ticker.headerAnimationStartTime )
        {
            s_ticker.isHeaderAnimating = 1;
            s_ticker.headerAnimationStartTime = curTime;
        }
        if ( s_ticker.headerAnimationStartTime <= 0 )
        {
            texta = LiveTicker_GetStreamNameTextInternal(currStream->streamName);
            *outAlpha = 1.0f;
            *outScale = baseScale;
        }
        else if ( (int)(curTime - s_ticker.headerAnimationStartTime) >= 100 )
        {
            if ( (int)(curTime - s_ticker.headerAnimationStartTime) >= 200 )
            {
                texta = LiveTicker_GetStreamNameTextInternal(currStream->streamName);
                s_ticker.previousStream = currStream->streamName;
                s_ticker.isHeaderAnimating = 0;
                s_ticker.headerAnimationStartTime = 0;
                *outAlpha = 1.0f;
                *outScale = baseScale;
            }
            else
            {
                texta = LiveTicker_GetStreamNameTextInternal(currStream->streamName);
                *outAlpha = (float)(int)(curTime - (s_ticker.headerAnimationStartTime + 100)) / 200.0;
                *outScale = baseScale * *outAlpha;
            }
        }
        else
        {
            texta = LiveTicker_GetStreamNameTextInternal(s_ticker.previousStream);
            *outAlpha = 1.0 - (float)((float)(int)(curTime - s_ticker.headerAnimationStartTime) / 100.0);
            *outScale = baseScale * *outAlpha;
        }
        *outXCoord = baseX;
        return texta;
    }
    else
    {
        *outAlpha = baseAlpha;
        *outScale = baseScale;
        *outXCoord = baseX;
        return text;
    }
}

void __cdecl LiveTicker_SetBatchSize(streamName_t streamName, int batchSize)
{
    s_ticker.streamBatchSize[streamName] = batchSize;
}

void __cdecl LiveTicker_SetContractMsgsBatchSize_f()
{
    const char *v0; // eax
    int v1; // eax

    if ( Cmd_Argc() == 2 )
    {
        v0 = Cmd_Argv(1);
        v1 = atoi(v0);
        LiveTicker_SetBatchSize(TICKER_STREAM_CONTRACTS, v1);
    }
    else
    {
        Com_Printf(0, "USAGE: setTickerContractsBatchSize <no. of messages in a batch>\n");
    }
}

void __cdecl LiveTicker_SetEventMsgsBatchSize_f()
{
    const char *v0; // eax
    int v1; // eax

    if ( Cmd_Argc() == 2 )
    {
        v0 = Cmd_Argv(1);
        v1 = atoi(v0);
        LiveTicker_SetBatchSize(TICKER_STREAM_EVENTS, v1);
    }
    else
    {
        Com_Printf(0, "USAGE: setTickerEventsBatchSize <no. of messages in a batch>\n");
    }
}

void __cdecl LiveTicker_SetCODMsgsBatchSize_f()
{
    const char *v0; // eax
    int v1; // eax

    if ( Cmd_Argc() == 2 )
    {
        v0 = Cmd_Argv(1);
        v1 = atoi(v0);
        LiveTicker_SetBatchSize(TICKER_STREAM_COD, v1);
    }
    else
    {
        Com_Printf(0, "USAGE: setTickerCODMsgsBatchSize <no. of messages in a batch>\n");
    }
}

void __cdecl LiveTicker_SetFriendMsgsBatchSize_f()
{
    const char *v0; // eax
    int v1; // eax

    if ( Cmd_Argc() == 2 )
    {
        v0 = Cmd_Argv(1);
        v1 = atoi(v0);
        LiveTicker_SetBatchSize(TICKER_STREAM_FRIENDS, v1);
    }
    else
    {
        Com_Printf(0, "USAGE: setTickerFriendMsgsBatchSize <no. of messages in a batch>\n");
    }
}

void __cdecl LiveTicker_SetClanMsgsBatchSize_f()
{
    const char *v0; // eax
    int v1; // eax

    if ( Cmd_Argc() == 2 )
    {
        v0 = Cmd_Argv(1);
        v1 = atoi(v0);
        LiveTicker_SetBatchSize(TICKER_STREAM_SQUADS, v1);
    }
    else
    {
        Com_Printf(0, "USAGE: setTickerClanMsgsBatchSize <no. of messages in a batch>\n");
    }
}

void __cdecl LiveTicker_DumpTickerInternal_f()
{
    if ( Cmd_Argc() == 1 )
        LiveTicker_DumpTickerInternal();
    else
        Com_Printf(0, "USAGE: dumpTickerContents\n");
}

cmd_function_s LiveTicker_SetContractMsgsBatchSize_f_VAR;
cmd_function_s LiveTicker_SetEventMsgsBatchSize_f_VAR;
cmd_function_s LiveTicker_SetCODMsgsBatchSize_f_VAR;
cmd_function_s LiveTicker_SetFriendMsgsBatchSize_f_VAR;
cmd_function_s LiveTicker_SetClanMsgsBatchSize_f_VAR;
cmd_function_s LiveTicker_DumpTickerInternal_f_VAR;

void __cdecl LiveTicker_Init()
{
    Cmd_AddCommandInternal(
        "setTickerContractsMsgsBatchSize",
        LiveTicker_SetContractMsgsBatchSize_f,
        &LiveTicker_SetContractMsgsBatchSize_f_VAR);
    Cmd_AddCommandInternal(
        "setTickerEventMsgsBatchSize",
        LiveTicker_SetEventMsgsBatchSize_f,
        &LiveTicker_SetEventMsgsBatchSize_f_VAR);
    Cmd_AddCommandInternal(
        "setTickerCODMsgsBatchSize",
        LiveTicker_SetCODMsgsBatchSize_f,
        &LiveTicker_SetCODMsgsBatchSize_f_VAR);
    Cmd_AddCommandInternal(
        "setTickerFriendMsgsBatchSize",
        LiveTicker_SetFriendMsgsBatchSize_f,
        &LiveTicker_SetFriendMsgsBatchSize_f_VAR);
    Cmd_AddCommandInternal(
        "setTickerClanMsgsBatchSize",
        LiveTicker_SetClanMsgsBatchSize_f,
        &LiveTicker_SetClanMsgsBatchSize_f_VAR);
    Cmd_AddCommandInternal("dumpTickerContents", LiveTicker_DumpTickerInternal_f, &LiveTicker_DumpTickerInternal_f_VAR);
    tickerWidescreenWidth = _Dvar_RegisterInt(
                                                        "tickerWidescreenWidth",
                                                        405,
                                                        0,
                                                        0x7FFFFFFF,
                                                        0,
                                                        "Ticker message text widescreen width.");
    tickerStandardWidth = _Dvar_RegisterInt(
                                                    "tickerStandardWidth",
                                                    300,
                                                    0,
                                                    0x7FFFFFFF,
                                                    0,
                                                    "Ticker message text standard width.");
}

