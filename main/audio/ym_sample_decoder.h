/*
 * author : Shuichi TAKANO
 * since  : Sun Nov 18 2018 4:18:30
 */
#ifndef B90B7CC4_E133_F06C_3FD5_2F07271E7942
#define B90B7CC4_E133_F06C_3FD5_2F07271E7942

#include <stdint.h>
#include <stdlib.h>

namespace audio
{

void decodeYM3012Sample(int16_t* dst, const uint32_t* src, size_t count);
void decodeYMF288Sample(int16_t* dst, const uint32_t* src, size_t count);

} // namespace audio

#endif /* B90B7CC4_E133_F06C_3FD5_2F07271E7942 */
