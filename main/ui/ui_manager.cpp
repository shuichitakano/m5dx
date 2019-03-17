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
UIManager::push(const WidgetPtr& p, Vec2 pos)
{
    std::lock_guard<sys::Mutex> lock(mutex_);
    next_.push_back({pos, p});
}

void
UIManager::push(const WidgetPtr& p)
{
    std::lock_guard<sys::Mutex> lock(mutex_);
    auto size = p->getSize();
    Vec2 pos  = {int((WindowSettings::SCREEN_WIDTH - size.w) >> 1),
                int((WindowSettings::SCREEN_HEIGHT - size.h) >> 1)};
    DBOUT(("push (%d, %d)\n", pos.x, pos.y));
    next_.push_back({pos, p});
}

void
UIManager::pop()
{
    ++popReq_;
}

void
UIManager::checkRenderStartLV()
{
    auto n = layer_.size();
    if (!n)
    {
        renderStartLV_ = 0;
        return;
    }
    for (size_t i = n - 1; i > 1; --i)
    {
        auto& l = layer_[i];
        BBox bb(l.pos_, l.widget_->getSize());
        if (bb.p[0].x <= 0 && bb.p[0].y <= 0 &&
            bb.p[1].x >= WindowSettings::SCREEN_WIDTH &&
            bb.p[1].y >= WindowSettings::SCREEN_HEIGHT)
        {
            renderStartLV_ = i;
            return;
        }
    }
    renderStartLV_ = 0;
}

void
UIManager::update(UpdateContext& ctx)
{
    std::lock_guard<sys::Mutex> lock(mutex_);
    for (auto p = layer_.rbegin(); p != layer_.rend(); ++p)
    {
        p->widget_->onUpdate(ctx);
    }

    if (popReq_)
    {
        layer_.resize(layer_.size() - popReq_);
        popReq_  = 0;
        refresh_ = true;
        checkRenderStartLV();
    }

    if (!next_.empty())
    {
        layer_.insert(layer_.end(), next_.begin(), next_.end());
        checkRenderStartLV();
        next_.clear();
    }
}

void
UIManager::render(RenderContext& ctx)
{
    std::lock_guard<sys::Mutex> lock(mutex_);

    if (refresh_)
    {
        ctx.updateInvalidatedRegion({320, 240});
        refresh_ = false;
    }

    auto n = layer_.size();
    for (auto i = 0; i < n; ++i)
    {
        // 0番目はButton tipなので必ず描く
        // もうちょっとマシに書きたい
        if (i != 0 && i < renderStartLV_)
        {
            continue;
        }

        auto& l          = layer_[i];
        auto recoverPos  = ctx.updatePosition(l.pos_);
        auto recoverClip = ctx.updateClipRegion(l.widget_->getSize());
        l.widget_->onRender(ctx);
    }
}

} // namespace ui
