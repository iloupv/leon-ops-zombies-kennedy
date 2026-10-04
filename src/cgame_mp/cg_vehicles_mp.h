#pragma once

struct DObj;
struct centity_s;
struct clientInfo_t;
struct vehicle_info_t;

DObj *__cdecl GetVehicleEntDObj(int localClientNum, centity_s *centVeh);
bool __cdecl CG_VehEntityUsingVehicle(int localClientNum, int entNum);
clientInfo_t *__cdecl ClientInfoForEntity(int localClientNum, int entNum);
bool __cdecl CG_VehEntityUsingRemoteControlVehicle(int localClientNum, int entNum);
const vehicle_info_t *__cdecl CG_GetVehicleInfo(int index);
int __cdecl CG_VehPlayerVehicleSeat(int localClientNum, int entNum);
void __cdecl CG_VehSeatTransformForPlayer(int localClientNum, int entNum, float *resultOrigin, float *resultAngles);
void __cdecl SeatTransformForClientInfo(int localClientNum, clientInfo_t *ci, float *resultOrigin, float *resultAngles);
void __cdecl SeatTransformForSlot(
                int localClientNum,
                int vehEntNum,
                int vehSlotIdx,
                float *resultOrigin,
                float *resultAngles);
void __cdecl GetTagMatrix(
                int localClientNum,
                int vehEntNum,
                unsigned __int16 tagName,
                float (*resultTagMat)[3],
                float *resultOrigin);
double __cdecl CG_VehGetHealthPercentageEntity(const centity_s *cent);
double __cdecl CG_VehGetHealthPercentageLeftTread(const centity_s *cent);
double __cdecl CG_VehGetHealthPercentageRightTread(const centity_s *cent);
int __cdecl CG_VehGetSeatOccupancyFlags(const centity_s *cent);
bool __cdecl CG_VehGetSeatGunTurningYaw(const centity_s *cent, unsigned int seatIndex);
bool __cdecl CG_VehGetSeatGunTurningPitch(const centity_s *cent, unsigned int seatIndex);
bool __cdecl CG_VehGetSeatGunOverheating(const centity_s *cent, unsigned int seatIndex);
