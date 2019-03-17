/*
 * author : Shuichi TAKANO
 * since  : Fri Mar 08 2019 2:16:17
 */

#include "simple_list.h"
#include "context.h"
#include "window_setting.h"

namespace ui
{

namespace
{

constexpr Dim2 widgetSize_ = {WindowSettings::SCREEN_WIDTH,
                              WindowSettings::SCREEN_HEIGHT -
                                  WindowSettings::TITLE_BAR_HEIGHT};
constexpr Dim2 listSize_   = {
    WindowSettings::SCREEN_WIDTH - WindowSettings::SCROLL_BAR_WIDTH, 12};

constexpr uint32_t itemTextColor_  = 0xe0e0e0;
constexpr uint32_t valueTextColor_ = 0xffffff;

} // namespace

SimpleList::SimpleList()
{
    setDirectionIsVertical(true);
}

Dim2
SimpleList::getSize() const
{
    return widgetSize_;
}

uint16_t
SimpleList::getBaseItemSize() const
{
    return listSize_.h;
}

size_t
SimpleList::getWidgetCount() const
{
    return items_.size();
}

const Widget*
SimpleList::getWidget(size_t i) const
{
    return const_cast<SimpleList*>(this)->items_[i];
}

Widget*
SimpleList::getWidget(size_t i)
{
    return items_[i];
}

void
SimpleList::append(Item* item)
{
    items_.push_back(item);
}

void
SimpleList::clear()
{
    items_.clear();
}

/////////

Dim2
SimpleList::Item::getSize() const
{
    return listSize_;
}

void
SimpleList::Item::onRender(RenderContext& ctx)
{
    if (needRefresh_ || ctx.isInvalidated(listSize_))
    {
        auto& tmpFB = ctx.getTemporaryFrameBuffer(listSize_);
        {
            auto recoverFB = ctx.updateFrameBuffer(&tmpFB);

            auto& ws      = ctx.getWindowSettings();
            bool selected = ctx.getSelectIndex() == ctx.getCurrentIndex();
            auto col =
                makeColor(tmpFB, selected ? ws.highlightedBGColor : ws.bgColor);

            ctx.applyClipRegion();
            tmpFB.fill(col);

            _render(ctx);
        }

        ctx.applyClipRegion();
        ctx.put({0, 0}, tmpFB);

        needRefresh_ = false;
    }
}

/////

void
SimpleList::ItemWithTitle::_render(RenderContext& ctx)
{
    auto& fm                   = ctx.getFontManager();
    static constexpr Vec2 pos  = {24, 2};
    static constexpr Dim2 size = {listSize_.w - pos.x - 8, 8};

    fm.setEdgedMode(false);
    fm.setTransparentMode(true);
    ctx.setFontColor(itemTextColor_);
    ctx.putText(getTitle().c_str(), pos, size, TextAlignH::LEFT);
}

/////

void
SimpleList::ItemWithValue::_render(RenderContext& ctx)
{
    super::_render(ctx);

    static constexpr Vec2 pos  = {24, 2};
    static constexpr Dim2 size = {listSize_.w - pos.x - 8, 8};

    ctx.setFontColor(valueTextColor_);
    ctx.putText(getValue().c_str(), pos, size, TextAlignH::RIGHT);
}

} // namespace ui
