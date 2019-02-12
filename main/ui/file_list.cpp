/*
 * author : Shuichi TAKANO
 * since  : Sat Feb 09 2019 22:52:37
 */

#include "file_list.h"
#include "context.h"
#include <SD.h>
#include <debug.h>
#include <music_player/music_player_manager.h>
#include <sys/job_manager.h>

namespace ui
{

namespace
{
constexpr Dim2 widgetSize_ = {320, 240 - WindowSettings::TITLE_BAR_HEIGHT};
constexpr Dim2 listSize_   = {320 - WindowSettings::SCROLL_BAR_WIDTH, 24};
} // namespace

FileList::FileList()
{
    setDirectionIsVertical(true);
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

std::pair<std::string, bool>
FileList::getItem(size_t i)
{
    if (i < directories_.size())
    {
        return {directories_[i].name_, true};
    }

    i -= directories_.size();
    if (i < files_.size())
    {
        return {files_[i].filename_, false};
    }
    return {};
}

void
FileList::setPath(const std::string& path)
{
    // 排他は上流で確保する

    sys::getDefaultJobManager().waitIdle();

    path_       = path;
    parseIndex_ = 0;

    directories_.clear();
    files_.clear();

    DBOUT(("open path = %s\n", path.c_str()));
    auto root = SD.open(path.c_str());
    if (!root || !root.isDirectory())
    {
        DBOUT(("open file list error: %s.\n", path.c_str()));
        return;
    }

    if (path != "/")
    {
        directories_.emplace_back("..");
    }

    while (auto file = root.openNextFile())
    {
        auto name = strrchr(file.name(), '/');
        //        DBOUT(("name = %s\n", name));
        if (name)
        {
            ++name;
        }

        if (!name || name[0] == '.')
        {
            continue;
        }

        if (file.isDirectory())
        {
            directories_.emplace_back(name);
        }
        else
        {
            if (auto p = music_player::findMusicPlayerFromFile(name))
            {
                files_.emplace_back(name, file.size(), p->getFormat());
            }
        }
    }
}

void
FileList::onUpdate(UpdateContext& ctx)
{
    super::onUpdate(ctx);

    auto& jm = sys::getDefaultJobManager();
    if (jm.isIdle() && parseIndex_ < files_.size())
    {
        int i = parseIndex_;
        jm.add([this, i] {
            auto& f = files_[i];
            auto fn = makeAbsPath(f.filename_);
            if (auto p = music_player::findMusicPlayerFromFile(fn.c_str()))
            {
                if (auto r = p->loadTitle(fn.c_str()))
                {
                    f.title_ = r.value();
                    f.touch();
                }
            }
        });
        ++parseIndex_;
    }
}

std::string
FileList::makeAbsPath(const std::string& name) const
{
    if (path_ == "/")
    {
        return path_ + name;
    }
    else
    {
        return path_ + "/" + name;
    }
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

        ctx.applyClipRegion();
        ctx.blit({0, 0}, tmpFB);

        updated_ = false;
        return;
    }
}

////
void
FileList::File::_render(RenderContext& ctx)
{
    auto& fm = ctx.getFontManager();

    // ♫ タイトル
    // hoge.mdx      12345 bytes MDX

    static constexpr int W             = listSize_.w - 2;
    static constexpr int line0Y        = 3;
    static constexpr int line1Y        = 13;
    static constexpr Vec2 titlePos     = {14, line0Y};
    static constexpr Dim2 titleSize    = {W - titlePos.x, 8};
    static constexpr Dim2 formatSize   = {5 * 4, 8};
    static constexpr Vec2 formatPos    = {W - formatSize.w, line1Y};
    static constexpr Dim2 fileSizeSize = {16 * 4, 8};
    static constexpr Vec2 fileSizePos  = {formatPos.x - fileSizeSize.w, line1Y};
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
    ctx.drawBits(iconPos, 8, 8, icon, iconColor);

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
    ctx.drawBits(iconPos, 8, 8, icon, iconColor);

    fm.setEdgedMode(false);
    fm.setTransparentMode(true);

    ctx.setFontColor(nameColor);
    ctx.putText(name_.c_str(), namePos, nameSize, TextAlignH::LEFT);
}

} // namespace ui
