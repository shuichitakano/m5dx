/*
 * author : Shuichi TAKANO
 * since  : Sat Feb 09 2019 17:53:18
 */

#include "scroll_list.h"
#include "context.h"
#include "key.h"
#include "scroll_bar.h"
#include <debug.h>
#include <mutex>

namespace ui
{

void
ScrollList::touchSelectWidget()
{
    auto w = getWidget(selectIndex_);
    if (w)
    {
        w->touch();
    }
}

void
ScrollList::setIndex(int i)
{
    DBOUT(("setIndex(%d)\n", i));
    std::lock_guard<sys::Mutex> lock(getMutex());
    if (i >= 0 && i < getWidgetCount())
    {
        touchSelectWidget();
        selectIndex_ = i;
        touchSelectWidget();
    }
    needRefreshScrollBar_ = true;
}

void
ScrollList::listInserted(int i)
{
    int step = getBaseItemSize();
    if (i <= selectIndex_ && i < getWidgetCount())
    {
        ++selectIndex_;
        displayOffset_ -= step;
    }

    if (step)
    {
        auto ofs        = step * i + displayOffset_;
        auto regionSize = vertical_ ? getSize().h : getSize().w;
        if (ofs > -step && ofs < regionSize)
        {
            refresh();
        }
    }
    else
    {
        refresh();
    }

    needRefreshScrollBar_ = true;
}

void
ScrollList::onUpdate(UpdateContext& ctx)
{
    {
        std::lock_guard<sys::Mutex> lock(getMutex());
        auto n = getWidgetCount();
        for (int i = 0; i < n; ++i)
        {
            getWidget(i)->onUpdate(ctx);
        }
    }

    auto* key = ctx.getKeyState();
    if (key && ctx.isEnableInput())
    {
        ctx.disableInput();

        int dial = key->getDial();

        if (key->isTrigger(0) || dial > 0)
        {
            std::lock_guard<sys::Mutex> lock(getMutex());
            selectChanged();
            auto n = getWidgetCount();
            if (n)
            {
                touchSelectWidget();
                int d = dial ? dial : 1;
                selectIndex_ -= d;
                if (selectIndex_ < 0)
                {
                    selectIndex_ = n - 1;
                }
                touchSelectWidget();
            }
        }
        else if (key->isTrigger(2) || dial < 0)
        {
            std::lock_guard<sys::Mutex> lock(getMutex());
            selectChanged();
            auto n = getWidgetCount();
            if (n)
            {
                touchSelectWidget();
                int d = dial ? -dial : 1;
                selectIndex_ += d;
                if (selectIndex_ >= n)
                {
                    selectIndex_ = 0;
                }
                touchSelectWidget();
            }
        }
        else if (key->isLongPressEdge(1) || key->isLongPressEdge(3))
        {
            if (longPressFunc_)
            {
                DBOUT(("long press %d\n", selectIndex_));
                ctx.acceptLongPress();
                longPressFunc_(selectIndex_);
            }
        }
        else if (key->isReleaseEdge(1) || key->isReleaseEdge(3))
        {
            if (decideFunc_)
            {
                DBOUT(("decide %d\n", selectIndex_));
                decideFunc_(selectIndex_);
            }
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
    std::lock_guard<sys::Mutex> lock(getMutex());

    if (needRefresh_)
    {
        ctx.updateInvalidatedRegion(getSize());
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

    if (needRefresh_ || needRefreshScrollBar_)
    {
        ctx.applyClipRegion();

        auto wsize = getSize();
        auto& ws   = ctx.getWindowSettings();

        // listのあまり領域
        Vec2 p{0, int(n * step + displayOffset_)};
        if (p.y < wsize.h)
        {
            ctx.fill(
                p,
                {wsize.w - WindowSettings::SCROLL_BAR_WIDTH, wsize.h - p.y},
                ws.borderColor);
        }

        // スクロールバー
        drawVScrollBar(ctx, wsize, -displayOffset_, wsize.h, step * n);

        needRefreshScrollBar_ = false;
    }

    needRefresh_ = false;
}

} // namespace ui
