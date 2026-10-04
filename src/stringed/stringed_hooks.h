#pragma once

#include <universal/dvar.h>

enum msgLocErrType_t : __int32
{                                       // XREF: ?SEH_LocalizeTextMessage@@YAPBDPBD0W4msgLocErrType_t@@@Z/r
                                        // SEH_GetLocalizedTokenReference/r
    LOCMSG_SAFE  = 0x0,
    LOCMSG_NOERR = 0x1,
};

struct LocalizeEntry // sizeof=0x8
{                                       // XREF: XAssetPoolEntry<LocalizeEntry>/r
    const char *value;
    const char *name;
};

template <typename T>
struct EXT_Link//<LocalizeEntry> // sizeof=0xC
{
    T *m_Data;
    EXT_Link<T> *m_Next;
    EXT_Link<T> *m_Last;
};

template <typename T>
struct EXT_List//<LocalizeEntry> // sizeof=0xC
{                                       // XREF: .data:EXT_List<LocalizeEntry> overrideLocalizedEntryList/r
    unsigned int m_Count;
    EXT_Link<T> *m_Head;    // XREF: ProcessStringEdCmds(void)+70/r
                                        // GetString(char const *)+17/r
    EXT_Link<T> *m_Tail;

public:

    ~EXT_List()
    {
        Empty(0);
    }

    EXT_List()
    {
        m_Count = 0;
        m_Head = 0;
        m_Tail = 0;
    }

    bool Remove(EXT_Link<T> *Link, bool DeleteData)
    {
        if (Link->m_Last && Link->m_Next)
        {
            Link->m_Last->m_Next = Link->m_Next;
            Link->m_Next->m_Last = Link->m_Last;
        }
        else if (Link == this->m_Head && Link == this->m_Tail)
        {
            this->m_Head = 0;
            this->m_Tail = 0;
        }
        else if (Link == this->m_Head)
        {
            Link->m_Next->m_Last = 0;
            this->m_Head = Link->m_Next;
        }
        else if (Link == this->m_Tail)
        {
            Link->m_Last->m_Next = 0;
            this->m_Tail = Link->m_Last;
        }
        if (DeleteData)
        {
            if (Link->m_Data)
                operator delete(Link->m_Data);
            Link->m_Data = 0;
        }
        if (Link)
            operator delete(Link);
        --this->m_Count;
        return 1;
    }

    void Empty(bool DeleteData)
    {
        while (this->m_Tail)
            Remove(this->m_Tail, DeleteData);
    }

    EXT_Link<T> *Add(T *m_Data,bool First)
    {
        EXT_Link<T> *v4; // [esp+0h] [ebp-10h]
        EXT_Link<T> *v6; // [esp+8h] [ebp-8h]

        v6 = (EXT_Link<T> *)operator new(sizeof(T));
        if (v6)
        {
            v6->m_Next = 0;
            v6->m_Last = 0;
            v6->m_Data = 0;
            v4 = v6;
        }
        else
        {
            v4 = 0;
        }
        if (!v4)
            return 0;
        v4->m_Data = m_Data;
        if (this->m_Head && this->m_Tail)
        {
            if (First)
            {
                this->m_Head->m_Last = v4;
                v4->m_Next = this->m_Head;
                this->m_Head = v4;
            }
            else
            {
                this->m_Tail->m_Next = v4;
                v4->m_Last = this->m_Tail;
                this->m_Tail = v4;
            }
        }
        else
        {
            this->m_Head = v4;
            this->m_Tail = v4;
        }
        ++this->m_Count;
        return v4;
    }

};

struct languageInfo_t // sizeof=0xC
{                                       // XREF: .data:g_languages/r
    const char *pszName;                // XREF: SEH_UpdateCurrentLanguage+62/r
    const char *pszNameAbbr;            // XREF: SEH_GetLanguageNameAbbr(int):loc_72500F/r
    int bPresent;                       // XREF: SEH_UpdateLanguageInfo(void)+77/w
};

int __cdecl SEH_GetCurrentLanguage();
void __cdecl SEH_InitLanguage();
void SEH_UpdateCurrentLanguage();
void __cdecl SEH_UpdateLanguageInfo();
int __cdecl SEH_VerifyLanguageSelection(int iLanguageSelection);
void __cdecl SEH_Init_StringEd();
void __cdecl SEH_Shutdown_StringEd();
int __cdecl SEH_StringEd_SetLanguageStrings(unsigned int iLanguage);
const char *__cdecl SE_GetString(char *psPackageAndStringReference);
const char *__cdecl SE_GetString_FastFile(char *psPackageAndStringReference);
const char *__cdecl SEH_StringEd_GetString(char *pszReference);
char *__cdecl SEH_SafeTranslateString(const char *pszReference);
int __cdecl SEH_ReplaceDirectiveInStringWithBinding(int localClientNum, char *translatedString, char *finalString);
char __cdecl SEH_ReplaceDirective(
                int localClientNum,
                unsigned int *searchPos,
                unsigned int *dstLen,
                char *dstString,
                unsigned int size);
void __cdecl SEH_GetBindingForDirective(int localClientNum, char *directive, char *result);
void __cdecl SEH_ParseDirective(char *directive, char *resultName, char *resultArg0);
char *__cdecl SEH_LocalizeTextMessage(const char *pszInputBuffer, const char *pszMessageType, msgLocErrType_t errType);
int __cdecl SEH_GetLocalizedTokenReference(
                char *token,
                char *reference,
                const char *messageType,
                msgLocErrType_t errType);
bool __cdecl Japanese_ValidShiftJISCode(unsigned int _iHi, unsigned int _iLo);
int __cdecl SEH_DecodeLetter(
                unsigned __int8 firstChar,
                unsigned __int8 secondChar,
                int *usedCount,
                int *pbIsTrailingPunctuation);
bool __cdecl Japanese_IsTrailingPunctuation(unsigned int uiCode);
unsigned int __cdecl SEH_ReadCharFromString(const char **text, int *isTrailingPunctuation);
int __cdecl Language_IsAsian();
int __cdecl SEH_PrintStrlen(const char *string);
const char *__cdecl SEH_GetLanguageName(unsigned int iLanguage);
const char *__cdecl SEH_GetLanguageNameAbbr(unsigned int iLanguage);
int __cdecl SEH_GetLanguageIndexForName(const char *pszLanguageName, int *piLanguageIndex);


//EXT_List<LocalizeEntry> *__thiscall EXT_List<LocalizeEntry>::EXT_List<LocalizeEntry>(EXT_List<LocalizeEntry> *this);
//void __thiscall EXT_List<LocalizeEntry>::~EXT_List<LocalizeEntry>(EXT_List<LocalizeEntry> *this);
//void __thiscall EXT_List<LocalizeEntry>::Empty(EXT_List<LocalizeEntry> *this, bool DeleteData);
//char __thiscall EXT_List<LocalizeEntry>::Remove(
//                EXT_List<LocalizeEntry> *this,
//                EXT_Link<LocalizeEntry> *Link,
//                bool DeleteData);


extern const dvar_t *loc_language;
extern const dvar_t *loc_forceEnglish;
extern const dvar_t *loc_translate;
extern const dvar_t *loc_warnings;
extern const dvar_t *loc_warningsAsErrors;