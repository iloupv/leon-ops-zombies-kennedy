#pragma once

int __cdecl Com_EncodeYEnc(unsigned __int8 *indata, int inlen, char *outdata, int outlen);
bool __cdecl YEncIsCritical(unsigned __int8 c);
int __cdecl Com_DecodeYEnc(const char *indata, unsigned __int8 *outdata, int outlen);
