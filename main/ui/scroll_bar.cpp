/*
 * author : Shuichi TAKANO
 * since  : Mon Feb 11 2019 6:34:45
 */

#include "scroll_bar.h"
#include <algorithm>

namespace ui
{

namespace
{
constexpr int minBarSize_ = 2;
}

void
drawVScrollBar(
    RenderContext& ctx, Dim2 size, int barPos, int barSize, int regionSize)
{
    auto bw = WindowSettings::SCROLL_BAR_WIDTH;
    _drawVScrollBar(
        ctx, {int(size.w - bw), 0}, {bw, size.h}, barPos, barSize, regionSize);
}

void
_drawVScrollBar(RenderContext& ctx,
                Vec2 pos,
                Dim2 size,
                int barPos,
                int barSize,
                int regionSize)
{
    int p0 = std::max<int>(0, barPos * size.h / regionSize);
    int p1 =
        std::min<int>(size.h - 1, (barPos + barSize) * size.h / regionSize);
    if (p1 - p0 < minBarSize_)
    {
        p1 = p0 + minBarSize_;
        if (p1 >= size.h)
        {
            p1 = size.h - 1;
            p0 = std::min(0, p0 - minBarSize_);
        }
    }

    auto& ws = ctx.getWindowSettings();
    ctx.fill(pos, {size.w, (uint32_t)p0}, ws.scrollBarBGColor);
    ctx.fill({pos.x, p0}, {size.w, (uint32_t)(p1 - p0)}, ws.scrollBarColor);
    ctx.fill({pos.x, p1}, {size.w, size.h - p1}, ws.scrollBarBGColor);
}

} // namespace ui
