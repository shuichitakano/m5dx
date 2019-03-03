/*
 * author : Shuichi TAKANO
 * since  : Sun Oct 28 2018 17:57:51
 */

#include "framebuffer.h"
#include "display.h"
#include <algorithm>
#include <debug.h>

namespace graphics
{
FrameBuffer::FrameBuffer()
    : img_(getDisplay()._getLCD()) // やむなく
{
}

FrameBuffer::FrameBuffer(uint32_t w, uint32_t h, int bpp)
    : FrameBuffer()
{
    initialize(w, h, bpp);
}

void
FrameBuffer::initialize(uint32_t w, uint32_t h, int bpp)
{
    release();

    img_.setColorDepth(bpp);
    buffer_ = img_.createSprite(w, h);
    bpp_    = bpp;
    bw_     = w;
    bh_     = h;
    setWindow(0, 0, w, h);
}

void
FrameBuffer::release()
{
    img_.deleteSprite();
    buffer_ = nullptr;
    bpp_    = 0;
}

void
FrameBuffer::setWindow(uint32_t x, uint32_t y, uint32_t w, uint32_t h)
{
    wx_ = x;
    wy_ = y;
    ww_ = w;
    wh_ = h;
}

uint32_t
FrameBuffer::getLeft() const
{
    return wx_;
}

uint32_t
FrameBuffer::getTop() const
{
    return wy_;
}

uint32_t
FrameBuffer::getWidth() const
{
    return ww_;
}

uint32_t
FrameBuffer::getHeight() const
{
    return wh_;
}

uint32_t
FrameBuffer::getBufferWidth() const
{
    return bw_;
}

uint32_t
FrameBuffer::getBufferHeight() const
{
    return bh_;
}

uint32_t
FrameBuffer::makeColor(int r, int g, int b) const
{
#if 0
    switch (bpp_)
    {
    case 8:
        return (r & 224) | ((g & 224) >> 3) | (b >> 6);

    case 16:
        return ((r & 0xf8) << 8) | ((g & 0xfc) << 3) | (b >> 3);

    default:
        return (r << 16) | (g << 8) | b;
    }
#else
    // Sprite は 常に16bit colorを受ける
    return ((r & 0xf8) << 8) | ((g & 0xfc) << 3) | (b >> 3);
#endif
}

void
FrameBuffer::fill(uint32_t c)
{
    img_.fillSprite(c);
}

void
FrameBuffer::fill(int x, int y, int w, int h, uint32_t c)
{
    img_.fillRect(x, y, w, h, c);
}

void
FrameBuffer::setPixel(uint32_t x, uint32_t y, uint32_t c)
{
    img_.drawPixel(x, y, c);
}

uint32_t
FrameBuffer::getPixel(uint32_t x, uint32_t y) const
{
    return const_cast<Img*>(&img_)->readPixel(x, y);
}

void
FrameBuffer::transferTo(
    FrameBufferBase& dst, int dx, int dy, int sx, int sy, int w, int h) const
{
    if (bpp_ != 16)
    {
        FrameBufferBase::transferTo(dst, dx, dy, sx, sy, w, h);
        return;
    }

    if (w == 0)
    {
        w = getBufferWidth();
    }
    if (h == 0)
    {
        h = getBufferHeight();
    }

    auto p = (uint16_t*)buffer_;
    p += sx + sy * bw_;
    dst.drawBits16(dx, dy, w, h, bw_ << 1, p);
}

} // namespace graphics
