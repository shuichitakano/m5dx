/*
 * author : Shuichi TAKANO
 * since  : Sun Oct 28 2018 21:3:2
 */

#include "framebuffer_base.h"
#include "../debug.h"
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
FrameBufferBase::blit(const FrameBufferBase& fb, int x, int y)
{
    int wx0 = getLeft();
    int wx1 = wx0 + getWidth();
    int wy0 = getTop();
    int wy1 = wy0 + getHeight();

    int dx0 = x;
    int dx1 = x + fb.getWidth();
    int dy0 = y;
    int dy1 = y + fb.getHeight();

    int sx0 = 0;
    int sy0 = 0;

    if (dx0 < wx0)
    {
        sx0 += wx0 - dx0;
        dx0 = wx0;
    }
    if (dx1 > wx1)
    {
        dx1 = wx1;
    }
    if (dy0 < wy0)
    {
        sy0 += wy0 - dy0;
        dy0 = wy0;
    }
    if (dy1 > wy1)
    {
        dy1 = wy1;
    }

    auto dy = dy0;
    auto sy = sy0;
    while (dy < dy1)
    {
        auto dx = dx0;
        auto sx = sx0;
        while (dx < dx1)
        {
            setPixel(dx, dy, getPixel(sx, sy));
            ++dx;
            ++sx;
        }
        ++dy;
        ++sy;
    }
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
