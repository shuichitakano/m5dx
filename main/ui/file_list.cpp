/*
 * author : Shuichi TAKANO
 * since  : Sat Feb 09 2019 22:52:37
 */

#include "file_list.h"
#include "context.h"
#include <algorithm>
#include <debug.h>
#include <dirent.h>
#include <io/file_util.h>
#include <music_player/music_player_manager.h>
#include <mutex>
#include <sys/job_manager.h>

namespace ui
{

namespace
{
constexpr Dim2 widgetSize_ = {320, 232 - WindowSettings::TITLE_BAR_HEIGHT};
constexpr Dim2 listSize_   = {320 - WindowSettings::SCROLL_BAR_WIDTH, 24};
} // namespace

FileList::FileList()
{
    setDirectionIsVertical(true);
}

FileList::~FileList()
{
    cancelAndWaitIdle();
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
FileList::cancelAndWaitIdle()
{
    abortReq_ = true;
    sys::getDefaultJobManager().waitIdle();
    abortReq_ = false;
}

#if 0
std::string
FileList::getParentDir()
{
    std::lock_guard<sys::Mutex> lock(getMutex());

    auto p = path_.rfind('/');
    if (p != std::string::npos)
    {
        return p == 0 ? "/" : path_.substr(0, p);
    }
    return path_;
}
#endif

std::pair<std::string, std::string>
FileList::getSeparatePath()
{
    std::lock_guard<sys::Mutex> lock(getMutex());
    auto path = getPath();

    std::string parent;
    std::string body;
    auto pos = path.rfind('/');
    if (pos != std::string::npos)
    {
        body = path.substr(pos + 1);
        if (pos == 0)
        {
            parent = "/";
        }
        else
        {
            parent = path.substr(0, pos);
        }
    }
    else
    {
        parent = "/";
        body   = path;
    }
    return {parent, body};
}

void
FileList::setPath(const std::string& path)
{
    cancelAndWaitIdle();
    std::lock_guard<sys::Mutex> lock(getMutex());

    path_       = path;
    parseIndex_ = 0;
    abortReq_   = false;

    directories_.clear();
    files_.clear();

    if (path == "/")
    {
        isRootDir_ = true;
    }
    else
    {
        directories_.emplace_back("..");
        isRootDir_ = false;
    }
#if 0
    sys::getDefaultJobManager().add([this] { loadFileList(); });
#else
    loadFileListDirect();
#endif
}

namespace
{

struct NameLess
{
    bool cmp(const std::string& a, const std::string& b)
    {
        // todo: sjis の比較を考慮する
        return strcasecmp(a.c_str(), b.c_str()) < 0;
    }

    bool operator()(const FileList::Directory& a, const FileList::Directory& b)
    {
        return cmp(a.name_, b.name_);
    }
    bool operator()(const FileList::Directory& a, const std::string& b)
    {
        return cmp(a.name_, b);
    }
    bool operator()(const std::string& a, const FileList::Directory& b)
    {
        return cmp(a, b.name_);
    }

    bool operator()(const FileList::File& a, const FileList::File& b)
    {
        return cmp(a.filename_, b.filename_);
    }
    bool operator()(const FileList::File& a, const std::string& b)
    {
        return cmp(a.filename_, b);
    }
    bool operator()(const std::string& a, const FileList::File& b)
    {
        return cmp(a, b.filename_);
    }
};

} // namespace

void
FileList::loadFileListDirect()
{
    DBOUT(("open path = %s\n", path_.c_str()));
    auto dir = opendir(path_.c_str());
    if (!dir)
    {
        DBOUT(("open file list error: %s.\n", path_.c_str()));
        return;
    }

    while (auto e = readdir(dir))
    {
        auto name = e->d_name;

        if (!name || name[0] == '.')
        {
            continue;
        }

        if (e->d_type & DT_DIR)
        {
            directories_.emplace_back(name);
        }
        else
        {
            if (auto p = music_player::findMusicPlayerFromFile(name))
            {
                files_.emplace_back(name, -1, p->getFormat());
            }
        }
    }

    {
        auto p = directories_.begin();
        if (isRootDir_)
        {
            ++p;
        }
        std::sort(p, directories_.end(), NameLess());
    }
    std::sort(files_.begin(), files_.end(), NameLess());

    [&] {
        for (auto p = directories_.begin(); p != directories_.end(); ++p)
        {
            if (p->name_ == followFile_)
            {
                setIndex(p - directories_.begin());
                return;
            }
        }
        for (auto p = files_.begin(); p != files_.end(); ++p)
        {
            if (p->filename_ == followFile_)
            {
                setIndex((p - files_.begin()) + directories_.size());
                return;
            }
        }
    }();
    followFile_.clear();
}

void
FileList::loadFileList()
{
    DBOUT(("open path = %s\n", path_.c_str()));
    auto dir = opendir(path_.c_str());
    if (!dir)
    {
        DBOUT(("open file list error: %s.\n", path_.c_str()));
        return;
    }

    while (auto e = readdir(dir))
    {
        if (abortReq_)
        {
            break;
        }

        auto name = e->d_name;

        if (!name || name[0] == '.')
        {
            continue;
        }

        if (e->d_type & DT_DIR)
        {
            appendDirectory(name);
        }
        else
        {
            if (auto p = music_player::findMusicPlayerFromFile(name))
            {
                appendFile(name, -1, p->getFormat());
            }
        }
    }
}

void
FileList::appendDirectory(std::string&& name)
{
    std::lock_guard<sys::Mutex> lock(getMutex());
    auto begin = directories_.begin();
    auto end   = directories_.end();
    if (isRootDir_ && begin != end)
    {
        ++begin;
    }
    auto p = std::upper_bound(begin, end, name, NameLess());

    int idx = int(p - directories_.begin());
    listInserted(idx);

    bool isFollowFile = followFile_ == name;
    directories_.emplace(p, std::move(name));
    if (isFollowFile)
    {
        setIndex(idx);
        followFile_.clear();
    }
}

void
FileList::appendFile(std::string&& name,
                     size_t size,
                     music_player::FileFormat fmt)
{
    std::lock_guard<sys::Mutex> lock(getMutex());
    auto p = std::upper_bound(files_.begin(), files_.end(), name, NameLess());

    int idx = int(p - files_.begin()) + directories_.size();
    listInserted(idx);

    bool isFollowFile = followFile_ == name;
    files_.emplace(p, std::move(name), size, fmt);
    if (isFollowFile)
    {
        setIndex(idx);
        followFile_.clear();
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
            std::lock_guard<sys::Mutex> lock(getMutex());
            auto& f = files_[i];
            auto fn = makeAbsPath(f.filename_);
            if (auto p = music_player::findMusicPlayerFromFile(fn.c_str()))
            {
                if (auto r = p->loadTitle(fn.c_str()))
                {
                    f.title_ = r.value();
                    f.size_  = io::getFileSize(fn.c_str());
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

    if (size_ >= 0)
    {
        char buf[32];
        snprintf(buf, sizeof(buf), "%zd bytes", size_);
        ctx.setFontColor(fileSizeColor);
        ctx.putText(buf, fileSizePos, fileSizeSize, TextAlignH::RIGHT);
    }
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
    if (name_.length() < 36)
    {
        fm.setAKConvertMode(true);
    }

    ctx.setFontColor(nameColor);
    ctx.putText(name_.c_str(), namePos, nameSize, TextAlignH::LEFT);

    fm.setAKConvertMode(false);
}

} // namespace ui
