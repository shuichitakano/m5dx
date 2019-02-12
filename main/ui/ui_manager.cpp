/*
 * author : Shuichi TAKANO
 * since  : Mon Feb 11 2019 16:42:5
 */

#include "ui_manager.h"
#include "context.h"
#include "window_setting.h"
#include <debug.h>
#include <mutex>

namespace ui
{

void
UIManager::append(const WidgetPtr& p, Vec2 pos)
{
    std::lock_guard<sys::Mutex> lock(mutex_);

    layer_.push_back({pos, p});
    checkRenderStartLV();
}

void
UIManager::checkRenderStartLV()
{
    auto n = layer_.size();
    for (size_t i = 0; i < n; ++i)
    {
        auto& l = layer_[i];
        BBox bb(l.pos_, l.widget_->getSize());
        if (bb.p[0].x > 0 || bb.p[0].y > 0 ||
            bb.p[1].x < WindowSettings::SCREEN_WIDTH ||
            bb.p[1].y < WindowSettings::SCREEN_HEIGHT)
        {
            renderStartLV_ = i - 1;
            return;
        }
    }
    renderStartLV_ = n ? n - 1 : 0;
    DBOUT(("render start lv = %d\n", renderStartLV_));
}

void
UIManager::update(UpdateContext& ctx)
{
    std::lock_guard<sys::Mutex> lock(mutex_);
    for (auto p = layer_.rbegin(); p != layer_.rend(); ++p)
    {
        p->widget_->onUpdate(ctx);
    }
}

void
UIManager::render(RenderContext& ctx)
{
    std::lock_guard<sys::Mutex> lock(mutex_);
    auto n = layer_.size();
    for (auto i = renderStartLV_; i < n; ++i)
    {
        auto& l          = layer_[i];
        auto recoverPos  = ctx.updatePosition(l.pos_);
        auto recoverClip = ctx.updateClipRegion(l.widget_->getSize());
        l.widget_->onRender(ctx);
    }
}

} // namespace ui
