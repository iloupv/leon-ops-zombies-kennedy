#pragma once

#include <string>
#include <map>
#include <universal/com_memory.h>

struct LocalizeName // sizeof=0x0
{                                       // XREF: LocalizeStringName/r
                                        // mapStringEntriesName_t/r ...
    int GetType();
};

struct LocalizeStringName : LocalizeName // sizeof=0x0
{
    const char *GetName();
};

 struct CStringEdPackage // sizeof=0x78
 {
     int m_bEndMarkerFound_ParseOnly;
     //std::basic_string<char,std::char_traits<char>,Allocator<char,LocalizeStringName> > m_strCurrentEntryRef_ParseOnly;
     //std::basic_string<char,std::char_traits<char>,Allocator<char,LocalizeStringName> > m_strCurrentEntryEnglish_ParseOnly;
     //std::basic_string<char,std::char_traits<char>,Allocator<char,LocalizeStringName> > m_strCurrentFileRef_ParseOnly;
     std::string m_strCurrentEntryRef_ParseOnly;
     std::string m_strCurrentEntryEnglish_ParseOnly;
     std::string m_strCurrentFileRef_ParseOnly;
     //std::map<std::basic_string<char,std::char_traits<char>,Allocator<char,LocalizeStringName> >,std::basic_string<char,std::char_traits<char>,Allocator<char,LocalizeStringName> >,std::less<std::basic_string<char,std::char_traits<char>,Allocator<char,LocalizeStringName> > >,Allocator<std::pair<std::basic_string<char,std::char_traits<char>,Allocator<char,LocalizeStringName> >,std::basic_string<char,std::char_traits<char>,Allocator<char,LocalizeStringName> > >,mapStringEntriesName_t> > m_StringEntries;
     std::map<std::string, std::string> m_StringEntries;

     void *operator new(size_t size)
     {
         void *mem = Z_Malloc(size, "CStringEdPackage operator new", 69);

         return mem;
     }

     void operator delete(void *ptr) noexcept
     {
         Z_Free((char*)ptr, 69);
     }

     CStringEdPackage();
     ~CStringEdPackage();

     void __thiscall Clear();
     char *__thiscall Filename_WithoutExt(const char *psFilename);
     char *__thiscall Filename_WithoutPath(const char *psFilename);
     void __thiscall SetupNewFileParse(const char *psFileName);
     int __thiscall CheckLineForKeyword(
         const char *psKeyword,
         const char **psLine);
     char __thiscall IsStringFormatCorrect(char *string);
     void ConvertCRLiterals_Read(
         std::string *result,
         std::string string);
     void __thiscall REMKill(char *psBuffer);
     int __thiscall ReadLine(const char **psParsePos, char *psDest);
     std::string *InsideQuotes(
         //std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName> > *result,
         std::string *result,
         const char *psLine);
     const char *ParseLine(char *psLine, bool forceEnglish);
     //std::basic_string<char, std::char_traits<char>, Allocator<char, LocalizeStringName> >::_Bxty *__thiscall CStringEdPackage::GetCurrentReference_ParseOnly( CStringEdPackage *this);
     std::string *GetCurrentReference_ParseOnly();
     void AddEntry(const char *psLocalReference);
     void SetString(
         const char *psLocalReference,
         const char *psNewString,
         int bSentenceIsEnglish);
 };


char *__cdecl SE_Load(char *psFileName, bool forceEnglish);
const char *__cdecl SE_GetString_LoadObj(const char *psPackageAndStringReference);
void __cdecl SE_NewLanguage();
void __cdecl SE_Init();
void __cdecl SE_ShutDown();
char *__cdecl SE_LoadLanguage(bool forceEnglish);
char *__cdecl SE_GetFoundFile(std::string *strResult);
