/*
 * author : Shuichi TAKANO
 * since  : Sun Jan 06 2019 21:2:12
 */
#ifndef _0CAA5930_8134_1394_140F_BB47204AD02C
#define _0CAA5930_8134_1394_140F_BB47204AD02C

#include <stdint.h>

namespace sys
{

void yield();
uint32_t micros();
uint32_t millis();
void delay(uint32_t ms);
void delayMicroseconds(uint32_t us);

inline uint32_t
getCycleCount()
{
    uint32_t ccount;
    __asm__ __volatile__("esync; rsr %0,ccount" : "=a"(ccount));
    // __asm__ __volatile__("rsr %0,ccount" : "=a"(ccount));
    return ccount;
}

} // namespace sys

#endif /* _0CAA5930_8134_1394_140F_BB47204AD02C */
