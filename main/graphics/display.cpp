/*
 * author : Shuichi TAKANO
 * since  : Sun Oct 28 2018 17:49:55
 */

#include "display.h"
#include "../debug.h"

namespace graphics
{

bool
Display::initialize()
{
    display_.begin();
    setWindow(0, 0, getBufferWidth(), getBufferHeight());
    return true;
}

void
Display::setWindow(uint32_t x, uint32_t y, uint32_t w, uint32_t h)
{
    wx_ = x;
    wy_ = y;
    ww_ = w;
    wh_ = h;
}

uint32_t
Display::getLeft() const
{
    return wx_;
}

uint32_t
Display::getTop() const
{
    return wy_;
}

uint32_t
Display::getWidth() const
{
    return ww_;
}

uint32_t
Display::getHeight() const
{
    return wh_;
}

uint32_t
Display::getBufferWidth() const
{
    return 320;
}

uint32_t
Display::getBufferHeight() const
{
    return 240;
}

uint32_t
Display::makeColor(int r, int g, int b) const
{
    return ((r & 0xf8) << 8) | ((g & 0xfc) << 3) | (b >> 3);
}

void
Display::fill(uint32_t c)
{
    display_.fillScreen(c);
}

void
Display::fill(int x, int y, int w, int h, uint32_t c)
{
    display_.fillRect(x, y, w, h, c);
}

void
Display::setPixel(uint32_t x, uint32_t y, uint32_t c)
{
    display_.drawPixel(x, y, c);
}
uint32_t
Display::getPixel(uint32_t x, uint32_t y) const
{
    return const_cast<Display*>(this)->display_.readPixel(x, y);
}

void
Display::drawBits16(
    int x, int y, int w, int h, int pitchInBytes, const void* img16)
{
    int sx = 0;
    int sy = 0;
    adjustTransferRegion(x, y, sx, sy, w, h);
    if (!w || !h)
    {
        return;
    }

    auto p = (uint8_t*)img16;
    p += sx << 1;
    p += sy * pitchInBytes;

    if (pitchInBytes == w << 1)
    {
        _getLCD()->pushImage(x, y, w, h, (uint16_t*)p);
    }
    else
    {
        for (; h; --h)
        {
            _getLCD()->pushImage(x, y, w, 1, (uint16_t*)p);
            ++y;
            p += pitchInBytes;
        }
    }
}

Display&
getDisplay()
{
    static Display inst;
    return inst;
}

} // namespace graphics
