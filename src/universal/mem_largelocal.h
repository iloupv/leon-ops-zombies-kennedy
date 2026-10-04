#pragma once

class LargeLocal
{
public:
    LargeLocal(int sizeParam);
    ~LargeLocal();

    unsigned __int8 *GetBuf();

    int startPos;
    int size;
};

int __cdecl LargeLocalBegin(int size);
void __cdecl LargeLocalEnd(int startPos);
unsigned __int8 *__cdecl LargeLocalGetBuf(int startPos);
void __cdecl LargeLocalReset();
unsigned int __cdecl LargeLocalRoundSize(int size);

int __cdecl LargeLocalBegin(int size);
unsigned int __cdecl LargeLocalRoundSize(int size);
void __cdecl LargeLocalEnd(int startPos);
unsigned __int8 *__cdecl LargeLocalGetBuf(int startPos, int size);
int __cdecl LargeLocalBeginRight(int size);
void __cdecl LargeLocalEndRight(int startPos);
void __cdecl LargeLocalReset();
