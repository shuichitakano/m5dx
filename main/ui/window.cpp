/*
 * author : Shuichi TAKANO
 * since  : Fri Feb 01 2019 2:53:43
 */

#include "window.h"
#include "context.h"
#include <debug.h>

namespace ui
{

Window::Window(const std::string& title, Widget* w)
{
    setTitle(title);
    setChild(w);
}

void
Window::setChild(Widget* w, bool fitToClient)
{
    child_ = w;
    if (w && fitToClient)
    {
        size_ = w->getSize();
        if (hasFrame_)
        {
            size_.w += 2;
            size_.h += 1;
        }
        size_.h += WindowSettings::TITLE_BAR_HEIGHT;
    }

    needRefresh_ = true;
}

void
Window::setTitle(const std::string& str)
{
    //    DBOUT(("title = %s\n", str.c_str());
    title_       = str;
    needRefresh_ = true;
}

void
Window::enableFrame(bool f)
{
    hasFrame_    = f;
    needRefresh_ = true;
}

void
Window::onUpdate(UpdateContext& ctx)
{
    if (child_)
    {
        child_->onUpdate(ctx);
    }
}

void
Window::onRender(RenderContext& ctx)
{
    Vec2 childPos  = {0, 0};
    Dim2 childSize = size_;

    auto& ws      = ctx.getWindowSettings();
    bool needDraw = needRefresh_ || ctx.isInvalidated(size_);

    ctx.applyClipRegion();

    if (hasFrame_)
    {
        if (needDraw)
        {
            ctx.drawRect({0, 0}, size_, ws.frameColor);
        }
        childPos.x += 1;
        childSize.w -= 2;
        childSize.h -= 1;
    }

    if (needDraw)
    {
        ctx.fill(childPos,
                 {childSize.w, WindowSettings::TITLE_BAR_HEIGHT - childPos.y},
                 ws.titleBarColor);

        Vec2 titlePos  = {2, 2};
        Dim2 titleSize = {size_.w - 4, 8};

        auto& fm = ctx.getFontManager();
        fm.setEdgedMode(true);
        fm.setTransparentMode(true);
        ctx.setFontColor(ws.titleTextColor);
        ctx.setFontBGColor(ws.titleTextEdgeColor);
        ctx.putText(title_.c_str(), titlePos, titleSize);
    }

    childPos.y += WindowSettings::TITLE_BAR_HEIGHT;
    childSize.h -= WindowSettings::TITLE_BAR_HEIGHT;

    if (child_)
    {
        auto restorePos  = ctx.updatePosition(childPos);
        auto restoreClip = ctx.updateClipRegion(childSize);

        child_->onRender(ctx);
    }

    needRefresh_ = false;
}

} // namespace ui
