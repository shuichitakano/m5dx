/*
 * author : Shuichi TAKANO
 * since  : Sat Feb 09 2019 17:53:18
 */

#include "scroll_list.h"
#include "context.h"
#include "key.h"
#include <debug.h>

namespace ui
{

void
ScrollList::onUpdate(UpdateContext& ctx)
{
    auto n    = getWidgetCount();
    auto* key = ctx.getKeyState();
    if (key)
    {
        if (key->isTrigger(0) && n)
        {
            getWidget(selectIndex_)->touch();
            --selectIndex_;
            if (selectIndex_ < 0)
            {
                selectIndex_ = n - 1;
            }
            getWidget(selectIndex_)->touch();
        }
        else if (key->isTrigger(2) && n)
        {
            getWidget(selectIndex_)->touch();
            ++selectIndex_;
            if (selectIndex_ >= n)
            {
                selectIndex_ = 0;
            }
            getWidget(selectIndex_)->touch();
        }
        else if (key->isLongPress(1))
        {
            DBOUT(("exit.\n"));
        }
        else if (key->isReleaseEdge(1))
        {
            DBOUT(("decide!\n"));
            needRefresh_ = true;
        }
    }

    auto prevDispOfs = displayOffset_;
    int itemSize     = getBaseItemSize();
    auto widgetSize  = getSize();
    int dispSize     = vertical_ ? widgetSize.h : widgetSize.w;
    int absPos       = itemSize * selectIndex_;
    int pos          = absPos + displayOffset_;
    int posTail      = pos + itemSize;
    if (pos < 0)
    {
        displayOffset_ = -absPos;
    }
    else if (posTail > dispSize)
    {
        displayOffset_ = dispSize - absPos - itemSize;
    }

    //    DBOUT(("ofs %d, pos %d, size %d\n", displayOffset_, pos, dispSize));

    if (prevDispOfs != displayOffset_)
    {
        needRefresh_ = true;
    }
}

void
ScrollList::onRender(RenderContext& ctx)
{
    // todo: scrollbarを描く

    if (needRefresh_)
    {
        ctx.updateInvalidatedRegion(getSize());
        needRefresh_ = false;
    }

    auto recoverSelectIndex = ctx.updateSelectIndex(selectIndex_);

    auto n                  = getWidgetCount();
    Vec2 p                  = {0, 0};
    (vertical_ ? p.y : p.x) = displayOffset_;

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

        (vertical_ ? p.y : p.x) += step * idx;

        auto recoverIndex = ctx.makeRestoreIndex();
        for (; idx < idxEnd; ++idx)
        {
            auto recoverPos = ctx.updatePosition(p);
            ctx.setCurrentIndex(idx);
            getWidget(idx)->onRender(ctx);

            (vertical_ ? p.y : p.x) += step;
        }
    }
    else
    {
        // todo: アイテムごとの大きさが違う場合
    }
}

} // namespace ui
