/*
 * author : Shuichi TAKANO
 * since  : Sat Feb 09 2019 17:53:18
 */

#include "scroll_list.h"
#include "context.h"

namespace ui
{

void
ScrollList::onUpdate(UpdateContext& ctx)
{
}

void
ScrollList::onRender(RenderContext& ctx)
{
    // scrollbarを描く

    auto recoverSelectIndex = ctx.updateSelectIndex(selectIndex_);

    auto n = getWidgetCount();
    Vec2 p = {0, 0};
    if (vertical_)
    {
        p.y = displayOffset_;
    }
    else
    {
        p.x = displayOffset_;
    }

    int step = getBaseItemSize();
    if (step)
    {
        auto regionSize = vertical_ ? getSize().h : getSize().w;

        auto idx    = -displayOffset_ / step;
        auto idxEnd = (-displayOffset_ + regionSize + step - 1) / step;
        if (idxEnd > n)
        {
            idxEnd = n;
        }

        auto recoverIndex = ctx.makeRestoreIndex();
        for (; idx < idxEnd; ++idx)
        {
            auto recoverPos = ctx.updatePosition(p);
            ctx.setCurrentIndex(idx);
            getWidget(idx)->onRender(ctx);

            if (vertical_)
            {
                p.y += step;
            }
            else
            {
                p.x += step;
            }
        }
    }
    else
    {
        // todo: アイテムごとの大きさが違う場合
    }
}

} // namespace ui
