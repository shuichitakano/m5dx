/*
 * author : Shuichi TAKANO
 * since  : Sun Oct 21 2018 20:36:39
 */
#ifndef _6334CD6A_A133_F008_136B_B244FD882788
#define _6334CD6A_A133_F008_136B_B244FD882788

#include <array>
#include <stdint.h>

namespace audio
{

void initialize();
void setFMClock(uint32_t freq, int sampleRateDiv);
void setFMVolume(float v);
void attachInternalSpeaker();

// const int16_t* getRecentSampleForTest();

} // namespace audio

#endif /* _6334CD6A_A133_F008_136B_B244FD882788 */
