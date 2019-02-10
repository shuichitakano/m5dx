/*
 * author : Shuichi TAKANO
 * since  : Sun Oct 28 2018 16:15:16
 */

#include "font_manager.h"
#include "font_data.h"
#include "framebuffer.h"
#include <debug.h>

namespace graphics
{

namespace
{
int
sjis2jis(int ch)
{
    int c0 = ch >> 8;
    int c1 = ch & 255;

    if (c0 >= 0xe0)
        c0 -= 0x40;
    c0 -= 0x81;
    c0 <<= 1;

    if (c1 >= 0x80)
        c1 -= 1;
    if (c1 >= 0x9e)
    {
        c0 += 1;
        c1 -= 0x9e;
    }
    else
        c1 -= 0x40;
    return ((c0 << 8) | c1) + 0x2121;
}

} // namespace

void
FontManager::putFont(int code, const FontData* f)
{
    //    DBOUT(("(%d, %d) %d %p\n", x_, y_, code, f));
    if (!f)
    {
        return;
    }

    int w = f->width();
    int h = f->height();

    if (code == '\n' || x_ + w > vx_ + vw_)
    {
        if (!multiLine_)
        {
            return;
        }
        x_ = vx_;
        y_ += h;
    }
    if (y_ + h > vy_ + vh_)
    {
        return;
    }

    auto& fb         = *frameBuffer_;
    const uint8_t* p = f->getGlyph(code);
    if (p)
    {
        if (edged_)
        {
            fb.drawBitsThick(x_, y_, w, h, p, color_, bgColor_);
        }
        else if (transparent_)
        {
            fb.drawBits(x_, y_, w, h, p, color_);
        }
        else
        {
            fb.drawBits(x_, y_, w, h, p, color_, bgColor_);
        }
    }
    else
    {
        if (!transparent_ && !edged_)
        {
            fb.fill(x_, y_, w, h, bgColor_);
        }
    }
    x_ += w;
}

void
FontManager::putAscii(int code)
{
    putFont(code, asciiFont_);
}

void
FontManager::putKanji(int code)
{
    putFont(code, kanjiFont_);
}

void
FontManager::put(int code)
{
    if (code < 0x80)
        putAscii(code);
    else
        putKanji(code);
}

void
FontManager::putString(const char* str)
{
    int code   = 0;
    bool shift = false;
    while (int c = (uint8_t)*str)
    {
        int column = int(c) & 0xf0;
        if (!shift && (column == 0x80 || column == 0x90 || column == 0xe0 ||
                       column == 0xf0))
        {
            shift = true;
            code  = c << 8;
        }
        else
        {
            if (shift)
                putKanji(sjis2jis(code | c));
            else
                putAscii(c);
            shift = false;
        }
        ++str;
    }
}

void
FontManager::computeTextSize(const char* str, int& w, int& h)
{
    w = 0;
    h = 0;
    if (!asciiFont_ || !kanjiFont_)
        return;

    /* かり */
    w = strlen(str) * kanjiFont_->width() >> 1;
    h = kanjiFont_->height();
}

void
FontManager::setFrameBuffer(FrameBufferBase* fb)
{
    frameBuffer_ = fb;

    if (fb)
    {
        vx_ = fb->getLeft();
        vy_ = fb->getTop();
        vw_ = fb->getWidth();
        vh_ = fb->getHeight();
        x_  = vx_;
        y_  = vy_;
    }
}

FontManager&
getDefaultFontManager()
{
    static FontManager inst;
    return inst;
}

} // namespace graphics
