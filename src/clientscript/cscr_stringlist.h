#pragma once

#include "cscr_main.h"

struct scrStringDebugGlob_t // sizeof=0x40008
{                                       // XREF: .data:scrStringDebugGlob_t * gScrStringDebugGlobBuff/r
                                        // scrStringDebugGlobArray_t/r
    //volatile int refCount[65536];
    volatile unsigned int refCount[65536];
    //volatile int totalRefCount;
    volatile unsigned int totalRefCount;
    int ignoreLeaks;
};

// BO1TODO: guessing because the below function has inst as the 2nd arg, it's got a default val
char *__cdecl SL_ConvertToString(unsigned int stringValue, scriptInstance_t inst);
RefString *__cdecl GetRefString(scriptInstance_t inst, unsigned int stringValue);
const char *__cdecl SL_DebugConvertToString(unsigned int stringValue, scriptInstance_t inst);
int __cdecl SL_GetStringLen(scriptInstance_t inst, unsigned int stringValue);
int __cdecl SL_GetRefStringLen(RefString *refString);
int __cdecl SL_ConvertFromString(scriptInstance_t inst, char *str);
int __cdecl SL_ConvertFromRefString(scriptInstance_t inst, RefString *refString);
int __cdecl Scr_GetHashCode(scriptInstance_t inst, const char *str, unsigned int len);
int __cdecl GetHashCode(scriptInstance_t inst, const char *str, unsigned int len);
void __cdecl SL_Init(scriptInstance_t inst);
void __cdecl SL_InitCheckLeaks(scriptInstance_t inst);
void __cdecl SL_Shutdown(scriptInstance_t inst);
void __cdecl SL_CheckLeaks(scriptInstance_t inst);
unsigned int __cdecl SL_FindStringOfSize(scriptInstance_t inst, const char *str, unsigned int len);
unsigned int __cdecl SL_FindString(const char *str, scriptInstance_t inst);
unsigned int __cdecl SL_FindLowercaseString(const char *str, scriptInstance_t inst);
int __cdecl SL_IsLowercaseString(unsigned int stringValue, scriptInstance_t inst);
void __cdecl SL_AddUser(unsigned int stringValue, unsigned int user, scriptInstance_t inst);
void __cdecl SL_AddUserInternal(scriptInstance_t inst, RefString *refStr, unsigned int user);
unsigned int __cdecl SL_GetUser(unsigned int stringValue, scriptInstance_t inst);
unsigned int __cdecl SL_GetStringOfSize(
                scriptInstance_t inst,
                char *str,
                unsigned int user,
                unsigned int len,
                int type);
unsigned int __cdecl SL_GetString_(scriptInstance_t inst, char *str, unsigned int user, int type);
inline unsigned int __cdecl SL_GetString_(scriptInstance_t inst, const char *str, unsigned int user, int type)
{
    return SL_GetString_(inst, (char *)str, user, type);
}

unsigned int __cdecl SL_GetString(char *str, unsigned int user, scriptInstance_t inst);
unsigned int __cdecl SL_GetLowercaseString_(const char *str, unsigned int user, int type, scriptInstance_t inst);
unsigned int __cdecl SL_GetLowercaseStringOfSize(
                scriptInstance_t inst,
                const char *str,
                unsigned int user,
                unsigned int len,
                int type);
unsigned int __cdecl SL_GetLowercaseString(const char *str, unsigned int user, scriptInstance_t inst);
unsigned int __cdecl SL_ConvertToLowercase(
                scriptInstance_t inst,
                unsigned int stringValue,
                unsigned int user,
                int type);
void __cdecl SL_TransferRefToUser(scriptInstance_t inst, unsigned int stringValue, unsigned int user);
void __cdecl SL_CheckExists(scriptInstance_t inst, unsigned int stringValue);
void __cdecl SL_AddRefToString(unsigned int stringValue, scriptInstance_t inst);
void __cdecl SL_RemoveRefToString(scriptInstance_t inst, unsigned int stringValue);
void __cdecl SL_RemoveRefToStringOfSize(scriptInstance_t inst, unsigned int stringValue, unsigned int len);
void __cdecl SL_FreeString(scriptInstance_t inst, unsigned int stringValue, RefString *refStr, unsigned int len);
void __cdecl Scr_SetStringLiveUpdateSafe(unsigned __int16 *to, char *value, scriptInstance_t inst);
void __cdecl Scr_SetString(unsigned __int16 *to, unsigned int from, scriptInstance_t inst);
void __cdecl Scr_SetStringFromCharString(unsigned __int16 *to, char *from, scriptInstance_t inst);
unsigned int __cdecl Scr_AllocString(const char *s, int sys, scriptInstance_t inst);
unsigned int __cdecl SL_GetStringForFloat(float f, scriptInstance_t inst);
unsigned int __cdecl SL_GetStringForInt(scriptInstance_t inst, int i);
unsigned int __cdecl SL_GetStringForVector(scriptInstance_t inst, const float *v);
void __cdecl SL_ShutdownSystem(scriptInstance_t inst, unsigned int user);
void __cdecl Scr_ShutdownGameStrings(scriptInstance_t inst);
void __cdecl SL_TransferSystem(unsigned int from, unsigned int to, scriptInstance_t inst);
unsigned int __cdecl Scr_CreateCanonicalFilename(scriptInstance_t inst, const char *filename);
void __cdecl CreateCanonicalFilename(char *newFilename, const char *filename, int count);


extern scrStringDebugGlob_t *gScrStringDebugGlob[2];
extern scrStringGlob_t gScrStringGlob[2];