#pragma once

struct qtime_s;

char __cdecl Com_Filter(const char *filter, char *name, int casesensitive);
char __cdecl Com_FilterPath(const char *filter, const char *name, int casesensitive);
int __cdecl Com_HashKey(const char *string, int maxlen);
void __cdecl Com_GetQTime(int time, qtime_s *qtime, bool useLocalTime);
int __cdecl Com_RealTime(qtime_s *qtime, bool useLocalTime);
void __cdecl _copyDWord(unsigned int *dest, unsigned int constant, unsigned int count);
//void __cdecl Com_Memcpy(unsigned __int8 *dest, unsigned __int8 *src, unsigned int count);
//void __cdecl Com_Memset(unsigned int *dest, int val, int count);
