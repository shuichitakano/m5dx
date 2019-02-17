/*
 * author : Shuichi TAKANO
 * since  : Sun Feb 17 2019 2:38:24
 */
#ifndef _52A47F05_4134_13F8_245B_5E8991F9011E
#define _52A47F05_4134_13F8_245B_5E8991F9011E

#include <algorithm>
#include <stdint.h>

namespace sound_sys
{

class M6258Coder
{
    int currentValue_;
    int predictIdx_;

public:
    M6258Coder()
        : currentValue_(0)
        , predictIdx_(0)
    {
    }

    void reset()
    {
        currentValue_ = 0;
        predictIdx_   = 0;
    }

    template <int SHIFT, class T>
    void decode(T* dst, const uint8_t* src, int srcSize, int scale = 16)
    {
        while (srcSize)
        {
            int x  = *src++;
            dst[0] = update(x & 15) * scale >> SHIFT;
            dst[1] = update(x >> 4) * scale >> SHIFT;
            dst += 2;
            --srcSize;
        }
    }

    template <int SHIFT, class T>
    inline void decodeByte(T& s0, T& s1, int data, int scale = 16)
    {
        s0 = update(data & 15) * scale >> SHIFT;
        s1 = update(data >> 4) * scale >> SHIFT;
    }

    template <int SHIFT, class T>
    void encode(uint8_t* dst, const T* src, int srcSize)
    {
        while (srcSize >= 2)
        {
            int v0 = encodeSample(
                std::min(2047, std::max(-2048, (int)src[0] >> SHIFT)));
            int v1 = encodeSample(
                std::min(2047, std::max(-2048, (int)src[1] >> SHIFT)));
            *dst++ = v0 | (v1 << 4);
            src += 2;
            srcSize -= 2;
        }
    }

    template <int SHIFT, class T>
    inline uint8_t encodeByte(T s0, T s1)
    {
        int v0 =
            encodeSample(std::min(2047, std::max(-2048, (int)s0 >> SHIFT)));
        int v1 =
            encodeSample(std::min(2047, std::max(-2048, (int)s1 >> SHIFT)));
        return v0 | (v1 << 4);
    }

    int update(int x);
    int encodeSample(int v);
};

} // namespace sound_sys

#endif /* 52A47F05_4134_13F8_245B_5E8991F9011E */
