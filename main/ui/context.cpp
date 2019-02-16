/*
 * author : Shuichi TAKANO
 * since  : Fri Feb 01 2019 2:33:15
 */

#include "context.h"
#include "key.h"
#include <debug.h>

namespace ui
{

void
Context::setFontRegion(const Vec2& pos, const Dim2& size)
{
    auto& fm = getFontManager();
    auto p   = getCurrentPosition() + pos;
    fm.setWindow(p.x, p.y, size.w, size.h);
    fm.setPosition(p.x, p.y);
}

///////////////

void
UpdateContext::acceptLongPress()
{
    if (keyState_)
    {
        keyState_->acceptLongPress();
    }
}

/////////////////

RenderContext::RenderContext()
{
    invalidatedRegion_.invalidate();
}

void
RenderContext::updateInvalidatedRegion(const Dim2& size)
{
    BBox b(getCurrentPosition(), size);
    invalidatedRegion_.update(b);
}

bool
RenderContext::isInvalidated(const Dim2& size) const
{
    return invalidatedRegion_.isIntersect({getCurrentPosition(), size});
}

void
RenderContext::setFrameBuffer(graphics::FrameBufferBase* b)
{
    frameBuffer_ = b;
    getFontManager().setFrameBuffer(b);
}

RenderContext::ScopeRestoreFrameBuffer
RenderContext::updateFrameBuffer(graphics::FrameBufferBase* b)
{
    FBRestoreState prev{
        getCurrentPosition(), getCurrentClipRegion(), frameBuffer_};
    setFrameBuffer(b);
    setCurrentPosition({0, 0});
    setCurrentClipRegion({Vec2{0, 0}, Dim2{b->getWidth(), b->getHeight()}});
    return {this, prev};
}

graphics::FrameBuffer&
RenderContext::getTemporaryFrameBuffer(uint32_t w, uint32_t h, int bpp)
{
    if (temporaryFrameBuffer_.getWidth() != w ||
        temporaryFrameBuffer_.getHeight() != h ||
        temporaryFrameBuffer_.getBitPerPixel() != bpp)
    {
        temporaryFrameBuffer_.initialize(w, h, bpp);
    }
    return temporaryFrameBuffer_;
}

graphics::FrameBuffer&
RenderContext::getTemporaryFrameBuffer(const Dim2& size, int bpp)
{
    return getTemporaryFrameBuffer(size.w, size.h, bpp);
}

void
RenderContext::applyClipRegion()
{
    auto* fb = getFrameBuffer();
    auto& r  = getCurrentClipRegion();
    fb->setWindow(r.p[0].x, r.p[0].y, r.getWidth(), r.getHeight());
}

uint32_t
RenderContext::makeColor(uint32_t c) const
{
    return graphics::makeColor(*frameBuffer_, c);
}

void
RenderContext::setFontColor(uint32_t c)
{
    getFontManager().setColor(makeColor(c));
}

void
RenderContext::setFontBGColor(uint32_t c)
{
    getFontManager().setBGColor(makeColor(c));
}

void
RenderContext::putText(
    const char* str, Vec2 pos, Dim2 size, TextAlignH alignH, TextAlignV alignV)
{
    if (!str || !str[0])
    {
        return;
    }

    auto& fm = getFontManager();

    pos += getCurrentPosition();

    int tw, th;
    fm.computeTextSize(str, tw, th);

    int ox = 0;
    switch (alignH)
    {
    case TextAlignH::LEFT:
        break;

    case TextAlignH::CENTER:
        ox = std::max<int>(0, size.w - tw) >> 1;
        break;

    case TextAlignH::RIGHT:
        ox = std::max<int>(0, size.w - tw);
        break;
    };
    pos.x += ox;
    size.w -= ox;

    int oy = 0;
    switch (alignV)
    {
    case TextAlignV::TOP:
        break;

    case TextAlignV::CENTER:
        oy = std::max<int>(0, size.h - th) >> 1;
        break;

    case TextAlignV::BOTTOM:
        oy = std::max<int>(0, size.h - th);
        break;
    };
    pos.y += oy;
    size.h -= oy;

    auto* fb = getFrameBuffer();
    fb->setWindow(pos.x, pos.y, size.w, size.h);

    //    DBOUT(("(%d, %d, %d, %d) %s\n", pos.x, pos.y, size.w, size.h, str));
    fm.setWindow(pos.x, pos.y, size.w, size.h);
    fm.setPosition(0, 0);
    fm.putString(str);
}

void
RenderContext::drawBits(
    Vec2 pos, int w, int h, const uint8_t* bits, uint32_t color)
{
    pos += getCurrentPosition();
    getFrameBuffer()->drawBits(pos.x, pos.y, w, h, bits, makeColor(color));
}

void
RenderContext::fill(Vec2 pos, const Dim2& size, uint32_t color)
{
    pos += getCurrentPosition();
    getFrameBuffer()->fill(pos.x, pos.y, size.w, size.h, makeColor(color));
}

void
RenderContext::drawRect(Vec2 pos, const Dim2& size, uint32_t color)
{
    pos += getCurrentPosition();
    auto* fb = getFrameBuffer();
    auto c   = makeColor(color);
    auto x0  = pos.x;
    auto x1  = pos.x + size.w - 1;
    auto y0  = pos.y;
    auto y1  = pos.y + size.h - 1;
    fb->fill(x0, y0, size.w, 1, c);
    fb->fill(x0, y0, 1, size.h, c);
    fb->fill(x1, y0, 1, size.h, c);
    fb->fill(x0, y1, size.w, 1, c);
}

void
RenderContext::blit(Vec2 pos, const graphics::FrameBufferBase& fb)
{
    pos += getCurrentPosition();
    getFrameBuffer()->blit(fb, pos.x, pos.y);
}

} // namespace ui
