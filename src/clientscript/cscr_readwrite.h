#pragma once
#include "cscr_main.h"

unsigned int __cdecl FindLastSibling(scriptInstance_t inst, unsigned int parentId);
unsigned int __cdecl FindVariableIndexInternal(scriptInstance_t inst, unsigned int parentId, unsigned int name);
unsigned int __cdecl FindVariableIndexInternal2(scriptInstance_t inst, unsigned int name, unsigned int index);
