#pragma once

struct SessionData_s;

int __cdecl Session_BuildQoSPayload();
unsigned __int8 *__cdecl Session_GetQosPayloadBuffer();
bool __cdecl Session_IsHost(SessionData_s *session, int clientNum);
int __cdecl Session_HostNum(SessionData_s *session);
