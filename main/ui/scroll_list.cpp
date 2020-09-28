/*
 * author : Shuichi TAKANO
 * since  : Sat Feb 09 2019 17:53:18
 */

#include "scroll_list.h"
#include "button_tip.h"
#include "context.h"
#include "key.h"
#include "scroll_bar.h"
#include "strings.h"
#include <debug.h>
#include <mutex>

namespace ui
{

ScrollList::ScrollList()
{
    decideText_ = get(strings::select); // default
}

void
ScrollList::touchSelectWidget()
{
    if (selectIndex_ >= 0)
    {
        auto w = getWidget(selectIndex_);
        if (w)
        {
            w->touch();
        }
    }
}

void
ScrollList::setIndex(int i)
{
    std::lock_guard<sys::Mutex> lock(getMutex());
    if (i >= -1 && i < static_cast<int>(getWidgetCount()))
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

        if (enableButton_)
        {
            if (auto* bt = ctx.getButtonTip())
            {
                bt->set(0, get(strings::up));
                bt->set(1, decideText_);
                bt->set(2, get(strings::down));
            }
        }

        auto bt = [&](bool f) { return enableButton_ ? f : false; };
        auto dl = [&](bool f) { return enableDial_ ? f : false; };

        int dial = enableDial_ ? key->getDial() : 0;
        if (bt(key->isTrigger(0)) || dial > 0)
        {
            std::lock_guard<sys::Mutex> lock(getMutex());
            selectChanged();
            auto n = getWidgetCount();
            if (n)
            {
                touchSelectWidget();
                int d = dial ? dial : 1;
                selectIndex_ -= d;
                if (selectIndex_ < (enableNoSelect_ ? -1 : 0))
                {
                    selectIndex_ = n - 1;
                }
                touchSelectWidget();
            }
        }
        else if (bt(key->isTrigger(2)) || dial < 0)
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
                    selectIndex_ = enableNoSelect_ ? -1 : 0;
                }
                touchSelectWidget();
            }
        }
        else if (bt(key->isLongPressEdge(1)) || dl(key->isLongPressEdge(3)))
        {
            if (longPressFunc_ && selectIndex_ >= 0)
            {
                DBOUT(("long press %d\n", selectIndex_));
                ctx.acceptLongPress();
                longPressFunc_(ctx, selectIndex_);
            }
        }
        else if (bt(key->isReleaseEdge(1)) || dl(key->isReleaseEdge(3)))
        {
            if (decideFunc_ && selectIndex_ >= 0)
            {
                DBOUT(("decide %d\n", selectIndex_));
                decideFunc_(ctx, selectIndex_);
            }
        }
    }

    auto prevDispOfs = displayOffset_;
    int itemSize     = getBaseItemSize();
    auto widgetSize  = getSize();
    int dispSize     = vertical_ ? widgetSize.h : widgetSize.w;
    int absPos       = itemSize * std::max(0, selectIndex_);
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

    ctx.applyClipRegion();

    auto wsize = getSize();
    auto& ws   = ctx.getWindowSettings();

    {
        // listのあまり領域
        Vec2 p{0, int(n * step + displayOffset_)};
        Dim2 s{wsize.w -
                   (needScrollBar_ ? WindowSettings::SCROLL_BAR_WIDTH : 0),
               wsize.h - p.y};
        if (p.y < wsize.h && (needRefresh_ || ctx.isInvalidated(p, s)))
        {
            ctx.fill(p, s, ws.borderColor);
        }
    }

    if (needScrollBar_)
    {
        // スクロールバー
        drawVScrollBar(ctx,
                       wsize,
                       -displayOffset_,
                       wsize.h,
                       step * n,
                       needRefreshScrollBar_ || needRefresh_);
        needRefreshScrollBar_ = false;
    }
    needRefresh_ = false;
}

} // namespace ui
