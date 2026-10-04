#pragma once

struct demoFileHandler // sizeof=0xC
{                                       // XREF: .data:s_fileHandler/r
    unsigned __int8 *ptr;               // XREF: Demo_InitFileHandlerSystem(void)+14/w
    int size;                           // XREF: Demo_InitFileHandlerSystem(void)+1A/w
    int offset;                         // XREF: Demo_InitFileHandlerSystem(void)+20/w
};

void __cdecl Demo_InitFileHandlerSystem();
void __cdecl Demo_AllocateMemoryFromStreamBuffer(int size);
void __cdecl Demo_ReturnStreamBufferMemory();
bool __cdecl Demo_IsStreamBufferAllocated();
int __cdecl Demo_GetCurrentAllocatedFileSize();
char *__cdecl Demo_GetStreamAllocatedBuffer();
void __cdecl Demo_GetDemoPath(char *demoPath);
int __cdecl Demo_OpenFileWrite(const char *filename, const char *dir, bool supressErrors);
int __cdecl Demo_OpenFileRead(const char *filename, const char *dir, bool supressErrors);
unsigned int __cdecl Demo_Write(char *buffer, unsigned int len, int handle);
unsigned int __cdecl Demo_Read(unsigned __int8 *buffer, unsigned int len, int handle);
void __cdecl Demo_CloseFile(int handle);
int __cdecl Demo_SeekFile(int handle, int offset, int origin);
