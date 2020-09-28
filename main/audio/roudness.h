/*
 * author : Shuichi TAKANO
 * since  : Mon Sep 21 2020 01:24:35
 */
#ifndef F4B3B1C5_A134_3DC5_12F8_7514922BD616
#define F4B3B1C5_A134_3DC5_12F8_7514922BD616

#include <cmath>

namespace audio
{

inline float
computeRoudness(float freq)
{
    float f2 = freq * freq;
    float f4 = f2 * f2;
    return (1.48699e8f * f4) / (sqrt(6.30958e9f + 556030.0f * f2 + f4) *
                                (6.30957e10f + 1.48699e8f * f2 + f4));
}

} // namespace audio

#endif /* F4B3B1C5_A134_3DC5_12F8_7514922BD616 */
