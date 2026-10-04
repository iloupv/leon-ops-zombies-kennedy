#pragma once

char __cdecl Monkey_InitCommunication(const char *server);
void __cdecl Monkey_ShutdownCommunication();
void __cdecl    Monkey_DisconnectAndExit();
void __cdecl Monkey_Send(const char *type, const char *string);
void __cdecl Monkey_SendString(const char *string);
bool __cdecl Monkey_Recv(char *type, int type_size, char *string, int string_size);
char __cdecl Monkey_RecvString(char *data, int data_size);
bool __cdecl Monkey_SocketHasData();
