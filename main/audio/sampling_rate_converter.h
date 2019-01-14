/*
 * author : Shuichi TAKANO
 * since  : Mon Jan 14 2019 3:11:28
 */
#ifndef _50F3D230_5134_1395_2F82_8DAED7E7994E
#define _50F3D230_5134_1395_2F82_8DAED7E7994E

#include <array>
#include <stdint.h>
#include <util/ring_buffer.h>
#include <util/simple_ring_buffer.h>

namespace audio
{

class SimpleLinearSamplingRateConverter
{
    uint32_t currentSrcPos_; // 9.23 fixed point
    uint32_t srcDeltaPos_;

    uint16_t scale_;

    using DstSample = std::array<int32_t, 2>; // 2ch stereo, 16.8

public:
    SimpleLinearSamplingRateConverter(float step = 1.0f, float scale = 1.0f)
        : currentSrcPos_(0)
    {
        setSamplingStep(step);
        setScale(scale);
    }

    void setSamplingStep(float r);
    void setScale(float r);

    bool convertAccumMono(DstSample* dst,
                          uint32_t dstCount,
                          util::RingBuffer<int16_t>& src,
                          int lrMask);

    bool convertAccum(DstSample* dst,
                      uint32_t dstSampleCount,
                      util::RingBuffer<int16_t>& src);

    bool convertNearestToMono(util::SimpleRingBuffer<int16_t>& dst,
                              const int32_t* src,
                              uint32_t srcSampleCount);
};

} // namespace audio

#endif /* _50F3D230_5134_1395_2F82_8DAED7E7994E */
