#pragma once

void __cdecl R_DebugAlloc(void **memPtr, int size, const char *name);
void __cdecl R_DebugFree(void **dataPtr);
