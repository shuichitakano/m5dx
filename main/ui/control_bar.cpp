/*
 * author : Shuichi TAKANO
 * since  : Sun Mar 03 2019 2:29:36
 */

#include "control_bar.h"
#include "context.h"

namespace ui
{

namespace
{
constexpr Dim2 widgetSize_ = {320, 8};
}

void
ControlBar::onUpdate(UpdateContext& ctx)
{
}

void
ControlBar::onRender(RenderContext& ctx)
{
    if (!needRefresh_ && !ctx.isInvalidated(widgetSize_))
    {
        return;
    }

    auto& tmpFB = ctx.getTemporaryFrameBuffer(widgetSize_);
    {
        auto recoverFB = ctx.updateFrameBuffer(&tmpFB);

        //        auto& ws = ctx.getWindowSettings();

        ctx.applyClipRegion();
        tmpFB.fill(0);

        ctx.setFontColor(0xa0a0a0);
        auto& fm = ctx.getFontManager();
        fm.setEdgedMode(false);
        fm.setTransparentMode(true);

        static constexpr uint16_t positions[] = {53, 160, 267};
        static constexpr Dim2 size            = {100, 8};
        for (int i = 0; i < 3; ++i)
        {
            auto& st = state_[i];
            Vec2 pos = {int(positions[i] - (size.w >> 1)), 1};
            ctx.putText(st.text_.c_str(), pos, size, TextAlignH::CENTER);
        }
    }

    ctx.applyClipRegion();
    ctx.put({0, 0}, tmpFB);

    needRefresh_ = false;
}

Dim2
ControlBar::getSize() const
{
    return widgetSize_;
}

} // namespace ui
