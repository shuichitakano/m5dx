/*
 * author : Shuichi TAKANO
 * since  : Sun Oct 28 2018 17:57:51
 */

#include "framebuffer.h"
#include "display.h"
#include <algorithm>

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
    return const_cast<Img*>(&img_)->width();
}

uint32_t
FrameBuffer::getBufferHeight() const
{
    return const_cast<Img*>(&img_)->height();
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
FrameBuffer::blit(const FrameBufferBase& fb, int x, int y)
{
    // todo
}

void
FrameBuffer::pushImage(TFT_eSPI* lcd,
                       int dx,
                       int dy,
                       int w,
                       int h,
                       void* p,
                       size_t stride,
                       int wx,
                       int wy) const
{
    auto draw = [&](int x, int y, int w, int h, uint8_t* p) {
        if (bpp_ == 16)
        {
            lcd->pushImage(x, y, w, h, (uint16_t*)p);
        }
        else
        {
            lcd->pushImage(x, y, w, h, p, bpp_ == 8);
        }
    };

    auto pp = (uint8_t*)p;

    if (dy < wy)
    {
        int d = wy - dy;
        h -= d;
        if (h <= 0)
        {
            return;
        }
        dy = wy;
        pp += stride * d;
    }

    if (dx < wx)
    {
        int d = wx - dx;
        if (bpp_ == 1)
        {
            d &= ~7;
        }
        w -= d;
        if (w <= 0)
        {
            return;
        }
        dx = wx;
        pp += (bpp_ * d) >> 3;

        for (; h; --h)
        {
            draw(dx, dy, w, 1, pp);
            pp += stride;
        }
    }
    else
    {
        draw(dx, dy, w, h, pp);
    }
}

bool
FrameBuffer::_blitToLCD(
    InternalLCD* ilcd, int x, int y, int wx, int wy, int ww, int wh) const
{
    //    const_cast<Img *>(&img_)->pushSprite(x, y, false);
    auto* lcd     = (TFT_eSPI*)ilcd;
    auto w        = getWidth();
    auto h        = getHeight();
    auto y1       = y + h;
    auto unitLine = unitTransferPixels_ / w;
    auto p        = (uint8_t*)buffer_;

    size_t stride       = (w * bpp_ + 7) >> 3;
    size_t unitLineSize = stride * unitLine;

    while (y < y1)
    {
        auto yn = std::min<int>(y1, y + unitLine);
        auto ch = yn - y;

#if 0
        if (bpp_ == 16)
        {
            lcd->pushImage(x, y, w, ch, (uint16_t*)p);
        }
        else
        {
            lcd->pushImage(x, y, w, ch, p, bpp_ == 8);
        }
#else
        pushImage(lcd, x, y, w, ch, p, stride, wx, wy);
#endif

        y = yn;
        p += unitLineSize;
    }

    return true;
}

} // namespace graphics
