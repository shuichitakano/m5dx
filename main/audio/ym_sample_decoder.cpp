/*
 * author : Shuichi TAKANO
 * since  : Sun Nov 18 2018 4:19:37
 */

#include "ym_sample_decoder.h"

namespace audio
{

inline uint32_t
reverseBit16_16(uint32_t v)
{
    v = (((v & 0xaaaaaaaa) >> 1) | ((v & 0x55555555) << 1));
    v = (((v & 0xcccccccc) >> 2) | ((v & 0x33333333) << 2));
    v = (((v & 0xf0f0f0f0) >> 4) | ((v & 0x0f0f0f0f) << 4));
    v = (((v & 0xff00ff00) >> 8) | ((v & 0x00ff00ff) << 8));
    return v;
}

inline int16_t
decode(uint16_t v)
{
    // [14:12] : exp
    // [11:2]  : mantissa
#if 0
    int m = (v >> 2) & 1023;
    int e = (v >> 12) & 7;
    return (m - 512) << (e - 1);
#else
    v ^= 15 << 11;
    int16_t m = ((v >> 2) & 1023) << 6;
    int e     = (v >> 12) & 7;
    return m >> e;
#endif
    // 1802
    // 0001100000000010
    // 4018
    // 0100000000011000
}

void
decodeYM3012Sample(int16_t* dst, const uint32_t* src, size_t count)
{
    while (count)
    {
        // LSB側がCH0
        auto s  = *src;
        auto rs = reverseBit16_16(s);
        dst[0]  = decode(rs & 0xffff);
        dst[1]  = decode(rs >> 16);

        src += 1;
        dst += 2;
        --count;
    }
}

} // namespace audio
