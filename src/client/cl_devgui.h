#pragma once

void __cdecl CL_DevGuiFrame(int localClientNum);
void __cdecl CL_CreateDevGui();
void CL_RegisterDevGuiDvars();
void CL_CreateMapMenuEntries();
void __cdecl CL_CreateMapMenuEntriesForLocation(int locationFlags, const char *locationName);
void __cdecl CL_AddMapDirSlider(const char *dir, int locationFlags, const char *locationName);
void __cdecl CL_DestroyDevGui();
int CL_UnregisterDevGuiDvars();
