#include "cg_spikeacoustic.h"

const dvar_t *acousticSpikeMaxRange;
const dvar_t *acousticSpikeMinRange;
const dvar_t *acousticSpikeMinRadius;
const dvar_t *acousticSpikeSize;
const dvar_t *acousticSpikeRingSize;

void __cdecl CG_SpikeAcousticRegisterDvars()
{
    acousticSpikeMaxRange = _Dvar_RegisterFloat(
                                                        "acousticSpikeMaxRange",
                                                        200.0,
                                                        0.000099999997,
                                                        3.4028235e38,
                                                        0x1081u,
                                                        "The maximum range from the player in world space that objects will be shown on the acoustic spike");
    acousticSpikeMinRange = _Dvar_RegisterFloat(
                                                        "acousticSpikeMinRange",
                                                        0.0,
                                                        0.0,
                                                        3.4028235e38,
                                                        0x1081u,
                                                        "The minimum range from the player in world space that objects will be shown on the acoustic spike");
    acousticSpikeMinRadius = _Dvar_RegisterFloat(
                                                         "acousticSpikeMinRadius",
                                                         0.000099999997,
                                                         0.000099999997,
                                                         3.4028235e38,
                                                         1u,
                                                         "The minimum radius from the center of the sensor that objects will appear.");
    acousticSpikeSize = _Dvar_RegisterFloat("acousticSpikeSize", 10.0, 0.0, 3.4028235e38, 1u, "Scale the acoustic sensor");
    acousticSpikeRingSize = _Dvar_RegisterFloat(
                                                        "acousticSpikeRingSize",
                                                        20.0,
                                                        0.0,
                                                        3.4028235e38,
                                                        1u,
                                                        "Max size of the ping ring visual for a sensor");
}

