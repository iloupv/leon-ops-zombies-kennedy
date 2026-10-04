#pragma once
#include "cscr_variable.h"

// zombies: SP 0x00642F70, called before the entity/VM serializers by G_SaveState.
void Scr_SavePre(scriptInstance_t inst);
// SP 0x0050DF00 / 0x0042AEB0, segment-4 payload writer/reader.
void Scr_Save(scriptInstance_t inst, struct MemoryFile *memFile);
void Scr_Load(scriptInstance_t inst, struct MemoryFile *memFile);
void Scr_LoadPost(scriptInstance_t inst);
// SP 0x005F5700: add an object and every object reachable from it to the save-ID maps.
void Scr_AddSaveObject(scriptInstance_t inst, unsigned int id);
// KB diagnostic: check the maps without changing script objects or their reference counts.
void Scr_CheckSaveObjects(scriptInstance_t inst);
