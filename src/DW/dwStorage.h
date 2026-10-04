#pragma once

enum bdLobbyErrorCode : __int32
{                                       // XREF: bdLobbyConnection::Status/r
                                        // bdConnection::Status/r ...
    BD_NO_ERROR = 0x0,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_TOO_MANY_TASKS = 0x1,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_NOT_CONNECTED = 0x2,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_SEND_FAILED = 0x3,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_HANDLE_TASK_FAILED = 0x4,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_START_TASK_FAILED = 0x5,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_RESULT_EXCEEDS_BUFFER_SIZE = 0x64,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_ACCESS_DENIED = 0x65,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_EXCEPTION_IN_DB = 0x66,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_MALFORMED_TASK_HEADER = 0x67,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_INVALID_ROW = 0x68,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_EMPTY_ARG_LIST = 0x69,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_PARAM_PARSE_ERROR = 0x6A,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_PARAM_MISMATCHED_TYPE = 0x6B,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_SERVICE_NOT_AVAILABLE = 0x6C,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_CONNECTION_RESET = 0x6D,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_INVALID_USER_ID = 0x6E,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_LOBBY_PROTOCOL_VERSION_FAILURE = 0x6F,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_LOBBY_INTERNAL_FAILURE = 0x70,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_LOBBY_PROTOCOL_ERROR = 0x71,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_ASYNCHRONOUS_ERROR = 0xC8,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_MEMBER_NO_PROPOSAL = 0x12C,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_TEAMNAME_ALREADY_EXISTS = 0x12D,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_MAX_TEAM_MEMBERSHIPS_LIMITED = 0x12E,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_MAX_TEAM_OWNERSHIPS_LIMITED = 0x12F,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_NOT_A_TEAM_MEMBER = 0x130,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_INVALID_TEAM_ID = 0x131,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_INVALID_TEAM_NAME = 0x132,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_NOT_A_TEAM_OWNER = 0x133,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_NOT_AN_ADMIN_OR_OWNER = 0x134,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_MEMBER_PROPOSAL_EXISTS = 0x135,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_MEMBER_EXISTS = 0x136,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_TEAM_FULL = 0x137,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_VULGAR_TEAM_NAME = 0x138,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_TEAM_USERID_BANNED = 0x139,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_TEAM_EMPTY = 0x13A,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_INVALID_TEAM_PROFILE_QUERY_ID = 0x13B,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_INVALID_LEADERBOARD_ID = 0x190,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_INVALID_STATS_SET = 0x191,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_EMPTY_STATS_SET_IGNORED = 0x193,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_NO_DIRECT_ACCESS_TO_ARBITRATED_LBS = 0x194,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_STATS_WRITE_PERMISSION_DENIED = 0x195,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_STATS_WRITE_TYPE_DATA_TYPE_MISMATCH = 0x196,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_NO_STATS_FOR_USER = 0x197,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_INVALID_ACCESS_TO_UNRANKED_LB = 0x198,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_INVALID_TITLE_ID = 0x1F4,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_MESSAGING_INVALID_MAIL_ID = 0x258,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_SELF_BLOCK_NOT_ALLOWED = 0x259,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_GLOBAL_MESSAGE_ACCESS_DENIED = 0x25A,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_AUTH_NO_ERROR = 0x2BC,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_AUTH_BAD_REQUEST = 0x2BD,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_AUTH_SERVER_CONFIG_ERROR = 0x2BE,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_AUTH_BAD_TITLE_ID = 0x2BF,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_AUTH_BAD_ACCOUNT = 0x2C0,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_AUTH_ILLEGAL_OPERATION = 0x2C1,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_AUTH_INCORRECT_LICENSE_CODE = 0x2C2,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_AUTH_CREATE_USERNAME_EXISTS = 0x2C3,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_AUTH_CREATE_USERNAME_ILLEGAL = 0x2C4,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_AUTH_CREATE_USERNAME_VULGAR = 0x2C5,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_AUTH_CREATE_MAX_ACC_EXCEEDED = 0x2C6,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_AUTH_MIGRATE_NOT_SUPPORTED = 0x2C7,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_AUTH_TITLE_DISABLED = 0x2C8,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_AUTH_ACCOUNT_EXPIRED = 0x2C9,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_AUTH_ACCOUNT_LOCKED = 0x2CA,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_AUTH_UNKNOWN_ERROR = 0x2CB,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_AUTH_INCORRECT_PASSWORD = 0x2CC,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_AUTH_IP_NOT_IN_ALLOWED_RANGE = 0x2CD,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_AUTH_WII_TOKEN_VERIFICATION_FAILED = 0x2CE,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_AUTH_WII_AUTHENTICATION_FAILED = 0x2CF,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_AUTH_IP_KEY_LIMIT_REACHED = 0x2D0,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_NO_PROFILE_INFO_EXISTS = 0x320,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_FRIENDSHIP_NOT_REQUSTED = 0x384,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_NOT_A_FRIEND = 0x385,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_SELF_FRIENDSHIP_NOT_ALLOWED = 0x387,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_FRIENDSHIP_EXISTS = 0x388,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_PENDING_FRIENDSHIP_EXISTS = 0x389,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_USERID_BANNED = 0x38A,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_FRIENDS_FULL = 0x38C,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_FRIENDS_NO_RICH_PRESENCE = 0x38D,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_RICH_PRESENCE_TOO_LARGE = 0x38E,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_NO_FILE = 0x3E8,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_PERMISSION_DENIED = 0x3E9,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_FILESIZE_LIMIT_EXCEEDED = 0x3EA,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_FILENAME_MAX_LENGTH_EXCEEDED = 0x3EB,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_CHANNEL_DOES_NOT_EXIST = 0x44D,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_CHANNEL_ALREADY_SUBSCRIBED = 0x44E,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_CHANNEL_NOT_SUBSCRIBED = 0x44F,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_CHANNEL_FULL = 0x450,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_CHANNEL_SUBSCRIPTIONS_FULL = 0x451,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_CHANNEL_NO_SELF_WHISPERING = 0x452,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_EVENT_DESC_TRUNCATED = 0x4B0,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_CONTENT_UNLOCK_UNKNOWN_ERROR = 0x514,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_UNLOCK_KEY_INVALID = 0x515,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_UNLOCK_KEY_ALREADY_USED_UP = 0x516,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_KEY_ARCHIVE_INVALID_WRITE_TYPE = 0x5DC,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_BANDWIDTH_TEST_TRY_AGAIN = 0x712,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_BANDWIDTH_TEST_STILL_IN_PROGRESS = 0x713,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_BANDWIDTH_TEST_NOT_PROGRESS = 0x714,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_BANDWIDTH_TEST_SOCKET_ERROR = 0x715,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_INVALID_SESSION_NONCE = 0x76D,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_ARBITRATION_FAILURE = 0x76F,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_ARBITRATION_USER_NOT_REGISTERED = 0x771,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_ARBITRATION_NOT_CONFIGURED = 0x772,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_CONTENTSTREAMING_FILE_NOT_AVAILABLE = 0x7D0,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_CONTENTSTREAMING_STORAGE_SPACE_EXCEEDED = 0x7D1,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_CONTENTSTREAMING_NUM_FILES_EXCEEDED = 0x7D2,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_CONTENTSTREAMING_UPLOAD_BANDWIDTH_EXCEEDED = 0x7D3,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_CONTENTSTREAMING_FILENAME_MAX_LENGTH_EXCEEDED = 0x7D4,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_CONTENTSTREAMING_MAX_THUMB_DATA_SIZE_EXCEEDED = 0x7D5,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_CONTENTSTREAMING_DOWNLOAD_BANDWIDTH_EXCEEDED = 0x7D6,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_CONTENTSTREAMING_NOT_ENOUGH_DOWNLOAD_BUFFER_SPACE = 0x7D7,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_CONTENTSTREAMING_SERVER_NOT_CONFIGURED = 0x7D8,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_CONTENTSTREAMING_INVALID_APPLE_RECEIPT = 0x7DA,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_CONTENTSTREAMING_APPLE_STORE_NOT_AVAILABLE = 0x7DB,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_CONTENTSTREAMING_APPLE_RECEIPT_FILENAME_MISMATCH = 0x7DC,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_CONTENTSTREAMING_HTTP_ERROR = 0x7E4,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_CONTENTSTREAMING_FAILED_TO_START_HTTP = 0x7E5,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_VOTERANK_ERROR_EMPTY_RATING_SUBMISSION = 0x7EE,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_VOTERANK_ERROR_MAX_VOTES_EXCEEDED = 0x7EF,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_VOTERANK_ERROR_INVALID_RATING = 0x7F0,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_MAX_NUM_TAGS_EXCEEDED = 0x82A,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_TAGGED_COLLECTION_DOES_NOT_EXIST = 0x82B,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_INVALID_QUERY_ID = 0x834,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_NO_ENTRY_TO_UPDATE = 0x835,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_SESSION_INVITE_EXISTS = 0x836,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_INVALID_SESSION_ID = 0x837,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_ATTACHMENT_TOO_LARGE = 0x838,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_INVALID_GROUP_ID = 0xAF0,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_TWITTER_AUTH_ATTEMPT_FAILED = 0xDAD,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_TWITTER_AUTH_TOKEN_INVALID = 0xDAE,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_TWITTER_UPDATE_LIMIT_REACHED = 0xDAF,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_TWITTER_UNAVAILABLE = 0xDB0,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_TWITTER_ERROR = 0xDB1,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_TWITTER_TIMED_OUT = 0xDB2,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_TWITTER_DISABLED_FOR_USER = 0xDB3,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_TWITTER_ACCOUNT_AMBIGUOUS = 0xDB4,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_TWITTER_MAXIMUM_ACCOUNTS_REACHED = 0xDB5,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_FACEBOOK_AUTH_ATTEMPT_FAILED = 0xE11,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_FACEBOOK_AUTH_TOKEN_INVALID = 0xE12,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_FACEBOOK_PHOTO_DOES_NOT_EXIST = 0xE13,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_FACEBOOK_PHOTO_INVALID = 0xE14,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_FACEBOOK_PHOTO_ALBUM_FULL = 0xE15,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_FACEBOOK_UNAVAILABLE = 0xE16,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_FACEBOOK_ERROR = 0xE17,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_FACEBOOK_TIMED_OUT = 0xE18,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_FACEBOOK_DISABLED_FOR_USER = 0xE19,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_FACEBOOK_ACCOUNT_AMBIGUOUS = 0xE1A,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_FACEBOOK_MAXIMUM_ACCOUNTS_REACHED = 0xE1B,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_APNS_USER_NOT_REGISTERED = 0xE74,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_APNS_INVALID_PAYLOAD = 0xE75,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_CRUX_ERROR = 0xF3D,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_CRUX_EMAIL_PASSWORD_INVALID = 0xF3E,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_CRUX_EMAIL_INVALID = 0xF3F,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_CRUX_BIRTH_DATE_INVALID = 0xF40,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_CRUX_BIRTH_DATE_NOT_ALLOWED = 0xF41,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_CRUX_PASSWORD_INVALID = 0xF42,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_CRUX_PLATFORM_ID_INVALID = 0xF43,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_CRUX_PLATFORM_UID_EXISTS = 0xF44,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_CRUX_COMMUNITY_ID_INVALID = 0xF45,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_CRUX_COMMUNITY_USERNAME_INVALID = 0xF46,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_CRUX_COMMUNITY_USERNAME_EXISTS = 0xF47,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_CRUX_TITLE_ID_INVALID = 0xF48,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_CRUX_TITLE_UID_EXISTS = 0xF49,
    // XREF: .rdata:lobbyErrorCodeLookup/s
    BD_MAX_ERROR_CODE = 0xF4A,
    // XREF: .rdata:lobbyErrorCodeLookup/s
};

#include <live/live_storage.h>
#include <demo/demo_common.h>

struct bdFileData : bdTaskResult // sizeof=0xC
{                                       // XREF: dwFileTask/r
    void *m_fileData;
    unsigned int m_fileSize;
};

struct __declspec(align(8)) bdFileInfo : bdTaskResult // sizeof=0xB0
{                                       // XREF: dwFileTask/r
    enum bdVisibility : __int32
    {                                       // XREF: bdFileInfo::bdVisibility/r
        BD_VISIBLE_PUBLIC = 0x0,
        BD_VISIBLE_PRIVATE = 0x1,
        BD_MAX_VISIBILITY_TYPE = 0x2,
    };
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    unsigned __int64 m_fileID;
    unsigned int m_createTime;
    unsigned int m_modifedTime;
    bdFileInfo::bdVisibility m_visibility;
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    unsigned __int64 m_ownerID;
    char m_fileName[128];
    unsigned int m_fileSize;
    // padding byte
    // padding byte
    // padding byte
    // padding byte
};

struct __declspec(align(4)) dwFileTask // sizeof=0xE0
{                                       // XREF: .data:s_fetchPlaylistsFileTask/r
    dwFileTask()
    {
        this->m_filename = 0;
        //bdFileData::bdFileData(&this->m_fileData, 0, 0);
        this->m_error = BD_NO_ERROR;
        this->m_fileID = 0;
        this->m_fileSize = 0;
        this->m_bufferSize = 0;
        //bdFileInfo::bdFileInfo(&this->m_fileInfo);
        this->m_buffer = 0;
        this->m_optional = 0;
    }

    char *m_filename;                   // XREF: SV_AP_GetControlFile(void)+31/w
    // LiveStorage_FetchOnlineWAD(int)+16A/w
    bdFileData m_fileData;              // XREF: _dynamic_atexit_destructor_for__s_finfo__+D/o
    // _dynamic_atexit_destructor_for__s_onlineWADFileInfo__+D/o ...
    bdLobbyErrorCode m_error;
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    unsigned __int64 m_fileID;
    unsigned int m_fileSize;
    unsigned int m_bufferSize;
    bdFileInfo m_fileInfo;              // XREF: _dynamic_atexit_destructor_for__s_finfo__+3/o
    // _dynamic_atexit_destructor_for__s_onlineWADFileInfo__+3/o ...
    void *m_buffer;
    bool m_optional;
    // padding byte
    // padding byte
    // padding byte
};
//dwFileTask *__thiscall dwFileTask::dwFileTask(dwFileTask *this);

#ifdef BO1_DW

struct TaskRecord;

void __cdecl dwReadFileFailure(TaskRecord *task);
void __cdecl dwReadFileSuccess(TaskRecord *task);
TaskRecord *__cdecl dwReadFile(int localControllerIndex, dwFileTask *const fileTask);
TaskRecord *__cdecl dwReadFileInternal(int localControllerIndex, dwFileTask *const fileTask, unsigned __int64 ownerUID);
TaskRecord *__cdecl dwReadUserFile(int localControllerIndex, dwFileTask *const fileTask);
TaskRecord *__cdecl dwReadUserFileByUserID(int localControllerIndex, dwFileTask *const fileTask, unsigned __int64 xuid);
void __cdecl dwDeleteFileFailure(TaskRecord *task);
TaskRecord *__cdecl dwDeleteFile(int localControllerIndex, char *filename);
void __cdecl dwWriteFileFailure(TaskRecord *task);
TaskRecord *__cdecl dwWriteFile(
                int localControllerIndex,
                char *filename,
                unsigned __int8 *data,
                unsigned int dataSize,
                bdFileInfo *fileInfo,
                unsigned __int64 ownerID);
TaskRecord *__cdecl dwFileShareGetListing(int localControllerIndex, dwFileShareListTask *const fsTask);
TaskRecord *__cdecl dwFileShareGetRating(int localControllerIndex, dwFileShareGetRatingTask *const fsTask);
TaskRecord *__cdecl dwFileShareGetTopRated(int localControllerIndex, dwFileShareGetTopRatedTask *const fsTask);
char __cdecl dwFileShareLocationToCollectionID(fileShareLocation location, unsigned int *collectionID);
TaskRecord *__cdecl dwFileShareSearch(int localControllerIndex, dwFileShareSearchTask *const fsTask);
TaskRecord *__cdecl dwFileShareGetDescriptors(int localControllerIndex, dwFileShareDescriptorsTask *const fsTask);
TaskRecord *__cdecl dwFileShareGetDescriptorsFromPooledStorage(
                int localControllerIndex,
                dwFileShareDescriptorsTask *const fsTask);
TaskRecord *__cdecl dwFileShareGetDescriptorsFromUserStorage(
                int localControllerIndex,
                dwFileShareDescriptorsTask *const fsTask);
TaskRecord *__cdecl dwFileShareReadFile(int controllerIndex, dwFileShareReadFileTask *const fsTask);
TaskRecord *__cdecl dwFileShareReadPooledStorageFile(int controllerIndex, dwFileShareReadFileTask *const fsTask);
TaskRecord *__cdecl dwFileShareReadUserStorageFile(int controllerIndex, dwFileShareReadFileTask *const fsTask);
TaskRecord *__cdecl dwFileShareRemoveFile(int controllerIndex, dwFileShareRemoveTask *const fsTask);
TaskRecord *__cdecl dwFileShareTransferFile(int controllerIndex, dwFileShareTransferTask *const fsTask);
TaskRecord *__cdecl dwFileShareGetSummary(int controllerIndex, dwFileShareSummaryTask *const fsTask);
TaskRecord *__cdecl dwFileShareSetPooledSummary(int controllerIndex, dwFileShareSummaryTask *const fsTask);
TaskRecord *__cdecl dwFileShareSetUserSummary(int controllerIndex, dwFileShareSummaryTask *const fsTask);
TaskRecord *__cdecl dwFileShareSetSummary(int controllerIndex, dwFileShareSummaryTask *const fsTask);
TaskRecord *__cdecl dwFileShareWriteFile(int controllerIndex, dwFileShareWriteFileTask *const fsTask);
TaskRecord *__cdecl dwFileShareWritePooledStorageFile(int controllerIndex, dwFileShareWriteFileTask *const fsTask);
TaskRecord *__cdecl dwFileShareWriteUserStorageFile(int controllerIndex, dwFileShareWriteFileTask *const fsTask);
void __cdecl dwFileShareGetProgress(
                int localControllerIndex,
                unsigned int *bytes,
                float *dataRate,
                fileShareLocation location);
void __cdecl dwFileShareGetProgressPooled(int localControllerIndex, unsigned int *bytes, float *dataRate);
void __cdecl dwFileShareGetProgressUser(int localControllerIndex, unsigned int *bytes, float *dataRate);
void __cdecl dwFileShareAbortOperation(int localControllerIndex, fileShareLocation location);
void __cdecl dwFileShareAbortOperationPooled(int localControllerIndex);
void __cdecl dwFileShareAbortOperationUser(int localControllerIndex);
unsigned int __cdecl dwFileShareGetLastHTTPError(int localControllerIndex, fileShareLocation location);
unsigned int __cdecl dwFileShareGetLastHTTPErrorPooled(int localControllerIndex);
unsigned int __cdecl dwFileShareGetLastHTTPErrorUser(int localControllerIndex);
TaskRecord *__cdecl dwFileShareSubmitRating(int localControllerIndex, dwFileShareSubmitRatingTask *const fsTask);

#endif