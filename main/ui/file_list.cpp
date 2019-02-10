/*
 * author : Shuichi TAKANO
 * since  : Sat Feb 09 2019 22:52:37
 */

#include "file_list.h"
#include "context.h"

namespace ui
{

namespace
{
constexpr Dim2 widgetSize_ = {320, 240};
constexpr Dim2 listSize_   = {320, 24};
} // namespace

FileList::FileList()
{
    setDirectionIsVertical(true);

    directories_.resize(2);
    files_.resize(3);
}

Dim2
FileList::getSize() const
{
    return widgetSize_;
}

uint16_t
FileList::getBaseItemSize() const
{
    return listSize_.h;
}

size_t
FileList::getWidgetCount() const
{
    return directories_.size() + files_.size();
}

const Widget*
FileList::getWidget(size_t i) const
{
    return const_cast<FileList*>(this)->_getWidget(i);
}

Widget*
FileList::getWidget(size_t i)
{
    return _getWidget(i);
}

Widget*
FileList::_getWidget(size_t i)
{
    if (i < directories_.size())
    {
        return &directories_[i];
    }

    i -= directories_.size();
    if (i < files_.size())
    {
        return &files_[i];
    }

    return nullptr;
}

////
Dim2
FileList::Item::getSize() const
{
    return listSize_;
}

void
FileList::Item::onRender(RenderContext& ctx)
{
    if (updated_ || ctx.isInvalidated(listSize_))
    {
        auto& tmpFB = ctx.getTemporaryFrameBuffer(listSize_, 8);
        {
            auto recoverFB = ctx.updateFrameBuffer(&tmpFB);

            auto& ws      = ctx.getWindowSettings();
            bool selected = ctx.getSelectIndex() == ctx.getCurrentIndex();
            auto col =
                makeColor(tmpFB, selected ? ws.highlightedBGColor : ws.bgColor);

            ctx.applyClipRegion();
            tmpFB.fill(col);

            _render(ctx);
            ctx.applyClipRegion();
        }

        auto* fb  = ctx.getFrameBuffer();
        auto& pos = ctx.getCurrentPosition();
        ctx.applyClipRegion();
        fb->blit(tmpFB, pos.x, pos.y);

        updated_ = false;
        return;
    }
}

////
void
FileList::File::_render(RenderContext& ctx)
{
    auto& fm = ctx.getFontManager();

    // ♫ タイトル             MDX
    // hoge.mdx      12345 bytes

    static constexpr int W             = listSize_.w;
    static constexpr int line0Y        = 3;
    static constexpr int line1Y        = 13;
    static constexpr Dim2 formatSize   = {5 * 4, 8};
    static constexpr Vec2 formatPos    = {W - formatSize.w - 2, line0Y};
    static constexpr Vec2 titlePos     = {16, line0Y};
    static constexpr Dim2 titleSize    = {formatPos.x - titlePos.x, 8};
    static constexpr Dim2 fileSizeSize = {16 * 4, 8};
    static constexpr Vec2 fileSizePos  = {W - fileSizeSize.w - 2, line1Y};
    static constexpr Vec2 fileNamePos  = {2, line1Y};
    static constexpr Dim2 fileNameSize = {fileSizePos.x - fileNamePos.x, 8};
    static constexpr Vec2 iconPos      = {2, line0Y};

    static constexpr uint32_t titleColor    = 0xffffff;
    static constexpr uint32_t formatColor   = 0x606060;
    static constexpr uint32_t fileNameColor = 0x000000;
    static constexpr uint32_t fileSizeColor = 0x000000;
    static constexpr uint32_t iconColor     = 0xffffff;

    uint8_t icon[] = {
        0b00011000,
        0b00010100,
        0b00010010,
        0b00010010,
        0b01110010,
        0b11110010,
        0b01100000,
        0b00000000,
    };
    ctx.drawBits(iconPos, 8, 8, icon, ctx.makeColor(iconColor));

    fm.setEdgedMode(false);
    fm.setTransparentMode(true);

    ctx.setFontColor(titleColor);
    ctx.putText(title_.c_str(), titlePos, titleSize, TextAlignH::LEFT);

    auto formatStr = music_player::getFileFormatString(format_);
    ctx.setFontColor(formatColor);
    ctx.putText(formatStr, formatPos, formatSize, TextAlignH::RIGHT);

    ctx.setFontColor(fileNameColor);
    ctx.putText(filename_.c_str(), fileNamePos, fileNameSize, TextAlignH::LEFT);

    char buf[32];
    snprintf(buf, sizeof(buf), "%zd bytes", size_);
    ctx.setFontColor(fileSizeColor);
    ctx.putText(buf, fileSizePos, fileSizeSize, TextAlignH::RIGHT);
}
////
void
FileList::Directory::_render(RenderContext& ctx)
{
    auto& fm = ctx.getFontManager();

    // 📁 Directory
    static constexpr int W         = listSize_.w;
    static constexpr Vec2 namePos  = {16, 8};
    static constexpr Dim2 nameSize = {W - namePos.x - 2, 8};
    static constexpr Vec2 iconPos  = {2, 8};

    static constexpr uint32_t nameColor = 0xc0c0c0;
    static constexpr uint32_t iconColor = 0xc0c0c0;

    uint8_t icon[] = {
        0b11110000,
        0b10001000,
        0b11111111,
        0b10000001,
        0b10000001,
        0b10000001,
        0b11111111,
        0b00000000,
    };
    ctx.drawBits(iconPos, 8, 8, icon, ctx.makeColor(iconColor));

    fm.setEdgedMode(false);
    fm.setTransparentMode(true);

    ctx.setFontColor(nameColor);
    ctx.putText(name_.c_str(), namePos, nameSize, TextAlignH::LEFT);
}

} // namespace ui
