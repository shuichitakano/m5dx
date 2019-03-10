/*
 * author : Shuichi TAKANO
 * since  : Sun Mar 10 2019 13:38:6
 */

#include "dialog.h"
#include "button_tip.h"
#include "context.h"
#include "key.h"
#include "strings.h"

namespace ui
{

Dialog::Dialog(const std::string& title, Dim2 size)
{
    setTitle(title);
    enableFrame();

    client_.size_ = size;
    setChild(&client_);
}

void
Dialog::setMessage(const std::string& str)
{
    client_.message_ = str;
    client_.touch();
}

void
Dialog::appendButton(const std::string& str, const Func& action)
{
    client_.buttons_.push_back({str, action});
    client_.touch();
}

/////

void
Dialog::Client::onUpdate(UpdateContext& ctx)
{
    if (buttons_.empty())
    {
        return;
    }

    auto* key = ctx.getKeyState();
    if (key && ctx.isEnableInput())
    {
        ctx.disableInput();

        if (auto* bt = ctx.getButtonTip())
        {
            bt->set(0, get(strings::up));
            bt->set(1, get(strings::select));
            bt->set(2, get(strings::down));
        }

        size_t n = buttons_.size();
        int dial = key->getDial();

        if ((key->isTrigger(0) || dial > 0) && currentIndex_ > 0)
        {
            --currentIndex_;
            needRefreshButton_ = true;
        }
        if ((key->isTrigger(2) || dial < 0) && currentIndex_ < n - 1)
        {
            ++currentIndex_;
            needRefreshButton_ = true;
        }
        if (key->isReleaseEdge(1) || key->isReleaseEdge(3))
        {
            auto& b = buttons_[currentIndex_];
            if (b.action_)
            {
                b.action_(ctx);
            }
            ctx.popManagedUI();
        }
    }
}

void
Dialog::Client::onRender(RenderContext& ctx)
{
    bool redraw    = needRefresh_ || ctx.isInvalidated(size_);
    const auto& ws = ctx.getWindowSettings();
    auto& fm       = ctx.getFontManager();

    ctx.applyClipRegion();
    ctx.updateInvalidatedRegion(size_);

    static constexpr int buttonHeight = 24;

    Vec2 messagePos  = {0, 0};
    Dim2 messageSize = {size_.w, size_.h - buttonHeight};

    if (redraw)
    {
        ctx.fill(messagePos, messageSize, ws.dialogBGColor);
        fm.setEdgedMode(false);
        fm.setTransparentMode(true);
        ctx.setFontColor(ws.dialogMessageColor);
        ctx.putText(message_.c_str(),
                    messagePos,
                    messageSize,
                    TextAlignH::CENTER,
                    TextAlignV::CENTER);
    }

    if (redraw || needRefreshButton_)
    {
        fm.setEdgedMode(true);
        ctx.setFontColor(ws.dialogButtonTextColor);
        ctx.setFontBGColor(ws.dialogButtonTextEdgeColor);

        size_t n    = buttons_.size();
        Dim2 size   = {(size_.w + n - 1) / n, buttonHeight};
        auto& tmpFB = ctx.getTemporaryFrameBuffer(size);

        for (size_t i = 0; i < buttons_.size(); ++i)
        {
            Vec2 pos         = {int(i * size_.w / n), int(messageSize.h)};
            auto restorePos  = ctx.updatePosition(pos);
            auto restoreClip = ctx.updateClipRegion(size);
            {
                auto recoverFB = ctx.updateFrameBuffer(&tmpFB);
                ctx.applyClipRegion();

                bool selected = i == currentIndex_;
                ctx.fill({0, 0},
                         size,
                         selected ? ws.dialogButtonSelectColor
                                  : ws.dialogBGColor);

                ctx.putText(buttons_[i].text_.c_str(),
                            {0, 0},
                            size,
                            TextAlignH ::CENTER,
                            TextAlignV::CENTER);
            }

            ctx.applyClipRegion();
            ctx.put({0, 0}, tmpFB);
        }

        needRefreshButton_ = false;
    }

    needRefresh_ = false;
}

} // namespace ui
