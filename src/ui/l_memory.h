#pragma once

unsigned int *__cdecl GetMemory(unsigned int size);
unsigned __int8 *__cdecl GetClearedMemory(unsigned int size);
void __cdecl FreeMemory(char *ptr);
