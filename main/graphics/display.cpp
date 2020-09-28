/*
 * author : Shuichi TAKANO
 * since  : Sun Oct 28 2018 17:49:55
 */

#include "display.h"
#include "../debug.h"
#include "texture.h"

namespace graphics
{

namespace
{
constexpr int BACKLIGHT_PWM_CH = 7;
constexpr int BACKLIGHT_FREQ   = 100000;

} // namespace

bool
Display::initialize()
{
    display_.begin();
    ledcSetup(BACKLIGHT_PWM_CH, BACKLIGHT_FREQ, 8);

    setBackLightIntensity(80);

    setWindow(0, 0, getBufferWidth(), getBufferHeight());
    return true;
}

void
Display::setBackLightIntensity(int v)
{
    ledcWrite(BACKLIGHT_PWM_CH, v);
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
        static constexpr uint32_t unitTransferPixels = 1280;
        // 40Mで2500pixelが1ms

        int unitLine = unitTransferPixels / w;
        while (h)
        {
            auto hh = std::min(unitLine, h);
            _getLCD()->pushImage(x, y, w, hh, (uint16_t*)p);
            y += hh;
            p += pitchInBytes * hh;
            h -= hh;
        }
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

namespace
{
DRAM_ATTR uint16_t workBuffer_[320];

uint16_t
swapEndian(uint16_t c)
{
    return (c >> 8) | (c << 8);
}

} // namespace

void
Display::put(const Texture& tex, int dx, int dy, int sx, int sy, int w, int h)
{
    adjustTransferRegion(dx, dy, sx, sy, w, h);
    if (!w || !h)
    {
        return;
    }

    auto srcPitch   = tex.getPitch();
    const auto* src = tex.getBits() + sx + srcPitch * sy;
    const auto* pal = tex.getPalette();

    for (int y = 0; y < h; ++y)
    {
        uint16_t* p = workBuffer_;
        for (int x = 0; x < w; ++x)
        {
            *p++ = swapEndian(pal[*(src + x)]);
        }
        _getLCD()->pushImage(dx, dy + y, w, 1, workBuffer_);
        src += srcPitch;
    }
}

void
Display::putReplaced(const Texture& tex,
                     int dx,
                     int dy,
                     int sx,
                     int sy,
                     int w,
                     int h,
                     uint16_t color,
                     uint16_t bg)
{
    adjustTransferRegion(dx, dy, sx, sy, w, h);
    if (!w || !h)
    {
        return;
    }

    color = swapEndian(color);
    bg    = swapEndian(bg);

    auto srcPitch   = tex.getPitch();
    const auto* src = tex.getBits() + sx + srcPitch * sy;

    for (int y = 0; y < h; ++y)
    {
        uint16_t* p = workBuffer_;
        for (int x = 0; x < w; ++x)
        {
            *p++ = *(src + x) ? color : bg;
        }
        _getLCD()->pushImage(dx, dy + y, w, 1, workBuffer_);
        src += srcPitch;
    }
}

Display&
getDisplay()
{
    static Display inst;
    return inst;
}

} // namespace graphics
