/*
 * author : Shuichi TAKANO
 * since  : Mon Jan 14 2019 3:26:8
 */

#include "sampling_rate_converter.h"
#include <debug.h>

namespace audio
{

namespace
{
static constexpr int FP_SHIFT       = 23;
static constexpr uint32_t RATE_MASK = (1 << FP_SHIFT) - 1;
} // namespace

void
SimpleLinearSamplingRateConverter::setSamplingStep(float r)
{
    srcDeltaPos_ = uint32_t(r * (1 << FP_SHIFT));
}

void
SimpleLinearSamplingRateConverter::setScale(float r)
{
    //  scale_ = uint16_t (r * 65535.0f);
    scale_ = uint16_t(r * 256.0f);
}

bool
SimpleLinearSamplingRateConverter::convertAccumMono(
    DstSample* dst,
    uint32_t dstSampleCount,
    util::RingBuffer<int16_t>& src,
    int lrMask)
{
    //  uint32_t srcOfs = src.getReadOffset ();
    //  uint32_t srcOfsMask = src.getBufferSize () - 1;
    uint32_t srcOfsMask = src.getBufferSize() - 1;

    uint32_t srcSizeFP =
        (currentSrcPos_ & RATE_MASK) + dstSampleCount * srcDeltaPos_;
    uint32_t srcConsume = srcSizeFP >> FP_SHIFT;
    if (src.getFullReadableSize() < srcConsume + 1)
        return false;

    uint16_t ls = (lrMask & 2) ? 0 : scale_;
    uint16_t rs = (lrMask & 1) ? 0 : scale_;

    const auto* sp = src.getBufferTop();
    auto* dstTail  = dst + dstSampleCount;

    while (dst != dstTail)
    {
        uint32_t ofs  = currentSrcPos_ >> FP_SHIFT;
        uint16_t rate = currentSrcPos_ >> (FP_SHIFT - 16);

        int16_t v0 = sp[(ofs + 0) & srcOfsMask];
        int16_t v1 = sp[(ofs + 1) & srcOfsMask];

        //      int16_t v = v0 + ((rate * (v1 - v0)) >> 16);
        int16_t v = ((65536 - rate) * v0 + (rate * v1)) >> 16;
        int32_t l = v * ls;
        int32_t r = v * rs;
        //      int32_t l = v0 * ls;
        //      int32_t r = v0 * rs;

        (*dst)[0] += l;
        (*dst)[1] += r;
        ++dst;

        currentSrcPos_ += srcDeltaPos_;
    }
    src.advanceReadPointer(srcConsume);

    return true;
}

bool
SimpleLinearSamplingRateConverter::convertAccum(DstSample* dst,
                                                uint32_t dstSampleCount,
                                                util::RingBuffer<int16_t>& src)
{
    uint32_t srcOfsMask = src.getBufferSize() - 1;

    uint32_t readableSize = src.getFullReadableSize();
    uint32_t srcSizeFP =
        (currentSrcPos_ & RATE_MASK) + dstSampleCount * srcDeltaPos_;
    uint32_t srcConsume = srcSizeFP >> FP_SHIFT << 1;
    if (readableSize < srcConsume + 2)
    {
        DBOUT((" src failed: %d sample, readable %d, consume %d\n",
               dstSampleCount,
               readableSize,
               srcConsume));
        return false;
    }

    const auto* sp = src.getBufferTop();
    auto* dstTail  = dst + dstSampleCount;

    while (dst != dstTail)
    {
        uint32_t ofs  = currentSrcPos_ >> FP_SHIFT << 1;
        uint16_t rate = currentSrcPos_ >> (FP_SHIFT - 16);

        int16_t v0l = sp[(ofs + 0) & srcOfsMask];
        int16_t v1l = sp[(ofs + 2) & srcOfsMask];

        int16_t v0r = sp[(ofs + 1) & srcOfsMask];
        int16_t v1r = sp[(ofs + 3) & srcOfsMask];

        int16_t l = (((65536 - rate) * v0l + (rate * v1l)) >> 16);
        int16_t r = (((65536 - rate) * v0r + (rate * v1r)) >> 16);

        (*dst)[0] += l * scale_;
        (*dst)[1] += r * scale_;
        ++dst;

        currentSrcPos_ += srcDeltaPos_;
    }

    src.advanceReadPointer(srcConsume);

    return true;
}

bool
SimpleLinearSamplingRateConverter::convertNearestToMono(
    util::SimpleRingBuffer<int16_t>& dst,
    const int32_t* src,
    uint32_t srcSampleCount)
{
    int dstOfsMask = dst.getMask();
    int dstOfs     = dst.getCurrentPos();
    auto dstTop    = dst.getBufferTop();

    uint32_t srcTailPos = srcSampleCount << FP_SHIFT;
    uint32_t pos        = currentSrcPos_;

    while (pos < srcTailPos)
    {
        uint32_t ofs = pos >> FP_SHIFT << 1;
        int32_t l    = src[ofs + 0] >> 17;
        int32_t r    = src[ofs + 1] >> 17;
        int16_t v    = l + r;

        dstTop[dstOfs] = v;

        pos += srcDeltaPos_;
        dstOfs = (dstOfs + 1) & dstOfsMask;
    }

    currentSrcPos_ = pos & (srcTailPos - 1);
    dst.setCurrentPos(dstOfs);
    return true;
}

} // namespace audio
