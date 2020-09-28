/*
 * author : Shuichi TAKANO
 * since  : Sun Oct 28 2018 21:3:2
 */

#include "framebuffer_base.h"
#include "../debug.h"
#include "texture.h"
#include <algorithm>

namespace graphics
{

void
FrameBufferBase::fill(int x, int y, int w, int h, uint32_t c)
{
    int wx0 = getLeft();
    int wx1 = wx0 + getWidth();
    int wy0 = getTop();
    int wy1 = wy0 + getHeight();

    int x0 = std::max(wx0, x);
    int x1 = std::min(wx1, x + w);
    int y0 = std::max(wy0, y);
    int y1 = std::min(wy1, y0 + h);

    while (y0 < y1)
    {
        auto tx = x0;
        while (tx < x1)
        {
            setPixel(tx, y0, c);
            ++tx;
        }
        ++y0;
    }
}

void
FrameBufferBase::fill(uint32_t c)
{
    fill(0, 0, getWidth(), getHeight(), c);
}

void
FrameBufferBase::adjustTransferRegion(
    int& dx, int& dy, int& sx, int& sy, int& w, int& h) const
{
    int wx0 = getLeft();
    int wx1 = wx0 + getWidth();
    int wy0 = getTop();
    int wy1 = wy0 + getHeight();

    int dx1 = dx + w;
    int dy1 = dy + h;

    if (dx < wx0)
    {
        sx += wx0 - dx;
        dx = wx0;
    }
    if (dx1 > wx1)
    {
        dx1 = wx1;
    }
    if (dy < wy0)
    {
        sy += wy0 - dy;
        dy = wy0;
    }
    if (dy1 > wy1)
    {
        dy1 = wy1;
    }
    w = dx1 - dx;
    h = dy1 - dy;
}

void
FrameBufferBase::drawBits16(
    int x, int y, int w, int h, int pitchInBytes, const void* img16)
{
    // todo
}

void
FrameBufferBase::transferTo(
    FrameBufferBase& dst, int dx, int dy, int sx, int sy, int w, int h) const
{
    if (w == 0)
    {
        w = getBufferWidth();
    }
    if (h == 0)
    {
        h = getBufferHeight();
    }
    dst.adjustTransferRegion(dx, dy, sx, sy, w, h);
    if (!w || !h)
    {
        return;
    }
    for (int y = 0; y < h; ++y)
    {
        for (int x = 0; x < w; ++x)
        {
            dst.setPixel(dx + x, dy + y, getPixel(sx + x, sy + y));
            // todo: フォーマット変換
        }
    }
}

void
FrameBufferBase::put(
    const Texture& tex, int dx, int dy, int sx, int sy, int w, int h)
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
        for (int x = 0; x < w; ++x)
        {
            auto c = pal[*(src + x)];
            setPixel(dx + x, dy + y, c);
        }
        src += srcPitch;
    }
}

void
FrameBufferBase::putTrans(
    const Texture& tex, int dx, int dy, int sx, int sy, int w, int h)
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
        for (int x = 0; x < w; ++x)
        {
            if (auto p = *(src + x))
            {
                auto c = pal[p];
                setPixel(dx + x, dy + y, c);
            }
        }
        src += srcPitch;
    }
}

void
FrameBufferBase::putReplaced(const Texture& tex,
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

    auto srcPitch   = tex.getPitch();
    const auto* src = tex.getBits() + sx + srcPitch * sy;

    for (int y = 0; y < h; ++y)
    {
        for (int x = 0; x < w; ++x)
        {
            auto c = *(src + x);
            setPixel(dx + x, dy + y, c ? color : bg);
        }
        src += srcPitch;
    }
}

void
FrameBufferBase::transfer(
    const FrameBufferBase& src, int dx, int dy, int sx, int sy, int w, int h)
{
    src.transferTo(*this, dx, dy, sx, sy, w, h);
}

void
FrameBufferBase::_drawBitsLineTrans(
    const uint8_t* bits, int bitOfs, uint32_t color, int x, int y, int w)
{
    while (w)
    {
        int v = bits[bitOfs >> 3];
        int b = bitOfs & 7;
        if ((v << b) & 128)
        {
            setPixel(x, y, color);
        }
        ++bitOfs;
        ++x;
        --w;
    }
}

void
FrameBufferBase::_drawBitsLine(const uint8_t* bits,
                               int bitOfs,
                               uint32_t color,
                               uint32_t bgColor,
                               int x,
                               int y,
                               int w)
{
    while (w)
    {
        int v = bits[bitOfs >> 3];
        int b = bitOfs & 7;
        if ((v << b) & 128)
        {
            setPixel(x, y, color);
        }
        else
        {
            setPixel(x, y, bgColor);
        }
        ++bitOfs;
        ++x;
        --w;
    }
}

void
FrameBufferBase::drawBits(
    int x, int y, int w, int h, const uint8_t* bits, uint32_t color)
{
    int wx0 = getLeft();
    int wx1 = wx0 + getWidth();
    int wy0 = getTop();
    int wy1 = wy0 + getHeight();

    int dx1 = x + w;
    int dy1 = y + h;

    int pitch      = (w + 7) >> 3;
    int lineBitOfs = 0;

    if (x < wx0)
    {
        lineBitOfs = wx0 - x;
        x          = wx0;
    }

    if (y < wy0)
    {
        bits += (wy0 - y) * pitch;
        y = wy0;
    }

    dx1 = std::min(dx1, wx1);
    dy1 = std::min(dy1, wy1);

    w = dx1 - x;

    if (w <= 0 || dy1 - y <= 0)
    {
        return;
    }

    while (y < dy1)
    {
        _drawBitsLineTrans(bits, lineBitOfs, color, x, y, w);
        bits += pitch;
        ++y;
    }
}

void
FrameBufferBase::drawBits(int x,
                          int y,
                          int w,
                          int h,
                          const uint8_t* bits,
                          uint32_t color,
                          uint32_t bgColor)
{
    int wx0 = getLeft();
    int wx1 = wx0 + getWidth();
    int wy0 = getTop();
    int wy1 = wy0 + getHeight();

    int dx1 = x + w;
    int dy1 = y + h;

    int pitch      = (w + 7) >> 3;
    int lineBitOfs = 0;

    if (x < wx0)
    {
        lineBitOfs = wx0 - x;
        x          = wx0;
    }

    if (y < wy0)
    {
        bits += (wy0 - y) * pitch;
        y = wy0;
    }

    dx1 = std::min(dx1, wx1);
    dy1 = std::min(dy1, wy1);

    w = dx1 - x;

    if (w <= 0 || dy1 - y <= 0)
    {
        return;
    }

    while (y < dy1)
    {
        _drawBitsLine(bits, lineBitOfs, color, bgColor, x, y, w);
        bits += pitch;
        ++y;
    }
}

void
FrameBufferBase::drawBitsThick(int x,
                               int y,
                               int w,
                               int h,
                               const uint8_t* bits,
                               uint32_t color,
                               uint32_t edgeColor)
{
    drawBits(x - 1, y, w, h, bits, edgeColor);
    drawBits(x + 1, y, w, h, bits, edgeColor);
    drawBits(x, y - 1, w, h, bits, edgeColor);
    drawBits(x, y + 1, w, h, bits, edgeColor);
    drawBits(x, y, w, h, bits, color);
}

} // namespace graphics
