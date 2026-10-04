#include "stringed_ingame.h"
#include <universal/q_shared.h>
#include "stringed_interface.h"
#include <universal/com_memory.h>

CStringEdPackage *TheStringPackage;

int LocalizeName::GetType()
{
    return 34;
}

const char *LocalizeStringName::GetName()
{
    return "LocalizeString";
}

void __thiscall CStringEdPackage::Clear()
{
    m_StringEntries.clear();
    m_bEndMarkerFound_ParseOnly = 0;
    m_strCurrentEntryRef_ParseOnly = "";
    m_strCurrentEntryEnglish_ParseOnly = "";
}

char *__thiscall CStringEdPackage::Filename_WithoutExt(const char *psFilename)
{
    char *v2; // eax
    char *v3; // eax
    char *v4; // eax
    char v6; // [esp+3h] [ebp-1Dh]
    char *v7; // [esp+8h] [ebp-18h]
    char *p2; // [esp+18h] [ebp-8h]
    char *p; // [esp+1Ch] [ebp-4h]

    static char sString[64];
    v7 = sString;
    do
    {
        v6 = *psFilename;
        *v7++ = *psFilename++;
    } while (v6);
    p = strrchr(sString, '.');
    p2 = strrchr(sString, '\\');
    v4 = strrchr(sString, '/');
    if (p && (!p2 || p > p2) && (!v4 || p > v4))
        *p = 0;
    return sString;
}

char *__thiscall CStringEdPackage::Filename_WithoutPath(const char *psFilename)
{
    static char sString_0[64];

    char v2; // dl
    char *v4; // [esp+8h] [ebp-10h]
    const char *v5; // [esp+Ch] [ebp-Ch]
    const char *psCopyPos; // [esp+14h] [ebp-4h]

    psCopyPos = psFilename;
    while (*psFilename)
    {
        if (*psFilename == '/' || *psFilename == '\\')
            psCopyPos = psFilename + 1;
        ++psFilename;
    }
    v5 = psCopyPos;
    v4 = sString_0;
    do
    {
        v2 = *v5;
        *v4++ = *v5++;
    } while (v2);
    return sString_0;
}

void __thiscall CStringEdPackage::SetupNewFileParse(const char *psFileName)
{
    const char *v2; // eax
    char v3; // al
    char *v4; // [esp+18h] [ebp-94h]
    char *v5; // [esp+1Ch] [ebp-90h]
    char sString[68]; // [esp+64h] [ebp-48h] BYREF

    v2 = Filename_WithoutExt(psFileName);
    v5 = Filename_WithoutPath(v2);
    v4 = sString;
    do
    {
        v3 = *v5;
        *v4++ = *v5++;
    } while (v3);
    I_strupr(sString);
    m_strCurrentFileRef_ParseOnly = sString;
}

int __thiscall CStringEdPackage::CheckLineForKeyword(
                const char *psKeyword,
                const char **psLine)
{
    if ( I_strnicmp(psKeyword, *psLine, strlen(psKeyword)) )
        return 0;
    for ( *psLine += strlen(psKeyword); **psLine == 9 || **psLine == 32; ++*psLine )
        ;
    return 1;
}

char __thiscall CStringEdPackage::IsStringFormatCorrect(char *string)
{
    const char *v2; // eax
    const char *v4; // eax
    CStringEdPackage *thisa; // [esp+0h] [ebp-1Ch]
    int argIndex; // [esp+4h] [ebp-18h]
    const char *convString; // [esp+8h] [ebp-14h]
    bool args[12]; // [esp+Ch] [ebp-10h] BYREF

    thisa = this;
    memset(args, 0, 9);
    for (convString = strstr(string, "&&"); convString; convString = v4)
    {
        convString += 2;
        if (!isdigit(*convString))
            return 0;
        argIndex = *convString - 48;
        if (args[--argIndex])
            return 0;
        args[argIndex] = 1;

        v4 = strstr(++convString, "&&");
    }
    return 1;
}

void CStringEdPackage::ConvertCRLiterals_Read(std::string *result, std::string string)
{
    // BO1TODO: technically not the blops version
    int v4[2]; // [esp-8h] [ebp-A8h]
    std::string str;
    int loc; // [esp+98h] [ebp-8h]

    str.assign(string);

    while (1)
    {
        v4[0] = 0;
        v4[1] = strlen("\\n");

        auto loc = str.find("\\n", 0);
        if (loc == std::string::npos)
            break;

        str[loc] = 10;

        str.erase(loc + 1, 1);
    }

    *result = str;
}

void __thiscall CStringEdPackage::REMKill(char *psBuffer)
{
    char *v2; // eax
    int iWhiteSpaceScanPos; // [esp+14h] [ebp-18h]
    int i; // [esp+18h] [ebp-14h]
    int iDoubleQuoteCount; // [esp+1Ch] [ebp-10h]
    int iDoubleQuotesSoFar; // [esp+20h] [ebp-Ch]

    for ( iDoubleQuotesSoFar = 0; ; iDoubleQuotesSoFar = iDoubleQuoteCount )
    {
        v2 = strstr(psBuffer, "//");
        if ( !v2 )
            break;
        iDoubleQuoteCount = iDoubleQuotesSoFar;
        for ( i = 0; i < v2 - psBuffer; ++i )
        {
            if ( psBuffer[i] == 34 )
                ++iDoubleQuoteCount;
        }
        if ( (iDoubleQuoteCount & 1) == 0 )
        {
            *v2 = 0;
            if ( *psBuffer )
            {
                for ( iWhiteSpaceScanPos = strlen(psBuffer) - 1;
                            iWhiteSpaceScanPos >= 0 && isspace(psBuffer[iWhiteSpaceScanPos]);
                            --iWhiteSpaceScanPos )
                {
                    psBuffer[iWhiteSpaceScanPos] = 0;
                }
            }
            return;
        }
        psBuffer = v2 + 1;
    }
}

int __thiscall CStringEdPackage::ReadLine(const char **psParsePos, char *psDest)
{
    const char *v3; // eax
    const char *v4; // eax
    unsigned __int8 v5; // cl
    char *v7; // [esp+28h] [ebp-18h]
    unsigned __int8 *v8; // [esp+2Ch] [ebp-14h]
    int iWhiteSpaceScanPos; // [esp+34h] [ebp-Ch]
    unsigned int iCharsToCopy; // [esp+38h] [ebp-8h]

    if ( !**psParsePos )
        return 0;
    v3 = strchr(*psParsePos, '\n');
    if ( v3 )
    {
        iCharsToCopy = (int)v3 - (unsigned int)*psParsePos;
        strncpy(psDest, *psParsePos, iCharsToCopy);
        psDest[iCharsToCopy] = 0;
        for ( *psParsePos += iCharsToCopy; **psParsePos; ++*psParsePos )
        {
            v4 = strchr("\r\n", **psParsePos);
            if ( !v4 )
                break;
        }
    }
    else
    {
        v8 = (unsigned __int8 *)*psParsePos;
        v7 = psDest;
        do
        {
            v5 = *v8;
            *v7++ = *v8++;
        }
        while ( v5 );
        *psParsePos += strlen(*psParsePos);
    }
    if ( *psDest )
    {
        for ( iWhiteSpaceScanPos = strlen(psDest) - 1;
                    iWhiteSpaceScanPos >= 0 && isspace(psDest[iWhiteSpaceScanPos]);
                    --iWhiteSpaceScanPos )
        {
            psDest[iWhiteSpaceScanPos] = 0;
        }
        CStringEdPackage::REMKill(psDest);
    }
    return 1;
}

std::string *CStringEdPackage::InsideQuotes(
                std::string *result,
                const char *psLine)
{
    std::string str; // [esp+10Ch] [ebp-20h] BYREF

    str = "";

    while (*psLine == ' ' || *psLine == '\t')
        ++psLine;
    if (*psLine == '"')
        ++psLine;

    str.assign(psLine);

    if (*psLine)
    {
        while (1)
        {
            if (str[str.size() - 1] != ' ')
            {
                if (str[str.size() - 1] != 9)
                {
                    break;
                }
            }

            str.erase(str.size() - 1, 1);
        }

        if (str[str.size() - 1] == '"')
        {
            str.erase(str.size() - 1, 1);
        }
    }

    *result = str;
    return result;
}

const char *__thiscall CStringEdPackage::ParseLine(char *psLine, bool forceEnglish)
{
    std::string *Ptr; // eax
    int v5; // eax
    std::string *p_sentence; // eax
    const char *v7; // edi
    std::string *p_m_strCurrentEntryRef_ParseOnly; // eax
    bool v9; // zf
    char v10; // al
    unsigned __int8 *v11; // edi
    unsigned __int8 *i; // esi
    unsigned int v13; // esi
    std::string *v14; // eax
    std::string *v15; // eax
    bool v16; // eax
    std::string *v17; // ecx
    std::string v19; // [esp-1Ch] [ebp-450h] BYREF
    const char *psErrorMessage; // [esp+Ch] [ebp-428h]
    const char *psReference; // [esp+10h] [ebp-424h] BYREF
    std::string sentence; // [esp+14h] [ebp-420h] BYREF
    char sThisLanguage[1024]; // [esp+30h] [ebp-404h] BYREF

    psReference = psLine;
    psErrorMessage = 0;
    if (psLine)
    {
        if (CStringEdPackage::CheckLineForKeyword("VERSION", &psReference))
        {
            CStringEdPackage::InsideQuotes(&sentence, psReference);
            //Ptr = (std::string *)sentence._Bx._Ptr;
            //if (sentence._Myres < 0x10)
            //    Ptr = &sentence;
            //v5 = atoi(Ptr->_Bx._Buf);
            v5 = atoi(sentence.data());
            if (v5 != 1)
                psErrorMessage = va("Unexpected version number %d, expecting %d!", v5, 1);
            //if (sentence._Myres >= 0x10)
            //    operator delete(sentence._Bx._Ptr);
        }
        else if (!CStringEdPackage::CheckLineForKeyword("CONFIG", &psReference)
            && !CStringEdPackage::CheckLineForKeyword("FILENOTES", &psReference)
            && !CStringEdPackage::CheckLineForKeyword("NOTES", &psReference)
            && !CStringEdPackage::CheckLineForKeyword("FLAGS", &psReference))
        {
            if (CStringEdPackage::CheckLineForKeyword("REFERENCE", &psReference))
            {
                CStringEdPackage::InsideQuotes(&sentence, psReference);
                //p_sentence = (std::string *)sentence._Bx._Ptr;
                //if (sentence._Myres < 0x10)
                //    p_sentence = &sentence;
                //CStringEdPackage::AddEntry(p_sentence->_Bx._Buf);
                p_sentence = &sentence;
                CStringEdPackage::AddEntry(p_sentence->data());
                //std::string::~string(&sentence);
            }
            else if (CStringEdPackage::CheckLineForKeyword("ENDMARKER", &psReference))
            {
                this->m_bEndMarkerFound_ParseOnly = 1;
            }
            else
            {
                v7 = psReference;
                if (I_strnicmp("LANG_", psReference, 5))
                {
                    return va("Unknown keyword at linestart: \"%s\"", v7);
                }
                else
                {
                    p_m_strCurrentEntryRef_ParseOnly = &this->m_strCurrentEntryRef_ParseOnly;
                    //if (this->m_strCurrentEntryRef_ParseOnly._Myres >= 0x10)
                    //    p_m_strCurrentEntryRef_ParseOnly = (std::string *)p_m_strCurrentEntryRef_ParseOnly->_Bx._Ptr;
                    //v9 = p_m_strCurrentEntryRef_ParseOnly->_Bx._Buf[0] == 0;
                    v9 = p_m_strCurrentEntryRef_ParseOnly->data()[0] == 0;
                    psReference = p_m_strCurrentEntryRef_ParseOnly->c_str();
                    if (v9)
                    {
                        return "Error parsing file: Unexpected \"LANG_\"";
                    }
                    else
                    {
                        v10 = v7[5];
                        v11 = (unsigned __int8 *)(v7 + 5);
                        for (i = v11; v10; v10 = *++i)
                        {
                            if (v10 == 32)
                                break;
                            if (v10 == 9)
                                break;
                        }
                        memset(sThisLanguage, 0, sizeof(sThisLanguage));
                        v13 = i - v11;
                        if (v13 > 1023)
                            v13 = 1023;
                        strncpy(sThisLanguage, (char *)v11, v13);
                        CStringEdPackage::InsideQuotes(&v19, (const char *)&v11[strlen(sThisLanguage)]);
                        CStringEdPackage::ConvertCRLiterals_Read(&sentence, v19);
                        //v14 = (std::string *)sentence._Bx._Ptr;
                        //if (sentence._Myres < 0x10)
                        //    v14 = &sentence;
                        v14 = &sentence;
                        //if (!CStringEdPackage::IsStringFormatCorrect(v14->_Bx._Buf))
                        if (!CStringEdPackage::IsStringFormatCorrect((char*)v14->data()))
                        {
                            //v15 = (std::string *)sentence._Bx._Ptr;
                            //if (sentence._Myres < 0x10)
                            //    v15 = &sentence;
                            v15 = &sentence;
                            //psErrorMessage = va("Illegal string format \"%s\"", v15->_Bx._Buf);
                            psErrorMessage = va("Illegal string format \"%s\"", v15->data());
                        }
                        v16 = I_stricmp(sThisLanguage, "english") == 0;
                        if (!psErrorMessage && (v16 || !forceEnglish))
                        {
                            //v17 = (std::string *)sentence._Bx._Ptr;
                            //if (sentence._Myres < 0x10)
                            //    v17 = &sentence;
                            v17 = &sentence;
                            //CStringEdPackage::SetString(psReference, v17->_Bx._Buf, v16);
                            CStringEdPackage::SetString(psReference, v17->data(), v16);
                        }
                        //std::string::~string(&sentence);
                    }
                }
            }
        }
    }
    return psErrorMessage;
}

std::string *__thiscall CStringEdPackage::GetCurrentReference_ParseOnly()
{
    return &this->m_strCurrentEntryRef_ParseOnly;
}

void __thiscall CStringEdPackage::AddEntry(const char *psLocalReference)
{
    char *v2; // eax
    char *v3; // eax
    char *v6; // [esp+F0h] [ebp-B8h]

    v2 = va("%s_%s", this->m_strCurrentFileRef_ParseOnly.data(), psLocalReference); // _Ptr

    auto itEntry = this->m_StringEntries.find(v2);

    if (itEntry == this->m_StringEntries.end())
    {
        std::string SE_Entry;

        v3 = va("%s_%s", this->m_strCurrentFileRef_ParseOnly.c_str(), psLocalReference);

        std::string v9;
        v9.assign(v3);
        m_StringEntries[v9] = SE_Entry;
    }
    this->m_strCurrentEntryRef_ParseOnly.assign(psLocalReference);
}

void __thiscall CStringEdPackage::SetString(
                const char *psLocalReference,
                const char *psNewString,
                int bSentenceIsEnglish)
{
    char *v4; // eax

    v4 = va("%s_%s", m_strCurrentFileRef_ParseOnly.c_str(), psLocalReference);

    auto itEntry = this->m_StringEntries.find(v4);

    iassert(itEntry != m_StringEntries.end());

    auto &Entry = (*itEntry).second;

    if (bSentenceIsEnglish)
    {
        Entry.assign(psNewString);

        m_strCurrentEntryEnglish_ParseOnly.assign(psNewString);
    }
    else if (I_stricmp(psNewString, "#same"))
    {
        Entry.assign(psNewString);
    }
    else
    {
        Entry.assign(m_strCurrentEntryEnglish_ParseOnly);
    }
}

char *__cdecl SE_Load(char *psFileName, bool forceEnglish)
{
    const char *v2; // edi
    unsigned __int8 *FileData; // eax
    char *v4; // esi
    int v6; // eax
    char *psParsePos; // [esp+14h] [ebp-4008h] BYREF
    char psDest[16384]; // [esp+18h] [ebp-4004h] BYREF

    v2 = 0;
    FileData = SE_LoadFileData(psFileName);
    v4 = (char *)FileData;
    if (!FileData)
        return va("Unable to load \"%s\"!", psFileName);
    v6 = *(_DWORD *)FileData;
    psParsePos = v4;
    if (v6 == -131072 || v6 == 65279)
    {
        psParsePos = v4 + 4;
    }
    else if ((_BYTE)v6 == 0xFE && v4[1] == -1 || (_BYTE)v6 == 0xFF && v4[1] == -2)
    {
        psParsePos = v4 + 2;
    }
    else if ((_BYTE)v6 == 0xEF && v4[1] == -69 && v4[2] == -65)
    {
        psParsePos = v4 + 3;
    }
    //CStringEdPackage::SetupNewFileParse(TheStringPackage, psFileName);
    TheStringPackage->SetupNewFileParse(psFileName);
    while (TheStringPackage->ReadLine((const char **)&psParsePos, psDest))
    {
        if (strlen(psDest))
            v2 = TheStringPackage->ParseLine(psDest, forceEnglish);
        if (v2)
        {
            v2 = va("%s in %s", v2, psFileName);
            break;
        }
    }
    SE_FreeFileDataAfterLoad((unsigned __int8 *)v4);
    if (!v2 && !TheStringPackage->m_bEndMarkerFound_ParseOnly)
        return va("Truncated file, failed to find \"%s\" at file end in file %s", "ENDMARKER", psFileName);
    return (char *)v2;
}

const char *__cdecl SE_GetString_LoadObj(const char *psPackageAndStringReference)
{
    auto itEntry = TheStringPackage->m_StringEntries.find(psPackageAndStringReference);

    if (itEntry == TheStringPackage->m_StringEntries.end())
    {
        return NULL;
    }

    return itEntry->second.c_str();
}

void __cdecl SE_NewLanguage()
{
    if ( !TheStringPackage
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\stringed\\stringed_ingame.cpp",
                    858,
                    0,
                    "%s",
                    "TheStringPackage") )
    {
        __debugbreak();
    }
    //CStringEdPackage::Clear(TheStringPackage);
    TheStringPackage->Clear();
}

void __cdecl SE_Init()
{
    CStringEdPackage *v0; // [esp+0h] [ebp-68h]
    CStringEdPackage *v1; // [esp+64h] [ebp-4h]

    if ( TheStringPackage
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\stringed\\stringed_ingame.cpp",
                    870,
                    0,
                    "%s",
                    "!TheStringPackage") )
    {
        __debugbreak();
    }
    v1 = new CStringEdPackage(); // moved to new/delete operator 
    //v1 = (CStringEdPackage *)Z_Malloc(120, "CStringEdPackage", 34);
    //if ( v1 )
    //    v0 = CStringEdPackage::CStringEdPackage(v1);
    //else
    //    v0 = 0;
    TheStringPackage = v1;
    TheStringPackage->Clear();
    //CStringEdPackage::Clear(v0);
}

CStringEdPackage::CStringEdPackage()
{
    m_strCurrentEntryRef_ParseOnly.clear();
    m_strCurrentEntryEnglish_ParseOnly.clear();
    m_strCurrentFileRef_ParseOnly.clear();
    m_StringEntries.clear();
    //std::basic_string<char,std::char_traits<char>,Allocator<char,LocalizeStringName>>::_Tidy(
    //    &this->m_strCurrentEntryRef_ParseOnly,
    //    0,
    //    0);
    //std::basic_string<char,std::char_traits<char>,Allocator<char,LocalizeStringName>>::_Tidy(
    //    &this->m_strCurrentEntryEnglish_ParseOnly,
    //    0,
    //    0);
    //std::basic_string<char,std::char_traits<char>,Allocator<char,LocalizeStringName>>::_Tidy(
    //    &this->m_strCurrentFileRef_ParseOnly,
    //    0,
    //    0);
    //std::map<std::basic_string<char,std::char_traits<char>,Allocator<char,LocalizeStringName>>,std::basic_string<char,std::char_traits<char>,Allocator<char,LocalizeStringName>>,std::less<std::basic_string<char,std::char_traits<char>,Allocator<char,LocalizeStringName>>>,Allocator<std::pair<std::basic_string<char,std::char_traits<char>,Allocator<char,LocalizeStringName>>,std::basic_string<char,std::char_traits<char>,Allocator<char,LocalizeStringName>>>,mapStringEntriesName_t>>::map<std::basic_string<char,std::char_traits<char>,Allocator<char,LocalizeStringName>>,std::basic_string<char,std::char_traits<char>,Allocator<char,LocalizeStringName>>,std::less<std::basic_string<char,std::char_traits<char>,Allocator<char,LocalizeStringName>>>,Allocator<std::pair<std::basic_string<char,std::char_traits<char>,Allocator<char,LocalizeStringName>>,std::basic_string<char,std::char_traits<char>,Allocator<char,LocalizeStringName>>>,mapStringEntriesName_t>>(&this->m_StringEntries);
    //return this;
}

void __cdecl SE_ShutDown()
{
    if ( TheStringPackage )
    {
        //CStringEdPackage::Clear(TheStringPackage);
        TheStringPackage->Clear();
        if ( TheStringPackage )
        {
            //CStringEdPackage::~CStringEdPackage(TheStringPackage);
            delete TheStringPackage;
            //Z_Free((char *)TheStringPackage, 34); // defined in operator delete
        }
        TheStringPackage = 0;
    }
}

CStringEdPackage::~CStringEdPackage()
{
    // operator delete should free it
    //int Type; // eax
    //std::map<std::basic_string<char,std::char_traits<char>,Allocator<char,LocalizeStringName> >,std::basic_string<char,std::char_traits<char>,Allocator<char,LocalizeStringName> >,std::less<std::basic_string<char,std::char_traits<char>,Allocator<char,LocalizeStringName> > >,Allocator<std::pair<std::basic_string<char,std::char_traits<char>,Allocator<char,LocalizeStringName> >,std::basic_string<char,std::char_traits<char>,Allocator<char,LocalizeStringName> > >,mapStringEntriesName_t> > *p_m_StringEntries; // [esp+40h] [ebp-44h]
    //char *ptr; // [esp+44h] [ebp-40h]
    //
    //p_m_StringEntries = &this->m_StringEntries;
    //std::_Tree<std::_Tmap_traits<std::basic_string<char,std::char_traits<char>,Allocator<char,LocalizeStringName>>,std::basic_string<char,std::char_traits<char>,Allocator<char,LocalizeStringName>>,std::less<std::basic_string<char,std::char_traits<char>,Allocator<char,LocalizeStringName>>>,Allocator<std::pair<std::basic_string<char,std::char_traits<char>,Allocator<char,LocalizeStringName>>,std::basic_string<char,std::char_traits<char>,Allocator<char,LocalizeStringName>>>,mapStringEntriesName_t>,0>>::_Tidy(&this->m_StringEntries);
    //ptr = (char *)p_m_StringEntries->_Myownedaux;
    //Type = LocalizeName::GetType();
    //Z_Free(ptr, Type);
    //std::basic_string<char,std::char_traits<char>,Allocator<char,LocalizeStringName>>::_Tidy(
    //    &this->m_strCurrentFileRef_ParseOnly,
    //    1,
    //    0);
    //std::basic_string<char,std::char_traits<char>,Allocator<char,LocalizeStringName>>::_Tidy(
    //    &this->m_strCurrentEntryEnglish_ParseOnly,
    //    1,
    //    0);
    //std::basic_string<char,std::char_traits<char>,Allocator<char,LocalizeStringName>>::_Tidy(
    //    &this->m_strCurrentEntryRef_ParseOnly,
    //    1,
    //    0);
}

char *__cdecl SE_LoadLanguage(bool forceEnglish)
{
    char *psErrorMessage; // [esp+30h] [ebp-28h]
    std::string strResults;
    const char *p; // [esp+54h] [ebp-4h]

    psErrorMessage = 0;
    strResults.clear();
    SE_NewLanguage();
    SE_BuildFileList("localizedstrings", &strResults);
    while (1)
    {
        p = SE_GetFoundFile(&strResults);
        if (!p || psErrorMessage)
            break;
        psErrorMessage = SE_Load((char *)p, forceEnglish);
    }
    return psErrorMessage;
}

static char sTemp[64];
char *__cdecl SE_GetFoundFile(std::string *strResult)
{
    _BYTE *v2; // eax

    if (!strResult->size())
    {
        return NULL;
    }

    strncpy(sTemp, strResult->data(), 63);
    sTemp[63] = 0;
    v2 = (_BYTE *)strchr(sTemp, ';');
    if (v2)
    {
        *v2 = 0;
        strResult->erase(0, (char *)v2 - sTemp + 1);
    }
    else
    {
        strResult->erase(0, std::string::npos);
    }
    return sTemp;
}

