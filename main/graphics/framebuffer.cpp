/*
 * author : Shuichi TAKANO
 * since  : Sun Oct 28 2018 17:57:51
 */

#include "framebuffer.h"
#include "display.h"

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
    img_.createSprite(w, h);
    setWindow(0, 0, w, h);
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
    // todo
    return ((r & 0xf8) << 8) | ((g & 0xfc) << 3) | (b >> 3);
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

bool
FrameBuffer::_blitToLCD(InternalLCD*, int x, int y) const
{
    const_cast<Img*>(&img_)->pushSprite(x, y, false);
    return true;
}

} // namespace graphics
